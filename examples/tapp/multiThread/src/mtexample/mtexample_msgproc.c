#include "mtexample.h"

int mtexample_onlymain_req(TAPPCTX *pstAppCtx,MTEXAMPLEENV *pstEnv)
{
	LPMTEXAMPLEMSG pstReq;
	LPMTEXAMPLEMSG pstRes;

	pstReq = &pstEnv->stReq;
	pstRes = &pstEnv->stRes;

	pstRes->stBody.stOnlyMainRes.iRes = pstReq->stBody.stOnlyMainReq.iAdd1 + pstReq->stBody.stOnlyMainReq.iAdd2;
	//= pstReq->stBody.stOnlyMainReq.iAdd1 + pstReq->stBody.stOnlyMainReq.iAdd2;

	pstRes->stHead = pstReq->stHead;

	pstRes->stHead.wCmdID = MT_ONLYMAIN_RES;
	pstRes->stBody.stOnlyMainRes.iResult= MT_NOERR;
	pstRes->stBody.stOnlyMainRes.stReq = pstReq->stBody.stOnlyMainReq;
	
	return 0;
}

int mtexample_msg_proc_timeout(TAPPCTX *pstAppCtx,MTEXAMPLEENV *pstEnv)
{
	//I have to deal with the meg type and return thre result.
	pstEnv->stRes.stHead = pstEnv->stReq.stHead;
	
	switch(pstEnv->stReq.stHead.wCmdID)
	{
		case MT_ONLYMAIN_REQ:
			pstEnv->stRes.stHead.wCmdID = MT_ONLYMAIN_RES;
			pstEnv->stRes.stBody.stOnlyMainRes.iResult = MT_ERROR_TIMEOUT;
			break;
			
		case MT_ONLYTHREAD_REQ:
			pstEnv->stRes.stHead.wCmdID = MT_ONLYTHREAD_RES;
			pstEnv->stRes.stBody.stOnlyMainRes.iResult = MT_ERROR_TIMEOUT;
			break;

		default:
			break;
	}
	return 0;
}

int mtexmaple_thread_onlythread_msg(TAPPTHREADCTX *pstCtx,MTEXAMPLETHREADENV *pstEnv)
{
	pstEnv->stRes.stHead = pstEnv->stReq.stHead;
	pstEnv->stRes.stHead.wCmdID = MT_ONLYTHREAD_RES;
	pstEnv->stRes.stBody.stOnlyThreadRes.stReq = pstEnv->stReq.stBody.stOnlyThreadReq;
	pstEnv->stRes.stBody.stOnlyThreadRes.iResult = MT_NOERR;
	pstEnv->stRes.stBody.stOnlyThreadRes.iRes = pstEnv->stReq.stBody.stOnlyThreadReq.iMult1 * \
		pstEnv->stReq.stBody.stOnlyThreadReq.iMult2;
	return 0;
}

int mtexample_thread_process_msg(TAPPTHREADCTX *pstCtx,MTEXAMPLETHREADENV *pstEnv)
{
	switch(pstEnv->stReq.stHead.wCmdID)
	{
		case MT_ONLYTHREAD_REQ:
			mtexmaple_thread_onlythread_msg(pstCtx,pstEnv);
			break;

		default:
		break;
	}
	
	return 0;
}




