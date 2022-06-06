/*
 * Copyright (c) 2010, 腾讯科技有限公司互动娱乐研发部
 * All rights reserved
 *
 * File name:       transfer_session_dirtycheck.h
 * Description:     definition for DirtyCheckSession
 * Version:         1.0
 *
 * Author:          flyma.IERD.IED.tencent
 * Creation time:   2010-06-09 10:32
 * Last modified:   2010-06-10 10:11
 *
 * History:
 * <date>        <author>      <version>   <description>
 * 2010-06-09     flyma         1.0         creation
 *
 */

#ifndef _TRANSFER_SESSION_DIRTYCHECK_H_
#define _TRANSFER_SESSION_DIRTYCHECK_H_


#include "transfer_session_base.h"

class DirtyCheckSession: public TransferSession
{
    public:
        virtual int procReqMsg(TRANSFERENV* a_pstEnv, FOREPKG *a_pstReq);
        virtual int procResMsg(TRANSFERENV* a_pstEnv, BACKPKG *a_pstRes);
        virtual int onTimeout(TRANSFERENV* a_pstEnv);

    public:
        void saveMsgID(unsigned long long a_ullMsgID) { m_ullMsgID = a_ullMsgID; }
        unsigned long long getMsgID() const { return m_ullMsgID; }

    private:
        unsigned long long m_ullMsgID;
};


#endif
