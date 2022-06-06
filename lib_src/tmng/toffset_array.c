//============================================================================
// @Id:       $Id: toffset_array.c 10522 2010-07-20 02:22:08Z tomaswang $
// @Author:   $Author: tomaswang $
// @Date:     $Date:: 2010-07-20 10:22:08 +0800 #$
// @Revision: $Revision: 10522 $
// @HeadURL:  $HeadURL:: https://tc-svn.tencent.com/ied/ied_tsf4g_rep/tsf4g_#$
//----------------------------------------------------------------------------


#include <stdio.h>
#include <string.h>

#include "err_stack.h"
#include "toffset_array.h"


//----------------------------------------------------------------------------
//  ��ʼ��һ�� OffsetArray  ����Ľṹ
//----------------------------------------------------------------------------
size_t toffset_array_size(size_t a_iSize, int a_iMaxCap)
{
    tassert(a_iMaxCap >= 0, NULL, 1, "��������: ����Ϊ����");

    return (sizeof(OffsetList) + a_iSize) * a_iMaxCap;
}

//----------------------------------------------------------------------------
//  ��ʼ��һ�� OffsetArray  ����Ľṹ
//----------------------------------------------------------------------------
int toffset_array_init(void   *a_pBase,
                       size_t  a_iArray,
                       size_t  a_iBegin,
                       size_t  a_iSize,
                       int     a_iMaxCap,
                       int     a_iMaxHash)
{
    OffsetArray *pstArray = NULL;
    size_t       iData;
    size_t       iEnd;
    int          i;

    tassert(a_pBase,               NULL, 1, "��������: ����ַΪ��");
    tassert((void *)-1 != a_pBase, NULL, 2, "��������: ����ַΪ-1");
    tassert(a_iMaxCap >= 0,        NULL, 3, "��������: ����С�� 0");

    iEnd = a_iBegin + toffset_array_size(a_iSize, a_iMaxCap);
    if (a_iArray < a_iBegin)
    {
        tassert(a_iBegin >= a_iArray + sizeof(OffsetArray),
                NULL, 4, "�ṹ�غ�: ����ͷ�ڿ��ƽṹ��");
    }
    else
    {
        tassert(a_iArray >= iEnd,
                NULL, 5, "�ṹ�غ�: ���ƽṹ��������");
    }

    pstArray = (OffsetArray *)OFF2PTR(a_iArray, a_pBase);

    pstArray->iBegin   = a_iBegin;
    pstArray->iEnd     = iEnd;
    pstArray->iMaxHash = a_iMaxHash;
    pstArray->iSize    = a_iSize;
    pstArray->iMaxCap  = a_iMaxCap;
    pstArray->iCurCap  = 0;

    iData = PTR2OFF(&pstArray->stHeadUsed, a_pBase);
    offset_list_init(iData, a_pBase);
    iData = PTR2OFF(&pstArray->stHeadFree, a_pBase);
    offset_list_init(iData, a_pBase);

    if (a_iMaxHash < 0)
    {
        for (i = 0; i < a_iMaxCap; ++i)
        {
            offset_list_init(a_iBegin, a_pBase);
            a_iBegin += sizeof(OffsetList) + a_iSize;
        }
    }
    else
    {
        for (i = 0; i < a_iMaxCap; ++i)
        {
            offset_list_add(a_iBegin, iData, a_pBase);
            a_iBegin += sizeof(OffsetList) + a_iSize;
        }
    }

    return 0;
}

//----------------------------------------------------------------------------
//  ��� OffsetArray  �Ƿ��п��нڵ���Է���
//----------------------------------------------------------------------------
int toffset_array_stat(void *a_pBase, size_t a_iArray)
{
    OffsetArray *pstArray = (OffsetArray *)OFF2PTR(a_iArray, a_pBase);

    return pstArray->iMaxCap - pstArray->iCurCap;
}

//----------------------------------------------------------------------------
//  ����һ�����нڵ�
//----------------------------------------------------------------------------
size_t toffset_array_alloc(void *a_pBase, size_t a_iArray)
{
    OffsetArray *pstArray = (OffsetArray *)OFF2PTR(a_iArray, a_pBase);
    size_t       iData;
    size_t       iUsed;

    if (pstArray->iCurCap >= pstArray->iMaxCap)
    {
        return (size_t)0;
    }

    iData = pstArray->stHeadFree.next;
    if (iData == a_iArray + MOFFSET(stHeadFree, OffsetArray))
    {
        terror_push(NULL, 1, "����״̬��һ��");
        return (size_t)0;
    }

    iUsed = a_iArray + MOFFSET(stHeadUsed, OffsetArray);
    offset_list_del(iData, a_pBase);
    offset_list_add(iData, iUsed, a_pBase);

    ++pstArray->iCurCap;

    iData += sizeof(OffsetList);
    memset(OFF2PTR(iData, a_pBase), 0, pstArray->iSize);

    return iData;
}

//----------------------------------------------------------------------------
//  �ͷ�һ����ǰ����Ľڵ�
//----------------------------------------------------------------------------
int toffset_array_free(void *a_pBase, size_t a_iArray, size_t a_iNode)
{
    OffsetArray *pstArray = (OffsetArray *)OFF2PTR(a_iArray, a_pBase);
    size_t       iData;
    size_t       iFree;

    tassert(pstArray->iCurCap > 0,         NULL, 1, "����״̬��һ��");
    tassert(a_iNode >= sizeof(OffsetList), NULL, 2, "ƫ��̫С");

    iData = a_iNode - sizeof(OffsetList);

    iFree = a_iArray + MOFFSET(stHeadFree, OffsetArray);
    offset_list_del(iData, a_pBase);
    offset_list_add(iData, iFree, a_pBase);

    --pstArray->iCurCap;

    return 0;
}

//----------------------------------------------------------------------------
//  ͨ����������ȡ������ƫ��
//----------------------------------------------------------------------------
size_t toffset_array_offset(void *a_pBase, size_t a_iArray, int a_iIndex)
{
    OffsetArray *pstArray = (OffsetArray *)OFF2PTR(a_iArray, a_pBase);
    size_t       iSize;

    tassert_r(0, a_iIndex >= 0,                NULL, 1, "��������Ϊ����");
    tassert_r(0, a_iIndex < pstArray->iMaxCap, NULL, 2, "����������������");

    iSize = sizeof(OffsetList) + pstArray->iSize;

    return pstArray->iBegin + iSize * a_iIndex + sizeof(OffsetList);
}

//----------------------------------------------------------------------------
//  ͨ����������ȡ������ָ��
//----------------------------------------------------------------------------
void *toffset_array_ptr(void *a_pBase, size_t a_iArray, int a_iIndex)
{
    size_t iOffset = toffset_array_offset(a_pBase, a_iArray, a_iIndex);

    return 0 == iOffset ? NULL : OFF2PTR(iOffset, a_pBase);
}

//----------------------------------------------------------------------------
//  ͨ������ƫ�ƣ�ȡ����������
//----------------------------------------------------------------------------
int toffset_array_index(void *a_pBase, size_t a_iArray, size_t a_iOffset)
{
    OffsetArray *pstArray = (OffsetArray *)OFF2PTR(a_iArray, a_pBase);
    size_t       iSize;

    tassert(a_iOffset >= pstArray->iBegin, NULL, 1, "ƫ��̫С");
    tassert(a_iOffset <  pstArray->iEnd,   NULL, 2, "ƫ��̫��");

    iSize = sizeof(OffsetList) + pstArray->iSize;
    tassert(sizeof(OffsetList) == (a_iOffset - pstArray->iBegin) % iSize,
            NULL, 3, "ƫ�ƺ����鲻����");

    return (int)((a_iOffset - pstArray->iBegin) / iSize);
}

//----------------------------------------------------------------------------
//  �õ�������ͷ�ڵ�ƫ��
//----------------------------------------------------------------------------
size_t toffset_array_begin(void *a_pBase, size_t a_iArray)
{
    return sizeof(OffsetList) + (
        (OffsetArray *)OFF2PTR(a_iArray, a_pBase)
    )->stHeadUsed.next;
}

//----------------------------------------------------------------------------
//  �õ�������β�ڵ�ƫ��
//----------------------------------------------------------------------------
size_t toffset_array_end(void *a_pBase, size_t a_iArray)
{
    return a_iArray + MOFFSET(stHeadUsed, OffsetArray) + sizeof(OffsetList);
}

//----------------------------------------------------------------------------
// �õ���������һ���ڵ�ƫ��
//----------------------------------------------------------------------------
size_t toffset_array_next(void *a_pBase, size_t a_iArray, size_t a_iNode)
{
    return a_iNode < sizeof(OffsetList)
        ? toffset_array_end(a_pBase, a_iArray)
        : sizeof(OffsetList) + (
            (OffsetList *)OFF2PTR(a_iNode - sizeof(OffsetList), a_pBase)
        )->next;
}


//----------------------------------------------------------------------------
// THE END
//============================================================================
