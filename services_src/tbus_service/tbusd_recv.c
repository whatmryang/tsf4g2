/** @file $RCSfile: tbusd_recv.c,v $
general description of this module
$Id: tbusd_recv.c,v 1.19 2009-11-09 05:54:28 jacky Exp $
@author $Author: jacky $
@date $Date: 2009-11-09 05:54:28 $
@version $Revision: 1.19 $
@note Editor: Vim 6.3, Gcc 4.0.2, tab=4
@note Platform: Linux
*/


#include "tbusd_log.h"
#include "tbusd_recv.h"

#include "tbusd_misc.h"
#include "tbus/tbus_channel.h"
#include "tbus/tbus_dyemsg.h"


extern TAPPCTX g_stAppCtx;
extern RELAYENV gs_stRelayEnv;

int tbusd_connection_accept (IN LPRELAYENV a_pstRunEnv, IN const int a_iFd, IN struct epoll_event* a_ptEvent ) ;



int tbusd_proc_pkg_recved(IN LPRELAY a_pstRelay, IN LPTBUSRELAYCONNSTAT a_pstRelayStat,
						  IN LPTBUSHEAD a_pstHead, IN const char *a_pszPkg, IN int a_iPkgLen)
{
	unsigned int iSeq = 0;
	int	iRet = 0;
	int	iPkgLen = 0 ;
	LPTBUSCHANNEL pstChannel;
	LPCONNNODE pstConn;

	assert(NULL != a_pstRelay);
	assert(NULL != a_pstRelayStat);
	assert(NULL != a_pszPkg);
	assert(NULL != a_pstHead);
	pstConn = (LPCONNNODE)a_pstRelay->pConnNode;
	assert(NULL != pstConn);

	iSeq = a_pstHead->iSeq;
	pstChannel = &a_pstRelay->stChl;
	tlog_trace(g_ptRelayLog,a_pstRelay->iID, TBUSD_LOGCLS_RELAYID, "tbusd_proc_pkg_recved, iSeq %u cmd:%d shFlag %i",
		iSeq, a_pstHead->bCmd, a_pstHead->bFlag ) ;

	switch(a_pstHead->bCmd)
	{
	case TBUS_HEAD_CMD_TRANSFER_DATA:
		{
			if (a_pstHead->bFlag & TBUS_HEAD_FLAG_WITH_ACK)
			{
				tbusd_syn_seq(pstChannel, a_pstHead->iRecvSeq);
				a_pstHead->bFlag &= ~TBUS_HEAD_FLAG_WITH_ACK;
			}
			iRet = tbusd_push_pkg(pstChannel, a_pstHead, a_pszPkg, a_iPkgLen);
			if (0 != iRet)
			{
				if ((int)(TBUS_ERR_CHANNEL_FULL) == iRet)
				{
					tbusd_log_msg(g_ptRelayLog, TBUSD_PRIORITY_ERROR,
						"tbusd_proc_pkg_recved, failed to push pkg<seq:%u> to channel on seockt %d of relay %d \
						for reccv channel is full!!! (iRet:%d)",
						iSeq , pstConn->iFd, a_pstRelay->iID, iRet) ;
				}else
				{
					tbusd_log_msg(g_ptRelayLog, TBUSD_PRIORITY_FATAL,
						"tbusd_proc_pkg_recved, failed to push pkg<seq:%u> to channel on seockt %d of relay %d, \
						and relay cannot work normally so  close socket. iRet:%d ",
						iSeq , pstConn->iFd, a_pstRelay->iID, iRet) ;
					pstConn->bNeedFree = TBUSD_TRUE;	/*断开连接*/
				}
				break;
			}/*if (0 != iRet)*/

			pstConn->iRecvPkg++;
			pstConn->iRecvByte += iPkgLen;
			if (pstConn->iRecvPkg >= TBUSD_PKG_NUM_RECVED_PERACKSEQ || pstConn->iRecvByte >= TBUSD_PKG_BYTES_RECVED_PERACKSEQ)
			{
				pstConn->iRecvPkg = 0;
				pstConn->iRecvByte = 0;
				pstConn->bNeedSynSeq = TBUSD_TRUE;
				tlog_debug( g_ptRelayLog,a_pstRelay->iID, TBUSD_LOGCLS_RELAYID,
					"tbusd_proc_pkg_recved, sed synseq<%u> flag  on seockt %d of relay %d",
					iSeq , pstConn->iFd, a_pstRelay->iID) ;
			}
		}
		break;
	case TBUS_HEAD_CMD_ACKSEQ:
		{
			tbusd_syn_seq(pstChannel, iSeq);
			a_pstRelayStat->dwRecvSynPkgCount++;
		}
		break;
	default:
		tlog_error( g_ptRelayLog,a_pstRelay->iID, TBUSD_LOGCLS_RELAYID,
			"tbusd_proc_pkg_recved, recv unsupported  pkg Cmd<%d> on seockt %d of relay %d, so discard it",
			a_pstHead->bCmd , pstConn->iFd, a_pstRelay->iID) ;
		break;
	}/*switch(a_pstHead->bCmd)*/

	return iRet;
}


int tbusd_push_pkg ( IN const TBUSCHANNEL *a_ptChannel, IN LPTBUSHEAD a_pstHead, IN const char *a_pszPkg, IN int a_iPkgLen )
{
	CHANNELVAR *pstVar = NULL ;
	int iRet = 0 ;
	int iSubSeq;
	int iPkgSeq;
	struct iovec astVectors[1];

	assert(NULL != a_ptChannel);
	assert(NULL != a_pstHead);
	assert(NULL != a_pszPkg);

	pstVar = TBUS_CHANNEL_VAR_PUSH(a_ptChannel);
	iPkgSeq = a_pstHead->iSeq;
	iSubSeq = iPkgSeq  - pstVar->iRecvSeq;
	tbusd_log_msg(g_ptRelayLog, TBUSD_PRIORITY_TRACE,"channel_auth_push, RecvSeq %u - pstVar->iRecvSeq %u = %d",
		a_pstHead->iSeq, pstVar->iRecvSeq, iSubSeq) ;

	assert(NULL != gs_stRelayEnv.pstConf);
	if (gs_stRelayEnv.pstConf->iNeedAckPkg)
	{
		if (iPkgSeq != (pstVar->iRecvSeq + 1))
		{/*后续收到的数据包的序列号只能比最后一个收到的数据包的序列号大1*/
			tbusd_log_msg(g_ptRelayLog, TBUSD_PRIORITY_FATAL,"channel_auth_push, RecvSeq %u, pstVar->iRecvSeq %u not match",
				a_pstHead->iSeq, pstVar->iRecvSeq ) ;
			return -1;
		}
	}else
	{
		if (iSubSeq <= 0)
		{/*后续收到的数据包的序列号只能比最后一个收到的数据包的序列号大*/
			tbusd_log_msg(g_ptRelayLog, TBUSD_PRIORITY_FATAL,"channel_auth_push, RecvSeq %u, pstVar->iRecvSeq %u not match",
				a_pstHead->iSeq, pstVar->iRecvSeq ) ;
			return -1;
		}
	}

    if (a_pstHead->bFlag & TBUS_HEAD_FLAG_WITH_TIMESTAMP)
    {
        a_pstHead->stTimeStamp.llSec = g_stAppCtx.stCurr.tv_sec;
        a_pstHead->stTimeStamp.dwUSec = (unsigned int)g_stAppCtx.stCurr.tv_usec;
    }

	astVectors[0].iov_base = (void *)a_pszPkg;
	astVectors[0].iov_len = a_iPkgLen;
	iRet = tbus_push_channel_pkgv((LPTBUSCHANNEL)a_ptChannel, a_pstHead, &astVectors[0], 1);
	if (iRet < 0)
	{
		tbusd_log_msg(g_ptRelayLog, TBUSD_PRIORITY_FATAL, "channel_auth_push, channelPush() failed %i", iRet ) ;
		return iRet;
	}

	if(a_pstHead->bFlag & TBUS_HEAD_FLAG_TACE)
	{
		tbus_log_dyedpkg(a_pstHead, "Recv(tbusd)");
	}

	pstVar->iRecvSeq = iPkgSeq;

	if (!pstVar->chPStart)
	{
		pstVar->chPStart = 1;
	}

	return 0;
}




/**从链接收数据缓冲区中接收数据包
*@param[in] a_pstRunEnv 服务器运行上下文
*@param[in] a_pstConn 链接数据结构指针
*@param[in] a_pstRelay relay通道数据结构
*return 成功返回0，失败返回非零值
*/
int tbusd_recv_pkg(IN LPRELAYENV a_pstRunEnv, IN LPCONNNODE a_pstConn, IN LPRELAY a_pstRelay)
{
	int iRet = TBUS_SUCCESS;
	int iPkgLen;
	unsigned int dwPkgCount = 0;
	int	iDealLen = 0;
	TBUSHEAD stHead;
	LPTBUSRELAYCONNSTAT pstRelayStat;
	LPTBUSDSTAT pstStat ;
	int iPkg;
	char *pszData;

	assert(NULL != a_pstRunEnv);
	assert(NULL != a_pstConn);
	assert(NULL != a_pstRelay);
	assert(NULL != a_pstRunEnv->pstStat);

	assert(NULL != a_pstRunEnv->pstConf);

	tlog_trace(g_ptRelayLog,a_pstRelay->iID, TBUSD_LOGCLS_RELAYID,
		"tbusd_recv_pkg, recv data len<%d> on socket<%d> for relay<%d>",
		a_pstConn->iBuff, a_pstConn->iFd, a_pstRelay->iID);

	if (0 >= a_pstRunEnv->dwRelayCnt )
	{
		tlog_trace(g_ptRelayLog,a_pstRelay->iID, TBUSD_LOGCLS_RELAYID,
			"no relay channel(%d),so need not recv msg", a_pstRunEnv->dwRelayCnt);
		return 0;
	}

	iDealLen = 0;
	pstStat = a_pstRunEnv->pstStat;
	pstRelayStat = &pstStat->stRelayConnStatList.astRelayConnStat[a_pstRelay->iID];
	while (0 < a_pstConn->iBuff)
	{
		iPkg = tbusd_read_tbushead(&stHead, a_pstConn->szRecvBuff + iDealLen, a_pstConn->iBuff);
		if (0 > iPkg)
		{
			tlog_error( g_ptRelayLog,a_pstRelay->iID, TBUSD_LOGCLS_RELAYID, "failed to get pkg head in socket %d, so close socket on relay(%d)",
				a_pstConn->iFd, a_pstRelay->iID);
			a_pstConn->bNeedFree = TBUSD_TRUE;
			iRet = -1;
			break;
		}else if (0 == iPkg)
		{
			break;
		}

		/*判断是否收到的整个数据包*/
		iPkgLen = stHead.bHeadLen + stHead.iBodyLen;
		if ((iPkgLen <= 0) || (iPkgLen > a_pstConn->iRecvBuffSize))
		{
			tbusd_log_msg(g_ptRelayLog, TBUSD_PRIORITY_FATAL,
				"tbusd_recv_pkg, invalid iPkgLen<%d> is less than sizeofHead<%d> or bigger than RecvBuffSize<%d>, so close socket %d",
				iPkgLen, (int)sizeof(TBUSHEAD), a_pstConn->iRecvBuffSize, a_pstConn->iFd);
			a_pstConn->bNeedFree = TBUSD_TRUE;	/*断开连接*/
			iRet = -1;
			break;
		}
		if (iPkgLen > a_pstConn->iBuff)
		{
			break;
		}

		pszData = a_pstConn->szRecvBuff + iDealLen + stHead.bHeadLen;
		iRet = tbusd_proc_pkg_recved(a_pstRelay, pstRelayStat, &stHead, pszData, stHead.iBodyLen);
		if (0 != iRet)
		{
			tlog_error( g_ptRelayLog,a_pstRelay->iID, TBUSD_LOGCLS_RELAYID,
				"tbusd_recv_pkg, failed to process recved pkg(seq:%d, cmd:%d) on socket<%d> for relay<%d>, iRet : %d",
				stHead.iSeq, stHead.bCmd, a_pstConn->iFd, a_pstRelay->iID, iRet);
			a_pstConn->bNeedFree = TBUSD_TRUE;
			break;
		}
		iDealLen += iPkgLen;
		a_pstConn->iBuff -= iPkgLen;
		pstRelayStat->dwRecvPkgCount++;
		pstRelayStat->ullRecvPkgBytes += iPkgLen;
		pstStat->ullRecvPkgBytes += iPkgLen;
		dwPkgCount++;
	}/*while (0 < a_pstConn->iBuff)*/

    if (0 == iRet)
    {
        pstStat->dwRecvPkgCount += dwPkgCount;
        if (dwPkgCount > pstRelayStat->dwMaxRecvCountPerloop)
        {
            pstRelayStat->dwMaxRecvCountPerloop = dwPkgCount;
        }
        /*将剩余没有处理的数据移到接收缓冲区的前面*/
        if ((0 < a_pstConn->iBuff)  && (0 < iDealLen))
        {
            /* TODO: 加强保护，防止越界 */
            memmove(a_pstConn->szRecvBuff, a_pstConn->szRecvBuff + iDealLen, a_pstConn->iBuff);
        }
    }


	return iRet;
}



int tbusd_proc_recv(IN TAPPCTX *a_pstAppCtx, IN LPRELAYENV a_pstRunEnv, IN int *a_piMaxSendPkg)
{
	int iRet = TBUS_ERROR ;
	struct epoll_event events[TBUSD_MAX_EVENTS];
	LPTBUSD pstConf;
	LPTMEMPOOL pstMemPool;
	int iEvents = 0;
	int i = 0;
	CONNNODE *ptNode = NULL ;
	static int s_iSendPkg = TBUS_DEFAULT_SENT_CNT;
	LPRELAY pstRelay;
	int iLen;
	time_t tNow;


	assert(NULL != a_pstRunEnv);
	assert(NULL != a_piMaxSendPkg);
	assert(NULL != a_pstRunEnv->pstConf);
	assert(NULL != a_pstRunEnv->ptMemPool);

    TOS_UNUSED_ARG(a_pstAppCtx);

    /* commented to avoid CPU 100%
	if (0 >= a_pstRunEnv->dwRelayCnt )
	{
		tbusd_log_msg(g_ptRelayLog, TBUSD_PRIORITY_TRACE,
			"no relay channel(%d),so need not recv msg", a_pstRunEnv->dwRelayCnt);
		return 0;
	}
    */

	pstConf = a_pstRunEnv->pstConf;
	pstMemPool = a_pstRunEnv->ptMemPool;


	/*从网络上接收消息*/
	iEvents = epoll_wait(a_pstRunEnv->iEpoolFd, events, TBUSD_MAX_EVENTS, a_pstRunEnv->iEpollWaitTime);
	a_pstRunEnv->iEpollWaitTime = 0;

	/*流量控制*/
	if (0 >= iEvents)
	{
		s_iSendPkg += TBUS_STEP_SENT_CNT;
	}
	if (TBUS_MAX_SENT_CNT >= s_iSendPkg)
	{
		s_iSendPkg = TBUS_DEFAULT_SENT_CNT * 2;
	}
	*a_piMaxSendPkg = s_iSendPkg;
	if (0 >= iEvents)
	{
		return 0;
	}


	time(&tNow);

	/*接受数据*/
	for ( i=0; i<iEvents; i++ )
	{
		ptNode = (CONNNODE *) tmempool_get(pstMemPool, events[i].data.fd ) ;
		if ( NULL == ptNode )
		{
			tbusd_log_msg ( g_ptRelayLog,TBUSD_PRIORITY_ERROR,   "tmempool_get() failed by index %i", events[i].data.fd ) ;
			continue ;
		}


		/* 监听模式，if new connection, accept it */
		if (RELAY_FD_LISTEN_MODE == ptNode->dwType)
		{
			iRet = tbusd_connection_accept(a_pstRunEnv, ptNode->iFd, events+i );
			continue;
		}


		if (!ptNode->bIsConnected)
		{
			/*连接中,检测连接*/
			iRet = tbusd_connecting_check(a_pstRunEnv, ptNode, events+i );
			if (0 != iRet)
			{
				tbusd_destroy_conn(a_pstRunEnv, ptNode, events+i );
			}
			continue;
		}/*if (ptNode->bConnecting)*/

		/*接收relay 通道连接上的数据*/
		if (0 >= (ptNode->iRecvBuffSize - ptNode->iBuff))
		{
			tbusd_log_msg ( g_ptRelayLog,TBUSD_PRIORITY_FATAL,  "no recv buffer(len:%d), close package from %s:%d !\n",
				ptNode->iRecvBuffSize - ptNode->iBuff, inet_ntoa(ptNode->stAddr.sin_addr), ntohs(ptNode->stAddr.sin_port));
			tbusd_destroy_conn(a_pstRunEnv, ptNode, events+i );
			continue;
		}
		iLen = epoll_recv(a_pstRunEnv->iEpoolFd, ptNode->iFd, events+i,
			ptNode->szRecvBuff + ptNode->iBuff, ptNode->iRecvBuffSize - ptNode->iBuff);
		if( iLen<=0 )
		{
			if( iLen<0 )
			{
				tbusd_log_msg ( g_ptRelayLog,TBUSD_PRIORITY_ERROR,  "peer(%s:%d) exit unnormal or connection(fd:%d) fail !need to free!error:%u  for %s\n",
					inet_ntoa(ptNode->stAddr.sin_addr), ntohs(ptNode->stAddr.sin_port), ptNode->iFd,
					TBUSD_GET_SOCKET_ERROR_CODE, tbusd_get_error_string(TBUSD_GET_SOCKET_ERROR_CODE) );
			}else
			{
				tbusd_log_msg ( g_ptRelayLog,TBUSD_PRIORITY_ERROR,  "recv len is equal to zero, may %s:%d close connection(fd:%d) or recv buffsize(%d) is zero! error:%u  for %s\n",
					inet_ntoa(ptNode->stAddr.sin_addr), ntohs(ptNode->stAddr.sin_port), ptNode->iFd,ptNode->iRecvBuffSize - ptNode->iBuff ,
					TBUSD_GET_SOCKET_ERROR_CODE, tbusd_get_error_string(TBUSD_GET_SOCKET_ERROR_CODE) );
			}
			tbusd_destroy_conn(a_pstRunEnv, ptNode, events+i );
			continue;
		}
		ptNode->iBuff += iLen;


		/*处理收到的数据包*/
		if (NULL == ptNode->pstBindRelay)
		{
			iRet = tbusd_handshake(a_pstRunEnv, ptNode);
		}else
		{
			pstRelay = ptNode->pstBindRelay;


			tlog_trace( g_ptRelayLog,pstRelay->iID, TBUSD_LOGCLS_RELAYID,  "connection type %i, events[%i]: fd=%i relay index=%d",
				ptNode->dwType, i, ptNode->iFd, pstRelay->iID);
			if (TBUSD_STATUS_ESTABLISHED != pstRelay->dwRelayStatus)
			{
				iRet = tbusd_handshake(a_pstRunEnv, ptNode);
			}else
			{
				//此relay通道 收到了数据，则更新心跳时间
				pstRelay->tLastHeartbeat = tNow;
				iRet = tbusd_recv_pkg(a_pstRunEnv, ptNode, pstRelay);
			}
		}/*if (TDR_INVALID_INDEX == ptNode->iRelayIdx)*/

		if (ptNode->bNeedFree)
		{
			tbusd_log_msg ( g_ptRelayLog,TBUSD_PRIORITY_ERROR,   "at connection[fd=%i] failed to process pkg so close connection(%d)",
				ptNode->iFd, ptNode->idx);
			tbusd_destroy_conn(a_pstRunEnv, ptNode, events+i );
		}
	}/*for ( i=0; i<iEvents; i++ )*/



	return iEvents;  /*仅仅是为了通过收包数目情况来反应忙闲,因此反应1表示有事件处理即可*/
}

int tbusd_connection_accept(IN LPRELAYENV a_pstRunEnv, IN const int a_iFd, IN struct epoll_event* a_ptEvent )
{
	int iRet = TBUS_ERROR ;
	int iSocket = 0 ;
	struct sockaddr_in sa;
	int iLen;

	LPCONNNODE pstConn;
	LPTBUSDSTAT pstStat;

	assert(NULL != a_pstRunEnv);
	assert(NULL != a_pstRunEnv->pstStat);
	pstStat = a_pstRunEnv->pstStat;


	iLen = sizeof(sa) ;
	iSocket = epoll_accept(a_pstRunEnv->iEpoolFd, a_iFd, a_ptEvent, (struct sockaddr *)&sa, &iLen);
	if (0 > iSocket)
	{
		pstStat->dwAcceptFailed++;
		tlog_log(g_ptRelayLog, TBUSD_PRIORITY_ERROR, 0,0, "tbusd_connection_accept, epoll_accept failed") ;
		return TBUS_ERROR ;
	}

	tbusd_log_msg(g_ptRelayLog, TBUSD_PRIORITY_DEBU, "tbusd_connection_accept, accept socket<fd:%d> from %s:%d",
		iSocket, inet_ntoa(sa.sin_addr), ntohs(sa.sin_port)) ;
	pstConn = tbusd_create_conn(a_pstRunEnv, iSocket, RELAY_FD_SERVER_MODE, EPOLLIN);
	if (NULL == pstConn)
	{
		tbusd_log_msg(g_ptRelayLog, TBUSD_PRIORITY_ERROR, "tbusd_connection_accept, failed to add socket<%d> to handle",
			iSocket) ;
		tnet_close( iSocket ) ;
		iRet = TBUS_ERROR;
	}else
	{
		memcpy(&pstConn->stAddr, &sa, sizeof(pstConn->stAddr));
	}

	return iRet ;
}


