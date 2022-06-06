//============================================================================
// @Id:       $Id: tsm_hash.c 13810 2010-09-26 03:24:17Z tomaswang $
// @Author:   $Author: tomaswang $
// @Date:     $Date:: 2010-09-26 11:24:17 +0800 #$
// @Revision: $Revision: 13810 $
// @HeadURL:  $HeadURL:: https://tc-svn.tencent.com/ied/ied_tsf4g_rep/tsf4g_#$
//----------------------------------------------------------------------------


#include <stdlib.h>
#include <sys/types.h>

#include "err_stack.h"
#include "tsm_base.h"
#include "tsm_hash.h"
#include "tsm_head.h"


//============================================================================
//  为 API  定义的一些宏
//----------------------------------------------------------------------------
#define TSM_URI_META_0(P)   ((P)->szUri)
#define TSM_URI_META_1(P)   ((P)->s.m.iBid)
#define TSM_URI_META_2(P)   ((P)->s.m.szLib)
#define TSM_URI_META_3(P)   ((P)->s.m.iLibVer)
#define TSM_URI_META_4(P)   ((P)->s.m.iLibBuildVer)

#define TSM_URI_DATA_0(P)   ((P)->szUri)
#define TSM_URI_DATA_1(P)   ((P)->s.d.iBid)
#define TSM_URI_DATA_2(P)   ((P)->s.d.szProgName)
#define TSM_URI_DATA_3(P)   ((P)->s.d.szProgId)
#define TSM_URI_DATA_4(P)   ((P)->s.d.szFuncName)
#define TSM_URI_DATA_5(P)   ((P)->s.d.szValueName)

#define TSM_HASH(P, T, N)   (T##_##N(P))

#define TSM_HASH_TYPE_I(I) (-1 == (I) ? '*' : 'V')

#define TSM_HASH_TYPE_S(S) ((S[1] || '*' != S[0]) ? 'V' : '*')

#define TSM_HASH_INT(P, T, N) (sizeof(int) == sizeof(TSM_HASH(P, T, N)))

//============================================================================
//  static functions
//----------------------------------------------------------------------------
static char tsm_hash_type(TsmUri *a_pstUri, int a_iSeg);
static char tsm_hash_type(TsmUri *a_pstUri, int a_iSeg)
{
    int i;

    tassert_r('X', a_pstUri, NULL, 1, "参数错误: 空句柄");

#define H_CASE(N, I, T) \
    case N: \
        return TSM_HASH_TYPE_##I(TSM_HASH(a_pstUri, TSM_URI_##T, N)); \
        break

    if (TSM_URI_META == a_pstUri->iType)
    {
        switch (a_iSeg)
        {
            H_CASE(1, I, META);
            H_CASE(2, S, META);
            H_CASE(3, I, META);
            H_CASE(4, I, META);
        case 0:
            for (i = 1; i < 5; ++i)
            {
                if ('V' != tsm_hash_type(a_pstUri, i))
                {
                    return 'X';
                }
            }
            return 'V';
            break;
        default:
            break;
        }
    }
    else if (TSM_URI_DATA == a_pstUri->iType)
    {
        switch (a_iSeg)
        {
            H_CASE(1, I, DATA);
            H_CASE(2, S, DATA);
            H_CASE(3, S, DATA);
            H_CASE(4, S, DATA);
            H_CASE(5, S, DATA);
        case 0:
            for (i = 1; i < 6; ++i)
            {
                if ('V' != tsm_hash_type(a_pstUri, i))
                {
                    return 'X';
                }
            }
            return 'V';
        default:
            break;
        }
    }

#undef H_CASE

    return 'X';
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
static void *tsm_hash_ptr(TsmUri *a_pstUri, int a_iSeg);
static void *tsm_hash_ptr(TsmUri *a_pstUri, int a_iSeg)
{
    tassert_r(NULL, a_pstUri, NULL, 1, "参数错误: 空句柄");

#define H_CASE(N, T) \
    case N: \
        return ((void *)&(TSM_HASH(a_pstUri, TSM_URI_##T, N))); \
        break

    if (TSM_URI_META == a_pstUri->iType)
    {
        switch (a_iSeg)
        {
            H_CASE(0, META);
            H_CASE(1, META);
            H_CASE(2, META);
            H_CASE(3, META);
            H_CASE(4, META);
        default:
            break;
        }
    }
    else if (TSM_URI_DATA == a_pstUri->iType)
    {
        switch (a_iSeg)
        {
            H_CASE(0, DATA);
            H_CASE(1, DATA);
            H_CASE(2, DATA);
            H_CASE(3, DATA);
            H_CASE(4, DATA);
            H_CASE(5, DATA);
        default:
            break;
        }
    }

#undef H_CASE

    return NULL;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
static size_t tsm_hash_size(TsmUri *a_pstUri, int a_iSeg);
static size_t tsm_hash_size(TsmUri *a_pstUri, int a_iSeg)
{
    size_t iRet;

    tassert_r(0, a_pstUri, NULL, 1, "参数错误: 空句柄");

#define H_CASE(N, T) \
    case N: \
        iRet = sizeof(TSM_HASH(a_pstUri, TSM_URI_##T, N)); \
        return (sizeof(int) * 2 < iRet) ? 0 : iRet; \
        break

    if (TSM_URI_META == a_pstUri->iType)
    {
        switch (a_iSeg)
        {
            H_CASE(0, META);
            H_CASE(1, META);
            H_CASE(2, META);
            H_CASE(3, META);
            H_CASE(4, META);
        default:
            break;
        }
    }
    else if (TSM_URI_DATA == a_pstUri->iType)
    {
        switch (a_iSeg)
        {
            H_CASE(0, DATA);
            H_CASE(1, DATA);
            H_CASE(2, DATA);
            H_CASE(3, DATA);
            H_CASE(4, DATA);
            H_CASE(5, DATA);
        default:
            break;
        }
    }

#undef H_CASE

    return 0;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
static int tsm_hash_cmp(TsmUri *a_pstUriS, TsmUri *a_pstUriT, int a_iSeg);
static int tsm_hash_cmp(TsmUri *a_pstUriS, TsmUri *a_pstUriT, int a_iSeg)
{
    char   *pDataS = NULL;
    char   *pDataT = NULL;
    size_t  iSize;
    size_t  i;

    tassert(a_pstUriS, NULL, 1, "参数错误: 空句柄");
    tassert(a_pstUriT, NULL, 2, "参数错误: 空句柄");

    if (a_pstUriS->iType != a_pstUriS->iType)
    {
        return 1;
    }

    pDataS = (char *)tsm_hash_ptr(a_pstUriS, a_iSeg);
    pDataT = (char *)tsm_hash_ptr(a_pstUriT, a_iSeg);
    if (NULL == pDataS || NULL == pDataT)
    {
        return 2;
    }

    iSize = tsm_hash_size(a_pstUriS, a_iSeg);
    if (iSize)
    {
        for (i = 0; i < iSize; ++i)
        {
            if (pDataS[i] != pDataT[i])
            {
                return 3;
            }
        }
    }
    else
    {
        i = 0;
        do
        {
            if (pDataS[i] != pDataT[i])
            {
                return 4;
            }
        } while (pDataS[i++]);
    }

    return 0;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
static int tsm_hash_get_0(TsmIndex *, TsmUri *, int [], int);
static int tsm_hash_get_0(TsmIndex *a_pstIndex,
                          TsmUri   *a_pstUri,
                          int       a_pArray[],
                          int       a_iSize)
{
    TsmBaseVar stHash;
    int        iRet = 0;
    int        i;
   
    i = tsm_hash_find(a_pstIndex, tsm_uri_get(a_pstUri), 0);

    for (; i >= 0 && iRet < a_iSize; i = tsm_hash_next(a_pstIndex, i))
    {
        stHash.iIndex = i;

        tassert1(0 == tsm_base_index(&stHash),
                 NULL, 2, "索引越界: (%d)", stHash.iIndex);

        if (stHash.iType != a_pstUri->iType || stHash.iCol)
        {
            continue;
        }

        a_pArray[iRet++] = stHash.iRow;
    }

    return iRet;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
static int tsm_hash_get_meta(TsmIndex *, TsmUri *, int [], int);
static int tsm_hash_get_meta(TsmIndex *a_pstIndex,
                             TsmUri   *a_pstUri,
                             int       a_pArray[],
                             int       a_iSize)
{
    TsmBase    *pstBase = TSINGLETON(TsmBase, I);
    TsmBaseVar  stHash;
    TsmUri      stUri;
    TsmObjMeta *pObj;
    char        cFlags[8];
    int         i;
    int         j;
    int        *pArray;
    int         iRet = 0;

    for (j = 0, i = 0; i < 8; ++i)
    {
        if ('V' == (cFlags[i] = tsm_hash_type(a_pstUri, i)))
        {
            ++j;
        }
    }

    if (0 == j)
    {
        size_t iArray = MOFFSET(stLib, TsmIndex);
        size_t e = toffset_array_end(a_pstIndex, iArray);
        size_t p = toffset_array_begin(a_pstIndex, iArray);

        while (p != e && iRet < a_iSize)
        {
            a_pArray[iRet++] = toffset_array_index(a_pstIndex, iArray, p);
            p = toffset_array_next(a_pstIndex, iArray, p);
        }

        return iRet;
    }

    pArray = malloc(sizeof(int) * pstBase->iLibCap);
    tassert(pArray, NULL, 1, "malloc error...");

    memset(pArray, 0, sizeof(int) * pstBase->iLibCap);

    for (j = 1; j < 8; ++j)
    {
        if ('V' != cFlags[j])
        {
            continue;
        }

        i = tsm_hash_find(a_pstIndex,
                          tsm_hash_ptr(a_pstUri, j),
                          tsm_hash_size(a_pstUri, j));
        for (; i >= 0; i = tsm_hash_next(a_pstIndex, i))
        {
            pObj = tsm_head_meta(a_pstIndex, i, j);
            stHash.iIndex = i;

            if (NULL == pObj || tsm_base_index(&stHash))
            {
                continue;
            }

            tsm_uri_set_meta(&stUri, &pObj->stMeta);
            if (0 == tsm_hash_cmp(a_pstUri, &stUri, j))
            {
                pArray[stHash.iRow] += 1;
            }
        }
    }

    for (j = 0, i = 1; i < 8; ++i)
    {
        if ('V' == cFlags[i])
        {
            ++j;
        }
    }

    for (i = 0; i < pstBase->iLibCap; ++i)
    {
        if (pArray[i] >= j)
        {
            a_pArray[iRet++] = i;
        }
    }

    free(pArray);

    return iRet;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
static int tsm_hash_get_data(TsmIndex *, TsmUri *, int [], int);
static int tsm_hash_get_data(TsmIndex *a_pstIndex,
                             TsmUri   *a_pstUri,
                             int       a_pArray[],
                             int       a_iSize)
{
    TsmBase    *pstBase = TSINGLETON(TsmBase, I);
    TsmBaseVar  stHash;
    TsmUri      stUri;
    TsmObjData *pObj;
    char        cFlags[8];
    int         i;
    int         j;
    int        *pArray;
    int         iRet = 0;

    for (j = 0, i = 0; i < 8; ++i)
    {
        if ('V' == (cFlags[i] = tsm_hash_type(a_pstUri, i)))
        {
            ++j;
        }
    }

    if (0 == j)
    {
        size_t iArray = MOFFSET(stUri, TsmIndex);
        size_t e = toffset_array_end(a_pstIndex, iArray);
        size_t p = toffset_array_begin(a_pstIndex, iArray);

        while (p != e && iRet < a_iSize)
        {
            a_pArray[iRet++] = toffset_array_index(a_pstIndex, iArray, p);
            p = toffset_array_next(a_pstIndex, iArray, p);
        }

        return iRet;
    }

    pArray = malloc(sizeof(int) * pstBase->iUriCap);
    tassert(pArray, NULL, 1, "malloc error...");

    memset(pArray, 0, sizeof(int) *pstBase->iUriCap);

    for (j = 1; j < 8; ++j)
    {
        if ('V' != cFlags[j])
        {
            continue;
        }

        i = tsm_hash_find(a_pstIndex,
                          tsm_hash_ptr(a_pstUri, j),
                          tsm_hash_size(a_pstUri, j));
        for (; i >= 0; i = tsm_hash_next(a_pstIndex, i))
        {
            stHash.iIndex = i;
            if (tsm_base_index(&stHash) \
                    || TSM_URI_DATA != stHash.iType \
                    || j != stHash.iCol \
                    || NULL == (pObj = tsm_head_data(a_pstIndex,   i, j)))
            {
                continue;
            }

            tsm_uri_set_data(&stUri, &pObj->stData);
            if (0 == tsm_hash_cmp(a_pstUri, &stUri, j))
            {
                pArray[stHash.iRow] += 1;
            }
        }
    }

    for (j = 0, i = 1; i < 8; ++i)
    {
        if ('V' == cFlags[i])
        {
            ++j;
        }
    }

    for (i = 0; i < pstBase->iUriCap; ++i)
    {
        if (pArray[i] >= j)
        {
            a_pArray[iRet++] = i;
        }
    }

    free(pArray);

    return iRet;
}

//============================================================================
//
//----------------------------------------------------------------------------
int tsm_hash_get(TsmIndex *a_pstIndex,
                 TsmUri   *a_pstUri,
                 int       a_pArray[],
                 int       a_iSize)
{
    tassert(a_pstIndex,  NULL, 1, "参数错误: 空索引句柄");
    tassert(a_pstUri,    NULL, 2, "参数错误: 空 Uri句柄");
    tassert(a_iSize > 0, NULL, 3, "参数错误: 希望取得的结果太小");

    tsm_uri_sync(a_pstUri);

    if ('V' == tsm_hash_type(a_pstUri, 0))
    {
        return tsm_hash_get_0(a_pstIndex, a_pstUri, a_pArray, a_iSize);
    }

    if (TSM_URI_META == a_pstUri->iType)
    {
        return tsm_hash_get_meta(a_pstIndex, a_pstUri, a_pArray, a_iSize);
    }

    if (TSM_URI_DATA == a_pstUri->iType)
    {
        return tsm_hash_get_data(a_pstIndex, a_pstUri, a_pArray, a_iSize);
    }

    return terror_push(NULL, 4, "参数错误: Uri 类型不认识");
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
static int tsm_hash_set_meta(TsmIndex *, TsmUri *, int);
static int tsm_hash_set_meta(TsmIndex *a_pstIndex,
                             TsmUri   *a_pstMeta,
                             int       a_iRow)
{
    TsmObjMeta *pObj;
    TsmBaseVar  stHash;
    int         i;

    pObj = toffset_array_ptr(a_pstIndex, MOFFSET(stLib, TsmIndex), a_iRow);

    memcpy(&pObj->stMeta, &a_pstMeta->s.m, sizeof(TsmTdrMetaInfo));

    stHash.iType = TSM_URI_META;
    stHash.iRow  = a_iRow;

    for (i = 0; i < 8; ++i)
    {
        if ('V' != tsm_hash_type(a_pstMeta, i))
        {
            continue;
        }

        stHash.iCol = i;
        stHash.iIndex = -1;

        tassert2(0 == tsm_base_index(&stHash),
                 NULL, 1,
                 "索引越界(t:%d, i:%d)", stHash.iType, stHash.iIndex);

        if (toffset_hash_add(a_pstIndex,
                             MOFFSET(stHash, TsmIndex),
                             stHash.iIndex,
                             tsm_hash_ptr(a_pstMeta, i),
                             tsm_hash_size(a_pstMeta, i)))
        {
            return terror_push1(NULL, 2, "添加索引失败(%d)", stHash.iIndex);
        }
    }

    return 0;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
static int tsm_hash_set_data(TsmIndex *, TsmUri *, TsmUri *, int);
static int tsm_hash_set_data(TsmIndex *a_pstIndex,
                             TsmUri   *a_pstData,
                             TsmUri   *a_pstMeta,
                             int       a_iRow)
{
    TsmObjData *pObj;
    TsmBaseVar  stHash;
    TsmUri      stUri;
    int         i;

    pObj = toffset_array_ptr(a_pstIndex, MOFFSET(stUri, TsmIndex), a_iRow);

    tsm_uri_std_i(&stUri, a_pstData);

    memcpy(&pObj->stData, &stUri.s.d, sizeof(TsmTdrDataInfo));
    if (a_pstMeta)
    {
        memcpy(&pObj->stMeta, &a_pstMeta->s.m, sizeof(TsmTdrMetaInfo));
    }

    stHash.iType = TSM_URI_DATA;
    stHash.iRow  = a_iRow;

    for (i = 0; i < 8; ++i)
    {
        if ('V' != tsm_hash_type(&stUri, i))
        {
            continue;
        }

        stHash.iCol = i;
        stHash.iIndex = -1;

        tassert2(0 == tsm_base_index(&stHash),
                 NULL, 1,
                 "索引越界(t:%d, i:%d)", stHash.iType, stHash.iIndex);

        if (toffset_hash_add(a_pstIndex,
                             MOFFSET(stHash, TsmIndex),
                             stHash.iIndex,
                             tsm_hash_ptr(&stUri, i),
                             tsm_hash_size(&stUri, i)))
        {
            return terror_push1(NULL, 2, "添加索引失败(%d)", stHash.iIndex);
        }
    }

    return 0;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int tsm_hash_set(TsmIndex *a_pstIndex,
                 TsmUri   *a_pstData,
                 TsmUri   *a_pstMeta,
                 int       a_iRow)
{
    tassert(a_pstIndex,             NULL, 1, "参数错误: 空索引句柄");
    tassert(a_pstMeta || a_pstData, NULL, 2, "参数错误: 空 Uri句柄");
    tassert(a_iRow >= 0,            NULL, 3, "参数错误: Row < 0");
    tassert(NULL == a_pstMeta || 'V' == tsm_hash_type(a_pstMeta, 0),
            NULL, 4, "参数错误: Meta Uri 不能是查询类型");
    tassert(NULL == a_pstData || 'V' == tsm_hash_type(a_pstData, 0),
            NULL, 5, "参数错误: Data Uri 不能是查询类型");

    tsm_uri_sync(a_pstData);
    tsm_uri_sync(a_pstMeta);

    return (a_pstData)
        ? tsm_hash_set_data(a_pstIndex, a_pstData, a_pstMeta, a_iRow)
        : tsm_hash_set_meta(a_pstIndex, a_pstMeta, a_iRow);
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
static int tsm_hash_del_meta(TsmIndex *, int);
static int tsm_hash_del_meta(TsmIndex *a_pstIndex, int a_iRow)
{
    TsmObjMeta *pObj;
    TsmBaseVar  stHash;
    TsmUri      stUri;
    int         i;
   
    pObj = toffset_array_ptr(a_pstIndex, MOFFSET(stLib, TsmIndex), a_iRow);

    stHash.iType = TSM_URI_META;
    stHash.iRow  = a_iRow;

    tsm_uri_set_meta(&stUri, &pObj->stMeta);

    for (i = 0; i < 8; ++i)
    {
        if ('V' != tsm_hash_type(&stUri, i))
        {
            continue;
        }

        stHash.iCol   = i;
        stHash.iIndex = -1;

        tassert(0 == tsm_base_index(&stHash), NULL, 1, "索引越界");

        tassert(0 == toffset_hash_del(a_pstIndex,
                                      MOFFSET(stHash, TsmIndex),
                                      stHash.iIndex),
                NULL, 2, "hash结点删除失败");
    }

    return 0;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
static int tsm_hash_del_data(TsmIndex *, int);
static int tsm_hash_del_data(TsmIndex *a_pstIndex, int a_iRow)
{
    TsmObjData *pObj;
    TsmBaseVar  stHash;
    TsmUri      stUri;
    int         i;
   
    pObj = toffset_array_ptr(a_pstIndex, MOFFSET(stUri, TsmIndex), a_iRow);

    stHash.iType = TSM_URI_DATA;
    stHash.iRow  = a_iRow;

    tsm_uri_set_data(&stUri, &pObj->stData);

    for (i = 0; i < 8; ++i)
    {
        if ('V' != tsm_hash_type(&stUri, i))
        {
            continue;
        }

        stHash.iCol   = i;
        stHash.iIndex = -1;

        tassert(0 == tsm_base_index(&stHash), NULL, 1, "索引越界");

        tassert(0 == toffset_hash_del(a_pstIndex,
                                      MOFFSET(stHash, TsmIndex),
                                      stHash.iIndex),
                NULL, 2, "hash结点删除失败");
    }

    return 0;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int tsm_hash_del(TsmIndex *a_pstIndex, int a_iType, int a_iRow)
{
    tassert(a_pstIndex, NULL, 1, "参数错误: 空句柄");

    if (TSM_URI_META == a_iType)
    {
        return tsm_hash_del_meta(a_pstIndex, a_iRow);
    }

    if (TSM_URI_DATA == a_iType)
    {
        return tsm_hash_del_data(a_pstIndex, a_iRow);
    }

    tassert(0, NULL, 2, "参数错误: 不认识的类型");
}


//----------------------------------------------------------------------------
// THE END
//============================================================================
