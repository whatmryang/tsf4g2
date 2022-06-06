//============================================================================
// @Id:       $Id: tsm_tool.c 14985 2010-11-18 03:42:30Z tomaswang $
// @Author:   $Author: tomaswang $
// @Date:     $Date:: 2010-11-18 11:42:30 +0800 #$
// @Revision: $Revision: 14985 $
// @HeadURL:  $HeadURL:: https://tc-svn.tencent.com/ied/ied_tsf4g_rep/tsf4g_#$
//----------------------------------------------------------------------------


#include "version.h"

#include "pal/pal.h"
#include "tdr/tdr.h"

/*
#include "tdr/tdr_data_io.h"
#include "tdr/tdr_XMLMetaLib.h"

*/
#include "err_stack.h"
#include "tsm_head.h"
#include "tsm_index.h"
#include "tsm_tdr.h"
#include "tsm_shm.h"

#include "tmng/tmng.h"

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
static char tsm_tool_version[] = "tsm: version %d.%d.%d  build at %d.\n";

typedef struct
{
    const char *cmd;            // command
    const char *meta_uri;       // meta uri
    const char *uri;            // data uri
    int         bid;            // business id
    int         index_key;      // index key
    int         lib_size;       // entries count at lib area
    int         uri_size;       // entries count at uri area
    const char *format;         // dump data format
    const char *save_to;        // dump data save to path
    const char *file;           // load data from file
    const char *value;          // modify data to set value
    int         help;           // is help
    int         version;        // is version
} TsmTdrToolConf;

static const char *s_options = "m:d:i:l:u:x:s:f:e:hv";
static struct option l_options[] = {
    { "bid",       1, 0, 'b' },
    { "meta-uri",  1, 0, 'm' },
    { "uri",       1, 0, 'd' },
    { "index-key", 1, 0, 'i' },
    { "lib-size",  1, 0, 'l' },
    { "uri-size",  1, 0, 'u' },
    { "format",    1, 0, 'x' },
    { "save-to",   1, 0, 's' },
    { "file",      1, 0, 'f' },
    { "value",     1, 0, 'e' },
    { "help",      0, 0, 'h' },
    { "version",   0, 0, 'v' },
    { 0,           0, 0,  0  }
};
static const char *options_help[] = {
    "businessID,unique identifier a business",
    "meta uri,  a meta lib resource",
    "data uri,  a application data resource",
    "index key, shm key for index, default: 410583828",
    "lib size,  max capcity for meta",
    "uri size,  max capcity for data",
    "format,    metalib dump format [txt|tdr|xml]",
    "save to,   metalib dump save path",
    "file,      load meta or load data",
    "value,     for modify data",
    "help,      print help infomation",
    "versoin,   print version infomation",
    NULL
};

int parse_option(TsmTdrToolConf *c, int argc, char **argv);
int parse_option(TsmTdrToolConf *c, int argc, char **argv)
{
    int i;
    int f;

    memset(c, 0, sizeof(TsmTdrToolConf));

    while (-1 != (f = getopt_long(argc, argv, s_options, l_options, &i)))
    {
#define SET_STR_OPT(F, N, V) case F: c->N = V; break
#define SET_INT_OPT(F, N, V) case F: c->N = atoi(V); break
        switch (f)
        {
            SET_INT_OPT('b', bid,       optarg);
            SET_STR_OPT('m', meta_uri,  optarg);
            SET_STR_OPT('d', uri,       optarg);
            SET_INT_OPT('i', index_key, optarg);
            SET_INT_OPT('l', lib_size,  optarg);
            SET_INT_OPT('u', uri_size,  optarg);
            SET_STR_OPT('x', format,    optarg);
            SET_STR_OPT('s', save_to,   optarg);
            SET_STR_OPT('f', file,      optarg);
            SET_STR_OPT('e', value,     optarg);
            SET_INT_OPT('h', help,      "1");
            SET_INT_OPT('v', version,   "1");
        default:
            return -1;
            break;
        }
#undef SET_INT_OPT
#undef SET_STR_OPT
    }

    if (1 + optind == argc)
    {
        c->cmd = argv[optind];
        return 0;
    }

    return optind == argc ? 0 : -2;
}

typedef int (*pfnCmd)(TsmTdrToolConf *, TsmTdrHandleP);
typedef struct
{
    const char *name;
    pfnCmd      func;
} CmdEntry;


//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int cmd_version(TsmTdrToolConf *conf)
{
    printf(tsm_tool_version, MAJOR, MINOR, REV, BUILD);

    return 0;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int cmd_create(TsmTdrToolConf *conf, TsmTdrHandleP handle)
{
    if (conf->help)
    {
        printf("Command: create\n"
               "    --index-key=<Integer>  (option)   key for index shm\n"
               "    --lib-size=<Integer>   (option)   lib entries max cap\n"
               "    --uri-size=<Integer>   (option)   uri entries max cap\n");
        return 0;
    }

    return 0;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int cmd_destroy(TsmTdrToolConf *conf, TsmTdrHandleP handle)
{
    TsmTdrResultP  res;
    TsmTdrRowP     row;
    const char    *uri;
    int            iRet;

    uri = conf->uri ? conf->uri : conf->meta_uri;

    if (conf->help || NULL == uri)
    {
        printf("Command: destroy\n"
               "    --index-key=<Integer> (option)   key for index shm\n"
               "    --uri=<String>        (option)   delete some data\n"
               "    --meta-uri=<String>   (option)   delete some meta\n");
        return 0;
    }

    iRet = tsm_tdr_query(handle, uri);
    if (iRet > 0)
    {
        fprintf(stderr, "found (%d) entries, delete...\n", iRet);

        res = tsm_tdr_store_result(handle);
        tassert(res, NULL, 3, "tsm_tdr_store_result 失败");

        while ((row = tsm_tdr_fetch_row(res)))
        {
            tsm_tdr_row_unlink(row, handle);
        }

        tsm_tdr_free_result(res);
    }

    return 0;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int cmd_dump(TsmTdrToolConf *conf, TsmTdrHandleP handle)
{
    TsmTdrResultP pRes;
    TsmTdrRowP    pRow;
    int           iFormat;
    int           iRet;

    if (conf->help)
    {
        printf("Command: dump\n"
               "    --index-key=<Integer>  (option)   key for index shm\n"
               "    --format=<tdr|txt|xml> (option)   file format for dump\n"
               "    --save-to=<String>     (option)   dump data save path\n"
               "    --meta-uri=<String>    (required) point to meta\n");
        return 0;
    }

    if (NULL == conf->meta_uri)
    {
        fprintf(stderr, "    --meta-uri is required.\n");
        return terror_push(NULL, 1, "没有找到 --meta-uri 选项");
    }

    iRet = tsm_tdr_query(handle, conf->meta_uri);
    tassert1(iRet >= 0, NULL, 2, "tsm_tdr_query 失败 (%d)", iRet);

    fprintf(stderr, "found (%d) entries\n", iRet);

    if (0 == iRet)
    {
        return 0;
    }

    if (NULL == conf->format)
    {
        conf->format = "tdr";
    }
    if (0 == memcmp("tdr", conf->format, 4))
    {
        iFormat = 0;
    }
    else if (0 == memcmp("txt", conf->format, 4))
    {
        iFormat = 1;
    }
    else if (0 == memcmp("xml", conf->format, 4))
    {
        iFormat = 2;
    }
    else
    {
        return terror_push1(NULL, 3, "不认识的格式 (%s)", conf->format);
    }

    if (NULL == conf->save_to)
    {
        conf->save_to = ".";
    }

    pRes = tsm_tdr_store_result(handle);
    tassert(pRes, NULL, 4, "tsm_tdr_store_result 失败");

    while ((pRow = tsm_tdr_fetch_row(pRes)))
    {
        const char   *pszUri = tsm_tdr_row_uri(pRow);
        char          szFile[256];
        TsmUri        stUri;
        LPTDRMETALIB  pData;
        size_t        iSize;
        FILE         *fp;

        if (!tsm_tdr_row_is_meta(pRow))
        {
            terror_push1(NULL, 5, "uri is not meta(%s)", pszUri);
            continue;
        }

        if (tsm_uri_set(&stUri, pszUri))
        {
            terror_push1(NULL, 6, "tsm_uri_set 失败(%s)", pszUri);
            continue;
        }

        snprintf(szFile,
                 255,
                 "%s/%s_%d_%d.%s",
                 conf->save_to,
                 TSM_URI_GET(&stUri, m, szLib),
                 TSM_URI_GET(&stUri, m, iLibVer),
                 TSM_URI_GET(&stUri, m, iLibBuildVer),
                 conf->format);

        pData = (LPTDRMETALIB)tsm_tdr_row_data(pRow, 1);
        iSize = tsm_tdr_row_size(pRow);

        if (NULL == pData || 0 == iSize)
        {
            terror_push1(NULL, 6, "bad meta(%s)", pszUri);
            continue;
        }

        fp = fopen(szFile, "w+");
        if (NULL == fp)
        {
            terror_push1(NULL, 7, "fopen 失败(%s)", szFile);
            break;
        }

        switch (iFormat)
        {
        case 0:
            tdr_save_metalib_fp(pData, fp);
            break;
        case 1:
            tdr_dump_metalib(pData, fp);
            break;
        default:
            tdr_save_xml_fp(pData, fp);
            break;
        }

        fclose(fp);
    }

    tsm_tdr_free_result(pRes);

    return 0;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int cmd_loadmeta(TsmTdrToolConf *conf, TsmTdrHandleP handle)
{
    if (conf->help)
    {
        printf("Command: loadmeta\n"
               "    --index-key<Integer>  (option)   key for index shm\n"
               "    --file=<String>       (required) tdr file path\n"
               "    --bid=<Integer>       (required) business id\n");
        return 0;
    }

    if (NULL == conf->file)
    {
        fprintf(stderr, "    --file is required.\n");
        return terror_push(NULL, 1, "没有找到 --file 选项");
    }

    /*
    if (0 == conf->bid)
    {
        fprintf(stderr, "    --bid is required.\n");
        return terror_push(NULL, 2, "没有找到 --bid 选项");
    }
    */

    return tsm_tdr_load_meta_file(handle, conf->file, conf->bid);
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int cmd_loaddata(TsmTdrToolConf *conf, TsmTdrHandleP handle)
{
    struct stat  stStat;
    char        *pData;
    int          iFd;
    int          iRet;
    int          i;

    if (conf->help)
    {
        printf("Command: loadmeta\n"
               "    --index-key<Integer>  (option)   key for index shm\n"
               "    --file=<String>       (required) data file path\n"
               "    --uri=<String>        (required) point to data\n"
               "    --meta-uri=<String>   (option)   point to meta\n");
        return 0;
    }

    if (NULL == conf->file)
    {
        fprintf(stderr, "    --file is required.\n");
        return terror_push(NULL, 1, "没有找到 --file 选项");
    }

    if (NULL == conf->uri)
    {
        fprintf(stderr, "    --uri is required.\n");
        return terror_push(NULL, 2, "没有找到 --uri 选项");
    }

    iRet = tsm_tdr_bind_data(handle, conf->uri);
    tassert1(0 == iRet, NULL, 3, "tsm_tdr_bind_data 失败(%d)", iRet);

    iFd = open(conf->file, O_RDONLY);
    tassert2(iFd >= 0, NULL, 4, "open(%s) 失败(%d)", conf->file, iFd);

    if (conf->meta_uri)
    {
        iRet = tsm_tdr_bind_meta(handle, conf->meta_uri);
        tassert1(0 == iRet, NULL, 5, "tsm_tdr_bind_meta 失败(%d)", iRet);
    }

    bzero(&stStat, sizeof(struct stat));
    iRet = fstat(iFd, &stStat);
    tassert2(-1 != iRet, NULL, 6, "fstat(%s) 失败 (%d)", conf->file, iRet);

    iRet = tsm_tdr_bind_size(handle, stStat.st_size);
    tassert1(0 == iRet, NULL, 7, "tsm_tdr_bind_size 失败 (%d)", iRet);

    pData = malloc(stStat.st_size);
    tassert1(pData, NULL, 8, "malloc 失败 (%d)", stStat.st_size);

    for (i = 0; i < stStat.st_size; i += iRet)
    {
        iRet = read(iFd, pData + i, stStat.st_size - i);
        if (-1 == iRet)
        {
            if (EINTR != errno && EAGAIN != errno)
            {
                free(pData);
                return terror_push1(NULL, 9, "read 失败 (%d)", errno);
            }
            iRet = 0;
        }
    }

    iRet = tsm_tdr_load_data(handle, pData, O_CREAT | O_EXCL);
    free(pData);

    if (iRet > 0)
    {
        printf("data in memory already...\n");
    }
    tassert1(iRet >= 0, NULL, 10, "tsm_tdr_load_data ret(%d)", iRet);

    return 0;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int cmd_list(TsmTdrToolConf *conf, TsmTdrHandleP handle)
{
    TsmTdrResultP  res;
    TsmTdrRowP     row;
    TsmObjPtr     *ptr;
    int            iRet;

    if (conf->help)
    {
        printf("Command: list\n"
               "    --index-key<Integer>  (option)   key for index shm\n"
               "    --uri=<String>        (required) point to data\n");
        return 0;
    }

    if (NULL == conf->uri)
    {
        fprintf(stderr, "    --uri is required.\n");
        return terror_push(NULL, 1, "没有找到 --uri 选项");
    }

    iRet = tsm_tdr_query(handle, conf->uri);
    tassert1(iRet >= 0, NULL, 2, "查询出错 (%d)", iRet);

    if (0 == iRet)
    {
        fprintf(stderr, "list: record not found...\n");
        return 0;
    }

    fprintf(stderr, "found (%d) records...\n", iRet);

    res = tsm_tdr_store_result(handle);
    tassert(res, NULL, 1, "tsm_tdr_store_result 失败");

    while ((row = tsm_tdr_fetch_row(res)))
    {
        printf("==> uri:  %s\n", tsm_tdr_row_uri(row));
        if (tsm_tdr_row_is_data(row))
        {
            const char *meta = tsm_tdr_row_meta_uri(row);
            if (meta && meta[0])
            {
                printf("    meta: %s\n", meta);
            }
        }
        ptr = tsm_tdr_row_data_ptr(row);
        printf("    ptr:  shm(%d:%zd)\n", ptr->iShmId, ptr->iOffset);
        printf("    size: %zd\n", tsm_tdr_row_size(row));
    }

    tsm_tdr_free_result(res);

    return 0;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int cmd_print(TsmTdrToolConf *conf, TsmTdrHandleP handle)
{
    TsmTdrRowP row;
    LPTDRMETA  pstMeta;
    TDRDATA    stHost;

    if (conf->help)
    {
        printf("Command: print\n"
               "    --index-key<Integer>  (option)   key for index shm\n"
               "    --uri=<String>        (required) point to data\n");
        return 0;
    }

    if (NULL == conf->uri)
    {
        fprintf(stderr, "    --uri is required.\n");
        return terror_push(NULL, 1, "没有找到 --uri 选项");
    }

    tassert(tsm_tdr_query(handle, conf->uri) >= 0,
            NULL, 2, "查询出错");

    row = tsm_tdr_row_only(handle);
    if (NULL == row)
    {
        fprintf(stderr, "print: record not found...\n");
        return 0;
    }

    pstMeta = tsm_tdr_row_meta(row);
    if (NULL == pstMeta)
    {
        fprintf(stderr, "get meta failed...\n");
        return -3;
    }

    stHost.pszBuff = tsm_tdr_row_data(row, 1);
    stHost.iBuff   = tsm_tdr_row_size(row);

    return tdr_fprintf(pstMeta, stdout, &stHost, 0);
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int cmd_modify(TsmTdrToolConf *conf, TsmTdrHandleP handle)
{
    // --alias, --path, --value
    /*
    LPTDRMETA       pstMeta;
    TDRDATA         stData;
    TMIBDATA        stMibData;
    LPTDRMETAENTRY  pstEntry;
    TDROFF          iOff;
    int32_t         iType;

    tmib_get_data(hMib, &stMibData, 0);
    tmb_open_metalib(hDesc, stMibData.szLib, stMibData.iVersion);
    tmb_meta_by_name(hDesc, stMibData.szMeta, &pstMeta);

    if (tdr_entry_path_to_off(pstMeta, &pstEntry, &iOff, pstCfg->pszPath))
    {
        goto err_tmd_close2;
    }

    iType = tdr_get_entry_type(pstEntry);
    if (TDR_TYPE_UNION == iType || TDR_TYPE_STRUCT == iType)
    {
        goto err_tmd_close2;
    }

    stData.iBuff   = stMibData.iSize;
    stData.pszBuff = stMibData.pszData;

    tdr_fprintf(pstMeta, stdout, &stData, 0);

    set_entry_value(pstEntry,
                    pstCfg->pszValue,
                    stMibData.pszData + iOff,
                    tdr_get_entry_unitsize(pstEntry),
                    stdout);
    */

    return -1;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
#define CMD_MAP(C) { #C, &cmd_##C }
#define CMD_NULL { NULL, NULL }
static CmdEntry cmd_maps[] = {
    CMD_MAP(create),
    CMD_MAP(destroy),
    CMD_MAP(dump),
    CMD_MAP(loadmeta),
    CMD_MAP(loaddata),
    CMD_MAP(list),
    CMD_MAP(print),
//    CMD_MAP(modify),
    CMD_NULL
};
#undef CMD_NULL
#undef CMD_MAP

static const char *maps_help[] = {
    "create  tsm index struct",
    "destroy tsm index struct",
    "dump metalib",
    "load metalib from file",
    "load data from file",
    "list all uri while match",
    "print a data resource",
//    "modify data",
    NULL
};


//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int usage(int argc, char **argv)
{
    int i;
    int j;

    printf("Usage: %s [OPTION] <command>\n", argv[0]);

    printf("\nOptions:\n");
    for (i = 0; l_options[i].name; ++i)
    {
        printf("  -%c, --%s%s",
               l_options[i].val,
               l_options[i].name,
               l_options[i].has_arg ? "=<value>" : "");
        j = (int)strlen(l_options[i].name);
        j = 20 - j - (l_options[i].has_arg ? 8 : 0);
        for (; j >= 0; --j)
        {
            printf(" ");
        }
        printf("%s\n", options_help[i]);
    }

    printf("\nCommand:\n");
    for (i = 0; cmd_maps[i].name; ++i)
    {
        printf("  %s", cmd_maps[i].name);
        j = 20 - (int)strlen(cmd_maps[i].name);
        for (; j >= 0; --j)
        {
            printf(" ");
        }
        printf("%s\n", maps_help[i]);
    }

    printf("\n\nExample:\n"
           " %s --help\n"
           " %s create --help\n"
           " %s --version\n"
           " %s create --lib-size=8000 --uri-size=10000\n"
           " %s destroy --meta-uri=\"tsm-meta://12/*\"\n"
           " %s destroy --uri=\"tsm-data://12/*\"\n"
           " %s dump --format=tdr --save-to tdrs --meta-uri=\"tsm-meta://*\"\n"
           " %s loadmeta --file=\"./conf.tdr\" --bid=12\n"
           " %s loaddata --file=\"./conf.data\" \\\n"
           "\t--uri=\"tsm-data://12/tsmtest:1.2.3/conf\" \\\n"
           "\t--meta-uri=\"tsm-meta://12/tormsvr:14:11/TORMConnectDBInfo:10\"\n"
           " %s list --meta-uri=\"tsm-meta://12/*\"\n"
           " %s list --uri=\"tsm-data://12/*\"\n"
           " %s print --uri=\"tsm-data://12/*\"\n\n",
           argv[0], argv[0], argv[0], argv[0], argv[0], argv[0],
           argv[0], argv[0], argv[0], argv[0], argv[0], argv[0]);

    return 0;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int run_command(TsmTdrToolConf *conf)
{
    size_t len;
    int    i;

    if (conf->version)
    {
        return cmd_version(conf);
    }

    len = strlen(conf->cmd);
    for (i = 0; cmd_maps[i].name; ++i)
    {
        if (0 == strncmp(conf->cmd, cmd_maps[i].name, len))
        {
            TsmTdrHandleP h = NULL;
            int iRet;

            tsm_tdr_base_set(conf->index_key, conf->lib_size, conf->uri_size);

            if (0 == conf->help)
            {
                iRet = tsm_tdr_open(&h);
                tassert1(0 == iRet, NULL, 3, "tsm_tdr_init 失败(%d)", iRet);
            }

            iRet = cmd_maps[i].func(conf, h);
            tassert2(0 == iRet,
                     NULL, 4,
                     "maps -> func(%s): 失败(%d)", cmd_maps[i].name, iRet);
 
            if (0 == conf->help)
            {
                tsm_tdr_close(&h);
            }

            return 0;
        }
    }

    return -1;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int main(int argc, char **argv)
{
    TsmTdrToolConf  conf;
    int             iRet;

    if (parse_option(&conf, argc, argv) < 0)
    {
        return usage(argc, argv);
    }

    if (NULL == conf.cmd && 0 == conf.version)
    {
        return usage(argc, argv);
    }

    terror_clear(NULL);

    iRet = run_command(&conf);

    if (terror_count(NULL))
    {
        terror_dump(NULL);
    }

    return iRet;
}

//----------------------------------------------------------------------------
// THE END
//============================================================================
