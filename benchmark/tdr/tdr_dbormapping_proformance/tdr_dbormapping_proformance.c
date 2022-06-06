/**
*
* @file     tdr_dbormapping_proformance.c 
* @brief    ����TDR�����ϵӳ��ģ�������
*
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
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>




#include "tgetopt.h"
#include "tdr/tdr.h"
#include "tdr_database.h"

#if defined(_WIN32) || defined(_WIN64)
#include "pal/ttime.h"

# ifdef _DEBUG
#  define LIB_D    "d"
# else
#  define LIB_D
# endif /* _DEBUG */

# if defined(LIB_D)
# pragma comment( lib, "libpal_"  LIB_D ".lib" )
# else
# pragma comment( lib, "libpal.lib" )
# endif

#pragma comment(lib, "winmm.lib")

#else
#include <sys/time.h>
#endif /*#if defined(_WIN32) || defined(_WIN64)*/

#define  DEFAULT_LOOP_TIMES		100000
#define TDR_EX_MAN_SQL_LEN	100000	/**<SQL���������󳤶�*/

struct tagOption 
{
	int iLoop;     /**<�ظ���/����Ĵ���*/	
};
typedef struct tagOption   TDROPTION;
typedef struct tagOption *LPTDROPTION;



static void tdr_show_help(const char *a_pszObj);
void init_extend_table_data(LPTESTEXTENDTABLE pstAllData);
static int open_dbhandle(OUT TDRDBHANDLE *a_phDHandle, IN const char *a_pszDBMSConf);
static void insert_proformance(int iloop, TDRDBHANDLE hDBHandle, LPTDRMETA pstMeta, int iVersion, LPTESTEXTENDTABLE pstTestData, int iSeqBase);
static void update_proformance(int iloop, TDRDBHANDLE hDBHandle, LPTDRMETA pstMeta, int iVersion, LPTESTEXTENDTABLE pstTestData, int iSeqBase);
static void select_proformance(int iloop, TDRDBHANDLE hDBHandle, LPTDRMETA pstMeta, int iVersion, LPTESTEXTENDTABLE pstTestData, int iSeqBase);
static void delete_proformance(int iloop, TDRDBHANDLE hDBHandle, LPTDRMETA pstMeta, int iVersion, LPTESTEXTENDTABLE pstTestData, int iSeqBase);
static void gen_update_dml_proformance(int iloop, TDRDBHANDLE hDBHandle, LPTDRMETA pstMeta, int iVersion, LPTESTEXTENDTABLE pstTestData, int iSeqBase);

/**�������й��ߵ�ѡ��
*@retval 0 ��ȷ��������ѡ��
*@retval >0 ��ȷ��������ѡ��,��ָʾ�����й��߲��ؼ���ִ��
*@retval <0 ����ѡ��ʧ��
*/
static int tdr_parse_option(OUT LPTDROPTION a_pstOption, IN int a_argc, IN char *a_argv[]);



int main(int argc, char *argv[])
{
	int iRet;
	TDROPTION stOption;
	TESTEXTENDTABLE stTestData;
	LPTDRMETALIB pstDBLib = NULL; /*Ԫ���ݿ��ָ��*/
	TDRDBHANDLE hDBHandle;		/*DR���ݿ�Ự���*/
	LPTDRMETA pstMeta;
	int iSeqBase;	
	int iVersion;
	

	memset(&stOption, 0, sizeof(stOption));
	stOption.iLoop = DEFAULT_LOOP_TIMES;
	iRet = tdr_parse_option(&stOption, argc, argv);	
	if (0 != iRet)
	{
		return iRet;
	}

	/*�������������Ԫ���ݿ�*/
	iRet = tdr_create_lib_file(&pstDBLib, "tdr_database.xml", 1, stderr);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("faild create metalib lib by file<%s>: %s\n", "tdr_database.xml", 
			tdr_error_string(iRet));
		return iRet;
	}
	pstMeta = tdr_get_meta_by_name(pstDBLib, "TestExtendTable");
	if (NULL == pstMeta)
	{
		printf("failed to get meta by name %s", "TestExtendTable");
		return -1;
	}

	/*��DB����Ự���*/
	iRet = open_dbhandle(&hDBHandle, "tdr_dbms_conf.xml");
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("faild open_dbhandle by file<%s>: %s\n", "tdr_dbms_conf.xml", 
			tdr_error_string(iRet));
		return iRet;
	}


	srand((unsigned int )time(NULL));
	iSeqBase = rand();
	init_extend_table_data(&stTestData);
	stTestData.stItem.stItems.dwGameid = (unsigned int)rand();
	stTestData.stItem.stItems.dwItemid = (unsigned int)rand();
	stTestData.stItem.dwUin = (unsigned int)rand();
	iVersion = tdr_get_meta_current_version(pstMeta);

	/*����*/
	insert_proformance(stOption.iLoop, hDBHandle, pstMeta, iVersion, &stTestData, iSeqBase);

	/*����*/
	update_proformance(stOption.iLoop, hDBHandle, pstMeta, iVersion, &stTestData, iSeqBase);
	gen_update_dml_proformance(stOption.iLoop, hDBHandle, pstMeta, iVersion, &stTestData, iSeqBase);

	/*��ѯ*/
	select_proformance(stOption.iLoop, hDBHandle, pstMeta, iVersion, &stTestData, iSeqBase);

	/*ɾ��*/
	delete_proformance(stOption.iLoop, hDBHandle, pstMeta, iVersion, &stTestData, iSeqBase);
	
	/*�ͷ���Դ*/
	tdr_free_lib(&pstDBLib);
	tdr_close_dbhanlde(&hDBHandle);

#if defined(_WIN32) || defined(_WIN64)
	getchar();
#endif


	return iRet;
}

void tdr_show_help(const char *a_pszObj)
{
	assert(NULL != a_pszObj);
	printf("�÷�: %s [OPTION] \n", a_pszObj);
	printf("�ظ���������DR�Ķ����ϵӳ�������\n");
	printf("ѡ��:\n");
	printf("-l <looptimes> ָ���ظ������Ĵ���.\n");
	printf("-h ���ʹ�ð���\n");
}

int tdr_parse_option(OUT LPTDROPTION a_pstOption, IN int a_argc, IN char *a_argv[])
{
	int opt;
	int iRet = 0;

	assert(NULL != a_pstOption);
	assert(0 < a_argc);
	assert(NULL != a_argv);

	while( -1 != (opt=getopt(a_argc, a_argv, "l:h")) )
	{
		switch( opt )
		{
		case 'l':
			{
				a_pstOption->iLoop = strtol(optarg, NULL, 0);
				break;
			}
		case 'h': /* show the help information. */
			{
				iRet = 1;
				tdr_show_help(a_argv[0]);
				break;
			}
			default:
			{
				iRet = -1;
				printf("%s: ��Чѡ�� --%c \n", a_argv[0], (char)opt);
				printf("����ִ�� %s -h ��ȡ������Ϣ\n", a_argv[0]);
				break;
			}			
		}
	}/*while( -1 != (opt=getopt(argc, argv, "o:t:hv")) )*/

	if (0 != iRet)
	{
		return iRet;
	}	

	return iRet;
}

void init_extend_table_data(LPTESTEXTENDTABLE pstAllData)
{
	unsigned int i;
	char szBuf[32];

	pstAllData->stGame.iGameid = rand();
	pstAllData->stGame.iGamepoint = rand();
	for (i = 0; i < sizeof(pstAllData->stGame.szReserve); i++)
	{
		pstAllData->stGame.szReserve[i] = i % 16;
	}


	pstAllData->stItem.iLongnum = rand();
	pstAllData->stItem.iOffLineNum = rand();
	pstAllData->stItem.llMoney = rand() * rand();
	

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


int open_dbhandle(OUT TDRDBHANDLE *a_phDHandle, IN const char *a_pszDBMSConf)
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

	/*��db������*/
	iRet = tdr_open_dbhanlde(a_phDHandle, &stDBMS, &szError[0]);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("faild open dbhanlde: %s %s\n", tdr_error_string(iRet), szError);
		return iRet;
	}

	return iRet;
}


void insert_proformance(int iloop, TDRDBHANDLE hDBHandle, LPTDRMETA pstMeta, int iVersion, LPTESTEXTENDTABLE pstTestData, int iSeqBase)
{
	int i;
	int iRet;
	TDRDATA stData;
	char szSql[TDR_EX_MAN_SQL_LEN];
	TDRDBOBJECT stDBObj;
	TDRDBRESULTHANDLE hDBResult;
	struct timeval stBeginTime;
	struct timeval stEndTime;
	struct timeval stTempTime;
	tdr_longlong lTotalUs;
	tdr_longlong lAveTime;


	memset(&stTempTime, 0, sizeof(stTempTime));

	gettimeofday(&stBeginTime, NULL);
	for (i = 0; i < iloop; i++)
	{
		/*����Ҫ���������*/
		pstTestData->iSeq = iSeqBase + i;

		/*��������*/
		
		stData.iBuff = sizeof(szSql);
		stData.pszBuff = &szSql[0];
		stDBObj.iObjSize = sizeof(TESTEXTENDTABLE);
		stDBObj.iVersion = iVersion;
		stDBObj.pstMeta = pstMeta;
		stDBObj.pszObj = (char *)pstTestData;
		iRet = tdr_obj2sql(hDBHandle, &stData, TDR_DBOP_INSERT, &stDBObj, NULL);
		if (TDR_ERR_IS_ERROR(iRet))
		{
			printf("failed to obj2sql: %s", tdr_error_string(iRet));
		}
		iRet = tdr_query(&hDBResult, hDBHandle, &stData);
		if (TDR_ERR_IS_ERROR(iRet))
		{
			const char *dbms_error = tdr_dbms_error(hDBHandle);
			printf("failed to tdr_query: %s,dbms error: %s", tdr_error_string(iRet),
				dbms_error);
		}		
	}/*for (i = 0; i < stOption)*/

	gettimeofday(&stEndTime, NULL);
	if (stEndTime.tv_usec < stBeginTime.tv_usec)
	{
		stTempTime.tv_sec = stEndTime.tv_sec - stBeginTime.tv_sec -1;
		stTempTime.tv_usec = 1000000 + stEndTime.tv_usec - stBeginTime.tv_usec;
	}else
	{
		stTempTime.tv_sec = stEndTime.tv_sec - stBeginTime.tv_sec;
		stTempTime.tv_usec = stEndTime.tv_usec - stBeginTime.tv_usec;
	}		

	if (i > 0)
	{
		lTotalUs = stTempTime.tv_sec * 1000000 + stTempTime.tv_usec;
		lAveTime = lTotalUs /i;
		printf("insert data <size=%d> %d times, total time:%6d.%06d seconds, ave tmie: %lld us\n",
			sizeof(TESTEXTENDTABLE), i, (int)stTempTime.tv_sec, (int)stTempTime.tv_usec, lAveTime);			
	}
}


void update_proformance(int iloop, TDRDBHANDLE hDBHandle, LPTDRMETA pstMeta, int iVersion, LPTESTEXTENDTABLE pstTestData, int iSeqBase)
{
	int i;
	int iRet;
	TDRDATA stData;
	char szSql[TDR_EX_MAN_SQL_LEN];
	TDRDBOBJECT stDBObj;
	TDRDBRESULTHANDLE hDBResult;
	struct timeval stBeginTime;
	struct timeval stEndTime;
	struct timeval stTempTime;
	tdr_longlong lTotalUs;
	tdr_longlong lAveTime;


	memset(&stTempTime, 0, sizeof(stTempTime));

	gettimeofday(&stBeginTime, NULL);
	for (i = 0; i < iloop; i++)
	{
		/*����Ҫ���������*/
		init_extend_table_data(pstTestData);
		pstTestData->iSeq = iSeqBase + i;

		stData.iBuff = sizeof(szSql);
		stData.pszBuff = &szSql[0];
		stDBObj.iObjSize = sizeof(TESTEXTENDTABLE);
		stDBObj.iVersion = iVersion;
		stDBObj.pstMeta = pstMeta;
		stDBObj.pszObj = (char *)pstTestData;
		iRet = tdr_obj2sql(hDBHandle, &stData, TDR_DBOP_UPDATE, &stDBObj, NULL);
		if (TDR_ERR_IS_ERROR(iRet))
		{
			printf("failed to obj2sql: %s", tdr_error_string(iRet));
		}
		iRet = tdr_query(&hDBResult, hDBHandle, &stData);
		if (TDR_ERR_IS_ERROR(iRet))
		{
			const char *dbms_error = tdr_dbms_error(hDBHandle);
			printf("failed to tdr_query: %s,dbms error: %s", tdr_error_string(iRet),
				dbms_error);
		}		
	}/*for (i = 0; i < stOption)*/

	gettimeofday(&stEndTime, NULL);
	if (stEndTime.tv_usec < stBeginTime.tv_usec)
	{
		stTempTime.tv_sec = stEndTime.tv_sec - stBeginTime.tv_sec -1;
		stTempTime.tv_usec = 1000000 + stEndTime.tv_usec - stBeginTime.tv_usec;
	}else
	{
		stTempTime.tv_sec = stEndTime.tv_sec - stBeginTime.tv_sec;
		stTempTime.tv_usec = stEndTime.tv_usec - stBeginTime.tv_usec;
	}		

	if (i > 0)
	{
		lTotalUs = stTempTime.tv_sec * 1000000 + stTempTime.tv_usec;
		lAveTime = lTotalUs /i;
		printf("Update data <size=%d> %d times, total time:%6d.%06d seconds, ave tmie: %lld us\n",
			sizeof(TESTEXTENDTABLE), i, (int)stTempTime.tv_sec, (int)stTempTime.tv_usec, lAveTime);			
	}
}

void select_proformance(int iloop, TDRDBHANDLE hDBHandle, LPTDRMETA pstMeta, int iVersion, LPTESTEXTENDTABLE pstTestData, int iSeqBase)
{
	int i;
	int iRet;
	TDRDATA stData;
	char szSql[TDR_EX_MAN_SQL_LEN];
	TDRDBOBJECT stDBObj;
	TDRDBRESULTHANDLE hDBResult;
	struct timeval stBeginTime;
	struct timeval stEndTime;
	struct timeval stTempTime;
	tdr_longlong lTotalUs;
	tdr_longlong lAveTime;
	TESTEXTENDTABLE stSelectData;


	memset(&stTempTime, 0, sizeof(stTempTime));

	gettimeofday(&stBeginTime, NULL);
	for (i = 0; i < iloop; i++)
	{
		/*����Ҫ���������*/
		pstTestData->iSeq = iSeqBase + i;

		stData.iBuff = sizeof(szSql);
		stData.pszBuff = &szSql[0];
		stDBObj.iObjSize = sizeof(TESTEXTENDTABLE);
		stDBObj.iVersion = iVersion;
		stDBObj.pstMeta = pstMeta;
		stDBObj.pszObj = (char *)pstTestData;
		iRet = tdr_obj2sql(hDBHandle, &stData, TDR_DBOP_SELECT, &stDBObj, NULL);
		if (TDR_ERR_IS_ERROR(iRet))
		{
			printf("failed to obj2sql: %s", tdr_error_string(iRet));
		}
		iRet = tdr_query(&hDBResult, hDBHandle, &stData);
		if (TDR_ERR_IS_ERROR(iRet))
		{
			const char *dbms_error = tdr_dbms_error(hDBHandle);
			printf("failed to tdr_query: %s,dbms error: %s", tdr_error_string(iRet),
				dbms_error);
		}else
		{
			stDBObj.iObjSize = sizeof(TESTEXTENDTABLE);
			stDBObj.pszObj = (char *)&stSelectData;
			stDBObj.pstMeta = pstMeta;
			stDBObj.iVersion = tdr_get_meta_current_version(pstMeta);;

			iRet = tdr_fetch_row(hDBResult, &stDBObj);
			if (TDR_ERR_IS_ERROR(iRet))
			{
				const char *dbms_error = tdr_dbms_error(hDBHandle);
				printf("failed to tdr_query: %s,dbms error: %s", tdr_error_string(iRet),
					dbms_error);
			}
			tdr_free_dbresult(&hDBResult);
		}
	}/*for (i = 0; i < stOption)*/

	gettimeofday(&stEndTime, NULL);
	if (stEndTime.tv_usec < stBeginTime.tv_usec)
	{
		stTempTime.tv_sec = stEndTime.tv_sec - stBeginTime.tv_sec -1;
		stTempTime.tv_usec = 1000000 + stEndTime.tv_usec - stBeginTime.tv_usec;
	}else
	{
		stTempTime.tv_sec = stEndTime.tv_sec - stBeginTime.tv_sec;
		stTempTime.tv_usec = stEndTime.tv_usec - stBeginTime.tv_usec;
	}		

	if (i > 0)
	{
		lTotalUs = stTempTime.tv_sec * 1000000 + stTempTime.tv_usec;
		lAveTime = lTotalUs /i;
		printf("select data <size=%d> %d times, total time:%6d.%06d seconds, ave tmie: %lld us\n",
			sizeof(TESTEXTENDTABLE), i, (int)stTempTime.tv_sec, (int)stTempTime.tv_usec, lAveTime);			
	}
}

void delete_proformance(int iloop, TDRDBHANDLE hDBHandle, LPTDRMETA pstMeta, int iVersion, LPTESTEXTENDTABLE pstTestData, int iSeqBase)
{
	int i;
	int iRet;
	TDRDATA stData;
	char szSql[TDR_EX_MAN_SQL_LEN];
	TDRDBOBJECT stDBObj;
	TDRDBRESULTHANDLE hDBResult;
	struct timeval stBeginTime;
	struct timeval stEndTime;
	struct timeval stTempTime;
	tdr_longlong lTotalUs;
	tdr_longlong lAveTime;


	memset(&stTempTime, 0, sizeof(stTempTime));

	gettimeofday(&stBeginTime, NULL);
	for (i = 0; i < iloop; i++)
	{
		/*����Ҫ���������*/
		pstTestData->iSeq = iSeqBase + i;

		stData.iBuff = sizeof(szSql);
		stData.pszBuff = &szSql[0];
		stDBObj.iObjSize = sizeof(TESTEXTENDTABLE);
		stDBObj.iVersion = iVersion;
		stDBObj.pstMeta = pstMeta;
		stDBObj.pszObj = (char *)pstTestData;
		iRet = tdr_obj2sql(hDBHandle, &stData, TDR_DBOP_DELETE, &stDBObj, NULL);
		if (TDR_ERR_IS_ERROR(iRet))
		{
			printf("failed to obj2sql: %s", tdr_error_string(iRet));
		}
		iRet = tdr_query(&hDBResult, hDBHandle, &stData);
		if (TDR_ERR_IS_ERROR(iRet))
		{
			const char *dbms_error = tdr_dbms_error(hDBHandle);
			printf("failed to tdr_query: %s,dbms error: %s", tdr_error_string(iRet),
				dbms_error);
		}		
	}/*for (i = 0; i < stOption)*/

	gettimeofday(&stEndTime, NULL);
	if (stEndTime.tv_usec < stBeginTime.tv_usec)
	{
		stTempTime.tv_sec = stEndTime.tv_sec - stBeginTime.tv_sec -1;
		stTempTime.tv_usec = 1000000 + stEndTime.tv_usec - stBeginTime.tv_usec;
	}else
	{
		stTempTime.tv_sec = stEndTime.tv_sec - stBeginTime.tv_sec;
		stTempTime.tv_usec = stEndTime.tv_usec - stBeginTime.tv_usec;
	}		

	if (i > 0)
	{
		lTotalUs = stTempTime.tv_sec * 1000000 + stTempTime.tv_usec;
		lAveTime = lTotalUs /i;
		printf("delete data <size=%d> %d times, total time:%6d.%06d seconds, ave tmie: %lld us\n",
			sizeof(TESTEXTENDTABLE), i, (int)stTempTime.tv_sec, (int)stTempTime.tv_usec, lAveTime);			
	}
}

void gen_update_dml_proformance(int iloop, TDRDBHANDLE hDBHandle, LPTDRMETA pstMeta, int iVersion, LPTESTEXTENDTABLE pstTestData, int iSeqBase)
{
	int i;
	int iRet;
	TDRDATA stData;
	char szSql[TDR_EX_MAN_SQL_LEN];
	TDRDBOBJECT stDBObj;
	struct timeval stBeginTime;
	struct timeval stEndTime;
	struct timeval stTempTime;
	tdr_longlong lTotalUs;
	tdr_longlong lAveTime;


	memset(&stTempTime, 0, sizeof(stTempTime));

	gettimeofday(&stBeginTime, NULL);
	for (i = 0; i < iloop; i++)
	{
		/*����Ҫ���������*/
		init_extend_table_data(pstTestData);
		pstTestData->iSeq = iSeqBase + i;

		stData.iBuff = sizeof(szSql);
		stData.pszBuff = &szSql[0];
		stDBObj.iObjSize = sizeof(TESTEXTENDTABLE);
		stDBObj.iVersion = iVersion;
		stDBObj.pstMeta = pstMeta;
		stDBObj.pszObj = (char *)pstTestData;
		iRet = tdr_obj2sql(hDBHandle, &stData, TDR_DBOP_UPDATE, &stDBObj, NULL);
		if (TDR_ERR_IS_ERROR(iRet))
		{
			printf("failed to obj2sql: %s", tdr_error_string(iRet));
		}			
	}/*for (i = 0; i < stOption)*/

	gettimeofday(&stEndTime, NULL);
	if (stEndTime.tv_usec < stBeginTime.tv_usec)
	{
		stTempTime.tv_sec = stEndTime.tv_sec - stBeginTime.tv_sec -1;
		stTempTime.tv_usec = 1000000 + stEndTime.tv_usec - stBeginTime.tv_usec;
	}else
	{
		stTempTime.tv_sec = stEndTime.tv_sec - stBeginTime.tv_sec;
		stTempTime.tv_usec = stEndTime.tv_usec - stBeginTime.tv_usec;
	}		

	if (i > 0)
	{
		lTotalUs = stTempTime.tv_sec * 1000000 + stTempTime.tv_usec;
		lAveTime = lTotalUs /i;
		printf("gen udpate dml <size=%d> %d times, total time:%6d.%06d seconds, ave tmie: %lld us\n",
			sizeof(TESTEXTENDTABLE), i, (int)stTempTime.tv_sec, (int)stTempTime.tv_usec, lAveTime);		
	}
}
