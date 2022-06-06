/**
*
* @file     tdrex_load_save_metalib.c  
* @brief    将元数据库保存在二进支文件中,及从二进支文件中加载元数据库
* @note TDR提供了运行控制工具xml2dr将XML描述文件转换成元数据库二进制进制文件,实际应用通常是先利用xml2dr
*将xml描述文件转换成二进制文件,然后在实际应用中加载此二进制文件,从而得到内存格式的元数据库
*
* @note 同时TDR提供了tdr_load_metalib tdr_load_metalib_fp tdr_save_metalib tdr_save_metalib_fp四个API函数
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
#include "tdr/tdr.h"



int main()
{
	LPTDRMETALIB pstLib = NULL;
	LPTDRMETALIB pstLibFromFile = NULL;
	int iRet;

	/*根据xml文件创建元数据库*/
	iRet = tdr_create_lib_file(&pstLib, "net_protocol.xml", 1, stderr);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_create_lib_file failed by file %s, for %s\n", "net_protocol.xml", tdr_error_string(iRet));
		return iRet;
	}

	
	/*将元素库保存到二进制文件中*/
	iRet = tdr_save_metalib(pstLib, "net_protocol.dr");
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_save_metalib failed  for %s\n", tdr_error_string(iRet));
		return iRet;
	}


	/*将二进制格式的元数据描述库加载到内存中*/
	iRet = tdr_load_metalib(&pstLibFromFile, "net_protocol.dr");
	//iRet = tdr_load_metalib(&pstLibFromFile, "a.tdr");
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_load_metalib failed  for %s\n", tdr_error_string(iRet));
		return iRet;
	}

	tdr_dump_metalib_file(pstLibFromFile, "a.txt");

	/*使用完毕释放元数据的空间*/
	tdr_free_lib(&pstLib);
	tdr_free_lib(&pstLibFromFile);

	return iRet;
}
