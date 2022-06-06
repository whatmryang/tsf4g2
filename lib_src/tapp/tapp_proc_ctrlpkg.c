/***********************************************************
 * FileName: tapp_proc_ctrlpkg.c
 * Author: jackyai
 * Version: 1.0
 * Date: 2010-01-26
 * Function List:
 *     tappctrl_proc_helppkg
 *     tappctrl_proc_echopkg
 *     tappctrl_proc_cmdlinepkg
 *     tappctrl_proc_binpkg
 *     tappctrl_proc_getconfkg
 *     tappctrl_proc_getcomulatefkg
 *     tappctrl_proc_getstatusfkg
 *     tappctrl_proc_authenfkg
 *     tappctrl_proc_reloadfkg
 *     tappctrl_proc_bus_global
 *     tappctrl_proc_bus_handle
 *     tappctrl_proc_bus_channel
 *     tappctrl_proc_show_by_addr
 *     tappctrl_proc_show_by_key
 *     tappctrl_fill_channel_res
 *     tappctrl_proc_pkg
 * Description:
 *      this source file implements msg process logic of tapp-ctrl's server end
 * History:
 *      <time>        <author>      <version>   <desc>
 *      2010-01-26     jackyai       1.0         创建
 * ***********************************************************/

#include "tapp_i.h"
#include "tappctrl_kernel_i.h"

#include "../tbus/tbus_kernel.h"
#include "../tbus/tbus_misc.h"
#include "../tbus/tbus_channel.h"

#define CLEARTAIL(str) (str)[sizeof(str)-1]='\0'

/////////////////////////////////////////////////////////////////////
int tappctrl_proc_pkg(TAPPCTX* a_pstCtx, void* a_pvArg);

extern TAPP_GS gs_stTapp ;
extern TAPPCTX_INTERNAL gs_stCtxInternal;

///////////////////////////////////////////////////////////////////////

static int tappctrl_proc_helppkg(TAPPCTX* a_pstCtx, COMMANDINJECTPKG *a_pstCtrlPkg)
{
	int iRet = 0;
	COMMANDINJECTPKG stResPkg;
	int iWriteLen;



	assert(NULL != a_pstCtrlPkg);
	assert(NULL != a_pstCtx);
	assert(TAPP_HELP_REQ == a_pstCtrlPkg->stHead.nCmdID);

	tappctrl_fill_pkghead(&stResPkg.stHead, TAPP_HELP_RES, tapp_controller_get_version());

	if (a_pstCtx->pfnGetCtrlUsage)
	{
		const char *pszHelp = a_pstCtx->pfnGetCtrlUsage();
		iWriteLen = snprintf(stResPkg.stBody.szHelpRes, sizeof(stResPkg.stBody.szHelpRes),"%s",
			((pszHelp== NULL)?"":pszHelp));
		if ((iWriteLen < 0)||(iWriteLen >= (int)sizeof(stResPkg.stBody.szHelpRes)))
		{
			tlog_error(gs_stTapp.pstAppCatText, 0, 0, "failed to get %s's control usage", a_pstCtx->pszApp);
			return -1;
		}

	}else
	{
		stResPkg.stBody.szHelpRes[0] = '\0';
	}

	iRet = tappctrl_send_pkg(&stResPkg);

	return iRet;
}

static int tappctrl_proc_echopkg(COMMANDINJECTPKG *a_pstCtrlPkg)
{
	int iRet = 0;
	COMMANDINJECTPKG stResPkg;



	assert(NULL != a_pstCtrlPkg);
	assert(TAPP_ECHO_REQ == a_pstCtrlPkg->stHead.nCmdID);

	tappctrl_fill_pkghead(&stResPkg.stHead, TAPP_ECHO_RES, tapp_controller_get_version());


	STRNCPY(stResPkg.stBody.szTestRes, "echo from server", sizeof(stResPkg.stBody.szTestRes));


	iRet = tappctrl_send_pkg(&stResPkg);

	return iRet;
}

static int tappctrl_proc_cmdlinepkg(TAPPCTX* a_pstCtx, void* a_pvArg, COMMANDINJECTPKG *a_pstCtrlPkg)
{
	int iRet = 0;
	LPCICMDLINEREQ pstCmdLine;
	int i = 0;
	const char* argv[TAPPCTRL_MAX_ARGC];

	assert(NULL != a_pstCtrlPkg);
	assert(TAPP_CMDLINE_REQ == a_pstCtrlPkg->stHead.nCmdID);

	pstCmdLine = (LPCICMDLINEREQ)&a_pstCtrlPkg->stBody.stCmdLineReq;


	if (TAPPCTRL_MAX_ARGC < pstCmdLine->wArgc || 0 == pstCmdLine->wArgc)
	{
		tlog_error(gs_stTapp.pstAppCatText, 0, 0, "recv invalid TAPP_CMDLINE_REQ, argc<%d> is 0 or more than MAX_ARGC<%d>",
			pstCmdLine->wArgc, TAPPCTRL_MAX_ARGC);
		return -1;
	}

	tlog_fatal(gs_stTapp.pstAppCatText, 0, 0, "recv cmdline msg, cmd<%s>", pstCmdLine->aszArgv[0]);

	if (0 < pstCmdLine->wArgc)
	{
		if (a_pstCtx->pfnProcCmdLine)
		{
			for (i = 0; i < pstCmdLine->wArgc; i++)
				argv[i] = pstCmdLine->aszArgv[i];
			iRet = a_pstCtx->pfnProcCmdLine(a_pstCtx, a_pvArg, pstCmdLine->wArgc, argv);
			tlog_info(gs_stTapp.pstAppCatText, 0, 0, "pfnProcCmdLine return %d", iRet);
		}else
		{
			tlog_error(gs_stTapp.pstAppCatText, 0, 0, "no interface was set for cmdline");
			tappctrl_send_string("server not set interface for \'cmdline\'");
		}
	}else
	{
		tappctrl_send_string("invalid for \'cmdline\'");
	}

	return iRet;
}



static int tappctrl_proc_binpkg(TAPPCTX* a_pstCtx, void* a_pvArg, COMMANDINJECTPKG *a_pstCtrlPkg)
{
	int iRet = 0;
	LPCIBINDATAREQ pstBinData;

	assert(NULL != a_pstCtrlPkg);
	assert(TAPP_BINARY_REQ == a_pstCtrlPkg->stHead.nCmdID);

	pstBinData = (LPCIBINDATAREQ)&a_pstCtrlPkg->stBody.stBinDataReq;
	if (a_pstCtx->pfnProcCtrlReq)
	{
		iRet = a_pstCtx->pfnProcCtrlReq(a_pstCtx, a_pvArg,
			pstBinData->szMetaName, pstBinData->szData, (int)pstBinData->wDataLen);
		tlog_info(gs_stTapp.pstAppCatText, 0, 0, "BinMsgProc return <%d>", iRet);
	}
	else
	{
		tappctrl_send_string("server not set interface for \'sendbin\'");
		tlog_error(gs_stTapp.pstAppCatText, 0, 0, "recv a TAPP_BINARY msg, but a_pstCtx->pfnProcCtrlReq not set");
	}

	return iRet;
}

static int tappctrl_proc_get_log_confkg(TAPPCTX* a_pstCtx, COMMANDINJECTPKG *a_pstCtrlPkg)
{
	int iRet = 0;
	TDRDATA stOutData;
	TDRDATA stHostData;
	COMMANDINJECTPKG stResPkg;

	TOS_UNUSED_ARG(a_pstCtx);

	assert(NULL != a_pstCtrlPkg);
	assert(TAPP_LOG_SHOW_CONF_REQ == a_pstCtrlPkg->stHead.nCmdID);

	if (NULL == gs_stCtxInternal.stLogConfData.pszBuff)
	{
		tappctrl_send_string("no conf-data available");
		return 0;
	}

	if ((NULL == gs_stCtxInternal.stLogConfData.pszBuff)||(0 >= gs_stCtxInternal.stLogConfData.iLen))
	{
	      tappctrl_send_string("no conf-data available");
		  return 0;
	}

	tappctrl_fill_pkghead(&stResPkg.stHead, TAPP_LOG_SHOW_CONF_RES, tapp_controller_get_version());

	stOutData.pszBuff = (char*)&stResPkg.stBody.szShowLogConfRes;
	stOutData.iBuff = sizeof(stResPkg.stBody.szShowLogConfRes) - 1;

	stHostData.pszBuff = gs_stCtxInternal.stLogConfData.pszBuff;
	stHostData.iBuff =gs_stCtxInternal.stLogConfData.iLen;

	iRet = tdr_sprintf((LPTDRMETA)gs_stCtxInternal.stLogConfData.iMeta, &stOutData,
		&stHostData, 0);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		const char* pszError = "server failed to generate visual string for conf data";
		tlog_error(gs_stTapp.pstAppCatText, 0, 0, "tdr_sprintf error, for %s",
			tdr_error_string(iRet));
		tappctrl_send_string(pszError);
	}else
	{
		stResPkg.stBody.szShowLogConfRes[stOutData.iBuff] = '\0';
		iRet = tappctrl_send_pkg(&stResPkg);
	}

	return iRet;
}


static int tappctrl_proc_getconfkg(TAPPCTX* a_pstCtx, COMMANDINJECTPKG *a_pstCtrlPkg)
{
	int iRet = 0;
	TDRDATA stOutData;
	TDRDATA stHostData;
	COMMANDINJECTPKG stResPkg;

	assert(NULL != a_pstCtrlPkg);
	assert(TAPP_GET_CONF_REQ == a_pstCtrlPkg->stHead.nCmdID);

	if (0 == a_pstCtx->stConfData.iMeta)
	{
		tappctrl_send_string("no conf-data available");
		return 0;
	}
    if ((NULL == a_pstCtx->stConfData.pszBuff)||(0 >= a_pstCtx->stConfData.iLen))
    {
        tappctrl_send_string("no conf-data available");
    }



	tappctrl_fill_pkghead(&stResPkg.stHead, TAPP_GET_CONF_RES, tapp_controller_get_version());

	stOutData.pszBuff = (char*)&stResPkg.stBody.szGetConfRes;
	stOutData.iBuff = sizeof(stResPkg.stBody.szGetConfRes) - 1;
	stHostData.pszBuff = a_pstCtx->stConfData.pszBuff;
	stHostData.iBuff = a_pstCtx->stConfData.iLen;
	iRet = tdr_sprintf((LPTDRMETA)a_pstCtx->stConfData.iMeta, &stOutData,
		&stHostData, 0);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		const char* pszError = "server failed to generate visual string for conf data";
		tlog_error(gs_stTapp.pstAppCatText, 0, 0, "tdr_sprintf error, for %s",
			tdr_error_string(iRet));
		tappctrl_send_string(pszError);
	}else
	{
		stResPkg.stBody.szGetConfRes[stOutData.iBuff] = '\0';
		iRet = tappctrl_send_pkg(&stResPkg);
	}

	return iRet;
}

static int tappctrl_proc_getcomulatefkg(TAPPCTX* a_pstCtx, COMMANDINJECTPKG *a_pstCtrlPkg)
{
	int iRet = 0;
	TDRDATA stOutData;
	TDRDATA stHostData;
	COMMANDINJECTPKG stResPkg;

	assert(NULL != a_pstCtrlPkg);
	assert(TAPP_GET_CUMULATE_REQ == a_pstCtrlPkg->stHead.nCmdID);

	if (0 == a_pstCtx->stRunDataCumu.iMeta)
	{
		tappctrl_send_string("no runtime-cumulate-data available");
		return 0;
	}
    if ((NULL == a_pstCtx->stRunDataCumu.pszBuff)||(0 >= a_pstCtx->stRunDataCumu.iLen))
    {
        tappctrl_send_string("no conf-data available");
    }


	tappctrl_fill_pkghead(&stResPkg.stHead, TAPP_GET_CUMULATE_RES, tapp_controller_get_version());

	stOutData.pszBuff = (char*)&stResPkg.stBody.szGetCumuRes;
	stOutData.iBuff = sizeof(stResPkg.stBody.szGetCumuRes) - 1;
	stHostData.pszBuff = a_pstCtx->stRunDataCumu.pszBuff;
	stHostData.iBuff = a_pstCtx->stRunDataCumu.iLen;
	iRet = tdr_sprintf((LPTDRMETA)a_pstCtx->stRunDataCumu.iMeta, &stOutData,
		&stHostData, 0);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		const char* pszError = "server failed to generate visual string for runtime-cumulate-data";
		tlog_error(gs_stTapp.pstAppCatText, 0, 0, "tdr_sprintf error, for %s",
			tdr_error_string(iRet));
		tappctrl_send_string(pszError);
	}else
	{
		stResPkg.stBody.szGetCumuRes[stOutData.iBuff] = '\0';
		iRet = tappctrl_send_pkg(&stResPkg);
	}


	return iRet;
}

static int tappctrl_proc_getstatusfkg(TAPPCTX* a_pstCtx, COMMANDINJECTPKG *a_pstCtrlPkg)
{
	int iRet = 0;
	TDRDATA stOutData;
	TDRDATA stHostData;
	COMMANDINJECTPKG stResPkg;

	assert(NULL != a_pstCtrlPkg);
	assert(TAPP_GET_STATUS_REQ == a_pstCtrlPkg->stHead.nCmdID);

	if (0 == a_pstCtx->stRunDataStatus.iMeta)
	{
		tappctrl_send_string("no runtime-status-data available");
		return 0;
	}
    if ((NULL == a_pstCtx->stRunDataStatus.pszBuff)||(0 >= a_pstCtx->stRunDataStatus.iLen))
    {
        tappctrl_send_string("no conf-data available");
    }


	tappctrl_fill_pkghead(&stResPkg.stHead, TAPP_GET_STATUS_RES, tapp_controller_get_version());

	stOutData.pszBuff = (char*)&stResPkg.stBody.szGetStatRes;
	stOutData.iBuff = sizeof(stResPkg.stBody.szGetStatRes) - 1;
	stHostData.pszBuff = a_pstCtx->stRunDataStatus.pszBuff;
	stHostData.iBuff = a_pstCtx->stRunDataStatus.iLen;
	iRet = tdr_sprintf((LPTDRMETA)a_pstCtx->stRunDataStatus.iMeta, &stOutData,
		&stHostData, 0);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		const char* pszError = "server failed to generate visual string for runtime-status-data ";
		tlog_error(gs_stTapp.pstAppCatText, 0, 0, "tdr_sprintf error, for %s",
			tdr_error_string(iRet));
		tappctrl_send_string(pszError);
	}else
	{
		stResPkg.stBody.szGetStatRes[stOutData.iBuff] = '\0';
		iRet = tappctrl_send_pkg(&stResPkg);
	}



	return iRet;
}

static int tappctrl_proc_authenfkg(COMMANDINJECTPKG *a_pstCtrlPkg)
{
	int iRet = 0;
	COMMANDINJECTPKG stResPkg;
	const char* pszConsoleAddr;
	int iRes = 0;

	assert(NULL != a_pstCtrlPkg);
	assert(TAPP_AUTHEN_REQ == a_pstCtrlPkg->stHead.nCmdID);


	tappctrl_fill_pkghead(&stResPkg.stHead, TAPP_AUTHEN_RES, tapp_controller_get_version());

	pszConsoleAddr = a_pstCtrlPkg->stBody.szAuthenReq;
	if ((NULL != gs_stCtxInternal.pszCIAddr) &&
		(!strncasecmp(gs_stCtxInternal.pszCIAddr, pszConsoleAddr, strlen(pszConsoleAddr))))
	{
		iRes = TAPP_AUTH_PASSED;
	}else
	{
		iRes = TAPP_AUTH_FAILED;
	}
	stResPkg.stBody.iAuthenRes = iRes;

	iRet = tappctrl_send_pkg(&stResPkg);

	return iRet;
}

static int tappctrl_proc_reloadfkg(COMMANDINJECTPKG *a_pstCtrlPkg)
{
	int iRet = 0;
	COMMANDINJECTPKG stResPkg;

	assert(NULL != a_pstCtrlPkg);
	assert(TAPP_RELOAD_CONF_REQ == a_pstCtrlPkg->stHead.nCmdID);


	tappctrl_fill_pkghead(&stResPkg.stHead, TAPP_RELOAD_CONF_RES, tapp_controller_get_version());

	STRNCPY(stResPkg.stBody.szReloadConfRes, "reload successfully!", sizeof(stResPkg.stBody.szReloadConfRes));
	tapp_on_reload();

	iRet = tappctrl_send_pkg(&stResPkg);

	return iRet;
}


static int tappctrl_proc_bus_global(TAPPCTX* a_pstCtx, COMMANDINJECTPKG *a_pstCtrlPkg)
{
	int iRet = 0;
    extern TBUSGLOBAL g_stBusGlobal;

	COMMANDINJECTPKG stResPkg;
    LPCITBUSGLOBAL pstGlobal;

	assert(NULL != a_pstCtrlPkg);
	assert(TAPP_BUS_GLOBAL_REQ == a_pstCtrlPkg->stHead.nCmdID);

    pstGlobal = &stResPkg.stBody.stBusGlobalRes;

    if (!a_pstCtx->iUseBus)
    {
        iRet = tappctrl_send_string("Error: server not use tbus");
        return iRet;
    }

    if (NULL == g_stBusGlobal.pstGCIM)
    {
        iRet = tappctrl_send_string("Error: tbus have not inited");
        return iRet;
    }


    // copy data
    pstGlobal->dwDyedMsgID = g_stBusGlobal.dwDyedMsgID;
    pstGlobal->dwGCIMVersion = g_stBusGlobal.dwGCIMVersion;
    pstGlobal->dwHandleCnt = g_stBusGlobal.dwHandleCnt;
    pstGlobal->dwInitStatus = g_stBusGlobal.dwInitStatus;
    pstGlobal->iBussID = g_stBusGlobal.iBussId;
    pstGlobal->iCounterfeitPkgLen = g_stBusGlobal.iCounterfeitPkgLen;
    pstGlobal->iFlag = g_stBusGlobal.iFlag;
    pstGlobal->stGCIMHead.dwBusiID = g_stBusGlobal.pstGCIM->stHead.dwBusiID;
    CLEARTAIL(pstGlobal->stGCIMHead.szCreatTime);
    snprintf(pstGlobal->stGCIMHead.szCreatTime, sizeof(pstGlobal->stGCIMHead.szCreatTime),
            "%s", ctime((const time_t*)&g_stBusGlobal.pstGCIM->stHead.dwCreateTime));
    pstGlobal->stGCIMHead.szCreatTime[strlen(pstGlobal->stGCIMHead.szCreatTime)-1] = '\0';
    CLEARTAIL(pstGlobal->stGCIMHead.szLastStamp);
    snprintf(pstGlobal->stGCIMHead.szLastStamp, sizeof(pstGlobal->stGCIMHead.szLastStamp),
            "%s", ctime((const time_t*)&g_stBusGlobal.pstGCIM->stHead.dwLastStamp));
    pstGlobal->stGCIMHead.szLastStamp[strlen(pstGlobal->stGCIMHead.szLastStamp)-1] = '\0';
    pstGlobal->stGCIMHead.dwMaxCnt = g_stBusGlobal.pstGCIM->stHead.dwMaxCnt;
    pstGlobal->stGCIMHead.dwShmKey = g_stBusGlobal.pstGCIM->stHead.dwShmKey;
    pstGlobal->stGCIMHead.dwShmSize = g_stBusGlobal.pstGCIM->stHead.dwShmSize;
    pstGlobal->stGCIMHead.dwUsedCnt = g_stBusGlobal.pstGCIM->stHead.dwUsedCnt;
    pstGlobal->stGCIMHead.dwVersion = g_stBusGlobal.pstGCIM->stHead.dwVersion;

	tappctrl_fill_pkghead(&stResPkg.stHead, TAPP_BUS_GLOBAL_RES, tapp_controller_get_version());

    iRet = tappctrl_send_pkg(&stResPkg);

	return iRet;
}

static int tappctrl_proc_bus_handle(TAPPCTX* a_pstCtx, COMMANDINJECTPKG *a_pstCtrlPkg)
{
	int iRet = 0;
    extern TBUSGLOBAL g_stBusGlobal;
    unsigned int dwIndex;
	COMMANDINJECTPKG stResPkg;
    LPCITBUSHANDLE pstHandle;
    LPTBUSHANDLE  pstSrcHandle;
    int i;
//    unsigned int dwCount;

	assert(NULL != a_pstCtrlPkg);
	assert(TAPP_BUS_HANDLE_REQ == a_pstCtrlPkg->stHead.nCmdID);

    pstHandle = &stResPkg.stBody.stBusHandleRes;

    if (!a_pstCtx->iUseBus)
    {
        iRet = tappctrl_send_string("Error: server not use tbus");
        return iRet;
    }

    // other checks
    if (TBUS_MODULE_INITED != g_stBusGlobal.dwInitStatus)
    {
        iRet = tappctrl_send_string("server's tbus module has not been initiated");
        return iRet;
    }

    dwIndex = a_pstCtrlPkg->stBody.dwBusHandleReq;
    if (dwIndex  > g_stBusGlobal.dwHandleCnt)
    {
        snprintf((char*)&stResPkg, sizeof(stResPkg)-1, "Error: only %d tbus handles, no handle<%d>.",
                g_stBusGlobal.dwHandleCnt, dwIndex );
        iRet = tappctrl_send_string((const char*)&stResPkg);
        return iRet;
    }

    pstSrcHandle = g_stBusGlobal.pastTbusHandle[dwIndex];
    if (NULL == pstSrcHandle)
    {
        snprintf((char*)&stResPkg, sizeof(stResPkg)-1, "Error:  %dth tbus handles is null",
            dwIndex );
        iRet = tappctrl_send_string((const char*)&stResPkg);
        return iRet;
    }

    // copy data
    pstHandle->dwChannelCnt = pstSrcHandle->dwChannelCnt;

    pstHandle->dwChannelCnt = 0;
    for (i = 0; i < (int)pstSrcHandle->dwChannelCnt; i++)
    {
        if (NULL == pstSrcHandle->pastChannelSet[i])
        {
            continue;
        }
        if (NULL == pstSrcHandle->pastChannelSet[i]->pstHead)
        {
            continue;
        }

        tbus_addr_ntop(pstSrcHandle->pastChannelSet[i]->pstHead->astAddr[0],
                pstHandle->astChannelAddrs[i].aszAddr[0],
                sizeof(pstHandle->astChannelAddrs[i].aszAddr[0]));
        tbus_addr_ntop(pstSrcHandle->pastChannelSet[i]->pstHead->astAddr[1],
                pstHandle->astChannelAddrs[i].aszAddr[1],
                sizeof(pstHandle->astChannelAddrs[i].aszAddr[1]));
        pstHandle->dwChannelCnt++;
        if (sizeof(pstHandle->astChannelAddrs)/sizeof(pstHandle->astChannelAddrs[0]) <= pstHandle->dwChannelCnt)
        {
            break;
        }
    }/*for (i = 0; i < (int)pstSrcHandle->dwChannelCnt; i++)*/
    pstHandle->dwFlag = pstSrcHandle->dwFlag;
    pstHandle->dwGCIMVersion = pstSrcHandle->dwGCIMVersion;
    pstHandle->dwRecvPkgCnt = pstSrcHandle->dwRecvPkgCnt;
    pstHandle->iRecvPos = pstSrcHandle->iRecvPos;
    pstHandle->iBindAddrNum = pstSrcHandle->iBindAddrNum;
    if (sizeof(pstHandle->aszBindAddr)/sizeof(pstHandle->aszBindAddr[0]) < (unsigned int)pstHandle->iBindAddrNum)
    {
        pstHandle->iBindAddrNum = sizeof(pstHandle->aszBindAddr)/sizeof(pstHandle->aszBindAddr[0]);
    }
    for (i = 0; i < pstHandle->iBindAddrNum; i++)
    {
        tbus_addr_ntop((TBUSADDR)pstSrcHandle->aiBindAddr[i], pstHandle->aszBindAddr[i], sizeof(pstHandle->aszBindAddr[i]));
    }

	tappctrl_fill_pkghead(&stResPkg.stHead, TAPP_BUS_HANDLE_RES, tapp_controller_get_version());

    iRet = tappctrl_send_pkg(&stResPkg);

	return iRet;
}

static int tappctrl_fill_channel_res(LPCOMMANDINJECTPKG a_pstResPkg,
       unsigned short a_wHandleIndex, unsigned short a_wChannelIndex)
{
    extern TBUSGLOBAL g_stBusGlobal;
    LPCISHOWCHNNLRES pstChannel;
    LPTBUSCHANNEL pstSrcChannel;
    LPCICHANNELVAR pstVar;
    int i;

    if (a_wHandleIndex  > (unsigned short)g_stBusGlobal.dwHandleCnt)
    {
        snprintf((char*)a_pstResPkg, sizeof(*a_pstResPkg)-1, "Error: only %d tbus handles, no handle<%hd>.",
                g_stBusGlobal.dwHandleCnt, a_wHandleIndex );
        return -1;
    }
    if (NULL == g_stBusGlobal.pastTbusHandle[a_wHandleIndex])
    {
        snprintf((char*)a_pstResPkg, sizeof(*a_pstResPkg)-1, "Error: %dth tbus handles is null.",
            a_wHandleIndex );
        return -1;
    }

    if (a_wChannelIndex + 1  > (unsigned short)g_stBusGlobal.pastTbusHandle[a_wHandleIndex]->dwChannelCnt)
    {
        snprintf((char*)a_pstResPkg, sizeof(*a_pstResPkg)-1, "Error: handle<%hd> has only %d channels, no channel<%hd>.",
                a_wHandleIndex, g_stBusGlobal.pastTbusHandle[a_wHandleIndex]->dwChannelCnt, a_wChannelIndex + 1);
        return -1;
    }



    pstChannel = &a_pstResPkg->stBody.stBusChannelRes;
    pstSrcChannel = g_stBusGlobal.pastTbusHandle[a_wHandleIndex]->pastChannelSet[a_wChannelIndex];
    if (NULL == pstSrcChannel)
    {
        snprintf((char*)a_pstResPkg, sizeof(*a_pstResPkg)-1, "Error: handle<%hd>'s  %dth channel is null.",
            a_wHandleIndex, a_wChannelIndex + 1);
        return -1;
    }
    if (NULL == pstSrcChannel->pstHead)
    {
        snprintf((char*)a_pstResPkg, sizeof(*a_pstResPkg)-1, "Error: handle<%hd>'s  %dth channel's head is null.",
            a_wHandleIndex, a_wChannelIndex + 1);
        return -1;
    }


    pstChannel->dwFlag = pstSrcChannel->dwFlag;
    pstChannel->iRecvSide = pstSrcChannel->iRecvSide;
    pstChannel->iSendSide = pstSrcChannel->iSendSide;
    pstChannel->iShmID = (int32_t)pstSrcChannel->iShmID;
    //    pstChannel->stBusHead = *(LPCITBUSHEAD)&pstSrcChannel->stHead;
    for (i = 0; i < 2; i++)
    {
        pstVar = &pstChannel->stChannelHead.astQueueVar[i];
        //        pstVar->chGStart = pstSrcChannel->pstHead->astQueueVar[i].chGStart;
        //        pstVar->chPStart = pstSrcChannel->pstHead->astQueueVar[i].chPStart;
        pstVar->dwGet = pstSrcChannel->pstHead->astQueueVar[i].dwGet;
        pstVar->dwHead = pstSrcChannel->pstHead->astQueueVar[i].dwHead;
        pstVar->dwSize = pstSrcChannel->pstHead->astQueueVar[i].dwSize;
        pstVar->dwTail = pstSrcChannel->pstHead->astQueueVar[i].dwTail;
        pstVar->iRecvSeq = pstSrcChannel->pstHead->astQueueVar[i].iRecvSeq;
        pstVar->iSeq = pstSrcChannel->pstHead->astQueueVar[i].iSeq;
    }
    tbus_addr_ntop((TBUSADDR)pstSrcChannel->pstHead->astAddr[0],
            pstChannel->stChannelHead.aszAddr[0], sizeof(pstChannel->stChannelHead.aszAddr[0]));
    tbus_addr_ntop((TBUSADDR)pstSrcChannel->pstHead->astAddr[1],
            pstChannel->stChannelHead.aszAddr[1], sizeof(pstChannel->stChannelHead.aszAddr[0]));
    pstChannel->stChannelHead.dwAlignLevel = pstSrcChannel->pstHead->dwAlignLevel;
    pstChannel->stChannelHead.dwMagic = pstSrcChannel->pstHead->dwMagic;
    pstChannel->stChannelHead.dwVersion = pstSrcChannel->pstHead->dwVersion;
    pstChannel->stChannelHead.iShmID = (int32_t)pstSrcChannel->pstHead->iShmID;
    //    CLEARTAIL(pstChannel->szBeginDisable);
    //    snprintf(pstChannel->szBeginDisable, sizeof(pstChannel->szBeginDisable),
    //            "%s", ctime((const time_t*)&pstSrcChannel->tBeginDisable));


    return 0;
}

static int tappctrl_proc_bus_channel(TAPPCTX* a_pstCtx, COMMANDINJECTPKG *a_pstCtrlPkg)
{
	int iRet = 0;
    extern TBUSGLOBAL g_stBusGlobal;
    unsigned short wHandleIndex;
    unsigned short wChannelIndex;
	COMMANDINJECTPKG stResPkg;

	assert(NULL != a_pstCtrlPkg);
	assert(TAPP_BUS_SHOW_CHNNL_REQ == a_pstCtrlPkg->stHead.nCmdID);

    if (!a_pstCtx->iUseBus)
    {
        iRet = tappctrl_send_string("Error: server not use tbus");
        return iRet;
    }

    // other checks
    if (TBUS_MODULE_INITED != g_stBusGlobal.dwInitStatus)
    {
        iRet = tappctrl_send_string("server's tbus module has not been initiated");
        return iRet;
    }

    wHandleIndex = a_pstCtrlPkg->stBody.stBusChannelReq.wHandleIndex;
    wChannelIndex = a_pstCtrlPkg->stBody.stBusChannelReq.wChannelIndex;

	tappctrl_fill_pkghead(&stResPkg.stHead, TAPP_BUS_SHOW_CHNNL_RES, tapp_controller_get_version());

    iRet = tappctrl_fill_channel_res(&stResPkg, wHandleIndex, wChannelIndex);
    if (0 > iRet)
    {
        iRet = tappctrl_send_string((const char*)&stResPkg);
    }
    else
    {
        iRet = tappctrl_send_pkg(&stResPkg);
    }

    return iRet;
}

static int tappctrl_proc_show_by_addr(TAPPCTX* a_pstCtx, COMMANDINJECTPKG *a_pstCtrlPkg)
{
	int iRet = 0;
    extern TBUSGLOBAL g_stBusGlobal;
    unsigned short wHandleIndex = 0;
    unsigned short wChannelIndex = 0;
	COMMANDINJECTPKG stResPkg;
    TBUSADDR srcAddr1;
    TBUSADDR srcAddr2;
    TBUSADDR dstAddr1;
    TBUSADDR dstAddr2;
    int iChannelCnt;
    const char *pszAddr1;
    const char *pszAddr2;
    int i;

	assert(NULL != a_pstCtrlPkg);
	assert(TAPP_BUS_SHOW_BY_ADDR_REQ == a_pstCtrlPkg->stHead.nCmdID);

    if (!a_pstCtx->iUseBus)
    {
        iRet = tappctrl_send_string("Error: server not use tbus");
        return iRet;
    }

    // other checks
    if (TBUS_MODULE_INITED != g_stBusGlobal.dwInitStatus)
    {
        iRet = tappctrl_send_string("server's tbus module has not been initiated");
        return iRet;
    }

    // set indexes
    pszAddr1 = a_pstCtrlPkg->stBody.stShowByAddrReq.szChnnlAddr1;
    pszAddr2 = a_pstCtrlPkg->stBody.stShowByAddrReq.szChnnlAddr2;
    wHandleIndex = (unsigned short)a_pstCtrlPkg->stBody.stShowByAddrReq.dwHandleIndex;
    iRet = tbus_addr_aton(pszAddr1, &srcAddr1);
    if (TBUS_ERR_IS_ERROR(iRet))
    {
        snprintf((char*)&stResPkg, sizeof(stResPkg), "Error: channel addr_1  %s, %s",
                pszAddr1, tbus_error_string(iRet));
        iRet = tappctrl_send_string((const char*)&stResPkg);
        return iRet;
    }
    iRet = tbus_addr_aton(pszAddr2, &srcAddr2);
    if (TBUS_ERR_IS_ERROR(iRet))
    {
        snprintf((char*)&stResPkg, sizeof(stResPkg), "Error: channel addr_2  %s, %s",
                pszAddr2, tbus_error_string(iRet));
        iRet = tappctrl_send_string((const char*)&stResPkg);
        return iRet;
    }
    if (NULL == g_stBusGlobal.pastTbusHandle[wHandleIndex])
    {
        snprintf((char*)&stResPkg, sizeof(stResPkg), "Error: %dth handle is null", wHandleIndex);
        iRet = tappctrl_send_string((const char*)&stResPkg);
        return iRet;
    }

    iChannelCnt = (int)g_stBusGlobal.pastTbusHandle[wHandleIndex]->dwChannelCnt;
    for (i = 0; i < iChannelCnt; i++)
    {
        if (NULL == g_stBusGlobal.pastTbusHandle[wHandleIndex]->pastChannelSet[i])
        {
            continue;
        }

        dstAddr1 = g_stBusGlobal.pastTbusHandle[wHandleIndex]->pastChannelSet[i]->pstHead->astAddr[0];
        dstAddr2 = g_stBusGlobal.pastTbusHandle[wHandleIndex]->pastChannelSet[i]->pstHead->astAddr[1];
        if ((srcAddr1 == dstAddr1 && srcAddr2 == dstAddr2)
                || (srcAddr1 == dstAddr2 && srcAddr2 == dstAddr1))
        {
            wChannelIndex = (unsigned short)i;
            break;
        }
    }

    if (iChannelCnt <= i)
    {
        snprintf((char*)&stResPkg, sizeof(stResPkg), "Error: handle<%hd> has no channel:  %s <-----> %s",
                wHandleIndex, pszAddr1, pszAddr2);
        iRet = tappctrl_send_string((const char*)&stResPkg);
        return iRet;
    }

	tappctrl_fill_pkghead(&stResPkg.stHead, TAPP_BUS_SHOW_BY_ADDR_RES, tapp_controller_get_version());

    iRet = tappctrl_fill_channel_res(&stResPkg, wHandleIndex, wChannelIndex);
    if (0 > iRet)
    {
        iRet = tappctrl_send_string((const char*)&stResPkg);
    }
    else
    {
        iRet = tappctrl_send_pkg(&stResPkg);
    }

    return iRet;
}

static int tappctrl_proc_show_by_key(TAPPCTX* a_pstCtx, COMMANDINJECTPKG *a_pstCtrlPkg)
{
	int iRet = 0;
    extern TBUSGLOBAL g_stBusGlobal;
    LPCICHANNELHEAD pstDstHead;
	COMMANDINJECTPKG stResPkg;
    LPCHANNELHEAD pstSrcHead;
    LPCICHANNELVAR pstVar;
    TBUSCHANNEL stChannel;
    int i;

    TOS_UNUSED_ARG(a_pstCtx);

    iRet = tbus_channel_get(a_pstCtrlPkg->stBody.iShowByKeyReq, &stChannel);
	if (0 > iRet)
	{
        iRet = tappctrl_send_string("Error: can't open shared memory by specified key.");
		return iRet;
	}

    pstSrcHead = stChannel.pstHead;
    if (NULL == pstSrcHead)
    {
        iRet = tappctrl_send_string("Error: the head of channel management is null.");
    }


    pstDstHead = &stResPkg.stBody.stShowByKeyRes;
    for (i = 0; i < 2; i++)
    {
        pstVar = &pstDstHead->astQueueVar[i];
        //        pstVar->chGStart = pstSrcHead->astQueueVar[i].chGStart;
        //        pstVar->chPStart = pstSrcHead->astQueueVar[i].chPStart;
        pstVar->dwGet = pstSrcHead->astQueueVar[i].dwGet;
        pstVar->dwHead = pstSrcHead->astQueueVar[i].dwHead;
        pstVar->dwSize = pstSrcHead->astQueueVar[i].dwSize;
        pstVar->dwTail = pstSrcHead->astQueueVar[i].dwTail;
        pstVar->iRecvSeq = pstSrcHead->astQueueVar[i].iRecvSeq;
        pstVar->iSeq = pstSrcHead->astQueueVar[i].iSeq;
    }
    tbus_addr_ntop((TBUSADDR)pstSrcHead->astAddr[0],
            pstDstHead->aszAddr[0], sizeof(pstDstHead->aszAddr[0]));
    tbus_addr_ntop((TBUSADDR)pstSrcHead->astAddr[1],
            pstDstHead->aszAddr[1], sizeof(pstDstHead->aszAddr[0]));
    pstDstHead->dwAlignLevel = pstSrcHead->dwAlignLevel;
    pstDstHead->dwMagic = pstSrcHead->dwMagic;
    pstDstHead->dwVersion = pstSrcHead->dwVersion;
    pstDstHead->iShmID = (int32_t)pstSrcHead->iShmID;

	tappctrl_fill_pkghead(&stResPkg.stHead, TAPP_BUS_SHOW_BY_KEY_RES, tapp_controller_get_version());

    iRet = tappctrl_send_pkg(&stResPkg);

    return iRet;
}

int tappctrl_proc_pkg(TAPPCTX* a_pstCtx, void* a_pvArg)
{
	COMMANDINJECTPKG stCtrlPkg;
	int iRet = 0;

	if ((NULL == a_pstCtx))
	{
		return -1;
	}

	iRet = tappctrl_recv_pkg(&stCtrlPkg);
	if (0 != iRet)
	{
		return -2;
	}
	//printf("Recv one package ");
	switch (stCtrlPkg.stHead.nCmdID)
	{
	case TAPP_HELP_REQ:
		iRet = tappctrl_proc_helppkg(a_pstCtx, &stCtrlPkg);
		break;
	case TAPP_ECHO_REQ:
		{
			iRet = tappctrl_proc_echopkg(&stCtrlPkg);
		}
		break;
	case TAPP_CMDLINE_REQ:
		{
			iRet = tappctrl_proc_cmdlinepkg(a_pstCtx, a_pvArg, &stCtrlPkg);
		}
		break;
	case TAPP_BINARY_REQ:
		{
			iRet = tappctrl_proc_binpkg(a_pstCtx, a_pvArg, &stCtrlPkg);
		}
		break;
	case TAPP_GET_CONF_REQ:
		{
			iRet = tappctrl_proc_getconfkg(a_pstCtx, &stCtrlPkg);
		}
		break;
	case TAPP_LOG_SHOW_CONF_REQ:
		{
			iRet = tappctrl_proc_get_log_confkg(a_pstCtx, &stCtrlPkg);
		}
		break;
	case TAPP_GET_CUMULATE_REQ:
		{
			iRet = tappctrl_proc_getcomulatefkg(a_pstCtx, &stCtrlPkg);
		}
		break;
	case TAPP_GET_STATUS_REQ:
		{
			iRet = tappctrl_proc_getstatusfkg(a_pstCtx, &stCtrlPkg);
		}
		break;
	case TAPP_AUTHEN_REQ:
		{
			iRet = 	tappctrl_proc_authenfkg(&stCtrlPkg);
		}
		break;
	case TAPP_RELOAD_CONF_REQ:
		{
			iRet = 	tappctrl_proc_reloadfkg(&stCtrlPkg);
		}
		break;
    case TAPP_BUS_GLOBAL_REQ:
        {
            iRet = tappctrl_proc_bus_global(a_pstCtx, &stCtrlPkg);
        }
        break;
    case TAPP_BUS_HANDLE_REQ:
        {
            iRet = tappctrl_proc_bus_handle(a_pstCtx, &stCtrlPkg);
        }
        break;
    case TAPP_BUS_SHOW_CHNNL_REQ:
        {
            iRet = tappctrl_proc_bus_channel(a_pstCtx, &stCtrlPkg);
        }
        break;
    case TAPP_BUS_SHOW_BY_ADDR_REQ:
        {
            iRet = tappctrl_proc_show_by_addr(a_pstCtx, &stCtrlPkg);
        }
        break;
    case TAPP_BUS_SHOW_BY_KEY_REQ:
        {
            iRet = tappctrl_proc_show_by_key(a_pstCtx, &stCtrlPkg);
        }
        break;
	default:
		tlog_error(gs_stTapp.pstAppCatText, 0, 0, "unknown tapp control command ID<%d>\n", stCtrlPkg.stHead.nCmdID);
		break;
	}

	return iRet;
}
