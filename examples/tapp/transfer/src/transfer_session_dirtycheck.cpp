/*
 * Copyright (c) 2010, 腾讯科技有限公司互动娱乐研发部
 * All rights reserved
 *
 * File name:       transfer_session_dirtycheck.cpp
 * Description:     member function definition for DirtyCheckSession
 * Version:         1.0
 *
 * Author:          flyma.IERD.IED.tencent
 * Creation time:   2010-06-09 10:09
 * Last modified:   2010-06-10 10:11
 *
 * History:
 * <date>        <author>      <version>   <description>
 * 2010-06-09     flyma         1.0         creation
 *
 */

#include "transfer.h"
#include "tlog/tlog.h"
#include "transfer_utils.h"
#include "transfer_session_dirtycheck.h"

int DirtyCheckSession::procReqMsg(TRANSFERENV* a_pstEnv, FOREPKG *a_pstReq)
{
    assert(NULL != a_pstEnv);
    assert(NULL != a_pstReq);

    int iRet = 0;

    /* construct msg to dirtycheck-svr */
    BACKPKG msgToDirtyCheckSvr;
    BACKDIRTYCHECKREQ* pstDstDirtyCheckReq = &msgToDirtyCheckSvr.stBody.stDirtyCheckReq;
    msgToDirtyCheckSvr.stHead.nCmd   = BACK_CMD_DIRTYCHECK_REQ;
    msgToDirtyCheckSvr.stHead.iSessionID = getID();
    const FOREDIRTYCHECKREQ* pstSrcDirtyCheckReq = &a_pstReq->stBody.stDirtyCheckReq;
    pstDstDirtyCheckReq->dwMsgLen    = pstSrcDirtyCheckReq->dwMsgLen;
    memcpy(pstDstDirtyCheckReq->szMsg, pstSrcDirtyCheckReq->szMsg, (int)pstSrcDirtyCheckReq->dwMsgLen);

    iRet = TransferUtil::sendPkgToServer(a_pstEnv, &msgToDirtyCheckSvr, a_pstEnv->iDirtyCheckAddr);

    return iRet;
}

int DirtyCheckSession::procResMsg(TRANSFERENV* a_pstEnv, BACKPKG *a_pstRes)
{
    assert(NULL != a_pstEnv);
    assert(NULL != a_pstRes);

    int iRet = 0;

    /* construct msg to tconnd */
    FOREPKG msgToTconnd;
    msgToTconnd.stHead.nCmd     = FORE_CMD_DIRTYCHECK_RES;
    msgToTconnd.stHead.ullMsgID = getMsgID();
    FOREDIRTYCHECKRES* pstDstDirtyCheckRes  = &msgToTconnd.stBody.stDirtyCheckRes;
    pstDstDirtyCheckRes->dwResultCode = FORE_NO_ERROR;
    const BACKDIRTYCHECKRES* pstSrcDirtyCheckRes  = &a_pstRes->stBody.stDirtyCheckRes;
    pstDstDirtyCheckRes->stResult.stNoError.dwMsgLen = pstSrcDirtyCheckRes->dwMsgLen;
    memcpy(pstDstDirtyCheckRes->stResult.stNoError.szMsg, pstSrcDirtyCheckRes->szMsg, (int)pstSrcDirtyCheckRes->dwMsgLen);

    iRet = TransferUtil::sendPkgToTconnd(a_pstEnv, &msgToTconnd, getFrameHead());

    setFinished();

    return iRet;
}

int DirtyCheckSession::onTimeout(TRANSFERENV* a_pstEnv)
{
    assert(NULL != a_pstEnv);

    int iRet = 0;

    /* construct msg to tconnd */
    FOREPKG msgToTconnd;
    msgToTconnd.stHead.nCmd     = FORE_CMD_DIRTYCHECK_RES;
    msgToTconnd.stHead.ullMsgID = getMsgID();
    FOREDIRTYCHECKRES* pstDstDirtyCheckRes  = &msgToTconnd.stBody.stDirtyCheckRes;
    pstDstDirtyCheckRes->dwResultCode = FORE_TIMEOUT;

    iRet = TransferUtil::sendPkgToTconnd(a_pstEnv, &msgToTconnd, getFrameHead());

    setFinished();

    return iRet;
}
