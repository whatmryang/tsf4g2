/*
 * Copyright (c) 2010, 腾讯科技有限公司互动娱乐研发部
 * All rights reserved
 *
 * File name:       tbus_timestamp_and_heartbeat.c
 * Description:     functions about timestamp and heartbeat.c
 * Version:         1.0
 *
 * Author:          flyma.IERD.IED.tencent
 * Creation time:   2010-07-16 18:15
 * Last modified:   2010-07-16 18:15
 *
 * History:
 * <date>        <author>      <version>   <description>
 * 2010-07-16     flyma         1.0         creation
 *
 */

#include "tbus/tbus.h"
#include "tbus/tbus_error.h"
#include "tbus_dyemsg.h"
#include "tbus_log.h"
#include "tbus_kernel.h"
#include "tbus_misc.h"
#include "tbus_channel.h"

#define TBUS_UPDATE_STATE_FLAG_BEATTIME 0x01
#define TBUS_UPDATE_STATE_FLAG_ENABLIVE 0x02

#define TBUS_COPY_PEER_STATE(pstOut, pstIn, piAddr)\
{\
    assert(NULL != (pstOut));\
    assert(NULL != (pstIn));\
    (pstOut)->iTimeOutGap = (pstIn)->iTimeOutGap;\
    (pstOut)->iIsAvailable = (pstIn)->iIsAvailable;\
    (pstOut)->llLastBeatTime = (pstIn)->llLastBeatTime;\
    if (NULL != (piAddr))\
    {\
        *(piAddr) = (pstIn)->iPeerAddr;\
    }\
}

#define TBUS_GET_PEER_STATE(iRet, iHandle, pstState, iPeerAddr)\
{\
    LPTBUSHANDLE pstHandle = NULL;\
    if (TBUS_ADDR_ALL == (iPeerAddr) || TBUS_ADDR_DEFAULT == (iPeerAddr))\
    {\
        tbus_log(TLOG_PRIORITY_ERROR, "parameter error, "#iPeerAddr"<%d> can't be TBUS_ADDR_ALL<%d>"\
                 " or TBUS_ADDR_DEFAULT<%d>", (iPeerAddr), TBUS_ADDR_ALL, TBUS_ADDR_DEFAULT) ;\
        (iRet) = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_UNEXPECTED_SPECIAL_ADDR);\
    }\
    \
    if (!TBUS_ERR_IS_ERROR((iRet)))\
    {\
            (pstHandle) = TBUS_GET_HANDLE((iHandle));\
            if (NULL == (pstHandle))\
            {\
                tbus_log(TLOG_PRIORITY_ERROR,"invalid handle %d", (iHandle));\
                iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_HANDLE);\
            }\
    }\
    \
    if (!TBUS_ERR_IS_ERROR((iRet)))\
    {\
        (pstState) = tbus_get_process_state_i((pstHandle), (iPeerAddr));\
        if (NULL == (pstState))\
        {\
            tbus_log(TLOG_PRIORITY_ERROR, "find no channel whose peeraddr is equal to %d", (iPeerAddr));\
            iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_NO_MATCHED_PEER);\
        }\
    }\
}

static int tbus_set_timestamp_flag(IN int a_iHandle, IN int a_iAddr1, IN int a_iAddr2, IN int a_iIsEnable);

static LPTBUSCHANNEL tbus_get_accurate_channel(IN LPTBUSHANDLE a_pstTbusHandle, IN int a_iAddr1, IN int a_iAddr2);

int tbus_send_heartbeat_msg_i(IN LPTBUSCHANNEL *a_ppstChannel,
                              IN int a_iChannleCnt, IN TBUSADDR a_iPeerAddr,
                              IN int a_iFlag, IN int a_iMsgType);

static int tbus_update_peer_state_i(IN int a_iHandle, IN TBUSADDR a_iPeerAddr,
                                    const struct timeval *a_pstCurTime, IN int a_iFlag);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int tbus_set_current_time(IN const struct timeval *a_pstTime)
{
    int iRet = TBUS_SUCCESS;

    if (NULL == a_pstTime)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"parameter error, a_pstTime is null" ) ;

        return TBUS_ERR_ARG;
    }

    g_stBusGlobal.stCurTime = *a_pstTime;

    return iRet;
}

int tbus_use_saved_time(void)
{
    int iRet = TBUS_SUCCESS;

    g_stBusGlobal.iFlag |= TBUS_INIT_FLAG_USE_SAVED_TIME;

    return iRet;
}

int tbus_use_sys_time(void)
{
    int iRet = TBUS_SUCCESS;

    g_stBusGlobal.iFlag &= ~TBUS_INIT_FLAG_USE_SAVED_TIME;

    return iRet;
}

int tbus_get_msg_timestamp(IN int a_iHandle, OUT LPTBUSTIMESTAMP a_pstTimeStamp)
{
    int iRet = TBUS_SUCCESS;
    LPTBUSHANDLE pstTbusHandle = NULL;

    if (NULL == a_pstTimeStamp)
    {
        tbus_log(TLOG_PRIORITY_ERROR, "parameter error, a_pstTimeStamp is null");

        return TBUS_ERR_ARG;
    }

    pstTbusHandle = TBUS_GET_HANDLE(a_iHandle);
    if (NULL == pstTbusHandle)
    {
        tbus_log(TLOG_PRIORITY_ERROR, "invalid handle %d", a_iHandle);

        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_HANDLE);
    }

    a_pstTimeStamp->iIsAvailable = pstTbusHandle->stRecvHead.bFlag & TBUS_HEAD_FLAG_WITH_TIMESTAMP;

    if (a_pstTimeStamp->iIsAvailable)
    {
        a_pstTimeStamp->llSec  = pstTbusHandle->stRecvHead.stTimeStamp.llSec;
        a_pstTimeStamp->dwUSec = pstTbusHandle->stRecvHead.stTimeStamp.dwUSec;
        a_pstTimeStamp->dwCumm = pstTbusHandle->stRecvHead.stTimeStamp.dwCumm;
    }

    return iRet;
}

int tbus_turn_timestamp_on(IN int a_iHandle, IN TBUSADDR a_iAddr1, IN TBUSADDR a_iAddr2)
{
    int iRet = TBUS_SUCCESS;

    iRet = tbus_set_timestamp_flag(a_iHandle, a_iAddr1, a_iAddr2, 1);

    return iRet;
}

int tbus_turn_timestamp_off(IN int a_iHandle, IN TBUSADDR a_iAddr1, IN TBUSADDR a_iAddr2)
{
    int iRet = TBUS_SUCCESS;

    iRet = tbus_set_timestamp_flag(a_iHandle, a_iAddr1, a_iAddr2, 0);

    return iRet;
}

LPTBUSCHANNEL tbus_get_accurate_channel(IN LPTBUSHANDLE a_pstTbusHandle, IN int a_iAddr1, IN int a_iAddr2)
{
    LPTBUSCHANNEL pstChannel = NULL;
    int i = 0;

    assert(NULL != a_pstTbusHandle);

    for (i = 0; i < a_pstTbusHandle->dwChannelCnt; i++)
    {
        LPCHANNELHEAD pstHead = a_pstTbusHandle->pastChannelSet[i]->pstHead;

        if ((a_iAddr1 == pstHead->astAddr[0] && a_iAddr2 == pstHead->astAddr[1])
            || (a_iAddr1 == pstHead->astAddr[1] && a_iAddr2 == pstHead->astAddr[0]))
        {
            pstChannel = a_pstTbusHandle->pastChannelSet[i];
            break;
        }
    }

    return pstChannel;
}

int tbus_get_next_peer_addr(IN int a_iHandle, INOUT TBUSADDR *a_piPeerAddr)
{
    int iRet = TBUS_SUCCESS;
    LPTBUSHANDLE pstTbusHandle = NULL;
    LPTBUSPROCESSSTATE pstProcessState = NULL;

    if (NULL == a_piPeerAddr)
    {
        tbus_log(TLOG_PRIORITY_ERROR, "invalid parameter, a_piPeerAddr is null");
        return TBUS_ERR_ARG;
    }

    pstTbusHandle = TBUS_GET_HANDLE(a_iHandle);
    if (NULL == pstTbusHandle)
    {
        tbus_log(TLOG_PRIORITY_ERROR, "invalid handle %d", a_iHandle);

        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_HANDLE);
    }

    if (0 == pstTbusHandle->dwPeerCnt)
    {
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_NO_MORE_PEER);
    }

    if (0 == *a_piPeerAddr)
    {
        *a_piPeerAddr = pstTbusHandle->astPeerState[0].iPeerAddr;
    } else
    {

        if (*a_piPeerAddr == pstTbusHandle->astPeerState[pstTbusHandle->dwChannelCnt-1].iPeerAddr)
        {
            return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_NO_MORE_PEER);
        }

        pstProcessState = tbus_get_process_state_i(pstTbusHandle, *a_piPeerAddr);
        if (NULL == pstProcessState)
        {
            return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_NO_MATCHED_PEER);
        }

        pstProcessState++;
        *a_piPeerAddr = pstProcessState->iPeerAddr;
    }

    return iRet;
}

int tbus_get_peer_state_detail(OUT LPTBUSPEERSTATE a_pstPeerState, IN int a_iHandle, IN TBUSADDR a_iPeerAddr)
{
    int iRet = TBUS_SUCCESS;
    LPTBUSPROCESSSTATE pstPeerState = NULL;

    if ( NULL == a_pstPeerState )
    {
        tbus_log(TLOG_PRIORITY_ERROR,"parameter error, a_pstPeerState is null" ) ;
        iRet = TBUS_ERR_ARG;
        return iRet ;
    }

    TBUS_GET_PEER_STATE(iRet, a_iHandle, pstPeerState, a_iPeerAddr);
    if (TBUS_ERR_IS_ERROR(iRet))
    {
        return iRet;
    }
    TBUS_COPY_PEER_STATE(a_pstPeerState, pstPeerState, (TBUSADDR*)NULL);

    return iRet;
}

int tbus_set_peer_timeout_gap(IN int a_iHandle, IN TBUSADDR a_iPeerAddr, IN int a_iTimeout)
{
    int iRet = TBUS_SUCCESS;
    LPTBUSPROCESSSTATE pstPeerState = NULL;

    if (TBUS_ADDR_DEFAULT == a_iPeerAddr)
    {
        tbus_log(TLOG_PRIORITY_ERROR, "parameter error, a_iPeerAddr<%d> can't be"
                 " TBUS_ADDR_DEFAULT<%d>", a_iPeerAddr, TBUS_ADDR_DEFAULT);
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_UNEXPECTED_SPECIAL_ADDR);
    }

    if (0 > a_iTimeout)
    {
        tbus_log(TLOG_PRIORITY_ERROR, "a_iTimeout<%d> should be non-minus", a_iTimeout);
        return TBUS_ERR_ARG;
    }

    if (TBUS_ADDR_ALL == a_iPeerAddr)
    {
        unsigned int i = 0;
        LPTBUSHANDLE pstTbusHandle = NULL;

        pstTbusHandle = TBUS_GET_HANDLE(a_iHandle);
        if (NULL == pstTbusHandle)
        {
            tbus_log(TLOG_PRIORITY_ERROR, "invalid handle %d", a_iHandle);
            return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_HANDLE);
        }

        for (i = 0; i < pstTbusHandle->dwPeerCnt; i++)
        {
            pstTbusHandle->astPeerState[i].iTimeOutGap = a_iTimeout;
        }
    } else
    {
        TBUS_GET_PEER_STATE(iRet, a_iHandle, pstPeerState, a_iPeerAddr);
        if (TBUS_ERR_IS_ERROR(iRet))
        {
            return iRet;
        }

        pstPeerState->iTimeOutGap = a_iTimeout;
    }
    tbus_log(TLOG_PRIORITY_INFO, "tbus_set_peer_timeout_gap<addr:%d, to:%d> sucessfully",
             a_iPeerAddr, a_iTimeout);

    return iRet;
}

int tbus_get_peer_states(OUT int *a_piPreState, OUT int *a_piCurState, IN int a_iHandle,
                         IN TBUSADDR a_iPeerAddr, IN int a_iNeedGetCurTime)
{
    int iRet = TBUS_SUCCESS;
    LPTBUSPROCESSSTATE pstPeerState = NULL;

    if ( NULL == a_piPreState)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"parameter error, a_piPreState is null" ) ;
        iRet = TBUS_ERR_ARG;
        return iRet ;
    }

    if ( NULL == a_piCurState)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"parameter error, a_piCurState is null" ) ;
        iRet = TBUS_ERR_ARG;
        return iRet ;
    }

    TBUS_GET_PEER_STATE(iRet, a_iHandle, pstPeerState, a_iPeerAddr);
    if (TBUS_ERR_IS_ERROR(iRet))
    {
        return iRet;
    }

    if (0 == pstPeerState->iTimeOutGap)
    {
        *a_piCurState = TBUS_PEER_STATE_AVAILABLE;
    } else
    {
        struct timeval stCurTime;
        struct timeval *pstReferTime = NULL;

        if (a_iNeedGetCurTime || !(g_stBusGlobal.iFlag & TBUS_INIT_FLAG_USE_SAVED_TIME))
        {
            pstReferTime = &stCurTime;
            gettimeofday(pstReferTime, NULL);
        } else
        {
            pstReferTime = &g_stBusGlobal.stCurTime;
        }

        tbus_log(TLOG_PRIORITY_TRACE, "peer addr<%d>, llLastBeatTime<%lld>, current time<%lld>",
                 a_iPeerAddr, pstPeerState->llLastBeatTime, (long long)pstReferTime->tv_sec);

        if ((long long)pstReferTime->tv_sec <= pstPeerState->llLastBeatTime)
        {
            *a_piCurState = TBUS_PEER_STATE_AVAILABLE;
        } else
        {
            long long llNoHeartBeatGap;
            llNoHeartBeatGap = (long long)pstReferTime->tv_sec - pstPeerState->llLastBeatTime;
            if (llNoHeartBeatGap >= pstPeerState->iTimeOutGap)
            {
                *a_piCurState = TBUS_PEER_STATE_UNAVAILABLE;
            } else
            {
                *a_piCurState = TBUS_PEER_STATE_AVAILABLE;
            }
        }
    }

    *a_piPreState = pstPeerState->iIsAvailable;
    if (*a_piCurState != *a_piPreState)
    {
        pstPeerState->iIsAvailable = *a_piCurState;
    }

    return iRet;
}


/* 下面一部分代码如果实现了基于控制通道的心跳机制，应该移动到 tbus.h */
/**
  @brief 查询本地保存的a_iPeerAddr对应的进程是否存活
  @param[out] a_piIsAlive    指向接收状态信息的本地变量
  @param[in]  a_iHandle      指定tbus句柄，句柄可由tbus_new获取
  @param[in]  a_iPeerAddr    指定通道对端进程的地址
 *
 @pre a_piIsAlive不能为NULL
 @pre a_iPeerAddr不能为特殊地址0
 @note 绑定通道时，就认为对端进程是正在运行的，只有tbus收到(通过tbus_recv触发)进程退出消息后，
 * -调用tbus_is_peer_alive才返回 *a_piIsAlive==0, 表明对端进程已经退出.
 * -tbus再次收到(通过tbus_recv触发)心跳消息时，调用tbus_is_peer_alive将返回 *a_piIsAlive==1,
 * -表明对端进程正在运行
 @ref tbus_get_peer_state_detail
 */
TSF4G_API int tbus_is_peer_alive(OUT int *a_piIsAlive, IN TBUSADDR a_iHandle, IN int a_iPeerAddr);

int tbus_is_peer_alive(OUT int *a_piIsAlive, IN TBUSADDR a_iHandle, IN int a_iPeerAddr)
{
    int iRet = TBUS_SUCCESS;
    LPTBUSPROCESSSTATE pstPeerState = NULL;

    if ( NULL == a_piIsAlive)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"parameter error, a_piIsAlive is null" ) ;
        iRet = TBUS_ERR_ARG;
        return iRet ;
    }

    TBUS_GET_PEER_STATE(iRet, a_iHandle, pstPeerState, a_iPeerAddr);
    if (TBUS_ERR_IS_ERROR(iRet))
    {
        return iRet;
    }

    *a_piIsAlive = pstPeerState->iIsAlive;

    return iRet;
}

int tbus_send_heartbeat(IN int a_iHandle, IN TBUSADDR a_iPeerAddr, IN int a_iFlag)
{
    return tbus_send_heartbeat_msg(a_iHandle, a_iPeerAddr, a_iFlag, TBUS_HEARTBEAT_MSG_TYPE_SYN);
}

/* 下面一部分代码如果实现了基于控制通道的心跳机制，应该移动到 tbus.h */
/**
  @brief 向a_iPeerAddr对应的进程的发送本地进程退出的消息
  @param[in]  a_iHandle      指定tbus句柄，句柄可由tbus_new获取
  @param[in]  a_iPeerAddr    指定通道对端进程的地址
  @param[in]  a_iFlag        保留参数，目前未使用
 *
 @pre a_iPeerAddr不能为特殊地址0
 */
TSF4G_API int tbus_notify_peer_quit(IN int a_iHandle, IN TBUSADDR a_iPeerAddr, IN int a_iFlag);

int tbus_notify_peer_quit(IN int a_iHandle, IN TBUSADDR a_iPeerAddr, IN int a_iFlag)
{
    return tbus_send_heartbeat_msg(a_iHandle, a_iPeerAddr, a_iFlag, TBUS_HEARTBEAT_MSG_TYPE_QUIT);
}

/* 下面一部分代码如果实现了基于控制通道的心跳机制，应该移动到 tbus.h */
/**
  @brief 清除a_iPeerAddr对应的进程状态的退出标志位
  @param[in]  a_iHandle      指定tbus句柄，句柄可由tbus_new获取
  @param[in]  a_iPeerAddr    指定通道对端进程的地址
 *
 @pre a_iPeerAddr不能为特殊地址0
 @note 如果a_iPeerAddr==-1, 表示清除所有a_iHandle句柄下的通道的进程退出标志位
 @note 这个接口用来清除一个通道上的进程退出状态
 */
TSF4G_API int tbus_set_peer_to_be_alive(IN int a_iHandle, IN TBUSADDR a_iPeerAddr);

int tbus_set_peer_to_be_alive(IN int a_iHandle, IN TBUSADDR a_iPeerAddr)
{
    return tbus_update_peer_state_i(a_iHandle, a_iPeerAddr, NULL, TBUS_UPDATE_STATE_FLAG_ENABLIVE);
}

int tbus_update_peer_state_i(IN int a_iHandle, IN TBUSADDR a_iPeerAddr,
                             const struct timeval *a_pstCurTime, IN int a_iFlag)
{
    int iRet = TBUS_SUCCESS;
    LPTBUSHANDLE pstTbusHandle = NULL;
    LPTBUSPROCESSSTATE pstPeerState = NULL;
    struct timeval stCurTime;
    const struct timeval *pstCurTime;

    if (TBUS_ADDR_DEFAULT == a_iPeerAddr)
    {
        tbus_log(TLOG_PRIORITY_ERROR, "parameter error, a_iPeerAddr<%d> can't be"
                 " TBUS_ADDR_DEFAULT<%d>", a_iPeerAddr, TBUS_ADDR_DEFAULT);
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_UNEXPECTED_SPECIAL_ADDR);
    }

    if (TBUS_UPDATE_STATE_FLAG_BEATTIME & a_iFlag)
    {
        if (NULL != a_pstCurTime)
        {
            pstCurTime = a_pstCurTime;
        } else
        {
            gettimeofday(&stCurTime, NULL);
            pstCurTime = &stCurTime;
        }
    }

    if (TBUS_ADDR_ALL == a_iPeerAddr)
    {
        unsigned int i = 0;
        pstTbusHandle = TBUS_GET_HANDLE(a_iHandle);
        if (NULL == pstTbusHandle)
        {
            tbus_log(TLOG_PRIORITY_ERROR, "invalid handle %d", a_iHandle);
            return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_HANDLE);
        }

        /* put for loop inside if can only have a little preformance enhancement */
        for (i = 0; i < pstTbusHandle->dwPeerCnt; i++)
        {
            if (TBUS_UPDATE_STATE_FLAG_BEATTIME & a_iFlag)
            {
                pstTbusHandle->astPeerState[i].llLastBeatTime = (long long)pstCurTime->tv_sec;
            }
            if (TBUS_UPDATE_STATE_FLAG_ENABLIVE & a_iFlag)
            {
                pstTbusHandle->astPeerState[i].iIsAlive = 1;
            }
        }
    } else
    {
        TBUS_GET_PEER_STATE(iRet, a_iHandle, pstPeerState, a_iPeerAddr);
        if (TBUS_ERR_IS_ERROR(iRet))
        {
            return iRet;
        }
        if (TBUS_UPDATE_STATE_FLAG_BEATTIME & a_iFlag)
        {
            pstPeerState->llLastBeatTime = (long long)pstCurTime->tv_sec;
        }
        if (TBUS_UPDATE_STATE_FLAG_ENABLIVE & a_iFlag)
        {
            pstPeerState->iIsAlive = 1;
        }
    }


    return iRet;
}

int tbus_set_timestamp_flag(IN int a_iHandle, IN int a_iAddr1, IN int a_iAddr2, IN int a_iIsEnable)
{
    int iRet = TBUS_SUCCESS;
    LPTBUSCHANNEL pstChannel = NULL;
    LPTBUSHANDLE pstTbusHandle = NULL;

    if (TBUS_ADDR_ALL == a_iAddr1 || TBUS_ADDR_DEFAULT == a_iAddr1
        || TBUS_ADDR_ALL == a_iAddr2 || TBUS_ADDR_DEFAULT == a_iAddr2)
    {
        tbus_log(TLOG_PRIORITY_ERROR, "invalid bus address, "
                 "both a_iAddr1<%d> and a_iAddr2<%d>"
                 "can't be special bus address(%d, %d)",
                 a_iAddr1, a_iAddr2,
                 TBUS_ADDR_DEFAULT, TBUS_ADDR_ALL);
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_UNEXPECTED_SPECIAL_ADDR);
    }

    pstTbusHandle = TBUS_GET_HANDLE(a_iHandle);
    if (NULL == pstTbusHandle)
    {
        tbus_log(TLOG_PRIORITY_ERROR, "invalid handle %d", a_iHandle);
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_HANDLE);
    }

    pstChannel = tbus_get_accurate_channel(pstTbusHandle, a_iAddr1, a_iAddr2);
    if (NULL == pstChannel)
    {
        tbus_log(TLOG_PRIORITY_ERROR, "no matched channel in handle<%d> for addrs<%d, %d>",
                 a_iHandle, a_iAddr1, a_iAddr2);
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_NO_CHANNEL_MATCHED);
    }

    if (a_iIsEnable)
    {
        TBUS_CHANNEL_ENABLE_TIMESTAMP(pstChannel);
    } else
    {
        TBUS_CHANNEL_DISABLE_TIMESTAMP(pstChannel);
    }

    return iRet;
}

int tbus_send_heartbeat_msg(IN int a_iHandle, IN TBUSADDR a_iPeerAddr, IN int a_iFlag, IN int a_iMsgType)
{
    LPTBUSHANDLE pstTbusHandle = NULL;

    if (TBUS_ADDR_DEFAULT == a_iPeerAddr)
    {
        tbus_log(TLOG_PRIORITY_ERROR, "parameter error, a_iPeerAddr<%d> can't be"
                 " TBUS_ADDR_DEFAULT<%d>", a_iPeerAddr, TBUS_ADDR_DEFAULT);
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_UNEXPECTED_SPECIAL_ADDR);
    }

    pstTbusHandle = TBUS_GET_HANDLE(a_iHandle);
    if (NULL == pstTbusHandle)
    {
        tbus_log(TLOG_PRIORITY_ERROR, "invalid handle %d", a_iHandle);
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_HANDLE);
    }

    if ( 0 >= pstTbusHandle->dwChannelCnt)
    {
        tbus_log(TLOG_PRIORITY_INFO,"no channel(count:%d) to send data", pstTbusHandle->dwChannelCnt) ;
        if (TBUS_ADDR_ALL != a_iPeerAddr)
        {
            /* 对于特定地址而言，如果没有通道，返回错误 */
            return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_NO_CHANNEL) ;
        } else
        {
            /* 对于广播地址而言，如果没有通道，返回成功 */
            return TBUS_SUCCESS;
        }
    }

    return tbus_send_heartbeat_msg_i(pstTbusHandle->pastChannelSet,
                                     (int)pstTbusHandle->dwChannelCnt,
                                     a_iPeerAddr, a_iFlag, a_iMsgType);
}

int tbus_send_heartbeat_msg_i(IN LPTBUSCHANNEL *a_ppstChannel, IN int a_iChannleCnt,
                              IN TBUSADDR a_iPeerAddr, IN int a_iFlag, IN int a_iMsgType)
{
    int iRet = TBUS_SUCCESS;
    unsigned int i = 0;
    TBUSHEAD stHead;

    assert(NULL != a_ppstChannel);

    tbus_init_head_for_send(&stHead, a_iFlag);
    tbus_set_head_timestamp(&stHead, NULL, (a_iFlag & TBUS_FLAG_USE_GETTIMEOFDAY));
    stHead.bFlag |= TBUS_HEAD_FLAG_HEARTBEAT;
    stHead.iHeartBeatMsgType = a_iMsgType;

    iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_NO_CHANNEL_MATCHED);

    for (i = 0; i < a_iChannleCnt; i++)
    {
        LPTBUSCHANNEL pstChannel = a_ppstChannel[i];
        TBUSADDR iSrcAddr = TBUS_ADDR_ALL;
        TBUSADDR iDstAddr = a_iPeerAddr;
        int iMatched = 0;
        int iPushRet = 0;

        TBUS_SELECT_CHANNEL(iMatched, pstChannel, iSrcAddr, iDstAddr);
        if (0 == iMatched)
        {
            continue ;
        }

        iRet = TBUS_SUCCESS;

        if (TBUS_PEER_STATE_UNAVAILABLE == pstChannel->pstPeerState->iIsAvailable)
        {
            continue;
        }

        stHead.iSrc = iSrcAddr;
        stHead.iDst = iDstAddr;
        TBUS_SET_LAST_ROUTE(stHead, iSrcAddr);

        iPushRet = tbus_push_channel_pkgv(pstChannel, &stHead, (struct iovec*)1, 0);
        if (iPushRet != TBUS_SUCCESS)
        {
            tbus_log(TLOG_PRIORITY_ERROR, "failed to send heartbeat<type:%d, iSrc:0x%x, iDst:0x%x>"
                     "iPushRet<%d> for %s", a_iMsgType, iSrcAddr, iDstAddr, iPushRet, tbus_error_string(iPushRet));
            continue ; /* batch sent mode, skipped individual error */
        }

        tbus_log(TLOG_PRIORITY_TRACE, "Send heartbeat<type:%d, iSrc:0x%x, iDst:0x%x>",
                 a_iMsgType, iSrcAddr, iDstAddr);

        if(stHead.bFlag & TBUS_HEAD_FLAG_TACE)
        {
            tbus_log_dyedpkg(&stHead, "Send");
        }
    }

    return iRet;
}
