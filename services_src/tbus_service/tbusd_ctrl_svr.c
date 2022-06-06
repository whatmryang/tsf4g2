#include "tapp/tapp.h"
#include "tbusd_ctrl_svr.h"
#include "tbusd_ctrl_kernel.h"
#include "tbusd.h"
#include "tbusd_log.h"
#include "tbusd_sync.h"

#include "../lib_src/comm/tmempool_i.h"

#define TBUSD_CTRL_CMD_LEN	128
typedef int (*PFNCMDPROC)(TAPPCTX *a_pstAppCtx,  int argc, const char **argv);

struct tagTbusdCmdProc
{
	PFNCMDPROC pfnProcFunc;
	char szCmdName[TBUSD_CTRL_CMD_LEN];
	char *pszHelp;
};
typedef struct tagTbusdCmdProc  TBUSDCMDPROC;
typedef struct tagTbusdCmdProc  *LPTBUSDCMDPROC;


//////////////////////////////////////////////////////////////////

static int tbusd_cmdproc_get_env(TAPPCTX *a_pstAppCtx,  int argc, const  char **argv);
static int tbusd_cmdproc_view_relay_idx(TAPPCTX *a_pstAppCtx,  int argc, const char **argv);
static int tbusd_cmdproc_view_relay_addr(TAPPCTX *a_pstAppCtx,  int argc, const char **argv);
static int tbusd_cmdproc_list_expct_relay(TAPPCTX *a_pstAppCtx,  int argc, const char **argv);
static int tbusd_cmdproc_view_conn(TAPPCTX *a_pstAppCtx,  int argc, const char **argv);
static int tbusd_cmdproc_close_relay_conn(TAPPCTX *a_pstAppCtx,  int argc, const char **argv);
static int tbusd_cmdproc_close_conn(TAPPCTX *a_pstAppCtx,  int argc, const char **argv);
static int tbusd_cmdproc_list_relay(TAPPCTX *a_pstAppCtx,  int argc, const char **argv);
static int tbusd_cmdproc_syn_seq(TAPPCTX *a_pstAppCtx,  int argc, const char **argv);
static int tbusdctrl_get_relay_info(TBUSCTRLRELAYINFO *a_pstRelayInfo, LPRELAY a_pstRelay);
////////////////////////////////////////////////////////////////
/////1234567890123456789012345678901234567890///////////////////
TBUSDCMDPROC g_stTbusdCmdProcTab[] = {
	{&tbusd_cmdproc_get_env,                "TbusdGetEnv",
    "TbusdGetEnv                             :get summarization of tbusd"},
    {&tbusd_cmdproc_list_relay,         "TbusdListRelay",
    "TbusdListRelay          [offset]        :list summarization of all relay, if offset is given, only list scope:[offset,total]"},

    {&tbusd_cmdproc_list_expct_relay,         "TbusdListExceptRelay",
    "TbusdListExceptRelay                    :list exceptional relay"},

    {&tbusd_cmdproc_view_relay_idx,         "TbusdGetRelayByIdx",
    "TbusdGetRelayByIdx      Index           :get relay info by its index"},
    {&tbusd_cmdproc_view_relay_addr,         "TbusdGetRelayByAddr",
    "TbusdGetRelayByAddr     Addr1 Addr2     :get relay info by channel's addr"},

    {&tbusd_cmdproc_view_conn,         "TbusdViewConn",
    "TbusdViewConn           ConnIdx         :get connection info by its index"},

    {&tbusd_cmdproc_close_relay_conn,         "TbusdCloseRelayConn",
    "TbusdCloseRelayConn     RelayIdx        :close connection of the relay by the index of relay"},
    {&tbusd_cmdproc_close_conn,         "TbusdCloseConn",
    "TbusdCloseConn          ConnIdx         :close connection  by its index "},

    {&tbusd_cmdproc_syn_seq,         "TbusdSynSeq",
    "TbusdSynSeq             RelayIdx Seq    :synchronize the the seq of the pkg recved by peer on the relay(by its index)"},

};

///////////////////////////////////////////////////////


//根据命令行获取此命令行的处理函数指针
PFNCMDPROC tbusd_get_cmdproc(const char *a_szCmdLine);

//获取此命令行的参数及使用帮忙
const char* tbusd_get_cmd_usage(const char *a_szCmdLine);

#if defined(_WIN32) || defined(_WIN64)
static void tbusd_send_string2controller(const char *format, ...);
#else
static void tbusd_send_string2controller(const char *format, ...)__attribute__((format(printf,1,2)));
#endif

/////////////////////////////////////////////////////
/*获取进程控制使用帮助*/
const char* tbusd_get_ctrl_usage(void)
{
#define TBUSD_CTRL_USAGE_INFO_MAX_LEN   2048
    static char s_szTbusdCtrlUsage[TBUSD_CTRL_USAGE_INFO_MAX_LEN];
    int iWriteLen;
    int iLeft;
    unsigned int i;
    char *pszCh;

    iLeft = sizeof(s_szTbusdCtrlUsage);
    pszCh = s_szTbusdCtrlUsage;
    for (i = 0; i< sizeof(g_stTbusdCmdProcTab)/sizeof(g_stTbusdCmdProcTab[0]);i++)
    {
        iWriteLen = snprintf(pszCh, iLeft, "%s\r\n", g_stTbusdCmdProcTab[i].pszHelp);
        if ((iWriteLen < 0) ||(iWriteLen >= iLeft))
        {
            break;
        }
        iLeft -= iWriteLen;
        pszCh += iWriteLen;
    }

    s_szTbusdCtrlUsage[sizeof(s_szTbusdCtrlUsage)-1] = '\0';

    return s_szTbusdCtrlUsage;
}


PFNCMDPROC tbusd_get_cmdproc(const char *a_szCmdLine)
{
    unsigned int i;
    PFNCMDPROC pfnProcFunc = NULL;

    assert(NULL != a_szCmdLine);
    for (i = 0; i< sizeof(g_stTbusdCmdProcTab)/sizeof(g_stTbusdCmdProcTab[0]);i++)
    {
        if (stricmp(g_stTbusdCmdProcTab[i].szCmdName, a_szCmdLine)== 0)
        {
            pfnProcFunc = g_stTbusdCmdProcTab[i].pfnProcFunc;
            break;
        }
    }

    return pfnProcFunc;
}

//获取此命令行的参数及使用帮忙
const char* tbusd_get_cmd_usage(const char *a_szCmdLine)
{
    unsigned int i;
    const char *pszUsage = "";

    assert(NULL != a_szCmdLine);
    for (i = 0; i< sizeof(g_stTbusdCmdProcTab)/sizeof(g_stTbusdCmdProcTab[0]);i++)
    {
        if (stricmp(g_stTbusdCmdProcTab[i].szCmdName, a_szCmdLine)== 0)
        {
            pszUsage = g_stTbusdCmdProcTab[i].pszHelp;
            break;
        }
    }

    return pszUsage;
}

/**处理控制命令名
*/
int tbusd_proc_ctrl_cmdline(struct tagTAPPCTX *a_pstCtx, void* a_pvArg, unsigned short argc, const char** argv)
{
    PFNCMDPROC pfnProcFunc = NULL;
    int iRet = 0;

    if ((NULL == a_pstCtx)||(NULL == a_pvArg) ||(0 >= argc)||(NULL == argv)||(NULL == argv[0]))
    {
        return -1;
    }

    pfnProcFunc = tbusd_get_cmdproc(argv[0]);
    if (NULL != pfnProcFunc)
    {
        iRet = (*pfnProcFunc)(a_pstCtx, argc, argv);
    }else
    {
        tbusd_send_string2controller("unsupported command:%s\n", argv[0]);
    }

    return 0;
}

static int tbusd_cmdproc_get_env(TAPPCTX *a_pstAppCtx,  int argc, const char **argv)
{
    TBUSCTRLPKG stResPkg;
    LPTBUSCTRLGETENVINFORES pstEnv;
    int iRet = 0;

    assert(NULL != a_pstAppCtx);
    assert(0 < argc);
    assert(NULL != argv);

    tbusdctrl_fill_pkghead(&stResPkg.stHead, TBUSCTRLCMD_GET_ENVINFO_RES,
        TDR_METALIB_TBUSDCTRL_VERSION);
    pstEnv = &stResPkg.stBody.stEnvInfo;
    pstEnv->dwGCIMKey = ((NULL == gs_stRelayEnv.pstShmGCIM)? 0: gs_stRelayEnv.pstShmGCIM->stHead.dwShmKey);
    pstEnv->dwGCIMVer = gs_stRelayEnv.dwGCIMVersion;
    pstEnv->dwGRMKey = gs_stRelayEnv.stShmGRMBackup.stHead.dwShmKey;
    pstEnv->dwGRMVer = gs_stRelayEnv.stShmGRMBackup.stHead.dwVersion;
    pstEnv->dwRelayCount = gs_stRelayEnv.dwRelayCnt;
    pstEnv->iConnPoolMax = ((NULL == gs_stRelayEnv.ptMemPool) ? -1 : gs_stRelayEnv.ptMemPool->iMax);
    pstEnv->iConnPoolUsed = ((NULL == gs_stRelayEnv.ptMemPool) ? -1 : gs_stRelayEnv.ptMemPool->iUsed);
    tdr_utctime_to_tdrdatetime(&pstEnv->tLastCheckHandShake, gs_stRelayEnv.tLastCheckHandShake);
    tdr_utctime_to_tdrdatetime(&pstEnv->tLastRefreshConf, gs_stRelayEnv.tLastRefreshConf);
    pstEnv->iMaxSendPkgPerLoop = gs_stRelayEnv.iMaxSendPkgPerLoop;

    iRet = tbusdctrl_send_pkg(&stResPkg);

    return iRet;
}

static void tbusdctrl_get_relay_flag(char *a_pszFlag, int a_iSize, LPRELAY a_pstRelay)
{
    int iWriteLen;
    int iLeft;
    char *pch;

    assert(NULL != a_pszFlag);
    assert(0 < a_iSize);
    assert(NULL != a_pstRelay);

    iLeft = a_iSize -1;
    pch = a_pszFlag;
    if (TBUSD_RELAY_IS_ENABLE(a_pstRelay))
    {
        iWriteLen = snprintf(pch, iLeft, "\'Enable\' ");
    }else
    {
        iWriteLen = snprintf(pch, iLeft, "\'Disable\' ");
    }
    iLeft -= iWriteLen;
    pch += iLeft;
    if (iLeft > 0)
    {
        if (TBUSD_RELAY_IS_NOT_IN_CONF(a_pstRelay))
        {
            snprintf(pch, iLeft, "\'Not in conf\' ");
        }
    }/*if (iLeft > 0)*/

    a_pszFlag[a_iSize -1] = '\0';
}

static void tbusdctrl_get_relay_status(char *a_pszStatus, int a_iSize, LPRELAY a_pstRelay)
{
    assert(NULL != a_pszStatus);
    assert(0 < a_iSize);
    assert(NULL != a_pstRelay);

    switch(a_pstRelay->dwRelayStatus)
    {
    case TBUSD_STATUS_NO_CONNECTION:
        snprintf(a_pszStatus, a_iSize -1, "NO_CONNECTION");
        break;
    case TBUSD_STATUS_CONNECTING:
        snprintf(a_pszStatus, a_iSize -1, "CONNECTING");
        break;
    case TBUSD_STATUS_CLIENT_SYN_SEND:
        snprintf(a_pszStatus, a_iSize -1, "CLIENT_SYN_SEND");
        break;
    case TBUSD_STATUS_SERVER_INIT:
        snprintf(a_pszStatus, a_iSize -1, "SERVER_INIT");
        break;
    case TBUSD_STATUS_ESTABLISHED:
        snprintf(a_pszStatus, a_iSize -1, "ESTABLISHED");
        break;
    default:
        snprintf(a_pszStatus, a_iSize -1, "Unkown");
        break;
    }
    a_pszStatus[a_iSize -1] = '\0';
}

static int tbusd_get_relay_channel_info(TBUSCTRLCHANNELINFO *a_pstCtrlChannel, TBUSCHANNEL *a_pstRelayChannel)
{
    int iWriteLen;
    int iLeft;
    char *pszFlag;
    CHANNELHEAD *pstHead ;
    CHANNELVAR *pstQueueVar;
    TBUSCTRLQUEUEVAR *pstCtrlQueVar;

    assert(NULL != a_pstCtrlChannel);
    assert(NULL != a_pstRelayChannel);

    if (NULL == a_pstRelayChannel->pstHead)
    {
        return -1;
    }
    if ((a_pstRelayChannel->iRecvSide >= TBUS_CHANNEL_SIDE_NUM) || (0 > a_pstRelayChannel->iRecvSide))
    {
        return -1;
    }
    if ((a_pstRelayChannel->iSendSide >= TBUS_CHANNEL_SIDE_NUM) || (0 > a_pstRelayChannel->iSendSide))
    {
        return -1;
    }

    //get channel flag
    iLeft = sizeof(a_pstCtrlChannel->szFlag) -1;
    pszFlag = a_pstCtrlChannel->szFlag;
    if (TBUS_CHANNEL_IS_ENABLE(a_pstRelayChannel))
    {
        iWriteLen = snprintf(pszFlag, iLeft, "\'Enable\' ");
    }else
    {
        iWriteLen = snprintf(pszFlag, iLeft, "\'Disable\' ");
    }
    iLeft -= iWriteLen;
    pszFlag += iLeft;
    if (iLeft > 0)
    {
        if (TBUS_CHANNEL_IS_NOT_IN_GCIM(a_pstRelayChannel))
        {
            snprintf(pszFlag, iLeft, "\'NOT_IN_GCIM\' ");
        }
        iLeft -= iWriteLen;
        pszFlag += iLeft;
    }/*if (iLeft > 0)*/
    if (iLeft > 0)
    {
        if (TBUS_CHANNEL_IS_DISABLE_BY_CTRL(a_pstRelayChannel))
        {
            snprintf(pszFlag, iLeft, "\'DISABLE_BY_CTRL\' ");
        }
        iLeft -= iWriteLen;
        pszFlag += iLeft;
    }/*if (iLeft > 0)*/
    if (iLeft > 0)
    {
        if (TBUS_CHANNEL_IS_CONNECTED(a_pstRelayChannel))
        {
            snprintf(pszFlag, iLeft, "\'TBUS_CONNECTED\' ");
        }
        iLeft -= iWriteLen;
        pszFlag += iLeft;
    }/*if (iLeft > 0)*/
    a_pstCtrlChannel->szFlag[sizeof(a_pstCtrlChannel->szFlag) -1] = '\0';

    pstHead = a_pstRelayChannel->pstHead;
    tbus_addr_ntop(pstHead->astAddr[a_pstRelayChannel->iRecvSide],a_pstCtrlChannel->szRecvAddr,
        sizeof(a_pstCtrlChannel->szRecvAddr));
    tbus_addr_ntop(pstHead->astAddr[a_pstRelayChannel->iSendSide],a_pstCtrlChannel->szSendAddr,
        sizeof(a_pstCtrlChannel->szSendAddr));
    a_pstCtrlChannel->iShmID = (int)a_pstRelayChannel->iShmID;
    a_pstCtrlChannel->stChannelHead.iShmID = (int)pstHead->iShmID;
    a_pstCtrlChannel->stChannelHead.dwAlignLevel = pstHead->dwAlignLevel;

    //get recv queue var
    pstQueueVar = TBUS_CHANNEL_VAR_GET(a_pstRelayChannel);
    pstCtrlQueVar = &a_pstCtrlChannel->stChannelHead.stRecvQueueVar;
    pstCtrlQueVar->dwSize = pstQueueVar->dwSize;
    pstCtrlQueVar->dwHead = pstQueueVar->dwHead;
    pstCtrlQueVar->dwTail = pstQueueVar->dwTail;
    pstCtrlQueVar->dwGet = pstQueueVar->dwGet;
    pstCtrlQueVar->iSeq = pstQueueVar->iSeq;
    pstCtrlQueVar->iRecvSeq = pstQueueVar->iRecvSeq;


    //get send queue var
    pstQueueVar = TBUS_CHANNEL_VAR_PUSH(a_pstRelayChannel);
    pstCtrlQueVar = &a_pstCtrlChannel->stChannelHead.stSendQueueVar;
    pstCtrlQueVar->dwSize = pstQueueVar->dwSize;
    pstCtrlQueVar->dwHead = pstQueueVar->dwHead;
    pstCtrlQueVar->dwTail = pstQueueVar->dwTail;
    pstCtrlQueVar->dwGet = pstQueueVar->dwGet;
    pstCtrlQueVar->iSeq = pstQueueVar->iSeq;
    pstCtrlQueVar->iRecvSeq = pstQueueVar->iRecvSeq;

    tdr_utctime_to_tdrdatetime(&a_pstCtrlChannel->tBeginDisabled, a_pstRelayChannel->tBeginDisable);

    return 0;
}


static int tbusdctrl_get_conn_info(TBUSCTRLCONNNODE *a_pstCtrlConnInfo, LPCONNNODE a_pstConn)
{
    assert(NULL != a_pstCtrlConnInfo);
    assert(NULL != a_pstConn);

    a_pstCtrlConnInfo->iIdxInPool = a_pstConn->idx;
    a_pstCtrlConnInfo->iIsConnected = a_pstConn->bIsConnected;

    switch(a_pstConn->dwType)
    {
    case RELAY_FD_LISTEN_MODE:
        STRNCPY(a_pstCtrlConnInfo->szType, "Listen mode", sizeof(a_pstCtrlConnInfo->szType));
        break;
    case RELAY_FD_SERVER_MODE:
        STRNCPY(a_pstCtrlConnInfo->szType, "Server mode", sizeof(a_pstCtrlConnInfo->szType));
        break;
    case RELAY_FD_CLIENT_MODE:
        STRNCPY(a_pstCtrlConnInfo->szType, "Client mode", sizeof(a_pstCtrlConnInfo->szType));
        break;
    default:
        STRNCPY(a_pstCtrlConnInfo->szType, "unexpect mode", sizeof(a_pstCtrlConnInfo->szType));
        break;
    }
    a_pstCtrlConnInfo->iFd = a_pstConn->iFd;
    a_pstCtrlConnInfo->ulPeerIP = a_pstConn->stAddr.sin_addr.s_addr;
    a_pstCtrlConnInfo->dwPeerPort = ntohs(a_pstConn->stAddr.sin_port);
    tdr_utctime_to_tdrdatetime(&a_pstCtrlConnInfo->tCreate, a_pstConn->tCreate);

    a_pstCtrlConnInfo->iNeedFree = a_pstConn->bNeedFree;
    if (NULL == a_pstConn->pstBindRelay)
    {
        a_pstCtrlConnInfo->iBindRelayID = -1;
    }else
    {
        a_pstCtrlConnInfo->iBindRelayID = a_pstConn->pstBindRelay->iID;
    }
    a_pstCtrlConnInfo->iNeedSynSeq = a_pstConn->bNeedSynSeq;
    a_pstCtrlConnInfo->iLastAckSeq = a_pstConn->iLastAckSeq;
    tdr_utctime_to_tdrdatetime(&a_pstCtrlConnInfo->tLastSynSeq, a_pstConn->tLastSynSeq);

    a_pstCtrlConnInfo->iSendingDataLen = a_pstConn->stSendMng.iSendBuff;
    a_pstCtrlConnInfo->iSendingDataPos = a_pstConn->stSendMng.iSendPos;
    switch(a_pstConn->stSendMng.enRemainDataType)
    {
    case TBUSD_DATA_TYPE_SYN:
            STRNCPY(a_pstCtrlConnInfo->szSendingDataType, "Syn", sizeof(a_pstCtrlConnInfo->szSendingDataType));
            break;
    case TBUSD_DATA_TYPE_BUSINESS:
        STRNCPY(a_pstCtrlConnInfo->szSendingDataType, "Business", sizeof(a_pstCtrlConnInfo->szSendingDataType));
        break;
    default:
        STRNCPY(a_pstCtrlConnInfo->szSendingDataType, "Unkown", sizeof(a_pstCtrlConnInfo->szSendingDataType));
        break;
    }

    a_pstCtrlConnInfo->iRecvBuffSize = a_pstConn->iRecvBuffSize;
    a_pstCtrlConnInfo->iRecvedDataLen = a_pstConn->iBuff;

    return 0;
}

int tbusdctrl_get_relay_info(TBUSCTRLRELAYINFO *a_pstRelayInfo, LPRELAY a_pstRelay)
{
    int iRet = 0;

    assert(NULL != a_pstRelayInfo);
    assert(NULL != a_pstRelay);

    //设置tbusd 通道相关数据结构的信息
    a_pstRelayInfo->iID = a_pstRelay->iID;
    tbusdctrl_get_relay_flag(a_pstRelayInfo->szFlag, sizeof(a_pstRelayInfo->szFlag), a_pstRelay);
    a_pstRelayInfo->iID = a_pstRelay->iID;
    tbusdctrl_get_relay_status(a_pstRelayInfo->szStatus, sizeof(a_pstRelayInfo->szStatus), a_pstRelay);

    tbus_addr_ntop(a_pstRelay->stRelayInfo.dwAddr, a_pstRelayInfo->stRelayConf.szRelayAddr,
        sizeof(a_pstRelayInfo->stRelayConf.szRelayAddr));
    STRNCPY(a_pstRelayInfo->stRelayConf.szRelayHost, a_pstRelay->stRelayInfo.szMConn,
        sizeof(a_pstRelayInfo->stRelayConf.szRelayHost));

    //拷贝通道配置信息
    iRet = tbusd_get_relay_channel_info(&a_pstRelayInfo->stChannelInfo, &a_pstRelay->stChl);
    if (0 != iRet)
    {
        return iRet;
    }

    if (NULL == a_pstRelay->pConnNode)
    {
        a_pstRelayInfo->iIsBindConn = 0;
    }else
    {
        a_pstRelayInfo->iIsBindConn = 1;

        tbusdctrl_get_conn_info(&a_pstRelayInfo->stConnNode, (LPCONNNODE)a_pstRelay->pConnNode);
    }

    tdr_utctime_to_tdrdatetime(&a_pstRelayInfo->tLastConnectPeer, a_pstRelay->tLastConnectPeer);
    tdr_utctime_to_tdrdatetime(&a_pstRelayInfo->tLastHeartbeat, a_pstRelay->tLastHeartbeat);
    tdr_utctime_to_tdrdatetime(&a_pstRelayInfo->tBeginDisabled, a_pstRelay->tBeginDisabled);

    return 0;
}


static void tbusd_send_string2controller(const char *format, ...)
{
    char szInfo[1024];
    va_list ap;

    va_start(ap, format);
    vsnprintf(szInfo, sizeof(szInfo)-1, format, ap);
    szInfo[sizeof(szInfo)-1] = '\0';
    va_end(ap);

    tappctrl_send_string(szInfo);

}

static int tbusd_cmdproc_view_relay_idx(TAPPCTX *a_pstAppCtx,  int argc, const char **argv)
{

    long index;
    LPRELAY pstRelay;
    TBUSCTRLPKG stResPkg;
    int iRet = 0;


    assert(NULL != a_pstAppCtx);
    assert(NULL != argv);
    assert(NULL != argv[0]);

    /*参数检查*/
    if (2 > argc)
    {
        tbusd_send_string2controller("miss param, usage:%s\n", tbusd_get_cmd_usage(argv[0]));
        return -1;
    }
    if (NULL == argv[1])
    {
        tbusd_send_string2controller("CtrlCmd:%s  first param(index of relay) should not be null\n",
            argv[0]);
        return -1;
    }
    index = strtol(argv[1], NULL, 0);
    if ((index < 0) ||(index >= (long)gs_stRelayEnv.dwRelayCnt))
    {
        tbusd_send_string2controller("CtrlCmd:%s  invalid index(%ld), its should be [0,%u)\n",
            argv[0],index, gs_stRelayEnv.dwRelayCnt);
        return -1;
    }
    pstRelay = gs_stRelayEnv.pastTbusRelay[index];
    if (NULL == pstRelay)
    {
        tbusd_send_string2controller("CtrlCmd:%s  relay(ID:%ld) is null\n",
            argv[0],index);
        return -1;
    }

    //generate res
    tbusdctrl_fill_pkghead(&stResPkg.stHead, TBUSCTRLCMD_GET_RELAYINFO_RES,
        TDR_METALIB_TBUSDCTRL_VERSION);

    memset(&stResPkg.stBody.stRelayInfo, 0, sizeof(stResPkg.stBody.stRelayInfo));
    iRet = tbusdctrl_get_relay_info(&stResPkg.stBody.stRelayInfo, pstRelay);
    if (0 != iRet)
    {
        tbusd_send_string2controller("CtrlCmd:%s  relay(ID:%ld)'s data is invalid, so only get part data\n",
            argv[0],index);
    }

    iRet = tbusdctrl_send_pkg(&stResPkg);

    return iRet;
}

static int tbusd_cmdproc_view_relay_addr(TAPPCTX *a_pstAppCtx,  int argc, const char **argv)
{
    TBUSADDR addr1;
    TBUSADDR addr2;
    LPRELAY pstRelay = NULL;
    TBUSCTRLPKG stResPkg;
    int iRet = 0;
    unsigned int i;


    assert(NULL != a_pstAppCtx);
    assert(NULL != argv);
    assert(NULL != argv[0]);

    /*参数检查*/
    if (3 > argc)
    {
        tbusd_send_string2controller("miss param, usage:%s\n", tbusd_get_cmd_usage(argv[0]));
        return -1;
    }

    //get addr1
    if (NULL == argv[1])
    {
        tbusd_send_string2controller("CtrlCmd:%s  first address should not be null\n",
            argv[0]);
        return -1;
    }
    iRet = tbus_addr_aton(argv[1], &addr1);
    if (0 != iRet)
    {
        tbusd_send_string2controller("CtrlCmd:%s  invalid channel address(%s): %s\n",
            argv[0],argv[1], tbus_error_string(iRet));
        return -1;
    }

    //get addr2
    if (NULL == argv[2])
    {
        tbusd_send_string2controller("CtrlCmd:%s  sencond address should not be null\n",
            argv[0]);
        return -1;
    }
    iRet = tbus_addr_aton(argv[2], &addr2);
    if (0 != iRet)
    {
        tbusd_send_string2controller("CtrlCmd:%s  invalid channel address(%s): %s\n",
            argv[0],argv[2], tbus_error_string(iRet));
        return -1;
    }


    //查找与地址匹配的通道
    for (i = 0; i < gs_stRelayEnv.dwRelayCnt; i++)
    {
        CHANNELHEAD *pstHead ;
        pstRelay = gs_stRelayEnv.pastTbusRelay[i];
        if (NULL == pstRelay)
        {
            continue;
        }
        pstHead = pstRelay->stChl.pstHead;
        if (NULL == pstHead)
        {
            tlog_fatal(g_ptRelayLog, pstRelay->iID, 0, "relay(id:%d)'s channel manage head is null", pstRelay->iID);
            continue;
        }
        if (((pstHead->astAddr[0] == addr1) && (pstHead->astAddr[1] == addr2)) ||
            ((pstHead->astAddr[1] == addr1) && (pstHead->astAddr[0] == addr2)))
        {
            break;
        }
    }/*for (i = 0; i < gs_stRelayEnv.dwRelayCnt; i++)*/
    if (i >= gs_stRelayEnv.dwRelayCnt)
    {
        tbusd_send_string2controller("CtrlCmd:%s  no relay channel match the address(%s <-- > %s)\n",
            argv[0],argv[1], argv[2]);
        return -1;
    }

    //generate res
    tbusdctrl_fill_pkghead(&stResPkg.stHead, TBUSCTRLCMD_GET_RELAYINFO_RES,
        TDR_METALIB_TBUSDCTRL_VERSION);

    memset(&stResPkg.stBody.stRelayInfo, 0, sizeof(stResPkg.stBody.stRelayInfo));
    iRet = tbusdctrl_get_relay_info(&stResPkg.stBody.stRelayInfo, pstRelay);
    if (0 != iRet)
    {
        tbusd_send_string2controller("CtrlCmd:%s  relay(ID:%d)'s data is invalid, so only get part data\n",
            argv[0],pstRelay->iID);
    }

    iRet = tbusdctrl_send_pkg(&stResPkg);

    return iRet;
}

static int tbusd_cmdproc_list_expct_relay(TAPPCTX *a_pstAppCtx,  int argc, const char **argv)
{

    unsigned int i;
    LPRELAY pstRelay;
    TBUSCTRLPKG stResPkg;
    int iRet = 0;

    assert(NULL != a_pstAppCtx);
    assert(0 < argc);

    TOS_UNUSED_ARG(argv);

    //generate res
    tbusdctrl_fill_pkghead(&stResPkg.stHead, TBUSCTRLCMD_GET_EXPCT_RELAYINFO_RES,
        TDR_METALIB_TBUSDCTRL_VERSION);


    //查找状态 不是“TBUSD_STATUS_ESTABLISHED”的relay数据
    stResPkg.stBody.stExpctRelayInfo.iCount = 0;
    for (i = 0; i < gs_stRelayEnv.dwRelayCnt; i++)
    {
        CHANNELHEAD *pstHead ;
        TBUSCTRLEXPCTRELAYINFO *pstExpctRelay;
        pstRelay = gs_stRelayEnv.pastTbusRelay[i];
        if (NULL == pstRelay)
        {
            continue;
        }
        pstHead = pstRelay->stChl.pstHead;
        if (NULL == pstHead)
        {
            tlog_fatal(g_ptRelayLog, pstRelay->iID, 0, "relay(id:%d)'s channel manage head is null", pstRelay->iID);
            continue;
        }
        if (TBUSD_STATUS_ESTABLISHED == pstRelay->dwRelayStatus)
        {
            continue;
        }


        //获取此异常relay通道的信息
        pstExpctRelay = stResPkg.stBody.stExpctRelayInfo.astExpctRelay + stResPkg.stBody.stExpctRelayInfo.iCount;
        pstExpctRelay->iID = pstRelay->iID;
        pstExpctRelay->iConnIdx = (pstRelay->pConnNode == NULL)? -1: pstRelay->pConnNode->idx;
        tbusdctrl_get_relay_flag(pstExpctRelay->szFlag, sizeof(pstExpctRelay->szFlag), pstRelay);
        tbusdctrl_get_relay_status(pstExpctRelay->szStatus, sizeof(pstExpctRelay->szStatus), pstRelay);



        if (NULL == pstRelay->pConnNode)
        {
            pstExpctRelay->iIsBindConn = 0;
            pstExpctRelay->iConnID = -1;
        }else
        {
            pstExpctRelay->iIsBindConn = 1;
            pstExpctRelay->iConnID = ((LPCONNNODE)(pstRelay->pConnNode))->idx;
        }

        tdr_utctime_to_tdrdatetime(&pstExpctRelay->tLastConnectPeer, pstRelay->tLastConnectPeer);
        tdr_utctime_to_tdrdatetime(&pstExpctRelay->tLastHeartbeat, pstRelay->tLastHeartbeat);
        tdr_utctime_to_tdrdatetime(&pstExpctRelay->tBeginDisabled, pstRelay->tBeginDisabled);

        stResPkg.stBody.stExpctRelayInfo.iCount++;
        if (TBUSD_CTRL_MAX_RELAY_NUM_PERRES <= stResPkg.stBody.stExpctRelayInfo.iCount)
        {
            tbusdctrl_send_pkg(&stResPkg);
            stResPkg.stBody.stExpctRelayInfo.iCount = 0;
        }
    }/*for (i = 0; i < gs_stRelayEnv.dwRelayCnt; i++)*/


    if (0 < stResPkg.stBody.stExpctRelayInfo.iCount)
    {
         iRet = tbusdctrl_send_pkg(&stResPkg);
    }

    return iRet;
}

static int tbusd_cmdproc_view_conn(TAPPCTX *a_pstAppCtx,  int argc, const char **argv)
{
    long index;
    LPCONNNODE pstConn;
    TBUSCTRLPKG stResPkg;
    int iRet = 0;


    assert(NULL != a_pstAppCtx);
    assert(NULL != argv);
    assert(NULL != argv[0]);

    /*参数检查*/
    if (2 > argc)
    {
        tbusd_send_string2controller("miss param, usage:%s\n", tbusd_get_cmd_usage(argv[0]));
        return -1;
    }
    if (NULL == argv[1])
    {
        tbusd_send_string2controller("CtrlCmd:%s  first param(index of relay) should not be null\n",
            argv[0]);
        return -1;
    }
    index = strtol(argv[1], NULL, 0);


    //get conn
    if (NULL == gs_stRelayEnv.ptMemPool)
    {
        tbusd_send_string2controller("CtrlCmd:%s  the memory pool of connections is null\n",
            argv[0]);
        return -1;
    }
    pstConn = (LPCONNNODE)tmempool_get(gs_stRelayEnv.ptMemPool, index);
    if (NULL == pstConn)
    {
        tbusd_send_string2controller("CtrlCmd:%s  cannnot get connection info by index(%ld)\n",
            argv[0],index);
        return -1;
    }


    //generate res
    tbusdctrl_fill_pkghead(&stResPkg.stHead, TBUSCTRLCMD_GET_CONNINFO_RES,
        TDR_METALIB_TBUSDCTRL_VERSION);


    tbusdctrl_get_conn_info(&stResPkg.stBody.stConnInfo, pstConn);

    iRet = tbusdctrl_send_pkg(&stResPkg);

    return iRet;
}

static int tbusd_cmdproc_close_relay_conn(TAPPCTX *a_pstAppCtx,  int argc, const char **argv)
{
    long index;
    LPRELAY pstRelay;
    int iRet = 0;
    LPCONNNODE pstConn;

    assert(NULL != a_pstAppCtx);
    assert(NULL != argv);
    assert(NULL != argv[0]);

    /*参数检查*/
    if (2 > argc)
    {
        tbusd_send_string2controller("miss param, usage:%s\n", tbusd_get_cmd_usage(argv[0]));
        return -1;
    }
    if (NULL == argv[1])
    {
        tbusd_send_string2controller("CtrlCmd:%s  first param(index of relay) should not be null\n",
            argv[0]);
        return -1;
    }
    index = strtol(argv[1], NULL, 0);
    if ((index < 0) ||(index >= (long)gs_stRelayEnv.dwRelayCnt))
    {
        tbusd_send_string2controller("CtrlCmd:%s  invalid index(%ld), its should be [0,%u)\n",
            argv[0],index, gs_stRelayEnv.dwRelayCnt);
        return -1;
    }
    pstRelay = gs_stRelayEnv.pastTbusRelay[index];
    if (NULL == pstRelay)
    {
       tbusd_send_string2controller( "CtrlCmd:%s  relay(ID:%ld) is null\n",
            argv[0],index);
        return -1;
    }
    pstConn = (LPCONNNODE)pstRelay->pConnNode;
    if (NULL == pstRelay->pConnNode)
    {
        tbusd_send_string2controller( "CtrlCmd:%s  relay(ID:%ld) no connection binded\n",
            argv[0],index);
        return -1;
    }

    //断开链接
    tlog_fatal(g_ptRelayLog, pstRelay->iID, 0, "recv cmd which to close the connection(idx:%d fd:%d) of the relay(%d) from controller, so close it",
        pstConn->idx, pstConn->iFd, pstRelay->iID);
    tbusd_destroy_conn(&gs_stRelayEnv, pstConn, NULL);
    tbusd_send_string2controller( "CtrlCmd:%s  close the connection of the relay(ID:%ld) successfully!\n",
        argv[0],index);

    return iRet;
}

int tbusd_cmdproc_close_conn(TAPPCTX *a_pstAppCtx,  int argc, const char **argv)
{
    long index;
    LPCONNNODE pstConn;
    int iRet = 0;


    assert(NULL != a_pstAppCtx);
    assert(NULL != argv);
    assert(NULL != argv[0]);

    /*参数检查*/
    if (2 > argc)
    {
        tbusd_send_string2controller("miss param, usage:%s\n", tbusd_get_cmd_usage(argv[0]));
        return -1;
    }
    if (NULL == argv[1])
    {
        tbusd_send_string2controller("CtrlCmd:%s  first param(index of relay) should not be null\n",
            argv[0]);
        return -1;
    }
    index = strtol(argv[1], NULL, 0);


    //get conn
    if (NULL == gs_stRelayEnv.ptMemPool)
    {
        tbusd_send_string2controller("CtrlCmd:%s  the memory pool of connections is null\n",
            argv[0]);
        return -1;
    }
    pstConn = (LPCONNNODE)tmempool_get(gs_stRelayEnv.ptMemPool, index);
    if (NULL == pstConn)
    {
        tbusd_send_string2controller("CtrlCmd:%s  cannnot get connection info by index(%ld)\n",
            argv[0],index);
        return -1;
    }

    //断开链接

    tlog_fatal(g_ptRelayLog, 0, 0, "recv cmd which to close the connection(idx:%d fd:%d)  from controller, so close it",
        pstConn->idx, pstConn->iFd);
    tbusd_destroy_conn(&gs_stRelayEnv, pstConn, NULL);
    tbusd_send_string2controller( "CtrlCmd:%s  close the connection (ID:%ld) successfully!\n",
        argv[0],index);

    return iRet;
}

int tbusd_cmdproc_list_relay(TAPPCTX *a_pstAppCtx,  int argc, const char **argv)
{
    long offset = 0;
    TBUSCTRLPKG stResPkg;
    int iRet = 0;

    unsigned int i;

    assert(NULL != a_pstAppCtx);
    assert(NULL != argv);
    assert(NULL != argv[0]);

    /*参数检查*/
    if (2 <= argc)
    {
        if (NULL == argv[1])
        {
            tbusd_send_string2controller("CtrlCmd:%s  first param(index of relay) should not be null\n",
                argv[0]);
            return -1;
        }
        offset = strtol(argv[1], NULL, 0);
        if ((offset < 0) ||(offset >= (long)gs_stRelayEnv.dwRelayCnt))
        {
            tbusd_send_string2controller("CtrlCmd:%s  invalid index(%ld), its should be [0,%u)\n",
                argv[0],offset, gs_stRelayEnv.dwRelayCnt);
            return -1;
        }
    }/*if (2 <= argc)*/


    //查找状态 不是“TBUSD_STATUS_ESTABLISHED”的relay数据
    tbusdctrl_fill_pkghead(&stResPkg.stHead, TBUSCTRLCMD_LIST_RELAYINFO_RES,
        TDR_METALIB_TBUSDCTRL_VERSION);

    stResPkg.stBody.stListRelay.iCount = 0;
    for (i = (int)offset; i < gs_stRelayEnv.dwRelayCnt; i++)
    {
        CHANNELHEAD *pstHead ;
        TBUSCTRLRELAYSUMINFO *pstRelaySum;
        LPRELAY pstRelay = gs_stRelayEnv.pastTbusRelay[i];
        if (NULL == pstRelay)
        {
            continue;
        }
        pstHead = pstRelay->stChl.pstHead;
        if (NULL == pstHead)
        {
            tlog_fatal(g_ptRelayLog, pstRelay->iID, 0, "relay(id:%d)'s channel manage head is null", pstRelay->iID);
            tbusd_send_string2controller("relay(id:%d)'s channel manage head is null", pstRelay->iID);
            continue;
        }
        if ((pstRelay->stChl.iRecvSide >= TBUS_CHANNEL_SIDE_NUM) || (0 > pstRelay->stChl.iRecvSide))
        {
            tbusd_send_string2controller("relay(id:%d)'s recv side id(%d) is invalid",
                pstRelay->iID, pstRelay->stChl.iRecvSide);
            continue;
        }
        if ((pstRelay->stChl.iSendSide >= TBUS_CHANNEL_SIDE_NUM) || (0 > pstRelay->stChl.iSendSide))
        {
            tbusd_send_string2controller("relay(id:%d)'s send side id(%d) is invalid",
                pstRelay->iID, pstRelay->stChl.iSendSide);
            continue;
        }


        //获取此异常relay通道的信息
        pstRelaySum = stResPkg.stBody.stListRelay.astRelay + stResPkg.stBody.stListRelay.iCount;
        pstRelaySum->iID = pstRelay->iID;
        pstRelaySum->iConnIdx = (pstRelay->pConnNode == NULL)? -1: pstRelay->pConnNode->idx;
        tbusdctrl_get_relay_flag(pstRelaySum->szFlag, sizeof(pstRelaySum->szFlag), pstRelay);
        tbusdctrl_get_relay_status(pstRelaySum->szStatus, sizeof(pstRelaySum->szStatus), pstRelay);



        tbus_addr_ntop(pstHead->astAddr[pstRelay->stChl.iRecvSide],pstRelaySum->szRecvAddr,
            sizeof(pstRelaySum->szRecvAddr));
        tbus_addr_ntop(pstHead->astAddr[pstRelay->stChl.iSendSide],pstRelaySum->szSendAddr,
            sizeof(pstRelaySum->szSendAddr));


        stResPkg.stBody.stListRelay.iCount++;
        if (TBUSD_CTRL_MAX_RELAY_NUM_PERRES <= stResPkg.stBody.stListRelay.iCount)
        {
            tbusdctrl_send_pkg(&stResPkg);
            stResPkg.stBody.stListRelay.iCount = 0;
        }

    }/*for (i = 0; i < gs_stRelayEnv.dwRelayCnt; i++)*/


    if (0 < stResPkg.stBody.stListRelay.iCount)
    {
        iRet = tbusdctrl_send_pkg(&stResPkg);
    }

    return iRet;

}

static int tbusd_cmdproc_syn_seq(TAPPCTX *a_pstAppCtx,  int argc, const char **argv)
{
    long index;
    LPRELAY pstRelay;

    int iSeq;
    int iRet = 0;


    assert(NULL != a_pstAppCtx);
    assert(NULL != argv);
    assert(NULL != argv[0]);

    /*参数检查*/
    if (3 > argc)
    {
        tbusd_send_string2controller("miss param, usage:%s\n", tbusd_get_cmd_usage(argv[0]));
        return -1;
    }
    if (NULL == argv[1])
    {
        tbusd_send_string2controller("CtrlCmd:%s  first param(index of relay) should not be null\n",
            argv[0]);
        return -1;
    }
    index = strtol(argv[1], NULL, 0);
    if ((index < 0) ||(index >= (long)gs_stRelayEnv.dwRelayCnt))
    {
        tbusd_send_string2controller("CtrlCmd:%s  invalid index(%ld), its should be [0,%u)\n",
            argv[0],index, gs_stRelayEnv.dwRelayCnt);
        return -1;
    }
    pstRelay = gs_stRelayEnv.pastTbusRelay[index];
    if (NULL == pstRelay)
    {
        tbusd_send_string2controller("CtrlCmd:%s  relay(ID:%ld) is null\n",
            argv[0],index);
        return -1;
    }
    if (NULL == argv[2])
    {
        tbusd_send_string2controller("CtrlCmd:%s  second param(recved sequnce to syn) should not be null\n",
            argv[0]);
        return -1;
    }
    iSeq = (int)strtol(argv[2], NULL, 0);

    //generate res
    tlog_fatal(g_ptRelayLog, 0, 0, "recv cmd which to synchronize the the seq(%d) of the pkg recved by peer "
        " on the relay(id:%d)  from controller, so syn it",
        iSeq, pstRelay->iID);

    tbusd_syn_seq(&pstRelay->stChl, iSeq);

    tbusd_send_string2controller( "CtrlCmd:%s  syn seq(%d) on the relay(ID:%d) successfully!\n",
        argv[0], iSeq, pstRelay->iID);


    return iRet;
}
