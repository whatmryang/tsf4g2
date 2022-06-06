/**
*
* @file     tdr_metalib_entry_manage_i.h
* @brief    TSF4G-Ԫ����������entryԪ�ع���ģ��
*
* @author steve jackyai
* @version 1.0
* @date 2007-04-16
*
*
* Copyright (c)  2007, ��Ѷ�Ƽ����޹�˾���������з���
* All rights reserved.
*
*/

#include <assert.h>
#include <stdlib.h>
#include <wchar.h>
#include "tdr/tdr_os.h"
#include "tdr/tdr_define_i.h"
#include "tdr/tdr_ctypes_info_i.h"
#include "tdr/tdr_ctypes_info.h"
#include "tdr_XMLtags_i.h"
#include "tdr/tdr_metalib_kernel_i.h"
#include "tdr_metalib_entry_manage_i.h"
#include "tdr_metalib_meta_manage_i.h"
#include "tdr/tdr_error_i.h"
#include "tdr/tdr_metalib_manage_i.h"
#include "tdr/tdr_auxtools.h"

#if defined (_WIN32) || defined (_WIN64)
#pragma warning(disable:4996)
#endif

static void tdr_init_entry_data_i(LPTDRMETAENTRY a_pstEntry, LPTDRMETA a_pstMeta);
static void tdr_init_index_data_i(LPTDRMETAINDEX a_pstIndex, LPTDRMETA a_pstMeta);

/** ��ȡentry����Ҫ����ƫ�ƵĹ�������: id version name cname desc
*@param[out] a_pstEntry �����Ԫ����Ϣ�����ݽṹ
*@param[in] a_pstMeta pstEntry���ڵ��Զ���ṹ
*@param[in] a_pstElement Ҫ�����Ԫ��
*@param[in] a_fpError ���������Ϣ���ļ����
*
*@return �ɹ�����TDR_SUCCESS�����򷵻ش������
*
*@pre \e a_pstEntry ����ΪNULL
*@pre \e a_pstMeta ����ΪNULL
*@pre \e a_pstElement ����ΪNULL
*@pre \e a_fpError ����ΪNULL
*/
static int tdr_get_entry_no_off_attribute_i(LPTDRMETAENTRY a_pstEntry, LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError);

/** ��ȡindex����Ҫ����ƫ�ƵĹ�������: version name
 *@param[out] a_pstIndex �����Ԫ����Ϣ�����ݽṹ
 *@param[in] a_pstMeta pstEntry���ڵ��Զ���ṹ
 *@param[in] a_pstElement Ҫ�����Ԫ��
 *@param[in] a_fpError ���������Ϣ���ļ����
 *
 *@return �ɹ�����TDR_SUCCESS�����򷵻ش������
 *
 *@pre \e a_pstIndex ����ΪNULL
 *@pre \e a_pstMeta ����ΪNULL
 *@pre \e a_pstElement ����ΪNULL
 *@pre \e a_fpError ����ΪNULL
 */
static int tdr_get_index_no_off_attribute_i(LPTDRMETAINDEX a_pstIndex, LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError);

/**��ȡentry��type����
*/
static int tdr_get_entry_type_i(LPTDRMETAENTRY a_pstEntry, LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError);

static int tdr_get_entry_count_attribute_i(LPTDRMETAENTRY a_pstEntry, LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError);


/**���entry���ݵ���Ч��
*/
static int tdr_check_entry_i(LPTDRMETAENTRY a_pstEntry, LPTDRMETA a_pstMeta, FILE *a_fpError);



LPTDRMETAENTRY tdr_get_entry_by_id_i(TDRMETAENTRY pstEntry[], int iMax, int iID);




static int tdr_get_entry_io_attribute_i(LPTDRMETAENTRY a_pstEntry, LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError);
static int tdr_get_entry_refer_attribute_i(LPTDRMETA a_pstMeta, int a_idxEntry, scew_element *a_pstElement, FILE *a_fpError);
static int tdr_get_entry_defaultvalue_attribute_i(LPTDRMETAENTRY a_pstEntry, LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError);
/**��ȡentry��size����
*@pre \e �����Ȼ�ȡtype����
*/
static int tdr_get_entry_size_attribute_i(LPTDRMETAENTRY pstEntry, LPTDRMETA pstMeta, scew_element *pstElement, FILE *fpError);
static int tdr_get_entry_notnull_attribute_i(LPTDRMETAENTRY a_pstEntry, scew_element *a_pstElement, FILE *a_fpError);
static int tdr_get_entry_unique_attribute_i(LPTDRMETAENTRY a_pstEntry, scew_element *a_pstElement, FILE *a_fpError);
static int tdr_get_entry_sizeinfo_attribute_i(LPTDRMETA a_pstMeta, int a_idxEntry, scew_element *a_pstElement, FILE *a_fpError);
static int tdr_get_entry_sortmethod_attribute_i(LPTDRMETAENTRY a_pstEntry, LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError);
static int tdr_get_entry_extendtotable_attribute_i(LPTDRMETAENTRY a_pstEntry, scew_element *a_pstElement,FILE *a_fpError);

static int tdr_get_entry_compress_attribute_i(LPTDRMETAENTRY a_pstEntry, scew_element *a_pstElement, FILE *a_fpError);

/**��ȡselect����
*/
static int tdr_get_entry_select_attribute_i(LPTDRMETA a_pstMeta, int a_idxEntry, scew_element *a_pstElement, FILE *a_fpError);

/**��ȡmaxid minid����
*@pre \e �����Ȼ�ȡԪ�ص�id����
*/
static int tdr_get_entry_maxmin_id_attribute_i(LPTDRMETAENTRY pstEntry, LPTDRMETA pstMeta, scew_element *pstElement, FILE *fpError);

/**��ȡtagsetversion = 0 entryԪ�����е�����:target
*/
static int tdr_get_entry_target_attribute_i(LPTDRMETA a_pstMeta, int a_idxEntry, scew_element *a_pstElement, FILE *a_fpError);

/**��ȡtagsetversion = 1 entryԪ�����еĲ������ƫ�Ƶ�����:unique nonull size defaultvalue  sortmethod
*/
static int tdr_get_entry_version1_nooff_attribute_i(LPTDRMETAENTRY a_pstEntry, LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError);

/**����meta��idxEntry��Ԫ�ص�ƫ��
*@note �˺���ֻ���ڴ������̵���,idxEntryȡֵ��Χ: [0, a_pstMeta->iEntriesNum]
*/
static void tdr_calc_meta_entry_off_i(LPTDRMETA a_pstMeta, int idxEntry);

static int tdr_parse_entry_sizeinfo_off_i(LPTDRMETA a_pstMeta, int a_iEntry, const char* a_pszName, FILE *a_fpError);

/** ��ȡbindmacrosgroup������Ϣ
*/
static int tdr_get_entry_bindmacrosgroup_attribute_i(IN LPTDRMETALIB a_pstLib, LPTDRMETAENTRY a_pstEntry, scew_element *a_pstElement, FILE *a_fpError);

static int tdr_get_entry_autoincrement_attribute_i(LPTDRMETA a_pstMeta, LPTDRMETAENTRY a_pstEntry, scew_element *a_pstElement,FILE *a_fpError);

/**��ȡ��Ա��customattr����
*/
static int tdr_get_customattr_attribute_i(IN LPTDRMETA a_pstMeta, IN LPTDRMETAENTRY a_pstEntry, IN scew_element *a_pstElement,
								   IN FILE *a_fpError);

static int tdr_get_struct_index_column_attribute_i(LPTDRMETA a_pstMeta, LPTDRMETAINDEX a_pstEntry,
                                                   int *a_piColumnNum, scew_element *a_pstElement, FILE *a_fpError);

static int tdr_parse_index_column_i(IN LPTDRMETA a_pstMeta, OUT LPTDRSelector a_pstSelector,
                                    IN const char* a_pszName, IN FILE *a_fpError);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int tdr_add_entry_to_meta_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError)
{
    LPTDRMETAENTRY pstEntry = NULL;
    int iRet = 0;
    LPTDRMETALIB pstLib = NULL;
	int idxEntry;


    assert(NULL != a_pstMeta);
    assert(NULL != a_pstElement);
    assert(NULL != a_fpError);

    pstLib = TDR_META_TO_LIB(a_pstMeta);
	idxEntry = a_pstMeta->iEntriesNum;
    pstEntry = a_pstMeta->stEntries + idxEntry;
    tdr_init_entry_data_i(pstEntry, a_pstMeta);

    iRet = tdr_get_entry_no_off_attribute_i(pstEntry, a_pstMeta, a_pstElement, a_fpError);

    /*����ƫ��ֵ*/
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		tdr_calc_meta_entry_off_i(a_pstMeta, a_pstMeta->iEntriesNum);
		a_pstMeta->iEntriesNum++;
	}


    /*��ȡrefer����*/
    if ((!TDR_ERR_IS_ERROR(iRet)) && (TDR_TYPE_UNION != a_pstMeta->iType))
    {
        iRet = tdr_get_entry_refer_attribute_i(a_pstMeta, idxEntry, a_pstElement, a_fpError);
    }

    /*select */
    if ((!TDR_ERR_IS_ERROR(iRet)) && (TDR_TYPE_UNION == pstEntry->iType))
    {
        iRet = tdr_get_entry_select_attribute_i(a_pstMeta, idxEntry, a_pstElement, a_fpError);
    }

    if (!TDR_ERR_IS_ERROR(iRet) && (TDR_XML_TAGSET_VERSION_0 == pstLib->iXMLTagSetVer))
    {
        iRet = tdr_get_entry_target_attribute_i(a_pstMeta, idxEntry, a_pstElement, a_fpError);
    }else if (!TDR_ERR_IS_ERROR(iRet) && (TDR_XML_TAGSET_VERSION_1 == pstLib->iXMLTagSetVer))
    {
        /*sizeinfo*/
        iRet = tdr_get_entry_sizeinfo_attribute_i(a_pstMeta, idxEntry, a_pstElement, a_fpError);


    }

    /*���entry���ݵ���Ч��*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_check_entry_i(pstEntry, a_pstMeta, a_fpError);
    }


    if (!TDR_ERR_IS_ERROR(iRet))
    {
        /*����meta�ĵ�ǰ�汾*/
        if (a_pstMeta->iCurVersion < pstEntry->iVersion)
        {
            a_pstMeta->iCurVersion = pstEntry->iVersion;
        }

        if (a_pstMeta->iMaxSubID < pstEntry->iID)
        {
            a_pstMeta->iMaxSubID = pstEntry->iID;
        }
    }

    return iRet;
}

int tdr_add_index_to_meta_i(LPTDRMETA a_pstMeta, int *a_piColumnNum, scew_element *a_pstElement, FILE *a_fpError)
{
    LPTDRMETAINDEX pstIndex = NULL;
    int iRet = 0;
    LPTDRMETALIB pstLib = NULL;
    int idxIndex;


    assert(NULL != a_pstMeta);
    assert(NULL != a_piColumnNum);
    assert(NULL != a_pstElement);
    assert(NULL != a_fpError);

    pstLib = TDR_META_TO_LIB(a_pstMeta);
    idxIndex = a_pstMeta->iIndexesNum;

    pstIndex = TDR_GET_META_INDEX_PTR(a_pstMeta) + idxIndex;

    tdr_init_index_data_i(pstIndex, a_pstMeta);

    iRet = tdr_get_index_no_off_attribute_i(pstIndex, a_pstMeta, a_pstElement, a_fpError);

    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_get_struct_index_column_attribute_i(a_pstMeta, pstIndex, a_piColumnNum,
                                                       a_pstElement, a_fpError);
    }

    if (!TDR_ERR_IS_ERROR(iRet))
    {
        pstIndex->iOffToMeta = (TDRPTR)((char*)pstIndex - (char*)a_pstMeta);
        a_pstMeta->iIndexesNum++;
    }

    return iRet;
}

void tdr_calc_meta_entry_off_i(LPTDRMETA a_pstMeta, int idxEntry)
{
    LPTDRMETAENTRY pstEntry;
    int iEntryAlign = TDR_DEFAULT_ALIGN_VALUE;
    int iRealAlign = TDR_DEFAULT_ALIGN_VALUE;
    int iPadding;
    int iCount;

    assert(NULL != a_pstMeta);
    assert((0 <= idxEntry) && (idxEntry <= a_pstMeta->iEntriesNum));

    pstEntry = a_pstMeta->stEntries + idxEntry;

    if (0 >= pstEntry->iCount)
    {
        iCount = 1;
    }else
    {
       iCount = pstEntry->iCount;
    }
    if (0 < pstEntry->iCustomHUnitSize && TDR_TYPE_INT != pstEntry->iType)
    {
        pstEntry->iHRealSize = pstEntry->iCustomHUnitSize * iCount;
    }else
    {
        pstEntry->iHRealSize = pstEntry->iHUnitSize * iCount;
    }
    pstEntry->iNRealSize = pstEntry->iNUnitSize * iCount;


	if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry) || TDR_ENTRY_IS_REFER_TYPE(pstEntry))
	{
		iEntryAlign = TDR_POINTER_TYPE_ALIGN;
	}else  if (TDR_INVALID_PTR != pstEntry->ptrMeta)
    {
        LPTDRMETALIB pstLib = TDR_META_TO_LIB(a_pstMeta);
        LPTDRMETA pstType = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);

        iEntryAlign = pstType->iValidAlign;
    }else
    {
        LPTDRCTYPEINFO pstType = tdr_idx_to_typeinfo(pstEntry->idxType);
        iEntryAlign = pstType->iSize;
    }

    /*�ṹ��Ķ��뷽ʽ ���� ���г�Ա�� �ֽڶ����������Ǹ�*/
    if (iEntryAlign > a_pstMeta->iValidAlign)
    {
        a_pstMeta->iValidAlign = iEntryAlign;
    }



    if( TDR_TYPE_UNION == a_pstMeta->iType )
    {
        pstEntry->iNOff = 0;
        pstEntry->iHOff = 0;
        if( pstEntry->iHRealSize > a_pstMeta->iHUnitSize )
        {
            a_pstMeta->iHUnitSize = pstEntry->iHRealSize;
        }
        if (pstEntry->iNRealSize > a_pstMeta->iNUnitSize)
        {
            a_pstMeta->iNUnitSize = pstEntry->iNRealSize;
        }

        return ;
    }

    /*struct ����ƫ�Ƽ���*/
    iPadding = 0;
    if( 0 < a_pstMeta->iEntriesNum )
    {
        int iDiff;

        pstEntry->iNOff = (pstEntry - 1)->iNOff + (pstEntry - 1)->iNRealSize;  /*����ƫ��*/

        iRealAlign = TDR_MIN(iEntryAlign, a_pstMeta->iCustomAlign);
        assert(0 != iRealAlign);
        pstEntry->iHOff = (pstEntry - 1)->iHOff + (pstEntry - 1)->iHRealSize;  /*���ش洢ƫ��*/
        iDiff = (int)(pstEntry->iHOff % iRealAlign);
        if (0 != iDiff)
        {
            iPadding = iRealAlign - iDiff;
            pstEntry->iHOff += iPadding;
        }
    } else
    {
        pstEntry->iHOff = 0;
        pstEntry->iNOff = 0;
    } /*if( 0 < a_pstMeta->iEntriesNum )*/

    a_pstMeta->iHUnitSize += iPadding;
    a_pstMeta->iHUnitSize += pstEntry->iHRealSize;
    a_pstMeta->iNUnitSize += pstEntry->iNRealSize;

}

void tdr_init_index_data_i(LPTDRMETAINDEX a_pstIndex, LPTDRMETA a_pstMeta)
{
    assert(NULL != a_pstIndex);
    assert(NULL != a_pstMeta);

    memset(a_pstIndex, 0, sizeof(*a_pstIndex));

    a_pstIndex->iColumnNum = 0;
    a_pstIndex->iOffToMeta = TDR_CALC_ENTRYOFF_OF_META(a_pstMeta, a_pstIndex);
    a_pstIndex->iVersion = a_pstMeta->iBaseVersion;
    a_pstIndex->ptrColumnBase = TDR_INVALID_PTR;
    a_pstIndex->ptrChineseName = TDR_INVALID_PTR;
    a_pstIndex->ptrDesc = TDR_INVALID_PTR;
    a_pstIndex->ptrName = TDR_INVALID_PTR;
}

void tdr_init_entry_data_i(LPTDRMETAENTRY a_pstEntry, LPTDRMETA a_pstMeta)
{
    assert(NULL != a_pstEntry);
    assert(NULL != a_pstMeta);

    memset(a_pstEntry, 0, sizeof(TDRMETAENTRY));

	a_pstEntry->iCount = 1;
    a_pstEntry->iVersion = a_pstMeta->iBaseVersion;
    a_pstEntry->iType = TDR_TYPE_UNKOWN;

    a_pstEntry->idxID = TDR_INVALID_INDEX;
    a_pstEntry->idxCount = TDR_INVALID_INDEX;
    a_pstEntry->idxIO = TDR_INVALID_INDEX;
    a_pstEntry->idxType = TDR_INVALID_INDEX;
    a_pstEntry->idxVersion = TDR_INVALID_INDEX;
    a_pstEntry->idxCustomHUnitSize = TDR_INVALID_INDEX;


    a_pstEntry->ptrDesc = TDR_INVALID_PTR;
    a_pstEntry->ptrChineseName = TDR_INVALID_PTR;
    a_pstEntry->ptrDefaultVal = TDR_INVALID_PTR;
    a_pstEntry->ptrMeta = TDR_INVALID_PTR;


	a_pstEntry->chDBFlag = TDR_ENTRY_DB_FLAG_NONE;

	a_pstEntry->iID = TDR_INVALID_INDEX;
	a_pstEntry->iMaxIdIdx = TDR_INVALID_INDEX;
	a_pstEntry->iMinIdIdx = TDR_INVALID_INDEX;
	a_pstEntry->iMinId = a_pstEntry->iMaxId + 1; /*��Ч��maxid minid����*/

    a_pstEntry->stRefer.iHOff = TDR_INVALID_OFFSET;
    a_pstEntry->stRefer.ptrEntry = TDR_INVALID_PTR;

    a_pstEntry->stSizeInfo.iHOff = TDR_INVALID_OFFSET;
    a_pstEntry->stSizeInfo.iNOff = TDR_INVALID_OFFSET;
	a_pstEntry->stSizeInfo.idxSizeType = TDR_INVALID_INDEX;

    a_pstEntry->stSelector.iHOff = TDR_INVALID_OFFSET;
    a_pstEntry->stSelector.ptrEntry = TDR_INVALID_PTR;
	a_pstEntry->chOrder = TDR_SORTMETHOD_NONE_SORT;

	a_pstEntry->ptrMacrosGroup = TDR_INVALID_PTR;

	TDR_ENTRY_CLEAR_ALL_FLAG(a_pstEntry);

	TDR_ENTRY_CLR_HAVE_ID(a_pstEntry);

	/*TO DO Ŀǰ������ֵ�����Ե����ݽṹ��Ϊ����ʵ����*/
	a_pstEntry->ptrCustomAttr = TDR_INVALID_PTR;

    a_pstEntry->ptrName = TDR_INVALID_PTR;

    a_pstEntry->iOffToMeta = TDR_CALC_ENTRYOFF_OF_META(a_pstMeta, a_pstEntry);

    a_pstEntry->iReserve1 = TDR_INVALID_PTR;
    a_pstEntry->iReserve2 = TDR_INVALID_PTR;
    a_pstEntry->iReserve3 = TDR_INVALID_PTR;
}

int tdr_get_index_no_off_attribute_i(LPTDRMETAINDEX a_pstIndex, LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError)
{
    int iRet = 0;
    LPTDRMETALIB pstLib;
    scew_attribute *pstAttr = NULL;

    assert(NULL != a_pstIndex);
    assert(NULL != a_pstMeta);
    assert(NULL != a_pstElement);
    assert(NULL != a_fpError);


    pstLib = TDR_META_TO_LIB(a_pstMeta);


    /*��ȡname ����*/
    pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_NAME);
    if (NULL != pstAttr)
    {
        const char *pszName = scew_attribute_value(pstAttr);
        if (TDR_NAME_LEN <= strlen(pszName))
        {
            fprintf(a_fpError, "error:��������(name:%s)�ĳ�Ա(name:%s)���ִ�̫����Ŀǰ��Ա�����֧��%d���ַ�\n",
                    TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), pszName, TDR_NAME_LEN-1);
            iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NAME_LEN_BEYOND_UPLIMIT);

        }else
        {
            char szName[TDR_NAME_LEN];
            size_t iNameLen;
            iNameLen = tdr_normalize_string(szName, sizeof(szName), pszName);
            if (!TDR_STRBUF_HAVE_FREE_ROOM(pstLib, iNameLen))
            {
                fprintf(a_fpError, "error: �ַ�����������ʣ��ռ�(%"TDRPRI_SIZET")���������ܴ洢��������(name:%s)��Ա��name����(%s)\n",
                        pstLib->iFreeStrBufSize,TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), pszName);
                iRet = TDR_ERRIMPLE_NO_SPACE_FOR_STRBUF;
            }else
            {
                TDR_COPY_STRING_TO_BUF(a_pstIndex->ptrName, szName, iNameLen, pstLib);
            }
        }/*if (TDR_NAME_LEN <= strlen(pszName))*/
    }else
    {
        a_pstIndex->ptrName = TDR_INVALID_PTR;
        fprintf(a_fpError, "error:\t %sԪ�صĵ�%d����Ԫ��û��name���Ի���ֵ����Ϊ�մ�.\n",
                TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), a_pstMeta->iEntriesNum + 1);
        iRet = TDR_ERRIMPLE_META_NO_NAME;
    }/* if (NULL != pstAttr)*/


    /*check name*/
    if (!TDR_ERR_IS_ERROR(iRet) && (TDR_XML_TAGSET_VERSION_0 < pstLib->iXMLTagSetVer))
    {
        /*�ж�meta���Ƿ���ͬ����entry����*/
        int iIdx = tdr_get_index_by_name_i(TDR_GET_META_INDEX_PTR(a_pstMeta), a_pstMeta->iIndexesNum,
                                           TDR_GET_STRING_BY_PTR(pstLib,a_pstIndex->ptrName));
        if (TDR_INVALID_INDEX != iIdx)
        {
            assert(TDR_TYPE_STRUCT == a_pstMeta->iType);
            fprintf(a_fpError, "error:\t %sԪ�ش�����������<name = %s>��ͬ��index��Ա.\n",
                    TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName),
                    TDR_GET_STRING_BY_PTR(pstLib,a_pstIndex->ptrName));
            iRet =TDR_ERRIMPLE_META_NAME_CONFLICT;
        }/*if (TDR_INVALID_INDEX != iIdx)*/

    }/*if ((!TDR_ERR_IS_ERROR(iRet)) && (TDR_XML_TAGSET_VERSION_0 < pstLib->iXMLTagSetVer))*/


    /*version����*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        int idxVersion;
        iRet = tdr_get_version_attribute_i(&a_pstIndex->iVersion, &idxVersion, pstLib, a_pstElement, a_fpError);
    }

    return iRet;
}

int tdr_get_entry_no_off_attribute_i(LPTDRMETAENTRY a_pstEntry, LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError)
{
    int iRet = 0;
    LPTDRMETALIB pstLib;
    scew_attribute *pstAttr = NULL;

    assert(NULL != a_pstEntry);
    assert(NULL != a_pstMeta);
    assert(NULL != a_pstElement);
    assert(NULL != a_fpError);


    pstLib = TDR_META_TO_LIB(a_pstMeta);


    /*��ȡname ����*/
    pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_NAME);
    if (NULL != pstAttr)
    {
        const char *pszName = scew_attribute_value(pstAttr);
        if (TDR_NAME_LEN <= strlen(pszName))
        {
            fprintf(a_fpError, "error:��������(name:%s)�ĳ�Ա(name:%s)���ִ�̫����Ŀǰ��Ա�����֧��%d���ַ�\n",
                TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), pszName, TDR_NAME_LEN-1);
            iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NAME_LEN_BEYOND_UPLIMIT);

        }else
        {
            char szName[TDR_NAME_LEN];
            size_t iNameLen;
            iNameLen = tdr_normalize_string(szName, sizeof(szName), pszName);
            if (!TDR_STRBUF_HAVE_FREE_ROOM(pstLib, iNameLen))
            {
                fprintf(a_fpError, "error: �ַ�����������ʣ��ռ�(%"TDRPRI_SIZET")���������ܴ洢��������(name:%s)��Ա��name����(%s)\n",
                    pstLib->iFreeStrBufSize,TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), pszName);
                iRet = TDR_ERRIMPLE_NO_SPACE_FOR_STRBUF;
            }else
            {
                TDR_COPY_STRING_TO_BUF(a_pstEntry->ptrName, szName, iNameLen, pstLib);
            }
        }/*if (TDR_NAME_LEN <= strlen(pszName))*/
    }else
    {
        a_pstEntry->ptrName = TDR_INVALID_PTR;
        fprintf(a_fpError, "error:\t %sԪ�صĵ�%d����Ԫ��û��name���Ի���ֵ����Ϊ�մ�.\n", TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), a_pstMeta->iEntriesNum + 1);
        iRet = TDR_ERRIMPLE_META_NO_NAME;
    }/* if (NULL != pstAttr)*/


     /*��ȡtype����*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_get_entry_type_i(a_pstEntry, a_pstMeta, a_pstElement, a_fpError);
    }

	/*check name*/
	if (!TDR_ERR_IS_ERROR(iRet) && (TDR_XML_TAGSET_VERSION_0 < pstLib->iXMLTagSetVer))
	{
        /*�ж�meta���Ƿ���ͬ����entry����*/
        int iIdx = tdr_get_entry_by_name_i(&a_pstMeta->stEntries[0], a_pstMeta->iEntriesNum, TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName));
        if (TDR_INVALID_INDEX != iIdx)
        {
            if (TDR_TYPE_UNION == a_pstMeta->iType)
            {
                LPTDRMETAENTRY pstEntry = a_pstMeta->stEntries + iIdx;
                if ((pstEntry->iType != a_pstEntry->iType) || (pstEntry->ptrMeta != a_pstEntry->ptrMeta))
                {
                    fprintf(a_fpError, "error:\t %sԪ�ش�����������<name = %s>��ͬ�ĳ�Ա,union�ṹ��ֻ�������������ͬ��ͬ����Ա.\n", TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName));
                    iRet =TDR_ERRIMPLE_META_NAME_CONFLICT;
                }
            }else /*if (TDR_TYPE_UNION == a_pstMeta->iType)*/
            {
                fprintf(a_fpError, "error:\t %sԪ�ش�����������<name = %s>��ͬ�ĳ�Ա.\n", TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName),
                    TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName));
                iRet =TDR_ERRIMPLE_META_NAME_CONFLICT;
            }
        }/*if (TDR_INVALID_INDEX != iIdx)*/

	}/*if ((!TDR_ERR_IS_ERROR(iRet)) && (TDR_XML_TAGSET_VERSION_0 < pstLib->iXMLTagSetVer))*/

    /*��ȡID����*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_get_id_attribute_i(&a_pstEntry->iID, &a_pstEntry->idxID, pstLib, a_pstElement, a_fpError);
        if (TDR_SUCCESS == iRet)
        {
            TDR_ENTRY_SET_HAVE_ID(a_pstEntry);

			a_pstEntry->iMaxId = a_pstEntry->iID;
			a_pstEntry->iMinId = a_pstEntry->iMaxId;
            if (TDR_XML_TAGSET_VERSION_0 < pstLib->iXMLTagSetVer)
            {
				/*���ID���Ե���Ч�ԣ�ͬ��Ԫ������Ԫ�ص�id���벻ͬ*/
				LPTDRMETAENTRY pstFindEntry = tdr_get_entry_by_id_i(&a_pstMeta->stEntries[0], a_pstMeta->iEntriesNum, a_pstEntry->iID);
				if (NULL != pstFindEntry)
				{
					fprintf(a_fpError, "error:\t %sԪ�س�Ա<name=%s>��ID����ֵ<%d>��������һ����Ա<name=%s>��ID����ֵ��ͬ.\n", TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName),
						TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName), a_pstEntry->iID,
                        TDR_GET_STRING_BY_PTR(pstLib,pstFindEntry->ptrName));
					iRet =TDR_ERRIMPLE_META_ID_CONFLICT;
				}
				if (0 > a_pstEntry->iID)
				{
					fprintf(a_fpError, "error:\t %sԪ�س�Ա<name=%s>��ID����ֵ<%d>����ȷ��������ֵֻ��Ϊ�Ǹ���.\n", TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName),
						TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName), a_pstEntry->iID);
					iRet =TDR_ERRIMPLE_META_ID_CONFLICT;
				}
            }/*if (TDR_XML_TAGSET_VERSION_0 =< pstLib->iXMLTagSetVer)*/

        }/*if (TDR_SUCCESS == iRet)*/
    }

    /*version����*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_get_version_attribute_i(&a_pstEntry->iVersion, &a_pstEntry->idxVersion, pstLib, a_pstElement, a_fpError);
    }

    /*desc*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_get_desc_attribute_i(&a_pstEntry->ptrDesc, pstLib, a_pstElement, a_fpError);
    }

    /*��ȡcount����*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_get_entry_count_attribute_i(a_pstEntry, a_pstMeta, a_pstElement, a_fpError);
    }

    /*��ȡIO����*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_get_entry_io_attribute_i(a_pstEntry, a_pstMeta, a_pstElement, a_fpError);
    }

    if (!TDR_ERR_IS_ERROR(iRet) && (TDR_XML_TAGSET_VERSION_0 < pstLib->iXMLTagSetVer))
    {
        iRet = tdr_get_entry_version1_nooff_attribute_i(a_pstEntry, a_pstMeta, a_pstElement, a_fpError);
    }

    return iRet;
}



int tdr_get_entry_count_attribute_i(LPTDRMETAENTRY a_pstEntry, LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError)
{
    scew_attribute *pstAttr = NULL;
    LPTDRMETALIB pstLib =NULL;
    int iRet = TDR_SUCCESS;

    assert(NULL != a_pstElement);
    assert(NULL != a_pstEntry);
    assert(NULL != a_pstMeta);
    assert(NULL != a_fpError);


    pstLib = TDR_META_TO_LIB(a_pstMeta);

	pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_COUNT);
    if( NULL != pstAttr )
    {
		char szCount[256] = {0};

        tdr_normalize_string(&szCount[0], sizeof(szCount),scew_attribute_value(pstAttr));
        iRet = tdr_get_macro_int_i(&a_pstEntry->iCount, &a_pstEntry->idxCount, pstLib, &szCount[0]);
        if (TDR_ERR_IS_ERROR(iRet))
        {
            fprintf(a_fpError, "\nerror:\t entryԪ��<name = %s>��count����ֵ<%s>��Ч,��ȷ����ֵ���������\n",
                TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName), szCount);

            iRet = TDR_ERRIMPLE_ENTRY_INVALID_COUNT_VALUE;
        }else if (0 > a_pstEntry->iCount)
        {
            fprintf(a_fpError, "error:\t entryԪ��<name = %s>��count����ֵ<count = %d>����С��0\n",
                TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName), a_pstEntry->iCount);

            iRet = TDR_ERRIMPLE_ENTRY_INVALID_COUNT_VALUE;
        }

		/*�����ϵĶ��壬����string������count���Զ�Ӧ�¶����size���ԣ���˽���ת��*/
		if ((TDR_TYPE_STRING == a_pstEntry->iType) && (TDR_XML_TAGSET_VERSION_0 == pstLib->iXMLTagSetVer))
		{
			if (0 == a_pstEntry->iCount)
			{
				fprintf(a_fpError, "error:\t entryԪ��<name = %s>Ϊstring������count����ֵ<count = %d>����Ϊ0\n",
					TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName), a_pstEntry->iCount);
				iRet = TDR_ERRIMPLE_ENTRY_INVALID_COUNT_VALUE;
			}else
			{
				a_pstEntry->idxCustomHUnitSize = a_pstEntry->idxCount;
				a_pstEntry->iCustomHUnitSize = a_pstEntry->iCount;
				a_pstEntry->idxCount = TDR_INVALID_INDEX;
				a_pstEntry->iCount = 1;
			}
		}
    }/*if( NULL != pstAttr )*/

    return iRet;
}

int tdr_get_entry_io_attribute_i(LPTDRMETAENTRY a_pstEntry, LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError)
{
    scew_attribute *pstAttr = NULL;
    LPTDRMETALIB pstLib =NULL;
    int iRet = TDR_SUCCESS;
    char szIO[128] = {0};

    assert(NULL != a_pstElement);
    assert(NULL != a_pstEntry);
    assert(NULL != a_pstMeta);
    assert(NULL != a_fpError);


    pstLib = TDR_META_TO_LIB(a_pstMeta);

    pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_IO);
    if (NULL == pstAttr)
    {
    	return TDR_SUCCESS;
    }

    tdr_normalize_string(&szIO[0], sizeof(szIO), scew_attribute_value(pstAttr));
    if (0 == tdr_stricmp(szIO, TDR_TAG_IO_NO_LIMIT))
    {
    	a_pstEntry->iIO  = TDR_IO_NOLIMIT;
    }else if (0 == tdr_stricmp(szIO, TDR_TAG_IO_NO_INPUT))
    {
    	a_pstEntry->iIO = TDR_IO_NOINPUT;
    }else if (0 == tdr_stricmp(szIO, TDR_TAG_IO_NO_OUTPUT))
    {
    	a_pstEntry->iIO = TDR_IO_NOOUTPUT;
    }else if (0 == tdr_stricmp(szIO, TDR_TAG_IO_NO_IO))
    {
    	 a_pstEntry->iIO = TDR_IO_NOIO;
    }else
    {
        iRet = tdr_get_macro_int_i(&a_pstEntry->iIO, &a_pstEntry->idxIO, pstLib, szIO);
        if (TDR_ERR_IS_ERROR(iRet))
        {
            fprintf(a_fpError, "\nerror:\t entryԪ��<name = %s>��io����ֵ<%s>��Ч,��ȷ����ֵ���������",
                TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName), szIO);
			iRet = TDR_ERRIMPLE_ENTRY_INVALID_IO_VALUE;
        }else if ((TDR_IO_MIN_VALUE > a_pstEntry->iIO) ||
            (TDR_IO_MAX_VALUE < a_pstEntry->iIO))
        {
            fprintf(a_fpError, "\nerror:\t entryԪ��<name = %s>��io����ֵ<%s>��Ч,��ȷ����ֵ���������",
                TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName), szIO);

            iRet = TDR_ERRIMPLE_ENTRY_INVALID_IO_VALUE;
        }
   }

    return iRet;
}

int tdr_get_entry_refer_attribute_i(LPTDRMETA a_pstMeta, int a_idxEntry, scew_element *a_pstElement, FILE *a_fpError)
{
    scew_attribute *pstAttr = NULL;
    LPTDRMETALIB pstLib =NULL;
    const char *pszRefer = NULL;
    int iRet = TDR_SUCCESS;
	LPTDRMETAENTRY pstEntry;

    assert(NULL != a_pstElement);
    assert(NULL != a_pstMeta);
    assert(NULL != a_fpError);
	assert((0 <= a_idxEntry) && (a_idxEntry < a_pstMeta->iEntriesNum));


    pstLib = TDR_META_TO_LIB(a_pstMeta);
	pstEntry = a_pstMeta->stEntries + a_idxEntry;

    pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_REFER);
    if( NULL != pstAttr )
    {
		LPTDRMETAENTRY pstFindEntry = NULL;

		if (TDR_TYPE_UNION == a_pstMeta->iType)
		{
			fprintf(a_fpError, "\nerror:\t union����<name = %s>���������ԱԪ��<name = %s>ָ��refer����ֵ<%s>\n",
				TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName), pszRefer);
			return TDR_ERRIMPLE_ENTRY_INVALID_REFER_VALUE;

		}


        pszRefer = scew_attribute_value(pstAttr);
        iRet = tdr_name_to_off_i(a_pstMeta, &pstEntry->stRefer, a_idxEntry, pszRefer);
        if (TDR_SUCCESS !=iRet)
        {
            fprintf(a_fpError, "\nerror:\t �Զ�������<name = %s>�ĳ�ԱԪ��<name = %s>��refer����ֵ<%s>��Ч����ȷ��ָ����ȷ��Ԫ������·������·�����漰�ĸ�Ԫ�ز���ָ�����������\n",
                TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName), pszRefer);
            iRet = TDR_ERRIMPLE_ENTRY_INVALID_REFER_VALUE;
        }else
        {
			pstFindEntry = TDR_PTR_TO_ENTRY(pstLib, pstEntry->stRefer.ptrEntry);
            if((pstFindEntry->iType <= TDR_TYPE_COMPOSITE)
                || (pstFindEntry->iType>TDR_TYPE_UINT) )
            {
                fprintf(a_fpError, "\nerror:\t �Զ�������<name = %s>�ĳ�ԱԪ��<name = %s>��refer����ֵ<%s>��Ч\n",
                    TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName), pszRefer);
                iRet = TDR_ERRIMPLE_ENTRY_INVALID_REFER_VALUE;
            }else if (pstEntry->stRefer.iHOff >= pstEntry->iHOff)
            {
				fprintf(a_fpError, "\nerror:\t �Զ�������<name = %s>�ĳ�ԱԪ��<name = %s>��refer����ֵ<%s>��Ч,�����Ǹ�Ԫ�ص�ֱ�Ӹ�Ԫ�ذ������ض���Ԫ��\n",
					TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName), pszRefer);
				iRet = TDR_ERRIMPLE_ENTRY_INVALID_REFER_VALUE;
			}else if (1 != pstFindEntry->iCount)
			{
				fprintf(a_fpError, "\nerror:\t �Զ�������<name = %s>�ĳ�ԱԪ��<name = %s>��refer����ֵ<%s>����Ϊ����\n",
					TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName), pszRefer);
				iRet = TDR_ERRIMPLE_ENTRY_INVALID_REFER_VALUE;
			}

			/*���ü�������־ ����������entry*/
			TDR_ENTRY_SET_COUNTER(pstFindEntry);
			pstFindEntry->stRefer.ptrEntry = TDR_ENTRY_TO_PTR(pstLib, pstEntry);
        }
    }/*if( NULL != pstAttr )*/

    return iRet;
}

int tdr_get_entry_defaultvalue_attribute_i(LPTDRMETAENTRY a_pstEntry, LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError)
{
    scew_attribute *pstAttr = NULL;
    size_t iDefaultLen = 0;
    int iRet = TDR_SUCCESS;
    LPTDRMETALIB pstLib = NULL;
	char szDefault[TDR_DEFAULT_VALUE_LEN] = {0};
	LPTDRCTYPEINFO pstTypeInfo;
	size_t iSize;
	char *pszFreeBuf;

    assert(NULL != a_pstEntry);
    assert(NULL != a_pstMeta);
    assert(NULL != a_pstElement);
    assert(NULL != a_fpError);

	pstLib = TDR_META_TO_LIB(a_pstMeta);
    pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_DEFAULT_VALUE);
	if (NULL == pstAttr)
	{
		return TDR_SUCCESS;
	}

	if (TDR_TYPE_COMPOSITE >= a_pstEntry->iType)
	{
		fprintf(a_fpError, "\nwarning:\t �Զ�������<name = %s>�ĳ�Ա<name = %s>Ϊ�Զ��帴���������ͣ�Ŀǰ�ݲ�֧������ȱʡֵ",
			TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName));
		return TDR_SUCCESS;
	}

	if (TDR_ENTRY_IS_POINTER_TYPE(a_pstEntry) || TDR_ENTRY_IS_REFER_TYPE(a_pstEntry))
	{
		fprintf(a_fpError, "\nwarning:\t �Զ�������<name = %s>�ĳ�Ա<name = %s>Ϊָ��������������ͣ�Ŀǰ�ݲ�֧������ȱʡֵ",
			TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName));
		return TDR_SUCCESS;
	}


	tdr_normalize_string(&szDefault[0], sizeof(szDefault), scew_attribute_value(pstAttr));
	pstTypeInfo = tdr_idx_to_typeinfo(a_pstEntry->idxType);
	if (TDR_TYPE_STRING > a_pstEntry->iType)
	{
		iDefaultLen = pstTypeInfo->iSize; /*��1��Ϊ�˱��������ַ�*/
	}else if (TDR_TYPE_STRING == a_pstEntry->iType)
	{
		iDefaultLen = strlen(&szDefault[0]) + 1; /*��1��Ϊ�˱��������ַ�*/
	}else if (TDR_TYPE_WSTRING == a_pstEntry->iType)
	{
		iDefaultLen = strlen(&szDefault[0]) + 1;
		iDefaultLen *= sizeof(tdr_wchar_t);
	}


	if (iDefaultLen > pstLib->iFreeStrBufSize)
	{
		fprintf(a_fpError, "\nwarning:\t Ԫ�����ַ���������ʣ��ռ�<%"TDRPRI_SIZET">�����Դ洢�Զ�������<name = %s>�ĳ�Ա<name = %s>��ȱʡֵ<%s>",
			pstLib->iFreeStrBufSize, TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName), szDefault);
		return iRet = TDR_ERRIMPLE_NO_SPACE_FOR_STRBUF;
	}



	pszFreeBuf = TDR_GET_FREE_BUF(pstLib);
	iSize = iDefaultLen;
	switch(a_pstEntry->iType)
	{
	case TDR_TYPE_CHAR:
	case TDR_TYPE_UCHAR:
	case TDR_TYPE_SMALLINT:
	case TDR_TYPE_SMALLUINT:
	case TDR_TYPE_INT:
	case TDR_TYPE_LONG:
	case TDR_TYPE_UINT:
	case TDR_TYPE_ULONG:
	case TDR_TYPE_LONGLONG:
	case TDR_TYPE_ULONGLONG:
		iRet = tdr_ioscanf_basedtype_i(pstLib, a_pstEntry, pszFreeBuf, &iSize, &szDefault[0]);
		if (TDR_ERR_IS_ERROR(iRet))
		{
			if (TDR_ERROR_VALUE_BEYOND_TYPE_FIELD == TDR_ERR_GET_ERROR_CODE(iRet))
			{
				fprintf(a_fpError, "\nerror:\t �Զ�������<name = %s>�ĳ�Ա<name = %s>����������Ϊ<type=%s>����ȱʡֵ<%s>�����Ͳ�ƥ��",
					TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName), pstTypeInfo->pszName, szDefault);
			}else
			{
				fprintf(a_fpError, "\nerror:\t �Զ�������<name = %s>�ĳ�Ա<name = %s>����������Ϊ<type=%s>����ȡ��ȱʡֵ<%s>ʧ��",
					TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName), pstTypeInfo->pszName, szDefault);
			}
		}/*if (TDR_ERR_IS_ERROR(iRet))*/
		break;
	case TDR_TYPE_FLOAT:
	case TDR_TYPE_DOUBLE:
	case TDR_TYPE_IP:
	case TDR_TYPE_WCHAR:
	case TDR_TYPE_STRING:
	case TDR_TYPE_WSTRING:
		iRet = tdr_ioscanf_basedtype_i(pstLib, a_pstEntry, pszFreeBuf, &iSize, &szDefault[0]);
		if (TDR_ERR_IS_ERROR(iRet))
		{
			fprintf(a_fpError, "\nerror:\t �Զ�������<name = %s>�ĳ�Ա<name = %s>����������Ϊ<type=%s>����ȡ��ȱʡֵ<%s>ʧ��",
				TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName), pstTypeInfo->pszName, szDefault);
		}/*if (TDR_ERR_IS_ERROR(iRet))*/
		break;
	case TDR_TYPE_DATETIME:
		{
			iRet = tdr_ioscanf_basedtype_i(pstLib, a_pstEntry, pszFreeBuf, &iSize, &szDefault[0]);
			if (TDR_ERR_IS_ERROR(iRet))
			{
				fprintf(a_fpError, "\nwarning:\t �Զ�������<name = %s>�ĳ�Ա<name = %s>����/ʱ��(datetime)���͵�ȱʡֵ��Ч,��ȷdatetime������ֵ��ʽӦΪ\"YYYY-MM-DD HH:MM:SS\".",
					TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName));
			}
			break;
		}
	case TDR_TYPE_DATE:
		{
			iRet = tdr_ioscanf_basedtype_i(pstLib, a_pstEntry, pszFreeBuf, &iSize, &szDefault[0]);
			if (TDR_ERR_IS_ERROR(iRet))
			{
				fprintf(a_fpError, "\nwarning:\t �Զ�������<name = %s>�ĳ�Ա<name = %s>����(date)���͵�ȱʡֵ��Ч,��ȷdate������ֵ��ʽӦΪ\"YYYY-MM-DD\".",
					TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName));
			}
			break;
		}
	case TDR_TYPE_TIME:
		{
			iRet = tdr_ioscanf_basedtype_i(pstLib, a_pstEntry, pszFreeBuf, &iSize, &szDefault[0]);
			if (TDR_ERR_IS_ERROR(iRet))
			{
				fprintf(a_fpError, "\nerror:\t �Զ�������<name = %s>�ĳ�Ա<name = %s>ʱ��(time)���͵�ȱʡֵ��Ч,��ȷdate������ֵ��ʽӦΪ\"HHH:MM:SS\".",
					TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName));
			}
			break;
		}
	default:
		fprintf(a_fpError, "\nwarning:\t �Զ�������<name = %s>�ĳ�Ա<name = %s>������<type=%s>Ŀǰ�ݲ�֧������ȱʡֵ.",
			TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName), pstTypeInfo->pszName);
		iDefaultLen = 0;
		break;
	}

	if (TDR_SUCCESS == iRet)
	{
		a_pstEntry->ptrDefaultVal = pstLib->ptrFreeStrBuf;
		a_pstEntry->iDefaultValLen = iDefaultLen;
		pstLib->ptrFreeStrBuf += iDefaultLen;
		pstLib->iFreeStrBufSize -= iDefaultLen;
	}

    return iRet;
}

int tdr_get_entry_size_attribute_i(LPTDRMETAENTRY a_pstEntry, LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError)
{
    scew_attribute *pstAttr = NULL;
    int iRet = TDR_SUCCESS;
    LPTDRMETALIB pstLib = NULL;
    int iCustomHUnitSize;

    assert(NULL != a_pstEntry);
    assert(NULL != a_pstMeta);
    assert(NULL != a_pstElement);
    assert(NULL != a_fpError);

	pstLib = TDR_META_TO_LIB(a_pstMeta);
    pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_SIZE);
    if( NULL != pstAttr )
    {
		char szSize[256] = {0};

		tdr_normalize_string(&szSize[0], sizeof(szSize), scew_attribute_value(pstAttr));
        iRet = tdr_get_macro_int_i(&iCustomHUnitSize, &a_pstEntry->idxCustomHUnitSize, pstLib, &szSize[0]);
        a_pstEntry->iCustomHUnitSize = iCustomHUnitSize;
        if ((TDR_SUCCESS != iRet) || (0 >= a_pstEntry->iCustomHUnitSize))
        {
            fprintf(a_fpError, "\nerror:\t �ṹ��<name=%s>�ĳ�Ա<name = %s>��size����ֵ<%s>��Ч,��ȷ����ֵ�����������Ѷ���ĺ�",
                TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName), &szSize[0]);

            iRet = TDR_ERRIMPLE_ENTRY_INVALID_SIZE_VALUE;
        }else if (0 < a_pstEntry->iCustomHUnitSize)
        {/*���Ԥָ����С�Ƿ��ʵ����Ҫ�Ĵ�*/
            LPTDRCTYPEINFO pstTypeInfo = tdr_idx_to_typeinfo(a_pstEntry->idxType);

			if ((TDR_TYPE_STRING == a_pstEntry->iType) || (TDR_TYPE_WSTRING == a_pstEntry->iType))
			{
				a_pstEntry->iCustomHUnitSize *= pstTypeInfo->iSize;
			}

            if ((TDR_TYPE_COMPOSITE < a_pstEntry->iType) && (TDR_TYPE_STRING > a_pstEntry->iType))
            {
                fprintf(a_fpError, "error:\t �Զ�������<name =%s>�ĳ�Ա<name = %s>Ϊ�洢��Ԫ�̶�����������<type =%s>��������ָ���洢size<%"TDRPRI_SIZET">\n",
                    TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName), pstTypeInfo->pszName, a_pstEntry->iCustomHUnitSize);

                iRet = TDR_ERRIMPLE_ENTRY_INVALID_SIZE_VALUE;
            }else if (a_pstEntry->iCustomHUnitSize < a_pstEntry->iHUnitSize)
            {
                /*ָ��size��ʵ�ʼ����sizeҪС*/
                fprintf(a_fpError, "error:\t �Զ�������<name =%s>�ĳ�Ա<name = %s>ָ���Ĵ洢�ռ�<size = %"TDRPRI_SIZET">����С��ʵ����Ҫ�洢�ռ�<%"TDRPRI_SIZET">\n",
                    TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName), a_pstEntry->iCustomHUnitSize, a_pstEntry->iHUnitSize);

                iRet = TDR_ERRIMPLE_ENTRY_INVALID_SIZE_VALUE;
            }
        } /*else if (0 < a_pstEntry->iExpectedUnit)*/
    }/* if( NULL != pstAttr )*/

    return iRet;
}


int tdr_get_entry_notnull_attribute_i(LPTDRMETAENTRY a_pstEntry, scew_element *a_pstElement, FILE *a_fpError)
{
    scew_attribute *pstAttr = NULL;
    int iRet = TDR_SUCCESS;


    assert(NULL != a_pstEntry);
    assert(NULL != a_pstElement);
    assert(NULL != a_fpError);

    pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_NOTNULL);
    if( NULL != pstAttr )
    {
		char szVal[256] = {0};

        tdr_normalize_string(&szVal[0], sizeof(szVal), scew_attribute_value(pstAttr));
        if (0 == tdr_stricmp(szVal, TDR_TAG_TRUE))
        {
            TDR_ENTRY_SET_NOT_NULL(a_pstEntry);
        }else if ( 0== tdr_stricmp(szVal, TDR_TAG_FALSE))
        {
            TDR_ENTRY_CLR_NOT_NULL(a_pstEntry);
        }else
        {
            LPTDRMETA  pstMeta = TDR_ENTRY_TO_META(a_pstEntry);
            LPTDRMETALIB pstLib = TDR_META_TO_LIB(pstMeta);
            fprintf(a_fpError, "error:\t entryԪ��<name = %s>��notnull����ֵ<%s>��Ч,��ֵΪ\"false\"����\"true\"\n",
                TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName), szVal);

            iRet = TDR_ERRIMPLE_ENTRY_INVALID_NOTNULL_VALUE;
        }/*if ( 0== strcmp(pszVal, TDR_TAG_FALSE))*/
    }

    return iRet ;
}

int tdr_get_entry_unique_attribute_i(LPTDRMETAENTRY a_pstEntry, scew_element *a_pstElement, FILE *a_fpError)
{
    scew_attribute *pstAttr = NULL;
    int iRet = TDR_SUCCESS;


    assert(NULL != a_pstEntry);
    assert(NULL != a_pstElement);
    assert(NULL != a_fpError);

    pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_UNIQUE);
    if( NULL != pstAttr )
    {
		char szVal[256] = {0};

		tdr_normalize_string(&szVal[0], sizeof(szVal), scew_attribute_value(pstAttr));
        if (0 == tdr_stricmp(szVal, TDR_TAG_TRUE))
        {
            TDR_ENTRY_SET_UNIQUE(a_pstEntry);
        }else if ( 0== tdr_stricmp(szVal, TDR_TAG_FALSE))
        {
            TDR_ENTRY_CLR_UNIQUE(a_pstEntry);
        }else
        {
            LPTDRMETA  pstMeta = TDR_ENTRY_TO_META(a_pstEntry);
            LPTDRMETALIB pstLib = TDR_META_TO_LIB(pstMeta);
            fprintf(a_fpError, "\nerror:\t entryԪ��<name = %s>��unique����ֵ<%s>��Ч,��ֵΪ\"false\"����\"true\"\n",
                TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName), szVal);

            iRet = TDR_ERRIMPLE_ENTRY_INVALID_UNIQUE_VALUE;
        }/*if ( 0== strcmp(pszVal, TDR_TAG_FALSE))*/
    }

    return iRet;
}

int tdr_get_entry_sizeinfo_attribute_i(LPTDRMETA a_pstMeta, int a_idxEntry, scew_element *a_pstElement, FILE *a_fpError)
{
    scew_attribute *pstAttr = NULL;
    LPTDRMETALIB pstLib =NULL;
    const char *pszVal = NULL;
    int iRet = TDR_SUCCESS;
	LPTDRMETAENTRY pstEntry;

    assert(NULL != a_pstElement);
	assert(NULL != a_pstMeta);
    assert(NULL != a_fpError);
	assert((0 <= a_idxEntry) && (a_idxEntry <= a_pstMeta->iEntriesNum));


    pstLib = TDR_META_TO_LIB(a_pstMeta);
	pstEntry = a_pstMeta->stEntries + a_idxEntry;

	pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_SIZEINFO);
    if( NULL != pstAttr )
    {
        pszVal = scew_attribute_value(pstAttr);
        iRet = tdr_parse_entry_sizeinfo_off_i(a_pstMeta, a_idxEntry, pszVal, a_fpError);
    }

    return iRet;
}

int tdr_get_entry_sortmethod_attribute_i(LPTDRMETAENTRY a_pstEntry, LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError)
{
    scew_attribute *pstAttr = NULL;
    int iRet = TDR_SUCCESS;

    assert(NULL != a_pstEntry);
    assert(NULL != a_pstMeta);
    assert(NULL != a_pstElement);
    assert(NULL != a_fpError);

    pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_SORTMETHOD);
    if( NULL != pstAttr )
    {
		char szVal[32] = {0};

        tdr_normalize_string(&szVal[0], sizeof(szVal), scew_attribute_value(pstAttr));
        if (0 == tdr_stricmp(szVal, TDR_TAG_NO))
        {
            a_pstEntry->chOrder = TDR_SORTMETHOD_NONE_SORT;
        }else if ( 0== tdr_stricmp(szVal, TDR_TAG_ASC))
        {
            a_pstEntry->chOrder = TDR_SORTMETHOD_ASC_SORT;
        }else if ( 0== tdr_stricmp(szVal, TDR_TAG_DESC))
        {
            a_pstEntry->chOrder = TDR_SORTMETHOD_DSC_SORT;
        }else
        {
            LPTDRMETALIB pstLib = TDR_META_TO_LIB(a_pstMeta);
            fprintf(a_fpError, "\nerror:\t �Զ�������<name = %s>�ĳ�ԱԪ��<name = %s>��sortmethod����ֵ<%s>��Ч,\n",
                TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName), szVal);
            iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_ENTRY_INVALID_SORTMETHOD_VALUE);
        }
    }

    return iRet ;
}


int tdr_get_entry_select_attribute_i(LPTDRMETA a_pstMeta, int a_idxEntry, scew_element *a_pstElement, FILE *a_fpError)
{
    scew_attribute *pstAttr = NULL;
    LPTDRMETALIB pstLib =NULL;
    const char *pszVal = NULL;
    int iRet = TDR_SUCCESS;
	LPTDRMETAENTRY pstEntry;

    assert(NULL != a_pstElement);
    assert(NULL != a_pstMeta);
    assert(NULL != a_fpError);
	assert((0 <= a_idxEntry) && (a_idxEntry < a_pstMeta->iEntriesNum));

    pstLib = TDR_META_TO_LIB(a_pstMeta);
	pstEntry = a_pstMeta->stEntries + a_idxEntry;

    pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_SELECT);
    if( NULL != pstAttr )
    {
        LPTDRMETAENTRY pstFindEntry = NULL;

        pszVal = scew_attribute_value(pstAttr);
        iRet = tdr_name_to_off_i(a_pstMeta, &pstEntry->stSelector, a_idxEntry, pszVal);
        if (TDR_SUCCESS !=iRet)
        {
            fprintf(a_fpError, "error:\t �Զ�������<name = %s>�ĳ�ԱԪ��<name = %s>��select����ֵ<%s>��Ч,��ȷ��ָ����ȷ��Ԫ������·������·�����漰�ĸ�Ԫ�ز���ָ�����������\n",
                 TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName), pszVal);

            iRet = TDR_ERRIMPLE_ENTRY_INVALID_SELECT_VALUE;
        }else
        {
			pstFindEntry = TDR_PTR_TO_ENTRY(pstLib, pstEntry->stSelector.ptrEntry);
            if((pstFindEntry->iType <= TDR_TYPE_COMPOSITE)
                || (pstFindEntry->iType>TDR_TYPE_UINT) )
            {
                fprintf(a_fpError, "error:\t �Զ�������<name = %s>�ĳ�ԱԪ��<name = %s>��select����ֵ<%s>��Ч,\n",
                    TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName), pszVal);

                iRet = TDR_ERRIMPLE_ENTRY_INVALID_SELECT_VALUE;
			}else if (pstEntry->stRefer.iHOff >= pstEntry->iHOff)
			{
				fprintf(a_fpError, "\nerror:\t �Զ�������<name = %s>�ĳ�ԱԪ��<name = %s>��select����ֵ<%s>��Ч,�����Ǹ�Ԫ�ص�ֱ�Ӹ�Ԫ�ذ������ض���Ԫ��",
					TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName), pszVal);
				iRet = TDR_ERRIMPLE_ENTRY_INVALID_REFER_VALUE;
			}else if (1 != pstFindEntry->iCount)
			{
				fprintf(a_fpError, "\nerror:\t �Զ�������<name = %s>�ĳ�ԱԪ��<name = %s>��select����ֵ<%s>����Ϊ����\n",
					TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName), pszVal);
				iRet = TDR_ERRIMPLE_ENTRY_INVALID_REFER_VALUE;
			}
        }
    }/*if( NULL != pstAttr )*/

    return iRet;
}



int tdr_get_entry_maxmin_id_attribute_i(LPTDRMETAENTRY a_pstEntry, LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError)
{
    scew_attribute *pstMaxIdAttr = NULL;
    scew_attribute *pstMinIdAttr = NULL;

    int iRet = TDR_SUCCESS;
    LPTDRMETALIB pstLib = NULL;
    int iMaxID = 0;
    int iMinID = 0;
    int idxMax = TDR_INVALID_INDEX;
    int idxMin = TDR_INVALID_INDEX;
	char szVal[256] = {0};


    assert(NULL != a_pstEntry);
    assert(NULL != a_pstMeta);
    assert(NULL != a_pstElement);
    assert(NULL != a_fpError);



	pstLib = TDR_META_TO_LIB(a_pstMeta);

    pstMaxIdAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_MAXID);
    pstMinIdAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_MINID);
    if ((NULL == pstMaxIdAttr) && (NULL == pstMinIdAttr))
    {
        return TDR_SUCCESS;
    }else if ((NULL == pstMaxIdAttr) || (NULL == pstMinIdAttr))
    {
         /*maxid��minid����ͬʱ����*/
        fprintf(a_fpError, "\nerror:\t �Զ�������<name = %s>�ĳ�ԱԪ��<name = %s>��maxid��minid����ֵ����ͬʱ����",
            TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName));

        iRet = TDR_ERRIMPLE_ENTRY_INVALID_MAXMINID_VALUE;
    }

    /*��ȡmaxid����*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        tdr_normalize_string(&szVal[0], sizeof(szVal), scew_attribute_value(pstMaxIdAttr));
        iRet = tdr_get_macro_int_i(&iMaxID, &idxMax, pstLib, szVal);
        if (TDR_ERR_IS_ERROR(iRet))
        {
            fprintf(a_fpError, "\nerror:\t �Զ�������<name = %s>�ĳ�ԱԪ��<name = %s>��maxid����ֵ<%s>��Ч,��ȷ����ֵ���������Ѷ���ĺ�",
                TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName), szVal);

            iRet = TDR_ERRIMPLE_ENTRY_INVALID_MAXID_VALUE;
        }
    }


    /*��ȡminid����*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
		tdr_normalize_string(&szVal[0], sizeof(szVal), scew_attribute_value(pstMinIdAttr));
        iRet = tdr_get_macro_int_i(&iMinID, &idxMin, pstLib, szVal);
        if (TDR_SUCCESS != iRet)
        {
            fprintf(a_fpError, "\nerror:\t �Զ�������<name = %s>�ĳ�ԱԪ��<name = %s>��minid����ֵ<%s>��Ч,��ȷ����ֵ���������Ѷ���ĺ�",
                TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName), szVal);

            iRet = TDR_ERRIMPLE_ENTRY_INVALID_MINID_VALUE;
        }
    }/*if (!TDR_ERR_IS_ERROR(iRet))*/

    /*���maxid, minid���Ե���ȷ��*/
    if ((!TDR_ERR_IS_ERROR(iRet)) && (iMaxID < iMinID))
    {
        fprintf(a_fpError, "\nerror:\t �Զ�������<name = %s>�ĳ�ԱԪ��<name = %s>��maxid��minid����ֵ����ͬʱ����",
            TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName));

        iRet = TDR_ERRIMPLE_ENTRY_INVALID_MAXMINID_VALUE;
    }/*if ((!TDR_ERR_IS_ERROR(iRet)) && (iMaxID < iMinID))*/


    if (!TDR_ERR_IS_ERROR(iRet))
    {
		TDR_ENTRY_SET_HAVE_MAXMIN_ID(a_pstEntry);
        a_pstEntry->iMaxId = iMaxID;
        a_pstEntry->iMaxIdIdx = idxMax;
        a_pstEntry->iMinId = iMinID;
        a_pstEntry->iMinIdIdx = idxMin;
    }


    return iRet;
}


int tdr_get_entry_target_attribute_i(LPTDRMETA a_pstMeta, int a_idxEntry, scew_element *a_pstElement, FILE *a_fpError)
{
    scew_attribute *pstAttr = NULL;
    const char *pszTargetVal = NULL;
    int iRet = TDR_SUCCESS;


    assert(NULL != a_pstElement);
    assert(NULL != a_pstMeta);
    assert(NULL != a_fpError);
	assert((0 <= a_idxEntry) && (a_idxEntry <= a_pstMeta->iEntriesNum));


    /*��ȡtarget����,union�ṹû�д�����*/
    if (TDR_TYPE_UNION != a_pstMeta->iType)
    {
        pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_TARGET);
        if( NULL != pstAttr )
        {
            pszTargetVal = scew_attribute_value(pstAttr);
            iRet = tdr_parse_entry_sizeinfo_off_i(a_pstMeta, a_idxEntry, pszTargetVal, a_fpError);
        }
    }/*if (TDR_TYPE_UNION != a_pstMeta->iType)*/

    return iRet;
}

int tdr_parse_entry_sizeinfo_off_i(LPTDRMETA a_pstMeta, int a_idxEntry, const char* a_pszName, FILE *a_fpError)
{
	int iRet = TDR_SUCCESS;
	LPTDRMETALIB pstLib = NULL;
	LPTDRMETAENTRY pstEntry;
	LPTDRSIZEINFO pstSizeInfo;

	assert(NULL != a_pstMeta);
	assert(NULL != a_pszName);
	assert(NULL != a_fpError);
	assert((0 <= a_idxEntry) && (a_idxEntry <= a_pstMeta->iEntriesNum));

	pstLib = TDR_META_TO_LIB(a_pstMeta);
	pstEntry = a_pstMeta->stEntries + a_idxEntry;
	pstSizeInfo = &pstEntry->stSizeInfo;
	iRet = tdr_sizeinfo_to_off(pstSizeInfo, a_pstMeta, a_idxEntry, a_pszName);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		fprintf(a_fpError, "\nerror:\t �Զ�������<name = %s>�ĳ�Ա<name=%s>sizeinfo����ֵ<%s>��Ч,������ֵֻ��Ϊ<tinyuint-uint>���߸�Ԫ�ص�ֱ�Ӹ�Ԫ�ذ������ض���Ԫ�س�Ա��Path\n",
			TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName), a_pszName);
		return iRet;
	}

	/*�洢�ռ�̶���Ԫ�أ���sizeinfo���Ա�����*/
	if (((TDR_TYPE_COMPOSITE < pstEntry->iType) && (TDR_TYPE_STRING > pstEntry->iType)) ||
		(TDR_TYPE_VOID == pstEntry->iType))
	{
		pstSizeInfo->idxSizeType = TDR_INVALID_INDEX;
		pstSizeInfo->iHOff = TDR_INVALID_OFFSET;
		pstSizeInfo->iNOff = TDR_INVALID_OFFSET;
		pstSizeInfo->iUnitSize = 0;
		fprintf(a_fpError, "\nwarning:\t �Զ�������<name = %s>�ĳ�Ա<name=%s>�Ĵ洢�ռ��ǹ̶��ģ���sizeinfo���Ա�����\n",
			TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName));
		return TDR_SUCCESS;
	}

	/*string,wstring��Ա��sizetype����ֻ����tinyint-unit*/
	if ((TDR_TYPE_STRING == pstEntry->iType) || (TDR_TYPE_WSTRING == pstEntry->iType))
	{
		if (TDR_INVALID_INDEX == pstSizeInfo->idxSizeType)
		{
			fprintf(a_fpError, "\nerror:\t �Զ�������<name = %s>�ĳ�Ա<name=%s>Ϊstring���ͣ���sizeinfo����ֵֻ��Ϊ<tinyuint-uint>\n",
				TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName));
			return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_ENTRY_INVALID_SIZEINFO_VALUE);
		}
	}


	if (TDR_TYPE_COMPOSITE >= pstEntry->iType)
	{
		/*�����������ͣ������ṹ�������Ѿ�ָ����sizetype���ԣ�������ָ��sizeinfo����*/
		LPTDRMETA pstType = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
		if (pstType->stSizeType.iUnitSize > 0)
		{
			fprintf(a_fpError, "\nerror:\t �Զ�������<name = %s>�ĳ�Ա<name=%s>��Ӧ�Ľṹ��<name=%s>�Ѿ�ָ����sizetype���ԣ��˳�Ա������ָ��sizeinfo������\n",
				TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName), TDR_GET_STRING_BY_PTR(pstLib,pstType->ptrName));
			return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_ENTRY_INVALID_SIZEINFO_VALUE);
		}

		/*�����������������Ա����sizeinfo����ֵ����Ϊ�˳�Աǰ���ֵܽڵ�Ԫ��Ϊ����Path*/
		if ((pstSizeInfo->iNOff < pstEntry->iNOff) && ((1 < pstEntry->iCount) || (0 == pstEntry->iCount)))
		{
			fprintf(a_fpError, "\nerror:\t �Զ�������<name = %s>�ĳ�Ա<name=%s>Ϊ�����������飬��sizeinfo����ֵֻ��Ϊ<tinyuint-uint>���Ե�ǰԪ��Ϊ���ĳ�ԱPath\n",
				TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName));
			return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_ENTRY_INVALID_SIZEINFO_VALUE);
		}
	}

	return iRet;
}

int tdr_get_string_compress_attribute_i(LPTDRMETAENTRY a_pstEntry, LPTDRMETA a_pstMeta,
                                        scew_element *a_pstElement, FILE *a_fpError)
{
    scew_attribute *pstStringCompressAttr = NULL;

    int iRet = TDR_SUCCESS;
    LPTDRMETALIB pstLib = NULL;
    char szStrCompress[16] = {0};

    assert(NULL != a_pstEntry);
    assert(NULL != a_pstMeta);
    assert(NULL != a_pstElement);
    assert(NULL != a_fpError);
	pstLib = TDR_META_TO_LIB(a_pstMeta);

    pstStringCompressAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_STRING_COMPRESS);
    if ((NULL == pstStringCompressAttr))
    {
        return TDR_SUCCESS;
    }
	else
    {
    	tdr_normalize_string(&szStrCompress[0], sizeof(szStrCompress),
                             scew_attribute_value(pstStringCompressAttr));
    	if (0 != tdr_stricmp(szStrCompress, TDR_TAG_TRUE)
            && 0 != tdr_stricmp(szStrCompress, TDR_TAG_FALSE))
    	{
    		fprintf(a_fpError, "\nerror:\t �Զ�������<name = %s>�ĳ�ԱԪ��"
                    "<name = %s>��stringcompress����ֵ������true��false",
            TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName),
            TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName));
			iRet = TDR_ERRIMPLE_ENTRY_INVALID_MAXMINID_VALUE;
    	}
		else if (0 == tdr_stricmp(szStrCompress, TDR_TAG_TRUE))
		{
            int idx = -1;
            LPTDRCTYPEINFO pstTypeInfo;

			TDR_ENTRY_SET_STR_COMPRESS(a_pstEntry);
			a_pstEntry->iType = TDR_TYPE_INT;
			idx = tdr_typename_to_idx(TDR_TAG_INT);
			a_pstEntry->idxType = idx;
			pstTypeInfo = tdr_idx_to_typeinfo(idx);
			a_pstEntry->iHUnitSize = pstTypeInfo->iSize;
			a_pstEntry->iNUnitSize = pstTypeInfo->iSize;
		}
		else
		{
			TDR_ENTRY_CLR_STR_COMPRESS(a_pstEntry);
		}
    }

	return iRet;
}

int tdr_get_entry_version1_nooff_attribute_i(LPTDRMETAENTRY a_pstEntry, LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError)
{
    LPTDRMETALIB pstLib = NULL;
    int iRet = TDR_SUCCESS;

    assert(NULL != a_pstElement);
    assert(NULL != a_pstEntry);
    assert(NULL != a_pstMeta);
    assert(NULL != a_fpError);

    pstLib = TDR_META_TO_LIB(a_pstMeta);

	/*bindmacrosgroup*/
	iRet = tdr_get_entry_bindmacrosgroup_attribute_i(pstLib, a_pstEntry, a_pstElement, a_fpError);


    /*��ȡunique����*/
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		 iRet = tdr_get_entry_unique_attribute_i(a_pstEntry, a_pstElement, a_fpError);
	}

    /*cname����*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_get_cname_attribute_i(&a_pstEntry->ptrChineseName, pstLib, a_pstElement, a_fpError);
    }


    /*��ȡnotnull����*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_get_entry_notnull_attribute_i(a_pstEntry, a_pstElement, a_fpError);
    }

    /*��ȡsize����*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_get_entry_size_attribute_i(a_pstEntry, a_pstMeta, a_pstElement, a_fpError);
    }/*if (!TDR_ERR_IS_ERROR(iRet))*/


    /*defaultvalue*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
       iRet = tdr_get_entry_defaultvalue_attribute_i(a_pstEntry, a_pstMeta, a_pstElement, a_fpError);
    }/*if (!TDR_ERR_IS_ERROR(iRet))*/



    /*sortmethod*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_get_entry_sortmethod_attribute_i(a_pstEntry, a_pstMeta, a_pstElement, a_fpError);
    }

	/*extendtotable*/
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		iRet = tdr_get_entry_extendtotable_attribute_i(a_pstEntry, a_pstElement, a_fpError);
	}

	/*�Ƿ�ѹ��*/
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		iRet = tdr_get_entry_compress_attribute_i(a_pstEntry, a_pstElement, a_fpError);
	}
	/*auto increment*/
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		iRet = tdr_get_entry_autoincrement_attribute_i(a_pstMeta, a_pstEntry, a_pstElement, a_fpError);
	}

	/*customattr����*/
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		iRet = tdr_get_customattr_attribute_i(a_pstMeta, a_pstEntry, a_pstElement, a_fpError);
	}

    /*�����union meta���ȡunion entry���������: maxid minid*/
    if (!TDR_ERR_IS_ERROR(iRet) && (TDR_TYPE_UNION == a_pstMeta->iType))
    {
         iRet = tdr_get_entry_maxmin_id_attribute_i(a_pstEntry, a_pstMeta, a_pstElement, a_fpError);
    }

	//added by layne supporting resource-transfer tools
	if (TDR_TYPE_STRING == a_pstEntry->iType || TDR_TYPE_WSTRING == a_pstEntry->iType)
	{
		iRet = tdr_get_string_compress_attribute_i(a_pstEntry, a_pstMeta, a_pstElement, a_fpError);
	}
	//end added by layne supporting resource-transfer tools

    return iRet;
}



int tdr_check_entry_i(LPTDRMETAENTRY a_pstEntry, LPTDRMETA a_pstMeta, FILE *a_fpError)
{
    int iRet = TDR_SUCCESS;
    LPTDRMETALIB pstLib = NULL;
    TDRBOOLEAN bDyn = TDR_FALSE;

    assert(NULL != a_pstEntry);
    assert(NULL != a_pstMeta);
    assert(NULL != a_fpError);

    pstLib = TDR_META_TO_LIB(a_pstMeta);

	/*union ���ͼ��*/
	if ((TDR_TYPE_UNION == a_pstMeta->iType) && (TDR_TYPE_UNION == a_pstEntry->iType))
	{
		fprintf(a_fpError, "\nerror:\t union����<name = %s>�����ٰ���union��Ա<name = %s>",
			TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName));
		return TDR_ERRIMPLE_ENTRY_INVALID_TYPE_VALUE;
	}

	/*���count����*/
    if (0 == a_pstEntry->iCount)
    {
    	  if (((TDR_INVALID_OFFSET == a_pstEntry->stRefer.iHOff) || (TDR_INVALID_PTR == a_pstEntry->stRefer.ptrEntry) ||
       	 (0 >= a_pstEntry->stRefer.iUnitSize)))
   	 {
       	 fprintf(a_fpError, "error:\t %s�ṹ�ĳ�Ա<name = %s>Ϊ�ɱ����飬����ͨ��refer����ȷ����ʵ�ʴ�С\n",
           		 TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName));
       	 return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_VARIABLE_ARRAY_NO_REFER);
  	  }
  	  if (((TDR_TYPE_STRING == a_pstEntry->iType) || (TDR_TYPE_WSTRING == a_pstEntry->iType)) && (0 >= a_pstEntry->iCustomHUnitSize))
         {
       	fprintf(a_fpError, "error:\t %s�ṹ�ĳ�Ա<name = %s>Ϊ��ָ���洢�ռ���ַ������ͣ�����Ϊ�ɱ�����\n",
           		TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName));
       	 return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_VARIABLE_ARRAY_NO_REFER);
        }
    }/* if (0 == a_pstEntry->iCount)*/

    /*select ���Լ��*/
    if (TDR_TYPE_UNION == a_pstEntry->iType)
    {
        if ((TDR_INVALID_INDEX == a_pstEntry->stSelector.iHOff) ||
            (TDR_INVALID_PTR == a_pstEntry->stSelector.ptrEntry) ||
            (0 >= a_pstEntry->stSelector.iUnitSize))
        {
            fprintf(a_fpError, "error:\t %s�ṹ�ĳ�Ա<name = %s>Ϊunion�������ͣ�û��select���Ի�select����ֵ��Ч\n",
            TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName));
            return TDR_ERRIMPLE_ENTRY_INVALID_SELECT_VALUE;
        }
    }

    if (0 == a_pstEntry->iCount)
    {
        bDyn = TDR_TRUE;
    }
    if ((TDR_FALSE == bDyn) && (0 < a_pstEntry->iCount) && (a_pstEntry->stRefer.iUnitSize>0))
    {
        bDyn = TDR_TRUE;
    }

    if ( (TDR_FALSE == bDyn) && (TDR_TYPE_UNION == a_pstEntry->iType) &&
        ( a_pstEntry->stSelector.iUnitSize > 0  ))
    {
        bDyn = TDR_TRUE;
    }

    if ( (TDR_FALSE == bDyn) && (TDR_TYPE_STRING == a_pstEntry->iType))
    {
        bDyn = TDR_TRUE;
    }
	if ( (TDR_FALSE == bDyn) && (TDR_TYPE_WSTRING == a_pstEntry->iType))
	{
		bDyn = TDR_TRUE;
	}

    if ((TDR_FALSE == bDyn) && (TDR_INVALID_INDEX != a_pstEntry->ptrMeta))
    {
        LPTDRMETA pstTypeMeta = TDR_PTR_TO_META(pstLib, a_pstEntry->ptrMeta);
        if (!TDR_META_IS_FIXSIZE(pstTypeMeta))
        {
            bDyn = TDR_TRUE;
        }
    }

    if (TDR_FALSE == bDyn)
    {
        TDR_ENTRY_SET_FIXSIZE(a_pstEntry);
    }

	/*string wstring���� ����ȱʡsizeinfo����*/
	if (((TDR_TYPE_STRING == a_pstEntry->iType) || (TDR_TYPE_WSTRING == a_pstEntry->iType)) && (0 >= a_pstEntry->stSizeInfo.iUnitSize))
	{
		LPTDRCTYPEINFO pstType;
		a_pstEntry->stSizeInfo.idxSizeType = tdr_typename_to_idx(TDR_STRING_DEFULT_SIZEINFO_VALUE);
		pstType = tdr_idx_to_typeinfo(a_pstEntry->stSizeInfo.idxSizeType);
		assert(NULL != pstType);
		a_pstEntry->stSizeInfo.iUnitSize = pstType->iSize;
	}



	/*���sortmethod���ԣ�Ŀǰ��֧�ֶԴ洢��Ԫ���̶��������������*/
	if (a_pstEntry->chOrder != TDR_SORTMETHOD_NONE_SORT)
	{
		TDRBOOLEAN bDynEntry = TDR_FALSE;

		if (((TDR_TYPE_WSTRING == a_pstEntry->iType) || (TDR_TYPE_STRING == a_pstEntry->iType)) &&
			(0 >= a_pstEntry->iCustomHUnitSize))
		{
			bDynEntry = TDR_TRUE;
		}
		if (TDR_INVALID_PTR != a_pstEntry->ptrMeta)
		{
			LPTDRMETA pstTypeMeta = TDR_PTR_TO_META(pstLib, a_pstEntry->ptrMeta);
			if (TDR_META_IS_VARIABLE(pstTypeMeta))
			{
				bDynEntry = TDR_TRUE;
			}
		}
		if (TDR_TRUE == bDynEntry)
		{
			LPTDRCTYPEINFO pstTypeInfo = tdr_idx_to_typeinfo(a_pstEntry->idxType);
			fprintf(a_fpError, "error:\t %s�ṹ�ĳ�Ա<name = %s>������Ϊ<%s>,�˳�ԱΪ��Ԫ�洢�ռ䲻�̶������飬Ŀǰ��֧�ֶ����������\n",
				TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName), pstTypeInfo->pszName);
			return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_ENTRY_INVALID_SORTMETHOD_VALUE);
		}
	}/*if (a_pstEntry->chOrder != TDR_SORTMETHOD_NONE_SORT)*/

	/*���汾����*/
	if ((a_pstEntry->iVersion < a_pstMeta->iBaseVersion) && (pstLib->iXMLTagSetVer > TDR_XML_TAGSET_VERSION_0))
	{
		/*�ṹ��Ա�İ汾ֵ����С��������汾*/
		fprintf(a_fpError, "error:\t %s�ṹ�ĳ�Ա<name=%s>�İ汾<%d>���ܱȽṹ�Ļ����汾<%d>��.\n", TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName),
			a_pstEntry->iVersion, a_pstMeta->iBaseVersion);
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_VERSION);
	}
	if ((TDR_INVALID_PTR != a_pstEntry->ptrMeta) && (pstLib->iXMLTagSetVer > TDR_XML_TAGSET_VERSION_0))
	{
		LPTDRMETA pstTypeMeta = TDR_PTR_TO_META(pstLib, a_pstEntry->ptrMeta);
		if (a_pstEntry->iVersion < pstTypeMeta->iBaseVersion)
		{
			/*�ṹ��Ա�İ汾ֵ����С��������汾*/
			fprintf(a_fpError, "error:\t %s�ṹ�ĳ�Ա<name=%s>�Ǹ�����������<type=%s>,�˳�Ա�İ汾<%d>���ܱ����������͵Ļ����汾<%d>��.\n", TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName),
				TDR_GET_STRING_BY_PTR(pstLib,pstTypeMeta->ptrName), a_pstEntry->iVersion, pstTypeMeta->iBaseVersion);
			return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_VERSION);
		}
	}

    return iRet;
}



LPTDRMETAENTRY tdr_get_entry_by_id_i(TDRMETAENTRY pstEntry[], int iMax, int iID)
{
    int i;

    assert( NULL != pstEntry );

    for(i=0; i<iMax; i++)
    {
        if (TDR_ENTRY_DO_HAVE_ID(pstEntry) && ( iID == pstEntry[i].iID ))
        {
            break;
        }
    }

    return (i < iMax) ? &pstEntry[i] : NULL;
}

int tdr_get_entry_type_i(LPTDRMETAENTRY a_pstEntry, LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError)
{
    scew_attribute *pstAttr = NULL;
    LPTDRMETALIB pstLib =NULL;
    const char *pszType = NULL;
    int idx = TDR_INVALID_INDEX;
    LPTDRCTYPEINFO pstTypeInfo = NULL;
    int iRet = TDR_SUCCESS;
    char szType[1024] = {0};

    assert(NULL != a_pstElement);
    assert(NULL != a_pstEntry);
    assert(NULL != a_pstMeta);
    assert(NULL != a_fpError);


    pstLib = TDR_META_TO_LIB(a_pstMeta);

    /* type attribute must be exist. */
    pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_TYPE);
    if( NULL == pstAttr )
    {
        fprintf(a_fpError, "error:\t �Զ�������<name = %s>�ĳ�ԱԪ��<name = %s>û�а���type����.\n",
            TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName));

        return TDR_ERRIMPLE_ENTRY_NO_TYPE;
    }

    tdr_normalize_string(&szType[0], sizeof(szType), scew_attribute_value(pstAttr));
    pszType = &szType[0];
    if (*pszType == '\0')
    {
        fprintf(a_fpError, "error:\t �ṹ<name = %s>�ĳ�ԱԪ��<name = %s>��type���Բ���Ϊ�մ�.\n",
            TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName));

        return TDR_ERRIMPLE_ENTRY_NO_TYPE;
    }


    /*tagsetversion1֧��ָ�������*/
    if (TDR_XML_TAGSET_VERSION_0 < pstLib->iXMLTagSetVer)
    {
        if (TDR_TAG_POINTER_TYPE == *pszType)
        {
			TDR_ENTRY_SET_POINT_TYPE(a_pstEntry);
            pszType++;
        }else if (TDR_TAG_REFER_TYPE == *pszType)
        {
			TDR_ENTRY_SET_REFER_TYPE(a_pstEntry);
            pszType++;
        }
    }


    /*��ȡentry��type*/
    idx = tdr_typename_to_idx(pszType);
    if (TDR_INVALID_INDEX != idx)
    {
        /*������������*/
        pstTypeInfo = tdr_idx_to_typeinfo(idx);
        if (TDR_TYPE_COMPOSITE < pstTypeInfo->iType)
        {
            a_pstEntry->idxType = idx;
            a_pstEntry->iType	 = pstTypeInfo->iType;
            a_pstEntry->iHUnitSize = pstTypeInfo->iSize;
			a_pstEntry->iNUnitSize = pstTypeInfo->iSize;
        }else/*if (TDR_TYPE_COMPOSITE < pstTypeInfo->iType)*/
        {
            fprintf(a_fpError, "error:\t �ṹ<name = %s>�ĳ�ԱԪ��<name = %s>��type����ֵ<%s>��Ч.\n",
                TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName), pszType);

            iRet = TDR_ERRIMPLE_ENTRY_INVALID_TYPE_VALUE;
        }/*if (TDR_TYPE_COMPOSITE < pstTypeInfo->iType)*/

    }else /*if (TDR_INVALID_INDEX != idx)*/
    {
        /*�Զ�������*/
        LPTDRMETA pstTypeMeta = tdr_get_meta_by_name_i(pstLib, pszType);
		if ((NULL == pstTypeMeta) ||
			((0 >= pstTypeMeta->iNUnitSize) && !TDR_ENTRY_IS_POINTER_TYPE(a_pstEntry) && !TDR_ENTRY_IS_REFER_TYPE(a_pstEntry)))
		{
			fprintf(a_fpError, "error:\t �Զ�������<name = %s>�ĳ�ԱԪ��<name = %s>������<type=%s>û�ж���,��ָ��������⣬��Ա���������Ͳ�֧�ֺ������á�\n",
				TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName), szType);

			iRet = TDR_ERRIMPLE_ENTRY_INVALID_TYPE_VALUE;
		}else
		{
			a_pstEntry->ptrMeta = pstTypeMeta->ptrMeta;
			a_pstEntry->idxType = pstTypeMeta->idxType;
			a_pstEntry->iType = pstTypeMeta->iType;
			a_pstEntry->iNUnitSize = pstTypeMeta->iNUnitSize;
			a_pstEntry->iHUnitSize = pstTypeMeta->iHUnitSize;
		}
    }/*if (TDR_INVALID_INDEX != idx)*/


    /*�����ָ����������ͣ������¼���unitsize*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        if (TDR_ENTRY_IS_POINTER_TYPE(a_pstEntry))
        {
			a_pstEntry->iHUnitSize = TDR_SIZEOF_POINTER;
			a_pstEntry->iNUnitSize = 0;
        }
		if (TDR_ENTRY_IS_REFER_TYPE(a_pstEntry))
		{
			a_pstEntry->iHUnitSize = TDR_SIZEOF_POINTER;
		}
    }/*if (!TDR_ERR_IS_ERROR(iRet))*/


	/*void ����֧��voidָ������*/
	if (!TDR_ERR_IS_ERROR(iRet) && (a_pstEntry->iType == TDR_TYPE_VOID) && !(TDR_ENTRY_IS_POINTER_TYPE(a_pstEntry)))
	{
		fprintf(a_fpError, "error:\t �Զ�������<name = %s>�ĳ�ԱԪ��<name = %s>������Ϊ<type=%s>��Ŀǰֻ֧��ͨ��ָ�루*void���������͡�\n",
			TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName), szType);

		iRet = TDR_ERRIMPLE_ENTRY_INVALID_TYPE_VALUE;
	}

    return iRet;
}

int tdr_sortkeyinfo_to_path_i(LPTDRMETALIB a_pstLib, LPTDRSORTKEYINFO a_pstSortKey, char* a_pszBuff, int a_iBuff)
{
	int iRet = TDR_SUCCESS;
	TDROFF iCurOff = 0;
	LPTDRMETAENTRY pstEntry = NULL;
	LPTDRMETA pstSearchMeta = NULL;
	int iTempLen = 0;
	char *pszBuff = NULL;
	int iLeftLen = 0;
	TDRBOOLEAN bGetFirstName = TDR_FALSE;
	int i = 0;
	char *pszName = NULL;
	TDROFF iSearchHOff;

	assert(NULL != a_pstLib);
	assert(NULL != a_pszBuff);
	assert(0 < a_iBuff);
	assert(NULL != a_pstSortKey);
	assert(TDR_INVALID_PTR != a_pstSortKey->ptrSortKeyMeta);
	assert(TDR_INVALID_OFFSET != a_pstSortKey->iSortKeyOff);


	i = 0;
	pszBuff = a_pszBuff;
	iLeftLen = a_iBuff;
	pstSearchMeta = TDR_PTR_TO_META(a_pstLib, a_pstSortKey->ptrSortKeyMeta);
	iSearchHOff = a_pstSortKey->iSortKeyOff;
	while (iCurOff < iSearchHOff )
	{
		if ( i >= pstSearchMeta->iEntriesNum )
		{
			/*�Ѿ�����������entry����û��entry��ƫ�ƺ�ָ����ƫ��ֵƥ��*/
			iRet = TDR_ERRIMPLE_INVALID_OFFSET;
			break;
		}

		pstEntry = pstSearchMeta->stEntries + i;
		if( ((iCurOff + pstEntry->iHOff) > iSearchHOff) ||
			((TDROFF)(iCurOff + pstEntry->iHOff + pstEntry->iHUnitSize) <= iSearchHOff) )
		{
			i++;
			continue;
		}

		/*����entry�ķ�Χ����a_iOff*/
		pszName = TDR_GET_STRING_BY_PTR(a_pstLib,pstEntry->ptrName);

		/*��¼entry������*/
		if( bGetFirstName == TDR_FALSE )
		{
			iTempLen = tdr_snprintf(pszBuff, iLeftLen, "%s", pszName);
			TDR_CHECK_BUFF(pszBuff, iLeftLen, iTempLen, iRet);
			bGetFirstName = TDR_TRUE;
		}else
		{
			iTempLen = tdr_snprintf(pszBuff, iLeftLen, ".%s", pszName);
			TDR_CHECK_BUFF(pszBuff, iLeftLen, iTempLen, iRet);
		}
		if (TDR_ERR_IS_ERROR(iRet))
		{/*���������*/
			break;
		}


		iCurOff += pstEntry->iHOff;
		if (( TDR_INVALID_PTR == pstEntry->ptrMeta ) ||
			TDR_ENTRY_IS_POINTER_TYPE(pstEntry) || TDR_ENTRY_IS_REFER_TYPE(pstEntry))
		{
			pstSearchMeta = NULL;
			break;
		}else
		{
			pstSearchMeta = TDR_PTR_TO_META(a_pstLib, pstEntry->ptrMeta);
			i =	0;
		}
	}

	/*�Ҳ���ָ��ƫ��ֵ��entry����ֱ�ӷ���*/
	if( iCurOff != iSearchHOff )
	{
		if (!TDR_ERR_IS_ERROR(iRet))
		{
			iRet = TDR_ERRIMPLE_INVALID_OFFSET;
		}
		return iRet;
	}


	/*����ҵ�ָ��ƫ��ֵ��entry���Ҵ�entryΪ����meta��
	*�������µݹ��ȡentry���֣�ֱ���������͵�entry*/
	if (0 == a_pstSortKey->idxSortEntry)
	{
		while( NULL != pstSearchMeta )
		{
			pstEntry = pstSearchMeta->stEntries;  /*ȡ��һ��Ԫ��*/
			pszName = TDR_GET_STRING_BY_PTR(a_pstLib,pstEntry->ptrName);


			/*��¼entry������*/
			if( bGetFirstName == TDR_FALSE )
			{
				iTempLen = tdr_snprintf(pszBuff, iLeftLen, "%s", pszName);
				TDR_CHECK_BUFF(pszBuff, iLeftLen, iTempLen, iRet);
				bGetFirstName = TDR_TRUE;
			}else
			{
				iTempLen = tdr_snprintf(pszBuff, iLeftLen, ".%s", pszName);
				TDR_CHECK_BUFF(pszBuff, iLeftLen, iTempLen, iRet);
			}
			if (TDR_ERR_IS_ERROR(iRet))
			{/*���������*/
				break;
			}

			if (pstSearchMeta->ptrMeta == a_pstSortKey->ptrSortKeyMeta)
			{
				break;
			}

			/*�����meta����Ҫ���ҵ�meta����*/
			if (( TDR_INVALID_PTR == pstEntry->ptrMeta ) ||
				TDR_ENTRY_IS_POINTER_TYPE(pstEntry) || TDR_ENTRY_IS_REFER_TYPE(pstEntry))
			{
				pstSearchMeta = NULL;
				break;
			}else
			{
				pstSearchMeta = TDR_PTR_TO_META(a_pstLib, pstEntry->ptrMeta);
			}
		}/*while( NULL != pstSearchMeta )*/
	}/*if (0 == a_pstSortKey->idxSortEntry)*/



	return iRet;
}



int tdr_get_entry_extendtotable_attribute_i(LPTDRMETAENTRY a_pstEntry, scew_element *a_pstElement, FILE *a_fpError)
{
	scew_attribute *pstAttr = NULL;
	int iRet = TDR_SUCCESS;
	char szVal[256] = {0};
    LPTDRMETA  pstMeta = TDR_ENTRY_TO_META(a_pstEntry);
    LPTDRMETALIB pstLib = TDR_META_TO_LIB(pstMeta);

	assert(NULL != a_pstEntry);
	assert(NULL != a_pstElement);
	assert(NULL != a_fpError);



	pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_EXTENDTOTABLE);
	if (NULL == pstAttr)
	{
		return TDR_SUCCESS;
	}

	/*ֻ��struct��Աָ�������ݲ���Ч*/
	if (TDR_TYPE_STRUCT != a_pstEntry->iType)
	{
		fprintf(a_fpError, "\nWarnnig:\t entryԪ��<name = %s>����struct�������ͣ���extendtotable���Խ����ԡ�\n",
			TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName));

		TDR_ENTRY_CLR_EXTENDABLE(a_pstEntry);
		return TDR_SUCCESS;
	}

	tdr_normalize_string(&szVal[0], sizeof(szVal), scew_attribute_value(pstAttr));
	if (0 == tdr_stricmp(szVal, TDR_TAG_TRUE))
	{
		TDR_ENTRY_SET_EXTENDABLE(a_pstEntry);
	}else if ( 0== tdr_stricmp(szVal, TDR_TAG_FALSE))
	{
		TDR_ENTRY_CLR_EXTENDABLE(a_pstEntry);
	}else
	{
		fprintf(a_fpError, "\nerror:\t entryԪ��<name = %s>��extendtotable����ֵ<%s>��Ч,��ֵΪ\"false\"����\"true\"\n",
			TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName), szVal);

		iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_ENTRY_INVALID_EXTENDTOTABLE_VALUE);
	}/*if ( 0== strcmp(pszVal, TDR_TAG_FALSE))*/


	return iRet;
}
int tdr_get_entry_compress_attribute_i(LPTDRMETAENTRY a_pstEntry, scew_element *a_pstElement, FILE *a_fpError)
{
	scew_attribute *pstAttr = NULL;
	int iRet = TDR_SUCCESS;
	char szVal[256] = {0};
    LPTDRMETA  pstMeta = TDR_ENTRY_TO_META(a_pstEntry);
    LPTDRMETALIB pstLib = TDR_META_TO_LIB(pstMeta);

	assert(NULL != a_pstEntry);
	assert(NULL != a_pstElement);
	assert(NULL != a_fpError);


	pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_COMPRESS);
	if (NULL == pstAttr)
	{
		return TDR_SUCCESS;
	}

	/*ֻ��structָ�������ݲ���Ч*/
	if (TDR_TYPE_STRUCT != a_pstEntry->iType && TDR_TYPE_UNION != a_pstEntry->iType)
	{
		fprintf(a_fpError, "\nWarnnig:\t entryԪ��<name = %s>����struct��union����,��compress���Խ����ԡ�\n",
			TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName));
		return TDR_SUCCESS;
	}

	tdr_normalize_string(&szVal[0], sizeof(szVal), scew_attribute_value(pstAttr));
	if (0 == tdr_stricmp(szVal, TDR_TAG_TRUE))
	{
		TDR_ENTRY_SET_COMPRESS(a_pstEntry);
	}else if ( 0== tdr_stricmp(szVal, TDR_TAG_FALSE))
	{
		TDR_ENTRY_CLR_COMPRESS(a_pstEntry);
	}else
	{
		fprintf(a_fpError, "\nerror:\t entryԪ��<name = %s>��compress����ֵ<%s>��Ч,��ֵΪ\"false\"����\"true\"\n",
			TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName), szVal);

		iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_ENTRY_INVALID_COMPRESS);
	}/*if ( 0== strcmp(pszVal, TDR_TAG_FALSE))*/


	/*���extendtotable��compress�Ƿ��ͻ*/
	if (TDR_ENTRY_DO_COMPRESS(a_pstEntry) && TDR_ENTRY_DO_EXTENDABLE(a_pstEntry))
	{
		fprintf(a_fpError, "\nerror:\t entryԪ��<name = %s>��compress����ֵ��extendtotable���Գ�ͻ\n",
			TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName));

		iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_ENTRY_INVALID_COMPRESS_EXTENDTOTABLE);
	}

	return iRet;
}



/** ��ȡbindmacrosgroup������Ϣ
*/
int tdr_get_entry_bindmacrosgroup_attribute_i(IN LPTDRMETALIB a_pstLib, LPTDRMETAENTRY a_pstEntry, scew_element *a_pstElement, FILE *a_fpError)
{
	scew_attribute *pstAttr = NULL;
	int iRet = TDR_SUCCESS;
	char szName[TDR_NAME_LEN] = {0};
	LPTDRMACROSGROUP pstTmpGroup;

	assert(NULL != a_pstEntry);
	assert(NULL != a_pstElement);
	assert(NULL != a_fpError);


	pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_BIND_MACROSGROUP);
	if (NULL == pstAttr)
	{
		return TDR_SUCCESS;
	}

	/*ֻ���������ͳ�Աָ�������ݲ���Ч*/
	if ((TDR_TYPE_CHAR > a_pstEntry->iType) || (TDR_TYPE_ULONG < a_pstEntry->iType))
	{
		fprintf(a_fpError, "\nWarnnig:\t entryԪ��<name = %s>���������������ͣ���%s���Խ����ԡ�\n",
			TDR_GET_STRING_BY_PTR(a_pstLib,a_pstEntry->ptrName), TDR_TAG_BIND_MACROSGROUP);
		return TDR_SUCCESS;
	}

	tdr_normalize_string(&szName[0], sizeof(szName), scew_attribute_value(pstAttr));
	pstTmpGroup = tdr_find_macrosgroup_i(a_pstLib, szName);
	if (NULL != pstTmpGroup)
	{
		a_pstEntry->ptrMacrosGroup = (TDRPTR)((char *)pstTmpGroup - (char *)a_pstLib - offsetof(TDRMETALIB, data));
	}else
	{
		fprintf(a_fpError, "error:\t entryԪ��<name = %s>��%s����ֵ<%s>��Ч,��Ԫ���ݿ����Ҳ�����Ӧ�ĺ궨����\n",
			TDR_GET_STRING_BY_PTR(a_pstLib,a_pstEntry->ptrName), TDR_TAG_BIND_MACROSGROUP, szName);

		iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_ENTRY_INVALID_BINDMACROSGROUP);
	}/*if ( 0== strcmp(pszVal, TDR_TAG_FALSE))*/


	return iRet;
}


int tdr_get_entry_autoincrement_attribute_i(LPTDRMETA a_pstMeta, LPTDRMETAENTRY a_pstEntry, scew_element *a_pstElement,FILE *a_fpError)
{
	scew_attribute *pstAttr = NULL;
	int iRet = TDR_SUCCESS;
	char szValue[256] = {0};
    LPTDRMETALIB pstLib = TDR_META_TO_LIB(a_pstMeta);

	assert(NULL != a_pstEntry);
	assert(NULL != a_pstElement);
	assert(NULL != a_fpError);
	assert(NULL != a_pstMeta);


	pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_AUTOINCREMENT);
	if (NULL == pstAttr)
	{
		return TDR_SUCCESS;
	}

	/*ֻ���������ͳ�Աָ�������ݲ���Ч*/
	if ((TDR_TYPE_CHAR > a_pstEntry->iType) || (TDR_TYPE_ULONGLONG < a_pstEntry->iType))
	{
		fprintf(a_fpError, "\nError:\t �ṹ��<name=%s>�ĳ�Ա"
                "<name = %s type=%d>���������������ͣ����ܶ���%s���ԡ�\n",
                TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName),
                TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName),
                a_pstEntry->iType,TDR_TAG_AUTOINCREMENT);
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_AUTOINCREMENT);
	}

	tdr_normalize_string(&szValue[0], sizeof(szValue), scew_attribute_value(pstAttr));
	if (0 == tdr_stricmp(szValue, TDR_TAG_TRUE))
	{
		TDR_ENTRY_SET_AUTOINCREMENT(a_pstEntry);
	}else if ( 0== tdr_stricmp(szValue, TDR_TAG_FALSE))
	{
		TDR_ENTRY_CLR_AUTOINCREMENT(a_pstEntry);
	}else
	{
		fprintf(a_fpError, "\nerror:\t �ṹ��<name=%s>�ĳ�Ա"
                "<name = %s>��%s����ֵ<%s>��Ч,��ֵΪ\"false\"����\"true\"\n",
                TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName),
                TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName),
                TDR_TAG_AUTOINCREMENT, szValue);

		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_AUTOINCREMENT);
	}/*if ( 0== tdr_stricmp(szValue, TDR_TAG_FALSE))*/

	if (1 != a_pstEntry->iCount)
	{
		fprintf(a_fpError, "\nerror:\t �ṹ��<name=%s>�ĳ�Ա"
                "<name = %s>��%s������Ч,ֻ�з������Ա���ܶ��������\n",
                TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName),
                TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName), TDR_TAG_AUTOINCREMENT);

		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_AUTOINCREMENT);
	}

    if (TDR_ENTRY_IS_AUTOINCREMENT(a_pstEntry))
    {
        if (TDR_META_DO_HAVE_AUTOINCREMENT_ENTRY(a_pstMeta))
        {
            fprintf(a_fpError, "\nerror:\t �ṹ����ֻ����һ����Ա����%s=\"true\"��"
                    "�ṹ��<name=%s>���Ѿ������˶���%s=\"true\"�ĳ�Ա��"
                    "��Ա<name = %s>�����ٶ��������ֵ\n",
                    TDR_TAG_AUTOINCREMENT, TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName),
                    TDR_TAG_AUTOINCREMENT, TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName));

            return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_AUTOINCREMENT);
        }else
        {
            TDR_META_SET_HAVE_AUTOINCREMENT_ENTRY(a_pstMeta);
        }
    }


	return iRet;
}

/**��ȡ��Ա��customattr����
*/
int tdr_get_customattr_attribute_i(IN LPTDRMETA a_pstMeta, IN LPTDRMETAENTRY a_pstEntry, IN scew_element *a_pstElement,
								   IN FILE *a_fpError)
{
	int iRet = TDR_SUCCESS;
	scew_attribute *pstAttr = NULL;
	const char *pszValue;
	size_t iStrLen = 0;
	LPTDRMETALIB pstLib;

	assert(NULL != a_pstMeta);
	assert(NULL != a_pstEntry);
	assert(NULL != a_pstElement);
	assert(NULL != a_fpError);

	pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_CUSTOMATTR);
	if( NULL == pstAttr )
	{
		return TDR_SUCCESS;
	}
	pszValue = scew_attribute_value(pstAttr);
	if (NULL == pszValue)
	{
		return TDR_SUCCESS;
	}

    pstLib = TDR_META_TO_LIB(a_pstMeta);
	iStrLen = strlen(pszValue) + 1; /*��1��Ϊ�˱��������ַ�*/
	if (iStrLen > TDR_MAX_CUMTOMATTR_VALUE_LEN)
	{
		fprintf(a_fpError, "\nerror:\t ������������<name=%s>�ĳ�Ա<name = %s>��%s����ֵ"
			"����(%"TDRPRI_SIZET")��������󳤶�<%d>����",
			TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName),  TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName), TDR_TAG_CUSTOMATTR,
			iStrLen, TDR_MAX_CUMTOMATTR_VALUE_LEN);
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUSTOMATTR);
	}

	pstLib = TDR_META_TO_LIB(a_pstMeta);
	if ((1 < iStrLen) && (iStrLen > pstLib->iFreeStrBufSize))
	{
		fprintf(a_fpError, "\nerror:\t metalib�ַ���������ʣ��ռ�<freeSpace = %"TDRPRI_SIZET">, ���ܴ洢custom����: %s",
			pstLib->iFreeStrBufSize, pszValue);

		iRet = TDR_ERRIMPLE_NO_SPACE_FOR_STRBUF;
	}else
	{
		TDR_COPY_STRING_TO_BUF(a_pstEntry->ptrCustomAttr, pszValue, iStrLen, pstLib);
	}


	return iRet;
}

int tdr_get_struct_index_column_attribute_i(LPTDRMETA a_pstMeta, LPTDRMETAINDEX a_pstIndex,
                                            int *a_piColumnNum, scew_element *a_pstElement, FILE *a_fpError)
{
    int iRet = TDR_SUCCESS;
    scew_attribute *pstAttr = NULL;
    LPTDRMETALIB pstLib;
    char *pszKey = NULL;
    char *pchDot;
    char *pchBegin;
    LPTDRMETAENTRY pstEntry;
    TDRSelector stSelector;
    LPTDRCOLUMNINFO pstColumn;

    assert(NULL != a_pstMeta);
    assert(NULL != a_pstIndex);
    assert(NULL != a_piColumnNum);
    assert(NULL != a_pstElement);
    assert(NULL != a_fpError);

    pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_INDEX_COLUMN);
    if (NULL == pstAttr)
    {
        return TDR_SUCCESS;
    }

    a_pstIndex->iColumnNum = 0;
    a_pstIndex->ptrColumnBase = a_pstMeta->ptrColumns + *a_piColumnNum * sizeof(TDRCOLUMNINFO);

    pstLib = TDR_META_TO_LIB(a_pstMeta);
    pstColumn = TDR_GET_COLUMN_PTR(a_pstMeta, a_pstIndex);

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

        iRet = tdr_parse_index_column_i(a_pstMeta, &stSelector, pchBegin, a_fpError);
        if (TDR_ERR_IS_ERROR(iRet))
        {
            fprintf(a_fpError, "error:\t <name = %s>��%s����ֵ<%s>��Ч,��ȷ����ֵ����\',\'�ָ������ӳ�Ա��.\n",
                    TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), TDR_TAG_INDEX_COLUMN, pchBegin);
            iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_INDEXCOLUMN);
            break;
        }

        pstEntry = TDR_PTR_TO_ENTRY(pstLib, stSelector.ptrEntry);
        if ((1 < pstEntry->iCount) &&
            ((TDR_TYPE_CHAR > pstEntry->iType) || (TDR_TYPE_BYTE < pstEntry->iType)))
        {
            fprintf(a_fpError, "error:\t <name = %s>��column����ֵ<%s>��Ч,��char��byte�����⣬"
                    "Ŀǰ��֧���������͵������Ա��Ϊ������Ա.\n", TDR_GET_STRING_BY_PTR(pstLib, a_pstIndex->ptrName), pszKey);
            iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_INDEXCOLUMN);
            break;
        }

        a_pstIndex->iColumnNum++;
        pstColumn->ptrEntry = stSelector.ptrEntry;
        pstColumn->iHOff = pstEntry->iHOff;
        pstColumn++;

        if (NULL == pchDot)
        {
            break;
        }
        pchDot++;
        pchBegin = pchDot;
    }/*while ('\0' != *pchBegin)*/

    free(pszKey);

    if (!TDR_ERR_IS_ERROR(iRet))
    {
        *a_piColumnNum += a_pstIndex->iColumnNum;
    }

    return iRet;
}

int tdr_parse_index_column_i(IN LPTDRMETA a_pstMeta, OUT LPTDRSelector a_pstSelector,
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

    int iPreNodeIsExtenable = 1;
    int iHasParsedOneEntry = 0;

    assert( NULL != a_pstMeta);
    assert(NULL != a_pstSelector);
    assert(NULL != a_pszName);
    assert(NULL != a_pstSelector);

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
        {
            break;
        }
        tdr_trim_str(szBuff);

        if (iHasParsedOneEntry && !iPreNodeIsExtenable)
        {
            fprintf(a_fpError, "error:\t column����ָ����·��%s������һ��������չ��<%s>���͵�entry <name = %s>\n",
                    a_pszName, TDR_GET_STRING_BY_PTR(pstLib, pstSearchMeta->ptrName),
                    TDR_GET_STRING_BY_PTR(pstLib, pstEntry->ptrName));
            iRet = TDR_ERRIMPLE_INVALID_PATH_VALUE;
            break;
        }

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

        iPreNodeIsExtenable = 0;
        if( TDR_INVALID_PTR != pstEntry->ptrMeta )
        {
            pstSearchMeta	=	TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
            if (TDR_ENTRY_DO_EXTENDABLE(pstEntry))
            {
                iPreNodeIsExtenable = 1;
            }
        }else
        {
            pstSearchMeta	=	NULL;
        }

        if (!iHasParsedOneEntry)
        {
            iHasParsedOneEntry = 1;
        }

    }while( NULL != pszPtr );

    if ((NULL != pszPtr)  || (NULL == pstEntry))
    {
        iRet = TDR_ERRIMPLE_INVALID_PATH_VALUE;
    }

    if (iHasParsedOneEntry && iPreNodeIsExtenable)
    {
        fprintf(a_fpError, "error:\t column����ָ����·��%s������һ������չ��<%s>���͵�entry <name = %s>\n",
                a_pszName, TDR_GET_STRING_BY_PTR(pstLib, pstSearchMeta->ptrName),
                TDR_GET_STRING_BY_PTR(pstLib, pstEntry->ptrName));
        iRet = TDR_ERRIMPLE_INVALID_PATH_VALUE;
    }

    if (!TDR_ERR_IS_ERROR(iRet))
    {
        a_pstSelector->iHOff = iHOff;
        a_pstSelector->iUnitSize = pstEntry->iNUnitSize;
        a_pstSelector->ptrEntry = TDR_ENTRY_TO_PTR(pstLib, pstEntry);
    }


    return iRet;
}
