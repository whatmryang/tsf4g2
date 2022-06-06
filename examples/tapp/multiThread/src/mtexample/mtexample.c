/*
**  @file RCSfile
**  general description of this module
**  Id
**  @author mickeyxu
**  @date Date
**  @version Revision
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#include "mtexample.h"
#include "tbus/tbus.h"


extern unsigned char g_szMetalib_example[];

TAPPCTX gs_stAppCtx;
MTEXAMPLEENV gs_stEnv;

static MTEXAMPLERUN_CUMULATE gs_stCumu;
static MTEXAMPLERUN_STATUS gs_stStat;

int mtexample_init(TAPPCTX *pstAppCtx, MTEXAMPLEENV *pstEnv)
{
	int iRet;

	memset(pstEnv,0,sizeof(*pstEnv));

	if (NULL == pstAppCtx->stConfData.pszBuff || 0 == pstAppCtx->stConfData.iMeta
		|| NULL == pstAppCtx->pszConfFile )
	{
		assert(0);
		return -1;
	}
	pstEnv->pstConf = (MTEXAMPLECONF *)pstAppCtx->stConfData.pszBuff;

	pstEnv->pstRunCumu = (MTEXAMPLERUN_CUMULATE *)pstAppCtx->stRunDataCumu.pszBuff;
	if (NULL == pstEnv->pstRunCumu)
	{
		pstEnv->pstRunCumu = &gs_stCumu;
	}

	pstEnv->pstRunStat = (MTEXAMPLERUN_STATUS *)pstAppCtx->stRunDataStatus.pszBuff;
	if (NULL == pstEnv->pstRunStat)
	{
		pstEnv->pstRunStat = &gs_stStat;
	}

	if (0 != tapp_get_category(TLOG_DEF_CATEGORY_TEXTROOT, &pstEnv->pstLogCat))
	{
		assert(0);
		printf("tapp_get_category run fail\n");
		return -1;
	}

	printf("example start\n");
	tlog_info(pstEnv->pstLogCat, 0, 0, "example start");


	iRet = mtexample_init_env(pstAppCtx,pstEnv);
	if(iRet)
	{
		assert(0);
		return -1;
	}

	iRet = mtexample_create_thread(pstAppCtx,pstEnv);
	if(iRet)
	{
		assert(0);
		return -1;
	}

	return 0;
}

extern unsigned char g_szMetalib_mtexamplecs[];
extern unsigned char g_szMetalib_mtexampless[];

int mtexample_init_env(TAPPCTX *pstAppCtx, MTEXAMPLEENV *pstEnv)
{
	int iRet;

	pstEnv->pstMetaMsg = tdr_get_meta_by_name((LPTDRMETALIB)g_szMetalib_mtexamplecs,"MTExampleMsg");
	if(!pstEnv->pstMetaMsg)
	{
		printf("Failed to get meta by name %s\n","MTExampleMsg");
		return -1;
	}

	pstEnv->pstMetaSS = tdr_get_meta_by_name((LPTDRMETALIB)g_szMetalib_mtexampless,"mtssdata");
	if(!pstEnv->pstMetaSS)
	{
		printf("Failed to get meta by name %s\n","mtssdata");
		return -1;
	}

	iRet = mtexample_task_init(pstEnv);

	if(iRet)
	{
		return -1;
	}
	return 0;
}

int mtexample_create_thread(TAPPCTX *pstAppCtx, MTEXAMPLEENV *pstEnv)
{
	int i;
	int iRet = 0;
	LPMTEXAMPLETHREADENV pstThreadEnv;

	pstEnv->inThread =  pstAppCtx->iNthreads;

	for(i = 0;i < pstAppCtx->iNthreads;i++)
	{
		pstThreadEnv = &(pstEnv->astThreadEnv[i]);
		memset(pstThreadEnv,0,sizeof(MTEXAMPLETHREADENV));

		pstThreadEnv->pstMetaMsg = pstEnv->pstMetaMsg;
		pstThreadEnv->pstMetaSS = pstEnv->pstMetaSS;

		iRet = tapp_create_thread(pstAppCtx, NULL, pstThreadEnv, &(pstEnv->apstThreadCtx[i]));
		if(iRet)
		{
			assert(0);
			return -1;
		}
	}

	return iRet;
}

int mtexample_version(TAPPCTX *pstAppCtx, MTEXAMPLEENV *pstEnv)
{
	printf("This is the version string to print when start with -v option\n");
	return 0;
}

int mtexample_proc(TAPPCTX *pstAppCtx, MTEXAMPLEENV *pstEnv)
{
	int iBusy=0;
	int iRet;

	iRet = mtexample_proc_thread_res(pstAppCtx,pstEnv);
	if(!iRet)
	{
		iBusy = 1;
	}

	iRet = mtexample_recv_client_req(pstAppCtx,pstEnv);
#if 0
	iRet = 0;
	pstEnv->stReq.stHead.wCmdID = MT_ONLYMAIN_REQ;
	pstEnv->stReq.stBody.stOnlyMainReq.iAdd1 =1;
	pstEnv->stReq.stBody.stOnlyMainReq.iAdd2 =2;
#endif
	if(0 == iRet)
	{
		mtexample_proc_client_req(pstAppCtx,pstEnv);
		//mtexample_backward_to_client(pstAppCtx,pstEnv);
		iBusy = 1;
	}

	//tlog_error(pstEnv->pstLogCat, 0, 0,"AAA");

	if (iBusy)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

int mtexample_reload(TAPPCTX *pstAppCtx, MTEXAMPLEENV *pstEnv)
{
	MTEXAMPLECONF   *pstPreConf;
	pstPreConf = (MTEXAMPLECONF   *)pstAppCtx->stConfPrepareData.pszBuff;
	printf("example reload\n");
	tlog_info(pstEnv->pstLogCat, 0, 0, "example reload");
	return 0;
}

int mtexample_fini(TAPPCTX *pstAppCtx, MTEXAMPLEENV *pstEnv)
{
	printf("example finish\n");
	tlog_info(pstEnv->pstLogCat, 0, 0, "example finish");
	return 0;
}

int mtexample_tick(TAPPCTX *pstAppCtx, MTEXAMPLEENV *pstEnv)
{
	pstEnv->iTick ++;
	mtexample_timeout_proc(pstAppCtx,pstEnv);
	return 0;
}

int mtexample_stop(TAPPCTX *pstAppCtx, MTEXAMPLEENV *pstEnv)
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

int example_proc_binary_req(TAPPCTX *pstAppCtx, void *pstEnv, const char* pszMetaName, const char* pBuf, int iLen)
{
    int iRet = 0;

    iRet = tappctrl_send_msg(pszMetaName, pBuf, iLen);
    if (0 > iRet)
    {
        printf("failed to backward recved msg\n");
    }

    return iRet;
}


int example_thread_init(TAPPTHREADCTX *pstThreadCtx,void *argv)
{
	printf("%s:%d\n",__FUNCTION__,pstThreadCtx->iThreadIdx);
	return 0;
}

int example_thread_reload(TAPPTHREADCTX *pstThreadCtx,void *argv)
{
	sleep(pstThreadCtx->iThreadIdx);
	printf("%s:%d\n",__FUNCTION__,pstThreadCtx->iThreadIdx);
	return 0;
}

int mtexample_thread_proc(TAPPTHREADCTX *pstThreadCtx,void *argv)
{
	//Just recv the data from the main thread. And deal with it.
	int iRet;

	iRet = mtexample_thread_recv_msg(pstThreadCtx,argv);
	if(0 == iRet)
	{
		usleep(1);
		mtexample_thread_process_msg(pstThreadCtx,argv);
		mtexample_thread_backward_msg(pstThreadCtx,argv);
		return 0;
	}

	return -1;
}

int example_thread_fini(TAPPTHREADCTX *pstThreadCtx,void *argv)
{
	printf("%s:%d\n",__FUNCTION__,pstThreadCtx->iThreadIdx);
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

	gs_stAppCtx.pfnInit	=	(PFNTAPPFUNC)mtexample_init;
	gs_stAppCtx.pfnFini	=	(PFNTAPPFUNC)mtexample_fini;
	gs_stAppCtx.pfnProc	=	(PFNTAPPFUNC)mtexample_proc;
	gs_stAppCtx.pfnTick	=	(PFNTAPPFUNC)mtexample_tick;
	gs_stAppCtx.pfnReload	=	(PFNTAPPFUNC)mtexample_reload;
	gs_stAppCtx.pfnStop  =   (PFNTAPPFUNC)mtexample_stop;

    gs_stAppCtx.pfnProcCmdLine = mtexample_proc_cmdline_req;
    gs_stAppCtx.pfnProcCtrlReq = example_proc_binary_req;
    gs_stAppCtx.pfnGetCtrlUsage = mtexample_console_help;

    gs_stAppCtx.pfnControllerInit = mtexample_controller_init;
    gs_stAppCtx.pfnProcCtrlRes = mtexample_proc_binary_res;

	gs_stAppCtx.pfnPreprocCmdLine = mtexample_proc_stdin;
    gs_stAppCtx.pfnControllerFini = example_controller_fini;

	gs_stAppCtx.iLib = (intptr_t)g_szMetalib_example;
	gs_stAppCtx.stConfData.pszMetaName = "mtexampleconf";
	gs_stAppCtx.uiVersion = TAPP_MAKE_VERSION(MAJOR, MINOR, REV, BUILD);

	gs_stAppCtx.stThreadConf.pfnInit = example_thread_init;
	gs_stAppCtx.stThreadConf.pfnProc = mtexample_thread_proc;
	gs_stAppCtx.stThreadConf.pfnFini= example_thread_fini;
	gs_stAppCtx.stThreadConf.pfnReload = example_thread_reload;
	strncpy(gs_stAppCtx.stThreadConf.szThreadName,"Hello",sizeof(gs_stAppCtx.stThreadConf.szThreadName));
	gs_stAppCtx.pfnVersionGenerator = (PFNTAPPFUNC)mtexample_version;

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
