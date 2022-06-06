/*
 * Copyright (c) 2010, 腾讯科技有限公司互动娱乐研发部
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

        //扫描session队列，找出处理超时的session，并回调此session的
        // OnTick回调接口，然后删除此session
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

/*初始化一个session管理器，每个管理器管理某种session的多个实例
@param  a_iMaxSessionNum 此管理器需要管理的session最大数目
@param a_iScanNum   每次session超时检查定时器操作最大扫描session数
@param a_iTimeout session 超时阀值，即从sessioni创建时间到当前时间的间隔时间
    大于此阀值就认为此session超时
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

            //session超时了，通过回调函数通知一下
            pSession->onTimeout(a_pstEnv);

            //超时session直接回收了
            freeSession(pSession->getID());

            g_stEnv.pstRunCumu->ullTotalTimeoutNum++;
        }
    }
}

#endif
