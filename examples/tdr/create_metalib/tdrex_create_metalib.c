/**
*
* @file     tdrex_create_metalib.c   
* @brief    根据XML描述文件生成元数据描述库
* @note TDR提供了tdr_create_lib tdr_create_lib_fp tdr_create_lib_file tdr_create_lib_multifile四个API函数
*将XML描述文件转换成元数据描述库.当创建的元数据库使用完毕,必须调用tdr_free_lib释放元数据库的空间.
* 
*@noet 同时TDR提供了tdr_save_xml_file tdr_save_xml_fp tdr_save_xml 三个API接口将内存格式的元数据库转换成XML格式
*      的元数据描述文件
* @author jackyai  
* @version 1.0
* @date 2007-06-14 
*
*
* Copyright (c)  2007, 腾讯科技有限公司互动娱乐研发部
* All rights reserved.
*
*/

#include <stdio.h>
#include "tdr/tdr.h"

int main()
{
	LPTDRMETALIB pstLib = NULL;
	int iRet;

	/*根据xml文件创建元数据库*/
	iRet = tdr_create_lib_file(&pstLib, "net_protocol.xml", 1, stderr);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_create_lib_file failed by file %s, for %s\n", "net_protocol.xml", tdr_error_string(iRet));
		return iRet;
	}

	/*将元数据库保存到XML文件中*/
	iRet = tdr_save_xml_file(pstLib, "new_net_protocol.xml");
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_save_xml_file failed by file %s, for %s\n", "net_protocol.xml", tdr_error_string(iRet));
		return iRet;
	}

	
	/*其他应用.........*/


	/*使用完毕释放元数据的空间*/
	tdr_free_lib(&pstLib);

	return 0;
}
