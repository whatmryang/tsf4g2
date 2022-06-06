#include "tbusd_controller.h"
#include "tbusd_ctrl_kernel.h"
#include "tbusd_log.h"

//////////////////////////////////////////////////////////////////////////
//static int tbusd_proc_get_envres( LPTBUSCTRLPKG a_pstPkg);

//////////////////////////////////////////////////////////////////////////

//tbusd 控制端初始化
int tbusd_contorller_init(struct tagTAPPCTX *pstCtx, void* pvArg)
{
    int iRet = 0;

    TOS_UNUSED_ARG(pvArg);
    if (NULL == pstCtx)
    {
        printf("tbusd_contorller_init, the tappctx is null");
        return -1;
    }

    //get log
    iRet = tapp_get_category (NULL, &g_ptRelayLog) ;
    if (0 != iRet)
    {
        printf("Warning!!!failed to get default logcategory iRet:%d\n", iRet);
        g_ptRelayLog = NULL;
    }

    iRet = tbusdctrl_init();
    if (0 != iRet)
    {
        printf("tbusdctrl_init failed, iRet:%d\n", iRet);
        return -2;
    }

    return 0;
}

//处理从tbusd返回的控制响应
int tbusd_proc_ctrlres(const char* pszMetaName, const char* pBuf, int iLen)
{
    TBUSCTRLPKG stResPkg;
    int iRet = 0;

    if ((NULL == pBuf) ||(0 >= iLen) ||(NULL == pszMetaName))
    {
        tlog_error(g_ptRelayLog, 0,0, "invalid param: ctrlres msg buf or MetaName is null(%p) or invalid len %d", pBuf, iLen);
        return -1;
    }

    iRet = tbusdctrl_unpack_pkg(&stResPkg, pBuf, iLen);
    if (0 != iRet)
    {
        printf("failed to unpack pkg\r\n");
        return iRet;
    }

    tbusdctrl_print_pkg(&stResPkg, stdout);
    /*
    switch(stResPkg.stHead.dwCmd)
    {
    case TBUSCTRLCMD_GET_ENVINFO_RES:
        iRet = tbusd_proc_get_envres(&stResPkg);
        break;
    default:
        tlog_error(g_ptRelayLog, 0,0, "unsupport protocol cmd %d", stResPkg.stHead.dwCmd);
        break;
    }*/

    return iRet;
}

//static int tbusd_proc_get_envres( LPTBUSCTRLPKG a_pstPkg)
//{
//    assert(NULL != a_pstPkg);
//
//    printf("\rtbusd summarization :\n");
//    tbusdctrl_print_pkg(a_pstPkg, stdout);
//
//    return 0;
//}
