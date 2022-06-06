
#if 0
#include "memlog.h"
#include "pal/pal.h"
#define LOG_BUFFER_LEN (512)
#include "comm/tlist.h"
#include "tlog/tlog_bill.h"


#ifndef min
#define min(a, b)	((a) < (b) ? (a) : (b))
#endif

struct tagMemLogItem
{
	char pszBuff[LOG_BUFFER_LEN];
	intptr_t iBuff;
	int iSessionID;
	TLISTNODE stBuffList;
	TLISTNODE stLockList;
};

typedef struct tagMemLogItem MEMLOGITEM;
typedef struct tagMemLogItem *LPMEMLOGITEM;

struct tagMemLog
{
	int iSessionID;
	int iMagic;
	int iIsInited;
	int iMaxBuff;
	char szOutFileName[256];
	LPTLIST pstBuffList;
	LPTLIST pstLockList;
	LPTMEMPOOL pstPool;
};

typedef struct tagMemLog MEMLOG;

int tmemlog_init(LPMEMLOG pstMemLog,char *outFileName,int inMaxItem);

int tmemlog_new(LPMEMLOG *ppstMemLog,int inMaxItem,char *outFileName)
{
	int iRet;
	if(!ppstMemLog || !outFileName)
	{
		return -1;
	}

	*ppstMemLog = calloc(1,sizeof(MEMLOG));
	if(!*ppstMemLog )
	{
		return -1;
	}

	iRet = tmemlog_init(*ppstMemLog,outFileName,inMaxItem);
	if(iRet)
	{
		return -1;
	}
	return 0;
}

int tmemlog_init(LPMEMLOG pstMemLog,char *outFileName,int inMaxItem)
{
	int iRet;
	memset(pstMemLog,0,sizeof(*pstMemLog));

	iRet = tmempool_new(&pstMemLog->pstPool,inMaxItem,sizeof(MEMLOGITEM));
	if(iRet)
	{
		return -1;
	}

	iRet = tlist_new(&pstMemLog->pstLockList,pstMemLog->pstPool);
	if(iRet)
	{
		return -1;
	}

	iRet = tlist_new(&pstMemLog->pstBuffList,pstMemLog->pstPool);
	if(iRet)
	{
		return -1;
	}

	strncpy(pstMemLog->szOutFileName,outFileName,sizeof(pstMemLog->szOutFileName));
	return 0;
}

int tmemlog_append_i(LPMEMLOG pstMemLog,char *pszBuff,size_t iBuff)
{
	LPMEMLOGITEM pstItem;
	int iIdx,iRet;

	if(!pstMemLog)
	{
		return -1;
	}

	if(tmempool_is_full(pstMemLog->pstPool))
	{
		pstItem = (LPMEMLOGITEM)tlist_get_head(pstMemLog->pstBuffList);
		if(!pstItem)
		{
			return -1;
		}

		tlist_remove(pstMemLog->pstBuffList, &pstItem->stBuffList);
		tlist_insert_prev(pstMemLog->pstBuffList, &pstItem->stBuffList);
	}
	else
	{
		iIdx = tmempool_alloc(pstMemLog->pstPool);
		pstItem = tmempool_get(pstMemLog->pstPool, iIdx);

		if(!pstItem)
		{
			return -1;
		}

		iRet = tlist_insert_prev(pstMemLog->pstBuffList, &pstItem->stBuffList);
		if(iRet)
		{
			return -1;
		}
	}

	pstItem->iBuff = min(sizeof(pstItem->pszBuff),iBuff);
	memcpy(pstItem->pszBuff,pszBuff,pstItem->iBuff);

	return 0;
}


int tmemlog_append(LPMEMLOG pstMemLog,char *pszBuff,size_t iBuff)
{
	size_t iWrited = 0;
	int iToWrite = 0;
	iWrited = 0;
	while(iWrited < iBuff)
	{
		iToWrite = min(iBuff - iWrited, LOG_BUFFER_LEN);
		tmemlog_append_i(pstMemLog,pszBuff+iWrited,iToWrite);
		iWrited += iToWrite;
	}
	return 0;
}
#define TLOG_BUFFER_SIZE_DEFAULT 4096

int tmemlog_print(LPMEMLOG pstMemLog, const char* a_pszFmt, ...)
{
	va_list ap;
    char szBuff[TLOG_BUFFER_SIZE_DEFAULT];
	intptr_t iBuff;
	intptr_t iMsgLen;
    int iRet;

	if(!pstMemLog)
	{
		return -1;
	}

	if( NULL == pstMemLog)
	{
		return -1;
	}

	if(NULL == a_pszFmt)
	{
		return -1;
	}

	memset(szBuff,0,sizeof(szBuff));
	iBuff = 0;

    va_start(ap, a_pszFmt);

    errno = 0;
    iMsgLen	=	vsnprintf(szBuff+iBuff, sizeof(szBuff) - iBuff, a_pszFmt, ap);

#if defined (_WIN32) || defined (_WIN64)
    if (iMsgLen < 0)
    {
        if (EINVAL == errno)
        {
            return -1;
        }

    }
#else
    if( iMsgLen<0 )
    {
        return -1;
    }

#endif

	iBuff += iMsgLen;
	if(iBuff < sizeof(szBuff))
	{
		szBuff[iBuff] = '\n';
		iBuff++;
	}

	iRet = tmemlog_append(pstMemLog,szBuff,iBuff);
	if(iRet)
	{
		return -1;
	}

    va_end(ap);
    return 0;
}

int tmemlog_lock(LPMEMLOG pstMemLog,int CtxId)
{
    return 0;
}

int tmemlog_for_each(LPTLIST pstList,void *pv1,void *pv2)
{
	int i = 0;
	LPMEMLOGITEM pstItem = pv1;
	FILE *fd = pv2;
	for(i= 0;i < pstItem->iBuff;i++)
	{
		fprintf(fd,"%c",pstItem->pszBuff[i]);
	}
	return 0;
}

int tmemlog_dump_to_fd(LPMEMLOG pstMemLog,FILE *fd)
{
	tlist_for_each(pstMemLog->pstBuffList, tmemlog_for_each, fd);
	return 0;
}

int tmemlog_for_each_bill(LPTLIST pstList,void *pv1,void *pv2)
{
	LPMEMLOGITEM pstItem = pv1;
	LPTLOGBILL pstBill = pv2;
	tlog_bill_write(pstBill,pstItem->pszBuff,pstItem->iBuff);
	return 0;
}

int tmemlog_dump_to_file(LPMEMLOG pstMemLog,LPTLOGBILL pstbill)
{
	tlist_for_each(pstMemLog->pstBuffList, tmemlog_for_each_bill, pstbill);
	return 0;
}

int tmempool_dump(LPMEMLOG pstMemLog)
{
	int iRet;
	LPTLOGBILL pstBill;
	iRet = tlog_bill_init(&pstBill,10240000, 36000, pstMemLog->szOutFileName);
	if(!iRet)
	{
		tmemlog_dump_to_file(pstMemLog, pstBill);
		tlog_bill_fini(&pstBill);
		return 0;
	}
	return -1;
}

int tmemlog_fini(LPMEMLOG pstMemLog,int isWrite)
{
	int iRet;
	if(isWrite)
	{
		LPTLOGBILL pstBill;
		iRet = tlog_bill_init(&pstBill,10240000, 36000, pstMemLog->szOutFileName);
		if(!iRet)
		{
			tmemlog_dump_to_file(pstMemLog, pstBill);
			tlog_bill_fini(&pstBill);
		}
	}

	tmempool_destroy(&pstMemLog->pstPool);
	free(pstMemLog);
	return 0;
}

#endif

