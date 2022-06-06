
#include <sys/select.h>
#include <errno.h>
#include "obus/og_bus.h"

//在bus_relay_client.c / bus_relay_server.c中定义
extern int g_iRemoteLog;

int SendBusPkg(int iSock, BusPkg *pstPkg)
{
	int iPkgLen, iWrite, iLen, iErrno;
	int iCount;
	
	iCount = 0;
	
	iPkgLen = BusPkgLen(pstPkg);
	iWrite = 0;
	
	//	HexShow((char *)pstPkg, iPkgLen, 0);
	
	while (iWrite < iPkgLen) {
		iLen = write(iSock, (char *)pstPkg + iWrite, iPkgLen - iWrite);
		if (iLen < 0) {
			iErrno= errno;
			printf("write err: %d %d\n", iPkgLen, iWrite);
			if (iErrno == EINTR || iErrno == EAGAIN) printf("write again\n");
			if (iErrno == EINTR || iErrno == EAGAIN) {
				if( iCount>=5 )
					return -1;
				
				usleep(1000);
				iCount++;
				continue;
			}
			
			return -1;
		}
		iWrite += iLen;
	}
	
	return iWrite;
}

int SendBusPkgBuff(int iSock, char* pszBuff, int iBuff)
{
int iWrite, iLen, iErrno;

	iWrite = 0;

	while (iWrite < iBuff)
	{
		iLen = write(iSock, pszBuff + iWrite, iBuff - iWrite);
		if (iLen < 0)
		{
			iErrno= errno;
			if (iErrno == EINTR || iErrno == EAGAIN)
				return iWrite;
			else
				return -1;
		}

		iWrite += iLen;
	}

	return iWrite;
}

int ConstructSynHead(BusPkgHead *pstHead, Channel *pstChannel, int iSeq, char cCmd)
{
	memset(pstHead, 0, sizeof(BusPkgHead));
	pstHead->cVer = BUS_VERSION;
	pstHead->cHeadLen = sizeof(BusPkgHead) / 4;
	if (pstChannel->iSide & CHANNEL_SIDE_UP) {
		*(int *)pstHead->sDst = pstChannel->pstHead->iUp;
		*(int *)pstHead->sSrc = pstChannel->pstHead->iDown;
	} else {
		*(int *)pstHead->sDst = pstChannel->pstHead->iDown;
		*(int *)pstHead->sSrc = pstChannel->pstHead->iUp;
	}
	*(short *)pstHead->sFlag = htons(BUS_SYN);
	*(int *)pstHead->sSeq = htonl(iSeq);
	pstHead->cRoute = cCmd;
	return 0;
}

int RestartSeq(int iSock, Channel *pstChannel)
{
BusPkgHead stHead;

	ConstructSynHead(&stHead, pstChannel, GetChannelHeadSeq(pstChannel), 5);
	SendBusPkg(iSock, (BusPkg *)&stHead);
	return 0;
}

int LastSeqOK(int iSock, Channel *pstChannel, int iSeq)
{
BusPkgHead stHead;

	ConstructSynHead(&stHead, pstChannel, iSeq, 3);
	SendBusPkg(iSock, (BusPkg *)&stHead);
	return 0;
}

int SynSend(int iSock,  Channel *pstChannel)
{
BusPkgHead stHead;
ChannelVar *pstVar;

	pstVar = ChannelVarPush(pstChannel);
	if (!pstVar->cPStart) ConstructSynHead(&stHead, pstChannel, 0, 4);
	else ConstructSynHead(&stHead, pstChannel, pstVar->iRecvSeq, 2);
	SendBusPkg(iSock, (BusPkg *)&stHead);
	return 0;
}

//保持心跳包
int KeepSend(int iSock, Channel * pstChannel)
{
	BusPkgHead stHead;
	memset(&stHead, 0, sizeof(stHead));
	stHead.cVer = BUS_VERSION;
	stHead.cHeadLen = sizeof(BusPkgHead) / 4;
	
	if (pstChannel->iSide & CHANNEL_SIDE_UP) 
	{
		*(int *)stHead.sDst = pstChannel->pstHead->iUp;
		*(int *)stHead.sSrc = pstChannel->pstHead->iDown;
	}
	else 
	{
		*(int *)stHead.sDst = pstChannel->pstHead->iDown;
		*(int *)stHead.sSrc = pstChannel->pstHead->iUp;
	}
	
	*(short *)stHead.sFlag = htons(BUS_KEEP);
	SendBusPkg(iSock, (BusPkg *)&stHead);
	
	return 0;
	
}

int SetRemoteAuthFlag(AttachChannel *pstChannel, CONNINFO* pstConn, int iSeq)
{
	pstConn->iSeq		=	iSeq;
	pstConn->iSeqValid	=	1;
	
	return 0;
}

int ReadSynHead(int iSock, BusPkgHead *pstHead)
{
int iLen;

	iLen = read(iSock, pstHead, sizeof(BusPkgHead));
	if (iLen != sizeof(BusPkgHead)) return -1;
	if (BusPkgLen(pstHead) != sizeof(BusPkgHead)) return -1;
	if (!(ntohs(*(short *)pstHead->sFlag) & BUS_SYN)) return -1;
	return 0;
}

int CloseSock(int *piSock, fd_set *pAllFds)
{
	printf("close %d\n", *piSock);
	close(*piSock);
	FD_CLR(*piSock, pAllFds);
	*piSock = -1;
	return 0;
}

int DealSynHead(AttachChannel *pstAch, BusPkgHead *pstHead, int iSock, fd_set *pAllFds)
{
int iSeq;
ChannelVar *pstVar;

	iSeq = ntohl(*(int *)pstHead->sSeq);
	switch (pstHead->cRoute) {
		case 2:
			pstVar = ChannelVarGet(&pstAch->stChannel);
			if (!pstVar->cGStart) {
				printf("restart channel.\n");
				RestartSeq(iSock, &pstAch->stChannel);
				break;
			}
			ChannelAuth(&pstAch->stChannel, iSeq);
			pstVar->iGet = pstVar->iHead;
			if (GetChannelHeadSeq(&pstAch->stChannel) == iSeq + 1) {
				LastSeqOK(iSock, &pstAch->stChannel, iSeq);
				break;
			}
			printf("restart seq: %d  %d\n", iSeq, GetChannelHeadSeq(&pstAch->stChannel));
			RestartSeq(iSock, &pstAch->stChannel);
			break;
		case 3:
			if (pstAch->iDst >= 0) {
				printf("ori: %d %d\n", pstAch->iDst, iSock);
				if (pstAch->iDst != iSock) CloseSock(&pstAch->iDst, pAllFds);
			}
			pstAch->iDst = iSock;
			break;
		case 4:
			pstVar = ChannelVarGet(&pstAch->stChannel);
			pstVar->iGet = pstVar->iHead;
			RestartSeq(iSock, &pstAch->stChannel);
			break;
		case 5:
			pstVar = ChannelVarPush(&pstAch->stChannel);
			pstVar->iRecvSeq = iSeq - 1;
			printf("ori: %d %d\n", pstAch->iDst, iSock);
			if (pstAch->iDst >= 0 && pstAch->iDst != iSock) CloseSock(&pstAch->iDst, pAllFds);
			pstAch->iDst = iSock;
			break;
		default:
			return -1;
	}
	return pstHead->cRoute;
}

int DealRemotePkg(BusInterface *pstBus, AttachChannel *pstAch, BusPkg *pstBusPkg, CONNINFO* pstConnInfo)
{
short shFlag;
int iSeq, iRet, iPkgLen;
ChannelVar *pstVar;

	shFlag = ntohs(*(short *)pstBusPkg->stHead.sFlag);
	iSeq = ntohl(*(int *)pstBusPkg->stHead.sSeq);
	if (shFlag & BUS_SYN) {
		if (pstBusPkg->stHead.cRoute != 1) return -1;
		ChannelAuth(&pstAch->stChannel, iSeq);
		//LogBus(pstBus, "Get auth: %d\n", iSeq);
		return 0;
	}
	else if ( shFlag & BUS_KEEP )	//bus_client向bus_server发送的心跳包
	{
		LogBus(pstBus, "Get keep alive\n");
		return 0;
	}
	
	BusCheckFlag(pstBus, pstBusPkg, "Relay recv socket");
	iPkgLen = BusPkgLen(&pstBusPkg->stHead);
	// HexShow((char *)pstBusPkg, iPkgLen, 0);
	*(short *)pstBusPkg->stHead.sCheckSum = 0;
	iRet = ChannelAuthPush(&pstAch->stChannel, pstBusPkg, iSeq);
	if (iRet < 0) {
		pstVar = ChannelVarPush(&pstAch->stChannel);
		LogBus(pstBus, "channel push err: %d  %d  %d\n", iRet, iSeq, pstVar->iRecvSeq);
		return -1;
	}
	BusCheckFlag(pstBus, pstBusPkg, "Relay push channel");
	//LogBus(pstBus, "seq: %d  pkgcount: %d  bytecount: %d\n", iSeq, pstConnInfo->iPkgCount, pstConnInfo->iByteCount);
	pstConnInfo->iPkgCount++;
	pstConnInfo->iByteCount += iPkgLen;
	pstConnInfo->iPkgTotal++;
	pstConnInfo->iByteTotal += iPkgLen;
	if (pstConnInfo->iPkgCount >= 10 || pstConnInfo->iByteCount >= 65536) {
		pstConnInfo->iPkgCount = pstConnInfo->iByteCount = 0;
		if( SetRemoteAuthFlag(pstAch, pstConnInfo, iSeq)<0 )
			return -1;
	}
	return 0;
}

int ReadRemotePkg(BusInterface *pstBus, AttachChannel *pstAch, BusRecvBuf *pstRecv, CONNINFO* pstConnInfo)
{
int iSock, iLen, iPkgLen, iDealLen, iErrno;
BusPkg *pstBusPkg;

	iSock = pstAch->iDst;
	iLen = read(iSock, pstRecv->sBuf + pstRecv->iGetLen, sizeof(pstRecv->sBuf) - pstRecv->iGetLen);
	iErrno = errno;

	if ( g_iRemoteLog )
	{
		LogBus(pstBus, "read %d %d  %d %d\n", iSock, iLen, pstRecv->iGetLen, sizeof(pstRecv->sBuf));
		if ( iLen > 0 )	LogBus(pstBus, HexString(pstRecv->sBuf + pstRecv->iGetLen, iLen)) ;
	}
	
	if (iLen <= 0) {
		// printf("err: %d %d\n", errno, EAGAIN);
		if (iLen < 0 && iErrno == EAGAIN) return 0;
		pstRecv->iGetLen = 0;
		return -1;
	}
	pstRecv->iGetLen += iLen;
	iDealLen = 0;
	while (pstRecv->iGetLen) {
		if (pstRecv->iGetLen < sizeof(BusPkgHead)) break;
		pstBusPkg = (BusPkg *)(pstRecv->sBuf + iDealLen);
		iPkgLen = BusPkgLen(&pstBusPkg->stHead);
		//LogBus(pstBus, "get:%d pkg:%d deal:%d\n", pstRecv->iGetLen, iPkgLen, iDealLen);
		if (iPkgLen > pstRecv->iGetLen) break;
		if (DealRemotePkg(pstBus, pstAch, pstBusPkg, pstConnInfo) < 0) {
			pstRecv->iGetLen = 0;
			return -1;
		}
		iDealLen += iPkgLen;
		pstRecv->iGetLen -= iPkgLen;
	}
	if (iDealLen && pstRecv->iGetLen) memmove(pstRecv->sBuf, pstRecv->sBuf + iDealLen, pstRecv->iGetLen);
	return 0;
}

int CheckChannelSend(BusInterface *pstBus, AttachChannel *pstChannel, CONNINFO* pstConnInfo)
{
	BusPkg stRecvPkg;
	BusPkg* pstPkg;
	int iSize;
	int iOff;
	int iSend;
	int iSock;
	
	if( !pstConnInfo->iPkgValid )
		return 0;

	if( pstConnInfo->iIsHead )
		pstPkg	=	(BusPkg*) &pstConnInfo->stHead;
	else
	{
		if( ChannelAuthPeekByPos(&pstChannel->stChannel, pstConnInfo->iPos, &stRecvPkg) )
		{
			pstConnInfo->iPkgValid	=	0;
			pstConnInfo->iSend		=	0;
			pstConnInfo->iPos		=	0;
			return -1;
		}

		pstPkg	=	&stRecvPkg;
	}
		
	iSize = BusPkgLen(pstPkg);
		
	if( iSize<=pstConnInfo->iSend )
	{
		pstConnInfo->iPkgValid	=	0;
		pstConnInfo->iSend		=	0;
		pstConnInfo->iPos		=	0;
		
		return -1;
	}
		
	iSock	=	pstChannel->iDst;
	iOff	=	pstConnInfo->iSend;
		
	iSend	=	SendBusPkgBuff(iSock, ((char*)(pstPkg))+iOff, iSize-iOff );
		
	if( iSend<0 )
		return -1;
		
	if( iSend == iSize - iOff )
	{
		pstConnInfo->iSend	=	0;
		pstConnInfo->iPos	=	0;
		pstConnInfo->iPkgValid=	0;
		pstConnInfo->iTry	=	0;

		if( (BusPkg*)&pstConnInfo->stHead!=pstPkg )
			BusCheckFlag(pstBus, &pstBus->stRecvPkg, "Relay send socket");
	}
	else
	{
		pstConnInfo->iSend	+=	iSend;
		pstConnInfo->iTry++;
	}

//	if( pstConnInfo->iTry>20 )
//		return -1;

	return 0;
}

int RemoteAuth(AttachChannel *pstChannel, CONNINFO* pstConnInfo)
{
	BusPkg* pstPkg;
	int iSize;
	int iSend;

	if( !pstConnInfo->iSeqValid )
		return 0;
	
	ConstructSynHead(&pstConnInfo->stHead, &pstChannel->stChannel, pstConnInfo->iSeq, 1);

	pstPkg	=	(BusPkg*) &pstConnInfo->stHead;
	iSize	=	BusPkgLen(pstPkg);

	iSend	=	SendBusPkgBuff(pstChannel->iDst, (char*)pstPkg, iSize);

	if( iSend<0 )
		return -1;

	pstConnInfo->iSeqValid	=	0;

	if( iSend==iSize )
		return 0;

	pstConnInfo->iIsHead	=	1;
	pstConnInfo->iSend		=	iSend;
	pstConnInfo->iPkgValid	=	1;
	
	return 0;
}

int RecvBus(BusInterface *pstBus, CONNINFO* pstConnInfo, int iCount)
{
AttachChannel *pstChannel;
int i;
int iPkgTotal;
int iPkgChannel;
int iSend;
int iSize;
int iChannel;

	if( pstBus->iAttach<=0 )
		return 0;

	i	=	(pstBus->iLast + 1) % pstBus->iAttach;
	
	iChannel	=	0;
	iPkgTotal	=	0;
	iPkgChannel	=	0;

	while(iChannel<pstBus->iAttach && iPkgTotal<iCount)
	{
		pstBus->iLast	=	i;

		pstChannel	=	pstBus->astAttach + i;

		if( pstChannel->iDst<0 )
		{
			iChannel++;
			i	=	(i + 1) % pstBus->iAttach;
			iPkgChannel	=	0;
			continue;
		}

		if( CheckChannelSend(pstBus, pstChannel, pstConnInfo + i)<0 )			
			return -1;

		if( pstConnInfo[i].iPkgValid )
		{
			iChannel++;
			i	=	(i + 1) % pstBus->iAttach;
			iPkgChannel	=	0;
			continue;
		}

		if( RemoteAuth(pstChannel, pstConnInfo + i)<0 )
			return -1;

		if( pstConnInfo[i].iPkgValid )
		{
			iChannel++;
			i	=	(i + 1) % pstBus->iAttach;
			iPkgChannel	=	0;
			continue;
		}

		ChannelAuthPos(&pstChannel->stChannel, &pstConnInfo[i].iPos);

		if( BusRealRecvChannel(pstBus, i, BUS_RELAY_SERVER) )
		{
			iChannel++;
			i	=	(i + 1) % pstBus->iAttach;
			iPkgChannel	=	0;
			continue;
		}

		pstConnInfo[i].iSend	=	0;

		iPkgTotal++;
		iPkgChannel++;		

		iSize	=	BusPkgLen(&pstBus->stRecvPkg);

		iSend	=	SendBusPkgBuff(pstChannel->iDst, (char*)&pstBus->stRecvPkg, iSize);

		if( iSend==iSize )
		{
			if( iPkgChannel>=10 )
			{
				iChannel++;
				i	=	(i + 1) % pstBus->iAttach;
				iPkgChannel	=	0;
			}

			BusCheckFlag(pstBus, &pstBus->stRecvPkg, "Relay send socket");

			continue;
		}

		pstConnInfo[i].iPkgValid	=	1;
		pstConnInfo[i].iIsHead		=	0;

		if( iSend<0 )
			return -1;

		pstConnInfo[i].iSend	=	iSend;

		iChannel++;
		i	=	(i+1) % pstBus->iAttach;
		iPkgChannel	=	0;
	}

	return 0;
}

