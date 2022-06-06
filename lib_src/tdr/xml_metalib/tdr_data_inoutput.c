/**
 *
 * @file     tdr_data_inoutput.c
 * @brief    TDR数据输入输出模块
 *
 * @author steve jackyai
 * @version 1.0
 * @date 2007-09-11
 *
 *
 * Copyright (c)  2007, 腾讯科技有限公司互动娱乐研发部
 * All rights reserved.
 *
 */
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include <scew/scew.h>
#include "tdr/tdr_os.h"
#include "scew/scew.h"
#include "tdr/tdr_define.h"
#include "tdr/tdr_data_io.h"
#include "tdr/tdr_error.h"
#include "tdr/tdr_metalib_init.h"
#include "tdr/tdr_metalib_kernel_i.h"
#include "tdr/tdr_ctypes_info_i.h"
#include "tdr/tdr_define_i.h"
#include "tdr/tdr_iostream_i.h"
#include "tdr_XMLMetalib_i.h"
#include "tdr/tdr_XMLtags.h"
#include "tdr/tdr_auxtools.h"
#include "tdr/tdr_metalib_manage_i.h"
#include "tdr_xml_inoutput_i.h"
#include "tdr_json_parser.h"

#if defined (_WIN32) || defined (_WIN64)
#pragma warning(disable:4996)
#endif


#define TDR_SET_ARRAY_REAL_COUNT(a_iArrayRealCount, a_pstEntry, a_pszHostBase) \
{										\
    if (0 < a_pstEntry->stRefer.iUnitSize)	\
    {										\
        char *pszPtr =	a_pszHostBase + a_pstEntry->stRefer.iHOff;	\
        TDR_SET_INT(pszPtr, a_pstEntry->stRefer.iUnitSize, a_iArrayRealCount);			\
    }\
}





#define TDR_GET_NEXT_ELEMET(a_pstItem, a_pstRoot, a_pstCurItem, a_pszName) \
{																						 \
    a_pstItem = scew_element_next((scew_element *)a_pstRoot,				 \
            (scew_element *)a_pstCurItem);     						\
    while (NULL != a_pstItem)															\
    {																					\
        if (0 == strcmp(scew_element_name(a_pstItem), a_pszName))						\
        {																				\
            break;																		\
        }else																			\
        {																				\
            a_pstItem = scew_element_next((scew_element *)a_pstRoot, a_pstItem);  	\
        }																				\
    }																					\
}

#define TDR_CLEAR_COUNT(a_pstTop, a_pstMeta, a_pszHostStart, a_pszHostEnd)	\
{																	\
    ptrdiff_t iLen;															\
    for (; a_pstTop->iCount > 0; a_pstTop->iCount--)				\
    {																\
        a_pszHostStart = a_pstTop->pszHostBase;						\
        if (a_pszHostStart > a_pszHostEnd)							\
        {															\
            a_pstTop->iCount = 0;									\
            break;													\
        }															\
        if ((a_pszHostEnd-a_pszHostStart) < (ptrdiff_t)a_pstMeta->iHUnitSize)	\
        {															\
            iLen = (a_pszHostEnd-a_pszHostStart);				\
        }else														\
        {															\
            iLen = a_pstMeta->iHUnitSize;							\
        }															\
        memset(a_pszHostStart, 0, iLen);							\
        a_pszHostStart += iLen;										\
        a_pstTop->pszHostBase += a_pstMeta->iHUnitSize;				\
    }																\
}



#define TDR_GET_VERSION_INDICATOR_BY_XML(a_pstMeta, a_pstRoot, a_iCutOffVersion, a_iBaseCutVersion) \
{																								\
    a_iCutOffVersion = a_iBaseCutVersion;														\
}

/**
  将本地内存数据结构按元数据描述转换成XML格式的数据
  */
static int tdr_output_i(IN LPTDRMETA a_pstMeta, INOUT LPTDRIOSTREAM a_pstIOStream, IN LPTDRDATA a_pstHost,
        IN int a_iCufOffVersion);

/**
  将本地内存数据结构按元数据描述转换成之前旧的XML格式的数据
  */
static int tdr_output_oldversion_i(IN LPTDRMETA a_pstMeta, INOUT LPTDRIOSTREAM a_pstIOStream, IN LPTDRDATA a_pstHost,
        IN int a_iCufOffVersion);


static int tdr_input_by_strict_xml_i(IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstHost, IN scew_element *a_pstRoot,
        IN int a_iCutOffVersion, IN int a_iIOVersion);

static int tdr_input_by_xml_i(IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstHost, IN scew_element *a_pstRoot,
        IN int a_iCutOffVersion);



static scew_element * tdr_input_root_i(IN scew_element *a_pstRoot, IN const char *a_pszRootName);

static int tdr_input_by_xml_oldversion_i(IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstHost, IN scew_element *a_pstRoot,
        IN int a_iCutOffVersion);

/*回调函数，在遍历结构成员时，如果此成员为struct/uion类型则回调此函数，获取此子成员处理句柄
@param[in] a_pHandle 此成员父结点的处理句柄
@param[in] a_pstChildEntry 此子成员的元数据描述句柄
@param[in] a_iTotal 此子成员的数组长度
@param[in] a_Index	如果此成员所在结构体为数组，则传入其数组当前索引值，否则传 -1
@note 根据子成员的a_pstChildEntry句柄调用tdr_get_entry_type_meta可以获得此子成员的类型
@return 0 成功 !0 失败
*/
typedef int (*PFN_JSON_ENTER_META)(IN void *a_pHandle, IN LPTDRMETAENTRY a_pstChildEntry,
                                   IN int a_iTotal, IN int a_Index);

/*回调函数，在遍历结构成员时，如果此成员为struct/uion类型则回调此函数，获取此子成员处理句柄
@param[in] a_pHandle 此结点的处理句柄
@param[in] a_pstMeta 此结点的元数据描述句柄
@return 0 成功 !0 失败
*/
typedef int (*PFN_JSON_LEAVE_META)(IN void *a_pHandle, IN LPTDRMETA a_pstMeta);

/*回调函数，在遍历结构成员时，如果此成员为简单数据类型,则回调此函数，
@param[in] a_pHandle 此成员父结点的处理句柄
@param[in] a_pstEntry 此子成员的元数据描述句柄
@param[in] a_iCount 此成员的数组长度
@param[in] a_pszHostStart 此成员本地存储起始地址
@param[in] a_iSize 此成员存储空间可用长度
@return 0 成功 !0 失败
*/
typedef int (*PFN_JSON_VISIT_SIMPLE_ENTRY)(IN void *a_pHandle, IN LPTDRMETAENTRY a_pstEntry,
                                           IN int a_iCount, IN char *a_pszHostStart, IN int a_iSize);

static int tdr_output_json_i(IN LPTDRMETA a_pstMeta, LPTDRIOSTREAM a_pstIOStream,
                             IN LPTDRDATA a_pstHost, IN int a_iCutOffVersion);

/*从XML数据树节点中读入简单类型成员的值到内存空间中
 *@param[in] a_pstLib 元数据描述库指针
 *@param[in] a_pstEntry 指定成员描述结构的指针
 *@param[in,out] a_piCount
 *	- in 指定最大读取的元素个数
 *	- out	输出实际读取的元素个数
 *@param[in] a_pstRoot xml数据的根节点指针
 *@param[in,out] a_ppstItem
 *	- in	起始查找的儿子节点指针
 *	- out	下一次查找的儿子节点指针
 *@param[in, out] a_ppszHostStart 保存成员值的内存空间指针
 *	- in	保存数据内存空间起始地址
 *	- out	下一个成员可以使用的数据内存空间起始地址
 *@param[in] a_pszHostEnd 可使用内存空间终止地址
 */
int tdr_input_simple_entry_i(IN LPTDRMETALIB a_pstLib, IN LPTDRMETAENTRY a_pstEntry, INOUT int *a_piCount,
        IN scew_element *a_pstRoot, INOUT scew_element **a_ppstItem,
        INOUT char **a_ppszHostStart, IN char *a_pszHostEnd);


int tdr_output_simpleentry_i(INOUT LPTDRIOSTREAM a_pstIOStream, IN LPTDRMETALIB a_pstLib,
        IN LPTDRMETAENTRY a_pstEntry, IN int a_iCount,
        IN char *a_pszHostStart, IN char *a_pszHostEnd, IN char *a_pszSpace);

int tdr_esc_xml_i(INOUT LPTDRIOSTREAM a_pstIOStream, IN const char *a_pszHostStart, IN ptrdiff_t a_iSize);

static int tdr_output_mata_attrs_i(INOUT LPTDRIOSTREAM a_pstIOStream, OUT int *a_piChildNum, IN LPTDRMETA a_pstMeta, IN int a_iFirstEntry,
        IN const char *a_pszHostStart, IN const char *a_pszHostEnd, int a_iVersion);

static int tdr_output_attr_format_t(IN LPTDRMETA a_pstMeta, INOUT LPTDRIOSTREAM a_pstIOStream, IN LPTDRDATA a_pstHost,
        IN int a_iCufOffVersion);

static int tdr_output_simpleentry_json_i(LPTDRIOSTREAM a_pstIOStream, LPTDRMETAENTRY a_pstEntry,
                                         const char* a_pszHostStart, const char* a_pszHostEnd, int a_iRealCount);

/** 从a_pstStream中解析出结构化数据到a_pstHost中
 * a_pstMeta 结构化数据的描述
 * a_pstHost 结构化数据的二进制内存
 * a_pstStream JSON流数据
 * a_iCutOffVersion 数据的剪裁版本，结构化数据的成员版本如果大于些值，则不解析出
 */
static int tdr_json_input_i(IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstHost,
                            IN LPTDRJSONPARSER a_pstParser, IN int a_iCutOffVersion);


extern char g_szEncoding[128];

////////////////////////////////////////////////////////////////////////////////////////////////////
int tdr_input(IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstHost, IN LPTDRDATA a_pstXml,
        IN int a_iCutOffVersion, IN int a_iXmlDataFormat)
{
    scew_tree* pstTree = NULL;
    scew_element *pstRoot;
    int iRet = TDR_SUCCESS;



    /*assert(NULL != a_pstMeta);
      assert(NULL != a_pstHost);
      assert(NULL != a_pstXml); */
    if ((NULL == a_pstMeta) || (NULL == a_pstXml)|| (NULL == a_pstHost))
    {
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
    }
    if (TDR_TYPE_STRUCT != a_pstMeta->iType)
    {
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
    }
    if ((NULL == a_pstXml->pszBuff) || (0 >= a_pstXml->iBuff) ||
        (NULL == a_pstHost->pszBuff) || (0 >= a_pstHost->iBuff))
    {
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
    }


    if (0 == a_iCutOffVersion)
    {
        a_iCutOffVersion = TDR_MAX_VERSION;
    }
    if (a_pstMeta->iBaseVersion > a_iCutOffVersion)
    {
        a_pstHost->iBuff = 0;
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
    }

    /*创建XML元素树*/
    iRet = tdr_create_XMLParser_tree_byBuff_i(&pstTree, a_pstXml->pszBuff, a_pstXml->iBuff, stderr);
    if (TDR_ERR_IS_ERROR(iRet))
    {
        return iRet;
    }

    pstRoot = scew_tree_root(pstTree);
    if (NULL == pstRoot)
    {
        a_pstHost->iBuff = 0;
        iRet = TDR_ERRIMPLE_NO_XML_ROOT;
    }


    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_input_i(a_pstMeta, a_pstHost, pstRoot, a_iCutOffVersion, a_iXmlDataFormat);
    }

    scew_tree_free( pstTree );

    return iRet;
}

int tdr_input_fp(IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstHost, IN FILE *a_fp,
        IN int a_iCutOffVersion, IN int a_iXmlDataFormat)
{
    scew_tree* pstTree = NULL;
    scew_element *pstRoot;
    int iRet = TDR_SUCCESS;


    /*assert(NULL != a_pstMeta);
      assert(NULL != a_pstHost);
      assert(NULL != a_fp);*/
    if ((NULL == a_pstMeta) || (NULL == a_fp)|| (NULL == a_pstHost))
    {
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
    }
    if ((NULL == a_pstHost->pszBuff) || (0 >= a_pstHost->iBuff))
    {
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
    }



    if (0 == a_iCutOffVersion)
    {
        a_iCutOffVersion = TDR_MAX_VERSION;
    }
    if (a_pstMeta->iBaseVersion > a_iCutOffVersion)
    {
        a_pstHost->iBuff = 0;
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
    }

    /*创建XML元素树*/
    iRet = tdr_create_XMLParser_tree_byfp(&pstTree, a_fp, stderr);
    if (TDR_ERR_IS_ERROR(iRet))
    {
        return iRet;
    }

    pstRoot = scew_tree_root(pstTree);
    if (NULL == pstRoot)
    {
        a_pstHost->iBuff = 0;
        iRet = TDR_ERRIMPLE_NO_XML_ROOT;
    }

    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_input_i(a_pstMeta, a_pstHost, pstRoot, a_iCutOffVersion, a_iXmlDataFormat);
    }

    scew_tree_free( pstTree );

    return iRet;
}

int tdr_input_file(IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstHost, IN const char *a_pszFile,
        IN int a_iCutOffVersion, IN int a_iXmlDataFormat)
{
    scew_tree* pstTree = NULL;
    scew_element *pstRoot;
    int iRet = TDR_SUCCESS;

    /*assert(NULL != a_pstMeta);
      assert(NULL != a_pstHost);
      assert(NULL != a_pszFile); */
    if ((NULL == a_pstMeta) || (NULL == a_pszFile)|| (NULL == a_pstHost))
    {
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
    }
    if ((NULL == a_pstHost->pszBuff) || (0 >= a_pstHost->iBuff))
    {
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
    }



    if (0 == a_iCutOffVersion)
    {
        a_iCutOffVersion = TDR_MAX_VERSION;
    }
    if (a_pstMeta->iBaseVersion > a_iCutOffVersion)
    {
        a_pstHost->iBuff = 0;
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
    }

    /*创建XML元素树*/
    iRet = tdr_create_XMLParser_tree_byFileName(&pstTree, a_pszFile, stderr);
    if (TDR_ERR_IS_ERROR(iRet))
    {
        return iRet;
    }

    pstRoot = scew_tree_root(pstTree);
    if (NULL == pstRoot)
    {
        a_pstHost->iBuff = 0;
        iRet = TDR_ERRIMPLE_NO_XML_ROOT;
    }

    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_input_i(a_pstMeta, a_pstHost, pstRoot, a_iCutOffVersion, a_iXmlDataFormat);
    }

    scew_tree_free( pstTree );

    return iRet;
}

int tdr_input_by_strict_xml_i(IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstHost, IN scew_element *a_pstRoot,
        IN int a_iCutOffVersion, IN int a_iIOVersion)
{
    int iRet = TDR_SUCCESS;
    LPTDRMETALIB pstLib;
    LPTDRMETA pstCurMeta;
    TDRSTACK  stStack;
    LPTDRSTACKITEM pstStackTop;
    int iStackItemCount;
    int iCutOffVersion;
    char *pszHostStart;
    char *pszHostEnd;
    int iChange;

    assert(NULL != a_pstMeta);
    assert(NULL != a_pstRoot);
    assert(NULL != a_pstHost);
    assert(a_pstMeta->iBaseVersion <= a_iCutOffVersion);


    pszHostStart = a_pstHost->pszBuff;
    pszHostEnd = a_pstHost->pszBuff + a_pstHost->iBuff;
    pstLib = TDR_META_TO_LIB(a_pstMeta);

    pstCurMeta = a_pstMeta;
    pstStackTop = &stStack[0];
    pstStackTop->pszNetBase  = (char *)a_pstRoot; /*the parent node */
    pstStackTop->pszMetaSizeInfoTarget = NULL; /*the current node*/
    pstStackTop->pstMeta = pstCurMeta;
    pstStackTop->pszHostBase = pszHostStart;
    pstStackTop->iCount = 1;
    pstStackTop->idxEntry = 0;
    pstStackTop->iChange = 0;
    pstStackTop->iRealCount = 0; /*实际读取的结构数*/
    TDR_GET_VERSION_INDICATOR_BY_XML(pstCurMeta, a_pstRoot, iCutOffVersion, a_iCutOffVersion);
    pstStackTop->iCutOffVersion = iCutOffVersion;
    pstStackTop->szMetaEntryName[0] = '\0';
    iStackItemCount = 1;
    pstStackTop->pszHostEnd = pszHostEnd;

    iChange = 0;
    while (0 < iStackItemCount)
    {
        LPTDRMETAENTRY pstEntry;
        int iCount;
        scew_element *pstParent;
        scew_element *pstCurNode;


        if (0 != iChange)
        {
            if (1 >= iStackItemCount)
            {
                iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_TOO_COMPLIEX_META);
                break;
            }

            pstStackTop->iRealCount++;  /*已经读取一个结构/uion*/
            if (TDR_XML_DATA_FORMAT_LIST_ENTRY_NAME == a_iIOVersion)
            {
                (pstStackTop -1)->pszMetaSizeInfoTarget = pstStackTop->pszNetBase;
                if (0 < pstStackTop->iCount)
                {
                    TDR_GET_NEXT_ELEMET(pstParent, (pstStackTop-1)->pszNetBase, (pstStackTop-1)->pszMetaSizeInfoTarget, pstStackTop->szMetaEntryName);
                    if (NULL == pstParent)
                    {
                        iChange = 0;
                        TDR_CLEAR_COUNT(pstStackTop, pstCurMeta, pszHostStart, pszHostEnd);
                        continue;
                    }

                    (pstStackTop -1)->pszMetaSizeInfoTarget = (char *)pstParent;
                    pstStackTop->pszNetBase = (char *)pstParent;
                    pstStackTop->pszMetaSizeInfoTarget = NULL;
                }/*if (0 < pstStackTop->iCount)*/
            }else
            {
                if (TDR_TYPE_STRUCT == pstCurMeta->iType)
                {
                    (pstStackTop -1)->pszMetaSizeInfoTarget = pstStackTop->pszNetBase;
                    if (0 < pstStackTop->iCount)
                    {
                        TDR_GET_NEXT_ELEMET(pstParent, (pstStackTop-1)->pszNetBase, (pstStackTop-1)->pszMetaSizeInfoTarget,
                            TDR_GET_STRING_BY_PTR(pstLib,pstCurMeta->ptrName));
                        if (NULL == pstParent)
                        {
                            iChange = 0;
                            TDR_CLEAR_COUNT(pstStackTop, pstCurMeta, pszHostStart, pszHostEnd);
                            continue;
                        }

                        (pstStackTop -1)->pszMetaSizeInfoTarget = (char *)pstParent;
                        pstStackTop->pszNetBase = (char *)pstParent;
                        pstStackTop->pszMetaSizeInfoTarget = NULL;
                    }/*if (0 < pstStackTop->iCount)*/
                }else
                {
                    (pstStackTop -1)->pszMetaSizeInfoTarget = pstStackTop->pszMetaSizeInfoTarget;
                    if (0 < pstStackTop->iCount)
                    {
                        pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;
                        if (TDR_TYPE_STRUCT == pstEntry->iType)
                        {
                            LPTDRMETA pstType = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
                            TDR_GET_NEXT_ELEMET(pstParent, pstStackTop->pszNetBase, pstStackTop->pszMetaSizeInfoTarget,
                                TDR_GET_STRING_BY_PTR(pstLib,pstType->ptrName));
                        }else
                        {
                            TDR_GET_NEXT_ELEMET(pstParent, pstStackTop->pszNetBase, pstStackTop->pszMetaSizeInfoTarget, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName));
                        }
                        if (NULL == pstParent)
                        {
                            iChange = 0;
                            TDR_CLEAR_COUNT(pstStackTop, pstCurMeta, pszHostStart, pszHostEnd);
                            continue;
                        }
                    }/*if (0 < pstStackTop->iCount)*/
                }/*if (TDR_TYPE_STRUCT == pstCurMeta->iType)*/
            }/*if (TDR_XML_DATA_FORMAT_LIST_ENTRY_NAME == a_iIOVersion)*/
        }/*if ((0 != iChange)*/

        iChange = 0;

        if (0 >= pstStackTop->iCount)
        {/*当前元数据数组已经处理完毕*/
            pstStackTop--;
            iStackItemCount--;
            if (0 < iStackItemCount)
            {
                pstCurMeta = pstStackTop->pstMeta;
                pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;
                TDR_SET_ARRAY_REAL_COUNT((pstStackTop + 1)->iRealCount, pstEntry, pstStackTop->pszHostBase);
                TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
            }
            continue;
        }

        pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;

        if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry) || (TDR_ENTRY_IS_REFER_TYPE(pstEntry)))
        {
            TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
            continue;
        }

        /*取出此entry的数组计数信息*/
        TDR_GET_ARRAY_REAL_COUNT(iCount, pstEntry, pstStackTop->pszHostBase, a_iCutOffVersion);
        if (0 > iCount)
        {	/*实际数目为负数则无效*/
            iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_REFER_VALUE);
            break;
        }


        /* 设置最大读取成员数目：
         *  1)如果通过refer属性指定了成员的数目，则最大读取数目按此refer指定数目读取；
         *  2)如果没有通过refer属性指定了成员的数目:
         *		2.1）如果成员数组长度确定(icount > 0),则最大读取数目按成员数组长度设置；
         *		2.2) 如果成员数组长度确定(icount == 0),则最大读取数目设置为最大整数;
         */
        if ((0 < pstEntry->iCount) && (pstEntry->iCount < iCount))
        {
            iCount = pstEntry->iCount;
        }
        if( (0 == iCount) && (pstEntry->iCount == 0))
        {
            iCount = TDR_MAX_INT;
        }


        pszHostStart = pstStackTop->pszHostBase + pstEntry->iHOff;

        if (TDR_IO_NOINPUT & pstEntry->iIO)
        {
            if (TDR_ENTRY_IS_COUNTER(pstEntry))
            {
                LPTDRMETAENTRY pstUsedEntry =  TDR_PTR_TO_ENTRY(pstLib, pstEntry->stRefer.ptrEntry);
                if (pstUsedEntry->iVersion >= a_iCutOffVersion && (!(TDR_IO_NOINPUT & pstUsedEntry->iIO)))
                {
                    TDR_SET_INT(pszHostStart, pstEntry->iHUnitSize, pstUsedEntry->iCount);
                    pszHostStart += pstEntry->iHUnitSize;
                }
            }else
            {
                TDR_SET_DEFAULT_VALUE(iRet, pszHostStart, pszHostEnd, pstLib, pstEntry, iCount);
            }
            TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
            continue;
        }

        pszHostEnd = pstStackTop->pszHostEnd;
        if (pstEntry->iVersion > a_iCutOffVersion)
        {
            TDR_SET_DEFAULT_VALUE(iRet, pszHostStart, pszHostEnd, pstLib, pstEntry, pstEntry->iCount);
            if (TDR_ERR_IS_ERROR(iRet))
            {
                break;
            }
            TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
            continue;
        }


        if (TDR_TYPE_COMPOSITE >= pstEntry->iType)
        {/*复合数据类型*/
            int idxSubEntry;
            LPTDRMETA pstTypeMeta;

            if (TDR_STACK_SIZE <=  iStackItemCount)
            {
                iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_TOO_COMPLIEX_META);
                break;
            }

            if (TDR_TYPE_UNION == pstEntry->iType)
            {
                TDR_GET_UNION_ENTRY_TYPE_META_INFO(pstStackTop->pszHostBase, pstLib, pstEntry, a_iCutOffVersion, pstTypeMeta, idxSubEntry);
                if (NULL == pstTypeMeta)
                {
                    TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
                    continue;
                }
            }else
            {
                pstTypeMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
                idxSubEntry = 0;
            }/*if (TDR_TYPE_UNION == pstEntry->iType)*/

            /*定位节点*/
            if (TDR_XML_DATA_FORMAT_LIST_ENTRY_NAME == a_iIOVersion)
            {
                TDR_GET_NEXT_ELEMET(pstParent, pstStackTop->pszNetBase, pstStackTop->pszMetaSizeInfoTarget, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName));
                pstCurNode = NULL;
            }else
            {
                if (TDR_TYPE_UNION == pstEntry->iType)
                {
                    pstParent = (scew_element *)pstStackTop->pszNetBase;
                    pstCurNode = (scew_element *)pstStackTop->pszMetaSizeInfoTarget;
                }else
                {
                    TDR_GET_NEXT_ELEMET(pstParent, pstStackTop->pszNetBase, pstStackTop->pszMetaSizeInfoTarget,
                        TDR_GET_STRING_BY_PTR(pstLib,pstTypeMeta->ptrName));
                    pstCurNode = NULL;
                }
            }/*if (TDR_XML_DATA_FORMAT_LIST_ENTRY_NAME == a_iIOVersion)*/
            if (NULL == pstParent)
            {
                TDR_SET_ARRAY_REAL_COUNT(0, pstEntry, pstStackTop->pszHostBase);
                TDR_SET_DEFAULT_VALUE(iRet, pszHostStart, pszHostEnd, pstLib, pstEntry, pstEntry->iCount);
                TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
                continue;
            }

            pstCurMeta = pstTypeMeta;
            iStackItemCount++;
            pstStackTop++;
            pstStackTop->pstMeta = pstCurMeta;
            pstStackTop->iCount = iCount;
            pstStackTop->idxEntry = idxSubEntry;
            pstStackTop->pszHostBase = pszHostStart;
            pstStackTop->pszNetBase = (char *)pstParent;
            pstStackTop->pszMetaSizeInfoTarget = (char *)pstCurNode;
            pstStackTop->pszHostEnd = pszHostEnd;

            /*实际读取的结构数*/
            pstStackTop->iRealCount = 0;
            if (TDR_XML_DATA_FORMAT_LIST_ENTRY_NAME == a_iIOVersion)
            {
                TDR_STRNCPY(pstStackTop->szMetaEntryName, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName), sizeof(pstStackTop->szMetaEntryName));
            }
            pstStackTop->iChange = 1;
            continue;
        }/*if (TDR_TYPE_COMPOSITE >= pstEntry->iType)*/

        iRet = tdr_input_simple_entry_i(pstLib, pstEntry, &iCount, (scew_element *)pstStackTop->pszNetBase,
                (scew_element **)&pstStackTop->pszMetaSizeInfoTarget, &pszHostStart, pszHostEnd);
        if (TDR_ERR_IS_ERROR(iRet))
        {
            break;
        }
        TDR_SET_ARRAY_REAL_COUNT(iCount, pstEntry, pstStackTop->pszHostBase);
        TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
    }/*while (0 < iStackItemCount)*/

    a_pstHost->iBuff = pszHostStart - a_pstHost->pszBuff;
    return iRet;
}

static int tdr_output_mata_attrs_i(INOUT LPTDRIOSTREAM a_pstIOStream, OUT int *a_piChildNum, IN LPTDRMETA a_pstMeta, IN int a_iFirstEntry,
        IN const char *a_pszHostStart, IN const char *a_pszHostEnd, int a_iVersion)
{
    int iRet = TDR_SUCCESS;

    TDRSTACKITEM stStackItem;
    LPTDRSTACKITEM pstStackTop;
    int iChange = 0;
    LPTDRMETAENTRY pstEntry;
    char *pszHostStart;
    int iArrayRealCount;
    LPTDRMETALIB pstLib;
    int iChildNum = 0;

    assert(NULL != a_pstIOStream);
    assert(NULL != a_pstMeta);
    assert(NULL != a_pszHostEnd);
    assert(a_iVersion >= a_pstMeta->iBaseVersion);
    assert((0 <= a_iFirstEntry) && (a_iFirstEntry < a_pstMeta->iEntriesNum));


    pstStackTop = &stStackItem;
    pstStackTop->iCount = 1;
    pstStackTop->iChange = 1;
    pstStackTop->idxEntry = a_iFirstEntry;
    pstStackTop->pszHostBase = (char *)a_pszHostStart;
    pstLib = TDR_META_TO_LIB(a_pstMeta);

    while (!iChange)
    {
        pstEntry = a_pstMeta->stEntries + pstStackTop->idxEntry;
        if (pstEntry->iVersion > a_iVersion)
        {
            TDR_GET_NEXT_ENTRY(pstStackTop, a_pstMeta, iChange);
            continue;
        }
        if (TDR_IO_NOOUTPUT & pstEntry->iIO)
        {
            TDR_GET_NEXT_ENTRY(pstStackTop, a_pstMeta, iChange);
            continue;
        }
        if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry)||TDR_ENTRY_IS_REFER_TYPE(pstEntry))
        {
            TDR_GET_NEXT_ENTRY(pstStackTop, a_pstMeta, iChange);
            continue;
        }

        pszHostStart = pstStackTop->pszHostBase + pstEntry->iHOff;

        /*取出此entry的数组计数信息*/
        TDR_GET_ARRAY_REAL_COUNT(iArrayRealCount, pstEntry, pstStackTop->pszHostBase, a_iVersion);
        if ((iArrayRealCount < 0) ||
                ((0 < pstEntry->iCount) && (pstEntry->iCount < iArrayRealCount)))
        {/*实际数目为负数或比数组最大长度要大，则无效*/
            iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_REFER_VALUE);
            break;
        }
        if (0 == iArrayRealCount)
        {
            TDR_GET_NEXT_ENTRY(pstStackTop, a_pstMeta, iChange);
            continue;
        }

        /*复合数据类型成员不以属性方式输出*/
        if (TDR_TYPE_COMPOSITE >= pstEntry->iType)
        {
            TDR_GET_NEXT_ENTRY(pstStackTop, a_pstMeta, iChange);
            iChildNum++;
            continue;
        }/*if (TDR_TYPE_COMPOSITE >= pstEntry->iType)*/

        /*简单数据类型数组成员不以属性方式输出*/
        if (1 != pstEntry->iCount)
        {
            TDR_GET_NEXT_ENTRY(pstStackTop, a_pstMeta, iChange);
            iChildNum++;
            continue;
        }

        /*基本数据类型*/
        iRet = tdr_iostream_write(a_pstIOStream, " %s=\"", TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName));
        if (TDR_TYPE_STRING == pstEntry->iType)
        {
            size_t iSize;
            if (0 < pstEntry->iCustomHUnitSize)
            {
                iSize = pstEntry->iCustomHUnitSize;
            }else
            {
                iSize = a_pszHostEnd - a_pszHostStart;
            }
            iRet = tdr_esc_xml_i(a_pstIOStream, pszHostStart, iSize);
        }else
        {
            iRet = tdr_ioprintf_basedtype_i(a_pstIOStream, pstLib, pstEntry, &pszHostStart,
                    a_pszHostEnd);
        }

        iRet = tdr_iostream_write(a_pstIOStream, "\" ");
        if (TDR_ERR_IS_ERROR(iRet))
        {
            break;
        }

        TDR_GET_NEXT_ENTRY(pstStackTop, a_pstMeta, iChange);
    }/*while (!iChange)*/

    *a_piChildNum = iChildNum;

    return iRet;
}

static int tdr_output_attr_format_t(IN LPTDRMETA a_pstMeta, INOUT LPTDRIOSTREAM a_pstIOStream, IN LPTDRDATA a_pstHost,
        IN int a_iCufOffVersion)
{
    int iRet = TDR_SUCCESS;
    LPTDRMETALIB pstLib;
    LPTDRMETA pstCurMeta;
    TDRSTACK  stStack;
    LPTDRSTACKITEM pstStackTop;
    int iStackItemCount;
    int iCutOffVersion;
    char szSpace[TDR_STACK_SIZE*TDR_TAB_SIZE+1];
    int i;

    char *pszHostStart;
    char *pszHostEnd;
    int iChange;

    assert(NULL != a_pstMeta);
    assert(NULL != a_pstIOStream);
    assert(NULL != a_pstHost);
    assert(a_pstMeta->iBaseVersion <= a_iCufOffVersion);

    pszHostStart = a_pstHost->pszBuff;
    pszHostEnd = a_pstHost->pszBuff + a_pstHost->iBuff;
    pstCurMeta = a_pstMeta;
    pstLib = TDR_META_TO_LIB(a_pstMeta);
    for (i = 0; i < (int)(sizeof(szSpace)); i++)
    {
        szSpace[i] = ' ';
    }
    szSpace[sizeof(szSpace) -1] = '\0';

    pstStackTop = &stStack[0];
    pstStackTop->pstMeta = pstCurMeta;
    pstStackTop->pszHostBase = pszHostStart;
    pstStackTop->pszHostEnd = pszHostEnd;
    pstStackTop->iCount = 1;
    pstStackTop->idxEntry = 0;
    pstStackTop->iChange = 1;
    iCutOffVersion = a_iCufOffVersion;
    if (TDR_ERR_IS_ERROR(iRet))
    {
        a_pstHost->iBuff = 0;
        return iRet;
    }
    pstStackTop->iCutOffVersion = iCutOffVersion;
    pstStackTop->szMetaEntryName[0] = '\0';
    pstStackTop->iMetaSizeInfoOff = 1; /*框架层次*/
    iStackItemCount = 1;

    /*输出根节点*/
    TDR_STRNCPY(pstStackTop->szMetaEntryName, TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), sizeof(pstStackTop->szMetaEntryName));
    iRet = tdr_iostream_write(a_pstIOStream, "<%s ", TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName));
    iRet = tdr_output_mata_attrs_i(a_pstIOStream, &pstStackTop->iRealCount, pstCurMeta, 0, pszHostStart, pszHostEnd, pstStackTop->iCutOffVersion);
    if (0 == pstStackTop->iRealCount)
    {
        iRet = tdr_iostream_write(a_pstIOStream, " __version=\"%d\" />\n", pstStackTop->iCutOffVersion);
    }else
    {
        iRet = tdr_iostream_write(a_pstIOStream, " __version=\"%d\" >\n", pstStackTop->iCutOffVersion);
    }



    iChange = 0;
    while (0 < iStackItemCount)
    {
        LPTDRMETAENTRY pstEntry;
        int iArrayRealCount ;

        if (0 != iChange)
        {
            iChange = 0;
            if (0 < pstStackTop->iRealCount)
            {/*有子节点*/
                szSpace[(pstStackTop->iMetaSizeInfoOff-1)*TDR_TAB_SIZE] = '\0';
                iRet = tdr_iostream_write(a_pstIOStream, "%s</%s>\n", szSpace, pstStackTop->szMetaEntryName);
                szSpace[(pstStackTop->iMetaSizeInfoOff-1)*TDR_TAB_SIZE] = ' ';
                if (TDR_ERR_IS_ERROR(iRet))
                {
                    break;
                }
                pstStackTop->iRealCount = 0;
            }/*if (0 < pstStackTop->iCode)*/


            if (0 < pstStackTop->iCount)
            {
                szSpace[(pstStackTop->iMetaSizeInfoOff-1)*TDR_TAB_SIZE] = '\0';
                pszHostStart = pstStackTop->pszHostBase;
                iRet = tdr_iostream_write(a_pstIOStream, "%s<%s ",
                        szSpace, pstStackTop->szMetaEntryName);
                iRet = tdr_output_mata_attrs_i(a_pstIOStream, &pstStackTop->iRealCount, pstCurMeta, pstStackTop->idxEntry,
                        pszHostStart, pszHostEnd, pstStackTop->iCutOffVersion);
                if (0 == pstStackTop->iRealCount)
                {
                    iRet = tdr_iostream_write(a_pstIOStream, " />\n");
                }else
                {
                    iRet = tdr_iostream_write(a_pstIOStream, " >\n");
                }
                szSpace[(pstStackTop->iMetaSizeInfoOff-1)*TDR_TAB_SIZE] = ' ';
                if (TDR_ERR_IS_ERROR(iRet))
                {
                    break;
                }
            }/*if (0 < pstStackTop->iCount)*/
        }/*if ((0 != iChange) && (TDR_TYPE_STRUCT == pstCurMeta->iType))*/

        if (0 >= pstStackTop->iCount)
        {/*当前元数据数组已经处理完毕*/
            pstStackTop--;
            iStackItemCount--;
            if (0 < iStackItemCount)
            {
                pstCurMeta = pstStackTop->pstMeta;
                TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
            }
            continue;
        }

        pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;
        if ((TDR_TYPE_COMPOSITE < pstEntry->iType) && (1 == pstEntry->iCount))
        {
            TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
            continue; /*简单非数组成员以属性的方式输出*/
        }
        if (pstEntry->iVersion > pstStackTop->iCutOffVersion)
        {
            TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
            continue;
        }
        if (TDR_IO_NOOUTPUT & pstEntry->iIO)
        {
            TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
            continue;
        }
        if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry)||TDR_ENTRY_IS_REFER_TYPE(pstEntry))
        {
            TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
            continue;
        }



        pszHostStart = pstStackTop->pszHostBase + pstEntry->iHOff;

        /*取出此entry的数组计数信息*/
        TDR_GET_ARRAY_REAL_COUNT(iArrayRealCount, pstEntry, pstStackTop->pszHostBase, pstStackTop->iCutOffVersion);
        if ((iArrayRealCount < 0) ||
                ((0 < pstEntry->iCount) && (pstEntry->iCount < iArrayRealCount)))
        {/*实际数目为负数或比数组最大长度要大，则无效*/
            iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_REFER_VALUE);
            break;
        }
        if (0 == iArrayRealCount)
        {
            TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
            continue;
        }


        if (TDR_TYPE_COMPOSITE >= pstEntry->iType)
        {
            /*复合数据类型*/
            int idxSubEntry;
            LPTDRMETA pstTypeMeta;


            if (TDR_STACK_SIZE <=  iStackItemCount)
            {
                iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_TOO_COMPLIEX_META);
                break;
            }

            if (TDR_TYPE_UNION == pstEntry->iType)
            {
                TDR_GET_UNION_ENTRY_TYPE_META_INFO(pstStackTop->pszHostBase, pstLib, pstEntry,pstStackTop->iCutOffVersion, pstTypeMeta, idxSubEntry);
                if (NULL == pstTypeMeta)
                {
                    TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
                    continue;
                }
            }else
            {
                pstTypeMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
                idxSubEntry = 0;
            }

            pstCurMeta = pstTypeMeta;
            iStackItemCount++;
            pstStackTop++;
            pstStackTop->pstMeta = pstCurMeta;
            pstStackTop->iCount = iArrayRealCount;
            pstStackTop->idxEntry = idxSubEntry;
            pstStackTop->pszHostBase = pszHostStart;
            iCutOffVersion = a_iCufOffVersion;
            if (TDR_ERR_IS_ERROR(iRet))
            {
                break;
            }
            pstStackTop->iCutOffVersion = iCutOffVersion;
            pstStackTop->iMetaSizeInfoOff = (pstStackTop - 1)->iMetaSizeInfoOff + 1;
            pstStackTop->iChange = 1;
            TDR_STRNCPY(pstStackTop->szMetaEntryName, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName), sizeof(pstStackTop->szMetaEntryName));
            szSpace[(pstStackTop->iMetaSizeInfoOff-1)*TDR_TAB_SIZE] = '\0';
            iRet = tdr_iostream_write(a_pstIOStream, "%s<%s ",
                    szSpace, pstStackTop->szMetaEntryName);
            iRet = tdr_output_mata_attrs_i(a_pstIOStream, &pstStackTop->iRealCount, pstCurMeta, pstStackTop->idxEntry,
                    pszHostStart, pszHostEnd, pstStackTop->iCutOffVersion);
            if (0 == pstStackTop->iRealCount)
            {
                iRet = tdr_iostream_write(a_pstIOStream, " />\n");
            }else
            {
                iRet = tdr_iostream_write(a_pstIOStream, " >\n");
            }
            szSpace[(pstStackTop->iMetaSizeInfoOff-1)*TDR_TAB_SIZE] = ' ';
            if (TDR_ERR_IS_ERROR(iRet))
            {
                break;
            }

            continue;
        }/*if (TDR_TYPE_COMPOSITE >= pstEntry->iType)*/

        /*基本数据类型数组成员*/
        szSpace[pstStackTop->iMetaSizeInfoOff*TDR_TAB_SIZE] = '\0';
        iRet = tdr_output_simpleentry_i(a_pstIOStream, pstLib, pstEntry, iArrayRealCount,
                pszHostStart, pszHostEnd, &szSpace[0]);
        szSpace[pstStackTop->iMetaSizeInfoOff*TDR_TAB_SIZE] = ' ';
        if (TDR_ERR_IS_ERROR(iRet))
        {
            break;
        }

        TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);

    }/*while (0 < iStackItemCount)*/

    return iRet;
}

int tdr_output_json(IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstJson,
                    IN LPTDRDATA a_pstHost, IN int a_iCutOffVersion)
{
    int iRet = TDR_SUCCESS;
    TDRIOSTREAM stIOStream;

    if ((NULL == a_pstMeta) || (NULL == a_pstJson) || (NULL == a_pstHost))
    {
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
    }
    if ((NULL == a_pstJson->pszBuff) || (0 >= a_pstJson->iBuff) ||
            (NULL == a_pstHost->pszBuff) || (0 >= a_pstHost->iBuff))
    {
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
    }
    if ((0 == a_iCutOffVersion) || (a_iCutOffVersion > a_pstMeta->iCurVersion))
    {
        a_iCutOffVersion = a_pstMeta->iCurVersion;;
    }
    if (a_pstMeta->iBaseVersion > a_iCutOffVersion)
    {
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
    }


    stIOStream.emIOStreamType = TDR_IOSTREAM_STRBUF;
    stIOStream.pszTDRIOBuff = a_pstJson->pszBuff;
    stIOStream.iTDRIOBuffLen = a_pstJson->iBuff;

    iRet = tdr_iostream_write(&stIOStream, "{");
    if (TDR_SUCCESS == iRet)
    {
        iRet = tdr_output_json_i(a_pstMeta, &stIOStream, a_pstHost, a_iCutOffVersion);
    }

    if (TDR_SUCCESS == iRet)
    {
        iRet = tdr_iostream_write(&stIOStream, "}");
    }

    a_pstJson->iBuff = stIOStream.pszTDRIOBuff - a_pstJson->pszBuff;

    return iRet;
}

int tdr_output_json_fp(IN LPTDRMETA a_pstMeta, IN FILE *a_fpJson,
                       IN LPTDRDATA a_pstHost, IN int a_iCutOffVersion)
{
    int iRet = TDR_SUCCESS;
    TDRIOSTREAM stIOStream;

    if ((NULL == a_pstMeta) || (NULL == a_fpJson) || (NULL == a_pstHost))
    {
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
    }
    if ((NULL == a_pstHost->pszBuff) || (0 >= a_pstHost->iBuff))
    {
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
    }
    if ((0 == a_iCutOffVersion) || (a_iCutOffVersion > a_pstMeta->iCurVersion))
    {
        a_iCutOffVersion = a_pstMeta->iCurVersion;;
    }
    if (a_pstMeta->iBaseVersion > a_iCutOffVersion)
    {
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
    }


    stIOStream.emIOStreamType = TDR_IOSTREAM_FILE;
    stIOStream.fpTDRIO = a_fpJson;

    iRet = tdr_iostream_write(&stIOStream, "{");
    if (TDR_SUCCESS == iRet)
    {
        iRet = tdr_output_json_i(a_pstMeta, &stIOStream, a_pstHost, a_iCutOffVersion);
    }

    if (TDR_SUCCESS == iRet)
    {
        iRet = tdr_iostream_write(&stIOStream, "}");
    }

    return iRet;
}

int tdr_output_json_file(IN LPTDRMETA a_pstMeta, IN const char *a_pszFile,
                         IN LPTDRDATA a_pstHost, IN int a_iCutOffVersion)
{
    int iRet = TDR_SUCCESS;
    TDRIOSTREAM stIOStream;
    FILE* fpOutput = NULL;

    if ((NULL == a_pstMeta) || (NULL == a_pszFile) || (NULL == a_pstHost))
    {
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
    }
    if ((NULL == a_pstHost->pszBuff) || (0 >= a_pstHost->iBuff))
    {
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
    }
    if ((0 == a_iCutOffVersion) || (a_iCutOffVersion > a_pstMeta->iCurVersion))
    {
        a_iCutOffVersion = a_pstMeta->iCurVersion;;
    }
    if (a_pstMeta->iBaseVersion > a_iCutOffVersion)
    {
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
    }

    fpOutput = fopen(a_pszFile, "w");
    if (NULL == fpOutput)
    {
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_FAILED_OPEN_FILE_TO_WRITE);
    }

    stIOStream.emIOStreamType = TDR_IOSTREAM_FILE;
    stIOStream.fpTDRIO = fpOutput;

    iRet = tdr_iostream_write(&stIOStream, "{");
    if (TDR_SUCCESS == iRet)
    {
        iRet = tdr_output_json_i(a_pstMeta, &stIOStream, a_pstHost, a_iCutOffVersion);
    }

    if (TDR_SUCCESS == iRet)
    {
        iRet = tdr_iostream_write(&stIOStream, "}");
    }

    fclose(fpOutput);

    return iRet;
}

int tdr_output(IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstXml, IN LPTDRDATA a_pstHost,
        IN int a_iCutOffVersion, IN int a_iXmlDataFormat)
{
    int iRet = TDR_SUCCESS;
    TDRIOSTREAM stIOStream;

    /*assert(NULL != a_pstMeta);
      assert(NULL != a_pstXml);
      assert(NULL != a_pstXml->pszBuff);
      assert(0 < a_pstXml->iBuff);
      assert(NULL != a_pstHost);
      assert(NULL != a_pstHost->pszBuff);
      assert(0 < a_pstHost->iBuff);*/
    if ((NULL == a_pstMeta) || (NULL == a_pstXml)|| (NULL == a_pstHost))
    {
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
    }
    if ((NULL == a_pstXml->pszBuff) || (0 >= a_pstXml->iBuff) ||
            (NULL == a_pstHost->pszBuff) || (0 >= a_pstHost->iBuff))
    {
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
    }
    if ((0 == a_iCutOffVersion) || (a_iCutOffVersion > a_pstMeta->iCurVersion))
    {
        a_iCutOffVersion = a_pstMeta->iCurVersion;;
    }
    if (a_pstMeta->iBaseVersion > a_iCutOffVersion)
    {
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
    }



    stIOStream.emIOStreamType = TDR_IOSTREAM_STRBUF;
    stIOStream.pszTDRIOBuff = a_pstXml->pszBuff;
    stIOStream.iTDRIOBuffLen = a_pstXml->iBuff;

    switch(a_iXmlDataFormat)
    {
        case TDR_XML_DATA_FORMAT_LIST_MATA_NAME:
            iRet = tdr_output_oldversion_i(a_pstMeta, &stIOStream, a_pstHost, a_iCutOffVersion);
            break;
        case TDR_XML_DATA_FORMAT_ATTR_ENTRY_NAME:
            iRet = tdr_output_attr_format_t(a_pstMeta, &stIOStream, a_pstHost, a_iCutOffVersion);
            break;
        default:
            iRet = tdr_output_i(a_pstMeta, &stIOStream, a_pstHost, a_iCutOffVersion);
            break;
    }
    a_pstXml->iBuff = stIOStream.pszTDRIOBuff - a_pstXml->pszBuff;


    return iRet;
}

int tdr_output_fp(IN LPTDRMETA a_pstMeta, IN FILE *a_fpXml, IN LPTDRDATA a_pstHost,
        IN int a_iCutOffVersion, IN int a_iXmlDataFormat)
{
    int iRet = TDR_SUCCESS;
    TDRIOSTREAM stIOStream;

    /*assert(NULL != a_pstMeta);
      assert(NULL != a_fpXml);
      assert(NULL != a_pstHost);
      assert(NULL != a_pstHost->pszBuff);
      assert(0 < a_pstHost->iBuff);*/
    if ((NULL == a_pstMeta) || (NULL == a_fpXml)|| (NULL == a_pstHost))
    {
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
    }
    if ((NULL == a_pstHost->pszBuff) || (0 >= a_pstHost->iBuff))
    {
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
    }


    if ((0 == a_iCutOffVersion) || (a_iCutOffVersion > a_pstMeta->iCurVersion))
    {
        a_iCutOffVersion = a_pstMeta->iCurVersion;;
    }
    if (a_pstMeta->iBaseVersion > a_iCutOffVersion)
    {
        a_pstHost->iBuff = 0;
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
    }



    stIOStream.emIOStreamType = TDR_IOSTREAM_FILE;
    stIOStream.fpTDRIO = a_fpXml;

    switch(a_iXmlDataFormat)
    {
        case TDR_XML_DATA_FORMAT_LIST_MATA_NAME:
            iRet = tdr_output_oldversion_i(a_pstMeta, &stIOStream, a_pstHost, a_iCutOffVersion);
            break;
        case TDR_XML_DATA_FORMAT_ATTR_ENTRY_NAME:
            iRet = tdr_output_attr_format_t(a_pstMeta, &stIOStream, a_pstHost, a_iCutOffVersion);
            break;
        default:
            iRet = tdr_output_i(a_pstMeta, &stIOStream, a_pstHost, a_iCutOffVersion);
            break;
    }

    return iRet;
}

int tdr_output_file(IN LPTDRMETA a_pstMeta, IN const char *a_szFile, IN LPTDRDATA a_pstHost,
        IN int a_iCutOffVersion, IN int a_iXmlDataFormat)
{
    int iRet = TDR_SUCCESS;
    TDRIOSTREAM stIOStream;
    FILE *fp;

    /*assert(NULL != a_pstMeta);
      assert(NULL != a_szFile);
      assert(NULL != a_pstHost);
      assert(NULL != a_pstHost->pszBuff);
      assert(0 < a_pstHost->iBuff);*/
    if ((NULL == a_pstMeta) || (NULL == a_szFile)|| (NULL == a_pstHost))
    {
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
    }
    if ((NULL == a_pstHost->pszBuff) || (0 >= a_pstHost->iBuff))
    {
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
    }

    if ((0 == a_iCutOffVersion) || (a_iCutOffVersion > a_pstMeta->iCurVersion))
    {
        a_iCutOffVersion = a_pstMeta->iCurVersion;;
    }
    if (a_pstMeta->iBaseVersion > a_iCutOffVersion)
    {
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
    }




    fp = fopen(a_szFile, "w");
    if (NULL == fp)
    {
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_FAILED_OPEN_FILE_TO_WRITE);
    }


    fprintf(fp,"<?xml version=\"1.0\" encoding=\"%s\" standalone=\"yes\" ?>\n",
            g_szEncoding);

    stIOStream.emIOStreamType = TDR_IOSTREAM_FILE;
    stIOStream.fpTDRIO = fp;
    switch(a_iXmlDataFormat)
    {
        case TDR_XML_DATA_FORMAT_LIST_MATA_NAME:
            iRet = tdr_output_oldversion_i(a_pstMeta, &stIOStream, a_pstHost, a_iCutOffVersion);
            break;
        case TDR_XML_DATA_FORMAT_ATTR_ENTRY_NAME:
            iRet = tdr_output_attr_format_t(a_pstMeta, &stIOStream, a_pstHost, a_iCutOffVersion);
            break;
        default:
            iRet = tdr_output_i(a_pstMeta, &stIOStream, a_pstHost, a_iCutOffVersion);
            break;
    }


    fclose(fp);

    return iRet;
}

int tdr_esc_xml_i(INOUT LPTDRIOSTREAM a_pstIOStream, IN const char *a_pszHostStart, IN ptrdiff_t a_iSize)
{
    char *pszXml = NULL;
    ptrdiff_t iXmlLen;
    ptrdiff_t iDataLen;
    const char *pSrc;
    char *pDst;
    int iRet =0;

    assert(NULL != a_pstIOStream);
    assert(NULL != a_pszHostStart);
    assert(0 < a_iSize);

    iXmlLen = 0;
    iDataLen = 0;
    pSrc = a_pszHostStart;
    do
    {
        switch(*pSrc)
        {
            case '<': /*&lt;*/
            case '>':/*&gt;*/
                iXmlLen += 4;
                break;
            case '&':/*&amp;*/
                iXmlLen += 5;
                break;
            case '\'': /*&apos;*/
            case '"':/*&quot;*/
                iXmlLen += 6;
                break;
            default:
                /*非转义字符*/
                iXmlLen++;
        }/*switch(*pSrc)*/
        pSrc++;
        iDataLen++;
    }while ((iDataLen < a_iSize) && (*pSrc != '\0'));
    if ((iDataLen >= a_iSize) && (*(--pSrc) != '\0'))
    {
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NET_INVALID_STRING_LEN);
    }


    if (iXmlLen <= iDataLen)
    {
        /*不需要 对xml转换字符进行处理*/
        return tdr_iostream_write(a_pstIOStream, "%s", a_pszHostStart);
    }


    /*需要对xml转换字符进行处理*/
    iXmlLen++; // for string end of '\0'
    pszXml = malloc(iXmlLen);
    if (NULL == pszXml)
    {
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_MEMORY);
    }
    pSrc = a_pszHostStart;
    pDst = pszXml;
    do
    {
        switch(*pSrc)
        {
            case '<': /*&lt;*/
                *pDst++ = '&';
                *pDst++ = 'l';
                *pDst++ = 't';
                *pDst++ = ';';
                break;
            case '>':/*&gt;*/
                *pDst++ = '&';
                *pDst++ = 'g';
                *pDst++ = 't';
                *pDst++ = ';';
                break;
            case '&':/*&amp;*/
                *pDst++ = '&';
                *pDst++ = 'a';
                *pDst++ = 'm';
                *pDst++ = 'p';
                *pDst++ = ';';
                break;
            case '\'': /*&apos;*/
                *pDst++ = '&';
                *pDst++ = 'a';
                *pDst++ = 'p';
                *pDst++ = 'o';
                *pDst++ = 's';
                *pDst++ = ';';
                break;
            case '"':/*&quot;*/
                *pDst++ = '&';
                *pDst++ = 'q';
                *pDst++ = 'u';
                *pDst++ = 'o';
                *pDst++ = 't';
                *pDst++ = ';';
                break;
            default:
                /*非转义字符*/
                *pDst++ = *pSrc;
        }/*switch(*pSrc)*/
        pSrc++;
    }while (*pSrc != '\0');
    *pDst = '\0';


    iRet =  tdr_iostream_write(a_pstIOStream, "%s", pszXml);
    free(pszXml);


    return iRet;
}
int tdr_output_simpleentry_i(INOUT LPTDRIOSTREAM a_pstIOStream, IN LPTDRMETALIB a_pstLib,
        IN LPTDRMETAENTRY a_pstEntry, IN int a_iCount,
        IN char *a_pszHostStart, IN char *a_pszHostEnd, IN char *a_pszSpace)
{
    int iRet = TDR_SUCCESS;
    int i;

    assert(NULL != a_pstLib);
    assert(NULL != a_pstEntry);
    assert(NULL != a_pszHostStart);
    assert(NULL != a_pszHostEnd);
    assert(NULL != a_pszSpace);


    switch(a_pstEntry->iType)
    {
        case TDR_TYPE_STRING:
            {
                size_t iSize;
                if (0 < a_pstEntry->iCustomHUnitSize)
                {
                    iSize = a_pstEntry->iCustomHUnitSize;
                }else
                {
                    iSize = a_pszHostEnd - a_pszHostStart;
                }
                for (i = 0; i < a_iCount; i++)
                {
                    /*对xml转换字符进行处理*/
                    iRet = tdr_iostream_write(a_pstIOStream, "%s<%s>",a_pszSpace, TDR_GET_STRING_BY_PTR(a_pstLib,a_pstEntry->ptrName));
                    iRet = tdr_esc_xml_i(a_pstIOStream, a_pszHostStart, iSize);
                    iRet = tdr_iostream_write(a_pstIOStream, "</%s>\n", TDR_GET_STRING_BY_PTR(a_pstLib,a_pstEntry->ptrName));
                    if (TDR_ERR_IS_ERROR(iRet))
                    {
                        break;
                    }
                    a_pszHostStart += iSize;
                }/*for (i = 0; i < a_iCount; i++)	*/
                break;
            }
        case TDR_TYPE_DATETIME:
            {
                if((a_pszHostStart + a_pstEntry->iHUnitSize*a_iCount) > a_pszHostEnd)
                {
                    iRet =	TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);
                    break;
                }
                for (i = 0; i < a_iCount; i++)
                {
                    char szDateTime[32]={0};
                    iRet = tdr_iostream_write(a_pstIOStream, "%s<%s>",a_pszSpace, TDR_GET_STRING_BY_PTR(a_pstLib,a_pstEntry->ptrName));
                    iRet = tdr_iostream_write(a_pstIOStream, "%s",
                            tdr_tdrdatetime_to_str_r((tdr_datetime_t *)a_pszHostStart,szDateTime,sizeof(szDateTime)));
                    a_pszHostStart += a_pstEntry->iHUnitSize;
                    iRet = tdr_iostream_write(a_pstIOStream, "</%s>\n", TDR_GET_STRING_BY_PTR(a_pstLib,a_pstEntry->ptrName));
                    if (TDR_ERR_IS_ERROR(iRet))
                    {
                        break;
                    }
                }/*for (i = 0; i < a_iCount; i++)	*/
            }
            break;
        default:
            {
                if((a_pszHostStart + a_pstEntry->iHUnitSize*a_iCount) > a_pszHostEnd)
                {
                    iRet =	TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);
                    break;
                }

                iRet = tdr_iostream_write(a_pstIOStream, "%s<%s>",a_pszSpace, TDR_GET_STRING_BY_PTR(a_pstLib,a_pstEntry->ptrName));
                for (i = 0; i < a_iCount; i++)
                {
                    iRet = tdr_ioprintf_basedtype_i(a_pstIOStream, a_pstLib, a_pstEntry, &a_pszHostStart, a_pszHostEnd);
                    iRet = tdr_iostream_write(a_pstIOStream, " ");
                }/*for (i = 0; i < a_iCount; i++)	*/
                iRet = tdr_iostream_write(a_pstIOStream, "</%s>\n", TDR_GET_STRING_BY_PTR(a_pstLib,a_pstEntry->ptrName));
            }
            break;
    }/*switch(a_pstEntry->iType)*/

    return iRet;
}

int tdr_output_i(IN LPTDRMETA a_pstMeta, INOUT LPTDRIOSTREAM a_pstIOStream, IN LPTDRDATA a_pstHost,
        IN int a_iCufOffVersion)
{
    int iRet = TDR_SUCCESS;
    LPTDRMETALIB pstLib;
    LPTDRMETA pstCurMeta;
    TDRSTACK  stStack;
    LPTDRSTACKITEM pstStackTop;
    int iStackItemCount;
    int iCutOffVersion;
    char szSpace[TDR_STACK_SIZE*TDR_TAB_SIZE+1];
    int i;

    char *pszHostStart;
    char *pszHostEnd;
    int iChange;

    assert(NULL != a_pstMeta);
    assert(NULL != a_pstIOStream);
    assert(NULL != a_pstHost);
    assert(a_pstMeta->iBaseVersion <= a_iCufOffVersion);

    pszHostStart = a_pstHost->pszBuff;
    pszHostEnd = a_pstHost->pszBuff + a_pstHost->iBuff;
    pstCurMeta = a_pstMeta;
    pstLib = TDR_META_TO_LIB(a_pstMeta);
    for (i = 0; i < (int)(sizeof(szSpace)); i++)
    {
        szSpace[i] = ' ';
    }
    szSpace[sizeof(szSpace) -1] = '\0';

    pstStackTop = &stStack[0];
    pstStackTop->pstMeta = pstCurMeta;
    pstStackTop->pszHostBase = pszHostStart;
    pstStackTop->iCount = 1;
    pstStackTop->idxEntry = 0;
    pstStackTop->iChange = 0;
    pstStackTop->iCutOffVersion = a_iCufOffVersion;
    if (TDR_ERR_IS_ERROR(iRet))
    {
        a_pstHost->iBuff = 0;
        return iRet;
    }
    pstStackTop->szMetaEntryName[0] = '\0';
    pstStackTop->iMetaSizeInfoOff = 1; /*框架层次*/
    iStackItemCount = 1;



    iRet = tdr_iostream_write(a_pstIOStream, "<%s version=\"%d\">\n", TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), pstStackTop->iCutOffVersion);
    if (TDR_ERR_IS_ERROR(iRet))
    {
        return iRet;
    }

    iChange = 0;
    while (0 < iStackItemCount)
    {
        LPTDRMETAENTRY pstEntry;
        int iArrayRealCount ;

        if (0 != iChange)
        {
            iChange = 0;
            szSpace[(pstStackTop->iMetaSizeInfoOff-1)*TDR_TAB_SIZE] = '\0';
            iRet = tdr_iostream_write(a_pstIOStream, "%s</%s>\n", szSpace, pstStackTop->szMetaEntryName);
            szSpace[(pstStackTop->iMetaSizeInfoOff-1)*TDR_TAB_SIZE] = ' ';
            if (TDR_ERR_IS_ERROR(iRet))
            {
                break;
            }

            if (0 < pstStackTop->iCount)
            {
                szSpace[(pstStackTop->iMetaSizeInfoOff-1)*TDR_TAB_SIZE] = '\0';
                if (a_iCufOffVersion != pstStackTop->iCutOffVersion)
                {
                    iRet = tdr_iostream_write(a_pstIOStream, "%s<%s type=\"%s\" version=\"%d\">\n",
                            szSpace, pstStackTop->szMetaEntryName, TDR_GET_STRING_BY_PTR(pstLib,pstCurMeta->ptrName), pstStackTop->iCutOffVersion);
                }else
                {
                    iRet = tdr_iostream_write(a_pstIOStream, "%s<%s type=\"%s\">\n",
                            szSpace, pstStackTop->szMetaEntryName, TDR_GET_STRING_BY_PTR(pstLib,pstCurMeta->ptrName));
                }
                szSpace[(pstStackTop->iMetaSizeInfoOff-1)*TDR_TAB_SIZE] = ' ';
                if (TDR_ERR_IS_ERROR(iRet))
                {
                    break;
                }
            }/*if (0 < pstStackTop->iCount)*/
        }/*if ((0 != iChange) && (TDR_TYPE_STRUCT == pstCurMeta->iType))*/

        if (0 >= pstStackTop->iCount)
        {/*当前元数据数组已经处理完毕*/
            pstStackTop--;
            iStackItemCount--;
            if (0 < iStackItemCount)
            {
                pstCurMeta = pstStackTop->pstMeta;
                TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
            }
            continue;
        }

        pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;
        if (pstEntry->iVersion > pstStackTop->iCutOffVersion)
        {
            TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
            continue;
        }
        if (TDR_IO_NOOUTPUT & pstEntry->iIO)
        {
            TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
            continue;
        }
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
        pszHostStart = pstStackTop->pszHostBase + pstEntry->iHOff;

        /*取出此entry的数组计数信息*/
        TDR_GET_ARRAY_REAL_COUNT(iArrayRealCount, pstEntry, pstStackTop->pszHostBase, pstStackTop->iCutOffVersion);
        if ((iArrayRealCount < 0) ||
                ((0 < pstEntry->iCount) && (pstEntry->iCount < iArrayRealCount)))
        {/*实际数目为负数或比数组最大长度要大，则无效*/
            iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_REFER_VALUE);
            break;
        }
        if (0 == iArrayRealCount)
        {
            TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
            continue;
        }


        if (TDR_TYPE_COMPOSITE >= pstEntry->iType)
        {
            /*复合数据类型*/
            int idxSubEntry;
            LPTDRMETA pstTypeMeta;


            if (TDR_STACK_SIZE <=  iStackItemCount)
            {
                iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_TOO_COMPLIEX_META);
                break;
            }

            if (TDR_TYPE_UNION == pstEntry->iType)
            {
                TDR_GET_UNION_ENTRY_TYPE_META_INFO(pstStackTop->pszHostBase, pstLib, pstEntry,
                        pstStackTop->iCutOffVersion,pstTypeMeta, idxSubEntry);
                if (NULL == pstTypeMeta)
                {
                    TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
                    continue;
                }
            }else
            {
                pstTypeMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
                idxSubEntry = 0;
            }

            pstCurMeta = pstTypeMeta;
            iStackItemCount++;
            pstStackTop++;
            pstStackTop->pstMeta = pstCurMeta;
            pstStackTop->iCount = iArrayRealCount;
            pstStackTop->idxEntry = idxSubEntry;
            pstStackTop->pszHostBase = pszHostStart;
            iCutOffVersion = a_iCufOffVersion;
            if (TDR_ERR_IS_ERROR(iRet))
            {
                break;
            }
            pstStackTop->iCutOffVersion = iCutOffVersion;
            pstStackTop->iMetaSizeInfoOff = (pstStackTop - 1)->iMetaSizeInfoOff + 1;
            pstStackTop->iChange = 1;
            szSpace[(pstStackTop->iMetaSizeInfoOff-1)*TDR_TAB_SIZE] = '\0';
            if (a_iCufOffVersion != pstStackTop->iCutOffVersion)
            {
                iRet = tdr_iostream_write(a_pstIOStream, "%s<%s type=\"%s\" version=\"%d\">\n",
                        szSpace, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName), TDR_GET_STRING_BY_PTR(pstLib,pstTypeMeta->ptrName), pstStackTop->iCutOffVersion);
            }else
            {
                iRet = tdr_iostream_write(a_pstIOStream, "%s<%s type=\"%s\">\n",
                        szSpace, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName), TDR_GET_STRING_BY_PTR(pstLib,pstTypeMeta->ptrName));
            }
            TDR_STRNCPY(pstStackTop->szMetaEntryName, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName), sizeof(pstStackTop->szMetaEntryName));
            szSpace[(pstStackTop->iMetaSizeInfoOff-1)*TDR_TAB_SIZE] = ' ';
            if (TDR_ERR_IS_ERROR(iRet))
            {
                break;
            }

            continue;
        }/*if (TDR_TYPE_COMPOSITE >= pstEntry->iType)*/

        /*基本数据类型*/
        szSpace[pstStackTop->iMetaSizeInfoOff*TDR_TAB_SIZE] = '\0';
        iRet = tdr_output_simpleentry_i(a_pstIOStream, pstLib, pstEntry, iArrayRealCount,
                pszHostStart, pszHostEnd, &szSpace[0]);
        szSpace[pstStackTop->iMetaSizeInfoOff*TDR_TAB_SIZE] = ' ';
        if (TDR_ERR_IS_ERROR(iRet))
        {
            break;
        }

        TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);

    }/*while (0 < iStackItemCount)*/


    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_iostream_write(a_pstIOStream, "</%s>\n", TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName));
    }
    return iRet;
}


int tdr_output_oldversion_i(IN LPTDRMETA a_pstMeta, INOUT LPTDRIOSTREAM a_pstIOStream, IN LPTDRDATA a_pstHost,
        IN int a_iCufOffVersion)
{
    int iRet = TDR_SUCCESS;
    LPTDRMETALIB pstLib;
    LPTDRMETA pstCurMeta;
    TDRSTACK  stStack;
    LPTDRSTACKITEM pstStackTop;
    int iStackItemCount;
    char szSpace[TDR_STACK_SIZE*TDR_TAB_SIZE+1];
    int i;

    char *pszHostStart;
    char *pszHostEnd;
    int iChange;

    assert(NULL != a_pstMeta);
    assert(NULL != a_pstIOStream);
    assert(NULL != a_pstHost);
    assert(a_pstMeta->iBaseVersion <= a_iCufOffVersion);

    pszHostStart = a_pstHost->pszBuff;
    pszHostEnd = a_pstHost->pszBuff + a_pstHost->iBuff;
    pstCurMeta = a_pstMeta;
    pstLib = TDR_META_TO_LIB(a_pstMeta);
    for (i = 0; i < (int)(sizeof(szSpace)); i++)
    {
        szSpace[i] = ' ';
    }
    szSpace[sizeof(szSpace) -1] = '\0';

    pstStackTop = &stStack[0];
    pstStackTop->pstMeta = pstCurMeta;
    pstStackTop->pszHostBase = pszHostStart;
    pstStackTop->iCount = 1;
    pstStackTop->idxEntry = 0;
    pstStackTop->iChange = 0;
    pstStackTop->szMetaEntryName[0] = '\0';
    pstStackTop->iMetaSizeInfoOff = 1; /*框架层次*/
    iStackItemCount = 1;
    pstStackTop->iCutOffVersion = a_iCufOffVersion;
    if (TDR_ERR_IS_ERROR(iRet))
    {
        a_pstHost->iBuff = 0;
        return iRet;
    }



    iRet = tdr_iostream_write(a_pstIOStream, "<%s version=\"%d\">\n", TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), a_iCufOffVersion);
    if (TDR_ERR_IS_ERROR(iRet))
    {
        return iRet;
    }

    iChange = 0;
    while (0 < iStackItemCount)
    {
        LPTDRMETAENTRY pstEntry;
        int iArrayRealCount ;

        if (0 != iChange)
        {
            iChange = 0;
            szSpace[(pstStackTop->iMetaSizeInfoOff-1)*TDR_TAB_SIZE] = '\0';
            iRet = tdr_iostream_write(a_pstIOStream, "%s</%s>\n", szSpace, TDR_GET_STRING_BY_PTR(pstLib,pstCurMeta->ptrName));
            szSpace[(pstStackTop->iMetaSizeInfoOff-1)*TDR_TAB_SIZE] = ' ';
            if (TDR_ERR_IS_ERROR(iRet))
            {
                break;
            }

            if (0 < pstStackTop->iCount)
            {
                szSpace[(pstStackTop->iMetaSizeInfoOff-1)*TDR_TAB_SIZE] = '\0';
                if (a_iCufOffVersion != pstStackTop->iCutOffVersion)
                {
                    iRet = tdr_iostream_write(a_pstIOStream, "%s<%s version=\"%d\">\n",
                            szSpace, TDR_GET_STRING_BY_PTR(pstLib,pstCurMeta->ptrName), pstStackTop->iCutOffVersion);
                }else
                {
                    iRet = tdr_iostream_write(a_pstIOStream, "%s<%s>\n",
                            szSpace, TDR_GET_STRING_BY_PTR(pstLib,pstCurMeta->ptrName));
                }
                szSpace[(pstStackTop->iMetaSizeInfoOff-1)*TDR_TAB_SIZE] = ' ';
                if (TDR_ERR_IS_ERROR(iRet))
                {
                    break;
                }
            }/*if (0 < pstStackTop->iCount)*/
        }/*if ((0 != iChange) && (TDR_TYPE_STRUCT == pstCurMeta->iType))*/

        if (0 >= pstStackTop->iCount)
        {/*当前元数据数组已经处理完毕*/
            pstStackTop--;
            iStackItemCount--;
            if (0 < iStackItemCount)
            {
                pstCurMeta = pstStackTop->pstMeta;
                TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
            }
            continue;
        }

        pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;
        if (pstEntry->iVersion > a_iCufOffVersion)
        {
            TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
            continue;
        }
        if (TDR_IO_NOOUTPUT & pstEntry->iIO)
        {
            TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
            continue;
        }
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
        pszHostStart = pstStackTop->pszHostBase + pstEntry->iHOff;

        /*取出此entry的数组计数信息*/
        TDR_GET_ARRAY_REAL_COUNT(iArrayRealCount, pstEntry, pstStackTop->pszHostBase, a_iCufOffVersion);
        if ((iArrayRealCount < 0) ||
                ((0 < pstEntry->iCount) && (pstEntry->iCount < iArrayRealCount)))
        {/*实际数目为负数或比数组最大长度要大，则无效*/
            iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_REFER_VALUE);
            break;
        }
        if (0 == iArrayRealCount)
        {
            TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
            continue;
        }


        if (TDR_TYPE_COMPOSITE >= pstEntry->iType)
        {
            /*复合数据类型*/
            int idxSubEntry;
            LPTDRMETA pstTypeMeta;


            if (TDR_STACK_SIZE <=  iStackItemCount)
            {
                iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_TOO_COMPLIEX_META);
                break;
            }

            if (TDR_TYPE_UNION == pstEntry->iType)
            {
                TDR_GET_UNION_ENTRY_TYPE_META_INFO(pstStackTop->pszHostBase, pstLib, pstEntry,
                        pstStackTop->iCutOffVersion,pstTypeMeta, idxSubEntry);
                if (NULL == pstTypeMeta)
                {
                    TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
                    continue;
                }
            }else
            {
                pstTypeMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
                idxSubEntry = 0;
            }

            pstCurMeta = pstTypeMeta;
            iStackItemCount++;
            pstStackTop++;
            pstStackTop->pstMeta = pstCurMeta;
            pstStackTop->iCount = iArrayRealCount;
            pstStackTop->idxEntry = idxSubEntry;
            pstStackTop->pszHostBase = pszHostStart;
            pstStackTop->iMetaSizeInfoOff = (pstStackTop - 1)->iMetaSizeInfoOff + 1;
            pstStackTop->iCutOffVersion = a_iCufOffVersion;
            pstStackTop->iCutOffVersion = a_iCufOffVersion;
            if (TDR_ERR_IS_ERROR(iRet))
            {
                break;
            }
            if (TDR_TYPE_STRUCT == pstEntry->iType)
            {
                pstStackTop->iChange = 1;
                szSpace[(pstStackTop->iMetaSizeInfoOff-1)*TDR_TAB_SIZE] = '\0';
                if (a_iCufOffVersion != pstStackTop->iCutOffVersion)
                {
                    iRet = tdr_iostream_write(a_pstIOStream, "%s<%s version=\"%d\">\n",
                            szSpace, TDR_GET_STRING_BY_PTR(pstLib,pstTypeMeta->ptrName), pstStackTop->iCutOffVersion);
                }else
                {
                    iRet = tdr_iostream_write(a_pstIOStream, "%s<%s>\n",
                            szSpace, TDR_GET_STRING_BY_PTR(pstLib,pstTypeMeta->ptrName));
                }
                szSpace[(pstStackTop->iMetaSizeInfoOff-1)*TDR_TAB_SIZE] = ' ';
            }else
            {
                pstStackTop->iChange = 0;
            }

            if (TDR_ERR_IS_ERROR(iRet))
            {
                break;
            }

            continue;
        }/*if (TDR_TYPE_COMPOSITE >= pstEntry->iType)*/

        /*基本数据类型*/
        szSpace[pstStackTop->iMetaSizeInfoOff*TDR_TAB_SIZE] = '\0';
        iRet = tdr_output_simpleentry_i(a_pstIOStream, pstLib, pstEntry, iArrayRealCount,
                pszHostStart, pszHostEnd, &szSpace[0]);
        szSpace[pstStackTop->iMetaSizeInfoOff*TDR_TAB_SIZE] = ' ';

        if (TDR_ERR_IS_ERROR(iRet))
        {
            break;
        }

        TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
    }/*while (0 < iStackItemCount)*/


    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_iostream_write(a_pstIOStream, "</%s>\n", TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName));
    }
    return iRet;
}

scew_element * tdr_input_root_i(IN scew_element *a_pstRoot, IN const char *a_pszRootName)
{
    scew_element *pstInputRoot = NULL;

    assert(NULL != a_pstRoot);
    assert(NULL != a_pszRootName);

    /* first check the root element. */
    if (0 == tdr_stricmp(scew_element_name(a_pstRoot), TDR_TAG_DATASET))
    {
        pstInputRoot = scew_element_by_name(a_pstRoot, a_pszRootName);
        if (NULL != pstInputRoot)
        {
            return pstInputRoot;
        }
    }

    /*与根节点匹配*/
    if (0 == tdr_stricmp(scew_element_name(a_pstRoot), a_pszRootName))
    {
        return a_pstRoot;
    }

    /*匹配根节点的儿子节点*/
    pstInputRoot = scew_element_next(a_pstRoot, NULL);
    while (NULL != pstInputRoot)
    {
        scew_attribute *pstAttr;

        if (0 == tdr_stricmp(scew_element_name(pstInputRoot), a_pszRootName))
        {
            break;
        }


        pstAttr = scew_attribute_by_name(pstInputRoot, TDR_TAG_TYPE);
        if( NULL == pstAttr )
        {
            pstInputRoot = scew_element_next(a_pstRoot, pstInputRoot);
            continue;
        }
        if (0 == tdr_stricmp(scew_attribute_value(pstAttr),  a_pszRootName))
        {
            break;
        }

        pstInputRoot = scew_element_next(a_pstRoot, pstInputRoot);
    }/*while (NULL != pstInputRoot)*/

    return pstInputRoot;
}

/**从以TDR_XML_DATA_FORMAT_ATTR_ENTRY_NAME格式存储的xml文件中读取某个节点的属性信息
 *@param[in] a_pstMeta 保存属性信息的数据结构的描述句柄
 *@param[in] a_pstNode xml节点
 *@param[in] a_iFirstEntry 指定起始成员，即从a_iFirstEntry指定的成员开始读取
 *@param[in] a_pszHostStart 保存数据信息的内存缓冲区起始地址
 *@param[in] a_iSize	保存数据信息的内存缓冲区可用大小，单位为字节
 *@param[in] a_iVersion 数据的版本
 */
int tdr_input_meta_attrs_i(IN LPTDRMETA a_pstMeta, IN scew_element *a_pstNode, IN int a_iFirstEntry,
        IN char *a_pszHostStart,IN ptrdiff_t a_iSize, int a_iVersion)
{
    int iRet = 0;
    TDRSTACKITEM stStackItem;
    LPTDRSTACKITEM pstStackTop;
    int iChange = 0;
    LPTDRMETAENTRY pstEntry;
    char *pszHostStart;
    char *pszHostEnd;
    int iArrayRealCount;
    LPTDRMETALIB pstLib;
    scew_attribute *pstAttr;


    if (0 == a_iVersion)
    {
        a_iVersion = a_pstMeta->iCurVersion;
    }

    assert(NULL != a_pstMeta);
    assert(NULL != a_pstNode);
    assert(NULL != a_pszHostStart);
    assert(a_iVersion >= a_pstMeta->iBaseVersion);
    assert((0 <= a_iFirstEntry) && (a_iFirstEntry < a_pstMeta->iEntriesNum));


    if (0 >= a_iSize)
    {
        iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);
        return iRet;
    }

    pstStackTop = &stStackItem;
    pstStackTop->iCount = 1;
    pstStackTop->iChange = 1;
    pstStackTop->idxEntry = a_iFirstEntry;
    pstStackTop->pszHostBase = a_pszHostStart;
    pstLib = TDR_META_TO_LIB(a_pstMeta);
    pszHostEnd = a_pszHostStart + a_iSize;

    while (!iChange)
    {
        pstEntry = a_pstMeta->stEntries + pstStackTop->idxEntry;
        if (pstEntry->iVersion > a_iVersion)
        {
            TDR_GET_NEXT_ENTRY(pstStackTop, a_pstMeta, iChange);
            continue;
        }
        if (TDR_IO_NOINPUT & pstEntry->iIO)
        {
            TDR_GET_NEXT_ENTRY(pstStackTop, a_pstMeta, iChange);
            continue;
        }
        if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry)||TDR_ENTRY_IS_REFER_TYPE(pstEntry))
        {
            TDR_GET_NEXT_ENTRY(pstStackTop, a_pstMeta, iChange);
            continue;
        }

        pszHostStart = pstStackTop->pszHostBase + pstEntry->iHOff;

        /*取出此entry的数组计数信息*/
        TDR_GET_ARRAY_REAL_COUNT(iArrayRealCount, pstEntry, pstStackTop->pszHostBase, a_iVersion);
        if ((iArrayRealCount < 0) ||
                ((0 < pstEntry->iCount) && (pstEntry->iCount < iArrayRealCount)))
        {/*实际数目为负数或比数组最大长度要大，则无效*/
            iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_REFER_VALUE);
            break;
        }
        if (0 == iArrayRealCount)
        {
            TDR_GET_NEXT_ENTRY(pstStackTop, a_pstMeta, iChange);
            continue;
        }

        /*复合数据类型成员不以属性方式输出*/
        if (TDR_TYPE_COMPOSITE >= pstEntry->iType)
        {
            TDR_GET_NEXT_ENTRY(pstStackTop, a_pstMeta, iChange);
            continue;
        }/*if (TDR_TYPE_COMPOSITE >= pstEntry->iType)*/

        /*简单数据类型数组成员不以属性方式输出*/
        if (1 != pstEntry->iCount)
        {
            TDR_GET_NEXT_ENTRY(pstStackTop, a_pstMeta, iChange);
            continue;
        }

        /*非数组基本数据类型成员以属性方式输入*/
        pstAttr = scew_attribute_by_name(a_pstNode, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName));
        if (NULL == pstAttr)
        {
            if (TDR_ENTRY_IS_COUNTER(pstEntry))
            {
                LPTDRMETAENTRY pstUsedEntry =  TDR_PTR_TO_ENTRY(pstLib, pstEntry->stRefer.ptrEntry);
                TDR_SET_INT(pszHostStart, pstEntry->iHUnitSize, pstUsedEntry->iCount);
            }else
            {
                TDR_SET_DEFAULT_VALUE(iRet, pszHostStart, pszHostEnd, pstLib, pstEntry, 1);
            }

        }else
        {
            size_t iSize = pszHostEnd - pszHostStart;
            iRet = tdr_ioscanf_basedtype_i(pstLib, pstEntry, pszHostStart, &iSize,
                    scew_attribute_value(pstAttr));
        }/*if (NULL == pstAttr)*/
        if (TDR_ERR_IS_ERROR(iRet))
        {
            break;
        }

        TDR_GET_NEXT_ENTRY(pstStackTop, a_pstMeta, iChange);
    }/*while (!iChange)*/

    return iRet;
}


int tdr_input_by_attr_xml_i(IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstHost, IN scew_element *a_pstRoot,
        IN int a_iCutOffVersion)
{
    int iRet = TDR_SUCCESS;
    LPTDRMETALIB pstLib;
    LPTDRMETA pstCurMeta;
    TDRSTACK  stStack;
    LPTDRSTACKITEM pstStackTop;
    int iStackItemCount;

    char *pszHostStart;
    char *pszHostEnd;
    int iChange;

    if (0 == a_iCutOffVersion)
    {
        a_iCutOffVersion = a_pstMeta->iCurVersion;
    }
    assert(NULL != a_pstMeta);
    assert(NULL != a_pstRoot);
    assert(NULL != a_pstHost);
    assert(a_pstMeta->iBaseVersion <= a_iCutOffVersion);



    pszHostStart = a_pstHost->pszBuff;
    pszHostEnd = a_pstHost->pszBuff + a_pstHost->iBuff;
    pstLib = TDR_META_TO_LIB(a_pstMeta);


    pstCurMeta = a_pstMeta;
    pstStackTop = &stStack[0];
    pstStackTop->pszNetBase  = (char *)a_pstRoot; /*the parent node */
    pstStackTop->pszMetaSizeInfoTarget = NULL; /*the current node*/
    pstStackTop->pstMeta = pstCurMeta;
    pstStackTop->pszHostBase = pszHostStart;
    pstStackTop->iCount = 1;
    pstStackTop->idxEntry = 0;
    pstStackTop->iChange = 0;
    pstStackTop->iRealCount = 0; /*实际读取的结构数*/
    pstStackTop->szMetaEntryName[0] = '\0';
    iStackItemCount = 1;


    iRet = tdr_input_meta_attrs_i(pstCurMeta, a_pstRoot, 0, pszHostStart, a_pstHost->iBuff, a_iCutOffVersion);
    if (TDR_ERR_IS_ERROR(iRet))
    {
        return iRet;
    }

    iChange = 0;
    while (0 < iStackItemCount)
    {
        LPTDRMETAENTRY pstEntry;
        int iCount;
        scew_element *pstParent;


        if (0 != iChange)
        {
            iChange = 0;
            if (1 >= iStackItemCount)
            {
                iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_TOO_COMPLIEX_META);
                break;
            }


            pstStackTop->iRealCount++;  /*已经读取一个结构/uion*/
            (pstStackTop -1)->pszMetaSizeInfoTarget = pstStackTop->pszNetBase;
            if (0 < pstStackTop->iCount)
            {
                TDR_GET_NEXT_ELEMET(pstParent, (pstStackTop-1)->pszNetBase, (pstStackTop-1)->pszMetaSizeInfoTarget, pstStackTop->szMetaEntryName);
                if (NULL == pstParent)
                {
                    iChange = 0;
                    TDR_CLEAR_COUNT(pstStackTop, pstCurMeta, pszHostStart, pszHostEnd);
                    continue;
                }
                pstStackTop->pszNetBase = (char *)pstParent;

                /*先读取属性成员*/
                pszHostStart = pstStackTop->pszHostBase;
                iRet = tdr_input_meta_attrs_i(pstCurMeta, pstParent, pstStackTop->idxEntry, pszHostStart,
                        (pszHostEnd - pszHostStart), a_iCutOffVersion);
                if (TDR_ERR_IS_ERROR(iRet))
                {
                    return iRet;
                }
            }/*if (0 < pstStackTop->iCount)*/
        }/*if ((0 != iChange)*/



        if (0 >= pstStackTop->iCount)
        {/*当前元数据数组已经处理完毕*/
            pstStackTop--;
            iStackItemCount--;
            if (0 < iStackItemCount)
            {
                pstCurMeta = pstStackTop->pstMeta;
                pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;
                TDR_SET_ARRAY_REAL_COUNT((pstStackTop + 1)->iRealCount, pstEntry, pstStackTop->pszHostBase);
                TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
            }
            continue;
        }

        pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;

        /*非数字简单成员以属性方式输入，不需要处理*/
        if ((TDR_TYPE_COMPOSITE < pstEntry->iType) && (1 == pstEntry->iCount))
        {
            TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
            continue;
        }

        if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry)||TDR_ENTRY_IS_REFER_TYPE(pstEntry))
        {
            TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
            continue;
        }
        if (pstEntry->iVersion > a_iCutOffVersion)
        {
            TDR_SET_DEFAULT_VALUE(iRet, pszHostStart, pszHostEnd, pstLib, pstEntry, pstEntry->iCount);
            if (TDR_ERR_IS_ERROR(iRet))
            {
                break;
            }else
            {
                TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
                continue;
            }
        }/*if (pstEntry->iVersion > a_iCutOffVersion)*/



        /* 取出此entry的数组计数信息,并设置最大读取成员数目：
         *  1)如果通过refer属性指定了成员的数目，则最大读取数目按此refer指定数目读取；
         *  2)如果没有通过refer属性指定了成员的数目:
         *		2.1）如果成员数组长度确定(icount > 0),则最大读取数目按成员数组长度设置；
         *		2.2) 如果成员数组长度确定(icount == 0),则最大读取数目设置为最大整数;
         */
        TDR_GET_ARRAY_REAL_COUNT(iCount, pstEntry, pstStackTop->pszHostBase, a_iCutOffVersion);
        if (0 > iCount)
        {	/*实际数目为负数或比数组最大长度要大，则无效*/
            iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_REFER_VALUE);
            break;
        }
        if ((0 < pstEntry->iCount) && (pstEntry->iCount < iCount))
        {
            iCount = pstEntry->iCount;
        }
        if( (0 == iCount) && (pstEntry->iCount == 0))
        {
            iCount = TDR_MAX_INT;
        }

        pszHostStart = pstStackTop->pszHostBase + pstEntry->iHOff;

        if (TDR_IO_NOINPUT & pstEntry->iIO)
        {
            if (TDR_ENTRY_IS_COUNTER(pstEntry))
            {
                LPTDRMETAENTRY pstUsedEntry =  TDR_PTR_TO_ENTRY(pstLib, pstEntry->stRefer.ptrEntry);
                if (pstUsedEntry->iVersion >= a_iCutOffVersion && (!(TDR_IO_NOINPUT & pstUsedEntry->iIO)))
                {
                    TDR_SET_INT(pszHostStart, pstEntry->iHUnitSize, pstUsedEntry->iCount);
                    pszHostStart += pstEntry->iHUnitSize;
                }
            }else
            {
                TDR_SET_DEFAULT_VALUE(iRet, pszHostStart, pszHostEnd, pstLib, pstEntry, iCount);
            }
            TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
            continue;
        }

        pstStackTop->pszMetaSizeInfoTarget = NULL;  /*从头开始搜索*/

        if (TDR_TYPE_COMPOSITE >= pstEntry->iType)
        {/*复合数据类型*/
            int idxSubEntry;
            LPTDRMETA pstTypeMeta;

            if (TDR_STACK_SIZE <=  iStackItemCount)
            {
                iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_TOO_COMPLIEX_META);
                break;
            }

            if (TDR_TYPE_UNION == pstEntry->iType)
            {
                TDR_GET_UNION_ENTRY_TYPE_META_INFO(pstStackTop->pszHostBase, pstLib, pstEntry,
                        a_iCutOffVersion, pstTypeMeta, idxSubEntry);
                if (NULL == pstTypeMeta)
                {
                    TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
                    continue;
                }
            }else
            {
                pstTypeMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
                idxSubEntry = 0;
            }/*if (TDR_TYPE_UNION == pstEntry->iType)*/

            /*定位节点*/
            TDR_GET_NEXT_ELEMET(pstParent, pstStackTop->pszNetBase, pstStackTop->pszMetaSizeInfoTarget, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName));
            if (NULL == pstParent)
            {
                TDR_SET_ARRAY_REAL_COUNT(0, pstEntry, pstStackTop->pszHostBase);
                TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
                continue;
            }

            pstCurMeta = pstTypeMeta;
            iStackItemCount++;
            pstStackTop++;
            pstStackTop->pstMeta = pstCurMeta;
            pstStackTop->iCount = iCount;
            pstStackTop->idxEntry = idxSubEntry;
            pstStackTop->pszHostBase = pszHostStart;
            pstStackTop->pszNetBase = (char *)pstParent;
            pstStackTop->pszMetaSizeInfoTarget = NULL;
            pstStackTop->iRealCount = 0; /*实际读取的结构数*/
            TDR_STRNCPY(pstStackTop->szMetaEntryName, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName), sizeof(pstStackTop->szMetaEntryName));
            pstStackTop->iChange = 1;

            /*先读取属性成员*/
            iRet = tdr_input_meta_attrs_i(pstCurMeta, pstParent, idxSubEntry, pszHostStart,
                    (pszHostEnd - pszHostStart), a_iCutOffVersion);
            if (TDR_ERR_IS_ERROR(iRet))
            {
                return iRet;
            }
            continue;
        }/*if (TDR_TYPE_COMPOSITE >= pstEntry->iType)*/

        iRet = tdr_input_simple_entry_i(pstLib, pstEntry, &iCount, (scew_element *)pstStackTop->pszNetBase,
                (scew_element **)&pstStackTop->pszMetaSizeInfoTarget, &pszHostStart, pszHostEnd);
        if (TDR_ERR_IS_ERROR(iRet))
        {
            break;
        }
        TDR_SET_ARRAY_REAL_COUNT(iCount, pstEntry, pstStackTop->pszHostBase);
        TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
    }/*while (0 < iStackItemCount)*/

    a_pstHost->iBuff = pszHostStart - a_pstHost->pszBuff;
    return iRet;
}

int tdr_input_i(IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstHost, IN scew_element *a_pstRoot,
        IN int a_iCutOffVersion, IN int a_iXmlDataFormat)
{
    int iRet = TDR_SUCCESS;
    scew_element *pstInputRoot;
    LPTDRMETALIB pstLib = TDR_META_TO_LIB(a_pstMeta);


    assert(NULL != a_pstMeta);
    assert(NULL != a_pstRoot);
    assert(NULL != a_pstHost);
    assert(a_pstMeta->iBaseVersion <= a_iCutOffVersion);
    assert(NULL != a_pstHost->pszBuff);
    assert(0 < a_pstHost->iBuff);

    /*支持struct结构的数据输入*/
    if (TDR_TYPE_STRUCT != a_pstMeta->iType)
    {
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
    }

    pstInputRoot = tdr_input_root_i(a_pstRoot, TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName));
    if (NULL == pstInputRoot)
    {
        a_pstHost->iBuff = 0;
        return TDR_ERRIMPLE_NO_XML_ROOT;
    }

    switch(a_iXmlDataFormat)
    {
        case TDR_XML_DATA_FORMAT_LIST_MATA_NAME:
            if (TDR_META_IS_STRICT_INPUT(a_pstMeta))
            {
                iRet = tdr_input_by_strict_xml_i(a_pstMeta, a_pstHost, pstInputRoot, a_iCutOffVersion, a_iXmlDataFormat);
            }else
            {
                iRet = tdr_input_by_xml_oldversion_i(a_pstMeta, a_pstHost, pstInputRoot, a_iCutOffVersion);
            }
            break;
        case TDR_XML_DATA_FORMAT_ATTR_ENTRY_NAME:
            /*目前此格式不支持TDR_META_IS_STRICT_INPUT属性 */
            iRet = tdr_input_by_attr_xml_i(a_pstMeta, a_pstHost, pstInputRoot, a_iCutOffVersion);
            break;
        default:
            if (TDR_META_IS_STRICT_INPUT(a_pstMeta))
            {
                iRet = tdr_input_by_strict_xml_i(a_pstMeta, a_pstHost, pstInputRoot, a_iCutOffVersion, a_iXmlDataFormat);
            }else
            {
                iRet = tdr_input_by_xml_i(a_pstMeta, a_pstHost, pstInputRoot, a_iCutOffVersion);
            }
            break; /*take as TDR_XML_DATA_FORMAT_LIST_ENTRY_NAME*/
    }/*switch(a_iXmlDataFormat)*/


    return iRet;

}

int tdr_input_by_xml_i(IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstHost, IN scew_element *a_pstRoot,
        IN int a_iCutOffVersion)
{
    int iRet = TDR_SUCCESS;
    LPTDRMETALIB pstLib;
    LPTDRMETA pstCurMeta;
    TDRSTACK  stStack;
    LPTDRSTACKITEM pstStackTop;
    int iStackItemCount;

    char *pszHostStart;
    char *pszHostEnd;
    int iChange;

    assert(NULL != a_pstMeta);
    assert(NULL != a_pstRoot);
    assert(NULL != a_pstHost);
    assert(a_pstMeta->iBaseVersion <= a_iCutOffVersion);



    pszHostStart = a_pstHost->pszBuff;
    pszHostEnd = a_pstHost->pszBuff + a_pstHost->iBuff;
    pstLib = TDR_META_TO_LIB(a_pstMeta);

    pstCurMeta = a_pstMeta;
    pstStackTop = &stStack[0];
    pstStackTop->pszNetBase  = (char *)a_pstRoot; /*the parent node */
    pstStackTop->pszMetaSizeInfoTarget = NULL; /*the current node*/
    pstStackTop->pstMeta = pstCurMeta;
    pstStackTop->pszHostBase = pszHostStart;
    pstStackTop->iCount = 1;
    pstStackTop->idxEntry = 0;
    pstStackTop->iChange = 0;
    pstStackTop->iRealCount = 0; /*实际读取的结构数*/
    pstStackTop->szMetaEntryName[0] = '\0';
    iStackItemCount = 1;


    iChange = 0;
    while (0 < iStackItemCount)
    {
        LPTDRMETAENTRY pstEntry;
        int iCount;
        scew_element *pstParent;


        if (0 != iChange)
        {
            if (1 >= iStackItemCount)
            {
                iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_TOO_COMPLIEX_META);
                break;
            }

            pstStackTop->iRealCount++;  /*已经读取一个结构/uion*/
            (pstStackTop -1)->pszMetaSizeInfoTarget = pstStackTop->pszNetBase;
            if (0 < pstStackTop->iCount)
            {
                TDR_GET_NEXT_ELEMET(pstParent, (pstStackTop-1)->pszNetBase, (pstStackTop-1)->pszMetaSizeInfoTarget, pstStackTop->szMetaEntryName);
                if (NULL == pstParent)
                {
                    iChange = 0;
                    TDR_CLEAR_COUNT(pstStackTop, pstCurMeta, pszHostStart, pszHostEnd);
                    continue;
                }
                pstStackTop->pszNetBase = (char *)pstParent;
            }/*if (0 < pstStackTop->iCount)*/
        }/*if ((0 != iChange)*/

        iChange = 0;

        if (0 >= pstStackTop->iCount)
        {/*当前元数据数组已经处理完毕*/
            pstStackTop--;
            iStackItemCount--;
            if (0 < iStackItemCount)
            {
                pstCurMeta = pstStackTop->pstMeta;
                pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;
                TDR_SET_ARRAY_REAL_COUNT((pstStackTop + 1)->iRealCount, pstEntry, pstStackTop->pszHostBase);
                TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
            }
            continue;
        }

        pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;
        if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry)||TDR_ENTRY_IS_REFER_TYPE(pstEntry))
        {
            TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
            continue;
        }

        /*取出此entry的数组计数信息*/
        TDR_GET_ARRAY_REAL_COUNT(iCount, pstEntry, pstStackTop->pszHostBase, a_iCutOffVersion);
        if (0 > iCount)
        {	/*实际数目为负数或比数组最大长度要大，则无效*/
            iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_REFER_VALUE);
            break;
        }


        /* 设置最大读取成员数目：
         *  1)如果通过refer属性指定了成员的数目，则最大读取数目按此refer指定数目读取；
         *  2)如果没有通过refer属性指定了成员的数目:
         *		2.1）如果成员数组长度确定(icount > 0),则最大读取数目按成员数组长度设置；
         *		2.2) 如果成员数组长度确定(icount == 0),则最大读取数目设置为最大整数;
         */
        if ((0 < pstEntry->iCount) && (pstEntry->iCount < iCount))
        {
            iCount = pstEntry->iCount;
        }
        if( (0 == iCount) && (pstEntry->iCount == 0))
        {
            iCount = TDR_MAX_INT;
        }

        pszHostStart = pstStackTop->pszHostBase + pstEntry->iHOff;

        if (TDR_IO_NOINPUT & pstEntry->iIO)
        {
            if (TDR_ENTRY_IS_COUNTER(pstEntry))
            {
                LPTDRMETAENTRY pstUsedEntry =  TDR_PTR_TO_ENTRY(pstLib, pstEntry->stRefer.ptrEntry);
                if (pstUsedEntry->iVersion >= a_iCutOffVersion && (!(TDR_IO_NOINPUT & pstUsedEntry->iIO)))
                {
                    TDR_SET_INT(pszHostStart, pstEntry->iHUnitSize, pstUsedEntry->iCount);
                    pszHostStart += pstEntry->iHUnitSize;
                }
            }else
            {
                TDR_SET_DEFAULT_VALUE(iRet, pszHostStart, pszHostEnd, pstLib, pstEntry, iCount);
            }
            TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
            continue;
        }

        if (pstEntry->iVersion > a_iCutOffVersion)
        {
            TDR_SET_DEFAULT_VALUE(iRet, pszHostStart, pszHostEnd, pstLib, pstEntry, pstEntry->iCount);
            if (TDR_ERR_IS_ERROR(iRet))
            {
                break;
            }else
            {
                TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
                continue;
            }
        }


        pstStackTop->pszMetaSizeInfoTarget = NULL;  /*从头开始搜索*/

        if (TDR_TYPE_COMPOSITE >= pstEntry->iType)
        {/*复合数据类型*/
            int idxSubEntry;
            LPTDRMETA pstTypeMeta;

            if (TDR_STACK_SIZE <=  iStackItemCount)
            {
                iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_TOO_COMPLIEX_META);
                break;
            }

            if (TDR_TYPE_UNION == pstEntry->iType)
            {
                TDR_GET_UNION_ENTRY_TYPE_META_INFO(pstStackTop->pszHostBase, pstLib, pstEntry, a_iCutOffVersion, pstTypeMeta, idxSubEntry);
                if (NULL == pstTypeMeta)
                {
                    TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
                    continue;
                }
            }else
            {
                pstTypeMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
                idxSubEntry = 0;
            }/*if (TDR_TYPE_UNION == pstEntry->iType)*/

            /*定位节点*/
            TDR_GET_NEXT_ELEMET(pstParent, pstStackTop->pszNetBase, pstStackTop->pszMetaSizeInfoTarget, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName));
            if (NULL == pstParent)
            {
                TDR_SET_ARRAY_REAL_COUNT(0, pstEntry, pstStackTop->pszHostBase);
                TDR_SET_DEFAULT_VALUE(iRet, pszHostStart, pszHostEnd, pstLib, pstEntry, pstEntry->iCount);
                TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
                continue;
            }

            pstCurMeta = pstTypeMeta;
            iStackItemCount++;
            pstStackTop++;
            pstStackTop->pstMeta = pstCurMeta;
            pstStackTop->iCount = iCount;
            pstStackTop->idxEntry = idxSubEntry;
            pstStackTop->pszHostBase = pszHostStart;
            pstStackTop->pszNetBase = (char *)pstParent;
            pstStackTop->pszMetaSizeInfoTarget = NULL;
            pstStackTop->iRealCount = 0; /*实际读取的结构数*/
            TDR_STRNCPY(pstStackTop->szMetaEntryName, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName), sizeof(pstStackTop->szMetaEntryName));
            pstStackTop->iChange = 1;
            continue;
        }/*if (TDR_TYPE_COMPOSITE >= pstEntry->iType)*/

        iRet = tdr_input_simple_entry_i(pstLib, pstEntry, &iCount, (scew_element *)pstStackTop->pszNetBase,
                (scew_element **)&pstStackTop->pszMetaSizeInfoTarget, &pszHostStart, pszHostEnd);
        if (TDR_ERR_IS_ERROR(iRet))
        {
            break;
        }
        TDR_SET_ARRAY_REAL_COUNT(iCount, pstEntry, pstStackTop->pszHostBase);
        TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
    }/*while (0 < iStackItemCount)*/

    a_pstHost->iBuff = pszHostStart - a_pstHost->pszBuff;
    return iRet;
}


int tdr_input_by_xml_oldversion_i(IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstHost, IN scew_element *a_pstRoot,
        IN int a_iCutOffVersion)
{
    int iRet = TDR_SUCCESS;
    LPTDRMETALIB pstLib;
    LPTDRMETA pstCurMeta;
    TDRSTACK  stStack;
    LPTDRSTACKITEM pstStackTop;
    int iStackItemCount;
    char *pszHostStart;
    char *pszHostEnd;
    int iChange;

    assert(NULL != a_pstMeta);
    assert(NULL != a_pstRoot);
    assert(NULL != a_pstHost);
    assert(a_pstMeta->iBaseVersion <= a_iCutOffVersion);



    pszHostStart = a_pstHost->pszBuff;
    pszHostEnd = a_pstHost->pszBuff + a_pstHost->iBuff;
    pstLib = TDR_META_TO_LIB(a_pstMeta);

    pstCurMeta = a_pstMeta;
    pstStackTop = &stStack[0];
    pstStackTop->pszNetBase  = (char *)a_pstRoot; /*the parent node */
    pstStackTop->pszMetaSizeInfoTarget = NULL; /*the current node*/
    pstStackTop->pstMeta = pstCurMeta;
    pstStackTop->pszHostBase = pszHostStart;
    pstStackTop->iCount = 1;
    pstStackTop->idxEntry = 0;
    pstStackTop->iChange = 0;
    pstStackTop->iRealCount = 0; /*实际读取的结构数*/
    pstStackTop->szMetaEntryName[0] = '\0';
    iStackItemCount = 1;


    iChange = 0;
    while (0 < iStackItemCount)
    {
        LPTDRMETAENTRY pstEntry;
        int iCount;
        scew_element *pstParent;


        if (0 != iChange)
        {
            if (1 >= iStackItemCount)
            {
                iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_TOO_COMPLIEX_META);
                break;
            }

            pstStackTop->iRealCount++;  /*已经读取一个结构/uion*/
            if (TDR_TYPE_STRUCT == pstCurMeta->iType)
            {
                (pstStackTop -1)->pszMetaSizeInfoTarget = pstStackTop->pszNetBase;
                if (0 < pstStackTop->iCount)
                {
                    TDR_GET_NEXT_ELEMET(pstParent, (pstStackTop-1)->pszNetBase, (pstStackTop-1)->pszMetaSizeInfoTarget, TDR_GET_STRING_BY_PTR(pstLib,pstCurMeta->ptrName));
                    if (NULL == pstParent)
                    {
                        iChange = 0;
                        TDR_CLEAR_COUNT(pstStackTop, pstCurMeta, pszHostStart, pszHostEnd);
                        continue;
                    }
                    pstStackTop->pszNetBase = (char *)pstParent;
                }/*if (0 < pstStackTop->iCount)*/
            }else
            {
                (pstStackTop -1)->pszMetaSizeInfoTarget = pstStackTop->pszMetaSizeInfoTarget;
                if (0 < pstStackTop->iCount)
                {
                    pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;
                    if (TDR_TYPE_STRUCT == pstEntry->iType)
                    {
                        LPTDRMETA pstType = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
                        TDR_GET_NEXT_ELEMET(pstParent, pstStackTop->pszNetBase, pstStackTop->pszMetaSizeInfoTarget, TDR_GET_STRING_BY_PTR(pstLib,pstType->ptrName));
                    }else
                    {
                        TDR_GET_NEXT_ELEMET(pstParent, pstStackTop->pszNetBase, pstStackTop->pszMetaSizeInfoTarget, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName));
                    }
                    if (NULL == pstParent)
                    {
                        iChange = 0;
                        TDR_CLEAR_COUNT(pstStackTop, pstCurMeta, pszHostStart, pszHostEnd);
                        continue;
                    }
                }/*if (0 < pstStackTop->iCount)*/
            }/*if (TDR_TYPE_STRUCT == pstCurMeta->iType)*/
        }/*if ((0 != iChange)*/

        iChange = 0;

        if (0 >= pstStackTop->iCount)
        {/*当前元数据数组已经处理完毕*/
            pstStackTop--;
            iStackItemCount--;
            if (0 < iStackItemCount)
            {
                pstCurMeta = pstStackTop->pstMeta;
                pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;
                TDR_SET_ARRAY_REAL_COUNT((pstStackTop + 1)->iRealCount, pstEntry, pstStackTop->pszHostBase);
                TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
            }
            continue;
        }

        pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;
        if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry)||TDR_ENTRY_IS_REFER_TYPE(pstEntry))
        {
            TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
            continue;
        }

        /*取出此entry的数组计数信息*/
        TDR_GET_ARRAY_REAL_COUNT(iCount, pstEntry, pstStackTop->pszHostBase, a_iCutOffVersion);
        if (0 > iCount)
        {	/*实际数目为负数或比数组最大长度要大，则无效*/
            iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_REFER_VALUE);
            break;
        }


        /* 设置最大读取成员数目：
         *  1)如果通过refer属性指定了成员的数目，则最大读取数目按此refer指定数目读取；
         *  2)如果没有通过refer属性指定了成员的数目:
         *		2.1）如果成员数组长度确定(icount > 0),则最大读取数目按成员数组长度设置；
         *		2.2) 如果成员数组长度确定(icount == 0),则最大读取数目设置为最大整数;
         */
        if ((0 < pstEntry->iCount) && (pstEntry->iCount < iCount))
        {
            iCount = pstEntry->iCount;
        }
        if( (0 == iCount) && (pstEntry->iCount == 0))
        {
            iCount = TDR_MAX_INT;
        }

        pszHostStart = pstStackTop->pszHostBase + pstEntry->iHOff;

        if (TDR_IO_NOINPUT & pstEntry->iIO)
        {
            if (TDR_ENTRY_IS_COUNTER(pstEntry))
            {
                LPTDRMETAENTRY pstUsedEntry =  TDR_PTR_TO_ENTRY(pstLib, pstEntry->stRefer.ptrEntry);
                if (pstUsedEntry->iVersion >= a_iCutOffVersion && (!(TDR_IO_NOINPUT & pstUsedEntry->iIO)))
                {
                    TDR_SET_INT(pszHostStart, pstEntry->iHUnitSize, pstUsedEntry->iCount);
                    pszHostStart += pstEntry->iHUnitSize;
                }
            }else
            {
                TDR_SET_DEFAULT_VALUE(iRet, pszHostStart, pszHostEnd, pstLib, pstEntry, iCount);
            }
            TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
            continue;
        }

        if (pstEntry->iVersion > a_iCutOffVersion)
        {
            TDR_SET_DEFAULT_VALUE(iRet, pszHostStart, pszHostEnd, pstLib, pstEntry, pstEntry->iCount);
            if (TDR_ERR_IS_ERROR(iRet))
            {
                break;
            }else
            {
                TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
                continue;
            }
        }


        if (TDR_TYPE_UNION != pstCurMeta->iType)
        {
            pstStackTop->pszMetaSizeInfoTarget = NULL;  /*从头开始搜索*/
        }


        if (TDR_TYPE_COMPOSITE >= pstEntry->iType)
        {/*复合数据类型*/
            int idxSubEntry;
            LPTDRMETA pstTypeMeta;

            if (TDR_STACK_SIZE <=  iStackItemCount)
            {
                iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_TOO_COMPLIEX_META);
                break;
            }

            if (TDR_TYPE_UNION == pstEntry->iType)
            {
                TDR_GET_UNION_ENTRY_TYPE_META_INFO(pstStackTop->pszHostBase, pstLib, pstEntry, a_iCutOffVersion, pstTypeMeta, idxSubEntry);
                if (NULL == pstTypeMeta)
                {
                    TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
                    continue;
                }
            }else
            {
                pstTypeMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
                idxSubEntry = 0;
            }/*if (TDR_TYPE_UNION == pstEntry->iType)*/

            /*定位节点*/
            if (TDR_TYPE_UNION == pstEntry->iType)
            {
                pstParent = (scew_element *)pstStackTop->pszNetBase;
            }else
            {
                TDR_GET_NEXT_ELEMET(pstParent, pstStackTop->pszNetBase, pstStackTop->pszMetaSizeInfoTarget, TDR_GET_STRING_BY_PTR(pstLib,pstTypeMeta->ptrName));
            }
            if (NULL == pstParent)
            {
                TDR_SET_ARRAY_REAL_COUNT(0, pstEntry, pstStackTop->pszHostBase);
                TDR_SET_DEFAULT_VALUE(iRet, pszHostStart, pszHostEnd, pstLib, pstEntry, pstEntry->iCount);
                TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
                continue;
            }

            pstCurMeta = pstTypeMeta;
            iStackItemCount++;
            pstStackTop++;
            pstStackTop->pstMeta = pstCurMeta;
            pstStackTop->iCount = iCount;
            pstStackTop->idxEntry = idxSubEntry;
            pstStackTop->pszHostBase = pszHostStart;
            pstStackTop->pszNetBase = (char *)pstParent;
            pstStackTop->pszMetaSizeInfoTarget = NULL;
            pstStackTop->iRealCount = 0; /*实际读取的结构数*/
            pstStackTop->iChange = 1;

            continue;
        }/*if (TDR_TYPE_COMPOSITE >= pstEntry->iType)*/

        iRet = tdr_input_simple_entry_i(pstLib, pstEntry, &iCount, (scew_element *)pstStackTop->pszNetBase,
                (scew_element **)&pstStackTop->pszMetaSizeInfoTarget, &pszHostStart, pszHostEnd);
        if (TDR_ERR_IS_ERROR(iRet))
        {
            break;
        }
        TDR_SET_ARRAY_REAL_COUNT(iCount, pstEntry, pstStackTop->pszHostBase);
        TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
    }/*while (0 < iStackItemCount)*/

    a_pstHost->iBuff = pszHostStart - a_pstHost->pszBuff;
    return iRet;
}

/*从XML数据树节点中读入简单类型成员的值到内存空间中
 *@param[in] a_pstLib 元数据描述库指针
 *@param[in] a_pstEntry 指定成员描述结构的指针
 *@param[in,out] a_piCount
 *	- in 指定最大读取的元素个数
 *	- out	输出实际读取的元素个数
 *@param[in] a_pstRoot xml数据的根节点指针
 *@param[in,out] a_ppstItem
 *	- in	起始查找的儿子节点指针
 *	- out	下一次查找的儿子节点指针
 *@param[in, out] a_ppszHostStart 保存成员值的内存空间指针
 *	- in	保存数据内存空间起始地址
 *	- out	下一个成员可以使用的数据内存空间起始地址
 *@param[in] a_pszHostEnd 可使用内存空间终止地址
 */
int tdr_input_simple_entry_i(IN LPTDRMETALIB a_pstLib, IN LPTDRMETAENTRY a_pstEntry, INOUT int *a_piCount,
        IN scew_element *a_pstRoot, INOUT scew_element **a_ppstItem,
        INOUT char **a_ppszHostStart, IN char *a_pszHostEnd)
{
    int iRet = TDR_SUCCESS;
    size_t iSize;
    scew_element *pstItem;
    const char *pszVal;
    char *pszHostStart;
    int iCount;

    assert(NULL != a_pstLib);
    assert(NULL != a_pstEntry);
    assert(NULL != a_piCount);
    assert(NULL != a_pstRoot);
    assert(NULL != a_ppstItem);
    assert(NULL != a_ppszHostStart);
    assert(NULL != *a_ppszHostStart);
    assert(NULL != a_pszHostEnd);

    pstItem = *a_ppstItem;
    pszHostStart = *a_ppszHostStart;
    iCount = 0;
    switch(a_pstEntry->iType)
    {
        case TDR_TYPE_STRING:
        case TDR_TYPE_WSTRING:
        case TDR_TYPE_DATETIME:
            {
                for (iCount = 0; iCount < *a_piCount; iCount++)
                {
                    TDR_GET_NEXT_ELEMET(pstItem, a_pstRoot, pstItem,  TDR_GET_STRING_BY_PTR(a_pstLib,a_pstEntry->ptrName));
                    if (NULL == pstItem)
                    {
                        TDR_SET_DEFAULT_VALUE(iRet, pszHostStart, a_pszHostEnd, a_pstLib, a_pstEntry, (*a_piCount-iCount));
                        break;
                    }
                    pszVal = (char*) scew_element_contents(pstItem);
                    if (NULL == pszVal)
                    {
                        pszVal = "";
                    }
                    iSize = a_pszHostEnd - pszHostStart;
                    iRet = tdr_ioscanf_basedtype_i(a_pstLib, a_pstEntry, pszHostStart, &iSize, pszVal);
                    if (TDR_ERR_IS_ERROR(iRet))
                    {
                        break;
                    }
                    pszHostStart += iSize;
                    *a_ppstItem = pstItem;
                }/*for (iCount = 0; iCount < *a_piCount; iCount++)*/
            }
            break;
        default:
            {
                char *pszTok = NULL;
                char *pszNextTok = NULL;

                TDR_GET_NEXT_ELEMET(pstItem, a_pstRoot, pstItem,  TDR_GET_STRING_BY_PTR(a_pstLib,a_pstEntry->ptrName));
                if (NULL == pstItem)
                {
                    if (TDR_ENTRY_IS_COUNTER(a_pstEntry))
                    {
                        LPTDRMETAENTRY pstUsedEntry =  TDR_PTR_TO_ENTRY(a_pstLib, a_pstEntry->stRefer.ptrEntry);
                        TDR_SET_INT(pszHostStart, a_pstEntry->iHUnitSize, pstUsedEntry->iCount);
                        pszHostStart += a_pstEntry->iHUnitSize;
                    }else
                    {
                        TDR_SET_DEFAULT_VALUE(iRet, pszHostStart, a_pszHostEnd, a_pstLib, a_pstEntry, (*a_piCount));
                    }
                    break;
                }/*if (NULL == pstItem)	*/

                pszVal = (char*) scew_element_contents(pstItem);
                if (NULL == pszVal)
                {
                    pszVal = "";
                }
                pszTok = tdr_strtok_r((char *)pszVal, " \r\n\t", &pszNextTok);
                for (iCount = 0; iCount < *a_piCount; iCount++)
                {
                    if (NULL == pszTok)
                    {
                        TDR_SET_DEFAULT_VALUE(iRet, pszHostStart, a_pszHostEnd, a_pstLib, a_pstEntry, (*a_piCount-iCount));
                        break;
                    }
                    iSize = a_pszHostEnd - pszHostStart;
                    iRet = tdr_ioscanf_basedtype_i(a_pstLib, a_pstEntry, pszHostStart, &iSize, pszTok);
                    if (TDR_ERR_IS_ERROR(iRet))
                    {
                        break;
                    }
                    pszHostStart += iSize;
                    pszTok = tdr_strtok_r(NULL, " \r\n\t", &pszNextTok);
                }/*for (iCount = 0; iCount < *a_piCount; iCount++)*/
                *a_ppstItem = pstItem;
            }
            break;
    }/*switch(a_pstEntry->iType)*/

    *a_ppszHostStart = pszHostStart;
    *a_piCount = iCount;

    return iRet;
}

int tdr_output_json_i(IN LPTDRMETA a_pstMeta, LPTDRIOSTREAM a_pstIOStream,
                      IN LPTDRDATA a_pstHost, IN int a_iCutOffVersion)
{
	int iRet = TDR_SUCCESS;
	LPTDRMETALIB pstLib;
	LPTDRMETA pstCurMeta;
	TDRSTACK  stStack;
	LPTDRSTACKITEM pstStackTop;
	int iStackItemCount;
	int iChange;
	LPTDRMETAENTRY pstEntry;
	int iArrayRealCount ;
	char *pszHostStart;
	char *pszHostEnd;
	int idxSubEntry;

	if ((NULL == a_pstMeta) || (NULL == a_pstHost) || (NULL == a_pstIOStream))
	{
		return TDR_ERRIMPLE_INVALID_PARAM;
	}

	if (TDR_TYPE_STRUCT != a_pstMeta->iType)
	{
		return TDR_ERRIMPLE_INVALID_PARAM;
	}

	if (0 == a_iCutOffVersion)
	{
		a_iCutOffVersion = TDR_MAX_VERSION;
	}

	if (a_pstMeta->iBaseVersion > a_iCutOffVersion)
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
	}

	pszHostStart = a_pstHost->pszBuff;
	pszHostEnd = a_pstHost->pszBuff + a_pstHost->iBuff;

	pstCurMeta = a_pstMeta;
	pstLib = TDR_META_TO_LIB(a_pstMeta);

	pstStackTop = &stStack[0];
	pstStackTop->pstMeta = pstCurMeta;
	pstStackTop->iCount = 1;
	pstStackTop->idxEntry = 0;
	pstStackTop->pszHostBase = pszHostStart;
	pstStackTop->iChange = 0;
	pstStackTop->iCode = 0;
	iStackItemCount = 1;
	pstStackTop->iCutOffVersion = TDR_MAX_VERSION;
	pstStackTop->iRealCount = 1; /*real count of meta*/
	pstStackTop->pstEntry = NULL;

	iChange = 0;
	while (0 < iStackItemCount)
	{
		if (0 != iChange)
		{
			if (0 < pstStackTop->iCount)
			{
				assert(NULL != pstStackTop->pstEntry);

                iRet = tdr_iostream_write(a_pstIOStream, "},{");
                if (TDR_SUCCESS != iRet)
                {
                    break;
                }

			}/*if (0 < pstStackTop->iCount)*/
			iChange = 0;
		}/*if (0 != iChange)*/

		if (0 >= pstStackTop->iCount)
		{/*当前元数据数组已经处理完毕*/
            if (1 < iStackItemCount) /*跳过第一层元数据，因为进入的时候也没有处理*/
            {
                if (1 == pstStackTop->pstEntry->iCount)
                {
                    iRet = tdr_iostream_write(a_pstIOStream, "}");
                } else
                {
                    iRet = tdr_iostream_write(a_pstIOStream, "}]");
                }
                if (TDR_SUCCESS != iRet)
                {
                    break;
                }
            }
			pstStackTop--;
			iStackItemCount--;
			if (0 < iStackItemCount)
			{
				pstCurMeta = pstStackTop->pstMeta;
				TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			}
			continue;
		}

		pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;
        if (pstEntry->iVersion > a_iCutOffVersion)
        {
            TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
            continue;
        }
        if (TDR_IO_NOOUTPUT & pstEntry->iIO)
        {
            TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
            continue;
        }
		if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry) || TDR_ENTRY_IS_REFER_TYPE(pstEntry))
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}
		pszHostStart = pstStackTop->pszHostBase + pstEntry->iHOff;

		/*取出此entry的数组计数信息*/
		TDR_GET_ARRAY_REAL_COUNT(iArrayRealCount, pstEntry, pstStackTop->pszHostBase, pstStackTop->iCutOffVersion);
		if (((0 < pstEntry->iCount) && (pstEntry->iCount < iArrayRealCount)) || (0 > iArrayRealCount))
		{/*实际数目为负数或比数组最大长度要大，则无效*/
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_REFER_VALUE);
			break;
		}
		if (0 >= iArrayRealCount)
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}

        if (TDR_TYPE_UNION !=pstCurMeta->iType && 0 < pstStackTop->idxEntry)
        {
            iRet = tdr_iostream_write(a_pstIOStream, ",");
            if (TDR_SUCCESS != iRet)
            {
                break;
            }
        }


		/*简单数据类型*/
		if (TDR_TYPE_COMPOSITE >= pstEntry->iType)
		{
			/*复合数据类型*/
			if (TDR_STACK_SIZE <=  iStackItemCount)
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_TOO_COMPLIEX_META);
				break;
			}
			if (TDR_TYPE_UNION == pstEntry->iType)
			{
				TDR_GET_UNION_ENTRY_TYPE_META_INFO(pstStackTop->pszHostBase, pstLib, pstEntry, pstStackTop->iCutOffVersion, pstCurMeta, idxSubEntry);
				if(NULL == pstCurMeta)
				{
					pstCurMeta = pstStackTop->pstMeta;
					TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
					continue;
				}
			}else
			{
				pstCurMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
				idxSubEntry = 0;
			}

			if (0 >= iArrayRealCount)
			{
				TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
				continue;
			}

			if (1 == pstEntry->iCount)
			{
                iRet = tdr_iostream_write(a_pstIOStream, "\"%s\":{", tdr_get_entry_name(pstEntry));
			}else
			{
                iRet = tdr_iostream_write(a_pstIOStream, "\"%s\":[{", tdr_get_entry_name(pstEntry));
			}
            if (TDR_SUCCESS != iRet)
            {
                break;
            }

			/*递归进行结构体内部进行处理*/
			iStackItemCount++;
			pstStackTop++;
			pstStackTop->pszHostBase = pszHostStart;
			pstStackTop->iChange = 0;
			pstStackTop->pstMeta = pstCurMeta;
			pstStackTop->iCount = iArrayRealCount;
			pstStackTop->idxEntry = idxSubEntry;
			pstStackTop->iCutOffVersion = TDR_MAX_VERSION;
			pstStackTop->iRealCount = iArrayRealCount;
			pstStackTop->pstEntry = pstEntry;
			pstStackTop->iChange = 1;
		}else
		{
			/*简单数据类型*/
			ptrdiff_t iSize;

			iSize = tdr_get_entry_unitsize(pstEntry) * iArrayRealCount;
            if ((pszHostEnd - pszHostStart) < iSize)
            {
                iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);
                break;
            }

            iRet = tdr_output_simpleentry_json_i(a_pstIOStream, pstEntry, pszHostStart,
                                                 pszHostEnd, iArrayRealCount);
            if (TDR_SUCCESS != iRet)
            {
                break;
            }

			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
		}/*if (TDR_TYPE_COMPOSITE >= pstEntry->iType)*/
	}/*while (0 < iStackItemCount)*/

	return iRet;
}

int tdr_output_simpleentry_json_i(LPTDRIOSTREAM a_pstIOStream, LPTDRMETAENTRY a_pstEntry,
                                  const char* a_pszHostStart, const char* a_pszHostEnd, int a_iCount)
{
    int i = 0;
    int iRet = TDR_SUCCESS;

    if (NULL == a_pstIOStream || NULL == a_pszHostStart || NULL == a_pstEntry)
    {
		return TDR_ERRIMPLE_INVALID_PARAM;
    }

    if (1 == a_pstEntry->iCount)
    {
        iRet = tdr_iostream_write(a_pstIOStream, "\"%s\":", tdr_get_entry_name(a_pstEntry));
    }else
    {
        iRet = tdr_iostream_write(a_pstIOStream, "\"%s\":[", tdr_get_entry_name(a_pstEntry));
    }
    if (TDR_SUCCESS != iRet)
    {
        return iRet;
    }

    for (i = 0; i < a_iCount; i++)
    {
        if (0 < i)
        {
            iRet = tdr_iostream_write(a_pstIOStream, ",");
            if (TDR_SUCCESS != iRet)
            {
                break;
            }
        }

        iRet = tdr_json_ioprintf_basedtype_i(a_pstIOStream, a_pstEntry, &a_pszHostStart, a_pszHostEnd);
        if (TDR_SUCCESS != iRet)
        {
            break;
        }
    }/*for (i = 0; i < a_iCount; i++)	*/


    if (1 != a_pstEntry->iCount)
    {
        iRet = tdr_iostream_write(a_pstIOStream, "]", tdr_get_entry_name(a_pstEntry));
    }

    return iRet;
}

int tdr_input_json(IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstHost,
                   IN LPTDRDATA a_pstJson, IN int a_iCutOffVersion)
{
    int iRet = TDR_SUCCESS;
    TDRJSONPARSER stJsonParser;

    if ((NULL == a_pstMeta) || (NULL == a_pstJson)|| (NULL == a_pstHost))
    {
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
    }
    if ((NULL == a_pstJson->pszBuff) || (0 >= a_pstJson->iBuff) ||
        (NULL == a_pstHost->pszBuff) || (0 >= a_pstHost->iBuff))
    {
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
    }

    if (0 == a_iCutOffVersion)
    {
        a_iCutOffVersion = TDR_MAX_VERSION;
    }
    if (a_pstMeta->iBaseVersion > a_iCutOffVersion)
    {
        a_pstHost->iBuff = 0;
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
    }

    tdr_json_stream_init_by_buf(&stJsonParser.stStream, a_pstJson->pszBuff, a_pstJson->iBuff);

#define TDR_DEFAULT_JSON_LEVEL 20

    iRet = tdr_json_parser_init(&stJsonParser, tdr_json_state_proc, TDR_DEFAULT_JSON_LEVEL);
    if (0 != iRet)
    {
        iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_JSON_FAILED_TO_CREATE_PARSER);
    }

    if (0 == iRet)
    {
        iRet = tdr_json_input_i(a_pstMeta, a_pstHost, &stJsonParser, a_iCutOffVersion);
    }

    tdr_json_parser_fini(&stJsonParser);

    return iRet;
}

int tdr_input_json_fp(IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstHost,
                      IN FILE *a_fpJson, IN int a_iCutOffVersion)
{
    int iRet = TDR_SUCCESS;
    TDRJSONPARSER stJsonParser;

    if ((NULL == a_pstMeta) || (NULL == a_fpJson) || (NULL == a_pstHost))
    {
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
    }
    if (NULL == a_pstHost->pszBuff || 0 >= a_pstHost->iBuff)
    {
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
    }

    if (0 == a_iCutOffVersion)
    {
        a_iCutOffVersion = TDR_MAX_VERSION;
    }
    if (a_pstMeta->iBaseVersion > a_iCutOffVersion)
    {
        a_pstHost->iBuff = 0;
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
    }

    tdr_json_stream_init_by_fp(&stJsonParser.stStream, a_fpJson);

#define TDR_DEFAULT_JSON_LEVEL 20

    iRet = tdr_json_parser_init(&stJsonParser, tdr_json_state_proc, TDR_DEFAULT_JSON_LEVEL);
    if (0 != iRet)
    {
        iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_JSON_FAILED_TO_CREATE_PARSER);
    }

    if (0 == iRet)
    {
        iRet = tdr_json_input_i(a_pstMeta, a_pstHost, &stJsonParser, a_iCutOffVersion);
    }

    tdr_json_parser_fini(&stJsonParser);

    return iRet;
}

int tdr_input_json_file(IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstHost,
                        IN const char *a_pszFile, IN int a_iCutOffVersion)
{
    int iRet = TDR_SUCCESS;
    TDRJSONPARSER stJsonParser;
    FILE* fpInput = NULL;

    if ((NULL == a_pstMeta) || (NULL == a_pszFile) || (NULL == a_pstHost))
    {
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
    }
    if (NULL == a_pstHost->pszBuff || 0 >= a_pstHost->iBuff)
    {
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
    }

    if (0 == a_iCutOffVersion)
    {
        a_iCutOffVersion = TDR_MAX_VERSION;
    }
    if (a_pstMeta->iBaseVersion > a_iCutOffVersion)
    {
        a_pstHost->iBuff = 0;
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
    }

    fpInput = fopen(a_pszFile, "rb");
    if (NULL == fpInput)
    {
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_FAILED_OPEN_FILE_TO_READ);
    }

    tdr_json_stream_init_by_fp(&stJsonParser.stStream, fpInput);

#define TDR_DEFAULT_JSON_LEVEL 20

    iRet = tdr_json_parser_init(&stJsonParser, tdr_json_state_proc, TDR_DEFAULT_JSON_LEVEL);
    if (0 != iRet)
    {
        iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_JSON_FAILED_TO_CREATE_PARSER);
    }

    if (0 == iRet)
    {
        iRet = tdr_json_input_i(a_pstMeta, a_pstHost, &stJsonParser, a_iCutOffVersion);
    }

    tdr_json_parser_fini(&stJsonParser);
    fclose(fpInput);

    return iRet;
}

/** 从a_pstStream中解析出结构化数据到a_pstHost中
 * a_pstMeta 结构化数据的描述
 * a_pstHost 结构化数据的二进制内存
 * a_pstStream JSON解析器
 * a_iCutOffVersion 数据的剪裁版本，结构化数据的成员版本如果大于些值，则不解析出
 */
int tdr_json_input_i(IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstHost,
                     IN LPTDRJSONPARSER a_pstParser, IN int a_iCutOffVersion)
{
    int iRet = TDR_SUCCESS;
    LPTDRMETALIB pstLib;
    LPTDRMETA pstCurMeta;
    TDRSTACK  stStack;
    LPTDRSTACKITEM pstStackTop;
    int iStackItemCount;

    char *pszHostStart;
    char *pszHostEnd;
    int iChange;

    assert(NULL != a_pstMeta);
    assert(NULL != a_pstParser);
    assert(NULL != a_pstHost);
    assert(a_pstMeta->iBaseVersion <= a_iCutOffVersion);

    iRet = tdr_json_parser_match_struct_begin(a_pstParser);
    if (TDR_ERR_IS_ERROR(iRet))
    {
        return iRet;
    }

    pszHostStart = a_pstHost->pszBuff;
    pszHostEnd = a_pstHost->pszBuff + a_pstHost->iBuff;
    pstLib = TDR_META_TO_LIB(a_pstMeta);

    pstCurMeta = a_pstMeta;
    pstStackTop = &stStack[0];
    pstStackTop->pstMeta = pstCurMeta;
    pstStackTop->pszHostBase = pszHostStart;
    pstStackTop->iCount = 1;
    pstStackTop->idxEntry = 0;
    pstStackTop->iChange = 0;
    pstStackTop->iRealCount = 0; /*实际读取的结构数*/
    pstStackTop->szMetaEntryName[0] = '\0';
    iStackItemCount = 1;


    iChange = 0;
    while (0 < iStackItemCount)
    {
        LPTDRMETAENTRY pstEntry;
        int iCount;

        if (0 != iChange)
        {
            if (1 >= iStackItemCount)
            {
                iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_TOO_COMPLIEX_META);
                break;
            }

            iRet = tdr_json_parser_match_struct_end(a_pstParser);
            if (TDR_ERR_IS_ERROR(iRet))
            {
                break;
            }

            pstStackTop->iRealCount++;  /*已经读取一个结构/uion*/
            if (0 < pstStackTop->iCount)
            {
                iRet = tdr_json_parser_match_struct_begin(a_pstParser);
                if (TDR_ERR_IS_ERROR(iRet))
                {
                    break;
                }
            }
        }/*if ((0 != iChange)*/

        iChange = 0;

        if (0 >= pstStackTop->iCount)
        {/*当前元数据数组已经处理完毕*/
            pstStackTop--;
            iStackItemCount--;
            if (0 < iStackItemCount)
            {
                pstCurMeta = pstStackTop->pstMeta;
                pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;

                if (1 != pstEntry->iCount)
                {
                    iRet = tdr_json_parser_match_array_end(a_pstParser);
                    if (TDR_ERR_IS_ERROR(iRet))
                    {
                        break;
                    }
                }

                TDR_SET_ARRAY_REAL_COUNT((pstStackTop + 1)->iRealCount, pstEntry, pstStackTop->pszHostBase);
                TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
            }
            continue;
        }

        /* 由于 json 数据不能保证各个entry的顺序，
         * 这里就循环处理所有的当前 {} 中的 key:value 对，
         * 直到遇到 } 认为当前 meta 处理完。
         * 对于有 refer 属性的 entry 有一个逻辑缺陷:
         * 如果 refer 指向的成员存在，但是本身不存在，则不能验证有效性。
         */
        while (1)
        {
            iRet = tdr_json_parser_get_next_key_or_end(a_pstParser);
            if (TDR_ERR_IS_ERROR(iRet))
            {
                break;
            }

            if (!a_pstParser->stCtx.iStructEndBuf)
            {
                iRet = tdr_get_entry_by_name(&pstStackTop->idxEntry, pstCurMeta, a_pstParser->stCtx.pszBuf);
                if (TDR_ERR_IS_ERROR(iRet))
                {
                    iRet = tdr_json_parser_skip_value(a_pstParser);
                    if (TDR_ERR_IS_ERROR(iRet))
                    {
                        break;
                    } else
                    {
                        continue;
                    }
                }

                pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;
                pszHostStart = pstStackTop->pszHostBase + pstEntry->iHOff;


                if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry) || TDR_ENTRY_IS_REFER_TYPE(pstEntry))
                {
                    iRet = tdr_json_parser_skip_value(a_pstParser);
                    if (TDR_ERR_IS_ERROR(iRet))
                    {
                        break;
                    } else
                    {
                        continue;
                    }
                }

                TDR_GET_ARRAY_REAL_COUNT(iCount, pstEntry, pstStackTop->pszHostBase, a_iCutOffVersion);
                if (0 > iCount)
                {
                    iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_REFER_VALUE);
                    break;
                } else if (0 == iCount)
                {
                    iRet = tdr_json_parser_skip_value(a_pstParser);
                    if (TDR_ERR_IS_ERROR(iRet))
                    {
                        break;
                    } else
                    {
                        continue;
                    }
                }

                if (pstEntry->iVersion > a_iCutOffVersion)
                {
                    TDR_SET_DEFAULT_VALUE(iRet, pszHostStart, pszHostEnd, pstLib, pstEntry, pstEntry->iCount);
                    if (TDR_ERR_IS_ERROR(iRet))
                    {
                        break;
                    }else
                    {
                        iRet = tdr_json_parser_skip_value(a_pstParser);
                        if (TDR_ERR_IS_ERROR(iRet))
                        {
                            break;
                        } else
                        {
                            continue;
                        }
                    }
                }

                if (TDR_TYPE_COMPOSITE >= pstEntry->iType)
                {/*复合数据类型*/
                    int idxSubEntry;
                    LPTDRMETA pstTypeMeta;

                    if (TDR_STACK_SIZE <=  iStackItemCount)
                    {
                        iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_TOO_COMPLIEX_META);
                        break;
                    }

                    if (TDR_TYPE_UNION == pstEntry->iType)
                    {
                        TDR_GET_UNION_ENTRY_TYPE_META_INFO(pstStackTop->pszHostBase, pstLib,
                                                           pstEntry, a_iCutOffVersion, pstTypeMeta, idxSubEntry);
                        if (NULL == pstTypeMeta)
                        {
                            iRet = tdr_json_parser_skip_value(a_pstParser);
                            if (TDR_ERR_IS_ERROR(iRet))
                            {
                                break;
                            } else
                            {
                                continue;
                            }
                        }
                    }else
                    {
                        pstTypeMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
                        idxSubEntry = 0;
                    }/*if (TDR_TYPE_UNION == pstEntry->iType)*/

                    if (0 == pstStackTop->iRealCount)
                    {
                        iRet = tdr_json_parser_match_entry_begin(a_pstParser, pstEntry);
                        if (TDR_ERR_IS_ERROR(iRet))
                        {
                            break;
                        }
                    } else
                    {
                        iRet = tdr_json_parser_match_struct_begin(a_pstParser);
                        if (TDR_ERR_IS_ERROR(iRet))
                        {
                            break;
                        }
                    }

                    pstCurMeta = pstTypeMeta;
                    iStackItemCount++;
                    pstStackTop++;
                    pstStackTop->pstMeta = pstCurMeta;
                    pstStackTop->iCount = iCount;
                    pstStackTop->idxEntry = idxSubEntry;
                    pstStackTop->pszHostBase = pszHostStart;
                    pstStackTop->iRealCount = 0;

                    TDR_STRNCPY(pstStackTop->szMetaEntryName,
                                TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName),
                                sizeof(pstStackTop->szMetaEntryName));
                    pstStackTop->iChange = 1;
                    break;
                } else /*if (TDR_TYPE_COMPOSITE >= pstEntry->iType)*/
                {
                    int i = 0;

                    iRet = tdr_json_parser_match_entry_begin(a_pstParser, pstEntry);
                    if (TDR_ERR_IS_ERROR(iRet))
                    {
                        break;
                    }

                    for(i = 0; i < iCount; i++)
                    {
                        iRet = tdr_json_parser_simple_entry(a_pstParser, pstLib, pstEntry, &pszHostStart, pszHostEnd);
                        if (TDR_ERR_IS_ERROR(iRet))
                        {
                            break;
                        }
                    }
                    if (TDR_ERR_IS_ERROR(iRet))
                    {
                        break;
                    }

                    iRet = tdr_json_parser_match_entry_end(a_pstParser, pstEntry);
                    if (TDR_ERR_IS_ERROR(iRet))
                    {
                        break;
                    }
                    TDR_SET_ARRAY_REAL_COUNT(i, pstEntry, pstStackTop->pszHostBase);
                }
            } else
            {
                pstStackTop->idxEntry = 0;
                pstStackTop->iCount--;
                pstStackTop->pszHostBase += pstCurMeta->iHUnitSize;
                iChange = pstStackTop->iChange;

                break;
            }
        }

        if (TDR_ERR_IS_ERROR(iRet))
        {
            break;
        }
    }/*while (0 < iStackItemCount)*/

    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_json_parser_match_struct_end(a_pstParser);
    }

    a_pstHost->iBuff = pszHostStart - a_pstHost->pszBuff;
    return iRet;
}

TDR_API int tdr_input_probe(INOUT int* a_piMatchedIdx,
                            IN LPTDRMETA* a_pastMetas,
                            IN const char* a_pszFile)
{
    int iRet = TDR_SUCCESS;

    scew_tree* pstTree = NULL;
    scew_element *pstRoot = NULL;

    if (NULL == a_piMatchedIdx
        || NULL == a_pastMetas
        || NULL == a_pszFile)
    {
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
    }

    iRet = tdr_create_XMLParser_tree_byFileName(&pstTree, a_pszFile, stderr);
    if (TDR_ERR_IS_ERROR(iRet))
    {
        return iRet;
    }

    pstRoot = scew_tree_root(pstTree);

    assert(NULL != pstRoot);

    /* 限制 局部变量 的作用域 */
    {
        int iIdx = 0;
        const char* pszRootName = scew_element_name(pstRoot);

        *a_piMatchedIdx = -1;
        for (iIdx = 0; NULL != a_pastMetas[iIdx]; iIdx++)
        {
            if (0 == tdr_stricmp(pszRootName, tdr_get_meta_name(a_pastMetas[iIdx])))
            {
                *a_piMatchedIdx = iIdx;
                break;
            }
        }
    }

    scew_tree_free(pstTree);

    return iRet;
}
