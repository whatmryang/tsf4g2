/*
 * Copyright (c) 2010, 腾讯科技有限公司互动娱乐研发部
 * All rights reserved
 *
 * File name:       transfer_session_base.cpp
 * Description:     member function definition for TransferSession
 * Version:         1.0
 *
 * Author:          flyma.IERD.IED.tencent
 * Creation time:   2010-06-04 14:47
 * Last modified:   2010-06-09 10:09
 *
 * History:
 * <date>        <author>      <version>   <description>
 * 2010-06-04     flyma         1.0         creation
 *
 */

#include "transfer.h"
#include "tlog/tlog.h"
#include "transfer_utils.h"
#include "transfer_session_base.h"


void TransferSession::saveFrameHead(const TFRAMEHEAD* a_pstHead)
{
    assert(NULL != a_pstHead);

    memcpy(&m_stFrameHead, a_pstHead, sizeof(m_stFrameHead));
}

int TransferSession::procReqMsg(TRANSFERENV* a_pstEnv,
                                FOREPKG *a_pstReq)
{
    assert(NULL != a_pstEnv);
    assert(NULL != a_pstReq);

    tlog_error(a_pstEnv->pstLogCat, 0, 0, "subclass of TransferSession should implement procReqMsg");

    return -1;
}

int TransferSession::procResMsg(TRANSFERENV* a_pstEnv,
                                BACKPKG *a_pstRes)
{
    assert(NULL != a_pstEnv);
    assert(NULL != a_pstRes);

    tlog_error(a_pstEnv->pstLogCat, 0, 0, "subclass of TransferSession should implement procResMsg");

    return -1;
}

int TransferSession::onTimeout(TRANSFERENV* a_pstEnv)
{
    assert(NULL != a_pstEnv);

    tlog_error(a_pstEnv->pstLogCat, 0, 0, "subclass of TransferSession should implement onTimeout");

    return -1;
}


