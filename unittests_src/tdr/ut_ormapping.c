/**
*
* @file     ut_test_tdr_db.c
* @brief    TDRDB对象－关系映射
*
* @author jackyai
* @version 1.0
* @date 2007-07-10
*
*
* Copyright (c)  2007, 腾讯科技有限公司互动娱乐研发部
* All rights reserved.
*
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <CUnit/CUnit.h>
#include <CUnit/TestDB.h>
#include <assert.h>
#include <time.h>

#include "pal/pal.h"
#include "tdr/tdr.h"
#include "ut_tdr.h"

#include "ut_tdr_db.h"
#include "../../lib_src/tdr/tdr_os.h"
#include "tdr_dbms.h"
#include "tdr/tdr_define_i.h"

static LPTDRMETALIB g_pstLib = NULL;
static LPTDRMETALIB g_pDBLib = NULL;
static TDRDBHANDLE g_hDBHandle;
static int ut_suite_init();
static int ut_suite_clean();
static void ut_insert_all_type_data();
static void ut_insert_dependonstable();
static void assert_equal_all_type_data(LPALLBASETYPEDATA pstSelectData, LPALLBASETYPEDATA pstAllData);
static void init_extend_table_data(LPTESTEXTENDTABLE pstAllData);
static void ut_extend_table();
static void ut_alter_meta();
static void ut_extend_array_table();
static void init_extend_array_table_data(LPTRANSNODEDESC pstData);
static void ut_auto_increment();

char g_szSql[100000];

int ut_add_ormapping_test_suites()
{
	CU_pSuite pSuite = NULL;


	pSuite = CU_add_suite("ut_add_ormapping_test_suites",
		(CU_InitializeFunc)ut_suite_init, (CU_CleanupFunc)ut_suite_clean);
	if(NULL == pSuite)
	{//检测注册suite情况　
		CU_cleanup_registry();
		return CU_get_error();
	}

	if( NULL == CU_add_test(pSuite, "ut_insert_all_type_data", (CU_TestFunc)ut_insert_all_type_data) )
	{
		CU_cleanup_registry();
		return CU_get_error();
	}

	if( NULL == CU_add_test(pSuite, "ut_insert_dependonstable", (CU_TestFunc)ut_insert_dependonstable) )
	{
		CU_cleanup_registry();
		return CU_get_error();
	}

	if( NULL == CU_add_test(pSuite, "ut_extend_table", (CU_TestFunc)ut_extend_table) )
	{
		CU_cleanup_registry();
		return CU_get_error();
	}

	/*if( NULL == CU_add_test(pSuite, "ut_alter_meta", (CU_TestFunc)ut_alter_meta) )
	{
		CU_cleanup_registry();
		return CU_get_error();
	}*/

	if( NULL == CU_add_test(pSuite, "ut_extend_array_table", (CU_TestFunc)ut_extend_array_table) )
	{
		CU_cleanup_registry();
		return CU_get_error();
	}

	if( NULL == CU_add_test(pSuite, "ut_auto_increment", (CU_TestFunc)ut_auto_increment) )
	{
		CU_cleanup_registry();
		return CU_get_error();
	}

	return 0;
}

int ut_suite_init()
{
	int iRet ;
	TDRDBMS stDBMS;
	LPTDRMETA pstMeta;
	TDRDATA stData;
	char szError[1024];
    TDRHPPRULE stRule ;
    DBMSINFO stDBMSConf;




	/*获取DBMS的信息*/
	iRet = tdr_create_lib_file(&g_pstLib, TEST_FILE_PATH"tdr_dbms.xml", 1, stderr);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("faild create metalib lib by file<%s>: %s\n", TEST_FILE_PATH"tdr_dbms.xml",
			tdr_error_string(iRet));
		return iRet;
	}
    memset(&stRule, 0, sizeof(stRule));
    iRet = tdr_metalib_to_hpp(g_pstLib, "tdr_dbms.h", &stRule);
    if (TDR_ERR_IS_ERROR(iRet))
    {
        printf("faild convert metalib(%s) to hpp : %s\n", TEST_FILE_PATH"tdr_dbms.xml",
            tdr_error_string(iRet));
        return iRet;
    }


	iRet = tdr_create_lib_file(&g_pDBLib, TEST_FILE_PATH"tdr_database.xml", 1, stderr);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("faild create metalib lib by file<%s>: %s\n", TEST_FILE_PATH"tdr_database.xml",
			tdr_error_string(iRet));
		return iRet;
	}

	pstMeta = tdr_get_meta_by_name(g_pstLib, "DBMSInfo");
	if (NULL == pstMeta)
	{
		printf("failed to get meta by name: %s\n", "DBMSInfo");
		return -1;
	}

	memset(&stDBMS, 0, sizeof(stDBMS));
	stData.iBuff = sizeof(stDBMSConf);
	stData.pszBuff = (char *)&stDBMSConf;
	iRet = tdr_input_file(pstMeta, &stData, TEST_FILE_PATH"tdr_dbms_conf.xml",
		tdr_get_meta_current_version(pstMeta), TDR_XML_DATA_FORMAT_LIST_ENTRY_NAME);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("faild tdr_input_file file<%s>: %s\n", TEST_FILE_PATH"tdr_dbms_conf.xml",
			tdr_error_string(iRet));
		return iRet;
	}
    stDBMS.iMajorVersion = stDBMSConf.iMajorVersion;
    stDBMS.iMinVersion = stDBMSConf.iMinVersion;
    stDBMSConf.iReconnectOpt = stDBMSConf.iReconnectOpt;
    TDR_STRNCPY(stDBMS.szDBMSConnectionInfo, stDBMSConf.szDBMSConnectionInfo, sizeof(stDBMS.szDBMSConnectionInfo));
    TDR_STRNCPY(stDBMS.szDBMSCurDatabaseName,stDBMSConf.szDBMSCurDatabaseName, sizeof(stDBMS.szDBMSCurDatabaseName));
    TDR_STRNCPY(stDBMS.szDBMSName, stDBMSConf.szDBMSName, sizeof(stDBMS.szDBMSName));
    TDR_STRNCPY(stDBMS.szDBMSPassword,stDBMSConf.szDBMSPassword,sizeof(stDBMS.szDBMSPassword));
    TDR_STRNCPY(stDBMS.szDBMSSock, stDBMSConf.szDBMSSock, sizeof(stDBMS.szDBMSSock));
    TDR_STRNCPY(stDBMS.szDBMSUser, stDBMSConf.szDBMSUser, sizeof(stDBMS.szDBMSUser));

	/*打开db处理句柄*/
	iRet = tdr_open_dbhanlde(&g_hDBHandle, &stDBMS, &szError[0]);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("faild open dbhanlde: %s %s\n", tdr_error_string(iRet), szError);
		return iRet;
	}

	srand((unsigned int)time(NULL));

	return 0;
}

int ut_suite_clean()
{
	puts("ut_add_ormapping_test_suites clean, begin... ");
	tdr_free_lib(&g_pstLib);
	tdr_free_lib(&g_pDBLib);
	tdr_close_dbhanlde(&g_hDBHandle);
	puts("ut_add_ormapping_test_suites clean, ok ");
	return 0;
}

void ut_insert_all_type_data()
{
	int iRet ;
	LPTDRMETA pstMeta;
	ALLBASETYPEDATA stAllData;
	TDRDATA stData;
	char szSql[100000];
	TDRDBOBJECT stDBObj;
	int i,j;
	TDRDBRESULTHANDLE hDBResult;
	ALLBASETYPEDATA stSelectData;
	unsigned int dwCount;
	unsigned int dwCount1;
    FILE *fp;

	puts("ut_insert_all_type_data begin...");
	iRet =  tdr_get_records_count(g_hDBHandle, "AllBaseTypeData", NULL, &dwCount);
	fp = fopen("AllBaseTypeData_insert.sql", "w");
	if (fp == NULL)
	{
		printf("failed to open %s to write\n", "AllBaseTypeData_insert.sql");
		return ;
	}

	pstMeta = tdr_get_meta_by_name(g_pDBLib, "AllBaseTypeData");
	CU_ASSERT(NULL != pstMeta);

	/*构造数据*/
	stData.iBuff = sizeof(stAllData);
	stData.pszBuff = (char *)&stAllData;
	memset(&stAllData, 0, sizeof(stAllData));
	iRet = tdr_init(pstMeta, &stData, tdr_get_meta_current_version(pstMeta));
	CU_ASSERT(0 <= iRet);

	stAllData.dwSeq = (unsigned int) rand();
	stAllData.iIRefer = rand() % 4;
	for (i = 0; i < stAllData.iIRefer; i++)
	{
		stAllData.intDataArray[i] = rand();
	}
	stAllData.szCharDataArr[0] = '\0';
	for (i = 1; i < 255; i++)
	{
		stAllData.szCharDataArr[i] = rand() % 256;
	}
	for (j = 31; j >= 0; j--)
	{
		stAllData.aszStrings[0][j] = (char)j;
	}
	stAllData.aszStrings[0][0] = '\0';

	iRet = tdr_output_file(pstMeta, "AllBaseTypeData_insert.xml", &stData, tdr_get_meta_current_version(pstMeta), 2);
	CU_ASSERT(0 <= iRet);

	/*生成insert语句*/
	stData.iBuff = sizeof(szSql);
	stData.pszBuff = &szSql[0];
	stDBObj.iObjSize = sizeof(stAllData);
	stDBObj.pszObj = (char *)&stAllData;
	stDBObj.iVersion = tdr_get_meta_current_version(pstMeta);
	stDBObj.pstMeta = pstMeta;

	iRet = tdr_obj2sql(g_hDBHandle, &stData, TDR_DBOP_INSERT, &stDBObj, NULL);
	CU_ASSERT(0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("failed to obj2sql: %s", tdr_error_string(iRet));
	}else
	{
		fwrite(stData.pszBuff, 1, stData.iBuff, fp);
	}

	/*执行查询*/
	iRet = tdr_query(&hDBResult, g_hDBHandle, &stData);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		const char *dbms_error = tdr_dbms_error(g_hDBHandle);
		printf("failed to tdr_query: %s,dbms error: %s", tdr_error_string(iRet),
			dbms_error);
	}

	/*指定查询条件的 select count(*)*/
	tdr_snprintf(szSql, sizeof(szSql), "Where seq=%u", stAllData.dwSeq);
	iRet =  tdr_get_records_count(g_hDBHandle, "AllBaseTypeData", &szSql[0], &dwCount1);
	CU_ASSERT(0 <= iRet);
	CU_ASSERT(1 == dwCount1);

	tdr_keep_dbmsconnection(g_hDBHandle);
	/*update*/
	stData.iBuff = sizeof(szSql);
	stAllData.iIntData = rand();
	iRet = tdr_obj2sql(g_hDBHandle, &stData, TDR_DBOP_UPDATE, &stDBObj, NULL);
	CU_ASSERT(0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("failed to obj2sql: %s", tdr_error_string(iRet));
	}else
	{
		fwrite(stData.pszBuff, 1, stData.iBuff, fp);
		fputs("\n",fp);
	}
	iRet = tdr_query(&hDBResult, g_hDBHandle, &stData);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		const char *dbms_error = tdr_dbms_error(g_hDBHandle);
		printf("failed to tdr_query: %s,dbms error: %s", tdr_error_string(iRet),
			dbms_error);
	}


	/*select*/
	stData.iBuff = sizeof(szSql);
	iRet = tdr_obj2sql(g_hDBHandle, &stData, TDR_DBOP_SELECT, &stDBObj, NULL);
	CU_ASSERT(0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("failed to obj2sql: %s", tdr_error_string(iRet));
	}else
	{
		fwrite(stData.pszBuff, 1, stData.iBuff, fp);
		fputs("\n",fp);
	}
	iRet = tdr_query(&hDBResult, g_hDBHandle, &stData);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		const char *dbms_error = tdr_dbms_error(g_hDBHandle);
		printf("failed to tdr_query: %s,dbms error: %s", tdr_error_string(iRet),
			dbms_error);
	}else
	{
		TDRDATA stDBData;

		stDBObj.iObjSize = sizeof(stSelectData);
		stDBObj.pszObj = (char *)&stSelectData;
		memset(&stSelectData, 0, sizeof(stSelectData));
		iRet = tdr_fetch_row(hDBResult, &stDBObj);
		CU_ASSERT(0 <= iRet);
		if (TDR_ERR_IS_ERROR(iRet))
		{
			printf("failed to tdr_fetch_row: %s", tdr_error_string(iRet));
		}
		assert_equal_all_type_data(&stSelectData, &stAllData);
		stDBData.iBuff = sizeof(stSelectData);
		stDBData.pszBuff = (char *)&stSelectData;
		tdr_output_file(stDBObj.pstMeta, "select_all_type.xml", &stDBData, 0, 2);
	}

	tdr_keep_dbmsconnection(g_hDBHandle);


	iRet =  tdr_get_records_count(g_hDBHandle, "AllBaseTypeData", "", &dwCount1);
	CU_ASSERT(0 <= iRet);
	CU_ASSERT(0 < dwCount1);
	CU_ASSERT((dwCount1 - dwCount) == 1);
	printf("Record num in table AllBaseTypeData is %d\n", dwCount1);

	/*delete*/
	stData.iBuff = sizeof(szSql);
	iRet = tdr_obj2sql(g_hDBHandle, &stData, TDR_DBOP_DELETE, &stDBObj, NULL);
	CU_ASSERT(0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("failed to obj2sql: %s", tdr_error_string(iRet));
	}else
	{
		fwrite(stData.pszBuff, 1, stData.iBuff, fp);
		fputs("\n",fp);
	}
	iRet = tdr_query(&hDBResult, g_hDBHandle, &stData);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		const char *dbms_error = tdr_dbms_error(g_hDBHandle);
		printf("failed to tdr_query: %s,dbms error: %s", tdr_error_string(iRet),
			dbms_error);
	}

	fclose(fp);

	puts("ut_insert_all_type_data ok");

}

void ut_insert_dependonstable()
{
	int iRet ;
	LPTDRMETA pstMeta;
	USERLONGININFO stAllData;
	TDRDATA stData;
	char szSql[100000];
	TDRDBOBJECT stDBObj;
	TDRDBRESULTHANDLE hDBResult;
	char szBuf[32];
	FILE *fp;
	USERLONGININFO stSelectData;
	TDRDBOBJECT stSelectDBObj;
	USERITEMINFO stUserInfo;

	puts("ut_insert_dependonstable begin...");
	fp = fopen("ut_insert_dependonstable.sql", "w");
	if (NULL == fp)
	{
		puts("failed to open ut_insert_dependonstable.sql to write");
		return ;
	}


	puts("ut_insert_dependonstable");
	pstMeta = tdr_get_meta_by_name(g_pDBLib, "UserlonginInfo");
	CU_ASSERT(NULL != pstMeta);

	/*构造数据*/
	stData.iBuff = sizeof(stAllData);
	stData.pszBuff = (char *)&stAllData;


	stAllData.stPrimaryInfo.dwUin =(unsigned int) rand();
	stAllData.iLongnum = rand();
	stAllData.iOffLineNum = rand();
	sprintf(szBuf, "%4d-%2d-%2d %2d:%2d:%2d", rand()%10000, rand()%13, rand()%28,rand()%24,rand()%60,rand()%60);
	tdr_str_to_tdrdatetime(&stAllData.tLastlogintime, szBuf);
	iRet = tdr_output_file(pstMeta, "UserlonginInfo_insert.xml", &stData, tdr_get_meta_current_version(pstMeta), 2);
	CU_ASSERT(0 <= iRet);

	/*生成insert语句*/
	stData.iBuff = sizeof(szSql);
	stData.pszBuff = &szSql[0];
	stDBObj.iObjSize = sizeof(stAllData);
	stDBObj.iVersion = tdr_get_meta_current_version(pstMeta);
	stDBObj.pstMeta = pstMeta;
	stDBObj.pszObj = (char *)&stAllData;
	iRet = tdr_obj2sql(g_hDBHandle, &stData, TDR_DBOP_INSERT, &stDBObj, NULL);
	CU_ASSERT(0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("failed to obj2sql: %s", tdr_error_string(iRet));
	}else
	{
		fwrite(stData.pszBuff, 1, stData.iBuff, fp);
		fputs("\n",fp);
	}

	/*执行insert查询*/
	iRet = tdr_query(&hDBResult, g_hDBHandle, &stData);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		const char *dbms_error = tdr_dbms_error(g_hDBHandle);
		printf("failed to tdr_query: %s,dbms error: %s", tdr_error_string(iRet),
			dbms_error);
	}else
	{
		printf("insert affected rows %ld\n", tdr_affected_rows(hDBResult));
	}


	/*update*/
	stData.iBuff = sizeof(szSql);
	stAllData.iLongnum = rand();
	iRet = tdr_obj2sql(g_hDBHandle, &stData, TDR_DBOP_UPDATE, &stDBObj, NULL);
	CU_ASSERT(0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("failed to obj2sql: %s", tdr_error_string(iRet));
	}else
	{
		fwrite(stData.pszBuff, 1, stData.iBuff, fp);
		fputs("\n",fp);
	}
	iRet = tdr_query(&hDBResult, g_hDBHandle, &stData);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		const char *dbms_error = tdr_dbms_error(g_hDBHandle);
		printf("failed to tdr_query: %s,dbms error: %s", tdr_error_string(iRet),
			dbms_error);
	}else
	{
		printf("update affected rows %ld\n", tdr_affected_rows(hDBResult));
	}


	/*select*/
	stData.iBuff = sizeof(szSql);
	iRet = tdr_obj2sql(g_hDBHandle, &stData, TDR_DBOP_SELECT, &stDBObj, NULL);
	CU_ASSERT(0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("failed to obj2sql: %s", tdr_error_string(iRet));
	}else
	{
		fwrite(stData.pszBuff, 1, stData.iBuff, fp);
		fputs("\n",fp);
	}
	iRet = tdr_query(&hDBResult, g_hDBHandle, &stData);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		const char *dbms_error = tdr_dbms_error(g_hDBHandle);
		printf("failed to tdr_query: %s,dbms error: %s", tdr_error_string(iRet),
			dbms_error);
	}else
	{
		printf("select affected rows %ld, get rows : %lu\n", tdr_affected_rows(hDBResult),
			tdr_num_rows(hDBResult));

        memcpy(&stSelectData, &stAllData, sizeof(stAllData));
		stSelectDBObj.iObjSize = sizeof(stSelectData);
		stSelectDBObj.pszObj = (char *)&stSelectData;
		stSelectDBObj.pstMeta = pstMeta;
		stSelectDBObj.iVersion = stDBObj.iVersion;

		iRet = tdr_fetch_row(hDBResult, &stSelectDBObj);
		CU_ASSERT(0 <= iRet);
		if (TDR_ERR_IS_ERROR(iRet))
		{
			printf("failed to obj2sql: %s", tdr_error_string(iRet));
		}
		CU_ASSERT(0 == memcmp(&stAllData, &stSelectData, sizeof(stAllData)));

		stSelectDBObj.iObjSize = sizeof(stSelectData);
		iRet = tdr_fetch_row(hDBResult, &stSelectDBObj);
		CU_ASSERT(TDR_ERROR_DB_NO_RECORD_IN_RESULTSET == TDR_ERR_GET_ERROR_CODE(iRet));
	}


	/*select user info*/
	stData.iBuff = sizeof(szSql);
	stSelectDBObj.iObjSize = sizeof(stUserInfo);
	stSelectDBObj.pszObj = (char *)&stUserInfo;
	stSelectDBObj.pstMeta = tdr_get_meta_by_name(g_pDBLib, "UserItemInfo");
	stSelectDBObj.iVersion = tdr_get_meta_current_version(stSelectDBObj.pstMeta);
	stUserInfo.dwUin = stAllData.stPrimaryInfo.dwUin;
	iRet = tdr_obj2sql(g_hDBHandle, &stData, TDR_DBOP_SELECT, &stSelectDBObj, NULL);
	CU_ASSERT(0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("failed to obj2sql: %s", tdr_error_string(iRet));
	}else
	{
		fwrite(stData.pszBuff, 1, stData.iBuff, fp);
		fputs("\n",fp);
	}
	iRet = tdr_query(&hDBResult, g_hDBHandle, &stData);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		const char *dbms_error = tdr_dbms_error(g_hDBHandle);
		printf("failed to tdr_query: %s,dbms error: %s", tdr_error_string(iRet),
			dbms_error);
	}else
	{
		printf("select affected rows %ld, get rows : %lu\n", tdr_affected_rows(hDBResult),
			tdr_num_rows(hDBResult));

		stSelectDBObj.iObjSize = sizeof(stUserInfo);
		iRet = tdr_fetch_row(hDBResult, &stSelectDBObj);
		CU_ASSERT(0 <= iRet);
		if (TDR_ERR_IS_ERROR(iRet))
		{
			printf("failed to obj2sql: %s", tdr_error_string(iRet));
		}
	}


	/*delete*/
	stData.iBuff = sizeof(szSql);
	iRet = tdr_obj2sql(g_hDBHandle, &stData, TDR_DBOP_DELETE, &stDBObj, NULL);
	CU_ASSERT(0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("failed to obj2sql: %s", tdr_error_string(iRet));
	}else
	{
		fwrite(stData.pszBuff, 1, stData.iBuff, fp);
		fputs("\n",fp);
	}
	iRet = tdr_query(&hDBResult, g_hDBHandle, &stData);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		const char *dbms_error = tdr_dbms_error(g_hDBHandle);
		printf("failed to tdr_query: %s,dbms error: %s", tdr_error_string(iRet),
			dbms_error);
	}else
	{
		printf("delete affected rows %ld\n", tdr_affected_rows(hDBResult));
	}

	fclose(fp);

	puts("ut_insert_dependonstable ok");
}

void assert_equal_all_type_data(LPALLBASETYPEDATA pstSelectData, LPALLBASETYPEDATA pstAllData)
{
	int i;

	CU_ASSERT(pstAllData->dwSeq == pstSelectData->dwSeq);
	CU_ASSERT(pstAllData->chCharData== pstSelectData->chCharData);
	CU_ASSERT(0 == memcmp(&pstAllData->szCharDataArr[0], &pstSelectData->szCharDataArr[0],
		sizeof(pstSelectData->szCharDataArr)));
	CU_ASSERT(pstAllData->bUcharData== pstSelectData->bUcharData);
	CU_ASSERT(pstAllData->nShortData== pstSelectData->nShortData);
	CU_ASSERT(pstAllData->wUshortData== pstSelectData->wUshortData);
	CU_ASSERT(pstAllData->iIntData== pstSelectData->iIntData);
	CU_ASSERT(pstAllData->iIRefer == pstSelectData->iIRefer);
	for (i = 0; i < pstAllData->iIRefer; i++)
	{
		CU_ASSERT(pstAllData->intDataArray[i] == pstSelectData->intDataArray[i]);
	}

	CU_ASSERT(pstAllData->dwUintData== pstSelectData->dwUintData);
	CU_ASSERT(pstAllData->llBigintData== pstSelectData->llBigintData);
	CU_ASSERT(pstAllData->ullBiguintData == pstSelectData->ullBiguintData);

	CU_ASSERT((pstAllData->fFloatData - pstSelectData->fFloatData) < 0.000001);
	CU_ASSERT((pstAllData->dDoubleData - pstSelectData->dDoubleData) < 0.000001);
	CU_ASSERT(pstAllData->tDatatimeData == pstSelectData->tDatatimeData);
	CU_ASSERT(pstAllData->tDataData == pstSelectData->tDataData);
	CU_ASSERT(pstAllData->tTimeData == pstSelectData->tTimeData);
	CU_ASSERT(pstAllData->ulIpData == pstSelectData->ulIpData);
	CU_ASSERT(pstAllData->wWcharData == pstSelectData->wWcharData);
	CU_ASSERT(pstAllData->bByteData == pstSelectData->bByteData);
	CU_ASSERT(0 == strcmp(pstSelectData->szStringData, pstAllData->szStringData));
	for (i = 0; i < 2; i++)
	{
		CU_ASSERT(0 == strcmp(pstSelectData->aszStrings[i], pstAllData->aszStrings[i]));
	}
	CU_ASSERT(0 == memcmp(pstSelectData->szWstringData, pstAllData->szWstringData, sizeof(pstSelectData->szWstringData)));
}

void init_extend_table_data(LPTESTEXTENDTABLE pstAllData)
{
	unsigned int i;


	pstAllData->iSeq = rand();

	pstAllData->stGame.iGameid = rand();
	pstAllData->stGame.iGamepoint = rand();
	for (i = 0; i < sizeof(pstAllData->stGame.szReserve); i++)
	{
		pstAllData->stGame.szReserve[i] = i % 16;
	}

	pstAllData->stItem.dwUin = (unsigned int)rand();
	pstAllData->stItem.iLongnum = rand();
	pstAllData->stItem.iOffLineNum = rand();
	pstAllData->stItem.llMoney = rand() * rand();
	pstAllData->stItem.stItems.dwGameid = (unsigned int)rand();
	pstAllData->stItem.stItems.dwItemid = (unsigned int)rand();

	ut_get_rand_datetime(&pstAllData->stItem.stItems.tBuytime);

	ut_get_rand_datetime(&pstAllData->stItem.stItems.tValidtime);

	ut_get_rand_datetime(&pstAllData->stItem.tLastlogintime);

	pstAllData->stUserAttachInfo.iAge = rand();
	pstAllData->stUserAttachInfo.iLevel = rand();
	for (i = 0; i< 4; i++)
	{
		pstAllData->stUserAttachInfo.reserver[i] = rand();
	}
}

void ut_extend_table()
{
	int iRet ;
	LPTDRMETA pstMeta;
	TESTEXTENDTABLE stAllData;
	TDRDATA stData;

	TDRDBOBJECT stDBObj;
	TDRDBRESULTHANDLE hDBResult;
    TDRDATA stOut;
	FILE *fp;
	TESTEXTENDTABLE stSelectData;
	TDRDBOBJECT stSelectDBObj;


	puts("ut_extend_table begin...");
	fp = fopen("ut_extend_table.sql", "w");
	if (NULL == fp)
	{
		puts("failed to open ut_extend_table.sql to write");
		return ;
	}


	puts("ut_extend_table");
	pstMeta = tdr_get_meta_by_name(g_pDBLib, "TestExtendTable");
	CU_ASSERT(NULL != pstMeta);

	/*构造数据*/
	init_extend_table_data(&stAllData);

    stOut.iBuff = sizeof(stAllData);
    stOut.pszBuff = (char *)&stAllData;
    tdr_output_file(pstMeta, "stAllData.xml", &stOut, stDBObj.iVersion, 2);

	/*生成insert语句*/
	stData.iBuff = sizeof(g_szSql);
	stData.pszBuff = &g_szSql[0];
	stDBObj.iObjSize = sizeof(stAllData);
	stDBObj.iVersion = tdr_get_meta_current_version(pstMeta);
	stDBObj.pstMeta = pstMeta;
	stDBObj.pszObj = (char *)&stAllData;
	iRet = tdr_obj2sql(g_hDBHandle, &stData, TDR_DBOP_INSERT, &stDBObj, NULL);
	CU_ASSERT(0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("failed to obj2sql: %s", tdr_error_string(iRet));
	}else
	{
		fwrite(stData.pszBuff, 1, stData.iBuff, fp);
		fputs("\n",fp);
	}

	/*执行*/
	iRet = tdr_query(&hDBResult, g_hDBHandle, &stData);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		const char *dbms_error = tdr_dbms_error(g_hDBHandle);
		printf("failed to tdr_query: %s,dbms error: %s", tdr_error_string(iRet),
			dbms_error);
	}else
	{
		printf("insert affected rows %ld\n", tdr_affected_rows(hDBResult));
	}


	/*update*/
	stData.iBuff = sizeof(g_szSql);
	stAllData.stGame.iGamepoint = rand();
	stAllData.stItem.llMoney = (tdr_longlong)(rand()*rand());
	stAllData.stUserAttachInfo.iLevel = rand();
	stDBObj.iObjSize = sizeof(stAllData);
	iRet = tdr_obj2sql(g_hDBHandle, &stData, TDR_DBOP_UPDATE, &stDBObj, NULL);
	CU_ASSERT(0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("failed to obj2sql: %s", tdr_error_string(iRet));
	}else
	{
		fwrite(stData.pszBuff, 1, stData.iBuff, fp);
		fputs("\n",fp);
	}
	iRet = tdr_query_quick(&hDBResult, g_hDBHandle, &stData);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		const char *dbms_error = tdr_dbms_error(g_hDBHandle);
		printf("failed to tdr_query: %s,dbms error: %s", tdr_error_string(iRet),
			dbms_error);
	}else
	{
		printf("update affected rows %ld\n", tdr_affected_rows(hDBResult));
	}


	/*select*/
	stData.iBuff = sizeof(g_szSql);
	stDBObj.iObjSize = sizeof(stAllData);
	iRet = tdr_obj2sql(g_hDBHandle, &stData, TDR_DBOP_SELECT, &stDBObj, NULL);
	CU_ASSERT(0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("failed to obj2sql: %s", tdr_error_string(iRet));
	}else
	{
		fwrite(stData.pszBuff, 1, stData.iBuff, fp);
		fputs("\n",fp);
	}
	iRet = tdr_query_quick(&hDBResult, g_hDBHandle, &stData);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		const char *dbms_error = tdr_dbms_error(g_hDBHandle);
		printf("failed to tdr_query: %s,dbms error: %s", tdr_error_string(iRet),
			dbms_error);
	}else
	{
		printf("select affected rows %ld, get rows : %lu\n", tdr_affected_rows(hDBResult),
			tdr_num_rows(hDBResult));

		memcpy(&stSelectData, &stAllData, sizeof(stAllData));
		stSelectDBObj.iObjSize = sizeof(stSelectData);
		stSelectDBObj.pszObj = (char *)&stSelectData;
		stSelectDBObj.pstMeta = pstMeta;
		stSelectDBObj.iVersion = stDBObj.iVersion;

		iRet = tdr_fetch_row(hDBResult, &stSelectDBObj);
		CU_ASSERT(0 <= iRet);
		if (TDR_ERR_IS_ERROR(iRet))
		{
			printf("failed to obj2sql: %s", tdr_error_string(iRet));
		}
		CU_ASSERT(0 == memcmp(&stAllData, &stSelectData, sizeof(stAllData)));
		//if (0 != memcmp(&stAllData, &stSelectData, sizeof(stAllData)))
		{


			stOut.iBuff = sizeof(stAllData);
			stOut.pszBuff = (char *)&stAllData;
			tdr_output_file(pstMeta, "stAllData.xml", &stOut, stDBObj.iVersion, 2);
			stOut.pszBuff = (char *)&stSelectData;
			tdr_output_file(pstMeta, "stSelectData.xml", &stOut, stDBObj.iVersion, 2);
		}

		stSelectDBObj.iObjSize = sizeof(stSelectData);
		iRet = tdr_fetch_row(hDBResult, &stSelectDBObj);
		CU_ASSERT(TDR_ERROR_DB_NO_RECORD_IN_RESULTSET == TDR_ERR_GET_ERROR_CODE(iRet));
		printf("select affected rows %ld, get rows : %lu\n", tdr_affected_rows(hDBResult),
			tdr_num_rows(hDBResult));

		tdr_free_dbresult(&hDBResult);
	}




	/*delete*/
	stData.iBuff = sizeof(g_szSql);
	iRet = tdr_obj2sql(g_hDBHandle, &stData, TDR_DBOP_DELETE, &stDBObj, NULL);
	CU_ASSERT(0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("failed to obj2sql: %s", tdr_error_string(iRet));
	}else
	{
		fwrite(stData.pszBuff, 1, stData.iBuff, fp);
		fputs("\n",fp);
	}
	iRet = tdr_query(&hDBResult, g_hDBHandle, &stData);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		const char *dbms_error = tdr_dbms_error(g_hDBHandle);
		printf("failed to tdr_query: %s,dbms error: %s", tdr_error_string(iRet),
			dbms_error);
	}else
	{
		printf("delete affected rows %ld\n", tdr_affected_rows(hDBResult));
	}

	fclose(fp);

	puts("ut_extend_table ok");

    ut_alter_meta();
}

void ut_alter_meta()
{
	int iRet ;
	LPTDRMETA pstMeta;
	TESTEXTENDTABLE stAllData;
	TDRDATA stData;
	char szSql[100000];
	TDRDBOBJECT stDBObj;
	TDRDBRESULTHANDLE hDBResult;

	TESTEXTENDTABLE stSelectData;
	TDRDBOBJECT stSelectDBObj;



	puts("ut_alter_meta begin...");
	pstMeta = tdr_get_meta_by_name(g_pDBLib, "TestExtendTable");
	CU_ASSERT(NULL != pstMeta);


	stData.iBuff = sizeof(stAllData);
	stData.pszBuff =(char *) &stAllData;
	iRet = tdr_input_file(pstMeta, &stData,"stAllData.xml", 0, 2);
    if (0 != iRet)
    {
        printf("tdr_input_file(%s) failed:%s\n", "stAllData.xml",
            tdr_error_string(iRet));
    }
	assert(0 <= iRet);


	/*select*/
	stData.iBuff = sizeof(szSql);
	stData.pszBuff = &szSql[0];
	stDBObj.iObjSize = sizeof(stAllData);
	stDBObj.iVersion = tdr_get_meta_current_version(pstMeta);
	stDBObj.pstMeta = pstMeta;
	stDBObj.pszObj = (char *)&stAllData;
	iRet = tdr_obj2sql(g_hDBHandle, &stData, TDR_DBOP_SELECT, &stDBObj, NULL);
	CU_ASSERT(0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("failed to obj2sql: %s", tdr_error_string(iRet));
	}

	iRet = tdr_query(&hDBResult, g_hDBHandle, &stData);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		const char *dbms_error = tdr_dbms_error(g_hDBHandle);
		printf("failed to tdr_query: %s,dbms error: %s", tdr_error_string(iRet),
			dbms_error);
	}else
	{
		printf("select affected rows %ld, get rows : %lu\n", tdr_affected_rows(hDBResult),
			tdr_num_rows(hDBResult));

		stSelectDBObj.iObjSize = sizeof(stSelectData);
		stSelectDBObj.pszObj = (char *)&stSelectData;
		stSelectDBObj.pstMeta = pstMeta;
		stSelectDBObj.iVersion = stDBObj.iVersion;

		iRet = tdr_fetch_row(hDBResult, &stSelectDBObj);
		//CU_ASSERT(0 <= iRet);
		if (TDR_ERR_IS_ERROR(iRet))
		{
			const char *dbms_error = tdr_dbms_error(g_hDBHandle);
			printf("tdr_fetch_row failed, %s, for %s ", tdr_error_string(iRet), dbms_error);
		}else
		{
			CU_ASSERT(0 == memcmp(&stAllData, &stSelectData, sizeof(stAllData)));
			if (0 != memcmp(&stAllData, &stSelectData, sizeof(stAllData)))
			{
				TDRDATA stOut;

				stOut.iBuff = sizeof(stAllData);
				stOut.pszBuff = (char *)&stAllData;
				tdr_output_file(pstMeta, "stAllData.xml", &stOut, stDBObj.iVersion, 2);
				stOut.iBuff = sizeof(stSelectData);
				stOut.pszBuff = (char *)&stSelectData;
				tdr_output_file(pstMeta, "stSelectData.xml", &stOut, stDBObj.iVersion, 2);
			}
		}


		tdr_free_dbresult(&hDBResult);
	}

	puts("ut_alter_meta ok");
}

void ut_extend_array_table()
{
	int iRet ;
	LPTDRMETA pstMeta;
	TRANSNODEDESC stAllData;
	TDRDATA stData;

	TDRDBOBJECT stDBObj;
	TDRDBRESULTHANDLE hDBResult;

	FILE *fp;
	TRANSNODEDESC stSelectData;
	TDRDBOBJECT stSelectDBObj;

	puts("ut_extend_array_table begin...");
	fp = fopen("ut_extend_array_table.sql", "w");
	if (NULL == fp)
	{
		puts("failed to open ut_extend_array_table.sql to write");
		return ;
	}


	puts("\nut_extend_array_table");
	pstMeta = tdr_get_meta_by_name(g_pDBLib, "TransNodeDesc");
	CU_ASSERT(NULL != pstMeta);

	/*构造数据*/
	memset(&stAllData, 0, sizeof(stAllData));
	init_extend_array_table_data(&stAllData);


	/*生成insert语句*/
	stData.iBuff = sizeof(g_szSql);
	stData.pszBuff = &g_szSql[0];
	stDBObj.iObjSize = sizeof(stAllData);
	stDBObj.iVersion = 0;
	stDBObj.pstMeta = pstMeta;
	stDBObj.pszObj = (char *)&stAllData;
	iRet = tdr_obj2sql(g_hDBHandle, &stData, TDR_DBOP_INSERT, &stDBObj, NULL);
	CU_ASSERT(0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		const char *dbms_error = tdr_dbms_error(g_hDBHandle);
		printf("failed to obj2sql: %s, dbms error: %s\n", tdr_error_string(iRet), dbms_error);
	}else
	{
		fwrite(stData.pszBuff, 1, stData.iBuff, fp);
		fputs("\n",fp);
	}

	/*执行*/
	iRet = tdr_query(&hDBResult, g_hDBHandle, &stData);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		const char *dbms_error = tdr_dbms_error(g_hDBHandle);
		printf("failed to tdr_query: %s,dbms error: %s, sql:%s\n", tdr_error_string(iRet),
			dbms_error, stData.pszBuff);
	}else
	{
		printf("insert affected rows %ld\n", tdr_affected_rows(hDBResult));
	}


	/*update*/
	stData.iBuff = sizeof(g_szSql);
	stAllData.astTransNodeEntryDesc[0].iTransDstDescID += 1;
	stAllData.astTransNodeEntryDesc[0].stAttachInfo.astAttach[0].iMapid += 10;
	stDBObj.iObjSize = sizeof(stAllData);
	iRet = tdr_obj2sql(g_hDBHandle, &stData, TDR_DBOP_UPDATE, &stDBObj, NULL);
	CU_ASSERT(0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("failed to obj2sql: %s\n", tdr_error_string(iRet));
	}else
	{
		fwrite(stData.pszBuff, 1, stData.iBuff, fp);
		fputs("\n",fp);
	}
	iRet = tdr_query_quick(&hDBResult, g_hDBHandle, &stData);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		const char *dbms_error = tdr_dbms_error(g_hDBHandle);
		printf("failed to tdr_query: %s,dbms error: %s, sql:%s\n", tdr_error_string(iRet),
			dbms_error, stData.pszBuff);
	}else
	{
		printf("update affected rows %ld\n", tdr_affected_rows(hDBResult));
	}


	/*select*/
	stData.iBuff = sizeof(g_szSql);
	stDBObj.iObjSize = sizeof(stAllData);
	iRet = tdr_obj2sql(g_hDBHandle, &stData, TDR_DBOP_SELECT, &stDBObj, NULL);
	CU_ASSERT(0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("failed to obj2sql: %s", tdr_error_string(iRet));
	}else
	{
		fwrite(stData.pszBuff, 1, stData.iBuff, fp);
		fputs("\n",fp);
	}
	iRet = tdr_query(&hDBResult, g_hDBHandle, &stData);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		const char *dbms_error = tdr_dbms_error(g_hDBHandle);
		printf("failed to tdr_query: %s,dbms error: %s, sql:%s\n", tdr_error_string(iRet),
			dbms_error, stData.pszBuff);
	}else
	{
        //int iGetRows = tdr_num_rows(hDBResult);
		printf("select affected rows %ld, get rows : %lu\n", tdr_affected_rows(hDBResult),
			tdr_num_rows(hDBResult));

		memcpy(&stSelectData, &stAllData, sizeof(stSelectData));
        CU_ASSERT(0 == memcmp(&stAllData, &stSelectData, sizeof(stAllData)));
		stSelectDBObj.iObjSize = sizeof(stSelectData);
		stSelectDBObj.pszObj = (char *)&stSelectData;
		stSelectDBObj.pstMeta = pstMeta;
		stSelectDBObj.iVersion = stDBObj.iVersion;

		iRet = tdr_fetch_row(hDBResult, &stSelectDBObj);
		CU_ASSERT(0 <= iRet);
		if (TDR_ERR_IS_ERROR(iRet))
		{
			const char *dbms_error = tdr_dbms_error(g_hDBHandle);
			printf("failed to obj2sql: %s, dbms error:%s\n", tdr_error_string(iRet), dbms_error);
		}else
        {
            size_t i;
            char *pch = (char *)&stAllData;
            char *pch1 = (char *)&stSelectData;

            printf("memcmp\n");
            for (i = 0; i < sizeof(stAllData); i++)
            {
                if (*pch++ != *pch1++)
                {
                    break;
                }

            }
            printf("sizeof(stSelectData)=%"TDRPRI_SIZET
                " i = %"TDRPRI_SIZET"\n",
                sizeof(stSelectData), i);
            CU_ASSERT(0 == memcmp(&stAllData, &stSelectData, sizeof(stAllData)));
            //if (0 != memcmp(&stAllData, &stSelectData, sizeof(stAllData)))
            {
                TDRDATA stOut;

                stOut.iBuff = sizeof(stAllData);
                stOut.pszBuff = (char *)&stAllData;
                iRet = tdr_output_file(pstMeta, "stTransNodeDesc.xml", &stOut, 0, 0);
                if (0 != iRet)
                {
                    printf("tdr_output_file failed to printf file:%s:%s\n", "stTransNodeDesc.xml", tdr_error_string(iRet));
                }
                stOut.pszBuff = (char *)&stSelectData;
                stOut.iBuff = sizeof(stSelectData);
                tdr_output_file(pstMeta, "stSelectTransNodeDesc.xml", &stOut, 0, 0);
                if (0 != iRet)
                {
                    printf("tdr_output_file failed to printf file:%s:%s\n", "stSelectTransNodeDesc.xml", tdr_error_string(iRet));
                }
            }
        }/*if (0 <= tdr_num_rows(hDBResult))*/
	}

    /*select*/
    stData.iBuff = sizeof(g_szSql);
    stDBObj.iObjSize = sizeof(stAllData);
    iRet = tdr_obj2sql(g_hDBHandle, &stData, TDR_DBOP_SELECT, &stDBObj, NULL);
    CU_ASSERT(0 <= iRet);
    if (TDR_ERR_IS_ERROR(iRet))
    {
        printf("failed to obj2sql: %s", tdr_error_string(iRet));
    }else
    {
        fwrite(stData.pszBuff, 1, stData.iBuff, fp);
        fputs("\n",fp);
    }
    iRet = tdr_query_quick(&hDBResult, g_hDBHandle, &stData);
    if (TDR_ERR_IS_ERROR(iRet))
    {
        const char *dbms_error = tdr_dbms_error(g_hDBHandle);
        printf("failed to tdr_query: %s,dbms error: %s, sql:%s\n", tdr_error_string(iRet),
            dbms_error, stData.pszBuff);
    }else
    {
        //int iGetRows = tdr_num_rows(hDBResult);
        printf("select affected rows %ld, get rows : %lu\n", tdr_affected_rows(hDBResult),
            tdr_num_rows(hDBResult));

        memcpy(&stSelectData, &stAllData, sizeof(stSelectData));
        stSelectDBObj.iObjSize = sizeof(stSelectData);
        stSelectDBObj.pszObj = (char *)&stSelectData;
        stSelectDBObj.pstMeta = pstMeta;
        stSelectDBObj.iVersion = stDBObj.iVersion;

        iRet = tdr_fetch_row(hDBResult, &stSelectDBObj);
        CU_ASSERT(0 <= iRet);
        if (TDR_ERR_IS_ERROR(iRet))
        {
            const char *dbms_error = tdr_dbms_error(g_hDBHandle);
            printf("failed to obj2sql: %s, dbms error:%s\n", tdr_error_string(iRet), dbms_error);
        }

        stSelectDBObj.iObjSize = sizeof(stSelectData);
        iRet = tdr_fetch_row(hDBResult, &stSelectDBObj);
        CU_ASSERT(TDR_ERROR_DB_NO_RECORD_IN_RESULTSET == TDR_ERR_GET_ERROR_CODE(iRet));
        printf("select affected rows %ld, get rows : %lu\n", tdr_affected_rows(hDBResult),
            tdr_num_rows(hDBResult));

        tdr_free_dbresult(&hDBResult);
    }



	/*delete*/
	stData.iBuff = sizeof(g_szSql);
	iRet = tdr_obj2sql(g_hDBHandle, &stData, TDR_DBOP_DELETE, &stDBObj, NULL);
	CU_ASSERT(0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("failed to obj2sql: %s , sql:%s", tdr_error_string(iRet), stData.pszBuff);
	}else
	{
		fwrite(stData.pszBuff, 1, stData.iBuff, fp);
		fputs("\n",fp);
	}
	iRet = tdr_query(&hDBResult, g_hDBHandle, &stData);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		const char *dbms_error = tdr_dbms_error(g_hDBHandle);
		printf("failed to tdr_query: %s,dbms error: %s, sql: %s\n", tdr_error_string(iRet),
			dbms_error, stData.pszBuff);
	}else
	{
		printf("delete affected rows %ld\n", tdr_affected_rows(hDBResult));
	}

	fclose(fp);
	puts("ut_extend_array_table ok");
}

void init_extend_array_table_data(LPTRANSNODEDESC pstData)
{
	int i;
	unsigned int j;

	assert(NULL != pstData);

	pstData->iId = rand();
	pstData->iEntryNum = rand() % (sizeof(pstData->astTransNodeEntryDesc)/sizeof(pstData->astTransNodeEntryDesc[0]) -1);
	pstData->iEntryNum += 1;
	for (i = 0; i < pstData->iEntryNum; i++)
	{

		LPTRANSNODEENTRYDESC pstDesc = &pstData->astTransNodeEntryDesc[i];
		pstDesc->iPrice = rand();
		pstDesc->iTransDstDescID = rand();
		for (j = 0; j < sizeof(pstDesc->stAttachInfo.astAttach)/sizeof(pstDesc->stAttachInfo.astAttach[0]);
			j++)
		{
			LPTRANSNODEATTACHINFO pstAttach = &(pstDesc->stAttachInfo.astAttach[j]);
			pstAttach->iMapid = rand();
			snprintf(pstAttach->szName,  sizeof(pstAttach->szName), "hello %d", j);
		}
	}/*for (i = 0; i < pstData->iEntryNum; i++)*/

	{
		LPTRANSNODEENTRYDESC pstDesc = &pstData->stDefaultTransNode;
		pstDesc->iPrice = rand();
		pstDesc->iTransDstDescID = rand();
		for (j = 0; j < sizeof(pstDesc->stAttachInfo.astAttach)/sizeof(pstDesc->stAttachInfo.astAttach[0]);
			j++)
		{
			LPTRANSNODEATTACHINFO pstAttach = &(pstDesc->stAttachInfo.astAttach[j]);
			pstAttach->iMapid = rand();
			snprintf(pstAttach->szName,  sizeof(pstAttach->szName), "default hello %d", j);
		}
	}

	{
		LPTRANSNODEENTRYDESC pstDesc = &pstData->stSencodTransNode;
		pstDesc->iPrice = rand();
		pstDesc->iTransDstDescID = rand();
		for (j = 0; j < sizeof(pstDesc->stAttachInfo.astAttach)/sizeof(pstDesc->stAttachInfo.astAttach[0]);
			j++)
		{
			LPTRANSNODEATTACHINFO pstAttach = &(pstDesc->stAttachInfo.astAttach[j]);
			pstAttach->iMapid = rand();
			snprintf(pstAttach->szName,  sizeof(pstAttach->szName), "second hello %d", j);
		}
	}


}

void ut_auto_increment()
{
	int iRet ;
	LPTDRMETA pstMeta;
	TESTAUTOINCREMENT stAllData;
	TDRDATA stData;
	static unsigned int dwLastID;
	TDRDBOBJECT stDBObj;
	TDRDBRESULTHANDLE hDBResult;


	TESTAUTOINCREMENT stSelectData;
	TDRDBOBJECT stSelectDBObj;

	puts("ut_auto_increment begin...");

	pstMeta = tdr_get_meta_by_name(g_pDBLib, "TestAutoIncrement");
	CU_ASSERT(NULL != pstMeta);
	if (NULL == pstMeta)
	{
		printf("failed to get meta by name %s\n", "TestAutoIncrement");
		return;
	}
	CU_ASSERT(0 != tdr_do_have_autoincrement_entry(pstMeta));


	/*构造数据*/
	stAllData.dwID = 0;
	snprintf(stAllData.szName, sizeof(stAllData.szName), "test%d", rand());


	/*生成insert语句*/
	stData.iBuff = sizeof(g_szSql);
	stData.pszBuff = &g_szSql[0];
	stDBObj.iObjSize = sizeof(stAllData);
	stDBObj.iVersion = 0;
	stDBObj.pstMeta = pstMeta;
	stDBObj.pszObj = (char *)&stAllData;
	iRet = tdr_obj2sql(g_hDBHandle, &stData, TDR_DBOP_INSERT, &stDBObj, NULL);
	CU_ASSERT(0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		const char *dbms_error = tdr_dbms_error(g_hDBHandle);
		printf("failed to obj2sql: %s, dbms error: %s\n", tdr_error_string(iRet), dbms_error);
	}

	/*执行*/
	iRet = tdr_query(&hDBResult, g_hDBHandle, &stData);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		const char *dbms_error = tdr_dbms_error(g_hDBHandle);
		printf("failed to tdr_query: %s,dbms error: %s, sql:%s\n", tdr_error_string(iRet),
			dbms_error, stData.pszBuff);
	}else
	{
		printf("insert affected rows %ld\n", tdr_affected_rows(hDBResult));
	}
	dwLastID = (unsigned int)tdr_dbms_insert_id(g_hDBHandle);
	printf("auto increment id=%u\n", dwLastID);


	/*再插入一条记录*/
	iRet = tdr_query(&hDBResult, g_hDBHandle, &stData);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		const char *dbms_error = tdr_dbms_error(g_hDBHandle);
		printf("failed to tdr_query: %s,dbms error: %s, sql:%s\n", tdr_error_string(iRet),
			dbms_error, stData.pszBuff);
	}else
	{
		printf("insert affected rows %ld\n", tdr_affected_rows(hDBResult));
	}
	CU_ASSERT((dwLastID + 1) == (unsigned int)tdr_dbms_insert_id(g_hDBHandle));



	/*update*/
	//stAllData.dwID = dwLastID + 1;
	stData.iBuff = sizeof(g_szSql);
	snprintf(stAllData.szName, sizeof(stAllData.szName), "test%d", rand());
	stDBObj.iObjSize = sizeof(stAllData);
	iRet = tdr_obj2sql(g_hDBHandle, &stData, TDR_DBOP_UPDATE, &stDBObj, NULL);
	CU_ASSERT(0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("failed to obj2sql: %s\n", tdr_error_string(iRet));
	}
	iRet = tdr_query_quick(&hDBResult, g_hDBHandle, &stData);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		const char *dbms_error = tdr_dbms_error(g_hDBHandle);
		printf("failed to tdr_query: %s,dbms error: %s, sql:%s\n", tdr_error_string(iRet),
			dbms_error, stData.pszBuff);
	}else
	{
		printf("update affected rows %ld\n", tdr_affected_rows(hDBResult));
	}


	/*select*/
	stData.iBuff = sizeof(g_szSql);
	stDBObj.iObjSize = sizeof(stAllData);
	iRet = tdr_obj2sql(g_hDBHandle, &stData, TDR_DBOP_SELECT, &stDBObj, NULL);
	CU_ASSERT(0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("failed to obj2sql: %s", tdr_error_string(iRet));
	}
	iRet = tdr_query_quick(&hDBResult, g_hDBHandle, &stData);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		const char *dbms_error = tdr_dbms_error(g_hDBHandle);
		printf("failed to tdr_query: %s,dbms error: %s, sql:%s\n", tdr_error_string(iRet),
			dbms_error, stData.pszBuff);
	}else
	{
		printf("select affected rows %ld, get rows : %lu\n", tdr_affected_rows(hDBResult),
			tdr_num_rows(hDBResult));

		memcpy(&stSelectData, &stAllData, sizeof(stSelectData));
		stSelectDBObj.iObjSize = sizeof(stSelectData);
		stSelectDBObj.pszObj = (char *)&stSelectData;
		stSelectDBObj.pstMeta = pstMeta;
		stSelectDBObj.iVersion = stDBObj.iVersion;

		iRet = tdr_fetch_row(hDBResult, &stSelectDBObj);
		CU_ASSERT(0 <= iRet);
		if (TDR_ERR_IS_ERROR(iRet))
		{
			const char *dbms_error = tdr_dbms_error(g_hDBHandle);
			printf("failed to obj2sql: %s, dbms error:%s\n", tdr_error_string(iRet), dbms_error);
		}
		CU_ASSERT(0 == memcmp(&stAllData, &stSelectData, sizeof(stAllData)));
		//if (0 != memcmp(&stAllData, &stSelectData, sizeof(stAllData)))
		{
			TDRDATA stOut;

			stOut.iBuff = sizeof(stAllData);
			stOut.pszBuff = (char *)&stAllData;
			tdr_output_file(pstMeta, "TestAutoIncrement.xml", &stOut, stDBObj.iVersion, 2);
			stOut.pszBuff = (char *)&stSelectData;
			tdr_output_file(pstMeta, "stSelectTestAutoIncrement.xml", &stOut, stDBObj.iVersion, 2);
		}

		stSelectDBObj.iObjSize = sizeof(stSelectData);
		iRet = tdr_fetch_row(hDBResult, &stSelectDBObj);
		printf("select affected rows %ld, get rows : %lu, iRet 0x%x\n", tdr_affected_rows(hDBResult),
			tdr_num_rows(hDBResult), iRet);

		tdr_free_dbresult(&hDBResult);
	}




	/*delete*/
	stData.iBuff = sizeof(g_szSql);
	iRet = tdr_obj2sql(g_hDBHandle, &stData, TDR_DBOP_DELETE, &stDBObj, NULL);
	CU_ASSERT(0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("failed to obj2sql: %s , sql:%s", tdr_error_string(iRet), stData.pszBuff);
	}
	iRet = tdr_query(&hDBResult, g_hDBHandle, &stData);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		const char *dbms_error = tdr_dbms_error(g_hDBHandle);
		printf("failed to tdr_query: %s,dbms error: %s, sql: %s\n", tdr_error_string(iRet),
			dbms_error, stData.pszBuff);
	}else
	{
		printf("delete affected rows %ld\n", tdr_affected_rows(hDBResult));
	}

	/*insert again*/
	stData.iBuff = sizeof(g_szSql);
	snprintf(stAllData.szName, sizeof(stAllData.szName), "test%d", rand());
	stDBObj.iObjSize = sizeof(stAllData);
	iRet = tdr_obj2sql(g_hDBHandle, &stData, TDR_DBOP_INSERT, &stDBObj, NULL);
	CU_ASSERT(0 <= iRet);

	iRet = tdr_query(&hDBResult, g_hDBHandle, &stData);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		const char *dbms_error = tdr_dbms_error(g_hDBHandle);
		printf("failed to tdr_query: %s,dbms error: %s, sql:%s\n", tdr_error_string(iRet),
			dbms_error, stData.pszBuff);
	}else
	{
		printf("insert affected rows %ld\n", tdr_affected_rows(hDBResult));
	}
	CU_ASSERT((dwLastID + 2) == (unsigned int)tdr_dbms_insert_id(g_hDBHandle));

	puts("ut_auto_increment ok");
}
