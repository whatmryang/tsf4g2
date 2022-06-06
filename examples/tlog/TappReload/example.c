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
typedef struct tagexampleEnv		*LEXAMPLEENV;


static TAPPCTX gs_stAppCtx;
extern unsigned char g_szMetalib_example[];

EXAMPLEENV gs_stEnv;
static EXAMPLERUN_CUMULATE gs_stCumu;
static EXAMPLERUN_STATUS gs_stStat;

static volatile int gs_ShouldWrite= 0;

int example_stop(TAPPCTX *pstAppCtx, EXAMPLEENV *pstEnv);
int example_tick(TAPPCTX *pstAppCtx, EXAMPLEENV *pstEnv);
int example_fini(TAPPCTX *pstAppCtx, EXAMPLEENV *pstEnv);
int example_reload(TAPPCTX *pstAppCtx, EXAMPLEENV *pstEnv);
int example_proc(TAPPCTX *pstAppCtx, EXAMPLEENV *pstEnv);
int example_init(TAPPCTX *pstAppCtx, EXAMPLEENV *pstEnv);

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

	if (0 > tapp_get_category(TLOG_DEF_CATEGORY_TEXTROOT, (&pstEnv->pstLogCat)))
	{
		printf("tapp_get_category run fail\n");
		return -1;
	}

	printf("example start\n");
	tlog_info(pstEnv->pstLogCat, 0, 0, "example start");
	tlog_info(pstEnv->pstLogCat, 0, 0, "writting info");
	tlog_info(pstEnv->pstLogCat, 0, 0, "This should appear");
	tlog_info(pstEnv->pstLogCat,10, 10, " 10  10This should not appear");
	tlog_info(pstEnv->pstLogCat,10, 20, " 10  20This should appear");
	return 0;
}

int example_proc(TAPPCTX *pstAppCtx, EXAMPLEENV *pstEnv)
{
	int iBusy=0;
	TOS_UNUSED_ARG(pstAppCtx);
	if(gs_ShouldWrite)
	{
		gs_ShouldWrite = 0;

		#define INFOCLSID(cls,id)  tlog_info(pstEnv->pstLogCat,id, cls, " id=%d cls=%d",cls,id);

		INFOCLSID(10,10);
		INFOCLSID(20,20);
		INFOCLSID(30,30);
		INFOCLSID(40,40);
		INFOCLSID(50,50);
		INFOCLSID(60,60);
		INFOCLSID(70,70);

        #undef INFOCLSID
		#define INFOCLSID(cls,id)  tlog_debug(pstEnv->pstLogCat,id, cls, " id=%d cls=%d",cls,id);

		INFOCLSID(10,10);
		INFOCLSID(20,20);
		INFOCLSID(30,30);
		INFOCLSID(40,40);
		INFOCLSID(50,50);
		INFOCLSID(60,60);
		INFOCLSID(70,70);

		#define ERRORCLSID(cls,id)  tlog_error(pstEnv->pstLogCat,id, cls, " id=%d cls=%d",cls,id);

		ERRORCLSID(10,10);
		ERRORCLSID(20,20);
		ERRORCLSID(30,30);
		ERRORCLSID(40,40);
		ERRORCLSID(50,50);
		ERRORCLSID(60,60);
		ERRORCLSID(70,70);

	}

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
	TOS_UNUSED_ARG(pstEnv);
	pstPreConf = (EXAMPLECONF   *)pstAppCtx->stConfPrepareData.pszBuff;

	printf("example reload\n");
	gs_ShouldWrite = 5;
	return 0;
}

int example_fini(TAPPCTX *pstAppCtx, EXAMPLEENV *pstEnv)
{
	TOS_UNUSED_ARG(pstAppCtx);
	printf("example finish\n");
	tlog_info(pstEnv->pstLogCat, 0, 0, "example finish");
	return 0;
}

int example_tick(TAPPCTX *pstAppCtx, EXAMPLEENV *pstEnv)
{
	TOS_UNUSED_ARG(pstAppCtx);
	TOS_UNUSED_ARG(pstEnv);
	//tlog_debug(pstEnv->pstLogCat, 0, 0, "tick log debug");
	//tlog_info(pstEnv->pstLogCat, 0, 0, "tick log info");
	return 0;
}

int example_stop(TAPPCTX *pstAppCtx, EXAMPLEENV *pstEnv)
{
	int iReady=0;
	TOS_UNUSED_ARG(pstAppCtx);
	TOS_UNUSED_ARG(pstEnv);
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

	gs_stAppCtx.pfnInit	=	(PFNTAPPFUNC)example_init;

	gs_stAppCtx.iLib = (intptr_t)g_szMetalib_example;
	gs_stAppCtx.stConfData.pszMetaName = "exampleconf";
	gs_stAppCtx.uiVersion = TAPP_MAKE_VERSION(MAJOR, MINOR, REV, BUILD);

	gs_stAppCtx.pfnFini	=	(PFNTAPPFUNC)example_fini;
	gs_stAppCtx.pfnProc	=	(PFNTAPPFUNC)example_proc;
	gs_stAppCtx.pfnTick	=	(PFNTAPPFUNC)example_tick;
	gs_stAppCtx.pfnReload	=	(PFNTAPPFUNC)example_reload;
	gs_stAppCtx.pfnStop  =   (PFNTAPPFUNC)example_stop;

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


