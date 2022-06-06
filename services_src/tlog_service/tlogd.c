/*
**  @file $RCSfile: tlogd.c,v $
**  general description of this module
**  $Id: tlogd.c,v 1.6 2009-07-08 01:27:49 kent Exp $
**  @author $Author: kent $
**  @date $Date: 2009-07-08 01:27:49 $
**  @version $Revision: 1.6 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#include "pal/pal.h"
#include "tdr/tdr.h"
#include "../lib_src/tlog/tlog_i.h"
#include "tdr/tdr_metalib_kernel_i.h"

#include "tapp/tapp.h"

#include "tlog/tlog.h"
#include "tlog/tlogbin.h"
#include "tlogddef.h"
#include "tlogdpool.h"
#include "tlogdmgr.h"
#include "version.h"
#include "../lib_src/comm/tmempool_i.h"


extern unsigned char g_szMetalib_tlogd[];

#define LISTEN_URI		"tcp://0.0.0.0:4444"
#define MAX_FD_USED		4096
#define MAX_EVENTS		8192

static TAPPCTX gs_stAppCtx;

static time_t gs_tNow	=	0;

TLOGCATEGORYINST* gs_pstTlogdCatInst=NULL;

struct tagTConnEnv
{
	CONFINST stConfInst;
	TLOGDTHREAD astThreads[TCONND_MAX_NETTRANS];
};

typedef struct tagTConnEnv		TCONNENV;
typedef struct tagTConnEnv		*LPTCONNENV;

static TCONNENV gs_stEnv;


#define DECLA
int tlogd_tconnd_get_none_pkglen(TCONNINST* pstInst, TLOGDTHREAD* pstThread);
int tlogd_tconnd_get_thttp_pkglen(TCONNINST* pstInst, TLOGDTHREAD* pstThread);
int tlogd_tconnd_proc_single(TAPPCTX* pstAppCtx, TCONNENV* pstEnv);
int tlogd_tconnd_proc_multi(TAPPCTX* pstAppCtx, TCONNENV* pstEnv);
int tlogd_tconnd_reload_single(TAPPCTX* pstAppCtx, TCONNENV* pstEnv);
int tlogd_tconnd_reload_multi(TAPPCTX* pstAppCtx, TCONNENV* pstEnv);
int tlogd_tconnd_reload(TAPPCTX* pstAppCtx, TCONNENV* pstEnv);
int tlogd_tconnd_tick_single(TAPPCTX* pstAppCtx, TCONNENV* pstEnv);
int tlogd_tconnd_tick_multi(TAPPCTX* pstAppCtx, TCONNENV* pstEnv);
int tlogd_tconnd_fini_single(TAPPCTX* pstAppCtx, TCONNENV* pstEnv);
int tlogd_tconnd_fini_multi(TAPPCTX* pstAppCtx, TCONNENV* pstEnv);
int tlogd_tconnd_init(TAPPCTX* pstAppCtx, TCONNENV* pstEnv);
int tlogd_tconnd_init_multi(TAPPCTX* pstAppCtx, TCONNENV* pstEnv);
void* tlogd_tconnd_thread_proc(void* pvArg);
int tlogd_tconnd_scan(TLOGDTHREAD* pstThread, int iIsShutdown);
int tlogd_tconnd_proc_once(TLOGDTHREAD* pstThread);
int tlogd_tconnd_proc_recv(TLOGDTHREAD* pstThread);
int tlogd_tconnd_recv_msg(TLOGDTHREAD* pstThread, TCONNINST* pstInst, struct sockaddr* pstSource);
int tlogd_tconnd_get_pkglen(TCONNINST* pstInst, TLOGDTHREAD* pstThread);
int tlogd_tconnd_init_single(TAPPCTX* pstAppCtx, TCONNENV* pstEnv);
int tlogd_tconnd_safe_close(TLOGDTHREAD* pstThread, int iIdx);



int tlogd_tconnd_safe_close(TLOGDTHREAD* pstThread, int iIdx)
{
	int iRet;
	TLOGD_TCONND* pstConnd;
	TCONNINST* pstInst;
	TLOGD_TCONNDRUN* pstRun;

	pstConnd	=	(TLOGD_TCONND*) pstThread->pstAppCtx->stConfData.pszBuff;
	pstRun		=	(TLOGD_TCONNDRUN*) pstThread->pstAppCtx->stRunDataStatus.pszBuff;

	pstInst	=	tlogd_tconnd_get_inst( pstThread->pstPool, iIdx);

	if( !pstInst )
		return -1;

	if( pstInst->s>=0 )
	{
		pstRun->iTotalConns--;
		tnet_close(pstInst->s);
		pstInst->s	=	-1;
	}

	iRet	=	tlogd_tconnd_free_inst(pstThread->pstPool, iIdx);

	return 0;
}


int tlogd_tconnd_init_single(TAPPCTX* pstAppCtx, TCONNENV* pstEnv)
{
	int i;
	int j;
	int s;
	int iRet;
	int iLimit;
	int iUnit;
	int iType;
	TLOGD_TCONND* pstConnd;
	TCONNPOOL* pstPool;
	CONFINST* pstConfInst;
	NETTRANSLIST* pstNetTransList;
	LISTENERLIST* pstListenerList;
	TRANSINST* pstTransInst;
	SERIALIZERLIST* pstSerializerList;
	LISTENER* pstListener;
	PDUINST* pstPDUInst;
	TCONNINST* pstInst;
	TLOGDTHREAD* pstThread;
	epoll_event_t e;

	pstThread	=	&pstEnv->astThreads[0];
	pstThread->pstAppCtx	=	pstAppCtx;

	if( pstAppCtx->stConfData.iLen < (int)sizeof(TLOGD_TCONND) )
	{
		printf("error: config meta is too old.\n");
		return -1;
	}

	pstConfInst		=	&pstEnv->stConfInst;
	pstConnd		=	(TLOGD_TCONND*) pstAppCtx->stConfData.pszBuff;

	pstNetTransList	=	&pstConnd->stNetTransList;
	pstListenerList	=	&pstConnd->stListenerList;
	pstSerializerList=	&pstConnd->stSerializerList;

	iLimit	=	pstConnd->iMaxFD;

	if( iLimit<=0 )
	{
		iLimit	=	0;

		for(i=0; i<pstConnd->stNetTransList.iCount; i++)
		{
			pstTransInst	=	&pstConfInst->stTransInstList.astInsts[i];
			for(j=0; j<(int)( sizeof(pstTransInst->aiLisLoc)/sizeof(pstTransInst->aiLisLoc[0]) ); j++)
			{
				pstListener	=	pstListenerList->astListeners + pstTransInst->aiLisLoc[j];
				iLimit	+=	pstListener->stConnLimit.iMaxConn;
			}
		}
	}

	if( iLimit<=0 )
		return -1;

	iUnit	=	0;

	for(i=0; i<pstConnd->stNetTransList.iCount; i++)
	{
		pstPDUInst	=	pstConfInst->stPDUInstList.astInsts + pstConfInst->stTransInstList.astInsts[i].iPDULoc;

		if( iUnit < pstPDUInst->iUnit )
			iUnit	=	pstPDUInst->iUnit;
	}

	if( iUnit<=0 )
		return -1;

	pstThread->iPkgUnit		=	iUnit;
	pstThread->iPoolUnit	=	pstThread->iPkgUnit + (int) offsetof(TCONNINST, szBuff);

	iRet	=	tlogd_tconnd_init_pool(&pstThread->pstPool, iLimit, pstThread->iPoolUnit);

	if( iRet<0 )
	{
		printf("Failed to init pool\n");
		return -1;
	}

	pstPool	=	pstThread->pstPool;

	pstThread->epfd	=	epoll_create(iLimit);
	if( -1==pstThread->epfd )
	{
		printf("Error: Create epoll-fd[MAXFD=%d] failed.\n", iLimit);
		perror("Error:");
		return -1;
	}

	for(i=0; i<pstNetTransList->iCount; i++)
	{
		pstTransInst	=	&pstConfInst->stTransInstList.astInsts[i];

		for(j=0; j<pstTransInst->iLisCount; j++)
		{
			pstListener	=	pstListenerList->astListeners + pstTransInst->aiLisLoc[j];

			s	=	tnet_listen(pstListener->szUrl, pstListener->stConnLimit.iBacklog);

			if( -1==s )
			{
				printf("Error: Listen on %s failed.\n", pstListener->szUrl);
				return -1;
			}

			iType	=	SOCK_STREAM;

			tsocket_get_type(s, &iType);

			iRet	=	tnet_set_nonblock(s, 1);
			if( iRet<0 )
			{
				printf("Error: Set listen socket non-block failed. \n");
				return -1;
			}

			memset(&e, 0, sizeof(e));

			e.events	=	EPOLLIN;
			e.data.fd	=	tlogd_tconnd_alloc_inst(pstPool);

			if(e.data.fd<0)
			{
				printf("Error: Alloc conn instance for listen socket failed.\n");
				return -1;
			}

			pstInst		=	tlogd_tconnd_get_inst(pstPool, e.data.fd);
			pstInst->fListen	=	1;

			if( SOCK_STREAM==iType )
				pstInst->fStream	=	1;
			else
				pstInst->fStream	=	0;

			pstInst->iID		=	-1;
			pstInst->iIdx		=	e.data.fd;
			pstInst->s	=	s;
			pstInst->iTransLoc	=	i;
			pstInst->iLisLoc	=	pstTransInst->aiLisLoc[j];

			pstInst->tLastRecv	=	gs_tNow;

			memset(&pstInst->stAddr, 0, sizeof(pstInst->stAddr));
			pstInst->iBuff	=	pstThread->iPkgUnit;

			if( -1==epoll_ctl(pstThread->epfd, EPOLL_CTL_ADD, s, &e) )
			{
				tlogd_tconnd_safe_close(pstThread, e.data.fd);

				printf("Error: Add listen socket to epoll-fd failed.\n");

				return -1;
			}
		}
	}

	pstThread->pstConfInst	=	&pstEnv->stConfInst;

	pstThread->iBuff		=	iUnit + sizeof(TLOGBINHEAD);
	pstThread->pszBuff		=	malloc(pstThread->iBuff);

	if( !pstThread->pszBuff )
		return -1;

	return 0;
}


int tlogd_tconnd_get_pkglen(TCONNINST* pstInst, TLOGDTHREAD* pstThread)
{
	TRANSINST* pstTransInst;
	PDUINST* pstPDUInst;
	char* pszBuff;
	LPPDULENTDRPARSERINST pstTDRParser;
	char *pszDataBase;

	pstTransInst	=	pstThread->pstConfInst->stTransInstList.astInsts + pstInst->iTransLoc;
	pstPDUInst	=	pstThread->pstConfInst->stPDUInstList.astInsts + pstTransInst->iPDULoc;
	pstTDRParser = &pstPDUInst->stLenParser.stTDRParser;

	if( pstInst->iData < pstPDUInst->iMinLen )
		return -1;

	//Read the ID and cls.
	pszDataBase = pstInst->szBuff + pstInst->iOff;

	if( 0 < pstTDRParser->iMsgIDUnitSize )
	{
		pszBuff	=	pszDataBase + pstTDRParser->iMsgIDNOff;
		TDR_GET_INT_NET(pstInst->iMsgID, pstTDRParser->iMsgIDUnitSize, pszBuff);
	}
	else
	{
		pstInst->iMsgID		=	0;
	}

	if( 0 < pstTDRParser->iMsgClsUnitSize )
	{
		pszBuff	=	pszDataBase + pstTDRParser->iMsgClsNOff;

		TDR_GET_INT_NET(pstInst->iMsgCls, pstTDRParser->iMsgClsUnitSize, pszBuff);
	}
	else
	{
		pstInst->iMsgCls		=	0;
	}

	if(pstInst->fStream)
	{
		// Get the pkglen
		if( 0 < pstTDRParser->iPkgLenUnitSize )
		{
			pszBuff	=	pszDataBase + pstTDRParser->iPkgLenNOff;

			TDR_GET_INT_NET(pstInst->iPkgLen, pstTDRParser->iPkgLenUnitSize, pszBuff);

			pstInst->iPkgLen	*=	pstTDRParser->iPkgLenMultiplex;

			//Get the size of data head.
			if( 0 < pstTDRParser->iHeadLenUnitSize )
			{
				pszBuff	=	pszDataBase + pstTDRParser->iHeadLenNOff;

				TDR_GET_INT_NET(pstInst->iHeadLen, pstTDRParser->iHeadLenUnitSize, pszBuff);

				pstInst->iHeadLen	*=	pstTDRParser->iHeadLenMultiplex;
			} // Get the body len size.
			else if( 0 < pstTDRParser->iBodyLenUnitSize )
			{
				pszBuff	=	pszDataBase + pstTDRParser->iBodyLenNOff;

				TDR_GET_INT_NET(pstInst->iHeadLen, pstTDRParser->iBodyLenUnitSize, pszBuff);

				pstInst->iHeadLen	*=	pstTDRParser->iBodyLenMultiplex;

				pstInst->iHeadLen	=	pstInst->iPkgLen - pstInst->iHeadLen;
			}
		}
		else
		{
			// Use Head Len and body len.
			pszBuff		=	pszDataBase + pstTDRParser->iHeadLenNOff;

			TDR_GET_INT_NET(pstInst->iHeadLen, pstTDRParser->iHeadLenUnitSize, pszBuff);

			pstInst->iHeadLen	*=	pstTDRParser->iHeadLenMultiplex;

			pszBuff		=	pszDataBase + pstTDRParser->iBodyLenNOff;

			TDR_GET_INT_NET(pstInst->iPkgLen, pstTDRParser->iBodyLenUnitSize, pszBuff);

			pstInst->iPkgLen	*=	pstTDRParser->iBodyLenMultiplex;

			pstInst->iPkgLen	+=	pstInst->iHeadLen;
		}
	}
	else
	{
			// Use Head Len and body len.
			pszBuff		=	pszDataBase + pstTDRParser->iHeadLenNOff;
			TDR_GET_INT_NET(pstInst->iHeadLen, pstTDRParser->iHeadLenUnitSize, pszBuff);
			pstInst->iHeadLen	*=	pstTDRParser->iHeadLenMultiplex;

			pstInst->iPkgLen = pstInst->iData;
			pstInst->iHeadLen = tmax(0,pstInst->iHeadLen);
			pstInst->iHeadLen = tmin(pstInst->iPkgLen,pstInst->iHeadLen);
	}

	if( pstInst->iPkgLen<=0 )
	{
		pstInst->iNeedFree	=	1;
		return -1;
	}
	else
		return 0;
}

int tlogd_tconnd_recv_msg(TLOGDTHREAD* pstThread, TCONNINST* pstInst, struct sockaddr* pstSrcAddr)
{
	int i;
	int iCount;
	TLOGD_TCONND* pstConnd;
	CONFINST* pstConfInst;
	TLOGD_TCONNDRUN* pstRun;
	NETTRANS* pstNetTrans;
	TRANSINST* pstTransInst;
	SERINSTLIST* pstSerInstList;
	SERINST* pstSerInst;
	NETTRANSRUN* pstTransRun;
	LISTENERRUN* pstLisRun;
	SERIALIZERRUN* pstSerRun;
	int iRet = 0;
	LPPDUINST pstPDUInst;
	PFNTCONND_GET_PKGLEN pfnGetPkgLen;
	char* pszMsg;
	int iMsg;

	iCount	=	0;

	pstTransInst	=	pstThread->pstConfInst->stTransInstList.astInsts + pstInst->iTransLoc;
	pstPDUInst	=	pstThread->pstConfInst->stPDUInstList.astInsts + pstTransInst->iPDULoc;
	pfnGetPkgLen 	=	pstPDUInst->pfnGetPkgLen;

	switch(pstPDUInst->iLenParsertype)
	{
	case PDULENPARSERID_BY_TDR:
		if( !pstInst->iPkgLen )
			iRet = (*pfnGetPkgLen)(pstInst, pstThread);
		break;

	default:
		iRet = (*pfnGetPkgLen)(pstInst, pstThread);
		break;
	}

	if( 0 != iRet)
		return iRet;

	pstConnd	=	(TLOGD_TCONND*)pstThread->pstAppCtx->stConfData.pszBuff;
	pstConfInst	=	pstThread->pstConfInst;
	pstRun		=	(TLOGD_TCONNDRUN*) pstThread->pstAppCtx->stRunDataStatus.pszBuff;
	pstSerInstList	=	&pstConfInst->stSerInstList;
	pstNetTrans	=	pstConnd->stNetTransList.astNetTrans + pstInst->iTransLoc;

	// Write data to serializer
	while( pstInst->iData >= pstInst->iPkgLen )
	{
		pstInst->iRecvMsg++;
		pstInst->iRecvByte		+=	pstInst->iPkgLen;

		if( pstInst->fWaitFirstPkg || !pstInst->fStream )
		{
			pstInst->fWaitFirstPkg	=	0;
		}

		pstTransRun		=	pstRun->stNetTransRunList.astNetTrans + pstInst->iTransLoc;
		pstLisRun		=	pstRun->stListenerRunList.astListeners + pstInst->iLisLoc;

		pstLisRun->stTransStat.llRecvPkg++;
		pstLisRun->stTransStat.llRecvByte +=	pstInst->iPkgLen;

		/* send to all serializer */
		for(i=0; i<pstTransInst->iSerCount; i++)
		{
			pstSerInst	=	pstSerInstList->astInsts + pstTransInst->aiSerLoc[i];
			pstSerRun	=	pstRun->stSerializerRunList.astSerializers + pstTransInst->aiSerLoc[i];

			if( pstConnd->stSerializerList.astSerializers[pstTransInst->aiSerLoc[i]].iDropHead )
			{
				iMsg	=	pstInst->iPkgLen - pstInst->iHeadLen;
				pszMsg	=	pstInst->szBuff + pstInst->iOff + pstInst->iHeadLen;
			}
			else
			{
				iMsg	=	pstInst->iPkgLen;
				pszMsg	=	pstInst->szBuff + pstInst->iOff;
			}

			if(iMsg > 0)
			{
				//add condition to log source by vinson 20110531
				if (pstConnd->stSerializerList.astSerializers[pstTransInst->aiSerLoc[i]].iLogSource)
				{
					char szSource[64] = {0};
					TLOGIOVEC stIOVec[2];
					snprintf(szSource, 64, "%s:%d ||", inet_ntoa(((struct sockaddr_in *)pstSrcAddr)->sin_addr), 
								ntohs(((struct sockaddr_in *)pstSrcAddr)->sin_port));

					stIOVec[0].iov_base = (void*) szSource;
					stIOVec[0].iov_len = strlen(szSource);

					stIOVec[1].iov_base = (void*) pszMsg;
					stIOVec[1].iov_len = (size_t) iMsg;
					
					iRet = tlogany_writev(&pstSerInst->stLogAny, pstInst->iMsgID, pstInst->iMsgCls, &stIOVec[0], 2);
				}
				else
				{
					iRet	=	tlogany_write( &pstSerInst->stLogAny, pstInst->iMsgID, pstInst->iMsgCls, pszMsg, iMsg);
				}

				if( iRet<0 )
				{
					pstInst->iFailedMsg++;

					pstSerRun->stTransStat.llFailPkg++;
					pstSerRun->stTransStat.llFailByte	+=	iMsg;

					continue;
				}
				else
				{
					pstSerRun->stTransStat.llSendPkg++;
					pstSerRun->stTransStat.llSendByte	+=	iMsg;
				}
			}
			else
			{
				pstInst->iFailedMsg++;

				pstSerRun->stTransStat.llFailPkg++;
				pstSerRun->stTransStat.llFailByte	+=	iMsg;
			}
		}

		pstInst->iData	-=	pstInst->iPkgLen;
		pstInst->iOff	+=	pstInst->iPkgLen;
		pstInst->iPkgLen	=	0;
		iCount++;

		iRet = (*pfnGetPkgLen)(pstInst, pstThread);
		if(0 != iRet)
			break;
	}/*while( pstInst->iData>=pstInst->iPkgLen )*/

	if( pstInst->iOff )
	{
		if( pstInst->iData )
		{
			memmove(pstInst->szBuff, pstInst->szBuff+pstInst->iOff, pstInst->iData);
		}

		pstInst->iOff	=	0;
	}

	return iCount;
}

int tlogd_tconnd_proc_recv(TLOGDTHREAD* pstThread)
{
	int s;
	struct sockaddr sa;
	int iLen;
	struct epoll_event e;
	struct epoll_event events[MAX_EVENTS];
	int iEvents;
	int i;
	int iCount;
	TCONNPOOL* pstPool;
	TLOGD_TCONND* pstConnd;
	TCONNINST* pstNew;
	TCONNINST* pstInst;
	TRANSINST* pstTransInst;
	PDUINST* pstPDUInst;
	LISTENER* pstListener;
	TLOGD_TCONNDRUN* pstRun;
	CONFINST* pstConfInst;
	NETTRANSRUN* pstTransRun;
	LISTENERRUN* pstLisRun;

	pstPool		=	pstThread->pstPool;
	pstConnd	=	(TLOGD_TCONND*)pstThread->pstAppCtx->stConfData.pszBuff;
	pstConfInst	=	pstThread->pstConfInst;
	pstRun		=	(TLOGD_TCONNDRUN*)pstThread->pstAppCtx->stRunDataStatus.pszBuff;

	iCount	=	0;

	iEvents	=	epoll_wait(pstThread->epfd, events, MAX_EVENTS, pstThread->pstAppCtx->iEpollWait);

	if( iEvents<0 )
	{
		return 0;
	}

	for(i=0; i<iEvents; i++)
	{
		// Find the event ctx.
		pstInst	=	tlogd_tconnd_get_inst(pstPool, events[i].data.fd);

		/* bad packet. */
		if( !pstInst )
		{
			printf("pstInst not find, idx=%d\n", events[i].data.fd);
			continue;
		}

		pstInst->tLastRecv	=	gs_tNow;

		pstTransInst	=	pstThread->pstConfInst->stTransInstList.astInsts + pstInst->iTransLoc;

		pstPDUInst		=	pstThread->pstConfInst->stPDUInstList.astInsts + pstTransInst->iPDULoc;
		pstListener		=	pstConnd->stListenerList.astListeners + pstInst->iLisLoc;

		pstTransInst	=	pstConfInst->stTransInstList.astInsts + pstInst->iTransLoc;
		pstTransRun		=	pstRun->stNetTransRunList.astNetTrans + pstInst->iTransLoc;
		pstLisRun		=	pstRun->stListenerRunList.astListeners + pstInst->iLisLoc;


		// The TCP part.
		if( pstInst->fListen && pstInst->fStream )
		{
			iLen	=	(int) sizeof(sa);

			s	=	epoll_accept(pstThread->epfd, pstInst->s, events+i, &sa, &iLen);

			pstLisRun->stConnStat.iTry++;

			// Accept ok, We haveto allocate an instance for it.
			if( s>=0 )
			{
				e.data.fd	=	s;
				e.events	=	EPOLLIN;
				e.data.fd	=	tlogd_tconnd_alloc_inst(pstPool);

				if( e.data.fd<0 )
				{
					pstLisRun->stConnStat.iAllocFail++;
					tnet_close(s);
					continue;
				}

				tnet_set_nonblock(s, 1);

				if( pstListener->iSendBuff>0 )
					tnet_set_sendbuff(s, pstListener->iSendBuff);

				if( pstListener->iRecvBuff>0 )
					tnet_set_recvbuff(s, pstListener->iRecvBuff);

				pstNew		=	tlogd_tconnd_get_inst(pstPool, e.data.fd);

				memset(pstNew, 0, offsetof(TCONNINST, szBuff));

				pstNew->s	=	s;
				pstNew->fListen	=	0;  //The status is accepted.
				pstNew->fStream	=	1;
				pstNew->fWaitFirstPkg	=	1;
				pstNew->iNeedFree		=	0;
				pstNew->iID				=	-1;
				pstNew->iIdx			=	e.data.fd;
				pstNew->iTransLoc		=	pstInst->iTransLoc;

				pstNew->tLastRecv		=	gs_tNow;

				memcpy(&pstNew->stAddr, &sa, sizeof(pstNew->stAddr));

				pstNew->iBuff	=	pstThread->iPkgUnit;

				pstRun->iTotalConns++;

				if( epoll_ctl(pstThread->epfd, EPOLL_CTL_ADD, s, &e)<0 )
				{
					printf("epoll_ctl error.\n");

					pstLisRun->stConnStat.iEpollFail++;

					tlogd_tconnd_safe_close(pstThread, e.data.fd);
				}
				else
				{
					pstLisRun->stConnStat.iActive++;
					pstLisRun->stConnStat.iTotal++;
				}
			}
			else
			{
				pstLisRun->stConnStat.iAcceptFail++;
			}
		}
		else // Recv data from tcp or udp connection.
		{
			int iAddrLen;
			iAddrLen	=	(int) sizeof(pstInst->stAddr);

			if( pstInst->fStream )
			{
				iLen	=	epoll_recv(pstThread->epfd, pstInst->s, events+i, pstInst->szBuff+pstInst->iData, pstInst->iBuff - pstInst->iData);

				// Close the connection.
				if( iLen<=0 )
				{
					epoll_ctl(pstThread->epfd, EPOLL_CTL_DEL, pstInst->s, events+i);

					pstLisRun->stConnStat.iActive--;

					iCount	=	0;

					tlogd_tconnd_safe_close(pstThread, events[i].data.fd);

					continue;
				}
			}
			else
			{
				pstInst->iData	=	0;

				iLen	=	epoll_recvfrom(pstThread->epfd, pstInst->s, events+i, pstInst->szBuff+pstInst->iData, pstInst->iBuff - pstInst->iData,
						&pstInst->stAddr, &iAddrLen);

				if( iLen<=0 )
					continue;
			}

			//
			pstInst->iData	+=	iLen;

			if( pstInst->iData < pstInst->iMinHeadLen )
				continue;

			if( pstInst->iPkgLen && pstInst->iData<pstInst->iPkgLen )
				continue;

			//add addr by vinsonzuo 20110531
			tlogd_tconnd_recv_msg(pstThread, pstInst, &(pstInst->stAddr));

			if( pstInst->iNeedFree )
			{
				/*
				printf("info: connection closed, bad head.\n");
				*/

				epoll_ctl(pstThread->epfd, EPOLL_CTL_DEL, pstInst->s, events+i);

				tlogd_tconnd_safe_close(pstThread, events[i].data.fd);

				continue;
			}

			/* need to process the message. */
		}
	}

	return 0;
}

int tlogd_tconnd_proc_once(TLOGDTHREAD* pstThread)
{
	struct timeval stStart;
	struct timeval stEnd;
	struct timeval stSub;

	gettimeofday(&stStart, NULL);

	tlogd_tconnd_proc_recv(pstThread);

	gettimeofday(&stEnd, NULL);

	TV_DIFF(stSub, stEnd, stStart);
	TV_TO_MS(pstThread->iMsRecv, stSub);

	return 0;
}

int tlogd_tconnd_scan(TLOGDTHREAD* pstThread, int iIsShutdown)
{
	CONFINST* pstConfInst;
	TLOGD_TCONND* pstConnd;
	int iMax;
	int i;
	TMEMBLOCK* pstBlock;
	TCONNINST* pstInst;
	LISTENERLIST* pstListenerList;
	LISTENER* pstListener;

	pstConfInst	=	pstThread->pstConfInst;
	pstConnd	=	(TLOGD_TCONND*)pstThread->pstAppCtx->stConfData.pszBuff;

	iMax	=	TMEMPOOL_GET_CAP(pstThread->pstPool);

	if( !iIsShutdown )
	{
		if( pstConnd->iScanRatioOnce )
			iMax	=	pstConnd->iScanRatioOnce * TMEMPOOL_GET_CAP(pstThread->pstPool)/ 100;

		if( pstConnd->iMaxScanOnce>0 && pstConnd->iMaxScanOnce<iMax )
			iMax	=	pstConnd->iMaxScanOnce;
	}

	pstListenerList	=	&pstConnd->stListenerList;

	for(i=0; i<iMax; i++)
	{
		pstBlock	=	TMEMPOOL_GET_PTR(pstThread->pstPool, pstThread->iScanPos);

		pstThread->iScanPos++;

		pstThread->iScanPos	=	pstThread->iScanPos % (TMEMPOOL_GET_CAP(pstThread->pstPool));

		pstInst = tmempool_get_bypos(pstThread->pstPool, pstThread->iScanPos);

		if( !pstInst)
			continue;

		if( pstInst->fListen && !iIsShutdown )
			continue;

		if( iIsShutdown )
		{
			tlogd_tconnd_safe_close(pstThread, TMEMPOOL_GET_BLOCK_IDX(pstBlock));
		}
		else if( pstInst->iNeedFree )
		{
			tlogd_tconnd_safe_close(pstThread, TMEMPOOL_GET_BLOCK_IDX(pstBlock));
		}
		else
		{
			pstListener	=	pstListenerList->astListeners + pstInst->iLisLoc;
			if( !pstListener->iMaxIdle )
				continue;

			if( (gs_tNow - pstInst->tLastRecv) <= pstListener->iMaxIdle )
				continue;

			/* begin to close the socket. */
			tlogd_tconnd_safe_close(pstThread, TMEMPOOL_GET_BLOCK_IDX(pstBlock));
		}
	}

	return 0;
}

void* tlogd_tconnd_thread_proc(void* pvArg)
{
	TLOGDTHREAD* pstThread;
	TAPPCTX* pstAppCtx;
	int iIdle=0;

	pstThread	=	(TLOGDTHREAD*) pvArg;
	pstAppCtx	=	(TAPPCTX*)pstThread->pstAppCtx;

	while( 1 )
	{
		if( pstThread->iIsExit )
			break;

		if( tlogd_tconnd_proc_once(pstThread)<0 )
			iIdle++;

		if( iIdle>pstAppCtx->iIdleCount )
		{
			iIdle	=	0;
			usleep(pstAppCtx->iIdleSleep);
		}

		if( pstThread->iTickCount!=pstAppCtx->iTickCount )
		{
			pstThread->iTickCount	=	pstAppCtx->iTickCount;
			tlogd_tconnd_scan(pstThread, 0);
		}
	}

	return (void*)pstThread->iIsExit;
}


int tlogd_tconnd_init_multi(TAPPCTX* pstAppCtx, TCONNENV* pstEnv)
{
	int i;
	int j;
	int s;
	int iLimit;
	int iUnit;
	int iRet;
	int iType;
	TCONNPOOL* pstPool;
	TLOGD_TCONND* pstConnd;
	CONFINST* pstConfInst;
	NETTRANSLIST* pstNetTransList;
	LISTENERLIST* pstListenerList;
	SERIALIZERLIST* pstSerializerList;
	TRANSINST* pstTransInst;
	LISTENER* pstListener;
	PDUINST* pstPDUInst;
	TCONNINST* pstInst;
	TLOGDTHREAD* pstThread;
	epoll_event_t e;


	pstConfInst		=	&pstEnv->stConfInst;
	pstConnd		=	(TLOGD_TCONND*)pstAppCtx->stConfData.pszBuff;
	pstNetTransList	=	&pstConnd->stNetTransList;
	pstListenerList	=	&pstConnd->stListenerList;
	pstSerializerList=	&pstConnd->stSerializerList;

	for(i=0; i<pstNetTransList->iCount; i++)
	{
		pstThread	=	&pstEnv->astThreads[i];

		iLimit  =   0;

		pstTransInst    =   &pstConfInst->stTransInstList.astInsts[i];

		for(j=0; j<(int)( sizeof(pstTransInst->aiLisLoc)/sizeof(pstTransInst->aiLisLoc[0]) ); j++)
		{
			pstListener =   pstListenerList->astListeners + pstTransInst->aiLisLoc[j];
			iLimit  +=  pstListener->stConnLimit.iMaxConn;
		}

		if( iLimit<=0 )
			return -1;

		pstPDUInst	=	pstConfInst->stPDUInstList.astInsts + pstConfInst->stTransInstList.astInsts[i].iPDULoc;

		iUnit		=	pstPDUInst->iUnit;

		pstThread->iPkgUnit		=	iUnit;
		pstThread->iPoolUnit	=	pstThread->iPkgUnit + (int) offsetof(TCONNINST, szBuff);

		iRet	=	tlogd_tconnd_init_pool(&pstThread->pstPool, iLimit, pstThread->iPoolUnit);

		if( iRet<0 )
			return -1;

		pstPool		=	pstThread->pstPool;

		pstThread->epfd	=	epoll_create(iLimit);

		if( -1==pstThread->epfd )
		{
			printf("Error: Create epoll-fd[MAXFD=%d] failed.\n", iLimit);
			perror("Error:");
			return -1;
		}

		for(j=0; j<pstTransInst->iLisCount; j++)
		{
			pstListener =   pstListenerList->astListeners + pstTransInst->aiLisLoc[j];

			s	=	tnet_listen(pstListener->szUrl, pstListener->stConnLimit.iBacklog);

			if( -1==s )
			{
				printf("Error: Listen on %s failed.\n", pstListener->szUrl);
				return -1;
			}

			iType	=	SOCK_STREAM;
			tsocket_get_type(s, &iType);

			iRet	=	tnet_set_nonblock(s, 1);
			if( iRet<0 )
			{
				tnet_close(s);

				printf("Error: Set listen socket non-block failed. \n");

				return -1;
			}

			memset(&e, 0, sizeof(e));

			e.events	=	EPOLLIN;
			e.data.fd	=	tlogd_tconnd_alloc_inst(pstPool);

			if(e.data.fd<0)
			{
				tnet_close(s);

				printf("Error: Alloc conn instance for listen socket failed.\n");

				return -1;
			}

			pstInst		=	tlogd_tconnd_get_inst(pstPool, e.data.fd);

			memset(pstInst, 0, offsetof(TCONNINST, szBuff));

			pstInst->fListen	=	1;

			if( SOCK_STREAM==iType )
				pstInst->fStream	=	1;
			else
				pstInst->fStream	=	0;

			pstInst->iID		=	-1;
			pstInst->iIdx		=	e.data.fd;
			pstInst->s	=	s;
			pstInst->iTransLoc	=	i;
			pstInst->iLisLoc	=	pstTransInst->aiLisLoc[j];

			pstInst->tLastRecv	=	gs_tNow;

			memset(&pstInst->stAddr, 0, sizeof(pstInst->stAddr));
			pstInst->iBuff	=	pstThread->iPkgUnit;

			if( -1==epoll_ctl(pstThread->epfd, EPOLL_CTL_ADD, s, &e) )
			{
				tlogd_tconnd_free_inst(pstPool, e.data.fd);
				printf("Error: Add listen socket to epoll-fd failed.\n");
				return -1;
			}
		}
	}

	/* multi-threading: one thread for a net-trans link. */

	for(i=0; i<pstNetTransList->iCount; i++)
	{
		pstThread	=	&pstEnv->astThreads[i];

		pstThread->pstConfInst	=	&pstEnv->stConfInst;
		pstThread->pstAppCtx	=	pstAppCtx;

		if( pthread_create(&pstThread->tid, NULL, tlogd_tconnd_thread_proc, pstThread)<0 )
			return -1;
	}

	return 0;
}

int tlogd_tconnd_init(TAPPCTX* pstAppCtx, TCONNENV* pstEnv)
{
	TLOGD_TCONND* pstConnd;

	if( !pstAppCtx->stConfData.iMeta )
	{
		printf("Error: can not find meta \'%s\'.\n", pstAppCtx->stConfData.pszMetaName);
		return -1;
	}

	if( !pstAppCtx->stRunDataStatus.iMeta )
	{
		printf("Error: can not find meta \'%s\'.\n", pstAppCtx->stRunDataStatus.pszMetaName);
		return -1;
	}

	pstConnd	=	(TLOGD_TCONND*)pstAppCtx->stConfData.pszBuff;

	if( tlogd_tconnd_init_confinst(&pstEnv->stConfInst, pstConnd)<0 )
	{
		return -1;
	}

	if( pstConnd->iThreading )
		return tlogd_tconnd_init_multi(pstAppCtx, pstEnv);
	else
		return tlogd_tconnd_init_single(pstAppCtx, pstEnv);
}

int tlogd_tconnd_fini_multi(TAPPCTX* pstAppCtx, TCONNENV* pstEnv)
{
	int i;
	void* pvRet;
	TLOGDTHREAD* pstThread;
	pstAppCtx=pstAppCtx;
	for(i=0; i<TCONND_MAX_NETTRANS; i++)
	{
		pstThread	=	&pstEnv->astThreads[i];

		if( pstThread->iIsValid )
			pstThread->iIsExit	=(void *)1;
	}

	for(i=0; i<TCONND_MAX_NETTRANS; i++)
	{
		pstThread	=	&pstEnv->astThreads[i];

		if( pstThread->iIsValid )
			pthread_join(pstThread->tid, &pvRet);
	}

	for(i=0; i<TCONND_MAX_NETTRANS; i++)
	{
		pstThread	=	&pstEnv->astThreads[i];

		if( pstThread->iIsValid )
		{
			tlogd_tconnd_scan(pstThread, 1);

			if( -1!=pstThread->epfd )
			{
				epoll_destroy(pstThread->epfd);
				pstThread->epfd	=	-1;
			}

			pstThread->iIsValid	=	0;
		}

		if( pstThread->pszBuff )
		{
			free(pstThread->pszBuff);
			pstThread->pszBuff	=	NULL;
		}
	}

	return 0;
}

int tlogd_tconnd_fini_single(TAPPCTX* pstAppCtx, TCONNENV* pstEnv)
{
	TLOGDTHREAD* pstThread;
	pstAppCtx = pstAppCtx;
	pstThread	=	&pstEnv->astThreads[0];

	tlogd_tconnd_scan(pstThread, 1);

	if( -1!=pstThread->epfd )
	{
		epoll_destroy(pstThread->epfd);
		pstThread->epfd	=	-1;
	}

	if( pstThread->pszBuff )
	{
		free(pstThread->pszBuff);

		pstThread->pszBuff	=	NULL;
	}

	return 0;
}

int tlogd_tconnd_tick_multi(TAPPCTX* pstAppCtx, TCONNENV* pstEnv)
{
	gs_tNow	=	time(NULL);
	pstAppCtx = pstAppCtx;
	pstEnv = pstEnv;
	return 0;
}

int tlogd_tconnd_tick_single(TAPPCTX* pstAppCtx, TCONNENV* pstEnv)
{
	gs_tNow	=	time(NULL);
	pstAppCtx = pstAppCtx;
	pstEnv = pstEnv;
	tlogd_tconnd_scan(&pstEnv->astThreads[0], 0);

	return 0;
}

int tlogd_tconnd_reload(TAPPCTX* pstAppCtx, TCONNENV* pstEnv)
{
	TDRDATA stHost;
	LPTDRMETA pstMeta;
	TCONNINST* pstInst;
	TLOGDTHREAD* pstThread;

	int iPos;
	int iIdx;

	pstMeta	=	(LPTDRMETA) pstAppCtx->stRunDataStatus.iMeta;

	if( !pstMeta )
		return -1;

	stHost.pszBuff	=	(char*)pstAppCtx->stRunDataStatus.pszBuff;
	stHost.iBuff	=	(int) pstAppCtx->stRunDataStatus.iLen;

	tdr_output_fp(pstMeta, stdout, &stHost, 0, 0);

	pstThread	=	&pstEnv->astThreads[0];

	if( tmempool_find_used_first(pstThread->pstPool, &iPos)<0 )
		return 0;

	while( 0 == tmempool_find_used_next(pstThread->pstPool, &iPos) )
	{
		iIdx = iPos;
		pstInst	=	tlogd_tconnd_get_inst(pstThread->pstPool, iIdx);

		printf("conn %""d"": Byte(S:%d/R:%d) Msg(S:%d/R:%d)\n", iIdx, pstInst->iSendByte, pstInst->iRecvByte, pstInst->iSendMsg, pstInst->iRecvMsg);
	}

	return 0;
}

int tlogd_tconnd_reload_multi(TAPPCTX* pstAppCtx, TCONNENV* pstEnv)
{
	return tlogd_tconnd_reload(pstAppCtx, pstEnv);
}

int tlogd_tconnd_reload_single(TAPPCTX* pstAppCtx, TCONNENV* pstEnv)
{
	return tlogd_tconnd_reload(pstAppCtx, pstEnv);
}

int tlogd_tconnd_proc_multi(TAPPCTX* pstAppCtx, TCONNENV* pstEnv)
{
	int i;
	TLOGDTHREAD* pstThread;
	pstAppCtx = pstAppCtx;

	for(i=0; i<TCONND_MAX_NETTRANS; i++)
	{
		pstThread	=	&pstEnv->astThreads[i];

		if( !pstThread->iIsValid )
			continue;

		if( pthread_kill(pstThread->tid, 0)<0 )
		{
			continue;
		}
	}

	usleep(100000);

	return 0;
}

int tlogd_tconnd_proc_single(TAPPCTX* pstAppCtx, TCONNENV* pstEnv)
{
	pstAppCtx = pstAppCtx;
	return tlogd_tconnd_proc_once(&pstEnv->astThreads[0]);
}

int main(int argc, char* argv[])
{
	int iRet;
	void* pvArg	=	&gs_stEnv;

	memset(&gs_stEnv, 0, sizeof(gs_stEnv));
	memset(&gs_stAppCtx, 0, sizeof(gs_stAppCtx));
	gs_stAppCtx.argc	=	argc;
	gs_stAppCtx.argv	=	argv;

	gs_stAppCtx.iTimer	=	100;

	gs_stAppCtx.pfnInit	=	(PFNTAPPFUNC)tlogd_tconnd_init;
	gs_stAppCtx.pfnFini	=	(PFNTAPPFUNC)NULL;
	gs_stAppCtx.pfnProc	=	(PFNTAPPFUNC)NULL;
	gs_stAppCtx.pfnTick	=	(PFNTAPPFUNC)NULL;
	gs_stAppCtx.pfnReload	=	(PFNTAPPFUNC)NULL;

	gs_tNow				=	time(NULL);

	gs_stAppCtx.stConfData.pszMetaName	=	"tlogdconf";
	gs_stAppCtx.stConfPrepareData.pszMetaName	=	gs_stAppCtx.stConfData.pszMetaName;
	gs_stAppCtx.stRunDataStatus.pszMetaName	=	"tlogdrun";

	gs_stAppCtx.iLib	=	(intptr_t)g_szMetalib_tlogd;

	gs_stAppCtx.uiVersion = TAPP_MAKE_VERSION(MAJOR, MINOR, REV, BUILD);

	iRet	=	tapp_def_init(&gs_stAppCtx, pvArg);
	if( iRet<0 )
	{
		printf("Error: Initialization failed.\n");
		return iRet;
	}



	if( gs_stAppCtx.stConfData.iLen<(int)sizeof(TLOGD_TCONND) )
	{
		printf("Error: bad confdata length.\n");
		return -1;
	}

	if( gs_stAppCtx.stRunDataStatus.iLen<(int)sizeof(TLOGD_TCONNDRUN) )
	{
		printf("Error: bad rundata length.\n");
		return -1;
	}

	tapp_get_category(NULL, &gs_pstTlogdCatInst);

	tlogd_tconnd_init_tlogd_tconndrun((TLOGD_TCONNDRUN*)gs_stAppCtx.stRunDataStatus.pszBuff, (TLOGD_TCONND*)gs_stAppCtx.stConfData.pszBuff);

	((TLOGD_TCONNDRUN*)gs_stAppCtx.stRunDataStatus.pszBuff)->iUpTime	=	(int) time(NULL);

	if( ((TLOGD_TCONND*)gs_stAppCtx.stConfData.pszBuff)->iThreading )
	{
		gs_stAppCtx.pfnFini	=	(PFNTAPPFUNC)tlogd_tconnd_fini_multi;
		gs_stAppCtx.pfnProc	=	(PFNTAPPFUNC)tlogd_tconnd_proc_multi;
		gs_stAppCtx.pfnTick	=	(PFNTAPPFUNC)tlogd_tconnd_tick_multi;
		gs_stAppCtx.pfnReload	=	(PFNTAPPFUNC)tlogd_tconnd_reload_multi;
	}
	else
	{
		gs_stAppCtx.pfnFini	=	(PFNTAPPFUNC)tlogd_tconnd_fini_single;
		gs_stAppCtx.pfnProc	=	(PFNTAPPFUNC)tlogd_tconnd_proc_single;
		gs_stAppCtx.pfnTick	=	(PFNTAPPFUNC)tlogd_tconnd_tick_single;
		gs_stAppCtx.pfnReload	=	(PFNTAPPFUNC)tlogd_tconnd_reload_single;
	}

	gs_tNow				=	time(NULL);

	iRet	=	tapp_def_mainloop(&gs_stAppCtx, pvArg);

	tapp_def_fini(&gs_stAppCtx, pvArg);

	return iRet;
}



int tlogd_tconnd_get_thttp_pkglen(TCONNINST* pstInst, TLOGDTHREAD* pstThread)
{
	TRANSINST* pstTransInst;
	PDUINST* pstPDUInst;
	LPPDULENTHTTPPARSERINST pstThttpParser ;
	char *pszDataBase;
	int iMaxLen;
	int i;

	assert(NULL != pstInst);
	assert(NULL != pstThread);
	assert(0 <= pstInst->iPkgLen);

	if (0 >= pstInst->iData)
	{
		return -1;
	}

	pstInst->iMsgID		=	0;
	pstInst->iMsgCls	=	0;

	pstInst->iHeadLen	=	0;

	pstTransInst	=	pstThread->pstConfInst->stTransInstList.astInsts + pstInst->iTransLoc;
	pstPDUInst	=	pstThread->pstConfInst->stPDUInstList.astInsts + pstTransInst->iPDULoc;
	pstThttpParser = &pstPDUInst->stLenParser.stThttpParser;


	/*分析数据流，以'\0'字节进行分包*/
	iMaxLen = pstInst->iData;
	if (iMaxLen > pstThttpParser->iMaxUpPkgLen)
	{
		iMaxLen = pstThttpParser->iMaxUpPkgLen;
	}
	pszDataBase = pstInst->szBuff + pstInst->iOff;
	for (i = pstInst->iPkgLen; i < iMaxLen; i++)
	{
	   if ('\0' == pszDataBase[i])
	   {
		   break;
	   }
	}/*for (i = pstInst->iPkgLen; i < iMaxLen; i++)*/
	if (i >= iMaxLen)
	{
		pstInst->iPkgLen = i;
		return -1;
	}else
	{
		pstInst->iPkgLen = i + 1;
	}


	if ( pstInst->iPkgLen > pstThttpParser->iMaxUpPkgLen)
	{
		/*上行包超过最大长度限制，则直接断开连接*/
		pstInst->iNeedFree	=	1;
		return -1;
	}

	return 0;
}

int tlogd_tconnd_get_none_pkglen(TCONNINST* pstInst, TLOGDTHREAD* pstThread)
{
	pstThread = pstThread;
	if (0 >= pstInst->iData)
	{
		return -1;
	}
	else
	{
		pstInst->iHeadLen	=	0;
		pstInst->iPkgLen	=	pstInst->iData;
		pstInst->iMsgID		=	0;
		pstInst->iMsgCls	=	0;
		return 0;
	}
}

