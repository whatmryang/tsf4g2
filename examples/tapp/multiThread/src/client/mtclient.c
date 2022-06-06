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
#include "mtclient_conf_desc.h"
#include "version.h"

#include "mtexample_cs.h"


struct tagmtclientEnv
{
	MTCLIENTCONF *pstConf;
	MTCLIENTRUN_CUMULATE *pstRunCumu;
	MTCLIENTRUN_STATUS   *pstRunStat;
	TLOGCATEGORYINST* pstLogCat;

	LPTDRMETA pstMetaCS;
};

typedef struct tagmtclientEnv		MTCLIENTENV;
typedef struct tagmtclientEnv		*LMTCLIENTENV;


static TAPPCTX gs_stAppCtx;
extern unsigned char g_szMetalib_mtclient[];

MTCLIENTENV gs_stEnv;
static MTCLIENTRUN_CUMULATE gs_stCumu;
static MTCLIENTRUN_STATUS gs_stStat;

extern unsigned char g_szMetalib_mtexamplecs[];

int mtclient_init(TAPPCTX *pstAppCtx, MTCLIENTENV *pstEnv)
{
	if (NULL == pstAppCtx->stConfData.pszBuff || 0 == pstAppCtx->stConfData.iMeta
		|| NULL == pstAppCtx->pszConfFile )
	{
		return -1;
	}
	pstEnv->pstConf = (MTCLIENTCONF *)pstAppCtx->stConfData.pszBuff;

	pstEnv->pstRunCumu = (MTCLIENTRUN_CUMULATE *)pstAppCtx->stRunDataCumu.pszBuff;
	if (NULL == pstEnv->pstRunCumu)
	{
		pstEnv->pstRunCumu = &gs_stCumu;
	}

	pstEnv->pstRunStat = (MTCLIENTRUN_STATUS *)pstAppCtx->stRunDataStatus.pszBuff;
	if (NULL == pstEnv->pstRunStat)
	{
		pstEnv->pstRunStat = &gs_stStat;
	}

	if (0 > tapp_get_category(TLOG_DEF_CATEGORY_TEXTROOT, (&pstEnv->pstLogCat)))
	{
		printf("tapp_get_category run fail\n");
		return -1;
	}

	printf("mtclient start\n");
	tlog_info(pstEnv->pstLogCat, 0, 0, "mtclient start");

	pstEnv->pstMetaCS = tdr_get_meta_by_name((LPTDRMETALIB)g_szMetalib_mtexamplecs,"MTExampleMsg");

	if(!pstEnv->pstMetaCS)
	{
		return -1;
	}

	return 0;
}

#define TDR_PACK_MSG(stMsg,szBuff,iBuff,stMeta,iRet) \
do{\
	TDRDATA stHost;\
	TDRDATA stNet;\
	stHost.iBuff = sizeof((stMsg));\
	stHost.pszBuff=(char *)&(stMsg);\
	stNet.iBuff = sizeof(szBuff);\
	stNet.pszBuff = szBuff;\
	iRet = tdr_hton((stMeta),&stNet,&stHost,0);\
	iBuff = stNet.iBuff;\
}while(0)

#define TDR_UNPACK_MSG(szBuff,iBuff,stMsg,stMeta,iRet) \
do{\
	TDRDATA stHost;\
	TDRDATA stNet;\
	stHost.iBuff = sizeof((stMsg));\
	stHost.pszBuff=(char *)&(stMsg);\
	stNet.iBuff = sizeof(szBuff);\
	stNet.pszBuff = szBuff;\
	iRet = tdr_ntoh((stMeta),&stHost,&stNet,0);\
	iBuff = stNet.iBuff;\
}while(0)


int mtclient_proc(TAPPCTX *pstAppCtx, MTCLIENTENV *pstEnv)
{
	int iBusy=0;
	int iRet;
	MTEXAMPLEMSG stReq;
	char szBuff[1024];
	size_t iBuff;
	TDRDATA stHost;

	int iSrc = 0;
	int iDst = 0;

	memset(&stReq,0,sizeof(stReq));

	stReq.stHead.wCmdID = MT_ONLYTHREAD_REQ;
	stReq.stBody.stOnlyMainReq.iAdd1 = 1;
	stReq.stBody.stOnlyMainReq.iAdd2 = 2;

	TDR_PACK_MSG(stReq,szBuff,iBuff,pstEnv->pstMetaCS,iRet);

	if(iRet)
	{
		printf("Error Packing msg:%s\n",tdr_error_string(iRet));
	}

	tbus_addr_aton("1.0.0.1",&iDst);

	iRet = tbus_send(pstAppCtx->iBus,&iSrc,&iDst,szBuff,iBuff,0);
	if(iRet)
	{
		printf("Error sending msg:%s\n",tbus_error_string(iRet));
	}


	stReq.stHead.wCmdID = MT_ONLYTHREAD_REQ;

	TDR_PACK_MSG(stReq,szBuff,iBuff,pstEnv->pstMetaCS,iRet);

	if(iRet)
	{
		printf("Error Packing msg:%s\n",tdr_error_string(iRet));
	}

	tbus_addr_aton("1.0.0.1",&iDst);

	iRet = tbus_send(pstAppCtx->iBus,&iSrc,&iDst,szBuff,iBuff,0);
	if(iRet)
	{
		printf("Error sending msg:%s\n",tbus_error_string(iRet));
	}

	while(1)
	{

	iSrc = 0;
	iDst = 0;
	iBuff = sizeof(szBuff);
	iRet = tbus_recv(pstAppCtx->iBus,&iSrc,&iDst,szBuff,&iBuff,0);
	if(iRet)
	{
		printf("Error recving msg:%s\n",tbus_error_string(iRet));
		break;
	}

	TDR_UNPACK_MSG(szBuff, iBuff, stReq, pstEnv->pstMetaCS, iRet);
	if(iRet)
	{
		printf("Error unpacking msg:%s\n",tdr_error_string(iRet));
	}

	stHost.iBuff = sizeof(stReq);
	stHost.pszBuff = (char *)&stReq;

	tdr_fprintf(pstEnv->pstMetaCS,stdout,&stHost,0);

	}

	usleep(10000);
	if (iBusy)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

int mtclient_reload(TAPPCTX *pstAppCtx, MTCLIENTENV *pstEnv)
{
	MTCLIENTCONF   *pstPreConf;

	pstPreConf = (MTCLIENTCONF   *)pstAppCtx->stConfPrepareData.pszBuff;

	printf("mtclient reload\n");
	tlog_info(pstEnv->pstLogCat, 0, 0, "mtclient reload");

	return 0;
}

int mtclient_fini(TAPPCTX *pstAppCtx, MTCLIENTENV *pstEnv)
{

	printf("mtclient finish\n");
	tlog_info(pstEnv->pstLogCat, 0, 0, "mtclient finish");
	return 0;
}

int mtclient_tick(TAPPCTX *pstAppCtx, MTCLIENTENV *pstEnv)
{
	printf("a\n");
	return 0;
}

int mtclient_stop(TAPPCTX *pstAppCtx, MTCLIENTENV *pstEnv)
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

	gs_stAppCtx.pfnInit	=	(PFNTAPPFUNC)mtclient_init;

	gs_stAppCtx.iLib = (intptr_t)g_szMetalib_mtclient;
	gs_stAppCtx.stConfData.pszMetaName = "mtclientconf";
	gs_stAppCtx.uiVersion = TAPP_MAKE_VERSION(MAJOR, MINOR, REV, BUILD);

	iRet	=	tapp_def_init(&gs_stAppCtx, pvArg);
	if( iRet<0 )
	{
		printf("Error: app Initialization failed.\n");
		return iRet;
	}

	gs_stAppCtx.pfnFini	=	(PFNTAPPFUNC)mtclient_fini;
	gs_stAppCtx.pfnProc	=	(PFNTAPPFUNC)mtclient_proc;
	gs_stAppCtx.pfnTick	=	(PFNTAPPFUNC)mtclient_tick;
	gs_stAppCtx.pfnReload	=	(PFNTAPPFUNC)mtclient_reload;
	gs_stAppCtx.pfnStop  =   (PFNTAPPFUNC)mtclient_stop;

	iRet	=	tapp_def_mainloop(&gs_stAppCtx, pvArg);

	tapp_def_fini(&gs_stAppCtx, pvArg);

	return iRet;
}


