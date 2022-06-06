/**
*
* @file     tdrex_insert_metalib_in_apps.c  
* @brief    ��Ԫ���ݿ�ֱ��Ƕ�뵽Ӧ�ó�����
* @note TDR�ṩ�����п��ƹ���xml2c��XML�����ļ�ת������c�ļ���ʽ�����Ԫ���ݿ�,
*	ʵ��Ӧ��ֱ�ӽ���.c�ļ���ӵ�������,Ȼ��.c�ļ����Ծ�̬�����ʽ�����Ԫ���ݿ�ǿ��
*	����ת�����ڴ��ʽ��Ԫ���ݿ⼴��ʹ��.
*@note TDR�ṩ��tdr_metalib_to_cfile API�ӿڽ��ڴ��ʽ��Ԫ���ݿ�ת����.c�ļ�
*
*�ڱ������Ѿ�ͨ��xml2c���߽�tdr_sort.xml�ļ�ת����tdr_sort.c�ļ�
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


extern unsigned char g_szMetalib_tdrsort[];

int main()
{
	LPTDRMETALIB pstLib = NULL;
	int iRet;

	/*����.c�ļ�����Ԫ���ݿ�,ֻ��ֱ������ת������*/
	pstLib = (LPTDRMETALIB)g_szMetalib_tdrsort;



	/*���ڿ���ֱ��ʹ��Ԫ���ݿ���.....*/

	/*Ӧ��һ: �ڴ��ʽmetalibת��.c�ļ�*/
	iRet = tdr_metalib_to_cfile(pstLib, "tdrex_insert_metalib.c");
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_metalib_to_cfile failed, for %s\n", tdr_error_string(iRet));
	}


	/*��һ�ַ�ʽ*/
	iRet = tdr_create_lib_file(&pstLib, "tdr_sort.xml", 1, stderr);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_create_lib_file failed by file %s, for %s\n", "tdr_sort.xml", tdr_error_string(iRet));
		return iRet;
	}

	iRet = tdr_metalib_to_cfile(pstLib, "tdr_sort.c");
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_metalib_to_cfile failed, for %s\n", tdr_error_string(iRet));
	}

	tdr_free_lib(&pstLib);

	return iRet;

}

