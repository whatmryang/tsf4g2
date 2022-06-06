/**
*
* @file    ut_test_escape_charactersvalue.c
* @brief   Ԫ����������ȱʡֵ��ز���
*
* @author jackyai
* @version 1.0
* @date 2007-05-15
*
*
* Copyright (c)  2007, ��Ѷ�Ƽ����޹�˾���������з���
* All rights reserved.
*
*/

#include <stdio.h>

#include <CUnit/CUnit.h>
#include <CUnit/TestDB.h>
#include "tdr/tdr.h"
#include "ut_tdr.h"
#include "ut_test_escape_characters.h"


LPTDRMETALIB  g_pstEscapeCharactersLib = NULL;

static int escape_characters_suite_init(void);
static int escape_characters_suite_clean(void);
static void ut_escape_characters_test();

int escape_characters_suite_init(void)
{
	int iRet;

	iRet = tdr_create_lib_file(&g_pstEscapeCharactersLib, TEST_FILE_PATH"test_escape_characters.xml", 1, stderr);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("failed to tdr_create_lib_file: %s\n", tdr_error_string(iRet));
	}else
	{
		ut_dump_metalib(g_pstEscapeCharactersLib, "test_escape_characters_metalib_dump.txt");
	}

	if (TDR_ERR_IS_ERROR(iRet))
	{
		return iRet;
	}

	return 0;
}

int escape_characters_suite_clean(void)
{
	tdr_free_lib(&g_pstEscapeCharactersLib);
	return 0;
}



int ut_add_escape_characters_test_suites()
{
	CU_pSuite pSuite = NULL;

	//CU_add_suite ����һ��Suite
	//InitSuite EndSuite:�ֱ��ǲ��Ե�Ԫ��ʼ���ͷź���,�粻��Ҫ��NULL����
	pSuite = CU_add_suite("ut_escape_characters_Suites", escape_characters_suite_init, escape_characters_suite_clean);
	if(NULL == pSuite)
	{//���ע��suite�����
		CU_cleanup_registry();
		return CU_get_error();
	}


	if( NULL == CU_add_test(pSuite, "ut_escape_characters_test", (CU_TestFunc)ut_escape_characters_test))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}

	return 0;
}

void ut_escape_characters_test()
{
    /**
     * ת���ַ�����������ں��ڻ������濼�ǣ�Ŀǰֻ�ǰ���صĴ���ʹ�ú�ע�͵�
     */
    /*
	STRINGSET stStringSet;
	TDRDATA stHostInfo;
	LPTDRMETA pstMeta;
	int iRet;

	puts("ut_initialize_data");
	pstMeta = tdr_get_meta_by_name(g_pstEscapeCharactersLib, "StringSet");
	CU_ASSERT(NULL != pstMeta);

	stHostInfo.iBuff = sizeof(stStringSet);
	stHostInfo.pszBuff = (char *)&stStringSet;
	iRet = tdr_init(pstMeta, &stHostInfo, 0);
	CU_ASSERT(0 <= iRet);
    CU_ASSERT(!strcmp(stStringSet.szOctA, "beginAtail"));
    CU_ASSERT(!strcmp(stStringSet.szOctTable1, "begin\ttail"));
    CU_ASSERT(!strcmp(stStringSet.szOctTable2, "begin\ttail"));
    CU_ASSERT(!strcmp(stStringSet.szOctReturn, "begin\ntail"));
    CU_ASSERT(!strcmp(stStringSet.szOctNul1, "begin"));
    CU_ASSERT(!strcmp(stStringSet.szOctNul2, "begin"));
    CU_ASSERT(!strcmp(stStringSet.szOctNul3, "begin"));
    CU_ASSERT(!strcmp(stStringSet.szOctNul4, "begin"));

    CU_ASSERT(!strcmp(stStringSet.szDoubleSlash, "begin\\040tail"));
    */
}
