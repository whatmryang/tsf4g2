/*
 **  @file RCSfile
 **  general description of this module
 **  Id
 **  @author Author
 **  @date Date
 **  @version Revision
 **  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
 **  @note Platform: Linux
 */
#include "tapp/tapp.h"
#include "tapp/tappctrlapi.h"
#include "version.h"
#include "../../../../lib_src/tapp/tapp_controller.h"

#define CLEARTAIL(str) (str)[sizeof(str)-1]='\0'
#define TAPP_MAX_ARGC 125


struct tagtappctrlEnv
{
    TLOGCATEGORYINST* pstLogCat;

    const char* pszSvrAppName;
    int iSvrBussId;
    const char* pszSvrId;
    const char* pszCIAddr;
    const char* pszSoFile;
    char szLogFile[PATH_MAX];
    char szPidFile[PATH_MAX];

    int argc;
    char* argv[TAPP_MAX_ARGC + 3];

    // next four functions is loaded from .so or .dll file
    int (*pfnInitPlugin)(CIPlugin* pstCIPlugin);
    int (*pfnInputProcess)(unsigned short argc, const char** argv);
    int (*pfnBinMsgProcess)(const char* pszMetaName, const char* pBuf, int iBuf);
    void (*pfnFreePlugin)(void);
};

typedef struct tagtappctrlEnv        CONSOLEENV;
typedef struct tagtappctrlEnv        *LCONSOLEENV;

static TAPPCTX gs_stAppCtx;
static CONSOLEENV gs_stEnv;

extern unsigned char g_szMetalib_tapp_ctrl[];

int tappctrl_dlopen(TAPPCTX *pstAppCtx, CONSOLEENV *pstEnv)
{
    int iRet = 0;
    void* pvDllHandle;

    pvDllHandle = dlopen(pstEnv->pszSoFile, RTLD_NOW);
    if (NULL == pvDllHandle)
    {
        printf("Error: failed to load sofile \'%s\'", pstEnv->pszSoFile);
        return -1;
    }else
    {
        pstEnv->pfnInitPlugin = dlsym(pvDllHandle, "InitPlugin");
        if (!pstEnv->pfnInitPlugin)
        {
            printf("Warning: \'%s\' contains no interface InitPlugin\n", pstEnv->pszSoFile);
        }
        pstEnv->pfnInputProcess = dlsym(pvDllHandle, "InputProcess");
        if (!pstEnv->pfnInputProcess)
        {
            printf("Warning: \'%s\' contains no interface InputProcess\n", pstEnv->pszSoFile);
        }
        pstEnv->pfnBinMsgProcess = dlsym(pvDllHandle, "BinMsgProcess");
        if (!pstEnv->pfnBinMsgProcess)
        {
            printf("Warning: \'%s\' contains no interface BinMsgProcess", pstEnv->pszSoFile);
        }
        pstEnv->pfnFreePlugin = dlsym(pvDllHandle, "FreePlugin");
        if (!pstEnv->pfnFreePlugin)
        {
            printf("Warning: \'%s\' contains no interface FreePlugin", pstEnv->pszSoFile);
        }
    }

    return iRet;
}

int tappctrl_normal_init(TAPPCTX *pstAppCtx, CONSOLEENV *pstEnv)
{
    int iRet = 0;

    if (0 > tapp_get_category(TLOG_DEF_CATEGORY_TEXTROOT, (void**)&pstEnv->pstLogCat))    
    {        
        printf("tapp_get_category run fail\n");
        return -1;    
    }    

    // set file-info for generating key of tbus channel
    if (!pstEnv->pszCIAddr)
    {
        iRet = tappctrl_set_ctrlfile_info(&pstEnv->pszCIAddr,
                pstEnv->pszSvrAppName, pstEnv->pszSvrId, pstEnv->iSvrBussId);
        if (0 > iRet)
        {
            printf("\nError: failed to set ctrl-file info\n");
            return -1;
        }
    }

    if (NULL != pstEnv->pszSoFile)
    {
        if (0 > tappctrl_dlopen(pstAppCtx, pstEnv))
        {
            printf("\nError: failed to load .so or .dll file\n");
            return -1;
        }
    }else
    {
        printf("\nWarning: no .so or .dll file is specified by \'--dll-file\'\n\n");
    }

    if (pstEnv->pfnInitPlugin)
    {
        CIPlugin stCIPlugin;
        stCIPlugin.tappctrl_send = tappctrl_send_msg;
        stCIPlugin.pstLogCat = pstEnv->pstLogCat;

        iRet = pstEnv->pfnInitPlugin(&stCIPlugin);
        if (0 > iRet)
        {
            printf("\nError: failed to init \'%s\'\n", pstEnv->pszSoFile);
            return -1;
        }
    }

    pstAppCtx->pfnPreprocCmdLine = pstEnv->pfnInputProcess;
    pstAppCtx->pfnProcCtrlRes = pstEnv->pfnBinMsgProcess;

    iRet = tappctrl_init(pstEnv->pszCIAddr, pstEnv->pstLogCat, (void*)g_szMetalib_tapp_ctrl);
    if (0 > iRet)
    {
        printf("\nError: tappctrl_init failed\n");
        return -1;
    }

    setvbuf(stderr, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);

    printf("# max input line length: %d characters\n", TAPPCTRL_INPUT_LINE_BUF_LEN);
    printf("# input \'help\' for usage information\n");
    printf("\n> ");
    tlog_info(pstEnv->pstLogCat, 0, 0, "tappctrl start");

    return iRet;
}

int tappctrl_normal_fini(TAPPCTX *pstAppCtx, CONSOLEENV *pstEnv)
{
    tlog_info(pstEnv->pstLogCat, 0, 0, "tappctrl finish");
    if (NULL != pstEnv->pfnFreePlugin)
        pstEnv->pfnFreePlugin();
    return 0;
}

void tappctrl_parse_opt(TAPPCTX *pstCtx, CONSOLEENV *pstEnv)
{
    int opt;
    int iOptIdx=0;
    int iOldOptErr;
    static int s_iOptChar;

    // --version(-v) and --help(-v) are processed by tapp-frame,
    // to list them here is to surpress getopt_long's error-msg-output
    static struct option s_astLongOptions[] = {
        {"svr-sock", 1, &s_iOptChar, 'a'},
        {"svr-name", 1, &s_iOptChar, 'n'},
        {"svr-id", 1, &s_iOptChar, 'i'},
        {"svr-buss-id", 1, &s_iOptChar, 'b'},
        {"dll-file", 1, &s_iOptChar, 's'},
        {"no-ctrl", 0, &s_iOptChar, 'c'},
        {"version", 0, &s_iOptChar, 'v'},
        {"help", 0, &s_iOptChar, 'h'},
        {0, 0, 0, 0}
    };

    iOldOptErr    =    opterr;
    while(-1 != (opt=getopt_long(pstCtx->argc, pstCtx->argv, "cvh", s_astLongOptions, &iOptIdx)) )
    {
        switch(opt)
        {
            case '\0':
                switch(*s_astLongOptions[iOptIdx].flag)
                {
                    case 'a':
                        pstEnv->pszCIAddr = optarg;
                        break;
                    case 'n':
                        pstEnv->pszSvrAppName = optarg;
                        break;
                    case 'i':
                        pstEnv->pszSvrId = optarg;
                        break;
                    case 'b':
                        pstEnv->iSvrBussId = strtol(optarg, NULL, 0);
                        break;
                    case 's':
                        pstEnv->pszSoFile = optarg;
                        break;
                    default:
                        break;
                }
                break;

            case '?':
                break;

            default:
                break;
        }
    }

    // restore the getopt environment.
    opterr = iOldOptErr;
    optarg    =    NULL;
    optind    =    1;
    optopt    =    '?';
}

int tappctrl_help_for_startup(int argc, char* argv[])
{
    printf("Usage:\n");
    printf("%s [ --svr-sock=path | --svr-name=name [--svr-bussid=bid] [--svr-id=id] ] [ --dll-file=path ]\n\n", argv[0]);
    printf("Options:\n");
    printf(" --svr-sock=path\n\tspecify communication file info of server\n");
    printf(" --svr-name=name\n\tspecify server's name\n");
    printf(" --svr-bussid=bid\n\tspecify server's bussiness id\n");
    printf(" --svr-id=id\n\tspecify server's bus id\n");
    printf(" --dll-file=path\n\tspecify .so or .dll file that implements interfaces defined by this tappctrl program\n");
    printf("\tBe careful: \"./tapptappctrl.so\" is different from \"tapptappctrl.so\"\n");
    printf(" --help, -h\n\tshow this help infomation\n");
    printf(" --version, -h\n\tshow version infomation\n");
    exit(0);
    return 0;
}

int tappctrl_adjust_argv(int argc, char** argv)
{
    if (TAPP_MAX_ARGC < argc)
    {
        printf("too many arguments<%d>, only %d arguments supported\n", argc, TAPP_MAX_ARGC);
        return -1;
    }

    gs_stEnv.argc = 0;
    for (gs_stEnv.argc = 0; gs_stEnv.argc < argc; gs_stEnv.argc++)
    {
        if (strcasecmp(argv[gs_stEnv.argc], "control")
                &&strcasecmp(argv[gs_stEnv.argc], "restart")
                &&strcasecmp(argv[gs_stEnv.argc], "stop")
                &&strcasecmp(argv[gs_stEnv.argc], "reload")
                &&strcasecmp(argv[gs_stEnv.argc], "kill")
                &&strcasecmp(argv[gs_stEnv.argc], "--daemon")
                &&strcasecmp(argv[gs_stEnv.argc], "-D"))
        {
            gs_stEnv.argv[gs_stEnv.argc] = argv[gs_stEnv.argc];
        }else
        {
            gs_stEnv.argv[gs_stEnv.argc] = "";
        }

    }

    gs_stEnv.argv[gs_stEnv.argc] = "start";
    gs_stEnv.argv[++gs_stEnv.argc] = "--no-ctrl";
    gs_stEnv.argv[++gs_stEnv.argc] = NULL;

    return 0;
}

int main(int argc, char* argv[])
{
    int iRet;
    void* pvArg    =    &gs_stEnv;

    memset(&gs_stAppCtx, 0, sizeof(gs_stAppCtx));
    memset(&gs_stEnv, 0, sizeof(gs_stEnv));

    iRet = tappctrl_adjust_argv(argc, argv);
    if (0 > iRet)
    {
        printf("Error: tappctrl_adjust_argv failed\n");
        return -1;
    }

    gs_stAppCtx.argc    =    gs_stEnv.argc;
    gs_stAppCtx.argv    =    gs_stEnv.argv;

    tappctrl_parse_opt(&gs_stAppCtx, &gs_stEnv);

    gs_stAppCtx.pfnUsage   =    tappctrl_help_for_startup;
    gs_stAppCtx.pfnInit    =    (PFNTAPPFUNC)tappctrl_normal_init;
    gs_stAppCtx.pfnProc    =    (PFNTAPPFUNC)tapp_controller_proc;
    gs_stAppCtx.pfnFini    =    (PFNTAPPFUNC)tappctrl_normal_fini;

    gs_stAppCtx.iNoLoadConf = 1;

    iRet = tapp_def_init(&gs_stAppCtx, pvArg);
    if( 0 > iRet )
    {
        printf("Error: tapp Initialization failed.\n");
        return iRet;
    }

    tapp_def_mainloop(&gs_stAppCtx, pvArg);

    tapp_def_fini(&gs_stAppCtx, pvArg);

    return iRet;
}


