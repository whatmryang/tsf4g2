#include "tbusd_ctrl_kernel.h"
#include "tdr/tdr.h"
#include "tbusd_log.h"

static LPTDRMETA gs_pstTbusdCtrlPkgMeta = NULL;

int tbusdctrl_init(void);
void tbusdctrl_fini(void);

extern unsigned char g_szMetalib_tbusdctrl[];
////////////////////////////////////////////
int tbusdctrl_init(void)
{
    LPTDRMETALIB pstLib = (LPTDRMETALIB)g_szMetalib_tbusdctrl;
    gs_pstTbusdCtrlPkgMeta = tdr_get_meta_by_name(pstLib, TBUSD_CTRL_PKG_META_NAME);
    if (NULL == gs_pstTbusdCtrlPkgMeta)
    {
        tlog_error(g_ptRelayLog, 0,0 , "failed to get meta by name:%s", TBUSD_CTRL_PKG_META_NAME);
    }

    return 0;
}

/*处理环境释放相关资源资源
*/
void tbusdctrl_fini(void)
{

}

//初始化控制消息头部
void tbusdctrl_fill_pkghead(OUT TBUSCTRLHEAD *a_pstHead, unsigned int a_dwCmd, unsigned int a_dwVer)
{
    assert(NULL != a_pstHead);

    a_pstHead->dwCmd = a_dwCmd;
    a_pstHead->dwVersion = a_dwVer;
    a_pstHead->dwMagic = TBUSD_CTRL_PROTO_MAGIC;
}

int tbusdctrl_send_pkg(IN  LPTBUSCTRLPKG a_pstPkg)
{
    char szNet[TBUSD_CTRL_MAX_PKG_LEN];
    int iRet = 0;
    TDRDATA stHost;
    TDRDATA stNet;

    if ((NULL == gs_pstTbusdCtrlPkgMeta) || (NULL == a_pstPkg))
    {
        return -1;
    }

    stHost.iBuff = sizeof(TBUSCTRLPKG);
    stHost.pszBuff = (char *)a_pstPkg;
    stNet.iBuff = sizeof(szNet);
    stNet.pszBuff = szNet;
    iRet = tdr_hton(gs_pstTbusdCtrlPkgMeta, &stNet, &stHost, a_pstPkg->stHead.dwVersion);
    if (0 != iRet)
    {
        tlog_error(g_ptRelayLog, 0,0 , "failed to pack ctrol msg:%s", tdr_error_string(iRet));
        tlog_debug_dr(g_ptRelayLog,0,0,gs_pstTbusdCtrlPkgMeta,stHost.pszBuff, stHost.iBuff,0);
        return iRet;
    }
    tlog_trace_dr(g_ptRelayLog,0,0,gs_pstTbusdCtrlPkgMeta,stHost.pszBuff, stHost.iBuff,0);

    iRet = tappctrl_send_msg(TBUSD_CTRL_PKG_META_NAME, stNet.pszBuff, stNet.iBuff);
    if (0 != iRet)
    {
        tlog_error(g_ptRelayLog,0,0, "failed to send crtl msg(len:%d),iRet:%d", (int)stNet.iBuff, iRet);
    }

    return iRet;
}

int tbusdctrl_unpack_pkg(OUT  LPTBUSCTRLPKG a_pstPkg, const char *a_pszNet, int a_iLen)
{
    int iRet = 0;
    TDRDATA stHost;
    TDRDATA stNet;

    if ((NULL == a_pstPkg) ||(NULL == a_pszNet)||(NULL == gs_pstTbusdCtrlPkgMeta)||(0 >= a_iLen))
    {
        return -1;
    }

    stHost.iBuff = sizeof(TBUSCTRLPKG);
    stHost.pszBuff = (char *)a_pstPkg;
    stNet.iBuff = a_iLen;
    stNet.pszBuff = (char *)a_pszNet;
    iRet = tdr_ntoh(gs_pstTbusdCtrlPkgMeta, &stHost, &stNet, 0);
    if (0 != iRet)
    {
        tlog_error(g_ptRelayLog, 0,0 , "failed to pack ctrol msg:%s", tdr_error_string(iRet));
        tlog_debug_dr(g_ptRelayLog,0,0,gs_pstTbusdCtrlPkgMeta,stHost.pszBuff, stHost.iBuff,0);
        return iRet;
    }
    tlog_trace_dr(g_ptRelayLog,0,0,gs_pstTbusdCtrlPkgMeta,stHost.pszBuff, stHost.iBuff,0);

    return iRet;
}

/** 打印控制消息
@param[in] a_pstPkg 保存解包后的消息
@return 0 成功 非0：失败
*/
int tbusdctrl_print_pkg(OUT  LPTBUSCTRLPKG a_pstPkg, FILE *a_fpOut)
{
    TDRDATA stHost;
    int iRet = 0;

    if ((NULL == a_pstPkg) ||(NULL == a_fpOut) ||(NULL == gs_pstTbusdCtrlPkgMeta))
    {
        return -1;
    }

    stHost.iBuff = sizeof(TBUSCTRLPKG);
    stHost.pszBuff = (char *)a_pstPkg;
    tdr_fprintf(gs_pstTbusdCtrlPkgMeta, a_fpOut, &stHost, 0);

    return iRet;
}

