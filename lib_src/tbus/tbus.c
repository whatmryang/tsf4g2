/** @file $RCSfile: tbus.c,v $
  general description of this module
  $Id: tbus.c,v 1.43 2009-09-16 11:55:25 jacky Exp $
  @author $Author: jacky $
  @date $Date: 2009-09-16 11:55:25 $
  @version $Revision: 1.43 $
  @note Editor: Vim 6.3, Gcc 4.0.2, tab=4
  @note Platform: Linux
  */

#include "tbus/tbus.h"
#include "tbus/tbus_error.h"
#include "tbus_dyemsg.h"
#include "tbus_log.h"
#include "tbus_kernel.h"
#include "tbus_misc.h"
#include "tbus_channel.h"

TBUSGLOBAL g_stBusGlobal = { //tbus全局
    TBUS_MODULE_NOT_INITED,
    PTHREAD_MUTEX_INITIALIZER,
    TBUS_DEFAUL_BUSSINESS_ID,
    TBUS_INIT_FLAG_NONE,
};

extern unsigned char g_szMetalib_TbusHead[];

#if defined (_WIN32) || defined (_WIN64)
extern void tbus_fini_security_attr();
extern int tbus_init_security_attr();
#endif

/**
  various internal function defined
  */

int tbus_global_init(void);

int handle_check_in ( IN const int a_iHandle ) ;

int tbus_disable_channel_i(TBUSADDR    a_iSrc, TBUSADDR a_iDst, LPTBUSHANDLE a_pstExcludeHandle);

//检查指定的通道是否已经被其他句柄调用tbus_connect指定，如果是则返回非零值
int tbus_owned_exclusive_by_others_i(TBUSADDR    a_iSrc, TBUSADDR a_iDst, LPTBUSHANDLE a_pstExcludeHandle);

// 设置tbus句柄为exlusive模式, 从而指定句柄只接收tbus_connect指定的通道
int tbus_handle_set_exclusive(LPTBUSHANDLE a_pstHandle);

int tbus_add_channels_associated_addr( IN int a_iHandle, IN TBUSADDR a_iBindAddr, TBUSSHMGCIM *a_pstGCIM);

int tbus_generate_counterfeit_pkg(LPTBUSHEAD a_pstHead, int *a_piPkgLen);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int tbus_init ( IN const unsigned int a_iMapKey, IN const unsigned int a_iSize)
{
    int iRet = TBUS_SUCCESS;


    TOS_UNUSED_ARG(a_iSize);
    if (0 == a_iMapKey)
    {
        iRet = tbus_init_ex(TBUS_DEFAULT_GCIM_KEY, TBUS_INIT_FLAG_NONE);
    }else
    {
        char szKey[32] = {0};
        snprintf(szKey, sizeof(szKey), "%d", a_iMapKey);
        iRet = tbus_init_ex(szKey, TBUS_INIT_FLAG_NONE);
    }

    return iRet ;
}


int tbus_new ( INOUT int *a_piHandle )
{
    int iRet = 0 ;
    LPTBUSHANDLE pstTbusHandle = NULL;

    tbus_set_logpriority(TLOG_PRIORITY_TRACE);


    if ( NULL == a_piHandle )
    {
        tbus_log(TLOG_PRIORITY_ERROR,"parameter error, a_piHandle is null" ) ;
        iRet = TBUS_ERR_ARG ;
        return iRet ;
    }

    *a_piHandle = 0;
    if ( TBUS_MODULE_INITED != g_stBusGlobal.dwInitStatus )
    {
        tbus_log(TLOG_PRIORITY_ERROR,"global bus module not inited, so cannot assign handle" ) ;
        iRet = TBUS_ERR_INIT ;
        return iRet ;
    }

    pthread_mutex_lock(&g_stBusGlobal.tMutex);
    if (g_stBusGlobal.dwHandleCnt >= HANDLE_COUNT - 1)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"global bus handle num reach the max number(%d)", g_stBusGlobal.dwHandleCnt ) ;
        iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_HANDLE_FULL) ; ;
    }

    if (0 == iRet)
    {
        int i = 0;

        for (i = 1; i < HANDLE_COUNT; i++)
        {
            if (NULL == g_stBusGlobal.pastTbusHandle[i])
            {
                break;
            }
        }
        assert(i < HANDLE_COUNT);

        pstTbusHandle = (TBUSHANDLE *) malloc ( sizeof(TBUSHANDLE) ) ;
        if ( NULL == pstTbusHandle )
        {
            tbus_log(TLOG_PRIORITY_ERROR,"malloc() size failed %"PRIdPTR"", sizeof(TBUSHANDLE) ) ;
            iRet =  TBUS_ERROR_NO_MEMORY ;
        }else
        {
            g_stBusGlobal.pastTbusHandle[i] = pstTbusHandle;

            memset(pstTbusHandle , 0x0, sizeof(TBUSHANDLE) ) ;
            pstTbusHandle->iRecvPos = TBUS_RECV_POS_INVALID ;
            pstTbusHandle->dwFlag |= TBUS_HANDLE_FLAG_ENABLE;
            pstTbusHandle->dwGCIMVersion = g_stBusGlobal.pstGCIM->stHead.dwVersion;

            tbus_log(TLOG_PRIORITY_DEBUG,"new handle(index:%i %p) successfully", i, pstTbusHandle);
            ++g_stBusGlobal.dwHandleCnt;
            *a_piHandle = i;
        }
    }
    pthread_mutex_unlock(&g_stBusGlobal.tMutex);

    tbus_set_logpriority(TLOG_PRIORITY_ERROR);

    return iRet ;
}



int tbus_bind( IN const int a_iHandle, IN const TBUSADDR a_iSrcAddr)
{
    char szTmpIP[128];
    STRNCPY(szTmpIP, tbus_addr_ntoa(a_iSrcAddr), sizeof(szTmpIP));
    return tbus_bind_by_str ( a_iHandle,  &szTmpIP[0]) ;
}

int tbus_add_channels_associated_addr( IN int a_iHandle, IN TBUSADDR a_iBindAddr, TBUSSHMGCIM *a_pstGCIM)
{
    LPTBUSSHMGCIMHEAD pstHead ;
    unsigned int i;
    unsigned int iChannelCount;
    int iRet = 0;
    TBUSADDR iPeerAddr = 0;
    LPTBUSHANDLE pstBusHandle = NULL;

    assert(NULL != a_pstGCIM);

    pstBusHandle = TBUS_GET_HANDLE(a_iHandle);
    assert(NULL != pstBusHandle);

    pstHead = &a_pstGCIM->stHead;
    iChannelCount = pstHead->dwUsedCnt ;
    for ( i=0; i<iChannelCount; i++ )
    {
        TBUSSHMCHANNELCNF *pstChannel = &g_stBusGlobal.pstGCIM->astChannels[i];
        tbus_log(TLOG_PRIORITY_TRACE,"channel(%d): Flag:0x%x  address(0x%08x <-->0x%08x\n",
                 i, pstChannel->dwFlag,  pstChannel->astAddrs[0], pstChannel->astAddrs[1]) ;
        if (!TBUS_GCIM_CHANNEL_IS_ENABLE(pstChannel) )
        {
            continue ;
        }
        if (a_iBindAddr == pstChannel->astAddrs[0])
        {
            iPeerAddr = pstChannel->astAddrs[1];
        }else if (a_iBindAddr == pstChannel->astAddrs[1])
        {
            iPeerAddr = pstChannel->astAddrs[0];
        }else
        {
            continue;
        }

        //如果此通道被其他句柄互斥式占有，则不能绑定此通道
        if (tbus_owned_exclusive_by_others_i(a_iBindAddr, iPeerAddr, pstBusHandle))
        {
            char szTmpIP[128];
            STRNCPY(szTmpIP, tbus_addr_ntoa(a_iBindAddr), sizeof(szTmpIP));
            tbus_log(TLOG_PRIORITY_INFO,"the channel(%s<-->%s) is "
                     "connected by other handle, so cannot bind to this hanle",
                     szTmpIP, tbus_addr_ntoa(iPeerAddr)) ;
            continue;
        }

        /* 将此通道加入到句柄中 */
        iRet = tbus_enable_addr(a_iBindAddr, iPeerAddr, pstChannel, a_iHandle, pstHead) ;
        if ( TBUS_SUCCESS != iRet )
        {
            char szTmpIP[128];
            STRNCPY(szTmpIP, tbus_addr_ntoa(a_iBindAddr), sizeof(szTmpIP));
            tbus_log(TLOG_PRIORITY_ERROR,"tbus_bind(), failed bind %ith channel(%s<-->%s), Ret: %d",
                     i+1, szTmpIP, tbus_addr_ntoa(iPeerAddr), iRet ) ;
            break ;
        }
    }/*for ( i=0; i<iChannelCount; i++ )*/

    return iRet;
}

int tbus_bind_by_str ( IN const int a_iHandle, IN const char *a_szSrcAddr )
{
    int iRet = TBUS_SUCCESS ;
    TBUSADDR iAddr = 0 ;
    TBUSHANDLE *pstTbusHandle;
    LPTBUSSHMGCIMHEAD pstHead ;

    tbus_set_logpriority(TLOG_PRIORITY_TRACE);
    if ( NULL == a_szSrcAddr )
    {
        tbus_log(TLOG_PRIORITY_ERROR,"parameter error, a_szSrcAddr is null" ) ;
        iRet = TBUS_ERR_ARG ;
        return iRet ;
    }

    tbus_log(TLOG_PRIORITY_TRACE,"begin to  bind addr(%s) to handle(%d)\n", a_szSrcAddr, a_iHandle) ;
    pstTbusHandle = TBUS_GET_HANDLE(a_iHandle);
    if (NULL == pstTbusHandle)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"invalid handle %d", a_iHandle);
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_HANDLE);
    }
    iRet = tbus_addr_aton(a_szSrcAddr, &iAddr);
    if (0 != iRet)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"tbus_addr_aton failed to convert %s to tbusaddr, iret %d",
                 a_szSrcAddr, iRet);
        return iRet;
    }

    /*记录绑定的地址*/
    if (pstTbusHandle->iBindAddrNum < TBUS_MAX_BIND_ADDR_NUM_PREHANDLE)
    {
        pstTbusHandle->aiBindAddr[pstTbusHandle->iBindAddrNum] = iAddr;
        pstTbusHandle->iBindAddrNum++;
    }


    /* 以读锁，锁住GCIM共享内存，以从GCIM中读出数据*/
    pstHead = &g_stBusGlobal.pstGCIM->stHead;
    tbus_wrlock(&pstHead->stRWLock);
    iRet = tbus_add_channels_associated_addr(a_iHandle, iAddr, g_stBusGlobal.pstGCIM);
    tbus_unlock(&pstHead->stRWLock);

    if ((TBUS_SUCCESS == iRet) && (0 >= pstTbusHandle->dwChannelCnt))
    {
        tbus_log(TLOG_PRIORITY_ERROR,"cannot bind any channel by addrress: %s",    a_szSrcAddr) ;
        iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_BIND_FAILED);
    }

    tbus_log(TLOG_PRIORITY_TRACE,"end of tbus_bind addr %s to handle('%d'), iRet=%i", a_szSrcAddr, a_iHandle, iRet ) ;
    tbus_set_logpriority(TLOG_PRIORITY_ERROR);

    return iRet ;
}

int IsChannelInGCIM(CHANNELHEAD * a_pstHead, TBUSSHMGCIM *a_pstGCIM)
{
    unsigned int i;

    assert(NULL != a_pstHead);
    assert(NULL != a_pstGCIM);

    for (i = 0; i < a_pstGCIM->stHead.dwUsedCnt; i++)
    {
        LPTBUSSHMCHANNELCNF  pstChannelCnf = &a_pstGCIM->astChannels[i];
        if (!TBUS_GCIM_CHANNEL_IS_ENABLE(pstChannelCnf))
        {
            continue;
        }
        if ((a_pstHead->astAddr[0] == pstChannelCnf->astAddrs[0]) &&
            (a_pstHead->astAddr[1] == pstChannelCnf->astAddrs[1]))
        {
            return 1;
        }
    }/*for (i = 0; i < a_pstGCIM->stHead.dwUsedCnt; i++)*/

    return 0;
}

/**
  @brief 根据全局GCIM中的配置刷新tbus句柄管理的相关通道
  如果绑定地址相关通道有添加则自动添加到tbus中；如果tbus句柄管理的相关通道已经不再gcim配置
  中则定期回收
  @param[in] a_iHandle tbus处理句柄，通过调用tbus_new() 获取

  @retval 0 success
  @retval <0 failed, 可能的错误代码如下:
  -    TBUS_ERR_ARG    传递给接口的参数不对
  -    TBUS_ERROR_NOT_INITIALIZED    bus系统还没有初始化
  -    TBUS_ERROR_CHANNEL_NUM_LIMITED
  -    TBUS_ERROR_NO_MEMORY
  -    TBUS_ERROR_SHMGET_FAILED
  -    TBUS_ERROR_SHMAT_FAILED
  -    TBUS_ERROR_CHANNEL_ADDRESS_CONFLICT
  -    TBUS_ERROR_BIND_FAILED
  @note tbus API使用者可以定时调用本接口以刷新该句柄下相关通道配置

  @pre a_iHandle 为有效句柄
  @see tbus_new
  @see tbus_bind
  */
TSF4G_API int tbus_refresh_handle( IN const int a_iHandle)
{
    int iRet = TBUS_SUCCESS ;
    unsigned int i = 0 ;
    TBUSCHANNEL *pstChannel;
    time_t tNow;
    TBUSHANDLE *pstTbusHandle;
    LPTBUSSHMGCIMHEAD pstHead ;


    pstTbusHandle = TBUS_GET_HANDLE(a_iHandle);
    if (NULL == pstTbusHandle)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"invalid handle %d", a_iHandle);
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_HANDLE);
    }

    /*首先检查回收不再使用的通道*/
    time(&tNow);
    for (i = 0; i < pstTbusHandle->dwChannelCnt; i++)
    {
        pstChannel = pstTbusHandle->pastChannelSet[i];
        if (TBUS_CHANNEL_IS_ENABLE(pstChannel) || (!TBUS_CHANNEL_IS_NOT_IN_GCIM(pstChannel)))
        {
            continue;
        }

        /*如果不在配置中的通道预留一段时间后超时，则关闭或尝试清除它*/

        if ((tNow - pstChannel->tBeginDisable) >= TBUS_DISABLE_CHANNEL_CLEAR_DISABLE_TIMEOUTGAP)
        {
            tbus_log(TLOG_PRIORITY_FATAL, "the time which the channel(0x%08x <-->0x%08x) is disabled reach"
                     "%d second, so clear the channel", pstChannel->pstHead->astAddr[0],
                     pstChannel->pstHead->astAddr[1], TBUS_DISABLE_CHANNEL_CLEAR_DISABLE_TIMEOUTGAP);
            tbus_close_channel(&pstTbusHandle->pastChannelSet[i]);
            if (i < pstTbusHandle->dwChannelCnt -1 )
            {
                pstTbusHandle->pastChannelSet[i] = pstTbusHandle->pastChannelSet[pstTbusHandle->dwChannelCnt -1];
                pstTbusHandle->pastChannelSet[pstTbusHandle->dwChannelCnt -1] = NULL;
            }
            pstTbusHandle->dwChannelCnt--;
            i--;
            if (pstTbusHandle->iRecvPos >= (int)pstTbusHandle->dwChannelCnt)
            {
                pstTbusHandle->iRecvPos = 0;
            }
        }/*if ((tNow - pstChannel->tBeginDisable) >= TBUS_DISABLE_CHANNEL_CLEAR_DISABLE_TIMEOUTGAP)*/
    }/*for (i = 0; i < pstTbusHandle->dwChannelCnt; i++)*/


    /*通过版本号检查gcim的配置是否有变化*/
    pstHead = &g_stBusGlobal.pstGCIM->stHead;
    if (pstHead->dwVersion == pstTbusHandle->dwGCIMVersion)
    {
        return TBUS_SUCCESS;
    }
    tbus_log(TLOG_PRIORITY_TRACE, "the gicmversion(%u) recorded at handle(%d) diff from "
             "the shm gcim version(%u), so try to refer configure", pstTbusHandle->dwGCIMVersion,
             a_iHandle, pstHead->dwVersion);

    /*尝试锁gcim配置，如果锁失败则下次再尝试*/
    if (0 != pthread_rwlock_trywrlock(&pstHead->stRWLock))
    {
        return TBUS_SUCCESS;
    }

    /*首先检查通用配置中去掉的通道*/
    for (i = 0; i < pstTbusHandle->dwChannelCnt; i++)
    {
        pstChannel = pstTbusHandle->pastChannelSet[i];
        if (IsChannelInGCIM(pstChannel->pstHead, g_stBusGlobal.pstGCIM))
        {
            continue;
        }else
        {
            /*如果此通道不在gcim通道中，则设置此通道不可用*/
            TBUS_CHANNEL_CLR_ENABLE(pstChannel);
            TBUS_CHANNEL_SET_NOT_IN_GCIM(pstChannel);
            pstChannel->tBeginDisable = tNow;
            tbus_log(TLOG_PRIORITY_DEBUG, "the time which the channel(0x%08x <-->0x%08x) is not in gcim "
                     "so set the NOT_IN_GCIM flag and disable it", pstChannel->pstHead->astAddr[0],
                     pstChannel->pstHead->astAddr[1]);
        }
    }/*for (i = 0; i < pstTbusHandle->dwChannelCnt; i++)*/

    /*通过绑定地址查询是否有新的通道需要添加到管理句柄中*/
    for (i = 0; i < (unsigned int)pstTbusHandle->iBindAddrNum; i++)
    {
        iRet = tbus_add_channels_associated_addr(a_iHandle, pstTbusHandle->aiBindAddr[i], g_stBusGlobal.pstGCIM);
        if (0 != iRet)
        {
            break;
        }
    }/*for (i = 0; i < (int)pstTbusHandle->iBindAddrNum; i++)*/

    if (0 == iRet)
    {
        pstTbusHandle->dwGCIMVersion = pstHead->dwVersion;
    }

    /*解锁*/
    pthread_rwlock_unlock(&pstHead->stRWLock);


    return iRet ;
}

int tbus_connect_by_str ( IN const int a_iHandle, IN const char *a_szDstAddr )

{
    int iRet = TBUS_SUCCESS,
        i = 0,
        iAddrCnt = 0 ;
    TBUSADDR iDstAddr = 0 ;
    TBUSCHANNEL *pstChannel = NULL ;
    TBUSHANDLE *pstTbusHandle;


    tbus_set_logpriority(TLOG_PRIORITY_TRACE);
    if ( NULL == a_szDstAddr )
    {
        tbus_log(TLOG_PRIORITY_ERROR,"parameter error,a_szDstAddr is null" ) ;
        iRet = TBUS_ERR_ARG ;
        return iRet ;
    }
    tbus_log(TLOG_PRIORITY_DEBUG,"Use handle(%d) connect to ('%s')", a_iHandle, a_szDstAddr ) ;

    pstTbusHandle = TBUS_GET_HANDLE(a_iHandle);
    if (NULL == pstTbusHandle)
    {
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_HANDLE);
    }
    iRet = tbus_addr_aton(a_szDstAddr, &iDstAddr);
    if (0 != iRet)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"tbus_addr_aton failed to convert %s to tbusaddr, iret %d",
                 a_szDstAddr, iRet);
        return iRet;
    }

    iAddrCnt = pstTbusHandle->dwChannelCnt ;
    tbus_wrlock(&g_stBusGlobal.pstGCIM->stHead.stRWLock);
    for ( i=0; i<iAddrCnt; i++ )
    {
        CHANNELHEAD *pstHead;
        pstChannel = pstTbusHandle->pastChannelSet[i] ;

        pstHead = pstChannel->pstHead;
        if (iDstAddr != pstHead->astAddr[pstChannel->iSendSide])
        {
            continue;
        }

        //找到了需要conncet的通道
        TBUS_CHANNEL_SET_CONNECTED(pstChannel);
        tbus_log(TLOG_PRIORITY_DEBUG,"connect channel(%s<-->%s) succefully, ",
                 tbus_addr_ntoa(pstHead->astAddr[pstChannel->iRecvSide]), a_szDstAddr) ;

        //句柄是互斥模式
        if (TBUS_HANDLE_IS_EXCLUSIVE(pstTbusHandle))
        {
            if (!TBUS_CHANNEL_IS_DISABLE_BY_CTRL(pstChannel))
            {
                TBUS_CHANNEL_SET_ENABLE(pstChannel);
            }

            //如果设置了 互斥通道模式，则清除其他句柄对此channel的应用
            iRet = tbus_disable_channel_i(pstChannel->pstHead->astAddr[pstChannel->iRecvSide], iDstAddr, pstTbusHandle);

        }/*if (TBUS_HANDLE_IS_EXCLUSIVE(pstTbusHandle))*/
        break;
    }/*for ( i=0; i<iAddrCnt; i++ )*/

    if (i >= iAddrCnt)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"failed to connect %s, no channel peer address match with it", a_szDstAddr);
        iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_NO_PEER_ADDRESS_MATCHED);
    }

    tbus_unlock(&g_stBusGlobal.pstGCIM->stHead.stRWLock);
    tbus_set_logpriority(TLOG_PRIORITY_ERROR);

    return iRet ;
}


int tbus_send (IN const int a_iHandle, INOUT TBUSADDR *a_piSrc, INOUT TBUSADDR *a_piDst,
               IN const void *a_pvData, IN const size_t a_iDataLen, IN const int a_iFlag)
{
    int iRet = TBUS_SUCCESS;
    struct iovec astVectors[1];

    if ((NULL == a_pvData) || (0 >= a_iDataLen))
    {
        tbus_log(TLOG_PRIORITY_ERROR,
                 "invalid param: a_pvData:%p a_iDataLen:%"PRIdPTR"",
                 a_pvData, a_iDataLen);
        return TBUS_ERR_ARG;
    }

    if (NULL == a_piSrc || NULL == a_piDst)
    {
        tbus_log(TLOG_PRIORITY_ERROR,
                 "invalid param: a_piSrc:%p a_piDst:%p",
                 a_piSrc, a_piDst);
        return TBUS_ERR_ARG;
    }

    astVectors[0].iov_base = (void *)a_pvData;
    astVectors[0].iov_len = a_iDataLen;

    iRet = tbus_sendv(a_iHandle, a_piSrc, a_piDst, &astVectors[0], 1, a_iFlag);

    return iRet ;
}

void tbus_set_head_timestamp(IN LPTBUSHEAD a_pstHead, IN LPTBUSHEAD a_pstPreHead, IN int a_iNeedGetCurTime)
{
    struct timeval stCurTime;
    struct timeval *pstReferTime = NULL;

    assert(NULL != a_pstHead);
    /* a_pstPreHead == NULL is permitted */
    /* assert(NULL != a_pstPreHead); */

    if (a_iNeedGetCurTime || !(g_stBusGlobal.iFlag & TBUS_INIT_FLAG_USE_SAVED_TIME))
    {
        pstReferTime = &stCurTime;
        gettimeofday(pstReferTime, NULL);
    } else
    {
        pstReferTime = &g_stBusGlobal.stCurTime;
    }
    a_pstHead->stTimeStamp.llSec = pstReferTime->tv_sec;
    a_pstHead->stTimeStamp.dwUSec = (unsigned int)pstReferTime->tv_usec;

    if (NULL != a_pstPreHead
        && (a_pstPreHead->bFlag & TBUS_FLAG_WITH_TIMESTAMP))
    {
        unsigned int dwTimeDelta = 0;

        a_pstHead->stTimeStamp.dwCumm = a_pstPreHead->stTimeStamp.dwCumm;
        TBUS_CALC_TIME_DELTA(dwTimeDelta, a_pstHead->stTimeStamp, a_pstPreHead->stTimeStamp);
        if (0 != dwTimeDelta)
        {
            a_pstHead->stTimeStamp.dwCumm += dwTimeDelta;
        }
    } else
    {
        a_pstHead->stTimeStamp.dwCumm = 0;
    }

    a_pstHead->bFlag |= TBUS_HEAD_FLAG_WITH_TIMESTAMP;
}

int tbus_sendv (
                IN const int a_iHandle,
                INOUT int *a_piSrc,
                INOUT int *a_piDst,
                IN const struct iovec *a_ptVector,
                IN const int a_iVecCnt,
                IN const int a_iFlag
               )
{
    int iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_NO_CHANNEL_MATCHED);
    int iAddrCnt = 0;
    int i = 0;
    TBUSADDR iSrcAddr = 0;
    TBUSADDR iDstAddr = 0;
    TBUSADDR iRetSrc = 0;
    TBUSADDR iRetDst = 0;
    LPTBUSHANDLE pstHandle;
    TBUSHEAD stHead;

    if ( (NULL == a_piSrc) || (NULL == a_piDst) || (NULL == a_ptVector) || (0 >= a_iVecCnt) )
    {
        tbus_log(TLOG_PRIORITY_ERROR,
                 "tbus_send, parameter error, null paramter or VecCnt(%d) less than 0",
                 a_iVecCnt ) ;
        return TBUS_ERR_ARG ;
    }


    iRetSrc = *a_piSrc;
    iRetDst = *a_piDst ;

    pstHandle = TBUS_GET_HANDLE(a_iHandle);
    if (NULL == pstHandle)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"failed to get handle(%d)", a_iHandle);
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_HANDLE);
    }
    iAddrCnt = pstHandle->dwChannelCnt ;
    if ( 0 >= iAddrCnt )
    {
        tbus_log(TLOG_PRIORITY_ERROR,"no channel(count:%d) to send data", iAddrCnt ) ;
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_NO_CHANNEL) ;
    }

    tbus_init_head_for_send(&stHead, a_iFlag);

    for ( i=0; i<iAddrCnt; i++ )
    {
        LPTBUSCHANNEL pstChannel = pstHandle->pastChannelSet[i] ;
        int iMatched =0;

        iSrcAddr = *a_piSrc ;
        iDstAddr = *a_piDst ;
        TBUS_SELECT_CHANNEL(iMatched, pstChannel, iSrcAddr, iDstAddr);
        if (0 == iMatched)
        {
            continue ;
        }

        stHead.iSrc = iSrcAddr;
        stHead.iDst = iDstAddr;
        TBUS_SET_LAST_ROUTE(stHead, iSrcAddr);

        if (TBUS_CHANNEL_WITH_TIMESTAMP(pstChannel)
            || (TBUS_FLAG_WITH_TIMESTAMP & a_iFlag))
        {
            tbus_set_head_timestamp(&stHead, NULL, (a_iFlag & TBUS_FLAG_USE_GETTIMEOFDAY));
        }

        iRet = tbus_push_channel_pkgv(pstChannel, &stHead, a_ptVector, a_iVecCnt) ;
        if (iRet != TBUS_SUCCESS)
        {
            tbus_log(TLOG_PRIORITY_ERROR,"tbus_send, failed to push data to channel(%d), iRet:%d", i, iRet ) ;
            if ( (TBUS_ADDR_ALL == *a_piSrc) || (TBUS_ADDR_ALL == *a_piDst) )
            {
                continue ; /* batch sent mode, skipped individual error */
            }else
            {
                /* failed return, break here, not batch sent mode */
                break ;
            }
        }/*if (iRet != TBUS_SUCCESS)*/

        tbus_log(TLOG_PRIORITY_DEBUG,
                 "Send data(len:%d) to channel %d,  destination  address %s",
                 stHead.iBodyLen, i, tbus_addr_ntoa(iDstAddr) ) ;
        if(stHead.bFlag & TBUS_HEAD_FLAG_TACE)
        {
            tbus_log_dyedpkg(&stHead, "Send");
        }
        iRet = TBUS_SUCCESS ;

        /* record first successful channel address */
        iRetSrc = iSrcAddr ;
        iRetDst = iDstAddr ;
        if ( (TBUS_ADDR_ALL != *a_piSrc) && (TBUS_ADDR_ALL != *a_piDst) )
        {
            /* break here, not batch sent mode */
            break ;
        }

    } /* end for ( i=0; i<iAddrCnt; i++ ) */

    /* set return address */
    if (0 != iRetSrc)
    {
        *a_piSrc = iRetSrc ;
    }
    if (0 != iRetDst)
    {
        *a_piDst = iRetDst ;
    }


    return iRet ;
}

int tbus_recv_ex (INOUT LPTBUSRECV a_pstRecv)
{
    int iRet = TBUS_SUCCESS;

    if (NULL == a_pstRecv)
    {
        tbus_log(TLOG_PRIORITY_ERROR, "paramter error, a_pstRecv is NULL");
        return TBUS_ERR_ARG;
    }

    iRet = tbus_recv(a_pstRecv->iHandle, &a_pstRecv->iSrc, &a_pstRecv->iDst,
                     a_pstRecv->pvData, &a_pstRecv->tDataLen, a_pstRecv->iFlag);
    if (TBUS_ERR_IS_ERROR(iRet))
    {
        return iRet;
    }

    if (NULL == a_pstRecv->pstTimeStamp)
    {
        return iRet;
    }

    iRet = tbus_get_msg_timestamp(a_pstRecv->iHandle, a_pstRecv->pstTimeStamp);
    if (TBUS_ERR_IS_ERROR(iRet))
    {
        a_pstRecv->pstTimeStamp->iIsAvailable = 0;
        /* 此处对tbus_get_msg_timestamp的调用错误需要向caller隐藏 */
        iRet = TBUS_SUCCESS;
    }

    return iRet;
}

int tbus_recv (    IN const int a_iHandle,    INOUT int *a_piSrc,    INOUT int *a_piDst,
               OUT void *a_pvData,    INOUT size_t *a_piDataLen,    IN const int a_iFlag)
{
    int iRet = TBUS_ERR_CHANNEL_EMPTY;
    TBUSADDR    iSrcAddr = 0;
    TBUSADDR    iDstAddr = 0;
    int i = 0 ;
    TBUSHANDLE *pstHandle = NULL ;
    TBUSCHANNEL *ptChannel = NULL ;
    int iAddrCnt;


    if ( (NULL == a_piSrc) || (NULL == a_piDst) || (NULL == a_pvData) || (NULL == a_piDataLen))
    {
        tbus_log(TLOG_PRIORITY_ERROR,"parameter error, null parameter " ) ;
        return TBUS_ERR_ARG ;
    }
    if (0 >= *a_piDataLen)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"parameter error, size(%"PRIdPTR") of buffer to recv data is less than 0", *a_piDataLen) ;
        return TBUS_ERR_ARG ;
    }


    /*Get handle*/
    pstHandle = TBUS_GET_HANDLE(a_iHandle);
    if (NULL == pstHandle)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"failed to get handle(%d)", a_iHandle);
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_HANDLE);
    }

    iAddrCnt = pstHandle->dwChannelCnt ;
    if ( 0 >= iAddrCnt )
    {
        tbus_log(TLOG_PRIORITY_ERROR,"no channel(count:%d) to recv data", iAddrCnt ) ;
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_NO_CHANNEL) ;
    }

    /*select position of channel which search channel to recv data from it */
    if (0 > pstHandle->iRecvPos )
    {
        pstHandle->iRecvPos = 0 ; /* first inited */
        pstHandle->dwRecvPkgCnt = 0 ;
    }


    for( i= 0; i< iAddrCnt; i++ )
    {
        int iMatched = 0;
        iSrcAddr = *a_piSrc ;
        iDstAddr = *a_piDst ;


        ptChannel = pstHandle->pastChannelSet[pstHandle->iRecvPos];
        TBUS_SELECT_CHANNEL(iMatched, ptChannel, iDstAddr, iSrcAddr);
        if (0 == iMatched)
        {
            pstHandle->iRecvPos = (pstHandle->iRecvPos + 1) % iAddrCnt ;
            pstHandle->dwRecvPkgCnt = 0 ;
            continue;
        }

        iRet = tbus_get_channel_pkgv(ptChannel, &pstHandle->stRecvHead, a_pvData, a_piDataLen) ;
        if ( TBUS_SUCCESS != iRet )
        {
            if ( TBUS_IS_SPECIFICAL_ADDRESS(*a_piSrc) ||    TBUS_IS_SPECIFICAL_ADDRESS(*a_piDst))
            {
                /* batch received, channel empty or error occured, try next channel */
                pstHandle->iRecvPos = (pstHandle->iRecvPos + 1) % iAddrCnt ;
                pstHandle->dwRecvPkgCnt = 0 ;
                continue;
            }else
            {
                break;
            }
        }/*if ( TBUS_SUCCESS != iRet )*/

        /*recv data successfully*/
        tbus_log(TLOG_PRIORITY_DEBUG,"recv one msg(len:%"PRIdPTR") from %s\n", *a_piDataLen, tbus_addr_ntoa(iSrcAddr));
        if (TBUS_FLAG_START_DYE_MSG & a_iFlag)
        {
            /*dye msg*/
            tbus_dye_pkg(&pstHandle->stRecvHead, NULL, TBUS_FLAG_START_DYE_MSG);
        }

        if(pstHandle->stRecvHead.bFlag & TBUS_HEAD_FLAG_TACE)
        {
            tbus_log_dyedpkg(&pstHandle->stRecvHead, "Recv");
        }

        /* not specified source and destination address, update current received position */
        if ( TBUS_IS_SPECIFICAL_ADDRESS(*a_piSrc) ||    TBUS_IS_SPECIFICAL_ADDRESS(*a_piDst))
        {
            pstHandle->dwRecvPkgCnt ++ ;
            if ( TBUS_MAX_RECV_CNT <= pstHandle->dwRecvPkgCnt )
            {
                pstHandle->iRecvPos = (pstHandle->iRecvPos + 1) % iAddrCnt ;
                pstHandle->dwRecvPkgCnt = 0 ;
            }
        }
        if ( TBUS_IS_SPECIFICAL_ADDRESS(*a_piSrc))
        {
            /* set return address */
            *a_piSrc = iSrcAddr ;
        }
        if (TBUS_IS_SPECIFICAL_ADDRESS(*a_piDst))
        {
            *a_piDst = iDstAddr ;
        }

        break ; /* ok return */
    }/*for( i= iCurPos; i< iAddrCnt; i++ )*/



    return iRet ;
}

TSF4G_API int tbus_peek_msg(IN const int a_iHandle,    INOUT TBUSADDR *a_piSrc,    INOUT TBUSADDR *a_piDst,
                            INOUT const char **a_ppvData,    OUT size_t *a_piDataLen,    IN const int a_iFlag)
{
    int iRet = TBUS_ERR_CHANNEL_EMPTY;
    TBUSADDR    iSrcAddr = 0;
    TBUSADDR    iDstAddr = 0;
    int i = 0 ;
    TBUSHANDLE *pstHandle = NULL ;
    TBUSCHANNEL *ptChannel = NULL ;
    int iAddrCnt;


    if ( (NULL == a_piSrc) || (NULL == a_piDst) || (NULL == a_ppvData) || (NULL == a_piDataLen))
    {
        tbus_log(TLOG_PRIORITY_ERROR,"parameter error, null parameter " ) ;
        return TBUS_ERR_ARG ;
    }
    pstHandle = TBUS_GET_HANDLE(a_iHandle);
    if (NULL == pstHandle)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"failed to get handle(%d)", a_iHandle);
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_HANDLE);
    }
    iAddrCnt = pstHandle->dwChannelCnt ;
    if ( 0 >= iAddrCnt )
    {
        tbus_log(TLOG_PRIORITY_ERROR,"no channel(count:%d) to recv data", iAddrCnt ) ;
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_NO_CHANNEL) ;
    }

    /*select position of channel which search channel to recv data from it */
    if ( 0 > pstHandle->iRecvPos )
    {
        pstHandle->iRecvPos = 0 ; /* first inited */
        pstHandle->dwRecvPkgCnt = 0 ;
    }


    for( i= 0; i< iAddrCnt; i++ )
    {
        int iMatched = 0;
        iSrcAddr = *a_piSrc ;
        iDstAddr = *a_piDst ;

        ptChannel = pstHandle->pastChannelSet[pstHandle->iRecvPos];
        TBUS_SELECT_CHANNEL(iMatched, ptChannel, iDstAddr, iSrcAddr);
        if (0 == iMatched)
        {
            pstHandle->iRecvPos = (pstHandle->iRecvPos + 1) % iAddrCnt ;
            pstHandle->dwRecvPkgCnt = 0 ;
            continue;
        }

        iRet = tbus_peek_channel_pkgv(ptChannel, &pstHandle->stRecvHead, a_ppvData, a_piDataLen) ;
        if ( TBUS_SUCCESS != iRet )
        {
            if ( TBUS_IS_SPECIFICAL_ADDRESS(*a_piSrc) ||    TBUS_IS_SPECIFICAL_ADDRESS(*a_piDst))
            {
                /* batch received, channel empty or error occured, try next channel */
                pstHandle->iRecvPos = (pstHandle->iRecvPos + 1) % iAddrCnt ;
                pstHandle->dwRecvPkgCnt = 0 ;
                continue;
            }else
            {
                break;
            }
        }/*if ( TBUS_SUCCESS != iRet )*/

        /*recv data successfully*/
        tbus_log(TLOG_PRIORITY_DEBUG,"recv one msg(len:%"PRIdPTR") from %s\n", *a_piDataLen, tbus_addr_ntoa(iSrcAddr));
        if (TBUS_FLAG_START_DYE_MSG & a_iFlag)
        {
            /*dye msg*/
            tbus_dye_pkg(&pstHandle->stRecvHead, NULL, TBUS_FLAG_START_DYE_MSG);
        }
        if(pstHandle->stRecvHead.bFlag & TBUS_HEAD_FLAG_TACE)
        {
            tbus_log_dyedpkg(&pstHandle->stRecvHead, "Recv");
        }

        /* not specified source and destination address, update current received position */
        if ( TBUS_IS_SPECIFICAL_ADDRESS(*a_piSrc) ||    TBUS_IS_SPECIFICAL_ADDRESS(*a_piDst))
        {
            pstHandle->dwRecvPkgCnt ++ ;
            if ( TBUS_MAX_RECV_CNT <= pstHandle->dwRecvPkgCnt )
            {
                pstHandle->iRecvPos = (pstHandle->iRecvPos + 1) % iAddrCnt ;
                pstHandle->dwRecvPkgCnt = 0 ;
            }
        }
        if ( TBUS_IS_SPECIFICAL_ADDRESS(*a_piSrc))
        {
            /* set return address */
            *a_piSrc = iSrcAddr ;
        }
        if (TBUS_IS_SPECIFICAL_ADDRESS(*a_piDst))
        {
            *a_piDst = iDstAddr ;
        }

        break ; /* ok return */
    }/*for( i= iCurPos; i< iAddrCnt; i++ )*/


    return iRet ;
}

int tbus_delete_msg(IN const int a_iHandle,    IN TBUSADDR a_iSrc,    INOUT TBUSADDR a_iDst)
{
    int iRet = TBUS_ERR_CHANNEL_EMPTY;
    int i = 0 ;
    TBUSHANDLE *pstHandle = NULL ;
    TBUSCHANNEL *ptChannel = NULL ;
    int iAddrCnt;

    pstHandle = TBUS_GET_HANDLE(a_iHandle);
    if (NULL == pstHandle)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"failed to get handle(%d)", a_iHandle);
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_HANDLE);
    }

    iAddrCnt = pstHandle->dwChannelCnt ;
    for( i= 0; i< iAddrCnt; i++ )
    {
        ptChannel = pstHandle->pastChannelSet[i];
        if ((TBUS_CHANNEL_LOCAL_ADDR(ptChannel) == a_iDst) &&
            (TBUS_CHANNEL_PEER_ADDR(ptChannel) == a_iSrc))
        {
            break;
        }
    }/*for( i= 0; i< iAddrCnt; i++ )*/
    if (i >= iAddrCnt)
    {
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_NO_CHANNEL_MATCHED);
    }

    iRet = tbus_delete_channel_headpkg(ptChannel);


    return iRet ;
}


int tbus_forward (IN const int a_iHandle,    INOUT TBUSADDR *a_piSrc,    INOUT TBUSADDR *a_piDst,
                  IN const void *a_pvData,    IN const size_t a_iDataLen,    IN const int a_iFlag)
{
    int iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_NO_CHANNEL_MATCHED);
    struct iovec astVectors[1];

    if ( (NULL == a_piSrc) || (NULL == a_piDst) || (NULL == a_pvData) || (0 >= a_iDataLen) )
    {
        tbus_log(TLOG_PRIORITY_ERROR,"parameter error,null paramter or invalid data len %"PRIdPTR"", a_iDataLen ) ;
        return TBUS_ERR_ARG ;
    }


    /*转发数据*/
    astVectors[0].iov_base = (void *)a_pvData;
    astVectors[0].iov_len = a_iDataLen;
    iRet = tbus_forwardv(a_iHandle, a_piSrc, a_piDst, &astVectors[0],1,a_iFlag);


    return iRet ;
}

int tbus_forwardv(IN const int a_iHandle,    INOUT TBUSADDR *a_piSrc,    INOUT TBUSADDR *a_piDst,
                            IN const struct iovec *a_ptVector,  IN const int a_iVecCnt,  IN const int a_iFlag)
{
    int iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_NO_CHANNEL_MATCHED);

    int    iAddrCnt = 0,
        i = 0;
    TBUSADDR iSrcAddr = 0,
        iDstAddr = 0,
        iRetSrc = 0,
        iRetDst = 0 ;
    TBUSHANDLE *pstHandle = NULL ;
    TBUSHEAD stHead;




    if ( (NULL == a_piSrc) || (NULL == a_piDst) || (NULL == a_ptVector) || (0 >= a_iVecCnt) )
    {
        tbus_log(TLOG_PRIORITY_ERROR,"parameter error,null paramter or invalid vector num %d", a_iVecCnt ) ;
        return TBUS_ERR_ARG ;
    }

    /*Get handle*/
    pstHandle = TBUS_GET_HANDLE(a_iHandle);
    if (NULL == pstHandle)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"failed to get handle(%d)", a_iHandle);
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_HANDLE);
    }
    iAddrCnt = pstHandle->dwChannelCnt ;
    if ( 0 >= iAddrCnt )
    {
        tbus_log(TLOG_PRIORITY_ERROR,"no channel(count:%d) to recv data", iAddrCnt ) ;
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_NO_CHANNEL) ;
    }

    tbus_init_head_for_forward(iRet, &stHead, a_iFlag, &pstHandle->stRecvHead);
    if (TBUS_ERR_IS_ERROR(iRet))
    {
        return iRet;
    }

    /*转发数据*/
    for ( i=0; i<iAddrCnt; i++ )
    {
        int iMatched = 0;
        LPTBUSCHANNEL pstChannel = pstHandle->pastChannelSet[i] ;
        iSrcAddr = *a_piSrc ;
        iDstAddr = *a_piDst ;

        TBUS_SELECT_CHANNEL(iMatched, pstChannel, iSrcAddr, iDstAddr);
        if (0 == iMatched)
        {
            continue ;
        }

        /* fill address */
        stHead.iDst = iDstAddr;
        stHead.iSrc = iSrcAddr;
        TBUS_SET_LAST_ROUTE(stHead, iSrcAddr);

        if (TBUS_CHANNEL_WITH_TIMESTAMP(pstChannel)
            || (TBUS_FLAG_WITH_TIMESTAMP & a_iFlag))
        {
            tbus_set_head_timestamp(&stHead, &pstHandle->stRecvHead,
                (a_iFlag & TBUS_FLAG_USE_GETTIMEOFDAY));
        }

        iRet = tbus_push_channel_pkgv(pstChannel, &stHead, a_ptVector, a_iVecCnt) ;
        if (iRet != TBUS_SUCCESS)
        {
            tbus_log(TLOG_PRIORITY_ERROR,"failed to push data to channel(%d), iRet:%d", i, iRet ) ;
            if ( (TBUS_ADDR_ALL == *a_piSrc) || (TBUS_ADDR_ALL == *a_piDst) )
            {
                continue ; /* batch sent mode, skipped individual error */
            }else
            {
                /* failed return, break here, not batch sent mode */
                break ;
            }
        }/*if (iRet != TBUS_SUCCESS)*/

        tbus_log(TLOG_PRIORITY_DEBUG,
            "Send data(len:%d) to channel %d,   distination address %s",
            stHead.iBodyLen, i, tbus_addr_ntoa(iDstAddr) ) ;
        if(stHead.bFlag & TBUS_HEAD_FLAG_TACE)
        {
            tbus_log_dyedpkg(&stHead, "Send(forward)");
        }
        iRet = TBUS_SUCCESS ;

        /* record first successful channel address */
        iRetSrc = iSrcAddr ;
        iRetDst = iDstAddr ;
        if ( (TBUS_ADDR_ALL != *a_piSrc) && (TBUS_ADDR_ALL != *a_piDst) )
        {
            /* break here, not batch sent mode */
            break ;
        }
    } /* end for ( i=0; i<iAddrCnt; i++ ) */

    /* set return address */
    if (0 != iRetSrc)
    {
        *a_piSrc = iRetSrc ;
    }
    if (0 != iRetDst)
    {
        *a_piDst = iRetDst ;
    }

    return iRet ;
}




int tbus_backward (IN const int a_iHandle,    INOUT TBUSADDR *a_piSrc,    INOUT TBUSADDR *a_piDst,
                   IN const void *a_pvData,    IN const size_t a_iDataLen,    IN const int a_iFlag)
{
    int iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_NO_CHANNEL_MATCHED);
    struct iovec astVectors[1];

    if ( (NULL == a_piSrc) || (NULL == a_piDst) || (NULL == a_pvData) || (0 >= a_iDataLen) )
    {
        tbus_log(TLOG_PRIORITY_ERROR,"parameter error,null parameter of invalid  len %"PRIdPTR"", a_iDataLen ) ;
        return TBUS_ERR_ARG ;
    }


    astVectors[0].iov_base = (void *)a_pvData;
    astVectors[0].iov_len = a_iDataLen;
    iRet = tbus_backwardv(a_iHandle, a_piSrc, a_piDst, &astVectors[0],1,a_iFlag);


    return iRet;
}
int tbus_backwardv (IN const int a_iHandle,    INOUT TBUSADDR *a_piSrc,    INOUT TBUSADDR *a_piDst,
                              IN const struct iovec *a_ptVector,  IN const int a_iVecCnt,     IN const int a_iFlag)
{
    int iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_NO_CHANNEL_MATCHED),
        iAddrCnt = 0,
        i = 0;
    TBUSADDR iSrcAddr = 0,
        iDstAddr = 0,
        iRetSrc = 0,
        iRetDst = 0 ;
    TBUSHANDLE *pstHandle = NULL ;
    TBUSHEAD stHead;


    if ( (NULL == a_piSrc) || (NULL == a_piDst) || (NULL == a_ptVector) || (0 >= a_iVecCnt) )
    {
        tbus_log(TLOG_PRIORITY_ERROR,"parameter error,null parameter of invalid vector num %d", a_iVecCnt ) ;
        return TBUS_ERR_ARG ;
    }


    pstHandle = TBUS_GET_HANDLE(a_iHandle);
    tbus_init_head_for_forward(iRet, &stHead, a_iFlag, &pstHandle->stRecvHead);
    if (TBUS_ERR_IS_ERROR(iRet))
    {
        return iRet;
    }
    if (NULL == pstHandle)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"failed to get handle(%d)", a_iHandle);
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_HANDLE);
    }
    iAddrCnt = pstHandle->dwChannelCnt ;
    if ( 0 >= iAddrCnt )
    {
        tbus_log(TLOG_PRIORITY_ERROR,"no channel(count:%d) to recv data", iAddrCnt ) ;
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_NO_CHANNEL) ;
    }


    tbus_init_head_for_backward(iRet, &stHead, a_iFlag, &pstHandle->stRecvHead, a_piDst);
    if (TBUS_ERR_IS_ERROR(iRet))
    {
        return iRet;
    }

    for ( i=0; i<iAddrCnt; i++ )
    {
        int iMatched = 0;
        LPTBUSCHANNEL pstChannel = pstHandle->pastChannelSet[i] ;

        iSrcAddr = *a_piSrc ;
        iDstAddr = *a_piDst ;
        TBUS_SELECT_CHANNEL(iMatched, pstChannel, iSrcAddr, iDstAddr);
        if (0 == iMatched)
        {
            continue ;
        }

        stHead.iDst = iDstAddr;
        stHead.iSrc = iSrcAddr;

        if (TBUS_CHANNEL_WITH_TIMESTAMP(pstChannel)
            || (TBUS_FLAG_WITH_TIMESTAMP & a_iFlag))
        {
            tbus_set_head_timestamp(&stHead, &pstHandle->stRecvHead,
                (a_iFlag & TBUS_FLAG_USE_GETTIMEOFDAY));
        }

        iRet = tbus_push_channel_pkgv(pstChannel, &stHead, a_ptVector, a_iVecCnt) ;
        if (iRet != TBUS_SUCCESS)
        {
            tbus_log(TLOG_PRIORITY_ERROR,"failed to push data to channel(%d), iRet:%d", i, iRet ) ;
            if ( (TBUS_ADDR_ALL == *a_piSrc) || (TBUS_ADDR_ALL == *a_piDst) )
            {
                continue ; /* batch sent mode, skipped individual error */
            }else
            {
                /* failed return, break here, not batch sent mode */
                break ;
            }
        }/*if (iRet != TBUS_SUCCESS)*/

        tbus_log(TLOG_PRIORITY_DEBUG,
            "Send data(len:%d) to channel %d,   distination address %s", stHead.iBodyLen, i, tbus_addr_ntoa(iDstAddr) ) ;
        if(stHead.bFlag & TBUS_HEAD_FLAG_TACE)
        {
            tbus_log_dyedpkg(&stHead, "Send(backward)");
        }
        iRet = TBUS_SUCCESS ;

        /* record first successful channel address */
        iRetSrc = iSrcAddr ;
        iRetDst = iDstAddr ;
        if ( (TBUS_ADDR_ALL != *a_piSrc) && (TBUS_ADDR_ALL != *a_piDst) )
        {
            /* break here, not batch sent mode */
            break ;
        }
    } /* end for ( i=0; i<iAddrCnt; i++ ) */

    /* set return address */
    if (0 != iRetSrc)
    {
        *a_piSrc = iRetSrc ;
    }
    if (0 != iRetDst)
    {
        *a_piDst = iRetDst ;
    }

    return iRet;
}


void tbus_delete ( INOUT int *a_piHandle )
{

    LPTBUSHANDLE pstTbusHandle;
    int i;

    if ( NULL == a_piHandle )
    {
        return  ; /* treat as deleted ok */
    }


    pstTbusHandle = TBUS_GET_HANDLE(*a_piHandle);
    if (NULL == pstTbusHandle)
    {
        return;
    }

    assert(0 < g_stBusGlobal.dwHandleCnt);

    /*close all channel handle with this bus handle*/
    pthread_mutex_lock(&g_stBusGlobal.tMutex);

    for (i = 0; i < (int)pstTbusHandle->dwChannelCnt; i++)
    {
        shmdt((void*)pstTbusHandle->pastChannelSet[i]->pstHead);
        free((void*)pstTbusHandle->pastChannelSet[i]);
    }
    free (g_stBusGlobal.pastTbusHandle[*a_piHandle]);
    g_stBusGlobal.pastTbusHandle[*a_piHandle] = NULL;

    g_stBusGlobal.dwHandleCnt--;

    *a_piHandle = 0 ;

    pthread_mutex_unlock(&g_stBusGlobal.tMutex);
}


void tbus_fini (void)
{
    unsigned int    i = 0 ;

    tbus_agent_api_destroy();

    if (g_stBusGlobal.dwInitStatus == TBUS_MODULE_NOT_INITED)
    {
        return;
    }

    g_stBusGlobal.dwInitStatus = TBUS_MODULE_NOT_INITED;
    /* release handle set */
    for ( i=1; i< g_stBusGlobal.dwHandleCnt; i++ )
    {
        int iHandle = i;
        if ( NULL != g_stBusGlobal.pastTbusHandle[i] )
        {
            tbus_delete(&iHandle) ;
        }
        g_stBusGlobal.pastTbusHandle[i] = NULL ;
    }

    shmdt((void*)g_stBusGlobal.pstGCIM);
    /* release route map structure */
    g_stBusGlobal.pstGCIM = NULL ;

    pthread_mutex_destroy(&g_stBusGlobal.tMutex);
#if defined (_WIN32) || defined (_WIN64)
    tbus_fini_security_attr();
#endif
}



int tbus_get_pkg_route ( IN const int a_iHandle, INOUT HEADROUTE *a_ptRouteVec, INOUT int *a_piCnt )
{
    int iRet = TBUS_SUCCESS,
        iHeadCnt = 0,
        i = 0 ;
    TBUSHEAD *ptHead = NULL ;
    LPTBUSHANDLE pstHandle;

    if ( (NULL == a_ptRouteVec) || (NULL == a_piCnt) || (0 >= *a_piCnt) )
    {
        tbus_log(TLOG_PRIORITY_ERROR,"parameter error, null paramter or invalid vector coount%d", *a_piCnt) ;
        return  TBUS_ERR_ARG ;
    }

    pstHandle = TBUS_GET_HANDLE(a_iHandle);
    if (NULL == pstHandle)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"failed to get handle(%d)", a_iHandle);
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_HANDLE);
    }

    ptHead = &pstHandle->stRecvHead ;
    iHeadCnt = ptHead->stExtHead.stDataHead.bRoute;
    if ( 0 >= iHeadCnt )
    {
        tbus_log(TLOG_PRIORITY_ERROR,"head cnt error %i", iHeadCnt ) ;
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_ROUTE);
    }else if ( *a_piCnt < iHeadCnt )
    {
        tbus_log(TLOG_PRIORITY_ERROR,"vector not large enough %i < %i", *a_piCnt, iHeadCnt ) ;
        return  TBUS_ERR_ARG ;
    }

    for ( i=0; i<iHeadCnt; i++ )
    {
        a_ptRouteVec[i].iSrc = ptHead->stExtHead.stDataHead.routeAddr[i];
    }
    *a_piCnt = i ;

    pthread_mutex_destroy(&g_stBusGlobal.tMutex);
    return iRet ;
}


int tbus_set_pkg_route ( IN const int a_iHandle, IN const HEADROUTE *a_ptRouteVec, IN const int a_iCnt )
{
    int iRet = TBUS_SUCCESS;

    int    i = 0 ;
    TBUSHEAD *ptHead = NULL ;
    LPTBUSHANDLE pstHandle;

    if ( (NULL == a_ptRouteVec) || (0 >= a_iCnt) )
    {
        tbus_log(TLOG_PRIORITY_ERROR,"tbus_set_pkg_route, parameter error, cnt %i", a_iCnt ) ;
        return TBUS_ERR_ARG ;
    }

    pstHandle = TBUS_GET_HANDLE(a_iHandle);
    if (NULL == pstHandle)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"failed to get handle(%d)", a_iHandle);
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_HANDLE);
    }
    if (TBUS_FORWARD_MAX_ROUTE <= a_iCnt)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"the route number(%d) is beyond the max limit(%d)",
                 a_iCnt, TBUS_FORWARD_MAX_ROUTE ) ;
        return TBUS_ERR_ARG;
    }

    ptHead = &pstHandle->stRecvHead ;
    for ( i=0; i<a_iCnt; i++ )
    {
        ptHead->stExtHead.stDataHead.routeAddr[i] = a_ptRouteVec[i].iSrc ;
    }
    ptHead->stExtHead.stDataHead.bRoute = i;

    return iRet ;
}




int tbus_peer_ctrl ( IN const int a_iHandle, IN const TBUSADDR a_iDstAddr, IN const int a_iMode, IN const int a_iBatch )
{
    int iRet = TBUS_SUCCESS,
        i = 0,
        iAddrCnt = 0 ;
    TBUSCHANNEL *pstChannel = NULL ;
    LPTBUSHANDLE pstHandle;

    pstHandle = TBUS_GET_HANDLE(a_iHandle);
    if (NULL == pstHandle)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"invalid handle %d", a_iHandle);
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_HANDLE);
    }


    iAddrCnt = pstHandle->dwChannelCnt ;
    if ( 0 >= iAddrCnt )
    {
        tbus_log(TLOG_PRIORITY_ERROR,"tbus_peer_ctrl(), no available address" ) ;
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_NO_CHANNEL) ;
    }

    for ( i=0; i<iAddrCnt; i++ )
    {
        pstChannel = pstHandle->pastChannelSet[i] ;

        if (a_iDstAddr != pstChannel->pstHead->astAddr[pstChannel->iSendSide])
        {
            continue;
        }

        if ( TBUS_MODE_ENABLE == a_iMode )
        {
            //如果这个通道被其他句柄 互斥式地占有的话，不能enable;
            TBUSADDR iSrc = pstChannel->pstHead->astAddr[pstChannel->iRecvSide];
            if (tbus_owned_exclusive_by_others_i(iSrc,a_iDstAddr, pstHandle))
            {
                char szAddr1[TBUS_MAX_ADDR_STRING_LEN];
                char szAddr2[TBUS_MAX_ADDR_STRING_LEN];
                tbus_log(TLOG_PRIORITY_DEBUG,"the channel(%s <--> %s) is own exclusively by other handle, so cannot be enable",
                         tbus_addr_ntop(a_iDstAddr, szAddr1, sizeof(szAddr1)),
                         tbus_addr_ntop(iSrc, szAddr2, sizeof(szAddr2)));
                iRet = 1;
            }else if ( TBUS_HANDLE_IS_EXCLUSIVE(pstHandle) && !TBUS_CHANNEL_IS_CONNECTED(pstChannel))
            {
                //否则 如果本句柄是互斥模式的，且这个通道没有调用connect，不能enable
                char szAddr1[TBUS_MAX_ADDR_STRING_LEN];
                char szAddr2[TBUS_MAX_ADDR_STRING_LEN];
                tbus_log(TLOG_PRIORITY_DEBUG,"the handle is exclusive channel mode, the channel(%s <--> %s) is not connected, so cannot be enable",
                         tbus_addr_ntop(a_iDstAddr, szAddr1, sizeof(szAddr1)),
                         tbus_addr_ntop(iSrc, szAddr2, sizeof(szAddr2)));
                iRet = 1;
            }else
            {
                TBUS_CHANNEL_CLR_DISABLE_BY_CTRL(pstChannel);
                TBUS_CHANNEL_SET_ENABLE(pstChannel);
            }

            tbus_log(TLOG_PRIORITY_DEBUG,"enable channel which peer address is %s", tbus_addr_ntoa(a_iDstAddr));
        }else if ( TBUS_MODE_DISABLE == a_iMode )
        {
            TBUS_CHANNEL_SET_DISABLE_BY_CTRL(pstChannel);
            TBUS_CHANNEL_CLR_ENABLE(pstChannel);
            tbus_log(TLOG_PRIORITY_DEBUG,"disable channel which peer address is %s", tbus_addr_ntoa(a_iDstAddr));
        }
        if ( TBUS_PEER_CTRL_ONE == a_iBatch )
        {
            break ;
        }

    }/*for ( i=0; i<iAddrCnt; i++ )*/

    return iRet ;

}



int tbus_save_bus_head ( IN const int a_iHandle )
{
    LPTBUSHANDLE pstHandle;

    pstHandle = TBUS_GET_HANDLE(a_iHandle);
    if (NULL == pstHandle)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"invalid handle %d", a_iHandle);
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_HANDLE);
    }

    memcpy(&pstHandle->stHeadBak, &pstHandle->stRecvHead, sizeof(TBUSHEAD));

    return TBUS_SUCCESS ;
}


int tbus_restore_bus_head ( IN const int a_iHandle )
{
    LPTBUSHANDLE pstHandle;

    pstHandle = TBUS_GET_HANDLE(a_iHandle);
    if (NULL == pstHandle)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"invalid handle %d", a_iHandle);
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_HANDLE);
    }

    memcpy(&pstHandle->stRecvHead, &pstHandle->stHeadBak, sizeof(TBUSHEAD));

    return TBUS_SUCCESS ;
}


int tbus_get_bus_head ( IN const int a_iHandle, INOUT void *a_pvBuffer, INOUT int *a_piLen )
{
    LPTBUSHANDLE pstHandle;
    int iRet = TBUS_SUCCESS;

    if ( (NULL == a_pvBuffer) || (NULL == a_piLen) || (0 >= *a_piLen) )
    {
        tbus_log(TLOG_PRIORITY_ERROR,"parameter error, null paramter or invalid len(%d)", *a_piLen) ;
        iRet = TBUS_ERR_ARG ;
        return iRet ;
    }

    pstHandle = TBUS_GET_HANDLE(a_iHandle);
    if (NULL == pstHandle)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"invalid handle %d", a_iHandle);
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_HANDLE);
    }

    iRet = tbus_encode_head(&pstHandle->stRecvHead, a_pvBuffer, a_piLen, 0);

    return iRet;
}


int tbus_set_bus_head( IN const int a_iHandle, IN const void *a_pvBuffer, IN const int a_iLen )
{
    int iRet = TBUS_SUCCESS ;
    TBUSHEAD stHead;
    LPTBUSHANDLE pstHandle;

    if ( (NULL == a_pvBuffer) || (0 >= a_iLen) )
    {
        tbus_log(TLOG_PRIORITY_ERROR,"parameter error, len %d", a_iLen ) ;
        return TBUS_ERR_ARG ;
    }

    pstHandle = TBUS_GET_HANDLE(a_iHandle);
    if (NULL == pstHandle)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"invalid handle %d", a_iHandle);
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_HANDLE);
    }

    iRet = tbus_decode_head(&stHead, (char *)a_pvBuffer, a_iLen, 0);
    if (TBUS_SUCCESS == iRet)
    {
        memcpy(&pstHandle->stRecvHead, &stHead, sizeof(TBUSHEAD));
    }else
    {
        memset(&pstHandle->stRecvHead, 0, sizeof(TBUSHEAD));
    }

    return iRet ;
}

int tbus_get_dst_list ( IN const int a_iHandle, INOUT unsigned int *a_piDstList, INOUT int *a_piVecCnt )
{

    int    iAddrCnt = 0,
           iTotal = 0,
           iVecCnt = 0,
           i = 0 ;
    TBUSHANDLE *pstHandle = NULL ;

    if ((NULL == a_piDstList)||(NULL == a_piVecCnt))
    {
        tbus_log(TLOG_PRIORITY_ERROR,"invalid param: a_piDstList: %p a_piVecCnt: %p",
                 a_piDstList, a_piVecCnt);
        return TBUS_ERR_ARG;
    }

    iTotal = *a_piVecCnt ;
    *a_piVecCnt = 0 ; /* reset */

    pstHandle = TBUS_GET_HANDLE(a_iHandle);
    if (NULL == pstHandle)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"failed to get handle(%d)", a_iHandle);
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_HANDLE);
    }
    iAddrCnt = pstHandle->dwChannelCnt ;
    if ( 0 >= iAddrCnt )
    {
        tbus_log(TLOG_PRIORITY_ERROR,"no channel(count:%d) to send data", iAddrCnt ) ;
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_NO_CHANNEL) ;
    }

    if ( iAddrCnt > iTotal )
    {
        tbus_log(TLOG_PRIORITY_ERROR,"dst list exceed %i > %i", iTotal, iTotal ) ;
        return TBUS_ERR_ARG ;
    }

    for ( i=0; i<iAddrCnt; i++ )
    {
        LPTBUSCHANNEL pstChannel = pstHandle->pastChannelSet[i];
        if ( !TBUS_CHANNEL_IS_ENABLE(pstChannel) )
        {
            tbus_log(TLOG_PRIORITY_TRACE,"tbus_get_dst_list, address info disable %i", i ) ;
            continue ;
        }



        a_piDstList[iVecCnt] = pstChannel->pstHead->astAddr[pstChannel->iSendSide] ;
        iVecCnt ++ ;
    }

    *a_piVecCnt = iVecCnt ;


    return TBUS_SUCCESS ;
}



int tbus_connect ( IN const int a_iHandle, IN const TBUSADDR a_iDstAddr )
{
    char szTmpIP[128];
    STRNCPY(szTmpIP, tbus_addr_ntoa(a_iDstAddr), sizeof(szTmpIP));
    return tbus_connect_by_str ( a_iHandle, &szTmpIP[0]) ;
}



int tbus_generate_counterfeit_pkg(LPTBUSHEAD a_pstHead, int *a_piPkgLen)
{
    int iRet = 0;
    TBUSHEAD stCounterfeit;

    assert(NULL != a_pstHead);
    assert(NULL != a_piPkgLen);

    /*为了方便tbus处理，当消息通道的尾部不够保存一个完整消息时，则尝试放置一个
      伪造的假消息占满深入空间。此假消息的判断特征：
      1.flag字段的值设置为TBUS_HEAD_FLAG_COUNTERFEIT_DATA
      2.BodyLen 字段设置为0
      */
    memset(&stCounterfeit,0, sizeof(stCounterfeit));
    stCounterfeit.bCmd = TBUS_HEAD_CMD_TRANSFER_DATA;
    stCounterfeit.bFlag = TBUS_HEAD_FLAG_COUNTERFEIT_DATA;
    stCounterfeit.bVer = TDR_METALIB_TBUSHEAD_VERSION;

    *a_piPkgLen = sizeof(TBUSHEAD);
    iRet = tbus_encode_head(&stCounterfeit, (char *)a_pstHead, a_piPkgLen, 0);
    if (0 != iRet)
    {
        tbus_log(TLOG_PRIORITY_ERROR, "failed to generate counterfeit pkg:%s", tbus_error_string(iRet));
    }else
    {
        tbus_log(TLOG_PRIORITY_INFO, "generate counterfeit pkg(Len:%d) successfully:", *a_piPkgLen);
        tbus_log_data(TLOG_PRIORITY_INFO, g_stBusGlobal.pstHeadMeta, &stCounterfeit, sizeof(TBUSHEAD));
    }

    return iRet;
}

int tbus_global_init(void)
{
    int iRet = TBUS_SUCCESS;
    LPTDRMETALIB pstLib;
    pthread_mutexattr_t attr;

    if (NULL != g_stBusGlobal.pstHeadMeta)
    {
        return TBUS_SUCCESS ;
    }

    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&g_stBusGlobal.tMutex, &attr);

    pstLib = (LPTDRMETALIB)(void*)g_szMetalib_TbusHead;
    g_stBusGlobal.pstHeadMeta = tdr_get_meta_by_name(pstLib, tdr_get_metalib_name(pstLib));
    if (NULL == g_stBusGlobal.pstHeadMeta)
    {
        printf("failed to get meta of tbus head by name %s", tdr_get_metalib_name(pstLib));
        iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_GET_HEAD_META);

    }


    /*构造伪造数据包*/
    if ((0 == iRet) && (0 >= g_stBusGlobal.iCounterfeitPkgLen))
    {
        iRet = tbus_generate_counterfeit_pkg(&g_stBusGlobal.stCounterfeitPkg, &g_stBusGlobal.iCounterfeitPkgLen);
    }

#if defined (_WIN32) || defined (_WIN64)
    tbus_init_security_attr();
#endif



    return iRet ;
}

int tbus_init_ex(IN const char *a_pszShmkey, IN int a_iFlag)
{

    int iRet = TBUS_SUCCESS;
    int iTempBussID =0;
    LPTBUSSHMGCIMHEAD pstHead;

    /*if it has been initialized, just return*/
    if ( TBUS_MODULE_INITED == g_stBusGlobal.dwInitStatus )
    {
        return TBUS_SUCCESS ;
    }

    /* init global handle set */
    iTempBussID = g_stBusGlobal.iBussId;
    memset ( &g_stBusGlobal, 0, sizeof(g_stBusGlobal) ) ;
    g_stBusGlobal.iBussId = iTempBussID;
    tbus_register_bussid();

    /*初始化全局共享信息*/
    iRet = tbus_global_init();
    /* init log system*/
    if (0 == iRet)
    {
        iRet = tbus_init_log();
        if (0 != iRet)
        {
            printf("failed to init log system\n");
        }
    }

    /* get GCIM */
    if (0 == iRet)
    {
        tbus_set_logpriority(TLOG_PRIORITY_DEBUG);
        iRet = tbus_get_gcimshm(&g_stBusGlobal.pstGCIM, a_pszShmkey, g_stBusGlobal.iBussId,
                                0, TBUS_ATTACH_SHM_TIMEOUT, NULL);
        if ( 0 == iRet )
        {
            pstHead = &g_stBusGlobal.pstGCIM->stHead;
            g_stBusGlobal.dwGCIMVersion = pstHead->dwVersion;
        }else
        {
            tbus_log(TLOG_PRIORITY_ERROR,"tbus_get_gcimshm() failed, iret=%x, %s\n",
                     iRet, tbus_error_string(iRet)) ;
        }/*if ( 0 == iRet )*/
    }/*if (0 == iRet)*/


    if (0 == iRet)
    {
        g_stBusGlobal.dwInitStatus = TBUS_MODULE_INITED ;
        g_stBusGlobal.iFlag = a_iFlag;
        tbus_log(TLOG_PRIORITY_DEBUG,"tbus_init_ex successfully, shmkey(%d) size(%d) channelnum(%d)",
                 pstHead->dwShmKey, pstHead->dwShmSize, pstHead->dwUsedCnt);

        tbus_set_logpriority(TLOG_PRIORITY_ERROR);
    }else
    {
        tbus_fini() ;
    }

    return iRet ;
}


int tbus_set_bussid(IN const int a_iBussId )
{
    g_stBusGlobal.iBussId = a_iBussId;

    return 0;
}

// 设置tbus句柄为exlusive模式, 从而指定句柄只接收tbus_connect指定的通道
int tbus_handle_set_exclusive(LPTBUSHANDLE a_pstHandle)
{
    int iRet = TBUS_SUCCESS;
    int iChannelCount = 0 ;
    int i = 0;

    if (NULL == a_pstHandle)
    {
        tlog_log(g_pstBusLogCat, TLOG_PRIORITY_ERROR, 0, 0, "parameter a_pstHandle is null");
        return TBUS_ERR_ARG;
    }

    if (TBUS_HANDLE_IS_EXCLUSIVE(a_pstHandle))
    {
        tlog_log(g_pstBusLogCat, TLOG_PRIORITY_INFO, 0, 0, "exclusive mode has been set");
        return iRet;
    }
    TBUS_HANDLE_SET_EXCLUSIVE(a_pstHandle);

    //disable all channels
    iChannelCount = a_pstHandle->dwChannelCnt ;

    for ( i=0; i<iChannelCount; i++ )
    {
        LPTBUSCHANNEL pstChannel = a_pstHandle->pastChannelSet[i] ;
        if (!TBUS_CHANNEL_IS_CONNECTED(pstChannel))
        {
            TBUS_CHANNEL_CLR_ENABLE(pstChannel);
        }
    }/*for ( i=0; i<iChannelCount; i++ )*/


    return iRet;
}

/**
  @brief 设置tbus 选项
  根据选项名字a_iOptionName 和相应值设置a_pvOptionValue
  @param[in] a_iOptionName	此参数指定需要设置的tbus选项.

  @param[in] a_pvOptionValue 相应选项的具体设置值，选项不同，其结构也不同：
  -	目前不支持任何选项.
  @param[in] a_dwOptionLen	指定选项值的长度，其具体值由a_pvOptionValue数据类型决定。
  @deprecated 目前已经不支持全局的通道互斥，TBUS_OPT_EXCLUSIVE_CHANNELES必须通过 tbus_set_handle_opt来完成。
  @see tbus_set_handle_opt
  @retval 0	success
  @retval <0	failed
  */
int tbus_set_opt(IN int a_iOptionName, IN const void *a_pvOptionValue, IN unsigned int a_dwOptionLen)
{
    int iRet = 0;

    TOS_UNUSED_ARG(a_pvOptionValue);
    TOS_UNUSED_ARG(a_dwOptionLen);
    switch(a_iOptionName)
    {
        default:
            tbus_log(TLOG_PRIORITY_ERROR, "unsuppoted option name(%d)", a_iOptionName);
            iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_UNSUPPORTED_OPTION);
            break;
    }/*switch(a_iOptionName)*/

    return 0;
}

/**
  @brief 获取tbus 选项
  根据选项名字a_iOptionName 和相应值设置a_pvOptionValue
  @param[in] a_iOptionName	此参数指定需要设置的tbus选项，目前支持的选项有：
  -	目前不支持任何选项.

  @param[in] a_pvOptionValue 相应选项的具体设置值，选项不同，其结构也不同：
  -	目前不支持任何选项.
  @deprecated 目前已经不支持全局的通道互斥，TBUS_OPT_EXCLUSIVE_CHANNELES必须通过 tbus_set_handle_opt来完成。
  @see tbus_set_handle_opt
  @param[in,out] a_pdwOptionLen	获取a_pvOptionValue值的具体长度，如果a_pdwOptionLen所指的长度小于实际需要的长度，则处理
  失败。此参数输入输出约定如下：
  -	输入	指定a_pvOptionValue所指缓冲区的大小
  -	输出	获取a_pvOptionValue获取数据的实际长度。

  @retval 0	success
  @retval <0	failed
  */
int tbus_get_opt(IN int a_iOptionName, IN const void* a_pvOptionValue, INOUT unsigned int* a_pdwOptionLen)
{
    int iRet = 0;

    TOS_UNUSED_ARG(a_pvOptionValue);
    TOS_UNUSED_ARG(a_pdwOptionLen);
    switch(a_iOptionName)
    {
        default:
            tbus_log(TLOG_PRIORITY_ERROR, "unsuppoted option name(%d)", a_iOptionName);
            iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_UNSUPPORTED_OPTION);
            break;
    }/*switch(a_iOptionName)*/

    return 0;
}

/**
  @brief 设置tbus通道选项
  根据选项名字a_iOptionName 和相应值设置a_pvOptionValue
  @param[in] a_pstHandle      此参数指定tbus句柄，句柄可由tbus_new获取
  @param[in] a_iOptionName    此参数指定需要设置的tbus句柄选项，目前支持的选项有：
  -    TBUS_HANDLE_OPT_NAME_EXCLUSIVE_CHANNELS    互斥通道选项。如果此选项被打开，则通过tbus_connect指定的channel只能属于a_pstHandle, a_pstHandle也只能通过tbus_connect添加通道
  -    TBUS_HANDLE_OPT_NAME_CHANNELS_TIMESTAMP    时间戳选项。如果些选项被打开，则此句柄下所有通道上发送或中转的消息都携带时间戳值
  @param[in] a_pvOptionValue 相应选项的具体设置值，选项不同，其结构也不同：
  -    TBUS_HANDLE_OPT_NAME_EXCLUSIVE_CHANNELS 此选项值的数据类型为int型boolean选项，即其值只能为0或1。其值为1表示打开
  互斥通道选项。其值为0，则忽略此调用，即互斥通道选项一旦打开就不支持动态关闭。
  -    TBUS_HANDLE_OPT_NAME_CHANNELS_TIMESTAMP 此选项值的数据类型为int型boolean选项，即其值只能为0或1。其值为1表示打开
  时间戳选项。其值为0，表示关闭时间戳选项。
  @param[in] a_dwOptionLen    指定选项值的长度，其具体值由a_pvOptionValue数据类型决定。

  @retval 0    success
  @retval <0    failed
  */
int tbus_set_handle_opt(IN int a_iHandle, IN TBUSHANDLEOPTIONAME a_iOptionName,
                        IN const void *a_pvOptionValue, IN unsigned int a_dwOptionLen)
{
    int iRet = 0;
    LPTBUSHANDLE pstHandle = TBUS_GET_HANDLE(a_iHandle);

    if (NULL == pstHandle)
    {
        tbus_log(TLOG_PRIORITY_ERROR, "invalid handle:%d", a_iHandle);
        return TBUS_ERR_ARG;
    }

    switch(a_iOptionName)
    {
        case TBUS_HANDLE_OPT_NAME_EXCLUSIVE_CHANNELS:
            {
                if (sizeof(int) > a_dwOptionLen)
                {
                    iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_OPTION_VALUE);
                    break;
                }
                if (NULL == a_pvOptionValue)
                {
                    iRet = TBUS_ERR_ARG;
                    break;
                }
                if (0 != *(int *)a_pvOptionValue)
                {
                    iRet = tbus_handle_set_exclusive(pstHandle);
                }
                break;
            }
        case TBUS_HANDLE_OPT_NAME_CHANNELS_TIMESTAMP:
            {
                if (sizeof(int) > a_dwOptionLen)
                {
                    iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_OPTION_VALUE);
                    break;
                }
                if (NULL == a_pvOptionValue)
                {
                    iRet = TBUS_ERR_ARG;
                    break;
                }
                if (0 != *(int *)a_pvOptionValue)
                {
                    int i = 0;
                    for (i = 0; i < pstHandle->dwChannelCnt; i++)
                    {
                        TBUS_CHANNEL_ENABLE_TIMESTAMP(pstHandle->pastChannelSet[i]);
                    }
                    TBUS_HANDLE_ENABLE_TIMESTAMP(pstHandle);
                } else
                {
                    int i = 0;
                    for (i = 0; i < pstHandle->dwChannelCnt; i++)
                    {
                        TBUS_CHANNEL_DISABLE_TIMESTAMP(pstHandle->pastChannelSet[i]);
                    }
                    TBUS_HANDLE_DISABLE_TIMESTAMP(pstHandle);
                }
                break;
                break;
            }
        default:
            tbus_log(TLOG_PRIORITY_ERROR, "unsuppoted option name(%d)", a_iOptionName);
            iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_UNSUPPORTED_OPTION);
            break;
    }/*switch(a_iOptionName)*/

    return iRet;
}

/**
  @brief 获取tbus句柄选项
  @param[in] a_pstHandle      指定tbus句柄，句柄可由tbus_new获取
  @param[in] a_iOptionName    此参数指定需要设置的tbus句柄选项，目前支持的选项有：
  -    TBUS_HANDLE_OPT_NAME_EXCLUSIVE_CHANNELS    互斥通道选项
  如果此选项被打开，则通过tbus_connect指定的channel只能属于a_pstChannel, a_pstChannel也只能添加由tbus_connect指定的通道
  -    TBUS_HANDLE_OPT_NAME_CHANNELS_TIMESTAMP    时间戳选项
  如果此选项被打开，则此句柄在的通道上发送或中转的消息都携带时间戳值

  @param[in] a_pvOptionValue 相应选项的具体设置值，选项不同，其结构也不同：
  -    TBUS_OPT_EXCLUSIVE_CHANNELES 此选项值的数据类型为int型boolean选项，即其值只能为0或1。其值为1表示
  互斥通道选项处在打开状态。其值为0，表示互斥通道选项处在关闭状态。
  @param[in,out] a_pdwOptionLen    获取a_pvOptionValue值的具体长度，如果a_pdwOptionLen所指的长度小于实际需要的长度，则处理
  失败。此参数输入输出约定如下：
  -    输入    指定a_pvOptionValue所指缓冲区的大小
  -    输出    获取a_pvOptionValue获取数据的实际长度。

  @retval 0    success
  @retval <0    failed
  */
int tbus_get_handle_opt(IN int a_iHandle, IN int a_iOptionName,
                        IN const void* a_pvOptionValue, INOUT unsigned int* a_pdwOptionLen)
{
    int iRet = 0;
    LPTBUSHANDLE pstHandle = TBUS_GET_HANDLE(a_iHandle);

    if (NULL == pstHandle)
    {
        tbus_log(TLOG_PRIORITY_ERROR, "invalid handle:%d", a_iHandle);
        return TBUS_ERR_ARG;
    }

    switch(a_iOptionName)
    {
        case TBUS_HANDLE_OPT_NAME_EXCLUSIVE_CHANNELS:
            {
                if (sizeof(int) > *a_pdwOptionLen)
                {
                    iRet = TBUS_ERR_ARG;
                    break;
                }
                if (NULL == a_pvOptionValue)
                {
                    iRet = TBUS_ERR_ARG;
                    break;
                }
                if (TBUS_HANDLE_IS_EXCLUSIVE(pstHandle))
                {
                    *(int *)a_pvOptionValue = 1;
                }else
                {
                    *(int *)a_pvOptionValue = 0;
                }
                break;
            }
        case TBUS_HANDLE_OPT_NAME_CHANNELS_TIMESTAMP:
            {
                if (sizeof(int) > *a_pdwOptionLen)
                {
                    iRet = TBUS_ERR_ARG;
                    break;
                }
                if (NULL == a_pvOptionValue)
                {
                    iRet = TBUS_ERR_ARG;
                    break;
                }
                if (TBUS_HANDLE_WITH_TIMESTAMP(pstHandle))
                {
                    *(int *)a_pvOptionValue = 1;
                }else
                {
                    *(int *)a_pvOptionValue = 0;
                }
                break;
            }
        default:
            tbus_log(TLOG_PRIORITY_ERROR, "unsuppoted option name(%d)", a_iOptionName);
            iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_UNSUPPORTED_OPTION);
            break;
    }/*switch(a_iOptionName)*/

    return iRet;
}

//除指定的a_iExcludeHandle句柄外，将其他句柄下指定的通道(a_iSrc,a_iDst)屏蔽掉
//各句柄通道的互斥访问由调用者保证
int tbus_disable_channel_i(TBUSADDR a_iSrc, TBUSADDR a_iDst, LPTBUSHANDLE a_pstExcludeHandle)
{
    unsigned int i,j;
    TBUSHANDLE *pstHandle;

    if (NULL == a_pstExcludeHandle)
    {
        tlog_log(g_pstBusLogCat, TLOG_PRIORITY_ERROR, 0, 0, "parameter a_pstHandle is null");
        return TBUS_ERR_ARG;
    }

    if (!TBUS_HANDLE_IS_EXCLUSIVE(a_pstExcludeHandle))
    {
        return 0;
    }

    for (i = 0; i < g_stBusGlobal.dwHandleCnt; i++)
    {
        pstHandle = g_stBusGlobal.pastTbusHandle[i];
        if (NULL == pstHandle)
        {
            continue;
        }
        if (pstHandle == a_pstExcludeHandle)
        {
            continue;
        }


        for (j = 0; j < pstHandle->dwChannelCnt; j++)
        {
            LPTBUSCHANNEL pstChannel = pstHandle->pastChannelSet[j];
            assert(NULL != pstChannel);

            if ((pstChannel->pstHead->astAddr[pstChannel->iRecvSide] != a_iSrc) ||
                (pstChannel->pstHead->astAddr[pstChannel->iSendSide] != a_iDst))
            {
                continue;
            }

            tbus_log(TLOG_PRIORITY_INFO, "the channel(src:%u dst:%u) is connected by the handle(%p), "
                     "so disable the same channel in the handle(%d)",
                     a_iSrc, a_iDst, a_pstExcludeHandle, i);
            TBUS_CHANNEL_CLR_ENABLE(pstChannel);
            break;
        }/*for (j = 0; j < pstHandle->dwChannelCnt; j++)*/

    }/*for (i = 0; i < g_stBusGlobal.dwHandleCnt; i++)*/


    return 0;
}

//检查指定的通道是否已经被其他句柄调用tbus_connect指定，如果是则返回非零值
int tbus_owned_exclusive_by_others_i(TBUSADDR    a_iSrc, TBUSADDR a_iDst, LPTBUSHANDLE a_pstExcludeHandle)
{
    unsigned int i,j;
    TBUSHANDLE *pstHandle;

    for (i = 0; i < g_stBusGlobal.dwHandleCnt; i++)
    {
        pstHandle = g_stBusGlobal.pastTbusHandle[i];
        if (NULL == pstHandle)
        {
            continue;
        }
        if (pstHandle == a_pstExcludeHandle)
        {
            continue;
        }

        if (!TBUS_HANDLE_IS_EXCLUSIVE(pstHandle))
        {
            continue;
        }
        for (j = 0; j < pstHandle->dwChannelCnt; j++)
        {
            LPTBUSCHANNEL pstChannel = pstHandle->pastChannelSet[j];
            assert(NULL != pstChannel);

            if ((pstChannel->pstHead->astAddr[pstChannel->iRecvSide] != a_iSrc) ||
                (pstChannel->pstHead->astAddr[pstChannel->iSendSide] != a_iDst))
            {
                continue;
            }

            if (TBUS_CHANNEL_IS_CONNECTED(pstChannel) )
            {
                return 1;
            }
        }/*for (j = 0; j < pstHandle->dwChannelCnt; j++)*/

    }/*for (i = 0; i < g_stBusGlobal.dwHandleCnt; i++)*/


    return 0;
}

int tbus_initialize (IN const char *a_pszShmkey, IN int a_iBussID)
{
    int iRet = 0;

    tbus_set_bussid(a_iBussID);

    iRet = tbus_init_ex(a_pszShmkey, 0);

    return iRet;
}

