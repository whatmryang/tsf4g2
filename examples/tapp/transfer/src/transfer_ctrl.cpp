/*
 * Copyright (c) 2010, 腾讯科技有限公司互动娱乐研发部
 * All rights reserved
 *
 * File name:       transfer_ctrl.cpp
 * Description:     definitions for functions about tapp-control
 * Version:         1.0
 *
 * Author:          flyma.IERD.IED.tencent
 * Creation time:   2010-06-03 09:13
 * Last modified:   2010-06-03 18:38
 *
 * History:
 * <date>        <author>      <version>   <description>
 * 2010-06-03     flyma         1.0         creation
 *
 */

#include "transfer_ctrl.h"
#include "tloghelp/tlogload.h"

void transfer_set_controller(TAPPCTX *a_pstTappCtx)
{
    assert(NULL != a_pstTappCtx);

    // 设置作为server启动时提供控制服务所需的接口函数
    a_pstTappCtx->pfnProcCmdLine = transfer_proc_cmd_req;
    a_pstTappCtx->pfnProcCtrlReq = transfer_proc_bin_req;
    a_pstTappCtx->pfnGetCtrlUsage = transfer_console_help;

    // 设置作为控制终端模式启动时需要使用的接口
    a_pstTappCtx->pfnControllerInit = transfer_controller_init;
    a_pstTappCtx->pfnProcCtrlRes = transfer_proc_bin_res;

    // pfnPreprocCmdLine return 0, 表示对输入不感兴趣，把这个输入交给tapp处理
    // pfnPreprocCmdLine return 1, 表示对输入感兴趣，由自己处理这个输入
    a_pstTappCtx->pfnPreprocCmdLine = transfer_proc_stdin;
    a_pstTappCtx->pfnControllerFini = transfer_controller_fini;
}

// when tapp recv a binary control msg, it will call this interface to process it
int transfer_proc_bin_req(TAPPCTX *a_pstTappCtx, void *a_pstEnv,
                          const char* a_pszMetaName, const char* a_pBuf, int a_iLen)
{
    assert(NULL != a_pstTappCtx);
    assert(NULL != a_pstEnv);
    assert(NULL != a_pszMetaName);
    assert(NULL != a_pBuf);

    int iRet = 0;

    // TODO
    // add code here to process binary request from tapp-control-consoler

    /* example code: just send back recved msg */
    /* example code begin */
    iRet = tappctrl_send_msg(a_pszMetaName, a_pBuf, a_iLen);
    if (0 > iRet)
    {
        printf("failed to backward recved msg\n");
    }
    /* example code end */

    return iRet;
}

// when tapp recv a "help" msg, it will call this interface to process it
const char* transfer_console_help(void)
{
    static const char* pszHelp;

    // TODO
    // construct help-info for help-request from tapp-control-consoler

    /* example code: just construct a simple help-info */
    /* example code begin */
    pszHelp = "This is an demo.\n"
        "All recved binary msgs will be send back.\n"
        "No cmdline msgs are supported.\n";
    /* example code end */

    return pszHelp;
}

// when tapp recv a text msg, it will call this interface to process it
int transfer_proc_cmd_req(TAPPCTX *a_pstTappCtx, void *a_pvArg,
                          unsigned short argc, const char** argv)
{
    assert(NULL != a_pstTappCtx);
    assert(NULL != a_pvArg);
    assert(0 < argc);
    assert(NULL != argv);

    int iRet = 0;
    LPTRANSFERENV a_pstEnv;

    a_pstEnv = (LPTRANSFERENV)a_pvArg;

    // TODO
    // add code here to process cmdline request from tapp-control-consoler

    /* example code: log, response and print into stdout */
    /* example code begin */
    tlog_info(a_pstEnv->pstLogCat, 0, 0, "recv cmdline msg: argc<%d>", argc);
    tappctrl_send_string("Error: unsupported command");
    for (int i = 0; i < argc; i++)
        printf("%s ", argv[i]);
    printf("\n");
    /* example code end */

    return iRet;
}

int transfer_controller_init(TAPPCTX *a_pstTappCtx, void *a_pvArg)
{
    assert(NULL != a_pstTappCtx);
    assert(NULL != a_pvArg);

    int iRet = 0;

    LPTRANSFERENV a_pstEnv = (LPTRANSFERENV)a_pvArg;
    if (0 > tapp_get_category(TLOG_DEF_CATEGORY_TEXTROOT,
                              &a_pstEnv->pstLogCat))
    {
        printf("tapp_get_category run fail\n");
        return -1;
    }

    // TODO
    // add code here to do init information that will be used for tapp-control

    /* example code: log a trivial msg */
    /* example code begin */
    tlog_error(a_pstEnv->pstLogCat, 0, 0, "transfer controller init do nothing");
    /* example code end */

    return iRet;
}

int transfer_proc_bin_res(const char* a_pszMetaName, const char* a_pBuf, int a_iLen)
{
    assert(NULL != a_pszMetaName);
    assert(NULL != a_pBuf);

    int iRet = 0;

    // TODO
    // add code here to process response msg from server

    /* example code: printf and log */
    /* example code begin */
    printf("recv response for meta: %s\n", a_pszMetaName);
    tlog_error(g_stEnv.pstLogCat, 0, 0, "recv response for meta: %s, a_iLen<%d>",
               a_pszMetaName, a_iLen);
    /* example code end */

    return iRet;
}

int transfer_proc_stdin(unsigned short argc, const char** argv)
{
    assert(0 < argc);
    assert(NULL != argv);

    int iRet = 0;

    // TODO
    // add code here to process input from stdin

    /* example code: log */
    /* example code begin */
    tlog_error(g_stEnv.pstLogCat, 0, 0, "recv command: %s,"
               " but not interested in it", argv[0]);
    /* example code end */


    // 注意，TAPP对些函数的返回值有如下约定：
    // return 0, 表示对输入不感兴趣，把这个输入交给tapp处理
    // return 1, 表示对输入感兴趣，由自己处理这个输入

    return iRet;
}

int transfer_controller_fini(TAPPCTX *a_pstTappCtx, void *a_pvArg)
{
    assert(NULL != a_pstTappCtx);
    assert(NULL != a_pvArg);

    int iRet = 0;

    // TODO
    // add code here to recycle any resources allocated in transfer_controller_init

    /* example code: nothing to recycle */
    /* example code begin */
    LPTRANSFERENV a_pstEnv = (LPTRANSFERENV)a_pvArg;
    tlog_error(a_pstEnv->pstLogCat, 0, 0, "transfer controller fini do nothing");
    /* example code end */

    return iRet;
}
