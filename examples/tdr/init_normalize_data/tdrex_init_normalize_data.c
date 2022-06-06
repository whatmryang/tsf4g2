/**
*
* @file     tdrex_init_normalize_data.c
* @brief    利用元数据库对数据结构进行初始化和正则化
*
* @note TDR提供的相关API函数有: tdr_init tdr_normalize
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
#include "test_defaultvalue.h"


int main()
{
	LPTDRMETALIB pstLib = NULL;
	LPTDRMETA pstMeta;
	ALLBASETYPEDATA stInitData;
	TDRDATA stHostData;
	int iRet;

	/*根据xml文件创建元数据库*/
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


	/*数据初始化*/
	stHostData.iBuff = sizeof(stInitData);
	stHostData.pszBuff = (char *)&stInitData;
	iRet = tdr_init(pstMeta, &stHostData, tdr_get_meta_current_version(pstMeta));
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_create_lib_file failed for %s\n", tdr_error_string(iRet));
		return iRet;
	}


	/*数据正则化*/
	iRet = tdr_normalize(pstMeta, &stHostData, tdr_get_meta_current_version(pstMeta));
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_create_lib_file failed for %s\n", tdr_error_string(iRet));
		return iRet;
	}

	/*使用完毕释放元数据的空间*/
	tdr_free_lib(&pstLib);
	return iRet;

}
