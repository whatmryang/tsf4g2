/**
*
* @file     test_tdr_iostream.c 
* @brief    测试TDR IO流处理函数
* 
* @author jackyai  
* @version 1.0
* @date 2007-04-23 
*
*
* Copyright (c)  2007, 腾讯科技有限公司互动娱乐研发部
* All rights reserved.
*
*/

#include <stdio.h>
#include <CUnit/CUnit.h>
#include <CUnit/TestDB.h>


#include "tdr_iostream_i.h"

void ut_tdr_iostream_strBuf();
void ut_tdr_iostream_file();

int ut_add_tdr_iostream_test_suites()
{
	CU_pSuite pSuite = NULL;

	//CU_add_suite 增加一个Suite 
	//InitSuite EndSuite:分别是测试单元初始和释放函数,如不需要则NULL传递
	pSuite = CU_add_suite("TestTdrIIOStreamSuites", NULL, NULL);
	if(NULL == pSuite)
	{//检测注册suite情况　
		CU_cleanup_registry();
		return CU_get_error();
	}

	//注册当前Suite下的测试用例　
	//pSuite:用例指针
	//"Test1": 测试单元名称 
	//Test1:测试函数 

	if( NULL == CU_add_test(pSuite, "ut_tdr_iostream_strBuf", (CU_TestFunc)ut_tdr_iostream_strBuf))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}

	if( NULL == CU_add_test(pSuite, "ut_tdr_iostream_file", (CU_TestFunc)ut_tdr_iostream_file))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}

	return 0;
}

void ut_tdr_iostream_strBuf()
{
	TDRIOSTREAM stIO;
	char szBuff[1024] = {0};
	int iLen = 0;

	stIO.emIOStreamType = TDR_IOSTREAM_STRBUF;
	stIO.stParam.stBuffIOParam.pszBuff = &szBuff[0];
	stIO.stParam.stBuffIOParam.iBuffLen = sizeof(szBuff);

	CU_ASSERT(tdr_iostream_write(&stIO, "%s=%d", "123", 123) == 0);
	CU_ASSERT(tdr_iostream_write(&stIO, "%s=%d", "123", 123) == 0);
	CU_ASSERT_STRING_EQUAL(szBuff, "123=123123=123");
	iLen = stIO.stParam.stBuffIOParam.pszBuff - szBuff;
	puts(szBuff);
	CU_ASSERT((int)strlen(szBuff) == iLen);

	stIO.stParam.stBuffIOParam.pszBuff = &szBuff[0];
	stIO.stParam.stBuffIOParam.iBuffLen = 4;
	CU_ASSERT(tdr_iostream_write(&stIO, "%s=%d", "123", 123) != 0);
}

void ut_tdr_iostream_file()
{
	FILE *a_fp = NULL;
	TDRIOSTREAM stIO;
	int iLen = 0;

	a_fp = fopen("iostream", "w");
	if (NULL == a_fp)
	{
		return;
	}

	stIO.emIOStreamType = TDR_IOSTREAM_FILE;
	stIO.fpTDRIO = a_fp;

	CU_ASSERT(tdr_iostream_write(&stIO, "%s=%d", "123", 123) == 0);
	CU_ASSERT(tdr_iostream_write(&stIO, "%s=%d", "123", 123) == 0);

	iLen = ftell(stIO.fpTDRIO);

	CU_ASSERT(iLen = strlen("123=123123=123"));

	fclose(a_fp);
}
