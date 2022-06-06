/**
*
* @file     tdrex_insert_metalib_in_apps.c  
* @brief    将元数据库直接嵌入到应用程序中
* @note TDR提供了运行控制工具xml2c将XML描述文件转换成以c文件格式保存的元数据库,
*	实际应用直接将此.c文件添加到工程中,然后将.c文件中以静态数组格式保存的元数据库强制
*	类型转换成内存格式的元数据库即可使用.
*@note TDR提供了tdr_metalib_to_cfile API接口将内存格式的元数据库转换成.c文件
*
*在本例中已经通过xml2c工具将tdr_sort.xml文件转换成tdr_sort.c文件
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


extern unsigned char g_szMetalib_tdrsort[];

int main()
{
	LPTDRMETALIB pstLib = NULL;
	int iRet;

	/*根据.c文件创建元数据库,只需直接类型转换即可*/
	pstLib = (LPTDRMETALIB)g_szMetalib_tdrsort;



	/*现在可以直接使用元数据库了.....*/

	/*应用一: 内存格式metalib转成.c文件*/
	iRet = tdr_metalib_to_cfile(pstLib, "tdrex_insert_metalib.c");
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_metalib_to_cfile failed, for %s\n", tdr_error_string(iRet));
	}


	/*另一种方式*/
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

