#include <stdio.h>
#include <CUnit/CUnit.h>
#include <CUnit/TestDB.h>


#include "tdr/tdr_error.h"
#include "tdr_error_i.h"


void ut_Test_TdrErr2Str()
{
    int iErr = 0;

    CU_ASSERT_STRING_EQUAL(tdr_error_string(0), "没有错误");

    tdr_set_last_error(TDR_ERRIMPLE_FAILED_EXPACT_XML);
    iErr = tdr_error_code();

    CU_ASSERT_STRING_EQUAL(tdr_error_string(iErr), "解析XML信息错误");
    //puts(tdr_error_string(iErr));

    CU_ASSERT_STRING_EQUAL(tdr_error_string(-1), "未知错误");
}

int ut_add_tdr_errorhanlder_test_suites()
{
	CU_pSuite pSuite = NULL;
	
	//CU_add_suite 增加一个Suite 
	//InitSuite EndSuite:分别是测试单元初始和释放函数,如不需要则NULL传递
	pSuite = CU_add_suite("ut_Test_ErrorHnadle", NULL, NULL);
	if(NULL == pSuite)
	{//检测注册suite情况　
		CU_cleanup_registry();
		return CU_get_error();
	}
	
	//注册当前Suite下的测试用例　
	//pSuite:用例指针
	//"Test1": 测试单元名称 
	//Test1:测试函数 
	if( NULL == CU_add_test(pSuite, "ut_Test_TdrErr2Str", (CU_TestFunc)ut_Test_TdrErr2Str) )
	{
		CU_cleanup_registry();
		return CU_get_error();
	}
	
	return 0;
}
