/**
*
* @file     tdrex_data_input_output.c
* @brief    利用元数据库以XML格式输入和输出数据
*
* @note TDR提供的相关API函数有: tdr_output tdr_output_fp tdr_output_file  tdr_input tdr_input_fp tdr_input_file
* @执行本例程前已经利用网络编解码元数据XML生成了相应的.h文件
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

	/*根据xml文件创建元数据库*/
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


	/*从XML格式的文件中读入数据*/
	stHostInfo.iBuff = sizeof(stHostData);
	stHostInfo.pszBuff = (char *)&stHostData;
	iRet = tdr_input_file(pstMeta, &stHostInfo, "pkg.xml",
		tdr_get_meta_current_version(pstMeta), TDR_XML_DATA_FORMAT_LIST_ENTRY_NAME);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_input_file failed , for %s\n", tdr_error_string(iRet));
		return iRet;
	}

	/*本地数据输出到XML格式的文件中*/
	iRet = tdr_output_file(pstMeta, "pkg2.xml", &stHostInfo, 10, TDR_XML_DATA_FORMAT_ATTR_ENTRY_NAME);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_output_file failed , for %s\n", tdr_error_string(iRet));
		return iRet;
	}

	/*使用完毕释放元数据的空间*/
	tdr_free_lib(&pstLib);


	return 0;
}
