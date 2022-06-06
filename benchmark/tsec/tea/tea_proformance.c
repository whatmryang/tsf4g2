/**
*
* @file     test_tea_proformance.c
* @brief    通过多次使用tea算法进行加，解 测试tea算法的性能
*
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
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#if !defined(_WIN32) && !defined(_WIN64)
#include <sys/time.h>
#endif



#include "tgetopt.h"
#include "pal/pal.h"
#include "tsec/oi_tea.h"



#define  DEFAULT_LOOP_TIMES		100000

#define DEFAULT_DATA_LEN	200

struct tagOption
{
	int iLoop;     /**<重复次数*/
	int iDataLen;	/**<测试数据的长度*/

};
typedef struct tagOption   TDROPTION;
typedef struct tagOption *LPTDROPTION;

static void tea_show_help(const char *a_pszObj);



/**分析运行工具的选项
*@retval 0 正确分析出各选项
*@retval >0 正确分析出各选项,但指示此运行工具不必继续执行
*@retval <0 分析选项失败
*/
static int tea_parse_option(OUT LPTDROPTION a_pstOption, IN int a_argc, IN char *a_argv[]);

int main(int argc, char *argv[])
{
#define TQQ_KEY_LEN		16


	TDROPTION stOption;
	int iRet = 0;
	int i;
	struct timeval stBeginTime;
	struct timeval stEndTime;
	struct timeval stTempTime;
	long long lTotalUs =0;
	long long lAveTime = 0;
	char *pszPlainData;
	int *piData;
	char szKey[TQQ_KEY_LEN];
	char *pszSecretData;
	int iPlainLen;
	int iSecretLen;

	memset(&stOption, 0, sizeof(stOption));
	stOption.iLoop = DEFAULT_LOOP_TIMES;
	iRet = tea_parse_option(&stOption, argc, argv);
	if (0 != iRet)
	{
		printf("failed to parse command line, please check you args\n");
		return iRet;
	}



	//为数据分配空间
	pszPlainData = calloc(1, stOption.iDataLen);
	if (NULL == pszPlainData)
	{
		printf("failed to malloc memory(iSize:%d) to store data\n", stOption.iDataLen);
		return -1;
	}
	pszSecretData = calloc(1, stOption.iDataLen*2);
	if (NULL == pszSecretData)
	{
		printf("failed to malloc memory(iSize:%d) to store data\n", stOption.iDataLen);
		free(pszPlainData);
		return -1;
	}
	srand(time(NULL));
	piData = (int *)pszPlainData;
	for (i = 0; i < stOption.iDataLen/(int)sizeof(int);i++)
	{
		*piData++ = rand();
	}
	piData = (int *)szKey;
	for (i = 0; i < (int)(sizeof(szKey)/sizeof(int)); i++)
	{
		*piData++ = rand();
	}



	//测试QQ对称加密第一代函数的时间
	gettimeofday(&stBeginTime, NULL);
	for (i = 0 ; i < stOption.iLoop; i++)
	{
		iSecretLen = stOption.iDataLen*2;
		oi_symmetry_encrypt(pszPlainData, stOption.iDataLen, &szKey[0], pszSecretData, &iSecretLen);


	}/*for (i = 0 ; i < stOption.iXMLFileNum; i++)*/
	gettimeofday(&stEndTime, NULL);
	TV_DIFF(stTempTime, stEndTime, stBeginTime);

	lTotalUs = stTempTime.tv_sec * 1000000 + stTempTime.tv_usec;
	if (i > 0)
	{
		lAveTime = lTotalUs /i;
	}
	printf("oi_symmetry_encrypt info: encryptdatalen:%d  %d times, total time:%6d.%06d seconds, ave tmie: %lld us\n",
		stOption.iDataLen, i,
		(int)stTempTime.tv_sec, (int)stTempTime.tv_usec, lAveTime);

	//测试QQ对称解密第一代函数的时间
	gettimeofday(&stBeginTime, NULL);
       for( i = 0 ; i < stOption.iLoop; i++ )
       {
             iPlainLen = stOption.iDataLen;
		if (!oi_symmetry_decrypt(pszSecretData, iSecretLen, &szKey[0], pszPlainData, &iPlainLen))
		{
			printf("oi_symmetry_decrypt failed \n");
			break;
		}
       }
	gettimeofday(&stEndTime, NULL);
	TV_DIFF(stTempTime, stEndTime, stBeginTime);

	lTotalUs = stTempTime.tv_sec * 1000000 + stTempTime.tv_usec;
	if (i > 0)
	{
		lAveTime = lTotalUs /i;
	}
	printf("oi_symmetry_decrypt info: decryptdatalen:%d  %d times, total time:%6d.%06d seconds, ave tmie: %lld us\n",
		                                                             iSecretLen, i,
      		                                                             (int)stTempTime.tv_sec, (int)stTempTime.tv_usec, lAveTime);



	//测试QQ对称加密第二代函数的时间
	//encrypt
	gettimeofday(&stBeginTime, NULL);
	for (i = 0 ; i < stOption.iLoop; i++)
	{
		iSecretLen = stOption.iDataLen*2;
		oi_symmetry_encrypt2(pszPlainData, stOption.iDataLen, &szKey[0], pszSecretData, &iSecretLen);


	}/*for (i = 0 ; i < stOption.iXMLFileNum; i++)*/
	gettimeofday(&stEndTime, NULL);
	TV_DIFF(stTempTime, stEndTime, stBeginTime);

	lTotalUs = stTempTime.tv_sec * 1000000 + stTempTime.tv_usec;
	if (i > 0)
	{
		lAveTime = lTotalUs /i;
	}
	printf("oi_symmetry_encrypt2: encrypt datalen:%d   %d times, total time:%6d.%06d seconds, ave tmie: %lld us\n",
		stOption.iDataLen, i,
		(int)stTempTime.tv_sec, (int)stTempTime.tv_usec, lAveTime);

	//decrypt
	gettimeofday(&stBeginTime, NULL);
	for (i = 0 ; i < stOption.iLoop; i++)
	{

		iPlainLen = stOption.iDataLen;
		if (!oi_symmetry_decrypt2(pszSecretData, iSecretLen, &szKey[0], pszPlainData, &iPlainLen))
		{
			printf("oi_symmetry_decrypt2 failed \n");
			break;
		}

	}/*for (i = 0 ; i < stOption.iXMLFileNum; i++)*/
	gettimeofday(&stEndTime, NULL);
	TV_DIFF(stTempTime, stEndTime, stBeginTime);

	lTotalUs = stTempTime.tv_sec * 1000000 + stTempTime.tv_usec;
	if (i > 0)
	{
		lAveTime = lTotalUs /i;
	}
	printf("oi_symmetry_decrypt2: decryptdatalen:%d  %d times, total time:%6d.%06d seconds, ave tmie: %lld us\n",
		iSecretLen, i,
		(int)stTempTime.tv_sec, (int)stTempTime.tv_usec, lAveTime);


	return iRet;
}

void tea_show_help(const char *a_pszObj)
{
	assert(NULL != a_pszObj);
	printf("\n%s test the performance tea encrypt &decrypt \n",	a_pszObj);
	printf("Usage: %s -b --data-len=len  -l --loop=Count [-h]  datafile \n",
		a_pszObj);
	printf("-b --data-len: specify length of the data to encrypt&decrypt\n");
	printf("-l --loop=Count: specify the call times of tea encrypt &decrypt\n");
	printf("-h: show this information\n");
}

int tea_parse_option(OUT LPTDROPTION a_pstOption, IN int a_argc, IN char *a_argv[])
{
	int opt;
	int iRet = 0;


	assert(NULL != a_pstOption);
	assert(0 < a_argc);
	assert(NULL != a_argv);

	while (1)
	{
		int option_index = 0;
		static struct option stlong_options[] = {
			{"help", 0, 0, 'h'},
			{"data-len", 1, 0, 'B'},
			{"loop", 1, 0, 'l'},
			{0, 0, 0, 0}
		};

		opt = getopt_long (a_argc, a_argv, "B:b:l:L:h",
			stlong_options, &option_index);

		if (opt == -1)
			break;

		if ((opt == '?') || (opt == ':') )
		{
			iRet = 1;
			break;
		}

		switch( opt )
		{
			case 'B':
			case 'b':
			{
				a_pstOption->iDataLen = (int)strtol(optarg, NULL, 0);
				break;
			}

		case 'l':
		case 'L':
			{
				a_pstOption->iLoop =  (int)strtol(optarg, NULL, 0);
				break;
			}

		case 'h': /* show the help information. */
			{
				iRet = 1;
				tea_show_help(a_argv[0]);
				exit(0);
				break;
			}
		default:
			{
				iRet = -1;
				printf("%s: 无效选项 --%c \n", a_argv[0], (char)opt);
				printf("可以执行 %s -h 获取更多信息\n", a_argv[0]);
				exit(0);
			}
		}
	}/*while (1)*/

	if (0 != iRet)
	{
		return iRet;
	}



	if (0 >= a_pstOption->iDataLen)
	{
		a_pstOption->iDataLen = DEFAULT_DATA_LEN;
	}

	if (0 >= a_pstOption->iLoop)
	{
		a_pstOption->iLoop = DEFAULT_LOOP_TIMES;
	}



	return iRet;
}




