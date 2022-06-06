/**
*
* @file     tdr_dbORMapping.c
* @brief    TDR���ݿ�����ϵӳ��ʾ������
*
*
* @author jackyai
* @version 1.0
* @date 2007-07-31
*
*
* Copyright (c)  2007, ��Ѷ�Ƽ����޹�˾���������з���
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

LPTDRMETALIB g_pDBLib = NULL; /*Ԫ���ݿ��ָ��*/
TDRDBHANDLE g_hDBHandle;		/*DR���ݿ�Ự���*/

static void init_extend_table_data(LPTESTEXTENDTABLE pstAllData);




int main()
{
	int iRet;
	TESTEXTENDTABLE stTestData;

	/*�������������Ԫ���ݿ�*/
	iRet = tdr_create_lib_file(&g_pDBLib, "tdr_database.xml", 1, stderr);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("faild create metalib lib by file<%s>: %s\n", "tdr_database.xml",
			tdr_error_string(iRet));
		return iRet;
	}

	/*��DB����Ự���*/
	iRet = tdrex_open_dbhandle(&g_hDBHandle, "tdr_dbms_conf.xml");
	if (0 != iRet)
	{
		return 0;
	}


	/*����Ҫ���������*/
	init_extend_table_data(&stTestData);


	/*��������*/
	insert_data(g_hDBHandle, g_pDBLib, &stTestData);


	/*��������*/
	update_data(g_hDBHandle, g_pDBLib, &stTestData);

	/*�������*/
	select_data(g_hDBHandle, g_pDBLib, &stTestData);


	/*����ִ�м�������*/
	select_data_quickly(g_hDBHandle, g_pDBLib, &stTestData);

	/*ɾ������*/
	delete_data(g_hDBHandle, g_pDBLib, &stTestData);


	/*�ͷ���Դ*/
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

