/**
*
* @file     tbusd_log.h
* @brief    tbusd日志处理相关接口
*
* @author jackyai
* @version 1.0
* @date 2008-05-19
*
*
* Copyright (c)  2008, 腾讯科技有限公司互动娱乐研发部架构组
* All rights reserved.
*
*/

#ifndef TBUSD_LOG_H
#define TBUSD_LOG_H

#include "tdr/tdr.h"
#include "tlog/tlog.h"
#include "tbusd.h"
#include "tapp/tapp.h"
#include "../../lib_src/tloghelp/tlogload_i.h"

#if defined(_WIN32) || defined(_WIN64)
#pragma warning(disable:4204)
#endif

enum tagTbusdLogCls
{
        TBUSD_LOGCLS_RELAYID = 1,   /**日志过滤clsid，根据relay的id来进行过滤*/
};

enum tagRealyLogLevel
{
	TBUSD_PRIORITY_FATAL = TLOG_PRIORITY_FATAL,
	TBUSD_PRIORITY_ERROR = TLOG_PRIORITY_ERROR,
	TBUSD_PRIORITY_INFO = TLOG_PRIORITY_INFO,
	TBUSD_PRIOROTY_WARN = TLOG_PRIORITY_WARN,
	TBUSD_PRIORITY_DEBU = TLOG_PRIORITY_DEBUG,
	TBUSD_PRIORITY_TRACE = TLOG_PRIORITY_TRACE,
};

extern LPTLOGCATEGORYINST g_ptRelayLog ;
extern LPTLOGCATEGORYINST g_ptStatLog  ;

/**根据日志配置问题初始化日志系统
*/
int tbusd_log_init(TAPPCTX *a_pstAppCtx,  LPRELAYENV a_pstRunEnv);

/** 释放日志系统
*/
int tbusd_log_fini(void);

/**
* Log a message
* @param a_category the log4c_category_t object
* @param a_priority priority of msg
* @param a_format Format specifier for the string to write
* in the log file.
* @param ... The arguments for a_format
**/
#define tbusd_log_msg(cat, priority, fmt, ...) \
{\
	if(tlog_category_is_priority_enabled(cat, priority))\
	{\
		TLOGEVENT stEvt;\
		const TLOGLOCINFO locinfo = TLOG_LOC_INFO_INITIALIZER(NULL);\
		stEvt.evt_priority = priority; \
		stEvt.evt_id = 0; \
		stEvt.evt_cls  = 0; \
		stEvt.evt_is_msg_binary = 0; \
		stEvt.evt_loc = &locinfo; \
	tlog_category_logv(cat, &stEvt, fmt, __VA_ARGS__);\
	}\
}


/**
* Log a message
* @param a_category the log4c_category_t object
* @param a_priority priority of msg
* @param a_format Format specifier for the string to write
* in the log file.
* @param a_pData point of data
* @param a_iDataLen bytes number of data
**/
void tbusd_log_bin(const LPTLOGCATEGORYINST a_category, int a_priority,
						   void *a_pData, int a_iDataLen);

/**
* Log a message
* @param a_category the log4c_category_t object
* @param a_priority priority of msg
* @param a_pstMeta meta of data
* @param a_format Format specifier for the string to write
* in the log file.
* @param a_pData point of data
* @param a_iDataLen bytes number of data
**/
void tbusd_log_data(const LPTLOGCATEGORYINST a_category, int a_priority,
				   LPTDRMETA a_pstMeta, void *a_pData, int a_iDataLen);


#endif
