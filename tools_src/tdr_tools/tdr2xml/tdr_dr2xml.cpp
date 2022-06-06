/**
*
* @file     tdr_dr2xml.c
* @brief    ��DR�������ļ�ת����XML�ļ��Ŀ���̨����
*
* @author jackyai
* @version 1.0
* @date 2007-06-01
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

#include "../tgetopt.h"
#include "pal/pal.h"
#include "tdr/tdr.h"
#include "version.h"
#include "../tdr_tools.h"

#if defined(_WIN32) || defined(_WIN64)
#pragma warning(disable:4996)
#endif

#if defined(_WIN32) || defined(_WIN64)
#ifndef snprintf
#define snprintf _snprintf
#endif
#endif


#define TDR_MAX_OUTPUT_FILE_LEN			256 /**<����ļ���������󳤶�*/


#define  TDR_DEFAULT_DR_OUTPUT_FILE		"a.xml"

struct tagTDRDR2Option
{
	char *pszInputFile;
	char szOutputFileName[TDR_MAX_OUTPUT_FILE_LEN]; /**<����ļ�������*/
};
typedef struct tagTDRDR2Option   TDRDR2XMLOPTION;
typedef struct tagTDRDR2Option *LPTDRDR2XMLOPTION;

static void tdr_show_help(const char *a_pszObj);

/**�������й��ߵ�ѡ��
*@retval 0 ��ȷ��������ѡ��
*@retval >0 ��ȷ��������ѡ��,��ָʾ�����й��߲��ؼ���ִ��
*@retval <0 ����ѡ��ʧ��
*/
static int tdr_parse_option(OUT LPTDRDR2XMLOPTION a_pstOption, IN int a_argc, IN char *a_argv[]);

int main(int argc, char *argv[])
{
	LPTDRMETALIB pstLib=NULL;
	TDRDR2XMLOPTION stOption;
	int iRet;

	memset(&stOption, 0, sizeof(stOption));
	strncpy(stOption.szOutputFileName, TDR_DEFAULT_DR_OUTPUT_FILE, TDR_MAX_OUTPUT_FILE_LEN);
	iRet = tdr_parse_option(&stOption, argc, argv);
	if (0 != iRet)
	{
	//	getchar();
		return iRet;
	}


	iRet = tdr_load_metalib(&pstLib, stOption.pszInputFile);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("�Ӷ�����Ԫ�����ļ�����Ԫ���ݿ�ʧ��, ԭ����: %s\n", tdr_error_string(iRet));
	}


	if (!TDR_ERR_IS_ERROR(iRet))
	{
		iRet = tdr_save_xml_file(pstLib, stOption.szOutputFileName);
		if (TDR_ERR_IS_ERROR(iRet))
		{
			printf("��Ԫ���ݿ�ת����XML��ʽ���ļ�ʧ��, ԭ����: %s\n", tdr_error_string(iRet));
		}else
		{
			printf("�ɹ���������Ԫ�����ļ�ת����XML��ʽ�������ļ�<%s>\n", stOption.szOutputFileName);
		}
	}

	tdr_free_lib(&pstLib);

	//getchar();

	return iRet;
}

void tdr_show_help(const char *a_pszObj)
{
	assert(NULL != a_pszObj);
	printf("�÷�: %s [-o --out_file=FILE] [-h --help] [-v --version] DRFILE \n", a_pszObj);
	printf("�������Ƹ�ʽԪ�����ļ�ת����һ��XML��ʽ��Ԫ���������ļ�, ����DRFILEָ����������Ƹ�ʽԪ�����ļ����ļ���\n");
	printf("ѡ��:\n");
	printf("-o, --out_file=FILE ָ������ļ�������,ȱʡΪ%s\n", TDR_DEFAULT_DR_OUTPUT_FILE);
	printf("-h, --help ���ʹ�ð���\n");
	printf("-v, --version ����汾��Ϣ\n");
}

int tdr_parse_option(OUT LPTDRDR2XMLOPTION a_pstOption, IN int a_argc, IN char *a_argv[])
{
	int opt;
	int iRet = 0;

	assert(NULL != a_pstOption);
	assert(0 < a_argc);
	assert(NULL != a_argv);

	while (1)
	{
		//int this_option_optind = optind ? optind : 1;
		int option_index = 0;
		static struct option stlong_options[] = {
			{"help", 0, 0, 'h'},
			{"out_file", 1, 0, 'o'},
			{"version", 0, 0, 'v'},
			{0, 0, 0, 0}
		};

		opt = getopt_long (a_argc, a_argv, "o:hv",
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
		case 'o':
			{
				if (NULL == strrchr(optarg, '.'))
				{
					snprintf(a_pstOption->szOutputFileName, TDR_MAX_OUTPUT_FILE_LEN -1, "%s.xml", optarg);
				}else
				{
					snprintf(a_pstOption->szOutputFileName, TDR_MAX_OUTPUT_FILE_LEN -1, "%s", optarg);
				}
				a_pstOption->szOutputFileName[TDR_MAX_OUTPUT_FILE_LEN -1] = '\0';

				break;
			}

		case 'h': /* show the help information. */
			{
				iRet = 1;
				tdr_show_help(a_argv[0]);
                exit(0);
				break;
			}
		case 'v':
			{
				iRet = 1;
                printf("%s:version %d.%d.%d  build at %d.\n", basename(a_argv[0]), MAJOR,
                    MINOR, REV, BUILD);
                exit(0);
				break;
			}
		default:
			{
				iRet = -1;
				printf("%s: ��Чѡ�� --%c \n", a_argv[0], (char)opt);
				printf("����ִ�� xml2dr -h ��ȡ������Ϣ\n");
                exit(0);
				break;
			}
		}
	}/*while( -1 != (opt=getopt(argc, argv, "o:t:hv")) )*/

	if (0 != iRet)
	{
		return iRet;
	}

	if (optind >= a_argc)
	{
		printf("����ͨ������ָ������Ԫ���ݿ�������ļ����ļ���\n");
		return -2;
	}
	a_pstOption->pszInputFile = a_argv[optind];

	return iRet;
}



