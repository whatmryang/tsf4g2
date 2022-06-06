/**
*
* @file     tdrex_net_pack_unpack.c
* @brief    利用元数据库对数据结构进行网络编码和解码
*
* @note TDR提供的相关API函数有: tdr_hton tdr_ntoh
* @执行本例程前已经利用网络编解码元数据XML生成了相应的.h文件
*
* @author jackyai
* @version 1.0
* @date 2007-06-15
*
*
* Copyright (c)  2007, 腾讯科技有限公司互动娱乐研发部
* All rights reserved.
*
*/

#include <stdio.h>
#include <string.h>
#include "tdr/tdr.h"
#include "net_protocol.h"
void ex_init_host_pkg_data(LPPKG pstHostData);

#if defined(_WIN32) || defined(_WIN64)
#pragma warning(disable:4996)
#endif

int main()
{
	LPTDRMETALIB pstLib = NULL;
	LPTDRMETA pstMeta;
	PKG stHostData;
	TDRDATA stHostInfo;
	char szNetBuff[32700];
	TDRDATA stNetInfo;
	PKG stDataFromNet;
	int iRet;

	/*根据xml文件创建元数据库*/
	iRet = tdr_load_metalib(&pstLib, "net_protocol.tdr");
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_load_metalib failed by file %s, for %s\n", "net_protocol.tdr", tdr_error_string(iRet));
		return iRet;
	}
	pstMeta = tdr_get_meta_by_name(pstLib, "Pkg");
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_get_meta_by_name Pkg, for %s\n", tdr_error_string(iRet));
		return iRet;
	}

	ex_init_host_pkg_data(&stHostData);

	/*本地数据编码成网络数据*/
	stHostInfo.iBuff = sizeof(stHostData);
	stHostInfo.pszBuff = (char *)&stHostData;
	stNetInfo.iBuff = sizeof(szNetBuff);
	stNetInfo.pszBuff = &szNetBuff[0];
	iRet = tdr_hton(pstMeta, &stNetInfo, &stHostInfo, tdr_get_meta_current_version(pstMeta));
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_hton failed , for %s\n", tdr_error_string(iRet));
		return iRet;
	}


	/*网络数据解码成本地数据*/
	stHostInfo.iBuff = sizeof(stDataFromNet);
	stHostInfo.pszBuff = (char *)&stDataFromNet;
	iRet = tdr_ntoh(pstMeta, &stHostInfo, &stNetInfo, tdr_get_meta_current_version(pstMeta));
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_ntoh failed , for %s\n", tdr_error_string(iRet));
		return iRet;
	}


	pstMeta = tdr_get_meta_by_name(pstLib, "PkgHead");
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_get_meta_by_name Pkg, for %s\n", tdr_error_string(iRet));
		return iRet;
	}
	stHostInfo.iBuff = sizeof(stHostData);
	stHostInfo.pszBuff = (char *)&stHostData;
	stNetInfo.iBuff = sizeof(szNetBuff);
	stNetInfo.pszBuff = &szNetBuff[0];
	iRet = tdr_hton(pstMeta, &stNetInfo, &stHostInfo, tdr_get_meta_current_version(pstMeta));
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_hton failed , for %s\n", tdr_error_string(iRet));
		return iRet;
	}

	stHostInfo.iBuff = sizeof(stDataFromNet);
	stHostInfo.pszBuff = (char *)&stDataFromNet;
	iRet = tdr_ntoh(pstMeta, &stHostInfo, &stNetInfo, tdr_get_meta_current_version(pstMeta));
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_ntoh failed , for %s\n", tdr_error_string(iRet));
		return iRet;
	}

	/*使用完毕释放元数据的空间*/
	tdr_free_lib(&pstLib);
	return iRet;
}

void ex_init_host_pkg_data(LPPKG pstHostData)
{
	pstHostData->stHead.dwMsgid = 100;
	pstHostData->stHead.nCmd = 0;
	pstHostData->stHead.nVersion = 9;
	pstHostData->stHead.nMagic = 0x3344;
	tdr_str_to_tdrdatetime(&pstHostData->stHead.tTime, "2007-05-21 12:23:45");

	strncpy(pstHostData->stBody.stLogin.szName, "haha", sizeof(pstHostData->stBody.stLogin.szName));
	strncpy(pstHostData->stBody.stLogin.szPass, "secret", sizeof(pstHostData->stBody.stLogin.szPass));
	strncpy(pstHostData->stBody.stLogin.szZone, "shenzhen", sizeof(pstHostData->stBody.stLogin.szZone));
}
