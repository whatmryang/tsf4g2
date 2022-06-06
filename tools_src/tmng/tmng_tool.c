
#include    <stdio.h>
#include    <stdlib.h>
#include    <curses.h>

#include    "tmng/tmng.h"
#include    "tdr/tdr.h"

#include    "terror.h"
#include    "tmng_def.h"
#include    "tmng_errno.h"
#include    "comm/tini.h"

#include    "tdr_metalib_kernel_i.h"
#include    "tdr_auxtools.h"

#define     CUR_VER    0x01

#define     CREATE_MIB          1
#define     CREATE_METEBASE     2

#define     DESTORY_MIB         3
#define     DESTORY_METEBASE    4

#define     DUMP_MIB            5
#define     DUMP_METEBASE       6

#define     LOAD_DATA           7
#define     LOAD_META           8

#define     REGIST_DATA         9
#define     MODIFY_DATA         10

#define     LIST_PROC           61

#define     FORMAT_TXT          0
#define     FORMAT_XML          1
#define     FORMAT_TDR          2


#define STR_NOT_NULL(S) ((S) && ((S)[0]))
#define STR_NULL(S) (!(STR_NOT_NULL(S)))

#define STR_N_CPY(D, S) \
    do \
    { \
        strncpy(D, S, sizeof(D) - 1); \
        D[sizeof(D) - 1] = '\0'; \
    } \
    while (0)


typedef struct tagGlobal
{
    int32_t     argc;
    char      **argv;
    int32_t     iAction;

    const char *pszLoadfile;
    const char *pszAlias;
    const char *pszDomain;
    const char *pszMeta;
    const char *pszLib;
    const char *pszPath;
    const char *pszValue;
    const char *pszDumpPath;
    int32_t     iMibDataVersion;
    int32_t     iDataSize;
    int32_t     iProcId;
    int32_t     iPeriods;
    int32_t     iForce;
    int32_t     iFormat;

    char        szConfFile[80];
    char        szMibKey[KEY_SIZE];
    int32_t     iMibCounts;
    int32_t     iMibSize;

    char        szMetabaseKey[KEY_SIZE];
    int32_t     iMetabaseCounts;
    int32_t     iMetabaseSize;
} GLOBALCFG;

// local function prototype
void FormatTime(time_t, char *, int32_t);
int32_t _usage(char *[]);
int32_t get_mng_cfg(GLOBALCFG *, char *);
int32_t set_entry_value(const LPTDRMETAENTRY,
                        const char *,
                        char *,
                        int32_t,
                        FILE *);
int32_t parse_option(OUT GLOBALCFG *, IN  int32_t, IN  char *[]);
int32_t tmng_load_meta(GLOBALCFG *);
int32_t tmng_load_data(GLOBALCFG *);
int32_t tmng_regist_data(GLOBALCFG *);
int32_t tmng_modify_data(GLOBALCFG *);
int32_t tmng_dump_mib(GLOBALCFG *);
int32_t tmng_list_proc(GLOBALCFG *);


void FormatTime(time_t tSecond, char *szBuf, int32_t iSize)
{
    struct tm stm;

    if (szBuf && iSize > 0)
        szBuf[0] = '\0';

    bzero(&stm, sizeof(struct tm));

    if (szBuf && iSize > 19 && tSecond > 0 && localtime_r(&tSecond, &stm))
    {
        strftime(szBuf, iSize, "%Y-%m-%d %H:%M:%S", &stm);
    }
    else
    {
        T_ERROR_PUSH(-1, "args error or localtime_r fail");
    }
}

int32_t _usage(char *argv[])
{
    printf("Common Usage: %s [options] \n", basename(argv[0]));
    printf(
        "options:\n"
        "\n"
        "\t--create     : specify [mib|metabase]\n"
        "\t--destroy    : specify [mib|metabase]\n"
        "\t--dump       : specify [mib|metabase]\n"
        "\t               option:\n"
        "\t                 --alias <name>, default dump all data\n"
        "\t                 --domain <domain>, default domain =\"CFG\"\n"
        "\t                 --data-ver <ver>, to specify version \n"
        "\t                 --procid <id>, to specify data owner proc \n"
        "\t               dump metabase option:\n"
        "\t                 --format [txt|tdr|xml], default format is txt\n"
        "\t                 --saveto <directory>, default output to stdout\n"
        "\n"
        "\t--loadmeta   : specify meta lib filename\n"
        "\t               option:\n"
        "\t                 --force to force load\n"
        "\t--loaddata   : specify data file use DR description\n"
        "\t               must:\n"
        "\t                 --alias --lib --meta \n"
        "\t               option:\n"
        "\t                 --force to force load\n"
        "\t                 --domain <domain> , default domain =\"CFG\"\n"
        "\t                 --lib --meta  or --size to specify data size\n"
        "\t                 --data-ver to specify version \n"
        "\t                 --periods to specify report periods\n"
        "\t                 --procid to specify data owner proc \n"
        "\t--registdata : to regist data space in mib\n"
        "\t               must  --alias to specify data name in mib\n"
        "\t               option:\n"
        "\t                 --domain <domain>, default domain =\"CFG\"\n"
        "\t                 --data-ver to specify version \n"
        "\t                 --procid to specify data owner proc \n"
        "\t                 --periods to specify report periods\n"
        "\t--modify     : must be use with argumet --alias --path --value \n"
        "\t               option:\n"
        "\t                 --domain <domain>, default domain =\"CFG\"\n"
        "\t                 --data-ver to specify version \n"
        "\t                 --procid to specify data owner proc \n"
        "\t                 --periods to specify report periods\n"
        "\t--alias      : specify alias of cfg data used in loadcfg ->\n"
        "\t               or modify action.\n"
        "\t--domain     : specify domain of mib data used in loadcfg ->\n"
        "\t               or modify action.\n"
        "\t--lib        : specify meta lib used in loadcfg action.\n"
        "\t--meta       : specify meta name used in loadcfg action.\n"
        "\t--data-ver   : specify data version used in loadcfg ->\n"
        "\t               or modify action.\n"
        "\t--Periods    : specify data report periods used in registdata ->\n"
        "\t               or modify action.\n"
        "\t--procid     : specify data owner procid used in loadcfg, ->\n"
        "\t               modify or dump action.\n"
        "\t--path       : specify entry path used in modify action.\n"
        "\t--values     : specify data value used in modify action.\n"
        "\t--size       : specify data size  used in regist data action.\n"
        "\t--config-file: \n"
        "\t          -C : specify the path of config file for this process.\n"
        "\t--conf-<key> : option\n"
        "\t             : use --conf-mibkey        to set MIB:Key\n"
        "\t             : use --conf-mibcount      to set MIB:Count\n"
        "\t             : use --conf-mibsize       to set MIB:Size\n"
        "\t             : use --conf-metabasekey   to set METABASE:Key\n"
        "\t             : use --conf-metabasecount to set METABASE:Count\n"
        "\t             : use --conf-metabasesize  to set METABASE:Size\n"
        "\t--log-file   : specify the path of the file for logging.\n"
        "\t--listproc,-l: list process data \n"
        "\t--version,-v : print version information. \n"
        "\t--help, -h   : print help information. \n");

    return 0;
}

int32_t get_mng_cfg(GLOBALCFG *a_pstCfg, char *a_szCfgFile)
{
    int32_t     iRet;
    char        szTmpStr[40];
    int32_t     iHasConf;
    FILE       *fp;

    /* *************************************************************
     * 命令行参数设置的变量优先
     * 其次是配置文件中设置的变量
     * 最后是缺省变量
     * ************************************************************/

    // iHasConf 为 1，表示配置文件存在且可读
    iHasConf = 0;
    if (STR_NOT_NULL(a_szCfgFile))
    {
        fp = fopen(a_szCfgFile, "r");
        if (NULL != fp)
        {
            fclose(fp);
            iHasConf = 1;
        }
    }

    if (1 == iHasConf)
    {
        /* ******************************************
         * SETVAL_STR(K, S, H, D)
         * SETVAL_INT(K, S, H, D)
         * K 是 GLOBALCFG *a_pstCfg 的成员
         * S 是配置文件的 segment
         * H 是配置文件的 segment 中对应的 key
         * D 是这个成员的缺省值
         *
         * 首先检查成员是否是空指针或者已赋值
         * *****************************************/
#define SETVAL_STR(K, S, H, D) \
    do \
    { \
        if (STR_NOT_NULL(a_pstCfg->K)) \
            break; \
        iRet = tini_read(a_szCfgFile, #S, #H, \
                a_pstCfg->K, sizeof(a_pstCfg->K)); \
        if (0 != iRet && (D)) \
            STR_N_CPY(a_pstCfg->K, D); \
    } \
    while (0)

#define SETVAL_INT(K, S, H, D) \
    do \
    { \
        if (0 != a_pstCfg->K) \
            break; \
        iRet = tini_read(a_szCfgFile, #S, #H, \
                szTmpStr, sizeof(szTmpStr)); \
        if (0 == iRet) \
            a_pstCfg->K = atoi(szTmpStr); \
        else if (D) \
            a_pstCfg->K = D; \
    } \
    while (0)

        SETVAL_STR(szMibKey,        MIB,      Key,   DEFAULT_MIB_KEY);
        SETVAL_INT(iMibCounts,      MIB,      Count, DEFAULT_COUNT);
        SETVAL_INT(iMibSize,        MIB,      Size,  DEFAULT_SIZE);
        SETVAL_STR(szMetabaseKey,   METABASE, Key,   DEFAULT_METABASE_KEY);
        SETVAL_INT(iMetabaseCounts, METABASE, Count, DEFAULT_COUNT);
        SETVAL_INT(iMetabaseSize,   METABASE, Size,  DEFAULT_SIZE);

#undef SETVAL_INT
#undef SETVAL_STR
    }
    else
    {
        /* ******************************************
         * SETVAL_STR(K, D)
         * SETVAL_INT(K, D)
         * K 是 (GLOBALCFG*) a_pstCfg 的成员
         * D 是这个成员的缺省值
         *
         * 首先检查成员是否已赋值
         * *****************************************/

#define SETVAL_STR(K, D) \
    do \
    { \
        if (STR_NULL(a_pstCfg->K) && D) \
            STR_N_CPY(a_pstCfg->K, D); \
    } \
    while (0)

#define SETVAL_INT(K, D) \
    do { \
        if (0 != a_pstCfg->K) \
            break; \
        if (D) \
            a_pstCfg->K = D; \
    } while (0)

        SETVAL_STR(szMibKey,        DEFAULT_MIB_KEY);
        SETVAL_INT(iMibCounts,      DEFAULT_COUNT);
        SETVAL_INT(iMibSize,        DEFAULT_SIZE);
        SETVAL_STR(szMetabaseKey,   DEFAULT_METABASE_KEY);
        SETVAL_INT(iMetabaseCounts, DEFAULT_COUNT);
        SETVAL_INT(iMetabaseSize,   DEFAULT_SIZE);

#undef SETVAL_INT
#undef SETVAL_STR
    }

    return 0;
}

int32_t set_entry_value(const LPTDRMETAENTRY  a_pstEntry,
                        const char           *a_szValue,
                        char                 *a_pValue,
                        int32_t               a_iSize,
                        FILE                 *a_fpError)
{
    tdr_longlong  llVal;
    long          lVal;
    int32_t       iRet;
    const char   *pszError = NULL;

    if (NULL == a_pstEntry || NULL == a_pValue)
    {
        T_ERROR_PUSH(-1, "args error");
        return -TMNG_ERR_ARGS;
    }

    if (NULL == a_fpError)
    {
        a_fpError = stderr;
    }

    iRet = TDR_SUCCESS;
    switch(tdr_get_entry_type(a_pstEntry))
    {
    case TDR_TYPE_STRING:
        {
            TDR_STRNCPY(a_pValue, &a_szValue[0], a_iSize);
            break;
        }
    case TDR_TYPE_CHAR:
        {
            lVal = strtol(a_szValue, NULL, 0);
            if (-128 > lVal || 127 < lVal)
            {
                fprintf(a_fpError,
                        "\nerror:\t 成员<name　= %s>的数据值<%s>与类型不匹配",
                        tdr_get_entry_name(a_pstEntry),
                        a_szValue);
                iRet = TDR_ERRIMPLE_MAKE_ERROR(
                    TDR_ERROR_ENTRY_INVALID_DEFAULT_VALUE
                );
                pszError = "TDR_TYPE_CHAR range";
            }
            else
                *a_pValue = (char)lVal;
            break;
        }
    case TDR_TYPE_UCHAR:
        {
            lVal = strtol(a_szValue, NULL, 0);
            if (0 > lVal || 0xFF < lVal)
            {
                fprintf(a_fpError,
                        "\nerror:\t 成员<name　= %s>的数据值<%s>与类型不匹配",
                        tdr_get_entry_name(a_pstEntry),
                        a_szValue);
                iRet = TDR_ERRIMPLE_MAKE_ERROR(
                    TDR_ERROR_ENTRY_INVALID_DEFAULT_VALUE
                );
                pszError = "TDR_TYPE_UCHAR range";
            }
            else
                *a_pValue = (unsigned char)lVal;
            break;
        }
    case TDR_TYPE_SHORT:
        {
            lVal = strtol(a_szValue, NULL, 0);
            if (-32768 > lVal || 0x7FFF < lVal)
            {
                fprintf(a_fpError,
                        "\nerror:\t 成员<name　= %s>的数据值<%s>与类型不匹配",
                        tdr_get_entry_name(a_pstEntry),
                        a_szValue);
                iRet = TDR_ERRIMPLE_MAKE_ERROR(
                    TDR_ERROR_ENTRY_INVALID_DEFAULT_VALUE
                );
                pszError = "TDR_TYPE_SHORT range";
            }
            else
                *(int16_t *)a_pValue = (int16_t)lVal;
            break;
        }
    case TDR_TYPE_USHORT:
        {
            lVal = strtol(a_szValue, NULL, 0);
            if  (0 > lVal || 0xFFFF < lVal)
            {
                fprintf(a_fpError,
                        "\nerror:\t 成员<name　= %s>的数据值<%s>与类型不匹配",
                        tdr_get_entry_name(a_pstEntry),
                        a_szValue);
                iRet = TDR_ERRIMPLE_MAKE_ERROR(
                    TDR_ERROR_ENTRY_INVALID_DEFAULT_VALUE
                );
                pszError = "TDR_TYPE_USHORT range";
            }
            else
                *(uint16_t *)a_pValue = (uint16_t)lVal;
            break;
        }
    case TDR_TYPE_INT:
    case TDR_TYPE_LONG:
        {
            lVal = strtol(a_szValue, NULL, 0);
            if ((int32_t)0x80000000 > (int32_t)lVal \
                    || 0x7FFFFFFF < (int32_t)lVal)
            {
                fprintf(a_fpError,
                        "\nerror:\t 成员<name　= %s>的数据值<%s>与类型不匹配",
                        tdr_get_entry_name(a_pstEntry),
                        a_szValue);
                iRet = TDR_ERRIMPLE_MAKE_ERROR(
                    TDR_ERROR_ENTRY_INVALID_DEFAULT_VALUE
                );
                pszError = "TDR_TYPE_INT range";
            }
            else
                *(int32_t *)a_pValue = (int32_t)lVal;
            break;
        }
    case TDR_TYPE_UINT:
    case TDR_TYPE_ULONG:
        {
            //lVal = strtol(a_szValue, NULL, 0);
            llVal = TDR_ATOLL(a_szValue);
            if (0 > llVal)
            {
                fprintf(a_fpError,
                        "\nerror:\t 成员<name　= %s>的数据值<%s>与类型不匹配",
                        tdr_get_entry_name(a_pstEntry),
                        a_szValue);
                iRet = TDR_ERRIMPLE_MAKE_ERROR(
                    TDR_ERROR_ENTRY_INVALID_DEFAULT_VALUE
                );
                pszError = "TDR_TYPE_UINT range";
            }
            else
                *(uint32_t *)a_pValue = (uint32_t)llVal;
            break;
        }
    case TDR_TYPE_LONGLONG:
        {
            llVal = TDR_ATOLL(a_szValue);
            *(tdr_longlong *)a_pValue = llVal;
            break;
        }
    case TDR_TYPE_ULONGLONG:
        {
            llVal = TDR_ATOLL(a_szValue);
            if (0 > llVal)
            {
                fprintf(a_fpError,
                        "\nerror:\t 成员<name　= %s>的数据值<%s>与类型不匹配",
                        tdr_get_entry_name(a_pstEntry),
                        a_szValue);
                iRet = TDR_ERRIMPLE_MAKE_ERROR(
                    TDR_ERROR_ENTRY_INVALID_DEFAULT_VALUE
                );
                pszError = "TDR_TYPE_ULONGLONG range";
            }
            else
                *(tdr_ulonglong *)a_pValue = (tdr_ulonglong)llVal;
            break;
        }
    case TDR_TYPE_FLOAT:
        {
            float  fVal;

            fVal = (float)atof(a_szValue);
            *(float *)a_pValue = fVal;
            break;
        }
    case TDR_TYPE_DOUBLE:
        {
            double  dVal;

            dVal = strtod(a_szValue, NULL);
            *(double *)a_pValue = dVal;
            break;
        }
    case TDR_TYPE_IP:
        {
            iRet = tdr_ineta_to_tdrip((tdr_ip_t *)a_pValue , &a_szValue[0]);
            pszError = "TDR_TYPE_ip range";
            break;
        }
    case TDR_TYPE_DATETIME:
        {
            iRet = tdr_str_to_tdrdatetime((tdr_datetime_t *)a_pValue,
                                          &a_szValue[0]);
            if (TDR_ERR_IS_ERROR(iRet))
            {
                fprintf(a_fpError,
                        "\nwarning:\t "
                        "成员<name = %s>日期/时间(datetime)类型的值无效,"
                        "正确datetime类型其值格式应为\"YYYY-MM-DD HH:MM:SS\".",
                        tdr_get_entry_name(a_pstEntry));
                pszError = "TDR_TYPE_DATETIME range";
            }
            break;
        }
    case TDR_TYPE_DATE:
        {
            iRet = tdr_str_to_tdrdate((tdr_date_t *)a_pValue, &a_szValue[0]);
            if (TDR_ERR_IS_ERROR(iRet))
            {
                fprintf(a_fpError,
                        "\nwarning:\t "
                        "成员<name = %s>日期(date)类型的缺省值无效,"
                        "正确date类型其值格式应为\"YYYY-MM-DD\".",
                        tdr_get_entry_name(a_pstEntry));
                pszError = "TDR_TYPE_DATE range";
            }
            break;
        }
    case TDR_TYPE_TIME:
        {
            iRet = tdr_str_to_tdrtime((tdr_time_t *)a_pValue, &a_szValue[0]);
            if (TDR_ERR_IS_ERROR(iRet))
            {
                fprintf(a_fpError,
                        "\nerror:\t "
                        "成员<name = %s>时间(time)类型的缺省值无效,"
                        "正确date类型其值格式应为\"HHH:MM:SS\".",
                        tdr_get_entry_name(a_pstEntry));
                pszError = "TDR_TYPE_TIME range";
            }
            break;
        }
    default:
        {
            fprintf(a_fpError,
                    "\nwarning:\t 成员<name = %s>的类型目前暂不支持设置值.",
                    tdr_get_entry_name(a_pstEntry));
            a_iSize = 0;
            break;
        }
    }

    if (iRet != TDR_SUCCESS)
    {
        T_ERROR_PUSH(iRet, pszError);
    }
    return iRet;
}

int32_t parse_option(OUT GLOBALCFG *a_pstGlData,
                     IN  int32_t    a_argc,
                     IN  char      *a_argv[])
{
    int32_t   opt;
    int32_t   iRet;
    char     *pszApp;

    if (NULL == a_pstGlData || 0 >= a_argc || NULL == a_argv)
    {
        T_ERROR_PUSH(-1, "args error");
        return -TMNG_ERR_ARGS;
    }

    iRet = 0;
    pszApp = basename(a_argv[0]);

    a_pstGlData->argc = a_argc;
    a_pstGlData->argv = a_argv;

    while (1)
    {
        int32_t option_index = 0;
        static struct option stlong_options[] = {
            {"create",              1, 0, 'c'},
            {"dump",                1, 0, 'p'},
            {"saveto",              1, 0, 'S'},
            {"format",              1, 0, 'F'},
            {"destroy",             1, 0, 'd'},
            {"loadmeta",            1, 0, 'L'},
            {"loaddata",            1, 0, 'G'},
            {"force",               0, 0, 'f'},
            {"listproc",            0, 0, 'l'},
            {"registdata",          0, 0, 'r'},
            {"modify",              0, 0, 'm'},
            {"alias",               1, 0, 'A'},
            {"domain",              1, 0, 'n'},
            {"meta",                1, 0, 'M'},
            {"lib",                 1, 0, 'B'},
            {"path",                1, 0, 'P'},
            {"value",               1, 0, 'U'},
            {"size",                1, 0, 's'},
            {"data-ver",            1, 0, 'V'},
            {"periods",             1, 0, 'e'},
            {"procid",              1, 0, 'R'},
            {"config-file",         1, 0, 'C'},
            {"conf-mibkey",         1, 0, 'k'},
            {"conf-mibcount",       1, 0, 't'},
            {"conf-mibsize",        1, 0, 'z'},
            {"conf-metabasekey",    1, 0, 'K'},
            {"conf-metabasecount",  1, 0, 'T'},
            {"conf-metabasesize",   1, 0, 'Z'},
            {"log-file",            1, 0, 'g'},
            {"help",                0, 0, 'h'},
            {"version",             0, 0, 'v'},
            {0,                     0, 0, 0  }
        };

        opt = getopt_long(a_argc,
                          a_argv,
                          "C:hvl",
                          stlong_options,
                          &option_index);
        if (-1 == opt)
            break;

        if ('?' == opt || ':' == opt)
        {
            iRet = 1;
            break;
        }

#define BAD_OPTION() \
    do \
    { \
        a_pstGlData->iAction = 0; \
        iRet = -1; \
        T_ERROR_PUSH(-1, optarg); \
        printf("%s: 无效选项 %s\n" \
               "可以执行  -h 获取更多信息\n", \
               pszApp, optarg); \
    } \
    while (0)

        switch (opt)
        {
        case 'c': /* --create mib or metabase */
            {
                if (0 == strcasecmp(optarg, "mib"))
                    a_pstGlData->iAction = CREATE_MIB ;
                else if (0 == strcasecmp(optarg, "metabase"))
                    a_pstGlData->iAction = CREATE_METEBASE;
                else
                    BAD_OPTION();
                break;
            }
        case 'p': /* --dump mib or metabase */
            {
                if (0 == strcasecmp(optarg, "mib"))
                    a_pstGlData->iAction = DUMP_MIB ;
                else if (0 == strcasecmp(optarg, "metabase"))
                    a_pstGlData->iAction = DUMP_METEBASE;
                else
                    BAD_OPTION();
                break;
            }
        case 'F': /* --format { 0: txt, 1: xml, 2: tdr } */
            {
                if (0 == strcasecmp(optarg, "xml"))
                    a_pstGlData->iFormat = FORMAT_XML;
                else if (0 == strcasecmp(optarg, "tdr"))
                    a_pstGlData->iFormat = FORMAT_TDR;
                else
                    a_pstGlData->iFormat = FORMAT_TXT;
                break;
            }
        case 'S': /* --saveto path for dump */
            {
                if (optarg && optarg[0])
                    a_pstGlData->pszDumpPath = optarg;
                break;
            }
        case 'd': /* --destroy mib or metabase */
            {
                if (0 == strcasecmp(optarg, "mib"))
                    a_pstGlData->iAction = DESTORY_MIB ;
                else if (0 == strcasecmp(optarg, "metabase"))
                    a_pstGlData->iAction = DESTORY_METEBASE;
                else
                    BAD_OPTION();
                break;
            }
        case 'L': /* --loadmeta metabase */
            {
                a_pstGlData->iAction = LOAD_META;
                a_pstGlData->pszLoadfile = optarg;
                break;
            }
        case 'G': /* --loaddata cfg data into mib */
            {
                a_pstGlData->iAction = LOAD_DATA;
                a_pstGlData->pszLoadfile = optarg;
                break;
            }
        case 'f': /* --force load metabase or data force */
            {
                a_pstGlData->iForce = 1;
                break;
            }
        case 'r': /* --registdata into mib */
            {
                a_pstGlData->iAction = REGIST_DATA;
                break;
            }

        case 'm': /* --modify cfg data into mib */
            {
                a_pstGlData->iAction = MODIFY_DATA;
                break;
            }
        case 'l': /* --listproc process */
            {
                a_pstGlData->iAction = LIST_PROC;
                break;
            }

        case 'P': /* --path mib data version in load cfg data atcio n*/
            {
                a_pstGlData->pszPath = optarg;
                break;
            }
        case 'U': /* --value mib data version in load cfg data atcion */
            {
                a_pstGlData->pszValue = optarg;
                break;
            }
        case 's': /* --size mib data size in regist data atcion */
            {
                a_pstGlData->iDataSize = atoi(optarg);
                break;
            }
        case 'V': /* --data-ver mib data version in load cfg data atcion */
            {
                a_pstGlData->iMibDataVersion = atoi(optarg);
                break;
            }
        case 'e': /* --periods mib data periods in regist or modify atcion */
            {
                a_pstGlData->iPeriods = atoi(optarg);
                break;
            }

        case 'R': /* --procid mib data procid in load cfg data atcion */
            {
                a_pstGlData->iProcId = inet_addr(optarg);
                break;
            }
        case 'A': /* --alias of cfg data in load cfg data atcion */
            {
                a_pstGlData->pszAlias = optarg;
                break;
            }

        case 'n': /* --domain of mib data in loadcfg or modify  atcion */
            {
                a_pstGlData->pszDomain = optarg;
                break;
            }
        case 'M': /* --meta name of cfg data in load cfg data atcion */
            {
                a_pstGlData->pszMeta = optarg;
                break;
            }
        case 'B': /* --lib meta lib of cfg data in load cfg data atcion */
            {
                a_pstGlData->pszLib = optarg;
                break;
            }
        case 'C': /* --config-file cfg file name */
            {
                if (access(optarg, 0))
                {
                    printf("can not access %s\n", optarg);
                    iRet = 1;
                }
                else
                    STR_N_CPY(a_pstGlData->szConfFile, optarg);
                break;
            }

        case 'k': /* --conf-mibkey 命令行参数设置 MIB:Key */
            {
                STR_N_CPY(a_pstGlData->szMibKey, optarg);
                break;
            }
        case 't': /* --conf-mibcount 命令行参数设置 MIB:Count */
            {
                a_pstGlData->iMibCounts = atoi(optarg);
                break;
            }
        case 'z': /* --conf-mibsize 命令行参数设置 MIB:Size */
            {
                a_pstGlData->iMibSize = atoi(optarg);
                break;
            }
        case 'K': /* --conf-metabasekey 命令行参数设置 METABASE:Key */
            {
                STR_N_CPY(a_pstGlData->szMetabaseKey, optarg);
                break;
            }
        case 'T': /* --conf-metabasecount 命令行参数设置 METABASE:Count */
            {
                a_pstGlData->iMetabaseCounts = atoi(optarg);
                break;
            }
        case 'Z': /* --conf-metabasesize 命令行参数设置 METABASE:Size */
            {
                a_pstGlData->iMetabaseSize = atoi(optarg);
                break;
            }

        case 'h': /* --help show the help information. */
            {
                iRet = 1;
                _usage(a_argv);
                break;
            }
        case 'v': /* --version show the version information. */
            {
                iRet = 1;
                printf("%s version: %03d\n" , pszApp, CUR_VER);
                break;
            }
        default:
            {
                iRet = -1;
                printf("%s: 无效选项 \n", pszApp);
                printf("可以执行  -h 获取更多信息\n");
                break;
            }
        }

#undef BAD_OPTION
    } /* while (1) */

    if (iRet < 0)
    {
        T_ERROR_PUSH(iRet, NULL);
    }
    return iRet;
}

int32_t tmng_load_meta(GLOBALCFG *pstCfg)
{
    HTMBDESC      hDesc;
    LPTDRMETALIB  pstLib;
    int32_t       iTdrRet;
    int32_t       iRet;
    const char   *pszError;

    iRet = -TMNG_ERR_ARGS;
    pszError = "缺少参数，请指定数据文件!!\n";
    if (STR_NULL(pstCfg->pszLoadfile))
    {
        goto err_tlm;
    }

    iRet = tmb_open(&hDesc, pstCfg->szMetabaseKey, 0);
    pszError = "open metabase fail\n";
    if (iRet < 0)
    {
        goto err_tlm;
    }

    iRet = strlen(pstCfg->pszLoadfile);
    if (strncmp(pstCfg->pszLoadfile + iRet - 4, ".xml", 4))
    {
        iRet = -TMNG_ERR_TDR_LOAD_METALIB;
        iTdrRet = tdr_load_metalib(&pstLib, pstCfg->pszLoadfile);
    }
    else
    {
        iRet = -TMNG_ERR_TDR_CREATE_LIB_FILE;
        iTdrRet = tdr_create_lib_file(&pstLib, pstCfg->pszLoadfile, 1, stderr);
    }

    pszError = NULL;
    if (iTdrRet || NULL == pstLib)
    {
        goto err_tlm_close;
    }

    printf("lib name=%s size=%Zd \n",
           tdr_get_metalib_name(pstLib),
           tdr_size(pstLib));

    tdr_dump_metalib(pstLib, stdout);

    if (0 == pstCfg->iForce)
    {
        printf("\n\n"
               "Are you sure load this lib into metabase: "
               "Y(es)/N(o), default No!\n");

        iRet = -TMNG_ERR_CANCEL;
        char chInput = getchar();
        pszError = "metalib can't load into metabase, now exit!\n";
        if ('Y' != chInput && 'y' != chInput)
        {
            goto err_tlm_free;
        }
    }

    tmb_lock(hDesc);

    pszError = "load into metabase error!\n";
    iRet = tmb_append_unlocked(hDesc, pstLib);
    if (iRet < 0)
    {
        goto err_tlm_unlock;
    }

    iRet = 0;
    pszError = NULL;

err_tlm_unlock:
    tmb_unlock(hDesc);

err_tlm_free:
    tdr_free_lib(&pstLib);

err_tlm_close:
    tmb_close(&hDesc);

err_tlm:
    if (pszError)
        printf(pszError);
    if (iRet < 0)
    {
        T_ERROR_PUSH(iRet, pszError);
    }
    return iRet;
}

int32_t tmng_load_data(GLOBALCFG *pstCfg)
{
    HTMIB       hMib;
    HTMBDESC    hDesc;
    LPTDRMETA   pstMeta;
    TDRDATA     stData;
    TMIBDATA    stMibData;
    const char *pszError;
    int32_t     iRet;

    iRet = -TMNG_ERR_ARGS;
    pszError = "缺少参数，请使用--alias指定cfg data的名字!!\n";
    if (STR_NULL(pstCfg->pszAlias))
    {
        goto err_tld;
    }

    pszError = "缺少参数，请使用--meta指定数据定义的名字!!\n";
    if (STR_NULL(pstCfg->pszMeta))
    {
        goto err_tld;
    }

    pszError = "缺少参数，请使用--lib 指定数据定义的lib!!\n";
    if (STR_NULL(pstCfg->pszLib))
    {
        goto err_tld;
    }

    iRet = tmib_open(&hMib, pstCfg->szMibKey);
    pszError = "open mib fail\n";
    if (iRet < 0)
    {
        goto err_tld;
    }

    iRet = tmb_open(&hDesc, pstCfg->szMetabaseKey, 1);
    pszError = "open metabase fail\n";
    if (iRet < 0)
    {
        goto err_tld_close;
    }

    iRet = tmb_open_metalib(hDesc, pstCfg->pszLib, 0);
    pszError = NULL;
    if (iRet < 0)
    {
        printf("cfg data=%s, lib=%s not found! \n",
               pstCfg->pszLoadfile, pstCfg->pszLib);
        goto err_tld_close2;
    }

    iRet = tmb_meta_by_name(hDesc, pstCfg->pszMeta, &pstMeta);
    if (NULL == pstMeta)
    {
        printf("data %s meta=%s define not found!\n",
               pstCfg->pszLoadfile,
               pstCfg->pszMeta);
        goto err_tld_close2;
    }

    stData.iBuff = tdr_get_meta_size(pstMeta);
    stData.pszBuff = (char *)malloc(stData.iBuff);
    if (tdr_input_file(pstMeta, &stData, pstCfg->pszLoadfile, 0, 0))
    {
        iRet = -TMNG_ERR_TDR_INPUT_FILE;
        printf("load data %s fail!\n", pstCfg->pszLoadfile);
        goto err_tld_free;
    }

    memset(&stMibData, 0, sizeof(stMibData));
    STR_N_CPY(stMibData.szDomain,
            STR_NULL(pstCfg->pszDomain) ? CFG_DOMAIN : pstCfg->pszDomain);
    STR_N_CPY(stMibData.szName, pstCfg->pszAlias);
    STR_N_CPY(stMibData.szLib, pstCfg->pszLib);
    STR_N_CPY(stMibData.szMeta, pstCfg->pszMeta);
    stMibData.iVersion = pstCfg->iMibDataVersion;
    stMibData.iPeriods = pstCfg->iPeriods;
    stMibData.iProcID  = pstCfg->iProcId ;
    stMibData.iSize    = stData.iBuff;

    tdr_fprintf(pstMeta, stdout, &stData, 0);

    if (0 == pstCfg->iForce)
    {
        printf("\n\n"
               "Are you sure load this data into mib: "
               "Y(es)/N(o), default No!\n");

        iRet = -TMNG_ERR_CANCEL;
        char chInput = getchar();
        if ('Y' != chInput && 'y' != chInput)
        {
            printf("config data %s can't laod into mib !\n",
                   pstCfg->pszLoadfile);
            goto err_tld_free;
        }
    }

    if (tmib_register_data(hMib, &stMibData))
    {
        printf("regist mib %s fail!\n", pstCfg->pszLoadfile);
        goto err_tld_free;
    }

    tmib_get_data(hMib, &stMibData, 0);
    memcpy(stMibData.pszData, stData.pszBuff, stData.iBuff);

    iRet = 0;

err_tld_free:
    free(stData.pszBuff);
    stData.pszBuff = NULL;

err_tld_close2:
    tmb_close(&hDesc);

err_tld_close:
    tmib_close(&hMib);

err_tld:
    if (pszError)
        printf(pszError);
    if (iRet < 0)
    {
        T_ERROR_PUSH(iRet, pszError);
    }
    return iRet;
}

int32_t tmng_regist_data(GLOBALCFG *pstCfg)
{
    HTMIB       hMib;
    HTMBDESC    hDesc;
    LPTDRMETA   pstMeta;
    TMIBDATA    stMibData;
    int32_t     iSize;
    const char *pszError;
    int32_t     iRet;

    iRet = -TMNG_ERR_ARGS;
    pszError = "缺少参数，请使用--alias指定cfg data的名字!!\n";
    if (STR_NULL(pstCfg->pszAlias))
    {
        goto err_trd;
    }

    if (STR_NOT_NULL(pstCfg->pszMeta))
    {
        pszError = "缺少参数，请使用--lib 指定数据定义的lib!!\n";
        if (STR_NULL(pstCfg->pszLib))
        {
            goto err_trd;
        }
    }
    else
    {
        pszError = "缺少参数，请使用--meta --lib指定数据定义 "
                   "或者--size　指定数据空间大小!\n";
        if (!pstCfg->iDataSize)
        {
            goto err_trd;
        }
    }

    iRet = tmib_open(&hMib, pstCfg->szMibKey);
    pszError = "open mib fail\n";
    if (iRet < 0)
    {
        goto err_trd;
    }

    iRet = tmb_open(&hDesc, pstCfg->szMetabaseKey, 1);
    pszError = "open metabase fail\n";
    if (iRet < 0)
    {
        goto err_trd_close;
    }

    memset(&stMibData, 0, sizeof(stMibData));
    iSize = pstCfg->iDataSize;

    pszError = NULL;
    if (STR_NOT_NULL(pstCfg->pszMeta))
    {
        iRet = tmb_open_metalib(hDesc, pstCfg->pszLib, 0);
        if (iRet < 0)
        {
            printf("cfg data=%s, lib=%s not found! \n",
                   pstCfg->pszLoadfile,
                   pstCfg->pszLib);
            goto err_trd_close2;
        }

        iRet = tmb_meta_by_name(hDesc, pstCfg->pszMeta, &pstMeta);
        if (NULL == pstMeta)
        {
            printf("data %s meta=%s define not found!\n",
                   pstCfg->pszLoadfile,
                   pstCfg->pszMeta);
            goto err_trd_close2;
        }
        iSize = tdr_get_meta_size(pstMeta);
    }

    STR_N_CPY(stMibData.szDomain,
            STR_NULL(pstCfg->pszDomain) ? CFG_DOMAIN : pstCfg->pszDomain);
    STR_N_CPY(stMibData.szName, pstCfg->pszAlias);
    if (pstCfg->pszLib)
        STR_N_CPY(stMibData.szLib, pstCfg->pszLib);
    if (pstCfg->pszMeta)
        STR_N_CPY(stMibData.szMeta, pstCfg->pszMeta);
    stMibData.iVersion = pstCfg->iMibDataVersion;
    stMibData.iProcID  = pstCfg->iProcId ;
    stMibData.iPeriods = pstCfg->iPeriods;
    stMibData.iSize    = iSize;

    iRet = tmib_register_data(hMib, &stMibData);
    if (iRet < 0)
    {
        printf("regist mib %s fail!\n", pstCfg->pszLoadfile);
        goto err_trd_close2;
    }

    if (iSize != stMibData.iSize)
    {
        printf("mib data %s already exist size=%d\n"
               "argument --size specify value=%d, can't match!\n",
               pstCfg->pszAlias,
               stMibData.iSize,
               pstCfg->iDataSize);
    }

    iRet = 0;

err_trd_close2:
    tmb_close(&hDesc);

err_trd_close:
    tmib_close(&hMib);

err_trd:
    if (pszError)
        printf(pszError);
    if (iRet < 0)
    {
        T_ERROR_PUSH(iRet, pszError);
    }
    return iRet;
}

int32_t tmng_modify_data(GLOBALCFG *pstCfg)
{
    HTMIB           hMib;
    HTMBDESC        hDesc;
    LPTDRMETA       pstMeta;
    TDRDATA         stData;
    TMIBDATA        stMibData;
    LPTDRMETAENTRY  pstEntry;
    const char     *pszError;
    int32_t         iRet;
    TDROFF          iOff;
    int32_t         iType;

    iRet = -TMNG_ERR_ARGS;
    pszError = "缺少参数，请使用--alias指定cfg data的名字!!\n";
    if (STR_NULL(pstCfg->pszAlias))
    {
        goto err_tmd;
    }

    pszError = "缺少参数，请使用--path 指定元数据路径!!\n";
    if (STR_NULL(pstCfg->pszPath))
    {
        goto err_tmd;
    }

    pszError = "缺少参数，请使用--value 指定数据修改的值!!\n";
    if (STR_NULL(pstCfg->pszValue))
    {
        goto err_tmd;
    }

    iRet = tmib_open(&hMib, pstCfg->szMibKey);
    pszError = "open mib fail\n";
    if (iRet < 0)
    {
        goto err_tmd;
    }

    iRet = tmb_open(&hDesc, pstCfg->szMetabaseKey, 1);
    pszError = "open metabase fail\n";
    if (iRet < 0)
    {
        goto err_tmd_close;
    }

    memset(&stMibData, 0, sizeof(stMibData));
    STR_N_CPY(stMibData.szDomain,
            STR_NULL(pstCfg->pszDomain) ? CFG_DOMAIN : pstCfg->pszDomain);
    STR_N_CPY(stMibData.szName, pstCfg->pszAlias);
    stMibData.iVersion = pstCfg->iMibDataVersion;
    stMibData.iPeriods = pstCfg->iPeriods;
    stMibData.iProcID  = pstCfg->iProcId;

    iRet = tmib_get_data(hMib, &stMibData, 0);
    pszError = NULL;
    if (iRet < 0)
    {
        goto err_tmd_close2;
    }

    iRet = tmb_open_metalib(hDesc, stMibData.szLib, stMibData.iVersion);
    if (iRet < 0)
    {
        printf("cfg data=%s, lib=%s not found! \n",
               pstCfg->pszAlias,
               stMibData.szLib);
        goto err_tmd_close2;
    }

    iRet = tmb_meta_by_name(hDesc, stMibData.szMeta, &pstMeta);
    if (NULL == pstMeta)
    {
        printf("data %s meta=%s define not found!\n",
               pstCfg->pszAlias,
               stMibData.szMeta);
        goto err_tmd_close2;
    }

    if (tdr_entry_path_to_off(pstMeta, &pstEntry, &iOff, pstCfg->pszPath))
    {
        iRet = -TMNG_ERR_TDR_ENTRY_PATH_TO_OFF;
        printf("data %s  meta path=%s define not found!\n",
               pstCfg->pszAlias,
               pstCfg->pszAlias);
        goto err_tmd_close2;
    }

    iType = tdr_get_entry_type(pstEntry);
    if (TDR_TYPE_UNION == iType || TDR_TYPE_STRUCT == iType)
    {
        iRet = -TMNG_ERR_TDR_DATA_TYPE;
        printf("data %s  meta path=%s 复合数据类型不支持修改!\n",
               pstCfg->pszAlias,
               pstCfg->pszAlias);
        goto err_tmd_close2;
    }

    stData.iBuff   = stMibData.iSize;
    stData.pszBuff = stMibData.pszData;

    tdr_fprintf(pstMeta, stdout, &stData, 0);

    if (0 == pstCfg->iForce)
    {
        printf("\n\n"
               "Are you sure modyfy %s with values %s: "
               "Y(es)/N(o), default No!\n",
               pstCfg->pszPath,
               pstCfg->pszValue);

        iRet = -TMNG_ERR_CANCEL;
        char chInput = getchar();
        if ('Y' != chInput && 'y' != chInput)
        {
            printf("config data %s can't modify, now exit!\n",
                   pstCfg->pszPath);
            goto err_tmd_close2;
        }
    }

    iRet = set_entry_value(pstEntry,
                           pstCfg->pszValue,
                           stMibData.pszData + iOff,
                           tdr_get_entry_unitsize(pstEntry),
                           stdout);

    if (iRet < 0)
    {
        iRet = -TMNG_ERR_SET_ENTRY_VALUE;
    }

err_tmd_close2:
    tmb_close(&hDesc);

err_tmd_close:
    tmib_close(&hMib);

err_tmd:
    if (pszError)
        printf(pszError);
    if (iRet < 0)
    {
        T_ERROR_PUSH(iRet, pszError);
    }
    return iRet;
}

int32_t tmng_dump_mib(GLOBALCFG *pstCfg)
{
    HTMIB       hMib;
    HTMBDESC    hDesc;
    TMIBDATA    stMibData;
    const char *pszError;
    int32_t     iRet;

    iRet = tmib_open(&hMib, pstCfg->szMibKey);
    pszError = "open mib fail\n";
    if (iRet < 0)
    {
        goto err_tdm;
    }

    iRet = tmb_open(&hDesc, pstCfg->szMetabaseKey, 1);
    pszError = "open metabase fail\n";
    if (iRet < 0)
    {
        goto err_tdm_close;
    }

    if (STR_NOT_NULL(pstCfg->pszAlias))
    {
        memset(&stMibData, 0, sizeof(stMibData));
        STR_N_CPY(stMibData.szDomain,
                STR_NULL(pstCfg->pszDomain) ? CFG_DOMAIN : pstCfg->pszDomain);
        STR_N_CPY(stMibData.szName, pstCfg->pszAlias);
        stMibData.iVersion = pstCfg->iMibDataVersion;
        stMibData.iProcID  = pstCfg->iProcId;

        if (0 == tmib_get_data(hMib, &stMibData, 0))
        {
            printf(" Domain=\"%s\" Name=\"%s\" Lib=\"%s\","
                   " Meta=\"%s\", ProcID=%s size=%d\n",
                   stMibData.szDomain,
                   stMibData.szName,
                   stMibData.szLib,
                   stMibData.szMeta,
                   inet_ntoa(*((struct in_addr *)&(stMibData.iProcID))),
                   stMibData.iSize);
            tmib_dump(hDesc, &stMibData, stdout);
        }
    }
    else
    {
        tmib_dump_head(hMib, stdout);
        tmib_dump_all(hMib, pstCfg->szMetabaseKey, stdout);
        printf("\n");
    }

    pszError = NULL;
    iRet = 0;

    tmb_close(&hDesc);

err_tdm_close:
    tmib_close(&hMib);

err_tdm:
    if (pszError)
        printf(pszError);
    if (iRet < 0)
    {
        T_ERROR_PUSH(iRet, pszError);
    }
    return iRet;
}

int32_t tmng_list_proc(GLOBALCFG *pstCfg)
{
    HTMIB          hMib;
    HTMBDESC       hDesc;
    TMIBINFO      *pstInfo;
    TMIBENTRY     *pstEntry;
    LPMNGPROCATTR  pstProcAttr;
    char           szStart[30];
    char           szStop[30];
    int32_t        i;
    const char    *pszError;
    int32_t        iRet;

    iRet = tmib_open(&hMib, pstCfg->szMibKey);
    pszError = "open mib fail\n";
    if (iRet < 0)
    {
        goto err_tlp;
    }

    iRet = tmb_open(&hDesc, pstCfg->szMetabaseKey, 1);
    pszError = "open metabase fail\n";
    if (iRet < 0)
    {
        goto err_tlp_close;
    }

    pstInfo = hMib->pstInfo;
    printf("-------------------------------------------"
           "------------------------------------------\n"
           "| ProcessID     | ProcName    | pid   | Status "
           "| User |     start    |     stop     |\n"
           "-------------------------------------------"
           "------------------------------------------\n");

    for (i = 0; i < pstInfo->iCurDatas; ++i)
    {
        pstEntry = pstInfo->entries + i;

        if (pstEntry->bDelete
                || pstEntry->bExtern
                || strcmp(pstEntry->szDomain, PROC_LIST_DOMAIN)
                || strcmp(pstEntry->szMeta, PROC_ATTR))
            continue;

        pstProcAttr = (LPMNGPROCATTR)((size_t)pstInfo
                     + pstEntry->iOff
                     + MIB_HEAD_SIZE);
        FormatTime(pstProcAttr->dwStart, szStart, sizeof(szStart));
        FormatTime(pstProcAttr->dwStop,  szStop,  sizeof(szStop));
        printf("  %-15s %-12s\t%-5d   %-6s  %-6s %-15s %-15s\n",
               pstProcAttr->szProcID,
               pstProcAttr->szProcName,
               pstProcAttr->iPid,
               pstProcAttr->iStatus ? "Active" : "",
               pstProcAttr->iStatus ? pstProcAttr->szUser : "",
               pstProcAttr->iStatus ? szStart : "",
               pstProcAttr->iStatus ? "" : pstProcAttr->dwStop ? szStop : "");
    }
    printf( "\n\n");

    iRet = 0;

    tmb_close(&hDesc);

err_tlp_close:
    tmib_close(&hMib);

err_tlp:
    if (pszError)
        printf(pszError);
    if (iRet < 0)
    {
        T_ERROR_PUSH(iRet, pszError);
    }
    return iRet;
}

int main(int argc, char **argv)
{
    GLOBALCFG   stCfg;
    HTMIB       hMib;
    HTMBDESC    hDesc;
    int32_t     iRet;
    const char *pszError;

    T_ERROR_CLEAR();

#define ERROR_EXIT(R, S) do { \
        T_ERROR_DUMP(); \
        if ((R) < 0) { \
            if ((S) && *(S)) { \
                printf("ERROR: %s\n", (S)); \
            } \
            printf("E: (%d) (%s)\n", (R), tmng_errstr((R))); \
        } \
        return (R); \
    } while (0)

    memset(&stCfg, 0, sizeof(GLOBALCFG));

    iRet = parse_option(&stCfg, argc, argv);
    pszError = "参数检查没有通过";
    if (iRet)
    {
        ERROR_EXIT(iRet, pszError);
    }

    if ('\0' == stCfg.szConfFile[0])
    {
        snprintf(stCfg.szConfFile,
                 sizeof(stCfg.szConfFile),
                 "./%s.conf",
                 basename(argv[0]));
    }

    iRet = get_mng_cfg(&stCfg, stCfg.szConfFile);
    pszError = "get config error";
    if (iRet < 0)
    {
        ERROR_EXIT(iRet, pszError);
    }

    pszError = NULL;
    switch (stCfg.iAction)
    {
    case CREATE_MIB:
        {
            pszError = "Create mib fail!";
            iRet = tmib_create(&hMib,
                               stCfg.szMibKey,
                               stCfg.iMibCounts,
                               stCfg.iMibSize);
            if (0 == iRet)
            {
                printf("Create mib sucess!\n");
            }
            break;
        }
    case CREATE_METEBASE:
        {
            pszError = "Create metabase fail!";
            iRet = tmb_create(&hDesc,
                              stCfg.szMetabaseKey,
                              stCfg.iMetabaseCounts,
                              stCfg.iMetabaseSize);
            if (0 == iRet)
            {
                printf("Create metabase sucess!\n");
            }
            break;
        }
    case DESTORY_MIB:
        {
            pszError = "Destory mib fail!\n";
            iRet = tmib_destroy(stCfg.szMibKey);
            if (0 == iRet)
            {
                printf("Destory mib sucess!\n");
            }
            break;
        }
    case DESTORY_METEBASE:
        {
            pszError = "Destory metabase fail!";
            iRet = tmb_destroy(stCfg.szMetabaseKey);
            if (0 == iRet)
            {
                printf("Destory metabase sucess!\n");
            }
            break;
        }
    case DUMP_MIB:
        {
            iRet = tmng_dump_mib(&stCfg);
            break;
        }
    case DUMP_METEBASE:
        {
            pszError = "open metabase fail!";
            iRet = tmb_open(&hDesc, stCfg.szMetabaseKey, 1);
            if (0 == iRet)
            {
                pszError = "close metabase fail!";
                tmb_dump_files(hDesc,
                               1,
                               FORMAT_XML == stCfg.iFormat
                                   ? "xml" : FORMAT_TDR == stCfg.iFormat
                                       ? "tdr" : "txt",
                               stCfg.pszDumpPath);
                iRet = tmb_close(&hDesc);
            }
            break;
        }
    case LOAD_META:
        {
            iRet = tmng_load_meta(&stCfg);
            if (iRet)
            {
                printf("load %s into metabase error %d\n",
                       stCfg.pszLoadfile,
                       iRet);
            }
            else
            {
                printf("load metalib %s  sucess!\n",
                       stCfg.pszLoadfile);
            }
            break;
        }
    case LOAD_DATA:
        {
            iRet = tmng_load_data(&stCfg);
            if (iRet)
                printf("load %s into mib error %d\n", stCfg.pszLoadfile, iRet);
            else
                printf("load mib data %s  sucess!\n", stCfg.pszLoadfile);
            break;
        }
    case REGIST_DATA:
        {
            iRet = tmng_regist_data(&stCfg);
            if (iRet)
                printf("regist %s in mib error %d\n", stCfg.pszAlias, iRet);
            else
                printf("regist %s  in mib sucess!\n", stCfg.pszAlias);
            break;
        }
    case MODIFY_DATA:
        {
            iRet = tmng_modify_data(&stCfg);
            if (iRet)
                printf("config data %s can't modify!\n", stCfg.pszPath);
            else
                printf("config data %s t modify  success!\n", stCfg.pszPath);
            break;
        }
    case LIST_PROC:
        {
            pszError = "list process!";
            iRet = tmng_list_proc(&stCfg);
            break;
        }
    case 0:
    default:
        {
            pszError = "可以执行  -h 获取更多信息";
            iRet = -TMNG_ERR_HELP;
            break;
        }
    }

    ERROR_EXIT(iRet, pszError);
    return 0;

#undef ERROR_EXIT
}

#undef STR_N_CPY
#undef STR_NULL
#undef STR_NOT_NULL

