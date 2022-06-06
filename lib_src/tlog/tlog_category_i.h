/**
 *
 * @file    tlogfile.h
 * @brief   ��־�ļ���¼ģ��
 *
 * @author steve
 * @version 1.0
 * @date 2007-04-05
 *
 *
 * Copyright (c)  2007, ��Ѷ�Ƽ����޹�˾���������з���
 * All rights reserved.
 *
 */

#ifndef TLOG_CATEGORY_I_H
#define TLOG_CATEGORY_I_H

#include <stdarg.h>
#include <tlog/tlog_category.h>
#include "tlog_i.h"
#include "tlog_event_i.h"

#define TLOG_NAME_LEN 64

enum enTlogCatFlags
{
    TLOGCAT_FLAG_NONE =0,
    TLOGCAT_FALG_NONE_USE_GETTIMEOFDAY = 0X00000001,
};

struct tagTlogCategoryInnerInst
{
    TLOGCATEGORYINST stCatInst;

    char szName[TLOG_NAME_LEN];
    struct tagTlogCategoryInnerInst* pstParent;
    struct tagTlogCategoryInnerInst* pstForward;
    struct tagTlogCategoryInnerInst* pstCloneParent;

    struct tagTLOGCategory * pstCat;
    struct timeval stCurr;
    char *pszBuff;

    int iInited;
    int iSeq;
    intptr_t iIndex;
    TLOGANY stTlogAnyInst;
    int iFlag;

    /*add by vinson to support thread safe 20110530*/
#ifdef TSF4G_TLOG_THREAD_SAFE
    pthread_mutex_t stLogMutex;
#endif
	
};

typedef struct tagTlogCategoryInnerInst TLOGCATEGORYINNERINST;

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * ����־����Ӽ�¼
 * @param[in] a_pstCatInst ��־����
 * @param[in] a_pstEvt     ��־�¼�����
 *
 * @return 0 �ɹ�
 *		��0 ʧ��
 */
int tlog_category_append(TLOGCATEGORYINNERINST *a_pstCatInst, TLOGEVENTBASE* a_pstEvt);

/**
 * �鿴category�����ȼ��Ƿ���ϣ���������־��������Ĵ�����־�����д����־������1�����򷵻�0
 * @param[in] a_pstCatInst ��־����
 * @param[in] priority     ָ�����ȼ�
 *
 * @return 0 ����Ҫд����־
 *       ��0 ��Ҫд����־
 */
int tlog_category_priority_check_chain(TLOGCATEGORYINNERINST *a_pstCatInst, int a_priority);

int tlog_category_filter_check_chain(TLOGCATEGORYINNERINST *a_pstCatInst, int a_iId, int a_iCls);

#ifdef __cplusplus
}
#endif

#endif /* TLOG_CATEGORY_H */

