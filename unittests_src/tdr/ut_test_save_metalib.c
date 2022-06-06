/**
*
* @file     test_save_metalib.c 
* @brief    ���Ա���metalib�ĺ���
* 
* @author jackyai  
* @version 1.0
* @date 2007-04-04 
*
*
* Copyright (c)  2007, ��Ѷ�Ƽ����޹�˾���������з���
* All rights reserved.
*
*/

#include <stdio.h>
#include <CUnit/CUnit.h>
#include <CUnit/TestDB.h>

#include <assert.h>
#include "tdr/tdr_XMLMetaLib.h"
#include "tdr/tdr_data_io.h"
#include "ut_tdr.h"
#include "tdr/tdr_error.h"

void ut_save_simplemetalib_to_file();
void ut_save_simplemetalibV0_to_file();
void ut_save_fometalib_to_file();

int ut_add_save_metalib_test_suites()
{
	CU_pSuite pSuite = NULL;

	//CU_add_suite ����һ��Suite 
	//InitSuite EndSuite:�ֱ��ǲ��Ե�Ԫ��ʼ���ͷź���,�粻��Ҫ��NULL����
	pSuite = CU_add_suite("TestSaveMetalibSuites", NULL, NULL);
	if(NULL == pSuite)
	{//���ע��suite�����
		CU_cleanup_registry();
		return CU_get_error();
	}

	//ע�ᵱǰSuite�µĲ���������
	//pSuite:����ָ��
	//"Test1": ���Ե�Ԫ���� 
	//Test1:���Ժ��� 

	if( NULL == CU_add_test(pSuite, "ut_save_simplemetalib_to_file", (CU_TestFunc)ut_save_simplemetalib_to_file))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}

	if( NULL == CU_add_test(pSuite, "ut_save_simplemetalibV0_to_file", (CU_TestFunc)ut_save_simplemetalibV0_to_file))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}

	if( NULL == CU_add_test(pSuite, "ut_save_fometalib_to_file", (CU_TestFunc)ut_save_fometalib_to_file))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}


	return 0;
}

void ut_save_simplemetalib_to_file()
{
	int iRet = 0;
	LPTDRMETALIB pstLib = NULL;

	printf("\ntest file: %s\n", TEST_FILE_PATH"testMetalibParam.xml");
	iRet = tdr_create_lib_file(&pstLib, TEST_FILE_PATH"testMetalibParam.xml", 1, stderr);
	CU_ASSERT(0 <= iRet);  

	ut_dump_metalib(pstLib, TEST_FILE_PATH"MetalibParamDump.txt");
	iRet = tdr_save_xml_file(pstLib, TEST_FILE_PATH"NewMetalibParam.xml");
	CU_ASSERT(0 <= iRet);  
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_save_xml_file failed:  %s, ret: %x, ", tdr_error_string(iRet), iRet);
	}
	
	tdr_free_lib(&pstLib);
}

void ut_save_simplemetalibV0_to_file()
{
	int iRet = 0;
	LPTDRMETALIB pstLib = NULL;

	printf("\ntest file: %s\n", TEST_FILE_PATH"testMetalibParamV0.xml");
	iRet = tdr_create_lib_file(&pstLib, TEST_FILE_PATH"testMetalibParamV0.xml", 0, stderr);
	CU_ASSERT(0 <= iRet);  

	ut_dump_metalib(pstLib, TEST_FILE_PATH"MetalibParamDumpV0.txt");
	iRet = tdr_save_xml_file(pstLib, TEST_FILE_PATH"NewMetalibParamV0.xml");
	printf("tdr_save_xml_file ret: %d", iRet);
	tdr_free_lib(&pstLib);
	CU_ASSERT(0 <= iRet);


	iRet = tdr_create_lib_file(&pstLib, TEST_FILE_PATH"NewMetalibParamV0.xml", 0, stderr);
	CU_ASSERT(0 <= iRet);  


	iRet = tdr_save_xml_file(pstLib, TEST_FILE_PATH"MetalibParamV0.xml");
	printf("tdr_save_xml_file ret: %d", iRet);
	tdr_free_lib(&pstLib);
}

void ut_save_fometalib_to_file()
{
	int iRet = 0;
	LPTDRMETALIB pstLib = NULL;


	printf("\ntest file: %s\n", TEST_FILE_PATH"ov_cs.xml");
	iRet = tdr_create_lib_file(&pstLib, TEST_FILE_PATH"ov_cs.xml", 0, stderr);
	CU_ASSERT(0 <= iRet);  

	ut_dump_metalib(pstLib, TEST_FILE_PATH"ov_csMetalibDump.txt");
	iRet = tdr_save_xml_file(pstLib, TEST_FILE_PATH"Newov_cs.xml");
	printf("tdr_save_xml_file ret: %d", iRet);
	tdr_free_lib(&pstLib);

	iRet = tdr_create_lib_file(&pstLib, TEST_FILE_PATH"Newov_cs.xml", 0, stderr);
	CU_ASSERT(0 <= iRet);  

	iRet = tdr_save_xml_file(pstLib, TEST_FILE_PATH"fo2.xml");
	printf("tdr_save_xml_file ret: %d", iRet);
	tdr_free_lib(&pstLib);
	CU_ASSERT(0 <= iRet); 
	
	
}

