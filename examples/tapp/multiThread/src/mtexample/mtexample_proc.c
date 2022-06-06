#include "mtexample.h"

//Return 0 if message recv
int mtexample_recv_client_req(TAPPCTX *pstAppCtx,MTEXAMPLEENV *pstEnv)
{
	int iDest = gs_stAppCtx.iId;
	int iRet;
	TDRDATA stHost;
	TDRDATA stNet;
	pstEnv->iClientBuff = sizeof(pstEnv->szClientBuff);
	pstEnv->iSrc = -1;
	iRet = tbus_recv(gs_stAppCtx.iBus,&pstEnv->iSrc,&iDest,pstEnv->szClientBuff,&pstEnv->iClientBuff,0);
	if(iRet != 0)
	{
		return -1;
	}

	stHost.iBuff = sizeof(pstEnv->stReq);
	stHost.pszBuff = (char *)&pstEnv->stReq;

	stNet.iBuff = pstEnv->iClientBuff;
	stNet.pszBuff = pstEnv->szClientBuff;

	iRet = tdr_ntoh(pstEnv->pstMetaMsg,&stHost,&stNet,0);
	if(iRet)
	{
		return -1;
	}
	return 0;
}

int mtexample_proc_client_req(TAPPCTX *pstAppCtx,MTEXAMPLEENV *pstEnv)
{
	switch(pstEnv->stReq.stHead.wCmdID)
	{
		case MT_ONLYMAIN_REQ:
			//Deal the message and call backward.
			mtexample_onlymain_req(pstAppCtx,pstEnv);
			mtexample_backward_to_client(pstAppCtx,pstEnv);
			break;
		case MT_ONLYTHREAD_REQ:
			//I have to send the message to client.
			//I have to backward the message to main.
			mtexample_forward_to_thread(pstAppCtx,pstEnv);
			//Pack the message and send it to main.
			break;

		case MT_ONLYTHREAD_RES:
			break;
		case MT_ONLYMAIN_RES:
			break;
		default:
			//Error
			break;
	}

	return 0;
}


int mtexample_proc_thread_res(TAPPCTX *pstAppCtx,MTEXAMPLEENV *pstEnv)
{
	int iRet;
	MTSSDATA stData;

	TDRDATA stHost;
	TDRDATA stNet;

	pstEnv->iClientBuff = sizeof(pstEnv->szClientBuff);
	iRet = tapp_recv_from_thread(pstAppCtx,NULL,\
				pstEnv->szClientBuff,&pstEnv->iClientBuff);

	if(iRet)
	{
		return -1;
	}

	stNet.iBuff = sizeof(pstEnv->szClientBuff);
	stNet.pszBuff = pstEnv->szClientBuff;

	stHost.iBuff = sizeof(stData);
	stHost.pszBuff = (char *)&stData;

	iRet = tdr_ntoh(pstEnv->pstMetaSS,&stHost,&stNet,0);
	if(iRet )
	{
		assert(0);
		return -1;
	}

    iRet = tapp_backward_from_thread(&gs_stAppCtx,stData.szData,stData.iLen);
	if(iRet)
	{
		return -1;
	}

	iRet = mtexample_task_del(pstEnv,stData.iIIdx);
	if(iRet)
	{

		tlog_error(pstEnv->pstLogCat,0,0,"Failed to delete task by id %d",stData.iIIdx);
		return -1;
	}

	return 0;
}

int mtexample_thread_recv_msg(TAPPTHREADCTX *pstCtx,MTEXAMPLETHREADENV *pstEnv)
{
	int iRet;
	TDRDATA stHost;
	TDRDATA stNet;

	pstEnv->iClientBuff = sizeof(pstEnv->szClientBuff);
	iRet = tapp_thread_recv(pstCtx,pstEnv->szClientBuff,&pstEnv->iClientBuff);
	if(iRet)
	{
		return -1;
	}

	stHost.iBuff = sizeof(pstEnv->stData);
	stHost.pszBuff = (char *)&pstEnv->stData;

	stNet.iBuff = pstEnv->iClientBuff;
	stNet.pszBuff  = pstEnv->szClientBuff;

	iRet = tdr_ntoh(pstEnv->pstMetaSS,&stHost,&stNet,0);

	if(iRet)
	{
		tlog_error(pstCtx->pstLogCategory,0,0,"Failed to decode msg(MetaSS) from main thread:%s",tdr_error_string(iRet));
		return -1;
	}

	stHost.iBuff = sizeof(pstEnv->stReq);
	stHost.pszBuff = (char *)&pstEnv->stReq;
	stNet.iBuff = pstEnv->stData.iLen;
	stNet.pszBuff  = pstEnv->stData.szData;

	iRet = tdr_ntoh(pstEnv->pstMetaMsg,&stHost,&stNet,0);
	if(iRet)
	{
		tlog_error(pstCtx->pstLogCategory,0,0,"Failed to decode msg(MetaCS) from main thread:%s",tdr_error_string(iRet));
		return -1;
	}

	return 0;
}

int mtexample_thread_backward_msg(TAPPTHREADCTX *pstCtx,MTEXAMPLETHREADENV *pstEnv)
{
	TDRDATA stHost;
	TDRDATA stNet;
	int iRet;

	stHost.iBuff = sizeof(pstEnv->stRes);
	stHost.pszBuff = (char *)&pstEnv->stRes;

	stNet.iBuff = sizeof(pstEnv->stData.szData);
	stNet.pszBuff = pstEnv->stData.szData;

	iRet = tdr_hton(pstEnv->pstMetaMsg,&stNet,&stHost,0);

	if(iRet)
	{
		return -1;
	}

	pstEnv->stData.iLen = stNet.iBuff;


	stHost.iBuff = sizeof(pstEnv->stData);
	stHost.pszBuff = (char *)&pstEnv->stData;

	stNet.iBuff = sizeof(pstEnv->szToClientBuff);
	stNet.pszBuff = pstEnv->szToClientBuff;

	iRet = tdr_hton(pstEnv->pstMetaSS,&stNet,&stHost,0);

	if(iRet)
	{
		return -1;
	}

	iRet = tapp_thread_backward(pstCtx,stNet.pszBuff,stNet.iBuff);

	if(iRet)
	{
		return -1;
	}

	return 0;
}

int mtexample_timeout_proc(TAPPCTX *pstAppCtx,MTEXAMPLEENV *pstEnv)
{
#if 0
	//Get the tail of the pool. And see if it has timed out.
	MTEXAMPLETASK *pstTask;
	int iIdx;
	int iRet;

	while(1)
	{
		int64_t idiff;
		iIdx = mtexample_task_get_first(pstEnv);

		if(iIdx == -1)
		{
			break;
		}

		pstTask = mtexample_task_get(pstEnv,iIdx);

		if(!pstTask)
		{
			return -1;
		}

		idiff =( pstTask->timeOutTick - pstEnv->iTick);
		#define abs(x) ((x) > 0 ? (x):(-(x)))
		if(abs(idiff) < 10)
		{
			pstEnv->stReq = pstTask->stReq;
			mtexample_msg_proc_timeout(pstAppCtx,pstEnv);
			iRet = mtexample_send_to_client(pstAppCtx,pstEnv,pstTask->iSrc);
			if(iRet)
			{
				return -1;
			}

			iRet = mtexample_task_del(pstEnv,iIdx);
			if(iRet)
			{
				return -1;
			}
		}
		else
		{
			break;
		}
	}
#endif
	return 0;
}

int mtexample_send_to_client(TAPPCTX *pstAppCtx,MTEXAMPLEENV *pstEnv,int iDest)
{
	TDRDATA stHost;
	TDRDATA stNet;
	int iRet;
	int iSrc = 0;
	int iDst = 0;

	iSrc = gs_stAppCtx.iId;
	iDst = iDest;

	stHost.iBuff = sizeof(pstEnv->stRes);
	stHost.pszBuff = (char *)&pstEnv->stRes;

	stNet.iBuff = sizeof(pstEnv->szToClientBuff);
	stNet.pszBuff = (char *)pstEnv->szToClientBuff;

	iRet = tdr_hton(pstEnv->pstMetaMsg,&stNet,&stHost,0);

	if(iRet)
	{
		return -1;
	}

	iRet = tbus_send(gs_stAppCtx.iBus,&iSrc,&iDst,stNet.pszBuff,stNet.iBuff,0);
	if(iRet)
	{
		return -1;
	}

	return 0;

}


int mtexample_backward_to_client(TAPPCTX *pstAppCtx,MTEXAMPLEENV *pstEnv)
{
	TDRDATA stHost;
	TDRDATA stNet;
	int iRet;
	int iSrc = 0;
	int iDst = 0;

	stHost.iBuff = sizeof(pstEnv->stRes);
	stHost.pszBuff = (char *)&pstEnv->stRes;

	stNet.iBuff = sizeof(pstEnv->szToClientBuff);
	stNet.pszBuff = (char *)pstEnv->szToClientBuff;

	iRet = tdr_hton(pstEnv->pstMetaMsg,&stNet,&stHost,0);

	if(iRet)
	{
		tlog_error(pstEnv->pstLogCat,0,0,"Failed to pack msg:%s",tdr_error_string(iRet));
		return -1;
	}
	//fprintf(stderr,"%d\n",pstAppCtx->iBus);
	iRet = tbus_backward(pstAppCtx->iBus,&iSrc,&iDst,stNet.pszBuff,stNet.iBuff,0);
	if(iRet)
	{
		tlog_error(pstEnv->pstLogCat,0,0,"Failed to send msg:%s",tbus_error_string(iRet));
		return -1;
	}

	return 0;
}

int mtexample_forward_to_thread(TAPPCTX *pstAppCtx,MTEXAMPLEENV *pstEnv)
{
	static int iLastDispatchThread = 0;
	int iRet;
	int iIdx;
	MTEXAMPLETASK *pstTask;
	MTSSDATA stData;
	TDRDATA stHost;
	TDRDATA stNet;

	iLastDispatchThread ++;
	iLastDispatchThread %= pstEnv->inThread;

	iIdx = mtexample_task_new(pstEnv);
	if(-1 == iIdx)
	{
		return -1;
	}

	pstTask = mtexample_task_get(pstEnv,iIdx);

	if(!pstTask)
	{
		tlog_error(pstEnv->pstLogCat,0,0,"Error Failed to create task");
		return -1;
	}

	pstTask->timeOutTick =pstEnv->iTick+1000;
	pstTask->stReq = pstEnv->stReq;
	pstTask->iSrc = pstEnv->iSrc;

	stData.iIIdx = iIdx;
	stData.iLen = pstEnv->iClientBuff;
	if(stData.iLen > sizeof(stData.szData))
	{
		return -1;
	}

	memcpy(stData.szData,pstEnv->szClientBuff,stData.iLen);

	stHost.iBuff = sizeof(stData);
	stHost.pszBuff = (char *)&stData;

	stNet.iBuff = sizeof(pstEnv->szToClientBuff);
	stNet.pszBuff = pstEnv->szToClientBuff;

	iRet = tdr_hton(pstEnv->pstMetaSS,&stNet,&stHost,0);

	if(iRet)
	{
		tlog_error(pstEnv->pstLogCat,0,0,"[%d] Failed to pack msg",iIdx);
		return -1;
	}

	iRet = tapp_forward_to_thread(&gs_stAppCtx,pstEnv->apstThreadCtx[iLastDispatchThread],\
		stNet.pszBuff,stNet.iBuff);

	if(iRet)
	{
		tlog_error(pstEnv->pstLogCat,0,0,"[%d] failed to dispatch to thread %d:",iIdx,iLastDispatchThread);
		return -1;
	}

	return 0;
}
//Send the res back to user.
int mtexample_backward_thread_res(MTEXAMPLEENV *pstEnv)
{
	return 0;
}


