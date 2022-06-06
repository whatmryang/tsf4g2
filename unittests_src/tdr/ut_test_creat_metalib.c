/**
*
* @file     test_creat_metalib.c
* @brief    测试创建Metalib的函数
*
* @author jackyai
* @version 1.0
* @date 2007-04-04
*
*
* Copyright (c)  2007, 腾讯科技有限公司互动娱乐研发部
* All rights reserved.
*
*/

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include <CUnit/CUnit.h>
#include <CUnit/TestDB.h>

#include <scew/scew.h>

#include "tdr/tdr_XMLMetaLib.h"
#include "tdr/tdr_metalib_init.h"
#include "tdr_metalib_kernel_i.h"
#include "tdr/tdr_error.h"
#include "tdr/tdr_auxtools.h"
#include "ut_tdr.h"
#include "tdr_XMLMetalib_i.h"
#include "tdr_metalib_param_i.h"
#include "tdr_metalib_meta_manage_i.h"
#include "tdr_xml_inoutput_i.h"


static int ut_load_xmlfile_to_buf(const char *pszFileName);
int ut_free_xmlbuf();



/*测试用例*/
void ut_create_metalib();
void ut_tdr_add_lib_param_i();
void ut_tdr_get_meta_size_i();
void ut_tdr_normalize_string();
void ut_tdr_add_macro();
void ut_ov_cs2metalib();
void ut_simpleXML2Metalib();
void ut_simpleXMLV0_to_Metalib();


char *g_szXMLBuf = NULL;
int g_iXML = 0;

int ut_add_create_metalib_test_suites()
{
    CU_pSuite pSuite = NULL;

    //CU_add_suite 增加一个Suite
    //InitSuite EndSuite:分别是测试单元初始和释放函数,如不需要则NULL传递
    pSuite = CU_add_suite("TestCreatMetalib", NULL, (CU_CleanupFunc)ut_free_xmlbuf);
    if(NULL == pSuite)
    {//检测注册suite情况　
        CU_cleanup_registry();
        return CU_get_error();
    }

    //注册当前Suite下的测试用例　
    //pSuite:用例指针
    //"Test1": 测试单元名称
    //Test1:测试函数

    if( NULL == CU_add_test(pSuite, "ut_create_metalib", (CU_TestFunc)ut_create_metalib) )
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if( NULL == CU_add_test(pSuite, "ut_tdr_add_lib_param_i", (CU_TestFunc)ut_tdr_add_lib_param_i) )
    {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if( NULL == CU_add_test(pSuite, "ut_tdr_get_meta_size_i", (CU_TestFunc)ut_tdr_get_meta_size_i) )
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if( NULL == CU_add_test(pSuite, "ut_tdr_normalize_string", (CU_TestFunc)ut_tdr_normalize_string) )
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if( NULL == CU_add_test(pSuite, "ut_tdr_add_macro", (CU_TestFunc)ut_tdr_add_macro) )
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if( NULL == CU_add_test(pSuite, "ut_ov_cs2metalib", (CU_TestFunc)ut_ov_cs2metalib) )
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if( NULL == CU_add_test(pSuite, "ut_simpleXML2Metalib", (CU_TestFunc)ut_simpleXML2Metalib) )
    {
        CU_cleanup_registry();
        return CU_get_error();
    }


    if( NULL == CU_add_test(pSuite, "ut_simpleXMLV0_to_Metalib", (CU_TestFunc)ut_simpleXMLV0_to_Metalib) )
    {
        CU_cleanup_registry();
        return CU_get_error();
    }


    return 0;
}

void ut_create_metalib()
{
    int iRet = 0;
    LPTDRMETALIB pstLib = NULL;

	puts("ut_create_metalib");

    /*测试XML文件不正确的情况*/
    printf("test file: %s\n", TEST_FILE_PATH"error.xml");
    CU_ASSERT(ut_load_xmlfile_to_buf(TEST_FILE_PATH"error.xml") == 0);
    iRet = tdr_create_lib(&pstLib, g_szXMLBuf, g_iXML, 3, stderr);
    CU_ASSERT((int)TDR_ERRIMPLE_FAILED_EXPACT_XML == iRet);

    /*测试tagserversion不正确的情况*/
    ut_free_xmlbuf();
    printf("test file: %s\n", TEST_FILE_PATH"InvalidTagSetVersion.xml");
    CU_ASSERT(ut_load_xmlfile_to_buf(TEST_FILE_PATH"InvalidTagSetVersion.xml") == 0);
    iRet = tdr_create_lib(&pstLib, g_szXMLBuf, g_iXML, 3, stderr);
    CU_ASSERT((int)TDR_ERRIMPLE_INVALID_TAGSET_VERSION == iRet);
    ut_free_xmlbuf();



      /*测试XML文件不存在根元素的情况*/
    printf("test file: %s\n", TEST_FILE_PATH"test1.xml");
    CU_ASSERT(ut_load_xmlfile_to_buf(TEST_FILE_PATH"test1.xml") == 0);
    iRet = tdr_create_lib(&pstLib, g_szXMLBuf, g_iXML, 1, stderr);
    CU_ASSERT((int)TDR_ERRIMPLE_FAILED_EXPACT_XML == iRet);
    ut_free_xmlbuf();



}

void ut_simpleXML2Metalib()
{
    int iRet = 0;
    LPTDRMETALIB pstLib = NULL;

	puts("ut_simpleXML2Metalib");
    printf("\ntest file: %s\n", TEST_FILE_PATH"testMetalibParam.xml");
    CU_ASSERT(ut_load_xmlfile_to_buf(TEST_FILE_PATH"testMetalibParam.xml") == 0);
    iRet = tdr_create_lib(&pstLib, g_szXMLBuf, g_iXML, 1, stderr);
    CU_ASSERT(0 <= iRet);

    ut_dump_metalib(pstLib, "testMetalibParamDump.txt");
    tdr_free_lib(&pstLib);
    ut_free_xmlbuf();

}

void ut_simpleXMLV0_to_Metalib()
{
    int iRet = 0;
    LPTDRMETALIB pstLib = NULL;

	puts("ut_simpleXMLV0_to_Metalib");
    printf("\ntest file: %s\n", TEST_FILE_PATH"testMetalibParamV0.xml");
    CU_ASSERT(ut_load_xmlfile_to_buf(TEST_FILE_PATH"testMetalibParamV0.xml") == 0);
    iRet = tdr_create_lib(&pstLib, g_szXMLBuf, g_iXML, 0, stderr);
    CU_ASSERT(0 <= iRet);

	ut_dump_metalib(pstLib, "testMetalibParamV0Dump.txt");
    tdr_free_lib(&pstLib);
    ut_free_xmlbuf();

}

void ut_ov_cs2metalib()
{
    int iRet = 0;
    LPTDRMETALIB pstLib = NULL;

	puts("ut_ov_cs2metalib");
    printf("\ntest file: %s\n", TEST_FILE_PATH"ov_cs.xml");
    CU_ASSERT(ut_load_xmlfile_to_buf(TEST_FILE_PATH"ov_cs.xml") == 0);
    iRet = tdr_create_lib(&pstLib, g_szXMLBuf, g_iXML, 0, stderr);
    CU_ASSERT(0 <= iRet);

	ut_dump_metalib(pstLib, TEST_FILE_PATH"ov_csDump.txt");
    tdr_free_lib(&pstLib);
    ut_free_xmlbuf();

}
int ut_load_xmlfile_to_buf(const char *pszFileName)
{
    FILE *a_fp = NULL;
    long llen = 0;

    assert(NULL != pszFileName);

	puts("ut_load_xmlfile_to_buf");
    a_fp = fopen(pszFileName, "rb");
    if (NULL == a_fp)
    {
        printf("failed to open file %s, for %s\n", pszFileName, strerror(errno));
        return -1;
    }

    fseek(a_fp, 0, SEEK_END);
    llen = ftell(a_fp);
    if( llen<=0 )
    {
        fclose(a_fp);
        return -2;
    }
    rewind(a_fp);

    g_szXMLBuf = (char *)calloc(1, (size_t)llen);
    if (NULL == g_szXMLBuf)
    {
        printf("failed to malloc memory\n");
        return -3;
    }

    g_iXML = fread(g_szXMLBuf, 1, llen, a_fp);
    if (g_iXML != (int)llen)
    {
        printf("failed to read file %s, len<%ld> readlen<%d>, for %s\n", pszFileName, llen, g_iXML, strerror(errno));
        return -4;
    }

    return 0;
}

int ut_free_xmlbuf()
{
    if (NULL != g_szXMLBuf)
    {
        free(g_szXMLBuf);
        g_szXMLBuf = NULL;
    }

    return 0;
}




void ut_tdr_add_lib_param_i()
{
    int iRet = 0;
    LPTDRMETALIB pstLib = NULL;
    scew_tree *pstTree =NULL;
    TDRLIBPARAM stParam;


	puts("ut_tdr_add_lib_param_i");
    CU_ASSERT(ut_load_xmlfile_to_buf(TEST_FILE_PATH"macros.xml") == 0);
    CU_ASSERT(tdr_create_XMLParser_tree_byBuff_i(&pstTree, g_szXMLBuf, g_iXML, stderr) >= 0);
    iRet = tdr_get_lib_param_i(&stParam, pstTree, 1, stderr);
    CU_ASSERT(0 <= iRet);
    ut_free_xmlbuf();


    CU_ASSERT(ut_load_xmlfile_to_buf(TEST_FILE_PATH"testMetalibParam.xml") == 0);
    CU_ASSERT(tdr_create_XMLParser_tree_byBuff_i(&pstTree, g_szXMLBuf, g_iXML, stderr) >= 0);
    iRet = tdr_get_lib_param_i(&stParam, pstTree, 1, stderr);
    CU_ASSERT(TDR_XML_TAGSET_VERSION_1 == stParam.iTagSetVersion);
    /*CU_ASSERT(stParam.iMetaSize == TDR_CALC_MIN_META_SIZE(stParam.iMaxMetas));
    printf("%d %d sizeof(TDRMETAENTRY)= %d offsetof(TDRMETA, stEntries)= %d\n", stParam.iMetaSize, TDR_CALC_MIN_META_SIZE(stParam.iMaxMetas),
        sizeof(TDRMETAENTRY), offsetof(TDRMETA, stEntries));*/
    CU_ASSERT(0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_get_lib_param_i failed :%s\n", tdr_error_string(iRet));
	}
    ut_free_xmlbuf();

    CU_ASSERT(ut_load_xmlfile_to_buf(TEST_FILE_PATH"testMetalibParamV0.xml") == 0);
    CU_ASSERT(tdr_create_XMLParser_tree_byBuff_i(&pstTree, g_szXMLBuf, g_iXML, stderr) >= 0);
    iRet = tdr_get_lib_param_i(&stParam, pstTree, 0, stderr);
    CU_ASSERT(TDR_XML_TAGSET_VERSION_0 == stParam.iTagSetVersion);
    //CU_ASSERT(stParam.iMetaSize == TDR_CALC_MIN_META_SIZE(stParam.iMaxMetas));
    CU_ASSERT(0 <= iRet);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_get_lib_param_i failed :%s\n", tdr_error_string(iRet));
	}
    ut_free_xmlbuf();

	CU_ASSERT(ut_load_xmlfile_to_buf(TEST_FILE_PATH"macros.xml") == 0);
	CU_ASSERT(tdr_create_XMLParser_tree_byBuff_i(&pstTree, g_szXMLBuf, g_iXML, stderr) >= 0);
	iRet = tdr_add_lib_param_i(&stParam, pstTree, 1, stderr);
	CU_ASSERT(0 <= iRet);
	ut_free_xmlbuf();


    CU_ASSERT(ut_load_xmlfile_to_buf(TEST_FILE_PATH"testMetalibParam.xml") == 0);
    CU_ASSERT(tdr_create_XMLParser_tree_byBuff_i(&pstTree, g_szXMLBuf, g_iXML, stderr) >= 0);
    iRet = tdr_add_lib_param_i(&stParam, pstTree, 1, stderr);
    CU_ASSERT(TDR_XML_TAGSET_VERSION_1 == stParam.iTagSetVersion);
    CU_ASSERT((int)TDR_ERRIMPLE_NAMESPACE_CONFLICT == iRet);

    ut_free_xmlbuf();

    CU_ASSERT(ut_load_xmlfile_to_buf(TEST_FILE_PATH"testMetalibParamV0.xml") == 0);
    CU_ASSERT(tdr_create_XMLParser_tree_byBuff_i(&pstTree, g_szXMLBuf, g_iXML, stderr) >= 0);
    iRet = tdr_add_lib_param_i(&stParam, pstTree, 0, stderr);
    CU_ASSERT((int)TDR_XML_TAGSET_VERSION_0 == stParam.iTagSetVersion);
    CU_ASSERT(0 <= iRet);
    ut_free_xmlbuf();

    iRet = tdr_init_metalib_i(&pstLib, &stParam);
    CU_ASSERT(0 <= iRet);
    CU_ASSERT(pstLib->iSize == (unsigned int)stParam.iSize);
    tdr_free_lib(&pstLib);



}


void ut_tdr_get_meta_size_i()
{

    const char szXML[] =
        "<struct name=\"CmdLogin\" version=\"10\" cname =\"CmdLogin\" desc=\"CmdLogin\">\n"
            "<entry name=\"uin\"  type=\"uint64_t\"/>"
	    "<entry name=\"name\" type=\"string\" size=\"32\" cname =\"&amp;123456\" desc=\"&amp;123456\" defaultvalue=\"&amp;123456\" />\n"
        "</struct>";

    size_t iMetaSize = 0;
    size_t iStrSize = 0;
    int iRet = 0;
    scew_tree *pstTree = NULL;
    scew_element *pstElement = NULL;
    int iEntrisNum = 0;

	puts("ut_tdr_get_meta_size_i");
    CU_ASSERT(tdr_create_XMLParser_tree_byBuff_i(&pstTree, szXML, strlen(szXML), stderr) >= 0);
    pstElement = scew_tree_root(pstTree);
    iRet = tdr_get_meta_size_i(&iEntrisNum, &iMetaSize, &iStrSize, 1, pstElement, pstElement);
    CU_ASSERT(0 <= iRet);
    CU_ASSERT(iMetaSize == TDR_CALC_MIN_META_SIZE(1));
    CU_ASSERT(iStrSize == (int)(sizeof("CmdLogin")*3 + sizeof("&123456")*3  + sizeof("name")))
}

void ut_tdr_normalize_string()
{
	char szBuf[128] = {0};

	puts("ut_tdr_normalize_string");
	tdr_normalize_string(&szBuf[0], sizeof(szBuf), "    hello world  ");
	CU_ASSERT_STRING_EQUAL(szBuf,"hello world");

	tdr_normalize_string(&szBuf[0], sizeof(szBuf), "hello world  ");
	CU_ASSERT_STRING_EQUAL(szBuf,"hello world");

	tdr_normalize_string(&szBuf[0], sizeof(szBuf), "    hello world");
	CU_ASSERT_STRING_EQUAL(szBuf,"hello world");

	tdr_normalize_string(&szBuf[0], sizeof(szBuf), "hello world");
	CU_ASSERT_STRING_EQUAL(szBuf,"hello world");

	strncpy(szBuf, "    hello world", sizeof(szBuf));
	tdr_trim_str(&szBuf[0]);
	CU_ASSERT_STRING_EQUAL(szBuf,"hello world");

	strncpy(szBuf, "    hello world  ", sizeof(szBuf));
	tdr_trim_str(&szBuf[0]);
	CU_ASSERT_STRING_EQUAL(szBuf,"hello world");

	strncpy(szBuf, "hello world  ", sizeof(szBuf));
	tdr_trim_str(&szBuf[0]);
	CU_ASSERT_STRING_EQUAL(szBuf,"hello world");

	strncpy(szBuf, "hello world", sizeof(szBuf));
	tdr_trim_str(&szBuf[0]);
	CU_ASSERT_STRING_EQUAL(szBuf,"hello world");

}


void ut_tdr_add_macro()
{
    const char szXML[] =
        "<macro name=\"MAX_LEN\" id=\"10\" />";


    TDRLIBPARAM stParam ;
    LPTDRMETALIB pstLib=NULL;
    int iRet ;
	LPTDRMACRO pstMacro;

    scew_tree *pstTree = NULL;
    scew_element *pstElement = NULL;

	puts("ut_tdr_add_macro");
    CU_ASSERT(tdr_create_XMLParser_tree_byBuff_i(&pstTree, szXML, strlen(szXML), stderr) >= 0);
    pstElement = scew_tree_root(pstTree);
    memset(&stParam, 0, sizeof(stParam));
	stParam.iMaxMacros = 2;
    stParam.iSize = 1000;
    stParam.iMetaSize = 10;
    stParam.iStrBufSize = strlen("MAX_LEN")+1;
    iRet = tdr_init_metalib_i(&pstLib, &stParam);
    CU_ASSERT(0 <= iRet);

    iRet = tdr_add_macro_i(pstLib,pstElement, stderr, &pstMacro);
    CU_ASSERT(0 <= iRet);
	tdr_free_lib(&pstLib);
}
