/**
*
* @file     ut_test_tdr_db.c 
* @brief    测试利用元数据生成DDL
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

#include "ut_tdr_db.h"
#include "tdr/tdr.h"
#include "ut_tdr.h"


#define DATE_SORT_MAX_VERSION 10

static LPTDRMETALIB g_pstLib = NULL;
static int ut_suite_init();
static int ut_suite_clean();
static void ut_save_metalib_to_hpp();
static void ut_save_metalib_to_xml();
static void ut_test_extend_to_table();
static void ut_test_create_splitable_table();
static void ut_test_create_all_base_type();
static void ut_test_alter_extend_table();
static void ut_test_alter_all_base_type_table();
static void ut_test_create_table_TransNodeDesc();
static void ut_test_create_table_TestAutoIncrement();


int ut_add_database_test_suites()
{
	CU_pSuite pSuite = NULL;


	pSuite = CU_add_suite("ut_add_database_test_suites", 
		(CU_InitializeFunc)ut_suite_init, (CU_CleanupFunc)ut_suite_clean);
	if(NULL == pSuite)
	{//检测注册suite情况　
		CU_cleanup_registry();
		return CU_get_error();
	}

	
	if( NULL == CU_add_test(pSuite, "ut_save_metalib_to_hpp", (CU_TestFunc)ut_save_metalib_to_hpp))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}

	if( NULL == CU_add_test(pSuite, "ut_save_metalib_to_xml", (CU_TestFunc)ut_save_metalib_to_xml))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}

	if( NULL == CU_add_test(pSuite, "ut_test_extend_to_table", (CU_TestFunc)ut_test_extend_to_table))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}

	if( NULL == CU_add_test(pSuite, "ut_test_create_splitable_table", (CU_TestFunc)ut_test_create_splitable_table))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}
	
	if( NULL == CU_add_test(pSuite, "ut_test_create_all_base_type", (CU_TestFunc)ut_test_create_all_base_type))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}

	if( NULL == CU_add_test(pSuite, "ut_test_alter_extend_table", (CU_TestFunc)ut_test_alter_extend_table))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}

	if( NULL == CU_add_test(pSuite, "ut_test_alter_all_base_type_table", (CU_TestFunc)ut_test_alter_all_base_type_table))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}

	if( NULL == CU_add_test(pSuite, "ut_test_create_table_TransNodeDesc", (CU_TestFunc)ut_test_create_table_TransNodeDesc))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}

	if( NULL == CU_add_test(pSuite, "ut_test_create_table_TestAutoIncrement", (CU_TestFunc)ut_test_create_table_TestAutoIncrement))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}
	
	
	return 0;
}

int ut_suite_init()
{
	int iRet ;

	srand((unsigned int)time(NULL));
	iRet = tdr_create_lib_file(&g_pstLib, TEST_FILE_PATH"tdr_database.xml", 1, stdout);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_create_lib_file failed : %s\n", tdr_error_string((iRet)));
		return iRet;
	}else
	{
		ut_dump_metalib(g_pstLib, "tdr_db_metalib_domp.txt");
	}

	return 0;	
}

int ut_suite_clean()
{
	tdr_free_lib(&g_pstLib);
	return 0;
}




void ut_save_metalib_to_hpp()
{
	int iRet = 0;
	TDRHPPRULE stRule ;

	memset(&stRule, 0, sizeof(stRule));
	stRule.iRule = TDR_HPPRULE_DEFAULT;
	iRet = tdr_metalib_to_hpp(g_pstLib, "ut_tdr_db.h", &stRule);
	CU_ASSERT(0 <= iRet); 
}

void ut_save_metalib_to_xml()
{
	int iRet = 0;
	LPTDRMETALIB pstLib;


	iRet = tdr_save_xml_file(g_pstLib, "tdr_database_new.xml");
	CU_ASSERT(0 <= iRet); 

	iRet = tdr_create_lib_file(&pstLib, "tdr_database_new.xml", 1, stderr);
	CU_ASSERT(0 <= iRet); 

	//CU_ASSERT(0 == memcmp(pstLib, g_pstLib, tdr_size(g_pstLib))); 
	ut_dump_metalib(pstLib, "tdr_new_db_metalib_domp.txt");

	tdr_free_lib(&pstLib);
}


void ut_test_create_table_TestAutoIncrement()
{
	int iRet = TDR_SUCCESS;
	LPTDRMETA pstMeta;
	TDRDBMS stDBMS;

	memset(&stDBMS, 0, sizeof(stDBMS));
	printf("tdr_create_table_file: ut_test_create_table_TestAutoIncrement\n");
	pstMeta = tdr_get_meta_by_name(g_pstLib, "TestAutoIncrement");
	CU_ASSERT(NULL != pstMeta);

	strncpy(stDBMS.szDBMSName, "mysql", sizeof(stDBMS.szDBMSName));
	iRet = tdr_create_table_file(&stDBMS, pstMeta, "TestAutoIncrement.sql",
		tdr_get_meta_current_version(pstMeta));
	CU_ASSERT(0 <= iRet); 
}

void ut_test_create_table_TransNodeDesc()
{
	int iRet = TDR_SUCCESS;
	LPTDRMETA pstMeta;
	TDRDBMS stDBMS;

	memset(&stDBMS, 0, sizeof(stDBMS));
	printf("tdr_create_table_file: ut_test_create_table_TransNodeDesc\n");
	pstMeta = tdr_get_meta_by_name(g_pstLib, "TransNodeDesc");
	CU_ASSERT(NULL != pstMeta);

	strncpy(stDBMS.szDBMSName, "mysql", sizeof(stDBMS.szDBMSName));
	iRet = tdr_create_table_file(&stDBMS, pstMeta, "TransNodeDesc.sql",
		tdr_get_meta_current_version(pstMeta));
	CU_ASSERT(0 <= iRet); 
}


void ut_test_extend_to_table()
{
	int iRet = TDR_SUCCESS;
	LPTDRMETA pstMeta;
	TDRDBMS stDBMS;

	memset(&stDBMS, 0, sizeof(stDBMS));
	printf("tdr_create_table_file: TestExtendTable\n");
	pstMeta = tdr_get_meta_by_name(g_pstLib, "TestExtendTable");
	CU_ASSERT(NULL != pstMeta);

	strncpy(stDBMS.szDBMSName, "mysql", sizeof(stDBMS.szDBMSName));
	iRet = tdr_create_table_file(&stDBMS, pstMeta, "TestExtendTable.sql",
		tdr_get_meta_current_version(pstMeta));
	CU_ASSERT(0 <= iRet); 
}

void ut_test_create_splitable_table()
{
	int iRet = TDR_SUCCESS;
	LPTDRMETA pstMeta;
	TDRDBMS stDBMS;

	memset(&stDBMS, 0, sizeof(stDBMS));
	printf("tdr_create_table_file: UserItemInfo\n");
	pstMeta = tdr_get_meta_by_name(g_pstLib, "UserItemInfo");
	CU_ASSERT(NULL != pstMeta);

	strncpy(stDBMS.szDBMSName, "mysql", sizeof(stDBMS.szDBMSName));
	iRet = tdr_create_table_file(&stDBMS, pstMeta, "UserItemInfo.sql",
		tdr_get_meta_current_version(pstMeta));
	CU_ASSERT(0 <= iRet); 
}

void ut_test_create_all_base_type()
{
	int iRet = TDR_SUCCESS;
	LPTDRMETA pstMeta;
	TDRDBMS stDBMS;

	memset(&stDBMS, 0, sizeof(stDBMS));
	printf("tdr_create_table_file: AllBaseTypeData\n");
	pstMeta = tdr_get_meta_by_name(g_pstLib, "AllBaseTypeData");
	CU_ASSERT(NULL != pstMeta);

	strncpy(stDBMS.szDBMSName, "mysql", sizeof(stDBMS.szDBMSName));
	iRet = tdr_create_table_file(&stDBMS, pstMeta, "AllBaseTypeData.sql",
		tdr_get_meta_current_version(pstMeta));
	CU_ASSERT(0 <= iRet); 
}

void ut_test_alter_all_base_type_table()
{
	int iRet = TDR_SUCCESS;
	LPTDRMETA pstMeta;
	TDRDBMS stDBMS;

	memset(&stDBMS, 0, sizeof(stDBMS));
	printf("tdr_create_table_file: alter AllBaseTypeData table\n");
	pstMeta = tdr_get_meta_by_name(g_pstLib, "AllBaseTypeData");
	CU_ASSERT(NULL != pstMeta);

	strncpy(stDBMS.szDBMSName, "mysql", sizeof(stDBMS.szDBMSName));
	iRet = tdr_alter_table_file(&stDBMS, pstMeta, "alter_AllBaseTypeData.sql",
		tdr_get_meta_based_version(pstMeta));
	CU_ASSERT(0 <= iRet); 
}

void ut_test_alter_extend_table()
{
	int iRet = TDR_SUCCESS;
	LPTDRMETA pstMeta;
	TDRDBMS stDBMS;

	memset(&stDBMS, 0, sizeof(stDBMS));
	printf("tdr_create_table_file: alter TestExtendTable\n");
	pstMeta = tdr_get_meta_by_name(g_pstLib, "TestExtendTable");
	CU_ASSERT(NULL != pstMeta);

	strncpy(stDBMS.szDBMSName, "mysql", sizeof(stDBMS.szDBMSName));
	iRet = tdr_alter_table_file(&stDBMS, pstMeta, "alter_TestExtendTable.sql",
		tdr_get_meta_based_version(pstMeta));
	CU_ASSERT(0 <= iRet); 
}

