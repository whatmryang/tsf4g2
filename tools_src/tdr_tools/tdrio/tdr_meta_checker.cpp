
/**
*
* @file     tdr_meta_checker.c
* @brief    check metafile and datafile's validation
*
* @note TDR提供的相关API函数有: tdr_output tdr_output_fp tdr_output_file  tdr_input tdr_input_fp tdr_input_file
*
* @author jackyai flyma
* @version 1.0
* @date 2009-9-10
*
*
* Copyright (c)  2009, 腾讯科技有限公司互动娱乐研发部
* All rights reserved.
*
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tdr/tdr.h"
#include "../../lib_src/tdr/tdr_metalib_kernel_i.h"

#if defined(_WIN32) || defined(_WIN64)
#pragma warning(disable:4996)
#endif

int main(int argc, const char* argv[])
{
	LPTDRMETALIB pstLib = NULL;
	LPTDRMETA pstMeta;
	TDRDATA stHostInfo;
    int inFormat, outFormat;

	int iRet = 0;

    if (argc < 6)
    {
        printf("Usage: %s metafile metaname datafile inFormat outFormat\n", argv[0]);
        printf("    inFormat | outFormat: ATTR or LIST\n");
        return -1;
    }

    if (0 == strcmp(argv[4], "ATTR"))
    {
        inFormat = TDR_XML_DATA_FORMAT_ATTR_ENTRY_NAME;
    }else
    {
        inFormat = TDR_XML_DATA_FORMAT_LIST_ENTRY_NAME;
    }

    if (0 == strcmp(argv[5], "ATTR"))
    {
        outFormat = TDR_XML_DATA_FORMAT_ATTR_ENTRY_NAME;
    }else
    {
        outFormat = TDR_XML_DATA_FORMAT_LIST_ENTRY_NAME;
    }
	/*根据xml文件创建元数据库*/
	iRet = tdr_create_lib_file(&pstLib, argv[1], 1, stderr);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_create_lib_file failed by file %s, for %s\n", argv[1], tdr_error_string(iRet));
		return iRet;
	}
	pstMeta = tdr_get_meta_by_name(pstLib, argv[2]);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_get_meta_by_name %s, for %s\n", argv[2], tdr_error_string(iRet));
		return iRet;
	}

	/*从XML格式的文件中读入数据*/
    stHostInfo.iBuff = 4 + 50 * pstMeta->iHUnitSize;
    stHostInfo.pszBuff = (char*)malloc((size_t)stHostInfo.iBuff);
    if (NULL == stHostInfo.pszBuff)
    {
        printf("alloc failed\n");
        return -1;
    }

	iRet = tdr_input_file(pstMeta, &stHostInfo, argv[3],
		tdr_get_meta_current_version(pstMeta), inFormat);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_input_file failed , for %s\n", tdr_error_string(iRet));
        if (NULL != stHostInfo.pszBuff)
        {
            free(stHostInfo.pszBuff);
            stHostInfo.pszBuff = NULL;
        }
		return iRet;
	}

	/*本地数据输出到XML格式的文件中*/
    stHostInfo.iBuff = 4 + 50 * pstMeta->iHUnitSize;
	iRet = tdr_output_file(pstMeta, "output.xml", &stHostInfo, tdr_get_meta_current_version(pstMeta), outFormat);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("tdr_output_file failed , for %s\n", tdr_error_string(iRet));
        if (NULL != stHostInfo.pszBuff)
        {
            free(stHostInfo.pszBuff);
            stHostInfo.pszBuff = NULL;
        }
		return iRet;
	}


    if (NULL != stHostInfo.pszBuff)
    {
        free(stHostInfo.pszBuff);
        stHostInfo.pszBuff = NULL;
    }
	/*使用完毕释放元数据的空间*/
	tdr_free_lib(&pstLib);


	return iRet;
}
