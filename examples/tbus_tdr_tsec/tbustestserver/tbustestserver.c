/*
 **  @file RCSfile
 **  general description of this module
 **  Id
 **  @author Author
 **  @date Date
 **  @version Revision
 **  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
 **  @note Platform: Linux
 */
#define __TSF4G_INCLUDE_HEADERS
#include "tapp/tapp.h"
#include "tdr/tdr.h"
#include "tlog/tlog.h"
#include "tloghelp/tlogload.h"
#include "tbustestserver_conf_desc.h"
#include "version.h"
#include "tbus/tbus.h"
#include "tsec/tsec.h"
#include "pal/pal.h"
#include "tbustestCS.h"
#include "tsec_wrapper.h"

struct tagtbustestserverEnv
{
    TBUSTESTSERVERCONF *pstConf;
    TBUSTESTSERVERRUN_CUMULATE *pstRunCumu;
    TBUSTESTSERVERRUN_STATUS *pstRunStat;
    TLOGCATEGORYINST* pstLogCat;

    LPTDRMETALIB pstCSProtoMetalib;
    LPTDRMETA pstCSProtoMeta;
    TBUSADDR stServerTbusAddr;
    int tBusHandle;
    uint64_t recvCount;
    uint64_t sendCount;
};

typedef struct tagtbustestserverEnv TBUSTESTSERVERENV;
typedef struct tagtbustestserverEnv *LTBUSTESTSERVERENV;

static TAPPCTX gs_stAppCtx;

static TSECWRAPPER gs_stTsec;
extern unsigned char g_szMetalib_tbustestserver[];

TBUSTESTSERVERENV gs_stEnv;
static TBUSTESTSERVERRUN_CUMULATE gs_stCumu;
static TBUSTESTSERVERRUN_STATUS gs_stStat;

#define _EXTERNAL_VAR_
extern unsigned char g_szMetalib_tbustestcs[];

#define TBUS_TEST_ENCRYPT_KEY_LEN 16
char g_szTbusTestEncryptKey[TBUS_TEST_ENCRYPT_KEY_LEN] = "0123456789abcdef";

////////////////////////////////////////////////////////////////////////////////////////

int butstestserver_init_checksum(TBUSTESTSERVERENV *pstEnv,
        TBUSTESTCSPKG *pstPkg);
int tbustestserver_check_package(TBUSTESTSERVERENV *pstEnv,
        TBUSTESTCSPKG *pstPkg);
int tbustestserver_init_tdr(TAPPCTX *pstAppCtx, TBUSTESTSERVERENV *pstEnv);
int tbustestserver_init_tbus(TAPPCTX *pstAppCtx, TBUSTESTSERVERENV *pstEnv);
int tbustestserver_init_tlog(TAPPCTX *pstAppCtx, TBUSTESTSERVERENV *pstEnv);

int tbustestserver_recv(TBUSTESTSERVERENV *pstEnv, char *szbuff, size_t *ibuff,
        int *a_piSrc);
int tbustestserver_send(TBUSTESTSERVERENV *pstEnv, char *szbuff, size_t ibuff,
        int a_iDst);

int tbustestserver_recv_msg(TBUSTESTSERVERENV *pstEnv, LPTBUSTESTCSPKG pstPkg,
        int *iSrc);
int tbustestserver_send_msg(TBUSTESTSERVERENV *pstEnv, LPTBUSTESTCSPKG pstPkg,
        int iDest);
// Function Imple
int tbustestserver_init_package(TBUSTESTSERVERENV *pstEnv,
        TBUSTESTCSPKG *pstPkg);

int tbustestserver_init_tdr(TAPPCTX *pstAppCtx, TBUSTESTSERVERENV *pstEnv)
{
    TOS_UNUSED_ARG(pstAppCtx);
    pstEnv->pstCSProtoMetalib = (void *) g_szMetalib_tbustestcs;
    pstEnv->pstCSProtoMeta = tdr_get_meta_by_name(pstEnv->pstCSProtoMetalib,
            "TBUSTESTCSPkg");
    if (!pstEnv->pstCSProtoMeta)
    {
        printf("Failed to find metalib CSPkg\n");
        return -1;
    }
    return 0;
}

int tbustestserver_init_tbus(TAPPCTX *pstAppCtx, TBUSTESTSERVERENV *pstEnv)
{

    pstEnv->tBusHandle = pstAppCtx->iBus;
    pstEnv->stServerTbusAddr = pstAppCtx->iId;
    pstEnv->recvCount = 0L;
    pstEnv->sendCount = 0L;

    return 0;
}

int tbustestserver_init_tlog(TAPPCTX *pstAppCtx, TBUSTESTSERVERENV *pstEnv)
{
    TOS_UNUSED_ARG(pstAppCtx);
    tapp_get_category(NULL, &pstEnv->pstLogCat);
    return 0;
}

int tbustestserver_recv(TBUSTESTSERVERENV *pstEnv, char *szbuff, size_t *ibuff,
        int *a_piSrc)
{
    int iRet;
    int iDst = pstEnv->stServerTbusAddr;
    int iSrc = 0;
    char szEncryptBuff[CS_PKG_MAX_PKG_LEN];
    size_t iEncryptLen = sizeof(szEncryptBuff);

    if (0 != pstEnv->pstConf->iEncryptMsg)
    {
        int iMsgLen;
        iRet = tbus_recv(pstEnv->tBusHandle, &iSrc, &iDst, szEncryptBuff,
                &iEncryptLen, 0);
        if (0 == iRet)
        {
            iMsgLen = (int) *ibuff;
            if (tsec_wrapper_decrypt(&gs_stTsec, szEncryptBuff, iEncryptLen,
                    &szbuff, &iMsgLen, 0, 1))
            {
                tlog_error(pstEnv->pstLogCat, 0, 0,
                        "failed to oi_symmetry_decrypt2, iRet: %d", iRet);
                iRet = -1;
            }
            /*
             if (!oi_symmetry_decrypt2(szEncryptBuff, iEncryptLen,
             g_szTbusTestEncryptKey, szbuff, &iMsgLen))
             {
             tlog_error(pstEnv->pstLogCat, 0, 0,
             "failed to oi_symmetry_decrypt2, iRet: %d", iRet);
             iRet = -1;
             }*/
            *ibuff = iMsgLen;
        }
    }
    else
    {
        iRet = tbus_recv(pstEnv->tBusHandle, &iSrc, &iDst, szbuff, ibuff, 0);
    }

    if (0 == iRet)
    {
        *a_piSrc = iSrc;
        pstEnv->pstRunCumu->iRecvPackCount++;
    }

    return iRet;
}

int tbustestserver_send(TBUSTESTSERVERENV *pstEnv, char *szbuff, size_t ibuff,
        int a_iDst)
{
    int iRet;
    int iSrc = pstEnv->stServerTbusAddr;
    int iDst = a_iDst;
    //printf("Src = %d,Dest = %d\n",iSrc,iDst);
    char szEncryptBuff[CS_PKG_MAX_PKG_LEN];
    char *pszEncBuff = szEncryptBuff;
    int iEncryptLen = sizeof(szEncryptBuff);

    if (0 != pstEnv->pstConf->iEncryptMsg)
    {
        if (tsec_wrapper_encrypt(&gs_stTsec, szbuff, ibuff, &pszEncBuff,
                &iEncryptLen, 1))
        {
            return -1;
        }
        /*
         oi_symmetry_encrypt2(szbuff, ibuff, g_szTbusTestEncryptKey,
         szEncryptBuff, &iEncryptLen);
         */
        iRet = tbus_send(pstEnv->tBusHandle, &iSrc, &iDst, szEncryptBuff,
                (size_t) iEncryptLen, TBUS_FLAG_WITH_TIMESTAMP|TBUS_FLAG_START_DYE_MSG
				);
    }
    else
    {
        iRet = tbus_send(pstEnv->tBusHandle, &iSrc, &iDst, szbuff, ibuff, TBUS_FLAG_WITH_TIMESTAMP|TBUS_FLAG_START_DYE_MSG);
    }

    if (0 == iRet)
    {
        pstEnv->pstRunCumu->iSendPackCount++;
    }
    return iRet;
}

int tbustestserver_recv_msg(TBUSTESTSERVERENV *pstEnv, LPTBUSTESTCSPKG pstPkg,
        int *iSrc)
{
    int iRet;
    TDRDATA stNet;
    TDRDATA stHost;
    char recvBuff[CS_PKG_MAX_PKG_LEN];
    size_t len = sizeof(recvBuff);

    iRet = tbustestserver_recv(pstEnv, recvBuff, &len, iSrc);

    if (0 != iRet)
    {
        return iRet;
    }

    tlog_debug_hex(pstEnv->pstLogCat,0,0,recvBuff,len);

    stNet.iBuff = len;
    stNet.pszBuff = recvBuff;

    stHost.iBuff = sizeof(*pstPkg);
    stHost.pszBuff = (char *) pstPkg;

    iRet = tdr_ntoh(pstEnv->pstCSProtoMeta, &stHost, &stNet, 0);
    if (0 != iRet)
    {
        tlog_error(pstEnv->pstLogCat, 0, 0, "failed to unpack the packet %s",
                tdr_error_string(iRet));
        return iRet;
    }

    return 0;
}

int tbustestserver_send_msg(TBUSTESTSERVERENV *pstEnv, LPTBUSTESTCSPKG pstPkg,
        int iDest)
{
    //First I have to pack the msg
    int iRet;
    TDRDATA stHost;
    TDRDATA stNet;
    char buff[CS_PKG_MAX_PKG_LEN];
    size_t ibuff = sizeof(buff);

    stHost.iBuff = sizeof(*pstPkg);
    stHost.pszBuff = (char *) pstPkg;

    stNet.iBuff = ibuff;
    stNet.pszBuff = (char *) buff;

    iRet = tdr_hton(pstEnv->pstCSProtoMeta, &stNet, &stHost, 0);
    if (0 != iRet)
    {
        tlog_error(pstEnv->pstLogCat, 0, 0, "failed to pack the packet %s",
                tdr_error_string(iRet));
        return iRet;
    }

    tlog_debug_hex(pstEnv->pstLogCat,0,0,buff,stNet.iBuff);
    iRet = tbustestserver_send(pstEnv, buff, stNet.iBuff, iDest);

    if (0 != iRet)
    {
        tlog_error(pstEnv->pstLogCat, 0, 0, "failed to send the packet %s",
                tbus_error_string(iRet));
        return iRet;
    }

    return 0;
}

int tbustestserver_init(TAPPCTX *pstAppCtx, TBUSTESTSERVERENV *pstEnv)
{
    TBUSTESTCSPKG stPkg;
    int iRet;
    char szKey[TQQ_KEY_LEN];

    if (NULL == pstAppCtx->stConfData.pszBuff || 0
            == pstAppCtx->stConfData.iMeta || NULL == pstAppCtx->pszConfFile)
    {
        return -1;
    }
    pstEnv->pstConf = (TBUSTESTSERVERCONF *) pstAppCtx->stConfData.pszBuff;

    pstEnv->pstRunCumu
            = (TBUSTESTSERVERRUN_CUMULATE *) pstAppCtx->stRunDataCumu.pszBuff;
    if (NULL == pstEnv->pstRunCumu)
    {
        pstEnv->pstRunCumu = &gs_stCumu;
    }

    pstEnv->pstRunStat
            = (TBUSTESTSERVERRUN_STATUS *) pstAppCtx->stRunDataStatus.pszBuff;
    if (NULL == pstEnv->pstRunStat)
    {
        pstEnv->pstRunStat = &gs_stStat;
    }

    if (0 > tapp_get_category(TLOG_DEF_CATEGORY_TEXTROOT, &pstEnv->pstLogCat))
    {
        printf("tapp_get_category run fail\n");
        return -1;
    }

    if (0 != tbustestserver_init_tlog(pstAppCtx, pstEnv))
    {
        return -1;
    }

    if (0 != tbustestserver_init_tdr(pstAppCtx, pstEnv))
    {
        return -1;
    }

    if (0 != tbustestserver_init_tbus(pstAppCtx, pstEnv))
    {
        return -1;
    }

    memset(szKey, 0, sizeof(szKey));
    strncpy(szKey, "abc", sizeof(szKey));
    iRet = tsec_wrapper_init(&gs_stTsec, szKey, pstEnv->pstConf->iEncryptType);
    if (iRet)
    {
        printf("Error failed to init tsec wrapper\n");
        return -1;
    }

    memset(&stPkg, 0, sizeof(stPkg));
    tbustestserver_init_package(pstEnv, &stPkg);
    butstestserver_init_checksum(pstEnv, &stPkg);

    if (tbustestserver_check_package(pstEnv, &stPkg))
    {
        printf("Failed to init checksum\n");
        return -1;
    }

    printf("tbustestserver start\n");
    tlog_info(pstEnv->pstLogCat, 0, 0, "tbustestserver start");

    return 0;
}

#define check(x,value) do{if((x) != (value)){\
	tlog_error(pstEnv->pstLogCat,0,0,"Value not match ");\
	printf("Error value not match\n");\
	}}while(0)

int tbustestserver_check_package(TBUSTESTSERVERENV *pstEnv,
        TBUSTESTCSPKG *pstPkg)
{
    TDRDATA stHost;
    size_t i = 0;
    char sum = 0x00;
    char *t = (char *) &pstPkg->stBody;

    stHost.pszBuff = (char *) pstPkg;
    stHost.iBuff = sizeof(*pstPkg);

    //tdr_fprintf(pstEnv->pstCSProtoMeta,stderr,&stHost,0);
    check(pstPkg->stHeader.iCmdID,10);

    for (i = 0; i < sizeof(pstPkg->stBody); i++)
    {
        sum ^= *t;
        t++;
    }

    return sum;
}

int tbustestserver_init_package(TBUSTESTSERVERENV *pstEnv,
        TBUSTESTCSPKG *pstPkg)
{
    TDRDATA stHost;
    stHost.pszBuff = (char *) pstPkg;
    stHost.iBuff = sizeof(*pstPkg);
    tdr_dump_metalib(pstEnv->pstCSProtoMetalib, stdout);
    tdr_init(pstEnv->pstCSProtoMeta, &stHost, 0);
    tdr_fprintf(pstEnv->pstCSProtoMeta, stderr, &stHost, 0);
    return 0;
}
#define random() rand()
int randomString(char *buff, int buffSize)
{
    //All the string should be printable.
    int j = 0;
    int i;
    memset(buff, 0, buffSize);

    for (i = 0; i < (buffSize - 1); i++)
    {
        char t = random();
        if (isprint(t))
        {
            buff[j++] = t;
        }
    }
    return 0;
}

int tbustestserver_random_fill(TBUSTESTSERVERENV *pstEnv, TBUSTESTCSPKG *pstPkg)
{
    int i;
    memset(&pstPkg->stBody,0,sizeof(pstPkg->stBody));
    pstPkg->stBody.bUcharDate = random();
    pstPkg->stBody.bByteDate = random();
    pstPkg->stBody.chCharDate = random();
    *(uint64_t *) (&pstPkg->stBody.dDoubleDate) = (((uint64_t) random()) << 32)
            + (uint64_t) random();
    pstPkg->stBody.dwUintDate = random();
    *(uint64_t *) (&pstPkg->stBody.fFloatDate) = (((uint64_t) random()) << 32)
            + (uint64_t) random();
    pstPkg->stBody.llBigintDate = random();
    pstPkg->stBody.nShortDate = random();
    pstPkg->stBody.tTimeDate = random();
    pstPkg->stBody.ulIpDate = random();
    pstPkg->stBody.wUshortDate = random();
    pstPkg->stBody.iIntDate = random();
    pstPkg->stBody.tDateDate = random();
    pstPkg->stBody.tDatetimeDate = (((uint64_t) random()) << 32)
            + (uint64_t) random();
    pstPkg->stBody.ullBiguintDate = random();

    pstPkg->stBody.iBufferLen = random() % BUFF_MAX_LEN;
    for(i= 0;i < pstPkg->stBody.iBufferLen;i++ )
    {
        pstPkg->stBody.szVbuffer[i] = random();
    }

    randomString(pstPkg->stBody.szStringDate,
            sizeof(pstPkg->stBody.szStringDate));

    return 0;
}

int tbustestserver_proc(TAPPCTX *pstAppCtx, TBUSTESTSERVERENV *pstEnv)
{
    int iBusy = 0;
    int iRemoteAddr = 0;

    TBUSTESTCSPKG stPkg;
    int iRet;

    memset(&stPkg, 0, sizeof(stPkg));
    if (0 == tbustestserver_recv_msg(pstEnv, &stPkg, &iRemoteAddr))
    {
        iRet = tbustestserver_check_package(pstEnv, &stPkg);
        if (iRet)
        {
            printf("Error caculating checksum\n");
            tlog_error_dr(pstEnv->pstLogCat,0,0,pstEnv->pstCSProtoMeta,&stPkg,sizeof(stPkg),0);
            tlog_error(pstEnv->pstLogCat, 0, 0,
                    "Recv package and caculate checksum failed");
        }
        else
        {
            tbustestserver_random_fill(pstEnv, &stPkg);
            butstestserver_init_checksum(pstEnv, &stPkg);
            tbustestserver_send_msg(pstEnv, &stPkg, iRemoteAddr);
        }
    }

    if (iBusy)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

int tbustestserver_reload(TAPPCTX *pstAppCtx, TBUSTESTSERVERENV *pstEnv)
{
    TBUSTESTSERVERCONF *pstPreConf;

    pstPreConf = (TBUSTESTSERVERCONF *) pstAppCtx->stConfPrepareData.pszBuff;

    printf("tbustestserver reload\n");
    tlog_info(pstEnv->pstLogCat, 0, 0, "tbustestserver reload");

    return 0;
}

int tbustestserver_fini(TAPPCTX *pstAppCtx, TBUSTESTSERVERENV *pstEnv)
{

    printf("tbustestserver finish\n");
    tlog_info(pstEnv->pstLogCat, 0, 0, "tbustestserver finish");
    return 0;
}

int tbustestserver_tick(TAPPCTX *pstAppCtx, TBUSTESTSERVERENV *pstEnv)
{
    return 0;
}

int tbustestserver_stop(TAPPCTX *pstAppCtx, TBUSTESTSERVERENV *pstEnv)
{
    int iReady = 0;

    iReady = 1;
    if (iReady)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

int butstestserver_init_checksum(TBUSTESTSERVERENV *pstEnv,
        TBUSTESTCSPKG *pstPkg)
{
    size_t i = 0;
    char sum = 0x00;
    char *t = (char *) &pstPkg->stBody;
    for (i = 0; i < sizeof(pstPkg->stBody); i++)
    {
        sum ^= *t;
        t++;
    }
    pstPkg->stBody.chCharDate ^= sum;
    return 0;
}

int tbustestserver_send_seed(TAPPCTX *pstAppCtx, TBUSTESTSERVERENV *pstEnv,
        TBUSADDR iDest)
{
    int iRet;
    TBUSTESTCSPKG stPkg;
    // Init the package here.
    memset(&stPkg, 0, sizeof(stPkg));
    iRet = tbustestserver_init_package(pstEnv, &stPkg);
    if (0 != iRet)
    {
        char msg[1024];
        snprintf(msg, sizeof(msg), "tsendseed  failed %s", tdr_error_string(
                iRet));
        tappctrl_send_string(msg);
        return 0;
    }
    butstestserver_init_checksum(pstEnv, &stPkg);

    iRet = tbustestserver_check_package(pstEnv, &stPkg);
    if (iRet)
    {
        char msg[1024];
        snprintf(msg, sizeof(msg), "tsendseed  checksum failed %s",
                tdr_error_string(iRet));
        tappctrl_send_string(msg);
    }

    //Done init.
    return tbustestserver_send_msg(pstEnv, &stPkg, iDest);
}

// when tapp recv a text msg, it will call this interface to process it
int tbustestserver_proc_cmdline_req(TAPPCTX *pstAppCtx, void *pvArg,
        unsigned short argc, const char** argv)
{
    TBUSTESTSERVERENV *pstEnv;
    int iRet;

    pstEnv = (TBUSTESTSERVERENV *) pvArg;

    tlog_info(pstEnv->pstLogCat, 0, 0, "recv cmdline msg: argc<%d>", argc);

    if (strcmp(argv[0], "tsendseed") == 0)
    {
        if (argc == 2)
        {
            TBUSADDR iAddr = 0;
            iRet = tbus_addr_aton(argv[1], &iAddr);
            if (0 != iRet)
            {
                char msg[1024];
                snprintf(msg, sizeof(msg), "tsendseed  failed %s",
                        tbus_error_string(iRet));
                tappctrl_send_string(msg);
                return 0;
            }

            iRet = tbustestserver_send_seed(pstAppCtx, pstEnv, iAddr);
            if (0 == iRet)
            {
                tappctrl_send_string("tsendseed  succ");
            }
            else
            {
                char msg[1024];
                snprintf(msg, sizeof(msg), "tsendseed  failed %s",
                        tbus_error_string(iRet));
                tappctrl_send_string(msg);
                return 0;
            }
        }
        else
        {
            tappctrl_send_string("usage:\n\t\tsendseed  a.b.c.d");
        }
    }

    return 0;
}

const char* tbustestserver_console_help(void)
{
    static const char pszHelp[] = "tsendseed a.b.c.d\n";
    return pszHelp;
}

int service_init(IN LPTAPPCTX a_pstCtx, void *a_pstEnv, IN int argc,
        IN char* argv[])
{
    int iRet;
    void* pvArg = &gs_stEnv;

    memset(&gs_stAppCtx, 0, sizeof(gs_stAppCtx));
    memset(&gs_stEnv, 0, sizeof(gs_stEnv));

    gs_stAppCtx.argc = argc;
    gs_stAppCtx.argv = argv;

    gs_stAppCtx.pfnInit = (PFNTAPPFUNC) tbustestserver_init;

    gs_stAppCtx.iLib = (intptr_t) g_szMetalib_tbustestserver;
    gs_stAppCtx.stConfData.pszMetaName = "tbustestserverconf";
    gs_stAppCtx.uiVersion = TAPP_MAKE_VERSION(MAJOR, MINOR, REV, BUILD);

    gs_stAppCtx.pfnFini = (PFNTAPPFUNC) tbustestserver_fini;
    gs_stAppCtx.pfnProc = (PFNTAPPFUNC) tbustestserver_proc;
    gs_stAppCtx.pfnTick = (PFNTAPPFUNC) tbustestserver_tick;
    gs_stAppCtx.pfnReload = (PFNTAPPFUNC) tbustestserver_reload;
    gs_stAppCtx.pfnStop = (PFNTAPPFUNC) tbustestserver_stop;

    gs_stAppCtx.pfnProcCmdLine = tbustestserver_proc_cmdline_req;
    gs_stAppCtx.pfnGetCtrlUsage = tbustestserver_console_help;

    //gs_stAppCtx.pfnPreprocCmdLine = tbustestserver_proc_stdin;

    iRet = tapp_def_init(&gs_stAppCtx, pvArg);
    if (iRet < 0)
    {
        printf("Error: app Initialization failed.\n");
        return iRet;
    }

    //iRet	=	tapp_def_mainloop(&gs_stAppCtx, pvArg);

    //tapp_def_fini(&gs_stAppCtx, pvArg);

    return 0;
}

int main(int argc, char* argv[])
{
    int iRet = 0;

    tapp_register_service_init((PFNTAPP_SERVICE_INIT) service_init);
    iRet = tapp_run_service(&gs_stAppCtx, &gs_stEnv, argc, argv);
    return iRet;
}

