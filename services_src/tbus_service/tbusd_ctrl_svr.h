/**
*
* @file     tbusd_ctrl_svr.h  
* @brief    tbusd命令行控制处理模块
* 
* @author jackyai  
* @version 1.0
* @date 2010-1-26 
*
*
* Copyright (c)  2008-2010, 腾讯科技有限公司互动娱乐研发部架构组
* All rights reserved.
*
*/
#ifndef TBUS_PROC_CTRLPKG_H
#define TBUS_PROC_CTRLPKG_H


#ifdef __cplusplus
extern "C" 
{
#endif 

/*获取进程控制使用帮助*/
const char* tbusd_get_ctrl_usage(void);

/**处理控制命令名 
*/
int tbusd_proc_ctrl_cmdline(struct tagTAPPCTX *a_pstCtx, void* a_pvArg, unsigned short argc, const char** argv);



#ifdef __cplusplus
}
#endif 
#endif /*#ifndef TBUS_PROC_CTRLPKG_H*/
