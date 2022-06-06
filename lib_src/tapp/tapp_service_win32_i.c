#if defined (_WIN32) || defined (_WIN64)

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <tchar.h>
#include "pal/tos.h"
#include "tapp_service_win32_i.h"


#define THIRTY_SECONDS (30 * 1000)
#define SCM_NOTIFICATION_INTERVAL 5000

#ifndef DIRSEP
#if !defined (_WIN32) && !defined (_WIN64)
#define DIRSEP '/'
#else
#define DIRSEP '\\'
#endif
#endif


SERVICE_STATUS ssStatus;
SERVICE_STATUS_HANDLE sshStatusHandle;
HANDLE hStopServerEvent = NULL;
HANDLE hServerStartedEvent = NULL;
HANDLE hServerStoppedEvent = NULL;
HANDLE hServerstartThread = NULL;
HANDLE hServerStopThread = NULL;



VOID  ServiceCtrl(DWORD dwCtrlCode);

static VOID  StartStatusRoutine(void);

static VOID   StopStatusRoutine(void);

/** if service is uninstalled, return nonzero, otherwise return zero */
DWORD  svc_is_uninstalled (LPTSTR lpszServiceName, LPTSTR lpszBinaryPathName);

/** if service is non running, return nonzero, otherwise return zero */
DWORD  svc_is_unrunning (LPTSTR lpszServiceName);

BOOL ReportStatusToSCMgr(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint);

BOOL  ControlHandler (DWORD dwCtrlType);

LPTSTR GetLastErrorString(void);

int CmdChangeServiceConfig(DWORD dwStartType, LPTSTR lpszServiceName,
                           LPTSTR lpszDisplayName, LPTSTR  lpszBinaryPathName, LPTSTR err);



int CmdInstallService(DWORD dwStartType, const LPTSTR lpszServiceName,
                      const LPSTR lpszServiceDesc, const LPTSTR lpszDisplayName,
                      const LPTSTR lpszBinaryPathName, LPTSTR err)
{
    HKEY hKey;
    DWORD dwValue, dwDisposition;
    SC_HANDLE schSCManager, schService;
    LONG iRet = 0;

    /* check parameters */
    /* lpszServiceDesc can be NULL */
    if ((NULL == lpszServiceName) ||
        (NULL == lpszDisplayName) ||
        (NULL == lpszBinaryPathName) ||
        (NULL == err))
    {
        return 1;
    }


    if (!svc_is_uninstalled(lpszServiceName, lpszBinaryPathName))
    {
        sprintf(err, "服务已经安装!");
        return 1;
    }

    switch (dwStartType)
    {
        case SERVICE_AUTO_START:
        case SERVICE_DEMAND_START:
        case SERVICE_DISABLED:
        case SERVICE_SYSTEM_START:
        case SERVICE_BOOT_START:
            break;
        default:
            strcpy(err, "服务启动模式参数不正确");
            return 1;
    }
    schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT|SC_MANAGER_CREATE_SERVICE);
    if (NULL != schSCManager)
    {
        schService = CreateService(schSCManager,
                                   lpszServiceName,
                                   lpszDisplayName,
                                   SERVICE_ALL_ACCESS,
                                   SERVICE_WIN32_OWN_PROCESS,
                                   dwStartType,
                                   SERVICE_ERROR_NORMAL,
                                   lpszBinaryPathName,
                                   NULL, NULL, NULL, NULL, NULL);
        if (NULL != schService)
        {
            /* create_service 成功 */
            char regpath[132];

            if (NULL != lpszServiceDesc)
            {
                SERVICE_DESCRIPTION serviceDesc;
                serviceDesc.lpDescription = lpszServiceDesc;

                iRet = ChangeServiceConfig2(schService,
                                            SERVICE_CONFIG_DESCRIPTION,
                                            (LPVOID)&serviceDesc);
                if (0 == iRet)
                {
                    return 1;
                }
            }

            CloseServiceHandle(schService);
            CloseServiceHandle(schSCManager);

            _snprintf(regpath, sizeof regpath,
                      "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\%s",
                      lpszServiceName);

            /* Create the registry key for event logging to the Windows NT event log*/
            iRet = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                                  regpath,                  /* subkey */
                                  0,                        /* 保留，必须为零 */
                                  "REG_SZ",                 /* 健类型 */
                                  REG_OPTION_NON_VOLATILE,  /* 选项 */
                                  KEY_ALL_ACCESS,           /* 访问掩码*/
                                  NULL,                     /* 安全描述 */
                                  &hKey,                    /* 健句柄 */
                                  /* 描述REG_CREATED_NEW_KEY，REG_OPENED_EXISTING_KEY */
                                  &dwDisposition
                                 );
            if (ERROR_SUCCESS != iRet)
            {
                RegCloseKey(hKey);
                return 0;
            }

            iRet = RegSetValueEx(hKey, "EventMessageFile", 0, REG_EXPAND_SZ,
                                 (const BYTE*)lpszBinaryPathName,
                                 strlen(lpszBinaryPathName) + 1);
            if (ERROR_SUCCESS != iRet)
            {
                RegCloseKey(hKey);
                return 0;
            }

            dwValue = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;
            iRet = RegSetValueEx(hKey, "TypesSupported", 0, REG_DWORD,
                                 (LPBYTE) &dwValue, sizeof(DWORD));
            if (ERROR_SUCCESS != iRet)
            {
                RegCloseKey(hKey);
                return 0;
            }

            RegCloseKey(hKey);
            return 0;
        } else
        {
            sprintf(err, "CreateService failed. Error=%lu :%s\n",
                    GetLastError(), GetLastErrorString());
            CloseServiceHandle(schSCManager);
            return 1;
        }
    }/*if ((schSCManager = OpenSCManager*/
    else
    {
        sprintf(err, "OpenSCManager failed. Error=%lu :%s\n",
                GetLastError(), GetLastErrorString());
    }

    return 1;
}

int CmdRemoveService(LPTSTR lpszServiceName, LPTSTR err)
{
    TCHAR regPath[132];
    LONG iRet = 0;
    SC_HANDLE schService;
    SC_HANDLE schSCManager;

    if ((NULL == lpszServiceName) || (NULL == err))
    {
        return 0xff;
    }

    if (!svc_is_unrunning(lpszServiceName))
    {
        sprintf(err, "服务正在运行，如果您确定要卸载，请先停止服务！");
        iRet = CmdStopService(lpszServiceName, err);
        if (0 != iRet)
        {
            return 1;
        }
    }

    schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT | SC_MANAGER_CREATE_SERVICE);
    if (NULL != schSCManager)
    {
        schService = OpenService(schSCManager, lpszServiceName, DELETE);
        if (NULL != schService)
        {
            /* now remove the service*/
            if (DeleteService(schService) == TRUE)
            {
                CloseServiceHandle(schService);
                CloseServiceHandle(schSCManager);

                _snprintf(regPath, sizeof regPath,
                          "System\\CurrentControlSet\\Services\\EventLog\\Application\\%s",
                          lpszServiceName);

                iRet = RegDeleteKey(HKEY_LOCAL_MACHINE, regPath);
                if (ERROR_SUCCESS != iRet)
                {
                    NULL; /* why NULL ? */
                }

                return 0;
            }
            else
            {
                sprintf(err, "DeleteService failed. Error=%lu :%s\n",
                        GetLastError(), GetLastErrorString());
                CloseServiceHandle(schService);
                CloseServiceHandle(schSCManager);
                return 1;
            }
        }
        else
        {
            sprintf(err, "OpenService failed. Error=%lu :%s\n",
                    GetLastError(), GetLastErrorString());
            CloseServiceHandle(schSCManager);
            return 1;
        }
    }
    else
    {
        sprintf(err, "OpenSCManager failed. GetLastError=%lu :%s\n",
                GetLastError(), GetLastErrorString());
    }

    return 1;
}

int CmdStartService(LPTSTR lpszServiceName, LPTSTR lpszDisplayName,
                    DWORD dwNumServiceArgs, LPCTSTR* lpServiceArgVectors, LPTSTR err)
{
    int iRet = 0;
    SC_HANDLE  schService, schSCManager;


    if ((NULL == lpszServiceName) ||
        (NULL == lpszDisplayName) ||
        (NULL == err))
    {
        return -1;
    }

    if (!svc_is_unrunning(lpszServiceName))
    {
        sprintf(err, "服务已经启动!!!");

        return -2;
    }

    schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (NULL == schSCManager)
    {
        sprintf(err, "OpenSCManager failed. Error=%lu :%s\n",
                GetLastError(), GetLastErrorString());
        return -3;
    }

    schService = OpenService(schSCManager, lpszServiceName, SERVICE_ALL_ACCESS);
    if (NULL == schService)
    {
        sprintf(err, "Open %s Service failed, Error:%s\n ", lpszServiceName,
                GetLastErrorString());
        CloseServiceHandle(schSCManager);
        return -4;
    }


    if (StartService(schService, dwNumServiceArgs, lpServiceArgVectors) == 0)
    {
        sprintf(err, "Start Service failed .Error=%lu :%s\n",
                GetLastError(), GetLastErrorString());
        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);

        return -5;
    }

    //检测是否启动
    _tprintf("Starting ....%s", lpszServiceName);
    Sleep(1000);
    while (QueryServiceStatus(schService, &ssStatus))
    {
        if (ssStatus.dwCurrentState == SERVICE_START_PENDING)
        {
            printf(".");
            Sleep(1000);
        } else
        {
            break;
        }
    }
    printf("\n");

    if (SERVICE_RUNNING != ssStatus.dwCurrentState)
    {
        sprintf(err, "StartService failed. Error=%lu :%s\n",
                GetLastError(), GetLastErrorString());
        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
        return -6;
    }

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
    return 0;
}


int CmdStopService(LPTSTR lpszServiceName, LPTSTR err)
{
    SC_HANDLE schService;
    SC_HANDLE schSCManager;
    SERVICE_STATUS ssStatus;
    schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);



    if (NULL != schSCManager)
    {
        schService = OpenService(schSCManager, lpszServiceName, SERVICE_ALL_ACCESS);
        if (NULL != schService)
        {
            if (ControlService(schService, SERVICE_CONTROL_STOP, &ssStatus))
            {
                _tprintf("Stopping ....%s", lpszServiceName);
                Sleep(1000);
                while (QueryServiceStatus(schService, &ssStatus))
                {
                    if (ssStatus.dwCurrentState == SERVICE_STOP_PENDING)
                    {
                        printf(".");
                        Sleep(1000);
                    } else
                    {
                        break;
                    }
                }
                printf("\n");

                if (SERVICE_STOPPED != ssStatus.dwCurrentState)
                {
                    sprintf(err, "StopService failed. Error=%lu :%s\n",
                            GetLastError(), GetLastErrorString());
                    CloseServiceHandle(schService);
                    CloseServiceHandle(schSCManager);
                    return 1;
                }


                CloseServiceHandle(schService);
                CloseServiceHandle(schSCManager);
                return 0;
            }
            if (ERROR_SERVICE_NOT_ACTIVE == GetLastError())
            {
                _tprintf("Info: service '%s' is not running\n", lpszServiceName);
                CloseServiceHandle(schService);
                CloseServiceHandle(schSCManager);
                return 0;
            } else
            {
                sprintf(err, "StopService failed. Error=%lu :%s\n",
                        GetLastError(), GetLastErrorString());
                CloseServiceHandle(schService);
                CloseServiceHandle(schSCManager);
                return 1;
            }
        }
        else
        {
            sprintf(err, "OpenService  failed. Error=%lu :%s\n",
                    GetLastError(), GetLastErrorString());
            CloseServiceHandle(schSCManager);
            return 1;
        }
    }
    else
    {
        sprintf(err, "OpenSCManager failed. Error=%lu :%s\n",
                GetLastError(), GetLastErrorString());
    }

    return 1;
}


static char *GetErrorString(int err)
{
    static char msgBuf[1024];

    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, err,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  msgBuf, 1024, NULL);

    return msgBuf;
}

LPTSTR GetLastErrorString(void)
{
    return GetErrorString(GetLastError());
}



LPTSTR GetLastErrorText(LPTSTR lpszBuf, DWORD dwSize)
{
    DWORD dwRet;
    LPTSTR lpszTemp = NULL;

    dwRet = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                          FORMAT_MESSAGE_FROM_SYSTEM |
                          FORMAT_MESSAGE_ARGUMENT_ARRAY,
                          NULL, GetLastError(), LANG_NEUTRAL,
                          (LPTSTR)&lpszTemp, 0, NULL);
    /* supplied buffer is not long enough*/
    if (!dwRet || (((long)dwRet + 14) > (long)dwSize))
    {
        lpszBuf[0] = TEXT('\0');
    }
    else
    {
        lpszTemp[lstrlen(lpszTemp)-2] = TEXT('\0');  //remove cr and newline character
        _stprintf(lpszBuf, TEXT("%s (0x%x)"), lpszTemp, GetLastError());
    }

    if (lpszTemp)
    {
        LocalFree((HLOCAL)lpszTemp);
    }

    return lpszBuf;
}


void AddToMessageLog(LPTSTR lpszMsg, WORD dwType, DWORD dwEventID, LPTSTR lpszServiceName)
{

    HANDLE  hEventSource;
    LPTSTR  lpszStrings[2];
    DWORD   dwErr = 0;

    if ((NULL == lpszServiceName) || (NULL == lpszMsg))
        return ;



    hEventSource = RegisterEventSource(NULL, lpszServiceName);
    lpszStrings[0] = lpszMsg;

    if (NULL != hEventSource)
    {
        dwErr = ReportEvent(hEventSource, // handle of event source
                            dwType,  // event type
                            0,                    // event category
                            dwEventID,                    // event ID
                            NULL,                 // current user's SID
                            1,                    // strings in lpszStrings
                            0,                    // no bytes of raw data
                            lpszStrings,          // array of error strings
                            NULL);                // no raw data
        (VOID)DeregisterEventSource(hEventSource);
    }
}

VOID ServiceCtrl(DWORD dwCtrlCode)
{
    switch(dwCtrlCode)
    {
        /* Stop the service.*/

        case SERVICE_CONTROL_STOP:
        case SERVICE_CONTROL_SHUTDOWN:
            ssStatus.dwCurrentState = SERVICE_STOP_PENDING;
            ssStatus.dwCheckPoint++;
            ssStatus.dwWaitHint = SCM_NOTIFICATION_INTERVAL * 2;
            SetServiceStatus(sshStatusHandle, &ssStatus);

            hServerStoppedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
            if (NULL == hServerStoppedEvent)
            {
                ssStatus.dwCheckPoint++;
                ssStatus.dwWaitHint = THIRTY_SECONDS;
                SetServiceStatus(sshStatusHandle, &ssStatus);
            }
            else
            {
                hServerStopThread = CreateThread(NULL, 0,
                                                 (LPTHREAD_START_ROUTINE)&StopStatusRoutine,
                                                 NULL, 0, NULL);
                if (NULL == hServerStopThread)
                {
                    ssStatus.dwCheckPoint++;
                    ssStatus.dwWaitHint = THIRTY_SECONDS;
                    SetServiceStatus(sshStatusHandle, &ssStatus);
                }
            }

            /*通知servie 准备关闭服务*/

            if (NULL != hStopServerEvent)
            {
                SetEvent(hStopServerEvent);
            }
            break;

            /*Update the service status.*/
        case SERVICE_CONTROL_INTERROGATE:
            break;

            /* invalid control code*/
        default:
            break;
    }

    ReportStatusToSCMgr(ssStatus.dwCurrentState, NO_ERROR, 0);
}

BOOL ReportStatusToSCMgr(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint)
{
    static DWORD dwCheckPoint = 1;
    BOOL fResult = TRUE;

    if (SERVICE_START_PENDING == dwCurrentState)
    {
        ssStatus.dwControlsAccepted = 0;
    }
    else
    {
        ssStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    }

    ssStatus.dwCurrentState = dwCurrentState;
    ssStatus.dwWin32ExitCode = dwWin32ExitCode;
    ssStatus.dwWaitHint = dwWaitHint;

    if ((SERVICE_RUNNING == dwCurrentState) ||
        (SERVICE_STOPPED == dwCurrentState))
    {
        ssStatus.dwCheckPoint = 0;
    }
    else
    {
        ssStatus.dwCheckPoint = dwCheckPoint++;
    }


    /* Report the status of the service to the service control manager.*/
    fResult = SetServiceStatus(sshStatusHandle, &ssStatus);
    if (!fResult) {
        AddToMessageLog(TEXT("SetServiceStatus"), EVENTLOG_ERROR_TYPE, 0, NULL);
    }

    return fResult;
}

static VOID  StartStatusRoutine(void)
{
    DWORD wait_result;
    int done = 0;

    while (!done)
    {
        wait_result = WaitForSingleObject(hServerStartedEvent, SCM_NOTIFICATION_INTERVAL);
        switch (wait_result)
        {
            case WAIT_ABANDONED:/*???????How to do*/
            case WAIT_OBJECT_0:
                CloseHandle(hServerStartedEvent);
                done = 1;
                break;
            case WAIT_TIMEOUT:
                ssStatus.dwCheckPoint++;
                ssStatus.dwWaitHint = SCM_NOTIFICATION_INTERVAL * 2;
                SetServiceStatus(sshStatusHandle, &ssStatus);
                break;
            case WAIT_FAILED:
                /* theres been some problem with WaitForSingleObject so tell the Service
                 * Control Manager to wait 30 seconds before deploying its assasin and
                 * then leave the thread.
                 */
                ssStatus.dwCheckPoint++;
                ssStatus.dwWaitHint = THIRTY_SECONDS;
                SetServiceStatus(sshStatusHandle, &ssStatus);
                done = 1;
                break;
        }
    }
    ExitThread(0);
}

static VOID StopStatusRoutine(void)
{
    DWORD wait_result;
    int done = 0;

    while (!done)
    {
        wait_result = WaitForSingleObject(hServerStoppedEvent, SCM_NOTIFICATION_INTERVAL);
        switch (wait_result)
        {
            case WAIT_ABANDONED:/*?????How to do*/
            case WAIT_OBJECT_0:
                CloseHandle(hServerStoppedEvent);
                done = 1;
                break;
            case WAIT_TIMEOUT:
                ssStatus.dwCheckPoint++;
                ssStatus.dwWaitHint = SCM_NOTIFICATION_INTERVAL * 2;
                SetServiceStatus(sshStatusHandle, &ssStatus);
                break;
            case WAIT_FAILED:
                ssStatus.dwCheckPoint++;
                ssStatus.dwWaitHint = THIRTY_SECONDS;
                SetServiceStatus(sshStatusHandle, &ssStatus);
                done = 1;
                break;
        }
    }
    ExitThread(0);
}

/* 只要服务名相同就认为已经安装相应的服务，不考虑可执行文件路径信息*/
DWORD svc_is_uninstalled(LPTSTR lpszServiceName, LPTSTR lpszBinaryPathName)
{
    char buf[256];
    HKEY key;
    DWORD rc;
    DWORD type;
    DWORD len;
    LONG iRet;

    TOS_UNUSED_ARG(lpszBinaryPathName);

    strcpy(buf, TEXT("SYSTEM\\CurrentControlSet\\Services\\"));
    strcat(buf, lpszServiceName);
    iRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, buf, 0, KEY_QUERY_VALUE, &key);
    if (ERROR_SUCCESS != iRet)
    {
        return (DWORD)-1;
    }

    rc = 0;
    RegCloseKey(key);

    return rc;
}

DWORD svc_is_unrunning (LPTSTR lpszServiceName)
{
    SC_HANDLE service;
    SC_HANDLE scm;
    DWORD rc;
    SERVICE_STATUS ss;

    scm = OpenSCManager(NULL, NULL, GENERIC_READ);
    if (!scm)
    {
        return GetLastError();
    }

    rc = 1;
    service = OpenService(scm, lpszServiceName, SERVICE_QUERY_STATUS);
    if (service)
    {
        while (QueryServiceStatus(service, &ss))
        {
            if (ssStatus.dwCurrentState == SERVICE_START_PENDING)
            {
                Sleep(1000);
            } else
            {
                break;
            }
        }

        if (ss.dwCurrentState == SERVICE_RUNNING)
        {
            rc = 0;
        }

        CloseServiceHandle(service);
    }
    CloseServiceHandle(scm);

    return rc;
}

int CommenceStartupProcessing(LPCTSTR lpszServiceName)
{
    sshStatusHandle = RegisterServiceCtrlHandler(lpszServiceName,
                                                 (LPHANDLER_FUNCTION)ServiceCtrl);
    if (0 == sshStatusHandle)
    {
        return 1;
    }

    /* initialize the Service Status structure */
    ssStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    ssStatus.dwCurrentState = SERVICE_START_PENDING;
    ssStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
    ssStatus.dwWin32ExitCode = NO_ERROR;
    ssStatus.dwServiceSpecificExitCode = 0;
    ssStatus.dwCheckPoint = 1;
    ssStatus.dwWaitHint = SCM_NOTIFICATION_INTERVAL * 2;

    SetServiceStatus(sshStatusHandle, &ssStatus);

    /* 启动一个线程发送SERVICE_START_PENDING 消息以使之能工作
     * start up a thread to keep sending SERVICE_START_PENDING to the Service Control Manager
     * until the slapd listener is completed and listening. Only then should we send
     * SERVICE_RUNNING to the Service Control Manager.
     */
    hServerStartedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (NULL == hServerStartedEvent)
    {
        ssStatus.dwCheckPoint++;
        ssStatus.dwWaitHint = THIRTY_SECONDS;
        SetServiceStatus(sshStatusHandle, &ssStatus);
    }
    else
    {
        hServerstartThread = CreateThread(NULL, 0,
                                          (LPTHREAD_START_ROUTINE)&StartStatusRoutine,
                                          NULL, 0, NULL);
        if (NULL == hServerstartThread)
        {
            ssStatus.dwCheckPoint++;
            ssStatus.dwWaitHint = THIRTY_SECONDS;
            SetServiceStatus(sshStatusHandle, &ssStatus);
        }
    }

    return 0;
}

void ReportServiceShutdownComplete()
{

    if (NULL != hServerStoppedEvent)
    {
        SetEvent(hServerStoppedEvent);
    }

    /* wait for the thread sending the SERVICE_STOP_PENDING messages
     * to the Service Control Manager to die.
     * if the wait fails then put ourselves to sleep
     * for half the Service Control Manager update interval
     */

    if (WaitForSingleObject(hServerStopThread, INFINITE) == WAIT_FAILED)
    {
        Sleep(SCM_NOTIFICATION_INTERVAL / 2);
    }


    ssStatus.dwCurrentState = SERVICE_STOPPED;
    ssStatus.dwCheckPoint++;
    ssStatus.dwWaitHint = SCM_NOTIFICATION_INTERVAL;
    SetServiceStatus(sshStatusHandle, &ssStatus);

}

int  CreateStopServiceEvent(void)
{
    int iRet = 0;

    hStopServerEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (NULL == hStopServerEvent)
    {
        iRet = 1;
    }
    return iRet;
}

void ReportStartUpComplete(int startUpTrue)
{
    if (NULL != hServerStartedEvent)
    {
        SetEvent(hServerStartedEvent);
    }

    if (WaitForSingleObject(hServerstartThread, INFINITE) == WAIT_FAILED)
    {
        Sleep(SCM_NOTIFICATION_INTERVAL / 2);
    }

    if (startUpTrue)
    {
        ssStatus.dwCurrentState = SERVICE_RUNNING;
    } else
    {
        ssStatus.dwCurrentState = SERVICE_STOP_PENDING;
    }


    ssStatus.dwCheckPoint++;
    ssStatus.dwWaitHint = SCM_NOTIFICATION_INTERVAL;
    SetServiceStatus(sshStatusHandle, &ssStatus);
}


int CmdChangeServiceConfig(DWORD dwStartType, LPTSTR lpszServiceName,
                           LPTSTR lpszDisplayName, LPTSTR  lpszBinaryPathName, LPTSTR err)
{
    SC_HANDLE schService;
    SC_HANDLE schSCManager;
    int iRet = 0;

    schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if ((NULL == lpszServiceName) || (NULL == err))
    {
        return 1;
    }

    if (NULL != schSCManager)
    {
        schService = OpenService(schSCManager, lpszServiceName, SERVICE_ALL_ACCESS);
        if (NULL != schService)
        {
            switch (dwStartType)
            {
                case SERVICE_AUTO_START:
                case SERVICE_DEMAND_START:
                case SERVICE_DISABLED:
                case SERVICE_SYSTEM_START:
                case SERVICE_BOOT_START:
                    iRet = ChangeServiceConfig(schService, SERVICE_WIN32_OWN_PROCESS,
                                               dwStartType, SERVICE_ERROR_NORMAL,
                                               lpszBinaryPathName, NULL, NULL,
                                               "", NULL, NULL, TEXT(lpszDisplayName)) ;
                    if (0 == iRet)
                    {
                        iRet = 1;
                        sprintf(err, "Change Service Config failed. Error=%lu :%s\n",
                                GetLastError(), GetLastErrorString());
                    } else
                    {
                        iRet = 0;
                    }
                    break;
                default:
                    strcpy(err, "服务启动模式参数不正确");
                    iRet = 1;
            }

            CloseServiceHandle(schService);
            CloseServiceHandle(schSCManager);
            return iRet;
        }
        else
        {
            sprintf(err, "Change Service Config  failed. Error=%lu :%s\n",
                    GetLastError(), GetLastErrorString());
            CloseServiceHandle(schSCManager);
            return 1;
        }
    }
    else
    {
        sprintf(err, "Change Service Config. Error=%lu :%s\n",
                GetLastError(), GetLastErrorString());
    }

    return 1;
}

int WaitForServerStop(void)
{
    DWORD dwWait = 0, iRet = 1;
    dwWait = WaitForSingleObject(hStopServerEvent, INFINITE);
    if ((WAIT_ABANDONED == dwWait) || (WAIT_OBJECT_0 == dwWait))
    {
        CloseHandle(hStopServerEvent);
        iRet = 0;
    }
    return iRet ;
}

/**Judge Service running status, if service is start pandding  wait 3 manite*/
int  QueryServiceStatusRunning(LPTSTR lpszServiceName, DWORD dwMaxWaitMinutes, LPTSTR err)
{
    SC_HANDLE schService;
    SC_HANDLE schSCManager;
    SERVICE_STATUS ssStatus;
    int minutes = dwMaxWaitMinutes;
    DWORD dwWaitTime = 0;
    DWORD time = 0;

    if (minutes > 3)
    {
        minutes = 3;
    }
    else if (minutes <= 0)
    {
        minutes = 1;
    }

    dwWaitTime = minutes * 60 * 1000;
    memset(&ssStatus, 0x0, sizeof ssStatus);

    schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if (NULL != schSCManager)
    {
        schService = OpenService(schSCManager, lpszServiceName, SERVICE_ALL_ACCESS);
        if (NULL != schService)
        {
            while ((time < dwWaitTime) &&  (QueryServiceStatus(schService, &ssStatus)))
            {
                if (ssStatus.dwCurrentState == SERVICE_START_PENDING)
                {
                    time += 1000;
                    Sleep(1000);
                } else  break;
            }


            CloseServiceHandle(schService);
            CloseServiceHandle(schSCManager);

            if (SERVICE_RUNNING == ssStatus.dwCurrentState)
            {
                return 0;
            } else
            {
                return 1;
            }
        }
        else
        {
            sprintf(err, "OpenService  failed. Error=%lu :%s\n",
                    GetLastError(), GetLastErrorString());
            CloseServiceHandle(schSCManager);
            return 1;
        }
    }
    else
    {
        sprintf(err, "OpenSCManager failed. Error=%lu :%s\n",
                GetLastError(), GetLastErrorString());
    }

    return 1;
}

int GetServiceParam(LPTSTR lpszServiceName, LPTSTR lpszDisplayName,
                    DWORD lpszDisplayNameLen, LPTSTR  lpszBinaryPathName,
                    DWORD lpszBinaryPathNameLen)
{
    char buf[256];
    HKEY key;
    DWORD rc;
    DWORD type;
    DWORD len;
    LONG iRet;

    if ((NULL == lpszDisplayName) || (NULL == lpszBinaryPathName))
    {
        return 0xFF;
    }

    strcpy(buf, TEXT("SYSTEM\\CurrentControlSet\\Services\\"));
    strcat(buf, lpszServiceName);
    iRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, buf, 0, KEY_QUERY_VALUE, &key);
    if (ERROR_SUCCESS != iRet)
    {
        return -1;
    }

    rc = 0;
    len = lpszBinaryPathNameLen;
    iRet = RegQueryValueEx(key, "ImagePath", NULL, &type, (LPBYTE)lpszBinaryPathName, &len);
    if (ERROR_SUCCESS != iRet)
    {
        rc = 1;
    }

    len = lpszDisplayNameLen;
    iRet = RegQueryValueEx(key, "DisplayName", NULL, &type, (LPBYTE)lpszDisplayName, &len);
    if (ERROR_SUCCESS != iRet)
    {
        rc = 1;
    }

    RegCloseKey(key);
    return rc;
}

int SetServiceCurrentPath(LPTSTR  lpszBinaryPathName)
{
    char path[MAX_PATH], *start_ch = NULL;

    if (NULL == lpszBinaryPathName)
    {
        return -1;
    }

    memset(path, 0x0, sizeof(path));
    strcpy(path, lpszBinaryPathName);

    start_ch = strrchr(path, DIRSEP);
    if (NULL != start_ch)
    {
        *start_ch = '\0';
        if (SetCurrentDirectory(path))
        {
            return 0;
        } else
        {
            return -2;
        }
    }

    return -3;
}

DWORD GetServiceStatus(LPTSTR lpszServiceName)
{
    SC_HANDLE service;
    SC_HANDLE scm;
    DWORD rc;
    SERVICE_STATUS ss;

    scm = OpenSCManager(NULL, NULL, GENERIC_READ);
    if (!scm)
    {
        return GetLastError();
    }

    rc = (DWORD)-1;
    service = OpenService(scm, lpszServiceName, SERVICE_QUERY_STATUS);
    if (service)
    {
        if (QueryServiceStatus(service, &ss) != 0)
        {
            rc = ss.dwCurrentState;
        }

        CloseServiceHandle(service);
    }
    CloseServiceHandle(scm);

    return rc;
}

void SvcDebugOut(LPSTR szFormat, ...)
{
    char  szBuffer[1024];
    va_list ap;
    int iWriteLen;

    va_start(ap, szFormat);
    iWriteLen = vsnprintf(szBuffer, sizeof(szBuffer), szFormat, ap);
    if (((int)sizeof(szBuffer) <= iWriteLen) || (0 > iWriteLen))
    {
        return ;
    }

    OutputDebugStringA(szBuffer);
}

/** 通过SERVICE_CONTROL_PAUSE信号来 通知进程reload
*/
int CmdNotifyServiceRefresh(LPTSTR lpszServiceName, LPTSTR err)
{
    SC_HANDLE schService;
    SC_HANDLE schSCManager;
    SERVICE_STATUS ssStatus;
    schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if (NULL != schSCManager)
    {
        schService = OpenService(schSCManager, lpszServiceName, SERVICE_ALL_ACCESS);
        if (NULL != schService)
        {
            if (ControlService(schService, SERVICE_CONTROL_PAUSE, &ssStatus))
            {
                //_tprintf("Notify Service(%s) Refresh succeessfully! ", lpszServiceName);

                CloseServiceHandle(schService);
                CloseServiceHandle(schSCManager);
                return 0;
            }
            sprintf(err, "StopService failed. Error=%lu :%s\n",
                    GetLastError(), GetLastErrorString());
            CloseServiceHandle(schService);
            CloseServiceHandle(schSCManager);
            return 1;
        }
        else
        {
            sprintf(err, "OpenService  failed. Error=%lu :%s\n",
                    GetLastError(), GetLastErrorString());
            CloseServiceHandle(schSCManager);
            return 1;
        }
    }
    else
    {
        sprintf(err, "OpenSCManager failed. Error=%lu :%s\n",
                GetLastError(), GetLastErrorString());
    }

    return 1;
}

#endif
