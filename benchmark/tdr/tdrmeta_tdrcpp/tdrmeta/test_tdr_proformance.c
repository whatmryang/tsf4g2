/**
*
* @file     test_tdr_proformance.c
* @brief    通过多次编/解码测试TDR模块的性能
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
#include "tdr/tdr.h"
#include "tdr/tdr_metalib_kernel_i.h"



#define  DEFAULT_LOOP_TIMES		100000
#define MAX_FILE_PATH_LEN	256


struct tagOption
{
	int iLoop;     /**<重复编/解码的次数*/
	char szMetalibFile[MAX_FILE_PATH_LEN]; /*metalib库的路径*/
	char szMetaName[TDR_NAME_LEN];		/**<元数据名*/
	char szDataFile[MAX_FILE_PATH_LEN]; /*metalib库的路径*/

};
typedef struct tagOption   TDROPTION;
typedef struct tagOption *LPTDROPTION;

static void tdr_show_help(const char *a_pszObj);



/**分析运行工具的选项
*@retval 0 正确分析出各选项
*@retval >0 正确分析出各选项,但指示此运行工具不必继续执行
*@retval <0 分析选项失败
*/
static int tdr_parse_option(OUT LPTDROPTION a_pstOption, IN int a_argc, IN char *a_argv[]);

//////////////////////////////////////////////////////////////////////////////////////////
int tdr_calc_entry_num(IN LPTDRMETA a_pstMeta, IN LPTDRDATA a_pstHost,int a_iVersion, IN int  *a_pEntryNum, IN int  *a_psimEntryNum, int *a_picomEntryNum)
{
	int iRet = TDR_SUCCESS;
	LPTDRMETALIB pstLib;
	LPTDRMETA pstCurMeta;
	TDRSTACK  stStack;
	LPTDRSTACKITEM pstStackTop;
	int iStackItemCount;
	int iChange;
	LPTDRMETAENTRY pstEntry;
	int iArrayRealCount ;
	char *pszHostStart;
	char *pszHostEnd;
	int idxSubEntry;

	assert(NULL != a_pstMeta);
	assert(NULL != a_pstHost);
	assert(NULL != a_psimEntryNum);
	assert(NULL != a_picomEntryNum);




	if ((NULL == a_pstMeta)||(TDR_TYPE_UNION == a_pstMeta->iType)||(NULL == a_pstHost))
	{
		return TDR_ERRIMPLE_INVALID_PARAM;
	}
	if ((NULL == a_pstHost->pszBuff)||(0 >= a_pstHost->iBuff))
	{
		return TDR_ERRIMPLE_INVALID_PARAM;
	}

	if (0 == a_iVersion)
	{
		a_iVersion = TDR_MAX_VERSION;
	}



	pszHostStart = a_pstHost->pszBuff;
	pszHostEnd = a_pstHost->pszBuff + a_pstHost->iBuff;
	pstCurMeta = a_pstMeta;
	pstLib = TDR_META_TO_LIB(a_pstMeta);

	pstStackTop = &stStack[0];
	pstStackTop->pstMeta = pstCurMeta;
	pstStackTop->iCount = 1;
	pstStackTop->idxEntry = 0;
	pstStackTop->pszHostBase = a_pstHost->pszBuff;
	pstStackTop->pszHostEnd = a_pstHost->pszBuff + a_pstHost->iBuff;
	pstStackTop->iChange = 0;

	TDR_GET_VERSION_INDICATOR(iRet, a_pstHost->pszBuff, pszHostEnd, pstCurMeta, pstStackTop->iCutOffVersion, a_iVersion);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		return iRet;
	}
	if (pstCurMeta->iBaseVersion > pstStackTop->iCutOffVersion)
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
	}
	iStackItemCount = 1;


	iChange = 0;
	while (0 < iStackItemCount)
	{
		if (0 >= pstStackTop->iCount)
		{/*当前元数据数组已经处理完毕*/
			pstStackTop--;
			iStackItemCount--;
			if (0 < iStackItemCount)
			{
				pstCurMeta = pstStackTop->pstMeta;
				TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			}
			continue;
		}

		pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;

		*a_pEntryNum += 1;

		if (pstEntry->iVersion > pstStackTop->iCutOffVersion)
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}

		/*指针不编码*/
		if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry))
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}
		pszHostStart = pstStackTop->pszHostBase + pstEntry->iHOff;
		pszHostEnd = pstStackTop->pszHostEnd;
		if (TDR_ENTRY_IS_REFER_TYPE(pstEntry))
		{
			pszHostStart = *(char **)pszHostStart;
			pszHostEnd = pszHostStart + pstEntry->iNRealSize;
		}



		/*取出此entry的数组计数信息*/
		TDR_GET_ARRAY_REAL_COUNT(iArrayRealCount, pstEntry, pstStackTop->pszHostBase, pstStackTop->iCutOffVersion);
		if ((0 > iArrayRealCount) || ((pstEntry->iCount < iArrayRealCount) && (0 < pstEntry->iCount)))
		{/*实际数目为负数或比数组最大长度要大，则无效*/
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_REFER_VALUE);
			break;
		}
		if (0 >= iArrayRealCount)
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}


		/*简单数据类型*/
		if (TDR_TYPE_COMPOSITE < pstEntry->iType)
		{
			*a_psimEntryNum += iArrayRealCount;
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}

		/*复合数据类型*/
		if (TDR_STACK_SIZE <=  iStackItemCount)
		{
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_TOO_COMPLIEX_META);
			pstStackTop->iCode += pstEntry->iHOff;
			break;
		}

		if (TDR_TYPE_UNION == pstEntry->iType)
		{
			TDR_GET_UNION_ENTRY_TYPE_META_INFO(pstStackTop->pszHostBase, pstLib, pstEntry, pstStackTop->iCutOffVersion, pstCurMeta, idxSubEntry);
			if(NULL == pstCurMeta)
			{
				pstCurMeta = pstStackTop->pstMeta;
				TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
				continue;
			}
		}else
		{
			pstCurMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
			idxSubEntry = 0;
		}

		/*递归进行结构体内部进行处理*/
		iStackItemCount++;
		pstStackTop++;
		TDR_GET_VERSION_INDICATOR(iRet, pszHostStart, pszHostEnd, pstCurMeta, pstStackTop->iCutOffVersion, (pstStackTop-1)->iCutOffVersion);
		if (TDR_ERR_IS_ERROR(iRet))
		{
			break;
		}
		if (pstCurMeta->iBaseVersion > pstStackTop->iCutOffVersion)
		{
			iRet =  TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
			break;
		}
		pstStackTop->iChange = 0;
		pstStackTop->iCount = iArrayRealCount;
		pstStackTop->idxEntry = idxSubEntry;
		pstStackTop->pszHostEnd = pszHostEnd;
		pstStackTop->pszHostBase = pszHostStart;
		*a_picomEntryNum += iArrayRealCount;
		pstStackTop->pstMeta = pstCurMeta;

	}/*while (0 < iStackItemCount)*/



	return iRet;

}

int main(int argc, char *argv[])
{
#define RESERVE_SIZE		256

	LPTDRMETALIB pstLib=NULL;
	TDROPTION stOption;
	LPTDRMETA pstMeta;
	char *pszHost = NULL;
	char *pszNet = NULL;
	int iSize;
	TDRDATA stHostData;
	TDRDATA stNetData;
	int iRet = 0;
	int i;
	struct timeval stBeginTime;
	struct timeval stEndTime;
	struct timeval stTempTime;
	long long lTotalUs =0;
	long long lAveTime = 0;
	int iEntryNum = 0;
	int simEntryNum = 0;
	int comEntryNum = 0;

	memset(&stOption, 0, sizeof(stOption));
	stOption.iLoop = DEFAULT_LOOP_TIMES;
	iRet = tdr_parse_option(&stOption, argc, argv);
	if (0 != iRet)
	{
		printf("failed to parse command line, please check you args\n");
		return iRet;
	}

	iRet = tdr_load_metalib(&pstLib, stOption.szMetalibFile);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("failed to tdr_load_metalib :%s\n", stOption.szMetalibFile);
		return iRet;
	}
	pstMeta = tdr_get_meta_by_name(pstLib, stOption.szMetaName);
	if (NULL == pstMeta)
	{
		printf("failed to tdr_get_meta_by_name %s\n", stOption.szMetaName);
		return -2;
	}

	//为数据分配空间
	iSize = RESERVE_SIZE + tdr_get_meta_size(pstMeta);
	pszHost = calloc(1, iSize);
	if (NULL == pszHost)
	{
		printf("failed to malloc memory(iSize:%d) to store data\n", iSize);
		iRet = -1;
	}
	if (0 == iRet)
	{
		pszNet = calloc(1, iSize);
		if (NULL == pszNet)
		{
			printf("failed to malloc memory(iSize:%d) to store data\n", iSize);
			iRet = -1;
		}
	}

	/*加载数据*/
	if (0 == iRet)
	{
		stHostData.iBuff = iSize;
		stHostData.pszBuff = pszHost;
		iRet = tdr_input_file(pstMeta, &stHostData, stOption.szDataFile, 0, 0);
		if (0 != iRet)
		{
			printf("failed to load data :%s\n", tdr_error_string(iRet));
		}
	}

	if (0 == iRet)
	{
		stHostData.iBuff = iSize;
		stHostData.pszBuff = pszHost;
		tdr_calc_entry_num(pstMeta, &stHostData, 0, &iEntryNum, &simEntryNum, &comEntryNum);
	}

	//测试tdr_hton 的时间
	if (0 == iRet)
	{
		gettimeofday(&stBeginTime, NULL);
		for (i = 0 ; i < stOption.iLoop; i++)
		{
			stHostData.iBuff = iSize;
			stHostData.pszBuff = pszHost;
			stNetData.iBuff = iSize;
			stNetData.pszBuff = pszNet;

			iRet = tdr_hton(pstMeta, &stNetData, &stHostData, 0);
			if (0 > iRet)
			{
				printf("failed to tdr_hton, for %s", tdr_error_string(iRet));
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
		printf("tdr_hton %s(Host size:%d Net size:%d) %d times, total time:%6d.%06d seconds, ave tmie: %lld us\n",
			stOption.szMetaName, tdr_get_meta_size(pstMeta), stNetData.iBuff,
			i, 	(int)stTempTime.tv_sec, (int)stTempTime.tv_usec, lAveTime);
	}/*if (0 == iRet)*/




	//测试tdr_ntoh 的时间
	if (0 == iRet)
	{
		gettimeofday(&stBeginTime, NULL);
		for (i = 0 ; i < stOption.iLoop; i++)
		{
			stHostData.iBuff = iSize;
			stHostData.pszBuff = pszHost;
			stNetData.iBuff = iSize;
			stNetData.pszBuff = pszNet;

			iRet = tdr_ntoh(pstMeta, &stHostData, &stNetData, 0);
			if (0 > iRet)
			{
				printf("failed to tdr_ntoh, for %s", tdr_error_string(iRet));
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
		printf("tdr_ntoh %s(Host size:%d Net size:%d  iEntryNum:%d simEntryNum:%d comEntryNum:%d) %d times, total time:%6d.%06d seconds, ave tmie: %lld us\n",
			stOption.szMetaName, tdr_get_meta_size(pstMeta), stNetData.iBuff,
			iEntryNum, simEntryNum, comEntryNum, i,
			(int)stTempTime.tv_sec, (int)stTempTime.tv_usec, lAveTime);
	}/*if (0 == iRet)*/


	//测试memcpy 的时间
	if (0 == iRet)
	{
		iSize = tdr_get_meta_size(pstMeta);
		gettimeofday(&stBeginTime, NULL);
		for (i = 0 ; i < stOption.iLoop; i++)
		{
			memcpy(pszNet, pszHost, iSize);
		}/*for (i = 0 ; i < stOption.iXMLFileNum; i++)*/
		gettimeofday(&stEndTime, NULL);
		TV_DIFF(stTempTime, stEndTime, stBeginTime);

		lTotalUs = stTempTime.tv_sec * 1000000 + stTempTime.tv_usec;
		if (i > 0)
		{
			lAveTime = lTotalUs /i;
		}
		printf("memcpy %s(Host size:%d ) %d times, total time:%6d.%06d seconds, ave tmie: %lld us\n",
			stOption.szMetaName, tdr_get_meta_size(pstMeta), i,
			(int)stTempTime.tv_sec, (int)stTempTime.tv_usec, lAveTime);
	}/*if (0 == iRet)*/



	if (NULL != pstLib)
	{
		tdr_free_lib(&pstLib);
	}
	if (NULL != pszHost)
	{
		free(pszHost);
	}
	if (NULL != pszNet)
	{
		free(pszNet);
	}

	return iRet;
}

void tdr_show_help(const char *a_pszObj)
{
	assert(NULL != a_pszObj);
	printf("\n%s test the performance tdr_hton/tdr_ntoh: load the data in the xmlfile to the memory, and then call the tdr_hton/tdr_ntoh to \n"
		"\t pack/unpack the data to network message.\n",
		a_pszObj);
	printf("Usage: %s -B --meta_file=FILE -m --meta_name=NAME -l --loop=Count [-h]  datafile \n",
		a_pszObj);
	printf("-B --meta_file=FILE: specify the binary file of metalib\n");
	printf("-m --meta_name=NAME: specify Meta Name of the data\n");
	printf("-l --loop=Count: specify the call times of tdr_hton/tdr_ntoh\n");
	printf("-h: show this information\n");
}

int tdr_parse_option(OUT LPTDROPTION a_pstOption, IN int a_argc, IN char *a_argv[])
{
	int opt;
	int iRet = 0;
	int iWriteLen;


	assert(NULL != a_pstOption);
	assert(0 < a_argc);
	assert(NULL != a_argv);

	while (1)
	{
		int option_index = 0;
		static struct option stlong_options[] = {
			{"help", 0, 0, 'h'},
			{"meta_file", 1, 0, 'B'},
			{"meta_name", 1, 0, 'm'},
			{"loop", 1, 0, 'l'},
			{0, 0, 0, 0}
		};

		opt = getopt_long (a_argc, a_argv, "B:b:m:M:l:L:h",
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
				iWriteLen = snprintf(a_pstOption->szMetalibFile, sizeof(a_pstOption->szMetalibFile), "%s", optarg);
				if ((0 > iWriteLen) || (iWriteLen >= (int)(sizeof(a_pstOption->szMetalibFile))))
				{
					printf("指定元数据文件名太长，目前最多支持%u个字符的元数据文件名\n", sizeof(a_pstOption->szMetalibFile) -1);
					iRet = 1;
				}
				break;
			}
		case 'm':
		case 'M':
			{
				iWriteLen = snprintf(a_pstOption->szMetaName, sizeof(a_pstOption->szMetaName), "%s", optarg);
				if ((0 > iWriteLen) || (iWriteLen >= (int)(sizeof(a_pstOption->szMetaName))))
				{
					printf("指定元数据名太长，目前最多支持%u个字符的元数据名\n", sizeof(a_pstOption->szMetaName) -1);
					iRet = 1;
				}
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
				tdr_show_help(a_argv[0]);
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



	if (a_pstOption->szMetaName[0] == '\0')
	{
		printf("\n至少必须指定测试数据在元数据描述库中的名称, 可以执行 %s -h 获取更多信息\n", a_argv[0]);
		return -2;
	}
	if (a_pstOption->szMetalibFile[0] == '\0')
	{
		printf("\n至少必须指定描述测试数据结构的元数据描述库文件, 可以执行 %s -h 获取更多信息\n", a_argv[0]);
		return -2;
	}

	/*通过参数获取xml元数据库文件*/
	if (optind >=a_argc)
	{
		printf("\n至少必须指定用来做性能测试的xml数据文件, 可以执行 %s -h 获取更多信息\n", a_argv[0]);
		return -2;
	}else
	{
		STRNCPY(a_pstOption->szDataFile, a_argv[optind], sizeof(a_pstOption->szDataFile));
	}

	if (0 >= a_pstOption->iLoop)
	{
		a_pstOption->iLoop = DEFAULT_LOOP_TIMES;
	}



	return iRet;
}




