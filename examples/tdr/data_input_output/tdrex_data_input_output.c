/**
*
* @file     tdrex_data_input_output.c
* @brief    ����Ԫ���ݿ���XML��ʽ������������
*
* @note TDR�ṩ�����API������: tdr_output tdr_output_fp tdr_output_file  tdr_input tdr_input_fp tdr_input_file
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
#include "tlogconf.h"
void test_input_tlog_conf();

#if defined(_WIN32) || defined(_WIN64)
#pragma warning(disable:4996)
#endif

int main()
{
	LPTDRMETALIB pstLib = NULL;
	LPTDRMETA pstMeta;
	PKG stHostData;
	TDRDATA stHostInfo;
	int iRet;

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


	/*��XML��ʽ���ļ��ж�������*/
	stHostInfo.iBuff = sizeof(stHostData);
	stHostInfo.pszBuff = (char *)&stHostData;
	iRet = tdr_input_file(pstMeta, &stHostInfo, "pkg.xml",
		tdr_get_meta_current_version(pstMeta), TDR_XML_DATA_FORMAT_LIST_ENTRY_NAME);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_input_file failed , for %s\n", tdr_error_string(iRet));
		return iRet;
	}

	/*�������������XML��ʽ���ļ���*/
	iRet = tdr_output_file(pstMeta, "pkg2.xml", &stHostInfo, 10, TDR_XML_DATA_FORMAT_ATTR_ENTRY_NAME);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_output_file failed , for %s\n", tdr_error_string(iRet));
		return iRet;
	}

	/*ʹ������ͷ�Ԫ���ݵĿռ�*/
	tdr_free_lib(&pstLib);


	return 0;
}
