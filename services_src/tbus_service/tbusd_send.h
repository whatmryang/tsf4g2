/** @file $RCSfile: tbusd_send.h,v $
  Invoked function header file for tsf4g-tbus relay module
  $Id: tbusd_send.h,v 1.7 2009-08-21 03:49:17 jacky Exp $
@author $Author: jacky $
@date $Date: 2009-08-21 03:49:17 $
@version $Revision: 1.7 $
@note Editor: Vim 6.3, Gcc 4.0.2, tab=4
@note Platform: Linux
*/


#ifndef TBUSD_SEND_H
#define TBUSD_SEND_H

#include "tbusd.h"

/** 向各relay通道发送数据包
@param[in] a_pstRunEnv 服务器运行上下文环境
@param[out] a_iMaxSendPkg 最大可以发送数据包
@return 处理成功返回0, 否则返回非零值
*/
int tbusd_proc_send(IN LPRELAYENV a_pstRunEnv, IN int a_iMaxSendPkg);

/** 在relay通道上发送数据
  @param[in] a_pstRunEnv -- 服务器运行上下文环境
  @param[in] a_pstRelay relay通道数据结构指针
  @param[out] a_iMaxSendPkg 最大可以发送数据包

  @retval 0 -- success
  @retval !0 -- failed

  @note
*/
int tbusd_send_pkg(IN LPRELAYENV a_pstRunEnv, IN LPRELAY a_pstRelay, IN int a_iMaxSendPkg) ;


/**
  @brief 发送上次剩余没有发送出去的tbusd消息消息

  @param[in] a_pstConn -- relay 通道
  @param[in] a_pstChannel -- a_pstChannel information
  @param[in,out] a_pstRelay -- relay通道

  @retval 0 -- success, no partial data remained or sent partial data successfully
  @retval !0 -- failed, sent partial data failed

  @note
*/
int tbusd_send_remain_data(IN LPCONNNODE a_pstConn,IN LPTBUSCHANNEL a_pstChannel, IN LPRELAY a_pstRelay) ;




/**
  @brief 从通道中取出一个准备发送的tbus消息,但此消息仍保留在TBUSCHANNEL中
  @param[in] a_pstChannel -- TBUSCHANNEL information
  @param[in] a_pstConn -- relay 通道
  @param[in,out] a_pstRelay  

  @retval 0 -- success, get data ok
  @retval !0 -- failed

  @note
*/
int tbusd_send_one_pkg( IN const TBUSCHANNEL *a_pstChannel, IN LPCONNNODE a_pstConn, 
					   IN LPRELAY a_pstRelay) ;


/**
@brief tbusd消息体已经发送出去,将发送数据的get指针移至下一个消息

@param[in] a_pstChannel -- TBUSCHANNEL information
@retval 0 -- success, stored ok
@retval !0 -- failed

@note
*/
#define TBUSD_ADJUST_SENDING_CHANNEL_POS(a_pstSendMng,  a_pstChannel) \
do																	  \
{																	  \
	CHANNELVAR *pstVar = NULL ;										  \
	LPTBUSHEAD pstHead;												  \
	int iPkgLen;													  \
	int iRet = 0;													  \
	assert(NULL != a_pstChannel);									  \
	assert(NULL != a_pstSendMng);									  \
	pstVar = TBUS_CHANNEL_VAR_GET(a_pstChannel);					  \
	if (pstVar->dwGet == pstVar->dwTail) /*检查是否已经移到队列末尾*/	  \
	{																  \
		break;														  \
	}																  \
																	  \
	/*修改Get移动指针*/												   \
	if (NULL == a_pstSendMng->pstSendHead)								\
	{																	\
		break;															\
	}																	\
	pstHead = a_pstSendMng->pstSendHead;								\
	iPkgLen = pstHead->bHeadLen + ntohl(pstHead->iBodyLen);				\
	TBUS_MOVETO_NEXT_PKG(iRet, iPkgLen, a_pstChannel->pstHead, pstVar->dwSize, pstVar->dwGet, pstVar->dwTail);	\
	if (!pstVar->chGStart)												\
	{																	\
		pstVar->chGStart = 1;											\
	}																	\
																		\
	/*设计更改：缺省tbusd通过对发送出去数据包序列号确认提供可靠数据传输，但应部分业务需求希望tbusd提供提供容许丢包的 \
	的版本，因此tbusd的配置中增加一个配置，通过这个配置控制是否需要对发送数据包作确认，如果不需要确认则除移动get指针外\ 
	直接移动head指针*/													\
	assert(NULL != gs_stRelayEnv.pstConf);								\
	if (0 == gs_stRelayEnv.pstConf->iNeedAckPkg)						\
	{																	\
		/*不需要对发送的数据包做确认，直接将Head指针移至Get指针处*/		\
		tlog_trace(g_ptRelayLog, 0,0, "Tbusd need not ack sended pkg," \
			" so just move head position to get position.");			\
		pstVar->dwHead = pstVar->dwGet;									\
	}																	\
}while (0)




#endif /**< TBUSD_SEND_H */

