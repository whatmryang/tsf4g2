/**
*
* @file     test_save_metalib.c 
* @brief    测试保存metalib的函数
* 
* @author jackyai  
* @version 1.0
* @date 2007-04-25 
*
*
* Copyright (c)  2007, 腾讯科技有限公司互动娱乐研发部
* All rights reserved.
*
*/

#include <stdio.h>
#include <CUnit/CUnit.h>
#include <CUnit/TestDB.h>

#include <assert.h>

#include "tdr/tdr_metalib_to_hpp.h"
#include "tdr/tdr_XMLMetaLib.h"
#include "ut_tdr.h"
#include "tdr/tdr_error.h"
#include "simpleMetalibV1.h"
#include "pal/pal.h"

void ut_save_simplemetalib_to_hpp();
void ut_save_simplemetalib_to_hpp_spec();
void ut_save_simplemetalibV0_to_hpp();
void ut_save_simplemetalibV0_to_hpp_spec();
void ut_save_fometalib_to_hpp();


int ut_add_metalib_to_hpp_test_suites()
{
	CU_pSuite pSuite = NULL;

	//CU_add_suite 增加一个Suite 
	//InitSuite EndSuite:分别是测试单元初始和释放函数,如不需要则NULL传递
	pSuite = CU_add_suite("ut_add_metalib_to_hpp_test_suites", NULL, NULL);
	if(NULL == pSuite)
	{//检测注册suite情况　
		CU_cleanup_registry();
		return CU_get_error();
	}

	//注册当前Suite下的测试用例　
	//pSuite:用例指针
	//"Test1": 测试单元名称 
	//Test1:测试函数 
	if( NULL == CU_add_test(pSuite, "ut_save_simplemetalib_to_hpp", (CU_TestFunc)ut_save_simplemetalib_to_hpp))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}

	if( NULL == CU_add_test(pSuite, "ut_save_simplemetalib_to_hpp_spec", (CU_TestFunc)ut_save_simplemetalib_to_hpp_spec))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}

	if( NULL == CU_add_test(pSuite, "ut_save_simplemetalibV0_to_hpp", (CU_TestFunc)ut_save_simplemetalibV0_to_hpp))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}

	if( NULL == CU_add_test(pSuite, "ut_save_simplemetalibV0_to_hpp_spec", (CU_TestFunc)ut_save_simplemetalibV0_to_hpp_spec))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}

	if( NULL == CU_add_test(pSuite, "ut_save_fometalib_to_hpp", (CU_TestFunc)ut_save_fometalib_to_hpp))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}
	

	return 0;
}

void ut_save_simplemetalib_to_hpp()
{
	int iRet = 0;
	LPTDRMETALIB pstLib = NULL;
	TDRHPPRULE stRule ;
	LPTDRMETA pstMeta;

	memset(&stRule, 0, sizeof(stRule));
	printf("\ntest file: %s to hpp\n", TEST_FILE_PATH"testMetalibParam.xml");
	iRet = tdr_create_lib_file(&pstLib, TEST_FILE_PATH"testMetalibParam.xml", 1, stderr);
	CU_ASSERT(0 <= iRet); 
	

	stRule.iRule = TDR_HPPRULE_ADD_CUSTOM_PREFIX;
	strncpy(stRule.szCustomNamePrefix, "", sizeof(stRule.szCustomNamePrefix));
	iRet = tdr_metalib_to_hpp(pstLib, "simpleMetalibV1.h", &stRule);
	CU_ASSERT(0 <= iRet); 

	stRule.iRule = TDR_HPPRULE_ADD_CUSTOM_PREFIX | TDR_HPPRULE_NO_LOWERCASE_PREFIX;
	strncpy(stRule.szCustomNamePrefix, "", sizeof(stRule.szCustomNamePrefix));
	iRet = tdr_metalib_to_hpp(pstLib, "simpleMetalibV2.h", &stRule);
	CU_ASSERT(0 <= iRet); 

	stRule.iRule = TDR_HPPRULE_ADD_CUSTOM_PREFIX | TDR_HPPRULE_NO_TYPE_PREFIX;
	strncpy(stRule.szCustomNamePrefix, "m_", sizeof(stRule.szCustomNamePrefix));
	iRet = tdr_metalib_to_hpp(pstLib, "simpleMetalibV3.h", &stRule);

	CU_ASSERT(0 <= iRet); 

	pstMeta = tdr_get_meta_by_name(pstLib, "TestAlign");
	CU_ASSERT(NULL != pstMeta);
	CU_ASSERT(sizeof(TESTALIGN) == tdr_get_meta_size(pstMeta));
	printf("sizeof TestAlign = %"PRIdPTR"\n", tdr_get_meta_size(pstMeta));

	tdr_free_lib(&pstLib);
}

void ut_save_simplemetalibV0_to_hpp()
{
	int iRet = 0;
	LPTDRMETALIB pstLib = NULL;
	TDRHPPRULE stRule ;

	printf("\ntest file: %s to hpp\n", TEST_FILE_PATH"testMetalibParamV0.xml");
	iRet = tdr_create_lib_file(&pstLib, TEST_FILE_PATH"/testMetalibParamV0.xml", 0, stderr);
	CU_ASSERT(0 <= iRet); 

	memset(&stRule, 0, sizeof(stRule));
	stRule.iRule = TDR_HPPRULE_NO_TYPE_PREFIX;
	iRet = tdr_metalib_to_hpp(pstLib, "simpleMetalibV0.h", &stRule);
	CU_ASSERT(0 <= iRet); 

	tdr_free_lib(&pstLib);
}

void ut_save_fometalib_to_hpp()
{
	int iRet = 0;
	LPTDRMETALIB pstLib = NULL;
	TDRHPPRULE stRule ;

	printf("\ntest file: %s to hpp\n", TEST_FILE_PATH"ov_cs.xml");
	iRet = tdr_create_lib_file(&pstLib, TEST_FILE_PATH"ov_cs.xml", 0, stderr);
	CU_ASSERT(0 <= iRet); 

	memset(&stRule, 0, sizeof(stRule));
	stRule.iRule = 0;
	iRet = tdr_metalib_to_hpp(pstLib, "ov_cs.h", &stRule);
	CU_ASSERT(0 <= iRet); 

	tdr_free_lib(&pstLib);
}

void ut_save_simplemetalib_to_hpp_spec()
{
	int iRet = 0;
	LPTDRMETALIB pstLib = NULL;
	TDRHPPRULE stRule ;
	char *aszFile[] = {TEST_FILE_PATH"testMetalibParam_spec1.xml",
		TEST_FILE_PATH"testMetalibParam_spec.xml"};

	printf("\nut_save_simplemetalib_to_hpp_spec\n");
	iRet = tdr_create_lib_multifile(&pstLib, (const char **)aszFile, sizeof(aszFile)/sizeof(char *), 1, stderr);
	CU_ASSERT(0 <= iRet); 

	memset(&stRule, 0, sizeof(stRule));
	stRule.iRule = TDR_HPPRULE_ADD_CUSTOM_PREFIX;
	strncpy(stRule.szCustomNamePrefix, "m_", sizeof(stRule.szCustomNamePrefix));
	iRet = tdr_metalib_to_hpp_spec(pstLib, TEST_FILE_PATH"testMetalibParam_spec.xml", 1,
		"simpleMetalibV1_spec.h", &stRule, stderr);
	CU_ASSERT(0 <= iRet); 


	stRule.iRule = TDR_HPPRULE_ADD_CUSTOM_PREFIX | TDR_HPPRULE_NO_TYPE_DECLARE;
	iRet = tdr_metalib_to_hpp_spec(pstLib, TEST_FILE_PATH"testMetalibParam_spec1.xml", 1,
		"simpleMetalibV1_spec1.h", &stRule, stderr);
	CU_ASSERT(0 <= iRet); 

	tdr_free_lib(&pstLib);
}

void ut_save_simplemetalibV0_to_hpp_spec()
{
	int iRet = 0;
	LPTDRMETALIB pstLib = NULL;
	TDRHPPRULE stRule ;

	printf("\ntest file: %s to hpp\n", TEST_FILE_PATH"testMetalibParamV0_spec.xml");
	iRet = tdr_create_lib_file(&pstLib, TEST_FILE_PATH"testMetalibParamV0.xml", 0, stderr);
	CU_ASSERT(0 <= iRet); 

	memset(&stRule, 0, sizeof(stRule));
	stRule.iRule = TDR_HPPRULE_ADD_CUSTOM_PREFIX;
	strncpy(stRule.szCustomNamePrefix, "m_", sizeof(stRule.szCustomNamePrefix));
	iRet = tdr_metalib_to_hpp_spec(pstLib, TEST_FILE_PATH"testMetalibParamV0_spec.xml", 0,
		"simpleMetalibV0_spec.h", &stRule, stderr);
	CU_ASSERT(0 <= iRet); 

	tdr_free_lib(&pstLib);
}






