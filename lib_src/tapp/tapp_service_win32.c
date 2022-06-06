/*
 * Copyright (c) 2010, 腾讯科技有限公司互动娱乐研发部
 * All rights reserved
 *
 * File name:       tapp_service_win32.c
 * Description:     undo
 * Version:         undo
 *
 * Author:          flyma.IERD.IED.tencent
 * Creation time:   2010-06-12 11:35
 * Last modified:   2010-07-23 10:46
 *
 * History:
 * <date>        <author>      <version>   <description>
 * 2010-06-12     flyma         1.0         creation
 *
 */

#if defined (_WIN32) || defined (_WIN64)

#include "tapp_i.h"
#include "tapp/tapp.h"
#include "tapp_service_win32_i.h"
#include "tapp_service.h"


int tapp_install_windows_service(struct tagTAPPCTX* a_pstTappCtx, void* a_pvArg)
{
    int iRet = 0;
    char szPath[MAX_PATH];
    char szError[1024] = {0};

    assert(NULL != a_pstTappCtx);

    if (GetModuleFileName(NULL, szPath, sizeof(szPath)) == 0)
    {
        printf("Error: failed to GetModuleFileName, for %s\n", GetLastErrorString());
        return -1;
    }

    iRet = CmdInstallService(SERVICE_DEMAND_START, g_stTappService.pszServiceName,
                             g_stTappService.pszServiceDesc,
                             g_stTappService.pszServiceDisplayName, szPath, szError);

    if (0 == iRet)
    {
        printf("install service %s successfully!\n", g_stTappService.pszServiceDisplayName);
    }else
    {
        printf("Error: failed to install service %s, for %s\n", g_stTappService.pszServiceDisplayName, szError);
    }

    return iRet;
}

int tapp_uninstall_service(struct tagTAPPCTX* a_pstTappCtx, void* a_pvArg)
{
    int iRet = 0;
    char szPath[MAX_PATH];
    char szError[1024] = {0};

    assert(NULL != a_pstTappCtx);

    if (GetModuleFileName(NULL, szPath, sizeof(szPath)) == 0)
    {
        printf("Error: failed to GetModuleFileName, for %s\n", GetLastErrorString());
        return -1;
    }

    iRet = CmdRemoveService(g_stTappService.pszServiceName, szError);
    if (0 == iRet)
    {
        printf("uninstall service %s successfully!\n", g_stTappService.pszServiceDisplayName);
    }else
    {
        printf("Error: failed to uninstall service %s, for %s\n", g_stTappService.pszServiceDisplayName, szError);
    }

    return iRet;
}

int tapp_control_service_start(DWORD dwNumServiceArgs, LPCTSTR* lpServiceArgVectors)
{
    int iRet = 0;
    char szPath[MAX_PATH];
    char szError[1024] = {0};



    iRet = CmdStartService(g_stTappService.pszServiceName, g_stTappService.pszServiceDisplayName,
        dwNumServiceArgs, lpServiceArgVectors, szError);

    if (0 == iRet)
    {
        printf("start service %s successfully!\n", g_stTappService.pszServiceDisplayName);
    }else
    {
        printf("Error: failed to start service %s, for %s\n", g_stTappService.pszServiceDisplayName, szError);
    }

    return iRet;
}

int tapp_control_service_stop()
{
    int iRet = 0;
    char szError[1024] = {0};



    iRet = CmdStopService(g_stTappService.pszServiceName,szError);
    if (0 == iRet)
    {
        printf("stop service %s successfully!\n", g_stTappService.pszServiceDisplayName);
    }else
    {
        printf("Error: failed to stop service %s, for %s\n", g_stTappService.pszServiceDisplayName, szError);
    }

    return iRet;
}



int tapp_set_windows_working_path(void)
{
    LONG iRet = 0;
    char szPath[MAX_PATH];


    if (GetModuleFileName(NULL, szPath, sizeof(szPath)) == 0)
    {
        printf("Error: failed to GetModuleFileName, for %s\n", GetLastErrorString());
        return -1;
    }

    iRet = SetServiceCurrentPath(szPath);
    if (0 != iRet)
    {
        printf("Error: failed to SetServiceCurrentPath to directory of %s, iRet<%d>, for %s\n",
               szPath, iRet, GetLastErrorString());
        return -1;
    }

    return 0;
}

int tapp_control_service_refresh()
{
    int iRet = 0;
    char szError[1024] = {0};



    iRet = CmdNotifyServiceRefresh(g_stTappService.pszServiceName,szError);
    if (0 == iRet)
    {
        printf("notify service %s refresh successfully!\n", g_stTappService.pszServiceDisplayName);
    }else
    {
        printf("Error: failed to  notify service %s reload, for %s\n", g_stTappService.pszServiceDisplayName, szError);
    }

    return iRet;
}


#endif
