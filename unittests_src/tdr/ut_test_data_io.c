/**
*
* @file     test_date_io.c
* @brief    测试数据输入/输出
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
#include <stdlib.h>
#include <CUnit/CUnit.h>
#include <CUnit/TestDB.h>

#include <math.h>
#include <assert.h>
#include <time.h>

#define MAX_META_VERSION	100

#include "pal/ttypes.h"
#include "tdr/tdr.h"
#include "ut_tdr.h"
#include "simpleMetalibV1.h"
#include "tdr_os.h"

LPTDRMETALIB g_pstMetaLib = NULL;

static int ut_date_sort_suite_init();
static int ut_date_sort_suite_clean();
static void ut_simplemetalib_io();
static void ut_tdr_assert_package_equal(PKG *pstInPutPkg, PKG *pstPkg);
static void ut_tdr_assert_testIO_package_equal(TESTIO *pstInPutIO, TESTIO *pstIO);
static void ut_tdr_test_ioprintf();
static void ut_test_input1();
static void ut_test_input2();
static void ut_test_input3();
static void ut_test_input4();
static void ut_test_input5();
static void ut_brower_data();
static int enter_meta(IN void *a_pHandle, IN LPTDRMETAENTRY a_pstChildEntry, IN int a_iTotal, IN int a_Index,  OUT void ** a_ppHandle);
static int vist_entry(IN void *a_pHandle, IN LPTDRMETAENTRY a_pstEntry, IN int a_iCount,
					  IN char *a_pszHostStart, IN int a_iSize);
static void ut_xmlesc();

static void ut_attr_xml();

static void ut_simplemetalib_io_attr();

int ut_add_dataio_test_suites()
{
	CU_pSuite pSuite = NULL;

	//CU_add_suite 增加一个Suite 
	//InitSuite EndSuite:分别是测试单元初始和释放函数,如不需要则NULL传递
	pSuite = CU_add_suite("ut_dataio_test_suites", (CU_InitializeFunc)ut_date_sort_suite_init, (CU_CleanupFunc)ut_date_sort_suite_clean);
	if(NULL == pSuite)
	{//检测注册suite情况　
		CU_cleanup_registry();
		return CU_get_error();
	}

	//注册当前Suite下的测试用例　
	//pSuite:用例指针
	//"Test1": 测试单元名称 
	//Test1:测试函数 
	if( NULL == CU_add_test(pSuite, "ut_simplemetalib_io", (CU_TestFunc)ut_simplemetalib_io))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}
	
	if( NULL == CU_add_test(pSuite, "ut_tdr_test_ioprintf", (CU_TestFunc)ut_tdr_test_ioprintf))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}

	if( NULL == CU_add_test(pSuite, "ut_test_input1", (CU_TestFunc)ut_test_input1))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}
	if( NULL == CU_add_test(pSuite, "ut_test_input2", (CU_TestFunc)ut_test_input2))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}
	if( NULL == CU_add_test(pSuite, "ut_test_input3", (CU_TestFunc)ut_test_input3))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}
	if( NULL == CU_add_test(pSuite, "ut_test_input4", (CU_TestFunc)ut_test_input4))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}
	if( NULL == CU_add_test(pSuite, "ut_test_input5", (CU_TestFunc)ut_test_input5))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}
	
	if( NULL == CU_add_test(pSuite, "ut_brower_data", (CU_TestFunc)ut_brower_data))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}

	if( NULL == CU_add_test(pSuite, "ut_xmlesc", (CU_TestFunc)ut_xmlesc))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}

	if( NULL == CU_add_test(pSuite, "ut_attr_xml", (CU_TestFunc)ut_attr_xml))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}

	if( NULL == CU_add_test(pSuite, "ut_simplemetalib_io_attr", (CU_TestFunc)ut_simplemetalib_io_attr))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}

	return 0;
}

int ut_date_sort_suite_init()
{
	int iRet ;

	srand((unsigned int)time(NULL));
	iRet = tdr_create_lib_file(&g_pstMetaLib, TEST_FILE_PATH"testMetalibParam.xml", 1, stderr);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("g_pstDateSortMetaLib failed : %s\n", tdr_error_string((iRet)));
		return iRet;
	}else
	{
		ut_dump_metalib(g_pstMetaLib, "testMetalibParam_lib.txt");
	}

	return 0;	
}

int ut_date_sort_suite_clean()
{
	tdr_free_lib(&g_pstMetaLib);
	return 0;
}

void ut_simplemetalib_io()
{
	int iRet = 0;
	LPTDRMETALIB pstLib = NULL;
	LPTDRMETA pstMeta = NULL;
	PKG stPkg;
	char szXml[TDR_MAX_NET_PACKAGE_LEN];
	TDRDATA stHostInfo;
	TDRDATA stXmlInfo;
	FILE *fp;
	PKG stInPutPkg;
	TESTIO stIO;
	TESTIO stInPutIO;

	iRet = tdr_create_lib_file(&pstLib, TEST_FILE_PATH"testMetalibParam.xml", 1, stderr);
	CU_ASSERT(0 <= iRet); 

	pstMeta = tdr_get_meta_by_name(pstLib, "Pkg");
	CU_ASSERT( NULL != pstMeta);

	memset(&stPkg, 0, sizeof(stPkg));
	stPkg.stHead.dwMsgid = 100;
	stPkg.stHead.nCmd = 0;
	stPkg.stHead.nVersion = 9;
	tdr_str_to_tdrdatetime(&stPkg.stHead.tTime, "2007-05-12 12:34:36");
	stPkg.stHead.nMagic = 0x3344;
	stPkg.nBodycount = 2;
	
	strncpy(stPkg.stBody.astLogin[0].szName, "haha", sizeof(stPkg.stBody.astLogin[0].szName));
	strncpy(stPkg.stBody.astLogin[0].szPass, "secret", sizeof(stPkg.stBody.astLogin[0].szPass));
	strncpy(stPkg.stBody.astLogin[0].aszZone[0], "shenzhen", sizeof(stPkg.stBody.astLogin[0].aszZone[0]));
	strncpy(stPkg.stBody.astLogin[0].aszZone[1], "beijing", sizeof(stPkg.stBody.astLogin[0].aszZone[0]));

	strncpy(stPkg.stBody.astLogin[1].szName, "hihi", sizeof(stPkg.stBody.astLogin[1].szName));
	strncpy(stPkg.stBody.astLogin[1].szPass, "secret1", sizeof(stPkg.stBody.astLogin[1].szPass));
	strncpy(stPkg.stBody.astLogin[1].aszZone[0], "hunan", sizeof(stPkg.stBody.astLogin[1].aszZone[0]));
	strncpy(stPkg.stBody.astLogin[1].aszZone[1], "hubei", sizeof(stPkg.stBody.astLogin[1].aszZone[0]));


	stPkg.nBody2num = 2;
	stPkg.astBody2[0].a = 0x56;
	stPkg.astBody2[0].nNamenum = 2;
	strncpy(stPkg.astBody2[0].aszName[0], "zhangsan", sizeof(stPkg.astBody2[0].aszName[0]));
	strncpy(stPkg.astBody2[0].aszName[1], "lisi", sizeof(stPkg.astBody2[0].aszName[1]));
	stPkg.astBody2[0].fMagic = (float)108.88;
	
	stPkg.astBody2[1].a = 0x1234;
	stPkg.astBody2[1].nNamenum = 1;
	strncpy(stPkg.astBody2[1].aszName[0], "wangwu", sizeof(stPkg.astBody2[1].aszName[0]));
	stPkg.astBody2[1].fMagic = (float)1586.78;

	stPkg.iChecksum = 0x12345678;

	stPkg.nStatusnum = 2;
	stPkg.astStatus[0].nType = 2;
	stPkg.astStatus[0].stValue.stFunny.a = 66;
	stPkg.astStatus[0].stValue.stFunny.b = 45;
	stPkg.astStatus[0].stValue.stFunny.c = 456;
	stPkg.astStatus[1].nType = 1;
	stPkg.astStatus[1].stValue.iDummy = 1234;

	stPkg.stNtstatus.nType = 1;
	stPkg.stNtstatus.stValue.iDummy = 5678;

	
	stHostInfo.iBuff = sizeof(stPkg);
	stHostInfo.pszBuff = (char *)&stPkg;
	fp = fopen("stPkg.txt" ,"w");
	if (NULL != fp)
	{
		tdr_fprintf(pstMeta, fp, &stHostInfo, 0);
		fclose(fp);
		//CU_ASSERT(stHostInfo.iBuff == (int)sizeof(stNetPkg));
	}

	stHostInfo.iBuff = sizeof(stPkg);
	stHostInfo.pszBuff = (char *)&stPkg;
	stXmlInfo.iBuff = sizeof(szXml);
	stXmlInfo.pszBuff = &szXml[0];
	iRet = tdr_output(pstMeta, &stXmlInfo, &stHostInfo, MAX_META_VERSION, 1);
	CU_ASSERT(0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("%s\n", tdr_error_string(iRet));
	}else
	{
		fp = fopen("stPkg.xml", "w");
		if (NULL != fp)
		{
			fwrite(stXmlInfo.pszBuff, 1, stXmlInfo.iBuff, fp);
			fclose(fp);
		}

		memset(&stInPutPkg, 0, sizeof(stInPutPkg));
		stHostInfo.iBuff = sizeof(stInPutPkg);
		stHostInfo.pszBuff = (char *)&stInPutPkg;
		iRet = tdr_input(pstMeta, &stHostInfo, &stXmlInfo, MAX_META_VERSION, 1);
		ut_tdr_assert_package_equal(&stInPutPkg, &stPkg);
		CU_ASSERT(0 <= iRet);
	}	

	/*test tdr_output_fp*/
	//stHostInfo.iBuff = sizeof(stPkg);
	//stHostInfo.pszBuff = (char *)&stPkg;
	fp = fopen("stPkg_fp.xml", "w");
	if (NULL != fp)
	{
		iRet = tdr_output_fp(pstMeta, fp, &stHostInfo, MAX_META_VERSION, 2);
		CU_ASSERT(0 <= iRet);
		fclose(fp);

		fp = fopen("stPkg_fp.xml", "r");
		if (NULL != fp)
		{
			memset(&stInPutPkg, 0, sizeof(stInPutPkg));
			stHostInfo.iBuff = sizeof(stInPutPkg);
			stHostInfo.pszBuff = (char *)&stInPutPkg;
			iRet = tdr_input_fp(pstMeta, &stHostInfo, fp, MAX_META_VERSION, 2);
			ut_tdr_assert_package_equal(&stInPutPkg, &stPkg);
			CU_ASSERT(0 <= iRet);

			fclose(fp);
		}		
	}

	/*test tdr_output_file*/
	//stHostInfo.iBuff = sizeof(stPkg);
	//stHostInfo.pszBuff = (char *)&stPkg;
	iRet = tdr_output_file(pstMeta, "stPkg_file.xml", &stHostInfo, 0, 2);
	CU_ASSERT(0 <= iRet);


	memset(&stInPutPkg, 0, sizeof(stInPutPkg));
	stHostInfo.iBuff = sizeof(stInPutPkg);
	stHostInfo.pszBuff = (char *)&stInPutPkg;
	iRet = tdr_input_file(pstMeta, &stHostInfo, "stPkg_file.xml", 0, 2);
	CU_ASSERT(0 <= iRet);

	pstMeta = tdr_get_meta_by_name(pstLib, "TestIO");
	CU_ASSERT( NULL != pstMeta);
	stIO.stIostatus.nType = 1;
	stIO.stIostatus.stValue.iDummy = 5678;
	stIO.stNtstatus.nType = 2;
	stIO.stNtstatus.stValue.stFunny.a = 1234;
	stIO.stNtstatus.stValue.stFunny.b = 34;
	stIO.stNtstatus.stValue.stFunny.c = 454;
	stHostInfo.iBuff = sizeof(stIO);
	stHostInfo.pszBuff = (char *)&stIO;
	stXmlInfo.iBuff = sizeof(szXml);
	stXmlInfo.pszBuff = &szXml[0];
	iRet = tdr_output(pstMeta, &stXmlInfo, &stHostInfo, tdr_get_meta_current_version(pstMeta), 1);
	CU_ASSERT(0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("%s\n", tdr_error_string(iRet));
	}else
	{
		fp = fopen("TestIO.xml", "w");
		if (NULL != fp)
		{
			fwrite(stXmlInfo.pszBuff, 1, stXmlInfo.iBuff, fp);
			fclose(fp);
		}

		memset(&stInPutIO, 0, sizeof(stInPutIO));
		stHostInfo.iBuff = sizeof(stInPutIO);
		stHostInfo.pszBuff = (char *)&stInPutIO;
		iRet = tdr_input(pstMeta, &stHostInfo, &stXmlInfo, tdr_get_meta_current_version(pstMeta), 1);
		CU_ASSERT(0 <= iRet);
		ut_tdr_assert_testIO_package_equal(&stInPutIO, &stIO);
	}	


	tdr_free_lib(&pstLib);
}

void ut_tdr_assert_testIO_package_equal(TESTIO *pstInPutIO, TESTIO *pstIO)
{
	CU_ASSERT(pstInPutIO->stNtstatus.nType == pstIO->stNtstatus.nType);
	switch(pstInPutIO->stNtstatus.nType)
	{
	case 1:
		{
			CU_ASSERT(pstInPutIO->stNtstatus.stValue.iDummy == pstIO->stNtstatus.stValue.iDummy);
		}
		break;
	case 2:
		{
			CU_ASSERT(pstInPutIO->stNtstatus.stValue.stFunny.a == pstIO->stNtstatus.stValue.stFunny.a);
			CU_ASSERT(pstInPutIO->stNtstatus.stValue.stFunny.b == pstIO->stNtstatus.stValue.stFunny.b);
			CU_ASSERT(pstInPutIO->stNtstatus.stValue.stFunny.c == pstIO->stNtstatus.stValue.stFunny.c);
		}
		break;
	default:
		break;
	}
	CU_ASSERT(pstInPutIO->stIostatus.nType == pstIO->stIostatus.nType);
	switch(pstInPutIO->stIostatus.nType)
	{
	case 1:
		{
			CU_ASSERT(pstInPutIO->stIostatus.stValue.iDummy == pstIO->stIostatus.stValue.iDummy);
		}
		break;
	case 2:
		{
			CU_ASSERT(pstInPutIO->stIostatus.stValue.stFunny.a == pstIO->stIostatus.stValue.stFunny.a);
			CU_ASSERT(pstInPutIO->stIostatus.stValue.stFunny.b == pstIO->stIostatus.stValue.stFunny.b);
			CU_ASSERT(pstInPutIO->stIostatus.stValue.stFunny.c == pstIO->stIostatus.stValue.stFunny.c);
		}
		break;
	default:
		break;
	}
}


void ut_tdr_assert_package_equal(PKG *pstInPutPkg, PKG *pstPkg)
{
	int i;

	CU_ASSERT(pstPkg->stHead.dwMsgid == pstInPutPkg->stHead.dwMsgid);
	CU_ASSERT(pstPkg->stHead.nBodylen == pstInPutPkg->stHead.nBodylen);
	CU_ASSERT(pstPkg->stHead.nCmd == pstInPutPkg->stHead.nCmd);
	CU_ASSERT(pstPkg->stHead.nMagic == pstInPutPkg->stHead.nMagic);
	CU_ASSERT(pstPkg->stHead.nVersion == pstInPutPkg->stHead.nVersion);
	//CU_ASSERT(pstPkg->stHead.tTime == pstInPutPkg->stHead.tTime);

	CU_ASSERT(pstPkg->nBodycount == pstInPutPkg->nBodycount);


	switch(pstPkg->stHead.nCmd)
	{
	case 0:
		{
			for (i = 0; i < 2; i++)
			{
				CU_ASSERT_STRING_EQUAL(pstPkg->stBody.astLogin[i].szName, pstInPutPkg->stBody.astLogin[i].szName);
				CU_ASSERT_STRING_EQUAL(pstPkg->stBody.astLogin[i].szPass, pstInPutPkg->stBody.astLogin[i].szPass);
				CU_ASSERT_STRING_EQUAL(pstPkg->stBody.astLogin[i].aszZone[0], pstInPutPkg->stBody.astLogin[i].aszZone[0]);
				CU_ASSERT_STRING_EQUAL(pstPkg->stBody.astLogin[i].aszZone[1], pstInPutPkg->stBody.astLogin[i].aszZone[1]);
			}
		}
		break;
	case 1:
		{
			CU_ASSERT(pstInPutPkg->stBody.stLogout.iReason == pstPkg->stBody.stLogout.iReason);
			CU_ASSERT_STRING_EQUAL(pstPkg->stBody.stLogout.szAttr, pstInPutPkg->stBody.stLogout.szAttr);
		}
		break;
	default:
		break;
	}

	for (i = 0; i < 2; i++)
	{
		int j;

		CU_ASSERT(pstInPutPkg->astBody2[i].a == pstPkg->astBody2[i].a);
		CU_ASSERT(pstInPutPkg->astBody2[i].chLen == pstPkg->astBody2[i].chLen);
		CU_ASSERT(pstInPutPkg->astBody2[i].nNamenum == pstPkg->astBody2[i].nNamenum);
		for (j = 0; j < 4; j++)
		{
			CU_ASSERT_STRING_EQUAL(pstInPutPkg->astBody2[i].aszName[j], pstPkg->astBody2[i].aszName[j]);
		}
		CU_ASSERT( fabs(pstInPutPkg->astBody2[i].fMagic - pstPkg->astBody2[i].fMagic) < 0.00001);
	}

	CU_ASSERT(pstInPutPkg->iChecksum == pstPkg->iChecksum);

	for (i = 0; i < 2; i++)
	{
		CU_ASSERT(pstInPutPkg->astStatus[i].nType == pstPkg->astStatus[i].nType);
		switch(pstInPutPkg->astStatus[i].nType)
		{
		case 1:
			{
				CU_ASSERT(pstInPutPkg->astStatus[i].stValue.iDummy == pstPkg->astStatus[i].stValue.iDummy);
			}
			break;
		case 2:
			{
				CU_ASSERT(pstInPutPkg->astStatus[i].stValue.stFunny.a == pstPkg->astStatus[i].stValue.stFunny.a);
				CU_ASSERT(pstInPutPkg->astStatus[i].stValue.stFunny.b == pstPkg->astStatus[i].stValue.stFunny.b);
				CU_ASSERT(pstInPutPkg->astStatus[i].stValue.stFunny.c == pstPkg->astStatus[i].stValue.stFunny.c);
			}
			break;
		default:
			break;
		}
	}

	/*
	CU_ASSERT(pstInPutPkg->stNtstatus.nType == pstPkg->stNtstatus.nType);
	switch(pstInPutPkg->stNtstatus.nType)
	{
	case 1:
		{
			CU_ASSERT(pstInPutPkg->stNtstatus.stValue.iDummy == pstPkg->stNtstatus.stValue.iDummy);
		}
		break;
	case 2:
		{
			CU_ASSERT(pstInPutPkg->stNtstatus.stValue.stFunny.a == pstPkg->stNtstatus.stValue.stFunny.a);
			CU_ASSERT(pstInPutPkg->stNtstatus.stValue.stFunny.b == pstPkg->stNtstatus.stValue.stFunny.b);
			CU_ASSERT(pstInPutPkg->stNtstatus.stValue.stFunny.c == pstPkg->stNtstatus.stValue.stFunny.c);
		}
		break;
	default:
		break;
	}
	*/
		
}

void ut_tdr_test_ioprintf()
{
	int iRet = 0;
	LPTDRMETALIB pstLib = NULL;
	LPTDRMETA pstMeta = NULL;
	PKG stPkg;
	TDRDATA stHostInfo;	
	FILE *fp;
    TDRPRINTFORMAT stFormat;
    char szBuff[4096];
    TDRDATA stOutInfo;	

	iRet = tdr_create_lib_file(&pstLib, TEST_FILE_PATH"testMetalibParam.xml", 1, stderr);
	CU_ASSERT(0 <= iRet); 

	pstMeta = tdr_get_meta_by_name(pstLib, "Pkg");
	CU_ASSERT( NULL != pstMeta);

	memset(&stPkg, 0, sizeof(stPkg));
	stPkg.stHead.dwMsgid = 100;
	stPkg.stHead.nCmd = 0;
	stPkg.stHead.nVersion = 9;
	tdr_str_to_tdrdatetime(&stPkg.stHead.tTime, "2007-05-23 23:34:35");
	stPkg.stHead.nMagic = 0x3344;
	stPkg.nBodycount = 2;

	strncpy(stPkg.stBody.astLogin[0].szName, "haha", sizeof(stPkg.stBody.astLogin[0].szName));
	strncpy(stPkg.stBody.astLogin[0].szPass, "secret", sizeof(stPkg.stBody.astLogin[0].szPass));
	strncpy(stPkg.stBody.astLogin[0].aszZone[0], "shenzhen", sizeof(stPkg.stBody.astLogin[0].aszZone[0]));
	strncpy(stPkg.stBody.astLogin[0].aszZone[1], "beijing", sizeof(stPkg.stBody.astLogin[0].aszZone[0]));

	strncpy(stPkg.stBody.astLogin[1].szName, "hihi", sizeof(stPkg.stBody.astLogin[1].szName));
	strncpy(stPkg.stBody.astLogin[1].szPass, "secret1", sizeof(stPkg.stBody.astLogin[1].szPass));
	strncpy(stPkg.stBody.astLogin[1].aszZone[0], "hunan", sizeof(stPkg.stBody.astLogin[1].aszZone[0]));
	strncpy(stPkg.stBody.astLogin[1].aszZone[1], "hubei", sizeof(stPkg.stBody.astLogin[1].aszZone[0]));


	stPkg.astBody2[0].a = 0x56;
	stPkg.astBody2[0].nNamenum = 2;
	strncpy(stPkg.astBody2[0].aszName[0], "zhangsan", sizeof(stPkg.astBody2[0].aszName[0]));
	strncpy(stPkg.astBody2[0].aszName[1], "lisi", sizeof(stPkg.astBody2[0].aszName[1]));
	stPkg.astBody2[0].fMagic = (float)108.88;

	stPkg.astBody2[1].a = 0x1234;
	stPkg.astBody2[1].nNamenum = 1;
	strncpy(stPkg.astBody2[1].aszName[0], "wangwu", sizeof(stPkg.astBody2[1].aszName[0]));
	stPkg.astBody2[1].fMagic = (float)1586.78;

	stPkg.iChecksum = 0x12345678;

	stPkg.astStatus[0].nType = 2;
	stPkg.astStatus[0].stValue.stFunny.a = 66;
	stPkg.astStatus[0].stValue.stFunny.b = 45;
	stPkg.astStatus[0].stValue.stFunny.c = 456;
	stPkg.astStatus[1].nType = 1;
	stPkg.astStatus[1].stValue.iDummy = 1234;

	stPkg.stNtstatus.nType = 1;
	stPkg.stNtstatus.stValue.iDummy = 5678;

	stHostInfo.iBuff = sizeof(stPkg);
	stHostInfo.pszBuff = (char *)&stPkg;

	fp = fopen(TEST_FILE_PATH"stPkg_ioprintf.txt", "w");
	assert(NULL != fp);
	if (NULL == fp)
	{
		return;
	}

	iRet = tdr_fprintf(pstMeta, fp, &stHostInfo, tdr_get_meta_current_version(pstMeta));
	CU_ASSERT(0 <= iRet);	

    stHostInfo.iBuff = sizeof(stPkg);
    stHostInfo.pszBuff = (char *)&stPkg;
	
	tdr_get_default_format(&stFormat);
    stFormat.chIndentChar =0;
    stFormat.pszSepStr = "|";
    iRet = tdr_fprintf_ex(pstMeta, fp, &stHostInfo, tdr_get_meta_current_version(pstMeta),&stFormat);
    CU_ASSERT(0 <= iRet);	

    stHostInfo.iBuff = sizeof(stPkg);
    stHostInfo.pszBuff = (char *)&stPkg;
    stOutInfo.iBuff = sizeof(szBuff);
    stOutInfo.pszBuff = (char *)szBuff;
    tdr_sprintf_ex(pstMeta, &stOutInfo, &stHostInfo, tdr_get_meta_current_version(pstMeta),&stFormat);
    CU_ASSERT(0 <= iRet);

    fprintf(fp, "\ntest:\n%s\n",szBuff);

	fclose(fp);
}

void ut_test_input1()
{
	int iRet = 0;
	LPTDRMETA pstMeta = NULL;
	PKG stPkg;
	TDRDATA stHostInfo;	

	pstMeta = tdr_get_meta_by_name(g_pstMetaLib, "Pkg");
	CU_ASSERT( NULL != pstMeta);
	if (NULL == pstMeta)
	{
		return;
	}

	memset(&stPkg, 0, sizeof(stPkg));
	stHostInfo.iBuff = (int)sizeof(stPkg);
	stHostInfo.pszBuff = (char *)&stPkg;

	iRet = tdr_input_file(pstMeta, &stHostInfo, TEST_FILE_PATH"test_input_stPkg.xml", tdr_get_meta_current_version(pstMeta),
		TDR_XML_DATA_FORMAT_LIST_MATA_NAME);
	CU_ASSERT(0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("failed to tdr_input_file: %s\n", tdr_error_string(iRet));
	}


	iRet = tdr_output_file(pstMeta, "test_output_stPkg.xml", &stHostInfo, tdr_get_meta_current_version(pstMeta),
		TDR_XML_DATA_FORMAT_LIST_MATA_NAME);
	CU_ASSERT(0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("failed to tdr_input_file%s: %s\n", "test_output_stPkg.xml", tdr_error_string(iRet));
	}
}


void ut_test_input2()
{
	int iRet = 0;
	LPTDRMETA pstMeta = NULL;
	PKG stPkg;
	TDRDATA stHostInfo;	

	pstMeta = tdr_get_meta_by_name(g_pstMetaLib, "Pkg");
	CU_ASSERT( NULL != pstMeta);
	if (NULL == pstMeta)
	{
		return;
	}

	memset(&stPkg, 0, sizeof(stPkg));
	stHostInfo.iBuff = (int)sizeof(stPkg);
	stHostInfo.pszBuff = (char *)&stPkg;

	iRet = tdr_input_file(pstMeta, &stHostInfo, TEST_FILE_PATH"test_input_stPkg1.xml", tdr_get_meta_current_version(pstMeta),
		TDR_XML_DATA_FORMAT_LIST_ENTRY_NAME);
	CU_ASSERT(0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("failed to tdr_input_file: %s\n", tdr_error_string(iRet));
	}

	iRet = tdr_output_file(pstMeta, "test_output_stPkg1.xml", &stHostInfo, tdr_get_meta_current_version(pstMeta),
		TDR_XML_DATA_FORMAT_LIST_ENTRY_NAME);
	CU_ASSERT(0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("failed to tdr_input_file: %s\n", tdr_error_string(iRet));
	}
}

void ut_test_input3()
{
	int iRet = 0;
	LPTDRMETA pstMeta = NULL;
	TESTCSSTATUS stPkg;
	TDRDATA stHostInfo;	

	pstMeta = tdr_get_meta_by_name(g_pstMetaLib, "TestCSStatus");
	CU_ASSERT( NULL != pstMeta);
	if (NULL == pstMeta)
	{
		return;
	}

	memset(&stPkg, 0, sizeof(stPkg));
	stHostInfo.iBuff = (int)sizeof(stPkg);
	stHostInfo.pszBuff = (char *)&stPkg;

	iRet = tdr_input_file(pstMeta, &stHostInfo, TEST_FILE_PATH"test_input_csvalues.xml", tdr_get_meta_current_version(pstMeta),
		TDR_XML_DATA_FORMAT_LIST_MATA_NAME);
	CU_ASSERT(0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("failed to tdr_input_file: %s\n", tdr_error_string(iRet));
	}


	iRet = tdr_output_file(pstMeta, "test_output_csvalues.xml", &stHostInfo, tdr_get_meta_current_version(pstMeta),
		TDR_XML_DATA_FORMAT_LIST_MATA_NAME);
	CU_ASSERT(0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("failed to tdr_input_file: %s\n", tdr_error_string(iRet));
	}

	iRet = tdr_output_file(pstMeta, "test_output_csvalues_attr.xml", &stHostInfo, tdr_get_meta_current_version(pstMeta),
		3);
	CU_ASSERT(0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("failed to tdr_input_file: %s\n", tdr_error_string(iRet));
	}
}


void ut_test_input4()
{
	int iRet = 0;
	LPTDRMETA pstMeta = NULL;
	TESTCSSTATUS stPkg;
	TDRDATA stHostInfo;	

	pstMeta = tdr_get_meta_by_name(g_pstMetaLib, "TestCSStatus");
	CU_ASSERT( NULL != pstMeta);
	if (NULL == pstMeta)
	{
		return;
	}

	memset(&stPkg, 0, sizeof(stPkg));
	stHostInfo.iBuff = (int)sizeof(stPkg);
	stHostInfo.pszBuff = (char *)&stPkg;

	iRet = tdr_input_file(pstMeta, &stHostInfo, TEST_FILE_PATH"test_input_csvalues1.xml", tdr_get_meta_current_version(pstMeta),
		TDR_XML_DATA_FORMAT_LIST_ENTRY_NAME);
	CU_ASSERT(0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("failed to tdr_input_file: %s\n", tdr_error_string(iRet));
	}

	iRet = tdr_output_file(pstMeta, "test_output_csvalues1.xml", &stHostInfo, tdr_get_meta_current_version(pstMeta),
		TDR_XML_DATA_FORMAT_LIST_ENTRY_NAME);
	CU_ASSERT(0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("failed to tdr_input_file: %s\n", tdr_error_string(iRet));
	}

	iRet = tdr_output_file(pstMeta, "test_output_csvalues1_attr.xml", &stHostInfo, tdr_get_meta_current_version(pstMeta),
		3);
	CU_ASSERT(0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("failed to tdr_input_file: %s\n", tdr_error_string(iRet));
	}
}


void ut_test_input5()
{
#define  MAX_DATA_NUM	5

	int iRet = 0;
	char szSrcData[sizeof(TESTUNSIZEDARR) + sizeof(TESTALIGNA)*MAX_DATA_NUM];
	char szInputData[sizeof(TESTUNSIZEDARR) + MAX_DATA_NUM *sizeof(TESTALIGNA)];
	LPTDRMETA pstMeta = NULL;
	TESTUNSIZEDARR *pstData;
	LPTESTALIGNA  pstAligna;
	TDRDATA stHostInfo;	
	int i;

	pstMeta = tdr_get_meta_by_name(g_pstMetaLib, "TestUnsizedArr");
	CU_ASSERT( NULL != pstMeta);
	if (NULL == pstMeta)
	{
		return;
	}

	memset(&szSrcData, 0, sizeof(szSrcData));
	memset(&szInputData, 0, sizeof(szInputData));
	pstData = (LPTESTUNSIZEDARR)&szSrcData[0];
	pstData->iCount = MAX_DATA_NUM;
	for (i = 0; i < MAX_DATA_NUM; i++)
	{
		pstAligna = &pstData->astData[i];
		pstAligna->a = i;
		pstAligna->b = 'A' + i;
		pstAligna->c = -i;
	}

	stHostInfo.iBuff = (int)sizeof(szSrcData);
	stHostInfo.pszBuff = (char *)&szSrcData[0];
	iRet = tdr_output_file(pstMeta, "test_output_TestUnsizedArr.xml", &stHostInfo, tdr_get_meta_current_version(pstMeta),
		TDR_XML_DATA_FORMAT_LIST_ENTRY_NAME);
	CU_ASSERT(0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("failed to tdr_input_file: %s\n", tdr_error_string(iRet));
	}

	iRet = tdr_output_file(pstMeta, "test_output_TestUnsizedArr_attr.xml", &stHostInfo, tdr_get_meta_current_version(pstMeta),
		3);
	CU_ASSERT(0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("failed to tdr_input_file: %s\n", tdr_error_string(iRet));
	}

	stHostInfo.iBuff = (int)sizeof(szInputData);
	stHostInfo.pszBuff = (char *)&szInputData[0];
	iRet = tdr_input_file(pstMeta, &stHostInfo, "test_output_TestUnsizedArr.xml", tdr_get_meta_current_version(pstMeta),
		TDR_XML_DATA_FORMAT_LIST_ENTRY_NAME);
	CU_ASSERT(0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("failed to tdr_input_file: %s\n", tdr_error_string(iRet));
	}

	CU_ASSERT(0 == memcmp(&szSrcData[0], &szInputData[0], stHostInfo.iBuff));

	iRet = tdr_output_file(pstMeta, "test_output_TestUnsizedArr2.xml", &stHostInfo, tdr_get_meta_current_version(pstMeta),
		TDR_XML_DATA_FORMAT_LIST_ENTRY_NAME);
	CU_ASSERT(0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("failed to tdr_input_file: %s\n", tdr_error_string(iRet));
	}
}

void ut_brower_data()
{
	LPTDRMETA pstMeta;
	int iHandle;
	BUDDYLISTRES stRes;
	TDRDATA stHost;
	int iRet = 0;
	int i;

	pstMeta = tdr_get_meta_by_name(g_pstMetaLib, "BuddyListRes");
	if (NULL == pstMeta)
	{
		printf("failed to get meta by name BuddyListRes");
		return;
	}

	stRes.iListNum = 3;
	for (i =0; i < stRes.iListNum; i++)
	{
		LPNAMEID pstName = &stRes.astNameIdList[i].stNameId;
		pstName->ullGid = rand();
		strncpy((char *)&pstName->szName[0], "hello", sizeof(pstName->szName));
		pstName->bSize = strlen((char *)&pstName->szName[0]);
		stRes.astNameIdList[i].wGroupId = i;
	}

	iHandle = 0;
	stHost.iBuff = sizeof(stRes);
	stHost.pszBuff = (char *)&stRes;
	iRet = tdr_browse(pstMeta, (void *)100, &stHost, 0, enter_meta, vist_entry);

}


/*回调函数，在遍历结构成员时，如果此成员为struct/uion类型则回调此函数，获取此子成员处理句柄
@param[in] a_pHandle 此成员父结点的处理句柄
@param[in] a_pstChildEntry 此子成员的元数据描述句柄
@param[in] a_iTotal 此子成员的数组长度
@param[in] a_Index	如果此成员所在结构体为数组，则传入其数组当前索引值，否则传 -1
@param[out] a_ppChildHandle 获取此子成员处理句柄的指针
@note 根据子成员的a_pstChildEntry句柄调用tdr_get_entry_type_meta可以获得此子成员的类型
@return 0 成功 !0 失败
*/
int enter_meta(IN void *a_pHandle, IN LPTDRMETAENTRY a_pstChildEntry, IN int a_iTotal, IN int a_Index,  OUT void ** a_ppHandle)
{
#if !defined(_WIN32) && !defined(_WIN64)
	static intptr_t iHandle = -1;

	assert(NULL != a_pHandle);
	assert(NULL != a_ppHandle);

	if ((intptr_t)a_pHandle != iHandle)
	{
		iHandle = (int)(intptr_t )a_pHandle;
		*a_ppHandle = (void *)iHandle + 100;
	}else
	{
		*a_ppHandle = (void *)iHandle + 1;
	}
	printf("Meta RH:%"PRIdPTR", child:%s  total:%d index:%d childHandle:%"PRIdPTR"\n",
		(intptr_t)a_pHandle, tdr_get_entry_name(a_pstChildEntry), a_iTotal, a_Index,
		(intptr_t)(*a_ppHandle));
#endif
	return 0;
}


/*回调函数，在遍历结构成员时，如果此成员为简单数据类型,则回调此函数，
@param[in] a_pHandle 此成员父结点的处理句柄
@param[in] a_Index	如果此成员所在结构体为数组，则传入其数组当前索引值，否则传 -1
@param[in] a_pstEntry 此子成员的元数据描述句柄
@param[in] a_iCount 此成员的数组长度
@param[in] a_pszHostStart 此成员本地存储起始地址
@param[in] a_iSize 此成员存储空间可用长度
@return 0 成功 !0 失败
*/
int vist_entry(IN void *a_pHandle, IN LPTDRMETAENTRY a_pstEntry, IN int a_iCount,
									  IN char *a_pszHostStart, IN int a_iSize)
{
	assert(NULL != a_pHandle);
	assert(NULL != a_pstEntry);
	
	printf("Entry RH:%"PRIdPTR", child:%s  total:%d  host:%p size:%d\n",
		(intptr_t)a_pHandle, tdr_get_entry_name(a_pstEntry), a_iCount,
		a_pszHostStart,  a_iSize);

	return 0;
}

void ut_xmlesc()
{
	LPTDRMETA pstMeta;
	TESTESC stData;
	TDRDATA stHost;
	TESTESC stInData;
	unsigned int i;
	int iRet= 0;

	pstMeta = tdr_get_meta_by_name(g_pstMetaLib, "TestEsc");
	if (NULL == pstMeta)
	{
		printf("failed to get meta by name:%s", "TestEsc");
		return ;
	}

	memset(&stData, 0, sizeof(stData));
	strcpy(stData.szStr1,"<>&'\"");
	for (i = 0; i < sizeof(stData.szStr2); i++)
	{
		stData.szStr2[i] = '&';
	}
	stData.szStr2[sizeof(stData.szStr2) -1] = '\0';

	stHost.iBuff = sizeof(stData);
	stHost.pszBuff = (char *)&stData;
	iRet = tdr_output_file(pstMeta, "TestXmlEsc.xml", &stHost, 0, TDR_XML_DATA_FORMAT_LIST_ENTRY_NAME);
	CU_ASSERT(0 == iRet);


	memset(&stInData, 0, sizeof(stInData));
	stHost.iBuff = sizeof(stInData);
	stHost.pszBuff = (char *)&stInData;
	iRet = tdr_input_file(pstMeta, &stHost, "TestXmlEsc.xml", 0, TDR_XML_DATA_FORMAT_LIST_ENTRY_NAME);
	CU_ASSERT(0 == iRet);

	CU_ASSERT(0 == memcmp(&stData, &stInData, sizeof(stData)));

	strcpy(stData.szStr1,"");
	for (i = 0; i < sizeof(stData.szStr2); i++)
	{
		stData.szStr2[i] = '&';
	}
    stData.szStr2[sizeof(stData.szStr2) -1] = '\0';
	stHost.iBuff = sizeof(stData);
	stHost.pszBuff = (char *)&stData;
	iRet = tdr_output_file(pstMeta, "TestXmlEscError.xml", &stHost, 0, TDR_XML_DATA_FORMAT_LIST_ENTRY_NAME);
	CU_ASSERT(0 == iRet);	

    strcpy(stData.szStr1,"abc");
    for (i = 0; i < sizeof(stData.szStr2); i++)
    {
        stData.szStr2[i] = 'a';
    }
    stData.szStr2[sizeof(stData.szStr2) -1] = '\0';
    stHost.iBuff = sizeof(stData);
    stHost.pszBuff = (char *)&stData;
    iRet = tdr_output_file(pstMeta, "TestXmlEscError1.xml", &stHost, 0, TDR_XML_DATA_FORMAT_LIST_ENTRY_NAME);
    CU_ASSERT(0 == iRet);	
}

void ut_attr_xml()
{
	LPTDRMETA pstMeta;
	CONF stData;
	CONF stDataIn;
	int i = 0;
	TDRDATA stHost;
	int iRet = 0;

	pstMeta = tdr_get_meta_by_name(g_pstMetaLib, "Conf");
	CU_ASSERT(NULL != pstMeta);

	memset(&stData, 0, sizeof(stData));
	TDR_STRNCPY(stData.szSvrURL, "tcp:192.168.1.198:9001?reuse=1", sizeof(stData.szSvrURL));
	stData.iTrustIPNum = 3;
	for (i = 0; i < stData.iTrustIPNum; i++)
	{
		tdr_snprintf(stData.aszTrustIP[i], sizeof(stData.aszTrustIP[i]), "192.168.1.%d", i+10);
	}
	for (i = 0; i < sizeof(stData.mngID)/sizeof(stData.mngID[0]); i++)
	{
		stData.mngID[i] = i;
	}
	stData.nStatusnum = 2;
	stData.astStatus[0].nType = 2;
	stData.astStatus[0].stValue.stFunny.a = 66;
	stData.astStatus[0].stValue.stFunny.b = 45;
	stData.astStatus[0].stValue.stFunny.c = 456;
	stData.astStatus[1].nType = 1;
	stData.astStatus[1].stValue.iDummy = 1234;

	stHost.iBuff = sizeof(stData);
	stHost.pszBuff = (char *)&stData;
	iRet = tdr_output_file(pstMeta, "Conf.xml", &stHost, 0, TDR_XML_DATA_FORMAT_ATTR_ENTRY_NAME);
	CU_ASSERT(0 == iRet);
	if (0 != iRet)
	{
		printf("failed to tdr_output_file: %s\n", tdr_error_string(iRet));
	}

	memset(&stDataIn, 0, sizeof(stDataIn));
	stHost.iBuff = sizeof(stDataIn);
	stHost.pszBuff = (char *)&stDataIn;
	iRet = tdr_input_file(pstMeta, &stHost, "Conf.xml", 0, TDR_XML_DATA_FORMAT_ATTR_ENTRY_NAME);
	CU_ASSERT(0 == iRet);
	if (0 != iRet)
	{
		printf("failed to tdr_input_file: %s\n", tdr_error_string(iRet));
	}

	CU_ASSERT(0 == memcmp(&stData, &stDataIn, sizeof(stDataIn)));

	stHost.iBuff = sizeof(stDataIn);
	stHost.pszBuff = (char *)&stDataIn;
	iRet = tdr_output_file(pstMeta,"Conf_In_attr.xml",  &stHost, 0, TDR_XML_DATA_FORMAT_ATTR_ENTRY_NAME);
	CU_ASSERT(0 == iRet);


	stHost.iBuff = sizeof(stDataIn);
	stHost.pszBuff = (char *)&stDataIn;
	iRet = tdr_output_file(pstMeta,"Conf_In_list.xml",  &stHost, 0, TDR_XML_DATA_FORMAT_LIST_ENTRY_NAME);
	CU_ASSERT(0 == iRet);

	stHost.iBuff = sizeof(stDataIn);
	stHost.pszBuff = (char *)&stDataIn;
	iRet = tdr_output_file(pstMeta,"Conf_In_Meta.xml",  &stHost, 0, TDR_XML_DATA_FORMAT_LIST_MATA_NAME);
	CU_ASSERT(0 == iRet);

}

void ut_simplemetalib_io_attr()
{
	int iRet = 0;
	LPTDRMETA pstMeta = NULL;
	PKG stPkg;
	char szXml[TDR_MAX_NET_PACKAGE_LEN];
	TDRDATA stHostInfo;
	TDRDATA stXmlInfo;
	FILE *fp;
	PKG stInPutPkg;


	pstMeta = tdr_get_meta_by_name(g_pstMetaLib, "Pkg");
	CU_ASSERT( NULL != pstMeta);

	memset(&stPkg, 0, sizeof(stPkg));
	stPkg.stHead.dwMsgid = 100;
	stPkg.stHead.nCmd = 0;
	stPkg.stHead.nVersion = 9;
	tdr_str_to_tdrdatetime(&stPkg.stHead.tTime, "2007-05-12 12:34:36");
	stPkg.stHead.nMagic = 0x3344;
	stPkg.nBodycount = 2;

	strncpy(stPkg.stBody.astLogin[0].szName, "haha<&\'\">", sizeof(stPkg.stBody.astLogin[0].szName));
	strncpy(stPkg.stBody.astLogin[0].szPass, "secret", sizeof(stPkg.stBody.astLogin[0].szPass));
	strncpy(stPkg.stBody.astLogin[0].aszZone[0], "shenzhen", sizeof(stPkg.stBody.astLogin[0].aszZone[0]));
	strncpy(stPkg.stBody.astLogin[0].aszZone[1], "beijing", sizeof(stPkg.stBody.astLogin[0].aszZone[0]));

	strncpy(stPkg.stBody.astLogin[1].szName, "hihi", sizeof(stPkg.stBody.astLogin[1].szName));
	strncpy(stPkg.stBody.astLogin[1].szPass, "secret1", sizeof(stPkg.stBody.astLogin[1].szPass));
	strncpy(stPkg.stBody.astLogin[1].aszZone[0], "hunan", sizeof(stPkg.stBody.astLogin[1].aszZone[0]));
	strncpy(stPkg.stBody.astLogin[1].aszZone[1], "hubei", sizeof(stPkg.stBody.astLogin[1].aszZone[0]));


	stPkg.nBody2num = 2;
	stPkg.astBody2[0].a = 0x56;
	stPkg.astBody2[0].nNamenum = 2;
	strncpy(stPkg.astBody2[0].aszName[0], "zhangsan", sizeof(stPkg.astBody2[0].aszName[0]));
	strncpy(stPkg.astBody2[0].aszName[1], "lisi", sizeof(stPkg.astBody2[0].aszName[1]));
	stPkg.astBody2[0].fMagic = (float)108.88;

	stPkg.astBody2[1].a = 0x1234;
	stPkg.astBody2[1].nNamenum = 1;
	strncpy(stPkg.astBody2[1].aszName[0], "wangwu", sizeof(stPkg.astBody2[1].aszName[0]));
	stPkg.astBody2[1].fMagic = (float)1586.78;

	stPkg.iChecksum = 0x12345678;

	stPkg.nStatusnum = 2;
	stPkg.astStatus[0].nType = 2;
	stPkg.astStatus[0].stValue.stFunny.a = 66;
	stPkg.astStatus[0].stValue.stFunny.b = 45;
	stPkg.astStatus[0].stValue.stFunny.c = 456;
	stPkg.astStatus[1].nType = 1;
	stPkg.astStatus[1].stValue.iDummy = 1234;

	stPkg.stNtstatus.nType = 1;
	stPkg.stNtstatus.stValue.iDummy = 5678;


	stHostInfo.iBuff = sizeof(stPkg);
	stHostInfo.pszBuff = (char *)&stPkg;
	stXmlInfo.iBuff = sizeof(szXml);
	stXmlInfo.pszBuff = &szXml[0];
	iRet = tdr_output(pstMeta, &stXmlInfo, &stHostInfo, 0, TDR_XML_DATA_FORMAT_ATTR_ENTRY_NAME);
	CU_ASSERT(0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("%s\n", tdr_error_string(iRet));
	}else
	{
		fp = fopen("stPkg_attr.xml", "w");
		if (NULL != fp)
		{
			fwrite(stXmlInfo.pszBuff, 1, stXmlInfo.iBuff, fp);
			fclose(fp);
		}

		memset(&stInPutPkg, 0, sizeof(stInPutPkg));
		stHostInfo.iBuff = sizeof(stInPutPkg);
		stHostInfo.pszBuff = (char *)&stInPutPkg;
		iRet = tdr_input(pstMeta, &stHostInfo, &stXmlInfo, 0, TDR_XML_DATA_FORMAT_ATTR_ENTRY_NAME);
		ut_tdr_assert_package_equal(&stInPutPkg, &stPkg);
		CU_ASSERT(0 <= iRet);
	}	

	/*test tdr_output_fp*/
	//stHostInfo.iBuff = sizeof(stPkg);
	//stHostInfo.pszBuff = (char *)&stPkg;
	fp = fopen("stPkg_fp_attr.xml", "w");
	if (NULL != fp)
	{
		iRet = tdr_output_fp(pstMeta, fp, &stHostInfo, 0, TDR_XML_DATA_FORMAT_ATTR_ENTRY_NAME);
		CU_ASSERT(0 <= iRet);
		fclose(fp);

		fp = fopen("stPkg_fp_attr.xml", "r");
		if (NULL != fp)
		{
			memset(&stInPutPkg, 0, sizeof(stInPutPkg));
			stHostInfo.iBuff = sizeof(stInPutPkg);
			stHostInfo.pszBuff = (char *)&stInPutPkg;
			iRet = tdr_input_fp(pstMeta, &stHostInfo, fp, 0, TDR_XML_DATA_FORMAT_ATTR_ENTRY_NAME);
			ut_tdr_assert_package_equal(&stInPutPkg, &stPkg);
			CU_ASSERT(0 <= iRet);

			fclose(fp);
		}		
	}

	

	iRet = tdr_output_file(pstMeta, "stPkg_file_attr.xml", &stHostInfo, 0, TDR_XML_DATA_FORMAT_ATTR_ENTRY_NAME);
	CU_ASSERT(0 <= iRet);
	if (0 != iRet)
	{
		printf("tdr_output_file faield :%s\n", tdr_error_string(iRet));
	}



	memset(&stInPutPkg, 0, sizeof(stInPutPkg));
	stHostInfo.iBuff = sizeof(stInPutPkg);
	stHostInfo.pszBuff = (char *)&stInPutPkg;
	iRet = tdr_input_file(pstMeta, &stHostInfo, "stPkg_file_attr.xml", 0, TDR_XML_DATA_FORMAT_ATTR_ENTRY_NAME);
	CU_ASSERT(0 <= iRet);

	
}



