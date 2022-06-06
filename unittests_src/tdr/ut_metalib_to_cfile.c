/**
*
* @file     ut_metalib_to_cfile.c 
* @brief    测试metalib转换到c文件
* 
* @author jackyai  
* @version 1.0
* @date 2007-06-14
*
*
* Copyright (c)  2007, 腾讯科技有限公司互动娱乐研发部
* All rights reserved.
*
*/

#include <stdio.h>
#include <CUnit/CUnit.h>
#include <CUnit/TestDB.h>

#include "tdr/tdr.h"
#include "ut_tdr.h"

extern unsigned char g_szMetalib_cs[];


LPTDRMETALIB g_pst_ovcs_metalib = NULL;

static void ut_use_cfile_ovcs_metalib();
static int ut_metalib2cfile_suite_init();
static int ut_metalib2cfile_suite_clean();

int ut_add_metalib_to_cfile_test_suites()
{
	CU_pSuite pSuite = NULL;


	pSuite = CU_add_suite("ut_metalib_to_cfile", 
		(CU_InitializeFunc)ut_metalib2cfile_suite_init, (CU_CleanupFunc)ut_metalib2cfile_suite_clean);
	if(NULL == pSuite)
	{//检测注册suite情况　
		CU_cleanup_registry();
		return CU_get_error();
	}

	if( NULL == CU_add_test(pSuite, "ut_use_cfile_ovcs_metalib", (CU_TestFunc)ut_use_cfile_ovcs_metalib) )
	{
		CU_cleanup_registry();
		return CU_get_error();
	}

	return 0;
}

int ut_metalib2cfile_suite_init()
{
	int iRet;

	iRet = tdr_create_lib_file(&g_pst_ovcs_metalib, TEST_FILE_PATH"ov_cs.xml", 0, stderr);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("failed to tdr_create_lib: %s\n", tdr_error_string(iRet));
	}else
	{
		iRet = tdr_metalib_to_cfile(g_pst_ovcs_metalib, "ov_cs.c");
		if (TDR_ERR_IS_ERROR(iRet))
		{
			printf("failed to tdr_create_lib: %s\n", tdr_error_string(iRet));
		}
	}

	if (TDR_ERR_IS_ERROR(iRet))
	{
		return iRet;
	}

	return 0;
}

int ut_metalib2cfile_suite_clean()
{
	tdr_free_lib(&g_pst_ovcs_metalib);

	return 0;
}

void ut_use_cfile_ovcs_metalib()
{
	LPTDRMETALIB pstLib;
	LPTDRMETALIB pstNewLib;
	int iRet;
	LPTDRMETA pstMeta;
	
	puts("ut_use_cfile_ovcs_metalib begin...");
	pstLib= (LPTDRMETALIB)g_szMetalib_cs;
	CU_ASSERT(0 == memcmp(g_pst_ovcs_metalib, pstLib, tdr_size(g_pst_ovcs_metalib)));

    tdr_dump_metalib_file(pstLib, "ov_cs_metalib.txt");
	iRet = tdr_save_xml_file(pstLib, "ov_cs_cfile.xml");
	CU_ASSERT(0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_save_metalib failed: %s. FILE: %s", tdr_error_string(iRet), __FILE__);
	}

	iRet = tdr_create_lib_file(&pstNewLib, "ov_cs_cfile.xml", 1, stderr);
	CU_ASSERT(0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_create_lib_file failed: %s. FILE: %s", tdr_error_string(iRet), __FILE__);
	}

	pstMeta = tdr_get_meta_by_name(pstNewLib, "ZoneFmBasic");
	CU_ASSERT(NULL != pstMeta);

	puts("ut_use_cfile_ovcs_metalib ok");
}
