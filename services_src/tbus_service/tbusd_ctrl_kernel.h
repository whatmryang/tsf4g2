/**
*
* @file     tbusd_ctrl_kernel.h 
* @brief    控制核心代码
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
#ifndef TBUS_CTRL_KERNEL_H
#define TBUS_CTRL_KERNEL_H

#include <stdio.h>
#include "pal/pal.h"
#include "tbusd_ctrl_proto.h"

#define TBUSD_CTRL_PKG_META_NAME    "TbusCtrlPkg"

#ifdef __cplusplus
extern "C" 
{
#endif 


    /** 初始化控制消息处理环境 本接口是非线程安全的。
    */
    int tbusdctrl_init();

    /*处理环境释放相关资源资源
    */
    void tbusdctrl_fini();


    /** 打包控制消息，并发送到控制通道中。本接口是非线程安全的。
    @param[in] a_pstPkg 待发送的数据
    @return 0 成功 非0：失败
    */
    int tbusdctrl_send_pkg(IN  LPTBUSCTRLPKG a_pstPkg);

    /** 解包控制消息
    @param[out] a_pstPkg 保存解包后的消息
    @param[in] a_pszNet 网络消息缓冲区
    @param[in] a_iLen 网络消息长度
    @return 0 成功 非0：失败
    */
    int tbusdctrl_unpack_pkg(OUT  LPTBUSCTRLPKG a_pstPkg, const char *a_pszNet, int a_iLen);


    /** 打印控制消息
    @param[in] a_pstPkg 保存解包后的消息
    @return 0 成功 非0：失败
    */
    int tbusdctrl_print_pkg(OUT  LPTBUSCTRLPKG a_pstPkg, FILE *fpOut);


    //初始化控制消息头部
    void tbusdctrl_fill_pkghead(OUT TBUSCTRLHEAD *a_pstHead, unsigned int a_dwCmd, unsigned int a_dwVer);

#ifdef __cplusplus
}
#endif 
#endif /*#ifndef TBUS_CTRL_KERNEL_H*/
