/**
*
* @file     tdrex_net_pack_unpack.c
* @brief    ����Ԫ���ݿ�����ݽṹ�����������ͽ���
*
* @note TDR�ṩ�����API������: tdr_hton tdr_ntoh
* @ִ�б�����ǰ�Ѿ�������������Ԫ����XML��������Ӧ��.h�ļ�
*
* @author jackyai
* @version 1.0
* @date 2007-06-15
*
*
* Copyright (c)  2007, ��Ѷ�Ƽ����޹�˾���������з���
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

	/*����xml�ļ�����Ԫ���ݿ�*/
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

	/*�������ݱ������������*/
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


	/*�������ݽ���ɱ�������*/
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

	/*ʹ������ͷ�Ԫ���ݵĿռ�*/
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
