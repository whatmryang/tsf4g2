/** @file $RCSfile: tbusd.c,v $
  general description of this module
  $Id: tbusd.c,v 1.30 2009-11-09 05:54:28 jacky Exp $
  @author $Author: jacky $
  @date $Date: 2009-11-09 05:54:28 $
  @version $Revision: 1.30 $
  @note Editor: Vim 6.3, Gcc 4.0.2, tab=4
  @note Platform: Linux
  */

#include <assert.h>
#include <string.h>
#include <errno.h>

#include "tloghelp/tlogload.h"
#include "comm/tmempool.h"
#include "tdr/tdr.h"
#include "pal/pal.h"
#include "tapp/tapp.h"
#include "tbus/tbus_macros.h"
#include "tbus/tbus_route.h"
#include "tbusd_misc.h"
#include "tbusd_log.h"
#include "tbus/tbus.h"
#include "tbus/tbus_channel.h"
#include "version.h"
#include "tbusd_ctrl_kernel.h"
#include "tbusd_ctrl_svr.h"
#include "tbusd_controller.h"

#define RELAY_CONFINFO "TbusRelay"

extern unsigned char g_szMetalib_TbusConf[] ;

RELAYENV gs_stRelayEnv;
TAPPCTX g_stAppCtx;

/*
 * various internal function defined
 */

static int tbusd_init(TAPPCTX *a_pstAppCtx,  LPRELAYENV a_pstRunEnv);
static void tbusd_init_stat(LPRELAYENV  a_pstRunEnv);
int app_epoll_create( LPRELAYENV  a_pstRunEnv ) ;
int	app_listen_create(LPRELAYENV  a_pstRunEnv) ;
int TbusdServiceInit(LPTAPPCTX a_pstCtx, RELAYENV*a_pstEnv, int argc, char* argv[]);

/*
 * 执行程序退出时的清除操作
 */
static int tbusd_fini(IN TAPPCTX *a_pstAppCtx,  IN LPRELAYENV a_pstRunEnv);

/*
 * tapp 时钟周期触发时执行的逻辑：输出统计信息，检查relay的链接尝试重连
 */
static int tbusd_tick(IN TAPPCTX *a_pstAppCtx,  IN LPRELAYENV a_pstRunEnv);

/*
 * 当tbusd idle时将epoll_wait的时间改成 10毫秒等待
 */
static int tbusd_idle(IN TAPPCTX *a_pstAppCtx,  IN LPRELAYENV a_pstRunEnv);

int relay_channel_attach ( LPRELAYENV  a_pstRunEnv ) ;

static  int tbusd_reload(IN TAPPCTX *a_pstAppCtx,  IN LPRELAYENV a_pstRunEnv);

static int relay_enable (LPTBUSSHMRELAYCNF a_pstRelay,
                         IN TBUSADDR iPeerAddr,	IN TBUSSHMCHANNELCNF *a_pstChlCnf,
                         LPRELAYENV  a_pstRunEnv, IN LPTBUSSHMGCIMHEAD a_pstHead);

static int tbusd_add_relay_channels(LPRELAYENV a_pstRunEnv,
                                    LPTBUSSHMGRM a_pstShmGRM, LPTBUSSHMGCIM a_pstShmGCIM);

static void tbusd_check_relay_connect_addr(LPRELAYENV a_pstRunEnv,
                                           LPTBUSSHMGRM a_pstShmGRM, LPTBUSSHMGCIM a_pstShmGCIM);

static int IsChannelNeedRelay(TBUSADDR	a_iAddr,  LPTBUSSHMGRM a_pstShmGRM);

static void tbusd_recycle_disabled_channels(LPRELAYENV a_pstRunEnv);

static int tbusd_refresh_relays_conf(LPRELAYENV a_pstRunEnv);

static int tbusd_travel_conn_init_recvbuf(void* pvConn, void* pvArg);
static int tbusd_travel_conn_free_recvbuf(void* pvConn, void* pvArg);
static int tbusd_travel_conn_destroy_conn(void* pvConn, void* pvArg);

/*
 * tbusd主循环逻辑
 */
static int tbusd_proc(IN TAPPCTX *a_pstAppCtx,  IN LPRELAYENV a_pstRunEnv);

int pkg_sent_loop ( IN const int a_iCnt ) ;
int pkg_recv_sent ( IN const int a_iSocket, INOUT RELAY *a_ptRelay ) ;
int pkg_sent_recv ( IN const int a_iSocket, INOUT RELAY *a_ptRelay ) ;

//////////////////////////////////////////////////////////////////////////
int TbusdServiceInit(LPTAPPCTX a_pstCtx, RELAYENV*a_pstEnv,int argc, char* argv[])
{
    if ((NULL == a_pstCtx) || (NULL == a_pstEnv))
    {
        printf("the ctx or env is null\n");
        return -1;
    }


    //printf("sizeof TBUSSHMGCIM = %d\n", sizeof(TBUSSHMGCIM));
    //printf("sizeof TBUSSHMGRM = %d\n", sizeof(TBUSSHMGRM));
    memset(a_pstEnv, 0, sizeof(*a_pstEnv));
    memset(a_pstCtx, 0, sizeof(*a_pstCtx));

    a_pstCtx->argc = argc;
    a_pstCtx->argv = argv;

    //tbusd 正常服务回调接口
    a_pstCtx->pfnInit	=	(PFNTAPPFUNC)tbusd_init;
    a_pstCtx->pfnFini	=	(PFNTAPPFUNC)tbusd_fini;
    a_pstCtx->pfnProc	=	(PFNTAPPFUNC)tbusd_proc;
    a_pstCtx->pfnTick	=	(PFNTAPPFUNC)tbusd_tick;
    a_pstCtx->pfnIdle	=	(PFNTAPPFUNC)tbusd_idle;
    a_pstCtx->pfnReload = (PFNTAPPFUNC)tbusd_reload;

    //tbusd 服务端 控制相关回调接口
    a_pstCtx->pfnGetCtrlUsage = tbusd_get_ctrl_usage;
    a_pstCtx->pfnProcCmdLine = tbusd_proc_ctrl_cmdline;

    //tbusd 控制端相关回调接口
    a_pstCtx->pfnControllerInit = tbusd_contorller_init;
    a_pstCtx->pfnProcCtrlRes = tbusd_proc_ctrlres;

    /*初始化系统数据元数据库*/
    a_pstCtx->iLib	=	(intptr_t)&g_szMetalib_TbusConf[0];
    a_pstCtx->stConfData.pszMetaName	=	RELAY_CONFIG_DATA_META_NAME;
    a_pstCtx->stRunDataStatus.pszMetaName = RELAY_RUN_DATA_META_NAME;
    a_pstCtx->uiVersion = TAPP_MAKE_VERSION(MAJOR, MINOR, REV, BUILD);

    return tapp_config_service_desc("转发tbus数据包，实现跨机tbus通信。");
}

int main(int argc, char* argv[])
{
    int iRet = 0;

    tapp_register_service_init((PFNTAPP_SERVICE_INIT)TbusdServiceInit);

    iRet = tapp_run_service(&g_stAppCtx, &gs_stRelayEnv, argc, argv);

    return iRet;
}

static void tbusd_normalize_conf(LPTBUSD a_pstConf)
{

    int iMinHeartBeat;

    assert(NULL != a_pstConf);

    if (TBUSD_DEFAULT_PKG_MIN_SIZE > a_pstConf->iPkgMaxSize)
    {
        a_pstConf->iPkgMaxSize = TBUSD_DEFAULT_PKG_MIN_SIZE;
    }
    if ('\0' == a_pstConf->szSvrLogName[0])
    {
        STRNCPY(a_pstConf->szSvrLogName, TLOG_DEF_CATEGORY_TEXTROOT, sizeof(a_pstConf->szSvrLogName));
    }
    if ('\0' == a_pstConf->szStatLogName[0])
    {
        STRNCPY(a_pstConf->szStatLogName, TLOG_DEF_CATEGORY_DATAROOT, sizeof(a_pstConf->szStatLogName));
    }


    if (0 >= a_pstConf->iSynSeqGap)
    {
        a_pstConf->iSynSeqGap = TBUSD_DEFAULT_SYN_SEQ_GAP;
    }
    if (0 >= a_pstConf->iReconnectGap)
    {
        a_pstConf->iReconnectGap = TBUSD_DEFAULT_RECONNECT_GAP;
    }
    if (0 >= a_pstConf->iHandShakeTimeout)
    {
        a_pstConf->iHandShakeTimeout = TBUSD_DEFAULT_HANDSHAKE_TIMEOUT;
    }
    iMinHeartBeat = a_pstConf->iSynSeqGap *3;
    if (iMinHeartBeat > a_pstConf->iMaxHeartBeatGap)
    {
        a_pstConf->iMaxHeartBeatGap = iMinHeartBeat;
    }
    if (a_pstConf->iNeedAckPkg == 0)
    {
        a_pstConf->iControlUnackedPkgNum = 0;
    }

    if (0 >= a_pstConf->iSendBuff)
    {
        a_pstConf->iSendBuff = TBUSD_DEFAULT_SOCKET_BUFF;
    }
    if (0 >= a_pstConf->iRecvBuff)
    {
        a_pstConf->iRecvBuff = TBUSD_DEFAULT_SOCKET_BUFF;
    }

    if (0 >= a_pstConf->iIdleTickNumBeforeSleep)
    {
        a_pstConf->iIdleTickNumBeforeSleep = TBUSD_DEFAULT_IDLE_COUNT;
    }
}

static int tbusd_travel_conn_init_recvbuf(void* pvConn, void* pvArg)
{
    LPCONNNODE pstConn;

    if (NULL == pvConn)
    {
        return -1;
    }

    pstConn = (LPCONNNODE)pvConn;
    pstConn->szRecvBuff = NULL;

    return 0;
}

static int tbusd_travel_conn_free_recvbuf(void* pvConn, void* pvArg)
{
    LPCONNNODE pstConn;

    if (NULL == pvConn)
    {
        return -1;
    }

    pstConn = (LPCONNNODE)pvConn;
    if (NULL != pstConn->szRecvBuff)
    {
        free(pstConn->szRecvBuff);
        pstConn->szRecvBuff = NULL;
    }

    return 0;
}

int tbusd_init(TAPPCTX *a_pstAppCtx,  LPRELAYENV a_pstRunEnv)
{
    int iRet = TBUS_SUCCESS ;
    time_t tNow;
    LPTBUSD pstConf ;

    assert(NULL != a_pstAppCtx);
    assert(NULL != a_pstRunEnv);

    /*检查tapp对 配置和运行数据结构的初始化结果 */
    a_pstRunEnv->pstConf = (LPTBUSD)a_pstAppCtx->stConfData.pszBuff;
    pstConf = a_pstRunEnv->pstConf;
    if ((NULL == a_pstRunEnv->pstConf) || ((int)sizeof(TBUSD) > a_pstAppCtx->stConfData.iLen)||
        (NULL == (LPTDRMETA)a_pstAppCtx->stConfData.iMeta))
    {
        printf("ERROR:tbusd_init, NULL config data or invalid size<%d>",
               (int)a_pstAppCtx->stConfData.iLen) ;
        return TBUS_ERROR ;
    }
    a_pstRunEnv->pstStat = (LPTBUSDSTAT)a_pstAppCtx->stRunDataStatus.pszBuff;
    if ((NULL == a_pstRunEnv->pstStat) || ((int)sizeof(TBUSDSTAT) > a_pstAppCtx->stRunDataStatus.iLen)||
        (NULL == (LPTDRMETA)a_pstAppCtx->stRunDataStatus.iMeta))
    {
        printf("ERROR:tbusd_init, NULL run data or invalid size<%d>, or NULL meta",
               (int)a_pstAppCtx->stRunDataStatus.iLen) ;
        return TBUS_ERROR ;
    }

    tbusd_normalize_conf(pstConf);

    a_pstAppCtx->iIdleCount = pstConf->iIdleTickNumBeforeSleep;


    /* init debug log */
    iRet = tbusd_log_init(a_pstAppCtx, a_pstRunEnv);
    if (0 != iRet)
    {
        printf("ERROR:tbusd_init, failed to init log system ");
        return iRet;
    }

    tlog_log(g_ptRelayLog, TBUSD_PRIORITY_FATAL,0,0,  "CALL: relay module initialize..." ) ;
    tbusd_log_data(g_ptRelayLog, TBUSD_PRIORITY_INFO, (LPTDRMETA)a_pstAppCtx->stConfData.iMeta,
                   pstConf, sizeof(TBUSD));


    /*init tbus api*/
    tbus_set_bussid(pstConf->iBussinessID);
    iRet = tbus_init_ex(pstConf->szGCIMShmKey, 0);
    if (TBUS_SUCCESS != iRet)
    {
        tbusd_log_msg(g_ptRelayLog, TBUSD_PRIORITY_FATAL, "tbus_init_ex failed, iRet %x", iRet);
        return iRet;
    }

    /* attach GCIM and GRM in share memory */
    iRet = tbus_get_gcimshm(&a_pstRunEnv->pstShmGCIM, pstConf->szGCIMShmKey,
                            pstConf->iBussinessID,	0, 0, NULL);
    if ( 0 != iRet )
    {
        printf("ERROR:tbusd_init, failed to get GCIM by shmkey<%s> and businessid<%d>\n",
               pstConf->szGCIMShmKey, pstConf->iBussinessID) ;
        tbusd_log_msg ( g_ptRelayLog,TBUSD_PRIORITY_ERROR,
                       "get GCIM failed  by shmkey<%s> and bussinessid %d, iRet %x",
                       pstConf->szGCIMShmKey, pstConf->iBussinessID, iRet) ;
    }else
    {
        iRet = tbus_get_grmshm(&a_pstRunEnv->pstShmGRM, pstConf->szRelayShmKey, pstConf->iBussinessID, 0, NULL);
        if (0 != iRet)
        {
            printf("ERROR:tbusd_init, failed to get GRM tables by key<%s> and businessid<%d>\n",
                   pstConf->szRelayShmKey, pstConf->iBussinessID) ;
            tbusd_log_msg(g_ptRelayLog, TBUSD_PRIORITY_FATAL,
                          "failed to get GRM tables by key %s and businessid %d, iRet %x",
                          pstConf->szRelayShmKey, pstConf->iBussinessID, iRet) ;
        }
    }


    /*attach ralay channel*/
    if (0 == iRet)
    {
        iRet = relay_channel_attach(a_pstRunEnv);
        if (0 != iRet)
        {
            printf("ERROR:tbusd_init, failed to attach relay channels\n");
            tbusd_log_msg(g_ptRelayLog, TBUSD_PRIORITY_ERROR, "relay_channel_attach failed, iRet<%d>", iRet);
        }
    }

    /* init mem node pool */
    if (0 == iRet)
    {
        iRet = tmempool_new ( &a_pstRunEnv->ptMemPool, TBUSD_MAX_EVENTS, sizeof(CONNNODE) ) ;
        if ( 0 != iRet )
        {
            printf("ERROR:tbusd_init, failed to allocate mempool\n");
            tbusd_log_msg(g_ptRelayLog, TBUSD_PRIORITY_ERROR, "initialize, tmempool_new failed, iRet<%d>\n", iRet);
        }

        if (0 == iRet)
        {
            iRet = tmempool_travel_all(a_pstRunEnv->ptMemPool, tbusd_travel_conn_init_recvbuf, NULL);
            if (0 != iRet)
            {
                printf("ERROR:tbusd_init, failed to init recvbuf\n");
                tbusd_log_msg(g_ptRelayLog, TBUSD_PRIORITY_ERROR,
                              "initialize, tmempool_travel_all failed, iRet<%d>\n", iRet);
            }
        }
    }

    /*init epoll*/
    if (0 == iRet)
    {
        iRet = app_epoll_create(a_pstRunEnv) ;
        if (0 != iRet)
        {
            printf("ERROR:tbusd_init, failed to init epoll\n");
            tbusd_log_msg(g_ptRelayLog, TBUSD_PRIORITY_ERROR, "app_epoll_create failed, iRet<%d>", iRet);
        }
    }

    /*create listen socket*/
    if (0 == iRet )
    {
        iRet = app_listen_create(a_pstRunEnv) ;
        if ( TBUS_SUCCESS != iRet )
        {
            printf("ERROR:tbusd_init, failed to listen at<%s>\n", a_pstRunEnv->pstConf->szListen);
            tbusd_log_msg(g_ptRelayLog, TBUSD_PRIORITY_ERROR,
                          "FATAL ERROR: Failed to listen at '%s', process aborted...\n",
                          a_pstRunEnv->pstConf->szListen ) ;
        }
    }

    //进程控制服务初始化
    tbusdctrl_init();


    if (0 == iRet)
    {
        /*init stat info*/
        LPTBUSSHMGRM pstShmGRMBackup;
        tbusd_init_stat(a_pstRunEnv);

        time(&tNow);

        a_pstRunEnv->bNeedRefresh = TDR_FALSE;
        a_pstRunEnv->dwGCIMVersion = a_pstRunEnv->pstShmGCIM->stHead.dwVersion;
        pstShmGRMBackup = &a_pstRunEnv->stShmGRMBackup;
        memcpy(&pstShmGRMBackup->stHead, &a_pstRunEnv->pstShmGRM->stHead, sizeof(pstShmGRMBackup->stHead));
        memcpy(&pstShmGRMBackup->astRelays[0], &a_pstRunEnv->pstShmGRM->astRelays[0],
               sizeof(TBUSSHMRELAYCNF)*(a_pstRunEnv->pstShmGRM->stHead.dwUsedCnt));


        a_pstRunEnv->tLastStat = tNow;
        a_pstRunEnv->tLastCheckHandShake = tNow;
        a_pstRunEnv->tLastRefreshConf = tNow;

        if (0 == a_pstAppCtx->iEpollWait)
        {
            a_pstAppCtx->iEpollWait = 10;
        }
        a_pstRunEnv->iEpollWaitTime = a_pstAppCtx->iEpollWait;
        a_pstRunEnv->iMaxSendPkgPerLoop = pstConf->iMaxSendPerloop;



        tlog_log(g_ptRelayLog, TBUSD_PRIORITY_FATAL,0,0,  "tbusd initialize...ok" ) ;
    }/*if (0 == iRet)*/


    return iRet ;
}

void tbusd_init_stat(LPRELAYENV a_pstRunEnv)
{
    LPTBUSDSTAT pstStat;


    assert(NULL != a_pstRunEnv);
    assert(NULL != a_pstRunEnv->pstConf);
    assert(NULL != a_pstRunEnv->pstStat);


    pstStat = a_pstRunEnv->pstStat;
    memset(pstStat, 0, offsetof(TBUSDSTAT, stRelayConnStatList));
    pstStat->stRelayConnStatList.dwRelayCnt = a_pstRunEnv->dwRelayCnt;
    memset(&pstStat->stRelayConnStatList.astRelayConnStat[0], 0, sizeof(TBUSRELAYCONNSTAT)*pstStat->stRelayConnStatList.dwRelayCnt);

}

static int tbusd_travel_conn_destroy_conn(void* pvConn, void* pvArg)
{
    LPRELAYENV pstRunEnv;
    LPCONNNODE pstConn;

    if (NULL == pvConn || NULL == pvArg)
    {
        return -1;
    }

    pstRunEnv = (LPRELAYENV)pvArg;
    pstConn   = (LPCONNNODE)pvConn;

    tbusd_destroy_conn(pstRunEnv, pstConn, NULL);

    return 0;
}

/** 执行程序退出时的清除操作
*/
int tbusd_fini(TAPPCTX *a_pstAppCtx,  LPRELAYENV a_pstRunEnv)
{
    unsigned int i;
    assert(NULL != a_pstAppCtx);

    assert(NULL != a_pstRunEnv);

    tlog_log(g_ptRelayLog, TBUSD_PRIORITY_FATAL,0,0,  "tbusd_fini, server quit..." ) ;


    if (NULL != a_pstRunEnv->ptMemPool)
    {
        tmempool_travel_used(a_pstRunEnv->ptMemPool, tbusd_travel_conn_destroy_conn, a_pstRunEnv);

        tmempool_travel_all(a_pstRunEnv->ptMemPool, tbusd_travel_conn_free_recvbuf, NULL);

        tmempool_destroy(&a_pstRunEnv->ptMemPool);

    }/*if (NULL != a_pstRunEnv->ptMemPool)*/

    if (0 <= a_pstRunEnv->iEpoolFd)
    {
        epoll_destroy(a_pstRunEnv->iEpoolFd);
        a_pstRunEnv->iEpoolFd = -1;
    }

    /*释放relay管理通道*/
    for (i = 0; i < a_pstRunEnv->dwRelayCnt; i++)
    {
        LPRELAY pstRelay = a_pstRunEnv->pastTbusRelay[i];
        if (NULL != pstRelay)
        {
            free(pstRelay);
            a_pstRunEnv->pastTbusRelay[i] = NULL;
        }
    }/*for (i = 0; i < a_pstRunEnv->stRelayGlobal.dwRelayCnt; i++)*/
    a_pstRunEnv->dwRelayCnt = 0;

    tbus_fini();

    tbusd_log_fini();

    return 0;
}

int relay_enable (LPTBUSSHMRELAYCNF a_pstRelay, IN TBUSADDR iPeerAddr,	IN TBUSSHMCHANNELCNF *a_pstChlCnf,
                  LPRELAYENV  a_pstRunEnv, IN LPTBUSSHMGCIMHEAD a_pstHead)
{
    int iRet = TBUS_SUCCESS ;
    RELAY *ptRelay = NULL;
    unsigned int i;

    assert(NULL != a_pstChlCnf);
    assert(NULL != a_pstRunEnv);
    assert(NULL != a_pstRelay);

    /*先检查此通道是否存在*/
    for (i = 0; i < a_pstRunEnv->dwRelayCnt; i++)
    {
        LPTBUSCHANNEL pstChl = &(a_pstRunEnv->pastTbusRelay[i]->stChl);
        if ((a_pstRelay->dwAddr == pstChl->pstHead->astAddr[pstChl->iRecvSide]) &&
            (iPeerAddr == pstChl->pstHead->astAddr[pstChl->iSendSide]))
        {
            TBUSD_RELAY_CLR_NOT_IN_CONF(a_pstRunEnv->pastTbusRelay[i]);
            TBUSD_RELAY_SET_ENABLE(a_pstRunEnv->pastTbusRelay[i]);
            break;
        }
    }
    if (i < a_pstRunEnv->dwRelayCnt)
    {
        return 0;
    }


    /*检查存储空间是否够*/
    if (a_pstRunEnv->dwRelayCnt >= sizeof(a_pstRunEnv->pastTbusRelay)/sizeof(a_pstRunEnv->pastTbusRelay[0]))
    {
        tbusd_log_msg ( g_ptRelayLog,TBUSD_PRIORITY_ERROR,
                       "relay_channel_attach relay count reach max %d, cannot attach",
                       a_pstRunEnv->dwRelayCnt) ;
        return -1;
    }

    /*为新的中转通道管理分配空间*/
    ptRelay = (RELAY *) malloc(sizeof(RELAY));
    if (NULL == ptRelay)
    {
        tbusd_log_msg ( g_ptRelayLog,TBUSD_PRIORITY_ERROR,
                       "relay_enable, failed to malloc memory<size:%d>",
                       (int)sizeof(RELAY)) ;
        return -2;
    }

    memset(ptRelay, 0, sizeof(RELAY)) ;
    memcpy(&(ptRelay->stRelayInfo), a_pstRelay, sizeof(TBUSSHMRELAYCNF)) ;
    ptRelay->dwFlag = TBUSD_FLAG_ENABLE ;
    ptRelay->dwRelayStatus = TBUSD_STATUS_NO_CONNECTION;
    ptRelay->iID = a_pstRunEnv->dwRelayCnt;
    TBUS_CHANNEL_SET_ENABLE(&ptRelay->stChl);

    iRet = tbus_attach_channel(a_pstRelay->dwAddr, a_pstChlCnf, &(ptRelay->stChl), a_pstHead) ;
    if (0 != iRet)
    {
        tbusd_log_msg ( g_ptRelayLog,TBUSD_PRIORITY_ERROR,
                       "failed to attach channel(0x%08x <--> 0x%08x), for %s", a_pstRelay->dwAddr,
                       iPeerAddr, tbus_error_string(iRet)) ;
    }

    if (0 == iRet)
    {
        iRet = tbusd_parse_url_inet_addr(ptRelay->stRelayInfo.szMConn, &ptRelay->stConnectAddr);
        if (0 != iRet)
        {
            char szAddr1[TBUS_MAX_ADDR_STRING_LEN];
            char szAddr2[TBUS_MAX_ADDR_STRING_LEN];
            assert(NULL != ptRelay->stChl.pstHead);
            tbusd_log_msg(g_ptRelayLog, TBUSD_PRIORITY_FATAL,
                          "failed to parse connecting URL: %s of relay(id:%d %s <-->%s)",
                          ptRelay->stRelayInfo.szMConn, ptRelay->iID,
                          tbus_addr_ntop(ptRelay->stChl.pstHead->astAddr[0],szAddr1,sizeof(szAddr1)),
                          tbus_addr_ntop(ptRelay->stChl.pstHead->astAddr[1],szAddr2,sizeof(szAddr2))) ;
        }
    }

    if (0 == iRet)
    {
        a_pstRunEnv->pastTbusRelay[a_pstRunEnv->dwRelayCnt] = ptRelay;
        a_pstRunEnv->dwRelayCnt++;
        tbusd_log_msg ( g_ptRelayLog,TBUSD_PRIORITY_ERROR,
                       "add channel(0x%08x <--> 0x%08x) to relaymgr,now relay channels num: %d",
                       a_pstRelay->dwAddr, iPeerAddr, a_pstRunEnv->dwRelayCnt) ;
    } else
    {
        if (NULL != ptRelay)
        {
            free(ptRelay);
            ptRelay = NULL;
        }
    }

    return iRet ;

}

int tbusd_add_relay_channels(LPRELAYENV a_pstRunEnv, LPTBUSSHMGRM a_pstShmGRM, LPTBUSSHMGCIM a_pstShmGCIM)
{
    int iRet = 0;
    unsigned int	i = 0;
    unsigned int	j = 0;

    unsigned int dwRelaysCount;
    unsigned int dwChannelsCount;

    assert(NULL != a_pstRunEnv);
    assert(NULL != a_pstShmGRM);
    assert(NULL != a_pstShmGRM);

    dwRelaysCount = a_pstShmGRM->stHead.dwUsedCnt;
    dwChannelsCount = a_pstShmGCIM->stHead.dwUsedCnt;
    for ( i=0; i< dwRelaysCount; i++ )
    {
        LPTBUSSHMRELAYCNF pstRelay;

        pstRelay = &a_pstShmGRM->astRelays[i];
        if (!(pstRelay->dwFlag & TBUS_RELAY_ENABLE))
        {
            /* __log4c_category_trace ( g_ptRelayLog, "relay_channel_attach, relay info disable %i", i ) ; */
            continue ;
        }

        for ( j=0; j< dwChannelsCount; j++ )
        {
            TBUSADDR iPeerAddr;
            TBUSSHMCHANNELCNF *pstChannel = &(a_pstShmGCIM->astChannels[j]);

            if (!(pstChannel->dwFlag & TBUS_CHANNEL_FLAG_ENABLE))
            {
                continue;
            }

            if (pstRelay->dwAddr == pstChannel->astAddrs[0])
            {
                iPeerAddr = pstChannel->astAddrs[1];
            }else if (pstRelay->dwAddr == pstChannel->astAddrs[1])
            {
                iPeerAddr = pstChannel->astAddrs[0];
            }else
            {
                continue;
            }

            iRet = relay_enable(pstRelay, iPeerAddr, pstChannel, a_pstRunEnv, &a_pstShmGCIM->stHead);
            if ( TBUS_SUCCESS != iRet )
            {
                char szPeerAddr[32]={0};
                STRNCPY(szPeerAddr, tbus_addr_nota_by_addrtemplet(&a_pstShmGRM->stHead.stAddrTemplet, iPeerAddr),
                        sizeof(szPeerAddr));
                tbusd_log_msg ( g_ptRelayLog,TBUSD_PRIORITY_ERROR,
                               "failed to add channel(%s <-->%s) to ralay managei", szPeerAddr,
                               tbus_addr_nota_by_addrtemplet(&a_pstShmGRM->stHead.stAddrTemplet, pstRelay->dwAddr)) ;
                break ;
            }

        }/*for ( j=0; j< dwChannelsCount; j++ )*/
        if (0 != iRet)
        {
            break;
        }
    }/*for ( i=0; i< dwRelaysCount; i++ )*/

    return iRet;
}

void tbusd_check_relay_connect_addr(LPRELAYENV a_pstRunEnv, LPTBUSSHMGRM a_pstShmGRM, LPTBUSSHMGCIM a_pstShmGCIM)
{
    int iRet = 0;
    unsigned int	i = 0;
    unsigned int	j = 0;

    unsigned int dwRelayConfCount;
    unsigned int dwCurRelayCount;

    assert(NULL != a_pstRunEnv);
    assert(NULL != a_pstShmGRM);
    assert(NULL != a_pstShmGRM);

    dwRelayConfCount = a_pstShmGRM->stHead.dwUsedCnt;
    dwCurRelayCount = a_pstRunEnv->dwRelayCnt;
    for ( i=0; i< dwRelayConfCount; i++ )
    {
        LPTBUSSHMRELAYCNF pstRelayConf;
        TNETADDR stConnectAddr;

        pstRelayConf = &a_pstShmGRM->astRelays[i];
        if (!(pstRelayConf->dwFlag & TBUS_RELAY_ENABLE))
        {
            continue ;
        }

        memset(&stConnectAddr, 0, sizeof(stConnectAddr));
        iRet = tbusd_parse_url_inet_addr(pstRelayConf->szMConn, &stConnectAddr);
        if (0 != iRet)
        {
            tbusd_log_msg(g_ptRelayLog, TBUSD_PRIORITY_FATAL,
                          "failed to parse connecting URL: %s of relay(bus-addr:%u)",
                          pstRelayConf->szMConn, pstRelayConf->dwAddr);
            continue;
        }

        for ( j=0; j< dwCurRelayCount; j++ )
        {
            RELAY* pstRelay;

            pstRelay = a_pstRunEnv->pastTbusRelay[j];

            if (TBUSD_STATUS_NO_CONNECTION == pstRelay->dwRelayStatus)
            {
                continue;
            }

            if (pstRelay->stChl.pstHead->astAddr[pstRelay->stChl.iRecvSide] == pstRelayConf->dwAddr)
            {
                if (memcmp(&pstRelay->stConnectAddr, &stConnectAddr, sizeof(stConnectAddr)))
                {
                    char szBusAddr[TBUS_MAX_ADDR_STRING_LEN];

                    tbusd_log_msg(g_ptRelayLog, TLOG_PRIORITY_ERROR,
                                  "relay<id:%d, fd:%d, peer:%s> connect-address changed from %s to %s,"
                                  " so close connection for reconnect",
                                  pstRelay->iID, pstRelay->pConnNode->iFd,
                                  tbus_addr_ntop(pstRelayConf->dwAddr, szBusAddr, sizeof(szBusAddr)),
                                  pstRelay->stRelayInfo.szMConn, pstRelayConf->szMConn);

                    memcpy(&pstRelay->stConnectAddr, &stConnectAddr, sizeof(stConnectAddr));

                    tbusd_destroy_conn(a_pstRunEnv, pstRelay->pConnNode, NULL);
                }
            }
        }/*for ( j=0; j< dwCurRelayCount; j++ )*/
    }/*for ( i=0; i< dwRelayConfCount; i++ )*/
}

void tbusd_recycle_disabled_channels(LPRELAYENV a_pstRunEnv)
{
    time_t tNow;
    unsigned int i;

    assert(NULL != a_pstRunEnv);

    time(&tNow);
    for (i = 0; i < a_pstRunEnv->dwRelayCnt; i++)
    {
        RELAY *pstRelay = a_pstRunEnv->pastTbusRelay[i];
        CHANNELHEAD *pstHead = pstRelay->stChl.pstHead;

        if (TBUSD_RELAY_IS_ENABLE(pstRelay))
        {
            continue;
        }

        /*如果不在配置中的通道预留一段时间后超时，则关闭或尝试清除它*/
        if (a_pstRunEnv->pstConf->iDisabledChannelTimeGap >= 0
            && (tNow - pstRelay->tBeginDisabled) >= a_pstRunEnv->pstConf->iDisabledChannelTimeGap)
        {
            tbusd_log_msg(g_ptRelayLog, TLOG_PRIORITY_FATAL,
                          "the time which the channel(0x%08x <-->0x%08x) is disabled"
                          " reach %d second, so clear the channel", pstHead->astAddr[0],
                          pstHead->astAddr[1], a_pstRunEnv->pstConf->iDisabledChannelTimeGap);
            if (NULL != pstRelay->pConnNode)
            {
                /*关闭连接*/
                tbusd_destroy_conn(a_pstRunEnv, pstRelay->pConnNode, NULL);
            }
            free(pstRelay);
            if (i < a_pstRunEnv->dwRelayCnt -1 )
            {
                a_pstRunEnv->pastTbusRelay[i] = a_pstRunEnv->pastTbusRelay[a_pstRunEnv->dwRelayCnt -1];
                a_pstRunEnv->pastTbusRelay[a_pstRunEnv->dwRelayCnt -1] = NULL;
                a_pstRunEnv->pastTbusRelay[i]->iID = i; //修改此中转通道的id
            }
            a_pstRunEnv->dwRelayCnt--;
            i--;
        }/* if (a_pstRunEnv->pstConf->iDisabledChannelTimeGap >= 0
            && (tNow - pstRelay->tBeginDisabled) >= a_pstRunEnv->pstConf->iDisabledChannelTimeGap) */
    }/*for (i = 0; i < a_pstRunEnv->dwRelayCnt; i++)*/
}

int IsChannelNeedRelay(TBUSADDR	a_iAddr,  LPTBUSSHMGRM a_pstShmGRM)
{
    unsigned int i;

    assert(NULL != a_pstShmGRM);
    for (i =0 ; i < a_pstShmGRM->stHead.dwUsedCnt; i++)
    {
        if (a_iAddr == a_pstShmGRM->astRelays[i].dwAddr)
        {
            return 1;
        }
    }

    return 0;
}

int tbusd_refresh_relays_conf(LPRELAYENV a_pstRunEnv)
{
    int iRet = 0;

    LPTBUSSHMGRM pstShmGRMBackup;
    LPTBUSSHMGRM pstShmGRM;
    LPTBUSSHMGCIM pstShmGCIM;
    unsigned int i;
    RELAY *pstRelay;
    CHANNELHEAD *pstChlHead ;
    time_t tNow;

    assert(NULL != a_pstRunEnv);

    /*首先检查回收不再使用的通道*/
    time(&tNow);
    tbusd_recycle_disabled_channels(a_pstRunEnv);


    /*检查gcim和grm的配置是否有更新*/
    pstShmGCIM = a_pstRunEnv->pstShmGCIM;
    assert(NULL != pstShmGCIM);
    if (a_pstRunEnv->dwGCIMVersion != pstShmGCIM->stHead.dwVersion)
    {
        a_pstRunEnv->bNeedRefresh = TDR_TRUE;
    }
    pstShmGRM = a_pstRunEnv->pstShmGRM;
    pstShmGRMBackup = &a_pstRunEnv->stShmGRMBackup;
    if (pstShmGRMBackup->stHead.dwVersion != pstShmGRM->stHead.dwVersion)
    {
        a_pstRunEnv->bNeedRefresh = TDR_TRUE;
    }
    if (a_pstRunEnv->bNeedRefresh == TDR_FALSE)
    {
        return 0; /*gcim和grm版本没有变化,因此不需要检查配置更新*/
    }

    /*先尝试获取grm配置的一个拷贝*/
    if (pstShmGRMBackup->stHead.dwVersion != pstShmGRM->stHead.dwVersion)
    {
        if (0 != pthread_rwlock_tryrdlock(&pstShmGRM->stHead.stRWLock))
        {
            return 0; /*锁定失败,下次在尝试*/
        }
        memcpy(&pstShmGRMBackup->stHead, &pstShmGRM->stHead, sizeof(pstShmGRMBackup->stHead));
        memcpy(&pstShmGRMBackup->astRelays[0], &pstShmGRM->astRelays[0],
               sizeof(TBUSSHMRELAYCNF)*(pstShmGRM->stHead.dwUsedCnt));
        pthread_rwlock_unlock(&pstShmGRM->stHead.stRWLock);
    }/*if (a_pstRunEnv->stShmGRMBackup.stHead.dwVersion != pstHead->dwVersion)*/



    /*尝试锁gcim配置，如果锁失败则下次再尝试*/
    if (0 != pthread_rwlock_trywrlock(&pstShmGCIM->stHead.stRWLock))
    {
        return 0;
    }

    /*先检查对于当前配置而言失效的中转通道*/
    for (i = 0; i < a_pstRunEnv->dwRelayCnt; i++)
    {
        pstRelay = a_pstRunEnv->pastTbusRelay[i];
        pstChlHead = pstRelay->stChl.pstHead;
        if (!IsChannelInGCIM(pstChlHead, pstShmGCIM))
        {
            /*relay中管理的通道不在gcim配置中，则此通道设置标志位*/
            tlog_debug(g_ptRelayLog, pstRelay->iID, TBUSD_LOGCLS_RELAYID,
                       "the relaychannel(0x%08x <--> 0x%08x)"
                       "is not in gcim, so set the NOT_IN_CONF flag and disable it",
                       pstChlHead->astAddr[pstRelay->stChl.iRecvSide],
                       pstChlHead->astAddr[pstRelay->stChl.iSendSide]);
            TBUSD_RELAY_SET_NOT_IN_CONF(pstRelay);
            TBUSD_RELAY_CLR_ENABLE(pstRelay);
            pstRelay->tBeginDisabled = tNow;
            continue;
        }
        if (!IsChannelNeedRelay(pstChlHead->astAddr[pstRelay->stChl.iRecvSide],
                                pstShmGRMBackup))
        {
            /*中转通道需要中转的一段不再需要中转，则此通道设置标志位*/
            tlog_debug(g_ptRelayLog, pstRelay->iID, TBUSD_LOGCLS_RELAYID,
                       "the relaychannel(0x%08x <--> 0x%08x)"
                       "need not relay, so set the NOT_IN_CONF flag and disable it",
                       pstChlHead->astAddr[pstRelay->stChl.iRecvSide],
                       pstChlHead->astAddr[pstRelay->stChl.iSendSide]);
            TBUSD_RELAY_SET_NOT_IN_CONF(pstRelay);
            TBUSD_RELAY_CLR_ENABLE(pstRelay);
            pstRelay->tBeginDisabled = tNow;
            continue;
        }
    }/*for (i = 0; i < a_pstRunEnv->dwRelayCnt; i++)*/

    /*检查需要添加的通道*/
    tbusd_add_relay_channels(a_pstRunEnv, pstShmGRMBackup, pstShmGCIM);

    a_pstRunEnv->dwGCIMVersion = pstShmGCIM->stHead.dwVersion;

    /*解锁*/
    pthread_rwlock_unlock(&pstShmGCIM->stHead.stRWLock);

    tbusd_check_relay_connect_addr(a_pstRunEnv, pstShmGRMBackup, pstShmGCIM);

    a_pstRunEnv->bNeedRefresh = TDR_FALSE;

    return iRet;
}



int relay_channel_attach (LPRELAYENV  a_pstRunEnv)
{
    int iRet = TBUS_ERROR;
    LPTBUSSHMGCIM pstGCIM;
    LPTBUSSHMGRM pstGRM;


    assert(NULL != a_pstRunEnv);
    pstGCIM = a_pstRunEnv->pstShmGCIM;
    pstGRM = a_pstRunEnv->pstShmGRM;
    assert(NULL != pstGCIM);
    assert(NULL != pstGRM);


    /*1.GCIM和GRM通信地址模板是否一致,tbusd不做检查,其一致性由外部的配置管理系统保证
      2.当机器上运行了多个tbusd时,如果不同时获取GCIM和GRM的锁,则,单独去锁GRM会导致死锁,这里采用一些折中
      的处理方法,每次只锁一个锁,将信息备份出来*/
    tbus_rdlock(&pstGRM->stHead.stRWLock);
    memcpy(&a_pstRunEnv->stShmGRMBackup.stHead, &pstGRM->stHead, sizeof(pstGRM->stHead));
    memcpy(&a_pstRunEnv->stShmGRMBackup.astRelays[0], &pstGRM->astRelays[0],
           sizeof(TBUSSHMRELAYCNF)*(pstGRM->stHead.dwUsedCnt));
    tbus_unlock(&pstGRM->stHead.stRWLock);
    pstGRM = &a_pstRunEnv->stShmGRMBackup;

    /*添加需要中转的通道*/
    tbus_wrlock(&pstGCIM->stHead.stRWLock);
    iRet = tbusd_add_relay_channels(a_pstRunEnv, pstGRM, pstGCIM);
    if (0 != iRet)
    {
        tbusd_log_msg ( g_ptRelayLog,TBUSD_PRIORITY_ERROR, "failed to add relay channels, iRet:%d",
                       iRet) ;
    }
    tbus_unlock(&pstGCIM->stHead.stRWLock);

    if (0 >= a_pstRunEnv->dwRelayCnt)
    {
        tlog_log(g_ptRelayLog, TBUSD_PRIOROTY_WARN,0,0,
                 "relay_channel_attach, no  relay need to attach, please check configure") ;
    }


    return iRet ;
}





int app_epoll_create ( LPRELAYENV  a_pstRunEnv )
{
    int iRet = TBUS_SUCCESS ;

    assert(NULL != a_pstRunEnv);

    /* create epoll event pool */
    a_pstRunEnv->iEpoolFd = epoll_create ( TBUSD_MAX_FD ) ;
    if ( 0 > a_pstRunEnv->iEpoolFd )
    {
        tbusd_log_msg ( g_ptRelayLog, TBUSD_PRIORITY_ERROR,
                       "app_epoll_create, epoll_create() failed, error:%u, %s",
                       TBUSD_GET_LAST_ERROR_CODE, tbusd_get_error_string(TBUSD_GET_LAST_ERROR_CODE)) ;
        iRet = TBUS_ERROR ;
    }


    tbusd_log_msg ( g_ptRelayLog,TBUSD_PRIORITY_DEBU,   "RETN: app_epoll_create(void)=%i", iRet ) ;

    return iRet ;
}




int app_listen_create (LPRELAYENV  a_pstRunEnv)
{
    int iRet = TBUS_SUCCESS ;
    int s ;

    LPTBUSD pstConf;
    LPCONNNODE pstConn;


    assert(NULL != a_pstRunEnv);
    pstConf = a_pstRunEnv->pstConf;
    assert(NULL != pstConf);


    /*校验监听url的合法性*/
    if(0 != tbusd_parse_url_inet_addr(pstConf->szListen, &a_pstRunEnv->stListenAddr) )
    {
        tbusd_log_msg ( g_ptRelayLog,TBUSD_PRIORITY_ERROR,
                       "failed to parse listening ip address and port from URL:%s",
                       pstConf->szListen) ;
        return -1;
    }


    /* create listen handle */
    s = tnet_listen ( pstConf->szListen, pstConf->dwBackLog ) ;
    if ( 0 > s )
    {
        tbusd_log_msg ( g_ptRelayLog,TBUSD_PRIORITY_ERROR,
                       "tnet_listen() URL:%s, backlog:%d failed, errno:%u, for %s ",
                       pstConf->szListen, pstConf->dwBackLog, TBUSD_GET_LAST_ERROR_CODE,
                       tbusd_get_error_string(TBUSD_GET_LAST_ERROR_CODE) ) ;
        return TBUS_ERROR ;
    }


    /* register listen handle to epoll */
    pstConn = tbusd_create_conn(a_pstRunEnv, s, RELAY_FD_LISTEN_MODE, EPOLLIN);
    if (NULL == pstConn)
    {
        tbusd_log_msg ( g_ptRelayLog,TBUSD_PRIORITY_ERROR,
                       "app_listen_create, failed to add fd %d to manage", s) ;
        return TBUS_ERROR;
    }

    tbusd_log_msg ( g_ptRelayLog,TBUSD_PRIORITY_DEBU,
                   "RETN: app_listen_create(), index %i, listen socket fd %i",
                   pstConn->idx, s ) ;
    return iRet ;
}


int tbusd_proc(IN TAPPCTX *a_pstAppCtx,  IN LPRELAYENV a_pstRunEnv)
{
    int	iMaxSendPkg = TBUS_DEFAULT_SENT_CNT;
    LPTBUSDSTAT pstStat;
    struct timeval stStart;
    struct timeval stEnd;
    struct timeval stSub;
    int iRecvPkgNum = 0;
    int iSendPkgNum = 0;

    assert(NULL != a_pstAppCtx);
    assert(NULL != a_pstRunEnv);
    assert(NULL != a_pstRunEnv->pstStat);

    gettimeofday(&stStart, NULL);
    pstStat = a_pstRunEnv->pstStat;

    iRecvPkgNum = tbusd_proc_recv(a_pstAppCtx, a_pstRunEnv, &iMaxSendPkg);

    iSendPkgNum = tbusd_proc_send(a_pstRunEnv, iMaxSendPkg);


    gettimeofday(&stEnd, NULL);
    TV_DIFF(stSub, stEnd, stStart);
    if ((pstStat->iSecPreloop < (int)stSub.tv_sec))
    {
        pstStat->iSecPreloop = (int)stSub.tv_sec;
        pstStat->iUsecPreloop = (int)stSub.tv_usec;
    }else if (pstStat->iUsecPreloop < (int)stSub.tv_usec)
    {
        pstStat->iSecPreloop = (int)stSub.tv_sec;
        pstStat->iUsecPreloop = (int)stSub.tv_usec;
    }
    pstStat->dwLoopTimes++;

    return iRecvPkgNum + iSendPkgNum -1;
}

int tbusd_tick(TAPPCTX *a_pstAppCtx,  LPRELAYENV a_pstRunEnv)
{
#define RELAY_STAT_BUFF_SIZE  10240

    LPTBUSD pstConf;
    time_t tNow;
    unsigned int	i = 0;
    RELAY *ptRelay = NULL ;
    LPCONNNODE pstNode;
    int iCount;


    assert(NULL != a_pstAppCtx);
    assert(NULL != a_pstRunEnv);
    pstConf = a_pstRunEnv->pstConf;
    assert(NULL != pstConf);

    /*write stat*/
    if (((a_pstAppCtx->stCurr.tv_sec - a_pstRunEnv->tLastStat) > pstConf->iStatGap) &&
        (0 < pstConf->iStatGap))
    {
        LPTBUSDSTAT pstStat = a_pstRunEnv->pstStat;
        for (i = 0; i < a_pstRunEnv->dwRelayCnt; i++)
        {
            LPTBUSRELAYCONNSTAT pstRelayStat = &pstStat->stRelayConnStatList.astRelayConnStat[i];
            pstStat->dwSendPkgCount += pstRelayStat->dwSendPkgCount;
            pstStat->ullSendPkgBytes += pstRelayStat->ullSendPkgBytes;
            pstStat->dwSendSynPkgCount += pstRelayStat->dwSendSynPkgCount;
            pstStat->dwRecvSynPkgCount += pstRelayStat->dwRecvSynPkgCount;
        }
        tbusd_log_data(g_ptStatLog, TBUSD_PRIORITY_FATAL, (LPTDRMETA)a_pstAppCtx->stRunDataStatus.iMeta,
                       a_pstAppCtx->stRunDataStatus.pszBuff, a_pstAppCtx->stRunDataStatus.iLen);


        tbusd_init_stat(a_pstRunEnv);
        a_pstRunEnv->tLastStat = a_pstAppCtx->stCurr.tv_sec;
    }/*if ((0 < pstConf->iStatGap) && (a_pstAppCtx->stCurr.tv_sec - sLastStat > pstConf->iStatGap))*/

    time(&tNow);

    /*定时检查gcim和grm的配置,刷新通道配置*/
    if ((0 < pstConf->iRefreshTbusConfGap) &&
        ((tNow - a_pstRunEnv->tLastRefreshConf) >= pstConf->iRefreshTbusConfGap))
    {
        tbusd_refresh_relays_conf(a_pstRunEnv);
        a_pstRunEnv->tLastRefreshConf = tNow;
    }


    /*检查各个relay 数据结构的状态，根据其状态启动各种定时检查。
      relay的状态可以描述成三大状态：未建立链接 -> 建立链接过程中(握手过程) -> 已建立链接
      */
    iCount=0;
    for ( i=0; i<a_pstRunEnv->dwRelayCnt; i++ )
    {
        ptRelay = a_pstRunEnv->pastTbusRelay[i] ;
        tlog_trace(g_ptRelayLog,ptRelay->iID, TBUSD_LOGCLS_RELAYID,
                   "tbusd_check_handshake, relay %d status %d", i, ptRelay->dwRelayStatus);
        if (!TBUSD_RELAY_IS_ENABLE(ptRelay))
        {
            continue ;
        }

        switch(ptRelay->dwRelayStatus)
        {
            case TBUSD_STATUS_ESTABLISHED:	/*已建立链接*/
                {
                    pstNode = (LPCONNNODE)ptRelay->pConnNode;
                    if (NULL == pstNode)
                    {
                        tlog_debug( g_ptRelayLog,ptRelay->iID, TBUSD_LOGCLS_RELAYID,
                                   "the relay(ID:%d) have not bind any connection, so change it status(%d) to NO_CONNECTION",
                                   ptRelay->iID, ptRelay->dwRelayStatus) ;
                        ptRelay->dwRelayStatus = TBUSD_STATUS_NO_CONNECTION;
                        break;;
                    }

                    //是否同步心跳超时，如果是则端口链接，重建
                    if ((tNow - ptRelay->tLastHeartbeat) > pstConf->iMaxHeartBeatGap)
                    {
                        tlog_error( g_ptRelayLog,ptRelay->iID, TBUSD_LOGCLS_RELAYID,
                                   "the time to hardbeat reach %d secondes, it's beyond "
                                   " the max limit(%d) , so break connection of relay(ID:%d)",
                                   (int)(tNow - ptRelay->tLastHeartbeat), pstConf->iMaxHeartBeatGap, ptRelay->iID) ;
                        tbusd_destroy_conn(a_pstRunEnv, pstNode, NULL);
                        break;
                    }/*if ((tNow - ptRelay->tLastHeartbeat) > pstConf->iMaxHeartBeatGap)*/

                    //定时同步序列号
                    if ((tNow - pstNode->tLastSynSeq) >= pstConf->iSynSeqGap)
                    {
                        pstNode->bNeedSynSeq = TBUSD_TRUE;
                        pstNode->tLastSynSeq = tNow;
                    }
                    iCount++;
                }
                break;
            case TBUSD_STATUS_NO_CONNECTION:  /*没有建立链接 则启动连接建立*/
                {
                    tbusd_connect_peer(a_pstRunEnv, tNow, ptRelay);
                }
                break;
#if defined(_WIN32) || defined(_WIN64)
                /*
                 * 受pal epoll windows版本(IOCP模拟实现)的限制,
                 * windows环境下不使用epoll判断非阻塞连接是否已成功,
                 * 所以在这里加上一个检测,pal层中的检测底层使用的是select机制,
                 */
            case TBUSD_STATUS_CONNECTING:
                {
                    int iRet;

                    iRet = tbusd_check_nonblock_connect(a_pstRunEnv, tNow, ptRelay);
                    if (0 != iRet)
                    {
                        tbusd_destroy_conn(a_pstRunEnv, ptRelay->pConnNode, NULL);
                    }
                }
                break;
#endif
            default:  /*建立链接过程中，即握手过程*/
                {
                    pstNode = (LPCONNNODE)ptRelay->pConnNode;
                    if (NULL == pstNode)
                    {
                        tlog_debug( g_ptRelayLog,ptRelay->iID, TBUSD_LOGCLS_RELAYID,
                                   "the relay(ID:%d) have not bind any connection,"
                                   " so change it status(%d) to NO_CONNECTION",
                                   ptRelay->iID, ptRelay->dwRelayStatus) ;
                        ptRelay->dwRelayStatus = TBUSD_STATUS_NO_CONNECTION;
                        break;;
                    }

                    //检查握手是否超时
                    if ((tNow - pstNode->tCreate) >= pstConf->iHandShakeTimeout)
                    {
                        tlog_debug( g_ptRelayLog,ptRelay->iID, TBUSD_LOGCLS_RELAYID,
                                   "the time to handshake reach %d secondes, it's beyond "
                                   " the max limit(%d) , so break handshake of relay(ID:%d)",
                                   (int)(tNow - pstNode->tCreate), pstConf->iHandShakeTimeout, ptRelay->iID) ;
                        tbusd_destroy_conn(a_pstRunEnv, pstNode, NULL);
                    }/*if ((tNow - pstNode->tCreate) >= pstConf->iHandShakeTimeout)*/
                }
                break;
        }/*switch(ptRelay->dwRelayStatus)*/
    }/*for ( i=0; i<a_pstRunEnv->dwRelayCnt; i++ )*/

    //调整最大发包
    if (0 == iCount)
    {
        a_pstRunEnv->iMaxSendPkgPerLoop = pstConf->iMaxSendPerloop;
    }else
    {
        a_pstRunEnv->iMaxSendPkgPerLoop = pstConf->iMaxSendPerloop / iCount;
    }

    return TBUS_SUCCESS;
}

/** 当tbusd idle时将epoll_wait的时间改成 10毫秒等待
*/
int tbusd_idle(IN TAPPCTX *a_pstAppCtx,  IN LPRELAYENV a_pstRunEnv)
{
    assert(NULL != a_pstAppCtx);
    assert(NULL != a_pstRunEnv);

    a_pstRunEnv->iEpollWaitTime = a_pstAppCtx->iEpollWait;

    return 0;
}

int tbusd_reload(IN TAPPCTX *a_pstAppCtx,  IN LPRELAYENV a_pstRunEnv)
{
    LPTBUSD pstCurConf;
    LPTBUSD pstPrepareConf;
    assert(NULL != a_pstAppCtx);
    assert(NULL != a_pstRunEnv);

    pstCurConf = a_pstRunEnv->pstConf;
    if (NULL == pstCurConf)
    {
        tlog_log( g_ptRelayLog,TBUSD_PRIORITY_FATAL, 0,0,  "the current conf struct is null") ;
        return -1;
    }
    pstPrepareConf = (LPTBUSD)a_pstAppCtx->stConfPrepareData.pszBuff;
    if (NULL == pstPrepareConf)
    {
        tlog_log( g_ptRelayLog,TBUSD_PRIORITY_FATAL, 0,0,   "the prepareed conf struct is null") ;
        return -2;
    }
    if ((int)sizeof(TBUSD) > a_pstAppCtx->stConfPrepareData.iLen )
    {
        tbusd_log_msg ( g_ptRelayLog,TBUSD_PRIORITY_FATAL,
                       "the size(%d) of prepareed conf struct is less than the size(%d) of TBUSD",
                       (int)a_pstAppCtx->stConfPrepareData.iLen, (int)sizeof(TBUSD)) ;
        return -3;
    }

    //复制部分可以刷新的配置
    pstCurConf->iControlUnackedPkgNum = pstPrepareConf->iControlUnackedPkgNum;
    pstCurConf->iHandShakeTimeout = pstPrepareConf->iHandShakeTimeout;
    pstCurConf->iMaxRecvPerloop = pstPrepareConf->iMaxRecvPerloop;
    pstCurConf->iMaxSendPerloop = pstPrepareConf->iMaxSendPerloop;
    pstCurConf->iNeedAckPkg = pstPrepareConf->iNeedAckPkg;
    pstCurConf->iPkgMaxSize = pstPrepareConf->iPkgMaxSize;
    pstCurConf->iReconnectGap = pstPrepareConf->iReconnectGap;
    pstCurConf->iRecvBuff = pstPrepareConf->iRecvBuff;
    pstCurConf->iSendBuff = pstPrepareConf->iSendBuff;
    pstCurConf->iRefreshTbusConfGap = pstPrepareConf->iRefreshTbusConfGap;
    pstCurConf->iStatGap = pstPrepareConf->iStatGap;
    pstCurConf->iSynSeqGap = pstPrepareConf->iSynSeqGap;
    pstCurConf->iMaxHeartBeatGap = pstPrepareConf->iMaxHeartBeatGap;
    pstCurConf->iDisabledChannelTimeGap = pstPrepareConf->iDisabledChannelTimeGap;


    tbusd_normalize_conf(pstCurConf);

    tlog_log( g_ptRelayLog,TBUSD_PRIORITY_INFO, 0,0,  "reload ok, now the configure of tbusd is:");
    tbusd_log_data(g_ptRelayLog, TBUSD_PRIORITY_INFO, (LPTDRMETA)a_pstAppCtx->stConfData.iMeta,
                   pstCurConf, sizeof(TBUSD));

    return 0;
}


