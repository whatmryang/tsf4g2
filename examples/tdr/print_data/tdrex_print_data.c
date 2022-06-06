/**
*
* @file     tdrex_print_data.c
* @brief    ����Ԫ���ݿ��Կ��ӻ��ķ�ʽ��ӡ����
*
* @note TDR�ṩ�����API������: tdr_fprintf�� tdr_sprintf
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
#include "ex_net_protocol.h"

#if defined(_WIN32) || defined(_WIN64)
#pragma warning(disable:4996)
#endif


void ex_init_host_pkg_data(LPPKG pstHostData);


int main()
{
	LPTDRMETALIB pstLib = NULL;
	LPTDRMETA pstMeta;
	PKG stHostData;
	TDRDATA stHostInfo;
	int iRet;
	FILE *fp;

	/*����xml�ļ�����Ԫ���ݿ�*/
	iRet = tdr_create_lib_file(&pstLib, "net_protocol.xml", 1, stderr);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_create_lib_file failed by file %s, for %s\n", "net_protocol.xml", tdr_error_string(iRet));
		return iRet;
	}
	pstMeta = tdr_get_meta_by_name(pstLib, "Pkg");
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_get_meta_by_name Pkg, for %s\n", tdr_error_string(iRet));
		return iRet;
	}

	fp = fopen("pkg.txt", "w");
	if (NULL == fp)
	{
		return -1;
	}

	/*��ӡ����*/
	ex_init_host_pkg_data(&stHostData);
	stHostInfo.iBuff = sizeof(stHostData);
	stHostInfo.pszBuff = (char *)&stHostData;
	iRet = tdr_fprintf(pstMeta, fp, &stHostInfo, tdr_get_meta_current_version(pstMeta));
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_input_file failed , for %s\n", tdr_error_string(iRet));
		return iRet;
	}

	printf("the data:\n");
	iRet = tdr_fprintf(pstMeta, stdout, &stHostInfo, tdr_get_meta_current_version(pstMeta));
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_input_file failed , for %s\n", tdr_error_string(iRet));
		return iRet;
	}


	fclose(fp);

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
	strncpy(pstHostData->stBody.stLogin.szZone[0], "shenzhen", sizeof(pstHostData->stBody.stLogin.szZone));
	strncpy(pstHostData->stBody.stLogin.szZone[1], "beijing", sizeof(pstHostData->stBody.stLogin.szZone));

}
