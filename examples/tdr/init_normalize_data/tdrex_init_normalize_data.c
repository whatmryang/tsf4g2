/**
*
* @file     tdrex_init_normalize_data.c
* @brief    ����Ԫ���ݿ�����ݽṹ���г�ʼ��������
*
* @note TDR�ṩ�����API������: tdr_init tdr_normalize
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
#include "tdr/tdr.h"
#include "test_defaultvalue.h"


int main()
{
	LPTDRMETALIB pstLib = NULL;
	LPTDRMETA pstMeta;
	ALLBASETYPEDATA stInitData;
	TDRDATA stHostData;
	int iRet;

	/*����xml�ļ�����Ԫ���ݿ�*/
	iRet = tdr_create_lib_file(&pstLib, "test_defaultvalue.xml", 1, stderr);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_create_lib_file failed by file %s, for %s\n", "net_protocol.xml", tdr_error_string(iRet));
		return iRet;
	}


	pstMeta = tdr_get_meta_by_name(pstLib, "AllBaseTypeData");
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_get_meta_by_name AllBaseTypeData, for %s\n", tdr_error_string(iRet));
		return iRet;
	}


	/*���ݳ�ʼ��*/
	stHostData.iBuff = sizeof(stInitData);
	stHostData.pszBuff = (char *)&stInitData;
	iRet = tdr_init(pstMeta, &stHostData, tdr_get_meta_current_version(pstMeta));
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_create_lib_file failed for %s\n", tdr_error_string(iRet));
		return iRet;
	}


	/*��������*/
	iRet = tdr_normalize(pstMeta, &stHostData, tdr_get_meta_current_version(pstMeta));
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_create_lib_file failed for %s\n", tdr_error_string(iRet));
		return iRet;
	}

	/*ʹ������ͷ�Ԫ���ݵĿռ�*/
	tdr_free_lib(&pstLib);
	return iRet;

}
