/**
*
* @file     test_save_metalib.c 
* @brief    ���Ա���metalib�ĺ���
* 
* @author jackyai  
* @version 1.0
* @date 2007-04-27 
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

#include "tdr/tdr_metalib_manage.h"
#include "tdr_metalib_kernel_i.h"
#include "ut_tdr.h"


void ut_simplemetalib_manage();


int ut_add_metalib_manage_test_suites()
{
	CU_pSuite pSuite = NULL;

	//CU_add_suite ����һ��Suite 
	//InitSuite EndSuite:�ֱ��ǲ��Ե�Ԫ��ʼ���ͷź���,�粻��Ҫ��NULL����
	pSuite = CU_add_suite("TestMetalibManageSuites", NULL, NULL);
	if(NULL == pSuite)
	{//���ע��suite�����
		CU_cleanup_registry();
		return CU_get_error();
	}

	//ע�ᵱǰSuite�µĲ���������
	//pSuite:����ָ��
	//"Test1": ���Ե�Ԫ���� 
	//Test1:���Ժ��� 
	if( NULL == CU_add_test(pSuite, "ut_simplemetalib_manage", (CU_TestFunc)ut_simplemetalib_manage))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}

	
	return 0;
}

void ut_simplemetalib_manage()
{
	int iRet = 0;
	LPTDRMETALIB pstLib = NULL;
	TDRMETA *pstMeta = NULL;
	int idxEntry;
	LPTDRMETAENTRY pstEntry;


	printf("\ntest file: %s to hpp\n", TEST_FILE_PATH"testMetalibParam.xml");
	iRet = tdr_create_lib_file(&pstLib, TEST_FILE_PATH"testMetalibParam.xml", 1, stderr);
	CU_ASSERT(0 <= iRet); 

	pstMeta = tdr_get_meta_by_name(pstLib, "Pkg");
	CU_ASSERT( NULL != pstMeta);

	pstEntry = tdr_get_entry_by_path(pstMeta, "this");
	CU_ASSERT(NULL == pstEntry);
	
	pstEntry = tdr_get_entry_by_path(pstMeta, "this.checksum");
	CU_ASSERT(NULL != pstEntry);
	CU_ASSERT_STRING_EQUAL("checksum", tdr_get_entry_name(pstEntry));
	
	pstEntry = tdr_get_entry_by_path(pstMeta, "checksum");
	CU_ASSERT(NULL != pstEntry);
	CU_ASSERT_STRING_EQUAL("checksum", tdr_get_entry_name(pstEntry));

	pstEntry = tdr_get_entry_by_path(pstMeta, ".checksum");
	CU_ASSERT(NULL == pstEntry);

	pstEntry = tdr_get_entry_by_path(pstMeta, "this.checksum.");
	CU_ASSERT(NULL == pstEntry);

	pstEntry = tdr_get_entry_by_path(pstMeta, "..checksum");
	CU_ASSERT(NULL == pstEntry);

	pstEntry = tdr_get_entry_by_path(pstMeta, "a.checksum");
	CU_ASSERT(NULL == pstEntry);


	pstEntry = tdr_get_entry_by_path(pstMeta, "this.head.bodylen");
	CU_ASSERT(NULL != pstEntry);
	CU_ASSERT_STRING_EQUAL("bodylen", tdr_get_entry_name(pstEntry));

	pstEntry = tdr_get_entry_by_path(pstMeta, "head.bodylen");
	CU_ASSERT(NULL != pstEntry);
	CU_ASSERT_STRING_EQUAL("bodylen", tdr_get_entry_name(pstEntry));
	

	pstMeta = tdr_get_meta_by_name(pstLib, "unkown");
	CU_ASSERT( NULL == pstMeta);

	pstMeta = tdr_get_meta_by_id(pstLib, 2);
	CU_ASSERT( NULL != pstMeta);
	CU_ASSERT_STRING_EQUAL(tdr_get_meta_name(pstMeta), "CmdLogin");

	iRet = tdr_get_entry_by_id(&idxEntry, pstMeta, 6);
	CU_ASSERT(0 <= iRet);
	CU_ASSERT(1 == idxEntry);

	iRet = tdr_get_entry_by_name(&idxEntry, pstMeta, "name");
	CU_ASSERT(0 <= iRet);
	CU_ASSERT(0 == idxEntry);


	pstMeta = tdr_get_meta_by_name(pstLib, "testUnion");
	CU_ASSERT( NULL != pstMeta);
	CU_ASSERT_STRING_EQUAL(tdr_get_meta_name(pstMeta), "testUnion");


	iRet = tdr_get_entry_by_id(&idxEntry, pstMeta, 6);
	CU_ASSERT(0 <= iRet);
	CU_ASSERT((1 <= idxEntry) && (idxEntry <= 2));


	iRet = tdr_get_entry_by_id(&idxEntry, pstMeta, 1000);
	CU_ASSERT(0 <= iRet);
	CU_ASSERT(0 == idxEntry);

	iRet = tdr_get_entry_by_name(&idxEntry, pstMeta, "d");
	CU_ASSERT(0 <= iRet);
	CU_ASSERT(0 == idxEntry);

	iRet = tdr_get_entry_by_name(&idxEntry, pstMeta, "unkown");
	CU_ASSERT(0 > iRet);

	pstMeta = tdr_get_meta_by_name(pstLib, "TestVarArr");
	CU_ASSERT( NULL != pstMeta);
	CU_ASSERT(6 < tdr_get_meta_size(pstMeta));


	pstMeta = tdr_get_meta_by_name(pstLib, "NameID");
	CU_ASSERT( NULL != pstMeta);
	pstEntry = tdr_get_entry_by_path(pstMeta, "Gid");
	CU_ASSERT(NULL != pstEntry);
	CU_ASSERT_STRING_EQUAL(tdr_get_entry_customattr(pstLib, pstEntry),
		"scope[10000000-99999999]")

	tdr_free_lib(&pstLib);
}
