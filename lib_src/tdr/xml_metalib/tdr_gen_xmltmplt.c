/**
 *
 * @file     tdr_gen_xmltmplt.c
 * @brief    TSF4G - generate a template for given metalib
 *
 * @author jackyai flyma
 * @version 1.0
 * @date 2009-9-10
 *
 *
 * Copyright (c)  2009, 腾讯科技有限公司互动娱乐研发部
 * All rights reserved.
 *
 */

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "tdr/tdr_data_io.h"
#include "tdr/tdr_error.h"
#include "tdr/tdr_metalib_manage_i.h"
#include "tdr/tdr_metalib_kernel_i.h"
#include "tdr/tdr_iostream_i.h"

#define TDR_MAX_ENTRIES_NUM_WITHOUT_ID  1
#define TDR_DEFAULT_ARRAY_DIMENSION     2

#define TDR_ERR_RETURN   !0
#define TDR_ERR_NO_RETURN  0

#define TDR_HAS_NO_DEEPER_COMMENT 0
#define TDR_HAS_DEEPER_COMMENT 1

#define TDR_HAS_ARRANGED !0
#define TDR_HAS_NOT_ARRANGED 0

#define TDR_INVALID_INDEX -1

#define TDR_HAS_OUTPUT 0
#define TDR_HAS_NOT_OUTPUT 1

#define TDR_UNIFORM_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange)    \
{                                                                       \
    if ((pstStackTop->idxEntry == pstStackTop->iEntrySizeInfoOff)       \
            && (TDR_HAS_NOT_OUTPUT == pstStackTop->iRealCount))         \
    {                                                                   \
        pstStackTop->iRealCount = TDR_HAS_OUTPUT;                       \
        pstStackTop->idxEntry = 0;                                      \
    }else                                                               \
    {                                                                   \
        pstStackTop->idxEntry++;                                        \
        if (pstStackTop->idxEntry >= pstCurMeta->iEntriesNum)           \
        {                                                               \
            pstStackTop->idxEntry = 0;                                  \
            pstStackTop->iCount--;                                      \
            pstStackTop->pszHostBase += pstCurMeta->iHUnitSize;         \
            iChange = pstStackTop->iChange;                             \
        }                                                               \
    }                                                                   \
}

#define TDR_ERR_CHECK_RETURN(iRet) \
{\
    if (TDR_ERR_IS_ERROR((iRet)))\
    {\
        return iRet;\
    }\
}\


int tdr_template_init_data(IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstData, IN int a_iCutOffVersion);

int tdr_template_format_output(IN LPTDRMETA a_pstMeta, IN FILE *a_fpFile,
        IN LPTDRDATA a_pstData, IN int a_iCutOffVersion, IN int a_iXmlDataFormat);

int tdr_template_list_output(IN LPTDRMETA a_pstMeta, IN LPTDRIOSTREAM a_pstIOStream,
        IN LPTDRDATA a_pstData, IN int a_iCutOffVersion);

int tdr_template_attr_output(IN LPTDRMETA a_pstMeta, IN LPTDRIOSTREAM a_pstIOStream,
        IN LPTDRDATA a_pstData, IN int a_iCutOffVersion);

int tdr_template_output_meta_attrs_i(IN LPTDRIOSTREAM a_pstIOStream, OUT int *a_piChildNum,
        IN LPTDRMETA a_pstMeta, IN int a_iFirstEntry, IN const char *a_pszHostStart,
        IN const char *a_pszHostEnd, int a_iVersion);

static int tdr_output_simpleentry_with_default_i(IN LPTDRIOSTREAM a_pstIOStream, IN LPTDRMETALIB a_pstLib,
        IN LPTDRMETAENTRY a_pstEntry, IN int a_iCount, IN char *a_pszHostStart,
        IN char *a_pszHostEnd, IN char *a_pszSpace);

int tdr_template_set_arr_count(IN LPTDRSTACKITEM a_pstStackTop,
        IN LPTDRMETAENTRY a_pstEntry, INOUT int * a_iArrayRealCount);

int tdr_init_union_entry(INOUT LPTDRSTACKITEM pstStackTop, IN LPTDRMETAENTRY pstEntry,
        IN LPTDRMETALIB pstLib, IN const char * pszHostEnd, IN int iArrayRealCount);

int tdr_template_get_arr_count(IN LPTDRSTACKITEM a_pstStackTop,
        IN LPTDRMETAENTRY a_pstEntry, INOUT int * a_iArrayRealCount);

int tdr_template_output_child_desc(IN LPTDRIOSTREAM a_pstIOStream,
        IN const char* a_pszParent, IN LPTDRMETA a_pstMeta, IN const char* a_szSpace);

int tdr_template_output_union_desc(IN LPTDRIOSTREAM a_pstIOStream,
        IN LPTDRSTACKITEM a_pstStackTop, IN LPTDRMETA a_pstCurMeta,
        IN LPTDRMETAENTRY a_pstEntry, IN char a_szSpace[]);

int tdr_template_output_arr_desc(IN LPTDRIOSTREAM a_pstIOStream,
        IN LPTDRSTACKITEM a_pstStackTop, IN LPTDRMETA a_pstCurMeta,
        IN LPTDRMETAENTRY a_pstEntry, IN char a_szSpace[]);

int tdr_output_simpleentry_i(INOUT LPTDRIOSTREAM a_pstIOStream, IN LPTDRMETALIB a_pstLib,
        IN LPTDRMETAENTRY a_pstEntry, IN int a_iCount,
        IN char *a_pszHostStart, IN char *a_pszHostEnd, IN char *a_pszSpace);

extern char g_szEncoding[128];

int tdr_esc_xml_i(INOUT LPTDRIOSTREAM a_pstIOStream, IN const char *a_pszHostStart, IN ptrdiff_t a_iSize);

// generate a XML template for a given Meta
int tdr_gen_xmltmplt(IN LPTDRMETA a_pstMeta,
        IN FILE *a_fpFile,
        IN int a_iCutOffVersion,
        IN int a_iXmlDataFormat)
{
    int iRet = TDR_SUCCESS;
    TDRDATA stHostInfo;
    void *ptrData = NULL;

    if (NULL == a_pstMeta || NULL == a_fpFile)
    {
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
    }

    if (0 == a_iCutOffVersion)
    {
        a_iCutOffVersion = a_pstMeta->iCurVersion;
    }

    // alocate space for sotring data
    stHostInfo.iBuff = (size_t)(a_pstMeta->iHUnitSize);
    ptrData = malloc(stHostInfo.iBuff);
    if (NULL == ptrData)
    {
        iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_MEMORY);
    }

    if (TDR_SUCCESS == iRet)
    {
        stHostInfo.pszBuff = (char*)ptrData;
        iRet = tdr_template_init_data(a_pstMeta, &stHostInfo, a_iCutOffVersion);
    }

    if (TDR_SUCCESS == iRet)
    {
        iRet = tdr_template_format_output(a_pstMeta, a_fpFile,
                &stHostInfo, a_iCutOffVersion, a_iXmlDataFormat);
    }

    if (NULL != ptrData)
    {
        free(ptrData);
    }

    return iRet;
}

// depth-firstly initialize a meta's default data, untill union meeted.
// uninitialized meta will be initialized when that meta to be output
int tdr_template_init_data(IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstData, IN int a_iCutOffVersion)
{
    int iRet = TDR_SUCCESS;

    char *pszHostStart = a_pstData->pszBuff;
    char *pszHostEnd = a_pstData->pszBuff + a_pstData->iBuff;
    int iStackItemCount = 1;
    int iChange = 0;
    int iArrayRealCount = 0;

    LPTDRMETALIB  pstLib = TDR_META_TO_LIB(a_pstMeta);
    LPTDRMETA pstCurMeta = a_pstMeta;
    TDRSTACK stStack;
    LPTDRSTACKITEM pstStackTop = &stStack[0];
    LPTDRMETAENTRY pstEntry = NULL;

    assert(NULL != a_pstMeta);
    assert(NULL != a_pstData);
    if (0 == a_iCutOffVersion)
    {
        a_iCutOffVersion = a_pstMeta->iCurVersion;
    }
    assert(a_pstMeta->iBaseVersion <= a_iCutOffVersion);
    assert(NULL != a_pstData->pszBuff);
    assert(0 < a_pstData->iBuff);


    pstStackTop->pstMeta = pstCurMeta;
    pstStackTop->pszHostBase = pszHostStart;
    pstStackTop->iCount = 1;
    pstStackTop->idxEntry = 0;
    pstStackTop->iChange = 0;
    pstStackTop->szMetaEntryName[0] = '\0';

    while (0 < iStackItemCount)
    {
        if (0 >= pstStackTop->iCount)
        {
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
        if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry) || TDR_ENTRY_IS_REFER_TYPE(pstEntry))
        {
            TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
            continue;
        }
        pszHostStart = pstStackTop->pszHostBase + pstEntry->iHOff;


        iRet = tdr_template_set_arr_count(pstStackTop, pstEntry, &iArrayRealCount);
        TDR_ERR_CHECK_RETURN(iRet);
        if (0 >= iArrayRealCount)
        {
            TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
            continue;
        }


        if (TDR_TYPE_COMPOSITE >= pstEntry->iType)
        {
            if (TDR_TYPE_UNION == pstEntry->iType)
            {
                int idxSubEntry = 0;
                LPTDRMETA pstTypeMeta = NULL;

                TDR_GET_UNION_ENTRY_TYPE_META_INFO(pstStackTop->pszHostBase, pstLib, pstEntry,
                        a_iCutOffVersion, pstTypeMeta, idxSubEntry);

                // adjust selector's value
                if (NULL == pstTypeMeta)
                {
                    int i = 0;
                    int iDefaultId = -1;
                    char *pszPtr = NULL;
                    LPTDRMETAENTRY pstChildEntry = NULL;

                    pstCurMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
                    for (i = 0; i < pstCurMeta->iEntriesNum; i++)
                    {
                        pstChildEntry = pstCurMeta->stEntries + i;
                        if (pstChildEntry->iVersion > a_iCutOffVersion)
                            continue;
                        if (TDR_ENTRY_DO_HAVE_ID(pstChildEntry))
                        {
                            iDefaultId = pstChildEntry->iID;
                            break;
                        }else if (TDR_ENTRY_DO_HAVE_MAXMIN_ID(pstChildEntry))
                        {
                            iDefaultId = pstChildEntry->iMinId;
                            break;
                        }
                    }

                    if (-1 == iDefaultId)
                        iDefaultId = 0;
                    pszPtr = pstStackTop->pszHostBase + pstEntry->stSelector.iHOff;
                    TDR_SET_INT(pszPtr, pstEntry->stSelector.iUnitSize, iDefaultId);
                } /* if (NULL = pstTypeMeta) */

                TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
                continue;
            }

            pstCurMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
            iStackItemCount++;
            pstStackTop++;
            pstStackTop->pstMeta = pstCurMeta;
            pstStackTop->iCount = iArrayRealCount;
            pstStackTop->idxEntry = 0;
            pstStackTop->pszHostBase = pszHostStart;
            pstStackTop->iChange = 1;
            continue;

        }else
        {
            TDR_SET_DEFAULT_VALUE(iRet, pszHostStart, pszHostEnd, pstLib, pstEntry, iArrayRealCount);
            TDR_ERR_CHECK_RETURN(iRet);
            TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
            continue;
        }
    }

    return iRet;
}

// a wrapper to tdr_template_list_output
// firstly check and adjust some parametes
// finally call tdr_template_list_output
int tdr_template_format_output(IN LPTDRMETA a_pstMeta,
        IN FILE *a_fpFile,
        IN LPTDRDATA a_pstData,
        IN int a_iCutOffVersion,
        IN int a_iXmlDataFormat)
{
    int iRet = TDR_SUCCESS;

    TDRIOSTREAM stIOStream;

    assert(NULL != a_pstMeta);
    assert(NULL != a_fpFile);
    assert(NULL != a_pstData);
    if (0 == a_iCutOffVersion)
    {
        a_iCutOffVersion = a_pstMeta->iCurVersion;
    }
    assert(a_pstMeta->iBaseVersion <= a_iCutOffVersion);
    assert(NULL != a_pstData->pszBuff);
    assert(0 < a_pstData->iBuff);

    fprintf(a_fpFile, "<?xml version=\"1.0\" encoding=\"%s\" standalone=\"yes\" ?>\n\n", g_szEncoding);

    stIOStream.emIOStreamType = TDR_IOSTREAM_FILE;
    stIOStream.fpTDRIO = a_fpFile;

    switch (a_iXmlDataFormat)
    {
        case TDR_XML_DATA_FORMAT_LIST_ENTRY_NAME:
            iRet = tdr_template_list_output(a_pstMeta, &stIOStream, a_pstData, a_iCutOffVersion);
            break;
        case TDR_XML_DATA_FORMAT_ATTR_ENTRY_NAME:
            iRet = tdr_template_attr_output(a_pstMeta, &stIOStream, a_pstData, a_iCutOffVersion);
            break;
        default:
            iRet = tdr_template_list_output(a_pstMeta, &stIOStream, a_pstData, a_iCutOffVersion);
            break;
    }


    return iRet;
}

// depth-firstly output a meta's default data
// when uninitialized entry meeted,
// first initialize it, and then output its default data
int tdr_template_list_output(IN LPTDRMETA a_pstMeta,
        IN LPTDRIOSTREAM a_pstIOStream,
        IN LPTDRDATA a_pstData,
        IN int a_iCutOffVersion)
{
    int iRet = TDR_SUCCESS;

    char *pszHostStart = a_pstData->pszBuff;
    char *pszHostEnd = a_pstData->pszBuff + a_pstData->iBuff;
    char szSpace[TDR_STACK_SIZE * TDR_TAB_SIZE + 1];
    int iCutOffVersion = 0;
    int iStackItemCount = 1;
    int iChange = 0;

    LPTDRMETA pstCurMeta = a_pstMeta;
    LPTDRMETALIB pstLib = TDR_META_TO_LIB(a_pstMeta);
    TDRSTACK stStack;
    LPTDRSTACKITEM pstStackTop = &stStack[0];

    assert(NULL != a_pstMeta);
    assert(NULL != a_pstIOStream);
    assert(NULL != a_pstData);
    if (0 == a_iCutOffVersion)
    {
        a_iCutOffVersion = a_pstMeta->iCurVersion;
    }
    assert(a_pstMeta->iBaseVersion <= a_iCutOffVersion);
    assert(NULL != a_pstData->pszBuff);
    assert(0 < a_pstData->iBuff);

    memset(szSpace, ' ', sizeof(szSpace));
    szSpace[sizeof(szSpace)-1] = '\0';

    pstStackTop->pstMeta = pstCurMeta;
    pstStackTop->pszHostBase = pszHostStart;

    // iEntrySizeInfoOff is used to record select's value
    pstStackTop->iEntrySizeInfoOff = TDR_INVALID_INDEX;
    pstStackTop->pszHostEnd = pszHostEnd;
    pstStackTop->iCount = 1;

    // iRealCount is used to record whether
    // selected union entry has been outputed
    pstStackTop->iRealCount = TDR_HAS_OUTPUT;
    pstStackTop->idxEntry = 0;
    pstStackTop->iChange = 1;
    pstStackTop->szMetaEntryName[0] = '\0';

    // iMetaSizeInfoOff is used to record indent size
    pstStackTop->iMetaSizeInfoOff = 1;

    iCutOffVersion = a_iCutOffVersion;
    TDR_ERR_CHECK_RETURN(iRet);
    pstStackTop->iCutOffVersion = iCutOffVersion;

    if (TDR_INVALID_PTR != pstCurMeta->ptrDesc)
    {
        iRet = tdr_iostream_write(a_pstIOStream, "<!--%s: %s-->\n",
                TDR_GET_STRING_BY_PTR(pstLib,pstCurMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib, pstCurMeta->ptrDesc));
        TDR_ERR_CHECK_RETURN(iRet);
    }

    TDR_STRNCPY(pstStackTop->szMetaEntryName, TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), sizeof(pstStackTop->szMetaEntryName));
    iRet = tdr_iostream_write(a_pstIOStream, "<%s __version=\"%d\">\n", TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), pstStackTop->iCutOffVersion);
    TDR_ERR_CHECK_RETURN(iRet);

    while (0 < iStackItemCount)
    {
        LPTDRMETAENTRY pstEntry = NULL;
        int iArrayRealCount = 0;
        // record which entry of an union is refered by select's value
        int iSelectedEntryIdx = TDR_INVALID_INDEX;
        int idxSubEntry = 0;

        // a struct or union has been output
        if (0 != iChange)
        {
            iChange = 0;
            szSpace[(pstStackTop->iMetaSizeInfoOff-1) * TDR_TAB_SIZE] = '\0';
            iRet = tdr_iostream_write(a_pstIOStream, "%s</%s>\n", szSpace, pstStackTop->szMetaEntryName);
            TDR_ERR_CHECK_RETURN(iRet);
            szSpace[(pstStackTop->iMetaSizeInfoOff-1) * TDR_TAB_SIZE] = ' ';
            pstStackTop->iCode = 0;

            // if pstCurMeta is an array,
            // it has more items to deal with
            if (0 < pstStackTop->iCount)
            {
                szSpace[(pstStackTop->iMetaSizeInfoOff-1) * TDR_TAB_SIZE] = '\0';
                pszHostStart = pstStackTop->pszHostBase;
                iRet = tdr_iostream_write(a_pstIOStream, "\n%s<%s>\n", szSpace, pstStackTop->szMetaEntryName);
                TDR_ERR_CHECK_RETURN(iRet);
                szSpace[(pstStackTop->iMetaSizeInfoOff-1) * TDR_TAB_SIZE] = ' ';
            }
        }

        if (0 >= pstStackTop->iCount)
        {
            pstStackTop--;
            iStackItemCount--;
            if (0 < iStackItemCount)
            {
                pstCurMeta = pstStackTop->pstMeta;
                szSpace[pstStackTop->iMetaSizeInfoOff * TDR_TAB_SIZE] = '\0';
                if ((TDR_TYPE_UNION == pstStackTop->pstMeta->iType)
                        && (TDR_INVALID_INDEX != pstStackTop->iEntrySizeInfoOff)
                        && (pstStackTop->iEntrySizeInfoOff != pstStackTop->idxEntry))
                {
                    ; // just empty
                } else
                {
                    iRet = tdr_iostream_write(a_pstIOStream, "\n");
                }
                TDR_ERR_CHECK_RETURN(iRet);
                szSpace[pstStackTop->iMetaSizeInfoOff * TDR_TAB_SIZE] = ' ';
                TDR_UNIFORM_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
            }
            continue;
        }

        pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;
        if (   (TDR_IO_NOOUTPUT & pstEntry->iIO)
                || TDR_ENTRY_IS_REFER_TYPE(pstEntry)
                || TDR_ENTRY_IS_POINTER_TYPE(pstEntry)
                || pstEntry->iVersion > pstStackTop->iCutOffVersion)
        {
            TDR_UNIFORM_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
            continue;
        }

        szSpace[pstStackTop->iMetaSizeInfoOff * TDR_TAB_SIZE] = '\0';

        pszHostStart = pstStackTop->pszHostBase + pstEntry->iHOff;

        iRet = tdr_template_get_arr_count(pstStackTop, pstEntry, &iArrayRealCount);
        if (TDR_ERR_IS_ERROR(iRet))
        {
            return iRet;
        }
        if (0 >= iArrayRealCount)
        {
            TDR_UNIFORM_GET_NEXT_ENTRY(pstStackTop, a_pstMeta, iChange);
            continue;
        }

        if (TDR_TYPE_UNION == pstCurMeta->iType)
        {
            if ((pstStackTop->idxEntry == pstStackTop->iEntrySizeInfoOff)
                    && (TDR_HAS_OUTPUT == pstStackTop->iRealCount))
            {
                // this entry has been output as first entry of union
                TDR_UNIFORM_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
                continue;
            }
            iRet = tdr_init_union_entry(pstStackTop, pstEntry, pstLib, pszHostEnd, iArrayRealCount);
            TDR_ERR_CHECK_RETURN(iRet);
            iRet = tdr_template_output_union_desc(a_pstIOStream,
                    pstStackTop, pstCurMeta, pstEntry, szSpace);
            TDR_ERR_CHECK_RETURN(iRet);
        } else
        {
            if (0 == pstStackTop->idxEntry)
            {
                iRet = tdr_iostream_write(a_pstIOStream, "\n");
                TDR_ERR_CHECK_RETURN(iRet);
            }

            if (TDR_INVALID_PTR != pstEntry->ptrDesc)
            {
                // when pstCurMeta is not an union,
                // output blank lines before this entry
                iRet = tdr_iostream_write(a_pstIOStream, "%s<!--%s: %s-->\n", szSpace,
                        TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName), TDR_GET_STRING_BY_PTR(pstLib, pstEntry->ptrDesc));
                TDR_ERR_CHECK_RETURN(iRet);
            }
        }

        if (1 < pstEntry->iCount || 0 == pstEntry->iCount)
        {
            iRet = tdr_template_output_arr_desc(a_pstIOStream, pstStackTop, pstCurMeta, pstEntry, szSpace);
            TDR_ERR_CHECK_RETURN(iRet);
        }
        szSpace[pstStackTop->iMetaSizeInfoOff * TDR_TAB_SIZE] = ' ';

        // pstEntry is an union or struct
        if (TDR_TYPE_COMPOSITE >= pstEntry->iType)
        {
            LPTDRMETA pstTypeMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);

            if (TDR_STACK_SIZE <= iStackItemCount)
            {
                iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_TOO_COMPLIEX_META);
                return iRet;
            }

            idxSubEntry = 0;
            if (TDR_TYPE_UNION == pstEntry->iType)
            {
                LPTDRMETA pstSelectedMeta = NULL;
                TDR_GET_UNION_ENTRY_TYPE_META_INFO(pstStackTop->pszHostBase, pstLib,
                        pstEntry,pstStackTop->iCutOffVersion, pstSelectedMeta, iSelectedEntryIdx);
                if (NULL == pstSelectedMeta)
                {
                    //pstSelectedMeta is NULL mean pstCurMeta needn't be output
                    TDR_UNIFORM_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
                    continue;
                }
            }

            pstCurMeta = pstTypeMeta;
            pstStackTop++;
            pstStackTop->pstMeta = pstCurMeta;
            pstStackTop->iCount = iArrayRealCount;
            pstStackTop->idxEntry = idxSubEntry;
            pstStackTop->pszHostBase = pszHostStart;

            if (TDR_INVALID_INDEX != iSelectedEntryIdx)
            {
                pstStackTop->iRealCount = TDR_HAS_NOT_OUTPUT;

                // record which entry of an union is refered by select's value
                pstStackTop->iEntrySizeInfoOff = iSelectedEntryIdx;

                // refered entry should be output at first
                // record this entry's index in idxEntry
                pstStackTop->idxEntry = iSelectedEntryIdx;
            }else
            {
                pstStackTop->iEntrySizeInfoOff = TDR_INVALID_INDEX;
            }

            // increase indent level
            pstStackTop->iMetaSizeInfoOff = (pstStackTop - 1)->iMetaSizeInfoOff + 1;

            pstStackTop->iChange = 1;
            iCutOffVersion = a_iCutOffVersion;
            TDR_ERR_CHECK_RETURN(iRet);
            pstStackTop->iCutOffVersion = iCutOffVersion;
            iStackItemCount++;

            TDR_STRNCPY(pstStackTop->szMetaEntryName, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName), sizeof(pstStackTop->szMetaEntryName));
            szSpace[(pstStackTop->iMetaSizeInfoOff-1)*TDR_TAB_SIZE] = '\0';

            // if entry is an union, first output some description
            if (TDR_TYPE_UNION == pstCurMeta->iType)
            {
                LPTDRMETAENTRY pstSelEntry;
                iRet = tdr_iostream_write(a_pstIOStream, "\n%s<!--\'%s\' defines some options-->\n", szSpace, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName));
                TDR_ERR_CHECK_RETURN(iRet);
                pstSelEntry = TDR_PTR_TO_ENTRY(pstLib, pstEntry->stSelector.ptrEntry);
                iRet = tdr_iostream_write(a_pstIOStream, "%s<!--\'%s\' decides which is used-->\n",
                        szSpace, TDR_GET_STRING_BY_PTR(pstLib, pstSelEntry->ptrName));
                TDR_ERR_CHECK_RETURN(iRet);
                pstStackTop->pszNetBase = ((pstLib->data + pstEntry->stSelector.ptrEntry));
            }

            iRet = tdr_iostream_write(a_pstIOStream, "%s<%s>\n", szSpace, pstStackTop->szMetaEntryName);
            TDR_ERR_CHECK_RETURN(iRet);

            szSpace[(pstStackTop->iMetaSizeInfoOff-1)*TDR_TAB_SIZE] = ' ';

            continue;
        }/* if (TDR_TYPE_COMPOSITE >= pstEntry->iType) */

        // the entry is a simple entry
        szSpace[pstStackTop->iMetaSizeInfoOff*TDR_TAB_SIZE] = '\0';
        iRet = tdr_output_simpleentry_with_default_i(a_pstIOStream, pstLib, pstEntry,
                iArrayRealCount, pszHostStart, pszHostEnd, &szSpace[0]);
        TDR_ERR_CHECK_RETURN(iRet);

        if (TDR_TYPE_UNION != pstCurMeta->iType)
        {
            iRet = tdr_iostream_write(a_pstIOStream, "\n");
            TDR_ERR_CHECK_RETURN(iRet);
        }

        szSpace[pstStackTop->iMetaSizeInfoOff*TDR_TAB_SIZE] = ' ';

        TDR_UNIFORM_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
    }/* while (0 < iStackItemCount) */

    return 0;
}

// depth-firstly output a meta's default data
// when uninitialized entry meeted,
// first initialize it, and then output its default data
int tdr_template_attr_output(IN LPTDRMETA a_pstMeta,
        IN LPTDRIOSTREAM a_pstIOStream,
        IN LPTDRDATA a_pstData,
        IN int a_iCutOffVersion)
{
    int iRet = TDR_SUCCESS;

    char *pszHostStart = a_pstData->pszBuff;
    char *pszHostEnd = a_pstData->pszBuff + a_pstData->iBuff;
    char szSpace[TDR_STACK_SIZE * TDR_TAB_SIZE + 1];
    int iCutOffVersion = 0;
    int iStackItemCount = 1;
    char *pszFormatString = NULL;
    int iChange = 0;
    int iChildNodeNum = 0;

    LPTDRMETA pstCurMeta = a_pstMeta;
    LPTDRMETALIB pstLib = TDR_META_TO_LIB(a_pstMeta);
    TDRSTACK stStack;
    LPTDRSTACKITEM pstStackTop = &stStack[0];

    assert(NULL != a_pstMeta);
    assert(NULL != a_pstIOStream);
    assert(NULL != a_pstData);
    if (0 == a_iCutOffVersion)
    {
        a_iCutOffVersion = a_pstMeta->iCurVersion;
    }
    assert(a_pstMeta->iBaseVersion <= a_iCutOffVersion);
    assert(NULL != a_pstData->pszBuff);
    assert(0 < a_pstData->iBuff);

    memset(szSpace, ' ', sizeof(szSpace));
    szSpace[sizeof(szSpace)-1] = '\0';

    pstStackTop->pstMeta = pstCurMeta;
    pstStackTop->pszHostBase = pszHostStart;

    // iEntrySizeInfoOff is used to record select's value
    pstStackTop->iEntrySizeInfoOff = TDR_INVALID_INDEX;
    pstStackTop->pszHostEnd = pszHostEnd;
    pstStackTop->iCount = 1;

    // iRealCount is used to record whether
    // selected union entry has been outputed
    pstStackTop->iRealCount = TDR_HAS_OUTPUT;
    pstStackTop->idxEntry = 0;
    pstStackTop->iChange = 1;
    pstStackTop->szMetaEntryName[0] = '\0';

    // iMetaSizeInfoOff is used to record indent size
    pstStackTop->iMetaSizeInfoOff = 1;

    iCutOffVersion = a_iCutOffVersion;
    TDR_ERR_CHECK_RETURN(iRet);
    pstStackTop->iCutOffVersion = iCutOffVersion;

    if (TDR_INVALID_PTR != pstCurMeta->ptrDesc)
    {
        iRet = tdr_iostream_write(a_pstIOStream, "<!--%s: %s-->\n",
                TDR_GET_STRING_BY_PTR(pstLib,pstCurMeta->ptrName), TDR_GET_STRING_BY_PTR(pstLib, pstCurMeta->ptrDesc));
        TDR_ERR_CHECK_RETURN(iRet);
    }

    iRet = tdr_template_output_child_desc(a_pstIOStream, TDR_GET_STRING_BY_PTR(pstLib,pstCurMeta->ptrName), pstCurMeta, "");
    TDR_ERR_CHECK_RETURN(iRet);

    TDR_STRNCPY(pstStackTop->szMetaEntryName, TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), sizeof(pstStackTop->szMetaEntryName));
    iRet = tdr_iostream_write(a_pstIOStream, "<%s", TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName));
    TDR_ERR_CHECK_RETURN(iRet);

    iRet = tdr_template_output_meta_attrs_i(a_pstIOStream, &iChildNodeNum, pstCurMeta,
            0, pszHostStart, pszHostEnd, pstStackTop->iCutOffVersion);
    TDR_ERR_CHECK_RETURN(iRet);

    pstStackTop->iCode = iChildNodeNum;
    pszFormatString = (0 == pstStackTop->iCode) ?  " __version=\"%d\"/>\n" : " __version=\"%d\">\n\n";
    iRet = tdr_iostream_write(a_pstIOStream, pszFormatString, pstStackTop->iCutOffVersion);
    TDR_ERR_CHECK_RETURN(iRet);

    while (0 < iStackItemCount)
    {
        LPTDRMETAENTRY pstEntry = NULL;
        int iArrayRealCount = 0;
        // record which entry of an union is refered by select's value
        int iSelectedEntryIdx = TDR_INVALID_INDEX;
        int idxSubEntry = 0;

        // a struct or union has been output
        if (0 != iChange)
        {
            iChange = 0;
            if (0 < pstStackTop->iCode)
            {
                szSpace[(pstStackTop->iMetaSizeInfoOff-1) * TDR_TAB_SIZE] = '\0';
                iRet = tdr_iostream_write(a_pstIOStream, "%s</%s>\n\n", szSpace, pstStackTop->szMetaEntryName);
                TDR_ERR_CHECK_RETURN(iRet);
                szSpace[(pstStackTop->iMetaSizeInfoOff-1) * TDR_TAB_SIZE] = ' ';
                pstStackTop->iCode = 0;
            }/* if (0 < pstStackTop->iCode) */

            if (0 < pstStackTop->iCount)
            {
                // pstCurMeta is an array,
                // it has more items to deal with
                szSpace[(pstStackTop->iMetaSizeInfoOff-1) * TDR_TAB_SIZE] = '\0';
                pszHostStart = pstStackTop->pszHostBase;

                iRet = tdr_iostream_write(a_pstIOStream, "%s<%s", szSpace, pstStackTop->szMetaEntryName);
                TDR_ERR_CHECK_RETURN(iRet);

                iRet = tdr_template_output_meta_attrs_i(a_pstIOStream, &iChildNodeNum, pstCurMeta,
                        0, pszHostStart, pszHostEnd, pstStackTop->iCutOffVersion);
                TDR_ERR_CHECK_RETURN(iRet);

                pstStackTop->iCode = iChildNodeNum;
                pszFormatString = (0 == pstStackTop->iCode) ?  "/>\n\n" : ">\n\n";
                iRet = tdr_iostream_write(a_pstIOStream, "%s", pszFormatString);
                TDR_ERR_CHECK_RETURN(iRet);

                szSpace[(pstStackTop->iMetaSizeInfoOff-1) * TDR_TAB_SIZE] = ' ';
            }/* if (0 < pstStackTop->iCount) */
        }/* if (0 != iChange) */

        if (0 >= pstStackTop->iCount)
        {
            pstStackTop--;
            iStackItemCount--;
            if (0 < iStackItemCount)
            {
                pstCurMeta = pstStackTop->pstMeta;
                TDR_UNIFORM_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
            }
            continue;
        }

        pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;
        if (   (TDR_IO_NOOUTPUT & pstEntry->iIO)
                || TDR_ENTRY_IS_REFER_TYPE(pstEntry)
                || TDR_ENTRY_IS_POINTER_TYPE(pstEntry)
                || pstEntry->iVersion > pstStackTop->iCutOffVersion)
        {
            TDR_UNIFORM_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
            continue;
        }

        if ((TDR_TYPE_STRUCT == pstCurMeta->iType)
                && (TDR_TYPE_COMPOSITE < pstEntry->iType)
                && (1 == pstEntry->iCount))
        {
            // this entry has been outputed as pstCurMeta's attibute
            TDR_UNIFORM_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
            continue;
        }

        szSpace[pstStackTop->iMetaSizeInfoOff * TDR_TAB_SIZE] = '\0';

        pszHostStart = pstStackTop->pszHostBase + pstEntry->iHOff;

        iRet = tdr_template_get_arr_count(pstStackTop, pstEntry, &iArrayRealCount);
        if (TDR_ERR_IS_ERROR(iRet))
        {
            return iRet;
        }
        if (0 >= iArrayRealCount)
        {
            TDR_UNIFORM_GET_NEXT_ENTRY(pstStackTop, a_pstMeta, iChange);
            continue;
        }

        if (TDR_TYPE_UNION == pstCurMeta->iType)
        {
            if ((pstStackTop->idxEntry == pstStackTop->iEntrySizeInfoOff)
                    && (TDR_HAS_OUTPUT == pstStackTop->iRealCount))
            {
                // this entry has been output as first entry of union
                TDR_UNIFORM_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
                continue;
            }

            iRet = tdr_init_union_entry(pstStackTop, pstEntry, pstLib, pszHostEnd, iArrayRealCount);
            TDR_ERR_CHECK_RETURN(iRet);
            iRet = tdr_template_output_union_desc(a_pstIOStream,
                    pstStackTop, pstCurMeta, pstEntry, szSpace);
            TDR_ERR_CHECK_RETURN(iRet);
        } else
        {
            if (TDR_INVALID_PTR != pstEntry->ptrDesc)
            {
                // when pstCurMeta is not an union,
                // output blank lines before this entry
                iRet = tdr_iostream_write(a_pstIOStream, "%s<!--%s: %s-->\n", szSpace,
                        TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName), TDR_GET_STRING_BY_PTR(pstLib, pstEntry->ptrDesc));
                TDR_ERR_CHECK_RETURN(iRet);
            }
        }


        if (1 < pstEntry->iCount || 0 == pstEntry->iCount)
        {
            iRet = tdr_template_output_arr_desc(a_pstIOStream, pstStackTop, pstCurMeta, pstEntry, szSpace);
            TDR_ERR_CHECK_RETURN(iRet);
        }
        szSpace[pstStackTop->iMetaSizeInfoOff * TDR_TAB_SIZE] = ' ';

        // pstEntry is an union or struct
        if (TDR_TYPE_COMPOSITE >= pstEntry->iType)
        {
            LPTDRMETA pstTypeMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);

            if (TDR_STACK_SIZE <= iStackItemCount)
            {
                iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_TOO_COMPLIEX_META);
                return iRet;
            }

            idxSubEntry = 0;
            if (TDR_TYPE_UNION == pstEntry->iType)
            {
                LPTDRMETA pstSelectedMeta = NULL;
                TDR_GET_UNION_ENTRY_TYPE_META_INFO(pstStackTop->pszHostBase, pstLib,
                        pstEntry,pstStackTop->iCutOffVersion, pstSelectedMeta, iSelectedEntryIdx);
                if (NULL == pstSelectedMeta)
                {
                    //pstSelectedMeta is NULL mean pstCurMeta needn't be output
                    TDR_UNIFORM_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
                    continue;
                }
            }

            pstCurMeta = pstTypeMeta;
            pstStackTop++;
            pstStackTop->pstMeta = pstCurMeta;
            pstStackTop->iCount = iArrayRealCount;
            pstStackTop->idxEntry = idxSubEntry;
            pstStackTop->pszHostBase = pszHostStart;

            if (TDR_INVALID_INDEX != iSelectedEntryIdx)
            {
                pstStackTop->iRealCount = TDR_HAS_NOT_OUTPUT;

                // record which entry of an union is refered by select's value
                pstStackTop->iEntrySizeInfoOff = iSelectedEntryIdx;

                // refered entry should be output at first
                // record this entry's index in idxEntry
                pstStackTop->idxEntry = iSelectedEntryIdx;
            }else
            {
                pstStackTop->iEntrySizeInfoOff = TDR_INVALID_INDEX;
            }

            // increase indent level
            pstStackTop->iMetaSizeInfoOff = (pstStackTop - 1)->iMetaSizeInfoOff + 1;

            pstStackTop->iChange = 1;
            iCutOffVersion = a_iCutOffVersion;
            TDR_ERR_CHECK_RETURN(iRet);
            pstStackTop->iCutOffVersion = iCutOffVersion;
            iStackItemCount++;

            TDR_STRNCPY(pstStackTop->szMetaEntryName, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName), sizeof(pstStackTop->szMetaEntryName));
            szSpace[(pstStackTop->iMetaSizeInfoOff-1)*TDR_TAB_SIZE] = '\0';

            // union's simple entry will not be output as attribute
            // so, there is no need to pre-output simple entry's desc attribute
            if ((TDR_TYPE_UNION != pstCurMeta->iType))
            {
                iRet = tdr_template_output_child_desc(a_pstIOStream, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName), pstCurMeta, szSpace);
                TDR_ERR_CHECK_RETURN(iRet);
            }

            // if entry is an union, first output some description
            if (TDR_TYPE_UNION == pstCurMeta->iType)
            {
                LPTDRMETAENTRY pstSelEntry = TDR_PTR_TO_ENTRY(pstLib, pstEntry->stSelector.ptrEntry);
                iRet = tdr_iostream_write(a_pstIOStream, "\n%s<!--\'%s\' defines some options-->\n", szSpace, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName));
                TDR_ERR_CHECK_RETURN(iRet);
                iRet = tdr_iostream_write(a_pstIOStream, "%s<!--\'%s\' decides which is used-->\n",
                        szSpace, TDR_GET_STRING_BY_PTR(pstLib,pstSelEntry->ptrName));
                TDR_ERR_CHECK_RETURN(iRet);
                pstStackTop->pszNetBase = ((pstLib->data + pstEntry->stSelector.ptrEntry));
            }

            iRet = tdr_iostream_write(a_pstIOStream, "%s<%s", szSpace, pstStackTop->szMetaEntryName);
            TDR_ERR_CHECK_RETURN(iRet);

            iRet = tdr_template_output_meta_attrs_i(a_pstIOStream, &iChildNodeNum, pstCurMeta,
                    0, pszHostStart, pszHostEnd, pstStackTop->iCutOffVersion);
            TDR_ERR_CHECK_RETURN(iRet);
            pstStackTop->iCode = iChildNodeNum;
            if (0 == pstStackTop->iCode)
            {
                iRet = tdr_iostream_write(a_pstIOStream, "/>\n\n");
            }else
            {
                iRet = tdr_iostream_write(a_pstIOStream, ">\n\n");
            }

            TDR_ERR_CHECK_RETURN(iRet);
            szSpace[(pstStackTop->iMetaSizeInfoOff-1)*TDR_TAB_SIZE] = ' ';

            continue;
        }/* if (TDR_TYPE_COMPOSITE >= pstEntry->iType) */

        // the entry is a simple entry
        szSpace[pstStackTop->iMetaSizeInfoOff*TDR_TAB_SIZE] = '\0';
        iRet = tdr_output_simpleentry_with_default_i(a_pstIOStream, pstLib, pstEntry,
                iArrayRealCount, pszHostStart, pszHostEnd, &szSpace[0]);
        TDR_ERR_CHECK_RETURN(iRet);

        if (TDR_TYPE_UNION != pstCurMeta->iType)
        {
            iRet = tdr_iostream_write(a_pstIOStream, "\n");
            TDR_ERR_CHECK_RETURN(iRet);
        }

        szSpace[pstStackTop->iMetaSizeInfoOff*TDR_TAB_SIZE] = ' ';

        TDR_UNIFORM_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
    }/* while (0 < iStackItemCount) */

    return 0;
}

int tdr_template_output_arr_desc(IN LPTDRIOSTREAM a_pstIOStream,
        IN LPTDRSTACKITEM a_pstStackTop, IN LPTDRMETA a_pstCurMeta,
        IN LPTDRMETAENTRY a_pstEntry, IN char a_szSpace[])
{
    int iArrayRealCount = 0;
    int iRet = TDR_SUCCESS;
    LPTDRMETALIB pstLib = TDR_META_TO_LIB(a_pstCurMeta);

    iRet = tdr_template_get_arr_count(a_pstStackTop, a_pstEntry, &iArrayRealCount);
    TDR_ERR_CHECK_RETURN(iRet);

    if (0 == a_pstEntry->iCount && 0 < iArrayRealCount)
    {
        iRet = tdr_iostream_write(a_pstIOStream, "%s<!--\'%s\' is a variable array-->\n",
                a_szSpace, TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName));
        TDR_ERR_CHECK_RETURN(iRet);
    }else
    {
        iRet = tdr_iostream_write(a_pstIOStream, "%s<!--\'%s\' is an array of %d elements-->\n",
                a_szSpace, TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName), a_pstEntry->iCount);
        TDR_ERR_CHECK_RETURN(iRet);
    }
    if (TDR_INVALID_PTR != a_pstEntry->stRefer.ptrEntry)
    {
        LPTDRMETAENTRY pstRefEntry = TDR_PTR_TO_ENTRY(pstLib, a_pstEntry->stRefer.ptrEntry);
        if (!(TDR_IO_NOOUTPUT & pstRefEntry->iIO))
        {
            iRet = tdr_iostream_write(a_pstIOStream, "%s<!--\'%s\' decides how many elements is used-->\n",
                                      a_szSpace,
                                      TDR_GET_STRING_BY_PTR(pstLib,pstRefEntry->ptrName));
            TDR_ERR_CHECK_RETURN(iRet);
        }
    }

    return iRet;
}

static char *tdr_get_bindmacro_name_i(IN LPTDRMETALIB a_pstLib, IN LPTDRMETAENTRY a_pstEntry, IN int a_iValue)
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
	pchSrc = TDR_GET_STRING_BY_PTR(a_pstLib, pstMacro->ptrMacro);
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

	return TDR_GET_STRING_BY_PTR(a_pstLib, pstMacro->ptrMacro);
}

int tdr_template_output_union_desc(IN LPTDRIOSTREAM a_pstIOStream,
        IN LPTDRSTACKITEM a_pstStackTop, IN LPTDRMETA a_pstCurMeta,
        IN LPTDRMETAENTRY a_pstEntry, IN char a_szSpace[])
{
    int iRet = TDR_SUCCESS;
    LPTDRMETALIB pstLib = TDR_META_TO_LIB(a_pstCurMeta);

    // output corresponding select's value before an entry is output
    if (TDR_ENTRY_DO_HAVE_ID(a_pstEntry))
    {
        char *pszVal;
        LPTDRMETAENTRY pstSelector;
        char *pszHostStart;
        LPTDRMETAENTRY pstTmpEntry = (LPTDRMETAENTRY)a_pstStackTop->pszNetBase;

        iRet = tdr_iostream_write(a_pstIOStream, "\n%s<!--\'%s\' : %d",
                a_szSpace, TDR_GET_STRING_BY_PTR(pstLib,pstTmpEntry->ptrName), a_pstEntry->iID);
        TDR_ERR_CHECK_RETURN(iRet);

        // output macro name
        pstSelector = (LPTDRMETAENTRY)a_pstStackTop->pszNetBase;
        pstLib = TDR_META_TO_LIB(a_pstCurMeta);
        pszHostStart = a_pstStackTop->pszHostBase + pstSelector->iHOff;
        pszVal = tdr_get_bindmacro_name_i(pstLib, pstSelector, a_pstEntry->iID);
        if(NULL != pszVal)
        {
            iRet = tdr_iostream_write(a_pstIOStream, " [%s]", pszVal);
            TDR_ERR_CHECK_RETURN(iRet);
        }

        iRet = tdr_iostream_write(a_pstIOStream, "-->\n");
        TDR_ERR_CHECK_RETURN(iRet);

    }else if(TDR_ENTRY_DO_HAVE_MAXMIN_ID(a_pstEntry))
    {
        LPTDRMETAENTRY pstTmpEntry = (LPTDRMETAENTRY)a_pstStackTop->pszNetBase;
        iRet = tdr_iostream_write(a_pstIOStream, "%s<!--\'%s\' : [%d, %d]-->\n",
                a_szSpace, TDR_GET_STRING_BY_PTR(pstLib, pstTmpEntry->ptrName),
                a_pstEntry->iMinId, a_pstEntry->iMaxId);
    }else
    {
        iRet = tdr_iostream_write(a_pstIOStream, "%s<!--default used-->\n", a_szSpace);
    }
    TDR_ERR_CHECK_RETURN(iRet);

    // output desc attribute as a line of xml comment
    if (TDR_INVALID_PTR != a_pstEntry->ptrDesc)
    {
        iRet = tdr_iostream_write(a_pstIOStream, "%s<!--%s: %s-->\n", a_szSpace,
                TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName), TDR_GET_STRING_BY_PTR(pstLib, a_pstEntry->ptrDesc));
        TDR_ERR_CHECK_RETURN(iRet);
    }


    return iRet;
}

// output a meta's simple (not array or struct or union) children as attributes
int tdr_template_output_meta_attrs_i(IN LPTDRIOSTREAM a_pstIOStream,
        OUT int *a_piChildNum, IN LPTDRMETA a_pstMeta, IN int a_iFirstEntry,
        IN const char *a_pszHostStart, IN const char *a_pszHostEnd, int a_iVersion)
{
    int iRet = TDR_SUCCESS;

    char *pszHostStart = NULL;
    int iArrayRealCount;
    int iChildNum = 0;
    int iChange = 0;

    TDRSTACKITEM stStackItem;
    LPTDRSTACKITEM pstStackTop = &stStackItem;
    LPTDRMETALIB pstLib = TDR_META_TO_LIB(a_pstMeta);
    LPTDRMETAENTRY pstEntry = NULL;

    assert(NULL != a_pstIOStream);
    assert(NULL != a_pstMeta);
    assert(NULL != a_pszHostEnd);
    assert(a_iVersion >= a_pstMeta->iBaseVersion);
    assert((0 <= a_iFirstEntry) && (a_iFirstEntry < a_pstMeta->iEntriesNum));

    pstStackTop->iCount = 1;
    pstStackTop->iChange = 1;
    pstStackTop->idxEntry = a_iFirstEntry;
    pstStackTop->pszHostBase = (char *)a_pszHostStart;

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

        TDR_GET_ARRAY_REAL_COUNT(iArrayRealCount, pstEntry, pstStackTop->pszHostBase, a_iVersion);
        if ((iArrayRealCount < 0) ||
                ((0 < pstEntry->iCount) && (pstEntry->iCount < iArrayRealCount)))
        {
            iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_REFER_VALUE);
            break;
        }
        if (0 == iArrayRealCount)
        {
            TDR_GET_NEXT_ENTRY(pstStackTop, a_pstMeta, iChange);
            continue;
        }

        if (TDR_TYPE_COMPOSITE >= pstEntry->iType)
        {
            TDR_GET_NEXT_ENTRY(pstStackTop, a_pstMeta, iChange);
            iChildNum++;
            continue;
        }/*if (TDR_TYPE_COMPOSITE >= pstEntry->iType)*/

        if (1 != pstEntry->iCount || TDR_TYPE_UNION == a_pstMeta->iType)
        {
            TDR_GET_NEXT_ENTRY(pstStackTop, a_pstMeta, iChange);
            iChildNum++;
            continue;
        }

        iRet = tdr_iostream_write(a_pstIOStream, " %s=\"", TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName));
        TDR_ERR_CHECK_RETURN(iRet);
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
        TDR_ERR_CHECK_RETURN(iRet);

        iRet = tdr_iostream_write(a_pstIOStream, "\"");
        TDR_ERR_CHECK_RETURN(iRet);

        TDR_GET_NEXT_ENTRY(pstStackTop, a_pstMeta, iChange);
    }/*while (!iChange)*/

    *a_piChildNum = iChildNum;

    return iRet;
}

static void tdr_get_entry_defaultvalue_i(OUT char *a_pszDefault, IN LPTDRMETALIB a_pstLib, IN LPTDRMETAENTRY a_pstEntry)
{
	char *pszDefault;

	assert(NULL != a_pszDefault);
	assert(NULL != a_pstLib);
	assert(NULL != a_pstEntry);
	assert(TDR_INVALID_PTR != a_pstEntry->ptrDefaultVal);

	pszDefault = TDR_GET_STRING_BY_PTR(a_pstLib, a_pstEntry->ptrDefaultVal);
	switch(a_pstEntry->iType)
	{
	case TDR_TYPE_STRING:
		sprintf(a_pszDefault, "\'%s\'", pszDefault);
		break;
	case TDR_TYPE_CHAR:
		sprintf(a_pszDefault, "\'%d\'", (int)pszDefault[0]);
		break;
	case TDR_TYPE_UCHAR:
	case TDR_TYPE_BYTE:
		sprintf(a_pszDefault, "\'%d\'", (int)(unsigned char )pszDefault[0]);
		break;
	case TDR_TYPE_SMALLINT:
		sprintf(a_pszDefault, "\'%d\'", (int)*(short *)pszDefault);
		break;
	case TDR_TYPE_SMALLUINT:
		sprintf(a_pszDefault, "\'%d\'", (int)*(unsigned short *)pszDefault);
		break;
	case TDR_TYPE_INT:
	case TDR_TYPE_LONG:
		sprintf(a_pszDefault, "\'%d\'", (int)*(int *)pszDefault);
		break;
	case TDR_TYPE_UINT:
	case TDR_TYPE_ULONG:
		sprintf(a_pszDefault, "\'%u\'", (uint32_t)*(uint32_t *)pszDefault);
		break;
	case TDR_TYPE_LONGLONG:
#if (defined(_WIN32) || defined(_WIN64)) &&  _MSC_VER < 1400  /*vc7,vc6,,*/
		sprintf(a_pszDefault, "\'%I64i\'", (int64_t)*(int64_t *)pszDefault);
#else
		sprintf(a_pszDefault, "\'%"TDRPRId64"\'", (int64_t)*(int64_t *)pszDefault);
#endif
		break;
	case TDR_TYPE_ULONGLONG:
#if (defined(_WIN32) || defined(_WIN64)) &&  _MSC_VER < 1400  /*vc7,vc6,,*/
		sprintf(a_pszDefault, "\'%I64u\'", (uint64_t)*(uint64_t *)pszDefault);
#else
		sprintf(a_pszDefault, "\'%"TDRPRIud64"\'", (uint64_t)*(uint64_t *)pszDefault);
#endif
		break;
	case TDR_TYPE_FLOAT:
		sprintf(a_pszDefault, "\'%f\'", *(float *)pszDefault);
		break;
	case TDR_TYPE_DOUBLE:
		sprintf(a_pszDefault, "\'%f\'", *(double *)pszDefault);
		break;
	case TDR_TYPE_DATETIME:
		{
			char szDateTime[32]={0};
			sprintf(a_pszDefault, "\'%s\'", tdr_tdrdatetime_to_str_r((tdr_datetime_t *)pszDefault,szDateTime,sizeof(szDateTime)));
		}
		break;
	case TDR_TYPE_DATE:
		{
			char szDate[16] = {0};
			sprintf(a_pszDefault, "\'%s\'", tdr_tdrdate_to_str_r((tdr_date_t *)pszDefault, szDate,sizeof(szDate)));
		}
		break;
	case TDR_TYPE_TIME:
		{
			char szTime[16]={0};
			sprintf(a_pszDefault, "\'%s\'", tdr_tdrtime_to_str_r((tdr_time_t *)pszDefault,szTime,sizeof(szTime)));
		}
		break;
	case TDR_TYPE_IP:
		sprintf(a_pszDefault, "\'%u\'", (uint32_t)*(tdr_ip_t *)pszDefault);
		break;
	case TDR_TYPE_WCHAR:
		{
			sprintf(a_pszDefault, "\'%u\'", (uint32_t)*(tdr_wchar_t *)pszDefault);
			break;
		}
	default:
		break;
	}
}

int tdr_output_simpleentry_with_default_i(IN LPTDRIOSTREAM a_pstIOStream, IN LPTDRMETALIB a_pstLib,
        IN LPTDRMETAENTRY a_pstEntry, IN int a_iCount, IN char *a_pszHostStart,
        IN char *a_pszHostEnd, IN char *a_pszSpace)
{
    int iRet = TDR_SUCCESS;

    assert(NULL != a_pstLib);
    assert(NULL != a_pstEntry);
    assert(NULL != a_pszHostStart);
    assert(NULL != a_pszHostEnd);
    assert(NULL != a_pszSpace);

    /* output default value */
    if (TDR_INVALID_PTR != a_pstEntry->ptrDefaultVal)
    {
        char szDefaultval[TDR_DEFAULT_VALUE_LEN] = {0};

        tdr_get_entry_defaultvalue_i(szDefaultval, a_pstLib, a_pstEntry);
        if (szDefaultval[0] != '\0')
        {
            char* pszVal = NULL;

            iRet = tdr_iostream_write(a_pstIOStream, "%s<!--DEFAULT VALUE: %s", a_pszSpace, szDefaultval);
            TDR_ERR_CHECK_RETURN(iRet);

            // output macro name
            switch (a_pstEntry->iType)
            {
                case TDR_TYPE_CHAR:
                    pszVal = tdr_get_bindmacro_name_i(a_pstLib, a_pstEntry, (int)(char)a_pszHostStart[0]);
                    break;
                case TDR_TYPE_UCHAR:
                case TDR_TYPE_BYTE:
                    pszVal = tdr_get_bindmacro_name_i(a_pstLib, a_pstEntry, (int)(unsigned char)a_pszHostStart[0]);
                    break;
                case TDR_TYPE_SMALLINT:
                    pszVal = tdr_get_bindmacro_name_i(a_pstLib, a_pstEntry, (int)*(short*)a_pszHostStart);
                    break;
                case TDR_TYPE_SMALLUINT:
                    pszVal = tdr_get_bindmacro_name_i(a_pstLib, a_pstEntry, (int)*(unsigned short*)a_pszHostStart);
                    break;
                case TDR_TYPE_LONG:
                case TDR_TYPE_INT:
                    pszVal = tdr_get_bindmacro_name_i(a_pstLib, a_pstEntry, (int)*(int*)a_pszHostStart);
                    break;
                case TDR_TYPE_ULONG:
                case TDR_TYPE_UINT:
                    pszVal = tdr_get_bindmacro_name_i(a_pstLib, a_pstEntry, (int)*(uint32_t*)a_pszHostStart);
                    break;
            }
            if(NULL != pszVal)
            {
                iRet = tdr_iostream_write(a_pstIOStream, " [%s]", pszVal);
                TDR_ERR_CHECK_RETURN(iRet);
            }

            iRet = tdr_iostream_write(a_pstIOStream, "-->\n");
            TDR_ERR_CHECK_RETURN(iRet);
        }
    }/*if (TDR_INVALID_PTR != a_pstEntry->ptrDefaultVal)*/

    iRet = tdr_output_simpleentry_i(a_pstIOStream, a_pstLib,
            a_pstEntry, a_iCount, a_pszHostStart, a_pszHostEnd, a_pszSpace);

    return iRet;
}

// set an array's refer value,
// according to how many items to output
int tdr_template_set_arr_count(IN LPTDRSTACKITEM a_pstStackTop,
        IN LPTDRMETAENTRY a_pstEntry, INOUT int * a_iArrayRealCount)
{
    int iRet= TDR_SUCCESS;

    iRet = tdr_template_get_arr_count(a_pstStackTop, a_pstEntry, a_iArrayRealCount);
    TDR_ERR_CHECK_RETURN(iRet);
    if (0 < a_pstEntry->stRefer.iUnitSize)
    {
        char *pszPtr = a_pstStackTop->pszHostBase + a_pstEntry->stRefer.iHOff;
        TDR_SET_INT(pszPtr, a_pstEntry->stRefer.iUnitSize, *a_iArrayRealCount);
    }

    return iRet;
}

// set how many items of an array to output
int tdr_template_get_arr_count(IN LPTDRSTACKITEM a_pstStackTop,
        IN LPTDRMETAENTRY a_pstEntry, INOUT int * a_iArrayRealCount)
{
    int iRet = TDR_SUCCESS;

    if (0 > a_pstEntry->iCount)
    {
        iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_ENTRY_INVALID_COUNT_VALUE);
        return iRet;
    }

    if (0 < a_pstEntry->stRefer.iUnitSize)
    {
        int64_t lVal;
        char *pszPtr = a_pstStackTop->pszHostBase + a_pstEntry->stRefer.iHOff;
        TDR_GET_INT(lVal, a_pstEntry->stRefer.iUnitSize, pszPtr);
        *a_iArrayRealCount = (int)lVal;
        if ((0 < a_pstEntry->iCount) && (*a_iArrayRealCount > a_pstEntry->iCount))
        {
            iRet = TDR_ERROR_INVALID_REFER_VALUE;
            return iRet;
        }
    }else
    {
       *a_iArrayRealCount = a_pstEntry->iCount;
    }

    // for simple mode
    //    if (1 == a_pstEntry->iCount)
    //    {
    //        *a_iArrayRealCount = a_pstEntry->iCount;
    //    }else
    //    {
    //        *a_iArrayRealCount = 0;
    //        if (0 < a_pstEntry->stRefer.iUnitSize)
    //        {
    //            int64_t lVal;
    //            char *pszPtr = a_pstStackTop->pszHostBase + a_pstEntry->stRefer.iHOff;
    //            TDR_GET_INT(lVal, a_pstEntry->stRefer.iUnitSize, pszPtr);
    //            *a_iArrayRealCount = (int)lVal;
    //        }
    //        if (0 >= *a_iArrayRealCount
    //                || (a_pstEntry->iCount > 0 && *a_iArrayRealCount > a_pstEntry->iCount))
    //        {
    //            *a_iArrayRealCount = TDR_DEFAULT_ARRAY_DIMENSION;
    //        }
    //    }

    return iRet;
}

int tdr_init_union_entry(INOUT LPTDRSTACKITEM a_pstStackTop, IN LPTDRMETAENTRY a_pstEntry,
        IN LPTDRMETALIB a_pstLib, IN const char * pszHostEnd, IN int iArrayRealCount)
{
    int iRet = TDR_SUCCESS;

    TDRDATA stData;
    LPTDRMETA pstUnionEntryMeta = NULL;
    char *pszSavedStackBase = a_pstStackTop->pszHostBase;

    if (TDR_TYPE_COMPOSITE < a_pstEntry->iType)
    {
        TDR_SET_DEFAULT_VALUE(iRet, a_pstStackTop->pszHostBase, pszHostEnd, a_pstLib, a_pstEntry, iArrayRealCount);
        TDR_ERR_CHECK_RETURN(iRet);
    }else
    {
        int i;
        pstUnionEntryMeta = TDR_PTR_TO_META(a_pstLib, a_pstEntry->ptrMeta);
        for (i = 0; i < iArrayRealCount; i++)
        {
            stData.pszBuff = a_pstStackTop->pszHostBase + i * a_pstEntry->iHUnitSize;
            stData.iBuff = a_pstEntry->iHUnitSize;
            iRet = tdr_template_init_data(pstUnionEntryMeta, &stData, 0);
            TDR_ERR_CHECK_RETURN(iRet);
        }
    }

    a_pstStackTop->pszHostBase = pszSavedStackBase;

    return iRet;
}

// output an entry's desc attribute as a line of xml comment
int tdr_template_output_child_desc(IN LPTDRIOSTREAM a_pstIOStream,
        IN const char* a_pszParent, IN LPTDRMETA a_pstMeta, IN const char* a_szSpace)
{
    int iRet = TDR_SUCCESS;

    LPTDRMETALIB pstLib = TDR_META_TO_LIB(a_pstMeta);
    LPTDRMETAENTRY pstEntry = NULL;

    int i;
    for (i = 0; i < a_pstMeta->iEntriesNum; i++)
    {
        pstEntry = a_pstMeta->stEntries + i;
        if ((TDR_INVALID_PTR != pstEntry->ptrDesc)
                && (TDR_TYPE_COMPOSITE < pstEntry->iType)
                && (1 == pstEntry->iCount))
        {
            iRet = tdr_iostream_write(a_pstIOStream, "%s<!--%s.%s: %s-->\n", a_szSpace,
                    a_pszParent, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName), TDR_GET_STRING_BY_PTR(pstLib, pstEntry->ptrDesc));
            TDR_ERR_CHECK_RETURN(iRet);
        }
    }

    return iRet;
}
