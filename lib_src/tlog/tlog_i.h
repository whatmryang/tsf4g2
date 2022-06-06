#ifndef __TLOG_I_H__
#define  __TLOG_I_H__

#define TLOG_INTERNAL

#include "pal/pal.h"
#include "tlog/tlog.h"
#include "tdr/tdr.h"
#include "tlog/tlogerr.h"
#include "tlogfilter.h"
#include "tlogfile.h"
#include "tlognet.h"
#include "tlogvec.h"
#include "tlogany.h"
#include "tlog_category_def.h"
#include "tlogfiledef.h"
#include "tlognetdef.h"
#include "tlogvecdef.h"
#include "tloganydef.h"
#include "tlogfilterdef.h"
#include "tlog/tlog_priority_def.h"
#include "tlog_category_def.h"
#include "comm/comm.h"
#include "tlogconf.h"

#include "tlog_layout.h"
#include "tlog_category_i.h"

#include "tlog_event_i.h"
#include "tlog/tlog_priority_def.h"

#include "tlogfilter.h"
#include "tlogfile.h"
#include "tlognet.h"
#include "tlogvec.h"
#include "tlogbin.h"

#include "../lib_src/comm/comm_i.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define TLOG_CHAR_DOT		'.'

#define TLOG_MAX_HOST_NAME 128
#define TLOG_MAX_MODULE_NAME 128

typedef struct tagTLOGConf *LPTLOGCONFHANDLER;

struct tagTLogCtx;
typedef struct tagTLogCtx TLOGCTX;

struct tagTLogConfInst;
typedef struct tagTLogConfInst *LPTLOGCONFINST;
typedef struct tagTLogConfInst TLOGCONFINST;

struct tagTLogConfInst
{
    time_t tLastSync;
    int iCount;
    TLOGCATEGORYINNERINST astCatInsts[TLOG_MAX_CATEGORY];
};

struct tagTLogCtx
{
    char szHostName[TLOG_MAX_HOST_NAME];
    char szModuleName[TLOG_MAX_MODULE_NAME];
    TLOGCONF* pstConf;
    TLOGCONFINST stInst;
};

#define CATINST2LOGCTX(a_pstConf)  ((LPTLOGCTX)(((intptr_t)a_pstConf) - a_pstConf->iIndex * sizeof(TLOGCATEGORYINNERINST) - offsetof(TLOGCTX,stInst) - offsetof(TLOGCONFINST,astCatInsts)))
#define CATINST2DEVANY(a_pstConf) (&(a_pstConf->stTlogAnyInst))

/**
 * 获取tlog_def的meta描述
 *
 * @return  NULL 失败
 *			非NULL 成功
 */
const void* tlog_get_meta_data(void);
/**
 * 分配日志上下文内存
 *
 * @return  NULL 失败
 *			非NULL 成功
 */
LPTLOGCTX tlog_alloc(void);

/**
 * 释放日志上下文内存
 * @param[in,out] a_ppstCtx 日志上下文
 *
 * @return  0 成功
 *			非0 失败
 */
int tlog_free(LPTLOGCTX* a_ppstCtx);

/**
 * 初始化日志上下文
 * @param[in,out] a_pstCtx 日志上下文
 * @param[in] a_pstConf 日志配置内容
 *
 * @return  0 成功
 *			非0 失败
 */
int tlog_init(LPTLOGCTX a_pstCtx, LPTLOGCONF a_pstConf);

/**
 * 重新加载日志配置
 * @param[in] a_pstCtx 日志上下文
 * @param[in] a_pstConf 新的配置
 *
 * @return  0 成功
 *			非0 失败
 * @note
 *		只能重新加载过滤器和优先级，多线程安全。
 */
int tlog_reload(LPTLOGCTX a_pstCtx, LPTLOGCONF a_pstConf);

/**
 * 清除日志上下文
 * @param[in] a_pstCtx 日志上下文
 *
 * @return  0 成功
 *			非0 失败
 */
int tlog_fini(LPTLOGCTX a_pstCtx);

#ifdef __cplusplus
}
#endif
#endif
