/**
 *
 * @file    tlogfile.h
 * @brief   日志文件记录模块
 *
 * @author steve
 * @version 1.0
 * @date 2007-04-05
 *
 *
 * Copyright (c)  2007, 腾讯科技有限公司互动娱乐研发部
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
 * 向日志类添加记录
 * @param[in] a_pstCatInst 日志类句柄
 * @param[in] a_pstEvt     日志事件内容
 *
 * @return 0 成功
 *		非0 失败
 */
int tlog_category_append(TLOGCATEGORYINNERINST *a_pstCatInst, TLOGEVENTBASE* a_pstEvt);

/**
 * 查看category的优先级是否符合，如果这个日志类或者他的传递日志类可以写入日志，返回1，否则返回0
 * @param[in] a_pstCatInst 日志类句柄
 * @param[in] priority     指定优先级
 *
 * @return 0 不需要写入日志
 *       非0 需要写入日志
 */
int tlog_category_priority_check_chain(TLOGCATEGORYINNERINST *a_pstCatInst, int a_priority);

int tlog_category_filter_check_chain(TLOGCATEGORYINNERINST *a_pstCatInst, int a_iId, int a_iCls);

#ifdef __cplusplus
}
#endif

#endif /* TLOG_CATEGORY_H */

