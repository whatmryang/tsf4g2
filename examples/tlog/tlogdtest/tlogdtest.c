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
#include "tlogdtest_conf_desc.h"
#include "version.h"

struct tagtlogdtestEnv
{
	TLOGDTESTCONF *pstConf;
	TLOGDTESTRUN_CUMULATE *pstRunCumu;
	TLOGDTESTRUN_STATUS   *pstRunStat;
	TLOGCATEGORYINST* pstLogCat;
};

typedef struct tagtlogdtestEnv		TLOGDTESTENV;
typedef struct tagtlogdtestEnv		*LTLOGDTESTENV;


static TAPPCTX gs_stAppCtx;
extern unsigned char g_szMetalib_tlogdtest[];

TLOGDTESTENV gs_stEnv;
static TLOGDTESTRUN_CUMULATE gs_stCumu;
static TLOGDTESTRUN_STATUS gs_stStat;

static int tlogdtest_init(TAPPCTX *pstAppCtx, TLOGDTESTENV *pstEnv)
{
	if (NULL == pstAppCtx->stConfData.pszBuff || 0 == pstAppCtx->stConfData.iMeta
		|| NULL == pstAppCtx->pszConfFile )
	{
		return -1;
	}
	pstEnv->pstConf = (TLOGDTESTCONF *)pstAppCtx->stConfData.pszBuff;

	pstEnv->pstRunCumu = (TLOGDTESTRUN_CUMULATE *)pstAppCtx->stRunDataCumu.pszBuff;
	if (NULL == pstEnv->pstRunCumu)
	{
		pstEnv->pstRunCumu = &gs_stCumu;
	}

	pstEnv->pstRunStat = (TLOGDTESTRUN_STATUS *)pstAppCtx->stRunDataStatus.pszBuff;
	if (NULL == pstEnv->pstRunStat)
	{
		pstEnv->pstRunStat = &gs_stStat;
	}

	if (0 > tapp_get_category(TLOG_DEF_CATEGORY_TEXTROOT, (&pstEnv->pstLogCat)))
	{
		printf("tapp_get_category run fail\n");
		return -1;
	}

	printf("tlogdtest start\n");
	tlog_info(pstEnv->pstLogCat, 0, 0, "tlogdtest start");
	tlog_info(pstEnv->pstLogCat, 0, 0, "tlogdtest start");
	tlog_info(pstEnv->pstLogCat, 0, 0, "tlogdtest start");
	tlog_info(pstEnv->pstLogCat, 0, 0, "tlogdtest start");
	tlog_info(pstEnv->pstLogCat, 0, 0, "tlogdtest start");
	tlog_info(pstEnv->pstLogCat, 0, 0, "tlogdtest start");
	tlog_info(pstEnv->pstLogCat, 0, 0, "tlogdtest start");

	return 0;
}

static int tlogdtest_proc(TAPPCTX *pstAppCtx, TLOGDTESTENV *pstEnv)
{
	int iBusy=0;
	tapp_exit_mainloop();
	if (iBusy)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

static int tlogdtest_reload(TAPPCTX *pstAppCtx, TLOGDTESTENV *pstEnv)
{
	TLOGDTESTCONF   *pstPreConf;

	pstPreConf = (TLOGDTESTCONF   *)pstAppCtx->stConfPrepareData.pszBuff;

	printf("tlogdtest reload\n");
	tlog_info(pstEnv->pstLogCat, 0, 0, "tlogdtest reload");

	return 0;
}

static int tlogdtest_fini(TAPPCTX *pstAppCtx, TLOGDTESTENV *pstEnv)
{

	printf("tlogdtest finish\n");
	tlog_info(pstEnv->pstLogCat, 0, 0, "tlogdtest finish");
	return 0;
}

static int tlogdtest_tick(TAPPCTX *pstAppCtx, TLOGDTESTENV *pstEnv)
{
	return 0;
}

static int tlogdtest_stop(TAPPCTX *pstAppCtx, TLOGDTESTENV *pstEnv)
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

int main(int argc, char* argv[])
{
	int iRet;
	void* pvArg	=	&gs_stEnv;

	memset(&gs_stAppCtx, 0, sizeof(gs_stAppCtx));
	memset(&gs_stEnv, 0, sizeof(gs_stEnv));

	gs_stAppCtx.argc	=	argc;
	gs_stAppCtx.argv	=	argv;

	gs_stAppCtx.pfnInit	=	(PFNTAPPFUNC)tlogdtest_init;

	gs_stAppCtx.iLib = (intptr_t)g_szMetalib_tlogdtest;
	gs_stAppCtx.stConfData.pszMetaName = "tlogdtestconf";
	gs_stAppCtx.uiVersion = TAPP_MAKE_VERSION(MAJOR, MINOR, REV, BUILD);

	iRet	=	tapp_def_init(&gs_stAppCtx, pvArg);
	if( iRet<0 )
	{
		printf("Error: app Initialization failed.\n");
		return iRet;
	}

	gs_stAppCtx.pfnFini	=	(PFNTAPPFUNC)tlogdtest_fini;
	gs_stAppCtx.pfnProc	=	(PFNTAPPFUNC)tlogdtest_proc;
	gs_stAppCtx.pfnTick	=	(PFNTAPPFUNC)tlogdtest_tick;
	gs_stAppCtx.pfnReload	=	(PFNTAPPFUNC)tlogdtest_reload;
	gs_stAppCtx.pfnStop  =   (PFNTAPPFUNC)tlogdtest_stop;

	iRet	=	tapp_def_mainloop(&gs_stAppCtx, pvArg);

	tapp_def_fini(&gs_stAppCtx, pvArg);

	return iRet;
}


