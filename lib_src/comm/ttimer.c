#if 0

#include "comm/ttimer.h"
#include "tmempool_i.h"
#include "comm/tlist.h"
#include "comm_i.h"

#define MAX_CALL_BACK 1024
#define MIN_CALL_BACK 1



#if !defined (_WIN32) && !defined (_WIN64)
#define TTIMER_ERROR(args...) snprintf(gs_lastError,sizeof(gs_lastError),##args)
#else
#define TTIMER_ERROR(...) snprintf(gs_lastError,sizeof(gs_lastError),__VA_ARGS__)

#endif
#define global
static char gs_lastError[1024];

int ttimer_add_internal(LPTTIMERHANDLE a_pstTimer,LPTTIMERELEMENT pstElement);



size_t ttimer_calc_min_size(size_t a_max_timer)
{
	size_t res = sizeof(TTIMERHANDLE);
	res += TMEMPOOL_CALC(a_max_timer,sizeof(TTIMERELEMENT));
	return res;
}

int ttimer_init_handler_from_buffer( LPTTIMERHANDLE *a_handle, size_t a_max_timer, char *a_pszBuff,size_t a_iBuff)
{
	LPTTIMERHANDLE pstTimerHandle;
	size_t iMempoolSize;
	int iRet;
	int i,j;

	if(a_iBuff < ttimer_calc_min_size(a_max_timer))
	{
		snprintf(gs_lastError,sizeof(gs_lastError),"The buffer size should be larger than %"PRIdPTR"\n", ttimer_calc_min_size(a_max_timer));
		return -1;
	}

	memset(a_pszBuff, 0,a_iBuff);

	*a_handle = (LPTTIMERHANDLE)a_pszBuff;
	pstTimerHandle = *a_handle;

	pstTimerHandle->iMaxTimer = a_max_timer;
	pstTimerHandle->pvBase = (void *)((intptr_t)pstTimerHandle + sizeof(TTIMERHANDLE));
	iMempoolSize = a_iBuff - sizeof(TTIMERHANDLE);
	iRet = tmempool_init(&pstTimerHandle->pstTimerPool, pstTimerHandle->iMaxTimer, sizeof(TTIMERELEMENT),
					pstTimerHandle->pvBase, iMempoolSize);

	//I have to init all the lists.
	for(i  = 0;i < NSLICE;i++)
	{
		for(j = 0;j < 256;j++)
		{
			TLIST_INIT(&(pstTimerHandle->tv[i][j].stList));
			//tlist_init_from_mem(&(pstTimerHandle->tv[i][j].stList),pstTimerHandle->pstTimerPool);
		}
	}

	if(iRet != 0)
	{
		snprintf(gs_lastError,sizeof(gs_lastError),"tmempool init failed\n");
		return iRet;
	}
	return 0;
}

int ttimer_init(LPTTIMERHANDLE *ppstTimer,size_t a_max_timer)
{
	intptr_t iSize;
	void *pvBuff;
	if(!ppstTimer)
	{
		return -1;
	}

	iSize = ttimer_calc_min_size(a_max_timer);
	pvBuff = calloc(1,iSize);

	if(!pvBuff)
	{
		return -1;
	}

	return ttimer_init_handler_from_buffer(ppstTimer,a_max_timer,pvBuff,iSize);
}

int ttimer_register_callback(LPTTIMERHANDLE a_handle,int a_id,TTIMERCALLBACK a_pfnCallBack)
{
	if(!a_pfnCallBack)
	{
		return -1;
	}

	if(a_id > MAX_CALL_BACK || a_id < MIN_CALL_BACK)
	{
		snprintf(gs_lastError,sizeof(gs_lastError),"The function id should be within range [%d,%d]\n",MIN_CALL_BACK,MAX_CALL_BACK);
		return -1;
	}
	 a_handle->apfnCallBack[a_id] = a_pfnCallBack;
	 return 0;
}

int ttimer_register_callbacks(LPTTIMERHANDLE pstTimer,LPTTIMERTICKCALLBACK astCallBack,int iCount)
{
	int i;
	int iRet;
	if(!pstTimer || !astCallBack)
	{
		return -1;
	}

	for(i = 0;i < iCount;i++)
	{
		iRet = ttimer_register_callback(pstTimer,astCallBack[i].iCallbackId,astCallBack[i].pfnCallback);
		if(iRet)
		{
			return -1;
		}
	}

	return 0;
}


TTIMERCALLBACK ttimer_get_callback_by_id(LPTTIMERHANDLE a_handle,int a_id)
{
	if(a_id > MAX_CALL_BACK || a_id < MIN_CALL_BACK)
	{
		return NULL;
	}
	return a_handle->apfnCallBack[a_id];
}

LPTTIMERELEMENT ttimer_new_element_i(LPTTIMERHANDLE a_handle,int *piIdx)
{
	LPTTIMERELEMENT result = NULL;
	intptr_t index;
	index = tmempool_alloc(a_handle->pstTimerPool);
	*piIdx = index;
	if(index > 0)
	{
		result = tmempool_get(a_handle->pstTimerPool, index);
	}

	return result;
}

int ttimer_add(LPTTIMERHANDLE a_handle, LPTTIMER a_pstTimer)
{
	LPTTIMERELEMENT pstElement;
	tick_t  timeoutTime;
	int iIdx = 0;

	pstElement = ttimer_new_element_i(a_handle,&iIdx);
	if(NULL == pstElement)
	{
		snprintf(gs_lastError,sizeof(gs_lastError),"Failed to allocate new element for timer\n");
		return -1;
	}

	pstElement->stTimer = *a_pstTimer;

	timeoutTime = a_handle->currentTime + a_pstTimer->timeOut;
	pstElement->timeoutTime = timeoutTime;

	ttimer_add_internal(a_handle, pstElement);

	return iIdx;
}

int ttimer_add_internal(LPTTIMERHANDLE a_pstTimer,LPTTIMERELEMENT pstElement)
{
	tick_t diff = (pstElement->timeoutTime) ^ (a_pstTimer->currentTime);
	int i;
	for(i = 0; i < NSLICE; i++)
	{
		int idx;
		int j = NSLICE - i-1;
		idx = 0xff & (diff >> 8 * j);

		if(idx)
		{
			idx = 0xff & (pstElement->timeoutTime >> 8 * j);
			//tlist_insert_prev(&((a_pstTimer->tv)[j][idx].stList),&(pstElement->stNode));

			TLIST_INSERT_NEXT(&((a_pstTimer->tv)[j][idx].stList),&(pstElement->stNode));
			break;
		}
	}

	return 0;
}

TTIMER *ttimer_find_by_index(LPTTIMERHANDLE a_handle, int a_index)
{
	return 0;
}


int ttimer_del(LPTTIMERHANDLE a_handle, int iIdx)
{
	TTIMERELEMENT *pstEle;
	pstEle = tmempool_get(a_handle->pstTimerPool,iIdx);
	if(pstEle)
	{
		TLIST_DEL(pstEle->stNode);
		tmempool_free(a_handle->pstTimerPool,iIdx);
		return 0;
	}
	return -1;
}

#if 0
int tlist_add_Callback(LPTLIST pstList,void *pstData,void *a_Handle)
{
	//	LPTTIMERHANDLE pstTimer = a_Handle;

	return 0;
}
#endif

int ttimer_cascade_inner(LPTTIMERHANDLE a_handle,TTIMERVAL* pstTV, tick_t iIdx)
{

	TLISTNODE *pstNode;
	TLISTNODE *pstTmp;
	TLIST_FOR_EACH_SAFE(pstNode,pstTmp,&(pstTV[iIdx].stList))
	{
		LPTTIMERELEMENT pstElement = container(TTIMERELEMENT,stNode, pstNode);
		TLIST_DEL(pstNode);
		//tlist_remove(pstList,&(pstElement->stNode));
		ttimer_add_internal(a_handle,pstElement);
	}
	//tlist_for_each(&(pstTV[iIdx].stList),tlist_add_Callback,a_handle);
	return iIdx;
}

#if 0
int tlist_call_Callback(LPTLIST pstList,void *pstData,void *a_Handle)
{
	LPTTIMERHANDLE pstTimer = a_Handle;
	LPTTIMERELEMENT pstElement = pstData;
	TTIMERCALLBACK pfnCallback;
	if((pfnCallback = ttimer_get_callback_by_id(a_Handle,pstElement->stTimer.iCallback)))
	{
		printf("Current time is %d\n",pstTimer->currentTime);
		pfnCallback(&pstElement->stTimer);
	}
	tlist_remove(pstList,&(pstElement->stNode));
	return 0;
}
#endif

int ttimer_do_timer(LPTTIMERHANDLE a_handle,TTIMERVAL *pstTv)
{
	//tlist_for_each(&(pstTv->stList),tlist_call_Callback,a_handle);
	TLISTNODE *pstNode,*pstTmp;
	TLIST_FOR_EACH_SAFE(pstNode,pstTmp,&(pstTv->stList))
	{
		LPTTIMERELEMENT pstElement = container(TTIMERELEMENT,stNode,pstNode);
		TTIMERCALLBACK pfnCallback;
		if((pfnCallback = ttimer_get_callback_by_id(a_handle,pstElement->stTimer.iCallback)))
		{
			//printf("Current time is %d\n",a_handle->currentTime);
			pfnCallback(&pstElement->stTimer);
		}
		TLIST_DEL(pstNode);
		//tlist_remove(pstList,&(pstElement->stNode));
		return 0;
	}

	return 0;
}

int ttimer_tick(LPTTIMERHANDLE a_handle,int MaxExpire)
{
	tick_t iIdx;
	int i;
	a_handle->currentTime++;

	iIdx = a_handle->currentTime & 0xff;

	ttimer_do_timer(a_handle,&(a_handle->tv[0][iIdx]));

	if(!iIdx)
	{
		for(i = 1;i < NSLICE ;i++)
		{
			iIdx = (a_handle->currentTime  >> (8*i))&0xff;
			if(ttimer_cascade_inner(a_handle,a_handle->tv[i],iIdx))
			{
				break;
			}
		}
	}

	return 0;
}

#endif
