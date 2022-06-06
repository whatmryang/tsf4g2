/**
*
* @file     tdr_metalib_manage.c
* @brief    TSF4G-Ԫ������������Ĺ���ģ��
*
* @author steve jackyai
* @version 1.0
* @date 2007-03-22
*
*
* Copyright (c)  2007, ��Ѷ�Ƽ����޹�˾���������з���
* All rights reserved.
*
*/

#include <assert.h>
#include <stdlib.h>

#include "tdr/tdr_os.h"
#include "tdr/tdr_metalib_manage_i.h"
#include "tdr/tdr_error_i.h"
#include "tdr_XMLtags_i.h"
#include "tdr/tdr_ctypes_info_i.h"
#include "tdr_metalib_param_i.h"
#include "tdr/tdr_define_i.h"
#include "tdr/tdr_auxtools.h"
#include "tdr_metalib_entry_manage_i.h"

#include "tdr_metalib_meta_manage_i.h"

#if defined (_WIN32) || defined (_WIN64)
#pragma warning(disable:4996)
#endif

static void tdr_init_meta_data_i(LPTDRMETA a_pstMeta, LPTDRMETALIB a_pstLib, size_t a_iMetaSize, int a_iEntryNum);






/**��ȡmeta�ṹ��type��Ϣ
*/
static int tdr_get_meta_type_i(LPTDRMETA a_pstMeta, LPTDRMETALIB a_pstLib, scew_element *a_pstElement, FILE *a_fpError);

/**��ȡ structԪ�ص�size����
*/
static int tdr_get_struct_meta_size_attribute_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError);

/**��ȡstructԪ�ص��ֽڶ���align ����
*/
static int tdr_get_struct_meta_align_attribute_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError);

/**��ȡstructԪ�ص�versionindicator����
*/
static int tdr_get_stuct_meta_versionindicator_attribute_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError);

/**��ȡstructԪ�ص�sizetype����
*/
static int tdr_get_struct_meta_packsize_attribute_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError, const char *a_pszAttr);

/**��ȡstructԪ�صĽ���ֱ�����
*/
static int tdr_get_struct_meta_splittablefactor_attribute_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError);

static int tdr_get_struct_meta_splittablekey_attribute_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError);

static int tdr_get_struct_meta_splittablerule_attribute_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError);

static int tdr_get_struct_meta_strictinput_attribute_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError);

static int tdr_get_struct_meta_dependonstruct_attribute_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError);

static int tdr_get_struct_meta_uniqueentryname_attribute_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError);


/**��ȡstructԪ�ص���������
*/
static int tdr_get_struct_meta_primarykey_attribute_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError);


/**����meta��ID��metalib�в��Ҵ�meta
*/
LPTDRMETA tdr_get_meta_by_id_i(TDRMETALIB* a_pstLib, int a_iID);



/** ���ԭ�����������Ƿ����㹻�Ŀռ�洢stuct/unionԪ��
*@param[inout] pstLib ��Ҫ���µ�Ԫ���ݿ�
*@param[in] iMetaSize meta��ռ�Ŀռ�
*@param[in] iStrBufSize meta��Ҫռ���ַ�����������С
*@param[in] fpError ������¼���������Ϣ���ļ����
*
*@return �ɹ�����TDR_SUCCESS,���򷵻ش����
*
*@pre \e pstLib ����ΪNULL
*@pre \e fpError ����ΪNULL
*/
static int tdr_check_space_for_meta_i(LPTDRMETALIB a_pstLib, size_t iMetaSize, size_t iStrBufSize, FILE *fpError);

/** ��ȡmeta�Ĺ�������
*@param[out] a_pstMeta    ��Ҫ���µ�meta
*@param[inout] a_pstLib ��Ҫ���µ�Ԫ���ݿ�
*@param[in] a_pstElement ����ӵ�Ԫ��
*@param[in] a_fpError ������¼���������Ϣ���ļ����
*
*@return �ɹ�����TDR_SUCCESS,���򷵻ش����
*
*@pre \e a_pstLib ����ΪNULL
*@pre \e a_pstElement ����ΪNULL
*@pre \e a_fpError ����ΪNULL
*@pre \e ���ô˺���֮ǰ�������ȵ���tdr_get_meta_type_i��ȡmeta������
*/
static int tdr_get_mata_common_attribute_i(LPTDRMETA a_pstMeta, LPTDRMETALIB a_pstLib, scew_element *a_pstElement, FILE *a_fpError);


static int tdr_parse_meta_versionindicator_i(LPTDRMETA a_pstMeta, const char* a_pszName);


/**�����Table��Ӧ��meta�ṹdb��ص������Ƿ�Ϸ�
*/
static int tdr_check_table_meta_dbattributes_i(IN LPTDRMETA a_pstMeta, IN FILE *a_fpError);

/**�������meta�ṹdb��ص�����,�������滯
*/
static int tdr_check_depend_meta_dbattributes_i(IN LPTDRMETA a_pstMeta, IN FILE *a_fpError);


/**��齫struct��Ա��չ���ɽ������ʱ���Ƿ��������ĳ�Ա����
��a_pstCheckMeta�ĳ�Ա��a_pstBaseMeta��ǰiMaxIdx����Ա���бȽ�
*@return 0�� û�������� ����ֵ��������
*/
static int tdr_check_meta_extendable_to_table_i(LPTDRMETA a_pstBaseMeta, int iMaxIdx, LPTDRMETA a_pstCheckMeta, OUT char *pszConflictName);

static int tdr_check_dbtable_name_conflict_i(LPTDRMETA a_pstBaseMeta, int iMaxIdx, const char *pszName);


/**���ݳ�Ա���ڽṹ�в���
*/
static int tdr_find_tablecolumn_entry_i(IN LPTDRDBKEYINFO a_pstDBKeyInfo, IN LPTDRMETA a_pstMeta, IN const char *a_pszName);

/**��ȡmeta��sortkey��Ϣ
*/
static  int tdr_get_meta_sortkey_attribute_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *fpError);

/*ȡ�궨�����������Ѿ�ʹ�õĿռ�*/
static size_t tdr_get_macrogroup_used_size_i(IN LPTDRMETALIB a_pstLib);

static int  tdr_create_macrosgroup_i(INOUT LPTDRMACROSGROUP *a_ppstMacrosGroup, IN TDRMETALIB* a_pstLib, IN scew_element *a_pstElement, IN FILE *a_fpError);

/** ��ȡmacrosgroupԪ�ص�����
*@return �ɹ�����0�����򷵻ط���ֵ
*/
static int tdr_get_macrosgroup_attributes(IN LPTDRMACROSGROUP a_pstMacrosGroup, IN TDRMETALIB* a_pstLib, IN scew_element *a_pstElement, IN FILE *a_fpError);

/** ��macrosgroupԪ�ص�macro��Ԫ�ؼ��뵽Ԫ���ݿ���
*@return �ɹ�����0�����򷵻ط���ֵ
*/
static int tdr_add_macros_of_group_i(IN LPTDRMACROSGROUP a_pstMacrosGroup, IN TDRMETALIB* a_pstLib, IN scew_element *a_pstElement, IN FILE *a_fpError);

static void tdr_sort_macrosgroup_valueidxmap_i(IN LPTDRMACROSGROUP a_pstMacrosGroup, IN TDRMETALIB* a_pstLib);
static void tdr_sort_macrosgroup_nameidxmap_i(IN LPTDRMACROSGROUP a_pstMacrosGroup, IN TDRMETALIB* a_pstLib);

/*������DB��ʹ�õĳ�Ա��ƫ��ֵ*/
static int tdr_parse_entryoff_db_i(IN LPTDRMETA a_pstMeta, OUT LPTDRSelector a_pstSelector,
                                   IN const char *a_pszAttr, IN const char* a_pszName, IN FILE *a_fpError);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int tdr_add_macros_i(TDRMETALIB* a_pstLib, scew_element *a_pstElement, FILE *a_fpError)
{
    scew_element* pstItem = NULL;
    int iMacroNum = 0;
    int iRet = TDR_SUCCESS;
	LPTDRMACRO pstMacro = NULL;

    assert(NULL != a_pstLib);
    assert(NULL != a_pstElement);
    assert(NULL != a_fpError);


    iMacroNum =	scew_element_count(a_pstElement);


    pstItem = scew_element_next(a_pstElement, NULL);
    while (NULL != pstItem)
    {
        if (0 != tdr_stricmp(scew_element_name(pstItem), TDR_TAG_MACRO))
        {
            pstItem = scew_element_next(a_pstElement, pstItem);
            continue;
        }

        if( a_pstLib->iCurMacroNum >= a_pstLib->iMaxMacroNum )
        {

            fprintf(a_fpError, "error:\t metalib����ܴ洢%d���궨�壬Ŀǰ�Ѵ洢%d��,�����ټ���궨��.\n",
                a_pstLib->iMaxMacroNum, a_pstLib->iCurMacroNum);
            iRet =  TDR_ERRIMPLE_TO_MUCH_MACROS;
            break;
        }

        iRet = tdr_add_macro_i(a_pstLib, pstItem, a_fpError, &pstMacro);
        if (TDR_ERR_IS_ERROR(iRet))
        {
            break;
        }

        pstItem = scew_element_next(a_pstElement, pstItem);
    }

    return iRet;
}




int tdr_add_macro_i(TDRMETALIB* a_pstLib, scew_element *a_pstElement, FILE *a_fpError, INOUT LPTDRMACRO *a_ppMacro)
{
    int iRet = TDR_SUCCESS;
    LPTDRMACRO pstMacrosTable = NULL;
    LPTDRMACRO pstMacro = NULL;
    scew_attribute *pstAttr = NULL;
    const char *pszMacroValTag = NULL;
    char szMacro[TDR_MACRO_LEN] = {0};
    size_t iMacroLen = 0;
    int idxFindMacro = TDR_INVALID_INDEX;

    assert(NULL != a_pstLib);
    assert(NULL != a_pstElement);
    assert(NULL != a_fpError);
	assert(NULL != a_ppMacro);

    /*��õ�ǰ���к궨���ĵ�ַ*/
    pstMacrosTable = TDR_GET_MACRO_TABLE(a_pstLib);
    pstMacro = pstMacrosTable + a_pstLib->iCurMacroNum;
    memset(pstMacro, 0, sizeof(TDRMACRO));

    /*��ȡ�궨�������*/
    pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_NAME);
    if ( NULL == pstAttr)
    {
        fprintf(a_fpError, "\nerror:\t ��%d��(����ǰ�濪ʼ)macroԪ��û��name����\n", a_pstLib->iCurMacroNum +1);

        iRet = TDR_ERRIMPLE_MACRO_NO_NAME_ATTR;
    }else
    {
        const char *pszName = scew_attribute_value(pstAttr);
        assert(NULL != pszName);

        if (TDR_MACRO_LEN <= strlen(pszName))
        {
            fprintf(a_fpError, "\nerror:\t ��%d��(����ǰ�濪ʼ)macroԪ�ص�name����(%s)̫����������֧�ֵ�����ַ���(%d)\n",
                a_pstLib->iCurMacroNum +1, pszName, TDR_MACRO_LEN-1);
             iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NAME_LEN_BEYOND_UPLIMIT);
        }else
        {
            iMacroLen = tdr_normalize_string(szMacro, sizeof(szMacro), pszName);
        }
    }/*if ( NULL == pstAttr) */


     /*���궨��name����Ч��*/
    if (!TDR_ERR_IS_ERROR(iRet) && (szMacro[0] == '\0'))
    {

        fprintf(a_fpError, "\nerror:\t ��%d��(����ǰ�濪ʼ)macroԪ�ص�name����ֵΪ�մ�\n", a_pstLib->iCurMacroNum +1);
        iRet = TDR_ERRIMPLE_MACRO_NO_NAME_ATTR;
    }



    /*�鿴�Ƿ��������궨��*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        idxFindMacro = tdr_get_macro_index_by_name_i(a_pstLib, szMacro);
        if (TDR_INVALID_INDEX != idxFindMacro)
        {
             fprintf(a_fpError, "\nwarning:\t  ��%d��(����ǰ�濪ʼ)macro�궨��Ԫ��<name=%s> ��ǰ���macroͬ��,��ǰ���ֵ��ȡ��ǰ�涨���ֵ\n",
                a_pstLib->iCurMacroNum +1, szMacro);
            iRet = TDR_SUCCEESS_WARNING(TDR_SUCWARN_MACRO_NAME_CONFLICT);
            pstMacro = pstMacrosTable + idxFindMacro;
        }else
        {
            pstMacro = pstMacrosTable + a_pstLib->iCurMacroNum;
        }

        if (!TDR_STRBUF_HAVE_FREE_ROOM(a_pstLib, iMacroLen))
        {
            fprintf(a_fpError, "\nwarning:\t Ԫ�����ַ���������ʣ��ռ�<%"TDRPRI_SIZET">�����Դ洢�궨��<name=%s>",
                a_pstLib->iFreeStrBufSize, szMacro);
            iRet = TDR_ERRIMPLE_NO_SPACE_FOR_STRBUF;
        }else
        {
            TDR_COPY_STRING_TO_BUF(pstMacro->ptrMacro, szMacro, iMacroLen, a_pstLib);
        }

    }/*if (!TDR_ERR_IS_ERROR(iRet))*/


    /*��ȡ�궨���ֵ*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        TDR_GET_MACRO_VALUE_TAG(pszMacroValTag, a_pstLib->iXMLTagSetVer);
        assert(NULL != pszMacroValTag);

        pstAttr	= scew_attribute_by_name(a_pstElement, pszMacroValTag);
        if(NULL == pstAttr )
        {
            fprintf(a_fpError, "\nerror:\t  ��%d��(����ǰ�濪ʼ)macro�궨��Ԫ��<name=%s> û�ж���%s����\n",
                a_pstLib->iCurMacroNum +1, TDR_GET_STRING_BY_PTR(a_pstLib,pstMacro->ptrMacro), pszMacroValTag);

            iRet = TDR_ERRIMPLE_MACRO_NO_VALUE;
        }else
        {
			int idx;
			char szMacroValue[TDR_MACRO_LEN];

			tdr_normalize_string(szMacroValue, (int)sizeof(szMacroValue), scew_attribute_value(pstAttr));
			iRet = tdr_get_macro_int_i(&pstMacro->iValue, &idx, a_pstLib, szMacroValue);
			if (TDR_ERR_IS_ERROR(iRet))
			{
				fprintf(a_fpError, "macro�궨��Ԫ��<name=%s> ������ֵ<%s>��Ч,Ŀǰ�궨������ֵ��֧���������Ѷ��ں������ַ�ʽ\n", szMacro, scew_attribute_value(pstAttr));
			}
        }
    } /*if (!TDR_ERR_IS_ERROR(iRet))*/


	/*��ȡ�궨�������ֵ*/
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		iRet = tdr_get_desc_attribute_i(&pstMacro->ptrDesc, a_pstLib, a_pstElement, a_fpError);
	} /*if (!TDR_ERR_IS_ERROR(iRet))*/


    /*û�������ĺ꣬�������궨��ı�*/
    if (!TDR_ERR_IS_ERROR(iRet) && (TDR_INVALID_INDEX == idxFindMacro))
    {
        a_pstLib->iCurMacroNum++;
    }

	*a_ppMacro = pstMacro;

    return iRet;
}




int tdr_check_space_for_meta_i(LPTDRMETALIB a_pstLib, size_t a_iMetaSize, size_t a_iStrBufSize, FILE *a_fpError)
{
    size_t iFreeMataSpace = 0;


    assert(NULL != a_pstLib);
    assert(NULL != a_fpError);

    if (a_pstLib->iCurMetaNum >= a_pstLib->iMaxMetaNum )
    {
        fprintf(a_fpError, "\nerror:\t Ԫ��������������ܴ洢%d���Զ������ͣ�Ŀǰ�Ѿ������%d��",
            a_pstLib->iMaxMetaNum, a_pstLib->iCurMetaNum);

        return TDR_ERRIMPLE_NO_SPACE_FOR_MATA;
    }

    /*���洢meta��ʣ��ռ��Ƿ��㹻*/
    iFreeMataSpace = TDR_GET_FREE_META_SPACE(a_pstLib);
    if (iFreeMataSpace < a_iMetaSize)
    {
        fprintf(a_fpError, "\nerror:\t Ԫ����������洢meta��Ϣʣ��ռ�Ϊ%"TDRPRI_SIZET"�ֽڣ���ʵ����Ҫ%"TDRPRI_SIZET"���ֽ�\n",
            iFreeMataSpace, a_iMetaSize);

        return TDR_ERRIMPLE_NO_SPACE_FOR_MATA;
    }

    /*����ַ�������Ŀռ��Ƿ��㹻*/
    if (a_pstLib->iFreeStrBufSize < a_iStrBufSize)
    {
        fprintf(a_fpError, "\nerror:\t Ԫ�����������ַ���������ʣ��ռ�Ϊ%"TDRPRI_SIZET"�ֽڣ���ʵ����Ҫ%"TDRPRI_SIZET"���ֽ�\n",
            a_pstLib->iFreeStrBufSize, a_iStrBufSize);

        return TDR_ERRIMPLE_NO_SPACE_FOR_STRBUF;
    }

    return TDR_SUCCESS;
}

int tdr_get_mata_common_attribute_i(LPTDRMETA a_pstMeta, LPTDRMETALIB a_pstLib, scew_element *a_pstElement, FILE *a_fpError)
{
    int iRet = 0;
    LPTDRCTYPEINFO pstTypeInfo = NULL;
    LPTDRMETA pstFindedMeta = NULL;
    scew_attribute *pstAttr = NULL;
    LPTDRMETALIB pstLib;

    assert(NULL != a_pstMeta);
    assert(NULL != a_pstLib);
    assert(NULL != a_pstElement);
    assert(NULL != a_fpError);

    pstLib = TDR_META_TO_LIB(a_pstMeta);
    pstTypeInfo = tdr_idx_to_typeinfo(a_pstMeta->idxType);
    assert(NULL != pstTypeInfo);



    /*��ȡname ����*/
    pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_NAME);
    if (NULL != pstAttr)
    {
        const char *pszName = scew_attribute_value(pstAttr);
        if (TDR_NAME_LEN <= strlen(pszName))
        {
            fprintf(a_fpError, "error:�������ݵ�(name:%s)���ִ�̫����Ŀǰ��Ա�����֧��%d���ַ�\n",
                pszName, TDR_NAME_LEN-1);
            iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NAME_LEN_BEYOND_UPLIMIT);

        }else
        {
            char szName[TDR_NAME_LEN];
            size_t iNameLen;
            iNameLen = tdr_normalize_string(szName, sizeof(szName), pszName);
            if (!TDR_STRBUF_HAVE_FREE_ROOM(pstLib, iNameLen))
            {
                fprintf(a_fpError, "error: �ַ�����������ʣ��ռ�(%"TDRPRI_SIZET")���������ܴ洢�������ݳ�Ա��name����(%s)\n",
                    a_pstLib->iFreeStrBufSize,pszName);
                iRet = TDR_ERRIMPLE_NO_SPACE_FOR_STRBUF;
            }else
            {
                TDR_COPY_STRING_TO_BUF(a_pstMeta->ptrName, szName, iNameLen, pstLib);
            }
        }
    }else
    {
        a_pstMeta->ptrName = TDR_INVALID_INDEX;
        fprintf(a_fpError, "error:\t ��%d���Զ���<type =%s>����Ԫ��û�ж���name���Ի���ֵΪ�մ�\n",
            a_pstLib->iCurMetaNum + 1, pstTypeInfo->pszCName);
        iRet = TDR_ERRIMPLE_META_NO_NAME;
    }/* if (NULL != pstAttr)*/


    if ((!TDR_ERR_IS_ERROR(iRet)) && (TDR_XML_TAGSET_VERSION_0 < a_pstLib->iXMLTagSetVer))
    {
        /*�°涨������name���Ե���Ч��*/

        /*�ж��Ƿ�������meta����*/
        pstFindedMeta = tdr_get_meta_by_name_i(a_pstLib, TDR_GET_STRING_BY_PTR(a_pstLib,a_pstMeta->ptrName));
        if (NULL != pstFindedMeta)
        {
            fprintf(a_fpError, "error:\t Ԫ����������<%s>������������<name = %s>��ͬ��%sԪ��\n", a_pstLib->szName,
                TDR_GET_STRING_BY_PTR(a_pstLib,a_pstMeta->ptrName), pstTypeInfo->pszCName);

            iRet =TDR_ERRIMPLE_META_NAME_CONFLICT;
        }

    }/*if ((!TDR_ERR_IS_ERROR(iRet)) && (TDR_XML_TAGSET_VERSION_0 < a_pstLib->iXMLTagSetVer))*/

    /*��ȡID����*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_get_id_attribute_i(&a_pstMeta->iID, &a_pstMeta->idxID, a_pstLib, a_pstElement, a_fpError);
        if (TDR_SUCCESS == iRet)
        {
            TDR_META_SET_HAVE_ID(a_pstMeta);

			if (0 > a_pstMeta->iID)
			{
				fprintf(a_fpError, "error:\t %sԪ��<name = %s>��ID<%d>����Ϊ����\n",
					pstTypeInfo->pszCName, TDR_GET_STRING_BY_PTR(a_pstLib,a_pstMeta->ptrName), a_pstMeta->iID);

				iRet =TDR_ERRIMPLE_META_ID_CONFLICT;
			}else	if ( TDR_XML_TAGSET_VERSION_0 < a_pstLib->iXMLTagSetVer)
			{
					/*ͬ��ڵ�Id�������ظ�*/
				pstFindedMeta = tdr_get_meta_by_id_i(a_pstLib, a_pstMeta->iID);
				if (NULL != pstFindedMeta)
				{
					fprintf(a_fpError, "error:\t %sԪ��<name = %s>��ID<%d>��ǰ�涨���Ԫ����ͬ\n",
						pstTypeInfo->pszCName, TDR_GET_STRING_BY_PTR(a_pstLib,a_pstMeta->ptrName), a_pstMeta->iID);

					iRet =TDR_ERRIMPLE_META_ID_CONFLICT;
				}
			}

        }/*if ((TDR_SUCCESS == iRet) && (TDR_XML_TAGSET_VERSION_0 < a_pstLib->iXMLTagSetVer))*/
    }

    /*version*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_get_version_attribute_i(&a_pstMeta->iBaseVersion, &a_pstMeta->idxVersion, a_pstLib, a_pstElement, a_fpError);
        if ((TDR_SUCCESS != iRet) && (TDR_XML_TAGSET_VERSION_0 < a_pstLib->iXMLTagSetVer))
        {
            fprintf(a_fpError, "\nerror:\t <%s>����<name = %s>�������version���ԣ���ָ�������ͼ���Ԫ���ݿ�ʱ�Ļ�׼�汾\n",
                pstTypeInfo->pszCName, TDR_GET_STRING_BY_PTR(a_pstLib,a_pstMeta->ptrName));

            iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_VERSION);
        }else if (TDR_SUCCESS == iRet)
        {
            a_pstMeta->iCurVersion = a_pstMeta->iBaseVersion;   /*����ǰ�汾������Ϊ��׼�汾*/
        }
    }


    if (!TDR_ERR_IS_ERROR(iRet) && (TDR_XML_TAGSET_VERSION_0 < a_pstLib->iXMLTagSetVer))
    {
        /*desc*/
        iRet = tdr_get_desc_attribute_i(&a_pstMeta->ptrDesc, a_pstLib, a_pstElement, a_fpError);

        if (!TDR_ERR_IS_ERROR(iRet))
        {
            /*cname*/
            iRet = tdr_get_cname_attribute_i(&a_pstMeta->ptrChineseName, a_pstLib, a_pstElement, a_fpError);
        }

		/*align*/
		if (!TDR_ERR_IS_ERROR(iRet))
		{
			iRet = tdr_get_struct_meta_align_attribute_i(a_pstMeta, a_pstElement, a_fpError);
		}

    }

    return iRet;
}





void tdr_init_meta_data_i(LPTDRMETA a_pstMeta, LPTDRMETALIB a_pstLib, size_t a_iMetaSize, int a_iEntryNum)
{
    memset( a_pstMeta, 0 , sizeof(TDRMETA) );

	a_pstMeta->iMemSize = a_iMetaSize;
    a_pstMeta->iType = TDR_TYPE_UNKOWN;
    a_pstMeta->iIdx = a_pstLib->iCurMetaNum;
    a_pstMeta->iMaxSubID = TDR_INVALID_ID;

    a_pstMeta->idxID = TDR_INVALID_INDEX;
	a_pstMeta->iID = TDR_INVALID_INDEX;
    a_pstMeta->idxVersion = TDR_INVALID_INDEX;

    a_pstMeta->iIndexesNum = 0;
    a_pstMeta->ptrIndexes = TDR_INVALID_PTR;

    a_pstMeta->ptrMeta = a_pstLib->ptrLaseMeta;

	a_pstMeta->idxType = TDR_INVALID_INDEX;
    a_pstMeta->ptrDesc = TDR_INVALID_PTR;

    a_pstMeta->ptrChineseName = TDR_INVALID_PTR;
    a_pstMeta->ptrDesc = TDR_INVALID_PTR;

	if (TDR_XML_TAGSET_VERSION_0 == a_pstLib->iXMLTagSetVer)
	{
		a_pstMeta->iBaseVersion = a_pstLib->iVersion;
		a_pstMeta->iCurVersion = a_pstMeta->iBaseVersion ;
	}

    a_pstMeta->iCustomAlign = TDR_DEFAULT_ALIGN_VALUE;
	a_pstMeta->iValidAlign = TDR_DEFAULT_ALIGN_VALUE;
    a_pstMeta->idxCustomHUnitSize = TDR_INVALID_INDEX;

	a_pstMeta->stVersionIndicator.iHOff = TDR_INVALID_OFFSET;
	a_pstMeta->stVersionIndicator.iNOff = TDR_INVALID_OFFSET;

	a_pstMeta->stSizeType.idxSizeType = TDR_INVALID_INDEX;
	a_pstMeta->stSizeType.iHOff = TDR_INVALID_OFFSET;
	a_pstMeta->stSizeType.iNOff = TDR_INVALID_OFFSET;

	a_pstMeta->ptrPrimayKeyBase = TDR_CALC_MIN_META_SIZE(a_iEntryNum);
	a_pstMeta->stSplitTableKey.ptrEntry = TDR_INVALID_PTR;
	a_pstMeta->stSplitTableKey.iHOff = TDR_INVALID_OFFSET;
	a_pstMeta->idxSplitTableFactor = TDR_INVALID_INDEX;

	a_pstMeta->ptrDependonStruct = TDR_INVALID_PTR;

	a_pstMeta->stSortKey.iSortKeyOff = TDR_INVALID_OFFSET;
	a_pstMeta->stSortKey.idxSortEntry = TDR_INVALID_ID;
	a_pstMeta->stSortKey.ptrSortKeyMeta = TDR_INVALID_PTR;
    a_pstMeta->ptrName = TDR_INVALID_PTR;

    a_pstMeta->iReserve1 = TDR_INVALID_PTR;
    a_pstMeta->iReserve2 = TDR_INVALID_PTR;
    a_pstMeta->iReserve3 = TDR_INVALID_PTR;

	TDR_META_CLR_HAVE_ID(a_pstMeta);
}


int tdr_add_meta_base_i(TDRMETALIB* a_pstLib, scew_element *a_pstElement, scew_element *a_pstRoot, FILE *a_fpError)
{
    int iRet = TDR_SUCCESS;
    LPTDRMETA pstFreeMeta = NULL;
    size_t iMetaSize = 0;
    size_t iStrBufSize = 0;
    int iEntrisNum = 0;
    LPTDRMAPENTRY pstMetaMap = NULL;
    LPTDRNAMEENTRY pstMetaNameMap = NULL;
    LPTDRIDENTRY    pstMetaIDMap = NULL;

    assert(NULL != a_pstLib);
    assert(NULL != a_pstElement);
    assert(NULL != a_fpError);

    /*��ȡmeta��size*/
    tdr_get_meta_size_i(&iEntrisNum, &iMetaSize, &iStrBufSize, a_pstLib->iXMLTagSetVer, a_pstElement, a_pstRoot);
    if (0 >= iEntrisNum)
    {
        char szName[TDR_NAME_LEN] = {0};
        const char *pszItemName = NULL;

        TDR_GET_ENTRY_TAG_NAME(pszItemName, a_pstLib->iXMLTagSetVer);
        tdr_get_name_attribute_i(&szName[0], sizeof(szName), a_pstElement);
        fprintf(a_fpError, "\nerror:\t �Զ������͡�name = %s> û�а����κ�\"%s\"��Ԫ�س�Ա", szName, pszItemName);
        iRet = TDR_ERRIMPLE_META_NO_ENTRY;
    }


    /*����Ƿ����㹻�Ŀռ�洢��meta*/
   if (!TDR_ERR_IS_ERROR(iRet))
   {
        iRet = tdr_check_space_for_meta_i(a_pstLib, iMetaSize, iStrBufSize, a_fpError);
   }

    if (!TDR_ERR_IS_ERROR(iRet))
    {
        /*ȡһ������meta�ṹ����ʼ��*/
        pstFreeMeta = (LPTDRMETA) (a_pstLib->data + a_pstLib->ptrLaseMeta);
        tdr_init_meta_data_i(pstFreeMeta, a_pstLib, iMetaSize, iEntrisNum);

        /*��ȡmeta��������Ϣ*/
        iRet = tdr_get_meta_type_i(pstFreeMeta, a_pstLib, a_pstElement, a_fpError);
    }

     /*��ȡ��meta�������Ե�*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_get_mata_common_attribute_i(pstFreeMeta, a_pstLib, a_pstElement, a_fpError);
    }


    /*����meta��ӳ����Ϣ*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        pstMetaMap = TDR_GET_MAP_TABLE(a_pstLib) + a_pstLib->iCurMetaNum;
        pstMetaMap->iPtr = a_pstLib->ptrLaseMeta;
        pstMetaMap->iSize = iMetaSize;

        pstMetaNameMap = TDR_GET_META_NAME_MAP_TABLE(a_pstLib) + a_pstLib->iCurMetaNum;
        pstMetaNameMap->ptrName = pstFreeMeta->ptrName;
        pstMetaNameMap->ptrMeta = pstMetaMap->iPtr;

        pstMetaIDMap = TDR_GET_META_ID_MAP_TABLE(a_pstLib) + a_pstLib->iCurMetaNum;
		if (TDR_META_DO_HAVE_ID(pstFreeMeta))
		{
			pstMetaIDMap->iID = pstFreeMeta->iID;
		}else
		{
			pstMetaIDMap->iID = TDR_INVALID_ID;
		}

        pstMetaIDMap->ptrMeta = pstMetaMap->iPtr;

        a_pstLib->iCurMetaNum++;
        a_pstLib->ptrLaseMeta += pstMetaMap->iSize;
    }

    return iRet;
}

int tdr_get_meta_type_i(LPTDRMETA a_pstMeta, LPTDRMETALIB a_pstLib, scew_element *a_pstElement, FILE *a_fpError)
{
    int iRet = TDR_SUCCESS;
    scew_attribute *pstAttr = NULL;
    const char *pszTypeName = NULL;
    LPTDRCTYPEINFO pstTypeInfo = NULL;
    int iTypeIndex = TDR_INVALID_INDEX;

    assert(NULL != a_pstMeta);
    assert(NULL != a_pstLib);
    assert(NULL != a_pstElement);
    assert(NULL != a_fpError);

    /*��ȡ��Ԫ�صľ�������*/
    if (TDR_XML_TAGSET_VERSION_0 == a_pstLib->iXMLTagSetVer)
    {
        pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_CLASS);
        if( NULL != pstAttr )
        {
            pszTypeName = scew_attribute_value(pstAttr);
        }else
        {
            pszTypeName = TDR_TAG_STRUCT;
        }/*if( NULL != pstAttr )*/
    }else
    {
        pszTypeName = scew_element_name(a_pstElement);
    }


    iTypeIndex = tdr_typename_to_idx(pszTypeName);
    if (TDR_INVALID_INDEX != iTypeIndex)
    {
        pstTypeInfo = tdr_idx_to_typeinfo(iTypeIndex);
        a_pstMeta->iType = pstTypeInfo->iType;
        a_pstMeta->idxType = iTypeIndex;
    }else
    {
        char szName[TDR_NAME_LEN] = {0};

        tdr_get_name_attribute_i(&szName[0], sizeof(szName), a_pstElement);
        fprintf(a_fpError, "\nerror:\t ��֧�ֵ�%d���Զ���ṹ<name = %s>����������: %s",
            a_pstLib->iCurMetaNum + 1, szName, pszTypeName);

        iRet = TDR_ERRIMPLE_UNSUPPORTED_TYPE;
    }

    return iRet;
}



LPTDRMETA tdr_get_meta_by_id_i(TDRMETALIB* a_pstLib, int a_iID)
{
    int i;
    LPTDRMAPENTRY pstMap;
    LPTDRMETA pstMeta;

    assert(NULL != a_pstLib);

    pstMap = TDR_GET_MAP_TABLE(a_pstLib);

    for(i=0; i< a_pstLib->iCurMetaNum; i++)
    {
        pstMeta	= (LPTDRMETA) (a_pstLib->data + pstMap[i].iPtr);
        if (TDR_META_DO_HAVE_ID(pstMeta) && ( pstMeta->iID == a_iID ))
        {
            return pstMeta;
        }
    }

    return NULL;
}

int tdr_add_meta_indexes_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement,
                           scew_element *a_pstRoot, FILE *a_fpError)
{
    int iRet = TDR_SUCCESS;
    scew_element *pstSubItem = NULL;
    LPTDRMETALIB pstLib = NULL;

    int iIndexNum = 0;
    int iColumnNum = 0;
    int iStrLen = 0;

    assert(NULL != a_pstMeta);
    assert(NULL != a_pstElement);
    assert(NULL != a_pstRoot);
    assert(NULL != a_fpError);


    a_pstMeta->ptrIndexes = TDR_INVALID_PTR;
    a_pstMeta->ptrColumns = TDR_INVALID_PTR;

    iRet = tdr_get_meta_index_info_i(&iIndexNum, &iColumnNum, &iStrLen, a_pstElement, a_pstRoot);
    if (TDR_ERR_IS_ERROR(iRet) || 0 >= iIndexNum)
    {
        return iRet;
    }

    a_pstMeta->ptrIndexes = (TDRPTR)((char*)a_pstMeta->stEntries +
                                     sizeof(TDRMETAENTRY) * a_pstMeta->iEntriesNum +
                                     sizeof(TDRDBKEYINFO) * a_pstMeta->nPrimayKeyMemberNum -
                                     (char*)a_pstMeta);
    a_pstMeta->ptrColumns = a_pstMeta->ptrIndexes + sizeof(TDRMETAINDEX) * iIndexNum;


    pstLib = TDR_META_TO_LIB(a_pstMeta);

    iColumnNum = 0;
    pstSubItem = scew_element_next(a_pstElement, NULL);
    while (NULL != pstSubItem)
    {
        if (0 == tdr_stricmp(scew_element_name(pstSubItem), TDR_TAG_INDEX))
        {
            if (TDR_TYPE_UNION == a_pstMeta->iType)
            {
                fprintf(a_fpError, "\nwarning:\t �Զ�������<union name = %s>����������ĳ�Ա<tag = %s>\n",
                        TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), scew_element_name(pstSubItem));
                pstSubItem = scew_element_next(a_pstElement, pstSubItem);
                continue;
            }
            iRet = tdr_add_index_to_meta_i(a_pstMeta, &iColumnNum, pstSubItem, a_fpError);
            if (TDR_ERR_IS_ERROR(iRet))
            {
                break;
            }
        }

        pstSubItem = scew_element_next(a_pstElement, pstSubItem);
    }

    return iRet;
}


int tdr_add_meta_entries_i(LPTDRMETA a_pstMeta, scew_element *a_stElement, FILE *a_fpError)
{
    int iRet = TDR_SUCCESS;
    scew_element *pstSubItem = NULL;
    LPTDRMETALIB pstLib = NULL;
    const char *pszEntryName = NULL;

    assert(NULL != a_pstMeta);
    assert(NULL != a_stElement);
    assert(NULL != a_fpError);


    pstLib = TDR_META_TO_LIB(a_pstMeta);
    TDR_GET_ENTRY_TAG_NAME(pszEntryName, pstLib->iXMLTagSetVer);
    assert(NULL != pszEntryName);

     /*��ӳ�Ա����*/
    pstSubItem = scew_element_next(a_stElement, NULL);
    while (NULL != pstSubItem)
    {
        if (0 == tdr_stricmp(scew_element_name(pstSubItem), pszEntryName))
        {
            iRet = tdr_add_entry_to_meta_i(a_pstMeta, pstSubItem, a_fpError);
        } else if (0 != tdr_stricmp(scew_element_name(pstSubItem), TDR_TAG_INDEX))
        {
            fprintf(a_fpError, "\nwarning:\t �Զ�������<struct name = %s>"
                    "������Ч�ĳ�Ա<tag = %s>���壬��ʹ�ñ�ǩ<%s>��<%s>�������Ա\n",
                    TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName),
                    scew_element_name(pstSubItem), pszEntryName, TDR_TAG_INDEX);
            pstSubItem = scew_element_next(a_stElement, pstSubItem);
            continue;
        }

        if (TDR_ERR_IS_ERROR(iRet))
        {
            break;
        }

        pstSubItem = scew_element_next(a_stElement, pstSubItem);
    }

    return iRet;
}

int tdr_get_meta_specail_attribute_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError)
{
    int iRet = TDR_SUCCESS;
    LPTDRMETALIB pstLib = NULL;


    assert(NULL != a_pstMeta);
    assert(NULL != a_pstElement);
    assert(NULL != a_fpError);


    pstLib = TDR_META_TO_LIB(a_pstMeta);
    if (TDR_TYPE_STRUCT == a_pstMeta->iType)
    {
        /*��ȡstruct�ṹ����������*/
        iRet = tdr_get_struct_meta_size_attribute_i(a_pstMeta, a_pstElement, a_fpError);

        /*versionindictor*/
        if (!TDR_ERR_IS_ERROR(iRet))
        {
            iRet = tdr_get_stuct_meta_versionindicator_attribute_i(a_pstMeta, a_pstElement, a_fpError);
        }

        /*sizetype����,�������ѱ�sizeinfo���������*/
        if (!TDR_ERR_IS_ERROR(iRet))
        {
            iRet = tdr_get_struct_meta_packsize_attribute_i(a_pstMeta, a_pstElement, a_fpError, TDR_TAG_SIZETYPE);
        }

		/*sizeinfo����*/
		if (!TDR_ERR_IS_ERROR(iRet))
		{
			iRet = tdr_get_struct_meta_packsize_attribute_i(a_pstMeta, a_pstElement, a_fpError, TDR_TAG_SIZEINFO);
		}

		if (TDR_XML_TAGSET_VERSION_0 < pstLib->iXMLTagSetVer)
		{
			if (!TDR_ERR_IS_ERROR(iRet))
			{
				iRet = tdr_get_meta_sortkey_attribute_i(a_pstMeta, a_pstElement, a_fpError);
			}


			/*primarykey*/
			if (!TDR_ERR_IS_ERROR(iRet))
			{
				iRet = tdr_get_struct_meta_primarykey_attribute_i(a_pstMeta, a_pstElement, a_fpError);
			}

			/*splittablefactor*/
			if (!TDR_ERR_IS_ERROR(iRet))
			{
				iRet = tdr_get_struct_meta_splittablefactor_attribute_i(a_pstMeta, a_pstElement, a_fpError);
			}

			/*splittablekey*/
			if (!TDR_ERR_IS_ERROR(iRet))
			{
				iRet = tdr_get_struct_meta_splittablekey_attribute_i(a_pstMeta, a_pstElement, a_fpError);
			}

			/*splittablerule*/
			if (!TDR_ERR_IS_ERROR(iRet))
			{
				iRet = tdr_get_struct_meta_splittablerule_attribute_i(a_pstMeta, a_pstElement, a_fpError);
			}

			/*strictinput*/
			if (!TDR_ERR_IS_ERROR(iRet))
			{
				iRet = tdr_get_struct_meta_strictinput_attribute_i(a_pstMeta, a_pstElement, a_fpError);
			}

			/*dependonstruct*/
			if (!TDR_ERR_IS_ERROR(iRet))
			{
				iRet = tdr_get_struct_meta_dependonstruct_attribute_i(a_pstMeta, a_pstElement, a_fpError);
			}

			/*uniqueentryname*/
			if (!TDR_ERR_IS_ERROR(iRet))
			{
				iRet = tdr_get_struct_meta_uniqueentryname_attribute_i(a_pstMeta, a_pstElement, a_fpError);
			}
		}/*if (TDR_XML_TAGSET_VERSION_0 < pstLib->iXMLTagSetVer)*/

    }/*if ((!TDR_ERR_IS_ERROR(iRet)) && (TDR_TYPE_STRUCT == pstMeta->iType))*/

    if (!TDR_ERR_IS_ERROR(iRet) &&
        (TDR_XML_TAGSET_VERSION_0 == pstLib->iXMLTagSetVer) && (TDR_TYPE_UNION == a_pstMeta->iType))
    {
        iRet = tdr_get_struct_meta_packsize_attribute_i(a_pstMeta, a_pstElement, a_fpError, TDR_TAG_SIZETYPE);
    }

    return iRet;
}

int tdr_get_struct_meta_size_attribute_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError)
{
    scew_attribute *pstAttr = NULL;
    int iRet = TDR_SUCCESS;
    LPTDRMETALIB pstLib = NULL;
    int iCustomHUnitSize = 0;

    assert(NULL != a_pstMeta);
    assert(NULL != a_pstElement);
    assert(NULL != a_fpError);


    pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_SIZE);
    if( NULL != pstAttr )
    {
        const char *pszVal = scew_attribute_value(pstAttr);

		pstLib = TDR_META_TO_LIB(a_pstMeta);
        iRet = tdr_get_macro_int_i(&iCustomHUnitSize, &a_pstMeta->idxCustomHUnitSize, pstLib, pszVal);
        a_pstMeta->iCustomHUnitSize = iCustomHUnitSize;
        if ((TDR_ERR_IS_ERROR(iRet)) || (0 >= a_pstMeta->iCustomHUnitSize))
        {
            fprintf(a_fpError, "error:\t Ԫ��<name = %s>��size����ֵ<%s>��Ч,��ȷ����ֵ�����������Ѷ���ĺ�\n",
                TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), pszVal);

            iRet = TDR_ERRIMPLE_META_INVALID_SIZE_VALUE;
        }
    }/*if (!TDR_ERR_IS_ERROR(iRet))*/

    return iRet;
}

int tdr_get_struct_meta_align_attribute_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError)
{
    scew_attribute *pstAttr = NULL;
    int iRet = TDR_SUCCESS;

    assert(NULL != a_pstMeta);
    assert(NULL != a_pstElement);
    assert(NULL != a_fpError);


    pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_ALIGN);
    if( NULL != pstAttr )
    {
        const char *pszVal = scew_attribute_value(pstAttr);
        a_pstMeta->iCustomAlign = (int)strtol(pszVal, NULL, 0);
        if (0 >= a_pstMeta->iCustomAlign)
        {
            LPTDRMETALIB pstLib = TDR_META_TO_LIB(a_pstMeta);
            fprintf(a_fpError, "error:\t �Զ�������<name = %s>��align����ֵ<%s>��Ч,��ȷ����ֵ�����������Ѷ���ĺ�\n",
                TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), pszVal);

            iRet = TDR_ERRIMPLE_META_INVALID_ALIGN_VALUE;
        }
    }/*if (!TDR_ERR_IS_ERROR(iRet))*/

    return iRet;
}


int tdr_get_stuct_meta_versionindicator_attribute_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError)
{
    int iRet = TDR_SUCCESS;
    scew_attribute *pstAttr = NULL;
    LPTDRMETALIB pstLib =NULL;

    const char *pszVersion = NULL;

    assert(NULL != a_pstMeta);
    assert(NULL != a_pstElement);
    assert(NULL != a_fpError);


    pstLib = TDR_META_TO_LIB(a_pstMeta);
	a_pstMeta->iVersionIndicatorMinVer = a_pstMeta->iBaseVersion;

    pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_VERSIONINDICATOR);
    if( NULL == pstAttr )
    {
        return TDR_SUCCESS;
    }

    pszVersion = scew_attribute_value(pstAttr);
	iRet = tdr_parse_meta_versionindicator_i(a_pstMeta, pszVersion);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		fprintf(a_fpError, "\nerror:\t �Զ�������<name = %s>��versionindicator����ֵ<%s>��Ч\n",
			TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), pszVersion);
		if ((int)(TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_SIZETYPE_VALUE)) == iRet)
		{
			fprintf(a_fpError, "����������ֵ�޷���λ��ָ��Ԫ��,��ȷ��ָ����ȷ��Ԫ������·������·�����漰�ĸ�Ԫ�ز���ָ�����������.\n");
		}
		if ((int)(TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_SIZETYPE_VALUE)) == iRet)
		{
			fprintf(a_fpError, "��ָ���Ա����������ֻ��Ϊ<char-uint>,�Ҵ�Ԫ�ز���Ϊ����,�˳�Աǰ��ĳ�Ա�洢�ռ�����ǹ̶���\n");
		}
		iRet = TDR_ERRIMPLE_META_INVALID_VERSIONINDICATOR_VALUE;
	}


    return iRet;
}

int tdr_get_struct_meta_packsize_attribute_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError, const char *a_pszAttr)
{
    int iRet = TDR_SUCCESS;
    scew_attribute *pstAttr = NULL;
    const char *pszSizeType = NULL;

    assert(NULL != a_pstMeta);
    assert(NULL != a_pstElement);
    assert(NULL != a_fpError);
	assert(NULL != a_pszAttr);

    pstAttr = scew_attribute_by_name(a_pstElement, a_pszAttr);
    if (NULL == pstAttr)
    {
        /*û��ָ��sizetype����*/
        return TDR_SUCCESS;
    }

    pszSizeType = scew_attribute_value(pstAttr);
	iRet = tdr_sizeinfo_to_off(&a_pstMeta->stSizeType, a_pstMeta, TDR_INVALID_INDEX, pszSizeType);
	if (TDR_ERR_IS_ERROR(iRet))
	{
        LPTDRMETALIB pstLib = TDR_META_TO_LIB(a_pstMeta);
		fprintf(a_fpError, "\nerror:\t �Զ�������<name = %s>��sizetype����ֵ<%s>��Ч,������ֵֻ��Ϊ<tinychar-uint>��ǰԪ�ص���Ԫ�س�Ա��Path\n",
			TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), pszSizeType);
	}


    return iRet;
}




int tdr_check_meta_i(LPTDRMETA a_pstMeta, FILE *a_fpError)
{
    int iRet = 0;
    int i = 0;
    TDRBOOLEAN bDyn = TDR_FALSE;
    LPTDRMETALIB pstLib = NULL;
	size_t iPadding;

    assert(NULL != a_pstMeta);
    assert(NULL != a_fpError);

    pstLib = TDR_META_TO_LIB(a_pstMeta);

    if (TDR_INVALID_INDEX == a_pstMeta->ptrName)
    {
        fprintf(a_fpError, "error:\t Fatal Error: �����������͵�name����ֵ�Ƿ���\n");
        return TDR_ERRIMPLE_META_NO_NAME;
    }

    /*���汾*/
    if (0 > a_pstMeta->iBaseVersion)
    {
        fprintf(a_fpError, "error:\t �Զ�������<name = %s>ָ����version����<%d>����Ϊ����\n",
                TDR_GET_STRING_BY_PTR(pstLib, a_pstMeta->ptrName), a_pstMeta->iBaseVersion);
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_VERSION);
    }


	/*����Զ���ռ�*/
	if ((0 < a_pstMeta->iCustomHUnitSize) && (a_pstMeta->iCustomHUnitSize < a_pstMeta->iHUnitSize))
	{
		fprintf(a_fpError, "error:\t �Զ�������<name = %s>ָ���Ĵ洢�ռ�<%"TDRPRI_SIZET">���ܱ�ʵ����Ҫ�Ĵ洢�ռ�<%"TDRPRI_SIZET">С\n",
			TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), a_pstMeta->iCustomHUnitSize, a_pstMeta->iHUnitSize);
		return TDR_ERRIMPLE_META_INVALID_SIZE_VALUE;
	}


	/*�ṹ����������*/
	if ((0 < a_pstMeta->iCustomHUnitSize) && (a_pstMeta->iHUnitSize < a_pstMeta->iCustomHUnitSize))
	{
		iPadding = a_pstMeta->iCustomHUnitSize - a_pstMeta->iHUnitSize;
		a_pstMeta->iHUnitSize += iPadding;
	}
    a_pstMeta->iValidAlign = TDR_MIN(a_pstMeta->iValidAlign, a_pstMeta->iCustomAlign);
	if (0 >= a_pstMeta->iValidAlign)
	{
		fprintf(a_fpError, "error:\t �Զ�������<name = %s>����Ч�ֽڶ���ֵ<%"TDRPRI_SIZET">����ȷ, ����align����\n",
			TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), a_pstMeta->iCustomHUnitSize);
		return TDR_ERRIMPLE_META_INVALID_ALIGN_VALUE;
	}
	if ((0 < a_pstMeta->iValidAlign ) && (0 != (a_pstMeta->iHUnitSize % a_pstMeta->iValidAlign)))
	{
		/*û�ж���,����ֽ�*/
		iPadding = (a_pstMeta->iValidAlign - (a_pstMeta->iHUnitSize % a_pstMeta->iValidAlign));
		a_pstMeta->iHUnitSize += iPadding;
	}/*if ((0 < iEntryValidAlign ) && (0 != (iCurHOff % iEntryValidAlign))*/


    /*�ṹ��Ա���*/
	for (i = 0; i < a_pstMeta->iEntriesNum; i++)
	{
		LPTDRMETAENTRY pstEntry = a_pstMeta->stEntries + i;

        if (TDR_INVALID_INDEX == pstEntry->ptrName)
        {
            fprintf(a_fpError, "error:\t Fatal Error: �����������͵�(name=%s)��Ա��name����ֵ�Ƿ���\n",
                TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName));
            return TDR_ERRIMPLE_META_NO_NAME;
        }

		/*����ԱΪ�ɱ�����*/
		if (0 == pstEntry->iCount )
		{
			if( (TDR_TYPE_UNION != a_pstMeta->iType) && (i+1 != a_pstMeta->iEntriesNum) )
			{
				fprintf(a_fpError,"error:\t ��union����<name = %s>ֻ�����һ����Ա<%d>Ϊ�ɱ�����,����%d����Ա<name = %s>�Ͳ����ϴ�Լ��\n",
					TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), a_pstMeta->iEntriesNum, i+1, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName));

				return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_VARIABLE_ARRAY_ENTRY);
			}/* only the last can be variable. */

			TDR_META_SET_VARIABLE(a_pstMeta);
		}

		if (((TDR_TYPE_WSTRING == pstEntry->iType) || (TDR_TYPE_STRING == pstEntry->iType)) && (0 >= pstEntry->iCustomHUnitSize))
		{
			if( (TDR_TYPE_UNION != a_pstMeta->iType) && (i+1 != a_pstMeta->iEntriesNum) )
			{
				fprintf(a_fpError,"error:\t ��union����<name = %s>ֻ�����һ����Ա<%d>Ϊû��ָ��size��string����,����%d����Ա<name = %s>�Ͳ����ϴ�Լ��\n",
					TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), a_pstMeta->iEntriesNum, i+1, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName));

				return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_VARIABLE_STRING_ENTRY);
			}/* only the last can be variable. */

			//�洢�ռ��Ա����Ϊ����
			if (1 != pstEntry->iCount)
			{
				fprintf(a_fpError,"error:\t �Զ�������<name = %s>�ĵ�%d����Ա<name = %s>�洢�ռ��ǿɱ�ģ���˴˳�Ա����Ϊ����<count:%d>\n",
					TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), i+1, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName),  pstEntry->iCount);
				return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_VARIABLE_STRING_ENTRY);
			}

			TDR_META_SET_VARIABLE(a_pstMeta);
		}


		/*����ԱΪ�Զ�����������*/
		if (TDR_INVALID_INDEX != pstEntry->ptrMeta)
		{
			LPTDRMETA pstTypeMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);

			/*meta��sizetype��sizeinfo����ͬʱָ��*/
			if ((0 < pstTypeMeta->stSizeType.iUnitSize) && ( 0 < pstEntry->stSizeInfo.iUnitSize))
			{
				fprintf(a_fpError,"error:\t �Զ�������<name = %s>�ĵ�%d����Ա<name = %s>ָ��sizeinfo/target���ԣ���ͬʱ�˳�Աָ��Ľṹ��ָ����sizetype���ԣ�Ŀǰ��֧���������\n",
					TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), i+1, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName));
				return TDR_ERRIMPLE_META_INVALID_SIZETYPE_VALUE;
			}

			if (TDR_META_IS_VARIABLE(pstTypeMeta))
			{
				//ֻ�������һ����Ա�洢�ռ�ɱ�
				if (( i != a_pstMeta->iEntriesNum-1) && (TDR_TYPE_UNION != a_pstMeta->iType))
				{
					fprintf(a_fpError,"error:\t �Զ�������<name = %s>�ĵ�%d����Ա<name = %s>�洢�ռ��ǿɱ�ģ����˳�Ա�������һ����Ա<%d>\n",
						TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), i+1, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName),  a_pstMeta->iEntriesNum);
					return TDR_ERRIMPLE_ENTRY_INVALID_COUNT_VALUE;
				}

				//�˴洢�ռ�ɱ�ĳ�Ա������Ϊ����
				if (1 != pstEntry->iCount)
				{
					fprintf(a_fpError,"error:\t �Զ�������<name = %s>�ĵ�%d����Ա<name = %s>�洢�ռ��ǿɱ�ģ���˴˳�Ա����Ϊ����<count:%d>\n",
						TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), i+1, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName),  pstEntry->iCount);
					return TDR_ERRIMPLE_ENTRY_INVALID_COUNT_VALUE;
				}

				TDR_META_SET_VARIABLE(a_pstMeta);
			}
		}/*if (TDR_INVALID_INDEX != pstEntry->ptrMeta)*/

		/*�ṹ��洢�ռ��Ƿ�̶�*/
		if (!TDR_ENTRY_IS_FIXSIZE(pstEntry))
		{
			bDyn = TDR_TRUE;
		}
	}/*for (i = 0; i < a_pstMeta->iEntriesNum; i++)*/
	if (TDR_FALSE == bDyn)
	{
		TDR_META_SET_FIXSIZE(a_pstMeta);
	}

	/*union�ṹ���*/
	if (TDR_TYPE_UNION == a_pstMeta->iType)
	{
		/*union�ṹ�в�ָ��id�ĳ�Աֻ����һ��*/
		int iNoIdEntris = 0;
		for (i = 0; i < a_pstMeta->iEntriesNum; i++)
		{
			LPTDRMETAENTRY pstEntry = a_pstMeta->stEntries + i;
			if (!TDR_ENTRY_IS_VALID_SELECTID(pstEntry))
			{
				iNoIdEntris++;
			}
		}/*for (i = 0; i < a_pstMeta->iEntriesNum; i++)*/
		if (1 < iNoIdEntris)
		{
			fprintf(a_fpError, "error:\t union����<name = %s>����Ԫ��ֻ�����ҽ���һ������û��id/(maxid,minid)����\n",
				TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName));

			return TDR_ERRIMPLE_ENTRY_INVALID_MAXMINID_VALUE;
		}
	} /*if (TDR_TYPE_UNION == a_pstMeta->iType)*/


	/*�����DB��ص�����*/
	if (TDR_INVALID_PTR != a_pstMeta->ptrDependonStruct)
	{
		iRet = tdr_check_depend_meta_dbattributes_i(a_pstMeta, a_fpError);
	}else
	{
		iRet = tdr_check_table_meta_dbattributes_i(a_pstMeta, a_fpError);
	}

    TDR_META_SET_RESOLVED(a_pstMeta);

    return iRet;
}





int tdr_parse_meta_versionindicator_i(LPTDRMETA a_pstMeta, const char* a_pszName)
{
	TDRMETALIB* pstLib;
	TDRMETAENTRY* pstEntry = NULL;
	LPTDRMETA pstSearchMeta;
	int iRet = TDR_SUCCESS;
	TDRBOOLEAN bOutMostEntry = TDR_TRUE;

	char szBuff[TDR_NAME_LEN];
	const char* pszPtr;
	int idx;
	TDROFF iNOff;
	TDROFF iHOff;
	size_t iUnit;
	int i;
    int iPreEntryMaxVer;

	assert( NULL != a_pstMeta);
	assert(NULL != a_pszName);

	pstLib = TDR_META_TO_LIB(a_pstMeta);
	pszPtr = a_pszName;

	/*�����meta��sizeinfo����,������'this'*/
	pszPtr = tdr_get_first_node_name_from_path_i(szBuff, sizeof(szBuff), pszPtr);
	if( 0 != tdr_stricmp(szBuff, TDR_TAG_THIS) )
	{
		pszPtr = a_pszName;
	}


	iNOff = 0;
	iHOff = 0;
	iUnit = 0;
	pstSearchMeta = a_pstMeta;
	do
	{
		if( NULL == pstSearchMeta )

		{
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_SIZETYPE_VALUE);
			break;
		}


		pszPtr = tdr_get_first_node_name_from_path_i(szBuff, sizeof(szBuff), pszPtr);
		if ('\0' == szBuff[0])
		{/*�մ������Ϸ�*/
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_SIZETYPE_VALUE);
			break;
		}
		tdr_trim_str(szBuff);

		idx	= tdr_get_entry_by_name_i(pstSearchMeta->stEntries, pstSearchMeta->iEntriesNum, szBuff);
		if( TDR_INVALID_INDEX == idx )
		{
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_SIZETYPE_VALUE);
			break;
		}

        /* ��¼�ҵ�versionindicator�Ǹ�entryʱ�� ǰ���entry�����versionֵ */
        iPreEntryMaxVer =  pstSearchMeta->stEntries[0].iVersion;
		for(i=0; i < idx; i++)
		{
			pstEntry =	pstSearchMeta->stEntries  + i;
			if (!TDR_ENTRY_IS_FIXSIZE(pstEntry))
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_VARIABLE_BEFOR_SIZEINFO);
				break;
			}
            if (iPreEntryMaxVer < pstEntry->iVersion)
            {
                iPreEntryMaxVer = pstEntry->iVersion;
            }
		}


		pstEntry = pstSearchMeta->stEntries + idx;
		if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry) || TDR_ENTRY_IS_REFER_TYPE(pstEntry))
		{
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_SIZETYPE_VALUE);
			break;
		}

        /* ��֤versionindicatorǰ���entry�İ汾��������versionindicator����İ汾 */
        if (pstEntry->iVersion < iPreEntryMaxVer)
        {
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_SIZETYPE_VALUE);
			break;
        }

		if (bOutMostEntry)
		{
			a_pstMeta->iVersionIndicatorMinVer = pstEntry->iVersion;
			bOutMostEntry = TDR_FALSE;
		}
        else
        {
            if (pstEntry->iVersion > a_pstMeta->iVersionIndicatorMinVer)
                a_pstMeta->iVersionIndicatorMinVer = pstEntry->iVersion;
        }

		iNOff += pstEntry->iNOff;
		iHOff += pstEntry->iHOff;
		iUnit =	pstEntry->iNUnitSize;

		if(TDR_INVALID_PTR != pstEntry->ptrMeta )
		{
			pstSearchMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
		}
		else
		{
			pstSearchMeta	=	NULL;
		}
	}while( NULL != pszPtr );

	if (TDR_ERR_IS_ERROR(iRet))
	{
		return iRet;
	}

	if ((NULL == pstEntry) || ( 1 != pstEntry->iCount) ||(pstEntry->stRefer.iUnitSize > 0 ))
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_VARIABLE_BEFOR_SIZEINFO);
	}

	if( pstEntry->iType <= TDR_TYPE_COMPOSITE || pstEntry->iType > TDR_TYPE_UINT )
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_VARIABLE_BEFOR_SIZEINFO);
	}


	a_pstMeta->stVersionIndicator.iHOff	=	iHOff;
	a_pstMeta->stVersionIndicator.iNOff = iNOff;
	a_pstMeta->stVersionIndicator.iUnitSize	=	iUnit;

	return TDR_SUCCESS;
}



int tdr_get_struct_meta_splittablefactor_attribute_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError)
{
	int iRet = TDR_SUCCESS;
	scew_attribute *pstAttr = NULL;
	char szFactor[TDR_MACRO_LEN] = {0};
	LPTDRMETALIB pstLib;


	assert(NULL != a_pstMeta);
	assert(NULL != a_pstElement);
	assert(NULL != a_fpError);

	pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_SPLITTABLEFACTOR);
	if (NULL == pstAttr)
	{
		/*û��ָ��sizetype����*/
		return TDR_SUCCESS;
	}

	tdr_normalize_string(szFactor, sizeof(szFactor), scew_attribute_value(pstAttr));
	pstLib = TDR_META_TO_LIB(a_pstMeta);
	iRet = tdr_get_macro_int_i(&a_pstMeta->iSplitTableFactor, &a_pstMeta->idxSplitTableFactor, pstLib, szFactor);
	if ((TDR_ERR_IS_ERROR(iRet)) || (0 > a_pstMeta->iSplitTableFactor))
	{
		fprintf(a_fpError, "error:\t Ԫ��<name = %s>��splittablefacotr����ֵ<%s>��Ч,��ȷ����ֵ�����������Ѷ���ĺ�\n",
			TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), szFactor);

		iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_SPLITTABLEFACTOR);
	}

	return iRet;
}

int tdr_get_struct_meta_primarykey_attribute_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError)
{
	int iRet = TDR_SUCCESS;
	scew_attribute *pstAttr = NULL;
	LPTDRMETALIB pstLib;
	char *pszKey = NULL;
	char *pchDot;
	char *pchBegin;
	LPTDRMETAENTRY pstEntry;
	TDRSelector stSelector;
	LPTDRDBKEYINFO pstDBKey;

	assert(NULL != a_pstMeta);
	assert(NULL != a_pstElement);
	assert(NULL != a_fpError);

	pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_PRIMARY_KEY);
	if (NULL == pstAttr)
	{
		/*û��ָ��sizetype����*/
		return TDR_SUCCESS;
	}

	pstLib = TDR_META_TO_LIB(a_pstMeta);
	pstDBKey = TDR_GET_PRIMARYBASEPTR(a_pstMeta);

	pszKey = strdup(scew_attribute_value(pstAttr));
	pchBegin = pszKey;

	while (1)
	{
		pchDot = strchr(pchBegin, ',');
		if (NULL != pchDot)
		{
			*pchDot = '\0';
		}
		tdr_trim_str(pchBegin);
		if (*pchBegin == '\0')
		{
			if (NULL == pchDot)
			{
				break;
			}else
			{
				pchDot++;
				pchBegin = pchDot;
				continue;
			}
		}/*if (*pchBegin == '\0') */

		iRet = tdr_parse_entryoff_db_i(a_pstMeta, &stSelector, TDR_TAG_PRIMARY_KEY, pchBegin, a_fpError);
		if (TDR_ERR_IS_ERROR(iRet))
		{
			if (iRet != (int)TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_EXTENABLE_ARRAY))
			{
				fprintf(a_fpError, "error:\t �ṹ<name = %s>��%s����ֵ<%s>��Ч,��ȷ����ֵ����\',\'�ָ�����<entry>���͵��ӳ�Ա��.\n",
					TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_TAG_PRIMARY_KEY, pchBegin);
			}
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_PRIMARYKEY);
			break;
		}

		pstEntry = TDR_PTR_TO_ENTRY(pstLib, stSelector.ptrEntry);
		if (TDR_TYPE_COMPOSITE >= pstEntry->iType)
		{
			fprintf(a_fpError, "error:\t �ṹ<name = %s>��primarykey����ֵ<%s>��Ч,Ŀǰ��֧��ʹ�ø������ͳ�Ա������\n",
				TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), pszKey);
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_PRIMARYKEY);
			break;
		}
		if ((1 < pstEntry->iCount) &&
			((TDR_TYPE_CHAR > pstEntry->iType) || (TDR_TYPE_BYTE < pstEntry->iType)))
		{
			fprintf(a_fpError, "error:\t �ṹ<name = %s>��primarykey����ֵ<%s>��Ч,��char��byte�����⣬Ŀǰ��֧���������͵������Ա��Ϊ����.\n",
				TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), pszKey);
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_PRIMARYKEY);
			break;
		}

		if ((((char *)pstDBKey - (char*)a_pstMeta) + (int)sizeof(TDRDBKEYINFO))> (ptrdiff_t)a_pstMeta->iMemSize)
		{
			fprintf(a_fpError, "error:\t �洢�ṹ<name = %s>��primarykey����ֵ<%s>ʱʣ��ռ䲻��,Ŀǰ�Ѵ洢��%d������Ԫ��.\n",
				TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), pszKey, a_pstMeta->nPrimayKeyMemberNum);

			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_PRIMARYKEY);
			break;
		}
		a_pstMeta->nPrimayKeyMemberNum++;
		pstDBKey->ptrEntry = stSelector.ptrEntry;
		pstDBKey->iHOff = stSelector.iHOff;
		TDR_ENTRY_SET_PRIMARYKEY(pstEntry);
		pstDBKey++;

		if (NULL == pchDot)
		{
			break;
		}
		pchDot++;
		pchBegin = pchDot;
	}/*while ('\0' != *pchBegin)*/

	free(pszKey);

	return iRet;
}

int tdr_parse_entryoff_db_i(IN LPTDRMETA a_pstMeta, OUT LPTDRSelector a_pstSelector, IN const char *a_pszAttr,
							IN const char* a_pszName, IN FILE *a_fpError)
{
	TDRMETALIB* pstLib;
	TDRMETAENTRY* pstEntry = NULL;
	LPTDRMETA pstSearchMeta;
	char szBuff[TDR_NAME_LEN];
	int iRet = TDR_SUCCESS;
	const char* pszPtr = NULL;

	TDROFF iHOff;
	int iIdx;

	assert( NULL != a_pstMeta);
	assert(NULL != a_pstSelector);
	assert(NULL != a_pszName);
	assert(NULL != a_pstSelector);
	assert(NULL != a_pszAttr);


	pstLib = TDR_META_TO_LIB(a_pstMeta);


	/*meta��������this*/
	pszPtr = a_pszName;
	pszPtr = tdr_get_first_node_name_from_path_i(szBuff, sizeof(szBuff), pszPtr);
	if( 0 != tdr_stricmp(szBuff, TDR_TAG_THIS))
	{
		pszPtr = a_pszName;
	}



	iHOff = 0;
	pstSearchMeta = a_pstMeta;
	do
	{
		if( NULL == pstSearchMeta )
		{
			break;
		}


		pszPtr = tdr_get_first_node_name_from_path_i(szBuff, sizeof(szBuff), pszPtr);
		if ('\0' == szBuff[0])
		{/*�մ������Ϸ�*/
			break;
		}
		tdr_trim_str(szBuff);


		/*����������ͬ��entry*/
		pstEntry = NULL;
		iIdx = tdr_get_entry_by_name_i(pstSearchMeta->stEntries, pstSearchMeta->iEntriesNum, szBuff);
		if( TDR_INVALID_INDEX == iIdx )
		{
			break;
		}

		pstEntry = pstSearchMeta->stEntries + iIdx;
		if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry) || TDR_ENTRY_IS_REFER_TYPE(pstEntry))
		{
			iRet = TDR_ERRIMPLE_INVALID_PATH_VALUE;
			break;
		}

		iHOff += pstEntry->iHOff;


		if( TDR_INVALID_PTR != pstEntry->ptrMeta )
		{
			pstSearchMeta	=	TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
			if (!TDR_ENTRY_DO_EXTENDABLE(pstEntry)  || (1 < pstEntry->iCount))
			{
				fprintf(a_fpError, "error:\t �洢�ṹ<name = %s>������<%s=%s>Ŀǰֻ֧�ֶ����ڿ���չ�Ľṹ��������Ա֮��,����Ա<name=%s>�����ϴ˴�Լ��.\n",
					TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), a_pszAttr, a_pszName, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName));

				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_EXTENABLE_ARRAY);
			}
		}else
		{
			pstSearchMeta	=	NULL;
		}/*if( TDR_INVALID_PTR != pstEntry->ptrMeta )*/

	}while( NULL != pszPtr );

	if (TDR_ERR_IS_ERROR(iRet))
	{
		return iRet;
	}

	if ((NULL != pszPtr)  || (NULL == pstEntry))
	{
		iRet = TDR_ERRIMPLE_INVALID_PATH_VALUE;
	}else if (NULL != pstEntry)
	{
		a_pstSelector->iHOff = iHOff;
		a_pstSelector->iUnitSize = pstEntry->iNUnitSize;
		a_pstSelector->ptrEntry = TDR_ENTRY_TO_PTR(pstLib, pstEntry);
	}


	return iRet;
}

int tdr_get_struct_meta_splittablekey_attribute_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError)
{
	int iRet = TDR_SUCCESS;
	scew_attribute *pstAttr = NULL;
	LPTDRMETALIB pstLib;
	LPTDRMETAENTRY pstEntry;
	TDRSelector stSelector;
	const char *pszKey = NULL;


	assert(NULL != a_pstMeta);
	assert(NULL != a_pstElement);
	assert(NULL != a_fpError);

	pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_SPLITTABLEKEY);
	if (NULL == pstAttr)
	{
		/*û��ָ��sizetype����*/
		return TDR_SUCCESS;
	}

	pstLib = TDR_META_TO_LIB(a_pstMeta);
	pszKey = scew_attribute_value(pstAttr);
	iRet = tdr_parse_entryoff_db_i(a_pstMeta, &stSelector, TDR_TAG_SPLITTABLEKEY, pszKey, a_fpError);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		return iRet;
	}

	pstEntry = TDR_PTR_TO_ENTRY(pstLib, stSelector.ptrEntry);
	if ( TDR_TYPE_COMPOSITE >= pstEntry->iType)
	{
		if (iRet != (int)TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_EXTENABLE_ARRAY))
		{
			fprintf(a_fpError, "error:\t �ṹ<name = %s>��splittablekey����ֵ<%s>��Ч,��ȷ����ֵ�ǵ�ǰ�ṹ����Ԫ����������ֻ���ǻ�����������.\n",
				TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), pszKey);
		}

		iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_SPLITTABLEFACTOR);
	}

	a_pstMeta->stSplitTableKey.ptrEntry = stSelector.ptrEntry;
	a_pstMeta->stSplitTableKey.iHOff = stSelector.iHOff;

	return iRet;
}

int tdr_get_struct_meta_splittablerule_attribute_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError)
{
	int iRet = TDR_SUCCESS;
	scew_attribute *pstAttr = NULL;
	LPTDRMETALIB pstLib;
	char szRule[256] = {0};


	assert(NULL != a_pstMeta);
	assert(NULL != a_pstElement);
	assert(NULL != a_fpError);

	pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_SPLITTABLERULE);
	if (NULL == pstAttr)
	{
		/*û��ָ��sizetype����*/
		return TDR_SUCCESS;
	}

	pstLib = TDR_META_TO_LIB(a_pstMeta);
	tdr_normalize_string(&szRule[0], sizeof(szRule), scew_attribute_value(pstAttr));
	if (0 == tdr_stricmp(szRule, TDR_TAG_SPLITTABLERULE_BY_MOD))
	{
		a_pstMeta->nSplitTableRuleID = TDR_SPLITTABLE_RULE_BY_MOD;
	}else
	{
		fprintf(a_fpError, "error:\t structԪ��<name = %s>��splittablerule����ֵ<%s>��Ч,Ŀǰֻ֧��\"%s\"����\n",
			TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), szRule, TDR_TAG_SPLITTABLERULE_BY_MOD);

		iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_SPLITTABLERULE);
	}


	return iRet;
}


int tdr_check_table_meta_dbattributes_i(IN LPTDRMETA a_pstMeta, IN FILE *a_fpError)
{
	int iRet = TDR_SUCCESS;
	int i;
	LPTDRMETAENTRY pstEntry;
	LPTDRMETA pstTypeMeta;
	LPTDRMETALIB pstLib;

	assert(NULL != a_pstMeta);
	assert(NULL != a_fpError);

	pstLib = TDR_META_TO_LIB(a_pstMeta);

	/*��齨��ʱ�������Եĳ�Ա�Ƿ�����ΪNotNULL�����û��ǿ������ΪNOTNULL*/
	if (0 <= a_pstMeta->nPrimayKeyMemberNum)
	{
		LPTDRDBKEYINFO pstKey = TDR_GET_PRIMARYBASEPTR(a_pstMeta);

		for (i = 0; i < a_pstMeta->nPrimayKeyMemberNum; i++)
		{
			pstEntry = TDR_PTR_TO_ENTRY(pstLib, pstKey->ptrEntry);
			if (!TDR_ENTRY_IS_NOT_NULL(pstEntry))
			{
				TDR_ENTRY_SET_NOT_NULL(pstEntry);
			}
		}
	}/*if (0 <= a_pstMeta->nPrimayKeyMemberNum)*/

	/*��齨��ʱ�ֱ������Ƿ���Ч*/
	if (0 < a_pstMeta->iSplitTableFactor)
	{
		/*��Ҫ�ֱ������ָ���ֱ�ؼ���*/
		if ((TDR_INVALID_OFFSET == a_pstMeta->stSplitTableKey.iHOff) ||
			(TDR_INVALID_PTR == a_pstMeta->stSplitTableKey.ptrEntry))
		{
			fprintf(a_fpError, "Error:\t �ṹ<name=%s>���ɽ������ʱ��ֳ�%d�����ݿ����д洢����û��ָ��splittablekey���ԣ������зֱ�ӳ��ĳ�Ա��\n",
				TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), a_pstMeta->iSplitTableFactor);
			return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_SPLITTABLEKEY);
		}

		/*û��ָ��ӳ�����ʱ��ȱʡʹ�á��ֱ�����ģ������*/
		if (TDR_SPLITTABLE_RULE_NONE == a_pstMeta->nSplitTableRuleID)
		{
			a_pstMeta->nSplitTableRuleID = TDR_SPLITTABLE_RULE_BY_MOD;
		}

		/*���ֱ�����ģ������,�ֱ�����ֻ�����������͵ĳ�Ա*/
		if (TDR_SPLITTABLE_RULE_BY_MOD == a_pstMeta->nSplitTableRuleID)
		{
			pstEntry = TDR_PTR_TO_ENTRY(pstLib, a_pstMeta->stSplitTableKey.ptrEntry);
			if ((TDR_TYPE_CHAR > pstEntry->iType) || (TDR_TYPE_WSTRING < pstEntry->iType))
			{
				fprintf(a_fpError, "Error:\t �ṹ<name=%s>���ɽ������ʱ����\"�ֱ�����ģ\"������ֱ�����<%s>ֻ��Ϊ��void����Ļ�����������\n",
					TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName));
				return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_SPLITTABLEKEY);
			}
			if (((TDR_TYPE_STRING == pstEntry->iType) || (TDR_TYPE_WSTRING == pstEntry->iType)) && (0 >= pstEntry->iCustomHUnitSize))
			{
				fprintf(a_fpError, "Error:\t �ṹ<name=%s>���ɽ������ʱ����\"�ֱ�����ģ\"������ֱ�����<%s>Ϊstring����,�������Ĵ洢�ռ�����ǹ̶���\n",
					TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName));
				return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_SPLITTABLEKEY);
			}
		}
	}/*if (0 < a_pstMeta->iSplitTableFactor)*/


	/*��齨��ʱstruct��Ա��չ�ĺϷ��ԣ�Ŀǰֻ��ָ������������
	��uniqueentryname����Ϊtrue��meta�ṹ���м��*/
	if ((0 <= a_pstMeta->nPrimayKeyMemberNum) && !(TDR_META_DO_NEED_PREFIX(a_pstMeta)))
	{
		char szConflictName[TDR_NAME_LEN * TDR_STACK_SIZE] = {0};


		for (i = 0; i < a_pstMeta->iEntriesNum; i++)
		{
			pstEntry = a_pstMeta->stEntries + i;
			if (!TDR_ENTRY_DO_EXTENDABLE(pstEntry))
			{
				continue;
			}
			if ((1 < pstEntry->iCount) || (0 == pstEntry->iCount))
			{
				fprintf(a_fpError, "Error:\t �ṹ��<name=%s>��Ա<name=%s>Ϊ�����������͵������Ա��չ���洢ʱ�����������ͬ���ֵĳ�Ա��\n",
					TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName));
				return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NAME_CONFLICT_WHEN_EXTEND);
			}

			pstTypeMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
			iRet = tdr_check_meta_extendable_to_table_i(a_pstMeta, i - 1, pstTypeMeta, szConflictName);
			if (TDR_ERR_IS_ERROR(iRet))
			{
				fprintf(a_fpError, "Error:\t ����Ԫ�����������ṹ<name=%s>�ĳ�Ա<name=%s>��չ�������ɽ�����䣬��չ��������������ͬ����<%s>�ĳ�Ա��\n",
					TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName), szConflictName);
				return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NAME_CONFLICT_WHEN_EXTEND);
			}
		}/*for (i = 0; i < a_pstMeta->iEntriesNum; i++)*/
	}/*if (TDR_INVALID_PTR != a_pstMeta->ptrPrimayKeyBase)*/



	/*�����������������ԱΪunique*/
	if (1 < a_pstMeta->nPrimayKeyMemberNum)
	{
		LPTDRDBKEYINFO pstKey = TDR_GET_PRIMARYBASEPTR(a_pstMeta);
		for (i = 0; i < a_pstMeta->nPrimayKeyMemberNum; i++)
		{
			pstEntry = TDR_PTR_TO_ENTRY(pstLib, pstKey->ptrEntry);
			if (TDR_ENTRY_IS_UNIQUE(pstEntry))
			{
				TDR_ENTRY_SET_NOT_NULL(pstEntry);
				fprintf(a_fpError, "Error:\t �ṹ<name=%s>�ĳ�Ա<name=%s>������<%s>���ԣ�����˽ṹ��Ӧ�����ݿ���ɶ�������<%d>����������������Գ�ͻ��\n",
					TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName), TDR_TAG_UNIQUE, a_pstMeta->nPrimayKeyMemberNum);
				return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_PRIMARYKEY);
			}
		}
	}/*if (1 < a_pstMeta->nPrimayKeyMemberNum)*/

	/*���autoincrement���Ե�Ψһ��*/
	for (i = 0; i < a_pstMeta->iEntriesNum; i++)
	{
		pstEntry = a_pstMeta->stEntries + i;
		if (!TDR_ENTRY_DO_EXTENDABLE(pstEntry))
		{
			continue;
		}

		assert(TDR_INVALID_PTR != pstEntry->ptrMeta);
		pstTypeMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
		if (!TDR_META_DO_HAVE_AUTOINCREMENT_ENTRY(pstTypeMeta))
		{
			continue;
		}

		if (1 != pstEntry->iCount)
		{
			fprintf(a_fpError, "\nerror:\t �ṹ��<name=%s>�ĳ�Ա<name = %s>��%s������Ч,ֻ�з������Ա���ܶ��������\n",
				TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName), TDR_TAG_AUTOINCREMENT);

			return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_AUTOINCREMENT);
		}
		if (TDR_META_DO_HAVE_AUTOINCREMENT_ENTRY(a_pstMeta))
		{
			fprintf(a_fpError, "\nerror:\t �ṹ����ֻ����һ����Ա����%s���ԣ��ṹ��<name=%s>���Ѿ������˶���%s���Եĳ�Ա����Ա<name = %s>�����ٶ��������\n",
				TDR_TAG_AUTOINCREMENT, TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_TAG_AUTOINCREMENT, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName));

			return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_AUTOINCREMENT);
		}else
		{
			TDR_META_SET_HAVE_AUTOINCREMENT_ENTRY(a_pstMeta);
		}
	}/*for (i = 0; i < a_pstMeta->iEntriesNum; i++)*/


	return iRet;
}

int tdr_check_depend_meta_dbattributes_i(IN LPTDRMETA a_pstMeta, IN FILE *a_fpError)
{
	LPTDRMETA pstDependOnMeta;
	LPTDRMETALIB pstLib;
	int i;
	int iRet = TDR_SUCCESS;
	LPTDRDBKEYINFO pstKey;
	LPTDRDBKEYINFO pstMetaKey;
	LPTDRDBKEYINFO pstDependMetaKeyBase;
	LPTDRMETAENTRY pstEntry;

	assert(NULL != a_pstMeta);
	assert(NULL != a_fpError);
	assert(TDR_INVALID_PTR != a_pstMeta->ptrDependonStruct);


	/*��λ���ṹ�е�������Ϣֻ��Ϊ�����ṹ�ж���ĳ�Ա*/
	pstLib = TDR_META_TO_LIB(a_pstMeta);
	pstDependOnMeta = TDR_PTR_TO_META(pstLib, a_pstMeta->ptrDependonStruct);
	pstMetaKey = TDR_GET_PRIMARYBASEPTR(a_pstMeta);
	pstDependMetaKeyBase = TDR_GET_PRIMARYBASEPTR(pstDependOnMeta);

	/*��������ṹ�����������Ϣ*/
	for (i = 0; i < a_pstMeta->nPrimayKeyMemberNum; i++)
	{
		pstKey = pstMetaKey + i;
		pstEntry = TDR_PTR_TO_ENTRY(pstLib, pstKey->ptrEntry);
		TDR_ENTRY_CLR_PRIMARYKEY(pstEntry);
	}/*for (i = 0; i < pstDependOnMeta->nPrimayKeyMemberNum; i++)*/


	/*�������ṹ�ж���ĳ�Ա,�ӵ����ṹ��*/
	if (((char *)pstMetaKey - (char *)a_pstMeta ) > (ptrdiff_t)(a_pstMeta->iMemSize - (int)(sizeof(TDRDBKEYINFO)*pstDependOnMeta->nPrimayKeyMemberNum)))
	{
		fprintf(a_fpError, "error:\t �ṹ��<name = %s>��ʣ��ռ䲻���洢%d����Ա��ɵ�������Ϣ.\n",
			TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), pstDependOnMeta->nPrimayKeyMemberNum);
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_PRIMARYKEY);
	}
	a_pstMeta->nPrimayKeyMemberNum = 0;
	for (i = 0; i < pstDependOnMeta->nPrimayKeyMemberNum; i++)
	{
		pstKey = pstDependMetaKeyBase + i;
		pstEntry = TDR_PTR_TO_ENTRY(pstLib, pstKey->ptrEntry);
		if (TDR_ERR_IS_ERROR(tdr_find_tablecolumn_entry_i(pstMetaKey, a_pstMeta, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName))))
		{
			continue;
		}

		pstEntry = TDR_PTR_TO_ENTRY(pstLib, pstMetaKey->ptrEntry);
		TDR_ENTRY_SET_PRIMARYKEY(pstEntry);
		pstMetaKey++;
		a_pstMeta->nPrimayKeyMemberNum++;
	}
	if (a_pstMeta->nPrimayKeyMemberNum != pstDependOnMeta->nPrimayKeyMemberNum)
	{
		fprintf(a_fpError, "error:\t �ṹ��<name = %s>��������(%d)�������������Ľṹ��<name=%s>��������(%d).\n",
			TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), a_pstMeta->nPrimayKeyMemberNum,
            TDR_GET_STRING_BY_PTR(pstLib,pstDependOnMeta->ptrName),
			pstDependOnMeta->nPrimayKeyMemberNum);
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_PRIMARYKEY);
	}


	/*���ṹ�б������dependonstruct�ж���ķֱ�������Ա*/
	a_pstMeta->stSplitTableKey.iHOff = TDR_INVALID_OFFSET;
	a_pstMeta->stSplitTableKey.ptrEntry = TDR_INVALID_PTR;
	if (TDR_INVALID_PTR != pstDependOnMeta->stSplitTableKey.ptrEntry)
	{
		pstEntry = TDR_PTR_TO_ENTRY(pstLib, pstDependOnMeta->stSplitTableKey.ptrEntry);
		tdr_find_tablecolumn_entry_i(&a_pstMeta->stSplitTableKey, a_pstMeta, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName));
	}

	return iRet;
}

int tdr_check_meta_extendable_to_table_i(LPTDRMETA a_pstBaseMeta, int iMaxIdx, LPTDRMETA a_pstCheckMeta, OUT char *a_pszConflictName)
{
	int iRet = TDR_SUCCESS;
	TDRSTACK stStack;
	LPTDRSTACKITEM pstStackTop;
	LPTDRMETAENTRY pstEntry;
	int iStackItemNum;
	LPTDRMETA pstCurMeta;
	int iChange;
	LPTDRMETALIB pstLib;

	assert(NULL != a_pstBaseMeta);
	assert(NULL != a_pstCheckMeta);

	if (0 > iMaxIdx)
	{
		return TDR_SUCCESS;
	}

	pstStackTop = &stStack[0];
	pstStackTop->pstMeta = a_pstCheckMeta;
	pstStackTop->iCount = 1;
	pstStackTop->idxEntry = 0;
	iStackItemNum = 1;
	pstCurMeta = a_pstCheckMeta;
	pstLib = TDR_META_TO_LIB(a_pstBaseMeta);
	pstStackTop->pszHostBase = NULL;  /*��¼��ͻ����·��*/

	while (0 < iStackItemNum)
	{
		if (0 >= pstStackTop->iCount)
		{
			iStackItemNum--;
			if (0 < iStackItemNum)
			{
				pstStackTop--;
				pstCurMeta = pstStackTop->pstMeta;
				TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			}
			continue;
		}

		pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;
		if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry))
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}
		if (TDR_ENTRY_IS_REFER_TYPE(pstEntry))
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}

		pstStackTop->pszHostBase = TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName);
		if ((TDR_TYPE_STRUCT == pstEntry->iType) && TDR_ENTRY_DO_EXTENDABLE(pstEntry))
		{
			LPTDRMETA pstTypeMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);

			/*���˳�Ա�ͻ����ṹ��Ա�Ƿ�����*/
			if (TDR_STACK_SIZE <=  iStackItemNum)
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_TOO_COMPLIEX_META);
				break;
			}

			/*���˳�Ա�͵�ǰ�ṹ��Ա�Ƿ�����*/
			iRet = tdr_check_meta_extendable_to_table_i(pstCurMeta, pstStackTop->idxEntry -1, pstTypeMeta, a_pszConflictName);
			if (TDR_ERR_IS_ERROR(iRet))
			{
				break;
			}
			iRet = tdr_check_dbtable_name_conflict_i(pstCurMeta, pstCurMeta->iEntriesNum, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName));
			if (TDR_ERR_IS_ERROR(iRet))
			{
				break;
			}

			pstStackTop++;
			iStackItemNum++;
			pstCurMeta = pstTypeMeta;
			pstStackTop->pstMeta = pstCurMeta;
			pstStackTop->iCount = 1;
			pstStackTop->idxEntry = 0;
			continue;
		}/*if ((TDR_TYPE_STRUCT == pstEntry->iType) && TDR_ENTRY_DO_EXTENDABLE(pstEntry))*/

		iRet = tdr_check_dbtable_name_conflict_i(a_pstBaseMeta, iMaxIdx, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName));
		if (TDR_ERR_IS_ERROR(iRet))
		{
			break;
		}
		TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
	}/*while (0 < iStackItemNum)*/

	if (TDR_ERR_IS_ERROR(iRet))
	{
		int i;

		/*��¼��ͻ����·��*/
		strcat(a_pszConflictName, stStack[0].pszHostBase);
		for (i = 1; i < iStackItemNum; i++)
		{
			strcat(a_pszConflictName, ".");
			strcat(a_pszConflictName, stStack[i].pszHostBase);
		}
	}/*if (TDR_ERR_IS_ERROR(iRet))*/


	return iRet;
}

int tdr_check_dbtable_name_conflict_i(LPTDRMETA a_pstBaseMeta, int iMaxIdx, const char *pszName)
{
	int iRet = TDR_SUCCESS;
	TDRSTACK stStack;
	LPTDRSTACKITEM pstStackTop;
	LPTDRMETAENTRY pstEntry;
	LPTDRMETA pstCurMeta;
	int iStackItemCount;
	LPTDRMETALIB pstLib;
	int iChange;

	assert(NULL != a_pstBaseMeta);
	assert(NULL != pszName);

	if (0 > iMaxIdx)
	{
		return TDR_SUCCESS;
	}

	pstStackTop = &stStack[0];
	pstCurMeta = a_pstBaseMeta;
	pstStackTop->pstMeta = pstCurMeta;
	pstStackTop->idxEntry = 0;
	pstStackTop->iCount = 1;
	pstStackTop->iMetaSizeInfoUnit = iMaxIdx;  /*ǰiMaxIdx��1����Ա���бȽ�*/
	pstStackTop->iChange = 0;
	iStackItemCount = 1;
	pstLib = TDR_META_TO_LIB(a_pstBaseMeta);

	while (0 < iStackItemCount)
	{
		if ((0 >= pstStackTop->iCount) || (pstStackTop->idxEntry > (int)pstStackTop->iMetaSizeInfoUnit))
		{
			iStackItemCount--;
			if (0 < iStackItemCount)
			{
				pstStackTop--;
				pstCurMeta = pstStackTop->pstMeta;
				TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			}
			continue;
		}

		pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;
		if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry))
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}
		if (TDR_ENTRY_IS_REFER_TYPE(pstEntry))
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}

		if ((TDR_TYPE_STRUCT == pstEntry->iType) && TDR_ENTRY_DO_EXTENDABLE(pstEntry))
		{
			if (TDR_STACK_SIZE <=  iStackItemCount)
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_TOO_COMPLIEX_META);
				break;
			}

			pstStackTop++;
			iStackItemCount++;
			pstCurMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
			pstStackTop->pstMeta = pstCurMeta;
			pstStackTop->iCount = 1;
			pstStackTop->idxEntry = 0;
			pstStackTop->iMetaSizeInfoUnit = pstCurMeta->iEntriesNum;
			pstStackTop->iChange = 0;
			continue;
		}/*if ((TDR_TYPE_STRUCT == pstEntry->iType) && TDR_ENTRY_DO_EXTENDABLE(pstEntry))*/

		if (0 == strcmp(TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName), pszName))
		{
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_NAME_CONFLICT);
			break;
		}
		TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
	}/*while (0 < iStackItemCount)*/

	return iRet;
}

int tdr_get_struct_meta_strictinput_attribute_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError)
{
	scew_attribute *pstAttr = NULL;
	int iRet = TDR_SUCCESS;


	assert(NULL != a_pstMeta);
	assert(NULL != a_pstMeta);
	assert(NULL != a_pstElement);
	assert(NULL != a_fpError);

	pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_STRICTINPUT);
	if( NULL != pstAttr )
	{
		char szVal[256] = {0};

		tdr_normalize_string(&szVal[0], sizeof(szVal), scew_attribute_value(pstAttr));
		if (0 == tdr_stricmp(szVal, TDR_TAG_TRUE))
		{
			TDR_META_SET_STRICT_INPUT(a_pstMeta);
		}else
		{
            LPTDRMETALIB pstLib = TDR_META_TO_LIB(a_pstMeta);
			fprintf(a_fpError, "error:\t �ṹ��<name = %s>��strictinput����ֵ<%s>��Ч,��ֵΪ\"false\"����\"true\"\n",
				TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), szVal);

			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_STRICTINPUT);
		}/*if ( 0== strcmp(pszVal, TDR_TAG_FALSE))*/
	}

	return iRet ;
}

/** ��
*/
int tdr_get_struct_meta_dependonstruct_attribute_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError)
{
	scew_attribute *pstAttr = NULL;
	int iRet = TDR_SUCCESS;
	char szName[TDR_NAME_LEN] = {0};
	LPTDRMETA pstDependOnMeta;
	LPTDRMETALIB pstLib;
	TDRSTACK stStack;
	LPTDRSTACKITEM pstStackTop;
	LPTDRMETAENTRY pstEntry;
	int iStackItemNum;
	LPTDRMETA pstCurMeta;
	int iChange;
	TDRDBKEYINFO stDBKeyInfo;

	assert(NULL != a_pstMeta);
	assert(NULL != a_pstMeta);
	assert(NULL != a_pstElement);
	assert(NULL != a_fpError);

	pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_DEPENDONSTRUCT);
	if (NULL == pstAttr)
	{
		return TDR_SUCCESS;
	}

	pstLib = TDR_META_TO_LIB(a_pstMeta);
	tdr_normalize_string(&szName[0], sizeof(szName), scew_attribute_value(pstAttr));
	pstDependOnMeta = tdr_get_meta_by_name_i(pstLib, szName);
	if ((NULL ==  pstDependOnMeta) || (TDR_TYPE_UNION == pstDependOnMeta->iType) ||
		(0 >= pstDependOnMeta->iNUnitSize))
	{
		fprintf(a_fpError, "error:\t �ṹ��<name = %s>��dependonstruc����ֵ<%s>��Ч,��ֵ����Ϊ֮ǰ�Ѿ�����structԪ���ݵ����֡�\n",
			TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), szName);
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_DEPENDONSTRUCT);
	}
	a_pstMeta->ptrDependonStruct = pstDependOnMeta->ptrMeta;


	/*��ǰmeta�ж���ĳ�Ա������dependonstructԪ�����ж���ĳ�Ա*/
	pstStackTop = &stStack[0];
	pstStackTop->pstMeta = a_pstMeta;
	pstStackTop->iCount = 1;
	pstStackTop->idxEntry = 0;
	iStackItemNum = 1;
	pstCurMeta = a_pstMeta;

	while (0 < iStackItemNum)
	{
		LPTDRMETAENTRY pstFindEntry;

		if (0 >= pstStackTop->iCount)
		{
			iStackItemNum--;
			if (0 < iStackItemNum)
			{
				pstStackTop--;
				pstCurMeta = pstStackTop->pstMeta;
				TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			}
			continue;
		}

		pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;
		if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry))
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}
		if (TDR_ENTRY_IS_REFER_TYPE(pstEntry))
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}


		if ((TDR_TYPE_STRUCT == pstEntry->iType) && TDR_ENTRY_DO_EXTENDABLE(pstEntry))
		{
			LPTDRMETA pstTypeMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);

			/*���˳�Ա�ͻ����ṹ��Ա�Ƿ�����*/
			if (TDR_STACK_SIZE <=  iStackItemNum)
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_TOO_COMPLIEX_META);
				break;
			}

			pstStackTop++;
			iStackItemNum++;
			pstCurMeta = pstTypeMeta;
			pstStackTop->pstMeta = pstCurMeta;
			pstStackTop->iCount = 1;
			pstStackTop->idxEntry = 0;
			continue;
		}/*if ((TDR_TYPE_STRUCT == pstEntry->iType) && TDR_ENTRY_DO_EXTENDABLE(pstEntry))*/

		iRet = tdr_find_tablecolumn_entry_i(&stDBKeyInfo, pstDependOnMeta, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName));
		if (TDR_ERR_IS_ERROR(iRet))
		{
			fprintf(a_fpError, "error:\t �ṹ��<name = %s>��ָ��dependonstruc����ֵ<%s>����˽ṹ��<name = %s>�ж���ĳ�Ա�������ڽṹ��<name = %s>�ж����\n",
				TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), szName, TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName),
                TDR_GET_STRING_BY_PTR(pstLib,pstDependOnMeta->ptrName));
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_DEPENDONSTRUCT);
			break;
		}

		/*���������Ա�Ƿ�ƥ��*/
		pstFindEntry = TDR_PTR_TO_ENTRY(pstLib, stDBKeyInfo.ptrEntry);
		if (pstFindEntry->iCount != pstEntry->iCount)
		{
			fprintf(a_fpError, "error:\t �ṹ��<name = %s>ָ����dependonstruc�����Ա<name=%s>��count����ֵ<%d>�����������ṹ��<name = %s>��Ӧ��Ա��count����ֵ<%d>��ͬ\n",
				TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName), pstEntry->iCount, TDR_GET_STRING_BY_PTR(pstLib,pstDependOnMeta->ptrName), pstFindEntry->iCount);
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_DEPENDONSTRUCT);
			break;
		}
		if (pstFindEntry->iHRealSize != pstEntry->iHRealSize)
		{
			fprintf(a_fpError, "error:\t �ṹ��<name = %s>ָ����dependonstruc�����Ա<name=%s>�Ĵ洢�ռ�<%"TDRPRI_SIZET">�����������ṹ��<name = %s>��Ӧ��Ա�Ĵ洢�ռ�<%"TDRPRI_SIZET">��ͬ\n",
				TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName), pstEntry->iHRealSize, TDR_GET_STRING_BY_PTR(pstLib,pstDependOnMeta->ptrName), pstFindEntry->iHRealSize);
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_DEPENDONSTRUCT);
			break;
		}
		if ((pstFindEntry->iType != pstEntry->iType)|| (pstFindEntry->ptrMeta != pstEntry->ptrMeta))
		{
			fprintf(a_fpError, "error:\t �ṹ��<name = %s>ָ����dependonstruc�����Ա<name=%s>����������<%d>�����������ṹ��<name = %s>��Ӧ��Ա����������<%d>��ͬ\n",
				TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName), pstEntry->iType, TDR_GET_STRING_BY_PTR(pstLib,pstDependOnMeta->ptrName), pstFindEntry->iType);
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_DEPENDONSTRUCT);
			break;
		}


		TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
	}/*while (0 < iStackItemNum)*/


	return iRet;
}


int tdr_find_tablecolumn_entry_i(IN LPTDRDBKEYINFO a_pstDBKeyInfo, IN LPTDRMETA a_pstMeta, IN const char *a_pszName)
{
	int iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_DEPENDONSTRUCT);
	TDRSTACK stStack;
	LPTDRSTACKITEM pstStackTop;
	LPTDRMETAENTRY pstEntry;
	LPTDRMETA pstCurMeta;
	int iStackItemCount;
	LPTDRMETALIB pstLib;
	int iChange;


	assert(NULL != a_pstDBKeyInfo);
	assert(NULL != a_pstMeta);
	assert(NULL != a_pszName);

	pstStackTop = &stStack[0];
	pstCurMeta = a_pstMeta;
	pstStackTop->pstMeta = pstCurMeta;
	pstStackTop->idxEntry = 0;
	pstStackTop->iCount = 1;
	iStackItemCount = 1;
	pstStackTop->iMetaSizeInfoOff = 0; /*meta�ı���*/
	pstLib = TDR_META_TO_LIB(pstCurMeta);


	while (0 < iStackItemCount)
	{
		if (0 >= pstStackTop->iCount)
		{
			iStackItemCount--;
			if (0 < iStackItemCount)
			{
				pstStackTop--;
				pstCurMeta = pstStackTop->pstMeta;
				TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			}
			continue;
		}

		pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;
		if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry))
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}
		if (TDR_ENTRY_IS_REFER_TYPE(pstEntry))
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}

		if ((TDR_TYPE_STRUCT == pstEntry->iType) && TDR_ENTRY_DO_EXTENDABLE(pstEntry))
		{
			if (TDR_STACK_SIZE <=  iStackItemCount)
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_TOO_COMPLIEX_META);
				break;
			}

			pstStackTop++;
			iStackItemCount++;
			pstCurMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
			pstStackTop->pstMeta = pstCurMeta;
			pstStackTop->iCount = 1;
			pstStackTop->idxEntry = 0;
			pstStackTop->iMetaSizeInfoOff = (pstStackTop-1)->iMetaSizeInfoOff + pstEntry->iHOff;
			continue;
		}/*if ((TDR_TYPE_STRUCT == pstEntry->iType) && TDR_ENTRY_DO_EXTENDABLE(pstEntry))*/

		if (0 == strcmp(TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName), a_pszName))
		{
			a_pstDBKeyInfo->iHOff = pstStackTop->iMetaSizeInfoOff + pstEntry->iHOff;
			a_pstDBKeyInfo->ptrEntry = (TDRPTR)((char *)pstEntry - pstLib->data);
			iRet = TDR_SUCCESS;
			break;
		}
		TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
	}/*while (0 < iStackItemCount)*/

	return iRet;
}

int tdr_get_cname_attribute_i(TDRPTR *a_pptrName, LPTDRMETALIB a_pstLib, scew_element *a_pstElement, FILE *a_fpError)
{
	scew_attribute *pstAttr = NULL;
	size_t iStrLen = 0;
	int iRet = TDR_SUCCESS;

	assert(NULL != a_pptrName);
	assert(NULL != a_pstLib);
	assert(NULL != a_pstElement);
	assert(NULL != a_fpError);

	*a_pptrName = TDR_INVALID_PTR;

	pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_CNNAME);
	if( NULL != pstAttr )
	{
		const char *pszName = scew_attribute_value(pstAttr);

		assert(NULL != pszName);
		iStrLen = strlen(pszName) + 1; /*��1��Ϊ�˱��������ַ�*/

		/*̫�����ض�*/
		if (iStrLen > TDR_CHNAME_LEN)
		{
			iStrLen = TDR_CHNAME_LEN;
			fprintf(a_fpError, "\nwarning:\t <cname = %s>������󳤶�<%d>,����ض�",
				pszName, TDR_CHNAME_LEN);
			iRet = TDR_SUCCEESS_WARNING(TDR_SUCWARN_CNAME_BE_TRANCATED);
		}

		if ((1 < iStrLen) && (iStrLen > a_pstLib->iFreeStrBufSize))
		{
			fprintf(a_fpError, "\nerror:\t metalib�ַ���������ʣ��ռ�<freeSpace = %"TDRPRI_SIZET">, ���ܴ洢cname����: %s",
				a_pstLib->iFreeStrBufSize, pszName);

			iRet = TDR_ERRIMPLE_NO_SPACE_FOR_STRBUF;
		}else
		{
			TDR_COPY_STRING_TO_BUF(*a_pptrName, pszName, iStrLen, a_pstLib);
		}
	}

	return iRet;
}

int tdr_get_name_attribute_i(char *pszName, int iNameSize, scew_element *pstElement)
{
	scew_attribute *pstAttr = NULL;
	int iRet = TDR_SUCCESS;


	assert(NULL != pszName);
	assert(0 < iNameSize);
	assert(NULL != pstElement);



	pstAttr = scew_attribute_by_name(pstElement, TDR_TAG_NAME);
	if (NULL != pstAttr)
	{
		tdr_normalize_string(pszName, iNameSize, scew_attribute_value(pstAttr));
	}else
	{
		iRet = TDR_SUCCEESS_WARNING(TDR_SUCWARN_NO_NAME);
	}

	return iRet;
}

int tdr_get_id_attribute_i(int *a_piID, int *a_piIdx, LPTDRMETALIB a_pstLib, scew_element *a_pstElement, FILE *a_fpError)
{
	scew_attribute *pstAttr = NULL;
	int iRet = TDR_SUCCESS;

	assert(NULL != a_piID);
	assert(NULL != a_piIdx);
	assert(NULL != a_pstLib);
	assert(NULL != a_pstElement);
	assert(NULL != a_fpError);

	pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_ID);

	if( NULL != pstAttr )
	{
		char szID[1024] = {0};

		tdr_normalize_string(&szID[0], sizeof(szID), scew_attribute_value(pstAttr));
		iRet = tdr_get_macro_int_i(a_piID, a_piIdx, a_pstLib, &szID[0]);
		if (TDR_ERR_IS_ERROR(iRet))
		{
			fprintf(a_fpError, "error:\t����ȷ��ID����ֵ<id = %s>,���������ֵΪ�꣬��ȷ���˺��Ѿ�����.\n", szID);
			iRet = TDR_ERRIMPLE_ENTRY_INVALID_ID_VALUE;
		}
	}else
	{
		iRet = TDR_SUCCEESS_WARNING(TDR_SUCWARN_NO_ID);
	}

	return iRet;
}

int tdr_get_desc_attribute_i(TDRPTR *a_pptrDesc, LPTDRMETALIB a_pstLib, scew_element *a_pstElement, FILE *a_fpError)
{
	scew_attribute *pstAttr = NULL;
	size_t iStrLen = 0;
	int iRet = TDR_SUCCESS;

	assert(NULL != a_pptrDesc);
	assert(NULL != a_pstLib);
	assert(NULL != a_pstElement);
	assert(NULL != a_fpError);

	*a_pptrDesc = TDR_INVALID_PTR;

	pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_DESCIPTION);
	if( NULL != pstAttr )
	{
		const char *pszName = scew_attribute_value(pstAttr);

		assert(NULL != pszName);
		iStrLen = strlen(pszName) + 1; /*��1��Ϊ�˱��������ַ�*/

		/*̫�����ض�*/
		if (iStrLen > TDR_DESC_LEN)
		{
			iStrLen = TDR_DESC_LEN;
			fprintf(a_fpError, "\nwarning:\t <desc = %s>����ֵ������󳤶�<%d>,����ض�",
				pszName, TDR_DESC_LEN);
			iRet = TDR_SUCCEESS_WARNING(TDR_SUCWARN_DESC_VALUE_BE_TRANCATED);
		}

		if ((1 < iStrLen) && (iStrLen > a_pstLib->iFreeStrBufSize))
		{
			/*�ַ������������пռ䲻��*/
			fprintf(a_fpError, "\nerror:\t metalib�ַ���������ʣ��ռ�<freeSpace = %"TDRPRI_SIZET">, ���ܴ洢desc����: %s",
				a_pstLib->iFreeStrBufSize, pszName);

			iRet = TDR_ERRIMPLE_NO_SPACE_FOR_STRBUF;
		}else
		{
			TDR_COPY_STRING_TO_BUF(*a_pptrDesc, pszName, iStrLen, a_pstLib);
		}
	}

	return iRet;
}


int tdr_get_version_attribute_i(int *a_piVersion, int *a_piIdxVersion, LPTDRMETALIB a_pstLib, scew_element *a_pstElement, FILE *a_fpError)
{
	scew_attribute *pstAttr = NULL;
	int iRet = TDR_SUCCESS;

	assert(NULL != a_piVersion);
	assert(NULL != a_piIdxVersion);
	assert(NULL != a_pstLib);
	assert(NULL != a_pstElement);
	assert(NULL != a_fpError);

	pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_VERSION);

	if( NULL != pstAttr )
	{
		char szVersion[1024] = {0};

		tdr_normalize_string(&szVersion[0], sizeof(szVersion), scew_attribute_value(pstAttr));

		iRet = tdr_get_macro_int_i(a_piVersion, a_piIdxVersion, a_pstLib, &szVersion[0]);
		if (TDR_ERR_IS_ERROR(iRet))
		{
			fprintf(a_fpError, "\nerror:\t����ȷ��version����ֵ<version = %s>,���������ֵΪ�꣬��ȷ���˺��Ѿ�����.", szVersion);
		}
	}else
	{
		iRet =  TDR_SUCCEESS_WARNING(TDR_SUCWARN_NO_VERSION);
	}

	return iRet;
}

int tdr_get_meta_sortkey_attribute_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError)
{
	scew_attribute *pstAttr = NULL;

	LPTDRMETALIB pstLib =NULL;
	int iRet = TDR_SUCCESS;

	const char *pszPtr = NULL;
	char szBuff[TDR_NAME_LEN] = {0};


	assert(NULL != a_pstElement);
	assert(NULL != a_pstMeta);
	assert(NULL != a_fpError);


	pstLib = TDR_META_TO_LIB(a_pstMeta);
	pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_SORTKEY);
	if( NULL == pstAttr )
	{
		return TDR_SUCCESS;
	}
	if (TDR_TYPE_UNION == a_pstMeta->iType)
	{
		fprintf(a_fpError, "\nwarnning:\t %sΪunion��������,Ŀǰ��֧��sortkey����,�����Դ�����sortkey����",
			TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName));
		return TDR_SUCCESS;
	}

	pszPtr = scew_attribute_value(pstAttr);
	pszPtr = tdr_get_first_node_name_from_path_i(szBuff, sizeof(szBuff), pszPtr);
	tdr_trim_str(szBuff);
	if (0 != tdr_stricmp(szBuff, TDR_TAG_THIS))
	{
		pszPtr = scew_attribute_value(pstAttr);
	}


	/*����sortkey��path��Ϣ�ּ���λsortkey�������е�λ����Ϣ*/
	iRet = tdr_parse_meta_sortkey_info_i(&a_pstMeta->stSortKey, a_pstMeta, pszPtr);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		fprintf(a_fpError, "\nerror:\t %s��sortkey����ֵ<%s>��Ч��������ֵֻ���ǵ�ǰԪ��Ϊ������Ԫ��,��ȷ��ָ����ȷ��Ԫ������·������·�����漰�ĸ�Ԫ�ز���ָ�����������",
			TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), scew_attribute_value(pstAttr));
	}

	return iRet;
}

/** ���궨����ӵ�Ԫ������������
*@param[in,out] a_pstLib ��Ҫ���µ�Ԫ���ݿ�
*@param[in] a_pstElement �궨����Ԫ��
*@param[in] a_fpError ������¼���������Ϣ���ļ����
*
*@return �ɹ�����TDR_SUCCESS,���򷵻ش����
*
*@pre \e a_pstLib ����ΪNULL
*@pre \e a_pstElement ����ΪNULL
*@pre \e a_fpError ����ΪNULL
*/
int tdr_add_macrosgroup_i(IN TDRMETALIB* a_pstLib, IN scew_element *a_pstElement, IN FILE *a_fpError)
{
	int iRet = TDR_SUCCESS;
	LPTDRMACROSGROUP pstMacrosGroup = NULL;

	assert(NULL != a_pstLib);
	assert(NULL != a_pstElement);
	assert(NULL != a_fpError);

	/*����Ƿ����㹻�Ĵ洢�ռ䱣��˺궨����*/
	iRet = tdr_create_macrosgroup_i(&pstMacrosGroup, a_pstLib, a_pstElement, a_fpError);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		return iRet;
	}


	/*��ȡ�궨����Ļ�����Ϣ*/
	iRet = tdr_get_macrosgroup_attributes(pstMacrosGroup, a_pstLib, a_pstElement, a_fpError);

	/*��Ӵ˺궨����ĺ�,����¼���궨����ṹ��*/
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		iRet = tdr_add_macros_of_group_i(pstMacrosGroup, a_pstLib, a_pstElement, a_fpError);
	}

	/*���滯����궨�������ݽṹ������������������*/
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		tdr_sort_macrosgroup_nameidxmap_i(pstMacrosGroup, a_pstLib);
		tdr_sort_macrosgroup_valueidxmap_i(pstMacrosGroup, a_pstLib);

		a_pstLib->iCurMacrosGroupNum++;
	}

	return iRet;
}

int  tdr_create_macrosgroup_i(INOUT LPTDRMACROSGROUP *a_ppstMacrosGroup, IN TDRMETALIB* a_pstLib, IN scew_element *a_pstElement, IN FILE *a_fpError)
{

	int iMacro = 0;
	size_t iGroupSize = 0;
	size_t iStrLen = 0;
	size_t iUsedSize;
	LPTDRMAPENTRY pstMap;
	LPTDRMACROSGROUP pstGroup;
	TDRIDX *pIndex;

	assert(NULL != a_pstLib);
	assert(NULL != a_pstElement);
	assert(NULL != a_fpError);

	if (a_pstLib->iCurMacrosGroupNum >= a_pstLib->iMaxMacrosGroupNum)
	{
		fprintf(a_fpError, "Error:Ԫ���ݿ�����ܴ洢%d���궨���飬Ŀǰ�Դﵽ�����Ŀ����������Ӻ궨����",
			a_pstLib->iMaxMacrosGroupNum);
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_MACROGSROUP_ATTRIBUTE);
	}

	/*���궨����ʣ��ռ�*/
	tdr_get_macrosgroup_size_i(&iMacro, &iGroupSize, &iStrLen, a_pstElement);
	iUsedSize = tdr_get_macrogroup_used_size_i(a_pstLib);
	if ((a_pstLib->iMacrosGroupSize - iUsedSize) < iGroupSize)
	{
		fprintf(a_fpError, "Error:Ԫ���ݿ����ڴ洢�궨�����ʣ��ռ�Ϊ%"TDRPRI_SIZET"�ֽڣ��µĺ궨������Ҫ%"TDRPRI_SIZET"���ֽڵĴ洢�ռ�",
			a_pstLib->iMacrosGroupSize - iUsedSize, iGroupSize);
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_MACROGSROUP_ATTRIBUTE);
	}

	/*����ַ�������Ŀռ��Ƿ��㹻*/
	if (a_pstLib->iFreeStrBufSize < iStrLen)
	{
		fprintf(a_fpError, "\nerror:\t ����궨����ʧ�ܣ�Ԫ�����������ַ���������ʣ��ռ�Ϊ%"TDRPRI_SIZET"�ֽڣ���ʵ����Ҫ%"TDRPRI_SIZET"���ֽ�\n",
			a_pstLib->iFreeStrBufSize, iStrLen);

		return TDR_ERRIMPLE_NO_SPACE_FOR_STRBUF;
	}

	/*��������*/
	pstMap = TDR_GET_MACROSGROUP_MAP_TABLE(a_pstLib);
	pstMap[a_pstLib->iCurMacrosGroupNum].iPtr = a_pstLib->ptrMacrosGroup + iUsedSize;
	pstMap[a_pstLib->iCurMacrosGroupNum].iSize = iGroupSize;

	pstGroup = TDR_PTR_TO_MACROSGROUP(a_pstLib, pstMap[a_pstLib->iCurMacrosGroupNum].iPtr);
	memset(pstGroup, 0, offsetof(TDRMACROSGROUP, data));
	pstGroup->ptrDesc = TDR_INVALID_PTR;
	pstGroup->iMaxMacroCount = iMacro;
	pstGroup->ptrNameIdxMap = offsetof(TDRMACROSGROUP, data);
	pstGroup->ptrValueIdxMap = pstGroup->ptrNameIdxMap + sizeof(int)*iMacro;
	pIndex = TDR_GET_MACROSGROUP_NAMEIDXMAP_TAB(pstGroup);
	memset(pIndex, TDR_INVALID_INDEX, sizeof(TDRIDX)*iMacro);
	pIndex = TDR_GET_MACROSGROUP_VALUEIDXMAP_TAB(pstGroup);
	memset(pIndex, TDR_INVALID_INDEX, sizeof(TDRIDX)*iMacro);
	*a_ppstMacrosGroup = pstGroup;

	return TDR_SUCCESS;
}

/*ȡ�궨�����������Ѿ�ʹ�õĿռ�*/
size_t tdr_get_macrogroup_used_size_i(IN LPTDRMETALIB a_pstLib)
{
	size_t iSize = 0;
	int i;
	LPTDRMAPENTRY pstMap;

	assert(NULL != a_pstLib);

	pstMap = TDR_GET_MACROSGROUP_MAP_TABLE(a_pstLib);
	for (i =0; i < a_pstLib->iCurMacrosGroupNum; i++)
	{
		iSize += pstMap->iSize;
		pstMap++;
	}

	return iSize;
}

/** ��ȡmacrosgroupԪ�ص�����
*@return �ɹ�����0�����򷵻ط���ֵ
*/
int tdr_get_macrosgroup_attributes(IN LPTDRMACROSGROUP a_pstMacrosGroup, IN TDRMETALIB* a_pstLib, IN scew_element *a_pstElement, IN FILE *a_fpError)
{
	int iRet = TDR_SUCCESS;

	assert(NULL != a_pstMacrosGroup);
	assert(NULL != a_pstLib);
	assert(NULL != a_pstElement);
	assert(NULL != a_fpError);


	/*��ȡname ����*/
	iRet = tdr_get_name_attribute_i(&a_pstMacrosGroup->szName[0], sizeof(a_pstMacrosGroup->szName), a_pstElement);
	if (a_pstMacrosGroup->szName[0] == '\0') /*�մ���û�ж���*/
	{
		fprintf(a_fpError, "error:\t ��%d���궨����<%s>Ԫ��û�ж���name���Ի���ֵΪ�մ�\n",
			a_pstLib->iCurMacrosGroupNum + 1, TDR_TAG_MACROSGROUP);
		iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_MACROGSROUP_ATTRIBUTE);;
	}else
	{
		/*�ж��Ƿ��������ĺ궨�������*/
		LPTDRMACROSGROUP pstTmpGroup = tdr_find_macrosgroup_i(a_pstLib, a_pstMacrosGroup->szName);
		if (NULL != pstTmpGroup)
		{
			fprintf(a_fpError, "error:\t Ԫ����������<%s>������������<name = %s>��ͬ�ĺ궨����(%s)Ԫ��\n",
				a_pstLib->szName, a_pstMacrosGroup->szName, TDR_TAG_MACROSGROUP);
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_MACROGSROUP_ATTRIBUTE);;
		}
	}/*if (a_pstMacrosGroup->szName[0] == '\0')*/

	/*desc*/
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		iRet = tdr_get_desc_attribute_i(&a_pstMacrosGroup->ptrDesc, a_pstLib, a_pstElement, a_fpError);
	}

	return iRet;
}

/**���ݺ궨��������Ʋ��Ҵ˺궨�������ݽṹ
*/
LPTDRMACROSGROUP tdr_find_macrosgroup_i(IN const LPTDRMETALIB a_pstLib, IN const char *a_pszName)
{
	int i;
	LPTDRMAPENTRY pstMap;
	LPTDRMACROSGROUP pstTmpGroup = NULL;

	assert(NULL != a_pstLib);
	assert(NULL != a_pszName);

	pstMap = TDR_GET_MACROSGROUP_MAP_TABLE(a_pstLib);
	for (i = 0; i < a_pstLib->iCurMacrosGroupNum; i++)
	{
		pstTmpGroup = TDR_PTR_TO_MACROSGROUP(a_pstLib, pstMap[i].iPtr);
		if (0 == tdr_stricmp(pstTmpGroup->szName, a_pszName))
		{
			break;
		}
	}
	if (i >= a_pstLib->iCurMacrosGroupNum)
	{
		pstTmpGroup = NULL;
	}

	return pstTmpGroup;
}

/** ��macrosgroupԪ�ص�macro��Ԫ�ؼ��뵽Ԫ���ݿ���
*@return �ɹ�����0�����򷵻ط���ֵ
*/
int tdr_add_macros_of_group_i(IN LPTDRMACROSGROUP a_pstMacrosGroup, IN TDRMETALIB* a_pstLib, IN scew_element *a_pstElement, IN FILE *a_fpError)
{
	int iRet = TDR_SUCCESS;
	scew_element* pstItem = NULL;
	TDRIDX *pNameIdxTab;
	TDRIDX *pValueIdxTab;
	LPTDRMACRO pstMacro = NULL;
	LPTDRMACRO pstMacroTable = NULL;
	int i;

	assert(NULL != a_pstMacrosGroup);
	assert(NULL != a_pstLib);
	assert(NULL != a_pstElement);
	assert(NULL != a_fpError);

	pstItem = scew_element_next(a_pstElement, NULL);
	pNameIdxTab = TDR_GET_MACROSGROUP_NAMEIDXMAP_TAB(a_pstMacrosGroup);
	pValueIdxTab = TDR_GET_MACROSGROUP_VALUEIDXMAP_TAB(a_pstMacrosGroup);
	pstMacroTable = TDR_GET_MACRO_TABLE(a_pstLib);
	while (NULL != pstItem)
	{
		if (0 != tdr_stricmp(scew_element_name(pstItem), TDR_TAG_MACRO))
		{
			pstItem = scew_element_next(a_pstElement, pstItem);
			continue;
		}

		/*���ռ��Ƿ��㹻*/
		if( a_pstLib->iCurMacroNum >= a_pstLib->iMaxMacroNum )
		{

			fprintf(a_fpError, "error:\t metalib����ܴ洢%d���궨�壬Ŀǰ�Ѵ洢%d��,�����ټ���궨��.\n",
				a_pstLib->iMaxMacroNum, a_pstLib->iCurMacroNum);
			iRet =  TDR_ERRIMPLE_TO_MUCH_MACROS;
			break;
		}
		if (a_pstMacrosGroup->iCurMacroCount >= a_pstMacrosGroup->iMaxMacroCount)
		{
			fprintf(a_fpError, "error:\t �궨����<name=%s>����ܴ洢%d���궨�壬Ŀǰ�Ѵ洢%d��,�����ټ���궨��.\n",
				a_pstMacrosGroup->szName, a_pstMacrosGroup->iMaxMacroCount, a_pstMacrosGroup->iCurMacroCount);
			iRet =  TDR_ERRIMPLE_TO_MUCH_MACROS;
			break;
		}

		/*���궨�����Ԫ����������궨�����*/
		iRet = tdr_add_macro_i(a_pstLib, pstItem, a_fpError, &pstMacro);
		if (TDR_ERR_IS_ERROR(iRet))
		{
			break;
		}

		/*���궨�������Ƿ���ֵ��ͬ�ĺ����*/
		for (i = 0; i < a_pstMacrosGroup->iCurMacroCount; i++)
		{
			LPTDRMACRO pstTmpMacro = pstMacroTable + pValueIdxTab[i];
			if (pstTmpMacro->iValue == pstMacro->iValue)
			{
				fprintf(a_fpError, "error:\t �궨����<name=%s>�к궨��<name=%s>��ֵ<value=%d>��궨��<name=%s>��ֵ��ͬ\n",
					a_pstMacrosGroup->szName, TDR_GET_STRING_BY_PTR(a_pstLib,pstTmpMacro->ptrMacro),
                    pstTmpMacro->iValue, TDR_GET_STRING_BY_PTR(a_pstLib,pstMacro->ptrMacro));
				iRet =  TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_MACROGSROUP_ATTRIBUTE);;
				break;
			}
		}/*for (i = 0; i < a_pstMacrosGroup->iCurMacroCount; i++)*/
		if (TDR_ERR_IS_ERROR(iRet))
		{
			break;
		}

		/*�궨�����м�¼�˺궨��*/
		pNameIdxTab[a_pstMacrosGroup->iCurMacroCount] = (TDRIDX)(pstMacro - pstMacroTable);
		pValueIdxTab[a_pstMacrosGroup->iCurMacroCount] = pNameIdxTab[a_pstMacrosGroup->iCurMacroCount];
		a_pstMacrosGroup->iCurMacroCount++;

		pstItem = scew_element_next(a_pstElement, pstItem);
	}/*while (NULL != pstItem)*/

	return iRet;
}

void tdr_sort_macrosgroup_nameidxmap_i(IN LPTDRMACROSGROUP a_pstMacrosGroup, IN TDRMETALIB* a_pstLib)
{
	int i,j;
	LPTDRMACRO pstMacro1 = NULL;
	LPTDRMACRO pstMacro2 = NULL;
	LPTDRMACRO pstMacroTable = NULL;
	TDRIDX *pNameIdxTab;
	TDRIDX idxTmp;


	assert(NULL != a_pstMacrosGroup);
	assert(NULL != a_pstLib);

	/*ð������*/
	pstMacroTable = TDR_GET_MACRO_TABLE(a_pstLib);
	pNameIdxTab = TDR_GET_MACROSGROUP_NAMEIDXMAP_TAB(a_pstMacrosGroup);
	for (i = 0; i < a_pstMacrosGroup->iCurMacroCount -1; i++)
	{
		for (j = i+1; j < a_pstMacrosGroup->iCurMacroCount; j++)
		{
			pstMacro1 = pstMacroTable + pNameIdxTab[i];
			pstMacro2 = pstMacroTable + pNameIdxTab[j];
			if (0 < tdr_stricmp(TDR_GET_STRING_BY_PTR(a_pstLib,pstMacro1->ptrMacro),
                TDR_GET_STRING_BY_PTR(a_pstLib,pstMacro2->ptrMacro)))
			{
				idxTmp = pNameIdxTab[i];
				pNameIdxTab[i] = pNameIdxTab[j];
				pNameIdxTab[j] = idxTmp;
			}
		}/*for (j = i+1; j < a_pstMacrosGroup->iCurMacroCount; j++)*/
	}/*for (i = 0; i < a_pstMacrosGroup->iCurMacroCount -1; i++)*/
}

void tdr_sort_macrosgroup_valueidxmap_i(IN LPTDRMACROSGROUP a_pstMacrosGroup, IN TDRMETALIB* a_pstLib)
{
	int i,j;
	LPTDRMACRO pstMacro1 = NULL;
	LPTDRMACRO pstMacro2 = NULL;
	LPTDRMACRO pstMacroTable = NULL;
	TDRIDX *pValueIdxTab;
	TDRIDX idxTmp;

	assert(NULL != a_pstMacrosGroup);
	assert(NULL != a_pstLib);

	/*ð������*/
	pstMacroTable = TDR_GET_MACRO_TABLE(a_pstLib);
	pValueIdxTab = TDR_GET_MACROSGROUP_VALUEIDXMAP_TAB(a_pstMacrosGroup);
	for (i = 0; i < a_pstMacrosGroup->iCurMacroCount -1; i++)
	{
		for (j = i+1; j < a_pstMacrosGroup->iCurMacroCount; j++)
		{
			pstMacro1 = pstMacroTable + pValueIdxTab[i];
			pstMacro2 = pstMacroTable + pValueIdxTab[j];
			if (pstMacro1->iValue > pstMacro2->iValue)
			{
				idxTmp = pValueIdxTab[i];
				pValueIdxTab[i] = pValueIdxTab[j];
				pValueIdxTab[j] = idxTmp;

			}
		}/*for (j = i+1; j < a_pstMacrosGroup->iCurMacroCount; j++)*/
	}/*for (i = 0; i < a_pstMacrosGroup->iCurMacroCount -1; i++)*/
}

int tdr_get_struct_meta_uniqueentryname_attribute_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError)
{
	scew_attribute *pstAttr = NULL;

	LPTDRMETALIB pstLib =NULL;
	int iRet = TDR_SUCCESS;
	const char *pszPtr = NULL;
	char szVal[TDR_NAME_LEN] = {0};


	assert(NULL != a_pstElement);
	assert(NULL != a_pstMeta);
	assert(NULL != a_fpError);


	pstLib = TDR_META_TO_LIB(a_pstMeta);
	pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_UNIQUEENTRYNAME);
	if( NULL == pstAttr )
	{
		return TDR_SUCCESS;
	}
	if (TDR_TYPE_UNION == a_pstMeta->iType)
	{
		fprintf(a_fpError, "\nwarnning:\t ���Ͻṹ<name = %s>Ϊunion��������,Ŀǰ��֧��%s����,�����Խ�����",
			TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_TAG_UNIQUEENTRYNAME);
		return TDR_SUCCESS;
	}

	pszPtr = scew_attribute_value(pstAttr);
	tdr_normalize_string(szVal, sizeof(szVal), pszPtr);
	if (0 == tdr_stricmp(szVal, TDR_TAG_TRUE))
	{
		TDR_META_CLR_NEED_PREFIX(a_pstMeta);
	}else if ( 0== tdr_stricmp(szVal, TDR_TAG_FALSE))
	{
		TDR_META_SET_NEED_PREFIX(a_pstMeta);
	}else
	{
		fprintf(a_fpError, "error:\t �ṹ��Ԫ��<name = %s>��%s����ֵ<%s>��Ч,��ֵΪ\"false\"����\"true\"\n",
			TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_TAG_UNIQUEENTRYNAME, szVal);

		iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_UNIQUEENTRYNAMEVALUE);
	}/*if ( 0== tdr_stricmp(pszVal, TDR_TAG_FALSE))*/

	return iRet;
}

