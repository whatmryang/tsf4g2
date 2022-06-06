/**
*
* @file     ut_tdr.h
* @brief    tdr单元测试主文件
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
#ifndef UT_TDR_H
#define UT_TDR_H

#include "tdr/tdr.h"
#include <CUnit/CUnit.h>
#include <CUnit/TestDB.h>

#if defined(_WIN32) || defined(_WIN64)
#pragma warning(disable:4996)
#pragma warning(disable:4996)
#endif


#if defined(_WIN32) || defined(_WIN64)
#define TEST_FILE_PATH		".\\testfile\\"
#else
#define TEST_FILE_PATH		"./testfile/"
#endif

#define  TDR_MAX_NET_PACKAGE_LEN  32767


#if defined(_WIN32) || defined(_WIN64)

# ifdef _DEBUG
#  define LIB_D    "_d"
# else
#  define LIB_D
# endif /* _DEBUG */

# if defined(LIB_D)
# pragma comment( lib, "libcunit"  LIB_D ".lib" )
# else
# pragma comment( lib, "libcunit.lib" )
# endif

#endif

void add_all_tdr_tests();

int ut_add_typeinfo_test_suites();
int ut_add_tdr_errorhanlder_test_suites();
int ut_add_create_metalib_test_suites();
int ut_add_tdr_iostream_test_suites();
int ut_add_save_metalib_test_suites();
int ut_add_metalib_to_hpp_test_suites();
int ut_add_metalib_manage_test_suites();
int ut_add_metalib_net_test_suites();
int ut_add_dataio_test_suites();
int ut_add_datasort_test_suites();
int ut_add_data_initialize_test_suites();
int ut_add_defaultvalue_test_suites();
int ut_add_metalib_to_cfile_test_suites();
int ut_add_database_test_suites();
int ut_add_ormapping_test_suites();
void ut_dump_metalib(LPTDRMETALIB a_pstLib, const char *a_pszFileName);
void ut_get_rand_datetime(tdr_datetime_t *pstTdrDateTime);
int ut_add_md5hash_test_suites();
int ut_add_meta_index_suites();
int ut_add_escape_characters_test_suites();
int ut_add_json_inoutput_test_suites();

#endif
