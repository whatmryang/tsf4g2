/*
 * Copyright (c) 2010, ��Ѷ�Ƽ����޹�˾���������з���
 * All rights reserved
 *
 * File name:       transfer_session_mgr.h
 * Description:     SessionMgr template
 * Version:         1.0
 *
 * Author:          flyma.IERD.IED.tencent
 * Creation time:   2010-06-09 10:09
 * Last modified:   2010-06-09 10:09
 *
 * History:
 * <date>        <author>      <version>   <description>
 * 2010-06-09     flyma         1.0         creation
 *
 */

#ifndef _TRANSFER_SESSION_MGR_
#define _TRANSFER_SESSION_MGR_

#include <new>
#include "transfer.h"
#include "tlog/tlog.h"
#include "comm/tmempool.h"
#include "transfer_session_base.h"

template <typename SessionType>
class SessionMgr
{
    public:
        static SessionMgr* getInstance();

        int init(size_t a_iMaxSessionNum,int  a_iScanNum, int a_iTimeout);

        SessionType* allocSession();

        //conf  reload refresh
        void SetTimeoutGap(int a_iTimeout) {m_iTimeout = a_iTimeout;};

        void SetScanNum(int iScanNum) {m_iScanNum = iScanNum;};

        SessionType* getSession(int a_iSessionID);

        int freeSession(int a_iSessionID);

        //ɨ��session���У��ҳ�����ʱ��session�����ص���session��
        // OnTick�ص��ӿڣ�Ȼ��ɾ����session
        void onTick(const timeval* a_pstCurTime, TRANSFERENV* a_pstEnv);

    private:
        static SessionMgr* s_instance;

        SessionMgr() : m_hasInited(false), m_pstSessionPool(NULL), m_beginIdx4onTick(0),
            m_iPoolSize(0),m_iScanNum(0),m_iTimeout(0){}

        ~SessionMgr()
        {
            if (NULL != m_pstSessionPool)
            {
                tmempool_destroy(&m_pstSessionPool);
                m_pstSessionPool = NULL;
            }
        }

        bool m_hasInited;

        TMEMPOOL* m_pstSessionPool;

        int m_beginIdx4onTick;

        int m_iPoolSize;
        int m_iScanNum;
        int m_iTimeout;

};

template <typename SessionType>
SessionMgr<SessionType>* SessionMgr<SessionType>::s_instance = NULL;

template <typename SessionType>
SessionMgr<SessionType>* SessionMgr<SessionType>::getInstance()
{
    if (NULL == s_instance)
    {
        s_instance = new(std::nothrow) SessionMgr();
        if (NULL == s_instance)
        {
            tlog_error(g_stEnv.pstLogCat, 0, 0, "falied to new SessionMgr s_instance");
            exit(1);
        }
    }
    return s_instance;
}

/*��ʼ��һ��session��������ÿ������������ĳ��session�Ķ��ʵ��
@param  a_iMaxSessionNum �˹�������Ҫ�����session�����Ŀ
@param a_iScanNum   ÿ��session��ʱ��鶨ʱ���������ɨ��session��
@param a_iTimeout session ��ʱ��ֵ������sessioni����ʱ�䵽��ǰʱ��ļ��ʱ��
    ���ڴ˷�ֵ����Ϊ��session��ʱ
*/
template <typename SessionType>
int SessionMgr<SessionType>::init(size_t a_iMaxSessionNum, int  a_iScanNum, int a_iTimeout)
{
    int iRet = 0;

    if (0 >= a_iMaxSessionNum)
    {
         tlog_error(g_stEnv.pstLogCat, 0, 0, "invalid  a_iMaxSessionNum<%u>",
                   a_iMaxSessionNum);
         return -1;
    }
    iRet = tmempool_new(&m_pstSessionPool, a_iMaxSessionNum, sizeof(SessionType));
    if (iRet > 0)
    {
        tlog_error(g_stEnv.pstLogCat, 0, 0, "tmempool_new failed, item-num<%d>, item-size<%d>",
                   a_iMaxSessionNum, (int)sizeof(SessionType));
        return -1;
    } else
    {
        tlog_trace(g_stEnv.pstLogCat, 0, 0, "tmempool_new sucessful, item-num<%d>, item-size<%d>",
                   a_iMaxSessionNum, (int)sizeof(SessionType));
    }

    m_iPoolSize = a_iMaxSessionNum;
    m_hasInited = true;
    m_iScanNum = a_iScanNum;
    m_iTimeout = a_iTimeout;
    return iRet;
}

template <typename SessionType>
SessionType* SessionMgr<SessionType>::allocSession()
{
    int idx = -1;

    assert(m_hasInited);

    idx = tmempool_alloc(m_pstSessionPool);
    if (0 > idx)
    {
        tlog_error(g_stEnv.pstLogCat, 0, 0, "tmempool_alloc failed, ret<%d>", idx);
        return NULL;
    }

    void* objSpace = tmempool_get(m_pstSessionPool, idx);
    if (NULL == objSpace)
    {
        tlog_fatal(g_stEnv.pstLogCat, 0, 0, "tmempool_get failed, idx<%d>", idx);
        return NULL;
    }
    SessionType* pSession = new (objSpace) SessionType();
    if (NULL == pSession)
    {
        tlog_fatal(g_stEnv.pstLogCat, 0, 0, "tmempool_get failed, idx<%d>", idx);
        return NULL;
    }
    tlog_trace(g_stEnv.pstLogCat, 0, 0, "allocate one session by tmempool_get, idx<%d>", idx);

    pSession->setID(idx);

    return pSession;
}


template <typename SessionType>
SessionType* SessionMgr<SessionType>::getSession(int a_iSessionID)
{
    assert(m_hasInited);

    if (0 > a_iSessionID)
    {
        tlog_error(g_stEnv.pstLogCat, 0, 0, "invalid session-id<%d> to getSession", a_iSessionID);
        return NULL;
    }

    SessionType* pSession = (SessionType*)tmempool_get(m_pstSessionPool, a_iSessionID);
    if (NULL == pSession)
    {
        tlog_fatal(g_stEnv.pstLogCat, 0, 0, "tmempool_get failed, idx<%d>", a_iSessionID);
        return NULL;
    }

    return pSession;
}

template <typename SessionType>
int SessionMgr<SessionType>::freeSession(int a_iSessionID)
{
    assert(m_hasInited);

    int iRet = tmempool_free(m_pstSessionPool, a_iSessionID);
    if (0 > iRet)
    {
        tlog_error(g_stEnv.pstLogCat, 0, 0, "tmempool_free<id:%d> failed, ret<%d>",
                   a_iSessionID, iRet);
        return -1;
    }

    return iRet;
}

template <typename SessionType>
void SessionMgr<SessionType>::onTick(const timeval* a_pstCurTime, TRANSFERENV* a_pstEnv)
{
    TransferSession* pSession = NULL;
    assert(NULL != a_pstCurTime);
    assert(NULL != a_pstEnv);



    assert(m_hasInited);


    for (int i = 0; i < m_iScanNum; i++)
    {
        m_beginIdx4onTick = (m_beginIdx4onTick + 1) % m_iPoolSize;
        pSession = (TransferSession*)tmempool_get_bypos(m_pstSessionPool,m_beginIdx4onTick);
        if (NULL == pSession)
        {
           continue;
        }

        /* check wether session is timeout */
        const timeval& beginTime = *pSession->getCreationTime();
        timeval timeGap;
        TV_DIFF(timeGap, *a_pstCurTime, beginTime);
        int timeGapInMS;
        TV_TO_MS(timeGapInMS, timeGap);

        if (m_iTimeout < timeGapInMS)
        {
            tlog_error(g_stEnv.pstLogCat, 0, 0, "session<idx: %d> timeout<%d>, limit<%d>",
                       pSession->getID(), timeGapInMS, m_iTimeout);

            //session��ʱ�ˣ�ͨ���ص�����֪ͨһ��
            pSession->onTimeout(a_pstEnv);

            //��ʱsessionֱ�ӻ�����
            freeSession(pSession->getID());

            g_stEnv.pstRunCumu->ullTotalTimeoutNum++;
        }
    }
}

#endif
