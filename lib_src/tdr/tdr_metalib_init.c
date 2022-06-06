/**
*
* @file     tdr_metalib_init.c
* @brief    Ԫ���ݿ���Դ����
* : ��ʼ�����ͷ���Դ,��ȡ��Դ��С
*
* @author steve jackyai
* @version 1.0
* @date 2007-03-26
*
*
* Copyright (c)  2007, ��Ѷ�Ƽ����޹�˾���������з���
* All rights reserved.
*
*/

#include <assert.h>
//#include <scew/scew.h>
#include <string.h>
#include <stdlib.h>

#include "tdr_os.h"
#include "tdr/tdr_metalib_init.h"
#include "tdr/tdr_error.h"
#include "tdr_error_i.h"
#include "tdr_metalib_manage_i.h"
#include "tdr_define_i.h"

#if defined (_WIN32) || defined (_WIN64)
#pragma warning(disable:4996)
#endif

int tdr_init_lib(INOUT LPTDRMETALIB pstLib, IN const LPTDRLIBPARAM pstParam)
{
    size_t iTotalSize = 0;

    //assert(NULL != pstLib);
    //assert(NULL != pstParam);
	if ((NULL == pstLib)||(NULL == pstParam))
	{
		return TDR_ERRIMPLE_INVALID_PARAM;
	}

    if ((pstParam->iMaxMacros <= 0) && (pstParam->iMaxMetas <= 0))
    {

        return TDR_ERRIMPLE_INVALID_METALIB_PARAM;
    }




    /*���ռ��Ƿ�*/
    iTotalSize = TDR_CALC_MIN_SIZE(pstParam->iMaxMetas, pstParam->iMaxMacros);
    iTotalSize += pstParam->iMetaSize;
    iTotalSize += pstParam->iStrBufSize;
	iTotalSize += TDR_CALC_MACROSGROUP_MAP_SIZE(pstParam->iMaxMacrosGroupNum);
	iTotalSize += pstParam->iMacrosGroupSize;
    if (iTotalSize > pstParam->iSize)
    {

        return TDR_ERRIMPLE_INVALID_METALIB_PARAM;
    }

    /*��ʼ��Ԫ���ݿ�*/
    memset(pstLib, 0, pstParam->iSize);

    pstLib->iFreeStrBufSize = pstParam->iStrBufSize;
    pstLib->iID = pstParam->iID;

    pstLib->iMaxMacroNum = pstParam->iMaxMacros;
    pstLib->iMaxMetaNum = pstParam->iMaxMetas;

    pstLib->iSize = pstParam->iSize;
    pstLib->iVersion = pstParam->iVersion;
    pstLib->iXMLTagSetVer = pstParam->iTagSetVersion;
	pstLib->iMaxMacrosGroupNum = pstParam->iMaxMacrosGroupNum;
	pstLib->iMacrosGroupSize = pstParam->iMacrosGroupSize;

    pstLib->nBuild = TDR_BUILD;
    pstLib->wMagic = TDR_MAGIC;
    pstLib->dwPlatformArch = TDR_FLATFORM_MAGIC;

    TDR_STRNCPY(pstLib->szName, pstParam->szName, sizeof(pstLib->szName));

    //����������ݿ��ƫ��
    pstLib->ptrMacro = TDR_CALC_MACRO_PTR(pstParam);
    pstLib->ptrID = TDR_CALC_ID_PTR(pstParam);
    pstLib->ptrName = TDR_CALC_NAME_PTR(pstParam);
    pstLib->ptrMap = TDR_CALC_MAP_PTR(pstParam);
    pstLib->ptrMeta = TDR_CALC_META_PTR(pstParam);
    pstLib->ptrStrBuf = TDR_CALC_STRBUF_PTR(pstParam);
	pstLib->ptrMacroGroupMap = TDR_CALC_MACROSGROUP_MAP_PTR(pstParam);
	pstLib->ptrMacrosGroup = TDR_CALC_MACROSGROUP_PTR(pstParam);


    pstLib->ptrLaseMeta = pstLib->ptrMeta;
    pstLib->ptrFreeStrBuf = pstLib->ptrStrBuf;

	/*TO DO Ŀǰ������ֵ�����Ե����ݽṹ��Ϊ����ʵ����*/
	pstLib->ptrValueFiledDefinitions = TDR_INVALID_PTR;
	pstLib->ptrValueFiledDefinitionsMap = TDR_INVALID_PTR;

    /* ��ʼ�������ֶ� */
    pstLib->iReserve1 = TDR_INVALID_PTR;
    pstLib->iReserve2 = TDR_INVALID_PTR;
    pstLib->iReserve3 = TDR_INVALID_PTR;

    return 0;
}


void tdr_free_lib(INOUT LPTDRMETALIB* ppstLib)
{
    if (NULL != ppstLib)
    {
        if (NULL != *ppstLib)
        {
            free(*ppstLib);
            *ppstLib = NULL;
        }
    }
}

size_t tdr_size(IN LPTDRMETALIB pstLib)
{
    //assert(NULL != pstLib);
	if (NULL == pstLib)
	{
		return 0;
	}

    return pstLib->iSize;
}

LPTDRMETALIB tdr_clone_lib(IN void* a_pvDst, INOUT int* a_piBuff, IN LPTDRMETALIB a_pstSrcLib)
{
	/*assert(NULL != a_pvDst);
	assert(NULL != a_piBuff);
	assert(NULL != a_pstSrcLib);
	assert(0 < *a_piBuff);*/
	if ((NULL == a_pvDst)||(NULL == a_piBuff)||(0 >= *a_piBuff)||(NULL == a_pstSrcLib))
	{
		return NULL;
	}


	if ((uint32_t)*a_piBuff < a_pstSrcLib->iSize)
	{
		return NULL;
	}

	memcpy(a_pvDst, a_pstSrcLib, a_pstSrcLib->iSize);
	*a_piBuff = (int)a_pstSrcLib->iSize;

	return (LPTDRMETALIB)a_pvDst;
}



