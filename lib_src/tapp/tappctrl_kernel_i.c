/***********************************************************
 * FileName: tappctrl_kernel_i.c
 * Author: jackyai
 * Version: 1.0
 * Date: 2010-01-26
 * Function List:
 *      tappctrl_set_ctrlfile_info
 *      tappctrl_init
 *      tappctrl_fini
 *      tappctrl_recv_pkg
 *      tappctrl_send_pkg
 *      tappctrl_fill_pkghead
 * Description:
 *      this source file contains key code and data structure of tapp-ctrl
 * History:
 *      <time>        <author>      <version>   <desc>
 *      2010-01-26     jackyai       1.0         创建
 * ***********************************************************/

#include "tappctrl_kernel_i.h"
#include "tlog/tlog.h"
#include "tdr/tdr.h"
#include "tbus/tbus.h"
#include "tapp_i.h"

// Internal types
struct  tagTappCtrlEnv
{
    int iVersion;

    int iIsInited;		/*记录是否已经初始化。0表示没有初始化，非零表示已经初始化好*/
    LPTDRMETA pstMsgMeta;	/*控制协议消息的描述句柄*/
    LPTBUSCHANNEL pstChnnl;	/*控制消息通道*/
    char szNetBuff[TAPPCTRL_MAX_MSG_LEN];
};

typedef struct tagTappCtrlEnv TAPPCTRLENV;
typedef struct tagTappCtrlEnv *LPTAPPCTRLENV;

// Global parameters
LPTLOGCATEGORYINST g_pstTappCtrlLogCat;
LPTDRMETALIB g_pstTappProtoMetaLib;
static TAPPCTRLENV gs_stCtrlEnv = {0, 0, NULL, NULL, {0}};


#define TAPPCTRL_SERVER_ADDRESS	0xFFFFFF01		/*服务端地址*/
#define TAPPCTRL_CONTROLOR_ADDRESS	0xFFFFFF02	/*控制端地址*/
#define TAPP_CTRL_PROTOCOL_META     "CommandInjectPkg"	/*控制消息*/

#define TAPPCTRL_SET_ENV_INITED	(gs_stCtrlEnv.iIsInited = 1)
#define TAPPCTRL_IS_ENV_INITED	(gs_stCtrlEnv.iIsInited == 1)

extern TAPP_GS gs_stTapp;
extern TAPPCTX_INTERNAL gs_stCtxInternal;




int tapp_controller_auth(const char* a_pszChannelInfo);
int tbus_gen_shmkey(OUT key_t *ptShmkey, IN const char *a_pszShmkey, IN int a_iBussId);


int tapp_ctrl_printf(const char *fmt, ...)
{
    char szString[4096];
    va_list ap;

    memset(szString,0,sizeof(szString));

    va_start(ap, fmt);
    vsnprintf(szString,sizeof(szString),fmt,ap);
    tappctrl_send_string(szString);
    va_end(ap);
    return 0;
}

////////////////////////////////////////////////////////////////////////////
int tappctrl_set_ctrlfile_info(OUT const char **a_ppszChannelInfo, IN const char *a_pszTappName,
                               IN const char *a_pszTappID, IN int a_iBusinessID)
{
    char szBusiness[128];
    int iWriteLen = -1;
    const char *pszTappName = NULL;
#if defined (_WIN32) || defined (_WIN64)
    char szPath[MAX_PATH];
    char *pchDot = NULL;
#endif

    if (NULL == a_pszTappName)
    {
        return -1;
    }

    szBusiness[0] = '\0';
    if (0 != a_iBusinessID)
    {
        snprintf(szBusiness, sizeof(szBusiness)-1, "_%d", a_iBusinessID);
    }

    pszTappName = a_pszTappName;

#if defined (_WIN32) || defined (_WIN64)
    {
        pchDot = strrchr(pszTappName, '.');
        if ((NULL != pchDot) && (0 == strcmp(pchDot, ".exe")))
        {
            if (sizeof(szPath) > (pchDot - pszTappName))
            {
                strncpy(szPath, pszTappName, pchDot - pszTappName);
                szPath[pchDot-pszTappName] = '\0';
            } else
            {
                strncpy(szPath, pszTappName, sizeof(szPath) - 1);
                szPath[sizeof(szPath)-1] = '\0';
            }
            pszTappName = szPath;
        }
    }
#endif

    if(NULL != a_pszTappID)
    {
        iWriteLen = snprintf(gs_stTapp.szCIAddr, sizeof(gs_stTapp.szCIAddr) - 1, "%s%s_%s%s.sock", TOS_TMP_DIR,
                             pszTappName, a_pszTappID, szBusiness);
    }else
    {
        iWriteLen =snprintf(gs_stTapp.szCIAddr, sizeof(gs_stTapp.szCIAddr) - 1, "%s%s%s.sock", TOS_TMP_DIR, pszTappName, szBusiness);
    }

    if ((iWriteLen < 0) || ((unsigned)iWriteLen >= sizeof(gs_stTapp.szCIAddr)-1))
    {
        return -1;
    }

    if (a_ppszChannelInfo)
        *a_ppszChannelInfo = gs_stTapp.szCIAddr;

    return 0;
}

int tappctrl_init_i(IN const char *a_pszChannelInfo, IN int a_iIsCtrlor, IN void *a_pstLogcat, IN void* a_iMetaLib)
{
    int iRet = 0;
    key_t iShmkey = 0;
    TBUSADDR iLocalAddr ;
    TBUSADDR iPeerAddr;

    assert(NULL != a_pszChannelInfo);
    assert(NULL != a_pstLogcat);
    assert(NULL != a_iMetaLib);

    if (0 != gs_stCtrlEnv.iIsInited)
    {
        return 0;
    }

    //获取控制协议的描述
    memset(&gs_stCtrlEnv, 0, sizeof(gs_stCtrlEnv));
    g_pstTappCtrlLogCat= (LPTLOGCATEGORYINST)a_pstLogcat;
    g_pstTappProtoMetaLib = (LPTDRMETALIB)a_iMetaLib;
    gs_stCtrlEnv.pstMsgMeta = tdr_get_meta_by_name((LPTDRMETALIB)a_iMetaLib, TAPP_CTRL_PROTOCOL_META);
    if (NULL == gs_stCtrlEnv.pstMsgMeta)
    {
        tlog_error(g_pstTappCtrlLogCat, 0, 0, "failed to get meta by name \'%s\'", TAPP_CTRL_PROTOCOL_META);
        return -1;
    }

    //打开控制通道
    if (0 == a_iIsCtrlor)
    {
        iLocalAddr = TAPPCTRL_SERVER_ADDRESS;
        iPeerAddr = TAPPCTRL_CONTROLOR_ADDRESS;
    }else
    {
        iPeerAddr = TAPPCTRL_SERVER_ADDRESS;
        iLocalAddr = TAPPCTRL_CONTROLOR_ADDRESS;
    }

    iRet = tbus_channel_mode_init(0);
    if (0 > iRet)
    {
        tlog_error(g_pstTappCtrlLogCat, 0, 0, "failed to init tbus-channel-mode for tapp-control, for %s",
                   tbus_error_string(iRet));
        return -1;
    }
    if (0 == iRet)
    {
        iRet = tbus_gen_shmkey(&iShmkey, a_pszChannelInfo, 0);
        if (0 > iRet)
        {
            tlog_error(g_pstTappCtrlLogCat, 0, 0, "unable to generate shmkey from filepath: %s, for %s",
                       a_pszChannelInfo, tbus_error_string(iRet));
        }
    }
    if (0 == iRet)
    {
        iRet = tbus_channel_open(&gs_stCtrlEnv.pstChnnl,
                                 iLocalAddr, iPeerAddr, iShmkey, TAPPCTRL_CHANNEL_QUEUE_SIZE);
        if (0 > iRet)
        {
            tlog_error(g_pstTappCtrlLogCat, 0, 0, "failed to open channel for tapp-control<iShmkey: %x>, for %s",
                       iShmkey, tbus_error_string(iRet));
        }
    }


    if (0 == iRet)
    {
        tbus_channel_clean(gs_stCtrlEnv.pstChnnl);
        TAPPCTRL_SET_ENV_INITED;
        tlog_info(g_pstTappCtrlLogCat, 0, 0, "tappctrl addr: %s", a_pszChannelInfo);
    }else
    {
        tbus_channel_mode_fini();
    }

    if (0 == iRet && a_iIsCtrlor)
    {
        iRet = tapp_controller_auth(a_pszChannelInfo);

        if (iRet)
        {
            printf(" Failed\n");
        }else
        {
            printf(" Done\n\n");
        }
    }

    return iRet;
}

void tappctrl_fini()
{
    if (TAPPCTRL_IS_ENV_INITED)
    {
        tbus_channel_close(&gs_stCtrlEnv.pstChnnl);
        tbus_channel_mode_fini();
        gs_stCtrlEnv.iIsInited = 0;
    }
}

int tappctrl_recv_pkg(INOUT  LPCOMMANDINJECTPKG a_pstPkg)
{
    LPCOMMANDINJECTPKG pstNetMsg = NULL;
    TDRDATA stNetData;
    TDRDATA stHostData;
    int iRet = 0;
    size_t iMsgLen = 0;
    short nCheckSum = 0;

    if ((NULL == a_pstPkg) || !TAPPCTRL_IS_ENV_INITED)
    {
        return -1;
    }

    assert(NULL != gs_stCtrlEnv.pstChnnl);
    iRet = tbus_channel_peek_msg(gs_stCtrlEnv.pstChnnl, (const char**)(void *)&pstNetMsg, &iMsgLen, 0);
    if ( 0 != iRet)
    {
        return -2;
    }
    //dbgprintf("Receive one package from tbus len=%d\n",iMsgLen);
    //校验
    TAPP_CALC_CHECKSUM(nCheckSum, &pstNetMsg->stHead, sizeof(CIHEAD));
    if (0 != nCheckSum)
    {
        //dbgprintf("Failed to caculate checksum of the package\n");
        tlog_error(g_pstTappCtrlLogCat, 0, 0, "recv a ctrl msg, but failed to check it");
        iRet = 0;
    }else
    {
        gs_stCtrlEnv.iVersion = a_pstPkg->stHead.nVersion;
    }

    if (0 == iRet)
    {
        //解包控制消息
        assert(NULL != gs_stCtrlEnv.pstMsgMeta);
        stNetData.pszBuff = (char *)pstNetMsg;
        stNetData.iBuff = iMsgLen;
        stHostData.pszBuff = (char*)a_pstPkg;
        stHostData.iBuff = sizeof(*a_pstPkg);


        iRet = tdr_ntoh(gs_stCtrlEnv.pstMsgMeta, &stHostData, &stNetData, 0);
        if (TDR_ERR_IS_ERROR(iRet))
        {
            tlog_error(g_pstTappCtrlLogCat, 0, 0, "failed to unpack a ctrl msg<length %"PRIdPTR">,"
                       " but tdr_ntoh failed for %s\n", iMsgLen, tdr_error_string(iRet));
            tlog_error_dr(g_pstTappCtrlLogCat, 0,0, gs_stCtrlEnv.pstMsgMeta,
                          stNetData.pszBuff, iMsgLen,0);
        }else
        {
            tlog_info(g_pstTappCtrlLogCat, 0, 0, "recv ctrl msg<cmdID: %d, total msg length: %"PRIdPTR">",
                      a_pstPkg->stHead.nCmdID, iMsgLen);
            tlog_trace_dr(g_pstTappCtrlLogCat, 0,0, gs_stCtrlEnv.pstMsgMeta,
                          stHostData.pszBuff, stHostData.iBuff,0);
        }/*if (TDR_ERR_IS_ERROR(iRet))*/
    }

    tbus_channel_delete_msg(gs_stCtrlEnv.pstChnnl);

    return iRet;
}

/** 打包控制消息，并发送到控制通道中。本接口是非线程安全的。
  @param[in] a_pstPkg 带发送的控制脚本保存解包后的控制消息
  @return 0 成功 非0：失败
  */
int tappctrl_send_pkg(INOUT  LPCOMMANDINJECTPKG a_pstPkg)
{
    int iRet = 0;
    LPCOMMANDINJECTPKG pstNetPkg;
    TDRDATA stHostData;
    TDRDATA stNetData;
    short nCheckSum;

    if ((NULL == a_pstPkg)||!TAPPCTRL_IS_ENV_INITED)
    {
        //dbgprintf("Null package or Not inited");
        return -1;
    }


    assert(NULL != gs_stCtrlEnv.pstMsgMeta);
    a_pstPkg->stHead.nCheckSum = 0;
    stHostData.iBuff = sizeof(*a_pstPkg);
    stHostData.pszBuff = (char*)a_pstPkg;
    stNetData.iBuff = sizeof(gs_stCtrlEnv.szNetBuff);
    stNetData.pszBuff = (char*)&gs_stCtrlEnv.szNetBuff[0];
    iRet = tdr_hton(gs_stCtrlEnv.pstMsgMeta, &stNetData, &stHostData, 0);

#ifdef _DEBUG
    //tdr_fprintf(gs_stCtrlEnv.pstMsgMeta, stdout,&stHostData,0);
#endif

    if (TDR_ERR_IS_ERROR(iRet))
    {
        //dbgprintf("Failed to pack msg:%s",tdr_error_string(iRet));
        tlog_error(g_pstTappCtrlLogCat, 0, 0, "failed to pack a ctrl msg:%s"
                   " but tdr_hton failed\n", tdr_error_string(iRet));
        tlog_debug_dr(g_pstTappCtrlLogCat, 0,0, gs_stCtrlEnv.pstMsgMeta,
                      stHostData.pszBuff, stHostData.iBuff,0);
    }else
    {
        pstNetPkg = (LPCOMMANDINJECTPKG)stNetData.pszBuff;
        TAPP_CALC_CHECKSUM(nCheckSum, stNetData.pszBuff, sizeof(CIHEAD));
        pstNetPkg->stHead.nCheckSum = nCheckSum;
        tlog_trace_dr(g_pstTappCtrlLogCat, 0,0, gs_stCtrlEnv.pstMsgMeta,
                      stHostData.pszBuff, stHostData.iBuff,0);
    }

    //发送数据
    if (0 == iRet)
    {
        assert(NULL != gs_stCtrlEnv.pstChnnl);
        iRet = tbus_channel_send(gs_stCtrlEnv.pstChnnl, stNetData.pszBuff, stNetData.iBuff, 0);
        if (0 > iRet)
        {
            //dbgprintf("Failed to send packet size %d\n",stNetData.iBuff);
            tlog_error(g_pstTappCtrlLogCat, 0, 0, "tbus_channel_send failed, for %s", tbus_error_string(iRet));
        }
        //dbgprintf("Send packet succ,package len=%d\n",stNetData.iBuff);
    }

    return iRet;
}

int tappctrl_fill_pkghead(OUT	LPCIHEAD a_pstHead, int a_iCmd, int a_iVersion)
{
    if (NULL == a_pstHead)
    {
        return -1;
    }

    memset(a_pstHead,0, sizeof(*a_pstHead));
    a_pstHead->nMagic = 0 | ('C' << 8);
    a_pstHead->nMagic |= 'I';
    a_pstHead->nCmdID = (short)a_iCmd;
    a_pstHead->nVersion = (int16_t)a_iVersion;
    return 0;
}

TSF4G_API int tappctrl_send_string(const char* a_pszString)
{
    int iRet = 0;
    COMMANDINJECTPKG stResPkg;

    assert(NULL != a_pszString);
    tappctrl_fill_pkghead(&stResPkg.stHead, TAPP_STRING, gs_stCtrlEnv.iVersion);


    STRNCPY(stResPkg.stBody.szErrorString, a_pszString, sizeof(stResPkg.stBody.szErrorString));


    iRet = tappctrl_send_pkg(&stResPkg);

    return iRet;
}

TSF4G_API int tappctrl_send_msg(const char* a_pszMetaName, const char* a_pBuff, int a_iLen)
{
    int iRet = 0;
    COMMANDINJECTPKG stResPkg;

    assert(NULL != a_pszMetaName);
    assert(NULL != a_pBuff);
    assert(0 < a_iLen);
    tappctrl_fill_pkghead(&stResPkg.stHead, TAPP_BINARY_RES, gs_stCtrlEnv.iVersion);


    STRNCPY(stResPkg.stBody.stBinDataRes.szMetaName, a_pszMetaName,
            sizeof(stResPkg.stBody.stBinDataRes.szMetaName));
    stResPkg.stBody.stBinDataRes.wDataLen = (unsigned short)a_iLen;
    memcpy(stResPkg.stBody.stBinDataRes.szData, a_pBuff, a_iLen);


    iRet = tappctrl_send_pkg(&stResPkg);

    return iRet;
}

int tapp_controller_get_version()
{
    return gs_stCtrlEnv.iVersion;
}

int tapp_controller_send_req_i(short a_nCmdID, int a_iVersion)
{
    int iRet = 0;
    COMMANDINJECTPKG stReqPkg;

    tappctrl_fill_pkghead(&stReqPkg.stHead, a_nCmdID, a_iVersion);
    stReqPkg.stHead.wBodyLen = 0;

    iRet = tappctrl_send_pkg(&stReqPkg);

    return iRet;
}
