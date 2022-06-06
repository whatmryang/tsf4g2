/*
 * Copyright (c) 2010, 腾讯科技有限公司互动娱乐研发部
 * All rights reserved
 *
 * File name:       tapp_service.c
 * Description:     definitions for tapp-windows-service-mode
 * Version: 1.0
 *
 * Author:          flyma.IERD.IED.tencent
 * Creation time:   2010-06-12 09:17
 * Last modified:   2010-07-27 14:51
 *
 * History:
 * <date>        <author>      <version>   <description>
 * 2010-06-12     flyma         1.0         creation
 *
 */

#include "tapp_i.h"
#include "tapp/tapp.h"
#if defined (_WIN32) || defined (_WIN64)
#include "tapp_service_win32.h"
#endif
#include "tapp_service.h"

struct tagTappService g_stTappService;
extern int tapp_control_mode_init(TAPPCTX* a_pstCtx, void* a_pvArg);

static int tapp_service_init_non_service(LPTAPPCTX a_pstCtx, void*a_pstEnv)
{
    int iRet;

    assert(NULL != a_pstCtx);

    iRet = tapp_def_init(a_pstCtx, a_pstEnv);
    if (0 != iRet)
    {
        fprintf(stderr, "ERROR: tapp_def_init failed, please check log for detail reason\n");
        return -1;
    }

    return iRet;
}

TSF4G_API int tapp_config_service_desc(const char* a_pszServiceDesc)
{
    int iRet = 0;

#if defined (_WIN32) || defined (_WIN64)
    if (NULL == a_pszServiceDesc)
    {
        fprintf(stderr, "parameter error, pointer to sverce-description is NULL\n");
        return -1;
    }

    /* check paramters */
    if (strlen(a_pszServiceDesc) == 0)
    {
        fprintf(stderr, "service-description can't be empty\n");
        return -1;
    }

    /* save service-description */
    if (strlen(a_pszServiceDesc) >= sizeof(g_stTappService.szServiceDesc))
    {
        fprintf(stderr, "service-description<length: %d> surpass max-supported length<%d>\n",
                strlen(a_pszServiceDesc), (int)sizeof(g_stTappService.szServiceDesc));
        return -1;
    }
    strncpy(g_stTappService.szServiceDesc, a_pszServiceDesc, sizeof(g_stTappService.szServiceDesc));
    g_stTappService.szServiceDesc[sizeof(g_stTappService.szServiceDesc)-1] = '\0';

    g_stTappService.pszServiceDesc = g_stTappService.szServiceDesc;

#endif

    return iRet;
}

TSF4G_API int tapp_set_service_display_name(const char* a_pszDisplayName)
{
    int iRet = 0;

#if defined (_WIN32) || defined (_WIN64)

    if (NULL == a_pszDisplayName)
    {
        fprintf(stderr, "parameter error, pointer to sverce-name is NULL\n");
        return -1;
    }

    if (strlen(a_pszDisplayName) == 0)
    {
        fprintf(stderr, "service-name can't be empty\n");
        return -1;
    }

    /* save service-name */
    if (strlen(a_pszDisplayName) >= sizeof(g_stTappService.szServiceDisplayName))
    {
        fprintf(stderr, "service-name<length: %d> surpass max-supported length<%d>\n",
                strlen(a_pszDisplayName), (int)sizeof(g_stTappService.szServiceDisplayName));
        return -1;
    }

    strncpy(g_stTappService.szServiceDisplayName, a_pszDisplayName,
            sizeof(g_stTappService.szServiceDisplayName));
    g_stTappService.szServiceDisplayName[sizeof(g_stTappService.szServiceDisplayName)-1] = '\0';
    g_stTappService.pszServiceDisplayName = g_stTappService.szServiceDisplayName;

#endif

    return iRet;
}

TSF4G_API int tapp_register_service_init(PFNTAPP_SERVICE_INIT a_pfnSvrInit)
{
    int iRet = 0;

    if (NULL == a_pfnSvrInit)
    {
        fprintf(stderr, "parameter error, function pointer to ServiceInit is NULL\n");
        return -1;
    }

    g_stTappService.pfnServiceInit = a_pfnSvrInit;

    return iRet;
}

#if defined (_WIN32) || defined (_WIN64)

/* global variable definition */
static SERVICE_STATUS gs_ServiceStatus;
static SERVICE_STATUS_HANDLE gs_ServiceStatusHandle;
static FILE*  gs_fpTappSvcLog;

void tapp_svc_log_init()
{
    char szFile[512] ={0};

    snprintf(szFile, sizeof(szFile)-1, "./%s.svc.log", g_stTappService.pszServiceName);

    gs_fpTappSvcLog = fopen(szFile, "wb");
}

void tapp_svc_log_fini()
{
    if (NULL != gs_fpTappSvcLog)
    {
        fclose(gs_fpTappSvcLog);
        gs_fpTappSvcLog = NULL;
    }
}

static void ReportSvcStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint)
{
    static DWORD dwCheckPoint = 1;

    gs_ServiceStatus.dwCurrentState = dwCurrentState;
    gs_ServiceStatus.dwWin32ExitCode = dwWin32ExitCode;
    gs_ServiceStatus.dwWaitHint = dwWaitHint;

    if (dwCurrentState == SERVICE_START_PENDING)
    {
        gs_ServiceStatus.dwControlsAccepted = 0;
    } else
    {
        gs_ServiceStatus.dwControlsAccepted
            = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_PAUSE_CONTINUE;
    }

    if ((dwCurrentState == SERVICE_RUNNING) || (dwCurrentState == SERVICE_STOPPED))
    {
        gs_ServiceStatus.dwCheckPoint = 0;
    } else
    {
        gs_ServiceStatus.dwCheckPoint = dwCheckPoint++;
    }

    SetServiceStatus(gs_ServiceStatusHandle, &gs_ServiceStatus);
}

void WINAPI TappServiceCtrlHandler(DWORD a_dwOpcode)
{
    switch(a_dwOpcode)
    {
        case SERVICE_CONTROL_STOP:
        case SERVICE_CONTROL_SHUTDOWN:
            {
                tapp_exit_mainloop();
                ReportSvcStatus(SERVICE_STOP_PENDING, 0, 30000);
            }
            break;
        case SERVICE_ACCEPT_PAUSE_CONTINUE:
            {
                /*通过此信号来通知进程 refresh*/
                tapp_on_reload();
            }

            break;
        default:
            break;
    }

    return;
}

static void WINAPI TappServiceMain(DWORD argc, LPTSTR *argv)
{
    int iRet = 0;
    DWORD specificError = 0;
    TCHAR szError[256];
    DWORD dwNumServiceArgs;
    LPCTSTR* lpServiceArgVectors = NULL;
    int i = 0;

    if (NULL == g_stTappService.pfnServiceInit)
    {
        SvcDebugOut("[%s] pfnServiceInit is null ", g_stTappService.pszServiceName);
        _stprintf(szError, TEXT("[%s] pfnServiceInit is null"), g_stTappService.pszServiceName);
        AddToMessageLog(szError, EVENTLOG_ERROR_TYPE, 0, NULL);
        return;
    }
    if (NULL == g_stTappService.pstCurCtx)
    {
        SvcDebugOut("[%s] pstCurCtx is null ", g_stTappService.pszServiceName);
        _stprintf(szError, TEXT("[%s] pstCurCtx is null"), g_stTappService.pszServiceName);
        AddToMessageLog(szError, EVENTLOG_ERROR_TYPE, 0, NULL);
        return;
    }

    /*修改参数列表，通过控制面板 服务启动时 只有一个参数*/
    /*此时添加一个start，以符合tapp规范*/
    dwNumServiceArgs = argc + 1;
    lpServiceArgVectors = malloc(dwNumServiceArgs * sizeof(LPCTSTR));
    if (NULL == lpServiceArgVectors)
    {
        SvcDebugOut("[%s] failed to malloc memory for arg ", g_stTappService.pszServiceName);
        _stprintf(szError, TEXT("[%s] failed to malloc memory for arg "), g_stTappService.pszServiceName);
        AddToMessageLog(szError, EVENTLOG_ERROR_TYPE, 0, NULL);
        return;
    }
    for (i = 0; i < argc; i++)
    {
        lpServiceArgVectors[i] = argv[i];
    }
    lpServiceArgVectors[argc] = "start";

    if (NULL != gs_fpTappSvcLog)
    {
        int i = 0;

        fprintf(gs_fpTappSvcLog, "[%s] SERVICE_MAIN argc:%d\n",
                g_stTappService.pszServiceName, dwNumServiceArgs);
        for (i = 0; i < dwNumServiceArgs; i++)
        {
            fprintf(gs_fpTappSvcLog, "[%s] SERVICE_MAIN argv[%d]:%s\n",
                    g_stTappService.pszServiceName, i, lpServiceArgVectors[i]);
        }
        fflush(gs_fpTappSvcLog);
    }

    /*设置进程控制器*/
    gs_ServiceStatus.dwServiceType        = SERVICE_WIN32;
    gs_ServiceStatus.dwCurrentState       = SERVICE_START_PENDING;
    gs_ServiceStatus.dwControlsAccepted
        = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_PAUSE_CONTINUE;
    gs_ServiceStatus.dwWin32ExitCode      = 0;
    gs_ServiceStatus.dwServiceSpecificExitCode = 0;
    gs_ServiceStatus.dwCheckPoint         = 0;
    gs_ServiceStatus.dwWaitHint           = 0;
    gs_ServiceStatusHandle = RegisterServiceCtrlHandler(g_stTappService.pszServiceName,
                                                       TappServiceCtrlHandler);
    if ((SERVICE_STATUS_HANDLE)0 == gs_ServiceStatusHandle)
    {
        SvcDebugOut("[%s] RegisterServiceCtrlHandler failed: %s",
                    g_stTappService.pszServiceName, GetLastErrorString());
        _stprintf(szError, TEXT("[%s] RegisterServiceCtrlHandler failed, error code<%d>"),
                  g_stTappService.pszServiceName, GetLastError());
        AddToMessageLog(szError, EVENTLOG_ERROR_TYPE, 0, NULL);

        iRet = -1;
    }

    /*服务初始化*/
    if (0 == iRet)
    {
        iRet = g_stTappService.pfnServiceInit(g_stTappService.pstCurCtx, g_stTappService.pstTappEnv,
                                              dwNumServiceArgs, lpServiceArgVectors);
        if (0 != iRet)
        {
            SvcDebugOut("[%s] call pfnServiceInit failed, iRet:%d ", g_stTappService.pszServiceName,iRet);
            _stprintf(szError, TEXT("[%s] call pfnServiceInit failed, iRet:%d "),
                      g_stTappService.pszServiceName, iRet);
            AddToMessageLog(szError, EVENTLOG_ERROR_TYPE, 0, NULL);
        } else
        {
            iRet = tapp_def_init(g_stTappService.pstCurCtx, g_stTappService.pstTappEnv);
            if (0 != iRet)
            {
                SvcDebugOut("[%s] call tapp_def_init failed, iRet:%d ", g_stTappService.pszServiceName,iRet);
                _stprintf(szError, TEXT("[%s] call tapp_def_init failed, iRet:%d "),
                          g_stTappService.pszServiceName, iRet);
                AddToMessageLog(szError, EVENTLOG_ERROR_TYPE, 0, NULL);
            }
        }
    }/*if (0 == iRet)*/

    /* 服务启动*/
    if (0 == iRet)
    {
        ReportSvcStatus(SERVICE_RUNNING, 0, 0);

        tapp_def_mainloop(g_stTappService.pstCurCtx, g_stTappService.pstTappEnv);

        tapp_def_fini(g_stTappService.pstCurCtx, g_stTappService.pstTappEnv);
    }

    if (NULL != lpServiceArgVectors)
    {
        free(lpServiceArgVectors);
        lpServiceArgVectors = NULL;
    }

    ReportSvcStatus(SERVICE_STOPPED, iRet, 0);
}

static int tapp_init_service_no_arg(int argc, char* argv[])
{
    int iRet = 0;

    iRet = g_stTappService.pfnServiceInit(g_stTappService.pstCurCtx, g_stTappService.pstTappEnv, argc, argv);
    if (0 != iRet)
    {
        if (NULL != gs_fpTappSvcLog)
        {
            fprintf(gs_fpTappSvcLog, "[%s] call pfnServiceInit failed, return: %d\n",
                    g_stTappService.pszServiceName, iRet);
            fflush(gs_fpTappSvcLog);
        }
    } else
    {
        if (NULL != gs_fpTappSvcLog)
        {
            fprintf(gs_fpTappSvcLog, "[%s] will call tapp_def_init\n", g_stTappService.pszServiceName);
            fflush(gs_fpTappSvcLog);
        }
        iRet = tapp_def_init(g_stTappService.pstCurCtx, g_stTappService.pstTappEnv);
        if (0 != iRet)
        {
            if (NULL != gs_fpTappSvcLog)
            {
                fprintf(gs_fpTappSvcLog, "[%s] call tapp_def_init failed, return: %d\n",
                        g_stTappService.pszServiceName, iRet);
                fflush(gs_fpTappSvcLog);
            }
        }
    }

    if (0 != iRet)
    {
        ReportSvcStatus(SERVICE_STOPPED, 0, 0);
    }

    return iRet;
}

static void WINAPI TappServiceMainNoArg(DWORD argc, LPTSTR *argv)
{
    int iRet = 0;
    DWORD specificError = 0;
    TCHAR szError[256];
    int i = 0;

    if (NULL == g_stTappService.pfnServiceInit)
    {
        SvcDebugOut("[%s] pfnServiceInit is null ", g_stTappService.pszServiceName);
        _stprintf(szError, TEXT("[%s] pfnServiceInit is null"), g_stTappService.pszServiceName);
        AddToMessageLog(szError, EVENTLOG_ERROR_TYPE, 0, NULL);
        return;
    }
    if (NULL == g_stTappService.pstCurCtx)
    {
        SvcDebugOut("[%s] pstCurCtx is null ", g_stTappService.pszServiceName);
        _stprintf(szError, TEXT("[%s] pstCurCtx is null"), g_stTappService.pszServiceName);
        AddToMessageLog(szError, EVENTLOG_ERROR_TYPE, 0, NULL);
        return;
    }

    /*设置进程控制器*/
    gs_ServiceStatus.dwServiceType        = SERVICE_WIN32;
    gs_ServiceStatus.dwCurrentState       = SERVICE_START_PENDING;
    gs_ServiceStatus.dwControlsAccepted
        = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_PAUSE_CONTINUE;
    gs_ServiceStatus.dwWin32ExitCode      = 0;
    gs_ServiceStatus.dwServiceSpecificExitCode = 0;
    gs_ServiceStatus.dwCheckPoint         = 0;
    gs_ServiceStatus.dwWaitHint           = 0;
    gs_ServiceStatusHandle = RegisterServiceCtrlHandler(g_stTappService.pszServiceName,
                                                       TappServiceCtrlHandler);
    if ((SERVICE_STATUS_HANDLE)0 == gs_ServiceStatusHandle)
    {
        if (NULL != gs_fpTappSvcLog)
        {
            fprintf(gs_fpTappSvcLog, "[%s] RegisterServiceCtrlHandler failed, for %s\n",
                    g_stTappService.pszServiceName, GetLastErrorString());
            fflush(gs_fpTappSvcLog);
        }

        iRet = -1;
    }

    /* 服务启动*/
    if (0 == iRet)
    {
        ReportSvcStatus(SERVICE_RUNNING, 0, 0);

        tapp_def_mainloop(g_stTappService.pstCurCtx, g_stTappService.pstTappEnv);

        tapp_def_fini(g_stTappService.pstCurCtx, g_stTappService.pstTappEnv);
    }

    ReportSvcStatus(SERVICE_STOPPED, iRet, 0);
}

int tapp_service_init(LPTAPPCTX a_pstCtx, void*a_pstEnv, int argc, char* argv[])
{
    int opt = 0;
    int iRet = 0;
    int iOptIdx = 0;
    int iRunAsService = 0;

    if (1 < argc)
    {
        int i;
        for (i = 1; i < argc; i++)
        {
            if (!strcmp(argv[i], "--run-as-service"))
            {
                iRunAsService = 1;
                break;
            }
        }
    }

    a_pstCtx->argc = argc;
    a_pstCtx->argv = argv;
    tapp_set_default_service_name(a_pstCtx);
    iRet = tapp_set_windows_working_path();
    if (0 > iRet)
    {
        return -1;
    }

    /* (1 == argc) 或者 (1 < argc && iRunAsService)
     * 表明些程序是由 windows 服务管理器启动的，
     * 也就是以服务模式启动的，
     * 编程模型与非服务模式启动不同，
     * 需要先注册服务处理分派函数，
     * 真正的处理逻辑(tapp_def_mainloop)都在分派函数中进行，
     */
    if (1 == argc)
    {
        SERVICE_TABLE_ENTRY DispatchTable[] =   {
            {TEXT(g_stTappService.pszServiceName), (LPSERVICE_MAIN_FUNCTION)TappServiceMain},
            {NULL, NULL},
        };

        tapp_svc_log_init();

        if (NULL != gs_fpTappSvcLog)
        {
            fprintf(gs_fpTappSvcLog, "[%s] --run-as-service NOT specified, only command-line-parameters to SERVICE_MAIN is processed\n",
                    g_stTappService.pszServiceName);
            fflush(gs_fpTappSvcLog);
        }

        iRet = StartServiceCtrlDispatcher(DispatchTable);
        if (0 == iRet)
        {
            if (NULL != gs_fpTappSvcLog)
            {
                fprintf(gs_fpTappSvcLog, "[%s] StartServiceCtrlDispatcher failed, error code<%d>",
                        g_stTappService.pszServiceName, GetLastError());
                fflush(gs_fpTappSvcLog);
            }

            iRet = -1;
        } else
        {
            if (NULL != gs_fpTappSvcLog)
            {
                fprintf(gs_fpTappSvcLog, "[%s] StartServiceCtrlDispatcher success.\n",
                        g_stTappService.pszServiceName);
                fflush(gs_fpTappSvcLog);
            }
        }

        tapp_svc_log_fini();
        exit(iRet);
    } else if (1 < argc && iRunAsService)
    {
        tapp_svc_log_init();
        if (NULL != gs_fpTappSvcLog)
        {
            fprintf(gs_fpTappSvcLog, "[%s] --run-as-service specified, only command-line-parameters to main is processed\n",
                    g_stTappService.pszServiceName);
            fflush(gs_fpTappSvcLog);
        }

        if (NULL != gs_fpTappSvcLog)
        {
            int i = 0;

            fprintf(gs_fpTappSvcLog, "[%s] main argc:%d\n", g_stTappService.pszServiceName, argc);
            for (i = 0; i < argc; i++)
            {
                fprintf(gs_fpTappSvcLog, "[%s] main argv[%d]:%s\n", g_stTappService.pszServiceName, i, argv[i]);
            }
            fflush(gs_fpTappSvcLog);
        }

        if (NULL != gs_fpTappSvcLog)
        {
            fprintf(gs_fpTappSvcLog, "[%s] call tapp_init_service_no_arg\n", g_stTappService.pszServiceName);
            fflush(gs_fpTappSvcLog);
        }

        iRet = tapp_init_service_no_arg(argc, argv);
        if (NULL != gs_fpTappSvcLog)
        {
            fprintf(gs_fpTappSvcLog, "[%s] tapp_init_service_no_arg return %d\n",
                    g_stTappService.pszServiceName, iRet);
            fflush(gs_fpTappSvcLog);
        }

        if (0 == iRet)
        {
            SERVICE_TABLE_ENTRY DispatchTable[] =   {
                {TEXT(g_stTappService.pszServiceName), (LPSERVICE_MAIN_FUNCTION)TappServiceMainNoArg},
                {NULL, NULL},
            };

            iRet = StartServiceCtrlDispatcher(DispatchTable);
            if (0 == iRet)
            {
                if (NULL != gs_fpTappSvcLog)
                {
                    fprintf(gs_fpTappSvcLog, "[%s] StartServiceCtrlDispatcher failed, error code<%d>",
                            g_stTappService.pszServiceName, GetLastError());
                    fflush(gs_fpTappSvcLog);
                }

                iRet = -1;
            } else
            {
                if (NULL != gs_fpTappSvcLog)
                {
                    fprintf(gs_fpTappSvcLog, "[%s] StartServiceCtrlDispatcher success.\n",
                            g_stTappService.pszServiceName);
                    fflush(gs_fpTappSvcLog);
                }
            }
        }

        tapp_svc_log_fini();
        exit(iRet);
    }

    assert(NULL != g_stTappService.pfnServiceInit);

    /* 程序运行到这里，说明是以非 windows 服务模式运行 */

    /*分析命令行，通过参数控制服务 安装、卸载、启动、停止*/
    iRet = g_stTappService.pfnServiceInit(a_pstCtx, a_pstEnv, argc, argv);
    if (0 != iRet)
    {
        fprintf(stderr, "ERROR: application init-function(registered by tapp_register_service_init) failed\n");
        return -1;
    }

    tapp_def_opt(a_pstCtx, a_pstEnv);

    /* (0 == gs_stCtxInternal.iIsDaemon)
     * 表明程序是以 windows 服务控制器的方式在运行，
     * 所以本质上，daemon 在 linux/windows 平台下的处理完全不同，
     */

    switch(gs_stCtxInternal.eOpCmd)
    {
        case TAPP_OPERATE_CMD_INSTALL:
            {
                /* install windows service */
                iRet = tapp_install_windows_service(a_pstCtx, a_pstEnv);
                exit(iRet);
            }
            break;
        case TAPP_OPERATE_CMD_UNINSTALL:
            {
                /* install windows service */
                iRet = tapp_uninstall_service(a_pstCtx, a_pstEnv);
                exit(iRet);
            }
            break;
        case TAPP_OPERATE_CMD_START:
            {
                if (0 == gs_stCtxInternal.iIsDaemon)
                {
                    //以非服务模式启动
                    iRet = tapp_service_init_non_service(a_pstCtx, a_pstEnv);
                }else
                {
                    /* control windows service */
                    iRet = tapp_control_service_start(argc, argv);
                    exit(0);
                }
            }
            break;
        case TAPP_OPERATE_CMD_KILL:
        case TAPP_OPERATE_CMD_STOP:
            {
                if (0 == gs_stCtxInternal.iIsDaemon)
                {
                    //以非服务模式启动
                    iRet = tapp_service_init_non_service(a_pstCtx, a_pstEnv);
                }else
                {
                    /* control windows service */
                    iRet = tapp_control_service_stop();
                    exit(0);
                }
            }
            break;
        case TAPP_OPERATE_CMD_RESTART:
            {
                if (0 == gs_stCtxInternal.iIsDaemon)
                {
                    //以非服务模式启动
                    iRet = tapp_service_init_non_service(a_pstCtx, a_pstEnv);
                }else
                {
                    /* control windows service */
                    iRet = tapp_control_service_stop();
                    if (0 == iRet)
                    {
                        iRet = tapp_control_service_start(argc, argv);
                    }else
                    {
                        printf("failed to stop service %s, so cannot restart\n",
                               g_stTappService.pszServiceName);
                    }
                    exit(0);
                }/*if (0 == gs_stCtxInternal.iIsDaemon)*/
            }
            break;

        case TAPP_OPERATE_CMD_RELOAD:
            {
                if (0 == gs_stCtxInternal.iIsDaemon)
                {
                    //以非服务模式启动
                    iRet = tapp_service_init_non_service(a_pstCtx, a_pstEnv);
                }else
                {
                    /* control windows service */
                    iRet = tapp_control_service_refresh();
                    exit(0);
                }
            }
            break;
        case TAPP_OPERATE_CMD_CONTROL:
            //控制模式只能以 非服务模式启动
            iRet = tapp_service_init_non_service(a_pstCtx, a_pstEnv);
            break;

        default:
            printf("Error: tapp-windows-service only support %s | %s |%s |%s |%s\n",
                   TAPP_START, TAPP_STOP, TAPP_RESTART, TAPP_INSTALL, TAPP_UNINSTALL);
            exit(0);
    }


    return iRet;
}
#else
int tapp_service_init(LPTAPPCTX a_pstCtx, void*a_pstEnv, int argc, char* argv[])
{
    int iRet ;

    iRet = g_stTappService.pfnServiceInit(a_pstCtx, a_pstEnv, argc, argv);
    if (0 != iRet)
    {
        fprintf(stderr, "ERROR: application init-function(registered by tapp_register_service_init) failed\n");
        return -1;
    }

    iRet = tapp_service_init_non_service(a_pstCtx, a_pstEnv);
    return iRet;
}
#endif

TSF4G_API int tapp_run_service(LPTAPPCTX a_pstCtx, void*a_pstEnv, int argc, char* argv[])
{
    int iRet = 0;

    if (NULL == a_pstCtx)
    {
        printf("the param a_pstCtx should not be null\n");
        return -1;
    }

    if (0 >= argc)
    {
        fprintf(stderr, "parameter error, argc should be greater that 0\n");
        return -1;
    }

    if (NULL == argv)
    {
        fprintf(stderr, "parameter error, argv is NULL\n");
        return -1;
    }

    if (NULL == g_stTappService.pfnServiceInit)
    {
        fprintf(stderr, "Error: must call tapp_register_service_init "
                "to register ServiceInit before calling tapp_run_service\n");
        return -1;
    }

    g_stTappService.pstCurCtx = a_pstCtx;
    g_stTappService.pstTappEnv = a_pstEnv;
    iRet = tapp_service_init(a_pstCtx, a_pstEnv, argc, argv);
    if (0 != iRet)
    {
        printf("tapp_service_init failed, so cannot start %s ,iRet:%d\n", argv[0],iRet);
        return -1;
    }

    tapp_def_mainloop(a_pstCtx, a_pstEnv);

    tapp_def_fini(a_pstCtx, a_pstEnv);

    return iRet;
}


void tapp_set_default_service_name(TAPPCTX* a_pstCtx)
{
    const char *pszBaseName = NULL;
    const char *pszSuffix = NULL;
    size_t tCopyLen = 0;

    pszBaseName = basename(a_pstCtx->argv[0]);
    pszSuffix = strrchr(a_pstCtx->argv[0], '.');

    assert(NULL != pszBaseName);

    /* drip off .exe from filename */
    if (NULL == pszSuffix)
    {
        tCopyLen = sizeof(g_stTappService.szServiceName);
    }else  if (sizeof(g_stTappService.szServiceName) < (pszSuffix - pszBaseName))
    {
        tCopyLen = sizeof(g_stTappService.szServiceName);
    }
    else
    {
        tCopyLen = pszSuffix - pszBaseName;
    }

    strncpy(g_stTappService.szServiceName, pszBaseName, tCopyLen);
    g_stTappService.szServiceName[sizeof(g_stTappService.szServiceName)-1] = '\0';
    g_stTappService.pszServiceName = g_stTappService.szServiceName;

    if (NULL == g_stTappService.pszServiceDisplayName)
    {
        tCopyLen = sizeof(g_stTappService.szServiceDisplayName);
        strncpy(g_stTappService.szServiceDisplayName, g_stTappService.szServiceName, tCopyLen);
        g_stTappService.szServiceDisplayName[sizeof(g_stTappService.szServiceDisplayName)-1] = '\0';
        g_stTappService.pszServiceDisplayName = g_stTappService.szServiceDisplayName;
    }
}
