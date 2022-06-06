/** @file $RCSfile: tbus_channel.c,v $
  general description of this module
  $Id: tbus_channel.c,v 1.32 2009-09-16 11:55:25 jacky Exp $
  @author $Author: jacky $
  @date $Date: 2009-09-16 11:55:25 $
  @version $Revision: 1.32 $
  @note Editor: Vim 6.3, Gcc 4.0.2, tab=4
  @note Platform: Linux
  */

#include "pal/pal.h"
#include "tbus/tbus.h"
#include "tbus_misc.h"
#include "tbus_dyemsg.h"
#include "tbus_config_mng.h"
#include "tbus_head.h"
#include "tbus_kernel.h"
#include "tbus_log.h"
#include "tbus_channel.h"

#if defined (_WIN32) || defined (_WIN64)
#include "tbus_mmap_for_win32.h"
#endif

#define TBUS_APP_CONTEXT_LEN 32

extern LPTLOGCATEGORYINST	g_pstBusLogCat;

extern int tbus_global_init();

extern int tbus_get_attached_num_i(HANDLE a_iShmID, int* a_piNum);

void tbus_channel_mode_fini(void);

static int tbus_process_heartbeat_msg(LPTBUSCHANNEL a_pstChannel, LPTBUSHEAD a_pstHead);

extern void tbus_init_channel_head_i(CHANNELHEAD *pstHead, LPTBUSSHMCHANNELCNF a_pstShmChl, int iAlign);

static int tbus_check_channel_param_i(LPTBUSSHMCHANNELCNF a_pstCnf,LPTBUSCHANNEL a_pstChannel, IN size_t iShmSize);

static int tbus_init_process_state(IN LPTBUSHANDLE a_pstHandle, IN LPTBUSCHANNEL a_pstChannel, IN TBUSADDR a_iPeerAddr);

int tbus_send_heartbeat_msg_i(IN LPTBUSCHANNEL *a_ppstChannel, IN int a_iChannleCnt,
                              IN TBUSADDR a_iPeerAddr, IN int a_iFlag, IN int a_iMsgType);

static int tbus_attach_channel_to_mem_i(IN TBUSCHANNEL *a_ptChannel, IN TBUSSHMCHANNELCNF *a_ptChannelCnf,
                                        IN const TBUSADDR a_iLocalAddr, IN int a_iNeedInit, char *a_pszMem,
                                        size_t a_iMemSize, HANDLE a_iShmID, int iAlign);
///////////////////////////////////////////////////////////////////////////////////////////////////////



/*设计说明：tbus数据通道设计为循环队列，将数据放入队列时，之前的设计可能将数据分拆成两段放入队列中，
 * 这意味着数据不保持在地址连续的缓冲区中，当需要访问消息时，需要将消息拷贝出来。
 *为加快数据访问速度，更改上述设计方法，当数据放入队列尾部时，总是将数据保存在地址连续的缓冲区中。
 *按照这种设计思路，当从数据尾指针开始到队列尾部之间的空间不够存储一个完整数据时，将这些剩余的空间放入一个
 *假数据，将数据尾指针移植队列最前面保持数据。
 *通过宏定义TBUS_SPLIT_PKG_IN_RECYCLE_QUEUE来同时保存这两种方法的代码，编译时如果定义了TBUS_SPLIT_PKG_IN_RECYCLE_QUEUE
 *则采用分拆数据的方式存储；否则采用将数据保持在连续空间的方式存储，此方法也是缺省提供的实现方式
 */

#if defined (_WIN32) || defined (_WIN64)
static char *GetLastErrorString()
{
    static char msgBuf[1024];

    FormatMessage(
                  FORMAT_MESSAGE_FROM_SYSTEM,
                  NULL,
                  GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  msgBuf, 1024, NULL );

    return &msgBuf[0];
}
#endif

#ifdef TBUS_SPLIT_PKG_IN_RECYCLE_QUEUE
int tbus_push_channel_pkgv(IN TBUSCHANNEL* a_pstChannel, IN LPTBUSHEAD a_pstHead,
                           IN const struct iovec *a_ptVector, IN const int a_iVecCnt)
{
    CHANNELVAR *pstVar = NULL;
    int i;
    char szNet[TBUS_HEAD_CODE_BUFFER_SIZE];
    int iHeadLen = 0;
    int iRet = TBUS_SUCCESS;
    int iRoom;
    int iPkgLen;
    int iTailRoom;
    char *pszQueue = NULL ;
    unsigned int dwTmpTail;

    assert(NULL != a_pstChannel);
    assert(NULL != a_pstHead);
    assert(NULL != a_ptVector);

    /*填充并将头部消息打包*/
    pstVar = &a_pstChannel->pstHead->astQueueVar[a_pstChannel->iSendSide];
    tbus_log(TLOG_PRIORITY_TRACE,"queue(%d --> %d) size:%d head:%d tail:%d seq:%d",
             a_pstChannel->pstHead->astAddr[a_pstChannel->iRecvSide],
             a_pstChannel->pstHead->astAddr[a_pstChannel->iSendSide],
             pstVar->dwSize, pstVar->dwHead,
             pstVar->dwTail, pstVar->iSeq);

    a_pstHead->iSeq = pstVar->iSeq;
    a_pstHead->bFlag = a_pstHead->bFlag & ~TBUS_HEAD_FLAG_SYN ; /* avoid illegal bit set */
    a_pstHead->iBodyLen = 0;
    for ( i=0; i<a_iVecCnt; i++ )
    {
        if ((0 >= a_ptVector[i].iov_len) || (NULL == a_ptVector[i].iov_base))
        {
            continue;
        }
        a_pstHead->iBodyLen += (int)a_ptVector[i].iov_len;
    }
    if (0 > a_pstHead->iBodyLen)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"invalid data len %d", a_pstHead->iBodyLen);
        return TBUS_ERR_ARG;
    }
    iHeadLen = sizeof(szNet);
    iRet = tbus_encode_head(a_pstHead, &szNet[0], &iHeadLen, 0);
    if (TBUS_SUCCESS != iRet)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"failed to pack tbus head,iRet %x", iRet);
        return iRet;
    }
    if (iHeadLen > TBUS_HEAD_MAX_LEN)
    {
        tbus_log(TLOG_PRIORITY_FATAL,"length(%d) of tbus head packed is beyond the max scope of Head.bHeadLen", iHeadLen);
        iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_TOO_BIG_HEAD);
    }
    a_pstHead->bHeadLen = (unsigned char)iHeadLen;

    /*计算消息队列空余长度*/
    iRoom = (int)pstVar->dwHead - (int)pstVar->dwTail - 1;
    if (iRoom < 0)
    {
        iRoom += pstVar->dwSize;
    }
    iPkgLen =(int)(a_pstHead->bHeadLen + a_pstHead->iBodyLen);
    if (iRoom < iPkgLen)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"channel(Dst:%s) is, full head %i, tail %i, freeroom:%d, pkglen:%d",
                 tbus_addr_ntoa(a_pstChannel->pstHead->astAddr[a_pstChannel->iSendSide]),
                 pstVar->dwHead, pstVar->dwTail, iRoom, iPkgLen) ;
        return TBUS_ERR_CHANNEL_FULL;
    }


    /* store tbus head */
    pszQueue = a_pstChannel->pszQueues[a_pstChannel->iSendSide];
    dwTmpTail = pstVar->dwTail;
    if (pstVar->dwTail + iHeadLen > pstVar->dwSize)
    {
        iTailRoom = pstVar->dwSize - dwTmpTail;
        memcpy(pszQueue + dwTmpTail, &szNet[0], iTailRoom);
        memcpy(pszQueue, &szNet[0] + iTailRoom, iHeadLen - iTailRoom);
        dwTmpTail = iHeadLen - iTailRoom;
    } else
    {
        memcpy(pszQueue + dwTmpTail, &szNet[0], iHeadLen);
        dwTmpTail = (dwTmpTail + iHeadLen) % pstVar->dwSize;
    }


    /* store data */
    for ( i=0; i<a_iVecCnt; i++ )
    {
        char *pszData = (char *)a_ptVector[i].iov_base;
        int iDataLen = (int)a_ptVector[i].iov_len;
        if ((0 >= iDataLen) || (NULL == pszData))
        {
            continue;
        }
        if (dwTmpTail + iDataLen > pstVar->dwSize)
        {
            iTailRoom = pstVar->dwSize - dwTmpTail;
            memcpy(pszQueue + dwTmpTail, pszData, iTailRoom);
            memcpy(pszQueue, pszData + iTailRoom, iDataLen - iTailRoom);
            dwTmpTail = iDataLen - iTailRoom;
        } else
        {
            memcpy(pszQueue + dwTmpTail, pszData, iDataLen);
            dwTmpTail = (dwTmpTail + iDataLen) % pstVar->dwSize;
        }
    }/*for ( i=0; i<a_iVecCnt; i++ )*/

    pstVar->dwTail = dwTmpTail;
    pstVar->iSeq++; /*序列号增加*/
    tbus_log(TLOG_PRIORITY_TRACE,"queue(%d --> %d) size:%d head:%d tail:%d headlen:%d bodyLen:%d",
             a_pstChannel->pstHead->astAddr[a_pstChannel->iRecvSide],
             a_pstChannel->pstHead->astAddr[a_pstChannel->iSendSide],
             pstVar->dwSize, pstVar->dwHead,
             pstVar->dwTail, iHeadLen, a_pstHead->iBodyLen);

    return iRet;
}

#else /*#ifdef TBUS_SPLIT_PKG_IN_RECYCLE_QUEUE*/


int tbus_push_channel_pkgv(IN TBUSCHANNEL* a_pstChannel, IN LPTBUSHEAD a_pstHead,
                           IN const struct iovec *a_ptVector, IN const int a_iVecCnt)
{
    CHANNELVAR *pstVar = NULL;
    CHANNELHEAD *pstChlHead;
    int i;
    char szNet[TBUS_HEAD_CODE_BUFFER_SIZE];
    int iHeadLen = 0;
    int iRet = TBUS_SUCCESS;
    int iRoom;
    int iPkgLen;
    int iTailRoom;
    char *pszQueue = NULL ;

    char *pch;

    assert(NULL != a_pstChannel);
    assert(NULL != a_pstHead);
    assert(NULL != a_ptVector);


    pstVar = TBUS_CHANNEL_VAR_PUSH(a_pstChannel);
    pszQueue = TBUS_CHANNEL_QUEUE_PUSH(a_pstChannel);
    pstChlHead = a_pstChannel->pstHead;
    assert(NULL != pstChlHead);
    tbus_log(TLOG_PRIORITY_TRACE,"queue(0x%08x --> 0x%08x) size:%d head:%d tail:%d seq:%d",
             pstChlHead->astAddr[a_pstChannel->iRecvSide],
             pstChlHead->astAddr[a_pstChannel->iSendSide],
             pstVar->dwSize, pstVar->dwHead,
             pstVar->dwTail, pstVar->iSeq);

    /*统计数据长度，并构造tbus头部结构*/
    a_pstHead->iSeq = pstVar->iSeq;
    a_pstHead->bFlag &= ~TBUS_HEAD_FLAG_SYN ; /* avoid illegal bit set */
    a_pstHead->iBodyLen = 0;
    for ( i=0; i<a_iVecCnt; i++ )
    {
        if ((0 >= a_ptVector[i].iov_len) || (NULL == a_ptVector[i].iov_base))
        {
            continue;
        }
        a_pstHead->iBodyLen += (int)a_ptVector[i].iov_len;
    }
    if (0 > a_pstHead->iBodyLen)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"invalid data len %d", a_pstHead->iBodyLen);
        return TBUS_ERR_ARG;
    }
    iHeadLen = sizeof(szNet);
    iRet = tbus_encode_head(a_pstHead, &szNet[0], &iHeadLen, 0);
    if (TBUS_SUCCESS != iRet)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"failed to pack tbus head,iRet %x", iRet);
        return iRet;
    }
    assert(iHeadLen <= TBUS_HEAD_MAX_LEN);
    a_pstHead->bHeadLen = (unsigned char)iHeadLen;


    /*计算消息队列空余长度，并判断剩余空间是否足够保存此消息*/
    iRoom = (int)pstVar->dwHead - (int)pstVar->dwTail - 1;
    if (iRoom < 0)
    {
        iRoom += pstVar->dwSize;
    }
    iPkgLen =(int)(a_pstHead->bHeadLen + a_pstHead->iBodyLen);
    iPkgLen = TBUS_CALC_ALIGN(iPkgLen, pstChlHead->dwAlignLevel);
    if (0 >= iPkgLen)
    {
        /*快速计算对齐方式溢出，只能用简单费时的方法计算*/
        iPkgLen =(int)(a_pstHead->bHeadLen + a_pstHead->iBodyLen);
        TBUS_CALC_ALIGN_VALUE(iPkgLen, (1<<pstChlHead->dwAlignLevel));
        tbus_log(TLOG_PRIORITY_WARN,"failed to calc pkglen(%d) by align level :%d, so change the calc method",
                 (a_pstHead->bHeadLen + a_pstHead->iBodyLen), pstChlHead->dwAlignLevel) ;
    }
    tbus_log(TLOG_PRIORITY_TRACE, "by alignlevel(%d) pkglen from %d change to %d",
             pstChlHead->dwAlignLevel, (int)(a_pstHead->bHeadLen + a_pstHead->iBodyLen),iPkgLen);
    if (iRoom < iPkgLen)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"channel(Dst:%s) is, full head %i, tail %i, freeroom:%d, pkglen:%d",
                 tbus_addr_ntoa(TBUS_CHANNEL_PEER_ADDR(a_pstChannel)),
                 pstVar->dwHead, pstVar->dwTail, iRoom, iPkgLen) ;
        return TBUS_ERR_CHANNEL_FULL;
    }


    /*检查队列末尾剩余空间是否能保存下整个数据,如果尾部剩余空间不够保存完整消息，则将尾指针调整到队列最前面
    */
    iTailRoom = pstVar->dwSize - pstVar->dwTail;
    if (iTailRoom < iPkgLen)
    {
        if (iTailRoom > g_stBusGlobal.iCounterfeitPkgLen)
        {
            /*如果对尾剩余空间还可以保存一个伪造消息，则放置一个假消息*/
            tbus_log(TLOG_PRIORITY_INFO, "the queue tail room(%d) is bigger than the counterfeit pkglen(%d), "
                     "so push counterfeit pkg into the queue", iTailRoom, g_stBusGlobal.iCounterfeitPkgLen);
            memcpy(pszQueue + pstVar->dwTail, &g_stBusGlobal.stCounterfeitPkg, g_stBusGlobal.iCounterfeitPkgLen);
        }/*if (iTailRoom > g_stBusGlobal.iCounterfeitPkgLen)*/

        tbus_log(TLOG_PRIORITY_INFO,"the queue tail room(%d) is less than pkg len(%d), so need move tail to zero",
                 iTailRoom, iPkgLen);
        pstVar->dwTail = 0;

        /*重新计算队列剩余空间，并检查剩余空空是否足够保存一个完整消息*/
        iRoom = (int)pstVar->dwHead - (int)pstVar->dwTail - 1;
        if (iRoom < iPkgLen)
        {
            tbus_log(TLOG_PRIORITY_ERROR,"channel(Dst:%s) is, full head %i, tail %i, freeroom:%d, pkglen:%d",
                     tbus_addr_ntoa(TBUS_CHANNEL_PEER_ADDR(a_pstChannel)),
                     pstVar->dwHead, pstVar->dwTail, iRoom, iPkgLen) ;
            return TBUS_ERR_CHANNEL_FULL;
        }
    }/*if (iTailRoom < iPkgLen)*/


    /*保存整个消息，先保存tbus头部信息*/
    pch = pszQueue + pstVar->dwTail;
    memcpy(pch, &szNet[0], iHeadLen);
    pch += iHeadLen;


    /* store data */
    for ( i=0; i<a_iVecCnt; i++ )
    {
        char *pszData = (char *)a_ptVector[i].iov_base;
        int iDataLen = (int)a_ptVector[i].iov_len;
        if ((0 >= iDataLen) || (NULL == pszData))
        {
            continue;
        }
        memcpy(pch, pszData, iDataLen);
        pch += iDataLen;
    }/*for ( i=0; i<a_iVecCnt; i++ )*/

    pstVar->dwTail = (pstVar->dwTail + iPkgLen) % pstVar->dwSize;
    pstVar->iSeq++; /*序列号增加*/

    tbus_log(TLOG_PRIORITY_TRACE,"queue(0x%08x --> 0x%08x) size:%d head:%d tail:%d headlen:%d bodyLen:%d pkglen:%d",
             pstChlHead->astAddr[a_pstChannel->iRecvSide],
             pstChlHead->astAddr[a_pstChannel->iSendSide],
             pstVar->dwSize, pstVar->dwHead,
             pstVar->dwTail, iHeadLen, a_pstHead->iBodyLen, iPkgLen);

    return iRet;
}
#endif /*#ifdef TBUS_SPLIT_PKG_IN_RECYCLE_QUEUE*/

#ifdef TBUS_SPLIT_PKG_IN_RECYCLE_QUEUE
int	tbus_get_channel_pkgv(IN TBUSCHANNEL* a_pstChannel, OUT LPTBUSHEAD a_pstHead,
                          IN char *a_pszData, INOUT size_t *a_piLen)
{
    int iRet = TBUS_SUCCESS;
    CHANNELVAR *pstVar = NULL;
    char *pszQueue;
    int iPkgLen = 0,
        iDataLen = 0,
        iTailLen = 0 ;
    unsigned int dwHead;

    assert(NULL != a_pstChannel);
    assert(NULL != a_pstHead);
    assert(NULL != a_pszData);
    assert(NULL != a_piLen);

    pstVar = &a_pstChannel->pstHead->astQueueVar[a_pstChannel->iRecvSide];
    if (pstVar->dwHead == pstVar->dwTail)
    {
        return TBUS_ERR_CHANNEL_EMPTY;
    }
    tbus_log(TLOG_PRIORITY_TRACE,"queue(%d <-- %d) size:%d head:%d tail:%d ",
             a_pstChannel->pstHead->astAddr[a_pstChannel->iRecvSide],
             a_pstChannel->pstHead->astAddr[a_pstChannel->iSendSide],
             pstVar->dwSize, pstVar->dwHead,
             pstVar->dwTail);

    /*get head */
    pszQueue = a_pstChannel->pszQueues[a_pstChannel->iRecvSide];
    iRet = tbus_get_pkghead(a_pstHead, pszQueue, pstVar->dwSize, &pstVar->dwHead, pstVar->dwTail);
    if (TBUS_SUCCESS != iRet)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"failed to get pkg head length, iRet %x", iRet);
        if (iRet != TBUS_ERR_CHANNEL_EMPTY)
        {
            tbus_log(TLOG_PRIORITY_FATAL,"failed to get tbus head from channel queue(%d <-- %d), iRet %x, so clear remain data",
                     a_pstChannel->pstHead->astAddr[a_pstChannel->iRecvSide],
                     a_pstChannel->pstHead->astAddr[a_pstChannel->iSendSide], iRet);
            pstVar->dwHead = pstVar->dwTail;
        }
        return iRet;
    }/*if (TBUS_SUCCESS != iRet)*/


    /*get body*/
    iPkgLen = a_pstHead->bHeadLen + a_pstHead->iBodyLen;
    iDataLen = (int)(pstVar->dwTail - pstVar->dwHead);
    if (iDataLen < 0)
    {
        iDataLen += pstVar->dwSize;
    }
    if (iDataLen < iPkgLen)
    {
        tbus_log(TLOG_PRIORITY_FATAL,"data length(%d) in channel(src:%d, dst:%d) is less than the length(%d) of tbus pkg, so clear remain data",
                 iDataLen, a_pstChannel->pstHead->astAddr[a_pstChannel->iSendSide],
                 a_pstChannel->pstHead->astAddr[a_pstChannel->iRecvSide],iPkgLen);
        pstVar->dwHead = pstVar->dwTail;
        return TBUS_ERR_CHANNEL_CONFUSE;
    }
    if (*a_piLen < (int)a_pstHead->iBodyLen)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"the size(%d) of buffer to recv data is less than the length(%d) of data",
                 *a_piLen, a_pstHead->iBodyLen);
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_RECV_BUFFER_LIMITED);
    }

    /*copy body*/
    dwHead = (pstVar->dwHead + a_pstHead->bHeadLen) % pstVar->dwSize;
    iTailLen = pstVar->dwSize - dwHead;
    *a_piLen = a_pstHead->iBodyLen;
    if (iTailLen < *a_piLen)
    {
        memcpy(a_pszData, pszQueue + dwHead, iTailLen);
        memcpy(a_pszData + iTailLen, pszQueue, *a_piLen - iTailLen);
    } else
    {
        memcpy(a_pszData, pszQueue + dwHead, *a_piLen);
    }
    pstVar->dwHead = (dwHead + *a_piLen) % pstVar->dwSize;

    tbus_log(TLOG_PRIORITY_TRACE,"queue(%d <-- %d) size:%d head:%d tail:%d headlen:%d bodylen:%d",
             a_pstChannel->pstHead->astAddr[a_pstChannel->iRecvSide],
             a_pstChannel->pstHead->astAddr[a_pstChannel->iSendSide],
             pstVar->dwSize, pstVar->dwHead,
             pstVar->dwTail, a_pstHead->bHeadLen, *a_piLen);

    return 0;

}

#else /*TBUS_SPLIT_PKG_IN_RECYCLE_QUEUE*/


int	tbus_peek_channel_pkgv(IN TBUSCHANNEL* a_pstChannel, OUT LPTBUSHEAD a_pstHead,
                           IN const char **a_ppszData, OUT size_t *a_piLen)
{
    int iRet = TBUS_SUCCESS;
    CHANNELVAR *pstVar = NULL;
    char *pszQueue;
    int iPkgLen = 0;
    int iPkgAlignLen = 0;
    int iDataLen = 0;


    CHANNELHEAD *pstChlHead;

    assert(NULL != a_pstChannel);
    assert(NULL != a_pstHead);
    assert(NULL != a_ppszData);
    assert(NULL != a_piLen);

    *a_ppszData = NULL;
    *a_piLen = 0;

    pstVar = TBUS_CHANNEL_VAR_GET(a_pstChannel);
    pstChlHead = a_pstChannel->pstHead;
    pszQueue = TBUS_CHANNEL_QUEUE_GET(a_pstChannel);

    do {

        if (pstVar->dwHead == pstVar->dwTail)
        {
            return TBUS_ERR_CHANNEL_EMPTY;
        }


        if ((pstVar->dwHead >= pstVar->dwSize)||(pstVar->dwTail >= pstVar->dwSize))
        {
            tbus_log(TLOG_PRIORITY_FATAL,"queue(0x%08x <-- 0x%08x) param is invalid: size:%d head:%d tail:%d ",
                     pstChlHead->astAddr[a_pstChannel->iRecvSide],
                     pstChlHead->astAddr[a_pstChannel->iSendSide],
                     pstVar->dwSize, pstVar->dwHead,
                     pstVar->dwTail);
            pstVar->dwHead = pstVar->dwTail = 0;
            return TBUS_ERR_CHANNEL_CONFUSE;
        }
        tbus_log(TLOG_PRIORITY_TRACE,"queue(0x%08x <-- 0x%08x) size:%d head:%d tail:%d ",
                 pstChlHead->astAddr[a_pstChannel->iRecvSide],
                 pstChlHead->astAddr[a_pstChannel->iSendSide],
                 pstVar->dwSize, pstVar->dwHead,
                 pstVar->dwTail);

        /*检查从头指针到队列末尾是否足够保存一条消息*/
        TBUS_CHECK_QUEUE_HEAD_VAR(pszQueue, pstVar->dwSize, pstVar->dwHead);

        /*再次检查数据队列中是否有数据*/
        iDataLen = (int)(pstVar->dwTail - pstVar->dwHead);
        if (iDataLen < 0)
        {
            iDataLen += pstVar->dwSize;
        }
        if (0 >= iDataLen)
        {
            return TBUS_ERR_CHANNEL_EMPTY;
        }

        /*get head */
        iRet = tbus_decode_head(a_pstHead, pszQueue +pstVar->dwHead, iDataLen, 0);
        if (TBUS_SUCCESS != iRet)
        {
            tbus_log(TLOG_PRIORITY_FATAL, "failed to unpack tbus head from channel"
                     " queue(0x%08x <-- 0x%08x), iRet %x, clear channel",
                     TBUS_CHANNEL_LOCAL_ADDR(a_pstChannel),
                     TBUS_CHANNEL_PEER_ADDR(a_pstChannel), iRet);
            if (TBUS_ERR_MAKE_ERROR(TBUS_ERROR_FAILED_CHECK_HEADSUM) == iRet)
            {
                int iContextLen = TBUS_APP_CONTEXT_LEN + a_pstHead->bHeadLen;

                tbus_log_data(TLOG_PRIORITY_FATAL, g_stBusGlobal.pstHeadMeta, a_pstHead, sizeof(TBUSHEAD));
                if (TBUS_APP_CONTEXT_LEN > iDataLen)
                {
                    iContextLen = iDataLen;
                }
                tlog_log_hex(g_pstBusLogCat, TLOG_PRIORITY_FATAL, 0, 0,
                             pszQueue + pstVar->dwHead, iContextLen);
            }
            pstVar->dwHead = pstVar->dwTail;
            return iRet;
        }


        /*检查数据长度与与队列中数据长度是否有效*/
        iPkgLen = a_pstHead->bHeadLen + a_pstHead->iBodyLen;
        if ((0 >= iPkgLen) || (iPkgLen > (int)pstVar->dwSize))
        {
            tbus_log_data(TLOG_PRIORITY_FATAL, g_stBusGlobal.pstHeadMeta, a_pstHead, sizeof(TBUSHEAD));
            tbus_log(TLOG_PRIORITY_FATAL,"queue(0x%08x <-- 0x%08x) size:%d head:%d tail:%d ",
                     pstChlHead->astAddr[a_pstChannel->iRecvSide],
                     pstChlHead->astAddr[a_pstChannel->iSendSide],
                     pstVar->dwSize, pstVar->dwHead,
                     pstVar->dwTail);
            tbus_log(TLOG_PRIORITY_FATAL,"invalid pkg length(%d) in channel(src:0x%08x, dst:0x%08x),"
                     " its should be bigger than 0 and less than queuesize(%d),so clear remain data",
                     iPkgLen, a_pstChannel->pstHead->astAddr[a_pstChannel->iSendSide],
                     a_pstChannel->pstHead->astAddr[a_pstChannel->iRecvSide],pstVar->dwSize);
            pstVar->dwHead = pstVar->dwTail;
            return TBUS_ERR_CHANNEL_CONFUSE;
        }
        iPkgAlignLen = TBUS_CALC_ALIGN(iPkgLen, pstChlHead->dwAlignLevel);
        tbus_log(TLOG_PRIORITY_TRACE, "by alignlevel(%d) pkglen from %d change to %d, iHeadLen:%d",
                 pstChlHead->dwAlignLevel, (int)(a_pstHead->bHeadLen + a_pstHead->iBodyLen),
                 iPkgAlignLen,a_pstHead->bHeadLen);

        if (iDataLen < iPkgAlignLen)
        {
            int iContextLen = TBUS_APP_CONTEXT_LEN + a_pstHead->bHeadLen;

            tbus_log(TLOG_PRIORITY_FATAL,"data length(%d) in channel(src:0x%08x, dst:0x%08x)"
                     " is less than the length(%d) of tbus pkg, so clear remain data",
                     iDataLen, a_pstChannel->pstHead->astAddr[a_pstChannel->iSendSide],
                     a_pstChannel->pstHead->astAddr[a_pstChannel->iRecvSide],iPkgAlignLen);
            tbus_log_data(TLOG_PRIORITY_FATAL, g_stBusGlobal.pstHeadMeta, a_pstHead, sizeof(TBUSHEAD));
            if (TBUS_APP_CONTEXT_LEN > iDataLen)
            {
                iContextLen = iDataLen;
            }
            tlog_log_hex(g_pstBusLogCat, TLOG_PRIORITY_FATAL, 0, 0,
                         pszQueue + pstVar->dwHead, iContextLen);

            pstVar->dwHead = pstVar->dwTail;
            return TBUS_ERR_CHANNEL_CONFUSE;
        }

        if (TBUS_HEAD_FLAG_TEST_MSG & a_pstHead->bFlag)
        {
            tbus_log(TLOG_PRIORITY_TRACE, "recv a msg for tbus tested, data length<%d>, this msg will be deleted", iDataLen);
            TBUS_MOVETO_NEXT_PKG(iRet, iPkgLen, pstChlHead, pstVar->dwSize, pstVar->dwHead, pstVar->dwTail);
        }else if (TBUS_HEAD_FLAG_HEARTBEAT & a_pstHead->bFlag)
        {
            tbus_log(TLOG_PRIORITY_TRACE, "recv a heartbeat msg, msg type<%d>, this msg will be deleted", a_pstHead->iHeartBeatMsgType);

            /* heartbeat response is triggered by tbus automatically */
            /* when error happened, application needn't to know it */
            tbus_process_heartbeat_msg(a_pstChannel, a_pstHead);
            TBUS_MOVETO_NEXT_PKG(iRet, iPkgLen, pstChlHead, pstVar->dwSize, pstVar->dwHead, pstVar->dwTail);
        }else
        {
            break;
        }

    }while(TBUS_SUCCESS == iRet);

    /*设置数据的位置及长度信息*/
    if (TBUS_SUCCESS == iRet)
    {
        *a_ppszData = pszQueue + pstVar->dwHead + a_pstHead->bHeadLen;
        *a_piLen = a_pstHead->iBodyLen;
    }


    return iRet;

}



int tbus_delete_channel_headpkg(IN TBUSCHANNEL* a_pstChannel)
{
    int iRet = TBUS_SUCCESS;
    LPTBUSHEAD pstHead;
    CHANNELVAR *pstVar = NULL;
    char *pszQueue;
    CHANNELHEAD *pstChlHead;
    int iDataLen;
    int iPkgLen;

    assert(NULL != a_pstChannel);

    pstVar = TBUS_CHANNEL_VAR_GET(a_pstChannel);
    pstChlHead = a_pstChannel->pstHead;
    pszQueue = TBUS_CHANNEL_QUEUE_GET(a_pstChannel);
    /*tbus_log(TLOG_PRIORITY_TRACE,"queue(0x%08x <-- 0x%08x) size:%d head:%d tail:%d ",
      pstChlHead->astAddr[a_pstChannel->iRecvSide],
      pstChlHead->astAddr[a_pstChannel->iSendSide],
      pstVar->dwSize, pstVar->dwHead,
      pstVar->dwTail);*/



    /*检查头指针是否需要移动*/
    TBUS_CHECK_QUEUE_HEAD_VAR(pszQueue, pstVar->dwSize, pstVar->dwHead);
    iDataLen = pstVar->dwTail - pstVar->dwHead;
    if (0 > iDataLen)
    {
        iDataLen += pstVar->dwSize;
    }
    if (0 >= iDataLen)
    {
        return TBUS_ERR_CHANNEL_EMPTY;
    }

    if ((int)TBUS_HEAD_MIN_SIZE > iDataLen)
    {
        tbus_log(TLOG_PRIORITY_FATAL,
                 "failed to Get tbus head, dwGet:%u dwTail:%u dataLen:%d is less than tbushead min length %"PRIdPTR", its impossible!",
                 pstVar->dwHead, pstVar->dwTail, iDataLen, TBUS_HEAD_MIN_SIZE);
        tbus_log(TLOG_PRIORITY_FATAL,"failed to move to next  pkg in channel(src:0x%08x, dst:0x%08x), "
                 "so clear remain data",
                 a_pstChannel->pstHead->astAddr[a_pstChannel->iSendSide],
                 a_pstChannel->pstHead->astAddr[a_pstChannel->iRecvSide]);
        pstVar->dwHead = pstVar->dwTail;
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_CHANNEL_DATA_CONFUSE);
    }
    pstHead = (LPTBUSHEAD)(pszQueue + pstVar->dwHead);
    iPkgLen = pstHead->bHeadLen + ntohl(pstHead->iBodyLen);

    TBUS_MOVETO_NEXT_PKG(iRet, iPkgLen, pstChlHead, pstVar->dwSize, pstVar->dwHead, pstVar->dwTail);
    if (0 != iRet)
    {
        tbus_log(TLOG_PRIORITY_FATAL,"failed to move to next  pkg in channel(src:0x%08x, dst:0x%08x), "
                 "so clear remain data",
                 a_pstChannel->pstHead->astAddr[a_pstChannel->iSendSide],
                 a_pstChannel->pstHead->astAddr[a_pstChannel->iRecvSide]);
        pstVar->dwHead = pstVar->dwTail;

    }


    /*	tbus_log(TLOG_PRIORITY_TRACE,"queue(0x%08x <-- 0x%08x) size:%d head:%d tail:%d ",
        pstChlHead->astAddr[a_pstChannel->iRecvSide],
        pstChlHead->astAddr[a_pstChannel->iSendSide],
        pstVar->dwSize, pstVar->dwHead,
        pstVar->dwTail);*/


    return iRet;
}

int	tbus_get_channel_pkgv(IN TBUSCHANNEL* a_pstChannel, OUT LPTBUSHEAD a_pstHead,
                          IN char *a_pszData, INOUT size_t *a_piLen)
{
    int iRet = TBUS_SUCCESS;
    const char *pszGetData = NULL;
    CHANNELVAR *pstVar = NULL;
    size_t iDataLen = 0;
    int iPkgLen;


    assert(NULL != a_pstChannel);
    assert(NULL != a_pstHead);
    assert(NULL != a_pszData);
    assert(NULL != a_piLen);

    iRet = tbus_peek_channel_pkgv(a_pstChannel, a_pstHead, &pszGetData, &iDataLen);
    if (0 != iRet)
    {
        if ((int)TBUS_ERR_CHANNEL_EMPTY == iRet)
        {
            tbus_log(TLOG_PRIORITY_TRACE,"no pkg in the channel");
        }else
        {
            tbus_log(TLOG_PRIORITY_ERROR,"failed to peek pkg iRet %x", iRet);
        }

        return iRet;
    }

    if (*a_piLen < iDataLen)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"the size(%"PRIdPTR") of buffer to recv data is less than the length(%"PRIdPTR") of data",
                 *a_piLen, iDataLen);
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_RECV_BUFFER_LIMITED);
    }

    /*copy body*/
    memcpy(a_pszData, pszGetData, iDataLen);
    *a_piLen = iDataLen;
    iPkgLen = a_pstHead->bHeadLen + a_pstHead->iBodyLen;
    pstVar = TBUS_CHANNEL_VAR_GET(a_pstChannel);
    TBUS_MOVETO_NEXT_PKG(iRet, iPkgLen,	a_pstChannel->pstHead, pstVar->dwSize,
                         pstVar->dwHead, pstVar->dwTail);

    return iRet;

}

#endif /*TBUS_SPLIT_PKG_IN_RECYCLE_QUEUE*/


int tbus_channel_mode_init(IN int a_iFlag)
{
    int iRet = 0;

    /*初始化全局共享信息*/
    iRet = tbus_global_init();
    if (TBUS_SUCCESS != iRet)
    {
        printf("failed to initialize global information, iRet %x", iRet);

    }

    if (TBUS_CHANNEL_MODE_INIT_FLAG_ENABLE_TRACE & a_iFlag)
    {
        tbus_init_log();
    }
    return iRet;
}

void tbus_channel_mode_fini(void)
{

#if defined (_WIN32) || defined (_WIN64)
    tbus_fini_security_attr();
#endif
}

void tbus_init_default_channelcnf_i(LPTBUSSHMCHANNELCNF a_pstChannelCnf, IN TBUSADDR a_iLocalAddr,
                                    IN TBUSADDR a_iPeerAddr, unsigned int a_dwRecvSize, unsigned int a_dwSendSize)
{
    memset(a_pstChannelCnf, 0, sizeof(TBUSSHMCHANNELCNF));
    if (a_iLocalAddr < a_iPeerAddr)
    {
        a_pstChannelCnf->astAddrs[TBUS_CHANNEL_SIDE_INDEX_1] = a_iLocalAddr;
        a_pstChannelCnf->astAddrs[TBUS_CHANNEL_SIDE_INDEX_2] = a_iPeerAddr;
    }else
    {
        a_pstChannelCnf->astAddrs[TBUS_CHANNEL_SIDE_INDEX_2] = a_iLocalAddr;
        a_pstChannelCnf->astAddrs[TBUS_CHANNEL_SIDE_INDEX_1] = a_iPeerAddr;
    }
    a_pstChannelCnf->dwRecvSize = a_dwRecvSize;
    a_pstChannelCnf->dwSendSize = a_dwSendSize;
}

int tbus_attach_channel_to_mem_i(IN TBUSCHANNEL *a_ptChannel, IN TBUSSHMCHANNELCNF *a_ptChannelCnf, IN const TBUSADDR a_iLocalAddr,
                                 IN int a_iNeedInit, char *a_pszMem, size_t a_iMemSize, HANDLE a_iShmID, int iAlign)
{
    int iChannelHeadLen;
    CHANNELHEAD *pstHead;
    int iRet = TBUS_SUCCESS;

    assert(NULL != a_ptChannel);
    assert(NULL != a_ptChannelCnf);
    assert(NULL != a_pszMem);

    iChannelHeadLen = sizeof(CHANNELHEAD);
    TBUS_CALC_ALIGN_VALUE(iChannelHeadLen, iAlign);
    a_ptChannel->pstHead = (CHANNELHEAD *)a_pszMem;
    a_ptChannel->pszQueues[TBUS_CHANNEL_SIDE_INDEX_1] = a_pszMem + iChannelHeadLen;
    a_ptChannel->pszQueues[TBUS_CHANNEL_SIDE_INDEX_2] = a_pszMem + iChannelHeadLen + a_ptChannelCnf->dwRecvSize;

    if (a_iNeedInit || (TBUS_SHM_WITH_CTRL_CHNNL <= a_ptChannel->pstHead->dwVersion))
    {
        a_ptChannel->pszCtrlQueues[TBUS_CHANNEL_SIDE_INDEX_1]
            = a_ptChannel->pszQueues[TBUS_CHANNEL_SIDE_INDEX_2] - TBUS_SHM_CTRL_CHNNL_SIZE;

        a_ptChannel->pszCtrlQueues[TBUS_CHANNEL_SIDE_INDEX_2]
            = a_ptChannel->pszQueues[TBUS_CHANNEL_SIDE_INDEX_2]
            + a_ptChannelCnf->dwSendSize - TBUS_SHM_CTRL_CHNNL_SIZE;
    }

    pstHead = a_ptChannel->pstHead;

    if (a_iNeedInit)
    {
        tbus_init_channel_head_i(pstHead, a_ptChannelCnf, iAlign);
    }else
    {
        //此内存之前已经初始化为channel模式
        iRet = tbus_check_channel_param_i(a_ptChannelCnf, a_ptChannel, a_iMemSize);
        if (0 != iRet)
        {
            tbus_log(TLOG_PRIORITY_ERROR,"failed to check the param of the channel on shm:%s", tbus_error_string(iRet));
            return iRet;
        }
    }/*if (0 != a_iNeedInit)*/


    /* set channel side: a_iAddr is source address*/
    if ( a_iLocalAddr == pstHead->astAddr[TBUS_CHANNEL_SIDE_INDEX_1] )
    {
        a_ptChannel->iRecvSide = TBUS_CHANNEL_SIDE_INDEX_1 ;
        a_ptChannel->iSendSide = TBUS_CHANNEL_SIDE_INDEX_2 ;
    }else
    {
        a_ptChannel->iRecvSide = TBUS_CHANNEL_SIDE_INDEX_2 ;
        a_ptChannel->iSendSide = TBUS_CHANNEL_SIDE_INDEX_1;
    }
    a_ptChannel->iShmID = a_iShmID;

    /* check attached process number */
    /* for next two reasons, no error is returned */
    /* 1, tbusmgr may attached to a channel */
    /* 2, operator's wrong operation may cause a running process exits, which may be unacceptable */
#if !defined (_WIN32) && !defined (_WIN64)
    if (0 != a_ptChannel->iShmID)
    {
        int iAttachedProcNum = 0;
        iRet = tbus_get_attached_num_i(a_ptChannel->iShmID, &iAttachedProcNum);
        if (0 != iRet)
        {
            iRet = 0;
            tbus_log(TLOG_PRIORITY_ERROR, "failed to get attached process number to shm<%d>", a_ptChannel->iShmID);
        } else
        {
            if (2 < iAttachedProcNum)
            {
                /* 因为iAttachedProcNum反映的是shmat的数量，
                 * 这个不等同于attached进程数，
                 * tormapi/tconndapi都有对tbus_bind的调用，
                 * 所以这里不输出warning到stderr/stdout
                 */
                /*fprintf(stderr, "Warnning: more than 2 attaches to tbus-channel<shmid:%d>\n", a_ptChannel->iShmID);*/
                tbus_log(TLOG_PRIORITY_FATAL, "%d attaches to tbus-channel<shmd:%d>", iAttachedProcNum, a_ptChannel->iShmID);
            }
        }
    }
#endif

    return TBUS_SUCCESS;
}


/* open a channel, if not exist, create it */
int tbus_channel_open(INOUT LPTBUSCHANNEL *a_ppstChannel, IN TBUSADDR a_iLocalAddr,
                      IN TBUSADDR a_iPeerAddr,IN int a_iShmkey, IN int a_iSize)
{
    int iRet = TBUS_SUCCESS;
    LPTBUSCHANNEL pstChannel = NULL;
    TBUSSHMCHANNELCNF stChannelCnf;
    int iFlag;
    int iSize;
    char *sShm;
    int iCreated = 0;
    HANDLE iShmID ;
    int iChannelHeadLen;

    //assert(NULL != a_ppstChannel);
    //assert(0 < a_iSize);
    if ((NULL == a_ppstChannel)||(0 >= a_iSize))
    {
        tbus_log(TLOG_PRIORITY_ERROR,"invalid param: a+ppstChannel:%p a_iSize:%d",
                 a_ppstChannel, a_iSize);
        return TBUS_ERR_ARG;
    }


    TBUS_CALC_ALIGN_VALUE(a_iSize, TBUS_DEFAULT_CHANNEL_DATA_ALIGN);
    tbus_init_default_channelcnf_i(&stChannelCnf, a_iLocalAddr, a_iPeerAddr,
                                   a_iSize, a_iSize);
    iChannelHeadLen = sizeof(CHANNELHEAD);
    TBUS_CALC_ALIGN_VALUE(iChannelHeadLen, TBUS_DEFAULT_CHANNEL_DATA_ALIGN);
    iSize = iChannelHeadLen + a_iSize*TBUS_CHANNEL_SIDE_NUM;

#if !defined (_WIN32) && !defined (_WIN64)
    iFlag = 0666 | IPC_CREAT;
    iRet =  tbus_auto_get_shm ((void**)(void*)&sShm, &iCreated, a_iShmkey, &iSize, iFlag, &iShmID);
#else
    iFlag = TMMAPF_READ | TMMAPF_WRITE | TMMAPF_CREATE;
    iRet = tbus_open_mmap_by_key((void**)(void*)&sShm, a_iShmkey, &iSize, iFlag, &iCreated, &iShmID);
#endif
    if (0 != iRet)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"ERROR: tbus_attach_channel, getShmAny() failed %i", iRet ) ;
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_SHMGET_FAILED);
    }

    pstChannel = (LPTBUSCHANNEL)calloc(1, sizeof(TBUSCHANNEL));
    if (NULL == pstChannel)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"failed to malloc memory for charnnel\n");
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_NO_MEMORY);
    }
    pstChannel->pstPeerState = &pstChannel->stPeerState;
    TBUS_CHANNEL_SET_ENABLE(pstChannel);
    iRet = tbus_attach_channel_to_mem_i(pstChannel, &stChannelCnf, a_iLocalAddr, iCreated,
                                        sShm, iSize, iShmID, TBUS_DEFAULT_CHANNEL_DATA_ALIGN);
    if (0 != iRet)
    {
        free(pstChannel);
        pstChannel = NULL;
    }else
    {
        *a_ppstChannel = pstChannel;
    }

    return iRet;

}




size_t tbus_channel_calc_mem_size(IN int a_iSize)
{
    size_t iSize;
    int iChannelHeadLen;

    iChannelHeadLen = sizeof(CHANNELHEAD);
    TBUS_CALC_ALIGN_VALUE(iChannelHeadLen, TBUS_DEFAULT_CHANNEL_DATA_ALIGN);
    iSize = iChannelHeadLen + a_iSize*TBUS_CHANNEL_SIDE_NUM;

    return iSize;
}


/* open a channel, if not exist, create it */
int tbus_channel_open_mem(INOUT LPTBUSCHANNEL *a_ppstChannel, IN TBUSADDR a_iLocalAddr,
                          IN TBUSADDR a_iPeerAddr,IN void *a_pvBuff, IN size_t a_iBuffSize,
                          int a_iNeedInit, IN int a_iQueueSize)
{
    int iRet = TBUS_SUCCESS;
    LPTBUSCHANNEL pstChannel = NULL;
    TBUSSHMCHANNELCNF stChannelCnf;
    size_t iSize;
    int iChannelHeadLen;


    if ((NULL == a_ppstChannel)||(0 >= a_iQueueSize) || (NULL == a_pvBuff))
    {
        tbus_log(TLOG_PRIORITY_ERROR,"invalid param: a+ppstChannel:%p a_iSize:%d",
                 a_ppstChannel, a_iQueueSize);
        return TBUS_ERR_ARG;
    }

    TBUS_CALC_ALIGN_VALUE(a_iQueueSize, TBUS_DEFAULT_CHANNEL_DATA_ALIGN);
    iChannelHeadLen = sizeof(CHANNELHEAD);
    TBUS_CALC_ALIGN_VALUE(iChannelHeadLen, TBUS_DEFAULT_CHANNEL_DATA_ALIGN);
    iSize = iChannelHeadLen + a_iQueueSize*TBUS_CHANNEL_SIDE_NUM;

    if (a_iBuffSize < iSize)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"the buff size(%"PRIuFAST32")"
                 "for tbus channel is less the need size(%"PRIuFAST32 ": headsize:%d, quesize:%d)",
                 a_iBuffSize, iSize,  iChannelHeadLen, a_iQueueSize);
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_CHANNEL_SIZE);
    }


    //生成通道配置
    tbus_init_default_channelcnf_i(&stChannelCnf, a_iLocalAddr, a_iPeerAddr,
                                   a_iQueueSize, a_iQueueSize);


    //创建通道句柄并初始化
    pstChannel = (LPTBUSCHANNEL)calloc(1, sizeof(TBUSCHANNEL));
    if (NULL == pstChannel)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"failed to malloc memory for charnnel\n");
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_NO_MEMORY);
    }
    TBUS_CHANNEL_SET_ENABLE(pstChannel);
    iRet = tbus_attach_channel_to_mem_i(pstChannel, &stChannelCnf, a_iLocalAddr, a_iNeedInit,
                                        a_pvBuff, a_iBuffSize, (HANDLE)0, TBUS_DEFAULT_CHANNEL_DATA_ALIGN);
    if (0 != iRet)
    {
        free(pstChannel);
        pstChannel = NULL;
    }else
    {
        *a_ppstChannel = pstChannel;
    }

    return iRet;
}



int tbus_attach_channel (
                         IN const TBUSADDR a_iAddr,
                         IN TBUSSHMCHANNELCNF *a_ptRoute,
                         INOUT TBUSCHANNEL *a_ptChannel,
                         IN LPTBUSSHMGCIMHEAD a_pstHead
                        )
{
    int iRet = TBUS_SUCCESS;
    char *sShm = NULL ;
    int iSize;
    int iFlag = 0;

    int iChannelHeadLen;

    assert(NULL != a_ptRoute);
    assert(NULL != a_ptChannel);
    assert(NULL != a_pstHead);

    iFlag = TBUS_CHECK_CHNNL_FLAG_KEEPOPEN;
    iRet = tbus_check_channel_shm_i(a_ptRoute, a_pstHead, iFlag);
    if (TBUS_SUCCESS != iRet)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"failed to check share memory of channel, iRet %x", iRet);
        return iRet;
    }

    iChannelHeadLen = sizeof(CHANNELHEAD);
    TBUS_CALC_ALIGN_VALUE(iChannelHeadLen, a_pstHead->iAlign);
    iSize = iChannelHeadLen + a_ptRoute->dwRecvSize + a_ptRoute->dwSendSize;

#if defined (_WIN32) || defined (_WIN64)
    {
        sShm = tmmap(a_ptRoute->iShmID, 0, iSize, TMMAPF_PROT_WRITE|TMMAPF_PROT_READ);
        if (NULL == sShm)
        {
            tbus_log(TLOG_PRIORITY_ERROR,"tmmap failed by id %d, for %s\n", a_ptRoute->iShmID, GetLastErrorString());
            return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_SHMAT_FAILED);
        }
        /* check size */
        /* MapViewOfFile called by tmmap may check iSize, so here ommit */
    }
#else
    {
        struct shmid_ds stShmStat ;
        sShm =	shmat(a_ptRoute->iShmID, NULL, 0 ) ;
        if ((void *)-1 == sShm)
        {
            tbus_log(TLOG_PRIORITY_ERROR,"shmat failed by id %d, for %s\n", a_ptRoute->iShmID, strerror(errno));
            return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_SHMAT_FAILED);
        }

        /*check size */
        memset(&stShmStat, 0, sizeof(stShmStat));
        if ( 0 != shmctl(a_ptRoute->iShmID, IPC_STAT, &stShmStat))
        {
            tbus_log(TLOG_PRIORITY_ERROR,"shmctl stat failed by id %d, for %s\n", a_ptRoute->iShmID, strerror(errno));
            return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_SHMAT_FAILED);
        }

        if (iSize > (int)stShmStat.shm_segsz)
        {
            tbus_log(TLOG_PRIORITY_ERROR,"shm size(%d) is smaller than the channel size(%d) expected\n",
                     (int)stShmStat.shm_segsz, iSize);
            return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_SHMAT_FAILED);
        }
    }
#endif

    iRet = tbus_attach_channel_to_mem_i(a_ptChannel, a_ptRoute, a_iAddr, 0, sShm, iSize,
                                        a_ptRoute->iShmID, a_pstHead->iAlign);
    if (TBUS_ERR_IS_ERROR(iRet))
    {
        tbus_log(TLOG_PRIORITY_ERROR, "tbus_attach_channel_to_mem_i failed, for %s", tbus_error_string(iRet));
        return iRet;
    }

    tbus_log(TLOG_PRIORITY_DEBUG,"attach to channel(shmid:%d) successfully, localside:%d, peerside:%d",
             a_ptRoute->iShmID, a_ptChannel->iRecvSide, a_ptChannel->iSendSide);

    return TBUS_SUCCESS ;
}


/* open a channel, if not exist, return error */
int tbus_channel_get(int a_iShmkey, LPTBUSCHANNEL a_pstChannel)
{
    int iRet = TBUS_SUCCESS;
    int iMinShmSize;
    int iHeadLen;
    int iCreated;
    int iFlag;
    int iSize;
    HANDLE iShmID;

    iHeadLen = sizeof(CHANNELHEAD);
    TBUS_CALC_ALIGN_VALUE(iHeadLen, TBUS_DEFAULT_CHANNEL_DATA_ALIGN);
    iSize = iHeadLen;
    iMinShmSize = iHeadLen;
#if !defined (_WIN32) && !defined (_WIN64)
    iFlag = 0666 & ~IPC_CREAT;
    iRet = tbus_auto_get_shm ((void**)(void*)&a_pstChannel->pstHead, &iCreated, a_iShmkey, &iSize, iFlag, &iShmID);
#else
    iFlag = TMMAPF_READ | TMMAPF_WRITE;
    iRet = tbus_open_mmap_by_key((void**)(void*)&a_pstChannel->pstHead, a_iShmkey, &iSize, iFlag, &iCreated, &iShmID);
#endif
    if ( iRet < 0 )
    {
        tbus_log(TLOG_PRIORITY_ERROR,"ERROR: tbus_channel_get_shm_head, get shm failed %s", tbus_error_string(iRet));
        return iRet;
    }


    iMinShmSize += (a_pstChannel->pstHead->astQueueVar[0].dwSize + TBUS_SHM_CTRL_CHNNL_SIZE);
    iMinShmSize += (a_pstChannel->pstHead->astQueueVar[1].dwSize + TBUS_SHM_CTRL_CHNNL_SIZE);
    if (TBUS_CHANNEL_MAGIC != a_pstChannel->pstHead->dwMagic)
    {
        tbus_log(TLOG_PRIORITY_ERROR, "Magic(%x) of the shm channel<key: %d> is not equal to Tbus Magic(%x)",
                 a_pstChannel->pstHead->dwMagic, a_iShmkey, TBUS_CHANNEL_MAGIC);
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_CHANNEL_MAGIC);
    }

    if (iMinShmSize > iSize)
    {
        tbus_log(TLOG_PRIORITY_ERROR, "shm's size<%d> is smaller than size<%d>"
                 "calculated by CHANNELHEAD's size<%d>, queue_1's size<%d> and queue_2's size<%d>", iSize, iMinShmSize,
                 iHeadLen, a_pstChannel->pstHead->astQueueVar[0].dwSize, a_pstChannel->pstHead->astQueueVar[1].dwSize);
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_CHANNEL_SIZE);
    }

    a_pstChannel->pszQueues[0] = (char*)a_pstChannel->pstHead + iHeadLen;
    a_pstChannel->pszQueues[1] = a_pstChannel->pszQueues[0] + a_pstChannel->pstHead->astQueueVar[0].dwSize;

    /* TODO a_pstChannel->pszCtrlQueues */

    return iRet;
}

static int tbus_check_channel_param_i(LPTBUSSHMCHANNELCNF a_pstCnf,
                                      LPTBUSCHANNEL a_pstChannel, IN size_t iShmSize)
{
    int iRet = TBUS_SUCCESS;
    LPCHANNELHEAD pstHead;
    size_t iExpectSize;
    unsigned int iCtrlChnnlSize = 0;
    char szAddr1[TBUS_MAX_ADDR_STRING_LEN];
    char szAddr2[TBUS_MAX_ADDR_STRING_LEN];

    assert(NULL != a_pstChannel);
    assert(NULL != a_pstCnf);

    pstHead = a_pstChannel->pstHead;
    if (NULL == pstHead)
    {
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_NULL_CHANNEL_HEAD);
    }
    if ((pstHead->astAddr[0] != a_pstCnf->astAddrs[0]) ||( pstHead->astAddr[1] != a_pstCnf->astAddrs[1]))
    {
        tbus_log(TLOG_PRIORITY_ERROR,"Channel config ERROR!\n");
        tbus_log(TLOG_PRIORITY_ERROR,"address1 in Channel Shm : %s   ", tbus_addr_ntop(pstHead->astAddr[0], szAddr1,sizeof(szAddr1)));
        tbus_log(TLOG_PRIORITY_ERROR,"address2 in Channel Shm : %s", tbus_addr_ntop(pstHead->astAddr[1],szAddr2,sizeof(szAddr2)));
        tbus_log(TLOG_PRIORITY_ERROR,"address1 in GCIM : %s   ", tbus_addr_ntop(a_pstCnf->astAddrs[0],szAddr1, sizeof(szAddr1)));
        tbus_log(TLOG_PRIORITY_ERROR,"address1 in GCIM : %s", tbus_addr_ntop(a_pstCnf->astAddrs[1],szAddr2, sizeof(szAddr2)));
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_CHANNEL_ADDRESS_CONFLICT);
    }
    tbus_addr_ntop(a_pstCnf->astAddrs[0], szAddr1, sizeof(szAddr1));
    tbus_addr_ntop(a_pstCnf->astAddrs[1], szAddr2, sizeof(szAddr2));
    if ((pstHead->dwVersion == TBUS_SHM_CHANNEL_VERSION) && (pstHead->dwMagic != TBUS_CHANNEL_MAGIC))
    {
        tbus_log(TLOG_PRIORITY_ERROR, "Magic(%x) of the shm channel(%s <--> %s) is not equal to Tbus Magic(%x)",
                 pstHead->dwMagic, szAddr1,szAddr2, TBUS_CHANNEL_MAGIC);
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_CHANNEL_MAGIC);
    }
    if (0 >= pstHead->dwAlignLevel)
    {
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_ALIGN);
    }
    //检查大小是否一致
    iExpectSize = sizeof(CHANNELHEAD);
    TBUS_CALC_ALIGN_VALUE(iExpectSize, (1 << pstHead->dwAlignLevel));
    iExpectSize += a_pstCnf->dwRecvSize;
    iExpectSize += a_pstCnf->dwSendSize;
    if ((0 < iShmSize) && (iExpectSize > iShmSize))
    {
        tbus_log(TLOG_PRIORITY_ERROR, "the size(%"PRIdPTR") of the channel(%s <--> %s) needed is less than the  real size(%"PRIdPTR") of share memory ",
                 iExpectSize, szAddr1, szAddr2, iShmSize);
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_CHANNEL_SIZE_NOTMATCH);
    }
    if (TBUS_SHM_WITH_CTRL_CHNNL <= pstHead->dwVersion)
    {
        iCtrlChnnlSize = TBUS_SHM_CTRL_CHNNL_SIZE;
    }
    if ((0>= pstHead->astQueueVar[0].dwSize) ||
        ((iCtrlChnnlSize + pstHead->astQueueVar[0].dwSize) != a_pstCnf->dwRecvSize))
    {
        tbus_log(TLOG_PRIORITY_ERROR, "the recvsize(%u) of the channel(%s <--> %s) is different to the size(%d) configured",
                 (iCtrlChnnlSize + pstHead->astQueueVar[0].dwSize), szAddr1,szAddr2, a_pstCnf->dwRecvSize);
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_CHANNEL_SIZE);
    }
    if ((0>= pstHead->astQueueVar[1].dwSize) ||
        ((iCtrlChnnlSize + pstHead->astQueueVar[1].dwSize) != a_pstCnf->dwSendSize))
    {
        tbus_log(TLOG_PRIORITY_ERROR, "the sendsize(%u) of the channel(%s <--> %s) is different to the size(%d) configured",
                 (iCtrlChnnlSize + pstHead->astQueueVar[1].dwSize), szAddr1,szAddr2, a_pstCnf->dwSendSize);
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_CHANNEL_SIZE);
    }
    if ((pstHead->astQueueVar[0].dwHead >= pstHead->astQueueVar[0].dwSize)||
        (pstHead->astQueueVar[0].dwTail >= pstHead->astQueueVar[0].dwSize))
    {
        tbus_log(TLOG_PRIORITY_ERROR, "the param[dwHead:%u dwTail:%u dwSize:%u] of the channel(%s <--> %s) is invalid",
                 pstHead->astQueueVar[0].dwHead, pstHead->astQueueVar[0].dwTail,
                 pstHead->astQueueVar[0].dwSize, szAddr1,szAddr2);
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_CAHNNEL_QUEUE_PARAM);
    }
    if ((pstHead->astQueueVar[1].dwHead >= pstHead->astQueueVar[1].dwSize)||
        (pstHead->astQueueVar[1].dwTail >= pstHead->astQueueVar[1].dwSize))
    {
        tbus_log(TLOG_PRIORITY_ERROR, "the param[dwHead:%u dwTail:%u dwSize:%u] of the channel(%s <--> %s) is invalid",
                 pstHead->astQueueVar[1].dwHead, pstHead->astQueueVar[1].dwTail,
                 pstHead->astQueueVar[1].dwSize, szAddr1,szAddr2);
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_CAHNNEL_QUEUE_PARAM);
    }
    if (TBUS_SHM_WITH_CTRL_CHNNL <= pstHead->dwVersion)
    {
        if (0 >= pstHead->astQueueVar[0].dwCtrlChnnlSize
            || TBUS_SHM_CTRL_CHNNL_SIZE != pstHead->astQueueVar[0].dwCtrlChnnlSize)
        {
            tbus_log(TLOG_PRIORITY_ERROR, "the ctrl channel recvsize(%u) of the "
                     "channel(%s <--> %s) is different to the size(%d) expected",
                     pstHead->astQueueVar[0].dwCtrlChnnlSize, szAddr1,szAddr2, TBUS_SHM_CTRL_CHNNL_SIZE);
            return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_CAHNNEL_QUEUE_PARAM);
        }
        if (0 >= pstHead->astQueueVar[1].dwCtrlChnnlSize
            || TBUS_SHM_CTRL_CHNNL_SIZE != pstHead->astQueueVar[1].dwCtrlChnnlSize)
        {
            tbus_log(TLOG_PRIORITY_ERROR, "the ctrl channel sendsize(%u) of the "
                     "channel(%s <--> %s) is different to the size(%d) expected",
                     pstHead->astQueueVar[1].dwCtrlChnnlSize, szAddr1,szAddr2, TBUS_SHM_CTRL_CHNNL_SIZE);
            return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_CAHNNEL_QUEUE_PARAM);
        }
        if ((pstHead->astQueueVar[0].dwCtrlChnnlHead >= pstHead->astQueueVar[0].dwCtrlChnnlSize)||
            (pstHead->astQueueVar[0].dwCtrlChnnlTail >= pstHead->astQueueVar[0].dwCtrlChnnlSize))
        {
            tbus_log(TLOG_PRIORITY_ERROR,
                     "the ctrl param[dwCtrlChnnlHead:%u dwCtrlChnnlTail:%u dwCtrlChnnlSize:%u] "
                     "of the channel(%s <--> %s) is invalid",
                     pstHead->astQueueVar[0].dwCtrlChnnlHead,
                     pstHead->astQueueVar[0].dwCtrlChnnlTail,
                     pstHead->astQueueVar[0].dwCtrlChnnlSize, szAddr1, szAddr2);
            return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_CAHNNEL_QUEUE_PARAM);
        }
        if ((pstHead->astQueueVar[1].dwHead >= pstHead->astQueueVar[1].dwSize)||
            (pstHead->astQueueVar[1].dwTail >= pstHead->astQueueVar[1].dwSize))
        {
            tbus_log(TLOG_PRIORITY_ERROR,
                     "the ctrl param[dwCtrlChnnlHead:%u dwCtrlChnnlTail:%u dwCtrlChnnlSize:%u] "
                     "of the channel(%s <--> %s) is invalid",
                     pstHead->astQueueVar[1].dwCtrlChnnlHead,
                     pstHead->astQueueVar[1].dwCtrlChnnlTail,
                     pstHead->astQueueVar[1].dwCtrlChnnlSize, szAddr1, szAddr2);
            return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_CAHNNEL_QUEUE_PARAM);
        }
    }

    return iRet;
}

int tbus_channel_open_by_str(INOUT LPTBUSCHANNEL *a_ppstChannel, IN const char *a_pszLocalAddr,
                             IN const char *a_pszPeerAddr,IN int a_iShmkey, IN int a_iSize)
{
    unsigned int dwLocal=0;
    unsigned int dwPeer=0;


    //assert(NULL != a_pszLocalAddr);
    //assert(NULL != a_pszPeerAddr);
    if ((NULL == a_ppstChannel)||(NULL ==  a_pszLocalAddr)||(NULL == a_pszPeerAddr)||
        (0 >= a_iSize))
    {
        tbus_log(TLOG_PRIORITY_ERROR,"invalid param: a_ppstChannel:%p a_pszLocalAddr:%p "
                 "a_pszPeerAddr: %p a_iSize:%d",
                 a_ppstChannel, a_pszLocalAddr, a_pszPeerAddr, a_iSize);
        return TBUS_ERR_ARG;
    }


    /*转换地址*/
    if (0 == inet_aton((char *)a_pszLocalAddr,  (struct in_addr *)&dwLocal))
    {
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_ADDR_STRING);
    }
    if (0 == inet_aton((char *)a_pszPeerAddr,  (struct in_addr *)&dwPeer))
    {
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_ADDR_STRING);
    }

    return  tbus_channel_open(a_ppstChannel, dwLocal, dwPeer, a_iShmkey, a_iSize);

}


/**
 * @brief 查询指定的通道上的发送队列中的消息数量
 * @param[in,out] a_iMsgNum    如成功返回，保存发送队列中的消息数量。
 * @param[in]     a_pstChannel 通道句柄
 * @retval        0  successful
 * @retval        !0 failed
 */
int tbus_channel_get_outmsg_num(IN LPTBUSCHANNEL a_pstChannel, OUT int* a_piMsgNum)
{
    int iRet = TBUS_SUCCESS;
    LPCHANNELVAR pstPutVar;
    LPCHANNELHEAD pstHead;
    TBUSHEAD stHead;
    int iDataLen;

    if (NULL == a_pstChannel)
    {
        tbus_log(TLOG_PRIORITY_ERROR, "tbus_channel_get_outmsg_num parameter error, a_pstChannel is NULL.");
        return TBUS_ERR_ARG ;
    }

    if (NULL == a_piMsgNum)
    {
        tbus_log(TLOG_PRIORITY_ERROR, "tbus_channel_get_outmsg_num parameter error, a_piMsgNum is NULL.");
        return TBUS_ERR_ARG ;
    }

    pstHead = a_pstChannel->pstHead;
    pstPutVar  = &pstHead->astQueueVar[a_pstChannel->iSendSide];

    iDataLen = pstPutVar->dwTail - pstPutVar->dwHead;
    if (iDataLen < 0)
    {
        iDataLen += pstPutVar->dwSize;
    }

    if (0 >= iDataLen)
    {
        *a_piMsgNum = 0;
        return TBUS_SUCCESS;
    }

    iRet = tbus_get_pkghead(&stHead, a_pstChannel->pszQueues[a_pstChannel->iSendSide],
                            pstPutVar->dwSize, (unsigned int*)&pstPutVar->dwHead, pstPutVar->dwTail);
    if (TBUS_SUCCESS == iRet)
    {
        *a_piMsgNum = pstPutVar->iSeq - stHead.iSeq;
        return TBUS_SUCCESS;
    } else
    {
        return iRet;
    }
}

int tbus_get_channel_inmsg_num(IN LPTBUSCHANNEL a_pstChannel, OUT int* a_piMsgNum)
{
    int iRet = TBUS_SUCCESS;
    LPCHANNELVAR pstGetVar;
    LPCHANNELHEAD pstHead;
    TBUSHEAD stHead;
    int iDataLen;

    if (NULL == a_pstChannel)
    {
        tbus_log(TLOG_PRIORITY_ERROR, "tbus_channel_get_outmsg_num parameter error, a_pstChannel is NULL.");
        return TBUS_ERR_ARG ;
    }
    if (NULL == a_piMsgNum)
    {
        tbus_log(TLOG_PRIORITY_ERROR, "tbus_channel_get_outmsg_num parameter error, a_piMsgNum is NULL.");
        return TBUS_ERR_ARG ;
    }

    *a_piMsgNum = 0;
    pstHead = a_pstChannel->pstHead;
    pstGetVar  = &pstHead->astQueueVar[a_pstChannel->iRecvSide];
    iDataLen = pstGetVar->dwTail - pstGetVar->dwHead;
    if (iDataLen < 0)
    {
        iDataLen += pstGetVar->dwSize;
    }
    if (0 >= iDataLen)
    {
        return TBUS_SUCCESS;
    }

    iRet = tbus_get_pkghead(&stHead, a_pstChannel->pszQueues[a_pstChannel->iRecvSide],
        pstGetVar->dwSize, (unsigned int*)&pstGetVar->dwHead, pstGetVar->dwTail);
    if (TBUS_SUCCESS == iRet)
    {
        *a_piMsgNum = pstGetVar->iSeq - stHead.iSeq;
    }

    return iRet;
}


/**
 *  @brief 在指定通道上发送一个数据包
 *	@param[in] a_pstChannel 通道句柄
 *	@param[in] a_pvData 保存数据的缓冲区首地址
 *	@param[in] a_iDataLen 数据长度
 *	@param[in] a_iFlag 控制字段
 *	@retval 0 success,
 *	@retval !0 failed.
 */
int tbus_channel_send(IN LPTBUSCHANNEL a_pstChannel,IN const void *a_pvData,
                      IN const int a_iDataLen, IN const int a_iFlag)
{
    int iRet = TBUS_SUCCESS;
    TBUSHEAD stHead;
    LPTBUSHEAD pstHead = NULL;
    struct iovec astVectors[1];

    pstHead = &stHead;

    if ((NULL == a_pstChannel) || (NULL == a_pvData) ||
        (0 >= a_iDataLen))
    {
        tbus_log(TLOG_PRIORITY_ERROR,"tbus_send, parameter error, datalen(%i)", a_iDataLen ) ;
        return TBUS_ERR_ARG ;
    }
    astVectors[0].iov_base = (void *)a_pvData;
    astVectors[0].iov_len = a_iDataLen;

    iRet = tbus_channel_sendv(a_pstChannel, &astVectors[0],1,a_iFlag);

    return iRet;
}

TSF4G_API int tbus_channel_sendv(IN LPTBUSCHANNEL a_pstChannel, IN const struct iovec *a_ptVector,
                                 IN const int a_iVecCnt,        IN const int a_iFlag   )
{
    int iRet = TBUS_SUCCESS;
    TBUSHEAD stHead;
    LPTBUSHEAD pstHead = NULL;


    pstHead = &stHead;

    if ((NULL == a_pstChannel) || (NULL == a_ptVector) ||
        (0 >= a_iVecCnt))
    {
        tbus_log(TLOG_PRIORITY_ERROR,"tbus_send, parameter error, vector num(%d)", a_iVecCnt ) ;
        return TBUS_ERR_ARG ;
    }

    /*构造tbus消息*/
    memset(pstHead, 0, sizeof(TBUSHEAD));
    pstHead->bCmd = TBUS_HEAD_CMD_TRANSFER_DATA;
    pstHead->iSrc = a_pstChannel->pstHead->astAddr[a_pstChannel->iRecvSide];
    pstHead->iDst = a_pstChannel->pstHead->astAddr[a_pstChannel->iSendSide];
    TBUS_SET_LAST_ROUTE(*pstHead, pstHead->iSrc);
    if ((TBUS_FLAG_START_DYE_MSG & a_iFlag) || (TBUS_FLAG_KEEP_DYE_MSG & a_iFlag))
        tbus_dye_pkg(pstHead, NULL, a_iFlag);

    iRet = tbus_push_channel_pkgv(a_pstChannel, pstHead, a_ptVector, a_iVecCnt) ;
    if (iRet != 0)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"tbus_push_channel_pkgv() failed, iRet %d", iRet ) ;
    }else
    {
        char szAddr1[TBUS_MAX_ADDR_STRING_LEN];
        tbus_log(TLOG_PRIORITY_DEBUG,"DEBUG: tbus_channel_send ok, source address %s",
            tbus_addr_ntop((TBUSADDR)(pstHead->iSrc), szAddr1, sizeof(szAddr1))) ;
    }
    if(pstHead->bFlag & TBUS_HEAD_FLAG_TACE)
    {
        tbus_log_dyedpkg(pstHead, "channel_sendv");
    }

    return iRet;
}


/**
  @brief clean remaining msg from channel
  @param[in] a_pstChannel handle of channel
  @retval 0 -- success
  @retval !0 -- failed
  */
int tbus_channel_clean(IN LPTBUSCHANNEL a_pstChannel)
{
    int iRet = TBUS_SUCCESS;
    CHANNELVAR *pstVar = NULL;

    if (NULL == a_pstChannel)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"tbus_channel_clean, null parameter error") ;
        return TBUS_ERR_ARG ;
    }

    pstVar = &a_pstChannel->pstHead->astQueueVar[a_pstChannel->iRecvSide];
    pstVar->dwHead = pstVar->dwTail;

    pstVar = &a_pstChannel->pstHead->astQueueVar[a_pstChannel->iSendSide];
    pstVar->dwTail = pstVar->dwHead;

    tbus_log(TLOG_PRIORITY_INFO,"all pkgs in channel(%d <--> %d) was cleaned",
             a_pstChannel->pstHead->astAddr[a_pstChannel->iRecvSide],
             a_pstChannel->pstHead->astAddr[a_pstChannel->iSendSide]);

    return iRet;
}

/*@brief 在指定通道上接受一个数据包
 *	@param[in] a_pstChannel 通道句柄
 *	@param[in] a_pvData 保存数据的缓冲区首地址
 *	@param[in,out] a_iDataLen 保存数据长度的指针
 *	in	-	接受缓冲区最大大小
 *	out -	接受数据的实际大小
 *	@param[in] a_iFlag 控制字段
 *	@retval 0 success,
 *	@retval !0 failed.
 */
int tbus_channel_recv(IN LPTBUSCHANNEL a_pstChannel, INOUT void *a_pvData,
                      INOUT size_t *a_piDataLen,
                      IN const int a_iFlag)
{
    int iRet = TBUS_SUCCESS;
    LPTBUSHEAD pstHead ;


    if ((NULL == a_pstChannel) || (NULL == a_pvData) ||
        (NULL == a_piDataLen) || (0 >= *a_piDataLen))
    {

        tbus_log(TLOG_PRIORITY_ERROR,"tbus_send, parameter error, datalen(%"PRIdPTR")", *a_piDataLen) ;
        return TBUS_ERR_ARG ;
    }

    pstHead = &a_pstChannel->stHead;
    iRet = tbus_get_channel_pkgv(a_pstChannel, pstHead, a_pvData, a_piDataLen) ;
    if (iRet != 0)
    {
        if ((int)TBUS_ERR_CHANNEL_EMPTY != iRet)
        {
            tbus_log(TLOG_PRIORITY_ERROR,"tbus_get_channel_pkgv() failed, iRet %x", iRet ) ;
        }else
        {
            tbus_log(TLOG_PRIORITY_TRACE,"no pkg in the channel") ;
        }

    }else
    {
        char szAddr1[TBUS_MAX_ADDR_STRING_LEN];
        tbus_log(TLOG_PRIORITY_DEBUG,"DEBUG: tbus_channel_recv ok, source address %s",
                 tbus_addr_ntop((TBUSADDR )(pstHead->iSrc), szAddr1, sizeof(szAddr1))) ;
    }

    if (0 == iRet)
    {
        if ((TBUS_FLAG_START_DYE_MSG & a_iFlag) || (TBUS_FLAG_KEEP_DYE_MSG & a_iFlag))
            tbus_dye_pkg(pstHead, NULL, a_iFlag);
        if(pstHead->bFlag & TBUS_HEAD_FLAG_TACE)
        {
            tbus_log_dyedpkg(pstHead, "channel_recv");
        }
    }


    return iRet;
}

int tbus_channel_peek_msg(IN LPTBUSCHANNEL a_pstChannel,
                          INOUT const char **a_ppvData,
                          OUT size_t *a_piDataLen,
                          IN const int a_iFlag)
{
    int iRet = TBUS_SUCCESS;
    LPTBUSHEAD pstHead ;


    if ((NULL == a_pstChannel)
        || (NULL == a_ppvData)
        || (NULL == a_piDataLen) )
    {
        tbus_log(TLOG_PRIORITY_ERROR,"tbus_send, parameter error") ;
        return TBUS_ERR_ARG ;
    }

    pstHead = &a_pstChannel->stHead;
    iRet = tbus_peek_channel_pkgv(a_pstChannel, pstHead, a_ppvData, a_piDataLen);

    if (0 == iRet)
    {
        if ((TBUS_FLAG_START_DYE_MSG & a_iFlag) || (TBUS_FLAG_KEEP_DYE_MSG & a_iFlag))
            tbus_dye_pkg(pstHead, NULL, a_iFlag);
        if(pstHead->bFlag & TBUS_HEAD_FLAG_TACE)
        {
            tbus_log_dyedpkg(pstHead, "tbus_channel_peek_msg");
        }
    }


    return iRet;

}

int tbus_channel_delete_msg(IN LPTBUSCHANNEL a_pstChannel)
{
    int iRet = TBUS_SUCCESS;

    if (NULL == a_pstChannel)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"tbus_send, parameter error") ;
        return TBUS_ERR_ARG ;
    }

    iRet = tbus_delete_channel_headpkg(a_pstChannel);

    return iRet;
}

static void tbus_channel_close_i(INOUT LPTBUSCHANNEL *a_ppstChannel, int a_iNeedDetach)
{
    if (NULL != a_ppstChannel)
    {
        if (NULL != *a_ppstChannel)
        {
            LPTBUSCHANNEL pstChannel = *a_ppstChannel;
            if (a_iNeedDetach)
            {
                shmdt((void *)pstChannel->pstHead);
            }
            free(pstChannel);
            *a_ppstChannel = NULL;
        }
    }
}

void tbus_channel_close(INOUT LPTBUSCHANNEL *a_ppstChannel)
{
    return tbus_channel_close_i(a_ppstChannel, 1);
}

void tbus_channel_close_mem(INOUT LPTBUSCHANNEL *a_ppstChannel)
{
    return tbus_channel_close_i(a_ppstChannel, 0);
}

int tbus_get_channel( IN const int a_iHandle, OUT LPTBUSCHANNEL *a_ppstChannel, IN TBUSADDR a_iSrcAddr, IN TBUSADDR a_iDstAddr)
{
    int iRet = TBUS_SUCCESS;
    int iChannelCnt;
    TBUSCHANNEL *pstChannel = NULL ;
    LPTBUSHANDLE pstHandle;
    int i;

    //assert(NULL != a_ppstChannel);
    if (NULL == a_ppstChannel)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"a_ppstChnnel is null cannot get channel by addr(src:%u dst :%u)",
                 a_iSrcAddr, a_iDstAddr);
        return TBUS_ERR_ARG;
    }

    pstHandle = TBUS_GET_HANDLE(a_iHandle);
    if (NULL == pstHandle)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"invalid handle %d", a_iHandle);
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_HANDLE);
    }
    iChannelCnt = pstHandle->dwChannelCnt ;
    if ( 0 >= iChannelCnt )
    {
        tbus_log(TLOG_PRIORITY_ERROR,"tbus_peer_ctrl(), no available address" ) ;
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_NO_CHANNEL) ;
    }

    for (i = 0; i < iChannelCnt; i++)
    {
        CHANNELHEAD *pstHead ;
        pstChannel = pstHandle->pastChannelSet[i];
        if (!TBUS_CHANNEL_IS_ENABLE(pstChannel) )
        {
            tbus_log(TLOG_PRIORITY_TRACE,"tbus_peer_ctrl(), address info disable %i", i ) ;
            continue ;
        }


        pstHead = pstChannel->pstHead;
        if ((a_iSrcAddr == pstHead->astAddr[pstChannel->iRecvSide]) &&
            (a_iDstAddr == pstHead->astAddr[pstChannel->iSendSide]))
        {
            break;
        }
    }/*for (i = 0; i < iChannelCnt; i++)*/

    if (i >= iChannelCnt)
    {
        *a_ppstChannel = NULL;
        iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_NO_CHANNEL);
    }else
    {
        *a_ppstChannel = pstHandle->pastChannelSet[i];
    }

    return iRet;
}

int tbus_get_peer_count(IN int a_iHandle, IN TBUSADDR a_iLocalAddr, OUT int* a_piPeerCount)
{
    int i = 0;
    int iMatchedChannelCount = 0;
    LPTBUSHANDLE pstHandle = NULL;
    CHANNELHEAD *pstHead = NULL;
    LPTBUSCHANNEL  pstChannel = NULL;

    if (NULL == a_piPeerCount)
    {
        tbus_log(TLOG_PRIORITY_ERROR, "a_piPeerCount to tbus_get_peer_count is NULL");
        return TBUS_ERR_ARG ;
    }

    pstHandle = TBUS_GET_HANDLE(a_iHandle);
    if (NULL == pstHandle)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"invalid handle %d", a_iHandle);
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_HANDLE);
    }

    for (i = 0; i < pstHandle->dwChannelCnt; i++)
    {
        pstChannel = pstHandle->pastChannelSet[i];
        pstHead = pstChannel->pstHead;
        if ((pstHead->astAddr[0] != a_iLocalAddr)
            &&(pstHead->astAddr[1] != a_iLocalAddr))
        {
            continue;
        }

        if (TBUS_CHANNEL_IS_DISABLE_BY_CTRL(pstChannel))
        {
            tbus_log(TLOG_PRIORITY_INFO,"the channel(%u <--> %u) is diabled by control policy"
                     ", so tbus_get_peer_count ignored it", pstHead->astAddr[0], pstHead->astAddr[1]);
            continue;
        }

        iMatchedChannelCount++;
    }

    *a_piPeerCount = iMatchedChannelCount;

    return TBUS_SUCCESS;
}

int tbus_get_peer_addr(IN int a_iHandle, IN TBUSADDR a_iLocalAddr, OUT TBUSADDR* a_piAddrBuff, INOUT int* a_piAddrCount)
{
    int i = 0;
    int iMatchedChannelCount = 0;
    LPTBUSHANDLE pstHandle = NULL;
    CHANNELHEAD *pstHead = NULL;
    LPTBUSCHANNEL  pstChannel = NULL;

    if (NULL == a_piAddrBuff)
    {
        tbus_log(TLOG_PRIORITY_ERROR, "a_piAddrBuff to tbus_get_peer_addr is NULL");
        return TBUS_ERR_ARG ;
    }

    if (NULL == a_piAddrCount)
    {
        tbus_log(TLOG_PRIORITY_ERROR, "a_piAddrCount to tbus_get_peer_addr is NULL");
        return TBUS_ERR_ARG ;
    }

    pstHandle = TBUS_GET_HANDLE(a_iHandle);
    if (NULL == pstHandle)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"invalid handle %d", a_iHandle);
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_HANDLE);
    }

    for (i = 0; i < pstHandle->dwChannelCnt; i++)
    {
        pstChannel = pstHandle->pastChannelSet[i];
        pstHead = pstChannel->pstHead;
        if ((pstHead->astAddr[0] != a_iLocalAddr)
            &&(pstHead->astAddr[1] != a_iLocalAddr))
        {
            continue;
        }

        if (TBUS_CHANNEL_IS_DISABLE_BY_CTRL(pstChannel))
        {
            tbus_log(TLOG_PRIORITY_INFO, "the channel(%u <--> %u) is diabled by control policy"
                     ", so tbus_get_peer_addr ignored it", pstHead->astAddr[0], pstHead->astAddr[1]);
            continue;
        }

        if (iMatchedChannelCount+1 > *a_piAddrCount)
        {
            tbus_log(TLOG_PRIORITY_ERROR, "%d has more than %d peers", a_iLocalAddr, *a_piAddrCount);
            return TBUS_ERR_LIMITED_BUF;
        }

        if ((pstHead->astAddr[0] == a_iLocalAddr))
        {
            a_piAddrBuff[iMatchedChannelCount++] = pstHead->astAddr[1];
        } else
        {
            a_piAddrBuff[iMatchedChannelCount++] = pstHead->astAddr[0];
        }
    }

    *a_piAddrCount = iMatchedChannelCount;

    return TBUS_SUCCESS;
}

int tbus_enable_addr (
                      IN const TBUSADDR a_iLocalAddr,
                      IN const TBUSADDR a_iPeerAddr,
                      IN TBUSSHMCHANNELCNF *a_ptRoute,
                      IN int a_iHandle,
                      IN LPTBUSSHMGCIMHEAD a_pstHead
                     )
{
    int iRet = TBUS_SUCCESS ;
    unsigned int i;
    CHANNELHEAD *pstHead;
    LPTBUSCHANNEL  pstChannel;
    LPTBUSHANDLE pstBusHandle = NULL;

    assert(NULL != a_ptRoute);
    assert(NULL != a_pstHead);

    pstBusHandle = TBUS_GET_HANDLE(a_iHandle);
    assert(NULL != pstBusHandle);

    /*检查此通道是否已经在tbus通道管理域中*/
    for (i = 0; i < pstBusHandle->dwChannelCnt; i++)
    {
        pstChannel = pstBusHandle->pastChannelSet[i];
        pstHead = pstChannel->pstHead;
        if ((pstHead->astAddr[0] != a_ptRoute->astAddrs[0]) ||
            (pstHead->astAddr[1] != a_ptRoute->astAddrs[1]))
        {
            continue;
        }

        if (TBUS_CHANNEL_IS_DISABLE_BY_CTRL(pstChannel))
        {
            tbus_log(TLOG_PRIORITY_INFO,"the channel(%u <--> %u) is diabled by control policy, "
                     "so connot bind", pstHead->astAddr[0], pstHead->astAddr[1]);
        }else
        {
            TBUS_CHANNEL_CLR_NOT_IN_GCIM(pstChannel);
            TBUS_CHANNEL_SET_ENABLE(pstChannel);
        }
        break;
    }/*for (i = 0; i < pstBusHandle->dwChannelCnt; i++)*/
    if (i < pstBusHandle->dwChannelCnt)
    {
        return TBUS_SUCCESS;
    }


    /*不存在则添加到tbus句柄中*/
    if (TUBS_MAX_CHANNEL_COUNT_PREHANDLE <= pstBusHandle->dwChannelCnt)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"tbus_bind, num<%d> of binded channel reach max num(%d) perhandle, so cannot bind any more channel(SrcAddr:%s)",
                 pstBusHandle->dwChannelCnt, TUBS_MAX_CHANNEL_COUNT_PREHANDLE, tbus_addr_ntoa(a_iLocalAddr)) ;
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_CHANNEL_NUM_LIMITED);
    }

    /*新分配一个通道管理句柄*/
    pstChannel = (TBUSCHANNEL *) malloc ( sizeof(TBUSCHANNEL) ) ;
    if (NULL == pstChannel)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"failed to malloc memory(size:%"PRIdPTR")", sizeof(TBUSCHANNEL));
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_NO_MEMORY);
    }

    memset(pstChannel, 0, sizeof(TBUSCHANNEL) ) ;


    // 如果设置了互斥模式，只有用tbus_connect才能添加通道
    // 这里虽然添加了通道，但是如果设置了互斥模式，则disable通道
    if (TBUS_HANDLE_IS_EXCLUSIVE(pstBusHandle))
    {
        TBUS_CHANNEL_CLR_ENABLE(pstChannel);
    }else
    {
        TBUS_CHANNEL_SET_ENABLE(pstChannel);
    }


    iRet = tbus_attach_channel ( a_iLocalAddr, a_ptRoute, pstChannel, a_pstHead) ;
    if (0 != iRet)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"tbus_attach_channel failed, iRet %d", iRet);
        free(pstChannel);
        return iRet;
    }

    iRet = tbus_init_process_state(pstBusHandle, pstChannel, a_iPeerAddr);
    if (TBUS_ERR_IS_ERROR(iRet))
    {
        tbus_log(TLOG_PRIORITY_ERROR, "faild to init process state about peer<%d>, for %s",
                 a_iPeerAddr, tbus_error_string(iRet));
    } else
    {
        char szTmpIP[128];
        CHANNELHEAD *pstHead = pstChannel->pstHead;
        STRNCPY(szTmpIP, tbus_addr_ntoa(pstHead->astAddr[0]), sizeof(szTmpIP));
        pstBusHandle->pastChannelSet[pstBusHandle->dwChannelCnt] = pstChannel;
        pstBusHandle->dwChannelCnt++;
        pstChannel->iHandle = a_iHandle;
        tbus_log(TLOG_PRIORITY_DEBUG, "add channel(%s <--> %s) to handle, now channel num in the hanndle is:%u",
                 szTmpIP, tbus_addr_ntoa(pstHead->astAddr[1]), pstBusHandle->dwChannelCnt);
    }

    return iRet ;
}



int tbus_channel_get_bus_head (IN LPTBUSCHANNEL a_pstChannel, INOUT void *a_pvBuffer, INOUT int *a_piLen )
{
    int iRet = TBUS_SUCCESS;

    if ( (NULL == a_pvBuffer) || (NULL == a_piLen) || (NULL == a_pstChannel) )
    {
        tbus_log(TLOG_PRIORITY_ERROR,"parameter error, null paramter") ;
        iRet = TBUS_ERR_ARG ;
        return iRet ;
    }

    if (0 >= *a_piLen)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"parameter error, invalid len(%d)", *a_piLen) ;
        iRet = TBUS_ERR_ARG ;
        return iRet ;
    }

    iRet = tbus_encode_head(&a_pstChannel->stHead, a_pvBuffer, a_piLen, 0);

    return iRet;
}


int tbus_channel_set_bus_head(IN LPTBUSCHANNEL a_pstChannel, IN const void *a_pvBuffer, IN const int a_iLen )
{
    int iRet = TBUS_SUCCESS ;

    if ( (NULL == a_pvBuffer) || (0 >= a_iLen) ||(NULL == a_pstChannel))
    {
        tbus_log(TLOG_PRIORITY_ERROR,"parameter error, len %d", a_iLen ) ;
        return TBUS_ERR_ARG ;
    }

    iRet = tbus_decode_head(&a_pstChannel->stHead, (char *)a_pvBuffer, a_iLen, 0);
    if (TBUS_SUCCESS != iRet)
    {
        memset(&a_pstChannel->stHead, 0, sizeof(TBUSHEAD));
    }

    return iRet ;
}


int tbus_channel_forward (IN LPTBUSCHANNEL a_pstChannel, IN const void *a_pvData, IN const size_t a_iDataLen,	IN const int a_iFlag)
{
    int iRet = TBUS_SUCCESS;
    struct iovec astVectors[1];


    if ((NULL == a_pstChannel) || (NULL == a_pvData) ||
        (0 >= a_iDataLen))
    {
        tbus_log(TLOG_PRIORITY_ERROR,"tbus_send, parameter error, datalen(%"PRIdPTR")", a_iDataLen ) ;
        return TBUS_ERR_ARG ;
    }

    astVectors[0].iov_base = (void *)a_pvData;
    astVectors[0].iov_len = a_iDataLen;
    iRet = tbus_channel_forwardv(a_pstChannel, &astVectors[0], 1,a_iFlag) ;


    return iRet;
}

int tbus_channel_forwardv(IN LPTBUSCHANNEL a_pstChannel, IN const struct iovec *a_ptVector,
                                    IN const int a_iVecCnt,    IN const int a_iFlag)
{
    int iRet = TBUS_SUCCESS;
    TBUSHEAD stHead;
    LPTBUSHEAD pstHead = NULL;



    if ((NULL == a_pstChannel) || (NULL == a_ptVector) ||
        (0 >= a_iVecCnt))
    {
        tbus_log(TLOG_PRIORITY_ERROR,"tbus_send, parameter error, vector num(%d)", a_iVecCnt ) ;
        return TBUS_ERR_ARG ;
    }

    /*构造tbus消息*/
    pstHead = &stHead;
    memset(pstHead, 0, sizeof(*pstHead));

    pstHead->stExtHead.stDataHead.bRoute = a_pstChannel->stHead.stExtHead.stDataHead.bRoute;
    if ((TBUS_FORWARD_MAX_ROUTE <= pstHead->stExtHead.stDataHead.bRoute))
    {
        tbus_log(TLOG_PRIORITY_ERROR,"invalid  route num(%d), its scope should be [0-%d), so cannot forward",
            pstHead->stExtHead.stDataHead.bRoute, TBUS_FORWARD_MAX_ROUTE);
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_FORWARD_ROUTE_LIMITED);
    }

    memcpy(pstHead->stExtHead.stDataHead.routeAddr,
        a_pstChannel->stHead.stExtHead.stDataHead.routeAddr,
        (pstHead->stExtHead.stDataHead.bRoute\
        * sizeof(pstHead->stExtHead.stDataHead.routeAddr[0])));

    pstHead->bCmd = TBUS_HEAD_CMD_TRANSFER_DATA;
    pstHead->iSrc = a_pstChannel->pstHead->astAddr[a_pstChannel->iRecvSide];
    pstHead->iDst = a_pstChannel->pstHead->astAddr[a_pstChannel->iSendSide];
    pstHead->bFlag |= TBUS_HEAD_FLAG_WITH_ROUTE;
    pstHead->stExtHead.stDataHead.routeAddr[pstHead->stExtHead.stDataHead.bRoute] = pstHead->iSrc;
    pstHead->stExtHead.stDataHead.bRoute++;



    //发送数据
    if ((TBUS_FLAG_START_DYE_MSG & a_iFlag) || (TBUS_FLAG_KEEP_DYE_MSG & a_iFlag))
        tbus_dye_pkg(pstHead, &a_pstChannel->stHead, a_iFlag);

    iRet = tbus_push_channel_pkgv(a_pstChannel, pstHead, a_ptVector, a_iVecCnt) ;
    if (iRet != 0)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"tbus_push_channel_pkgv() failed, iRet %d", iRet ) ;
    }else
    {
        char szAddr1[TBUS_MAX_ADDR_STRING_LEN];
        tbus_log(TLOG_PRIORITY_DEBUG,"DEBUG: tbus_channel_send ok, source address %s",
            tbus_addr_ntop((TBUSADDR )(pstHead->iSrc), szAddr1, sizeof(szAddr1))) ;
    }
    if(pstHead->bFlag & TBUS_HEAD_FLAG_TACE)
    {
        tbus_log_dyedpkg(pstHead, "channel_forward");
    }

    return iRet;
}

int tbus_channel_backward (IN LPTBUSCHANNEL a_pstChannel, IN const void *a_pvData,
                           IN const size_t a_iDataLen,	IN const int a_iFlag)
{
    int iRet = TBUS_SUCCESS;
    struct iovec astVectors[1];


    if ((NULL == a_pstChannel) || (NULL == a_pvData) ||
        (0 >= a_iDataLen))
    {
        tbus_log(TLOG_PRIORITY_ERROR,"tbus_send, parameter error, datalen(%"PRIdPTR")", a_iDataLen ) ;
        return TBUS_ERR_ARG ;
    }

    astVectors[0].iov_base = (void *)a_pvData;
    astVectors[0].iov_len = a_iDataLen;
    iRet = tbus_channel_backwardv(a_pstChannel, &astVectors[0], 1,a_iFlag) ;


    return iRet;
}

int tbus_channel_backwardv(IN LPTBUSCHANNEL a_pstChannel,IN const struct iovec *a_ptVector,
                                     IN const int a_iVecCnt,  IN const int a_iFlag)
{
    int iRet = TBUS_SUCCESS;
    TBUSHEAD stHead;
    LPTBUSHEAD pstHead = NULL;



    if ((NULL == a_pstChannel) || (NULL == a_ptVector) ||
        (0 >= a_iVecCnt))
    {
        tbus_log(TLOG_PRIORITY_ERROR,"tbus_send, parameter error, vector num(%d)", a_iVecCnt ) ;
        return TBUS_ERR_ARG ;
    }

    pstHead = &stHead;
    memset(pstHead, 0, sizeof(*pstHead));

    pstHead->stExtHead.stDataHead.bRoute = a_pstChannel->stHead.stExtHead.stDataHead.bRoute;
    if ((0 >= pstHead->stExtHead.stDataHead.bRoute ) ||
        (TBUS_FORWARD_MAX_ROUTE < pstHead->stExtHead.stDataHead.bRoute))
    {
        tbus_log(TLOG_PRIORITY_ERROR,"invalid  route num(%d), its scope must be [1-%d], so cannot backward",
            pstHead->stExtHead.stDataHead.bRoute, TBUS_FORWARD_MAX_ROUTE);
        return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_ROUTE);
    }
    pstHead->stExtHead.stDataHead.bRoute--;

    if ( 0 < pstHead->stExtHead.stDataHead.bRoute)
    {
        pstHead->bFlag |= TBUS_HEAD_FLAG_WITH_ROUTE;
        memcpy(pstHead->stExtHead.stDataHead.routeAddr,
            a_pstChannel->stHead.stExtHead.stDataHead.routeAddr,
            (pstHead->stExtHead.stDataHead.bRoute\
            * sizeof(pstHead->stExtHead.stDataHead.routeAddr[0])));
    }

    pstHead->bCmd = TBUS_HEAD_CMD_TRANSFER_DATA;
    pstHead->iSrc = a_pstChannel->pstHead->astAddr[a_pstChannel->iRecvSide];
    pstHead->iDst = a_pstChannel->pstHead->astAddr[a_pstChannel->iSendSide];



    //发送数据
    if ((TBUS_FLAG_START_DYE_MSG & a_iFlag) || (TBUS_FLAG_KEEP_DYE_MSG & a_iFlag))
        tbus_dye_pkg(pstHead, &a_pstChannel->stHead, a_iFlag);

    iRet = tbus_push_channel_pkgv(a_pstChannel, pstHead, a_ptVector,a_iVecCnt) ;
    if (iRet != 0)
    {
        tbus_log(TLOG_PRIORITY_ERROR,"tbus_push_channel_pkgv() failed, iRet %d", iRet ) ;
    }else
    {
        char szAddr1[TBUS_MAX_ADDR_STRING_LEN];
        tbus_log(TLOG_PRIORITY_DEBUG,"DEBUG: tbus_channel_send ok, source address %s",
            tbus_addr_ntop((TBUSADDR )(pstHead->iSrc), szAddr1, sizeof(szAddr1))) ;
    }
    if(pstHead->bFlag & TBUS_HEAD_FLAG_TACE)
    {
        tbus_log_dyedpkg(pstHead, "channel_backward");
    }

    return iRet;
}

int tbus_init_process_state(IN LPTBUSHANDLE a_pstHandle, IN LPTBUSCHANNEL a_pstChannel, IN TBUSADDR a_iPeerAddr)
{
    int iRet = TBUS_SUCCESS;
    struct timeval stCurTime;
    LPTBUSPROCESSSTATE pstPeerState = NULL;

    assert(NULL != a_pstHandle);
    assert(NULL != a_pstChannel);

    assert(TBUS_ADDR_ALL != a_iPeerAddr);
    assert(TBUS_ADDR_DEFAULT != a_iPeerAddr);

    pstPeerState = tbus_get_process_state_i(a_pstHandle, a_iPeerAddr);
    if (NULL != pstPeerState)
    {
        assert(0 < pstPeerState->iBindedChnnlNum);
        a_pstChannel->pstPeerState = pstPeerState;
        pstPeerState->iBindedChnnlNum++;
        return iRet;
    }

    gettimeofday(&stCurTime, NULL);

    assert(TUBS_MAX_CHANNEL_COUNT_PREHANDLE > a_pstHandle->dwPeerCnt);
    pstPeerState = &a_pstHandle->astPeerState[a_pstHandle->dwPeerCnt];
    pstPeerState->llLastBeatTime = (long long)stCurTime.tv_sec;
    pstPeerState->iIsAlive = 1;
    pstPeerState->iIsAvailable = TBUS_PEER_STATE_AVAILABLE;
    pstPeerState->iTimeOutGap = 0;
    pstPeerState->iPeerAddr = a_iPeerAddr;
    a_pstChannel->pstPeerState = pstPeerState;
    pstPeerState->iBindedChnnlNum = 1;
    a_pstHandle->dwPeerCnt++;

    return iRet;
}

int tbus_process_heartbeat_msg(LPTBUSCHANNEL a_pstChannel, LPTBUSHEAD a_pstHead)
{
    int iRet = TBUS_SUCCESS;
    struct timeval stCurTime;
    struct timeval *pstReferTime;

    assert(NULL != a_pstChannel);
    assert(NULL != a_pstHead);

    if (g_stBusGlobal.iFlag & TBUS_INIT_FLAG_USE_SAVED_TIME)
    {
        pstReferTime= &g_stBusGlobal.stCurTime;
    } else
    {
        gettimeofday(&stCurTime, NULL);
        pstReferTime = &stCurTime;
    }

    assert(NULL != a_pstChannel->pstPeerState);
    a_pstChannel->pstPeerState->llLastBeatTime = (long long)pstReferTime->tv_sec;

    if (TBUS_HEARTBEAT_MSG_TYPE_QUIT & a_pstHead->iHeartBeatMsgType)
    {
        a_pstChannel->pstPeerState->iIsAlive = 0;
    } else
    {
        a_pstChannel->pstPeerState->iIsAlive = 1;
    }

    if (TBUS_HEARTBEAT_MSG_TYPE_SYN & a_pstHead->iHeartBeatMsgType)
    {
        LPTBUSCHANNEL astChannels[1];

        astChannels[0] = a_pstChannel;
        iRet = tbus_send_heartbeat_msg_i(astChannels, 1, TBUS_ADDR_ALL, 0, TBUS_HEARTBEAT_MSG_TYPE_ACK);
    }

    return iRet;
}
