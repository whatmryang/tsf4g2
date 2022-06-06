#ifndef  _TAPP_NT_SERVICE_H_
#define	 _TAPP_NT_SERVICE_H_

#if defined(_WIN32) || defined(_WIN64)

#include <windows.h>
#include <winsvc.h>

#ifdef  __cplusplus
extern "C"	{
#endif

    void AddToMessageLog(LPTSTR lpszMsg, WORD dwType, DWORD dwEventID, LPTSTR lpszServiceName);

    int CmdInstallService(DWORD dwStartType, const LPTSTR lpszServiceName, const LPSTR lpszServiceDesc,
                          const LPTSTR lpszDisplayName, const LPTSTR lpszBinaryPathName, LPTSTR err);

    int CmdRemoveService(LPTSTR lpszServiceName, LPTSTR err);

    int CmdStartService(LPTSTR lpszServiceName, LPTSTR lpszDisplayName,
        DWORD dwNumServiceArgs, LPCTSTR* lpServiceArgVectors,LPTSTR err);

    int CmdStopService(LPTSTR lpszServiceName, LPTSTR err);

    int CmdNotifyServiceRefresh(LPTSTR lpszServiceName, LPTSTR err);

    /* Get path and displayName */
    int GetServiceParam(LPTSTR lpszServiceName, LPTSTR lpszDisplayName,
                        DWORD lpszDisplayNameLen, LPTSTR  lpszBinaryPathName,
                        DWORD lpszBinaryPathNameLen);

    int SetServiceCurrentPath(LPTSTR  lpszBinaryPathName);

    DWORD GetServiceStatus(LPTSTR lpszServiceName);

    LPTSTR GetLastErrorString(void);
    LPTSTR GetLastErrorText(LPTSTR lpszBuf, DWORD dwSize);

    int CommenceStartupProcessing(LPCTSTR lpszServiceName);
    void ReportStartUpComplete(int startUpTrue);

    void ReportServiceShutdownComplete();
    int  CreateStopServiceEvent(void);
    int WaitForServerStop(void);

    void SvcDebugOut(LPSTR szFormat, ...);

#ifdef  __cplusplus
}
#endif

#endif

#endif /* #ifndef  _TAPP_NT_SERVICE_H_ */
