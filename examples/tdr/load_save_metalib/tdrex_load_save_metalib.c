/**
*
* @file     tdrex_load_save_metalib.c  
* @brief    ��Ԫ���ݿⱣ���ڶ���֧�ļ���,���Ӷ���֧�ļ��м���Ԫ���ݿ�
* @note TDR�ṩ�����п��ƹ���xml2dr��XML�����ļ�ת����Ԫ���ݿ�����ƽ����ļ�,ʵ��Ӧ��ͨ����������xml2dr
*��xml�����ļ�ת���ɶ������ļ�,Ȼ����ʵ��Ӧ���м��ش˶������ļ�,�Ӷ��õ��ڴ��ʽ��Ԫ���ݿ�
*
* @note ͬʱTDR�ṩ��tdr_load_metalib tdr_load_metalib_fp tdr_save_metalib tdr_save_metalib_fp�ĸ�API����
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



int main()
{
	LPTDRMETALIB pstLib = NULL;
	LPTDRMETALIB pstLibFromFile = NULL;
	int iRet;

	/*����xml�ļ�����Ԫ���ݿ�*/
	iRet = tdr_create_lib_file(&pstLib, "net_protocol.xml", 1, stderr);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_create_lib_file failed by file %s, for %s\n", "net_protocol.xml", tdr_error_string(iRet));
		return iRet;
	}

	
	/*��Ԫ�ؿⱣ�浽�������ļ���*/
	iRet = tdr_save_metalib(pstLib, "net_protocol.dr");
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_save_metalib failed  for %s\n", tdr_error_string(iRet));
		return iRet;
	}


	/*�������Ƹ�ʽ��Ԫ������������ص��ڴ���*/
	iRet = tdr_load_metalib(&pstLibFromFile, "net_protocol.dr");
	//iRet = tdr_load_metalib(&pstLibFromFile, "a.tdr");
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_load_metalib failed  for %s\n", tdr_error_string(iRet));
		return iRet;
	}

	tdr_dump_metalib_file(pstLibFromFile, "a.txt");

	/*ʹ������ͷ�Ԫ���ݵĿռ�*/
	tdr_free_lib(&pstLib);
	tdr_free_lib(&pstLibFromFile);

	return iRet;
}
