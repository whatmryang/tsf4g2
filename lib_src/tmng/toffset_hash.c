//============================================================================
// @Id:       $Id: toffset_hash.c 10522 2010-07-20 02:22:08Z tomaswang $
// @Author:   $Author: tomaswang $
// @Date:     $Date:: 2010-07-20 10:22:08 +0800 #$
// @Revision: $Revision: 10522 $
// @HeadURL:  $HeadURL:: https://tc-svn.tencent.com/ied/ied_tsf4g_rep/tsf4g_#$
//----------------------------------------------------------------------------


#include <stdio.h>

#include "err_stack.h"
#include "toffset_hash.h"


//============================================================================
//  static functions
//----------------------------------------------------------------------------
//  哈希函数
//      a_pData         待计算的数据头指针
//      a_iSize         待计算的数据长度，如果是 0，则当作字符串计算
//  返回哈希值
//----------------------------------------------------------------------------
static int toffset_hash_code(const char *a_pData, size_t a_iSize);
static int toffset_hash_code(const char *a_pData, size_t a_iSize)
{
    int iRet = 0;

    if (0 == a_iSize)
    {
        for (; *a_pData; ++a_pData)
        {
            iRet = 5 * iRet + *a_pData;
        }
    }
    else
    {
        size_t i = 0;

        for (i = 0; i < a_iSize; ++i, ++a_pData)
        {
            iRet = 5 * iRet + *a_pData;
        }
    }

    return iRet < 0 ? -iRet : iRet;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
static int toffset_hash_bucket_count(int a_iNodeCount);
static int toffset_hash_bucket_count(int a_iNodeCount)
{
    static const int _bucket_count_table[] = {
        53,         97,         193,        389,        769,
        1543,       3079,       6151,       12289,      24593,
        49157,      98317,      196613,     393241,     786433,
        1572869,    3145739,    6291469,    12582917,   25165843,
        50331653,   100663319,  201326611,  402653189,  805306457,
        1610612741, 3221225473U,4294967291U
    };
    int raw = a_iNodeCount * 10 / 7;
    int i;

    for (i = 0; i < sizeof(_bucket_count_table) / sizeof(int); ++i)
    {
        if (_bucket_count_table[i] >= raw)
        {
            return _bucket_count_table[i];
        }
    }

    return raw;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
size_t toffset_hash_size(int a_iNodeCount)
{
    tassert_r(0, a_iNodeCount >= 0, NULL, 1, "结点数为负数");

    return sizeof(OffsetList) * (
        a_iNodeCount + toffset_hash_bucket_count(a_iNodeCount)
    );
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int toffset_hash_init(void   *a_pBase,
                      size_t  a_iHash,
                      size_t  a_iBegin,
                      int     a_iNodeCount)
{
    OffsetHash *pstHash = NULL;
    size_t      iBegin;
    size_t      iEnd;
    int         iRet;

    tassert(a_pBase,               NULL, 1, "参数错误: 空基地址");
    tassert((void *)-1 != a_pBase, NULL, 2, "参数错误: 基地址为-1");
    tassert(a_iNodeCount >= 0,     NULL, 3, "参数错误: 结点数小于 0");

    iEnd = a_iBegin + toffset_hash_size(a_iNodeCount);
    if (a_iHash < a_iBegin)
    {
        tassert(a_iBegin >= a_iHash + sizeof(OffsetHash),
                NULL, 4, "参数错误: 管理结构与结点存储区重合");
    }
    else
    {
        tassert(a_iHash >= iEnd,
                NULL, 4, "参数错误: 管理结构与结点存储区重合");
    }

    pstHash = (OffsetHash *)OFF2PTR(a_iHash, a_pBase);

    pstHash->iBegin       = a_iBegin;
    pstHash->iEnd         = iEnd;
    pstHash->iNodeCount   = a_iNodeCount;
    pstHash->iBucketCount = toffset_hash_bucket_count(a_iNodeCount);

    iRet = toffset_array_init(a_pBase,
                              PTR2OFF(&pstHash->stNode, a_pBase),
                              a_iBegin,
                              0,
                              a_iNodeCount,
                              -1);
    tassert(iRet >= 0, NULL, 5, "结点区初始化失败");

    iBegin = pstHash->stNode.iBegin;
    for (iRet = 0; iRet < pstHash->stNode.iMaxCap; ++iRet)
    {
        offset_list_init(iBegin, a_pBase);
        iBegin += sizeof(OffsetList);
    }

    iRet = toffset_array_init(a_pBase,
                              PTR2OFF(&pstHash->stBucket, a_pBase),
                              pstHash->stNode.iEnd,
                              0,
                              pstHash->iBucketCount,
                              -1);
    tassert(iRet >= 0, NULL, 6, "桶区初始化失败");

    iBegin = pstHash->stBucket.iBegin;
    for (iRet = 0; iRet < pstHash->stBucket.iMaxCap; ++iRet)
    {
        offset_list_init(iBegin, a_pBase);
        iBegin += sizeof(OffsetList);
    }

    return 0;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int toffset_hash_add(void       *a_pBase,
                     size_t      a_iHash,
                     int         a_iIndex,
                     const char *a_pData,
                     size_t      a_iSize)
{
    OffsetHash *pstHash = NULL;
    size_t      iNode;
    int         iCode;

    tassert(a_pBase,               NULL, 1, "参数错误: 空基地址");
    tassert((void *)-1 != a_pBase, NULL, 2, "参数错误: 基地址为-1");
    tassert(a_iIndex >= 0,         NULL, 3, "索引为负数");

    pstHash = (OffsetHash *)OFF2PTR(a_iHash, a_pBase);
    tassert(a_iIndex < pstHash->iNodeCount, NULL, 3, "索引溢出");

    iNode = pstHash->iBegin + sizeof(OffsetList) * a_iIndex;
    tassert(offset_list_is_null(iNode, a_pBase),
            NULL, 4, "带加入结点已经在使用");

    iCode  = toffset_hash_code(a_pData, a_iSize) % pstHash->iBucketCount;
    iCode += pstHash->iNodeCount;

    offset_list_add(iNode,
                    pstHash->iBegin + sizeof(OffsetList) * iCode,
                    a_pBase);

    return 0;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int toffset_hash_find(void       *a_pBase,
                      size_t      a_iHash,
                      const char *a_pData,
                      size_t      a_iSize)
{
    OffsetHash *pstHash = NULL;
    size_t      iNode;
    int         iCode;

    tassert(a_pBase,               NULL, 2, "参数错误: 基地址为空");
    tassert((void *)-1 != a_pBase, NULL, 3, "参数错误: 基地址为-1");

    pstHash = (OffsetHash *)OFF2PTR(a_iHash, a_pBase);

    iCode  = toffset_hash_code(a_pData, a_iSize) % pstHash->iBucketCount;
    iCode += pstHash->iNodeCount;

    iNode = pstHash->iBegin + sizeof(OffsetList) * iCode;

    if (offset_list_is_null(iNode, a_pBase))
    {
        return -1;
    }

    iNode = ((OffsetList *)OFF2PTR(iNode, a_pBase))->next;

    iCode = (int)((iNode - pstHash->iBegin) / sizeof(OffsetList));

    tassert(iCode >= 0,                   NULL, 5, "错误: Code下越界");
    tassert(iCode <  pstHash->iNodeCount, NULL, 5, "错误: Code上越界");

    return iCode;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int toffset_hash_next(void   *a_pBase,
                      size_t  a_iHash,
                      int     a_iPrev)
{
    OffsetHash *pstHash = NULL;
    size_t      iNode;
    int         iCode;

    tassert(a_pBase,               NULL, 2, "参数错误: 基地址为空");
    tassert((void *)-1 != a_pBase, NULL, 3, "参数错误: 基地址为-1");
    tassert(a_iPrev >= 0,          NULL, 4, "索引为负数");

    pstHash = (OffsetHash *)OFF2PTR(a_iHash, a_pBase);
    tassert(a_iPrev < pstHash->iNodeCount,
            NULL, 5, "参数错误: 前一个迭代器指针越界");

    iNode = pstHash->iBegin + sizeof(OffsetList) * a_iPrev;
    iNode = ((OffsetList *)OFF2PTR(iNode, a_pBase))->next;
    tassert(iNode >= pstHash->iBegin, NULL, 6, "结点下越界");

    iCode = (int)((iNode - pstHash->iBegin) / sizeof(OffsetList));

    if (iCode < pstHash->iNodeCount)
    {
        return iCode;
    }
    iCode -= pstHash->iNodeCount;
    tassert(iCode < pstHash->iBucketCount, NULL, 7, "结点上越界");

    //------------------------------------------
    //  有可能是被破坏了，但是我们不验证这种情况
    //------------------------------------------

    return -1;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int toffset_hash_del(void   *a_pBase,
                     size_t  a_iHash,
                     int     a_iIndex)
{
    OffsetHash *pstHash = NULL;
    OffsetList *pstNode = NULL;
    size_t      iNode;
    int         iCode;

    tassert(a_pBase,               NULL, 1, "参数错误: 基地址为空");
    tassert((void *)-1 != a_pBase, NULL, 2, "参数错误: 基地址为-1");
    tassert(a_iIndex >= 0,         NULL, 3, "参数错误: 索引为负数");

    pstHash = (OffsetHash *)OFF2PTR(a_iHash, a_pBase);
    tassert(a_iIndex < pstHash->iNodeCount, NULL, 4, "参数错误: 索引越界");

    iNode = pstHash->iBegin + sizeof(OffsetList) * a_iIndex;
    tassert(!offset_list_is_null(iNode, a_pBase),
            NULL, 5, "结点是空值，不可删除");

    pstNode = (OffsetList *)OFF2PTR(iNode, a_pBase);
    tassert(pstNode->prev >= pstHash->iBegin, NULL, 6, "前一结点下越界");
    tassert(pstNode->next >= pstHash->iBegin, NULL, 7, "后一结点下越界");

    iCode = (int)((pstNode->prev - pstHash->iBegin) / sizeof(OffsetList));
    tassert(iCode < pstHash->iNodeCount + pstHash->iBucketCount,
            NULL, 8, "前一结点上越界");

    iCode = (int)((pstNode->next - pstHash->iBegin) / sizeof(OffsetList));
    tassert(iCode < pstHash->iNodeCount + pstHash->iBucketCount,
            NULL, 9, "后一结点上越界");

    offset_list_del(iNode, a_pBase);

    return 0;
}


//----------------------------------------------------------------------------
// THE END
//============================================================================
