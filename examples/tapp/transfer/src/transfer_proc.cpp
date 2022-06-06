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
 * Last modified:   2010-06-09 10:09
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
#include "transfer_session_dirtycheck.h"

void transfer_proc_connect_msg(TAPPCTX* a_pstTappCtx, TRANSFERENV* a_pstEnv,
                               TFRAMEHEAD* a_pstFrameHead)
{
    assert(NULL != a_pstTappCtx);
    assert(NULL != a_pstEnv);
    assert(NULL != a_pstFrameHead);

    tlog_trace(a_pstEnv->pstLogCat, 0, 0, "a_pstFrameHead->chCmd<%d>",
               (int)a_pstFrameHead->chCmd);

    //TODO 如果逻辑服务器关注前端接入连接的创建和关闭
    //并且需要将连接维护和回话关联起来，就在此处添加
    //连接管理代码
    /* example code: process tconnapi msg about connention */
    /* example code begin */
    switch (a_pstFrameHead->chCmd)
    {
        case TFRAMEHEAD_CMD_START:
            // TODO
            break;
        case TFRAMEHEAD_CMD_STOP:
            // TODO
            break;
        default:
            // TODO
            break;
    }
    /* example code end */
}

void transfer_proc_request_pkg(TAPPCTX* a_pstTappCtx, TRANSFERENV* a_pstEnv,
                               FOREPKG *a_pstReq)
{
    assert(NULL != a_pstTappCtx);
    assert(NULL != a_pstEnv);
    assert(NULL != a_pstReq);

    int iRet = 0;

    tlog_trace(a_pstEnv->pstLogCat, 0, 0, "recved msg nCmd<%d>", a_pstReq->stHead.nCmd);

    /* add dispatch logic here */
    /* example code: dispatch by msg.cmd */
    /* example code begin */
    switch(a_pstReq->stHead.nCmd)
    {
        case FORE_CMD_DIRTYCHECK_REQ:
            a_pstEnv->pstRunCumu->ullTotalDirtyCheckReqNum++;
            transfer_proc_dirtycheck_req(a_pstTappCtx, a_pstEnv, a_pstReq);
            break;
        default:
            tlog_error(a_pstEnv->pstLogCat, 0, 0, "unsupported msg.cmd<%d>",
                       a_pstReq->stHead.nCmd);
            iRet = -1;
            break;
    }
    /* example code end */
}



void transfer_proc_dirtycheck_req(TAPPCTX* a_pstTappCtx, TRANSFERENV* a_pstEnv, FOREPKG *a_pstReq)
{
    assert(NULL != a_pstTappCtx);
    assert(NULL != a_pstEnv);
    assert(NULL != a_pstReq);

    int iRet = 0;

    /* wether to create a session depends on a_pstReq */
    /* for dirtyCheckReq we create a session as an example */
    DirtyCheckSession* pSession = SessionMgr<DirtyCheckSession>::getInstance()->allocSession();
    if (NULL == pSession)
    {
        if (a_pstEnv->pstConf->iKeepReqWhenFull)
        {
            TconndHandler::getInstance()->keepBusMsgUntouched();
        } else
        {
            a_pstEnv->pstRunCumu->ullTotalDirtyCheckReqDroped++;
        }
        tlog_error(a_pstEnv->pstLogCat, 0, 0, "SessionMgr<DirtyCheckSession> may be full");
        return;
    }

    /* this code shows how to save data for subclass of TransferSession */
    pSession->saveMsgID(a_pstReq->stHead.ullMsgID);

    /* tconnd frame-head in session */
    pSession->saveFrameHead(TconndHandler::getInstance()->getCurMsgFrame());

    iRet = pSession->procReqMsg(a_pstEnv, a_pstReq);
    if (0 > iRet)
    {
        tlog_error(a_pstEnv->pstLogCat, 0, 0, "DirtyCheckSession procReqMsg failed");
        return;
    }
}

void transfer_dispatch_response_pkg(TAPPCTX* a_pstTappCtx, TRANSFERENV* a_pstEnv, BACKPKG* a_pstRes)
{
    assert(NULL != a_pstTappCtx);
    assert(NULL != a_pstEnv);
    assert(NULL != a_pstRes);

    tlog_trace(a_pstEnv->pstLogCat, 0, 0, "dispatch response pkg<cCmd:%d>", a_pstRes->stHead.nCmd);

    /* add dispatch logic here */
    /* example code: dispatch by msg.cmd */
    /* example code begin */
    TransferSession* pSession = NULL;
    switch(a_pstRes->stHead.nCmd)
    {
        case BACK_CMD_DIRTYCHECK_RES:
            {
                a_pstEnv->pstRunCumu->ullTotalDirtyCheckResNum++;
                pSession = SessionMgr<DirtyCheckSession>::getInstance()
                    ->getSession(a_pstRes->stHead.iSessionID);
                if (NULL == pSession)
                {
                    a_pstEnv->pstRunCumu->ullTotalIllDirtyCheckRes++;
                    tlog_error(a_pstEnv->pstLogCat, 0, 0, "can't find DirtyCheckSession by id<%d>",
                               a_pstRes->stHead.iSessionID);
                    return;
                }
                pSession->procResMsg(a_pstEnv, a_pstRes);
                if (pSession->isFinished())
                {
                    SessionMgr<DirtyCheckSession>::getInstance()->freeSession(pSession->getID());
                    pSession = NULL;
                }
            }
            break;
        default:
            tlog_error(a_pstEnv->pstLogCat, 0, 0, "unsupported msg.cmd<%d>", a_pstRes->stHead.nCmd);
            break;
    }
    /* example code end */
}
