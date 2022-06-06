#include <stdio.h>
#include <time.h>
#include <CUnit/CUnit.h>
#include <CUnit/TestDB.h>
#include "tdr/tdr_ctypes_info.h"
#include "tdr/tdr_define.h"
#include "tdr_define_i.h"
#include "tdr_ctypes_info_i.h"


void ut_Test_IdxToTypeInfo()
{
	int iRet = 0;
	tdr_datetime_t tDatetime;
	time_t tNow,tTmpTime;

	//	CU_ASSERT(tdr_typename_to_idx("char") == TDR_INVALID_INDEX);
	CU_ASSERT(tdr_typename_to_idx("char") != TDR_INVALID_INDEX);


	CU_ASSERT_EQUAL(tdr_typename_to_idx("CHAR"), 30);

	CU_ASSERT_EQUAL(tdr_typename_to_idx("string"), 24);
	CU_ASSERT_EQUAL(tdr_typename_to_idx("invalid"), TDR_INVALID_INDEX);

	iRet = tdr_typename_to_idx("smallint");
	CU_ASSERT(iRet > 0);
	CU_ASSERT(tdr_typename_to_idx("struct") == 1);

	CU_ASSERT(tdr_typename_to_idx("string") > 0);
	CU_ASSERT(tdr_typename_to_idx("invalid") == TDR_INVALID_INDEX);
	CU_ASSERT(tdr_typename_to_idx("wstring") > 0);
	CU_ASSERT(tdr_typename_to_idx("wchar") > 0);
	CU_ASSERT(tdr_typename_to_idx("datetime") > 0);
	CU_ASSERT(tdr_typename_to_idx("date") > 0);
	CU_ASSERT(tdr_typename_to_idx("time") > 0);
	CU_ASSERT(tdr_typename_to_idx("ip") > 0);

	tNow = time(NULL);
	CU_ASSERT(tdr_utctime_to_tdrdatetime(&tDatetime, tNow) == 0);
	puts(tdr_tdrdatetime_to_str(&tDatetime));

	CU_ASSERT(0 == tdr_tdrdatetime_to_utctime(&tTmpTime, tDatetime));
	CU_ASSERT(tTmpTime == tNow);

	printf("TDR_TYPE_UNION=%d\n", TDR_TYPE_UNION);
	printf("TDR_TYPE_COMPOSITE=%d\n", TDR_TYPE_COMPOSITE);
	printf("TDR_TYPE_CHAR=%d\n", TDR_TYPE_CHAR);
	printf("TDR_TYPE_UINT=%d\n", TDR_TYPE_UINT);
	printf("TDR_TYPE_VOID=%d\n", TDR_TYPE_VOID);
	printf("TDR_TYPE_MAX=%d\n", TDR_TYPE_MAX);

}

int ut_add_typeinfo_test_suites()
{
	CU_pSuite pSuite = NULL;
	
	//CU_add_suite ����һ��Suite 
	//InitSuite EndSuite:�ֱ��ǲ��Ե�Ԫ��ʼ���ͷź���,�粻��Ҫ��NULL����
	pSuite = CU_add_suite("ut_Test_TypeInfo", NULL, NULL);
	if(NULL == pSuite)
	{//���ע��suite�����
		CU_cleanup_registry();
		return CU_get_error();
	}
	
	//ע�ᵱǰSuite�µĲ���������
	//pSuite:����ָ��
	//"Test1": ���Ե�Ԫ���� 
	//Test1:���Ժ��� 
	if( NULL == CU_add_test(pSuite, "ut_Test_IdxToTypeInfo", (CU_TestFunc)ut_Test_IdxToTypeInfo) )
	{
		CU_cleanup_registry();
		return CU_get_error();
	}
	
	return 0;
}
