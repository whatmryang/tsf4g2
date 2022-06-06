//============================================================================
// @Id:       $Id: tsm_index.c 15076 2010-11-23 02:56:24Z flyma $
// @Author:   $Author: flyma $
// @Date:     $Date:: 2010-11-23 10:56:24 +0800 #$
// @Revision: $Revision: 15076 $
// @HeadURL:  $HeadURL:: https://tc-svn.tencent.com/ied/ied_tsf4g_rep/tsf4g_#$
//----------------------------------------------------------------------------


#include "err_stack.h"
#include "tpage_align.h"
#include "tsm_shm.h"
#include "tsm_head.h"
#include "tsm_index.h"


//============================================================================
//  static functions
//----------------------------------------------------------------------------
//  根据四个变量，计算 index  需要的空间尺寸
//----------------------------------------------------------------------------
static size_t tsm_index_size();
static size_t tsm_index_size()
{
    TsmBase *pstBase = TSINGLETON(TsmBase, I);

    if ((size_t) 0 == pstBase->iIndexSize)
    {
        size_t iVar = sizeof(TsmIndex);
        size_t iRet = tpage_align(iVar);

        pstBase->iOffsetLib = iRet;

        iVar  = toffset_array_size(sizeof(TsmObjMeta), pstBase->iLibCap);
        iRet += tpage_align(iVar);

        pstBase->iOffsetUri = iRet;

        iVar  = toffset_array_size(sizeof(TsmObjData), pstBase->iUriCap);
        iRet += tpage_align(iVar);

        pstBase->iOffsetHash = iRet;

        pstBase->iHashCap = pstBase->iLibHash * pstBase->iLibCap + \
                            pstBase->iUriHash * pstBase->iUriCap;

        iVar = toffset_hash_size(pstBase->iHashCap);
        pstBase->iIndexSize = iRet + tpage_align(iVar);
    }

    return pstBase->iIndexSize;
}

//----------------------------------------------------------------------------
//  计算 index的 crc
//----------------------------------------------------------------------------
static int tsm_index_crc(TsmIndex *a_pstIndex);
static int tsm_index_crc(TsmIndex *a_pstIndex)
{
    int iRet = 0;

#define CRC_CALC(P, T) \
    do \
    { \
        int i = 0; \
        const char *b = (const char *)(P); \
        const char *e = b + sizeof(T); \
        while (b != e) \
        { \
            *((char *)&iRet + i) ^= *b++; \
            i = (i + 1) % sizeof(int); \
        } \
    } \
    while (0)

    CRC_CALC(&a_pstIndex->iMagic,    int);
    CRC_CALC(&a_pstIndex->iVersion,  int);
    CRC_CALC(&a_pstIndex->iSelfSize, size_t);

#undef CRC_CALC

    return iRet;
}

//----------------------------------------------------------------------------
//  根据四个变量，初始化一个 index
//----------------------------------------------------------------------------
static int tsm_index_init(TsmIndex *a_pstIndex);
static int tsm_index_init(TsmIndex *a_pstIndex)
{
    TsmBase              *pstBase = TSINGLETON(TsmBase, I);
    pthread_rwlockattr_t  stAttr;
    int                   iRet;

    tassert(a_pstIndex,                   NULL, 1, "参数错误: 句柄为空");
    tassert((TsmIndex *)-1 != a_pstIndex, NULL, 2, "参数错误: 句柄为-1");

    if ((size_t)0 == pstBase->iIndexSize)
    {
        tsm_index_size();
    }

    iRet = toffset_array_init(a_pstIndex,
                              MOFFSET(stLib, TsmIndex),
                              pstBase->iOffsetLib,
                              sizeof(TsmObjMeta),
                              pstBase->iLibCap,
                              pstBase->iLibHash);
    tassert(iRet >= 0, NULL, 4, "初始化失败: Lib");

    iRet = toffset_array_init(a_pstIndex,
                              MOFFSET(stUri, TsmIndex),
                              pstBase->iOffsetUri,
                              sizeof(TsmObjData),
                              pstBase->iUriCap,
                              pstBase->iUriHash);
    tassert(iRet >= 0, NULL, 5, "初始化失败: Uri");

    iRet = toffset_hash_init(a_pstIndex,
                             (char *)&((TsmIndex *)0)->stHash - (char *)0,
                             pstBase->iOffsetHash,
                             pstBase->iHashCap);
    tassert(iRet >= 0, NULL, 6, "初始化失败: Hash");

    a_pstIndex->iVersion  = TSM_TDR_VERSION;
    a_pstIndex->iSelfSize = sizeof(TsmIndex);

#if !defined (_WIN32) && !defined (_WIN64)
    iRet = pthread_rwlockattr_init(&stAttr);
    tassert1(0 == iRet, NULL, 7, "lock attr init: (%d)", iRet);

    iRet = pthread_rwlockattr_setpshared(&stAttr, PTHREAD_PROCESS_SHARED);
    tassert1(0 == iRet, NULL, 8, "lock set attr: (%d)", iRet);

    iRet = pthread_rwlock_init(&a_pstIndex->iLock, &stAttr);
    tassert1(0 == iRet, NULL, 9, "lock init: (%d)", iRet);

    pthread_rwlockattr_destroy(&stAttr);

    tassert(0 == pthread_rwlock_wrlock(&a_pstIndex->iLock),
            NULL, 10, "获得写锁失败");
#endif

    a_pstIndex->iMagic    = TSM_TDR_C_INDEX_KEY;
    a_pstIndex->iCrc      = tsm_index_crc(a_pstIndex);

#if !defined (_WIN32) && !defined (_WIN64)
    tassert(0 == pthread_rwlock_unlock(&a_pstIndex->iLock),
            NULL, 11, "释放写锁失败");
#endif

    return 0;
}

//----------------------------------------------------------------------------
//  检查索引区是否合法
//----------------------------------------------------------------------------
static int tsm_index_check(TsmIndex *a_pstIndex);
static int tsm_index_check(TsmIndex *a_pstIndex)
{
    tassert(a_pstIndex,                   NULL, 1, "参数错误: 句柄为空");
    tassert((TsmIndex *)-1 != a_pstIndex, NULL, 2, "参数错误: 句柄为-1");

    if (TSM_TDR_C_INDEX_KEY != a_pstIndex->iMagic)
    {
        return 1;
    }

    tassert(TSM_TDR_VERSION == a_pstIndex->iVersion, NULL, 3, "版本不匹配");

    tsm_index_base_get(a_pstIndex);

    return 0;
}

//----------------------------------------------------------------------------
//  建立索引区
//----------------------------------------------------------------------------
static int tsm_index_create();
static int tsm_index_create()
{
    int iRet = tsm_shmget(TSINGLETON(TsmBase, I)->iIndexKey,
                          tsm_index_size(),
                          TSM_TDR_SHM_PROP);
    tassert(iRet >= 0, NULL, 1, "shmget 失败");

    return iRet;
}

//============================================================================
//  APIs
//----------------------------------------------------------------------------
//  设置四个变量，以后用这些变量控制 index
//      a_iLibHash      每一条 Lib记录，需要多少个哈希结点做索引
//      a_iLibCap       每个 Inxex中最多可以容纳多少条 Lib记录
//      a_iUriHash      每一条 Uri记录，需要多少个哈希结点做索引
//      a_iUriCap       每个 Inxex中最多可以容纳多少条 Uri记录
//  返回的变量表示，有效的设置个数
//      设置的值和系统中的值一样，看做无效
//  PS: 这四个变量，只能在 tsm_index_init  之前设置
//      否则使用缺省值，缺省值在 tsm_base  中单件初始化函数中设置
//----------------------------------------------------------------------------
int tsm_index_base_set(int a_iIndexKey,
                       int a_iLibHash,
                       int a_iLibCap,
                       int a_iUriHash,
                       int a_iUriCap)
{
    TsmBase *pstBase = TSINGLETON(TsmBase, I);
    int iRet = 0;

#define COMP_SET(V) \
    do \
    { \
        if (a_##V && a_##V != pstBase->V) \
        { \
            ++iRet; \
            pstBase->V = a_##V; \
        } \
    } \
    while (0)

    COMP_SET(iIndexKey);
    COMP_SET(iLibHash);
    COMP_SET(iLibCap);
    COMP_SET(iUriHash);
    COMP_SET(iUriCap);

#undef COMP_SET

    return iRet;
}

//----------------------------------------------------------------------------
//  从 index中取得系统的四个变量，返回有效取得数
//----------------------------------------------------------------------------
int tsm_index_base_get(TsmIndex *a_pstIndex)
{
    TsmBase *pstBase = TSINGLETON(TsmBase, I);
    int iRet = 0;

#define COMP_GET(V, T) \
    do \
    { \
        if (pstBase->i##V##T != a_pstIndex->st##V.iMax##T) \
        { \
            pstBase->i##V##T = a_pstIndex->st##V.iMax##T; \
            ++iRet; \
        } \
    } \
    while (0)
    COMP_GET(Lib, Hash);
    COMP_GET(Lib, Cap);
    COMP_GET(Uri, Hash);
    COMP_GET(Uri, Cap);
#undef COMP_GET

    if (iRet > 0)
    {
        pstBase->iIndexSize = 0;
        tsm_index_size();
    }

    return iRet;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int tsm_index_shmid()
{
    int iRet = tsm_shmget(TSINGLETON(TsmBase, I)->iIndexKey, 0, 0);

    if (iRet < 0)
    {
        iRet = tsm_index_create();
    }
    tassert(iRet >= 0, NULL, 1, "tsm_index_create 失败");

    return iRet;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
TsmIndex *tsm_index_mount()
{
    TsmIndex *pRet = (TsmIndex *)0;
    int       iRet = tsm_index_shmid();

    tassert_r(NULL, -1 != iRet, NULL, 1, "shmget: 返回错误");

    pRet = (TsmIndex *)tsm_shmat(iRet, NULL, 0);
    tassert_r(NULL, pRet && (TsmIndex *)-1 != pRet,
              NULL, 2, "shmget: 返回错误");

    if (tsm_index_check(pRet) > 0)
    {
        if (tsm_index_init(pRet) < 0)
        {
            tsm_shmdt(pRet);
            tassert_r(NULL, 0, NULL, 3, "Index 初始化失败");
        }
    }

    return pRet;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int tsm_index_unmount(TsmIndex *a_pstIndex)
{
    tassert(a_pstIndex,       NULL, 1, "参数错误: 空指针");
    tassert((TsmIndex *)-1 != a_pstIndex, NULL, 2, "参数错误: 指针为-1");

    return tsm_shmdt(a_pstIndex);
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
TsmObjMeta *tsm_head_meta(TsmIndex *a_pstIndex,
                          int       a_iIndex,
                          int       a_iSeg)
{
    TsmObjMeta *pObj;
    TsmBaseVar  stVar;
    size_t      iOffset = MOFFSET(stLib, TsmIndex);

    stVar.iIndex = a_iIndex;

    tassert_r(NULL, a_pstIndex,              NULL, 1, "空句柄");
    tassert_r(NULL, a_iSeg >= 0,             NULL, 2, "段没有对齐");
    tassert_r(NULL, !tsm_base_index(&stVar), NULL, 3, "索引越界");

    if (TSM_URI_META != stVar.iType || a_iSeg != stVar.iCol)
    {
        return NULL;
    }

    pObj = (TsmObjMeta *)toffset_array_ptr(a_pstIndex, iOffset, stVar.iRow);
    tassert_r1(NULL, pObj, NULL, 4, "行越界 row: %d", stVar.iRow);

    return pObj;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
TsmObjData *tsm_head_data(TsmIndex *a_pstIndex,
                          int       a_iIndex,
                          int       a_iSeg)
{
    TsmObjData *pObj;
    TsmBaseVar  stVar;
    size_t      iOffset = MOFFSET(stUri, TsmIndex);

    stVar.iIndex = a_iIndex;

    tassert_r(NULL, a_pstIndex,              NULL, 1, "空句柄");
    tassert_r(NULL, a_iSeg >= 0,             NULL, 2, "段没有对齐");
    tassert_r(NULL, !tsm_base_index(&stVar), NULL, 3, "索引越界");

    if (TSM_URI_DATA != stVar.iType || a_iSeg != stVar.iCol)
    {
        return NULL;
    }

    pObj = (TsmObjData *)toffset_array_ptr(a_pstIndex, iOffset, stVar.iRow);
    tassert_r1(NULL, pObj, NULL, 4, "行越界 row: %d", stVar.iRow);

    return pObj;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int tsm_head_obj_alloc(TsmIndex *a_pstIndex,
                       int       a_iType)
{
    size_t iObj;
    size_t iData;

    tassert(a_pstIndex, NULL, 1, "参数错误: 空句柄");

    if (TSM_URI_META == a_iType)
    {
        iObj = MOFFSET(stLib, TsmIndex);
    }
    else if (TSM_URI_DATA == a_iType)
    {
        iObj = MOFFSET(stUri, TsmIndex);
    }
    else
    {
        tassert1(0, NULL, 2, "错误的类型(%d)", a_iType);
    }

    iData = toffset_array_alloc(a_pstIndex, iObj);
    tassert(iData, NULL, 2, "Meta: toffset_array_alloc 失败");

    return toffset_array_index(a_pstIndex, iObj, iData);
}


//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int tsm_head_obj_free(TsmIndex *a_pstIndex,
                      int       a_iType,
                      int       a_iRow)
{
    size_t iArray;
    size_t iObj;

    tassert(a_pstIndex, NULL, 1, "参数错误: 空句柄");

    if (TSM_URI_META == a_iType)
    {
        iArray = MOFFSET(stLib, TsmIndex);
    }
    else if (TSM_URI_DATA == a_iType)
    {
        iArray = MOFFSET(stUri, TsmIndex);
    }
    else
    {
        tassert1(0, NULL, 2, "错误的类型(%d)", a_iType);
    }

    iObj = toffset_array_offset(a_pstIndex, iArray, a_iRow);
    tassert(iObj, NULL, 3, "行越界");

    return toffset_array_free(a_pstIndex, iArray, iObj);
}


//----------------------------------------------------------------------------
// THE END
//============================================================================
