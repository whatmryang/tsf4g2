/**
*
* @file     test_data_initialize.c 
* @brief    �������ݳ�ʼ��
* 
* @author jackyai  
* @version 1.0
* @date 2007-06-05 
*
*
* Copyright (c)  2007, ��Ѷ�Ƽ����޹�˾���������з���
* All rights reserved.
*
*/


#include <stdio.h>
#include <CUnit/CUnit.h>
#include <CUnit/TestDB.h>

#include <math.h>
#include <assert.h>



#include "tdr/tdr_error.h"
#include "tdr/tdr_operate_data.h"
#include "tdr_metalib_kernel_i.h"
#include "ut_tdr.h"
#include "simpleMetalibV1.h"


void ut_test_init_simple_metalib_data();


int ut_add_data_initialize_test_suites()
{
	CU_pSuite pSuite = NULL;

	//CU_add_suite ����һ��Suite 
	//InitSuite EndSuite:�ֱ��ǲ��Ե�Ԫ��ʼ���ͷź���,�粻��Ҫ��NULL����
	pSuite = CU_add_suite("ut_Data_Initialize_Test_Suites", NULL, NULL);
	if(NULL == pSuite)
	{//���ע��suite�����
		CU_cleanup_registry();
		return CU_get_error();
	}

	if( NULL == CU_add_test(pSuite, "ut_test_init_simple_metalib_data", (CU_TestFunc)ut_test_init_simple_metalib_data))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}
	

	return 0;
}

void ut_test_init_simple_metalib_data()
{
	int iRet = 0;
	LPTDRMETALIB pstLib = NULL;
	LPTDRMETA pstMeta = NULL;
	PKG stPkg;
	TDRDATA stHostInfo;
	

	iRet = tdr_create_lib_file(&pstLib, TEST_FILE_PATH"testMetalibParam.xml", 1, stderr);
	CU_ASSERT(0 <= iRet); 

	pstMeta = tdr_get_meta_by_name(pstLib, "Pkg");
	CU_ASSERT( NULL != pstMeta);

	
	stPkg.stHead.nVersion = 9;
	stHostInfo.iBuff = sizeof(stPkg);
	stHostInfo.pszBuff = (char *)&stPkg;
	
	iRet = tdr_init(pstMeta, &stHostInfo, pstMeta->iBaseVersion + 10);
	CU_ASSERT(0 <= iRet);

	iRet = tdr_output_file(pstMeta, "inittialize_pkg.xml", &stHostInfo, pstMeta->iBaseVersion + 10, 1);
	CU_ASSERT(0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_output_file failed ,%s", tdr_error_string(iRet));
	}

	iRet = tdr_output_file(pstMeta, "inittialize_pkg_attr.xml", &stHostInfo, pstMeta->iBaseVersion + 10, 3);
	CU_ASSERT(0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_output_file failed ,%s", tdr_error_string(iRet));
	}

	tdr_free_lib(&pstLib);
}

