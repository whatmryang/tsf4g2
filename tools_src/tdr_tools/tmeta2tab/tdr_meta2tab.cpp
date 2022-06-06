/**
*
* @file     tdr_meta2tab.c
* @brief    根据元数据描述生成所需要的建表或者修改表语句
*
* @author jackyai
* @version 1.0
* @date 2007-08-02
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


#define TDR_MAX_PATH_LEN		   512 /**<文件目录或文件名最大长度*/
#define TDR_DEFAUTL_SQL_FILE	"a.sql"

#if defined(_WIN32) || defined(_WIN64)
#define TDR_DIR_PATH_CHAR		'\\'
#else
#define TDR_DIR_PATH_CHAR		'/'
#endif



struct tagTDRMeta2TabOption
{
	int iVersion;     /**<指定元数据的版本*/
	int iAlter;		/*是否生成修改表语句，0：不生成，非0值： 生成修改表语句*/
    int iHasSpecifySqlFile;
    int iGenerateForAllMeta;
	char szTDRDir[TDR_MAX_PATH_LEN];	/**<元数据文件查找目录*/
	char szTDRFile[TDR_MAX_PATH_LEN];	/**<元数据文件名*/
	char szMetaName[TDR_NAME_LEN];		/**<元数据名*/
	char szDBMS[TDR_DBMS_NAME_LEN];		/**<DBMS系统名*/
	char szSqlFile[TDR_MAX_PATH_LEN];	/**<保存生成好的建表或修改表SQL语句的文件名*/
	char szDBMSEngine[TDR_DBMS_TABLE_ENGINE_LEN];		/**<DBMS数据库表的引擎,对于mysql可以是：ISAM,MyISAM, InnoDB*/
	char szDBMSCharset[TDR_DBMS_TABLE_CHARTSET_LEN];     /**<DBMS数据库表的字符集*/
};
typedef struct tagTDRMeta2TabOption   TDRMETA2TABOPTION;
typedef struct tagTDRMeta2TabOption *LPTDRMETA2TABOPTION;

static void tdr_show_help(const char *a_pszObj);

static int tdrtools_meta2tab_file_i(IN LPTDRMETA2TABOPTION a_pstOption,
        IN LPTDRMETA a_pstMeta, IN FILE* a_fpSqlFile, IN const char* a_pszFormat);

/**分析运行工具的选项
*@retval 0 正确分析出各选项
*@retval >0 正确分析出各选项,但指示此运行工具不必继续执行
*@retval <0 分析选项失败
*/
static int tdr_parse_option(OUT LPTDRMETA2TABOPTION a_pstOption, IN int a_argc, IN char *a_argv[]);

/**使用指定元数据文件来生成建表语句
*/
static int tdrtools_meta2tab_file(IN LPTDRMETA2TABOPTION a_pstOption,
        IN const char *a_pszLibFile, IN const char* a_pszFormat, IN int a_iNeedReplaceSqlFile);

/**使用指定目录下的元数据文件来生成建表语句
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
		/*只使用指定的文件来查找指定的元数据,以生成建表或修改表的语句*/
		iRet = tdrtools_meta2tab_file(&stOption, stOption.szTDRFile, "", 0);
	}else
	{
		/*使用指定的目录中的元数据文件来查找指定的元数据,以生成建表或修改表的语句*/
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
	printf ("\n- 利用元数据描述生成所需要的建表或者修改表语句，元数据描述所在的元数据文件使用xml2dr工具生成。\n");
	printf ("\n用法: %s [-D --metalib-dir=DIR] [-B --meta_file=FILE]   [-a --alter_table=MetaVersion] "
            "[-o --out_file=FILE] -m --meta_name=NAME [-s --dbms_name=DBMS] [-e --dbms_engine=Engine]"
		"[-c --charset=Charset] [-h --help] [-v --version]\n",
		pszApp);
	printf ("\n选项:\n");
	printf ("-D, --metalib_dir=DIR\n"
            "\t\t\t指定元数据文件搜索的目录，缺省为当前目录。\n");
    printf ("\t\t\t搜索特定的元数据类型的时候，会在该目录下的所有以.tdr结尾的文件中寻找。\n");
	printf ("-B, --meta_file=FILE\n"
            "\t\t\t指定元数据文件的名字。\n");
    printf ("\t\t\t如果指定-B，则只使用指定的文件来查找指定的元数据。\n");
	printf ("-a, --alter_table=MetaVersion\n"
            "\t\t\t指定元数据的版本号。\n");
    printf ("\t\t\t此选项指示工具生成从元数据指定版本到最新版本的Alter Table的Sql语句。\n");
	printf ("-s, --dbms_name=DBMS\n"
            "\t\t\t指定目前使用的DBMS系统名，缺省为MySql。\n");
	printf ("-o, --out_file=FILE\n"
            "\t\t\t指定保存生成好的建表或修改表语句的文件名。\n");
    printf ("\t\t\t如果指定了--meta_name=NAME和--meta_file=FILE选项，则默认值为a.xml。\n");
    printf ("\t\t\t如果指定了--meta_name=NAME和--metalib_dir=DIR选项，则默认值为DIR/*.tdr.NAME.sql。\n");
    printf ("\t\t\t如果没有指定--meta_name=NAME选项，则默认值为[DIR/]*.tdr.sql。\n");
	printf ("-m, --meta_name=NAME\n"
            "\t\t\t指定元数据类型的名字。\n");
    printf ("\t\t\t如未指定此选项，则tdr根据元数据是否有数据库相关属性决定是否生成sql语句。\n");
    printf ("-A, --for-all-meta\n"
            "\t\t\t在未指定--meta_name=NAME选项的条件下，指示工具为所有元数据生成sql语句。\n");
	printf ("-e --dbms_engine=Engine\n"
            "\t\t\t指定数据库表引擎的类型名，缺省值为'%s'。\n",
		TDR_MYSQL_DEFAULT_TABLE_ENGINE);
	printf ("-c --charset=Charset\n"
            "\t\t\t指定数据库表字符集名称，缺省值为'%s'。\n",
		TDR_MYSQL_DEFAULT_TABLE_CHARTSET);
	printf ("-h, --help\n"
            "\t\t\t输出使用帮助。\n");
	printf ("-v, --version\n"
            "\t\t\t输出版本信息。\n");
	printf ("\n使用示例:\n");
	printf ("%s -D lib -o res_item.sql   -m ResItem \n", pszApp);
	printf ("\t#利用lib目录下以*.tdr结尾的元数据描述库文件生成ResITem的数据库建表语句\n");
    printf ("%s -B ov_res.tdr --out_file=res_item.sql --meta_name=ResItem\n", pszApp);
    printf ("\t#使用ov_res.tdr元数据库生成ResItem的建表语句\n");
	printf ("%s --meta_file=ov_res.tdr --alter_table=1 -m ResItem\n", pszApp);
	printf ("\t#使用ov_res.tdr元数据库生成ResItem从版本1到当前最新版本的修改数据表语句\n");
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
					printf("Error: 指定目录名太长，目前最多支持%d个字符的目录名\n", (int)sizeof(a_pstOption->szTDRDir) -1);
					iRet = 1;
				}
				break;
			}
		case 'B':
			{
				iWriteLen = snprintf(a_pstOption->szTDRFile, sizeof(a_pstOption->szTDRFile), "%s", optarg);
				if ((0 > iWriteLen) || (iWriteLen >= (int)(sizeof(a_pstOption->szTDRFile))))
				{
					printf("Error: 指定元数据文件名太长，目前最多支持%d个字符的元数据文件名\n", (int)sizeof(a_pstOption->szTDRFile) -1);
					iRet = 1;
				}
				break;
			}
		case 'm':
			{
				iWriteLen = snprintf(a_pstOption->szMetaName, sizeof(a_pstOption->szMetaName), "%s", optarg);
				if ((0 > iWriteLen) || (iWriteLen >= (int)(sizeof(a_pstOption->szMetaName))))
				{
					printf("Error: 指定元数据名太长，目前最多支持%d个字符的元数据名\n", (int)sizeof(a_pstOption->szMetaName) -1);
					iRet = 1;
				}
				break;
			}
		case 's':
			{
				iWriteLen = snprintf(a_pstOption->szDBMS, sizeof(a_pstOption->szDBMS), "%s", optarg);
				if ((0 > iWriteLen) || (iWriteLen >= (int)(sizeof(a_pstOption->szDBMS))))
				{
					printf("Error: 指定DBMS系统名太长，目前最多支持%d个字符的DBMS系统名\n", (int)sizeof(a_pstOption->szDBMS) -1);
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
					printf("Error: 指定保存SQL语句的文件的文件名太长，目前最多支持%d个字符的文件名\n", (int)sizeof(a_pstOption->szSqlFile) -1);
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
				printf("%s: 无效选项 --%c \n", a_argv[0], (char)opt);
				printf("可以执行 %s -h 获取更多信息\n", a_argv[0]);
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
		printf("未指定用来生成建表或修改表语句的元数据名称, "
                "将生成指定[目录下的所有]元数据文件所包含的所有元数据的建表或修改表的语句。\n");
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
	printf("%s结构体%s当前版本：%d\n", a_pszFormat, tdr_get_meta_name(a_pstMeta), tdr_get_meta_current_version(a_pstMeta));

	memset(&stDBMS, 0, sizeof(stDBMS));
	STRNCPY(stDBMS.szDBMSName, a_pstOption->szDBMS, sizeof(stDBMS.szDBMSName));
	STRNCPY(stDBMS.szDBMSEngine, a_pstOption->szDBMSEngine, sizeof(stDBMS.szDBMSEngine));
	STRNCPY(stDBMS.szDBMSCharset, a_pstOption->szDBMSCharset, sizeof(stDBMS.szDBMSCharset));
	if (0 == a_pstOption->iAlter)
	{
		iRet = tdr_create_table_fp(&stDBMS, a_pstMeta, a_fpSqlFile, TDR_MAX_VERSION);
		if (TDR_ERR_IS_ERROR(iRet))
		{
			printf("%s\tError: 为%s生成建表语句失败, for %s\n",
                    a_pszFormat, tdr_get_meta_name(a_pstMeta), tdr_error_string(iRet));
		}
	}else
	{
		iRet = tdr_alter_table_fp(&stDBMS, a_pstMeta, a_fpSqlFile, a_pstOption->iVersion);
		if (TDR_ERR_IS_ERROR(iRet))
		{
			printf("%s\tError: 为%s生成修改表语句失败, for %s\n",
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
		printf("Error: 通过文件<%s>加载元数据库失败: %s\n", a_pszLibFile, tdr_error_string(iRet));
		return iRet;
	}

	if ('\0' != a_pstOption->szMetaName[0])
    {
        int iWriteLen;
        /*查找指定元数据*/
        pstMeta = tdr_get_meta_by_name(pstLib, a_pstOption->szMetaName);
        if (NULL == pstMeta)
        {
            printf("在元数据库<%s>中没有找到名字为<%s>的元数据\n",
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
                printf("Error: 根据%s和%s生成保存sql语句的文件名失败，因为生成的文件名太长\n",
                        a_pszLibFile, tdr_get_meta_name(pstMeta));
                return TDR_ERR_ERROR;
            }
        }

        fpSqlFile = fopen(a_pstOption->szSqlFile, "w");
        if (NULL == fpSqlFile)
        {
            printf("Error: 无法打开文件%s, for write\n", a_pstOption->szSqlFile);
            return TDR_ERR_ERROR;
        }

        iRet = tdrtools_meta2tab_file_i(a_pstOption, pstMeta, fpSqlFile, a_pszFormat);
        if (!TDR_ERR_IS_ERROR(iRet))
        {
            if (0 != a_pstOption->iAlter)
            {
                printf("%s成功为%s生成修改表语句，并保存在文件<%s>中\n",
                        a_pszFormat, tdr_get_meta_name(pstMeta), a_pstOption->szSqlFile);
            }else
            {
                printf("%s成功为%s生成建表语句，并保存在文件<%s>中\n",
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
            printf("Error: 无法打开文件%sfor write\n", a_pstOption->szSqlFile);
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
                    printf("%s成功为%s生成修改表语句，并保存在文件<%s>中\n",
                            a_pszFormat, tdr_get_meta_name(pstMeta), a_pstOption->szSqlFile);
                }else
                {
                    printf("%s成功为%s生成建表语句，并保存在文件<%s>中\n",
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

		/*查找以.tdr结尾的文件*/
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
			printf("Warniing: %s目录下的文件%s文件名太长, 未处理此文件\n", szPath, pstDirent->d_name);
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
		/*一个元数据文件也没有*/
		printf("Error: 生成建表或修改表语句失败，请确保目录<%s>下有定义了元数据<%s>的元数据文件，并确保输入参数的有效性\n",
			a_pstOption->szTDRDir, a_pstOption->szMetaName);
		iRet = TDR_ERR_ERROR;
	}else
	{
		iRet = TDR_SUCCESS;
	}

	return iRet;
}
