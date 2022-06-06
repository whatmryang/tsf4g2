//============================================================================
// @Id:       $Id: tsm_head.c 13779 2010-09-26 02:27:10Z tomaswang $
// @Author:   $Author: tomaswang $
// @Date:     $Date:: 2010-09-26 10:27:10 +0800 #$
// @Revision: $Revision: 13779 $
// @HeadURL:  $HeadURL:: https://tc-svn.tencent.com/ied/ied_tsf4g_rep/tsf4g_#$
//----------------------------------------------------------------------------


#include <time.h>
#include <errno.h>
#include <string.h>

#include "pal/tos.h"

#include "tsm_shm.h"
#include "err_stack.h"
#include "tsm_uri_i.h"
#include "tsm_head.h"


#define TSM_TDR_HEAD_LEN sizeof(TsmShmHead)


//============================================================================
//  �ֲ�����
//----------------------------------------------------------------------------
static int tsm_head_check_i(TsmObjPtr *a_pstPtr, TsmShmHead *a_pstData);
static int tsm_head_check_i(TsmObjPtr *a_pstPtr, TsmShmHead *a_pstData)
{
    tassert(a_pstPtr,  NULL, 1, "��������: ����ָ��Ϊ��");
    tassert(a_pstData, NULL, 2, "��������: ͷָ��Ϊ��");

    a_pstPtr->iRef = a_pstData->stPtr.iRef;
    tassert(0 == memcmp(a_pstPtr, a_pstData, sizeof(TsmObjPtr)),
            NULL, 3, "ָ��������ͷ���ݲ�ƥ��");

    return 0;
}


//============================================================================
//  
//----------------------------------------------------------------------------
int tsm_head_init(void        *a_pstPtr,
                  int         a_iUriType,
                  const void *a_pData,
                  size_t      a_iSize)
{
    TsmObjPtr  *pstPtr  = a_pstPtr;
    TsmShmHead *pstHead = NULL;
    int         iShmId;

    tassert(a_pstPtr,             NULL, 1, "��������: ��ָ��");
    tassert((void *)-1 != pstPtr, NULL, 2, "��������: ָ��Ϊ-1");
    tassert(a_iSize,              NULL, 3, "��������: �ߴ�������0");
    tassert(TSM_URI_META == a_iUriType || TSM_URI_DATA == a_iUriType,
            NULL, 4, "����� Uri Type");

    iShmId = tsm_shmget(IPC_PRIVATE,
                        a_iSize + TSM_TDR_HEAD_LEN,
                        TSM_TDR_SHM_PROP);
    tassert(-1 != iShmId, NULL, 5, "���������ڴ����");

    pstHead = (TsmShmHead *)tsm_shmat(iShmId, NULL, 0);
    tassert((TsmShmHead *)-1 != pstHead, NULL, 6, "ȡ�ù����ڴ�ָ�����");

    memset(pstHead, 0, TSM_TDR_HEAD_LEN);

    pstPtr->iRef      = 0;
    pstPtr->iIndexKey = TSINGLETON(TsmBase, I)->iIndexKey;
    pstPtr->iShmId    = iShmId;
    pstPtr->iOffset   = TSM_TDR_HEAD_LEN;
    pstPtr->iSize     = a_iSize;

    pstHead->stTime   = time(NULL);
    pstHead->iUriType = a_iUriType;

    if (TSM_URI_META == a_iUriType)
    {
        memcpy(pstHead, pstPtr, sizeof(TsmObjMeta));
    }
    else if (TSM_URI_DATA == a_iUriType)
    {
        memcpy(pstHead, pstPtr, sizeof(TsmObjData));
    }

    pstHead->iMagic = TSM_TDR_C_INDEX_KEY;

    if (NULL != a_pData)
    {
        memcpy((char *)pstHead + TSM_TDR_HEAD_LEN, a_pData, a_iSize);
    }
    else
    {
        memset((char *)pstHead + TSM_TDR_HEAD_LEN, 0, a_iSize);
    }

    tsm_shmdt(pstHead);
    return 0;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int tsm_head_destroy(TsmObjPtr *a_pstPtr)
{
    tassert(a_pstPtr,               NULL, 1, "��������: ��ָ��");
    tassert(-1 != a_pstPtr->iShmId, NULL, 2, "��������: ������ָ��");
    tassert2(TSINGLETON(TsmBase, I)->iIndexKey == a_pstPtr->iIndexKey,
             NULL, 3, "Index ��ƥ�� (%d:%d)",
             TSINGLETON(TsmBase, I)->iIndexKey,
             a_pstPtr->iIndexKey);

    tassert1(0 == tsm_shmctl(a_pstPtr->iShmId, IPC_RMID, 0),
             NULL, 4, "ɾ�������ڴ�ʧ�� (%d)", errno);

    a_pstPtr->iShmId = -1;

    return 0;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
void *tsm_head_at(TsmObjPtr *a_pstPtr, int a_iReadOnly)
{
    TsmShmHead *pstHead = NULL;

    tassert_r(NULL, a_pstPtr,               NULL, 1, "��������: ��ָ��");
    tassert_r(NULL, (void *)-1 != a_pstPtr, NULL, 2, "��������: ָ��Ϊ-1");
    tassert_r(NULL, a_pstPtr->iSize,        NULL, 3, "��������: �ճߴ�");
    tassert_r2(NULL, TSINGLETON(TsmBase, I)->iIndexKey == a_pstPtr->iIndexKey,
               NULL, 4, "Index ��ƥ�� (%d:%d)",
               TSINGLETON(TsmBase, I)->iIndexKey,
               a_pstPtr->iIndexKey);

    pstHead = (TsmShmHead *)tsm_shmat(a_pstPtr->iShmId, NULL, 0);
    tassert_r(NULL, (TsmShmHead *)-1 != pstHead,
              NULL, 5, "ȡ�ù����ڴ�ָ�����");

    if (tsm_head_check_i(a_pstPtr, pstHead))
    {
        tsm_shmdt(pstHead);

        tassert_r(NULL, 0, NULL, 6, "ָ����ָ����ڴ治ƥ��");
    }

    ++a_pstPtr->iRef;
    ++pstHead->stPtr.iRef;

    return (void *)((char *)pstHead + TSM_TDR_HEAD_LEN);
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int tsm_head_dt(void *a_pstData)
{
    TsmShmHead *pstHead = (TsmShmHead *)((char *)a_pstData - TSM_TDR_HEAD_LEN);
    TsmObjPtr  *pstPtr  = &pstHead->stPtr;

    tassert(a_pstData, NULL, 1, "��������: ��ָ��");
    tassert((void *)-1 != a_pstData, NULL, 2, "��������: ָ��Ϊ-1");
    tassert(!tsm_head_check_i(pstPtr, pstHead),
            NULL, 3, "ָ����ָ����ڴ治ƥ��");
    tassert2(TSINGLETON(TsmBase, I)->iIndexKey == pstPtr->iIndexKey,
             NULL, 4, "Index ��ƥ�� (%d:%d)",
             TSINGLETON(TsmBase, I)->iIndexKey,
             pstPtr->iIndexKey);

    --pstPtr->iRef;
    --pstHead->stPtr.iRef;

    tsm_shmdt(pstHead);

    return 0;
}


//----------------------------------------------------------------------------
// THE END
//============================================================================
