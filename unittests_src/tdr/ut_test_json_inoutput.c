/**
*
* @file    ut_test_json_inoutput.c
* @brief   JSON数据输入输出相关测试用例
*
* @author flyma
* @version 1.0
* @date 2011-02-25
*
*
* Copyright (c)  2011, 腾讯科技有限公司互动娱乐研发部
* All rights reserved.
*
*/

#include <stdio.h>

#include <CUnit/CUnit.h>
#include <CUnit/TestDB.h>
#include <assert.h>
#include "tdr/tdr.h"
#include "ut_tdr.h"
#include "ut_test_json_inoutput.h"


LPTDRMETALIB  g_pstJsonInOutputLib = NULL;

static int json_inoutput_suite_init(void);
static int json_inoutput_suite_clean(void);

static void ut_json_inoutput_test_parameters();
static void ut_json_inoutput_test_buffer();
static void ut_json_inoutput_test_file();

int json_inoutput_suite_init(void)
{
	int iRet;

	iRet = tdr_create_lib_file(&g_pstJsonInOutputLib , TEST_FILE_PATH"test_json_inoutput.xml", 1, stderr);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("failed to tdr_create_lib_file: %s\n", tdr_error_string(iRet));
	}else
	{
		ut_dump_metalib(g_pstJsonInOutputLib , "test_json_inoutput_metalib_dump.txt");
	}

	if (TDR_ERR_IS_ERROR(iRet))
	{
		return iRet;
	}

	return 0;
}

int json_inoutput_suite_clean(void)
{
	tdr_free_lib(&g_pstJsonInOutputLib );
	return 0;
}



int ut_add_json_inoutput_test_suites()
{
	CU_pSuite pSuite = NULL;

	//CU_add_suite 增加一个Suite
	//InitSuite EndSuite:分别是测试单元初始和释放函数,如不需要则NULL传递
	pSuite = CU_add_suite("ut_json_inoutput_Suites", json_inoutput_suite_init, json_inoutput_suite_clean);
	if(NULL == pSuite)
	{//检测注册suite情况　
		CU_cleanup_registry();
		return CU_get_error();
	}

	if( NULL == CU_add_test(pSuite, "ut_json_inoutput_test_parameters",
                            (CU_TestFunc)ut_json_inoutput_test_parameters))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}

	if( NULL == CU_add_test(pSuite, "ut_json_inoutput_test_buffer",
                            (CU_TestFunc)ut_json_inoutput_test_buffer))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}

	if( NULL == CU_add_test(pSuite, "ut_json_inoutput_test_file",
                            (CU_TestFunc)ut_json_inoutput_test_file))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}

	return 0;
}

static void ut_json_set_pkg(LPPKG a_pstPkg)
{
    int i = 0;

    assert(NULL != a_pstPkg);

    memset(a_pstPkg, 0, sizeof(*a_pstPkg));

    a_pstPkg->stHead.nCmd = CMD_LOGIN;

    a_pstPkg->iSize = 6;
    for (i = 0; i < a_pstPkg->iSize; i++)
    {
        int j = 0;

        a_pstPkg->astArray[i].iReason = i;
        strcpy((char*)a_pstPkg->astArray[i].szAttr, "reason?");

        a_pstPkg->astArray[i].iSize = i;
        for (j = 0; j < a_pstPkg->astArray[i].iSize; j++)
        {
            a_pstPkg->astArray[i].array[j] = i * 1000 + j;
        }
    }

    strcpy(a_pstPkg->stBody.stLogin.szName, "n\\a\"me");
    strcpy(a_pstPkg->stBody.stLogin.szPass, "p\ba\fs\ns");
    strcpy(a_pstPkg->stBody.stLogin.szZone, "z\ro\tn/e");
}

void ut_json_inoutput_test_parameters()
{
    TDRDATA stHost;
    LPTDRMETA pstMeta = NULL;
    int iRet = TDR_SUCCESS;

	puts("ut_initialize_data");
	pstMeta = tdr_get_meta_by_name(g_pstJsonInOutputLib , "Pkg");
	CU_ASSERT(NULL != pstMeta);

    iRet = tdr_input_json_fp(NULL, &stHost, stdin, 0);
    CU_ASSERT(iRet == TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM));

    iRet = tdr_input_json_fp(pstMeta, NULL, stdin, 0);
    CU_ASSERT(iRet == TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM));

    iRet = tdr_input_json_fp(pstMeta, &stHost, NULL, 0);
    CU_ASSERT(iRet == TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM));

    iRet = tdr_input_json_file(pstMeta, &stHost, NULL, 0);
    CU_ASSERT(iRet == TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM));

    iRet = tdr_input_json_file(pstMeta, &stHost, "none_existed_file", 0);
    CU_ASSERT(iRet == TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_FAILED_OPEN_FILE_TO_READ));
}

void ut_json_inoutput_test_buffer()
{
	LPTDRMETA pstMeta;
    TDRDATA stHost;
    TDRDATA stJson;
    PKG stPkgSrc;
    PKG stPkgDst;
    int iRet = TDR_SUCCESS;

    static char buffer[1024] = {0};

	puts("ut_initialize_data");
	pstMeta = tdr_get_meta_by_name(g_pstJsonInOutputLib , "Pkg");
	CU_ASSERT(NULL != pstMeta);

    ut_json_set_pkg(&stPkgSrc);

    stHost.pszBuff = (char*)&stPkgSrc;
    stHost.iBuff = sizeof(stPkgSrc);

    stJson.iBuff = sizeof(buffer);
    stJson.pszBuff = buffer;

    iRet = tdr_output_json(pstMeta, &stJson, &stHost, 0);
    CU_ASSERT(!TDR_ERR_IS_ERROR(iRet));

    memset(&stPkgDst, 0, sizeof(stPkgDst));
    stHost.pszBuff = (char*)&stPkgDst;
    stHost.iBuff = sizeof(stPkgDst);

    iRet = tdr_input_json(pstMeta, &stHost, &stJson, 0);
    CU_ASSERT(!TDR_ERR_IS_ERROR(iRet));

    CU_ASSERT(!memcmp(&stPkgSrc, &stPkgDst, sizeof(stPkgDst)));
}

void ut_json_inoutput_test_file()
{
	LPTDRMETA pstMeta;
    TDRDATA stHost;
    PKG stPkgSrc;
    PKG stPkgDst;
    int iRet = TDR_SUCCESS;

	puts("ut_initialize_data");
	pstMeta = tdr_get_meta_by_name(g_pstJsonInOutputLib , "Pkg");
	CU_ASSERT(NULL != pstMeta);

    ut_json_set_pkg(&stPkgSrc);

    stHost.pszBuff = (char*)&stPkgSrc;
    stHost.iBuff = sizeof(stPkgSrc);

    iRet = tdr_output_json_file(pstMeta, "json.txt", &stHost, 0);
    CU_ASSERT(!TDR_ERR_IS_ERROR(iRet));

    iRet = tdr_output_json_fp(pstMeta, stdout, &stHost, 0);
    CU_ASSERT(!TDR_ERR_IS_ERROR(iRet));

    memset(&stPkgDst, 0, sizeof(stPkgDst));
    stHost.pszBuff = (char*)&stPkgDst;
    stHost.iBuff = sizeof(stPkgDst);

    iRet = tdr_input_json_file(pstMeta, &stHost, "json.txt", 0);
    CU_ASSERT(!TDR_ERR_IS_ERROR(iRet));

    CU_ASSERT(!memcmp(&stPkgSrc, &stPkgDst, sizeof(stPkgDst)));
}
