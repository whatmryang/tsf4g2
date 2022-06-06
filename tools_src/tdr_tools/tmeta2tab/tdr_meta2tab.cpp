/**
*
* @file     tdr_meta2tab.c
* @brief    ����Ԫ����������������Ҫ�Ľ�������޸ı����
*
* @author jackyai
* @version 1.0
* @date 2007-08-02
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
#include "pal/pal.h"
#include "../tgetopt.h"
#include "tdr/tdr.h"
#include "version.h"
#include "../tdr_tools.h"
#include "../../../lib_src/tdr/tdr_metalib_kernel_i.h"



#if defined(_WIN32) || defined(_WIN64)
#pragma warning(disable:4996)
#endif

#if defined(_WIN32) || defined(_WIN64)


# ifdef _DEBUG
#  define LIB_D    "_d"
# else
#  define LIB_D
# endif


# if defined(LIB_D)
# pragma comment( lib, "libpal"  LIB_D ".lib" )
# else
# pragma comment( lib, "libpal.lib" )
# endif




#ifndef snprintf
#define snprintf _snprintf
#endif

#endif /*#if defined(_WIN32) || defined(_WIN64)*/


#define TDR_MAX_PATH_LEN		   512 /**<�ļ�Ŀ¼���ļ�����󳤶�*/
#define TDR_DEFAUTL_SQL_FILE	"a.sql"

#if defined(_WIN32) || defined(_WIN64)
#define TDR_DIR_PATH_CHAR		'\\'
#else
#define TDR_DIR_PATH_CHAR		'/'
#endif



struct tagTDRMeta2TabOption
{
	int iVersion;     /**<ָ��Ԫ���ݵİ汾*/
	int iAlter;		/*�Ƿ������޸ı���䣬0�������ɣ���0ֵ�� �����޸ı����*/
    int iHasSpecifySqlFile;
    int iGenerateForAllMeta;
	char szTDRDir[TDR_MAX_PATH_LEN];	/**<Ԫ�����ļ�����Ŀ¼*/
	char szTDRFile[TDR_MAX_PATH_LEN];	/**<Ԫ�����ļ���*/
	char szMetaName[TDR_NAME_LEN];		/**<Ԫ������*/
	char szDBMS[TDR_DBMS_NAME_LEN];		/**<DBMSϵͳ��*/
	char szSqlFile[TDR_MAX_PATH_LEN];	/**<�������ɺõĽ�����޸ı�SQL�����ļ���*/
	char szDBMSEngine[TDR_DBMS_TABLE_ENGINE_LEN];		/**<DBMS���ݿ�������,����mysql�����ǣ�ISAM,MyISAM, InnoDB*/
	char szDBMSCharset[TDR_DBMS_TABLE_CHARTSET_LEN];     /**<DBMS���ݿ����ַ���*/
};
typedef struct tagTDRMeta2TabOption   TDRMETA2TABOPTION;
typedef struct tagTDRMeta2TabOption *LPTDRMETA2TABOPTION;

static void tdr_show_help(const char *a_pszObj);

static int tdrtools_meta2tab_file_i(IN LPTDRMETA2TABOPTION a_pstOption,
        IN LPTDRMETA a_pstMeta, IN FILE* a_fpSqlFile, IN const char* a_pszFormat);

/**�������й��ߵ�ѡ��
*@retval 0 ��ȷ��������ѡ��
*@retval >0 ��ȷ��������ѡ��,��ָʾ�����й��߲��ؼ���ִ��
*@retval <0 ����ѡ��ʧ��
*/
static int tdr_parse_option(OUT LPTDRMETA2TABOPTION a_pstOption, IN int a_argc, IN char *a_argv[]);

/**ʹ��ָ��Ԫ�����ļ������ɽ������
*/
static int tdrtools_meta2tab_file(IN LPTDRMETA2TABOPTION a_pstOption,
        IN const char *a_pszLibFile, IN const char* a_pszFormat, IN int a_iNeedReplaceSqlFile);

/**ʹ��ָ��Ŀ¼�µ�Ԫ�����ļ������ɽ������
*/
static int tdrtools_meta2tab_dir(IN LPTDRMETA2TABOPTION a_pstOption);


int main(int argc, char *argv[])
{
	TDRMETA2TABOPTION stOption;
	int iRet;

	memset(&stOption, 0, sizeof(stOption));
	strncpy(stOption.szDBMS, "mysql", sizeof(stOption.szDBMS));
	strncpy(stOption.szTDRDir, ".", sizeof(stOption.szTDRDir));
	strncpy(stOption.szSqlFile, TDR_DEFAUTL_SQL_FILE, sizeof(stOption.szTDRDir));
	strncpy(stOption.szDBMSEngine, TDR_MYSQL_DEFAULT_TABLE_ENGINE, sizeof(stOption.szDBMSEngine));
	strncpy(stOption.szDBMSCharset, TDR_MYSQL_DEFAULT_TABLE_CHARTSET, sizeof(stOption.szDBMSCharset));
	stOption.iVersion = TDR_MAX_VERSION;
	iRet = tdr_parse_option(&stOption, argc, argv);
	if (0 != iRet)
	{
		//getchar();
		return iRet;
	}

	if ('\0' != stOption.szTDRFile[0])
	{
		/*ֻʹ��ָ�����ļ�������ָ����Ԫ����,�����ɽ�����޸ı�����*/
		iRet = tdrtools_meta2tab_file(&stOption, stOption.szTDRFile, "", 0);
	}else
	{
		/*ʹ��ָ����Ŀ¼�е�Ԫ�����ļ�������ָ����Ԫ����,�����ɽ�����޸ı�����*/
		iRet = tdrtools_meta2tab_dir(&stOption);
	}


	//getchar();

	return iRet;
}

void tdr_show_help(const char *a_pszObj)
{
	const char *pszApp;

	assert(NULL != a_pszObj);
	pszApp = tdr_basename(a_pszObj);
	printf ("\n- ����Ԫ����������������Ҫ�Ľ�������޸ı���䣬Ԫ�����������ڵ�Ԫ�����ļ�ʹ��xml2dr�������ɡ�\n");
	printf ("\n�÷�: %s [-D --metalib-dir=DIR] [-B --meta_file=FILE]   [-a --alter_table=MetaVersion] "
            "[-o --out_file=FILE] -m --meta_name=NAME [-s --dbms_name=DBMS] [-e --dbms_engine=Engine]"
		"[-c --charset=Charset] [-h --help] [-v --version]\n",
		pszApp);
	printf ("\nѡ��:\n");
	printf ("-D, --metalib_dir=DIR\n"
            "\t\t\tָ��Ԫ�����ļ�������Ŀ¼��ȱʡΪ��ǰĿ¼��\n");
    printf ("\t\t\t�����ض���Ԫ�������͵�ʱ�򣬻��ڸ�Ŀ¼�µ�������.tdr��β���ļ���Ѱ�ҡ�\n");
	printf ("-B, --meta_file=FILE\n"
            "\t\t\tָ��Ԫ�����ļ������֡�\n");
    printf ("\t\t\t���ָ��-B����ֻʹ��ָ�����ļ�������ָ����Ԫ���ݡ�\n");
	printf ("-a, --alter_table=MetaVersion\n"
            "\t\t\tָ��Ԫ���ݵİ汾�š�\n");
    printf ("\t\t\t��ѡ��ָʾ�������ɴ�Ԫ����ָ���汾�����°汾��Alter Table��Sql��䡣\n");
	printf ("-s, --dbms_name=DBMS\n"
            "\t\t\tָ��Ŀǰʹ�õ�DBMSϵͳ����ȱʡΪMySql��\n");
	printf ("-o, --out_file=FILE\n"
            "\t\t\tָ���������ɺõĽ�����޸ı������ļ�����\n");
    printf ("\t\t\t���ָ����--meta_name=NAME��--meta_file=FILEѡ���Ĭ��ֵΪa.xml��\n");
    printf ("\t\t\t���ָ����--meta_name=NAME��--metalib_dir=DIRѡ���Ĭ��ֵΪDIR/*.tdr.NAME.sql��\n");
    printf ("\t\t\t���û��ָ��--meta_name=NAMEѡ���Ĭ��ֵΪ[DIR/]*.tdr.sql��\n");
	printf ("-m, --meta_name=NAME\n"
            "\t\t\tָ��Ԫ�������͵����֡�\n");
    printf ("\t\t\t��δָ����ѡ���tdr����Ԫ�����Ƿ������ݿ�������Ծ����Ƿ�����sql��䡣\n");
    printf ("-A, --for-all-meta\n"
            "\t\t\t��δָ��--meta_name=NAMEѡ��������£�ָʾ����Ϊ����Ԫ��������sql��䡣\n");
	printf ("-e --dbms_engine=Engine\n"
            "\t\t\tָ�����ݿ���������������ȱʡֵΪ'%s'��\n",
		TDR_MYSQL_DEFAULT_TABLE_ENGINE);
	printf ("-c --charset=Charset\n"
            "\t\t\tָ�����ݿ���ַ������ƣ�ȱʡֵΪ'%s'��\n",
		TDR_MYSQL_DEFAULT_TABLE_CHARTSET);
	printf ("-h, --help\n"
            "\t\t\t���ʹ�ð�����\n");
	printf ("-v, --version\n"
            "\t\t\t����汾��Ϣ��\n");
	printf ("\nʹ��ʾ��:\n");
	printf ("%s -D lib -o res_item.sql   -m ResItem \n", pszApp);
	printf ("\t#����libĿ¼����*.tdr��β��Ԫ�����������ļ�����ResITem�����ݿ⽨�����\n");
    printf ("%s -B ov_res.tdr --out_file=res_item.sql --meta_name=ResItem\n", pszApp);
    printf ("\t#ʹ��ov_res.tdrԪ���ݿ�����ResItem�Ľ������\n");
	printf ("%s --meta_file=ov_res.tdr --alter_table=1 -m ResItem\n", pszApp);
	printf ("\t#ʹ��ov_res.tdrԪ���ݿ�����ResItem�Ӱ汾1����ǰ���°汾���޸����ݱ����\n");
    printf ("\n");
}

int tdr_parse_option(OUT LPTDRMETA2TABOPTION a_pstOption, IN int a_argc, IN char *a_argv[])
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
			{"metalib_dir", 1, 0, 'D'},
			{"meta_file", 1, 0, 'B'},
			{"meta_name", 1, 0, 'm'},
			{"alter_table", 1, 0, 'a'},
			{"dbms_name", 1, 0, 's'},
			{"out_file", 1, 0, 'o'},
			{"dbms_engine", 1, 0, 'e'},
			{"charset", 1, 0, 'c'},
			{"version", 0, 0, 'v'},
            {"for-all-meta", 0, 0, 'A'},
			{0, 0, 0, 0}
		};

		opt = getopt_long (a_argc, a_argv, "e:c:D:B:m:a:s:o:Ahv",
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
		case 'D':
			{
				iWriteLen = snprintf(a_pstOption->szTDRDir, sizeof(a_pstOption->szTDRDir), "%s", optarg);
				if ((0 > iWriteLen) || (iWriteLen >= (int)(sizeof(a_pstOption->szTDRDir))))
				{
					printf("Error: ָ��Ŀ¼��̫����Ŀǰ���֧��%d���ַ���Ŀ¼��\n", (int)sizeof(a_pstOption->szTDRDir) -1);
					iRet = 1;
				}
				break;
			}
		case 'B':
			{
				iWriteLen = snprintf(a_pstOption->szTDRFile, sizeof(a_pstOption->szTDRFile), "%s", optarg);
				if ((0 > iWriteLen) || (iWriteLen >= (int)(sizeof(a_pstOption->szTDRFile))))
				{
					printf("Error: ָ��Ԫ�����ļ���̫����Ŀǰ���֧��%d���ַ���Ԫ�����ļ���\n", (int)sizeof(a_pstOption->szTDRFile) -1);
					iRet = 1;
				}
				break;
			}
		case 'm':
			{
				iWriteLen = snprintf(a_pstOption->szMetaName, sizeof(a_pstOption->szMetaName), "%s", optarg);
				if ((0 > iWriteLen) || (iWriteLen >= (int)(sizeof(a_pstOption->szMetaName))))
				{
					printf("Error: ָ��Ԫ������̫����Ŀǰ���֧��%d���ַ���Ԫ������\n", (int)sizeof(a_pstOption->szMetaName) -1);
					iRet = 1;
				}
				break;
			}
		case 's':
			{
				iWriteLen = snprintf(a_pstOption->szDBMS, sizeof(a_pstOption->szDBMS), "%s", optarg);
				if ((0 > iWriteLen) || (iWriteLen >= (int)(sizeof(a_pstOption->szDBMS))))
				{
					printf("Error: ָ��DBMSϵͳ��̫����Ŀǰ���֧��%d���ַ���DBMSϵͳ��\n", (int)sizeof(a_pstOption->szDBMS) -1);
					iRet = 1;
				}
				break;
			}
		case 'o':
			{
                a_pstOption->iHasSpecifySqlFile = 1;
				iWriteLen = snprintf(a_pstOption->szSqlFile, sizeof(a_pstOption->szSqlFile), "%s", optarg);
				if ((0 > iWriteLen) || (iWriteLen >= (int)(sizeof(a_pstOption->szSqlFile))))
				{
					printf("Error: ָ������SQL�����ļ����ļ���̫����Ŀǰ���֧��%d���ַ����ļ���\n", (int)sizeof(a_pstOption->szSqlFile) -1);
					iRet = 1;
				}
				break;
			}
		case 'a':
			{
				a_pstOption->iVersion =  (int)strtol(optarg, NULL, 0);
				a_pstOption->iAlter = 1;
				break;
			}
		case 'e':
			{
				STRNCPY(a_pstOption->szDBMSEngine, optarg, sizeof(a_pstOption->szDBMSEngine));
				break;
			}
		case 'c':
			{
				STRNCPY(a_pstOption->szDBMSCharset, optarg, sizeof(a_pstOption->szDBMSCharset));
				break;
			}
        case 'A':
            {
                a_pstOption->iGenerateForAllMeta = 1;
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
				break;
			}
		default:
			{
				iRet = -1;
				printf("%s: ��Чѡ�� --%c \n", a_argv[0], (char)opt);
				printf("����ִ�� %s -h ��ȡ������Ϣ\n", a_argv[0]);
                exit(0);
				break;
			}
		}
	}/*while (1)*/

	if (0 != iRet)
	{
		return iRet;
	}


	if (a_pstOption->szMetaName[0] == '\0')
	{
		printf("δָ���������ɽ�����޸ı�����Ԫ��������, "
                "������ָ��[Ŀ¼�µ�����]Ԫ�����ļ�������������Ԫ���ݵĽ�����޸ı����䡣\n");
	}

	return iRet;
}

int tdrtools_meta2tab_file_i(IN LPTDRMETA2TABOPTION a_pstOption, IN LPTDRMETA a_pstMeta, IN FILE* a_fpSqlFile, IN const char* a_pszFormat)
{
	int iRet = TDR_SUCCESS;
	TDRDBMS stDBMS;

	assert(NULL != a_pstOption);
	assert(NULL != a_pstMeta);
    assert(NULL != a_fpSqlFile);

	memset(&stDBMS, 0, sizeof(stDBMS));

    printf("%sProcessing meta \'%s\' ......\n", a_pszFormat, tdr_get_meta_name(a_pstMeta));
	printf("%s�ṹ��%s��ǰ�汾��%d\n", a_pszFormat, tdr_get_meta_name(a_pstMeta), tdr_get_meta_current_version(a_pstMeta));

	memset(&stDBMS, 0, sizeof(stDBMS));
	STRNCPY(stDBMS.szDBMSName, a_pstOption->szDBMS, sizeof(stDBMS.szDBMSName));
	STRNCPY(stDBMS.szDBMSEngine, a_pstOption->szDBMSEngine, sizeof(stDBMS.szDBMSEngine));
	STRNCPY(stDBMS.szDBMSCharset, a_pstOption->szDBMSCharset, sizeof(stDBMS.szDBMSCharset));
	if (0 == a_pstOption->iAlter)
	{
		iRet = tdr_create_table_fp(&stDBMS, a_pstMeta, a_fpSqlFile, TDR_MAX_VERSION);
		if (TDR_ERR_IS_ERROR(iRet))
		{
			printf("%s\tError: Ϊ%s���ɽ������ʧ��, for %s\n",
                    a_pszFormat, tdr_get_meta_name(a_pstMeta), tdr_error_string(iRet));
		}
	}else
	{
		iRet = tdr_alter_table_fp(&stDBMS, a_pstMeta, a_fpSqlFile, a_pstOption->iVersion);
		if (TDR_ERR_IS_ERROR(iRet))
		{
			printf("%s\tError: Ϊ%s�����޸ı����ʧ��, for %s\n",
                    a_pszFormat, tdr_get_meta_name(a_pstMeta), tdr_error_string(iRet));
		}
	}/*if (0 != a_pstOption->iAlter)*/

	return iRet;
}

int tdrtools_meta2tab_file(IN LPTDRMETA2TABOPTION a_pstOption,
        IN const char *a_pszLibFile, IN const char* a_pszFormat, IN int a_iNeedReplaceSqlFile)
{
	int iRet = TDR_SUCCESS;
	LPTDRMETALIB pstLib;
	LPTDRMETA pstMeta;
	TDRDBMS stDBMS;
    FILE* fpSqlFile;

	assert(NULL != a_pstOption);
	assert(NULL != a_pszLibFile);

	memset(&stDBMS, 0, sizeof(stDBMS));
	iRet = tdr_load_metalib(&pstLib, a_pszLibFile);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("Error: ͨ���ļ�<%s>����Ԫ���ݿ�ʧ��: %s\n", a_pszLibFile, tdr_error_string(iRet));
		return iRet;
	}

	if ('\0' != a_pstOption->szMetaName[0])
    {
        int iWriteLen;
        /*����ָ��Ԫ����*/
        pstMeta = tdr_get_meta_by_name(pstLib, a_pstOption->szMetaName);
        if (NULL == pstMeta)
        {
            printf("��Ԫ���ݿ�<%s>��û���ҵ�����Ϊ<%s>��Ԫ����\n",
                    tdr_get_metalib_name(pstLib), a_pstOption->szMetaName);
            tdr_free_lib(&pstLib);
            return TDR_ERR_ERROR;
        }

        if (a_iNeedReplaceSqlFile)
        {
            /* replace a_pstOption->szSqlFile */
            iWriteLen = snprintf(a_pstOption->szSqlFile, sizeof(a_pstOption->szSqlFile),
                    "%s.%s.sql", a_pszLibFile, tdr_get_meta_name(pstMeta));
            if (0 > iWriteLen || (int)sizeof(a_pstOption->szSqlFile) <= iWriteLen)
            {
                printf("Error: ����%s��%s���ɱ���sql�����ļ���ʧ�ܣ���Ϊ���ɵ��ļ���̫��\n",
                        a_pszLibFile, tdr_get_meta_name(pstMeta));
                return TDR_ERR_ERROR;
            }
        }

        fpSqlFile = fopen(a_pstOption->szSqlFile, "w");
        if (NULL == fpSqlFile)
        {
            printf("Error: �޷����ļ�%s, for write\n", a_pstOption->szSqlFile);
            return TDR_ERR_ERROR;
        }

        iRet = tdrtools_meta2tab_file_i(a_pstOption, pstMeta, fpSqlFile, a_pszFormat);
        if (!TDR_ERR_IS_ERROR(iRet))
        {
            if (0 != a_pstOption->iAlter)
            {
                printf("%s�ɹ�Ϊ%s�����޸ı���䣬���������ļ�<%s>��\n",
                        a_pszFormat, tdr_get_meta_name(pstMeta), a_pstOption->szSqlFile);
            }else
            {
                printf("%s�ɹ�Ϊ%s���ɽ�����䣬���������ļ�<%s>��\n",
                        a_pszFormat, tdr_get_meta_name(pstMeta), a_pstOption->szSqlFile);
            }
        }
    }else
    {
        int i;

        if (!a_pstOption->iHasSpecifySqlFile)
        {
            /* replace a_pstOption->szSqlFile */
            /* if replace failed, a_pstOption->szSqlFile contains TDR_DEFAUTL_SQL_FILE */
            snprintf(a_pstOption->szSqlFile, sizeof(a_pstOption->szSqlFile), "%s.sql", a_pszLibFile);
        }

        fpSqlFile = fopen(a_pstOption->szSqlFile, "w");
        if (NULL == fpSqlFile)
        {
            printf("Error: �޷����ļ�%sfor write\n", a_pstOption->szSqlFile);
            return TDR_ERR_ERROR;
        }

        for (i = 0; i < pstLib->iCurMetaNum; i++)
        {
            pstMeta =  TDR_IDX_TO_META(pstLib, i);
            if (a_pstOption->iGenerateForAllMeta
                    || pstMeta->nPrimayKeyMemberNum > 0
                    || pstMeta->idxSplitTableFactor >= 0
                    || TDR_SPLITTABLE_RULE_NONE != pstMeta->nSplitTableRuleID)
            {
                iRet = tdrtools_meta2tab_file_i(a_pstOption, pstMeta, fpSqlFile, a_pszFormat);
                if (0 != a_pstOption->iAlter)
                {
                    printf("%s�ɹ�Ϊ%s�����޸ı���䣬���������ļ�<%s>��\n",
                            a_pszFormat, tdr_get_meta_name(pstMeta), a_pstOption->szSqlFile);
                }else
                {
                    printf("%s�ɹ�Ϊ%s���ɽ�����䣬���������ļ�<%s>��\n",
                            a_pszFormat, tdr_get_meta_name(pstMeta), a_pstOption->szSqlFile);
                }
            }
        }
    }

	tdr_free_lib(&pstLib);

	return iRet;
}

int tdrtools_meta2tab_dir(IN LPTDRMETA2TABOPTION a_pstOption)
{
	int iRet = TDR_SUCCESS;
	struct  dirent  *pstDirent = NULL;
	DIR		*pstDir = NULL;
	char *pCh;
	int iSucNum;
	char szPath[TDR_MAX_PATH_LEN+2] = {0};
	char szFile[TDR_MAX_PATH_LEN*2];
	int iLen;

	assert(NULL != a_pstOption);

	pstDir = opendir(a_pstOption->szTDRDir);
	if (NULL == pstDir)
	{
		printf("Error: failed to open directory %s\n", a_pstOption->szTDRDir);
		return -1;
	}

	iLen = (int)strlen(a_pstOption->szTDRDir);
	strncpy(szPath, a_pstOption->szTDRDir, iLen);
	if (a_pstOption->szTDRDir[iLen - 1] != TDR_DIR_PATH_CHAR)
	{
		szPath[iLen] = TDR_DIR_PATH_CHAR;
	}



	iSucNum = 0;
	while ((pstDirent = readdir(pstDir)) != NULL)
	{
		if ((strcmp((const char*)pstDirent->d_name, ".")  ==0) ||
			(strcmp((const char*)pstDirent->d_name, "..") == 0))
		{
			continue;
		}

		/*������.tdr��β���ļ�*/
		pCh = (char*)strrchr((const char*)pstDirent->d_name, '.');
		if (NULL == pCh)
		{
			continue;
		}
		pCh++;
		if (0 != strcmp(pCh, "tdr"))
		{
			continue;
		}

        printf("Processing meta-file \"%s\" ......\n", pstDirent->d_name);
		iLen = snprintf(szFile, sizeof(szFile),"%s%s", szPath, pstDirent->d_name);
		if ((0 > iLen) || (iLen >= (int)sizeof(szFile)))
		{
			printf("Warniing: %sĿ¼�µ��ļ�%s�ļ���̫��, δ������ļ�\n", szPath, pstDirent->d_name);
			continue;
		}
		iRet = tdrtools_meta2tab_file(a_pstOption, szFile, "\t", 1);
		if (TDR_SUCCESS == iRet)
		{
			iSucNum++;
		}
	}//while ((dirp = readdir(dp)) != NULL)

	closedir(pstDir);

	if (0 >= iSucNum)
	{
		/*һ��Ԫ�����ļ�Ҳû��*/
		printf("Error: ���ɽ�����޸ı����ʧ�ܣ���ȷ��Ŀ¼<%s>���ж�����Ԫ����<%s>��Ԫ�����ļ�����ȷ�������������Ч��\n",
			a_pstOption->szTDRDir, a_pstOption->szMetaName);
		iRet = TDR_ERR_ERROR;
	}else
	{
		iRet = TDR_SUCCESS;
	}

	return iRet;
}
