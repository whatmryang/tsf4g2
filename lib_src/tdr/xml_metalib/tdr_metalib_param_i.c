/**
*
* @file     tdr_metalib_param_i.c
* @brief    Ԫ���������������ȡ
*
* @author steve jackyai
* @version 1.0
* @date 2007-04-05
*
*
* Copyright (c)  2007, ��Ѷ�Ƽ����޹�˾���������з���
* All rights reserved.
*
*/

#include <string.h>
#include <stdlib.h>
#include <assert.h>


#include "tdr/tdr_ctypes_info.h"
#include "tdr/tdr_os.h"

#include "tdr/tdr_define_i.h"
#include "tdr_metalib_param_i.h"
#include "tdr_metalib_meta_manage_i.h"
#include "tdr/tdr_error_i.h"
#include "tdr_XMLtags_i.h"
#include "tdr/tdr_ctypes_info_i.h"
#include "tdr/tdr_auxtools.h"

#if defined (_WIN32) || defined (_WIN64)
#pragma warning(disable:4996)
#endif


/**����Ԫ��������Ԫ���Ƿ���DR XML�����ĸ�Ԫ��
*@pre \e ���ô˷���֮ǰ�������tdr_get_metalib_root_attribte_i��ȡ��ǩ���İ汾��Ϣ
*/
static int tdr_check_metalib_root_i(IN scew_element* pstRoot, IN int iTagsVersion, IN FILE *fpError);

/**��XMLԪ���л�ȡ ��Ԫ�ص���������
*@note ���pstParam���Ѿ����������֣���pstRoot��������֮��ͬ��
* ����Ϊ��������ͬ��Ԫ����������ֱ�ӱ�����������
*@pre \e ���ô˷���֮ǰ�������tdr_get_metalib_root_attribte_i��ȡ��ǩ���İ汾��Ϣ
*/
static int tdr_metalib_get_root_name_i(INOUT LPTDRLIBPARAM pstParam, IN scew_element *pstRoot, IN FILE *fpError);


/**��ȡmetalibԪ�ص�ID����*/
static int tdr_metalib_get_id_i(INOUT LPTDRLIBPARAM pstParam, IN scew_element *pstRoot);


/*��ȡmetalib��Ԫ�ص�����*/
static int tdr_get_metalib_root_attribte_i(INOUT LPTDRLIBPARAM a_pstParam, IN scew_element *a_pstRoot, IN int a_iTagsVersion, IN FILE *a_fpError);

/*��ȡ�궨�壬�ṹ��Ԫ�ص���Ŀ��������ԭ���ݿ���ڴ��С*/
static int tdr_get_metalib_size_i(INOUT LPTDRLIBPARAM pstParam, IN scew_element *pstRoot, IN FILE *fpError);

static int tdr_get_metalib_ver0_size_i(INOUT LPTDRLIBPARAM pstParam, IN scew_element *pstRoot, IN FILE *fpError);

static int tdr_get_metalib_ver1_size_i(INOUT LPTDRLIBPARAM pstParam, IN scew_element *pstRoot, IN FILE *fpError);

/**����ȱʡֵ������ڴ�ռ��ֽ���
*/
static size_t tdr_get_entry_defaultvalue_size_i(scew_element *a_pstItem);

/**����description����ֵ������ڴ�ռ��ֽ���
*/
static size_t tdr_get_description_size_i(scew_element *a_pstItem);

/**����cname����ֵ������ڴ�ռ��ֽ���
*/
static size_t tdr_get_cname_size_i(scew_element *a_pstItem);

static int tdr_get_meta_primaykey_size_i(scew_element *a_pstElement, scew_element *a_pstRoot);

static int tdr_get_index_column_count_i(scew_element *a_pstElement, scew_element *a_pstRoot);

static int tdr_get_str_groups_num_i(char* a_pszTarget, const char a_chSep);

static scew_attribute *tdr_get_primarykey_by_depended_strut_i(scew_element *a_pstElement, scew_element *a_pstRoot);

static size_t tdr_get_entry_customattr_size_i(scew_element *a_pstItem);

static size_t tdr_get_name_size_i(scew_element *a_pstItem);
///////////////////////////////////////////////////////////////////

void tdr_init_lib_param_i(LPTDRLIBPARAM a_pstParam)
{
    assert(NULL != a_pstParam);

    memset(a_pstParam, 0, sizeof(TDRLIBPARAM));

    a_pstParam->iID = TDR_INVALID_ID;
    a_pstParam->iVersion = TDR_INVALID_VERSION;
    a_pstParam->iTagSetVersion = TDR_SUPPORTING_MAX_XMLTAGSET_VERSION;  /*ȱʡ�����°汾���н���*/
}


int tdr_get_lib_param_i(LPTDRLIBPARAM a_pstParam, IN scew_tree* a_pstTree, IN int a_iTagsVersion, IN FILE *a_fpError)
{
    int iRet = TDR_SUCCESS;

    assert(NULL != a_pstParam);
    assert(NULL != a_pstTree);
    assert(NULL != a_fpError);

    /*assert((TDR_SUPPORTING_MIN_XMLTAGSET_VERSION <= a_iTagsVersion) &&
        (a_iTagsVersion <= TDR_SUPPORTING_MAX_XMLTAGSET_VERSION));*/

    tdr_init_lib_param_i(a_pstParam);

    iRet = tdr_add_lib_param_i(a_pstParam, a_pstTree, a_iTagsVersion, a_fpError);
    if (TDR_ERR_IS_ERROR(iRet))
    {
        return iRet;
    }

    if ((a_pstParam->iMaxMacros <= 0) && (a_pstParam->iMaxMetas <= 0))
    {
        fprintf(a_fpError, "error:\t ��XMLԪ������û�ж����κκ�Ԫ�غ�union/structԪ��\n");

        return TDR_ERRIMPLE_INVALID_METALIB_PARAM;
    }


    return TDR_SUCCESS;
}

int tdr_add_lib_param_i(LPTDRLIBPARAM a_pstParam, IN scew_tree* a_pstTree, IN int a_iTagsVersion, IN FILE *a_fpError)
{
    int iRet = 0;
    scew_element* pstRoot;

    assert(NULL != a_pstParam);
    assert(NULL != a_pstTree);
    assert(NULL != a_fpError);
    /*assert((TDR_SUPPORTING_MIN_XMLTAGSET_VERSION <= a_iTagsVersion) &&
        (a_iTagsVersion <= TDR_SUPPORTING_MAX_XMLTAGSET_VERSION));*/


    /*ȡ����Ԫ�ؽ��з���*/
    pstRoot = scew_tree_root( a_pstTree );
    if (NULL == pstRoot )
    {
        fprintf(a_fpError, "error:\t��XMLԪ������û�и�Ԫ��\n");

        return TDR_ERRIMPLE_NO_XML_ROOT;
    }


    /*��ȡ��Ԫ�ص�����ֵ*/
    iRet = tdr_get_metalib_root_attribte_i(a_pstParam, pstRoot, a_iTagsVersion, a_fpError);


     /*���a_pstTree�ĸ�Ԫ���ǲ��Ƿ���Ԫ���������﷨*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_check_metalib_root_i(pstRoot, a_pstParam->iTagSetVersion, a_fpError);
    }


    /*��ȡmetalib����Ԫ����Ϣ��������Ԫ���ݿ���Ҫ���ڴ��С*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_get_metalib_size_i(a_pstParam, pstRoot, a_fpError);
    }


    return iRet;

}

int tdr_get_metalib_root_attribte_i(INOUT LPTDRLIBPARAM a_pstParam, IN scew_element *a_pstRoot, IN int a_iTagsVersion, IN FILE *a_fpError)
{
    int iRet = TDR_SUCCESS;

    /*��ȡmetalib��tagsveriosn����*/
    iRet = tdr_get_metalib_tagsversion_i(&a_pstParam->iTagSetVersion, a_pstRoot, a_iTagsVersion);
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        /*����ǩ���汾*/
        if((TDR_SUPPORTING_MIN_XMLTAGSET_VERSION > a_pstParam->iTagSetVersion) ||
            (a_pstParam->iTagSetVersion > TDR_SUPPORTING_MAX_XMLTAGSET_VERSION))
        {
            fprintf(a_fpError, "error:\t��֧�ֵ�Ԫ��������XML��ǩ���汾<%d>��Ŀǰ֧�ֵ���С�汾��: %d, ���汾����: %d,",
                    a_pstParam->iTagSetVersion, TDR_SUPPORTING_MIN_XMLTAGSET_VERSION, TDR_SUPPORTING_MAX_XMLTAGSET_VERSION);
            fprintf(a_fpError, "���鴫��İ汾������XML��Ϣ��Ԫ�ص�tagsetversion����.\n");
            iRet = TDR_ERRIMPLE_INVALID_TAGSET_VERSION;
        }
    }

    /*��ȡԪ���ݿ���������*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_metalib_get_root_name_i(a_pstParam, a_pstRoot, a_fpError);
    }

    /*��ȡmetalib��version����*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        int iVersion = TDR_INVALID_VERSION;

        iRet = tdr_get_metalib_version_i(&iVersion, a_pstRoot);
        if ((TDR_XML_TAGSET_VERSION_1 <= a_iTagsVersion) &&
            (TDR_ERR_IS_ERROR(iRet)))
        {
            /**��tagsversion1��ʼ����ָ��version����*/
            fprintf(a_fpError, "Error:\t ��Ԫ��������XML��ǩ���汾%d��ʼ����Ԫ�ر���ָ��version���ԡ�\n",
                TDR_XML_TAGSET_VERSION_1 );

            return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_VERSION);
        }else if (!TDR_ERR_IS_ERROR(iRet))
        {
            if (iVersion > a_pstParam->iVersion)
            {
                a_pstParam->iVersion = iVersion;
            }
        }
    }/*if (TDR_SUCCESS == iRet)*/

    /**��ȡmetalib��ID����*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_metalib_get_id_i(a_pstParam, a_pstRoot);
    }

    return iRet;
}

int tdr_check_metalib_root_i(IN scew_element* pstRoot, IN int iTagsVersion, IN FILE *fpError)
{
    const char *pszExpectedName = NULL;
    const char *pstTagName = NULL;

    assert(NULL != pstRoot);
    assert(NULL != fpError);

    assert((TDR_SUPPORTING_MIN_XMLTAGSET_VERSION <= iTagsVersion) &&
        (iTagsVersion <= TDR_SUPPORTING_MAX_XMLTAGSET_VERSION));


    TDR_GET_METALIB_ROOT_TAG(pszExpectedName, iTagsVersion);
    assert(NULL != pszExpectedName);

    pstTagName = scew_element_name(pstRoot);
    assert(NULL != pstTagName);

    /*������ϱ�ǩ���汾���壬���ǩ��Ӧ�����*/
    if (0 != strcmp(pstTagName, pszExpectedName))
    {
        fprintf(fpError, "error:\tXMLԪ�����ĸ�Ԫ�ر�ǩ����ȷ,����Ԫ����������ǩ�汾%d�Ķ���: \n", iTagsVersion);
        fprintf(fpError, "��ȷ�ĸ�Ԫ�ر�ǩӦ����:%s ����ʵ�ʵĸ�Ԫ�ر�ǩ��: %s,���������Ԫ��������xml��ǩ���汾�Ƿ���ȷ\n", pszExpectedName, pstTagName);

        return TDR_ERRIMPLE_INVALID_XML_ROOT;
    }

    return TDR_SUCCESS;
}


int tdr_metalib_get_root_name_i(INOUT LPTDRLIBPARAM pstParam, IN scew_element *pstRoot, IN FILE *fpError)
{
    scew_attribute *pstAttr = NULL;
    const char *pszName = NULL;


    assert(NULL != pstParam);
    assert(NULL != pstRoot);
    assert(NULL != fpError);

    pstAttr = scew_attribute_by_name(pstRoot, TDR_TAG_NAME);
    if ((NULL == pstAttr) && (TDR_XML_TAGSET_VERSION_1 <= pstParam->iTagSetVersion))
    {
        /**û��name����*/
        fprintf(fpError, "error:\t ��Ԫ����������ǩ���汾%d��ʼ����Ԫ�ر���ָ��name����\n", TDR_XML_TAGSET_VERSION_1);

        return TDR_ERRIMPLE_METALIB_ROOT_NO_NAME;
    }

    pszName = scew_attribute_value(pstAttr);
	if ((pszName[0] == '\0'))
	{
		return TDR_SUCCESS;
	}


	if (pstParam->szName[0] == '\0')
    {
        /*�������л�û�б���name����*/
        TDR_STRNCPY(pstParam->szName, pszName, sizeof(pstParam->szName));
    }else if (0 != strcmp(pstParam->szName, pszName))
    {
        /*
        ��֮ǰ��XML�ļ����Ѿ���ȡ��name���ԣ����XMLԪ������������ͬ���ֲ��ܼӵ�Ԫ�ؿ���
        ����ֵ���ִ�Сд*/

        fprintf(fpError, "error:\t���ܽ���ͬ���ֿռ��Ԫ���������ӵ�ͬһ��Ԫ���ݿ���, ������XML��Ϣ�и�Ԫ�ص�name����ֵ�Ƿ���ͬ:\n");
        fprintf(fpError, "\t\t�Ѿ�������<name = %s>���ֿռ��Ԫ��������������Ҫ����<name = %s>���ֿռ�\n", pstParam->szName, pszName);

        return TDR_ERRIMPLE_NAMESPACE_CONFLICT;
    }/*if (0 != tdr_stricmp(pstParam->szName, pszName))*/


    return TDR_SUCCESS;
}


int tdr_metalib_get_id_i(INOUT LPTDRLIBPARAM pstParam, IN scew_element *pstRoot)
{
    scew_attribute *pstAttr = NULL;
    int iId = TDR_INVALID_ID;


    assert(NULL != pstParam);
    assert(NULL != pstRoot);


    pstAttr = scew_attribute_by_name(pstRoot, TDR_TAG_ID);
    if (NULL == pstAttr)
    {
        /**û��ID����*/
       return TDR_SUCCESS;
    }

    iId = atoi(scew_attribute_value(pstAttr));
	pstParam->iID = iId;

    return TDR_SUCCESS;
}


int tdr_get_metalib_tagsversion_i(OUT int *a_piTagVersion, IN scew_element *pstRoot, IN int iTagsVersion)
{
    scew_attribute *pstAttr = NULL;

    assert(NULL != a_piTagVersion);
    assert(NULL != pstRoot);


    pstAttr = scew_attribute_by_name(pstRoot, TDR_TAG_TAGSET_VERSION);
    if (NULL == pstAttr)
    {
        /*�ļ���û��ָ�� ��ǩ���汾���Բ����д���İ汾Ϊ׼*/
       *a_piTagVersion = iTagsVersion;
    }else
    {
        *a_piTagVersion = (int)strtol(scew_attribute_value(pstAttr), NULL, 0);
    }

    return TDR_SUCCESS;
}

int tdr_get_metalib_size_i(INOUT LPTDRLIBPARAM pstParam, IN scew_element *pstRoot, IN FILE *fpError)
{
    int iRet = TDR_SUCCESS;
    size_t iTotalSize = 0;

    assert(NULL != pstParam);
    assert(NULL != pstRoot);
    assert(NULL != fpError);

    if (TDR_XML_TAGSET_VERSION_0 == pstParam->iTagSetVersion)
    {
        iRet = tdr_get_metalib_ver0_size_i(pstParam, pstRoot, fpError);
    }else if (TDR_XML_TAGSET_VERSION_1 == pstParam->iTagSetVersion)
    {
        iRet = tdr_get_metalib_ver1_size_i(pstParam, pstRoot, fpError);
    }

    if (TDR_ERR_IS_ERROR(iRet))
    {
        return iRet;
    }

    //��������metalib�Ĵ�С
    iTotalSize = pstParam->iStrBufSize + pstParam->iMetaSize;

    iTotalSize += TDR_CALC_MIN_SIZE(pstParam->iMaxMetas, pstParam->iMaxMacros);

	/*����궨����������ڴ�ռ�����*/
	iTotalSize += TDR_CALC_MACROSGROUP_MAP_SIZE(pstParam->iMaxMacrosGroupNum);
	iTotalSize += pstParam->iMacrosGroupSize;

    pstParam->iSize = iTotalSize;
    return iRet;

}

int tdr_get_metalib_ver0_size_i(INOUT LPTDRLIBPARAM pstParam, IN scew_element *pstRoot, IN FILE *fpError)
{
    scew_element *pstSubElement = NULL;


    int iRet = TDR_SUCCESS;
    size_t iTotalMetaSize = 0;
    size_t iTotalStrBufSize = 0;

    assert(NULL != pstParam);
    assert(NULL != pstRoot);
    assert(NULL != fpError);

    pstSubElement = scew_element_next(pstRoot, NULL);
    while (NULL != pstSubElement)
    {
        if( 0==tdr_stricmp(scew_element_name(pstSubElement), TDR_TAG_MACROS) )
        {
            /*�궨��*/
            scew_element *pstItem = NULL;

            pstItem = scew_element_next(pstSubElement, NULL);
            while (NULL != pstItem)
            {
                if (0 == tdr_stricmp(scew_element_name(pstItem), TDR_TAG_MACRO))
                {
                     pstParam->iMaxMacros++;

					 /*desc*/
					 pstParam->iStrBufSize += tdr_get_description_size_i(pstItem);

                     pstParam->iStrBufSize += tdr_get_name_size_i(pstItem);
                }else
                {
					fprintf(fpError, "warning:��Ч�ĺ궨���ǩ<%s>,��ʹ�ñ�ǩ<%s>�������\n", scew_element_name(pstItem), TDR_TAG_MACRO);
                }
                pstItem = scew_element_next(pstSubElement, pstItem);
            }/*while (NULL != pstItem)*/
        }else if (0 == tdr_stricmp(scew_element_name(pstSubElement), TDR_TAG_COMPOSITE_TYPE) )
        {
            size_t iMetaSize = 0;
            size_t iStrBufSize = 0;
            int iEntrisNum = 0;

            /*����˽ṹ�� struct/union*/
            iRet = tdr_get_meta_size_i(&iEntrisNum, &iMetaSize, &iStrBufSize, pstParam->iTagSetVersion, pstSubElement, pstRoot);
            if (TDR_ERR_IS_ERROR(iRet))
            {
                return iRet;
            }

            if (0 >= iEntrisNum)
            {
                char szName[TDR_NAME_LEN] = {0};

                tdr_get_name_attribute_i(&szName[0], sizeof(szName), pstSubElement);
                fprintf(fpError, "\nerror:\t �Զ������͡�name = %s> û�а����κ�\"item\"��Ԫ�س�Ա", szName);
                iRet = TDR_ERRIMPLE_META_NO_ENTRY;
                break;
            }


            pstParam->iMaxMetas++;
            iTotalMetaSize += iMetaSize;
            iTotalStrBufSize += iStrBufSize;
        }//if( 0==tdr_stricmp(scew_element_name(pstSubElement), TDR_TAG_MACROS) )

        pstSubElement =	scew_element_next( pstRoot, pstSubElement );
    }

    pstParam->iMetaSize += iTotalMetaSize;
    pstParam->iStrBufSize += iTotalStrBufSize;

    return iRet;
}

scew_attribute *tdr_get_primarykey_by_depended_strut_i(scew_element *a_pstElement, scew_element *a_pstRoot)
{
	scew_attribute *pstAttr = NULL;
	char szStructName[TDR_NAME_LEN] = {0};
	char szName[TDR_NAME_LEN] = {0};
	scew_element *pstSubItem;

	assert(NULL != a_pstElement);
	assert(NULL != a_pstRoot);

	pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_DEPENDONSTRUCT);
	if (NULL == pstAttr)
	{
		return NULL;
	}

	tdr_normalize_string(&szStructName[0], sizeof(szStructName), scew_attribute_value(pstAttr));

	/*���������Ľṹ*/
	pstSubItem = scew_element_next(a_pstRoot, NULL);
	while( NULL != pstSubItem )
	{
		if (0 != strcmp(scew_element_name(pstSubItem), TDR_TAG_STRUCT))
		{
			pstSubItem = scew_element_next( a_pstRoot, pstSubItem );
			continue;
		}
		pstAttr = scew_attribute_by_name(pstSubItem, TDR_TAG_NAME);
		if (NULL == pstAttr)
		{
			pstSubItem = scew_element_next( a_pstRoot, pstSubItem );
			continue;
		}

		tdr_normalize_string(&szName[0], sizeof(szName), scew_attribute_value(pstAttr));
		if (0 == strcmp(szName, szStructName))
		{
			pstAttr = scew_attribute_by_name(pstSubItem, TDR_TAG_PRIMARY_KEY);
			break;
		}
		pstSubItem = scew_element_next( a_pstRoot, pstSubItem );
	}

	if (pstSubItem == NULL)
	{
		return NULL;
	}

	return pstAttr;
}

int tdr_get_index_column_count_i(scew_element *a_pstElement, scew_element *a_pstRoot)
{
	scew_attribute *pstAttr = NULL;
	char *pszColumn;
	int iCount;

	assert(NULL != a_pstElement);
	assert(NULL != a_pstRoot);

	pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_INDEX_COLUMN);
	if (NULL == pstAttr)
	{
        return 0;
	}

	pszColumn = strdup(scew_attribute_value(pstAttr));
    iCount = tdr_get_str_groups_num_i(pszColumn, ',');
    free(pszColumn);

	return iCount;
}

int tdr_get_meta_primaykey_size_i(scew_element *a_pstElement, scew_element *a_pstRoot)
{
	scew_attribute *pstAttr = NULL;
	char *pszKey;
	char *pchDot;
	char *pchBegin;
	int iCount;

	assert(NULL != a_pstElement);
	assert(NULL != a_pstRoot);

	pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_PRIMARY_KEY);
	if (NULL == pstAttr)
	{
		/*�鿴�˽ṹ��������Ϣ�Ƿ������������ṹ*/
		pstAttr = tdr_get_primarykey_by_depended_strut_i(a_pstElement, a_pstRoot);
		if (NULL == pstAttr)
		{
			return 0;
		}
	}

	pszKey = strdup(scew_attribute_value(pstAttr));

	iCount = 0;
	pchBegin = pszKey;
	pchDot = strchr(pchBegin, ',');
	while (NULL != pchDot)
	{
		*pchDot = '\0';
		tdr_trim_str(pchBegin);
		if (*pchBegin != '\0')
		{
			iCount++;
		}
		pchDot++;
		pchBegin = pchDot;
		pchDot = strchr(pchBegin, ',');
	}

	tdr_trim_str(pchBegin);
	if (*pchBegin != '\0')
	{
		iCount++;
	}

	free(pszKey);

	return iCount * sizeof(TDRDBKEYINFO);
}

int tdr_get_str_groups_num_i(char* a_pszTarget, const char a_chSep)
{
	char *pchDot;
	char *pchBegin;
	int iCount;

	iCount = 0;
	pchBegin = a_pszTarget;
	pchDot = strchr(pchBegin, a_chSep);
	while (NULL != pchDot)
	{
		*pchDot = '\0';
		tdr_trim_str(pchBegin);
		if (*pchBegin != '\0')
		{
			iCount++;
		}
		pchDot++;
		pchBegin = pchDot;
		pchDot = strchr(pchBegin, a_chSep);
	}

	tdr_trim_str(pchBegin);
	if (*pchBegin != '\0')
	{
		iCount++;
	}

	return iCount;
}

int tdr_get_meta_size_i(int *a_piEntirsNum, size_t *a_piMetaSize, size_t *a_piStrBufSize, IN int a_iTagversion,
						scew_element *a_pstElement, scew_element *a_pstRoot)
{
    int iRet = TDR_SUCCESS;
    int iEntryNum = 0;

    size_t iTotalLen = 0;
	size_t iPrimaryKeySize = 0;
	const char *pszItemName;
    scew_element *pstSubItem = NULL;

    /* for_index */
    const char *pszElementName = NULL;
    int iIndexesNum = 0;
    int iColumnNum = 0;
    int iStrLen = 0;
    /* for_index */

    assert(NULL != a_piEntirsNum);
    assert(NULL != a_piMetaSize);
    assert(NULL != a_piStrBufSize);
    assert(NULL != a_pstElement);
    assert((TDR_XML_TAGSET_VERSION_0 <= a_iTagversion) && (a_iTagversion <= TDR_XML_TAGSET_VERSION_1));

    iTotalLen += tdr_get_name_size_i(a_pstElement);


     /*�ӱ�ǩ���汾1��ʼ������ cname��defaultvalue����ֵ���������ַ����������ַ�����������*/
    if (TDR_XML_TAGSET_VERSION_1 <= a_iTagversion)
    {
        /*cname*/
		iTotalLen += tdr_get_cname_size_i(a_pstElement);


        /*desc*/
		iTotalLen += tdr_get_description_size_i(a_pstElement);


		/*��ȡ����������Ϣ*/
		iPrimaryKeySize = tdr_get_meta_primaykey_size_i(a_pstElement, a_pstRoot);


    }

    /**�����Զ������͸���Ա���ַ�����Ϣ����Ŀռ䣬Ŀǰ������ַ��������У� name, cname,desc
     *default value. �ڼ����ַ�����ռ�ռ��Сʱ�Ὣ�䳤�ȼ�1���Ա�������ַ�*/
    TDR_GET_ENTRY_TAG_NAME(pszItemName, a_iTagversion);
    pstSubItem = scew_element_next(a_pstElement, NULL);
    iEntryNum = 0;
    while (NULL != pstSubItem)
    {
        if( 0 == tdr_stricmp(scew_element_name(pstSubItem), pszItemName) )
        {

        iEntryNum++;
		iTotalLen += tdr_get_description_size_i(pstSubItem);

        iTotalLen += tdr_get_name_size_i(pstSubItem);


        /*�ӱ�ǩ���汾1��ʼ������ cname��defaultvalue����ֵ*/
        if (TDR_XML_TAGSET_VERSION_1 <= a_iTagversion)
        {
            /*cname*/
			iTotalLen += tdr_get_cname_size_i(pstSubItem);


            /*defalut*/
			iTotalLen += tdr_get_entry_defaultvalue_size_i(pstSubItem);

                iTotalLen += tdr_get_entry_customattr_size_i(pstSubItem);
            }
        }

        pstSubItem = scew_element_next(a_pstElement, pstSubItem);
    }


    *a_piEntirsNum = iEntryNum;
    *a_piMetaSize = TDR_CALC_MIN_META_SIZE(iEntryNum) + iPrimaryKeySize;
    *a_piStrBufSize = iTotalLen;

    /* for_index */
    pszElementName = scew_element_name(a_pstElement);
    if (0 == tdr_stricmp(pszElementName, TDR_TAG_STRUCT))
    {
        iIndexesNum = 0;
        iColumnNum = 0;
        iStrLen = 0;
        iRet = tdr_get_meta_index_info_i(&iIndexesNum, &iColumnNum, &iStrLen, a_pstElement, a_pstRoot);
        if (!TDR_ERR_IS_ERROR(iRet) && 0 < iIndexesNum)
        {
            *a_piMetaSize += sizeof(TDRMETAINDEX)*iIndexesNum;
            *a_piMetaSize += sizeof(TDRCOLUMNINFO)*iColumnNum;
            *a_piStrBufSize += iStrLen;
        }
    }
    /* for_index */

    return iRet;
}

int tdr_get_meta_index_info_i(int *a_piIndexNum, int *a_piColumnNum, int *a_piStrLen,
                              scew_element *a_pstElement, scew_element *a_pstRoot)
{
    int iRet = TDR_SUCCESS;
    scew_element *pstSubItem = NULL;
    int iIndexNum = 0;
    int iColumnNum = 0;
    int iStrLen = 0;


    assert(NULL != a_piIndexNum);
    assert(NULL != a_piColumnNum);
    assert(NULL != a_piStrLen);
    assert(NULL != a_pstElement);
    assert(NULL != a_pstRoot);

    pstSubItem = scew_element_next(a_pstElement, NULL);
    while (NULL != pstSubItem)
    {
        int iCurIndexColumnNum = 0;

        if(0 == tdr_stricmp(scew_element_name(pstSubItem), TDR_TAG_INDEX))
        {
            iIndexNum++;
            iCurIndexColumnNum = tdr_get_index_column_count_i(pstSubItem, a_pstRoot);
            if (0 >= iCurIndexColumnNum)
            {
                fprintf(stderr, "error: ����<name=%s>��column����ֵ����ȷ�������������һ����Ч��<entry>Ԫ��\n",
                        scew_attribute_value(scew_attribute_by_name(pstSubItem, TDR_TAG_NAME)));
                iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_INDEXCOLUMN);
                break;
            }
            iColumnNum += tdr_get_index_column_count_i(pstSubItem, a_pstRoot);
            iStrLen += tdr_get_name_size_i(pstSubItem);
        }

        pstSubItem = scew_element_next(a_pstElement, pstSubItem);
    }

    if (!TDR_ERR_IS_ERROR(iRet) && 0 < iIndexNum)
    {
        *a_piIndexNum  = iIndexNum;
        *a_piColumnNum = iColumnNum;
        *a_piStrLen    = iStrLen;
    }

    return iRet;
}

size_t tdr_get_entry_customattr_size_i(scew_element *a_pstItem)
{
	scew_attribute *pstAttr;
	const char *pszValue;
	size_t iSize = 0;


	assert(NULL != a_pstItem);


	pstAttr = scew_attribute_by_name(a_pstItem, TDR_TAG_CUSTOMATTR);
	if (NULL == pstAttr)
	{
		return 0;
	}
	pszValue = scew_attribute_value(pstAttr);
	if (NULL == pszValue)
	{
		return 0;
	}


	iSize = strlen(pszValue) + 1;


	return iSize;
}

size_t tdr_get_entry_defaultvalue_size_i(scew_element *a_pstItem)
{
	scew_attribute *pstAttr;
	scew_attribute *pszTypeAttr;
	const char *pszTypeName;
	int idxType;
	LPTDRCTYPEINFO pstTypeInfo;
	size_t iSize = 0;
	char szType[1024] = {0};


	assert(NULL != a_pstItem);


	pstAttr = scew_attribute_by_name(a_pstItem, TDR_TAG_DEFAULT_VALUE);
	if (NULL == pstAttr)
	{
		return 0;
	}

	pszTypeAttr = scew_attribute_by_name(a_pstItem, TDR_TAG_TYPE);
	if (NULL == pszTypeAttr)
	{
		return 0;
	}
	tdr_normalize_string(&szType[0], sizeof(szType), scew_attribute_value(pszTypeAttr));
	pszTypeName = &szType[0];
	idxType = tdr_typename_to_idx(pszTypeName);
	if (TDR_INVALID_INDEX == idxType)
	{
		return 0;
	}

	pstTypeInfo = tdr_idx_to_typeinfo(idxType);
	if (TDR_TYPE_COMPOSITE >= pstTypeInfo->iType)
	{
		return 0;
	}

	if (TDR_TYPE_STRING > pstTypeInfo->iType)
	{
		iSize = pstTypeInfo->iSize;
	}else if (TDR_TYPE_STRING == pstTypeInfo->iType)
	{
		iSize = strlen(scew_attribute_value(pstAttr)) + 1;
	}else if (TDR_TYPE_WSTRING == pstTypeInfo->iType)
	{
		iSize = (strlen(scew_attribute_value(pstAttr)) + 1)*sizeof(tdr_wchar_t);
	}


	return iSize;
}

int tdr_get_metalib_ver1_size_i(INOUT LPTDRLIBPARAM pstParam, IN scew_element *pstRoot, IN FILE *fpError)
{
    scew_element *pstSubElement = NULL;
    int iRet = TDR_SUCCESS;
    size_t iTotalMetaSize = 0;
    size_t iTotalStrBufSize = 0;


    assert(NULL != pstParam);
    assert(NULL != pstRoot);
    assert(NULL != fpError);

    pstSubElement = scew_element_next(pstRoot, NULL);
    while (NULL != pstSubElement)
    {
         const char *pszName = NULL;

        pszName = scew_element_name(pstSubElement);
		assert(NULL != pszName);

		if ( 0==tdr_stricmp(pszName, TDR_TAG_MACRO) )
        {
            /*�궨��*/
            pstParam->iMaxMacros++;

			/*desc*/
			iTotalStrBufSize += tdr_get_description_size_i(pstSubElement);

            iTotalStrBufSize += tdr_get_name_size_i(pstSubElement);

        }

		if (0 == tdr_stricmp(pszName, TDR_TAG_MACROSGROUP))
		{
			int iMacroNum = 0;
			size_t iMacrosGroupSize = 0;
			size_t iStrBuffSize = 0;
			tdr_get_macrosgroup_size_i(&iMacroNum, &iMacrosGroupSize, &iStrBuffSize, pstSubElement);
			pstParam->iMaxMacros += iMacroNum;
			pstParam->iMacrosGroupSize += iMacrosGroupSize;
			iTotalStrBufSize += iStrBuffSize;
			pstParam->iMaxMacrosGroupNum++;
		}/*if (0 == tdr_stricmp(pszName, TDR_TAG_MACROSGROUP))*/

        if ((0 == tdr_stricmp(pszName, TDR_TAG_STRUCT)) ||
            (0 == tdr_stricmp(pszName, TDR_TAG_UNION)))
        {
            size_t iMetaSize = 0;
            size_t iStrBufSize = 0;
            int iEntrisNum = 0;

            /*����˽ṹ�� struct/union*/
            iRet = tdr_get_meta_size_i(&iEntrisNum, &iMetaSize, &iStrBufSize, pstParam->iTagSetVersion, pstSubElement, pstRoot);
            if (TDR_ERR_IS_ERROR(iRet))
            {
                return iRet;
            }

            if (0 >= iEntrisNum)
            {
                char szName[TDR_NAME_LEN] = {0};

                tdr_get_name_attribute_i(&szName[0], sizeof(szName), pstSubElement);
                fprintf(fpError, "error:\t �Զ������͡�name = %s> û�а����κ�\"entry\"��Ԫ�س�Ա.\n", szName);
                iRet = TDR_ERRIMPLE_META_NO_ENTRY;
                break;
            }

            pstParam->iMaxMetas++;
            iTotalMetaSize += iMetaSize;
            iTotalStrBufSize += iStrBufSize;
        }//if( 0==tdr_stricmp(scew_element_name(pstSubElement), TDR_TAG_MACROS) )

        pstSubElement =	scew_element_next( pstRoot, pstSubElement );
    }

    pstParam->iMetaSize += iTotalMetaSize;
    pstParam->iStrBufSize += iTotalStrBufSize;

    return iRet;
}

int tdr_get_metalib_version_i(OUT int *a_plVersion, IN scew_element* a_pstRoot)
{
    scew_attribute *pstAttr = NULL;

    assert(NULL != a_plVersion);
    assert(NULL != a_pstRoot);



    *a_plVersion = TDR_INVALID_VERSION;

    pstAttr = scew_attribute_by_name(a_pstRoot, TDR_TAG_VERSION);
    if (NULL != pstAttr)
    {
        *a_plVersion = (int)strtol(scew_attribute_value(pstAttr), NULL, 0);
    }

    return TDR_SUCCESS;
}

/**����description����ֵ������ڴ�ռ��ֽ���
*/
size_t tdr_get_description_size_i(scew_element *a_pstItem)
{
	size_t iStrLen = 0;
	scew_attribute *pstAttr = NULL;

	assert(NULL != a_pstItem);

	pstAttr = scew_attribute_by_name(a_pstItem, TDR_TAG_DESCIPTION);
	if (NULL != pstAttr)
	{
		iStrLen = strlen(scew_attribute_value(pstAttr)) + 1;
		iStrLen = TDR_MIN(iStrLen, TDR_DESC_LEN); /*����Ԥ����󳤶Ⱦͽض�*/
	}

	return iStrLen;
}

/**����name����ֵ������ڴ�ռ��ֽ���
*/
size_t tdr_get_name_size_i(scew_element *a_pstItem)
{
    size_t iStrLen = 0;
    scew_attribute *pstAttr = NULL;

    assert(NULL != a_pstItem);

    pstAttr = scew_attribute_by_name(a_pstItem, TDR_TAG_NAME);
    if (NULL != pstAttr)
    {
        iStrLen = strlen(scew_attribute_value(pstAttr)) + 1;
    }

    return iStrLen;
}


/**����cname����ֵ������ڴ�ռ��ֽ���
*/
size_t tdr_get_cname_size_i(scew_element *a_pstItem)
{
	size_t iStrLen = 0;
	scew_attribute *pstAttr = NULL;

	assert(NULL != a_pstItem);

	pstAttr = scew_attribute_by_name(a_pstItem, TDR_TAG_CNNAME);
	if (NULL != pstAttr)
	{
		iStrLen = strlen(scew_attribute_value(pstAttr)) + 1;
		iStrLen = TDR_MIN(iStrLen, TDR_CHNAME_LEN); /*����Ԥ����󳤶Ⱦͽض�*/
	}

	return iStrLen;
}

/**��ȡ�����궨���������ڴ�ռ�
*@param[out] a_piMacrosNum ��ȡ�˺궨���������macro��Ŀ
*@param[out] a_piGroupSize ��ȡ�˺궨����Ԫ�ش洢��ռ�õ��ֽ���
*@param[out] a_piStrBufSize ��ȡ��metaԪ�����ַ�������������ռ���ֽ���
*@param[in] a_pstElement �迼���Ԫ��
*/
int tdr_get_macrosgroup_size_i(int *a_piMacrosNum, size_t *a_piGroupSize, size_t *a_piStrBufSize, scew_element *a_pstElement)
{
	int iMacroNum = 0;
	size_t iStrLen = 0;

	scew_element *pstSubItem = NULL;


	assert(NULL != a_piMacrosNum);
	assert(NULL != a_piGroupSize);
	assert(NULL != a_piStrBufSize);
	assert(NULL != a_pstElement);


	iStrLen += tdr_get_description_size_i(a_pstElement);


	/*����˺궨���������macro����������ռ�*/
	pstSubItem = scew_element_next(a_pstElement, NULL);
	while (NULL != pstSubItem)
	{
		if( 0 != tdr_stricmp(scew_element_name(pstSubItem), TDR_TAG_MACRO) )
		{
			pstSubItem = scew_element_next(a_pstElement, pstSubItem);
			continue;
		}

		iMacroNum++;
		iStrLen += tdr_get_description_size_i(pstSubItem);
        iStrLen += tdr_get_name_size_i(pstSubItem);

		pstSubItem = scew_element_next(a_pstElement, pstSubItem);
	}


	*a_piMacrosNum = iMacroNum;
	*a_piGroupSize = TDR_CALC_MIN_MACROSGROUP_SIZE(iMacroNum);
	*a_piStrBufSize = iStrLen;
	return TDR_SUCCESS;
}

