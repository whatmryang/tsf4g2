
#include "tlog_bill_i.h"
#include "tlogfile.h"
#include "tlog_category_def.h"

int tlog_bill_init(LPTLOGBILL *ppstTlogBill,size_t iSizeLimit,int iPrecision,const char *szName)
{
	LPTLOGDEVFILE pstFileConf = NULL;
	LPTLOGFILE pstFile= NULL;
	int iRet;

	if(!ppstTlogBill)
	{
		return -1;
	}

	if(iSizeLimit > TLOGFILE_MAX_SIZE)
	{
		return -1;
	}

	if(!szName)
	{
		return -1;
	}


	pstFileConf = calloc(1,sizeof(TLOGDEVFILE));

	if(!pstFileConf)
	{
		return -1;
	}

	pstFile = calloc(1,sizeof(TLOGFILE));

	if(!pstFile)
	{
		free(pstFileConf);
		return -1;
	}

	pstFileConf->iBuffSize = 0;
	pstFileConf->iMaxRotate = 0;
	pstFileConf->iNoFindLatest = 0;
	pstFileConf->iPrecision = iPrecision;
	pstFileConf->dwSizeLimit = iSizeLimit;

	memset(pstFileConf->szPattern,0,sizeof(pstFileConf->szPattern));
	STRNCPY(pstFileConf->szPattern,szName,sizeof(pstFileConf->szPattern));
	pstFileConf->szPattern[sizeof(pstFileConf->szPattern)-1] = '\0';

	iRet = tlogfile_init(pstFile,pstFileConf);
	if(iRet)
	{
		free(pstFile);
		free(pstFileConf);
		return -1;
	}

	*ppstTlogBill = pstFile;
	return 0;
}



int tlog_bill_log(LPTLOGBILL pstTlogBill, const char* a_pszFmt, ...)
{
	va_list ap;
    	char szBuff[TLOG_DEFAULT_MAX_MSG_SIZE];
	intptr_t iBuff;
	intptr_t iMsgLen;
	struct timeval stCurrentTime;
	struct tm stTm;
    	int iRet;
	time_t tTimeTemp;

	if(!pstTlogBill)
	{
		return -1;
	}

	if( NULL == pstTlogBill)
	{
		return -1;
	}

	if(NULL == a_pszFmt)
	{
		return -1;
	}

	memset(szBuff,0,sizeof(szBuff));
	iBuff = 0;
	gettimeofday(&stCurrentTime, NULL);

	tTimeTemp = stCurrentTime.tv_sec;
	localtime_r(&tTimeTemp, &stTm);
	iMsgLen = strftime(szBuff,sizeof(szBuff),"%Y%m%d-%H:%M:%S | ",&stTm);

	iBuff = iMsgLen;
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
	else
	{
		szBuff[sizeof(szBuff)-1] = '\n';
	}

	iRet = tlogfile_write(pstTlogBill,szBuff,iBuff);
	if(iRet)
	{
		return -1;
	}

    va_end(ap);
    return 0;
}

int tlog_bill_write(LPTLOGBILL pstTlogBill,char *pszBuff,size_t iBuff)
{
	int iRet;

	/*add by vinsonzuo 20110701*/
	if (!pstTlogBill || !pszBuff || iBuff < 0)
	{
		return -1;
	}

	iRet = tlogfile_write(pstTlogBill,pszBuff,iBuff);
	return iRet;
}

int tlog_bill_fini(LPTLOGBILL *ppstTlogBill)
{
	void *pstConf;
	if(!ppstTlogBill)
	{
		return -1;
	}

	if(!*ppstTlogBill)
	{
		return -1;
	}

	pstConf = (*ppstTlogBill)->pstDev;
	tlogfile_fini(*ppstTlogBill);

	free(*ppstTlogBill);
	free(pstConf);

	*ppstTlogBill = NULL;
	return 0;
}


