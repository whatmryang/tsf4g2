//============================================================================
// @Id:       $Id: tsm_tdr_i.c 15076 2010-11-23 02:56:24Z flyma $
// @Author:   $Author: flyma $
// @Date:     $Date:: 2010-11-23 10:56:24 +0800 #$
// @Revision: $Revision: 15076 $
// @HeadURL:  $HeadURL:: https://tc-svn.tencent.com/ied/ied_tsf4g_rep/tsf4g_#$
//----------------------------------------------------------------------------


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "pal/tos.h"
#include "pal/tstdio.h"
#include "tdr/tdr_data_io.h"
#include "tdr/tdr_metalib_manage.h"

#include "tsm_tdr_i.h"
#include "tsm_hash.h"
#include "tsm_head.h"


//----------------------------------------------------------------------------
// 0
//----------------------------------------------------------------------------
static int tsm_tdr_status(TsmTdrHandle *a_pHandle);
static int tsm_tdr_status(TsmTdrHandle *a_pHandle)
{
    tassert(a_pHandle, NULL, 1, "²ÎÊý´íÎó: ¾ä±úÎª¿Õ");

    return TSM_TDR_C_INDEX_KEY == a_pHandle->iMagic ? 0 : 1;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int tsm_tdr_open_i(TsmTdrHandle **a_pHandle)
{
    int iRet;

    tassert(a_pHandle, NULL, 1, "²ÎÊý´íÎó: ¾ä±úÎª¿Õ");

    if (*a_pHandle)
    {
        return 1;
    }

    *a_pHandle = malloc(sizeof(TsmTdrHandle));
    tassert(*a_pHandle, NULL, 2, "¾ä±úÄÚ´æ·ÖÅä");
    memset(*a_pHandle, 0, sizeof(TsmTdrHandle));

    if ((iRet = tsm_tdr_init_i(*a_pHandle)))
    {
        tsm_tdr_destroy_i(*a_pHandle);
        free(*a_pHandle);
        *a_pHandle = NULL;
    }

    return iRet;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int tsm_tdr_init_i(TsmTdrHandle *a_pHandle)
{
    tassert(a_pHandle, NULL, 1, "²ÎÊý´íÎó: ¾ä±úÎª¿Õ");

    if (TSM_TDR_C_INDEX_KEY == a_pHandle->iMagic)
    {
        return 1;
    }
    memset(a_pHandle, 0, sizeof(TsmTdrHandle));

    a_pHandle->pstIndex = tsm_index_mount();
    tassert(a_pHandle->pstIndex, NULL, 2, "Index »ñµÃÊ§°Ü");

    a_pHandle->iMagic = TSM_TDR_C_INDEX_KEY;

    return 0;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int tsm_tdr_close_i(TsmTdrHandle **a_pHandle)
{
    int iRet;

    tassert( a_pHandle, NULL, 1, "²ÎÊý´íÎó: ¾ä±úÖ¸ÕëÎª¿Õ");
    tassert(*a_pHandle, NULL, 2, "²ÎÊý´íÎó: ¾ä±úÎª¿Õ");

    iRet = tsm_tdr_destroy_i(*a_pHandle);
    free(*a_pHandle);
    *a_pHandle = NULL;

    return iRet;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int tsm_tdr_destroy_i(TsmTdrHandle *a_pHandle)
{
    tassert(a_pHandle, NULL, 1, "²ÎÊý´íÎó: ¾ä±úÎª¿Õ");

    if (TSM_TDR_C_INDEX_KEY != a_pHandle->iMagic)
    {
        return 1;
    }

    if (a_pHandle->pstIndex)
    {
        tsm_index_unmount(a_pHandle->pstIndex);
    }

    if (a_pHandle->pstResult)
    {
        tsm_tdr_free_result_i(a_pHandle->pstResult);
    }

    memset(a_pHandle, 0, sizeof(TsmTdrHandle));

    return 0;
}

//----------------------------------------------------------------------------
// ¼ÓËø
//----------------------------------------------------------------------------
int tsm_tdr_lock(TsmTdrHandle *a_pHandle, int a_iReadOnly)
{
    int iRet = -1;
    int i = 0;

#if !defined (_WIN32) && !defined (_WIN64)
    if (a_iReadOnly)
    {
        for (i = 0; iRet && i < 5; ++i)
        {
            iRet = pthread_rwlock_tryrdlock(&a_pHandle->pstIndex->iLock);
            if (iRet)
            {
                usleep(100000);
            }
        }
    }
    else
    {
        for (i = 0; iRet && i < 5; ++i)
        {
            iRet = pthread_rwlock_trywrlock(&a_pHandle->pstIndex->iLock);
            if (iRet)
            {
                usleep(100000);
            }
        }
    }
    iRet = 0;
#else
    iRet = i;
#endif

    return iRet;
}

//----------------------------------------------------------------------------
// ½âËø
//----------------------------------------------------------------------------
int tsm_tdr_unlock(TsmTdrHandle *a_pHandle)
{
#if !defined (_WIN32) && !defined (_WIN64)
    return pthread_rwlock_unlock(&a_pHandle->pstIndex->iLock);
#else
    return 0;
#endif
}

//----------------------------------------------------------------------------
// 1
//----------------------------------------------------------------------------
TErrorSet *tsm_tdr_error_i(TsmTdrHandle *a_pHandle)
{
    return NULL;
}

//----------------------------------------------------------------------------
// 2
//----------------------------------------------------------------------------
int tsm_tdr_bind_data_i(TsmTdrHandle *a_pHandle,
                        const char   *a_pszUri)
{
    TsmUri      stUri;
    TsmObjData *pObj;
    int         i;

    tassert(a_pHandle,                      NULL, 1, "²ÎÊý´íÎó: ¾ä±úÎª¿Õ");
    tassert(!tsm_tdr_status(a_pHandle),     NULL, 2, "¾ä±ú×´Ì¬: ¹Ø±Õ");
    tassert(!tsm_uri_set(&stUri, a_pszUri), NULL, 3, "Uri ×Ö·û´®¸ñÊ½´íÎó");
    tassert(TSM_URI_DATA == stUri.iType,    NULL, 4, "Uri ÀàÐÍ´íÎó");

    tsm_uri_std_i(&stUri, NULL);

    i = tsm_hash_find(a_pHandle->pstIndex, tsm_uri_get(&stUri), 0);
    if (i < 0)
    {
        tassert1(-1 == i, NULL, 4, "²éÕÒ¹ý³ÌÖÐ³ö´í(%d)", i);

        memset(&a_pHandle->stMeta, 0,      sizeof(TsmUri));
        memcpy(&a_pHandle->stData, &stUri, sizeof(TsmUri));

        return 0;
    }

    pObj = tsm_head_data(a_pHandle->pstIndex, i, 0);
    tassert(pObj, NULL, 5, "tsm_head_data ·µ»Ø¿Õ");

    tsm_uri_set_data(&a_pHandle->stData, &pObj->stData);
    tsm_uri_set_meta(&a_pHandle->stMeta, &pObj->stMeta);

    return 0;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int tsm_tdr_bind_meta_i(TsmTdrHandle *a_pHandle,
                        const char   *a_pszUri)
{
    LPTDRMETALIB    pstLib;
    LPTDRMETA       pstMeta;
    const char     *pszMeta;
    TsmObjMeta     *pObj;
    TsmTdrDataInfo *pstData;
    TsmUri          stUri;
    TsmUri          stTmp;
    size_t          iSize;
    int             i;

    tassert(a_pHandle,                            NULL, 1, "²ÎÊý´íÎó: ¿Õ¾ä±ú");
    tassert(!tsm_tdr_status(a_pHandle),           NULL, 2, "¾ä±ú×´Ì¬: ¹Ø±Õ");
    tassert(!tsm_uri_is_null(&a_pHandle->stData), NULL, 3, "ÐèÒªÏÈ bind data");
    tassert(!tsm_uri_set(&stUri, a_pszUri),       NULL, 4, "Uri ¸ñÊ½´íÎó");
    tassert(TSM_URI_META == stUri.iType,          NULL, 5, "Uri ÀàÐÍ´íÎó");

    if (!tsm_uri_is_null(&a_pHandle->stMeta))
    {
        tassert(0 == tsm_uri_compare(&stUri, &a_pHandle->stMeta),
                NULL, 6, "¾ä±úÖÐ Meta ºÍ´ýÉèÖÃµÄ Meta ²»Æ¥Åä");
        return 0;
    }

    tsm_uri_std_i(&stTmp, &stUri);
    i = tsm_hash_find(a_pHandle->pstIndex, tsm_uri_get(&stTmp), 0);
    tassert1(i >= 0, NULL, 7, "´ý°ó¶¨µÄ meta ÏµÍ³ÖÐ²»´æÔÚ (%d)", i);

    pObj = tsm_head_meta(a_pHandle->pstIndex, i, 0);
    tassert(pObj, NULL, 8, "tsm_head_meta ·µ»Ø¿Õ");

    pstLib = (LPTDRMETALIB)tsm_head_at(&pObj->stPtr, 1);
    tassert(pstLib && (LPTDRMETALIB)-1 != pstLib,
            NULL, 9, "tsm_head_at ³ö´í");

    pszMeta = (const char *)TSM_URI_GET(&stUri, m, szMeta);
    pstMeta = tdr_get_meta_by_name(pstLib, pszMeta);
    iSize = NULL == pstMeta ? 0 : tdr_get_meta_size(pstMeta);

    tsm_head_dt(pstLib);

    tassert2(iSize,
             NULL, 10, "tdr_get_meta_by_name (%s:%s)",
             tdr_get_metalib_name(pstLib),
             pszMeta);

    pstData = &a_pHandle->stData.s.d;
    tassert(0 == pstData->iSize || iSize == pstData->iSize,
            NULL, 11, "¾ä±úÖÐ³ß´çÓë´ý°ó¶¨µÄ Meta ²»Æ¥Åä");

    memcpy(&a_pHandle->stMeta, &stUri, sizeof(TsmUri));
    pstData->iSize = iSize;

    return 0;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int tsm_tdr_bind_size_i(TsmTdrHandle *a_pHandle,
                        size_t        a_iSize)
{
    tassert(a_pHandle,                  NULL, 1, "²ÎÊý´íÎó: ¾ä±úÎª¿Õ");
    tassert(!tsm_tdr_status(a_pHandle), NULL, 2, "¾ä±ú×´Ì¬: ¹Ø±Õ");
    tassert(a_iSize,                    NULL, 3, "²ÎÊý´íÎó: a_iSize Îª 0");
    tassert(!tsm_uri_is_null(&a_pHandle->stData),
            NULL, 4, "¾ä±ú×´Ì¬: ÐèÒªÏÈ bind data");

    if (a_iSize == a_pHandle->stData.s.d.iSize)
    {
        return 0;
    }

    tassert(tsm_uri_is_null(&a_pHandle->stMeta),
            NULL, 5, "meta °ó¶¨Óë size ³åÍ»");
    tassert(0 == a_pHandle->stData.s.d.iSize,
            NULL, 6, "¾ä±úÖÐ Size ºÍ´ýÉèÖÃµÄ Size ²»Æ¥Åä");

    a_pHandle->stData.s.d.iSize = a_iSize;

    return 0;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
const char *tsm_tdr_bind_get_data_i(TsmTdrHandle *a_pHandle)
{
    tassert_r(NULL, a_pHandle,                  NULL, 1, "¿Õ¾ä±ú");
    tassert_r(NULL, !tsm_tdr_status(a_pHandle), NULL, 2, "¾ä±ú×´Ì¬: ¹Ø±Õ");

    return tsm_uri_get(&a_pHandle->stData);
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
const char *tsm_tdr_bind_get_meta_i(TsmTdrHandle *a_pHandle)
{
    tassert_r(NULL, a_pHandle,                  NULL, 1, "¿Õ¾ä±ú");
    tassert_r(NULL, !tsm_tdr_status(a_pHandle), NULL, 2, "¾ä±ú×´Ì¬: ¹Ø±Õ");

    return tsm_uri_get(&a_pHandle->stMeta);
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
size_t tsm_tdr_bind_get_size_i(TsmTdrHandle *a_pHandle)
{
    tassert_r(0, a_pHandle,                  NULL, 1, "¿Õ¾ä±ú");
    tassert_r(0, !tsm_tdr_status(a_pHandle), NULL, 2, "¾ä±ú×´Ì¬: ¹Ø±Õ");

    return a_pHandle->stData.s.d.iSize;
}

//----------------------------------------------------------------------------
// 3
//----------------------------------------------------------------------------
int tsm_tdr_load_data_i(TsmTdrHandle *a_pHandle,
                        void         *a_pData,
                        int           a_iFlag)
{
    TsmObjPtr *pPtr;
    void      *pAt;
    size_t     iSize = tsm_tdr_bind_get_size_i(a_pHandle);

    tassert(iSize, NULL, 1, "È¡µÃ size Ê§°Ü");

    if ((TsmObjPtr *)-1 == (pPtr = tsm_tdr_ptr_i(a_pHandle, a_iFlag)))
    {
        return 1;
    }

    pAt = tsm_tdr_at_i(pPtr, 0);
    tassert(pAt && (void *)-1 != pAt, NULL, 2, "tsm_tdr_at Ê§°Ü");

    if (NULL == a_pData)
    {
        memset(pAt, 0, iSize);
    }
    else
    {
        memcpy(pAt, a_pData, iSize);
    }
    tassert(!tsm_tdr_dt_i(pAt), NULL, 3, "tsm_tdr_dt Ê§°Ü");

    return 0;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int tsm_tdr_load_meta_i(TsmTdrHandle *a_pHandle,
                        LPTDRMETALIB  a_pstLib,
                        int           a_iBid)
{
    TsmObjMeta *p;
    TsmUri      stUri;
    char        szUri[URI_MAX_LEN];
    size_t      iSize;
    int         iRow;
    int         iRet;

    tassert(a_pHandle,                  NULL, 1, "¾ä±úÎª¿Õ");
    tassert(!tsm_tdr_status(a_pHandle), NULL, 2, "¾ä±ú×´Ì¬: ¹Ø±Õ");
    tassert(a_pstLib,                   NULL, 3, "Lib Îª¿Õ");

    snprintf(szUri,
             URI_MAX_LEN - 1,
             "tsm-meta://%d/%s:%d:%d/",
             a_iBid,
             tdr_get_metalib_name(a_pstLib),
             tdr_get_metalib_version(a_pstLib),
             tdr_get_metalib_build_version(a_pstLib));
    tassert(0 == tsm_uri_set(&stUri, szUri), NULL, 4, "Uri ¸ñÊ½´íÎó");
    tassert(TSM_URI_META == stUri.iType,     NULL, 5, "Uri ÀàÐÍ´íÎó");

    iRet = tsm_tdr_query_i(a_pHandle, tsm_uri_get(&stUri));
    tassert1(iRet >= 0, NULL, 6, "Uri ²éÑ¯³ö´í(%d)", iRet);

    if (1 == iRet)
    {
        return 0;
    }
    tassert1(0 == iRet, NULL, 7, "ÕÒµ½¶àÌõÆ¥Åä¼ÇÂ¼ (%d)", iRet);

    iRow = tsm_head_obj_alloc(a_pHandle->pstIndex, TSM_URI_META);
    tassert(iRow >= 0, NULL, 8, "Meta Obj ·ÖÅäÊ§°Ü");

    p = toffset_array_ptr(a_pHandle->pstIndex, MOFFSET(stLib, TsmIndex), iRow);
    tassert(p, NULL, 9, "Ë÷ÒýÔ½½ç");

    iSize = tdr_size(a_pstLib);
    stUri.s.m.iSize = iSize;
    if ((iRet = tsm_head_init(&p->stPtr, TSM_URI_META, a_pstLib, iSize)))
    {
        tsm_head_obj_free(a_pHandle->pstIndex, TSM_URI_META, iRow);
        tassert(0, NULL, 10, "ÉèÖÃ¹²ÏíÄÚ´æÖ¸ÕëÊ§°Ü");
    }

    if ((iRet = tsm_hash_set(a_pHandle->pstIndex, NULL, &stUri, iRow)))
    {
        tsm_head_destroy(&p->stPtr);
        tsm_head_obj_free(a_pHandle->pstIndex, TSM_URI_META, iRow);
        tassert(0, NULL, 11, "ÉèÖÃ hash ½áµãÊ§°Ü");
    }

    memcpy(&p->stMeta, &stUri.s.m, sizeof(TsmTdrMetaInfo));

    return 0;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int tsm_tdr_load_meta_file_i(TsmTdrHandle *a_pHandle,
                             const char   *a_pszPath,
                             int           a_iBid)
{
    LPTDRMETALIB pstLib = NULL;
    int          iRet;

    tassert(a_pHandle,                  NULL, 1, "²ÎÊý´íÎó: ¾ä±úÎª¿Õ");
    tassert(!tsm_tdr_status(a_pHandle), NULL, 2, "¾ä±ú×´Ì¬: ¹Ø±Õ");

    iRet = tdr_load_metalib(&pstLib, a_pszPath);
    tassert1(iRet >= 0, NULL, 3, "×°Èë metalib Ê§°Ü(%d)", iRet);

    iRet = tsm_tdr_load_meta_i(a_pHandle, pstLib, a_iBid);
    tdr_free_lib(&pstLib);

    return iRet ? terror_push1(NULL, 4, "Lib ¸ñÊ½´íÎó(%d)", iRet) : 0;
}

//----------------------------------------------------------------------------
// 4
//----------------------------------------------------------------------------
TsmObjPtr *tsm_tdr_ptr_i(TsmTdrHandle *a_pHandle, int a_iFlag)
{
    TsmObjData *p;
    int         iRow;
    int         i;

    tassert_r(NULL, tsm_tdr_bind_get_size_i(a_pHandle), NULL, 1, "sizeÎªÁã");

    i = tsm_hash_find(a_pHandle->pstIndex, tsm_uri_get(&a_pHandle->stData), 0);
    if (i >= 0)
    {
        if ((O_CREAT & a_iFlag) && (O_EXCL & a_iFlag))
        {
            return (void *)-1;
        }

        p = tsm_head_data(a_pHandle->pstIndex, i, 0);
        tassert_r(NULL, p, NULL, 2, "tsm_head_data ·µ»Ø¿Õ");
        tassert_r(NULL, p->stPtr.iSize == p->stData.iSize,
                  NULL, 3, "Ö¸ÕëÓë½á¹¹µÄ³ß´ç²»Æ¥Åä");

        return &p->stPtr;
    }
    tassert_r1(NULL, -1 == i, NULL, 4, "²éÕÒ¹ý³ÌÖÐ³ö´í(%d)", i);

    if (!(a_iFlag & O_CREAT))
    {
        return (void *)-1;
    }

    iRow = tsm_head_obj_alloc(a_pHandle->pstIndex, TSM_URI_DATA);
    tassert_r(NULL, iRow >= 0, NULL, 5, "Uri Êý×é·ÖÅäÊ§°Ü");

    p = toffset_array_ptr(a_pHandle->pstIndex, MOFFSET(stUri, TsmIndex), iRow);
    tassert_r1(NULL, p, NULL, 6, "ÐÐÔ½½ç row: %d", iRow);
    tassert_r(NULL, a_pHandle->stData.s.d.iSize, NULL, 7, "sizeÎªÁã");

    memcpy(&p->stMeta, &a_pHandle->stMeta.s.m, sizeof(TsmTdrMetaInfo));
    memcpy(&p->stData, &a_pHandle->stData.s.d, sizeof(TsmTdrDataInfo));

    i = tsm_hash_set(a_pHandle->pstIndex,
                     &a_pHandle->stData,
                     tsm_uri_is_null(&a_pHandle->stMeta)
                         ? NULL
                         : &a_pHandle->stMeta,
                     iRow);
    tassert_r1(NULL, 0 == i, NULL, 8, "tsm_hash_setÊ§°Ü(%d)", i);

    i = tsm_head_init(&p->stPtr, TSM_URI_DATA, NULL, p->stData.iSize);
    tassert_r1(NULL, 0 == i, NULL, 9, "tsm_head_initÊ§°Ü(%d)", i);

    return &p->stPtr;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
void *tsm_tdr_at_i(TsmObjPtr *a_pPtr, int a_iReadOnly)
{
    return tsm_head_at(a_pPtr, a_iReadOnly);
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int tsm_tdr_dt_i(void *a_pAt)
{
    return tsm_head_dt(a_pAt);
}

//----------------------------------------------------------------------------
// 5
//----------------------------------------------------------------------------
int tsm_tdr_query_i(TsmTdrHandle *a_pHandle,
                    const char   *a_pszUri)
{
    TsmIndex   *pIndex = a_pHandle->pstIndex;
    TsmObjMeta *m;
    TsmTdrRow  *pRow;
    TsmUri      stUri;
    char       *pData;
    int         pArray[1000];
    int         iRet;
    int         i;

    tassert(a_pHandle,                          NULL, 1, "²ÎÊý´íÎó: ¿Õ¾ä±ú");
    tassert(0 == tsm_uri_set(&stUri, a_pszUri), NULL, 2, "Uri ¸ñÊ½´íÎó");

    if (a_pHandle->pstResult)
    {
        tsm_tdr_free_result_i(a_pHandle->pstResult);
        a_pHandle->pstResult = NULL;
    }

    if (0 == (iRet = tsm_hash_get(a_pHandle->pstIndex, &stUri, pArray, 1000)))
    {
        return 0;
    }
    tassert1(iRet > 0, NULL, 3, "²éÑ¯³ö´í: (%d)", iRet);

    pData = malloc(sizeof(TsmTdrResult) + iRet * sizeof(TsmTdrRow));
    tassert(pData, NULL, 4, "ÄÚ´æ·ÖÅäÊ§°Ü");

    a_pHandle->pstResult = (TsmTdrResult *)pData;
    pData += sizeof(TsmTdrResult);

    a_pHandle->pstResult->iCount     = iRet;
    a_pHandle->pstResult->pstHead    = (TsmTdrRow *)pData;
    a_pHandle->pstResult->pstCurrent = (TsmTdrRow *)pData;

    if (TSM_URI_META == stUri.iType)
    {
        for (i = 0; i < iRet; ++i)
        {
            m = toffset_array_ptr(pIndex, MOFFSET(stLib, TsmIndex), pArray[i]);

            pRow = a_pHandle->pstResult->pstHead + i;
            memset(pRow, 0, sizeof(TsmTdrRow));

            pRow->iType  = stUri.iType;
            pRow->iIndex = i;
            pRow->iObj   = PTR2OFF(m, pIndex);

            tsm_uri_set_meta(&pRow->stMeta, &m->stMeta);

            pRow->iSize  = pRow->stMeta.s.m.iSize;

            memcpy(&pRow->stDataPtr, &m->stPtr, sizeof(TsmObjPtr));
        }
    }
    else if (TSM_URI_DATA == stUri.iType)
    {
        TsmObjData *d;

        for (i = 0; i < iRet; ++i)
        {
            d = toffset_array_ptr(pIndex, MOFFSET(stUri, TsmIndex), pArray[i]);

            pRow = a_pHandle->pstResult->pstHead + i;
            memset(pRow, 0, sizeof(TsmTdrRow));

            pRow->iType  = stUri.iType;
            pRow->iIndex = i;
            pRow->iObj   = PTR2OFF(d, pIndex);

            tsm_uri_set_meta(&pRow->stMeta, &d->stMeta);
            tsm_uri_set_data(&pRow->stData, &d->stData);

            pRow->iSize  = pRow->stData.s.d.iSize;
            memcpy(&pRow->stDataPtr, &d->stPtr, sizeof(TsmObjPtr));

            if (!tsm_tdr_meta_is_null(&d->stMeta))
            {
                TsmUri u;
                int    j;

                tsm_uri_std_i(&u, &pRow->stMeta);

                if ((j = tsm_hash_find(pIndex, tsm_uri_get(&u), 0)) < 0)
                {
                    continue;
                }

                if ((m = tsm_head_meta(pIndex, j, 0)))
                {
                    memcpy(&pRow->stMetaPtr, &m->stPtr, sizeof(TsmObjPtr));
                }
            }
        }
    }

    return iRet;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
TsmTdrResult *tsm_tdr_store_result_i(TsmTdrHandle *a_pHandle)
{
    TsmTdrResult *pRet;

    tassert_r(NULL, a_pHandle, NULL, 1, "²ÎÊý´íÎó: ¿Õ¾ä±ú");

    pRet = a_pHandle->pstResult;
    a_pHandle->pstResult = NULL;

    return pRet;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int tsm_tdr_num_rows_i(TsmTdrResult *a_pResult)
{
    tassert(a_pResult, NULL, 1, "²ÎÊý´íÎó: ¿Õ¾ä±ú");

    return a_pResult->iCount;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int tsm_tdr_data_seek_i(TsmTdrResult *a_pResult, int a_iIndex)
{
    tassert(a_pResult, NULL, 1, "²ÎÊý´íÎó: ¿Õ¾ä±ú");
    tassert(a_iIndex >= 0 && a_iIndex < a_pResult->iCount,
            NULL, 2, "²ÎÊý´íÎó: Ë÷ÒýÔ½½ç");

    a_pResult->pstCurrent = a_pResult->pstHead + a_iIndex;
    return 0;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
TsmTdrRow *tsm_tdr_fetch_row_i(TsmTdrResult *a_pResult)
{
    tassert_r(NULL, a_pResult, NULL, 1, "²ÎÊý´íÎó: ¿Õ¾ä±ú");

    if (a_pResult->pstCurrent < a_pResult->pstHead + a_pResult->iCount)
    {
        return a_pResult->pstCurrent++;
    }

    a_pResult->pstCurrent = a_pResult->pstHead;
    return NULL;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int tsm_tdr_free_result_i(TsmTdrResult *a_pResult)
{
    if (a_pResult)
    {
        TsmTdrRow *pRow;

        a_pResult->pstCurrent = a_pResult->pstHead;
        while ((pRow = tsm_tdr_fetch_row_i(a_pResult)))
        {
            if (pRow->pstMeta)
            {
                tsm_head_dt(pRow->pstMeta);
                pRow->pstMeta = NULL;
            }
            if (pRow->pstData)
            {
                tsm_head_dt(pRow->pstData);
                pRow->pstData = NULL;
            }
        }

        free(a_pResult);
    }

    return 0;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int tsm_tdr_row_unlink_i(TsmTdrRow *a_pRow, TsmTdrHandle *a_pHandle)
{
    tassert(a_pHandle,                  NULL, 1, "¾ä±úÎª¿Õ");
    tassert(!tsm_tdr_status(a_pHandle), NULL, 2, "¾ä±ú×´Ì¬: ¹Ø±Õ");
    tassert(a_pRow,                     NULL, 3, "²ÎÊý´íÎó: ¿Õ¾ä±ú");

    if (tsm_tdr_row_is_data_i(a_pRow))
    {
        int i = toffset_array_index(a_pHandle->pstIndex,
                                    MOFFSET(stUri, TsmIndex),
                                    a_pRow->iObj);
        TsmObjData *pObj = OFF2PTR(a_pRow->iObj, a_pHandle->pstIndex);

        tassert(0 == tsm_hash_del(a_pHandle->pstIndex, TSM_URI_DATA, i),
                NULL, 3, "É¾³ý hash ½áµãÊ§°Ü");
        tassert(0 == tsm_head_destroy(&pObj->stPtr),
                NULL, 4, "É¾³ý¹²ÏíÄÚ´æÊ§°Ü");

        tsm_head_obj_free(a_pHandle->pstIndex, TSM_URI_DATA, i);
        return 0;
    }

    if (tsm_tdr_row_is_meta_i(a_pRow))
    {
        int i = toffset_array_index(a_pHandle->pstIndex,
                                    MOFFSET(stLib, TsmIndex),
                                    a_pRow->iObj);
        TsmObjMeta *pObj = OFF2PTR(a_pRow->iObj, a_pHandle->pstIndex);

        tassert(0 == tsm_hash_del(a_pHandle->pstIndex, TSM_URI_META, i),
                NULL, 5, "É¾³ý hash ½áµãÊ§°Ü");
        tassert(0 == tsm_head_destroy(&pObj->stPtr),
                NULL, 6, "É¾³ý¹²ÏíÄÚ´æÊ§°Ü");

        tsm_head_obj_free(a_pHandle->pstIndex, TSM_URI_META, i);
        return 0;
    }

    return terror_push(NULL, 1, "´íÎóµÄÀàÐÍ");
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int tsm_tdr_row_type_i(TsmTdrRow *a_pRow)
{
    tassert(a_pRow, NULL, 1, "²ÎÊý´íÎó: ¿Õ¾ä±ú");

    return a_pRow->iType;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int tsm_tdr_row_is_data_i(TsmTdrRow *a_pRow)
{
    return TSM_URI_DATA == tsm_tdr_row_type_i(a_pRow);
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int tsm_tdr_row_is_meta_i(TsmTdrRow *a_pRow)
{
    return TSM_URI_META == tsm_tdr_row_type_i(a_pRow);
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
const char *tsm_tdr_row_uri_i(TsmTdrRow *a_pRow)
{
    if (tsm_tdr_row_is_meta_i(a_pRow))
    {
        return tsm_uri_get(&a_pRow->stMeta);
    }

    if (tsm_tdr_row_is_data_i(a_pRow))
    {
        return tsm_uri_get(&a_pRow->stData);
    }

    tassert_r(NULL, 0, NULL, 2, "¼ÇÂ¼ÀàÐÍ´íÎó");
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
const char *tsm_tdr_row_meta_uri_i(TsmTdrRow *a_pRow)
{
    if (tsm_tdr_row_is_data_i(a_pRow))
    {
        return tsm_tdr_meta_is_null(&a_pRow->stMeta.s.m)
           ? NULL
           : tsm_uri_get(&a_pRow->stMeta);
    }

    return NULL;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
TsmObjPtr *tsm_tdr_row_meta_ptr_i(TsmTdrRow *a_pRow)
{
    tassert_r(NULL, a_pRow, NULL, 1, "²ÎÊý´íÎó: ¿Õ¾ä±ú");

    return 0 == a_pRow->stMetaPtr.iSize ? NULL : &a_pRow->stMetaPtr;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
TsmObjPtr *tsm_tdr_row_data_ptr_i(TsmTdrRow *a_pRow)
{
    tassert_r(NULL, a_pRow, NULL, 1, "²ÎÊý´íÎó: ¿Õ¾ä±ú");

    return 0 == a_pRow->stDataPtr.iSize ? NULL : &a_pRow->stDataPtr;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
LPTDRMETA tsm_tdr_row_meta_i(TsmTdrRow *a_pRow)
{
    tassert_r(NULL, a_pRow, NULL, 1, "²ÎÊý´íÎó: ¿Õ¾ä±ú");

    if (NULL == a_pRow->pstMeta)
    {
        a_pRow->pstMeta = tsm_head_at(&a_pRow->stMetaPtr, 1);
    }

    if (NULL == a_pRow->pstMeta)
    {
        return NULL;
    }

    return tdr_get_meta_by_name(a_pRow->pstMeta,
                                TSM_URI_GET(&a_pRow->stMeta, m, szMeta));
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
void *tsm_tdr_row_data_i(TsmTdrRow *a_pRow, int a_iReadOnly)
{
    tassert_r(NULL, a_pRow, NULL, 1, "²ÎÊý´íÎó: ¿Õ¾ä±ú");

    if (NULL == a_pRow->pstData)
    {
        a_pRow->pstData = tsm_head_at(&a_pRow->stDataPtr, a_iReadOnly);
    }

    return a_pRow->pstData;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
size_t tsm_tdr_row_size_i(TsmTdrRow *a_pRow)
{
    tassert_r(0, a_pRow, NULL, 1, "²ÎÊý´íÎó: ¿Õ¾ä±ú");

    if (tsm_tdr_row_is_data_i(a_pRow))
    {
        return a_pRow->stDataPtr.iSize;
    }

    if (tsm_tdr_row_is_meta_i(a_pRow))
    {
        return a_pRow->stDataPtr.iSize;
    }

    tassert_r(0, 0, NULL, 2, "´íÎóµÄÐÐÀàÐÍ");
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
TsmTdrRow *tsm_tdr_row_only_i(TsmTdrHandle *a_pHandle)
{
    tassert_r(NULL, a_pHandle, NULL, 1, "²ÎÊý´íÎó: ¿Õ¾ä±ú");

    if (NULL == a_pHandle->pstResult || 0 == a_pHandle->pstResult->iCount)
    {
        return NULL;
    }

    tassert_r1(NULL, 1 == a_pHandle->pstResult->iCount,
               NULL, 2, "count(%d)", a_pHandle->pstResult->iCount);

    return a_pHandle->pstResult->pstHead;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
TsmTdrRow *tsm_tdr_row_first_i(TsmTdrHandle *a_pHandle)
{
    tassert_r(NULL, a_pHandle, NULL, 1, "²ÎÊý´íÎó: ¿Õ¾ä±ú");

    return a_pHandle->pstResult && a_pHandle->pstResult->iCount
        ? a_pHandle->pstResult->pstHead
        : NULL;
}


//----------------------------------------------------------------------------
// THE END
//============================================================================
