/**
*
* @file     tdrex_sort_data.c 
* @brief    ����Ԫ���ݿ�����ݽ�������
*
* @note TDR�ṩ�����API������: tdr_sort�� tdr_sort_spec
* @ִ�б�����ǰ�Ѿ�������������Ԫ����XML��������Ӧ��.h�ļ�
* 
* @author jackyai  
* @version 1.0
* @date 2007-06-15 
*
*
* Copyright (c)  2007, ��Ѷ�Ƽ����޹�˾���������з���
* All rights reserved.
*
*/

#include <stdio.h>
#include <stdlib.h>
#include "tdr/tdr.h"
#include "tdr_sort.h"


void ut_tdr_init_metas(LPCOMPOSEARR a_pstMetas, int iMaxFloatArrNum);
void ut_tdr_init_simplesort(LPSIMPLESORT a_pstSimpleSort);
extern unsigned char g_szMetalib_tdrsort[];

int main()
{
	LPTDRMETALIB pstLib = NULL;
	int iRet;
	SIMPLESORT stData;
	LPTDRMETA pstMeta;
	TDRDATA stHostInfo;


	/*����xml�ļ�����Ԫ���ݿ�*/
	iRet = tdr_create_lib_file(&pstLib, "tdr_sort.xml", 1, stderr);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_create_lib_file failed by file %s, for %s\n", "tdr_sort.xml", tdr_error_string(iRet));
		return iRet;
	}

	iRet = tdr_metalib_to_cfile(pstLib, "tdr_sort.c");
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_metalib_to_cfile failed, for %s\n", tdr_error_string(iRet));
	}


	pstMeta = tdr_get_meta_by_name(pstLib, "SimpleSort");
	if (NULL == pstMeta)
	{
		return -1;
	}


	ut_tdr_init_simplesort(&stData);
	stHostInfo.pszBuff = (char *)&stData;
	stHostInfo.iBuff = sizeof(stData);
	iRet = tdr_sort(pstMeta, &stHostInfo, tdr_get_meta_current_version(pstMeta));
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_sort failed for %s\n", tdr_error_string(iRet));
		return iRet;
	}

	iRet = tdr_output_file(pstMeta, "ex_sort_data.xml", &stHostInfo,
		tdr_get_meta_current_version(pstMeta), TDR_IO_NEW_XML_VERSION);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("failed to output into file for %s\n", tdr_error_string(iRet));
		return iRet;
	}

	return 0;

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
		}
		break;
	default:
		break;
	}

	ut_tdr_init_metas(&a_pstSimpleSort->stSortMetaArr, MAX_NUM_OF_ARRAY);
}
