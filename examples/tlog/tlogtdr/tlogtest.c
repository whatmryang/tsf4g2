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
#include "tlogtest_conf_desc.h"
#include "version.h"

struct tagtlogtestEnv
{
	TLOGTESTCONF *pstConf;
	LPTDRMETA  pstConfMeta;

	TLOGTESTRUN_CUMULATE *pstRunCumu;
	TLOGTESTRUN_STATUS   *pstRunStat;
	TLOGCATEGORYINST* pstLogCat;
};

typedef struct tagtlogtestEnv		TLOGTESTENV;
typedef struct tagtlogtestEnv		*LTLOGTESTENV;


static TAPPCTX gs_stAppCtx;
extern unsigned char g_szMetalib_tlogtest[];

TLOGTESTENV gs_stEnv;
static TLOGTESTRUN_CUMULATE gs_stCumu;
static TLOGTESTRUN_STATUS gs_stStat;

static int tlogtest_init(TAPPCTX *pstAppCtx, TLOGTESTENV *pstEnv)
{
	if (NULL == pstAppCtx->stConfData.pszBuff || 0 == pstAppCtx->stConfData.iMeta
		|| NULL == pstAppCtx->pszConfFile )
	{
		return -1;
	}
	pstEnv->pstConf = (TLOGTESTCONF *)pstAppCtx->stConfData.pszBuff;

	pstEnv->pstRunCumu = (TLOGTESTRUN_CUMULATE *)pstAppCtx->stRunDataCumu.pszBuff;
	if (NULL == pstEnv->pstRunCumu)
	{
		pstEnv->pstRunCumu = &gs_stCumu;
	}

	pstEnv->pstRunStat = (TLOGTESTRUN_STATUS *)pstAppCtx->stRunDataStatus.pszBuff;
	if (NULL == pstEnv->pstRunStat)
	{
		pstEnv->pstRunStat = &gs_stStat;
	}

	//gs_stAppCtx.iLib = (int)g_szMetalib_tlogtest;
	//gs_stAppCtx.stConfData.pszMetaName = "tlogtestconf";

	pstEnv->pstConfMeta = tdr_get_meta_by_name((LPTDRMETALIB)gs_stAppCtx.iLib , gs_stAppCtx.stConfData.pszMetaName);

	if( NULL == pstEnv->pstConfMeta)
	{
		printf("Failed to init config meta\n");
		return -1;
	}

	if (0 > tapp_get_category(TLOG_DEF_CATEGORY_TEXTROOT, (&pstEnv->pstLogCat)))
	{
		printf("tapp_get_category run fail\n");
		return -1;
	}

	printf("tlogtest start\n");
	tlog_info(pstEnv->pstLogCat, 0, 0, "tlogtest start");
	tlog_info(pstEnv->pstLogCat, 10, 10, "10 10 tlogtest start");
	tlog_info(pstEnv->pstLogCat, 99, 0, "99 0 tlogtest start");
	tlog_info(pstEnv->pstLogCat, 0, 1000, "0 1000 tlogtest start");
	tlog_info(pstEnv->pstLogCat, 1000, 0, "1000 0 tlogtest start");


	tlog_fatal_dr(pstEnv->pstLogCat, 0,0,pstEnv->pstConfMeta,pstEnv->pstConf,sizeof(*(pstEnv->pstConf)),0);

	return 0;
}

static int tlogtest_proc(TAPPCTX *pstAppCtx, TLOGTESTENV *pstEnv)
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

static int tlogtest_reload(TAPPCTX *pstAppCtx, TLOGTESTENV *pstEnv)
{
	TLOGTESTCONF   *pstPreConf;

	pstPreConf = (TLOGTESTCONF   *)pstAppCtx->stConfPrepareData.pszBuff;

	printf("tlogtest reload\n");
	tlog_info(pstEnv->pstLogCat, 0, 0, "tlogtest reload");

	return 0;
}

static int tlogtest_fini(TAPPCTX *pstAppCtx, TLOGTESTENV *pstEnv)
{

	printf("tlogtest finish\n");
	tlog_info(pstEnv->pstLogCat, 0, 0, "tlogtest finish");
	return 0;
}

static int tlogtest_tick(TAPPCTX *pstAppCtx, TLOGTESTENV *pstEnv)
{
	static int time = 0;

	if(time != pstAppCtx->stCurr.tv_sec)
	{
		time =  pstAppCtx->stCurr.tv_sec;
		tlog_info(pstEnv->pstLogCat, 99, 0, "99 0 tlogtest start");
	}
	return 0;
}

static int tlogtest_stop(TAPPCTX *pstAppCtx, TLOGTESTENV *pstEnv)
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

	gs_stAppCtx.pfnInit	=	(PFNTAPPFUNC)tlogtest_init;

	gs_stAppCtx.iLib = (intptr_t)g_szMetalib_tlogtest;
	gs_stAppCtx.stConfData.pszMetaName = "tlogtestconf";
	gs_stAppCtx.uiVersion = TAPP_MAKE_VERSION(MAJOR, MINOR, REV, BUILD);

	iRet	=	tapp_def_init(&gs_stAppCtx, pvArg);
	if( iRet<0 )
	{
		printf("Error: app Initialization failed.\n");
		return iRet;
	}

	gs_stAppCtx.pfnFini	=	(PFNTAPPFUNC)tlogtest_fini;
	gs_stAppCtx.pfnProc	=	(PFNTAPPFUNC)tlogtest_proc;
	gs_stAppCtx.pfnTick	=	(PFNTAPPFUNC)tlogtest_tick;
	gs_stAppCtx.pfnReload	=	(PFNTAPPFUNC)tlogtest_reload;
	gs_stAppCtx.pfnStop  =   (PFNTAPPFUNC)tlogtest_stop;

	iRet	=	tapp_def_mainloop(&gs_stAppCtx, pvArg);

	tapp_def_fini(&gs_stAppCtx, pvArg);

	return iRet;
}


