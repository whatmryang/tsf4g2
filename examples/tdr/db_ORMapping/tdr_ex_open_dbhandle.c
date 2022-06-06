/**
*
* @file     tdr_ex_open_dbhandle.c 
* @brief    打开DB处理会话句柄
*
* 
* @author jackyai  
* @version 1.0
* @date 2007-07-31 
*
*
* Copyright (c)  2007, 腾讯科技有限公司互动娱乐研发部
* All rights reserved.
*
*/
#include <assert.h>
#include "tdr_dbORMapping.h"

int tdrex_open_dbhandle(OUT TDRDBHANDLE *a_phDHandle, IN const char *a_pszDBMSConf)
{
	int iRet = TDR_SUCCESS;
	LPTDRMETALIB pstLib;
	TDRDBMS stDBMS;
	LPTDRMETA pstMeta;
	TDRDATA stData;
	char szError[1024];

	assert(NULL != a_phDHandle);
	assert(NULL != a_pszDBMSConf);

	iRet = tdr_create_lib_file(&pstLib, "tdr_database.xml", 1, stderr);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("faild create metalib lib by file<%s>: %s\n", "tdr_database.xml", 
			tdr_error_string(iRet));
		return iRet;
	}

	pstMeta = tdr_get_meta_by_name(pstLib, "DBMSInfo");
	if (NULL == pstMeta)
	{
		printf("failed to get meta by name: %s\n", "DBMSInfo");
		return -1;
	}

	stData.iBuff = sizeof(stDBMS);
	stData.pszBuff = (char *)&stDBMS;
	iRet = tdr_input_file(pstMeta, &stData, a_pszDBMSConf, 
		tdr_get_meta_current_version(pstMeta), TDR_IO_NEW_XML_VERSION);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("faild tdr_input_file file<%s>: %s\n", a_pszDBMSConf, 
			tdr_error_string(iRet));
		return iRet;
	}

	/*打开db处理句柄*/
	iRet = tdr_open_dbhanlde(a_phDHandle, &stDBMS, &szError[0]);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("faild open dbhanlde: %s %s\n", tdr_error_string(iRet), szError);
		return iRet;
	}

	return iRet;
}
