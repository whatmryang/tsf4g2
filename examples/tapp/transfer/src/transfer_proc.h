/*
 * Copyright (c) 2010, 腾讯科技有限公司互动娱乐研发部
 * All rights reserved
 *
 * File name:       transfer_proc.h
 * Description:     description for functions about TAPPCTX->pfnProc
 * Version:         1.0
 *
 * Author:          flyma.IERD.IED.tencent
 * Creation time:   2010-06-09 10:09
 * Last modified:   2010-06-09 10:29
 *
 * History:
 * <date>        <author>      <version>   <description>
 * 2010-06-09     flyma         1.0         creation
 *
 */

#include "transfer.h"
#include "tlog/tlog.h"
#include "transfer_session_base.h"

void transfer_proc_connect_msg(TAPPCTX* a_pstTappCtx, TRANSFERENV* a_pstEnv, TFRAMEHEAD* a_pstFrameHead);
void transfer_proc_request_pkg(TAPPCTX* a_pstTappCtx, TRANSFERENV* a_pstEnv, FOREPKG* a_pstReq);

void transfer_dispatch_response_pkg(TAPPCTX* a_pstTappCtx, TRANSFERENV* a_pstEnv, BACKPKG* a_pstRes);
void transfer_proc_dirtycheck_req(TAPPCTX* a_pstTappCtx, TRANSFERENV* a_pstEnv, FOREPKG *a_pstReq);
