/**
*
* @file    tlog_layout.h  
* @brief   日志格式化模块
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


#ifndef TLOG_LAYOUT_H
#define TLOG_LAYOUT_H

#include "tlog_event_i.h"

#ifdef __cplusplus
extern "C"
{
#endif


#define TLOG_LAYOUT_DEF_FORMAT  "[%d] %m"

/**
* 格式化日志内容
* @param[in,out] a_pstEvt 日志事件结构体
* @param[in] a_pszFmt 日志格式化串
*

*/
void tlog_layout_format(TLOGEVENTBASE* a_pstEvt, const char* a_pszFmt);

#ifdef __cplusplus
}
#endif

#endif /* TLOG_LAYOUT_H */


