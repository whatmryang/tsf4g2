/**
*
* @file     tdr_ex_select_data_quick.c 
* @brief    ���ټ�������
*
* 
* @author jackyai  
* @version 1.0
* @date 2007-07-31 
*
*
* Copyright (c)  2007, ��Ѷ�Ƽ����޹�˾���������з���
* All rights reserved.
*
*/
#include <assert.h>
#include "tdr_dbORMapping.h"

static int parse_result(TDRDBHANDLE a_hDBHandle, TDRDBRESULTHANDLE a_hDBResult, LPTDRMETA a_pstMeta, int a_iVersion);


int select_data_quickly(IN TDRDBHANDLE a_hDBHandle, IN LPTDRMETALIB a_pstLib, LPTESTEXTENDTABLE a_pstData)
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
	iRet = tdr_obj2sql(a_hDBHandle, &stData, TDR_DBOP_SELECT, &stDBObj, NULL);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("failed to obj2sql: %s", tdr_error_string(iRet));
	}

	/*ִ��*/
	iRet = tdr_query_quick(&hDBResult, a_hDBHandle, &stData);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		const char *dbms_error = tdr_dbms_error(a_hDBHandle);
		printf("failed to tdr_query: %s,dbms error: %s", tdr_error_string(iRet),
			dbms_error);
		return iRet;
	}

	/*�������*/
	printf("insert affected rows %ld\n", tdr_affected_rows(hDBResult));
	parse_result(a_hDBHandle, hDBResult, pstMeta, stDBObj.iVersion);


	/*�ͷŽ�����*/
	tdr_free_dbresult(&hDBResult);

	return iRet;
}

/**ʹ��tdr_query_quick����������������м�¼�Ѿ������꣬����֪����������ж��ٸ���¼ 
*/
int parse_result(TDRDBHANDLE a_hDBHandle, TDRDBRESULTHANDLE a_hDBResult, LPTDRMETA a_pstMeta, int a_iVersion)
{
#define  MAX_RECORDS_NUM	10  /*�����ܵļ�¼��*/

	TESTEXTENDTABLE stData[MAX_RECORDS_NUM];
	int i;
	TDRDBOBJECT stSelectDBObj;
	int iRet = TDR_SUCCESS;

	
	for (i = 0; i < MAX_RECORDS_NUM; i++)
	{
		stSelectDBObj.iObjSize = sizeof(TESTEXTENDTABLE);
		stSelectDBObj.pszObj = (char *)&stData[i];
		stSelectDBObj.pstMeta = a_pstMeta;
		stSelectDBObj.iVersion = a_iVersion;

		iRet = tdr_fetch_row(a_hDBResult, &stSelectDBObj);
		if (TDR_ERR_IS_ERROR(iRet))
		{
			const char *dbms_error = tdr_dbms_error(a_hDBHandle);
			printf("failed to tdr_fetch_row: %s,dbms error: %s", tdr_error_string(iRet),
				dbms_error);
			break;
		}
	}/*for (i = 0; i < iRecordNum; i++)*/

	return iRet;
}
