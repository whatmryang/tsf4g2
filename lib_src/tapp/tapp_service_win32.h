/*
 * Copyright (c) 2010, 腾讯科技有限公司互动娱乐研发�? * All rights reserved
 *
 * File name:       tapp_service_win32.h
 * Description:     declarations for tapp-windows-service-mode
 * Version:         1.0
 *
 * Author:          flyma.IERD.IED.tencent
 * Creation time:   2010-06-12 11:34
 * Last modified:   2010-06-12 11:34
 *
 * History:
 * <date>        <author>      <version>   <description>
 * 2010-06-12     flyma         1.0         creation
 *
 */


#if defined(_WIN32) || defined(_WIN64)

#ifndef _TAPP_SERVICE_WIN32_H_
#define _TAPP_SERVICE_WIN32_H_

#include "tapp/tapp.h"



int tapp_install_windows_service(struct tagTAPPCTX* a_pstTappCtx, void* a_pvArg);
int tapp_uninstall_service(struct tagTAPPCTX* a_pstTappCtx, void* a_pvArg);
int tapp_control_service_start(DWORD dwNumServiceArgs, LPCTSTR* lpServiceArgVectors);
int tapp_control_service_stop();

int tapp_control_service_refresh();
int tapp_set_windows_working_path(void);

#endif

#endif
