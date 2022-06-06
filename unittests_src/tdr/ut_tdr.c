/**
*
* @file     ut_tdr.c
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
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "ut_tdr.h"
#include "tdr_ctypes_info_i.h"

static TDRBOOLEAN ut_isValid_tdrdatetime_i(LPTDRDATETIME pstDateTime);
static TDRBOOLEAN ut_isValid_tdrdate_i(LPTDRDATE pstDate);

void add_all_tdr_tests()
{

	ut_add_typeinfo_test_suites();
	ut_add_tdr_errorhanlder_test_suites();
	ut_add_tdr_iostream_test_suites();
	ut_add_create_metalib_test_suites();

	ut_add_save_metalib_test_suites();
	ut_add_metalib_to_hpp_test_suites();
	ut_add_metalib_manage_test_suites();
	ut_add_metalib_net_test_suites();

	ut_add_data_initialize_test_suites();
	ut_add_metalib_to_cfile_test_suites();
	ut_add_datasort_test_suites();
	ut_add_defaultvalue_test_suites();
	ut_add_database_test_suites();
	ut_add_ormapping_test_suites();

	ut_add_dataio_test_suites();

	ut_add_md5hash_test_suites();
	ut_add_meta_index_suites();

    ut_add_escape_characters_test_suites();
    ut_add_json_inoutput_test_suites();
}


void ut_dump_metalib(LPTDRMETALIB a_pstLib, const char *a_pszFileName)
{
	FILE *a_fp = NULL;

	assert(NULL != a_pstLib);
	assert(NULL != a_pszFileName);


	a_fp = fopen(a_pszFileName, "w");
	if (NULL == a_fp)
	{
		printf("failed to open file for writing : %s", strerror(errno));
		return ;
	}

	tdr_dump_metalib(a_pstLib, a_fp);

	fclose(a_fp);
}

void ut_get_rand_datetime(tdr_datetime_t *pstTdrDateTime)
{
	char szBuf[32];
	tdr_datetime_t stTime;

	for (;;)
	{
		sprintf(szBuf, "%4d-%2d-%2d %2d:%2d:%2d", rand()%10000, rand()%13, rand()%28,rand()%24,rand()%60,rand()%60);
		tdr_str_to_tdrdatetime(&stTime, szBuf);
		if (ut_isValid_tdrdatetime_i((LPTDRDATETIME)&stTime))
		{
			*pstTdrDateTime = stTime;
			break;
		}
	}

}


TDRBOOLEAN ut_isValid_tdrdate_i(LPTDRDATE pstDate)
{
	assert(pstDate);

	if ((-9999 > pstDate->nYear) || (pstDate->nYear > 9999))
	{
		return TDR_FALSE;
	}

	if (12 < pstDate->bMonth)
	{
		return TDR_FALSE;
	}

	if (31 < pstDate->bDay)
	{
		return TDR_FALSE;
	}
	switch(pstDate->bDay)
	{
	case 2:
		{
			if ((pstDate->nYear % 4) == 0 && ((pstDate->nYear % 100) != 0 ||
				(pstDate->nYear % 400) == 0))
			{
				if (28 < pstDate->bDay)
				{
					return TDR_FALSE;
				}
			}else
			{
				if (29 < pstDate->bDay)
				{
					return TDR_FALSE;
				}
			}/*if ((pstDate->nYear % 4) == 0 && ((pstDate->nYear % 100) != 0 || */
			break;
		}
	case 4:
	case 6:
	case 9:
	case 11:
		{
			if (30 < pstDate->bDay)
			{
				return TDR_FALSE;
			}
			break;
		}
	default:
		break;
	}

	return TDR_TRUE;
}

TDRBOOLEAN ut_isValid_tdrdatetime_i(LPTDRDATETIME pstDateTime)
{
	TDRBOOLEAN bValid;

	assert(pstDateTime);


	bValid = ut_isValid_tdrdate_i(&pstDateTime->stDate);
	if (TDR_FALSE == bValid)
	{
		return bValid;
	}

	if ((23 < pstDateTime->stTime.nHour))
	{
		return TDR_FALSE;
	}
	if ( (59 < pstDateTime->stTime.bMin))
	{
		return TDR_FALSE;
	}

	if ((59 < pstDateTime->stTime.bSec))
	{
		return TDR_FALSE;
	}


	return TDR_TRUE;
}
