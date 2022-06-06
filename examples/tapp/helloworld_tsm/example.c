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
#include "tdr/tdr.h"
#include "tlog/tlog.h"
#include "tloghelp/tlogload.h"
#include "example_conf_desc.h"
#include "version.h"

struct tagexampleEnv
{
	EXAMPLECONF *pstConf;
	EXAMPLERUN_CUMULATE *pstRunCumu;
	EXAMPLERUN_STATUS   *pstRunStat;
	TLOGCATEGORYINST* pstLogCat;
};

typedef struct tagexampleEnv		EXAMPLEENV;
typedef struct tagexampleEnv		*LPEXAMPLEENV;


static TAPPCTX gs_stAppCtx;
extern unsigned char g_szMetalib_example[];

EXAMPLEENV gs_stEnv;
static EXAMPLERUN_CUMULATE gs_stCumu;
static EXAMPLERUN_STATUS gs_stStat;
////////////////////////////////////////////////////////
int example_init(TAPPCTX *pstAppCtx, EXAMPLEENV *pstEnv);
int example_proc(TAPPCTX *pstAppCtx, EXAMPLEENV *pstEnv);
int example_reload(TAPPCTX *pstAppCtx, EXAMPLEENV *pstEnv);
int example_fini(TAPPCTX *pstAppCtx, EXAMPLEENV *pstEnv);
int example_tick(TAPPCTX *pstAppCtx, EXAMPLEENV *pstEnv);
int example_stop(TAPPCTX *pstAppCtx, EXAMPLEENV *pstEnv);

int example_proc_binary_req(TAPPCTX *pstAppCtx,
        void *pstEnv, const char* pszMetaName, const char* pBuf, int iLen);

const char* example_console_help(void);

int example_proc_cmdline_req(TAPPCTX *pstAppCtx,
        void *pvArg, unsigned short argc, const char** argv);

int example_controller_init(TAPPCTX *pstAppCtx, void *argv);
int example_proc_binary_res(const char* pszMetaName, const char* pBuf, int tLen);
int example_proc_stdin(unsigned short argc, const char** argv);
int example_controller_fini(TAPPCTX *pstAppCtx, void *argv);
////////////////////////////////////////////////////////

int example_init(TAPPCTX *pstAppCtx, EXAMPLEENV *pstEnv)
{
	if (NULL == pstAppCtx->stConfData.pszBuff || 0 == pstAppCtx->stConfData.iMeta
		|| NULL == pstAppCtx->pszConfFile )
	{
		return -1;
	}
	pstEnv->pstConf = (EXAMPLECONF *)pstAppCtx->stConfData.pszBuff;

	pstEnv->pstRunCumu = (EXAMPLERUN_CUMULATE *)pstAppCtx->stRunDataCumu.pszBuff;
	if (NULL == pstEnv->pstRunCumu)
	{
		pstEnv->pstRunCumu = &gs_stCumu;
	}

	pstEnv->pstRunStat = (EXAMPLERUN_STATUS *)pstAppCtx->stRunDataStatus.pszBuff;
	if (NULL == pstEnv->pstRunStat)
	{
		pstEnv->pstRunStat = &gs_stStat;
	}

	if (0 > tapp_get_category(TLOG_DEF_CATEGORY_TEXTROOT, (void *)(&pstEnv->pstLogCat)))
	{
		printf("tapp_get_category run fail\n");
		return -1;
	}

	printf("example start\n");
	tlog_info(pstEnv->pstLogCat, 0, 0, "example start");

	return 0;
}

int example_proc(TAPPCTX *pstAppCtx, EXAMPLEENV *pstEnv)
{
	int iBusy=0;

	if (iBusy)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

int example_reload(TAPPCTX *pstAppCtx, EXAMPLEENV *pstEnv)
{
	EXAMPLECONF   *pstPreConf;

	pstPreConf = (EXAMPLECONF   *)pstAppCtx->stConfPrepareData.pszBuff;

	printf("example reload\n");
	tlog_info(pstEnv->pstLogCat, 0, 0, "example reload");

	return 0;
}

int example_fini(TAPPCTX *pstAppCtx, EXAMPLEENV *pstEnv)
{

	printf("example finish\n");
	tlog_info(pstEnv->pstLogCat, 0, 0, "example finish");
	return 0;
}

int example_tick(TAPPCTX *pstAppCtx, EXAMPLEENV *pstEnv)
{
	return 0;
}

int example_stop(TAPPCTX *pstAppCtx, EXAMPLEENV *pstEnv)
{
	int iReady=0;

	iReady = 1;
	if (iReady)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

// when tapp recv a binary msg, it will call this interface to process it
int example_proc_binary_req(TAPPCTX *pstAppCtx, void *pstEnv, const char* pszMetaName, const char* pBuf, int iLen)
{
    int iRet = 0;

    // just send back recved msg
    iRet = tappctrl_send_msg(pszMetaName, pBuf, iLen);
    if (0 > iRet)
    {
        printf("failed to backward recved msg\n");
    }

    return iRet;
}

// when tapp recv a "help" msg, it will call this interface to process it
const char* example_console_help(void)
{
    static const char* pszHelp;

    tlog_info(gs_stEnv.pstLogCat, 0, 0, "request for help info");

    pszHelp = "This is an demo.\n"
        "All recved binary msgs will be send back.\n"
        "No cmdline msgs are supported.\n";

    return pszHelp;
}

// when tapp recv a text msg, it will call this interface to process it
int example_proc_cmdline_req(TAPPCTX *pstAppCtx, void *pvArg, unsigned short argc, const char** argv)
{
    int i = 0;
    LPEXAMPLEENV pstEnv;

    pstEnv = (LPEXAMPLEENV)pvArg;

    tlog_info(pstEnv->pstLogCat, 0, 0, "recv cmdline msg: argc<%d>", argc);

    printf("recv cmd line msg: ");
    tappctrl_send_string("Error: unsupported command");
    for (i = 0; i < argc; i++)
        printf("%s ", argv[i]);
    printf("\n");

    return 0;
}

int example_controller_init(TAPPCTX *pstAppCtx, void *argv)
{
    int iRet = 0;

    LPEXAMPLEENV pstEnv = (LPEXAMPLEENV)argv;
	if (0 > tapp_get_category(TLOG_DEF_CATEGORY_TEXTROOT, (void *)(&pstEnv->pstLogCat)))
	{
		printf("tapp_get_category run fail\n");
		return -1;
	}

    tlog_error(pstEnv->pstLogCat, 0, 0, "example controller init do nothing");

    return iRet;
}

int example_proc_binary_res(const char* pszMetaName, const char* pBuf, int tLen)
{
    int iRet = 0;

    printf("recv response for meta: %s\n", pszMetaName);
    tlog_error(gs_stEnv.pstLogCat, 0, 0, "recv response for meta: %s", pszMetaName);

    return iRet;
}

//	int (*pfnPreprocCmdLine)(unsigned short argc, const char** argv);
int example_proc_stdin(unsigned short argc, const char** argv)
{
    tlog_error(gs_stEnv.pstLogCat, 0, 0, "recv command: %s, but not interested in it", argv[0]);
    // return 0, 表示对输入不感兴趣，把这个输入交给tapp处理
    // return 1, 表示对输入感兴趣，由自己处理这个输入
    return 0;
}

int example_controller_fini(TAPPCTX *pstAppCtx, void *argv)
{
    int iRet = 0;

    LPEXAMPLEENV pstEnv = (LPEXAMPLEENV)argv;
    tlog_error(pstEnv->pstLogCat, 0, 0, "example controller fini do nothing");

    return iRet;
}

static int example_gen_version(TAPPCTX *pstAppCtx, void *argv)
{
	printf("Gen version from function\n");
 	return 0;
}


int main(int argc, char* argv[])
{
	int iRet;
	void* pvArg	=	&gs_stEnv;

	memset(&gs_stAppCtx, 0, sizeof(gs_stAppCtx));
	memset(&gs_stEnv, 0, sizeof(gs_stEnv));

	gs_stAppCtx.argc	=	argc;
	gs_stAppCtx.argv	=	argv;

	gs_stAppCtx.pfnInit	=	(PFNTAPPFUNC)example_init;
	gs_stAppCtx.pfnFini	=	(PFNTAPPFUNC)example_fini;
	gs_stAppCtx.pfnProc	=	(PFNTAPPFUNC)example_proc;
	gs_stAppCtx.pfnTick	=	(PFNTAPPFUNC)example_tick;
	gs_stAppCtx.pfnReload	=	(PFNTAPPFUNC)example_reload;
	gs_stAppCtx.pfnStop  =   (PFNTAPPFUNC)example_stop;

    // 设置作为server启动时提供控制服务所需的接口函数
    gs_stAppCtx.pfnProcCmdLine = example_proc_cmdline_req;
    gs_stAppCtx.pfnProcCtrlReq = example_proc_binary_req;
    gs_stAppCtx.pfnGetCtrlUsage = example_console_help;

    // 设置作为控制终端模式启动时需要使用的接口
    gs_stAppCtx.pfnControllerInit = example_controller_init;
    gs_stAppCtx.pfnProcCtrlRes = example_proc_binary_res;

    // pfnPreprocCmdLine return 0, 表示对输入不感兴趣，把这个输入交给tapp处理
    // pfnPreprocCmdLine return 1, 表示对输入感兴趣，由自己处理这个输入
    gs_stAppCtx.pfnPreprocCmdLine = example_proc_stdin;
    gs_stAppCtx.pfnControllerFini = example_controller_fini;

	gs_stAppCtx.iLib = (intptr_t)g_szMetalib_example;
	gs_stAppCtx.stConfData.pszMetaName = "exampleconf";
	gs_stAppCtx.uiVersion = TAPP_MAKE_VERSION(MAJOR, MINOR, REV, BUILD);

	gs_stAppCtx.pfnVersionGenerator = example_gen_version;

	iRet	=	tapp_def_init(&gs_stAppCtx, pvArg);
	if( iRet<0 )
	{
		printf("Error: app Initialization failed.\n");
		return iRet;
	}

	iRet	=	tapp_def_mainloop(&gs_stAppCtx, pvArg);

	tapp_def_fini(&gs_stAppCtx, pvArg);

	return iRet;
}
