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
#include "MultiThread_conf_desc.h"
#include "version.h"
#include "pal/tthread.h"

#define N_THREAD 3

struct tagMultiThreadEnv
{
	MULTITHREADCONF *pstConf;
	MULTITHREADRUN_CUMULATE *pstRunCumu;
	MULTITHREADRUN_STATUS   *pstRunStat;
	TLOGCATEGORYINST* pstLogCat;
};

typedef struct tagMultiThreadEnv		MULTITHREADENV;
typedef struct tagMultiThreadEnv		*LMULTITHREADENV;


struct tagPerThreadData
{
	pthread_t threadHandle;
	int threadID;
	MULTITHREADCONF *pstConf;
	MULTITHREADRUN_CUMULATE *pstRunCumu;
	MULTITHREADRUN_STATUS   *pstRunStat;
	TLOGCATEGORYINST* pstLogCat;
};

typedef struct tagPerThreadData		PERTHREADDATA;
typedef struct tagPerThreadData		*LPERTHREADDATA;


static TAPPCTX gs_stAppCtx;
extern unsigned char g_szMetalib_MultiThread[];

MULTITHREADENV gs_stEnv;
static MULTITHREADRUN_CUMULATE gs_stCumu;
static MULTITHREADRUN_STATUS gs_stStat;
PERTHREADDATA astThreadData[N_THREAD];

static void *log_thread(void *pData)
{
	LPERTHREADDATA pstThreadData=pData;
	tlog_error(pstThreadData->pstLogCat,0, 0,"Thread %d logging",pstThreadData->threadID);
	return NULL;
}


static int MultiThread_init(TAPPCTX *pstAppCtx, MULTITHREADENV *pstEnv)
{
	if (NULL == pstAppCtx->stConfData.pszBuff || 0 == pstAppCtx->stConfData.iMeta
		|| NULL == pstAppCtx->pszConfFile )
	{
		return -1;
	}
	pstEnv->pstConf = (MULTITHREADCONF *)pstAppCtx->stConfData.pszBuff;

	pstEnv->pstRunCumu = (MULTITHREADRUN_CUMULATE *)pstAppCtx->stRunDataCumu.pszBuff;
	if (NULL == pstEnv->pstRunCumu)
	{
		pstEnv->pstRunCumu = &gs_stCumu;
	}

	pstEnv->pstRunStat = (MULTITHREADRUN_STATUS *)pstAppCtx->stRunDataStatus.pszBuff;
	if (NULL == pstEnv->pstRunStat)
	{
		pstEnv->pstRunStat = &gs_stStat;
	}

	if (0 > tapp_get_category(TLOG_DEF_CATEGORY_TEXTROOT, (&pstEnv->pstLogCat)))
	{
		printf("tapp_get_category run fail\n");
		return -1;
	}

	printf("MultiThread start\n");
	tlog_info(pstEnv->pstLogCat, 0, 0, "MultiThread start");


	// 在初始化过程，先通过getCategory获取需要克隆的category，注意，category不能是没有设备的日志类
	//
	//
	{

	TLOGCATEGORYINST *pstCat=NULL;

	int i = 0;
	if(0 > tapp_get_category("texttrace",&pstCat))
	{
		printf("Failed to get category\n");
		return -1;
	}

	for(i = 0;i < N_THREAD;i++)
	{
		char catname[128];
		snprintf(catname,sizeof(catname),"thread%d",i);
		tlog_clone_category(pstCat,catname);
		tapp_get_category(catname, &(astThreadData[i].pstLogCat));
		astThreadData[i].threadID = i;
		//astThreadData[i].threadHandle = ;
	}


	for(i = 0;i < N_THREAD;i++)
	{
		pthread_create(&(astThreadData[i].threadHandle),NULL,log_thread,astThreadData+i);
	}

	}


	return 0;
}


static int MultiThread_proc(TAPPCTX *pstAppCtx, MULTITHREADENV *pstEnv)
{
	int iBusy=0;
	sleep(1);
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

static int MultiThread_reload(TAPPCTX *pstAppCtx, MULTITHREADENV *pstEnv)
{
	MULTITHREADCONF   *pstPreConf;

	pstPreConf = (MULTITHREADCONF   *)pstAppCtx->stConfPrepareData.pszBuff;

	printf("MultiThread reload\n");
	tlog_info(pstEnv->pstLogCat, 0, 0, "MultiThread reload");

	return 0;
}

static int MultiThread_fini(TAPPCTX *pstAppCtx, MULTITHREADENV *pstEnv)
{

	printf("MultiThread finish\n");
	tlog_info(pstEnv->pstLogCat, 0, 0, "MultiThread finish");
	return 0;
}

static int MultiThread_tick(TAPPCTX *pstAppCtx, MULTITHREADENV *pstEnv)
{
	return 0;
}

static int MultiThread_stop(TAPPCTX *pstAppCtx, MULTITHREADENV *pstEnv)
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

	gs_stAppCtx.pfnInit	=	(PFNTAPPFUNC)MultiThread_init;

	gs_stAppCtx.iLib = (intptr_t)g_szMetalib_MultiThread;
	gs_stAppCtx.stConfData.pszMetaName = "MultiThreadconf";
	gs_stAppCtx.uiVersion = TAPP_MAKE_VERSION(MAJOR, MINOR, REV, BUILD);

	iRet	=	tapp_def_init(&gs_stAppCtx, pvArg);
	if( iRet<0 )
	{
		printf("Error: app Initialization failed.\n");
		return iRet;
	}

	gs_stAppCtx.pfnFini	=	(PFNTAPPFUNC)MultiThread_fini;
	gs_stAppCtx.pfnProc	=	(PFNTAPPFUNC)MultiThread_proc;
	gs_stAppCtx.pfnTick	=	(PFNTAPPFUNC)MultiThread_tick;
	gs_stAppCtx.pfnReload	=	(PFNTAPPFUNC)MultiThread_reload;
	gs_stAppCtx.pfnStop  =   (PFNTAPPFUNC)MultiThread_stop;

	iRet	=	tapp_def_mainloop(&gs_stAppCtx, pvArg);

	tapp_def_fini(&gs_stAppCtx, pvArg);

	return iRet;
}


