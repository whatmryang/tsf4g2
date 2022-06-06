/**
 *
 * @file     tdr_metalib_manage.c
 * @brief    元数据库管理相关模块
 *
 * @author steve jackyai
 * @version 1.0
 * @date 2007-04-02
 *
 *
 * Copyright (c)  2007, 腾讯科技有限公司互动娱乐研发部
 * All rights reserved.
 *
 */

#include <assert.h>
#include <ctype.h>
#include "tdr_error_i.h"
#include "tdr/tdr_metalib_manage.h"
#include "tdr/tdr_XMLtags.h"
#include "tdr/tdr_metalib_init.h"
#include "tdr_define_i.h"
#include "tdr_os.h"
#include "tdr_auxtools.h"
#include "tdr_metalib_manage_i.h"
#include "tdr_ctypes_info_i.h"

#if defined (_WIN32) || defined (_WIN64)
#pragma warning(disable:4996)
#endif

#define TDR_SCANF_INT_FROM_STR(a_lVal, a_pstLib, a_pstEntry, a_pszValue) \
{																	\
    LPTDRMACRO pstMacro = NULL;											\
    if (TDR_INVALID_PTR != (a_pstEntry)->ptrMacrosGroup)					\
    {																	\
        pstMacro = tdr_get_bindmacro_by_name(a_pstLib, a_pstEntry, a_pszValue);\
        if (NULL == pstMacro)													\
        {																		\
            LPTDRMACROSGROUP pstGroup;											\
            char szValueAddPrefix[TDR_NAME_LEN*2 + 2];							\
            char *pDst = &szValueAddPrefix[0];									\
            char *pSrc;															\
            pstGroup = TDR_PTR_TO_MACROSGROUP(a_pstLib, a_pstEntry->ptrMacrosGroup); \
            pSrc = &pstGroup->szName[0];										\
            while ('\0' != *pSrc)												\
            {																	\
                *pDst = (char)toupper(*pSrc);									\
                pDst++;															\
                pSrc++;															\
            }																	\
            *pDst++ = '_';														\
            TDR_STRNCPY(pDst, a_pszValue, (sizeof(szValueAddPrefix)- (pDst - &szValueAddPrefix[0]))); \
            pstMacro = tdr_get_bindmacro_by_name(a_pstLib, a_pstEntry, &szValueAddPrefix[0]);		\
        }/*if (NULL == pstMacro)*/												\
    }																		\
    if (NULL != pstMacro)													\
    {																		\
        a_lVal = pstMacro->iValue;											\
    }else																	\
    {																		\
        int iID = 0, iIndex;														\
        tdr_get_macro_int_i(&iID, &iIndex, a_pstLib, a_pszValue);				\
        a_lVal = iID;															\
    }																		\
}

static char *tdr_get_bindmacro_name_i(IN LPTDRMETALIB a_pstLib, IN LPTDRMETAENTRY a_pstEntry, IN int a_iValue);

/**
 *获取复合元数据类型各成员中最大id属性值
 *@param[in] a_pstMeta 元数据描述的指针
 *
 *@return 最大Id值
 *
 * @pre \e a_pstMeta 不能为 NULL
 */
int tdr_get_max_id_of_meta_entries(IN LPTDRMETA a_pstMeta);


/**
 *获取复合数据类型(mata)元数据描述所在的元数据库的指针
 *@param[in] a_pstMeta 元数据描述的指针
 *
 *@return 元数据库的指针
 *
 * @pre \e a_pstMeta 不能为 NULL
 */
LPTDRMETALIB tdr_get_metalib_of_meta(const LPTDRMETA a_pstMeta);

size_t tdr_copy_with_escape_i(char* a_pszOut, const char* a_pszInput, size_t a_tSize);

int tdr_get_meta_based_version(IN LPTDRMETA a_pstMeta)
{
    //assert(NULL != a_pstMeta);
    if (NULL == a_pstMeta)
    {
        return -1;
    }


    return a_pstMeta->iBaseVersion;
}

int tdr_get_meta_current_version(IN LPTDRMETA a_pstMeta)
{
    //assert(NULL != a_pstMeta);
    if (NULL == a_pstMeta)
    {
        return -1;
    }

    return a_pstMeta->iCurVersion;
}

int tdr_get_meta_type(IN LPTDRMETA a_pstMeta)
{
    //assert(NULL != a_pstMeta);
    if (NULL == a_pstMeta)
    {
        return TDR_TYPE_UNKOWN;
    }

    return a_pstMeta->iType;
}

LPTDRMETAENTRY tdr_get_entry_by_index(IN LPTDRMETA a_pstMeta, IN int a_idxEntry)
{
    assert(NULL != a_pstMeta);

    if ((0 > a_idxEntry) || (a_idxEntry >= a_pstMeta->iEntriesNum))
    {
        return NULL;
    }

    return a_pstMeta->stEntries + a_idxEntry;
}

LPTDRMETALIB tdr_get_metalib_of_meta(const LPTDRMETA a_pstMeta)
{
    assert(NULL != a_pstMeta);

    return TDR_META_TO_LIB(a_pstMeta);
}

int tdr_get_max_id_of_meta_entries(IN LPTDRMETA a_pstMeta)
{
    assert(NULL != a_pstMeta);

    return a_pstMeta->iMaxSubID;
}

int tdr_get_entry_num(IN LPTDRMETA a_pstMeta)
{
    //assert(NULL != a_pstMeta);
    if (NULL == a_pstMeta)
    {
        return -1;
    }

    return a_pstMeta->iEntriesNum;
}

const char *tdr_get_meta_name(IN LPTDRMETA a_pstMeta)
{
    //assert(NULL != a_pstMeta);
    LPTDRMETALIB pstLib ;
    if (NULL == a_pstMeta)
    {
        return "";
    }

    pstLib = TDR_META_TO_LIB(a_pstMeta);

    return TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName);
}

const char *tdr_get_metalib_name(IN LPTDRMETALIB a_pstLib)
{
    //assert(NULL != a_pstLib);
    if (NULL == a_pstLib)
    {
        return "";
    }

    if( (TDR_MAGIC != (a_pstLib)->wMagic) || (TDR_BUILD != (a_pstLib)->nBuild))
    {
        tdr_set_last_error(TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_BUILD_VERSION_CONFLICT));
        return "";
    }
    if (TDR_FLATFORM_MAGIC != (a_pstLib)->dwPlatformArch)
    {
        tdr_set_last_error(TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_BUILD_VERSION_CONFLICT));
        return "";
    }
    return a_pstLib->szName;
}

int tdr_get_metalib_version(IN LPTDRMETALIB a_pstLib)
{
    //assert(NULL != a_pstLib);
    if (NULL == a_pstLib)
    {
        return -1;
    }

    if( (TDR_MAGIC != (a_pstLib)->wMagic) || (TDR_BUILD != (a_pstLib)->nBuild))
    {
        tdr_set_last_error(TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_BUILD_VERSION_CONFLICT));
        return -1;
    }
    if (TDR_FLATFORM_MAGIC != (a_pstLib)->dwPlatformArch)
    {
        tdr_set_last_error(TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_BUILD_VERSION_CONFLICT));
        return -1;
    }


    return a_pstLib->iVersion;
}


int tdr_get_metalib_build_version(IN LPTDRMETALIB a_pstLib)
{
    //assert(NULL != a_pstLib);
    if (NULL == a_pstLib)
    {
        return -1;
    }

    return a_pstLib->nBuild;
}

size_t tdr_get_meta_size(IN LPTDRMETA a_pstMeta)
{
    size_t iSize;

    //assert(NULL != a_pstMeta);
    if (NULL == a_pstMeta)
    {
        return 0;
    }

    if (0 < a_pstMeta->iCustomHUnitSize)
    {
        iSize =  a_pstMeta->iCustomHUnitSize;
    }else
    {
        iSize = a_pstMeta->iHUnitSize;
    }

    return iSize;
}





int tdr_netoff_to_path_i(LPTDRMETA a_pstMeta, int a_iEntry, TDROFF a_iNetOff, char* a_pszBuff, int a_iBuff)
{
    int iRet = TDR_SUCCESS;
    TDROFF iCurOff = 0;
    LPTDRMETAENTRY pstEntry = NULL;
    LPTDRMETA pstSearchMeta = NULL;
    LPTDRMETALIB pstLib = NULL;
    int iTempLen = 0;
    char *pszBuff = NULL;
    int iLeftLen = 0;
    TDRBOOLEAN bGetFirstName = TDR_FALSE;
    int i = 0;
    char *pszName = NULL;
    TDROFF iSearchNOff;

    assert(NULL != a_pstMeta);
    assert(0 <= a_iNetOff);
    assert(NULL != a_pszBuff);
    assert(0 < a_iBuff);

    i = 0;
    pszBuff = a_pszBuff;
    iLeftLen = a_iBuff;
    pstLib = TDR_META_TO_LIB(a_pstMeta);
    pstSearchMeta = a_pstMeta;
    iSearchNOff = a_iNetOff;
    while (iCurOff < iSearchNOff )
    {
        if ( i >= pstSearchMeta->iEntriesNum )
        {   /*已经搜索完所有entry，但没有entry的偏移和指定的偏移值匹配*/
            iRet = TDR_ERRIMPLE_INVALID_OFFSET;
            break;
        }

        pstEntry = pstSearchMeta->stEntries + i;
        if( ((iCurOff + pstEntry->iNOff) > iSearchNOff) ||
           ((TDROFF)(iCurOff + pstEntry->iNOff + pstEntry->iNUnitSize) <= iSearchNOff) )
        {
            i++;
            continue;
        }

        /*发现entry的范围包含a_iOff*/
        if( i == a_iEntry )
        {
            pszName = TDR_TAG_THIS;
        }
        else
        {
            pszName = TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName);
        }
        a_iEntry = -1;  /*只有path第一层名字会出现this*/


        /*记录entry的名字*/
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
        {/*缓冲区溢出*/
            break;
        }


        iCurOff += pstEntry->iNOff;
        if (( TDR_INVALID_PTR == pstEntry->ptrMeta ) ||
            TDR_ENTRY_IS_POINTER_TYPE(pstEntry) || TDR_ENTRY_IS_REFER_TYPE(pstEntry))
        {
            pstSearchMeta = NULL;
            break;
        }else
        {
            pstSearchMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
            i =	0;
        }
    }

    /*找不到指定偏移值的entry，则直接返回*/
    if( iCurOff != iSearchNOff )
    {
        if (!TDR_ERR_IS_ERROR(iRet))
        {
            iRet = TDR_ERRIMPLE_INVALID_OFFSET;
        }
        return iRet;
    }


    /*如果找到指定偏移值的entry，且此entry为复合meta则
     *继续向下递归获取entry名字，直至到简单类型的entry*/
    while( NULL != pstSearchMeta )
    {
        pstEntry = pstSearchMeta->stEntries;  /*取第一个元素*/
        if ( 0 == a_iEntry )
        {
            pszName = TDR_TAG_THIS;
        }
        else
        {
            pszName = TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName);;
        }
        a_iEntry = -1;  /*只有path第一层名字会出现this*/


        /*记录entry的名字*/
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
        {/*缓冲区溢出*/
            break;
        }


        if (( TDR_INVALID_PTR == pstEntry->ptrMeta ) ||
            TDR_ENTRY_IS_POINTER_TYPE(pstEntry) || TDR_ENTRY_IS_REFER_TYPE(pstEntry))
        {
            pstSearchMeta = NULL;
            break;
        }else
        {
            pstSearchMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
        }
    }/*while( NULL != pstSearchMeta )*/


    return iRet;
}


int tdr_hostoff_to_path_i(LPTDRMETA a_pstMeta, int a_iEntry, TDROFF a_iHostOff, char* a_pszBuff, int a_iBuff)
{
    int iRet = TDR_SUCCESS;
    TDROFF iCurOff = 0;
    LPTDRMETAENTRY pstEntry = NULL;
    LPTDRMETA pstSearchMeta = NULL;
    LPTDRMETALIB pstLib = NULL;
    int iTempLen = 0;
    char *pszBuff = NULL;
    int iLeftLen = 0;
    TDRBOOLEAN bGetFirstName = TDR_FALSE;
    int i = 0;
    char *pszName = NULL;
    TDROFF iSearchHOff;

    assert(NULL != a_pstMeta);
    assert(0 <= a_iHostOff);
    assert(NULL != a_pszBuff);
    assert(0 < a_iBuff);

    i = 0;
    pszBuff = a_pszBuff;
    iLeftLen = a_iBuff;
    pstLib = TDR_META_TO_LIB(a_pstMeta);
    pstSearchMeta = a_pstMeta;
    iSearchHOff = a_iHostOff;
    while (iCurOff < iSearchHOff )
    {
        if ( i >= pstSearchMeta->iEntriesNum )
        {   /*已经搜索完所有entry，但没有entry的偏移和指定的偏移值匹配*/
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

        /*发现entry的范围包含a_iOff*/
        if( i == a_iEntry )
        {
            pszName = TDR_TAG_THIS;
        }
        else
        {
            pszName = TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName);;
        }
        a_iEntry = -1;  /*只有path第一层名字会出现this*/


        /*记录entry的名字*/
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
        {/*缓冲区溢出*/
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
            pstSearchMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
            i =	0;
        }
    }

    /*找不到指定偏移值的entry，则直接返回*/
    if( iCurOff != iSearchHOff )
    {
        if (!TDR_ERR_IS_ERROR(iRet))
        {
            iRet = TDR_ERRIMPLE_INVALID_OFFSET;
        }
        return iRet;
    }


    /*如果找到指定偏移值的entry，且此entry为复合meta则
     *继续向下递归获取entry名字，直至到简单类型的entry*/
    while( NULL != pstSearchMeta )
    {
        pstEntry = pstSearchMeta->stEntries;  /*取第一个元素*/
        if ( 0 == a_iEntry )
        {
            pszName = TDR_TAG_THIS;
        }
        else
        {
            pszName = TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName);;
        }
        a_iEntry = -1;  /*只有path第一层名字会出现this*/


        /*记录entry的名字*/
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
        {/*缓冲区溢出*/
            break;
        }

        if (( TDR_INVALID_PTR == pstEntry->ptrMeta ) ||
            TDR_ENTRY_IS_POINTER_TYPE(pstEntry) || TDR_ENTRY_IS_REFER_TYPE(pstEntry))
        {
            pstSearchMeta = NULL;
            break;
        }else
        {
            pstSearchMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
        }
    }/*while( NULL != pstSearchMeta )*/


    return iRet;
}



char *tdr_get_first_node_name_from_path_i(char *pszName, int iNameSize, const char *pszPath)
{
    char *pszDot = NULL;
    ptrdiff_t iLen = 0;

    assert(NULL != pszName);
    assert(0 < iNameSize);
    assert(NULL != pszPath);

    pszName[0] = '\0';
    pszDot = strchr(pszPath, TDR_TAG_DOT);
    if (NULL != pszDot)
    {
        iLen = pszDot - pszPath;
    }else
    {
        iLen = strlen(pszPath);
    }
    if ( iLen >= iNameSize )
    {/*名字太长*/
        return NULL;
    }
    TDR_STRNCPY(pszName, pszPath, iLen + 1);

    if (NULL != pszDot)
    {
        pszDot++;
    }

    return pszDot;
}

int tdr_sizeinfo_to_off(LPTDRSIZEINFO a_pstRedirector, LPTDRMETA a_pstMeta, int a_iEntry, const char* a_pszName)
{
    LPTDRMETALIB pstLib;
    TDRMETAENTRY* pstEntry = NULL;
    LPTDRMETA pstSearchMeta;
    int iRet = TDR_SUCCESS;

    char szBuff[TDR_NAME_LEN];
    const char* pszPtr;
    int idxThisEntry;
    int idx;
    TDROFF iNOff;
    TDROFF iHOff;
    size_t iUnit;
    int i;
    int iIdxType;

    if ( NULL == a_pstMeta || NULL == a_pstRedirector || NULL == a_pszName)
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
	}


    /*检查是不是通过简单数据类型来打包*/
    iIdxType = tdr_typename_to_idx(a_pszName);
    if (TDR_INVALID_INDEX != iIdxType)
    {
        /*基本数据类型*/
        LPTDRCTYPEINFO pstTypeInfo = tdr_idx_to_typeinfo(iIdxType);

        assert(NULL != pstTypeInfo);
        if ((TDR_TYPE_CHAR <= pstTypeInfo->iType) && (TDR_TYPE_UINT >= pstTypeInfo->iType))
        {
            a_pstRedirector->idxSizeType = iIdxType;
            a_pstRedirector->iUnitSize = pstTypeInfo->iSize;
        }else
        {
            iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_SIZETYPE_VALUE);
        }/*if ((TDR_TYPE_CHAR <= pstTypeInfo->iType) && (TDR_TYPE_ULONG >= pstTypeInfo->iType))*/

        return iRet;
    }


    /*sizeinfo/sizetypy为path的情况, 如果是meta的sizeinfo属性,先跳过'this'*/
    pstLib = TDR_META_TO_LIB(a_pstMeta);
    pszPtr = a_pszName;
    if (TDR_INVALID_INDEX == a_iEntry)
    {
        pszPtr = tdr_get_first_node_name_from_path_i(szBuff, sizeof(szBuff), pszPtr);
        if( 0 != tdr_stricmp(szBuff, TDR_TAG_THIS) )
        {
            pszPtr = a_pszName;
        }
    }

    iNOff = 0;
    iHOff = 0;
    iUnit = 0;
    idxThisEntry = a_iEntry;
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
        {/*空串，不合法*/
            iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_SIZETYPE_VALUE);
            break;
        }
        tdr_trim_str(szBuff);

        if( 0==tdr_stricmp(szBuff, TDR_TAG_THIS) )
        {
            idx	= idxThisEntry;
        }else
        {
            idx	= tdr_get_entry_by_name_i(pstSearchMeta->stEntries, pstSearchMeta->iEntriesNum, szBuff);
        }
        if( TDR_INVALID_INDEX == idx )
        {
            iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_SIZETYPE_VALUE);
            break;
        }

        for(i=0; i < idx; i++)
        {
            pstEntry =	pstSearchMeta->stEntries  + i;
            if (!TDR_ENTRY_IS_FIXSIZE(pstEntry))
            {
                iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_VARIABLE_BEFOR_SIZEINFO);
                break;
            }
        }


        idxThisEntry = -1;
        pstEntry = pstSearchMeta->stEntries + idx;
        if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry) || TDR_ENTRY_IS_REFER_TYPE(pstEntry))
        {
            iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_SIZETYPE_VALUE);
            break;
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


    a_pstRedirector->iHOff	=	iHOff;
    a_pstRedirector->iNOff = iNOff;
    a_pstRedirector->iUnitSize	=	iUnit;

    return TDR_SUCCESS;
}







LPTDRMETA tdr_get_meta_by_name_i(LPTDRMETALIB pstLib, const char* pszName)
{
    int i;
    LPTDRMAPENTRY pstMap;
    LPTDRMETA pstMeta;

    assert(NULL != pstLib);
    assert(NULL != pszName);

    pstMap = TDR_GET_MAP_TABLE(pstLib);

    for (i=0; i < pstLib->iCurMetaNum; i++)
    {
        pstMeta	= (LPTDRMETA) (pstLib->data + pstMap[i].iPtr);

        if ( 0 == strcmp(TDR_GET_STRING_BY_PTR(pstLib,pstMeta->ptrName), pszName))
        {
            return pstMeta;
        }
    }

    return NULL;
}


int tdr_get_macro_int_i(int* a_piID, int* a_piIdx, LPTDRMETALIB a_pstLib, const  char *a_pszValue)
{
    assert( NULL != a_pstLib);
    assert(NULL != a_pszValue);
    assert(NULL != a_piID);
    assert(NULL != a_piIdx );

    if( ( a_pszValue[0] >= '0' && a_pszValue[0] <= '9' ) ||
       ('+' == a_pszValue[0]) || ('-' == a_pszValue[0]) )
    {
        *a_piIdx = -1;
        *a_piID = (int)strtol(a_pszValue, NULL, 0);;
    }
    else
    {
        TDRMACRO* pstMacro;
        int iIdx = TDR_INVALID_INDEX;

        iIdx = tdr_get_macro_index_by_name_i(a_pstLib, a_pszValue);
        if (TDR_INVALID_INDEX == iIdx)
        {

            return TDR_ERRIMPLE_UNDEFINED_MACRO_NAME;
        }else
        {
            pstMacro = TDR_GET_MACRO_TABLE(a_pstLib);
            *a_piIdx = iIdx;
            *a_piID = pstMacro[iIdx].iValue;
        }
    }

    return TDR_SUCCESS;
}

int tdr_get_macro_index_by_name_i(LPTDRMETALIB pstLib, const char* pszName)
{
    LPTDRMACRO pstMacro;
    int i;


    assert(NULL != pstLib);
    assert(NULL != pszName);

    pstMacro = TDR_GET_MACRO_TABLE(pstLib);


    for (i=0; i< pstLib->iCurMacroNum; i++)
    {
        if (0 == strcmp(TDR_GET_STRING_BY_PTR(pstLib,pstMacro[i].ptrMacro), pszName))
        {
            return i;
        }
    }

    return TDR_INVALID_INDEX;
}


int tdr_get_entry_by_name_i(TDRMETAENTRY pstEntry[], int iMax, const char* pszName)
{
    int i;
    LPTDRMETA pstMeta;
    LPTDRMETALIB pstLib;

    assert(NULL != pstEntry );

    if (0 >= iMax)
    {
        return TDR_INVALID_INDEX;
    }

    pstMeta = TDR_ENTRY_TO_META(pstEntry);
    pstLib = TDR_META_TO_LIB(pstMeta);
    for (i=0; i<iMax; i++)
    {
        if ( 0 == strcmp(TDR_GET_STRING_BY_PTR(pstLib,pstEntry[i].ptrName), pszName) )
        {
            break;
        }
    }

    return (i < iMax) ? i : TDR_INVALID_INDEX;
}

int tdr_get_index_by_name_i(LPTDRMETAINDEX pstIndex, int iMax, const char* pszName)
{
    int i;
    LPTDRMETA pstMeta;
    LPTDRMETALIB pstLib;

    assert(NULL != pstIndex );

    if (0 >= iMax)
    {
        return TDR_INVALID_INDEX;
    }

    pstMeta = TDR_ENTRY_TO_META(pstIndex);
    pstLib = TDR_META_TO_LIB(pstMeta);
    for (i=0; i<iMax; i++)
    {
        if ( 0 == strcmp(TDR_GET_STRING_BY_PTR(pstLib, pstIndex->ptrName), pszName) )
        {
            break;
        }
        pstIndex++;
    }

    return (i < iMax) ? i : TDR_INVALID_INDEX;
}

LPTDRMETA  tdr_get_meta_by_name(IN LPTDRMETALIB a_pstLib, IN const char* a_pszName)
{
    LPTDRNAMEENTRY pstName = NULL;
    int iRet;
    int iMin;
    int iMax;
    int i;
    LPTDRMETA pstMeta = NULL;

    //assert(NULL != a_pstLib);
    //assert(NULL != a_pszName);
    if ((NULL == a_pstLib)||(NULL == a_pszName))
    {
        return NULL;
    }

    if( (TDR_MAGIC != (a_pstLib)->wMagic) || (TDR_BUILD != (a_pstLib)->nBuild))
    {
        tdr_set_last_error(TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_BUILD_VERSION_CONFLICT));
        return NULL;
    }
    if (TDR_FLATFORM_MAGIC != (a_pstLib)->dwPlatformArch)
    {
        tdr_set_last_error(TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_BUILD_VERSION_CONFLICT));
        return NULL;
    }
    pstName	= TDR_GET_META_NAME_MAP_TABLE(a_pstLib);
    iMin = 0;
    iMax = a_pstLib->iCurMetaNum -1;

    while(iMin <= iMax)
    {
        i = (iMin+iMax)>>1;

        iRet = strcmp(a_pszName, TDR_GET_STRING_BY_PTR(a_pstLib,pstName[i].ptrName));
        if( iRet>0 )
        {
            iMin    =       i + 1;
        }
        else if( iRet<0 )
        {
            iMax    =       i - 1;
        }
        else
        {
            pstMeta =  TDR_PTR_TO_META(a_pstLib, pstName[i].ptrMeta);
            break;
        }
    }

    return pstMeta;
}



LPTDRMETA tdr_get_meta_by_id(IN LPTDRMETALIB a_pstLib, IN int a_iID)
{
    LPTDRIDENTRY pstIDMapping = NULL;
    int iRet;
    int iMin;
    int iMax;
    int i;
    LPTDRMETA pstMeta = NULL;

    //assert(NULL != a_pstLib);
    if (NULL == a_pstLib)
    {
        return NULL;
    }

    if (TDR_INVALID_ID == a_iID)
    {
        return NULL;
    }
    if( (TDR_MAGIC != (a_pstLib)->wMagic) || (TDR_BUILD != (a_pstLib)->nBuild))
    {
        tdr_set_last_error(TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_BUILD_VERSION_CONFLICT));
        return NULL;
    }
    if (TDR_FLATFORM_MAGIC != (a_pstLib)->dwPlatformArch)
    {
        tdr_set_last_error(TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_BUILD_VERSION_CONFLICT));
        return NULL;
    }

    pstIDMapping = TDR_GET_META_ID_MAP_TABLE(a_pstLib);
    iMin = 0;
    iMax = a_pstLib->iCurMetaNum -1;

    while(iMin <= iMax)
    {
        i = (iMin+iMax)>>1;


        iRet = a_iID - pstIDMapping[i].iID;
        if( iRet>0 )
        {
            iMin    =       i + 1;
        }
        else if( iRet<0 )
        {
            iMax    =       i - 1;
        }
        else
        {
            pstMeta =  TDR_PTR_TO_META(a_pstLib, pstIDMapping[i].ptrMeta);
            break;
        }
    }

    return pstMeta;
}

const char *tdr_get_meta_desc(IN LPTDRMETA a_pstMeta)
{
    LPTDRMETALIB pstLib = NULL;

    if (NULL == a_pstMeta || TDR_INVALID_PTR == a_pstMeta->ptrDesc)
    {
        return "";
    }

    pstLib = TDR_META_TO_LIB(a_pstMeta);

    return TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrDesc);
}

int tdr_get_entry_by_id(OUT int* a_piIdx, IN LPTDRMETA a_pstMeta, IN int a_iId)
{
    int ids = TDR_INVALID_INDEX;


    //assert(NULL != a_piIdx);
    //assert(NULL != a_pstMeta);
    if ((NULL == a_pstMeta)||(NULL == a_piIdx))
    {
        return TDR_ERRIMPLE_INVALID_PARAM;
    }

    if (TDR_TYPE_UNION != a_pstMeta->iType)
    {
        int i;
        for (i = 0; i < a_pstMeta->iEntriesNum; i++)
        {
            if (TDR_ENTRY_DO_HAVE_ID(&a_pstMeta->stEntries[i]) && (a_iId == a_pstMeta->stEntries[i].iID))
            {
                *a_piIdx = i;
                return TDR_SUCCESS;
            }
        }

        *a_piIdx = TDR_INVALID_INDEX;
        return TDR_ERR_ERROR;
    }

    /*union 结构*/
    TDR_GET_ENTRY(ids, a_pstMeta->stEntries, a_pstMeta->iEntriesNum, a_iId);


    /*如果没有找到，则找缺省的成员*/
    *a_piIdx = ids;
    if (0 <= ids)
    {
        return TDR_SUCCESS;
    }

    return TDR_ERR_ERROR;
}

int tdr_get_entry_by_name(OUT int* a_piIdx, IN LPTDRMETA a_pstMeta, IN const char* a_pszName)
{
    int i;
    LPTDRMETALIB pstLib;

    if ((NULL == a_piIdx)||(NULL == a_pstMeta)||(NULL == a_pszName)||(0 >= a_pstMeta->iEntriesNum))
    {
        return	TDR_ERRIMPLE_INVALID_PARAM;
    }

    pstLib = TDR_META_TO_LIB(a_pstMeta);
    for (i = 0; i < a_pstMeta->iEntriesNum; i++)
    {
        if (0 == strcmp(a_pszName, TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->stEntries[i].ptrName)))
        {
            break;
        }
    }

    if (i >= a_pstMeta->iEntriesNum)
    {
        *a_piIdx = TDR_INVALID_INDEX;
        return TDR_ERR_ERROR;
    }

    *a_piIdx = i;

    return TDR_SUCCESS;
}

LPTDRMETAENTRY tdr_get_entryptr_by_name(IN LPTDRMETA a_pstMeta, IN const char* a_pszName)
{
    int i;
    LPTDRMETALIB pstLib;

    //assert(NULL != a_pstMeta);
    //assert(NULL != a_pszName);
    if ((NULL == a_pstMeta)||(NULL == a_pszName))
    {
        return NULL;
    }

    pstLib = TDR_META_TO_LIB(a_pstMeta);
    for (i = 0; i < a_pstMeta->iEntriesNum; i++)
    {
        if (0 == strcmp(a_pszName, TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->stEntries[i].ptrName)))
        {
            break;
        }
    }

    if (i >= a_pstMeta->iEntriesNum)
    {
        return NULL;
    }


    return &a_pstMeta->stEntries[i];
}



int tdr_get_entry_id(IN LPTDRMETAENTRY a_pstEntry)
{
    int iID;

    assert(NULL != a_pstEntry);


    if (TDR_ENTRY_DO_HAVE_ID(a_pstEntry))
    {
        iID =  a_pstEntry->iID;
    }else
    {
        iID = TDR_INVALID_ID;
    }

    return iID;
}



LPTDRMETA tdr_get_entry_type_meta(IN LPTDRMETALIB a_pstLib, IN LPTDRMETAENTRY a_pstEntry)
{
    //assert(NULL != a_pstEntry);
    //assert(NULL != a_pstLib);
    if ((NULL == a_pstLib)||(NULL == a_pstEntry))
    {
        return NULL;
    }

    if (TDR_INVALID_PTR == a_pstEntry->ptrMeta)
    {
        return NULL;
    }
    if( (TDR_MAGIC != (a_pstLib)->wMagic) || (TDR_BUILD != (a_pstLib)->nBuild))
    {
        tdr_set_last_error(TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_BUILD_VERSION_CONFLICT));
        return NULL;
    }
    if (TDR_FLATFORM_MAGIC != (a_pstLib)->dwPlatformArch)
    {
        tdr_set_last_error(TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_BUILD_VERSION_CONFLICT));
        return NULL;
    }

    return (LPTDRMETA)(TDR_PTR_TO_META(a_pstLib, a_pstEntry->ptrMeta));
}

int tdr_parse_meta_sortkey_info_i(IN LPTDRSORTKEYINFO a_pstSortKey, LPTDRMETA a_pstMeta, const char *a_pszSortKeyPath)
{
    LPTDRMETA pstSearchMeta;
    int idxEntry;
    TDROFF iHOff;
    const char *pszPtr;
    char szBuff[TDR_NAME_LEN] = {0};
    LPTDRMETAENTRY pstSearchEntry;
    int iRet = TDR_SUCCESS;
    LPTDRMETALIB pstLib;

    assert(NULL != a_pstSortKey);
    assert(NULL != a_pstMeta);
    assert(NULL != a_pszSortKeyPath);

    pstLib = TDR_META_TO_LIB(a_pstMeta);
    pstSearchMeta = a_pstMeta;      /*从当前entry开始查找*/
    idxEntry = TDR_INVALID_INDEX;
    iHOff = 0;
    pszPtr = a_pszSortKeyPath;
    while( NULL != pszPtr )
    {
        if (NULL == pstSearchMeta)
        {
            break;
        }

        pszPtr = tdr_get_first_node_name_from_path_i(szBuff, sizeof(szBuff), pszPtr);
        tdr_trim_str(szBuff);

        /*查找名字相同的entry*/
        pstSearchEntry = NULL;
        idxEntry = tdr_get_entry_by_name_i(pstSearchMeta->stEntries, pstSearchMeta->iEntriesNum, szBuff);
        if( TDR_INVALID_INDEX == idxEntry )
        {
            break;
        }

        /*找到名字相同的entry*/
        pstSearchEntry = pstSearchMeta->stEntries + idxEntry;
        a_pstSortKey->ptrSortKeyMeta = pstSearchMeta->ptrMeta;
        if (TDR_ENTRY_IS_POINTER_TYPE(pstSearchEntry))
        {
            idxEntry = TDR_INVALID_INDEX;
            break;
        }
        if (TDR_ENTRY_IS_REFER_TYPE(pstSearchEntry))
        {
            idxEntry = TDR_INVALID_INDEX;
            break;
        }
        iHOff +=	pstSearchEntry->iHOff; /*累加偏移*/

        if (TDR_INVALID_PTR != pstSearchEntry->ptrMeta)
        {
            pstSearchMeta = TDR_PTR_TO_META(pstLib, pstSearchEntry->ptrMeta);
        }else
        {
            pstSearchMeta = NULL;
        }
    }


    if ((NULL != pszPtr) || (TDR_INVALID_INDEX == idxEntry ))
    {
        iRet = TDR_ERRIMPLE_ENTRY_IVALID_SORTKEY_VALUE;
    }else
    {
        a_pstSortKey->iSortKeyOff = iHOff;
        a_pstSortKey->idxSortEntry = idxEntry;
    }

    return iRet;
}

/**获取单个entry成员的本地存储空间大小
 *@param[in] a_pstEntry entry元素描述结构的指针
 *
 *@note 通过调用tdr_get_entry_by_index可以获取entry元素描述结构的指针
 *
 *@return entry元素单个变量的本地存储空间大小
 *
 *@pre \e a_pstEntry 不能为NULL
 *@see tdr_get_entry_by_index
 */
size_t tdr_get_entry_unitsize(IN LPTDRMETAENTRY a_pstEntry)
{
    size_t iSize;

    //assert(NULL != a_pstEntry);
    if (NULL == a_pstEntry)
    {
        return 0;
    }

    if (0 < a_pstEntry->iCustomHUnitSize)
    {
        iSize = a_pstEntry->iCustomHUnitSize;
    }else
    {
        iSize = a_pstEntry->iHUnitSize;
    }

    return iSize;
}

int tdr_get_macro_value(OUT int *a_piID, IN LPTDRMETALIB a_pstLib, IN const  char *a_pszName)
{
    LPTDRMACRO pstMacro;
    int i;

    //assert(NULL != a_piID);
    //assert(NULL != a_pstLib);
    //assert(NULL != a_pszName);
    if ((NULL == a_piID)||(NULL == a_pstLib)||(NULL == a_pszName))
    {
        return TDR_ERRIMPLE_INVALID_PARAM;
    }

    pstMacro = TDR_GET_MACRO_TABLE(a_pstLib);
    for (i=0; i< a_pstLib->iCurMacroNum; i++)
    {
        if (0 == strcmp(TDR_GET_STRING_BY_PTR(a_pstLib, pstMacro[i].ptrMacro), a_pszName))
        {
            *a_piID = pstMacro[i].iValue;
            return TDR_SUCCESS;
        }
    }

    return TDR_ERRIMPLE_UNDEFINED_MACRO_NAME;
}

LPTDRMACRO tdr_get_macro_by_name(IN LPTDRMETALIB a_pstLib, IN const  char *a_pszName)
{
    LPTDRMACRO pstMacro;
    int i;

    //assert(NULL != a_pstLib);
    //assert(NULL != a_pszName);
    if ((NULL == a_pstLib)||(NULL == a_pszName))
    {
        return NULL;
    }

    pstMacro = TDR_GET_MACRO_TABLE(a_pstLib);
    for (i=0; i< a_pstLib->iCurMacroNum; i++)
    {
        if (0 == strcmp(TDR_GET_STRING_BY_PTR(a_pstLib, pstMacro[i].ptrMacro), a_pszName))
        {
            return pstMacro + i;
        }
    }

    return NULL;
}

char *tdr_entry_off_to_path(IN LPTDRMETA a_pstMeta, IN int a_iOff)
{
    int iRet = TDR_SUCCESS;
    static char szPath[(TDR_NAME_LEN+1)*TDR_STACK_SIZE] = {0};
    char *pszPath;

    /*assert(NULL != a_pstMeta);
      assert(0 <= a_iOff);*/
    if ((NULL == a_pstMeta)||(0 > a_iOff))
    {
        return "";
    }

    iRet = tdr_hostoff_to_path_i(a_pstMeta, TDR_INVALID_INDEX, a_iOff, &szPath[0], sizeof(szPath));

    if (TDR_ERR_IS_ERROR(iRet))
    {
        pszPath= &szPath[0];
    }else
    {
        pszPath = "";
    }
    return pszPath;
}

int tdr_get_entry_type(IN LPTDRMETAENTRY a_pstEntry)
{
    //assert(NULL != a_pstEntry);
    if (NULL == a_pstEntry)
    {
        return TDR_TYPE_UNKOWN;
    }

    return a_pstEntry->iType;
}

int tdr_get_entry_count(IN LPTDRMETAENTRY a_pstEntry)
{
    //assert(NULL != a_pstEntry);
    if (NULL == a_pstEntry)
    {
        return 1;
    }

    return a_pstEntry->iCount;
}
int tdr_name_to_off_i(IN LPTDRMETA a_pstMeta, OUT LPTDRSelector a_pstSelector, IN int a_iEntry, IN const char* a_pszName)
{
    LPTDRMETALIB pstLib;
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


    pstLib = TDR_META_TO_LIB(a_pstMeta);


    /*meta属性跳过this*/
    pszPtr = a_pszName;
    if (TDR_INVALID_INDEX == a_iEntry)
    {
        pszPtr = tdr_get_first_node_name_from_path_i(szBuff, sizeof(szBuff), pszPtr);
        if( 0 != tdr_stricmp(szBuff, TDR_TAG_THIS))
        {
            pszPtr = a_pszName;
        }
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
        {/*空串，不合法*/
            break;
        }
        tdr_trim_str(szBuff);


        /*查找名字相同的entry*/
        pstEntry = NULL;
        if( 0 == tdr_stricmp(szBuff, TDR_TAG_THIS))
        {
            iIdx = a_iEntry;
        }else
        {
            iIdx = tdr_get_entry_by_name_i(pstSearchMeta->stEntries, pstSearchMeta->iEntriesNum, szBuff);
        }
        if( TDR_INVALID_INDEX == iIdx )
        {
            break;
        }

        a_iEntry	= -1;  /*this只能出现在path的最前面*/
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

LPTDRMETAENTRY tdr_get_entry_by_path(IN LPTDRMETA a_pstMeta, IN const char* a_pszEntryPath)
{
    LPTDRMETALIB pstLib;
    TDRMETAENTRY* pstEntry = NULL;
    LPTDRMETA pstSearchMeta;
    char szBuff[TDR_NAME_LEN];

    const char* pszPtr = NULL;


    //assert( NULL != a_pstMeta);
    //assert(NULL != a_pszEntryPath);
    if ((NULL == a_pstMeta)||(NULL == a_pszEntryPath))
    {
        return NULL;
    }


    pstLib = TDR_META_TO_LIB(a_pstMeta);


    /*meta属性跳过this*/
    pszPtr = a_pszEntryPath;
    pszPtr = tdr_get_first_node_name_from_path_i(szBuff, sizeof(szBuff), pszPtr);
    if( 0 != tdr_stricmp(szBuff, TDR_TAG_THIS))
    {
        pszPtr = a_pszEntryPath;
    }else if (NULL == pszPtr)
    {
        return NULL;
    }



    pstSearchMeta = a_pstMeta;
    do
    {
        pstEntry = NULL;
        if( NULL == pstSearchMeta )
        {
            break;
        }


        pszPtr = tdr_get_first_node_name_from_path_i(szBuff, sizeof(szBuff), pszPtr);
        tdr_trim_str(szBuff);
        if ('\0' == szBuff[0])
        {/*空串，不合法*/
            break;
        }


        /*查找名字相同的entry*/
        pstEntry = tdr_get_entryptr_by_name(pstSearchMeta, szBuff);
        if(NULL  == pstEntry )
        {
            break;
        }

        if( TDR_INVALID_PTR != pstEntry->ptrMeta )
        {
            pstSearchMeta	=	TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
        }
        else
        {
            pstSearchMeta	=	NULL;
        }
    }while( NULL != pszPtr );




    return pstEntry;
}


int tdr_entry_path_to_off(IN LPTDRMETA a_pstMeta, INOUT LPTDRMETAENTRY *a_ppstEntry, OUT TDROFF *a_piHOff, IN const char *a_pszPath)
{
    int iRet = TDR_SUCCESS;
    TDRSelector stSelector;
    LPTDRMETALIB pstLib;

    /*assert(NULL != a_pstMeta);
      assert(NULL != a_ppstEntry);
      assert(NULL != a_piHOff);
      assert(NULL != a_pszPath);*/
    if ((NULL == a_pstMeta)||(NULL == a_ppstEntry)||(NULL == a_piHOff)||(NULL == a_pszPath))
    {
        return TDR_ERRIMPLE_INVALID_PARAM;
    }

    iRet = tdr_name_to_off_i(a_pstMeta, &stSelector, TDR_INVALID_INDEX, a_pszPath);
    if (TDR_ERR_IS_ERROR(iRet))
    {
        return iRet;
    }

    pstLib = TDR_META_TO_LIB(a_pstMeta);
    *a_ppstEntry = TDR_PTR_TO_ENTRY(pstLib, stSelector.ptrEntry);
    *a_piHOff = stSelector.iHOff;

    return iRet;
}


/*根据成员的值获取此值绑定的宏定义，只有当成员定义了macrosgroup属性时才有效*/
LPTDRMACRO tdr_get_bindmacro_by_value(IN LPTDRMETALIB a_pstLib, IN LPTDRMETAENTRY a_pstEntry, IN int a_iValue)
{
    LPTDRMACRO pstMacro = NULL;
    LPTDRMACRO pstMacroTable = NULL;
    LPTDRMACROSGROUP pstGroup;
    TDRIDX *pValueIdxTab;
    int iMax,iMin,iMid;
    int iDiff;

    assert(NULL != a_pstLib);
    assert(NULL != a_pstEntry);


    if (TDR_INVALID_PTR == a_pstEntry->ptrMacrosGroup)
    {
        return NULL;
    }

    pstMacroTable = TDR_GET_MACRO_TABLE(a_pstLib);
    pstGroup = TDR_PTR_TO_MACROSGROUP(a_pstLib, a_pstEntry->ptrMacrosGroup);
    pValueIdxTab= TDR_GET_MACROSGROUP_VALUEIDXMAP_TAB(pstGroup);
    iMin = 0;
    iMax = pstGroup->iCurMacroCount -1;
    while (iMin <= iMax)
    {
        iMid = (iMin + iMax) / 2;
        pstMacro = pstMacroTable + pValueIdxTab[iMid];
        iDiff = pstMacro->iValue - a_iValue;
        if (iDiff < 0)
        {
            iMin = iMid + 1;
        }else if (iDiff > 0)
        {
            iMax = iMid - 1;
        }else
        {
            break;
        }
    }/*while (iMin <= iMax)*/

    if (iMin > iMax)
    {
        return NULL;
    }

    return pstMacro;
}

/*根据指定名字获取此值绑定的宏定义，只有当成员定义了macrosgroup属性时才有效*/
LPTDRMACRO tdr_get_bindmacro_by_name(IN LPTDRMETALIB a_pstLib, IN LPTDRMETAENTRY a_pstEntry, IN const char *a_pszName)
{
    LPTDRMACRO pstMacro = NULL;
    LPTDRMACRO pstMacroTable = NULL;
    LPTDRMACROSGROUP pstGroup;
    TDRIDX *pNameIdxTab;
    int iMax,iMin,iMid;
    int iDiff;

    assert(NULL != a_pstLib);
    assert(NULL != a_pstEntry);
    assert(NULL != a_pszName);


    if (TDR_INVALID_PTR == a_pstEntry->ptrMacrosGroup)
    {
        return NULL;
    }

    pstMacroTable = TDR_GET_MACRO_TABLE(a_pstLib);
    pstGroup = TDR_PTR_TO_MACROSGROUP(a_pstLib, a_pstEntry->ptrMacrosGroup);
    pNameIdxTab= TDR_GET_MACROSGROUP_NAMEIDXMAP_TAB(pstGroup);
    iMin = 0;
    iMax = pstGroup->iCurMacroCount -1;
    while (iMin <= iMax)
    {
        iMid = (iMin + iMax) / 2;
        pstMacro = pstMacroTable + pNameIdxTab[iMid];
        iDiff = strcmp(TDR_GET_STRING_BY_PTR(a_pstLib,pstMacro->ptrMacro), a_pszName);
        if (iDiff < 0)
        {
            iMin = iMid + 1;
        }else if (iDiff > 0)
        {
            iMax = iMid - 1;
        }else
        {
            break;
        }
    }/*while (iMin <= iMax)*/

    if (iMin > iMax)
    {
        return NULL;
    }

    return pstMacro;
}


/**输出基本数据类型成员的值
 *@param[in] a_pstIOStream 输出信息的流句柄
 *@param[in] a_pstLib 元数据描述库指针
 *@param[in] a_pstEntry 成员的描述结构指针
 *@param[in,out] a_ppszHostStart 此成员值存储空间的起始地址
 *@param[in] a_pszHostEnd 此成员值存储空间的结束地址
 *@return 成功返回0，否则返回非零值
 */
int tdr_ioprintf_basedtype_i(IN LPTDRIOSTREAM a_pstIOStream, IN LPTDRMETALIB a_pstLib, IN LPTDRMETAENTRY a_pstEntry,
                             INOUT char **a_ppszHostStart, IN const char *a_pszHostEnd)
{
    int iRet = TDR_SUCCESS;
    size_t iSize;
    size_t iLen;
    char *pszHostStart;
    char *pszVal = NULL;

    assert(NULL != a_pstIOStream);
    assert(NULL != a_pstLib);
    assert(NULL != a_pstEntry);
    assert(NULL != a_ppszHostStart);
    assert(NULL != *a_ppszHostStart);
    assert(NULL != a_pszHostEnd);
    assert(TDR_TYPE_COMPOSITE < a_pstEntry->iType);

    pszHostStart = *a_ppszHostStart;

    switch(a_pstEntry->iType)
    {
        case TDR_TYPE_CHAR:
            {
                pszVal = tdr_get_bindmacro_name_i(a_pstLib, a_pstEntry, (int)(char)pszHostStart[0]);
                if(NULL != pszVal)
                {
                    iRet = tdr_iostream_write(a_pstIOStream, "%s", pszVal);
                }else
                {
                    iRet = tdr_iostream_write(a_pstIOStream, "%d", (int)(char)pszHostStart[0]);
                }/*if(NULL != pstMacro)*/
                pszHostStart += a_pstEntry->iHUnitSize;
            }
            break;
        case TDR_TYPE_UCHAR:
        case TDR_TYPE_BYTE:
            {
                pszVal = tdr_get_bindmacro_name_i(a_pstLib, a_pstEntry, (int)(unsigned char)pszHostStart[0]);
                if(NULL != pszVal)
                {
                    iRet = tdr_iostream_write(a_pstIOStream, "%s", pszVal);
                }else
                {
                    iRet = tdr_iostream_write(a_pstIOStream, "0x%x", (int)(unsigned char)pszHostStart[0]);
                }
                pszHostStart += a_pstEntry->iHUnitSize;
            }
            break;
        case TDR_TYPE_SMALLINT:
            {
                pszVal = tdr_get_bindmacro_name_i(a_pstLib, a_pstEntry, (int)*(short*)pszHostStart);
                if(NULL != pszVal)
                {
                    iRet = tdr_iostream_write(a_pstIOStream, "%s", pszVal);
                }else
                {
                    iRet = tdr_iostream_write(a_pstIOStream, "%d", (int)*(short*)pszHostStart);
                }
                pszHostStart += a_pstEntry->iHUnitSize;
                break;
            }
        case TDR_TYPE_SMALLUINT:
            {
                pszVal = tdr_get_bindmacro_name_i(a_pstLib, a_pstEntry, (int)*(unsigned short*)pszHostStart);
                if(NULL != pszVal)
                {
                    iRet = tdr_iostream_write(a_pstIOStream, "%s", pszVal);
                }else
                {
                    iRet = tdr_iostream_write(a_pstIOStream, "%d", (int)*(unsigned short*)pszHostStart);
                }
                pszHostStart += a_pstEntry->iHUnitSize;
                break;
            }
        case TDR_TYPE_LONG:
        case TDR_TYPE_INT:
            {
                pszVal = tdr_get_bindmacro_name_i(a_pstLib, a_pstEntry, (int)*(int*)pszHostStart);
                if(NULL != pszVal)
                {
                    iRet = tdr_iostream_write(a_pstIOStream, "%s", pszVal);
                }else
                {
                    iRet = tdr_iostream_write(a_pstIOStream, "%d", (int)*(int*)pszHostStart);
                }
                pszHostStart += a_pstEntry->iHUnitSize;
                break;
            }
        case TDR_TYPE_ULONG:
        case TDR_TYPE_UINT:
            {
                pszVal = tdr_get_bindmacro_name_i(a_pstLib, a_pstEntry, (int)*(unsigned int*)pszHostStart);
                if(NULL != pszVal)
                {
                    iRet = tdr_iostream_write(a_pstIOStream, "%s", pszVal);
                }else
                {
                    iRet = tdr_iostream_write(a_pstIOStream, "%u", (unsigned
                                                                    int)*(unsigned int*)pszHostStart);
                }
                pszHostStart += a_pstEntry->iHUnitSize;
                break;
            }
        case TDR_TYPE_LONGLONG:
            {
#if (defined(_WIN32) || defined(_WIN64)) &&  _MSC_VER < 1400  /*vc7,vc6,,*/
                iRet = tdr_iostream_write(a_pstIOStream, "%I64i ", *(int64_t*)pszHostStart);
#else
                iRet = tdr_iostream_write(a_pstIOStream, "%lld", *(int64_t*)pszHostStart);
#endif
                pszHostStart += a_pstEntry->iHUnitSize;
                break;
            }

        case TDR_TYPE_ULONGLONG:
            {
#if (defined(_WIN32) || defined(_WIN64)) &&  _MSC_VER < 1400  /*vc7,vc6,,*/
                iRet = tdr_iostream_write(a_pstIOStream, "%I64u ", *(uint64_t*)pszHostStart);
#else
                iRet = tdr_iostream_write(a_pstIOStream, "%llu", *(uint64_t*)pszHostStart);
#endif
                pszHostStart += a_pstEntry->iHUnitSize;
                break;
            }
        case TDR_TYPE_FLOAT:
            iRet = tdr_iostream_write(a_pstIOStream, "%f", *(float*)pszHostStart);
            pszHostStart += a_pstEntry->iHUnitSize;
            break;
        case TDR_TYPE_DOUBLE:
            iRet = tdr_iostream_write(a_pstIOStream, "%f", *(double*)pszHostStart);
            pszHostStart += a_pstEntry->iHUnitSize;
            break;

        case TDR_TYPE_DATE:
            {
                char szDate[16]={0};
                iRet = tdr_iostream_write(a_pstIOStream, "%s", tdr_tdrdate_to_str_r((tdr_date_t *)pszHostStart,szDate,sizeof(szDate)));
                pszHostStart += a_pstEntry->iHUnitSize;
            }
            break;
        case TDR_TYPE_TIME:
            {
                char szTime[16]={0};
                iRet = tdr_iostream_write(a_pstIOStream, "%s", tdr_tdrtime_to_str_r((tdr_time_t *)pszHostStart,szTime,sizeof(szTime)));
                pszHostStart += a_pstEntry->iHUnitSize;
            }
            break;
        case TDR_TYPE_DATETIME:
            {
                char szDateTime[32]={0};
                iRet = tdr_iostream_write(a_pstIOStream, "%s", tdr_tdrdatetime_to_str_r((tdr_datetime_t *)pszHostStart,szDateTime,sizeof(szDateTime)));
                pszHostStart += a_pstEntry->iHUnitSize;
            }
            break;
        case TDR_TYPE_IP:
            {
                char szIP[32] = {0};
                if (NULL != tdr_tdrip_ntop(*(tdr_ip_t *)pszHostStart, szIP,sizeof(szIP)))
                {
                    iRet = tdr_iostream_write(a_pstIOStream, "%s", szIP);
                }else
                {
                    iRet = tdr_iostream_write(a_pstIOStream, "");
                }
                pszHostStart += a_pstEntry->iHUnitSize;
            }
            break;
        case TDR_TYPE_WCHAR:
            {
                tdr_wchar_t szTemp[2] = {0};
                char szMbs[4] = {0};
                size_t iLen = sizeof(szMbs);

                szTemp[0] = *(tdr_wchar_t *)pszHostStart;
                tdr_wcstochinesembs(&szMbs[0], &iLen, &szTemp[0], 2);
                tdr_iostream_write(a_pstIOStream, "%s",	szMbs);
                pszHostStart += a_pstEntry->iHUnitSize;
                break;
            }
        case TDR_TYPE_STRING:
            {
                if (0 < a_pstEntry->iCustomHUnitSize)
                {
                    iSize = a_pstEntry->iCustomHUnitSize;
                }else
                {
                    iSize = a_pszHostEnd - pszHostStart;
                }
                iLen = tdr_strnlen(pszHostStart, iSize);
                if (iLen >= iSize)
                {
                    iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NET_INVALID_STRING_LEN);
                    break;
                }
                iRet = tdr_iostream_write(a_pstIOStream, "%s", pszHostStart);
                pszHostStart += iSize;
                break;
            }
        case TDR_TYPE_WSTRING:
            {
                char *pszMbs = NULL;
                if (0 < a_pstEntry->iCustomHUnitSize)
                {
                    iSize = a_pstEntry->iCustomHUnitSize;
                }else
                {
                    iSize = a_pszHostEnd - pszHostStart;
                }
                pszMbs = tdr_wcstochinesembs_i((tdr_wchar_t *)pszHostStart, iSize/sizeof(tdr_wchar_t));
                if (NULL == pszMbs)
                {
                    iRet = tdr_iostream_write(a_pstIOStream, "");
                }else
                {
                    iRet = tdr_iostream_write(a_pstIOStream, "%s", pszMbs);
                    free(pszMbs);
                }
                pszHostStart += iSize;
                break;
            }
        default:	/* must be 8 bytes. */
            break;
    }/*switch(a_pstEntry->iType)*/

    *a_ppszHostStart = pszHostStart;

    return iRet;
}

/**输出json格式基本数据类型成员的值
 *@param[in] a_pstIOStream 输出信息的流句柄
 *@param[in] a_pstEntry 成员的描述结构指针
 *@param[in,out] a_ppszHostStart 此成员值存储空间的起始地址
 *@param[in] a_pszHostEnd 此成员值存储空间的结束地址
 *@return 成功返回0，否则返回非零值
 */
int tdr_json_ioprintf_basedtype_i(IN LPTDRIOSTREAM a_pstIOStream, IN LPTDRMETAENTRY a_pstEntry,
                                  INOUT const char **a_ppszHostStart, IN const char *a_pszHostEnd)
{
    int iRet = TDR_SUCCESS;
    size_t iSize;
    size_t iLen;
    size_t i;
    const char *pszHostStart;

    assert(NULL != a_pstIOStream);
    assert(NULL != a_pstEntry);
    assert(NULL != a_ppszHostStart);
    assert(NULL != *a_ppszHostStart);
    assert(NULL != a_pszHostEnd);
    assert(TDR_TYPE_COMPOSITE < a_pstEntry->iType);

    pszHostStart = *a_ppszHostStart;

    switch(a_pstEntry->iType)
    {
        case TDR_TYPE_CHAR:
            {
                iRet = tdr_iostream_write(a_pstIOStream, "%d", (int)(char)pszHostStart[0]);
                pszHostStart += a_pstEntry->iHUnitSize;
            }
            break;
        case TDR_TYPE_UCHAR:
        case TDR_TYPE_BYTE:
            {
                iRet = tdr_iostream_write(a_pstIOStream, "%d", (int)(unsigned char)pszHostStart[0]);
                pszHostStart += a_pstEntry->iHUnitSize;
            }
            break;
        case TDR_TYPE_SMALLINT:
            {
                iRet = tdr_iostream_write(a_pstIOStream, "%d", (int)*(short*)pszHostStart);
                pszHostStart += a_pstEntry->iHUnitSize;
                break;
            }
        case TDR_TYPE_SMALLUINT:
            {
                iRet = tdr_iostream_write(a_pstIOStream, "%d", (int)*(unsigned short*)pszHostStart);
                pszHostStart += a_pstEntry->iHUnitSize;
                break;
            }
        case TDR_TYPE_LONG:
        case TDR_TYPE_INT:
            {
                iRet = tdr_iostream_write(a_pstIOStream, "%d", (int)*(int*)pszHostStart);
                pszHostStart += a_pstEntry->iHUnitSize;
                break;
            }
        case TDR_TYPE_ULONG:
        case TDR_TYPE_UINT:
            {
                iRet = tdr_iostream_write(a_pstIOStream, "%u", (unsigned int)*(unsigned int*)pszHostStart);
                pszHostStart += a_pstEntry->iHUnitSize;
                break;
            }
        case TDR_TYPE_LONGLONG:
            {
#if (defined(_WIN32) || defined(_WIN64)) &&  _MSC_VER < 1400  /*vc7,vc6,,*/
                iRet = tdr_iostream_write(a_pstIOStream, "%I64i ", *(int64_t*)pszHostStart);
#else
                iRet = tdr_iostream_write(a_pstIOStream, "%lld", *(int64_t*)pszHostStart);
#endif
                pszHostStart += a_pstEntry->iHUnitSize;
                break;
            }

        case TDR_TYPE_ULONGLONG:
            {
#if (defined(_WIN32) || defined(_WIN64)) &&  _MSC_VER < 1400  /*vc7,vc6,,*/
                iRet = tdr_iostream_write(a_pstIOStream, "%I64u ", *(uint64_t*)pszHostStart);
#else
                iRet = tdr_iostream_write(a_pstIOStream, "%llu", *(uint64_t*)pszHostStart);
#endif
                pszHostStart += a_pstEntry->iHUnitSize;
                break;
            }
        case TDR_TYPE_FLOAT:
            iRet = tdr_iostream_write(a_pstIOStream, "%f", *(float*)pszHostStart);
            pszHostStart += a_pstEntry->iHUnitSize;
            break;
        case TDR_TYPE_DOUBLE:
            iRet = tdr_iostream_write(a_pstIOStream, "%f", *(double*)pszHostStart);
            pszHostStart += a_pstEntry->iHUnitSize;
            break;

        case TDR_TYPE_DATE:
            {
                char szDate[16]={0};
                iRet = tdr_iostream_write(a_pstIOStream, "\"%s\"", tdr_tdrdate_to_str_r((tdr_date_t *)pszHostStart,szDate,sizeof(szDate)));
                pszHostStart += a_pstEntry->iHUnitSize;
            }
            break;
        case TDR_TYPE_TIME:
            {
                char szTime[16]={0};
                iRet = tdr_iostream_write(a_pstIOStream, "\"%s\"", tdr_tdrtime_to_str_r((tdr_time_t *)pszHostStart,szTime,sizeof(szTime)));
                pszHostStart += a_pstEntry->iHUnitSize;
            }
            break;
        case TDR_TYPE_DATETIME:
            {
                char szDateTime[32]={0};
                iRet = tdr_iostream_write(a_pstIOStream, "\"%s\"", tdr_tdrdatetime_to_str_r((tdr_datetime_t *)pszHostStart,szDateTime,sizeof(szDateTime)));
                pszHostStart += a_pstEntry->iHUnitSize;
            }
            break;
        case TDR_TYPE_IP:
            {
                char szIP[32] = {0};
                if (NULL != tdr_tdrip_ntop(*(tdr_ip_t *)pszHostStart, szIP,sizeof(szIP)))
                {
                    iRet = tdr_iostream_write(a_pstIOStream, "\"%s\"", szIP);
                }else
                {
                    iRet = tdr_iostream_write(a_pstIOStream, "\"\"");
                }
                pszHostStart += a_pstEntry->iHUnitSize;
            }
            break;
        case TDR_TYPE_WCHAR: /* TODO： coding and escape chars*/
            {
                tdr_wchar_t szTemp[2] = {0};
                char szMbs[4] = {0};
                size_t iLen = sizeof(szMbs);

                szTemp[0] = *(tdr_wchar_t *)pszHostStart;
                tdr_wcstochinesembs(&szMbs[0], &iLen, &szTemp[0], 2);
                tdr_iostream_write(a_pstIOStream, "\"%s\"",	szMbs);
                pszHostStart += a_pstEntry->iHUnitSize;
                break;
            }
        case TDR_TYPE_STRING:
            {
                if (0 < a_pstEntry->iCustomHUnitSize)
                {
                    iSize = a_pstEntry->iCustomHUnitSize;
                }else
                {
                    iSize = a_pszHostEnd - pszHostStart;
                }
                iLen = tdr_strnlen(pszHostStart, iSize);
                if (iLen >= iSize)
                {
                    iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NET_INVALID_STRING_LEN);
                    break;
                }

                iRet = tdr_iostream_write(a_pstIOStream, "\"", pszHostStart);
                if (TDR_SUCCESS != iRet)
                {
                    break;
                }

                for (i = 0; i < iLen; i++)
                {
                    switch (*(pszHostStart + i))
                    {
                        case '\b':
                            iRet = tdr_iostream_write(a_pstIOStream, "\\b");
                            break;
                        case '\t':
                            iRet = tdr_iostream_write(a_pstIOStream, "\\t");
                            break;
                        case '\r':
                            iRet = tdr_iostream_write(a_pstIOStream, "\\r");
                            break;
                        case '\n':
                            iRet = tdr_iostream_write(a_pstIOStream, "\\n");
                            break;
                        case '\f':
                            iRet = tdr_iostream_write(a_pstIOStream, "\\f");
                            break;
                        case '/':
                            iRet = tdr_iostream_write(a_pstIOStream, "\\/");
                            break;
                        case '\\':
                            iRet = tdr_iostream_write(a_pstIOStream, "\\\\");
                            break;
                        case '"':
                            iRet = tdr_iostream_write(a_pstIOStream, "\\\"");
                            break;
                        default:
                            iRet = tdr_iostream_write(a_pstIOStream, "%c", *(pszHostStart + i));
                            break;
                    }
                    if (TDR_SUCCESS != iRet)
                    {
                        break;
                    }
                }
                if (TDR_SUCCESS != iRet)
                {
                    break;
                }

                iRet = tdr_iostream_write(a_pstIOStream, "\"", pszHostStart);
                if (TDR_SUCCESS != iRet)
                {
                    break;
                }

                pszHostStart += iSize;
                break;
            }
        case TDR_TYPE_WSTRING: /* TODO： coding and escape chars*/
            {
                char *pszMbs = NULL;
                if (0 < a_pstEntry->iCustomHUnitSize)
                {
                    iSize = a_pstEntry->iCustomHUnitSize;
                }else
                {
                    iSize = a_pszHostEnd - pszHostStart;
                }
                pszMbs = tdr_wcstochinesembs_i((tdr_wchar_t *)pszHostStart, iSize/sizeof(tdr_wchar_t));
                if (NULL == pszMbs)
                {
                    iRet = tdr_iostream_write(a_pstIOStream, "\"\"");
                }else
                {
                    iRet = tdr_iostream_write(a_pstIOStream, "\"%s\"", pszMbs);
                    free(pszMbs);
                }
                pszHostStart += iSize;
                break;
            }
        default:	/* must be 8 bytes. */
            break;
    }/*switch(a_pstEntry->iType)*/

    *a_ppszHostStart = pszHostStart;

    return iRet;
}

/**根据从字符串值中输入基本数据类型成员的值
 *@param[in] a_pstLib 元数据描述库指针
 *@param[in] a_pstEntry 成员的描述结构指针
 *@param[in] a_pszHostStart 此成员值存储空间的起始地址
 *@param[in,out] a_piSize 保存输入数据内存空间大小的指针
 *	-	输入	可使用的内存空间大小
 *	-	输出	实际以使用的内存空间大小
 *@return 成功返回0，否则返回非零值
 */
int tdr_ioscanf_basedtype_i(IN LPTDRMETALIB a_pstLib, IN LPTDRMETAENTRY a_pstEntry,
                            IN char *a_pszHostStart, INOUT size_t *a_piSize, IN const char *a_pszValue)
{
    int iRet = TDR_SUCCESS;
    long lVal;
    int64_t llVal;
    uint64_t ullVal;
    size_t iSize;
    size_t iLen;

    assert(NULL != a_pstLib);
    assert(NULL != a_pstEntry);
    assert(NULL != a_pszHostStart);
    assert(NULL != a_piSize);
    assert(NULL != a_pszValue);

    switch(a_pstEntry->iType)
    {
        case TDR_TYPE_CHAR:
            {
                if (*a_piSize < a_pstEntry->iHUnitSize)
                {
                    iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);
                    break;
                }
                TDR_SCANF_INT_FROM_STR(lVal, a_pstLib, a_pstEntry, a_pszValue);
                if ((-128 > lVal) || (127 < lVal) )
                {
                    iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_VALUE_BEYOND_TYPE_FIELD);
                    break;
                }
                *a_pszHostStart = (int8_t)lVal;
                *a_piSize = a_pstEntry->iHUnitSize;
                break;
            }
        case TDR_TYPE_BYTE:
        case TDR_TYPE_UCHAR:
            {
                if (*a_piSize < a_pstEntry->iHUnitSize)
                {
                    iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);
                    break;
                }
                TDR_SCANF_INT_FROM_STR(lVal, a_pstLib, a_pstEntry, a_pszValue);
                if ((0 > lVal) || (0xFF < lVal) )
                {
                    iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_VALUE_BEYOND_TYPE_FIELD);
                    break;
                }
                *(int8_t*)a_pszHostStart = (uint8_t)lVal;
                *a_piSize = a_pstEntry->iHUnitSize;
                break;
            }
        case TDR_TYPE_SMALLINT:
            {
                if (*a_piSize < a_pstEntry->iHUnitSize)
                {
                    iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);
                    break;
                }
                TDR_SCANF_INT_FROM_STR(lVal, a_pstLib, a_pstEntry, a_pszValue);
                if ((-32768 > lVal) || (0x7FFF < lVal) )
                {
                    iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_VALUE_BEYOND_TYPE_FIELD);
                    break;
                }
                *(int16_t *)a_pszHostStart = (int16_t)lVal;
                *a_piSize = a_pstEntry->iHUnitSize;
                break;
            }
        case TDR_TYPE_SMALLUINT:
            {
                if (*a_piSize < a_pstEntry->iHUnitSize)
                {
                    iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);
                    break;
                }
                TDR_SCANF_INT_FROM_STR(lVal, a_pstLib, a_pstEntry, a_pszValue);
                if ((0 > lVal) || (0xFFFF < lVal) )
                {
                    iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_VALUE_BEYOND_TYPE_FIELD);
                    break;
                }
                *(uint16_t *)a_pszHostStart = (uint16_t)lVal;
                *a_piSize = a_pstEntry->iHUnitSize;
                break;
            }
        case TDR_TYPE_INT:
        case TDR_TYPE_LONG:
            {
                if (*a_piSize < a_pstEntry->iHUnitSize)
                {
                    iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);
                    break;
                }
                TDR_SCANF_INT_FROM_STR(lVal, a_pstLib, a_pstEntry, a_pszValue);
                if (((int32_t)0x80000000 > (int32_t)lVal) || (0x7FFFFFFF < (int32_t)lVal) )
                {
                    iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_VALUE_BEYOND_TYPE_FIELD);
                    break;
                }
                *(int32_t *)a_pszHostStart = (int32_t)lVal;
                *a_piSize = a_pstEntry->iHUnitSize;
                break;
            }
        case TDR_TYPE_UINT:
        case TDR_TYPE_ULONG:
            {
                LPTDRMACRO pstMacro = NULL;
                if (*a_piSize < a_pstEntry->iHUnitSize)
                {
                    iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);
                    break;
                }
                if (TDR_INVALID_PTR != (a_pstEntry)->ptrMacrosGroup)
                {
                    pstMacro = tdr_get_bindmacro_by_name(a_pstLib, a_pstEntry, a_pszValue);
                }
                if (NULL != pstMacro)
                {
                    llVal = pstMacro->iValue;
                }else
                {
                    int iIdx = tdr_get_macro_index_by_name_i(a_pstLib, a_pszValue);
                    if (TDR_INVALID_INDEX == iIdx)
                    {
                        llVal = TDR_ATOLL(a_pszValue);
                    }else
                    {
                        pstMacro = TDR_GET_MACRO_TABLE(a_pstLib);
                        llVal = pstMacro[iIdx].iValue;
                    }
                }
                if (0 > llVal)
                {
                    iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_VALUE_BEYOND_TYPE_FIELD);
                    break;
                }
                *(uint32_t *)a_pszHostStart = (int32_t)llVal;
                *a_piSize = a_pstEntry->iHUnitSize;
                break;
            }
        case TDR_TYPE_LONGLONG:
            {
                if (*a_piSize < a_pstEntry->iHUnitSize)
                {
                    iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);
                    break;
                }
                llVal = TDR_ATOLL(a_pszValue);
                *(int64_t *)a_pszHostStart = llVal;
                *a_piSize = a_pstEntry->iHUnitSize;
                break;
            }
        case TDR_TYPE_ULONGLONG:
            {
                if (*a_piSize < a_pstEntry->iHUnitSize)
                {
                    iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);
                    break;
                }
                ullVal = TDR_ATOULL(a_pszValue);
                *(uint64_t *)a_pszHostStart = ullVal;
                *a_piSize = a_pstEntry->iHUnitSize;
                break;
            }
        case TDR_TYPE_FLOAT:
            {
                if (*a_piSize < a_pstEntry->iHUnitSize)
                {
                    iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);
                    break;
                }
                *(float *)a_pszHostStart = (float)atof(a_pszValue);
                *a_piSize = a_pstEntry->iHUnitSize;
                break;
            }
        case TDR_TYPE_DOUBLE:
            {
                if (*a_piSize < a_pstEntry->iHUnitSize)
                {
                    iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);
                    break;
                }
                *(double *)a_pszHostStart = strtod(a_pszValue, NULL);
                *a_piSize = a_pstEntry->iHUnitSize;
                break;
            }
        case TDR_TYPE_IP:
            {
                if (*a_piSize < a_pstEntry->iHUnitSize)
                {
                    iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);
                    break;
                }
                iRet = tdr_ineta_to_tdrip((tdr_ip_t *)a_pszHostStart , a_pszValue);
                *a_piSize = a_pstEntry->iHUnitSize;
                break;
            }
        case TDR_TYPE_DATETIME:
            {
                if (*a_piSize < a_pstEntry->iHUnitSize)
                {
                    iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);
                    break;
                }
                iRet = tdr_str_to_tdrdatetime((tdr_datetime_t *)a_pszHostStart, a_pszValue);
                *a_piSize = a_pstEntry->iHUnitSize;
                break;
            }
        case TDR_TYPE_DATE:
            {
                if (*a_piSize < a_pstEntry->iHUnitSize)
                {
                    iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);
                    break;
                }
                iRet = tdr_str_to_tdrdate((tdr_date_t *)a_pszHostStart, a_pszValue);
                *a_piSize = a_pstEntry->iHUnitSize;
                break;
            }
        case TDR_TYPE_TIME:
            {
                if (*a_piSize < a_pstEntry->iHUnitSize)
                {
                    iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);
                    break;
                }
                iRet = tdr_str_to_tdrtime((tdr_time_t *)a_pszHostStart, a_pszValue);
                *a_piSize = a_pstEntry->iHUnitSize;
                break;
            }
        case TDR_TYPE_WCHAR:
            {
                tdr_wchar_t swTemp[8];
                size_t iLen = 8*sizeof(tdr_wchar_t);

                if (*a_piSize < a_pstEntry->iHUnitSize)
                {
                    iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);
                    break;
                }
                iRet = tdr_chinesembstowcs((char *)&swTemp[0], &iLen, (char *)a_pszValue, (size_t)3);
                if (TDR_SUCCESS != iRet)
                {
                    break;
                }
                *(tdr_wchar_t *)a_pszHostStart = swTemp[0];
                *a_piSize = a_pstEntry->iHUnitSize;
                break;
            }
        case TDR_TYPE_WSTRING:
            {
                size_t iMbsLen;
                size_t iWcLen;
                if (0 < a_pstEntry->iCustomHUnitSize)
                {
                    iSize = a_pstEntry->iCustomHUnitSize;
                }else
                {
                    iSize = *a_piSize;
                }
                if ((int)(sizeof(tdr_wchar_t)) > iSize)
                {
                    iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);
                    break;
                }
                iMbsLen = strlen(a_pszValue) + 1;
                iWcLen = iSize;
                iRet = tdr_chinesembstowcs(a_pszHostStart, &iWcLen, (char *)a_pszValue, iMbsLen);
                if (TDR_ERR_IS_ERROR(iRet))
                {
                    *(tdr_wchar_t *)a_pszHostStart = L'\0';
                    break;
                }
                *a_piSize = iSize;
                break;
            }
        case TDR_TYPE_STRING:
            {
                if (0 < a_pstEntry->iCustomHUnitSize)
                {
                    iSize = a_pstEntry->iCustomHUnitSize;
                }else
                {
                    iSize = *a_piSize;
                }

#ifndef TDR_ESCAPE_CHARACTER
                iLen = tdr_strnlen(a_pszValue, iSize) + 1;
                if (iLen > iSize)
                {
                    iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NET_INVALID_STRING_LEN);
                    break;
                }
                if (iLen > *a_piSize)
                {
                    iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);
                    break;
                }
                TDR_MEMCPY(a_pszHostStart, a_pszValue, iLen, TDR_MIN_COPY);
#else
                if (*a_piSize < 1)
                {
                    iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);
                    break;
                }
                iLen = tdr_copy_with_escape_i(a_pszHostStart, a_pszValue, *a_piSize - 1);
                a_pszHostStart[iLen] = '\0';
#endif
                *a_piSize = iSize;
                break;
            }
        default:
            break;
    }

    return iRet;
}

char *tdr_get_bindmacro_name_i(IN LPTDRMETALIB a_pstLib, IN LPTDRMETAENTRY a_pstEntry, IN int a_iValue)
{

    char *pchDst;
    char *pchSrc;
    char pch;
    LPTDRMACROSGROUP pstGroup;
    LPTDRMACRO pstMacro;

    assert(NULL != a_pstLib);
    assert(NULL != a_pstEntry);

    if (TDR_INVALID_PTR == a_pstEntry->ptrMacrosGroup)
    {
        return NULL;
    }
    pstMacro = tdr_get_bindmacro_by_value(a_pstLib, a_pstEntry, a_iValue);
    if (NULL == pstMacro)
    {
        return NULL;
    }

    pstGroup = TDR_PTR_TO_MACROSGROUP(a_pstLib, a_pstEntry->ptrMacrosGroup);
    pchSrc = TDR_GET_STRING_BY_PTR(a_pstLib,pstMacro->ptrMacro);
    pchDst = &pstGroup->szName[0];
    while (*pchSrc && *pchDst)
    {
        pch = (char)toupper(*pchDst);
        if (*pchSrc != pch)
        {
            break;
        }
        pchSrc++;
        pchDst++;
    }

    if ('\0' == *pchDst)
    {
        return pchSrc + 1;
    }

    return TDR_GET_STRING_BY_PTR(a_pstLib,pstMacro->ptrMacro);
}

const char *tdr_get_entry_cname(IN LPTDRMETAENTRY a_pstEntry)
{
    LPTDRMETALIB pstLib = NULL;

    if (NULL == a_pstEntry)
    {
        return "";
    }

    pstLib = TDR_META_TO_LIB(TDR_ENTRY_TO_META(a_pstEntry));

    return TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrChineseName);
}

const char *tdr_get_entry_desc(IN LPTDRMETAENTRY a_pstEntry)
{
    LPTDRMETALIB pstLib = NULL;

    if (NULL == a_pstEntry || TDR_INVALID_PTR == a_pstEntry->ptrDesc)
    {
        return "";
    }

    pstLib = TDR_META_TO_LIB(TDR_ENTRY_TO_META(a_pstEntry));

    return TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrDesc);
}

LPTDRMACROSGROUP tdr_get_entry_macrosgroup(IN LPTDRMETAENTRY a_pstEntry)
{
    LPTDRMETALIB pstLib = NULL;
    LPTDRMACROSGROUP pstGroup = NULL;

    if (NULL == a_pstEntry || TDR_INVALID_PTR == a_pstEntry->ptrMacrosGroup)
    {
        return NULL;
    }

    pstLib = TDR_META_TO_LIB(TDR_ENTRY_TO_META(a_pstEntry));
    pstGroup = TDR_PTR_TO_MACROSGROUP(pstLib, a_pstEntry->ptrMacrosGroup);

    return pstGroup;
}

const char *tdr_get_entry_name(IN LPTDRMETAENTRY a_pstEntry)
{
    LPTDRMETALIB pstLib = NULL;

    if (NULL == a_pstEntry)
    {
        return "";
    }

    pstLib = TDR_META_TO_LIB(TDR_ENTRY_TO_META(a_pstEntry));

    return TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName);
}

int tdr_get_entry_version(IN LPTDRMETAENTRY a_pstEntry)
{
    if (NULL == a_pstEntry)
    {
        return -1;
    }

    return a_pstEntry->iVersion;
}

int tdr_get_entry_offset(IN LPTDRMETAENTRY a_pstEntry)
{
    if (NULL == a_pstEntry)
    {
        return -1;
    }
    return a_pstEntry->iHOff;
}

static const char *tdr_get_entry_id_name_i(IN LPTDRMETALIB a_pstLib, TDRIDX a_iId)
{
    LPTDRMACRO pstMacroTable = NULL;
    char *pszName = "";

    if( (TDR_MAGIC != (a_pstLib)->wMagic) || (TDR_BUILD != (a_pstLib)->nBuild))
    {
        tdr_set_last_error(TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_BUILD_VERSION_CONFLICT));
        return "";
    }
    if (TDR_FLATFORM_MAGIC != (a_pstLib)->dwPlatformArch)
    {
        tdr_set_last_error(TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_BUILD_VERSION_CONFLICT));
        return "";
    }


    pstMacroTable = TDR_GET_MACRO_TABLE(a_pstLib);
    pszName = TDR_GET_STRING_BY_PTR(a_pstLib,pstMacroTable[a_iId].ptrMacro); ;

    return pszName;
}

const char *tdr_get_entry_id_name(IN LPTDRMETALIB a_pstLib, IN LPTDRMETAENTRY a_pstEntry)
{
    if ((NULL == a_pstLib)||(NULL == a_pstEntry))
    {
        return "";
    }

    if ((TDR_INVALID_INDEX != a_pstEntry->idxID) && (a_pstEntry->idxID < a_pstLib->iCurMacroNum))
    {
        return tdr_get_entry_id_name_i(a_pstLib, a_pstEntry->idxID);
    } else
    {
        return "";
    }
}

const char *tdr_get_entry_maxid_name(IN LPTDRMETALIB a_pstLib, IN LPTDRMETAENTRY a_pstEntry)
{
    if ((NULL == a_pstLib)||(NULL == a_pstEntry))
    {
        return "";
    }

    if ((TDR_INVALID_INDEX != a_pstEntry->iMaxIdIdx) && (a_pstEntry->iMaxIdIdx < a_pstLib->iCurMacroNum))
    {
        return tdr_get_entry_id_name_i(a_pstLib, a_pstEntry->iMaxIdIdx);
    } else
    {
        return "";
    }
}

const char *tdr_get_entry_minid_name(IN LPTDRMETALIB a_pstLib, IN LPTDRMETAENTRY a_pstEntry)
{
    if ((NULL == a_pstLib)||(NULL == a_pstEntry))
    {
        return "";
    }

    if ((TDR_INVALID_INDEX != a_pstEntry->iMinIdIdx) && (a_pstEntry->iMinIdIdx < a_pstLib->iCurMacroNum))
    {
        return tdr_get_entry_id_name_i(a_pstLib, a_pstEntry->iMinIdIdx);
    } else
    {
        return "";
    }
}

int tdr_do_have_autoincrement_entry(IN LPTDRMETA a_pstMeta)
{
    //assert(NULL != a_pstMeta);
    if (NULL == a_pstMeta)
    {
        return 0;
    }

    return TDR_META_DO_HAVE_AUTOINCREMENT_ENTRY(a_pstMeta);
}

const char *tdr_get_entry_customattr(IN LPTDRMETALIB a_pstLib, IN LPTDRMETAENTRY a_pstEntry)
{
    char *pszCustomAttr = "";

    //assert(NULL != a_pstEntry);
    //assert(NULL != a_pstLib);
    if ((NULL == a_pstLib)||(NULL == a_pstEntry))
    {
        return pszCustomAttr;
    }

    if (TDR_INVALID_PTR != a_pstEntry->ptrCustomAttr)
    {
        pszCustomAttr = TDR_GET_STRING_BY_PTR(a_pstLib, a_pstEntry->ptrCustomAttr);
    }

    return pszCustomAttr;
}

LPTDRMETAENTRY tdr_get_entry_refer(IN LPTDRMETAENTRY a_pstEntry)
{
    LPTDRMETALIB pstLib = NULL;

    if (NULL == a_pstEntry)
    {
        return NULL;
    }

    if (0 >= a_pstEntry->stRefer.iUnitSize
        || TDR_INVALID_PTR == a_pstEntry->stRefer.ptrEntry)
    {
        return NULL;
    }

    pstLib = TDR_META_TO_LIB(TDR_ENTRY_TO_META(a_pstEntry));

    return TDR_PTR_TO_ENTRY(pstLib, a_pstEntry->stRefer.ptrEntry);
}

LPTDRMETAENTRY tdr_get_entry_select(IN LPTDRMETAENTRY a_pstEntry)
{
    LPTDRMETALIB pstLib = NULL;

    if (NULL == a_pstEntry)
    {
        return NULL;
    }

    if (TDR_TYPE_UNION != a_pstEntry->iType)
    {
        return NULL;
    }

    pstLib = TDR_META_TO_LIB(TDR_ENTRY_TO_META(a_pstEntry));

    return TDR_PTR_TO_ENTRY(pstLib, a_pstEntry->stSelector.ptrEntry);
}

size_t tdr_copy_with_escape_i(char* a_pszOut, const char* a_pszInput, size_t a_tSize)
{
    int index = 0;
    size_t tSizeSaved;
    int iScaned = 0;
    size_t tWrited = 0;
    char chCurrent;
    char chSpecialOne;   /* the first  char after '\' */
    char chSpecialTwo;   /* the second char after '\' */
    char chSpecialThree; /* the third  char after '\' */

    tSizeSaved = a_tSize;

    while ((0 < a_tSize) && ((chCurrent = a_pszInput[index]) != '\0'))
    {
        if (chCurrent == '\\')
        {
            chSpecialOne = a_pszInput[index + 1];

            if ('0' <= chSpecialOne && chSpecialOne <= '7')
            {
                int iOctNum;

                iOctNum = chSpecialOne - '0';
                chSpecialTwo = a_pszInput[index + 2];
                if ('0' <= chSpecialTwo && chSpecialTwo <= '7')
                {
                    iOctNum <<= 3;
                    iOctNum += chSpecialTwo - '0';
                    chSpecialThree = a_pszInput[index + 3];
                    if ('0' <= chSpecialThree  && chSpecialThree <= '7')
                    {
                        iOctNum <<= 3;
                        iOctNum += chSpecialThree - '0';

                        if (!iOctNum)
                        {
                            return tSizeSaved - a_tSize;
                        }else
                        {
                            iScaned = 4;
                            tWrited = 1;
                        }
                    }else
                    {
                        if (!iOctNum)
                        {
                            return tSizeSaved - a_tSize;
                        }else
                        {
                            iScaned = 3;
                            tWrited = 1;
                        }
                    }
                }else
                {
                    if (!iOctNum)
                    {
                        return tSizeSaved - a_tSize;
                    }else
                    {
                        iScaned = 2;
                        tWrited = 1;
                    }
                }
                *a_pszOut++ = (char)iOctNum;

                index += iScaned;
                a_tSize -= tWrited;
            } else if ('\\' == chSpecialOne)
            {
                iScaned = 2;
                tWrited = 1;
                *a_pszOut++ = chSpecialOne;

                index += iScaned;
                a_tSize -= tWrited;
            } else
            {
                *a_pszOut++ = chCurrent;
                index++;
                a_tSize--;
            } // if ('0' <= chSpecialOne && chSpecialOne <= '7')
        } else // if (chCurrent == '\\')
        {
            *a_pszOut++ = chCurrent;
            index++;
            a_tSize--;
        } // if (chCurrent == '\\')
    } // while ((0 < a_tSize) && ((chCurrent = a_pszInput[index]) != '\0'))

    return tSizeSaved - a_tSize;
}

int tdr_get_macrosgroup_num(IN LPTDRMETALIB a_pstLib)
{
    if (NULL == a_pstLib)
    {
        return 0;
    }

    return a_pstLib->iCurMacrosGroupNum;
}

LPTDRMACROSGROUP tdr_get_macrosgroup_by_name(IN LPTDRMETALIB a_pstLib, const char* a_pszName)
{
	LPTDRMAPENTRY pstMap = NULL;
    int iIdx = 0;

    if (NULL == a_pstLib || NULL == a_pszName)
    {
        return NULL;
    }

	pstMap = TDR_GET_MACROSGROUP_MAP_TABLE(a_pstLib);
    for (iIdx = 0; iIdx < a_pstLib->iCurMacrosGroupNum; iIdx++)
    {
        LPTDRMACROSGROUP pstGroup = TDR_PTR_TO_MACROSGROUP(a_pstLib, pstMap[iIdx].iPtr);
        if (0 == strcmp(pstGroup->szName, a_pszName))
        {
            return pstGroup;
        }
    }

    return NULL;
}

LPTDRMACROSGROUP tdr_get_macrosgroup_by_index(IN LPTDRMETALIB a_pstLib, IN int a_iIdx)
{
	LPTDRMAPENTRY pstMap = NULL;
    LPTDRMACROSGROUP pstGroup = NULL;

    if (NULL == a_pstLib)
    {
        return NULL;
    }

    if (0 > a_iIdx || a_iIdx >= a_pstLib->iCurMacrosGroupNum)
    {
        return NULL;
    }

	pstMap = TDR_GET_MACROSGROUP_MAP_TABLE(a_pstLib);
    pstGroup = TDR_PTR_TO_MACROSGROUP(a_pstLib, pstMap[a_iIdx].iPtr);

    return pstGroup;
}

void tdr_calc_checksum_i(IN LPTDRMETALIB a_pstLib)
{
    size_t i = 0 ;
    unsigned int iSum = 0;
    unsigned int *pTemp ;

    assert(NULL != a_pstLib);

    a_pstLib->checksum = 0;
    pTemp = (unsigned int *)a_pstLib;
    for (i = 0; i < a_pstLib->iSize/sizeof(int);i++)
    {
        iSum ^= *pTemp++;
    }
    a_pstLib->checksum = iSum;
}

unsigned int tdr_check_checksum_i(IN LPTDRMETALIB a_pstLib)
{
    size_t i = 0 ;
    unsigned int iSum = 0;
    unsigned int *pTemp ;

    assert(NULL != a_pstLib);


    pTemp = (unsigned int *)a_pstLib;
    for (i = 0; i < a_pstLib->iSize/sizeof(int);i++)
    {
        iSum ^= *pTemp++;
    }

    return iSum;
}

int tdr_get_metalib_macro_num(IN LPTDRMETALIB a_pstLib)
{
    if (NULL == a_pstLib)
    {
        return 0;
    }

    return a_pstLib->iCurMacroNum;
}

LPTDRMACRO tdr_get_metalib_macro_by_index(IN LPTDRMETALIB a_pstLib, IN int a_iIdx)
{
    if (NULL == a_pstLib)
    {
        return NULL;
    }

    if (0 > a_iIdx || a_pstLib->iCurMacroNum <= a_iIdx)
    {
        return NULL;
    }

    return TDR_GET_MACRO_TABLE(a_pstLib) + a_iIdx;
}

const char* tdr_get_macro_name_by_ptr(IN LPTDRMETALIB a_pstLib, IN LPTDRMACRO a_pstMacro)
{
    if (NULL == a_pstLib || NULL == a_pstMacro)
    {
        return "";
    }

    return TDR_GET_STRING_BY_PTR(a_pstLib, a_pstMacro->ptrMacro);
}

const char* tdr_get_macro_desc_by_ptr(IN LPTDRMETALIB a_pstLib, IN LPTDRMACRO a_pstMacro)
{
    if (NULL == a_pstLib || NULL == a_pstMacro || TDR_INVALID_PTR == a_pstMacro->ptrDesc)
    {
        return "";
    }

    return TDR_GET_STRING_BY_PTR(a_pstLib, a_pstMacro->ptrDesc);
}

int tdr_get_macro_value_by_ptr(OUT int *a_piID, IN LPTDRMACRO a_pstMacro)
{
    if (NULL == a_piID || NULL == a_pstMacro)
    {
        return -1;
    }

    *a_piID = a_pstMacro->iValue;

    return 0;
}

LPTDRMACROSGROUP tdr_get_macro_macrosgroup(IN LPTDRMETALIB a_pstLib, IN LPTDRMACRO a_pstMacro)
{
    int i, j;
    int iIdx;
    TDRIDX* pValueTable = NULL;
	LPTDRMAPENTRY pstMap = NULL;
    LPTDRMACROSGROUP pstGroup = NULL;

    if (NULL == a_pstLib || NULL == a_pstMacro)
    {
        return NULL;
    }

    iIdx = a_pstMacro - TDR_GET_MACRO_TABLE(a_pstLib);
    if (0 > iIdx || iIdx >= a_pstLib->iCurMacroNum)
    {
        return NULL;
    }

	pstMap = TDR_GET_MACROSGROUP_MAP_TABLE(a_pstLib);
	for (i = 0; i < a_pstLib->iCurMacrosGroupNum; i++)
	{
		pstGroup = TDR_PTR_TO_MACROSGROUP(a_pstLib, pstMap[i].iPtr);
		pValueTable = TDR_GET_MACROSGROUP_VALUEIDXMAP_TAB(pstGroup);
		for (j = 0; j < pstGroup->iCurMacroCount; j++)
		{
			if ((const int)iIdx == pValueTable[j])
            {
                return pstGroup;
            }
		}
	}

    return NULL;
}

int tdr_get_macrosgroup_macro_num(IN LPTDRMACROSGROUP a_pstGroup)
{
    if (NULL == a_pstGroup)
    {
        return 0;
    }

    return a_pstGroup->iCurMacroCount;
}

const char* tdr_get_macrosgroup_name(IN LPTDRMACROSGROUP a_pstGroup)
{
    if (NULL == a_pstGroup)
    {
        return "";
    }

    return a_pstGroup->szName;
}

LPTDRMACRO tdr_get_macrosgroup_macro_by_index(IN LPTDRMETALIB a_pstLib,
                                              IN LPTDRMACROSGROUP a_pstGroup,
                                              IN int a_iIdx)
{
    LPTDRMACRO pstMacro = NULL;
    LPTDRMACRO pstMacroTable = NULL;
    TDRIDX* pValueTable = NULL;

    if (NULL == a_pstLib || NULL == a_pstGroup)
    {
        return NULL;
    }

    if (0 > a_iIdx || a_iIdx >= a_pstGroup->iCurMacroCount)
    {
        return NULL;
    }

    pValueTable = TDR_GET_MACROSGROUP_VALUEIDXMAP_TAB(a_pstGroup);
    pstMacroTable = TDR_GET_MACRO_TABLE(a_pstLib);
    pstMacro = pstMacroTable + pValueTable[a_iIdx];

    return pstMacro;
}
