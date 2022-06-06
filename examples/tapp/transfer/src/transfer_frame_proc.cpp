/*
 * Copyright (c) 2010, 腾讯科技有限公司互动娱乐研发部
 * All rights reserved
 *
 * File name:       transfer_proc.cpp
 * Description:     definition for functions about TAPPCTX->pfnProc
 * Version:         1.0
 *
 * Author:          flyma.IERD.IED.tencent
 * Creation time:   2010-06-04 16:44
 * Last modified:   2010-06-06 22:21
 *
 * History:
 * <date>        <author>      <version>   <description>
 * 2010-06-04     flyma         1.0         creation
 *
 */

#include "transfer.h"
#include "tlog/tlog.h"
#include "transfer_proc.h"
#include "transfer_utils.h"
#include "transfer_session_mgr.h"
#include "transfer_session_base.h"

static int transfer_frame_proc_request(TAPPCTX* a_pstTappCtx, TRANSFERENV* a_pstEnv);
static int transfer_proc_response(TAPPCTX* a_pstTappCtx, TRANSFERENV* a_pstEnv);


int transfer_proc(TAPPCTX* a_pstTappCtx, TRANSFERENV* a_pstEnv)
{
    assert(NULL != a_pstTappCtx);
    assert(NULL != a_pstEnv);

    int iCount = 0;
    iCount += transfer_frame_proc_request(a_pstTappCtx, a_pstEnv);
    iCount += transfer_proc_response(a_pstTappCtx, a_pstEnv);

    return iCount -1;
}

int  transfer_frame_proc_request(TAPPCTX* a_pstTappCtx, TRANSFERENV* a_pstEnv)
{
    assert(NULL != a_pstTappCtx);
    assert(NULL != a_pstEnv);

    int iRet = 0;
    int iBuff = 0;
    const char* pBuff = NULL;
    TFRAMEHEAD stFrameHead;
    unsigned int iAcceptLimit;
    unsigned int msgCount = 0;;

    iAcceptLimit = a_pstEnv->pstConf->dwMaxReqPerProc;

    do {
        iBuff = 0;
        iRet = TconndHandler::getInstance()->peekMsg(&pBuff, &iBuff);
        if (0 != iRet)
        {
            tlog_trace(a_pstEnv->pstLogCat, 0, 0, "peekMsg return no msg");
            break;
        }
        msgCount++;

        transfer_proc_connect_msg(a_pstTappCtx, a_pstEnv, &stFrameHead);
        if (0 >= iBuff)
        {
            TconndHandler::getInstance()->deleteMsg();
            continue;
        }

        /* decode msg from tconnd */
        FOREPKG stPkg;
        int iHostBuff = (int)sizeof(stPkg);
        iRet = TransferUtil::decodeMsg(a_pstEnv->pstRequestMeta,
                                       (char*)&stPkg, &iHostBuff, pBuff, &iBuff);
        if (0 > iRet)
        {
            tlog_error(a_pstEnv->pstLogCat, 0, 0, "failed to decode msg from tconnd");
            return -1;
        }

        /* pass request pkg from tconnd to application to process */
        transfer_proc_request_pkg(a_pstTappCtx, a_pstEnv, &stPkg);

        TconndHandler::getInstance()->deleteMsg();

    } while (msgCount < iAcceptLimit);

    return msgCount;
}

int transfer_proc_response(TAPPCTX* a_pstTappCtx, TRANSFERENV* a_pstEnv)
{
    assert(NULL != a_pstTappCtx);
    assert(NULL != a_pstEnv);

    int iRet = 0;

    size_t iBuff = 0;
    const char* pBuff = NULL;
    unsigned int iAcceptLimit;
    unsigned int msgCount = 0;;

    iAcceptLimit = a_pstEnv->pstConf->dwMaxResPerProc;

    while (msgCount < iAcceptLimit)
    {
        TBUSADDR iSrcAddr = -1;
        TBUSADDR iDstAddr = (TBUSADDR)a_pstTappCtx->iId;
        /* peek msg from background servers */
        iRet = tbus_peek_msg(a_pstTappCtx->iBus, &iSrcAddr, &iDstAddr, &pBuff, &iBuff, 0);
        if (TBUS_ERR_IS_ERROR(iRet))
        {
            tlog_trace(a_pstEnv->pstLogCat, 0, 0, "recv no msg from background servers");
            break;
        }
        msgCount++;

        /* decode msg */
        BACKPKG msgFromBackSvr;
        int iHostLen = (int)sizeof(msgFromBackSvr);
        int iNetMsgLen = iBuff;
        iRet = TransferUtil::decodeMsg(a_pstEnv->pstResponseMeta, (char*)&msgFromBackSvr,
                                       &iHostLen, pBuff, &iNetMsgLen);
        tbus_delete_msg(a_pstTappCtx->iBus, iSrcAddr, iDstAddr);
        if (TDR_ERR_IS_ERROR(iRet))
        {
            tlog_error(a_pstEnv->pstLogCat, 0, 0, "decode msg-head from servers failed");
            break;
        }

        /* dispatch response msg */
        transfer_dispatch_response_pkg(a_pstTappCtx, a_pstEnv, &msgFromBackSvr);
    }

    return msgCount;
}
