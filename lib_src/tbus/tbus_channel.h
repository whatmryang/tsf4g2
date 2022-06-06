/** @file $RCSfile: tbus_channel.h,v $
  Channel function header file for tsf4g-tbus module
  $Id: tbus_channel.h,v 1.6 2009-08-27 02:25:14 jacky Exp $
@author $Author: jacky $
@date $Date: 2009-08-27 02:25:14 $
@version $Revision: 1.6 $
@note Editor: Vim 6.3, Gcc 4.0.2, tab=4
@note Platform: Linux
*/


#ifndef _TBUS_CHANNEL_H
#define _TBUS_CHANNEL_H

#include "tbus/tbus.h"
#include "tbus_kernel.h"
#include "tbus_log.h"


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	/**
	@brief
	@retval 0 --
	@retval !0 --

	@note
	*/
	int tbus_enable_addr (
		IN const TBUSADDR a_iLocalAddr,
        IN const TBUSADDR a_iPeerAddr,
		IN  TBUSSHMCHANNELCNF *a_ptRoute,
		IN int a_iHandle,
		IN LPTBUSSHMGCIMHEAD a_pstHead
		) ;


	/**
	@brief
	@retval 0 --
	@retval !0 --
	@note
	*/
	int tbus_attach_channel (
		IN const TBUSADDR a_iAddr,
		IN  TBUSSHMCHANNELCNF *a_ptRoute,
		INOUT TBUSCHANNEL *a_ptChannel,
		IN LPTBUSSHMGCIMHEAD a_pstHead
		) ;



int tbus_channel_get(int a_iShmkey, LPTBUSCHANNEL a_pstChannel);


int tbus_push_channel_pkgv(IN TBUSCHANNEL* a_pstChannel, IN LPTBUSHEAD a_pstHead,
						   IN const struct iovec *a_ptVector, IN const int a_iVecCnt) ;

int	tbus_get_channel_pkgv(IN TBUSCHANNEL* a_pstChannel, OUT LPTBUSHEAD a_pstHead,
						  IN char *a_pszData, INOUT size_t *a_piLen);


int	tbus_peek_channel_pkgv(IN TBUSCHANNEL* a_pstChannel, OUT LPTBUSHEAD a_pstHead,
						   IN const char **a_ppszData, OUT size_t *a_piLen);


int tbus_delete_channel_headpkg(IN TBUSCHANNEL* a_pstChannel);

int tbus_send_heartbeat_msg(IN int a_iHandle, IN TBUSADDR a_iPeerAddr, IN int a_iFlag, IN int a_iMsgType);

/** 读取数据通道消息输入队列指定位置所在消息的序列号
@param[out] a_iSeq	获取序列号
@param[in] a_pstChannel tbus通道数据结构的指针
@param[in] a_dwPos 消息在队列中的起始位置
*/
#define TBUS_GET_PKGSEQ_BY_POS(a_iSeq, a_pstChannel, a_dwPos)	\
do{																\
	LPCHANNELVAR pstVar;										\
	const char *pszQueue;										\
	LPTBUSHEAD pstHead;											\
																\
	assert(NULL != a_pstChannel);								\
																\
	pstVar = TBUS_CHANNEL_VAR_GET(a_pstChannel);				\
	pszQueue = TBUS_CHANNEL_QUEUE_GET(a_pstChannel);			\
																\
	if (a_dwPos == pstVar->dwTail)							\
	{/*队列为空，返回下一个待接收消息的序列号*/					\
		a_iSeq = pstVar->iSeq;								\
		break;													\
	}															\
																\
	TBUS_CHECK_QUEUE_HEAD_VAR(pszQueue, pstVar->dwSize, a_dwPos)	\
	if (a_dwPos == pstVar->dwTail)							\
	{/*队列为空，返回下一个待接收消息的序列号*/					\
		a_iSeq = pstVar->iSeq;								\
		break;													\
	}															\
																\
	pstHead = (LPTBUSHEAD)(pszQueue + a_dwPos);				\
	a_iSeq = tdr_ntoh32(pstHead->iSeq);								\
}while (0)


#define TBUS_MOVETO_NEXT_PKG(a_iRet, a_iPkgLen, a_pstChlHead, a_dwSize, a_dwHead, a_dwTail) \
do{																			\
	int iDataLen = 0;														\
    int iPkgAlignLen = 0;\
\
	if (a_dwHead == a_dwTail)												\
	{																		\
		break;																\
	}																		\
	tbus_log(TLOG_PRIORITY_TRACE,"queue size:%d head:%d tail:%d ",			\
		a_dwSize, a_dwHead, a_dwTail);										\
	iDataLen = a_dwTail - a_dwHead;											\
	if (iDataLen < 0)														\
	{																		\
		iDataLen += a_dwSize;												\
	}																		\
    iPkgAlignLen = TBUS_CALC_ALIGN(a_iPkgLen, a_pstChlHead->dwAlignLevel);\
	if (iDataLen < iPkgAlignLen)												\
	{																		\
		tbus_log(TLOG_PRIORITY_FATAL,"data length(%d) in queue is less than the length(%d) of tbus pkg",	\
			iDataLen, iPkgAlignLen);											\
		a_iRet = TBUS_ERR_CHANNEL_CONFUSE;									\
		break;																\
	}																		\
	if ((0 > iPkgAlignLen) || (iPkgAlignLen > (int)a_dwSize))					\
	{																		\
		tbus_log(TLOG_PRIORITY_FATAL,"queue size:%d head:%d tail:%d ",		\
			a_dwSize, a_dwHead, a_dwTail);									\
		tbus_log(TLOG_PRIORITY_FATAL,"invalid pkg length(%d) in queue, "	\
			"its should be bigger than 0 and less than queuesize(%d),so clear remain data",		\
			iPkgAlignLen, a_dwSize);											\
        a_dwHead = a_dwTail;\
		a_iRet = TBUS_ERR_CHANNEL_CONFUSE;									\
		break;																\
	}																		\
	a_dwHead = (a_dwHead + iPkgAlignLen) % a_dwSize; /*修改头指针，以将数据包移除*/					\
	tbus_log(TLOG_PRIORITY_TRACE,"queue size:%d head:%d tail:%d ",			\
		a_dwSize, a_dwHead, a_dwTail);										\
}while(0);


#ifdef __cplusplus
}
#endif


#endif /**< _TBUS_CHANNEL_H */

