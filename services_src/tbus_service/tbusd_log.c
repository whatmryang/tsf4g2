#include "tlog/tlog.h"
#include "../../lib_src/tlog/tlog_i.h"
#include <assert.h>
#include "tbusd_log.h"
#include "tloghelp/tlogload.h"
#include "tbus/tbus.h"
#include "../../lib_src/tloghelp/tlogload_i.h"


TLOGCTX	g_stLogCtx;
TLOGCONF	g_stLogConf;
LPTLOGCATEGORYINST g_ptRelayLog = NULL ;
LPTLOGCATEGORYINST g_ptStatLog = NULL ;

void tbusd_log_bin(const LPTLOGCATEGORYINST a_category, int a_priority,
						   void *a_pData, int a_iDataLen)
{
#define RELAY_LOG_BIN_MAX_DATA_LEN	10240
#define RELAY_LOG_BIN_SIZE_PREBYTE	3
	char szBuff[RELAY_LOG_BIN_MAX_DATA_LEN];
	char *pch;
	int i;
	char *pchData;
	char chTmp;


	assert(NULL != a_pData);

	if (NULL == a_category)
	{
		return ;
	}
	if (!(tlog_category_is_priority_enabled(a_category, a_priority))) 
	{
		return;
	}	

	pch = &szBuff[0];
	pchData = (char *)a_pData;
	if (a_iDataLen*RELAY_LOG_BIN_SIZE_PREBYTE >= (int )sizeof(szBuff))
		a_iDataLen = (int)sizeof(szBuff)/RELAY_LOG_BIN_SIZE_PREBYTE -1;
	for(i = 0; i < a_iDataLen; i++)
	{
		chTmp = (*pchData & 0xF0)>>4;
		if ((0 <= chTmp) && (chTmp < 9) )
		{
			*pch = '0' + chTmp;
		}else
		{
			*pch = 'A' + chTmp - 10;
		}
		pch++;
		chTmp = (*pchData & 0x0F);
		if ((0 <= chTmp) && (chTmp < 9) )
		{
			*pch = '0' + chTmp;
		}else
		{
			*pch = 'A' + chTmp - 10;
		}
		pch++ ;
		*pch++ = ' ';
		if ((0 < i ) && (i % 32 == 0))
			*pch++ = '\n';
		pchData++;
	}
	*pch = '\0';

	tbusd_log_msg(a_category, a_priority, "%s\n", szBuff);
}

void tbusd_log_data(const LPTLOGCATEGORYINST a_category, int a_priority,
					 LPTDRMETA a_pstMeta, void *a_pData, int a_iDataLen)
{
#define RELAY_LOG_MAX_DATA_LEN	10240
	char szBuff[RELAY_LOG_MAX_DATA_LEN];
	TDRDATA stHost;
	TDRDATA stOut;

	assert(NULL != a_pData);
	assert(NULL != a_pstMeta);
	if (NULL == a_category)
	{
		return ;
	}


	if (!(tlog_category_is_priority_enabled(a_category, a_priority))) 
	{
		return;
	}

	stOut.iBuff = sizeof(szBuff);
	stOut.pszBuff = &szBuff[0];
	stHost.iBuff = a_iDataLen;
	stHost.pszBuff = a_pData;
	tdr_sprintf(a_pstMeta, &stOut, &stHost, 0);
	if (0 < stOut.iBuff)
	{
		if (stOut.iBuff < (int)sizeof(szBuff))
		{
			szBuff[stOut.iBuff] = '\0';
		}else
		{
			szBuff[sizeof(szBuff) -1] = '\0';
		}		
		tbusd_log_msg(a_category, a_priority, "%s\n", szBuff);
	}/*if (0 < stOut.iBuff)*/	
	
}



/**根据日志配置问题初始化日志系统
*/
int tbusd_log_init(IN TAPPCTX *a_pstAppCtx,  IN LPRELAYENV a_pstRunEnv)
{
	int iRet = 0;
	LPTBUSD pstConf;
    LPTLOGCATEGORYINST ptRelayLog = NULL ;

	assert(NULL != a_pstAppCtx);
	assert(NULL != a_pstRunEnv);
	assert(NULL != a_pstRunEnv->pstConf);

	pstConf = a_pstRunEnv->pstConf ;
	iRet = tapp_get_category (a_pstRunEnv->pstConf->szSvrLogName, &g_ptRelayLog) ;
	if (0 != iRet)
	{
		printf("Warnning!!!failed to get logcategory by name %s\n", a_pstRunEnv->pstConf->szSvrLogName);
		g_ptRelayLog = NULL;
	}

	
	iRet = tapp_get_category (a_pstRunEnv->pstConf->szStatLogName, &g_ptStatLog) ;
	if (0 != iRet)
	{
		printf("Warnning!!!failed to get logcategory by name %s\n", a_pstRunEnv->pstConf->szStatLogName);
		g_ptStatLog = NULL;
	}

	/*将tbus的日志也写入tbusd的日志类*/
    iRet = tapp_get_category (TLOG_DEF_CATEGORY_BUS, &ptRelayLog) ;
    if (0 != iRet)
    {
        printf("Warnning!!!failed to get logcategory by name %s for bus\n", TLOG_DEF_CATEGORY_BUS);
        g_ptStatLog = NULL;
    }    
	tbus_set_logcat(ptRelayLog);
	
	

	return 0;

}

/** 释放日志系统
*/
int tbusd_log_fini(void)
{
	
	return 0;
}
