/**
*
* @file     tdr_dbORMapping.c
* @brief    TDR数据库对象关系映射示例程序
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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "tdr_dbORMapping.h"

#if defined(_WIN32) || defined(_WIN64)
#pragma warning(disable:4996)
#endif

LPTDRMETALIB g_pDBLib = NULL; /*元数据库的指针*/
TDRDBHANDLE g_hDBHandle;		/*DR数据库会话句柄*/

static void init_extend_table_data(LPTESTEXTENDTABLE pstAllData);




int main()
{
	int iRet;
	TESTEXTENDTABLE stTestData;

	/*创建处理所需的元数据库*/
	iRet = tdr_create_lib_file(&g_pDBLib, "tdr_database.xml", 1, stderr);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("faild create metalib lib by file<%s>: %s\n", "tdr_database.xml",
			tdr_error_string(iRet));
		return iRet;
	}

	/*打开DB处理会话句柄*/
	iRet = tdrex_open_dbhandle(&g_hDBHandle, "tdr_dbms_conf.xml");
	if (0 != iRet)
	{
		return 0;
	}


	/*构造要处理的数据*/
	init_extend_table_data(&stTestData);


	/*插入数据*/
	insert_data(g_hDBHandle, g_pDBLib, &stTestData);


	/*更新数据*/
	update_data(g_hDBHandle, g_pDBLib, &stTestData);

	/*检查数据*/
	select_data(g_hDBHandle, g_pDBLib, &stTestData);


	/*快速执行检索命令*/
	select_data_quickly(g_hDBHandle, g_pDBLib, &stTestData);

	/*删除数据*/
	delete_data(g_hDBHandle, g_pDBLib, &stTestData);


	/*释放资源*/
	tdr_free_lib(&g_pDBLib);
	tdr_close_dbhanlde(&g_hDBHandle);

#if defined(_WIN32) || defined(_WIN64)
	getchar();
#endif

	return iRet;
}



void init_extend_table_data(LPTESTEXTENDTABLE pstAllData)
{
	unsigned int i;
	char szBuf[32];

	srand((unsigned int)time(NULL));

	pstAllData->iSeq = rand();

	pstAllData->stGame.iGameid = rand();
	pstAllData->stGame.iGamepoint = rand();
	for (i = 0; i < sizeof(pstAllData->stGame.szReserve); i++)
	{
		pstAllData->stGame.szReserve[i] = i % 16;
	}

	pstAllData->stItem.dwUin = (unsigned int)rand();
	pstAllData->stItem.iLongnum = rand();
	pstAllData->stItem.iOffLineNum = rand();
	pstAllData->stItem.llMoney = rand() * rand();
	pstAllData->stItem.stItems.dwGameid = (unsigned int)rand();
	pstAllData->stItem.stItems.dwItemid = (unsigned int)rand();

	sprintf(szBuf, "%4d-%2d-%2d %2d:%2d:%2d", rand()%10000, rand()%13, rand()%28,rand()%24,rand()%60,rand()%60);
	tdr_str_to_tdrdatetime(&pstAllData->stItem.stItems.tBuytime, szBuf);

	sprintf(szBuf, "%4d-%2d-%2d %2d:%2d:%2d", rand()%10000, rand()%13, rand()%28,rand()%24,rand()%60,rand()%60);
	tdr_str_to_tdrdatetime(&pstAllData->stItem.stItems.tValidtime, szBuf);

	sprintf(szBuf, "%4d-%2d-%2d %2d:%2d:%2d", rand()%10000, rand()%13, rand()%28,rand()%24,rand()%60,rand()%60);
	tdr_str_to_tdrdatetime(&pstAllData->stItem.tLastlogintime, szBuf);

	pstAllData->stUserAttachInfo.iAge = rand();
	pstAllData->stUserAttachInfo.iLevel = rand();
	for (i = 0; i< 4; i++)
	{
		pstAllData->stUserAttachInfo.reserver[i] = rand();
	}
}

