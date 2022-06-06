/**
*
* @file     tbusd_controller.h 
* @brief    控制端代码
* 
* @author jackyai  
* @version 1.0
* @date 2010-1-27 
*
*
* Copyright (c)  2008-2010, 腾讯科技有限公司互动娱乐研发部架构组
* All rights reserved.
*
*/
#ifndef TBUS_CONTROLLER_H
#define TBUS_CONTROLLER_H

#include "tapp/tapp.h"

#ifdef __cplusplus
extern "C" 
{
#endif 

//tbusd 控制端初始化
int tbusd_contorller_init(struct tagTAPPCTX *pstCtx, void* pvArg);

//处理从tbusd返回的控制响应
int tbusd_proc_ctrlres(const char* pszMetaName, const char* pBuf, int tLen);
#ifdef __cplusplus
}
#endif 
#endif /*#ifndef TBUS_CONTROLLER_H*/
