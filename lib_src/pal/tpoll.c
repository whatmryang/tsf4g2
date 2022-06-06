/*
**  @file $RCSfile: tpoll.c,v $
**  general description of this module
**  $Id: tpoll.c,v 1.5 2009-11-09 05:54:39 jacky Exp $
**  @author $Author: jacky $
**  @date $Date: 2009-11-09 05:54:39 $
**  @version $Revision: 1.5 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/


#include <stddef.h>
#include <assert.h>
#include "pal/tos.h"
#include "pal/ttypes.h"
#include "pal/tsocket.h"
#include "pal/tpoll.h"

#if defined(_WIN32) || defined(_WIN64)
	#include "mswsock.h"

	#define TPOLL_BUFF_SIZE		128
	#define TPOLL_BAD_POS		-1
	#define TPOLL_KEY_CANCEL	-1

	#define TPOLLF_USED			0x0001
	#define TPOLLF_INWAITQUE		0x0002	//此FD上是否等待读数据的队列中

struct tagTPollFd
{
	TSOCKET hFile;
	LPFN_ACCEPTEX lpfnAcceptEx;
	LPFN_GETACCEPTEXSOCKADDRS lpfnAddr;
	SOCKET sAccept;
	int iState;
	int iBuff;
	int iData;
	int iOff;
	tpoll_event_t ev;
	OVERLAPPED o;
	int fStream;
	int iSrcLen;
	struct sockaddr stSrcAddr;
	char szBuff[TPOLL_BUFF_SIZE];
};

typedef struct tagTPollFd		TPOLLFD;
typedef struct tagTPollFd		*LPTPOLLFD;
typedef struct tagTPollFd		*PTPOLLFD;

struct tagTPollItem
{
	int iPrev;
	int iNext;
	int iWaitQueNext;
	int iWaitQuePrev;
	int iSeq;
	int iFlag;
	TPOLLFD stFd;
};

typedef struct tagTPollItem		TPOLLITEM;
typedef struct tagTPollItem		*LPTPOLLITEM;

struct tagTPollPort
{
	int iMaxFd;
	int iUsed;
	int iHead;
	int iTail;

	/*由于完成端口是边缘触发，调用recv不能把socket中的数据全部收完，此时需要将这些socket记录下来，
	以便下次epoll_wait事件中能返回*/
	int iWaitQueHead;
	int iFree;
	HANDLE hIOCP;
	TPOLLITEM items[1];
};

typedef struct tagTPollPort		TPOLLPORT;
typedef struct tagTPollPort		*LPTPOLLPORT;

TEPOLL_HANDLE tpoll_create (int a_iSize)
{
	int iLen;
	int iNext;
	LPTPOLLPORT pstPort;
	int i;

	if( a_iSize>TPOLL_MAX_FD )
		return (TEPOLL_HANDLE)-1;

	iLen	=	offsetof(TPOLLPORT, items) + a_iSize*sizeof(TPOLLITEM);
	if( iLen<0 )
		return (TEPOLL_HANDLE)-1;

	pstPort	=	calloc(1, iLen);
	if(!pstPort)
		return (TEPOLL_HANDLE)-1;

	pstPort->iMaxFd	=	a_iSize;
	pstPort->iUsed	=	0;

	iNext	=	TPOLL_BAD_POS;

	/* initialize the free chain. */
	for(i=a_iSize-1; i>=0; i--)
	{
		pstPort->items[i].iNext	=	iNext;
		pstPort->items[i].iPrev	=	TPOLL_BAD_POS;
		pstPort->items[i].iSeq	=	0;
		pstPort->items[i].iFlag	=	0;
		pstPort->items[i].stFd.hFile	=	INVALID_SOCKET;
		pstPort->items[i].stFd.sAccept=	INVALID_SOCKET;
		pstPort->items[i].stFd.iBuff	=	(int)sizeof(pstPort->items[i].stFd.szBuff);
		pstPort->items[i].iWaitQueNext = TPOLL_BAD_POS;
		pstPort->items[i].iWaitQuePrev = TPOLL_BAD_POS;
		iNext	=	i;
	}

	pstPort->iFree	=	0;
	pstPort->iHead	=	TPOLL_BAD_POS;
	pstPort->iTail	=	TPOLL_BAD_POS;
	pstPort->iWaitQueHead = TPOLL_BAD_POS;

	pstPort->hIOCP	=	CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if( !pstPort->hIOCP )
	{
		free(pstPort);
		return (TEPOLL_HANDLE)-1;
	}

	return (TEPOLL_HANDLE)pstPort;
}

int tpoll_start_recv_i(LPTPOLLFD a_pstFD)
{
	WSABUF stBuff;
	int iRet;
	DWORD dwFlags;

	assert( !a_pstFD->iState && !a_pstFD->lpfnAcceptEx && HasOverlappedIoCompleted(&a_pstFD->o) );

	a_pstFD->iData	=	0;
	a_pstFD->iOff		=	0;
	a_pstFD->iState	=	-1;

	stBuff.len	=	a_pstFD->iBuff;
	stBuff.buf	=	a_pstFD->szBuff;
	dwFlags	=	0;

	iRet	=	WSARecv(a_pstFD->hFile, &stBuff, 1, (LPDWORD)&a_pstFD->iData, &dwFlags, &a_pstFD->o, NULL);

	if( !iRet )
		a_pstFD->iState	=	0;
	else if( WSA_IO_PENDING==WSAGetLastError() )
		a_pstFD->iState	=	WSA_IO_PENDING;

	return iRet;
}

int tpoll_start_accept_i(LPTPOLLFD a_pstFD)
{
	int iRet;

	assert( !a_pstFD->iState && a_pstFD->lpfnAcceptEx && HasOverlappedIoCompleted(&a_pstFD->o) );

	a_pstFD->iData	=	0;
	a_pstFD->iOff		=	0;
	a_pstFD->iState	=	-1;

	a_pstFD->sAccept	=	socket(AF_INET, SOCK_STREAM, 0);
	if( -1==a_pstFD->sAccept )
		return -1;

	iRet	=	a_pstFD->lpfnAcceptEx((int)a_pstFD->hFile, a_pstFD->sAccept, a_pstFD->szBuff, 0,
		sizeof(struct sockaddr_in) + 16, sizeof(struct sockaddr_in) + 16, (LPDWORD)&a_pstFD->iData, &a_pstFD->o);

	if( !iRet )
		a_pstFD->iState	=	0;
	else if( WSA_IO_PENDING==WSAGetLastError() )
		a_pstFD->iState	=	WSA_IO_PENDING;

	return iRet;
}

int tpoll_ctl_add_i(LPTPOLLPORT a_pstPort, TSOCKET a_iFd, TPOLL_EVENT* a_pstEvent)
{
	int iRet;
	BOOL bAccept;
	int iLen;
	int iFree;
	ULONG_PTR ulKey;
	TPOLLITEM* pstItem;
	int iType;
	GUID GuidAcceptEx = WSAID_ACCEPTEX;
	GUID GuidAddrEx = WSAID_GETACCEPTEXSOCKADDRS;
	DWORD dwBytes;

	assert(a_pstPort && a_pstEvent);

	/* not support flags other than EPOLLIN */
	if( a_pstEvent->events & ~EPOLLIN )
		return -1;

	bAccept	=	0;
	iLen	=	(int)sizeof(bAccept);

	//此socket是否是监听的
	if( -1==getsockopt(a_iFd, SOL_SOCKET, SO_ACCEPTCONN, (char*)&bAccept, &iLen) )
		return -2;

	iLen	=	(int)sizeof(iType);

	if( -1==getsockopt(a_iFd, SOL_SOCKET, SO_TYPE, (char*)&iType, &iLen) )
		return -3;

	if( TPOLL_BAD_POS==a_pstPort->iFree )
		return -4;

	iFree			=	a_pstPort->iFree;
	pstItem			=	a_pstPort->items + iFree;

	memset(&pstItem->stFd, 0, sizeof(pstItem->stFd));

	pstItem->stFd.sAccept	=	INVALID_SOCKET;

	if( SOCK_STREAM==iType )
		pstItem->stFd.fStream	=	1;
	else
		pstItem->stFd.fStream	=	0;

	pstItem->stFd.iBuff	=	sizeof(pstItem->stFd.szBuff);

	if( bAccept )
	{
		WSAIoctl(a_iFd,
			SIO_GET_EXTENSION_FUNCTION_POINTER,
			&GuidAcceptEx,
			sizeof(GuidAcceptEx),
			&pstItem->stFd.lpfnAcceptEx,
			sizeof(pstItem->stFd.lpfnAcceptEx),
			&dwBytes,
			NULL,
			NULL);

		WSAIoctl(a_iFd,
			SIO_GET_EXTENSION_FUNCTION_POINTER,
			&GuidAddrEx,
			sizeof(GuidAddrEx),
			&pstItem->stFd.lpfnAddr,
			sizeof(pstItem->stFd.lpfnAddr),
			&dwBytes,
			NULL,
			NULL);

		if( !pstItem->stFd.lpfnAcceptEx || !pstItem->stFd.lpfnAddr )
			return -5;
	}

	pstItem->iSeq++;
	ulKey	=	(ULONG_PTR) TPOLL_MAKE_KEY(iFree, pstItem->iSeq);

	/* now begin initialize the tpoll item. */
	if( NULL==CreateIoCompletionPort((HANDLE)a_iFd, a_pstPort->hIOCP, ulKey, 0) )
		return -6;

	pstItem->stFd.hFile	=	a_iFd;
	a_pstEvent->ulKey	=	ulKey;
	memcpy(&pstItem->stFd.ev, a_pstEvent, sizeof(*a_pstEvent));

	if( bAccept )
		iRet	=	tpoll_start_accept_i(&pstItem->stFd);
	else
		iRet	=	tpoll_start_recv_i(&pstItem->stFd);

	if( -1==pstItem->stFd.iState )
		return -7;

	/* remove it from free chain */
	a_pstPort->iFree	=	pstItem->iNext;

	/* put it into used chain. */
	pstItem->iNext	=	a_pstPort->iHead;
	pstItem->iPrev	=	TPOLL_BAD_POS;

	pstItem->iFlag	=	TPOLLF_USED;

	if( TPOLL_BAD_POS!=a_pstPort->iHead )
		a_pstPort->items[a_pstPort->iHead].iPrev	=	iFree;
	a_pstPort->iHead = iFree;

	if( TPOLL_BAD_POS==a_pstPort->iTail )
		a_pstPort->iTail	=	iFree;

	pstItem->iWaitQueNext = TPOLL_BAD_POS;
	pstItem->iWaitQuePrev = TPOLL_BAD_POS;;
	a_pstPort->iUsed++;

	return 0;
}

int tpoll_ctl_mod_i(LPTPOLLPORT a_pstPort, TSOCKET a_iFd, TPOLL_EVENT* a_pstEvent)
{
	int iIdx;
	LPTPOLLITEM pstItem;

	iIdx	=	TPOLL_GET_IDX(a_pstEvent->ulKey);
	if( iIdx<0 || iIdx>=a_pstPort->iMaxFd )
		return -1;

	pstItem	=	a_pstPort->items + iIdx;

	if( a_pstEvent->ulKey!=pstItem->stFd.ev.ulKey || pstItem->stFd.hFile!=a_iFd)
		return -1;

	/* not implementation. */

	return 0;
}

int tpoll_ctl_del_i(LPTPOLLPORT a_pstPort, TSOCKET a_iFd, TPOLL_EVENT* a_pstEvent)
{
	int iIdx;
	LPTPOLLITEM pstItem;

	assert(a_pstPort && a_pstEvent);

	iIdx	=	TPOLL_GET_IDX(a_pstEvent->ulKey);
	if( iIdx<0 || iIdx>=a_pstPort->iMaxFd )
		return -1;

	pstItem	=	a_pstPort->items + iIdx;

	if( a_pstEvent->ulKey!=pstItem->stFd.ev.ulKey)
		return -2;
	if (pstItem->stFd.hFile!=a_iFd )
		return -3;

	CancelIo((HANDLE)a_iFd);

	/* remove it from the used chain. */
	if( TPOLL_BAD_POS==pstItem->iPrev )
		a_pstPort->iHead	=	pstItem->iNext;
	else
		a_pstPort->items[pstItem->iPrev].iNext	=	pstItem->iNext;

	if( TPOLL_BAD_POS==pstItem->iNext )
		a_pstPort->iTail	=	pstItem->iPrev;
	else
		a_pstPort->items[pstItem->iNext].iPrev	=	pstItem->iPrev;

	/* put it into free chain. */
	pstItem->iNext	=	a_pstPort->iFree;
	pstItem->iPrev	=	TPOLL_BAD_POS;
	pstItem->iFlag	=	0;
	a_pstPort->iFree	=	iIdx;
	a_pstPort->iUsed--;

	pstItem->stFd.hFile=	INVALID_SOCKET;

	return 0;
}

int tpoll_ctl(TEPOLL_HANDLE a_iTpfd, int a_iOp, TSOCKET a_iFd, TPOLL_EVENT* a_pstEvent)
{
	LPTPOLLPORT pstPort;
	int iRet;

	if( (TEPOLL_HANDLE)-1==a_iTpfd || 0==a_iTpfd )
		return -1;

	pstPort	=	(LPTPOLLPORT) a_iTpfd;

	switch( a_iOp )
	{
	case TPOLL_CTL_ADD:
		iRet	=	tpoll_ctl_add_i(pstPort, a_iFd, a_pstEvent);
		break;
	case TPOLL_CTL_MOD:
		iRet	=	tpoll_ctl_mod_i(pstPort, a_iFd, a_pstEvent);
		break;
	case TPOLL_CTL_DEL:
		iRet	=	tpoll_ctl_del_i(pstPort, a_iFd, a_pstEvent);
		break;
	default:
		return -1;
	}

	return iRet;
}


int tpoll_wait(TEPOLL_HANDLE a_iTpfd, struct epoll_event* a_pstEvents, int a_iMaxEvents, int a_iTimeout)
{
	LPTPOLLPORT pstPort;
	LPTPOLLFD pstFD;
	DWORD dwCount;
	ULONG_PTR ulKey;
	int iIdx;
	LPOVERLAPPED pstOverlap;
	BOOL bRet;

	int iEnentsCount;
	LPTPOLLITEM pstItem;

	assert( a_pstEvents );

	if((TEPOLL_HANDLE) -1==a_iTpfd || 0==a_iTpfd || a_iMaxEvents<1 )
		return -1;


	pstPort	=	(LPTPOLLPORT) a_iTpfd;
	iEnentsCount = 0;

	//优先将等待队列中的描述符加到事件队列中
	iIdx = pstPort->iWaitQueHead;
	while (iIdx>=0)
	{
		assert( iIdx<pstPort->iMaxFd );
		pstFD = &pstPort->items[iIdx].stFd;
		assert(TPOLLF_USED & pstPort->items[iIdx].iFlag);
		assert(pstFD->iData>=0 && pstFD->iData<=pstFD->iBuff);

		if (iEnentsCount >= a_iMaxEvents)
		{
			return iEnentsCount;
		}
		a_pstEvents[iEnentsCount].events	=	EPOLLIN;
		a_pstEvents[iEnentsCount].ulKey	=	pstFD->ev.ulKey;
		memcpy(&a_pstEvents[iEnentsCount].data, &pstFD->ev.data, sizeof(pstFD->ev.data));
		iEnentsCount++;

		iIdx = pstPort->items[iIdx].iWaitQueNext;
	}/*while (iIdx>=0 )*/

	pstOverlap=	NULL;
	bRet	=	GetQueuedCompletionStatus(pstPort->hIOCP, &dwCount, &ulKey, &pstOverlap, a_iTimeout);

	if( !bRet && !pstOverlap )
	{
		if( WAIT_TIMEOUT==GetLastError() )
			return iEnentsCount;
		else
			return (iEnentsCount > 0) ? iEnentsCount : -1;
	}

	assert(pstOverlap);

	iIdx	=	TPOLL_GET_IDX(ulKey);


	if( iIdx>=0 && iIdx<pstPort->iMaxFd && (TPOLLF_USED & pstPort->items[iIdx].iFlag) &&
		ulKey==pstPort->items[iIdx].stFd.ev.ulKey )
	{
		pstItem = &pstPort->items[iIdx];
		pstFD	=	&pstItem->stFd;

		if( WSA_IO_PENDING==pstFD->iState )
		{
			pstFD->iData	=	(int)dwCount;
			pstFD->iState	=	0;
		}

		assert(pstFD->iData>=0 && pstFD->iData<=pstFD->iBuff);
		if (iEnentsCount >= a_iMaxEvents)
		{
			//将此FD 加到等待队列中，以记录此socket有数据读 事件，
            tpoll_add_envent_que_i(pstPort,pstItem,iIdx);
			return iEnentsCount;
		}/*if (iEnentsCount >= a_iMaxEvents)*/
		a_pstEvents[iEnentsCount].events	=	EPOLLIN;
		a_pstEvents[iEnentsCount].ulKey	=	ulKey;
		memcpy(&a_pstEvents[iEnentsCount].data, &pstFD->ev.data, sizeof(pstFD->ev.data));
		iEnentsCount++;
	}

	while(iEnentsCount<a_iMaxEvents)
	{
		pstOverlap=	NULL;
		bRet	=	GetQueuedCompletionStatus(pstPort->hIOCP, &dwCount, &ulKey, &pstOverlap, 0);

		if( !bRet && !pstOverlap )
		{
			if( iEnentsCount > 0 )	return iEnentsCount;

			if( WAIT_TIMEOUT==GetLastError() )
				return 0;
			else
				return -1;
		}

		assert(pstOverlap);

		iIdx	=	TPOLL_GET_IDX(ulKey);


		if( iIdx>=0 && iIdx<pstPort->iMaxFd && (TPOLLF_USED & pstPort->items[iIdx].iFlag) &&
			ulKey==pstPort->items[iIdx].stFd.ev.ulKey )
		{
			pstItem = &pstPort->items[iIdx];
			pstFD	=	&pstItem->stFd;

			if( WSA_IO_PENDING==pstFD->iState )
			{
				pstFD->iData	=	(int)dwCount;
				pstFD->iState	=	0;
			}

			assert(pstFD->iData>=0 && pstFD->iData<=pstFD->iBuff);
			a_pstEvents[iEnentsCount].events	=	EPOLLIN;
			a_pstEvents[iEnentsCount].ulKey	=	ulKey;
			memcpy(&a_pstEvents[iEnentsCount].data, &pstFD->ev.data, sizeof(pstFD->ev.data));
			iEnentsCount++;
		}
	}

	return iEnentsCount;
}

int tpoll_destroy(TEPOLL_HANDLE a_iTpfd)
{
	LPTPOLLPORT pstPort;
	LPTPOLLITEM pstItem;
	int iPtr;

	if((TEPOLL_HANDLE) -1==a_iTpfd || 0==a_iTpfd )
		return 0;

	pstPort	=	(LPTPOLLPORT) a_iTpfd;

	CloseHandle(pstPort->hIOCP);
	pstPort->hIOCP	=	NULL;

	iPtr	=	pstPort->iHead;

	while(TPOLL_BAD_POS!=iPtr)
	{
		pstItem	=	pstPort->items + iPtr;

		if( pstItem->stFd.hFile )
			CancelIo((HANDLE)pstItem->stFd.hFile);

		iPtr	=	pstItem->iNext;
	}

	free(pstPort);

	return 0;
}

int tpoll_add_envent_que_i(LPTPOLLPORT pstPort,LPTPOLLITEM pstItem, int iIdx)
{
    assert(NULL != pstPort);
    assert(NULL != pstItem);

    if (pstItem->iFlag & TPOLLF_INWAITQUE)
    {
        return 0;
    }

    if((TPOLL_BAD_POS != pstItem->iWaitQueNext) ||(TPOLL_BAD_POS != pstItem->iWaitQuePrev) ||
        (iIdx == pstPort->iWaitQueHead))
    {
        return -1;
    }

    pstItem->iWaitQueNext = pstPort->iWaitQueHead;
    pstItem->iWaitQuePrev = TPOLL_BAD_POS;
    if (0 <= pstPort->iWaitQueHead)
    {
        pstPort->items[pstPort->iWaitQueHead].iWaitQuePrev = iIdx;
    }
    pstPort->iWaitQueHead = iIdx;
    pstItem->iFlag |= TPOLLF_INWAITQUE;

    return 0;
}

int tpoll_recvfrom(TEPOLL_HANDLE a_iTpfd, TSOCKET a_iFd, struct epoll_event* a_pstEvent, char* a_pszBuff, int a_iBuff, struct sockaddr* a_pstSrcAddr, int* a_piSrcLen)
{
	LPTPOLLPORT pstPort;
	int iIdx;
	LPTPOLLFD pstFD;
	int iData;
	int iRecv;

	LPTPOLLITEM pstItem;

	assert( a_pstEvent && a_pszBuff && a_iBuff>0 );

	if( -1==a_iTpfd || 0==a_iTpfd || a_iBuff<0 )
		return -1;

	pstPort	=	(LPTPOLLPORT) a_iTpfd;

	iIdx	=	TPOLL_GET_IDX(a_pstEvent->ulKey);
	pstItem = &pstPort->items[iIdx];
	if( iIdx<0 || iIdx>=pstPort->iMaxFd ||  !(TPOLLF_USED & pstItem->iFlag) )
		return -1;

	pstFD	=	&pstPort->items[iIdx].stFd;
	if( pstFD->hFile!=a_iFd || a_pstEvent->ulKey!=pstFD->ev.ulKey || pstFD->iState )
		return -1;

	if( pstFD->iData<=0 )
		return pstFD->iData;

	assert(pstFD->iOff<=pstFD->iBuff);

	if( pstFD->iData>a_iBuff )
	{
		memcpy(a_pszBuff, pstFD->szBuff + pstFD->iOff, a_iBuff );
		pstFD->iData	-=	a_iBuff;
		pstFD->iOff		+=	a_iBuff;

		//将此FD 加到等待队列中，以记录此socket有数据读 事件，
        tpoll_add_envent_que_i(pstPort,pstItem,iIdx);
		return a_iBuff;
	}

	memcpy(a_pszBuff, pstFD->szBuff + pstFD->iOff, pstFD->iData );
	iData	=	pstFD->iData;
	pstFD->iData	=	0;
	pstFD->iOff		=	0;

	a_iBuff	-=	iData;
	a_pszBuff	+=	iData;

	if( a_iBuff>0 )
		iRecv	=	recv(a_iFd, a_pszBuff, a_iBuff, 0);
	else
		iRecv	=	0;

	if( iRecv>0 )
	{
		iData	+=	iRecv;
	}

	//从等待队列中删除
	if (pstItem->iFlag & TPOLLF_INWAITQUE)
	{
		int iNext = pstItem->iWaitQueNext;
		int iPrev = pstItem->iWaitQuePrev;
		if (0 <= iPrev)
		{
			pstPort->items[iPrev].iWaitQueNext = iNext;
		}
		if (0 <= iNext)
		{
			pstPort->items[iNext].iWaitQuePrev = iPrev;
		}
		if (pstPort->iWaitQueHead == iIdx)
		{
			pstPort->iWaitQueHead = iNext;
		}
		pstItem->iWaitQueNext = TPOLL_BAD_POS;
		pstItem->iWaitQuePrev = TPOLL_BAD_POS;
		pstItem->iFlag &= ~TPOLLF_INWAITQUE;
	}/*if (pstItem->iFlag & TPOLLF_INWAITQUE)*/


	tpoll_start_recv_i(pstFD);

	if( pstFD->iState<0 )
		return -1;
	else
		return iData;
}


int tpoll_recv(TEPOLL_HANDLE a_iTpfd, TSOCKET a_iFd, struct epoll_event* a_pstEvent, char* a_pszBuff, int a_iBuff)
{
	return tpoll_recvfrom(a_iTpfd, a_iFd, a_pstEvent, a_pszBuff, a_iBuff, NULL, NULL);
}

int tpoll_sendto(TEPOLL_HANDLE a_iTpfd, TSOCKET a_iFd, TPOLL_EVENT* a_pstEvent, char* a_pszBuff, int a_iBuff, struct sockaddr* a_pstDst, int a_iDstLen)
{
	LPTPOLLPORT pstPort;
	int iIdx;
	LPTPOLLFD pstFD;

	assert( a_pstEvent && a_pszBuff && a_iBuff>0 );

	if( (TEPOLL_HANDLE)-1==a_iTpfd || 0==a_iTpfd )
		return -1;

	pstPort	=	(LPTPOLLPORT) a_iTpfd;

	iIdx	=	TPOLL_GET_IDX(a_pstEvent->ulKey);
	if( iIdx<0 || iIdx>=pstPort->iMaxFd )
		return -1;

	pstFD	=	&pstPort->items[iIdx].stFd;
	if( pstFD->hFile!=a_iFd || pstFD->ev.ulKey!=a_pstEvent->ulKey )
		return -1;

	if( pstFD->fStream )
		return send(a_iFd, a_pszBuff, a_iBuff, 0);
	else
		return sendto(a_iFd, a_pszBuff, a_iBuff, 0, a_pstDst, a_iDstLen);
}

int tpoll_send(TEPOLL_HANDLE a_iTpfd, TSOCKET a_iFd, TPOLL_EVENT* a_pstEvent, char* a_pszBuff, int a_iBuff)
{
	return tpoll_sendto(a_iTpfd, a_iFd, a_pstEvent, a_pszBuff, a_iBuff, NULL, 0);
}

TSOCKET tpoll_accept(TEPOLL_HANDLE a_iTpfd, TSOCKET a_iFd, struct epoll_event* a_pstEvent, struct sockaddr* a_pstAddr, int* a_piLen)
{
	LPTPOLLPORT pstPort;
	int iIdx;
	LPTPOLLFD pstFD;
	int iConnTime;
	int iBuff;
	int iRet;
	TSOCKET s;
	int iLocal;
	struct sockaddr* pstLocal;
	int iRemote;
	struct sockaddr* pstRemote;

	assert( a_pstEvent );

	s	=	(TSOCKET)-1;

	if( (TEPOLL_HANDLE)-1==a_iTpfd || 0==a_iTpfd )
	{
		WSASetLastError(WSAEINVAL);
		return (TSOCKET)-1;
	}

	pstPort	=	(LPTPOLLPORT) a_iTpfd;

	iIdx	=	TPOLL_GET_IDX(a_pstEvent->ulKey);
	if( iIdx<0 || iIdx>=pstPort->iMaxFd )
	{
		WSASetLastError(WSAEINVAL);
		return (TSOCKET)-1;
	}

	pstFD	=	&pstPort->items[iIdx].stFd;
	if( pstFD->hFile!=a_iFd || pstFD->ev.ulKey!=a_pstEvent->ulKey ||
		-1==pstFD->sAccept || pstFD->iState )
	{
		WSASetLastError(WSAEINVAL);
		return (TSOCKET)-1;
	}

	iBuff		= (int)sizeof(iConnTime);
	iConnTime	= -1;
	iRet = getsockopt( pstFD->sAccept, SOL_SOCKET, SO_CONNECT_TIME, (char *)&iConnTime, &iBuff);

	if( iRet<0 )
	{
		WSASetLastError(WSAEINVAL);
		return (TSOCKET)-1;
	}

	if( -1==iConnTime )
	{
		s	=	(TSOCKET)-1;
		closesocket(pstFD->sAccept);
		pstFD->sAccept	=	INVALID_SOCKET;
	}
	else
	{
		pstFD->lpfnAddr(pstFD->szBuff, 0, sizeof(struct sockaddr_in)+16,
			sizeof(struct sockaddr_in)+16, &pstLocal, &iLocal, &pstRemote, &iRemote);

		if( a_piLen && a_pstAddr && *a_piLen>=iRemote )
			memcpy(a_pstAddr, pstRemote, iRemote);

		if( a_piLen )
			*a_piLen	=	iRemote;

		s	=	pstFD->sAccept;
		pstFD->sAccept	=	INVALID_SOCKET;
	}

	tpoll_start_accept_i(pstFD);

	return s;
}

#else

int tpoll_recvfrom(TEPOLL_HANDLE a_iTpfd, TSOCKET a_iFd, struct epoll_event* a_pstEvent, char* a_pszBuff, int a_iBuff, struct sockaddr* pstSrcAddr, int* piSrcLen)
{
	TOS_UNUSED_ARG(a_iTpfd);
	TOS_UNUSED_ARG(a_pstEvent);
	return recvfrom(a_iFd, a_pszBuff, (size_t)a_iBuff, 0, pstSrcAddr, (unsigned int*)piSrcLen);
}

int tpoll_recv(TEPOLL_HANDLE a_iTpfd, TSOCKET a_iFd, TPOLL_EVENT* a_pstEvent, char* a_pszBuff, int a_iBuff)
{
	TOS_UNUSED_ARG(a_iTpfd);
	TOS_UNUSED_ARG(a_pstEvent);
	return recv(a_iFd, a_pszBuff, (size_t)a_iBuff, 0);
}

int tpoll_sendto(TEPOLL_HANDLE a_iTpfd, TSOCKET a_iFd, TPOLL_EVENT* a_pstEvent, char* a_pszBuff, int a_iBuff, struct sockaddr* pstDst, int iDstLen)
{
	TOS_UNUSED_ARG(a_iTpfd);
	TOS_UNUSED_ARG(a_pstEvent);
	return sendto(a_iFd, a_pszBuff, (size_t)a_iBuff, 0, pstDst, iDstLen);
}

int tpoll_send(TEPOLL_HANDLE a_iTpfd, TSOCKET a_iFd, TPOLL_EVENT* a_pstEvent, char* a_pszBuff, int a_iBuff)
{
	TOS_UNUSED_ARG(a_iTpfd);
	TOS_UNUSED_ARG(a_pstEvent);
	return send(a_iFd, a_pszBuff, (size_t)a_iBuff, 0);
}

TSOCKET tpoll_accept(TEPOLL_HANDLE a_iTpfd, TSOCKET a_iFd, TPOLL_EVENT* a_pstEvent, struct sockaddr* a_pstAddr, int* a_piLen)
{
	TOS_UNUSED_ARG(a_iTpfd);
	TOS_UNUSED_ARG(a_pstEvent);
	int iRet;
	socklen_t iLen;

	if( a_piLen )
	{
		iLen		= (socklen_t) *a_piLen;
		iRet		= accept(a_iFd, a_pstAddr, &iLen);
		*a_piLen	= iLen;
	}
	else
	{
		iRet		= accept(a_iFd, a_pstAddr, NULL);
	}

	return iRet;
}

#endif /* #if defined(_WIN32) || defined(_WIN64) */
