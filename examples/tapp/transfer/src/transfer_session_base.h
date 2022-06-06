/*
 * Copyright (c) 2010, 腾讯科技有限公司互动娱乐研发部
 * All rights reserved
 *
 * File name:       transfer_session_base.h
 * Description:     definition of TransferSession
 * Version:         1.0
 *
 * Author:          flyma.IERD.IED.tencent
 * Creation time:   2010-06-03 22:03
 * Last modified:   2010-06-09 10:09
 *
 * History:
 * <date>        <author>      <version>   <description>
 * 2010-06-03     flyma         1.0         creation
 *
 */

#ifndef _TRANSFER_SESSION_H_
#define _TRANSFER_SESSION_H_

#include <new>
#include "tdr/tdr.h"
#include "tconnapi/tconnapi.h"
#include "transfer_fore_proto.h"
#include "transfer_backsvr_proto.h"

using std::nothrow;

class TransferSession
{
    public:

        TransferSession()
        {
            gettimeofday(&m_birthTime, NULL);
            m_finished = false;
        }

        virtual ~TransferSession() {}

        void setID(int a_ID)
        {
            m_ID = a_ID;
        }

        int getID() const
        {
            return m_ID;
        }

        virtual int procReqMsg(TRANSFERENV* a_pstEnv, FOREPKG *a_pstReq);
        virtual int procResMsg(TRANSFERENV* a_pstEnv, BACKPKG *a_pstRes);
        virtual int onTimeout(TRANSFERENV* a_pstEnv);

        void saveFrameHead(const TFRAMEHEAD* a_pstHead);

        const TFRAMEHEAD* getFrameHead() const
        {
            return &m_stFrameHead;
        }

        const timeval* getCreationTime()
        {
            return &m_birthTime;
        }

        void setFinished() { m_finished = true; };

        bool isFinished() { return true == m_finished; }

        void* operator new (size_t, void* a_pSpace)
        {
            return a_pSpace;
        }

    private:

        TFRAMEHEAD m_stFrameHead;

        int m_ID;

        timeval m_birthTime;

        bool m_finished;
};

#endif
