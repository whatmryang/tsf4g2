/*
 * Copyright (c) 2010, 腾讯科技有限公司互动娱乐研发部
 * All rights reserved
 *
 * File name:       transfer_ctrl.h
 * Description:     declaration for funcions about tapp-control
 * Version:         1.0
 *
 * Author:          flyma.IERD.IED.tencent
 * Creation time:   2010-06-03 08:59
 * Last modified:   2010-06-03 18:38
 *
 * History:
 * <date>        <author>      <version>   <description>
 * 2010-06-03     flyma         1.0         creation
 *
 */

#ifndef _TRANSFER_CTRL_H_
#define _TRANSFER_CTRL_H_

#include "tapp/tapp.h"
#include "transfer.h"

void transfer_set_controller(TAPPCTX *a_pstTappCtx);

/* 作为server启动时提供控制服务所需的接口函数 */
const char* transfer_console_help(void);

int transfer_proc_cmd_req(TAPPCTX *a_pstTappCtx, void *a_pvArg,
                          unsigned short argc, const char** argv);

int transfer_proc_bin_req(TAPPCTX *a_pstTappCtx, void *a_pstEnv,
                          const char* a_pszMetaName, const char* a_pBuf, int a_iLen);


/* 作为控制终端模式启动时需要使用的接口 */
int transfer_controller_init(TAPPCTX *a_pstTappCtx, void *a_pvArg);

int transfer_proc_bin_res(const char* a_pszMetaName, const char* a_pBuf, int a_iLen);

int transfer_proc_stdin(unsigned short argc, const char** argv);

int transfer_controller_fini(TAPPCTX *a_pstTappCtx, void *a_pvArg);

#endif
