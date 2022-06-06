#include <stdio.h>
#include <CUnit/CUnit.h>
#include <CUnit/TestDB.h>


#include "tdr/tdr_error.h"
#include "tdr_error_i.h"


void ut_Test_TdrErr2Str()
{
    int iErr = 0;

    CU_ASSERT_STRING_EQUAL(tdr_error_string(0), "û�д���");

    tdr_set_last_error(TDR_ERRIMPLE_FAILED_EXPACT_XML);
    iErr = tdr_error_code();

    CU_ASSERT_STRING_EQUAL(tdr_error_string(iErr), "����XML��Ϣ����");
    //puts(tdr_error_string(iErr));

    CU_ASSERT_STRING_EQUAL(tdr_error_string(-1), "δ֪����");
}

int ut_add_tdr_errorhanlder_test_suites()
{
	CU_pSuite pSuite = NULL;
	
	//CU_add_suite ����һ��Suite 
	//InitSuite EndSuite:�ֱ��ǲ��Ե�Ԫ��ʼ���ͷź���,�粻��Ҫ��NULL����
	pSuite = CU_add_suite("ut_Test_ErrorHnadle", NULL, NULL);
	if(NULL == pSuite)
	{//���ע��suite�����
		CU_cleanup_registry();
		return CU_get_error();
	}
	
	//ע�ᵱǰSuite�µĲ���������
	//pSuite:����ָ��
	//"Test1": ���Ե�Ԫ���� 
	//Test1:���Ժ��� 
	if( NULL == CU_add_test(pSuite, "ut_Test_TdrErr2Str", (CU_TestFunc)ut_Test_TdrErr2Str) )
	{
		CU_cleanup_registry();
		return CU_get_error();
	}
	
	return 0;
}
