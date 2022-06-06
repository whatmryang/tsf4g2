/**
*
* @file     test_data_sort.c 
* @brief    测试数据排序
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
#include <assert.h>
#include <time.h>


#include "tdr/tdr.h"
#include "ut_tdr.h"
#include "ut_tdr_sort.h"

#define DATE_SORT_MAX_VERSION 10

LPTDRMETALIB g_pstDateSortMetaLib = NULL;

static void ut_simple_data_sort();
static void ut_tdr_init_metas(LPCOMPOSEARR a_pstMetas, int iMaxFloatArrNum);
static void ut_tdr_init_simplesort(LPSIMPLESORT a_pstSimpleSort);
static void ut_compsoe_data_sort();
static void ut_special_data_sort();
static int ut_date_sort_suite_init();
static int ut_date_sort_suite_clean();
static 	void ut_save_sortxml_to_hpp();
static void ut_sort_custom_type_data();
static void ut_init_custom_type_sorting_data(LPCUSTOMTYPEDATA pstData);
static void ut_compsoe_data_normalize();


int ut_add_datasort_test_suites()
{
	CU_pSuite pSuite = NULL;


	pSuite = CU_add_suite("ut_DataSort_Test_Suites", 
		(CU_InitializeFunc)ut_date_sort_suite_init, (CU_CleanupFunc)ut_date_sort_suite_clean);
	if(NULL == pSuite)
	{//检测注册suite情况　
		CU_cleanup_registry();
		return CU_get_error();
	}



	if( NULL == CU_add_test(pSuite, "ut_save_sortxml_to_hpp", (CU_TestFunc)ut_save_sortxml_to_hpp))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}
	if( NULL == CU_add_test(pSuite, "ut_simple_data_sort", (CU_TestFunc)ut_simple_data_sort))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}

    if( NULL == CU_add_test(pSuite, "ut_compsoe_data_sort", (CU_TestFunc)ut_compsoe_data_sort))
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

	if( NULL == CU_add_test(pSuite, "ut_special_data_sort", (CU_TestFunc)ut_special_data_sort))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}

	if( NULL == CU_add_test(pSuite, "ut_sort_custom_type_data", (CU_TestFunc)ut_sort_custom_type_data))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}

	if( NULL == CU_add_test(pSuite, "ut_compsoe_data_normalize", (CU_TestFunc)ut_compsoe_data_normalize))
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
	iRet = tdr_create_lib_file(&g_pstDateSortMetaLib, TEST_FILE_PATH"tdr_sort.xml", 1, stderr);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("g_pstDateSortMetaLib failed : %s\n", tdr_error_string((iRet)));
		return iRet;
	}else
	{
		ut_dump_metalib(g_pstDateSortMetaLib, "tdr_sort_dump.txt");
	}
	
	return 0;	
}

int ut_date_sort_suite_clean()
{
	tdr_free_lib(&g_pstDateSortMetaLib);
	return 0;
}

void ut_simple_data_sort()
{
	SIMPLESORT stData;
	int iRet = 0;
	LPTDRMETA pstMeta;
	TDRDATA stHostInfo;

	puts("ut_simple_data_sort");
	ut_tdr_init_simplesort(&stData);

	
	pstMeta = tdr_get_meta_by_name(g_pstDateSortMetaLib, "SimpleSort");
	CU_ASSERT( NULL != pstMeta);

	stHostInfo.pszBuff = (char *)&stData;
	stHostInfo.iBuff = sizeof(stData);

	iRet = tdr_output_file(pstMeta, "tdr_sort_data.xml", &stHostInfo,
		DATE_SORT_MAX_VERSION, TDR_XML_DATA_FORMAT_LIST_ENTRY_NAME);
	CU_ASSERT(0 <= iRet); 

	iRet = tdr_sort_spec(pstMeta, &stHostInfo, 1, "sortint", DATE_SORT_MAX_VERSION);
	CU_ASSERT(0 <= iRet); 

	iRet = tdr_output_file(pstMeta, "tdr_sort_sortint_data.xml", &stHostInfo,
		DATE_SORT_MAX_VERSION, TDR_XML_DATA_FORMAT_LIST_ENTRY_NAME);
	CU_ASSERT(0 <= iRet); 
	printf("ut_simple_data_sort, tdr_output_file failed for %s", tdr_error_string(iRet));

	iRet = tdr_sort(pstMeta, &stHostInfo, DATE_SORT_MAX_VERSION);
	CU_ASSERT(0 <= iRet); 

	iRet = tdr_output_file(pstMeta, "tdr_sort_data.xml", &stHostInfo,
		DATE_SORT_MAX_VERSION, TDR_XML_DATA_FORMAT_LIST_ENTRY_NAME);
	CU_ASSERT(0 <= iRet); 

	tdr_normalize(pstMeta, &stHostInfo, 0);
	iRet = tdr_output_file(pstMeta, "tdr_sort_data2.xml", &stHostInfo,
		DATE_SORT_MAX_VERSION, TDR_XML_DATA_FORMAT_LIST_ENTRY_NAME);
	CU_ASSERT(0 <= iRet); 


	iRet = tdr_output_file(pstMeta, "tdr_sort_data_attr.xml", &stHostInfo,
		DATE_SORT_MAX_VERSION, TDR_XML_DATA_FORMAT_ATTR_ENTRY_NAME);
	CU_ASSERT(0 <= iRet); 

}	

void ut_compsoe_data_sort()
{
    COMPOSESORT stData;
    int i,j;
    int iRet = 0;
    LPTDRMETA pstMeta;
    TDRDATA stHostInfo;


	puts("ut_compsoe_data_sort");
    stData.nSelect1 = rand() % 2;
    for (j = 0; j < MAX_NUM_OF_ARRAY; j++)
    {
        switch(stData.nSelect1)
        {
        case 0:
            {
                for (i = 0; i < MAX_NUM_OF_ARRAY; i++)
                {
                    stData.astSorttinyint[j].szTinyintArr[i] = (char)rand();
                }
            }
            break;
        case 1:
            {
                for (i = 0; i < MAX_NUM_OF_ARRAY; i++)
                {
                    stData.astSorttinyint[j].szTinyuintArr[i] = (unsigned char)rand();
                }
            }
            break;
        default:
            break;
        }
    }
   

    for (i = 0; i < MAX_NUM_OF_ARRAY; i++)
    {
        ut_tdr_init_metas(&stData.astSortMetaArr1[i], MAX_NUM_OF_ARRAY);
        ut_tdr_init_metas(&stData.astSortMetaArr2[i], MAX_NUM_OF_ARRAY);
        ut_tdr_init_metas(&stData.astSortMetaArr3[i], 1);
        ut_tdr_init_simplesort(&stData.astSortSimpleSort1[i]);
        ut_tdr_init_simplesort(&stData.astSortSimpleSort2[i]);
    }

   

    pstMeta = tdr_get_meta_by_name(g_pstDateSortMetaLib, "ComposeSort");
    CU_ASSERT( NULL != pstMeta);

    stHostInfo.pszBuff = (char *)&stData;
    stHostInfo.iBuff = sizeof(stData);
    iRet = tdr_output_file(pstMeta, "tdr_presort_compose_data.xml", &stHostInfo,
        DATE_SORT_MAX_VERSION, TDR_XML_DATA_FORMAT_LIST_ENTRY_NAME);
    CU_ASSERT(0 <= iRet); 
    iRet = tdr_sort(pstMeta, &stHostInfo, DATE_SORT_MAX_VERSION);
    CU_ASSERT(0 <= iRet); 

    iRet = tdr_output_file(pstMeta, "tdr_sort_compose_data.xml", &stHostInfo,
        DATE_SORT_MAX_VERSION, TDR_XML_DATA_FORMAT_LIST_ENTRY_NAME);
    CU_ASSERT(0 <= iRet); 
}

void ut_tdr_init_metas(LPCOMPOSEARR a_pstMetas, int iMaxFloatArrNum)
{
    int i;
 
    if ( 0 >= iMaxFloatArrNum)
    {
        iMaxFloatArrNum = MAX_NUM_OF_ARRAY;
    }

    a_pstMetas->nStrArrNum = rand() % MAX_NUM_OF_ARRAY;
    for (i = 0; i < a_pstMetas->nStrArrNum; i++)
    {	
        int j;
        int istrLen;       
        istrLen = rand() % MAX_NUM_OF_ARRAY;
        if (0 >= istrLen)
        {
            istrLen = 1;
        }
        for (j = 0; j < istrLen; j++)
        {
            a_pstMetas->aszStrArr[i][j] = (char)('a' + (rand() %('z'-'a')));
        }
        a_pstMetas->aszStrArr[i][istrLen] = '\0';
    }

    a_pstMetas->nFloatArrNum = rand() % iMaxFloatArrNum;
    if (0 == a_pstMetas->nFloatArrNum)
    {
        a_pstMetas->nFloatArrNum = 1;
    }
    for (i = 0; i < a_pstMetas->nFloatArrNum; i++)
    {	
        int j;
        int istrLen;

        a_pstMetas->floatArr[i] = (float)(rand() /1.0);
        a_pstMetas->doubleArr[i] = (double)(rand()/1.0005);
        istrLen = rand() % MAX_NUM_OF_ARRAY;
        if (0 >= istrLen)
        {
            istrLen = 1;
        }
        for (j = 0; j < istrLen; j++)
        {
            a_pstMetas->aszStrArr[i][j] = (char)('a' + (rand() %('z'-'a')));
        }
        a_pstMetas->aszStrArr[i][istrLen] = '\0';
    }

    for (i = 0; i < MAX_NUM_OF_ARRAY; i++)
    {	
        int j;
        int istrLen;

        a_pstMetas->floatArr[i] = (float)(rand() /1.0);
        a_pstMetas->doubleArr[i] = (double)(rand()/1.0005);
        istrLen = rand() % MAX_NUM_OF_ARRAY;
        if (0 >= istrLen)
        {
            istrLen = 1;
        }
        for (j = 0; j < istrLen; j++)
        {
            a_pstMetas->aszStrArr[i][j] = (char)('a' + (rand() %('z'-'a')));
        }
        a_pstMetas->aszStrArr[i][istrLen] = '\0';
    }
}

void ut_tdr_init_simplesort(LPSIMPLESORT a_pstSimpleSort)
{
    int i;

    a_pstSimpleSort->chIntArrCount = rand() % MAX_NUM_OF_ARRAY;
    for (i = 0; i < a_pstSimpleSort->chIntArrCount; i++)
    {
        a_pstSimpleSort->sortint[i] = rand();
    }

    a_pstSimpleSort->nSelect1 = rand() % 2;
    switch(a_pstSimpleSort->nSelect1)
    {
    case 0:
        {
            for (i = 0; i < MAX_NUM_OF_ARRAY; i++)
            {
                a_pstSimpleSort->stSorttinyint.szTinyintArr[i] = (char)rand();
            }
        }
        break;
    case 1:
        {
            for (i = 0; i < MAX_NUM_OF_ARRAY; i++)
            {
                a_pstSimpleSort->stSorttinyint.szTinyuintArr[i] = (unsigned char)rand();
            }
        }
        break;
    default:
        break;
    }

    a_pstSimpleSort->nSelect2 = rand() % 2;
    switch(a_pstSimpleSort->nSelect2)
    {
    case 0:
        {
            for (i = 0; i < MAX_NUM_OF_ARRAY; i++)
            {
                a_pstSimpleSort->stSortsmallint.smallintArr[i] = (short)rand();
            }
        }
        break;
    case 1:
        {
            for (i = 0; i < MAX_NUM_OF_ARRAY; i++)
            {
                a_pstSimpleSort->stSortsmallint.smalluintArr[i] = (unsigned short)rand();
            }
        }
        break;
    default:
        break;
    }

    for (i = 0; i < MAX_NUM_OF_ARRAY; i++)
    {
        a_pstSimpleSort->sortuint[i] = (unsigned int)rand();
    }

    a_pstSimpleSort->nSelect3 = rand() % 2;
    switch(a_pstSimpleSort->nSelect3)
    {
    case 0:
        {
            for (i = 0; i < MAX_NUM_OF_ARRAY; i++)
            {
                a_pstSimpleSort->stSortlong.longArr[i] = (long)rand();
            }
        }
        break;
    case 1:
        {
            for (i = 0; i < MAX_NUM_OF_ARRAY; i++)
            {
                a_pstSimpleSort->stSortlong.ulongArr[i] = (unsigned long)rand();
            }
        }
        break;
    default:
        break;
    }

    a_pstSimpleSort->nSelect4 = rand() % 2;
	//a_pstSimpleSort->nSelect4 = 1;
    switch(a_pstSimpleSort->nSelect4)
    {
    case 0:
        {
            for (i = 0; i < MAX_NUM_OF_ARRAY; i++)
            {
                a_pstSimpleSort->stSortlonglong.longlongArr[i] = (tdr_longlong)(rand() * rand());
            }
        }
        break;
    case 1:
        {
            for (i = 0; i < MAX_NUM_OF_ARRAY; i++)
            {
                a_pstSimpleSort->stSortlonglong.ulonglongArr[i] = (unsigned long)(rand() * rand());
            }
			/*a_pstSimpleSort->stSortlonglong.ulonglongArr[0] = 1;
			a_pstSimpleSort->stSortlonglong.ulonglongArr[1] = 2;
			a_pstSimpleSort->stSortlonglong.ulonglongArr[2] = 65538;
			a_pstSimpleSort->stSortlonglong.ulonglongArr[3] = 3;*/
        }
        break;
    default:
        break;
    }

    ut_tdr_init_metas(&a_pstSimpleSort->stSortMetaArr, MAX_NUM_OF_ARRAY);
}


void ut_special_data_sort()
{
	COMPOSESORT stData;
	int i,j;
	int iRet = 0;
	LPTDRMETA pstMeta;
	TDRDATA stHostInfo;

	puts("ut_special_data_sort");
	stData.nSelect1 = rand() % 2;
	for (j = 0; j < MAX_NUM_OF_ARRAY; j++)
	{
		switch(stData.nSelect1)
		{
		case 0:
			{
				for (i = 0; i < MAX_NUM_OF_ARRAY; i++)
				{
					stData.astSorttinyint[j].szTinyintArr[i] = (char)rand();
				}
			}
			break;
		case 1:
			{
				for (i = 0; i < MAX_NUM_OF_ARRAY; i++)
				{
					stData.astSorttinyint[j].szTinyuintArr[i] = (unsigned char)rand();
				}
			}
			break;
		default:
			break;
		}
	}


	for (i = 0; i < MAX_NUM_OF_ARRAY; i++)
	{
		ut_tdr_init_metas(&stData.astSortMetaArr1[i], MAX_NUM_OF_ARRAY);
		ut_tdr_init_metas(&stData.astSortMetaArr2[i], MAX_NUM_OF_ARRAY);
		ut_tdr_init_metas(&stData.astSortMetaArr3[i], 1);
		ut_tdr_init_simplesort(&stData.astSortSimpleSort1[i]);
		ut_tdr_init_simplesort(&stData.astSortSimpleSort2[i]);
	}

	
	pstMeta = tdr_get_meta_by_name(g_pstDateSortMetaLib, "ComposeSort");
	CU_ASSERT( NULL != pstMeta);

	stHostInfo.pszBuff = (char *)&stData;
	stHostInfo.iBuff = sizeof(stData);
	iRet = tdr_sort_spec(pstMeta, &stHostInfo, 1, "sorttinyint", DATE_SORT_MAX_VERSION);
	CU_ASSERT(0 <= iRet); 

	iRet = tdr_output_file(pstMeta, "tdr_sort_compose_sorttinyintArr_data.xml", &stHostInfo,
		DATE_SORT_MAX_VERSION, TDR_XML_DATA_FORMAT_LIST_ENTRY_NAME);
	CU_ASSERT(0 <= iRet); 

	iRet = tdr_sort_spec(pstMeta, &stHostInfo, 2, "sortSimpleSort1", DATE_SORT_MAX_VERSION);
	CU_ASSERT(0 <= iRet); 

	iRet = tdr_output_file(pstMeta, "tdr_sort_compose_sortSimpleSort1_data.xml", &stHostInfo,
		DATE_SORT_MAX_VERSION, TDR_XML_DATA_FORMAT_LIST_ENTRY_NAME);
	CU_ASSERT(0 <= iRet); 

	iRet = tdr_sort_spec(pstMeta, &stHostInfo, 2, "sortMetaArr1.strArr", DATE_SORT_MAX_VERSION);
	CU_ASSERT(0 <= iRet); 

	iRet = tdr_output_file(pstMeta, "tdr_sort_compose_sortMetaArr1_data.xml", &stHostInfo,
		DATE_SORT_MAX_VERSION, TDR_XML_DATA_FORMAT_LIST_ENTRY_NAME);
	CU_ASSERT(0 <= iRet);

	iRet = tdr_sort_spec(pstMeta, &stHostInfo, 2, "sortSimpleSort2.sortint", DATE_SORT_MAX_VERSION);
	CU_ASSERT(0 <= iRet); 

	iRet = tdr_output_file(pstMeta, "tdr_sort_compose_sortSimpleSort2_sortint_data.xml", &stHostInfo,
		DATE_SORT_MAX_VERSION, TDR_XML_DATA_FORMAT_LIST_ENTRY_NAME);
	CU_ASSERT(0 <= iRet);
}

void ut_save_sortxml_to_hpp()
{
	int iRet = 0;
	TDRHPPRULE stRule;

	memset(&stRule, 0, sizeof(stRule));
	stRule.iRule = 0;
	iRet = tdr_metalib_to_hpp(g_pstDateSortMetaLib, "ut_tdr_sort.h", &stRule);
	CU_ASSERT(0 <= iRet); 
}


void ut_sort_custom_type_data()
{
	CUSTOMTYPEDATA stData;
	int iRet = 0;
	LPTDRMETA pstMeta;
	TDRDATA stHostInfo;

	puts("ut_sort_custom_type_data");
	ut_init_custom_type_sorting_data(&stData);

	pstMeta = tdr_get_meta_by_name(g_pstDateSortMetaLib, "CustomTypeData");
	CU_ASSERT( NULL != pstMeta);

	stHostInfo.pszBuff = (char *)&stData;
	stHostInfo.iBuff = sizeof(stData);
	iRet = tdr_sort(pstMeta, &stHostInfo, DATE_SORT_MAX_VERSION);
	CU_ASSERT(0 <= iRet); 

	iRet = tdr_output_file(pstMeta, "tdr_sort_custom_type_data.xml", &stHostInfo,
		DATE_SORT_MAX_VERSION, TDR_XML_DATA_FORMAT_LIST_ENTRY_NAME);
	CU_ASSERT(0 <= iRet); 
	
}

void ut_init_custom_type_sorting_data(LPCUSTOMTYPEDATA pstData)
{
	int i,j;

	assert(NULL != pstData);

	pstData->chDatetimerefer = rand() % MAX_NUM_OF_ARRAY;
	for (i = 0; i < pstData->chDatetimerefer; i++)
	{
		char szBuf[32];
		sprintf(szBuf, "%4d-%2d-%2d %2d:%2d:%2d", rand()%10000, rand()%13, rand()%28,rand()%24,rand()%60,rand()%60);
		tdr_str_to_tdrdatetime(&pstData->sortdatetime[i], &szBuf[0]);	
	}

	pstData->chDaterefer = rand() % MAX_NUM_OF_ARRAY;
	for (i = 0; i < pstData->chDaterefer; i++)
	{
		char szBuf[32];
		sprintf(szBuf, "%4d-%2d-%2d", rand()%10000, rand()%13, rand()%28);
		tdr_str_to_tdrdate(&pstData->sortdate[i], &szBuf[0]); 
	}

	for (i = 0; i < MAX_NUM_OF_ARRAY; i++)
	{
		char szBuf[32];
		sprintf(szBuf, "%3d:%2d:%2d", rand()%1000, rand()%60, rand()%60);
		tdr_str_to_tdrtime(&pstData->sorttime[i], &szBuf[0]); 
	}

	for (i = 0; i < MAX_NUM_OF_ARRAY; i++)
	{
		pstData->sortip[i] = (tdr_ip_t)rand();
	}

	for (i = 0; i < MAX_NUM_OF_ARRAY; i++)
	{
		pstData->sortwchar[i] = (tdr_wchar_t)('a' + (rand() %('z'-'a')));
	}

	for (i = 0; i < MAX_NUM_OF_ARRAY; i++)
	{
		int istrLen;
		int itemp;

		istrLen = rand() % MAX_NUM_OF_ARRAY;
		if (1 >= istrLen)
		{
			istrLen = 2;
		}
		for (j = 0; j < istrLen; j++)
		{
			pstData->aszSortwstring[i][j] = (tdr_wchar_t)('a' + (rand() %('z'-'a')));
		}
		itemp = rand() % istrLen;
		pstData->aszSortwstring[i][itemp] = 0x597d;
		pstData->aszSortwstring[i][istrLen] = L'\0';
	}
}

void ut_compsoe_data_normalize()
{
	COMPOSESORT stData;
	int i,j;
	int iRet = 0;
	LPTDRMETA pstMeta;
	TDRDATA stHostInfo;


	puts("ut_compsoe_data_normalize");
	stData.nSelect1 = rand() % 2;
	for (j = 0; j < MAX_NUM_OF_ARRAY; j++)
	{
		switch(stData.nSelect1)
		{
		case 0:
			{
				for (i = 0; i < MAX_NUM_OF_ARRAY; i++)
				{
					stData.astSorttinyint[j].szTinyintArr[i] = (char)rand();
				}
			}
			break;
		case 1:
			{
				for (i = 0; i < MAX_NUM_OF_ARRAY; i++)
				{
					stData.astSorttinyint[j].szTinyuintArr[i] = (unsigned char)rand();
				}
			}
			break;
		default:
			break;
		}
	}


	for (i = 0; i < MAX_NUM_OF_ARRAY; i++)
	{
		ut_tdr_init_metas(&stData.astSortMetaArr1[i], MAX_NUM_OF_ARRAY);
		ut_tdr_init_metas(&stData.astSortMetaArr2[i], MAX_NUM_OF_ARRAY);
		ut_tdr_init_metas(&stData.astSortMetaArr3[i], 1);
		ut_tdr_init_simplesort(&stData.astSortSimpleSort1[i]);
		ut_tdr_init_simplesort(&stData.astSortSimpleSort2[i]);
	}



	pstMeta = tdr_get_meta_by_name(g_pstDateSortMetaLib, "ComposeSort");
	CU_ASSERT( NULL != pstMeta);

	stHostInfo.pszBuff = (char *)&stData;
	stHostInfo.iBuff = sizeof(stData);
	iRet = tdr_normalize(pstMeta, &stHostInfo,DATE_SORT_MAX_VERSION);
	CU_ASSERT(0 <= iRet); 
	iRet = tdr_sort(pstMeta, &stHostInfo, DATE_SORT_MAX_VERSION);
	CU_ASSERT(0 <= iRet); 

	iRet = tdr_output_file(pstMeta, "tdr_normalize_compose_data.xml", &stHostInfo,
		DATE_SORT_MAX_VERSION, TDR_XML_DATA_FORMAT_LIST_ENTRY_NAME);
	CU_ASSERT(0 <= iRet); 
}
