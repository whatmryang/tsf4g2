/**
*
* @file     tdr_ex_updata_data.c
* @brief    更新数据
*
*
* @author jackyai
* @version 1.0
* @date 2007-07-31
*
*
* Copyright (c)  2007, 腾讯科技有限公司互动娱乐研发部
* All rights reserved.
*
*/
#include <assert.h>
#include "tdr_dbORMapping.h"

#if defined(_WIN32) || defined(_WIN64)
#pragma warning(disable:4996)
#endif

static int update_all_entries(IN TDRDBHANDLE a_hDBHandle, IN LPTDRMETALIB a_pstLib, LPTESTEXTENDTABLE a_pstData);

static int update_part_entries(IN TDRDBHANDLE a_hDBHandle, IN LPTDRMETALIB a_pstLib, LPTESTEXTENDTABLE a_pstData);



int update_data(IN TDRDBHANDLE a_hDBHandle, IN LPTDRMETALIB a_pstLib, LPTESTEXTENDTABLE a_pstData)
{
	int iRet ;
	iRet = update_all_entries(a_hDBHandle, a_pstLib, a_pstData);

	iRet = update_part_entries(a_hDBHandle, a_pstLib, a_pstData);

	return iRet;
}



int update_all_entries(IN TDRDBHANDLE a_hDBHandle, IN LPTDRMETALIB a_pstLib, LPTESTEXTENDTABLE a_pstData)
{
	int iRet ;
	LPTDRMETA pstMeta;
	TDRDATA stData;
	char szSql[TDR_EX_MAN_SQL_LEN];
	TDRDBOBJECT stDBObj;
	TDRDBRESULTHANDLE hDBResult;

	assert(NULL != a_hDBHandle);
	assert(NULL != a_pstLib);
	assert(NULL != a_pstData);


	/*更新数据*/
	a_pstData->stGame.iGamepoint = rand();
	a_pstData->stItem.llMoney = (tdr_longlong)(rand()*rand());
	a_pstData->stUserAttachInfo.iLevel = rand();

	stData.iBuff = sizeof(szSql);
	stData.pszBuff = &szSql[0];

	pstMeta = tdr_get_meta_by_name(a_pstLib, "TestExtendTable");
	if (NULL == pstMeta)
	{
		printf("failed to get meta by name %s", "TestExtendTable");
		return -1;
	}

	stDBObj.iObjSize = sizeof(TESTEXTENDTABLE);
	stDBObj.iVersion = tdr_get_meta_current_version(pstMeta);
	stDBObj.pstMeta = pstMeta;
	stDBObj.pszObj = (char *)a_pstData;
	iRet = tdr_obj2sql(a_hDBHandle, &stData, TDR_DBOP_UPDATE, &stDBObj, NULL);

	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("failed to obj2sql: %s", tdr_error_string(iRet));
	}

	/*执行*/
	iRet = tdr_query(&hDBResult, a_hDBHandle, &stData);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		const char *dbms_error = tdr_dbms_error(a_hDBHandle);
		printf("failed to tdr_query: %s,dbms error: %s", tdr_error_string(iRet),
			dbms_error);
	}else
	{
		printf("update affected rows %ld\n", tdr_affected_rows(hDBResult));
	}

	return iRet;
}


int update_part_entries(IN TDRDBHANDLE a_hDBHandle, IN LPTDRMETALIB a_pstLib, LPTESTEXTENDTABLE a_pstData)
{
	int iRet ;
	LPTDRMETA pstMeta;
	TDRDATA stData;
	char szBuf[32];
	char szSql[TDR_EX_MAN_SQL_LEN];
	USERLONGININFO stLoginInfo;
	TDRDBOBJECT stDBObj;
	TDRDBRESULTHANDLE hDBResult;

	assert(NULL != a_hDBHandle);
	assert(NULL != a_pstLib);
	assert(NULL != a_pstData);


	/*更新数据*/
	stLoginInfo.stPri.dwUin =  a_pstData->stItem.dwUin;
	stLoginInfo.stPri.iSeq = a_pstData->iSeq;
	stLoginInfo.stPri.dwGameid = a_pstData->stItem.stItems.dwGameid;
	stLoginInfo.stPri.dwItemid = a_pstData->stItem.stItems.dwItemid;
	stLoginInfo.iLongnum = rand();
	stLoginInfo.iOffLineNum = rand();
	sprintf(szBuf, "%4d-%2d-%2d %2d:%2d:%2d", rand()%10000, rand()%13, rand()%28,rand()%24,rand()%60,rand()%60);
	tdr_str_to_tdrdatetime(&stLoginInfo.tLastlogintime, szBuf);


	pstMeta = tdr_get_meta_by_name(a_pstLib, "UserlonginInfo");
	if (NULL == pstMeta)
	{
		printf("failed to get meta by name %s", "UserlonginInfo");
		return -1;
	}

	stData.iBuff = sizeof(szSql);
	stData.pszBuff = &szSql[0];

	stDBObj.iObjSize = sizeof(USERLONGININFO);
	stDBObj.iVersion = tdr_get_meta_current_version(pstMeta);
	stDBObj.pstMeta = pstMeta;
	stDBObj.pszObj = (char *)&stLoginInfo;
	iRet = tdr_obj2sql(a_hDBHandle, &stData, TDR_DBOP_UPDATE, &stDBObj, NULL);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("failed to obj2sql: %s", tdr_error_string(iRet));
	}

	/*执行*/
	iRet = tdr_query(&hDBResult, a_hDBHandle, &stData);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		const char *dbms_error = tdr_dbms_error(a_hDBHandle);
		printf("failed to tdr_query: %s,dbms error: %s", tdr_error_string(iRet),
			dbms_error);
	}else
	{
		printf("update affected rows %ld\n", tdr_affected_rows(hDBResult));
	}

	return iRet;
}
