/**
*
* @file     ut_meta_index.c 
* @brief    测试meta结构的索引
* 
* @author jackyai  
* @version 1.0
* @date 2007-12-05 
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
#include "tdr_metalib_kernel_i.h"


#define DATE_SORT_MAX_VERSION 10

static LPTDRMETALIB g_pstLib = NULL;
static int ut_suite_init();
static int ut_suite_clean();
static void ut_test_extend_table_index();


int ut_add_meta_index_suites()
{
	CU_pSuite pSuite = NULL;


	pSuite = CU_add_suite("ut_add_meta_index_suites", 
		(CU_InitializeFunc)ut_suite_init, (CU_CleanupFunc)ut_suite_clean);
	if(NULL == pSuite)
	{//检测注册suite情况　
		CU_cleanup_registry();
		return CU_get_error();
	}


	if( NULL == CU_add_test(pSuite, "ut_test_extend_table_index", (CU_TestFunc)ut_test_extend_table_index))
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


void ut_test_extend_table_index()
{
	LPTDRMETA pstMeta;
	LPTDRMETAENTRYNAMEINDEX pstNameIndex = NULL;
	LPTDRMETAENTRYOFFINDEX pstOffIndex = NULL;
	char szBuff[102400];
	TDRDATA stBuff;
	int iRet;
	FILE *fp = NULL;
	LPTDRENTRYOFFINDEXINFO pstOff;
	LPTDRNAMEENTRYINDEXINFO pstName;

	puts("ut_test_extend_table_index begin...");
	pstMeta = tdr_get_meta_by_name(g_pstLib, "TestExtendTable");
	CU_ASSERT(NULL != pstMeta);
	if (NULL == pstMeta)
	{
		return;
	}

	fp = fopen("extend_index_dump.txt", "w");
	CU_ASSERT(NULL != fp)
	if (NULL == fp)
	{
		return;
	}

	stBuff.iBuff = sizeof(szBuff);
	stBuff.pszBuff = &szBuff[0];
	iRet = tdr_create_entry_name_index(&pstNameIndex, pstMeta, &stBuff);
	CU_ASSERT(0 == iRet);
	if (0 != iRet)
	{
		printf("failed to tdr_create_entry_name_index: %s\n", tdr_error_string(iRet));
		fclose(fp);
		return;
	}

	iRet = tdr_create_entry_off_index(&pstOffIndex, pstMeta, NULL);
	CU_ASSERT(0 == iRet);
	if (0 != iRet)
	{
		printf("failed to tdr_create_entry_off_index: %s\n", tdr_error_string(iRet));
		fclose(fp);
		tdr_destroy_entry_name_index(&pstNameIndex);
		return;
	}

	tdr_dump_entry_name_index(fp, pstNameIndex);
	tdr_dump_entry_off_index(fp, pstOffIndex);

	pstName = tdr_find_entry_name_index(pstNameIndex, "uin");
	CU_ASSERT(NULL != pstName);
	if (NULL != pstName)
	{
		LPTDRMETA pstTmp;
		pstTmp = tdr_get_meta_by_name(g_pstLib, "UserItemInfo");
		if (NULL != pstTmp)
		{
			CU_ASSERT(pstName->pstEntry == tdr_get_entryptr_by_name(pstTmp, "uin"));
	
		}
	}
	pstName = tdr_find_entry_name_index(pstNameIndex, "items");
	CU_ASSERT(NULL != pstName);
	if (NULL != pstName)
	{
		LPTDRMETA pstTmp;
		pstTmp =tdr_get_entry_type_meta(g_pstLib, pstName->pstEntry);
	}

	
	if (NULL != pstName)
	{
		pstOff = tdr_find_entry_off_index(pstOffIndex, pstName->pstEntry);
		CU_ASSERT(NULL != pstOff);
		if (NULL != pstOff)
		{
			CU_ASSERT(pstOff->iHOff > pstName->pstEntry->iHOff);
		}
	}

	pstName = tdr_find_entry_name_index(pstNameIndex, "noentry");
	CU_ASSERT(NULL == pstName);
	pstName = tdr_find_entry_name_index(pstNameIndex, "validtime");
	CU_ASSERT(NULL != pstName);

	pstOff = tdr_find_entry_off_index(pstOffIndex, (LPTDRMETAENTRY)1);
	CU_ASSERT(NULL == pstOff);

	fclose(fp);
	tdr_destroy_entry_name_index(&pstNameIndex);
	tdr_destroy_entry_off_index(&pstOffIndex);

	puts("ut_test_extend_table_index ok");
}
