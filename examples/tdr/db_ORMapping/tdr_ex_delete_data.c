/**
*
* @file     tdr_ex_delete_data.c  
* @brief    删除数据
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

int delete_data(IN TDRDBHANDLE a_hDBHandle, IN LPTDRMETALIB a_pstLib, LPTESTEXTENDTABLE a_pstData)
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
	iRet = tdr_obj2sql(a_hDBHandle, &stData, TDR_DBOP_DELETE, &stDBObj, NULL);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("failed to obj2sql: %s", tdr_error_string(iRet));
	}

	/*执行*/
	iRet = tdr_query_quick(&hDBResult, a_hDBHandle, &stData);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		const char *dbms_error = tdr_dbms_error(a_hDBHandle);
		printf("failed to tdr_query: %s,dbms error: %s", tdr_error_string(iRet),
			dbms_error);
	}else
	{
		printf("delete affected rows %ld\n", tdr_affected_rows(hDBResult));
	}

	return iRet;
}
