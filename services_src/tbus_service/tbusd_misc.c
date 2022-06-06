/** @file $RCSfile: tbusd_misc.c,v $
  general description of this module
  $Id: tbusd_misc.c,v 1.13 2009-05-14 06:58:50 jacky Exp $
@author $Author: jacky $
@date $Date: 2009-05-14 06:58:50 $
@version $Revision: 1.13 $
@note Editor: Vim 6.3, Gcc 4.0.2, tab=4
@note Platform: Linux
*/


#include <assert.h>
#include "tbus/tbus.h"
#include "tbus/tbus_channel.h"
#include "tbusd_misc.h"
#include "tbusd_desc.h"
#include "tbusd_log.h"
#include "tbus/tbus_log.h"





LPRELAY trelay_channel_get (
  IN const TBUSADDR a_iSrcAddr,
  IN const TBUSADDR a_iDstAddr,
  IN RELAY *a_pptRelay[],
  IN const unsigned int a_iRelayCnt
)
{
	unsigned int i = 0 ;

	assert(NULL != a_pptRelay);

	for ( i=0; i<a_iRelayCnt; i++ )
	{
		LPTBUSCHANNEL pstChl = &(a_pptRelay[i]->stChl);

		if (!TBUSD_RELAY_IS_ENABLE(a_pptRelay[i]))
		{
			/* __log4c_category_trace ( g_ptRelayLog, "relay_channel_get, relay info disable %i", i ) ; */
			continue ;
		}

		tlog_trace(g_ptRelayLog,a_pptRelay[i]->iID, TBUSD_LOGCLS_RELAYID,"src %i, dst %i channel(src:%d, dst:%d) releay status %d\n",
			a_iSrcAddr, a_iDstAddr, pstChl->pstHead->astAddr[pstChl->iRecvSide],
			pstChl->pstHead->astAddr[pstChl->iSendSide], a_pptRelay[i]->dwRelayStatus);
		if ((a_iSrcAddr == pstChl->pstHead->astAddr[pstChl->iRecvSide]) &&
			(a_iDstAddr == pstChl->pstHead->astAddr[pstChl->iSendSide]))
		{
			break;
		}
	}/*for ( i=0; i<(signed int)a_iRelayCnt; i++ )*/

	if (i >= a_iRelayCnt)
	{
		return NULL;
	}

	return a_pptRelay[i] ;
}

/** 通过连接管理器接受数据
@param[in] pstNode relay服务器上下文环境
*/
int tbusd_recv(IN LPCONNNODE pstNode, IN struct epoll_event* a_ptEvent)
{
	int iLen;

	assert(NULL != pstNode);
	assert(NULL != a_ptEvent);

	iLen = tnet_recv(pstNode->iFd, &pstNode->szRecvBuff[pstNode->iBuff],
		pstNode->iRecvBuffSize - pstNode->iBuff, 0);
	if (0 > iLen)
	{
		return TBUS_ERROR;
	}else if ((0 == iLen) && (0 == (pstNode->iRecvBuffSize - pstNode->iBuff)))
	{
		return TBUS_ERROR;
	}

	pstNode->iBuff += iLen;

	return TBUS_SUCCESS;
}

int tbusd_send_all(IN LPCONNNODE pstNode, IN const char *a_pszBuff, IN int a_iBuff)
{
	int iLen;

	assert(NULL != pstNode);
	assert(NULL != a_pszBuff);
	assert(0 < a_iBuff);

	iLen = tnet_send(pstNode->iFd, a_pszBuff, a_iBuff, 0);
	if (iLen != a_iBuff)
	{
		tbusd_log_msg ( g_ptRelayLog,TBUSD_PRIORITY_ERROR,   "tbusd_send, socket %i, errno:%u, %s, a_iBuffLen %i, iWrite %i",
			pstNode->iFd, TBUSD_GET_SOCKET_ERROR_CODE, tbusd_get_error_string(TBUSD_GET_SOCKET_ERROR_CODE), a_iBuff, iLen ) ;
		return TBUS_ERROR;
	}

	return TBUS_SUCCESS;
}

int tbusd_send( IN const int a_iSocket, IN const char *a_pszBuff, IN const int a_iBuffLen )
{
	int iWrite = 0,
		iLen = 0,
		iErrno = 0 ;

	while (iWrite < a_iBuffLen)
	{
		iLen = send(a_iSocket, a_pszBuff + iWrite, a_iBuffLen - iWrite, 0);
		if (iLen < 0)
		{
			iErrno= errno;
			if (iErrno == EINTR || iErrno == EAGAIN)
			{
				return iWrite;
			}
			else
			{
				tbusd_log_msg ( g_ptRelayLog,TBUSD_PRIORITY_ERROR,   "tbusd_send, socket %i, errno:%u, %s, a_iBuffLen %i, iWrite %i",
					a_iSocket, TBUSD_GET_SOCKET_ERROR_CODE, tbusd_get_error_string(TBUSD_GET_SOCKET_ERROR_CODE), a_iBuffLen, iWrite ) ;
				return -1;
			}
		}

		iWrite += iLen;
	}

	return iWrite;
}

int tbusd_read_tbushead(OUT LPTBUSHEAD a_pstHead, IN const char *a_pszNet, IN int a_iNetLen)
{
	int iHeadLen;
	int iHeadVer;


	int iRet = TBUS_SUCCESS;

	assert(NULL != a_pstHead);
	assert(NULL != a_pszNet);


	if (a_iNetLen < TBUS_MIN_NET_LEN_TO_GET_HEADLEN)
	{
		tbusd_log_msg(g_ptRelayLog, TBUSD_PRIORITY_TRACE, "net msg len %d is  less than min length(%d) to get headlen",
			a_iNetLen,  TBUS_MIN_NET_LEN_TO_GET_HEADLEN) ;
		return 0;
	}

	iHeadVer = (int)(unsigned char)a_pszNet[0];
	iHeadLen = (int)(unsigned char)a_pszNet[TBUS_HEAD_LEN_NET_OFFSET];
	if (0 >= iHeadLen)
	{
		tbusd_log_msg(g_ptRelayLog, TBUSD_PRIORITY_FATAL, "invalid tbus head len (%d), version of head(%d) msglen %d",
			iHeadLen, iHeadVer, a_iNetLen);
		return -1;
	}
	if (a_iNetLen < iHeadLen)
	{
		tbusd_log_msg(g_ptRelayLog, TBUSD_PRIORITY_TRACE, "net msg len %d is  less than headlen(%d)",
			a_iNetLen,  iHeadLen) ;
		return 0;
	}

	iRet = tbus_decode_head(a_pstHead, (char *)a_pszNet, a_iNetLen, 0);
	if (TBUS_SUCCESS != iRet)
	{
		tbusd_log_msg(g_ptRelayLog, TBUSD_PRIORITY_ERROR, "failed to unpack tbus head,iRet %x", iRet);
		return -1;
	}

	return 1;
}

const char *tbusd_get_error_string(int icode)
{
#if defined(_WIN32) || defined(_WIN64)
	static char msgBuf[1024];

	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		icode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		msgBuf, 1024, NULL );

	return msgBuf;
#else
	return strerror(icode);
#endif
}

int tbusd_parse_url_inet_addr(const char *a_pszUrl, TNETADDR *a_pstAddr)
{
	int iRet = 0;
	TNETOPT stOpt;
	const TNETPROTODEF* pstProto;

	assert(NULL != a_pszUrl);
	assert(NULL != a_pstAddr);

	if( -1==tnet_get_opt(&stOpt, a_pszUrl) )
	{
		tbusd_log_msg ( g_ptRelayLog,TBUSD_PRIORITY_ERROR,   "tnet_get_opt failed to parse  URL:%s",
			a_pszUrl) ;
		return -1;
	}
	pstProto = tnet_find_proto(stOpt.szProto);
	if (NULL == pstProto)
	{
		tbusd_log_msg(g_ptRelayLog, TBUSD_PRIORITY_ERROR, "tnet_find_proto failed  by URL: %s",
			a_pszUrl) ;
		return -2;
	}

	//分析地址
	switch(pstProto->iID)
	{
	case TNET_ID_TCP:
	case TNET_ID_UDP:
		{
			if (0 != tnet_str2inet(stOpt.szAddr, &a_pstAddr->stIn))
			{
				tbusd_log_msg ( g_ptRelayLog,TBUSD_PRIORITY_ERROR,   "tnet_str2inet failed to get inet address info from address URL:%s",
					stOpt.szAddr) ;
				iRet = -3;
			}
		}
		break;
	default:
		tbusd_log_msg ( g_ptRelayLog,TBUSD_PRIORITY_ERROR,   "unsupported protocl(%s) by address URL:%s",
			pstProto->pszName,stOpt.szAddr) ;
		iRet = -3;
		break;
	}/*switch(pstProto->iID)*/


	if (0 == iRet)
	{
		if (0 == a_pstAddr->stIn.sin_port)
		{
			tbusd_log_msg ( g_ptRelayLog,TBUSD_PRIORITY_ERROR,   "get invalid listening port(%d) from URL:%s",
				a_pstAddr->stIn.sin_port,a_pszUrl) ;
			iRet = -4;
		}
	}

	return iRet;
}

