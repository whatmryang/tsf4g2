/**
*
* @file    ut_test_defaultvalue.c 
* @brief   元数据描述库缺省值相关测试
* 
* @author jackyai  
* @version 1.0
* @date 2007-05-15 
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
#include "ut_test_defaultvalue.h"
#include "pal/ttypes.h"


LPTDRMETALIB  g_pstDefaultValueLib = NULL;

static int default_suite_init(void);
static int default_suite_clean(void);
static void ut_metalib_to_hpp();
static void ut_test_unsigned();
static void ut_initialize_data();
static void ut_test_data_io();
static void ut_test_data_net();



int default_suite_init(void)
{
	int iRet;

	iRet = tdr_create_lib_file(&g_pstDefaultValueLib, TEST_FILE_PATH"test_defaultvalue.xml", 1, stderr);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("failed to tdr_create_lib_file: %s\n", tdr_error_string(iRet));
	}else
	{
		ut_dump_metalib(g_pstDefaultValueLib, "test_defaultvalue_metalib_dump.txt");
	}

	if (TDR_ERR_IS_ERROR(iRet))
	{
		return iRet;
	}

	return 0;
}

int default_suite_clean(void)
{
	tdr_free_lib(&g_pstDefaultValueLib);
	return 0;
}



int ut_add_defaultvalue_test_suites()
{
	CU_pSuite pSuite = NULL;

	//CU_add_suite 增加一个Suite 
	//InitSuite EndSuite:分别是测试单元初始和释放函数,如不需要则NULL传递
	pSuite = CU_add_suite("ut_DefaultValue_Test_Suites", default_suite_init, default_suite_clean);
	if(NULL == pSuite)
	{//检测注册suite情况　
		CU_cleanup_registry();
		return CU_get_error();
	}


	if( NULL == CU_add_test(pSuite, "ut_metalib_to_hpp", (CU_TestFunc)ut_metalib_to_hpp))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}
	if( NULL == CU_add_test(pSuite, "ut_initialize_data", (CU_TestFunc)ut_initialize_data))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}
    if( NULL == CU_add_test(pSuite, "ut_test_unsigned", (CU_TestFunc)ut_test_unsigned))
    {
        CU_cleanup_registry();
        return CU_get_error();
    }
	if( NULL == CU_add_test(pSuite, "ut_test_data_io", (CU_TestFunc)ut_test_data_io))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}
	if( NULL == CU_add_test(pSuite, "ut_test_data_net", (CU_TestFunc)ut_test_data_net))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}


	return 0;
}

void ut_metalib_to_hpp()
{	
	int iRet = 0;	
	TDRHPPRULE stRule;

	memset(&stRule, 0, sizeof(stRule));
	stRule.iRule = 0;
	iRet = tdr_metalib_to_hpp(g_pstDefaultValueLib, "ut_test_defaultvalue.h", &stRule);
	CU_ASSERT(0 <= iRet); 
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("failed to tdr_metalib_to_hpp: %s\n", tdr_error_string(iRet));
	}else
	{
		tdr_dump_metalib_file(g_pstDefaultValueLib, "test_defaultvalue.txt");
	}
	
}

void ut_initialize_data()
{
	ALLBASETYPEDATA stTest;
	TDRDATA stHostInfo;
	LPTDRMETA pstMeta;
	int iRet;

	puts("ut_initialize_data");
	pstMeta = tdr_get_meta_by_name(g_pstDefaultValueLib, "AllBaseTypeData");
	CU_ASSERT(NULL != pstMeta);

	stHostInfo.iBuff = sizeof(stTest);
	stHostInfo.pszBuff = (char *)&stTest;
	iRet = tdr_init(pstMeta, &stHostInfo, 0);
	CU_ASSERT(0 <= iRet);

	iRet = tdr_output_file(pstMeta, "inittialize_defaultvalue_data.xml", &stHostInfo, 0, TDR_XML_DATA_FORMAT_LIST_ENTRY_NAME);
	CU_ASSERT(0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_output_file failed: %s", tdr_error_string(iRet));
	}

	iRet = tdr_output_file(pstMeta, "inittialize_defaultvalue_data_attr.xml", &stHostInfo, 0, 3);
	CU_ASSERT(0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_output_file failed: %s", tdr_error_string(iRet));
	}
}

void ut_test_unsigned()
{
	int iRet = 0;
	ALLBASETYPEDATA stTest;
	ALLBASETYPEDATA stTestHex;
	TDRDATA stHostInfo;
	LPTDRMETA pstMeta;

	puts("ut_test_unsigned_value");
	pstMeta = tdr_get_meta_by_name(g_pstDefaultValueLib, "AllBaseTypeData");
	CU_ASSERT(NULL != pstMeta);

	memset(&stTest, 0, sizeof(stTest));
	stHostInfo.iBuff = sizeof(stTest);
	stHostInfo.pszBuff = (char *)&stTest;
	iRet = tdr_input_file(pstMeta, &stHostInfo, "./initialize_with_dec.xml", 0, TDR_XML_DATA_FORMAT_LIST_ENTRY_NAME);
	CU_ASSERT(!TDR_ERR_IS_ERROR(iRet));
	memset(&stTestHex, 0, sizeof(stTestHex));
	stHostInfo.iBuff = sizeof(stTestHex);
	stHostInfo.pszBuff = (char *)&stTestHex;
	iRet = tdr_input_file(pstMeta, &stHostInfo, "./initialize_with_hex.xml", 0, TDR_XML_DATA_FORMAT_LIST_ENTRY_NAME);
	CU_ASSERT(0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_output_file failed: %s", tdr_error_string(iRet));
	}
    CU_ASSERT(0 == memcmp(&stTest, &stTestHex, sizeof(stTest)));


	memset(&stTest, 0, sizeof(stTest));
	stHostInfo.iBuff = sizeof(stTest);
	stHostInfo.pszBuff = (char *)&stTest;
	iRet = tdr_input_file(pstMeta, &stHostInfo, "initialize_with_nothing.xml", 0, TDR_XML_DATA_FORMAT_LIST_ENTRY_NAME);
	CU_ASSERT(!TDR_ERR_IS_ERROR(iRet));
	memset(&stTestHex, 0, sizeof(stTestHex));
	stHostInfo.iBuff = sizeof(stTestHex);
	stHostInfo.pszBuff = (char *)&stTestHex;
	iRet = tdr_input_file(pstMeta, &stHostInfo, "initialize_with_default_dec.xml", 0, TDR_XML_DATA_FORMAT_LIST_ENTRY_NAME);
	CU_ASSERT(!TDR_ERR_IS_ERROR(iRet));
    CU_ASSERT(0 == memcmp(&stTest, &stTestHex, sizeof(stTest)));
    tdr_fprintf(pstMeta, stdout, &stHostInfo, 0);
    iRet = tdr_output_file(pstMeta, "initialize_with_default_dec_out.xml", &stHostInfo,  0, TDR_XML_DATA_FORMAT_LIST_ENTRY_NAME);
	CU_ASSERT(!TDR_ERR_IS_ERROR(iRet));
}

void ut_test_data_io()
{
	ALLBASETYPEDATA stTest;
	TDRDATA stHostInfo;
	LPTDRMETA pstMeta;
	int iRet;
	FILE *fp;
	ALLBASETYPEDATA stTest1;
	ALLBASETYPEDATA stTest2;
	TDRDATA stDst;

	puts("ut_test_data_io");
	pstMeta = tdr_get_meta_by_name(g_pstDefaultValueLib, "AllBaseTypeData");
	CU_ASSERT(NULL != pstMeta);

	memset(&stTest, 0, sizeof(stTest));
	memset(&stTest1, 0, sizeof(stTest1));
	memset(&stTest2, 0, sizeof(stTest2));
	stHostInfo.iBuff = sizeof(stTest);
	stHostInfo.pszBuff = (char *)&stTest;
	iRet = tdr_input_file(pstMeta, &stHostInfo, "inittialize_defaultvalue_data.xml", 0, TDR_XML_DATA_FORMAT_LIST_ENTRY_NAME);
	CU_ASSERT(0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_output_file failed: %s", tdr_error_string(iRet));
	}

	iRet = tdr_output_file(pstMeta, "inittialize_defaultvalue_data1.xml", &stHostInfo, 0, TDR_XML_DATA_FORMAT_LIST_ENTRY_NAME);
	CU_ASSERT(0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_output_file failed: %s", tdr_error_string(iRet));
	}

	fp = fopen("defaultvalue_data_fpritnf.txt", "w");
	if (NULL != fp)
	{
		iRet = tdr_fprintf(pstMeta, fp, &stHostInfo, 0);
		CU_ASSERT(0 <= iRet);
		if (TDR_ERR_IS_ERROR(iRet))
		{
			printf("tdr_output_file failed: %s", tdr_error_string(iRet));
		}
		fclose(fp);
	}

	memcpy(&stTest1, &stTest, sizeof(stTest1));
	stHostInfo.iBuff = sizeof(stTest);
	stHostInfo.pszBuff = (char *)&stTest;
	stDst.iBuff = sizeof(stTest2);
	stDst.pszBuff = (char *)&stTest2;
	iRet = tdr_copy(pstMeta, &stDst, &stHostInfo);
	CU_ASSERT(0 <= iRet);
	CU_ASSERT(0 == memcmp(&stTest2, &stTest1, sizeof(stTest1)));

}

void ut_test_data_net()
{
	ALLBASETYPEDATA stTest;
	TDRDATA stHostInfo;
	LPTDRMETA pstMeta;
	int iRet;
	FILE *fp;
	char szNetDate[32700];
	TDRDATA stNetInfo;
	ALLBASETYPEDATA stNet2Host;

	puts("ut_test_data_net");
	pstMeta = tdr_get_meta_by_name(g_pstDefaultValueLib, "AllBaseTypeData");
	CU_ASSERT(NULL != pstMeta);

	stHostInfo.iBuff = sizeof(stTest);
	stHostInfo.pszBuff = (char *)&stTest;
	memset(&stTest, 0, sizeof(stTest));
	iRet = tdr_input_file(pstMeta, &stHostInfo, "inittialize_defaultvalue_data.xml", 0, TDR_XML_DATA_FORMAT_LIST_ENTRY_NAME);
	CU_ASSERT(0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_output_file failed: %s\n", tdr_error_string(iRet));
	}

	stNetInfo.iBuff = sizeof(szNetDate);
	stNetInfo.pszBuff = &szNetDate[0];
    //stHostInfo.iBuff = offsetof(ALLBASETYPEDATA, iDefaultint);
    stHostInfo.iBuff = sizeof(stTest);
	iRet = tdr_hton(pstMeta, &stNetInfo, &stHostInfo, 0);
    printf("HostInfo.iBuff=%"PRIdPTR"\n", stHostInfo.iBuff);
	CU_ASSERT(0 <= iRet);
	CU_ASSERT(stHostInfo.iBuff == (offsetof(ALLBASETYPEDATA,iDefaultint) + sizeof(int32_t)));
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_hton failed: %s\n", tdr_error_string(iRet));
	}
	CU_ASSERT(stHostInfo.iBuff == (offsetof(ALLBASETYPEDATA,iDefaultint) + sizeof(int32_t)));

	stHostInfo.iBuff = sizeof(stNet2Host);
	stHostInfo.pszBuff = (char *)&stNet2Host;
	memset(&stNet2Host, 0, sizeof(stNet2Host));
	iRet = tdr_ntoh(pstMeta, &stHostInfo,&stNetInfo, 0);
	CU_ASSERT(0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_ntoh failed: %s\n", tdr_error_string(iRet));
	}

	CU_ASSERT(0 == memcmp(&stTest, &stNet2Host, sizeof(ALLBASETYPEDATA)));

	fp = fopen("defaultvalue_data_net_fpritnf.txt", "w");
	if (NULL != fp)
	{
		iRet = tdr_fprintf(pstMeta, fp, &stHostInfo, 0);
		CU_ASSERT(0 <= iRet);
		if (TDR_ERR_IS_ERROR(iRet))
		{
			printf("tdr_output_file failed: %s", tdr_error_string(iRet));
		}
		fclose(fp);
	}

}

