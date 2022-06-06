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

/** ���relayͨ���������ݰ�
@param[in] a_pstRunEnv ���������������Ļ���
@param[out] a_iMaxSendPkg �����Է������ݰ�
@return ����ɹ�����0, ���򷵻ط���ֵ
*/
int tbusd_proc_send(IN LPRELAYENV a_pstRunEnv, IN int a_iMaxSendPkg);

/** ��relayͨ���Ϸ�������
  @param[in] a_pstRunEnv -- ���������������Ļ���
  @param[in] a_pstRelay relayͨ�����ݽṹָ��
  @param[out] a_iMaxSendPkg �����Է������ݰ�

  @retval 0 -- success
  @retval !0 -- failed

  @note
*/
int tbusd_send_pkg(IN LPRELAYENV a_pstRunEnv, IN LPRELAY a_pstRelay, IN int a_iMaxSendPkg) ;


/**
  @brief �����ϴ�ʣ��û�з��ͳ�ȥ��tbusd��Ϣ��Ϣ

  @param[in] a_pstConn -- relay ͨ��
  @param[in] a_pstChannel -- a_pstChannel information
  @param[in,out] a_pstRelay -- relayͨ��

  @retval 0 -- success, no partial data remained or sent partial data successfully
  @retval !0 -- failed, sent partial data failed

  @note
*/
int tbusd_send_remain_data(IN LPCONNNODE a_pstConn,IN LPTBUSCHANNEL a_pstChannel, IN LPRELAY a_pstRelay) ;




/**
  @brief ��ͨ����ȡ��һ��׼�����͵�tbus��Ϣ,������Ϣ�Ա�����TBUSCHANNEL��
  @param[in] a_pstChannel -- TBUSCHANNEL information
  @param[in] a_pstConn -- relay ͨ��
  @param[in,out] a_pstRelay  

  @retval 0 -- success, get data ok
  @retval !0 -- failed

  @note
*/
int tbusd_send_one_pkg( IN const TBUSCHANNEL *a_pstChannel, IN LPCONNNODE a_pstConn, 
					   IN LPRELAY a_pstRelay) ;


/**
@brief tbusd��Ϣ���Ѿ����ͳ�ȥ,���������ݵ�getָ��������һ����Ϣ

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
	if (pstVar->dwGet == pstVar->dwTail) /*����Ƿ��Ѿ��Ƶ�����ĩβ*/	  \
	{																  \
		break;														  \
	}																  \
																	  \
	/*�޸�Get�ƶ�ָ��*/												   \
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
	/*��Ƹ��ģ�ȱʡtbusdͨ���Է��ͳ�ȥ���ݰ����к�ȷ���ṩ�ɿ����ݴ��䣬��Ӧ����ҵ������ϣ��tbusd�ṩ�ṩ�������� \
	�İ汾�����tbusd������������һ�����ã�ͨ��������ÿ����Ƿ���Ҫ�Է������ݰ���ȷ�ϣ��������Ҫȷ������ƶ�getָ����\ 
	ֱ���ƶ�headָ��*/													\
	assert(NULL != gs_stRelayEnv.pstConf);								\
	if (0 == gs_stRelayEnv.pstConf->iNeedAckPkg)						\
	{																	\
		/*����Ҫ�Է��͵����ݰ���ȷ�ϣ�ֱ�ӽ�Headָ������Getָ�봦*/		\
		tlog_trace(g_ptRelayLog, 0,0, "Tbusd need not ack sended pkg," \
			" so just move head position to get position.");			\
		pstVar->dwHead = pstVar->dwGet;									\
	}																	\
}while (0)




#endif /**< TBUSD_SEND_H */

