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
#include "TappTlog_conf_desc.h"
#include "version.h"

struct tagTappTlogEnv
{
    TAPPTLOGCONF *pstConf;
    TAPPTLOGRUN_CUMULATE *pstRunCumu;
    TAPPTLOGRUN_STATUS *pstRunStat;
    TLOGCATEGORYINST* pstLogCat;

    TLOGCATEGORYINST *pstBillCat;
};

typedef struct tagTappTlogEnv TAPPTLOGENV;
typedef struct tagTappTlogEnv *LTAPPTLOGENV;

static TAPPCTX gs_stAppCtx;
extern unsigned char g_szMetalib_TappTlog[];

TAPPTLOGENV gs_stEnv;
static TAPPTLOGRUN_CUMULATE gs_stCumu;
static TAPPTLOGRUN_STATUS gs_stStat;

int TappTlog_init(TAPPCTX *pstAppCtx, TAPPTLOGENV *pstEnv)
{
    if (NULL == pstAppCtx->stConfData.pszBuff || 0
            == pstAppCtx->stConfData.iMeta || NULL == pstAppCtx->pszConfFile)
    {
        return -1;
    }
    pstEnv->pstConf = (TAPPTLOGCONF *) pstAppCtx->stConfData.pszBuff;

    pstEnv->pstRunCumu
            = (TAPPTLOGRUN_CUMULATE *) pstAppCtx->stRunDataCumu.pszBuff;
    if (NULL == pstEnv->pstRunCumu)
    {
        pstEnv->pstRunCumu = &gs_stCumu;
    }

    pstEnv->pstRunStat
            = (TAPPTLOGRUN_STATUS *) pstAppCtx->stRunDataStatus.pszBuff;
    if (NULL == pstEnv->pstRunStat)
    {
        pstEnv->pstRunStat = &gs_stStat;
    }

    if (0 > tapp_get_category(TLOG_DEF_CATEGORY_TEXTROOT, (&pstEnv->pstLogCat)))
    {
        printf("tapp_get_category run fail\n");
        return -1;
    }

    if(0 > tapp_get_category("bill",&pstEnv->pstBillCat))
    {
        printf("Failed to get bill cat\n");
        return -1;
    }

    printf("TappTlog start\n");

    tlog_bill(pstEnv->pstBillCat,"Hello");

    tlog_info(pstEnv->pstLogCat, 0, 0, "TappTlog start");
    tlog_info(pstEnv->pstLogCat, 1, 1, "TappTlog start");
    tlog_info(pstEnv->pstLogCat, 2, 2, "TappTlog start");
    tlog_info(pstEnv->pstLogCat, 3, 3, "TappTlog start");
    tlog_info(pstEnv->pstLogCat, 4, 4, "TappTlog start");
    tlog_info(pstEnv->pstLogCat, 5, 5, "TappTlog start");
    tlog_info(pstEnv->pstLogCat, 6, 6, "TappTlog start");
    tlog_info(pstEnv->pstLogCat, 7, 7, "TappTlog start");
    tlog_info(pstEnv->pstLogCat, 8, 8, "TappTlog start");
    tlog_info(pstEnv->pstLogCat, 9, 9, "TappTlog start");
    tlog_info(pstEnv->pstLogCat, 10, 10, "TappTlog start");
    tlog_info(pstEnv->pstLogCat, 11, 11, "TappTlog start");
    tlog_info(pstEnv->pstLogCat, 12,12, "TappTlog start");
    tlog_info(pstEnv->pstLogCat, 13, 13, "TappTlog start");

    tlog_error(pstEnv->pstLogCat, 0, 0, "TappTlog start");
    tlog_error(pstEnv->pstLogCat, 1, 1, "TappTlog start");
    tlog_error(pstEnv->pstLogCat, 2, 2, "TappTlog start");
    tlog_error(pstEnv->pstLogCat, 3, 3, "TappTlog start");
    tlog_error(pstEnv->pstLogCat, 4, 4, "TappTlog start");
    tlog_error(pstEnv->pstLogCat, 5, 5, "TappTlog start");
    tlog_error(pstEnv->pstLogCat, 6, 6, "TappTlog start");
    tlog_error(pstEnv->pstLogCat, 7, 7, "TappTlog start");
    tlog_error(pstEnv->pstLogCat, 8, 8, "TappTlog start");
    tlog_error(pstEnv->pstLogCat, 9, 9, "TappTlog start");
    tlog_error(pstEnv->pstLogCat, 10, 10, "TappTlog start");
    tlog_error(pstEnv->pstLogCat, 11, 11, "TappTlog start");
    tlog_error(pstEnv->pstLogCat, 12,12, "TappTlog start");
    tlog_error(pstEnv->pstLogCat, 13, 13, "TappTlog start");
    return 0;
}

int TappTlog_proc(TAPPCTX *pstAppCtx, TAPPTLOGENV *pstEnv)
{
    int iBusy = 0;
    int i = 10;
    while(i--)
    {
        tlog_bill(pstEnv->pstBillCat,"Hello");
    }

    if (iBusy)
    {
        return 0;
    }
    else
    {
        //sleep(10);
        //tlog_info(pstEnv->pstLogCat, 0, 0, "TappTlog in proc");
        //tapp_exit_mainloop();
        return -1;
    }
}

int TappTlog_reload(TAPPCTX *pstAppCtx, TAPPTLOGENV *pstEnv)
{
    TAPPTLOGCONF *pstPreConf;

    pstPreConf = (TAPPTLOGCONF *) pstAppCtx->stConfPrepareData.pszBuff;

    printf("TappTlog reload\n");
    tlog_info(pstEnv->pstLogCat, 0, 0, "TappTlog reload");

    return 0;
}

int TappTlog_fini(TAPPCTX *pstAppCtx, TAPPTLOGENV *pstEnv)
{

    printf("TappTlog finish\n");
    tlog_info(pstEnv->pstLogCat, 0, 0, "TappTlog finish");
    return 0;
}

int TappTlog_tick(TAPPCTX *pstAppCtx, TAPPTLOGENV *pstEnv)
{
    return 0;
}

int TappTlog_stop(TAPPCTX *pstAppCtx, TAPPTLOGENV *pstEnv)
{
    int iReady = 0;

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
    void* pvArg = &gs_stEnv;

    memset(&gs_stAppCtx, 0, sizeof(gs_stAppCtx));
    memset(&gs_stEnv, 0, sizeof(gs_stEnv));

    gs_stAppCtx.argc = argc;
    gs_stAppCtx.argv = argv;

    gs_stAppCtx.pfnInit = (PFNTAPPFUNC) TappTlog_init;

    gs_stAppCtx.iLib = (intptr_t) g_szMetalib_TappTlog;
    gs_stAppCtx.stConfData.pszMetaName = "TappTlogconf";
    gs_stAppCtx.uiVersion = TAPP_MAKE_VERSION(MAJOR, MINOR, REV, BUILD);

    iRet = tapp_def_init(&gs_stAppCtx, pvArg);
    if (iRet < 0)
    {
        printf("Error: app Initialization failed.\n");
        return iRet;
    }

    gs_stAppCtx.pfnFini = (PFNTAPPFUNC) TappTlog_fini;
    gs_stAppCtx.pfnProc = (PFNTAPPFUNC) TappTlog_proc;
    gs_stAppCtx.pfnTick = (PFNTAPPFUNC) TappTlog_tick;
    gs_stAppCtx.pfnReload = (PFNTAPPFUNC) TappTlog_reload;
    gs_stAppCtx.pfnStop = (PFNTAPPFUNC) TappTlog_stop;

    iRet = tapp_def_mainloop(&gs_stAppCtx, pvArg);

    tapp_def_fini(&gs_stAppCtx, pvArg);

    return iRet;
}

