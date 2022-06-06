/**
*
* @file     tdrex_create_metalib.c   
* @brief    ����XML�����ļ�����Ԫ����������
* @note TDR�ṩ��tdr_create_lib tdr_create_lib_fp tdr_create_lib_file tdr_create_lib_multifile�ĸ�API����
*��XML�����ļ�ת����Ԫ����������.��������Ԫ���ݿ�ʹ�����,�������tdr_free_lib�ͷ�Ԫ���ݿ�Ŀռ�.
* 
*@noet ͬʱTDR�ṩ��tdr_save_xml_file tdr_save_xml_fp tdr_save_xml ����API�ӿڽ��ڴ��ʽ��Ԫ���ݿ�ת����XML��ʽ
*      ��Ԫ���������ļ�
* @author jackyai  
* @version 1.0
* @date 2007-06-14 
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
	int iRet;

	/*����xml�ļ�����Ԫ���ݿ�*/
	iRet = tdr_create_lib_file(&pstLib, "net_protocol.xml", 1, stderr);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_create_lib_file failed by file %s, for %s\n", "net_protocol.xml", tdr_error_string(iRet));
		return iRet;
	}

	/*��Ԫ���ݿⱣ�浽XML�ļ���*/
	iRet = tdr_save_xml_file(pstLib, "new_net_protocol.xml");
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_save_xml_file failed by file %s, for %s\n", "net_protocol.xml", tdr_error_string(iRet));
		return iRet;
	}

	
	/*����Ӧ��.........*/


	/*ʹ������ͷ�Ԫ���ݵĿռ�*/
	tdr_free_lib(&pstLib);

	return 0;
}
