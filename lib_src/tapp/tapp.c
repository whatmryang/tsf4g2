/*
 **  @file $RCSfile: tapp.c,v $
 **  general description of this module
 **  $Id: tapp.c,v 1.49 2009-10-15 09:24:28 flyma Exp $
 **  @author $Author: flyma $
 **  @date $Date: 2009-10-15 09:24:28 $
 **  @version $Revision: 1.49 $
 **  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
 **  @note Platform: Linux
 */

#include "tlog/tlog.h"
#include "../tlog/tlog_i.h"
#include "tappctrl_kernel_i.h"
#include "tapp_i.h"
#include "tapp_controller.h"
#include "../lib_src/tloghelp/tlogload_i.h"
#include "comm/comm.h"

#include "tapp/tapp_def.h"

#if defined (_WIN32) || defined (_WIN64)
#else
#include <sys/file.h>
#endif

#if defined (_WIN32) || defined (_WIN64)
#pragma warning(disable:4996)
#endif

TAPPCTX_INTERNAL gs_stCtxInternal;
TAPP_GS gs_stTapp;

extern unsigned char g_szMetalib_tapp_basedata_def[];
extern unsigned char g_szMetalib_tapp_rundata_timer_def[];
extern unsigned char g_szMetalib_tapp_ctrl[];

static int tapp_proc_rundata(TAPPCTX* a_pstCtx, void* a_pvArg);
static int tapp_operate_init(TAPPCTX* a_pstCtx, void* a_pvArg);
static int tapp_lock_pid_file(TAPPCTX* a_pstCtx);
////////////////////////////////////////////////////////////////////////////
void tapp_on_reload()
{
    gs_stTapp.iReload = 1;
}

void tapp_on_sigterm(int a_iSig)
{
    switch (a_iSig)
    {
        default:
            gs_stTapp.iIsExit = TAPP_EXIT_QUIT;
            break;
    }
}

void tapp_on_sigusr1(int a_iSig)
{
    switch (a_iSig)
    {
        default:
            gs_stTapp.iIsExit = TAPP_EXIT_STOP;
            break;
    }
}

void tapp_on_sigusr2(int a_iSig)
{
    switch (a_iSig)
    {
        default:
            gs_stTapp.iReload = 1;
            break;
    }
}

int tapp_def_usage(int argc, char* argv[])
{
    const char* pszApp;

    TOS_UNUSED_ARG(argc);

    pszApp = (void *) basename(argv[0]);

#if !defined (_WIN32) && !defined (_WIN64)
    printf("Common Usage: %s [options] [%s| %s| %s| %s| %s| %s]\n",
           pszApp, TAPP_START, TAPP_STOP, TAPP_RESTART, TAPP_RELOAD, TAPP_KILL, TAPP_CONTROL);
#else
    printf("Common Usage: %s [options] [%s| %s| %s| %s| %s| %s| %s| %s]\n",
           pszApp, TAPP_INSTALL, TAPP_UNINSTALL, TAPP_START, TAPP_STOP,
           TAPP_RESTART, TAPP_RELOAD, TAPP_KILL, TAPP_CONTROL);
#endif
    printf("options:\n");
    printf("\t--id=[name]: specify the identifier of this process.\n");
    printf("\t--timer=[millisec]: specify time period to call pfnTick, default %d ms.\n",
           TAPP_DEFAULT_TIMER);
    printf("\t--wait=[seconds]: specify time period to wait the previous process to exit, default %d s.\n",
           TAPP_DEFAULT_KILL_WAIT);
    printf("\t--epoll-wait=[millisec]: specify epall_wait timeout gap, default %d ms.\n",
           TAPP_DEFAULT_EPOLL_WAIT);
    printf("\t--idle-sleep=[millisec]: specify sleep-time-gap for idle status, default %d ms.\n",
           TAPP_DEFAULT_IDLE_SLEEP);
    printf("\t--idle-count=[int]: specify idle-cycle-count before entering idle status, default %d.\n",
           TAPP_DEFAULT_IDLE_COUNT);
    printf("\t--conf-file=[path]: specify path of config file.\n");
    printf("\t--conf-format=[formatid]: specify format-id [1|2|3] of xml config file, default %d.\n",
           TDR_XML_DATA_FORMAT_LIST_ENTRY_NAME);
    printf("\t--noloadconf: do NOT load configure file.\n");
    printf("\t--pid-file=[path]: specify path of the file for storing the process\'s pid.\n");
    printf("\t--log-file=[path]: specify path of the file for logging.\n");
    printf("\t--log-level=[int]: specify logging level.\n");
    printf("\t--ctrl-sock=[path]: specify file path used to make shm-key for tapp-control.\n");
    printf("\t--no-ctrl: do NOT open tapp-control.\n");
    printf("\t--tdr-file=[path]: specify path of the tdr file for data presentation.\n");
    printf("\t--meta=[name] --conf-meta=[name]: specify name of the config meta defined in tdr file.\n");
    printf("\t--use-tsm: use tsm to allocate and manage memory.\n");
    printf("\t--tsm-index=[int]: specify tsm index-key, default %s.\n", TSM_TDR_C_INDEX_KEY_STR);
    printf("\t--generate: generate config-file-template.\n");
    printf("\t--use-bus: use tbus as communication mechanism.\n");
    printf("\t--bus-key: specify tbus GCIM key, default %s.\n", TBUS_DEFAULT_GCIM_KEY);
    printf("\t--no-bus: do NOT use tbus as communication mechanism.\n");
    printf("\t--refresh-bus-timer: specify the timer to refresh tbus channel configure. default %d seconds .\n",
           TAPP_REFRESH_TBUS_CONFIGRE_DEFAULT_TIMER);
    printf("\t--resume: start as resume mode.\n");
    printf("\t--nostat: do NOT output statistic information into log.\n");


    printf("\t--metabase-key=[key], --mbase-key=[key]: specify the key for metabase.\n");

#ifdef TAPP_TLOG
    printf("\t--tlogconf=[path] : specify the tlog cfg file path.\n");
#endif
    printf("\t--rundata_timer=[path] : specify the tlog cfg file path.\n");
    printf("\t--nthread=[int]: specify number of slaver threads you want to start, default 0.\n");
    printf("\t--thread-init-timeout=[millisec]: specify timeout gap for slaver-thread initiation.\n");
    printf("\t--threadInitTimeOut=[millisec]: deprecated. use --thread-init-timeout instead.\n");
    printf("\t--channel-size=[int]: specify tbus channel size for comm between main and slaver threads.\n");
    printf("\t--threadChannelSize=[int]: deprecated. use --channel-size instead.\n");
    printf("\t--use-shm-channel: use shared-memory when allocating tbus channel for slaver threads.\n");
    printf("\t--useShmChannel: deprecated. use --use-shm-channel instead.\n");

    printf("\t--business-id=[id]: specify business id, default %d.\n",
           TAGENT_DEFAULT_BUSINESS_ID);
    printf("\t--bus-timeout-gap=[seconds]: specify bus channel timeout gap, default %ds.\n",
           TAPP_DEFAULT_BUS_TIMEOUT_GAP);
    printf("\t--bus-beat-gap=[seconds]: specify bus channel time gap to send heartbeat msg, default %ds.\n",
           TAPP_DEFAULT_BUS_HEARTBEAT_GAP);
    printf("\t--bus-check-gap=[seconds]: sepcify bus channel time gap to check peer states, default %ds.\n",
           TAPP_DEFAULT_BUS_CHECKPEER_GAP);
    printf("\t--bus-fresh-time: use gettimeofday to get current time, when tbus api need datetime.\n");

#if defined (_WIN32) || defined (_WIN64)
    printf("\t--daemon, -D: control service registed under WINDOWS.\n");
#else
    printf("\t--daemon, -D: start this process as a daemon.\n");
    printf("\t--no-std-output: when --daemon is specified, surpress output to stdout and stderr.\n");
#endif
    printf("\t--version, -v: output version information. \n");
    printf("\t--help, -h: output this help information. \n");

    return 0;
}

int tapp_set_pid_file(TAPPCTX* a_pstCtx, const char *a_pszFileBase)
{
    char szBusiness[128];

    assert(NULL != a_pstCtx);
    assert(NULL != a_pszFileBase);

    if (!gs_stCtxInternal.pszPidFile)
    {
        szBusiness[0] = '\0';
        if (0 != a_pstCtx->iBusinessID)
        {
            snprintf(szBusiness, sizeof(szBusiness) - 1, "_%d",
                     a_pstCtx->iBusinessID);
            CLEARTAIL(szBusiness);
        }

        if (a_pstCtx->pszId)
        {
            snprintf(gs_stTapp.szPidFile, sizeof(gs_stTapp.szPidFile) - 1,
                     "%s%s_%s%s.pid", TOS_TMP_DIR, a_pszFileBase,
                     a_pstCtx->pszId, szBusiness);
        }
        else
        {
            snprintf(gs_stTapp.szPidFile, sizeof(gs_stTapp.szPidFile) - 1,
                     "%s%s%s.pid", TOS_TMP_DIR, a_pszFileBase, szBusiness);
        }
        CLEARTAIL(gs_stTapp.szPidFile);
        gs_stCtxInternal.pszPidFile = gs_stTapp.szPidFile;
    }

    return 0;
}

int tapp_init_bus(TAPPCTX* a_pstCtx, void* a_pvArg)
{
    int iRet = 0;

    assert(NULL != a_pstCtx);

    TOS_UNUSED_ARG(a_pvArg);

    if (a_pstCtx->iUseBus)
    {
#ifdef TAPP_TBUS
        a_pstCtx->iBus = -1;
        tbus_set_bussid(a_pstCtx->iBusinessID);

        iRet = tbus_init_ex(a_pstCtx->pszGCIMKey, 0);
        if (0 != iRet)
        {
            fprintf(stderr, "%s: Error: tbus_init failed, for %s.\n",
                    a_pstCtx->pszApp, tbus_error_string(iRet));
            return -1;
        }

        iRet = tbus_new(&a_pstCtx->iBus);
        if (0 != iRet)
        {
            fprintf(stderr, "%s: Error: tbus_new failed, for %s.\n",
                    a_pstCtx->pszApp, tbus_error_string(iRet));
            return -1;
        }

        if (NULL != a_pstCtx->pszId)
        {
            iRet = tbus_addr_aton(a_pstCtx->pszId, (TBUSADDR *) &a_pstCtx->iId);
            if (0 != iRet)
            {
                fprintf(stderr, "%s: Error: failed to convert %s to tbus addr, for %s\n",
                        a_pstCtx->pszApp, a_pstCtx->pszId, tbus_error_string(iRet));
                return -1;
            }

            iRet = tbus_bind(a_pstCtx->iBus, a_pstCtx->iId);
            if (0 != iRet)
            {
                a_pstCtx->iBus = -1;
                fprintf(stderr, "%s: Error: can not bind addr %s, for %s.\n",
                        a_pstCtx->pszApp, a_pstCtx->pszId, tbus_error_string(iRet));
                return -1;
            }
        }

        if (!a_pstCtx->iBusUseFreshTime)
        {
            tbus_use_saved_time();
        }

        if (0 != a_pstCtx->iBusTimeOutGap)
        {
            iRet = tbus_set_peer_timeout_gap(a_pstCtx->iBus, TBUS_ADDR_ALL,
                                             a_pstCtx->iBusTimeOutGap);
            if (TBUS_ERR_IS_ERROR(iRet))
            {
                fprintf(stderr, "%s: Error: failed to set tbus channel timeout gap, for %s.\n",
                        a_pstCtx->pszApp, tbus_error_string(iRet));
                return -1;
            }
            iRet = 0;
        }

#endif /* TAPP_TBUS */
    }

    return iRet;
}

int tapp_init_tdr(TAPPCTX* a_pstCtx, void* a_pvArg)
{
    int iRet = 0;

    assert(NULL != a_pstCtx);
    TOS_UNUSED_ARG(a_pvArg);

    tdr_init_for_thread();
#ifdef TAPP_TDR
    if (gs_stCtxInternal.pszTdrFile && !a_pstCtx->iLib)
    {
        LPTDRMETALIB pstLib = NULL;
        iRet = tdr_load_metalib(&pstLib, gs_stCtxInternal.pszTdrFile);
        if (0 > iRet)
        {
            fprintf(stderr, "%s: Error: failed to load metalib by file \'%s\', for %s.\n",
                    a_pstCtx->pszApp, gs_stCtxInternal.pszTdrFile,
                    tdr_error_string(iRet));

            return -1;
        }
        a_pstCtx->iLib = (intptr_t) pstLib;

        if (gs_stCtxInternal.iUseTsm)
        {
            iRet = tsm_tdr_load_meta(gs_stCtxInternal.iTsm,
                                     pstLib, a_pstCtx->iBusinessID);
            if (0 > iRet)
            {
                fprintf(stderr, "%s: Error: failed to load meta into tsm (%s).\n",
                        a_pstCtx->pszApp, gs_stCtxInternal.pszTdrFile);
                return -1;
            }
        }
        gs_stCtxInternal.iNeedFreeLib = 1;
    }

    if (!a_pstCtx->iLib)
    {
        return 0;
    }

    if (!a_pstCtx->stConfData.pszMetaName)
    {
        a_pstCtx->stConfData.pszMetaName
            = (char*) tdr_get_metalib_name((LPTDRMETALIB) a_pstCtx->iLib);
    }

    if (a_pstCtx->stConfData.pszMetaName && !a_pstCtx->stConfData.iMeta)
    {
        a_pstCtx->stConfData.iMeta
            = (intptr_t) tdr_get_meta_by_name((LPTDRMETALIB) a_pstCtx->iLib,
                                              a_pstCtx->stConfData.pszMetaName);
        if (a_pstCtx->stConfData.iMeta)
        {
            a_pstCtx->stConfData.iLen
                = tdr_get_meta_size((LPTDRMETA) a_pstCtx->stConfData.iMeta);
        }
        else if (!a_pstCtx->iNoLoadConf)
        {
            fprintf(stderr, "%s: Error: failed to get meta by name '%s'\n",
                    a_pstCtx->pszApp, a_pstCtx->stConfData.pszMetaName);
            return -1;
        }
    }

    a_pstCtx->stConfPrepareData.pszMetaName
        = gs_stCtxInternal.stConfBackupData.pszMetaName
        = a_pstCtx->stConfData.pszMetaName;

    a_pstCtx->stConfPrepareData.iLen = gs_stCtxInternal.stConfBackupData.iLen
        = a_pstCtx->stConfData.iLen;

    a_pstCtx->stConfPrepareData.iMeta = gs_stCtxInternal.stConfBackupData.iMeta
        = a_pstCtx->stConfData.iMeta;

    if (!a_pstCtx->stRunDataStatus.pszMetaName)
    {
        snprintf(gs_stTapp.szDefRunDataStatusMeta,
                 sizeof(gs_stTapp.szDefRunDataStatusMeta) - 1, "%srun_status",
                 a_pstCtx->pszApp);
        CLEARTAIL(gs_stTapp.szDefRunDataStatusMeta);
        a_pstCtx->stRunDataStatus.pszMetaName
            = gs_stTapp.szDefRunDataStatusMeta;
    }

    if (a_pstCtx->stRunDataStatus.pszMetaName && !a_pstCtx->stRunDataStatus.iMeta)
    {
        a_pstCtx->stRunDataStatus.iMeta
            = (intptr_t) tdr_get_meta_by_name((LPTDRMETALIB) a_pstCtx->iLib,
                                              a_pstCtx->stRunDataStatus.pszMetaName);
        if (a_pstCtx->stRunDataStatus.iMeta)
        {
            a_pstCtx->stRunDataStatus.iLen
                = tdr_get_meta_size((LPTDRMETA) a_pstCtx->stRunDataStatus.iMeta);
        }
    }

    if (!a_pstCtx->stRunDataCumu.pszMetaName)
    {
        snprintf(gs_stTapp.szDefRunDataCumuMeta,
                 sizeof(gs_stTapp.szDefRunDataCumuMeta) - 1, "%srun_cumulate",
                 a_pstCtx->pszApp);
        CLEARTAIL(gs_stTapp.szDefRunDataCumuMeta);
        a_pstCtx->stRunDataCumu.pszMetaName = gs_stTapp.szDefRunDataCumuMeta;
    }

    if (a_pstCtx->stRunDataCumu.pszMetaName && !a_pstCtx->stRunDataCumu.iMeta)
    {
        a_pstCtx->stRunDataCumu.iMeta
            = (intptr_t) tdr_get_meta_by_name((LPTDRMETALIB) a_pstCtx->iLib,
                                              a_pstCtx->stRunDataCumu.pszMetaName);
        if (a_pstCtx->stRunDataCumu.iMeta)
        {
            a_pstCtx->stRunDataCumu.iLen
                = tdr_get_meta_size((LPTDRMETA) a_pstCtx->stRunDataCumu.iMeta);
        }
    }

#endif /* TAPP_TDR */

    return iRet;
}

int tapp_do_generate(TAPPCTX* a_pstCtx, void* a_pvArg)
{
    int iRet = 0;
    FILE* fpConfFile = NULL;

    assert(NULL != a_pstCtx);
    TOS_UNUSED_ARG(a_pvArg);

#ifdef TAPP_TDR_XML
    if (!a_pstCtx->stConfData.iMeta)
    {
        fprintf(stderr, "%s: Error: tdr-file or meta is not valid.\n", a_pstCtx->pszApp);
        return -1;
    }

    if (a_pstCtx->pszConfFile)
    {
        fpConfFile = fopen(a_pstCtx->pszConfFile, "wb");
        if (NULL == fpConfFile)
        {
            fprintf(stderr, "%s: Error: unable to open %s for write\n",
                    a_pstCtx->pszApp, a_pstCtx->pszConfFile);
            return -1;
        }
    }
    else
    {
        fpConfFile = stdout;
    }

    iRet = tdr_gen_xmltmplt((LPTDRMETA) a_pstCtx->stConfData.iMeta, fpConfFile,
                            0, gs_stCtxInternal.stOption.iConfigFileFormat);

    if (a_pstCtx->pszConfFile && NULL != fpConfFile)
    {
        fclose(fpConfFile);
        fpConfFile = NULL;
    }
#endif /* TAPP_TDR_XML */

    return iRet;
}

int tapp_fini_tdr(TAPPCTX* a_pstCtx, void* a_pvArg)
{
    assert(NULL != a_pstCtx);

    TOS_UNUSED_ARG(a_pvArg);

#ifdef TAPP_TDR
    if (a_pstCtx->iLib && gs_stCtxInternal.iNeedFreeLib)
    {
        tdr_free_lib((LPTDRMETALIB*) (void *) &a_pstCtx->iLib);
    }
#endif
    tdr_fini_for_thread();

    return 0;
}

int tapp_init_tsm(TAPPCTX* a_pstCtx, void* a_pvArg)
{
    TsmTdrHandleP pstTsm = NULL;
    int iTsmKey = 0;

    TOS_UNUSED_ARG(a_pstCtx);
    TOS_UNUSED_ARG(a_pvArg);

    gs_stCtxInternal.iTsm = (intptr_t) NULL;

    if (!gs_stCtxInternal.iUseTsm)
    {
        return 0;
    }

    iTsmKey = strtol(a_pstCtx->pszTsmIdxKey, NULL, 0);
    fprintf(stdout, "%s: Info: tsm-index-key %d <0x%x>\n", a_pstCtx->pszApp, iTsmKey, iTsmKey);
    tsm_tdr_base_set(iTsmKey, 0, 0);

    if (tsm_tdr_open(&pstTsm) < 0)
    {
        fprintf(stderr, "%s: Error: failed to init tsm\n", a_pstCtx->pszApp);
        gs_stCtxInternal.iTsm = (intptr_t) NULL;
        return -1;
    }
    gs_stCtxInternal.iTsm = pstTsm;

    return 0;
}

int tapp_fini_tsm(TAPPCTX* a_pstCtx, void* a_pvArg)
{
    TOS_UNUSED_ARG(a_pstCtx);
    TOS_UNUSED_ARG(a_pvArg);

    if (gs_stCtxInternal.iUseTsm && gs_stCtxInternal.iTsm)
    {
        tsm_tdr_close(&gs_stCtxInternal.iTsm);
        gs_stCtxInternal.iTsm = NULL;
    }

    return 0;
}

static int tapp_init_data_by_file(TAPPCTX* a_pstCtx, TAPPDATA* a_pstAppData,
                                  const char* a_pszPath, int a_iXmlDataFormat)
{
    int iRet = 0;

    if (!a_pstAppData->iMeta)
    {
        fprintf(stderr, "%s: Error: no tdr-meta for configuration\n", a_pstCtx->pszApp);
        return -1;
    }

    if (!a_pstAppData->pszBuff || a_pstAppData->iLen <= 0)
    {
        return -1;
    }

#ifdef TAPP_TDR_XML
    {
        TDRDATA stHost;

        stHost.pszBuff = a_pstAppData->pszBuff;
        stHost.iBuff = a_pstAppData->iLen;

        iRet = tdr_input_file((LPTDRMETA) a_pstAppData->iMeta, &stHost,
                              a_pszPath, 0, a_iXmlDataFormat);
        if (0 != iRet)
        {
            fprintf(stderr, "%s: Error: tdr input by file '%s' failed, for %s\n",
                    a_pstCtx->pszApp, a_pszPath, tdr_error_string(iRet));
            tlog_error(gs_stTapp.pstAppCatData, 0, 0,
                       "tdr input by file %s failed, for %s\n",
                       a_pszPath, tdr_error_string(iRet));
        }
    }
#endif

    return iRet;
}

int tapp_tlog_init_default_log_file(TAPPCTX* a_pstCtx, const char* a_pszConfFileBase)
{
    /* 生成日志的输出文件 */
    char szBusiness[128];

    assert(NULL != a_pstCtx);
    assert(NULL != a_pszConfFileBase);

    if (NULL == gs_stCtxInternal.pszLogFile)
    {
        szBusiness[0] = '\0';
        if (0 != a_pstCtx->iBusinessID)
        {
            snprintf(szBusiness, sizeof(szBusiness) - 1, "_%d",
                     a_pstCtx->iBusinessID);
            CLEARTAIL(szBusiness);
        }

        if (a_pstCtx->pszId)
        {
            snprintf(gs_stTapp.szLogFile, sizeof(gs_stTapp.szLogFile) - 1,
                     "%s%s_%s%s", TOS_TMP_DIR, a_pszConfFileBase,
                     a_pstCtx->pszId, szBusiness);
        }
        else
        {
            snprintf(gs_stTapp.szLogFile, sizeof(gs_stTapp.szLogFile) - 1,
                     "%s%s%s", TOS_TMP_DIR, a_pszConfFileBase, szBusiness);
        }
        CLEARTAIL(gs_stTapp.szLogFile);

        gs_stCtxInternal.pszLogFile = gs_stTapp.szLogFile;
    }

    return 0;
}

int tapp_init_log(TAPPCTX* a_pstCtx, const char* a_pszConfFileBase)
{
    int iRet = 0;

    TLOGCONF* pstLogConf;

    assert(NULL != a_pstCtx);
    assert(NULL != a_pszConfFileBase);

    if (NULL == gs_stCtxInternal.stLogConfData.pszBuff)
    {
        return -1;
    }

    pstLogConf = (TLOGCONF*) gs_stCtxInternal.stLogConfData.pszBuff;
    gs_stTapp.pstLogBackCtx = NULL;

    if (0 == gs_stCtxInternal.iUseTsmLogCfg)
    {
        struct stat stStat;

        tapp_tlog_init_default_log_file(a_pstCtx, a_pszConfFileBase);

        /* 生成日志的配置文件 */
        if (NULL == a_pstCtx->pszLogConfFile)
        {
            snprintf(gs_stTapp.szLogConfFile, sizeof(gs_stTapp.szLogConfFile)
                     - 1, "%s_log.xml", a_pszConfFileBase);
            CLEARTAIL(gs_stTapp.szLogConfFile);
            a_pstCtx->pszLogConfFile = gs_stTapp.szLogConfFile;
        }

        if (0 == access(a_pstCtx->pszLogConfFile, R_OK))
        {
            iRet = tapp_init_data_by_file(a_pstCtx, &gs_stCtxInternal.stLogConfData,
                                          a_pstCtx->pszLogConfFile, 0);
            if (0 > iRet)
            {
                fprintf(stderr, "%s: Error: cant load log conf file %s\n",
                        a_pstCtx->pszApp, a_pstCtx->pszLogConfFile);
                return -1;
            }
        }
        else
        {
            TDRDATA stData;

            iRet = tlog_init_cfg_default(pstLogConf,
                                         gs_stCtxInternal.pszLogFile);
            if (0 > iRet)
            {
                return -1;
            }

            if (NULL != gs_stCtxInternal.pszLogLevel)
            {
                pstLogConf->iPriorityLow = atoi(gs_stCtxInternal.pszLogLevel);
            }

            stData.iBuff = gs_stCtxInternal.stLogConfData.iLen;
            stData.pszBuff = gs_stCtxInternal.stLogConfData.pszBuff;
            tdr_output_file((LPTDRMETA) gs_stCtxInternal.stLogConfData.iMeta,
                            a_pstCtx->pszLogConfFile, &stData, 0, 0);
        }

        if (0 == stat(a_pstCtx->pszLogConfFile, &stStat))
        {
            gs_stCtxInternal.tLogConfMod = stStat.st_mtime;
        }
    }
    else
    {
        pstLogConf = (LPTLOGCONF)gs_stCtxInternal.stLogConfData.pszBuff;
        assert(sizeof(*pstLogConf) == gs_stCtxInternal.stLogConfData.iLen);

        iRet = tlog_init(&gs_stTapp.stLogCtx, pstLogConf);
        if (0 != iRet)
        {
            iRet = tlog_init_cfg_default(pstLogConf, gs_stCtxInternal.pszLogFile);
            if (0 != iRet)
            {
                return -1;
            }
        }
    }

    /* 生成日志的备份文件 */
    if (a_pstCtx->pszLogConfFile)
    {
        snprintf(gs_stTapp.szLogConfBakFile, sizeof(gs_stTapp.szLogConfBakFile)
                 - 1, "%s.bak", a_pstCtx->pszLogConfFile);
    }
    else
    {
        snprintf(gs_stTapp.szLogConfBakFile, sizeof(gs_stTapp.szLogConfBakFile)
                 - 1, "%s_log.xml.bak", a_pszConfFileBase);
    }
    CLEARTAIL(gs_stTapp.szLogConfBakFile);

    if (gs_stCtxInternal.pszLogLevel)
    {
        pstLogConf->iPriorityLow = atoi(gs_stCtxInternal.pszLogLevel);
    }

    iRet = tlog_init(&gs_stTapp.stLogCtx, pstLogConf);

    /* Mickey added here. */
    {
        char szAppName[256];
        if (a_pstCtx->pszId)
        {
            snprintf(szAppName, sizeof(szAppName) - 1, "%s", a_pstCtx->pszId);
            tlog_set_host_name(&gs_stTapp.stLogCtx, szAppName);
        }
        tlog_set_module_name(&gs_stTapp.stLogCtx, a_pszConfFileBase);
    }

    if (0 > iRet)
    {
        fprintf(stderr, "%s: Error: failed to init log, for %s, please check log conf file again\n",
                a_pstCtx->pszApp, tlog_error_string(iRet));
        return -1;
    }

    if (a_pstCtx->iUseBus)
    {
        LPTLOGCATEGORYINST pstLogCat = NULL;
        tapp_get_category(TLOG_DEF_CATEGORY_BUS, &pstLogCat);
        tbus_set_logcat(pstLogCat);
    }

    tapp_get_category(TLOG_DEF_CATEGORY_TEXTROOT, &(gs_stTapp.pstAppCatText));
    if (NULL == gs_stTapp.pstAppCatText)
    {
        printf("%s: Warning: tlog category named '%s' is needed by TAPP for operation log"
               ", please configure it in tlog conf-file\n",
               a_pstCtx->pszApp, TLOG_DEF_CATEGORY_TEXTROOT);
    }
    tapp_get_category(TLOG_DEF_CATEGORY_DATAROOT, &(gs_stTapp.pstAppCatData));

    return 0;
}

int tapp_init_rundata_timer(TAPPCTX* a_pstCtx, void* a_pvArg)
{
    int iRet = 0;

    TAPP_RUNDATA_TIMER *pstConf;

    assert(NULL != a_pstCtx);
    TOS_UNUSED_ARG(a_pvArg);

    if (NULL == gs_stCtxInternal.stRunDataTimer.pszBuff)
    {
        return -1;
    }

    pstConf = (TAPP_RUNDATA_TIMER*) gs_stCtxInternal.stRunDataTimer.pszBuff;

    if (0 == gs_stCtxInternal.iUseTsmRuntimerLogCfg)
    {
        struct stat stStat;

        if (NULL == gs_stCtxInternal.pszRundataTimerConf)
        {
            snprintf(gs_stTapp.szRunDataTimerConfFile,
                     sizeof(gs_stTapp.szRunDataTimerConfFile) - 1,
                     "%s_rundata_timer.xml", a_pstCtx->pszApp);
        }
        else
        {
            STRNCPY(gs_stTapp.szRunDataTimerConfFile,
                    gs_stCtxInternal.pszRundataTimerConf,
                    sizeof(gs_stTapp.szRunDataTimerConfFile) - 1);
        }
        CLEARTAIL(gs_stTapp.szRunDataTimerConfFile);

        if (0 == access(gs_stTapp.szRunDataTimerConfFile, R_OK))
        {
            iRet = tapp_init_data_by_file(a_pstCtx, &gs_stCtxInternal.stRunDataTimer,
                                          gs_stTapp.szRunDataTimerConfFile, 0);
            if (0 > iRet)
            {
                fprintf(stderr, "%s: Error: cant load rundata timer conf file %s\n",
                        a_pstCtx->pszApp, gs_stTapp.szRunDataTimerConfFile);
                return -1;
            }
        }
        else
        {
            TDRDATA stData;

            memset(pstConf, 0, sizeof(*pstConf));
            pstConf->stBasedata_timer.lGlobalTime = TAPP_BASEDATA_TIMER;
            pstConf->stRundata_cumu_timer.lGlobalTime = TAPP_RUNDATA_CUMU_TIMER;
            pstConf->stRundata_status_timer.lGlobalTime
                = TAPP_RUNDATA_STATUS_TIMER;

            stData.iBuff = gs_stCtxInternal.stRunDataTimer.iLen;
            stData.pszBuff = gs_stCtxInternal.stRunDataTimer.pszBuff;
            tdr_output_file((LPTDRMETA) gs_stCtxInternal.stRunDataTimer.iMeta,
                            gs_stTapp.szRunDataTimerConfFile, &stData, 0, 0);
        }

        if (0 == stat(gs_stTapp.szRunDataTimerConfFile, &stStat))
        {
            gs_stCtxInternal.tRunTimerConfMod = stStat.st_mtime;
        }
    }
    else
    {
        printf("%s: Warning: use tsm runtimer log is not supported now\n", a_pstCtx->pszApp);
    }

    return 0;
}

int tapp_fini_log(TAPPCTX* a_pstCtx, void* a_pvArg)
{
    int iRet = 0;

    TOS_UNUSED_ARG(a_pstCtx);
    TOS_UNUSED_ARG(a_pvArg);
#ifdef TAPP_TLOG

    iRet = tlog_fini(&gs_stTapp.stLogCtx);

#endif /* TAPP_TLOG */

    return iRet;
}

static int tapp_alloc_one_data(TAPPDATA* a_pstAppData)
{
    assert(NULL != a_pstAppData);

    if (NULL != a_pstAppData->pszBuff || 0 == a_pstAppData->iLen)
    {
        return 0;
    }

    a_pstAppData->pszBuff = (char*) calloc(1, a_pstAppData->iLen);

    return a_pstAppData->pszBuff ? 0 : -1;
}

int tapp_reload_log(TAPPCTX* a_pstCtx, void* a_pvArg)
{
    int iRet = 0;

    TDRDATA stHost;
    TLOGCONF *pstLogConfPre;
    TLOGCONF *pstLogConf;

    TOS_UNUSED_ARG(a_pvArg);

    if (NULL == gs_stCtxInternal.stLogConfData.pszBuff
        || NULL == gs_stCtxInternal.stLogConfBackupData.pszBuff
        || NULL == gs_stCtxInternal.stLogConfPrepareData.pszBuff)
    {
        return -1;
    }

    memcpy(gs_stCtxInternal.stLogConfBackupData.pszBuff,
           gs_stCtxInternal.stLogConfData.pszBuff,
           gs_stCtxInternal.stLogConfBackupData.iLen);

    if (0 == gs_stCtxInternal.iUseTsmLogCfg)
    {
        struct stat stStat;
        if (0 == stat(a_pstCtx->pszLogConfFile, &stStat))
        {
            if (gs_stCtxInternal.tLogConfMod == stStat.st_mtime)
            {
                return 0;
            }
            gs_stCtxInternal.tLogConfMod = stStat.st_mtime;
        }

        iRet = tapp_init_data_by_file(a_pstCtx, &gs_stCtxInternal.stLogConfPrepareData,
                                      a_pstCtx->pszLogConfFile, 0);
        if (0 > iRet)
        {
            return -1;
        }
    }

    stHost.pszBuff = (char*) gs_stCtxInternal.stLogConfBackupData.pszBuff;
    stHost.iBuff = (int) gs_stCtxInternal.stLogConfBackupData.iLen;
    tdr_output_file((LPTDRMETA) gs_stCtxInternal.stLogConfBackupData.iMeta,
                    gs_stTapp.szLogConfBakFile, &stHost, 0, 0);

    pstLogConf = (TLOGCONF *) gs_stCtxInternal.stLogConfData.pszBuff;
    pstLogConfPre = (TLOGCONF *) gs_stCtxInternal.stLogConfPrepareData.pszBuff;
    memcpy(gs_stCtxInternal.stLogConfData.pszBuff,
           gs_stCtxInternal.stLogConfPrepareData.pszBuff,
           gs_stCtxInternal.stLogConfData.iLen);

    tlog_reload(&(gs_stTapp.stLogCtx), pstLogConfPre);

    printf("---------reload log conf-----------\n");
    printf("old log conf in file %s, new log conf:\n",
           gs_stTapp.szLogConfBakFile);
    stHost.pszBuff = (char*) gs_stCtxInternal.stLogConfData.pszBuff;
    stHost.iBuff = (int) gs_stCtxInternal.stLogConfData.iLen;
    tdr_output_fp((LPTDRMETA) gs_stCtxInternal.stLogConfData.iMeta, stdout,
                  &stHost, 0, 0);
    printf("---------reload log conf end--------\n");

    tlog_info(gs_stTapp.pstAppCatData, 0, 0, "reload log conf successfully\n");

    if (a_pstCtx->iUseBus)
    {
        tbus_set_logcat(tlog_get_category(&gs_stTapp.stLogCtx,
                                          TLOG_DEF_CATEGORY_BUS));
    }

    return iRet;
}

static int tapp_tsm_alloc_one_data(TAPPCTX* a_pstCtx, TAPPDATA* a_pstAppData,
                                   const char* a_pszDomain)
{
    TsmUri     stUri;
    TsmTdrRowP pstRow;
    TsmObjPtrP pstPtr;
    char       szMetaUri[URI_MAX_LEN + 1];
    char       szDataUri[URI_MAX_LEN + 1];
    LPTDRMETA  pstMeta;
    int        iRet;

    if (!gs_stCtxInternal.iUseTsm)
    {
        return -1;
    }

    iRet = tsm_tdr_load_meta(gs_stCtxInternal.iTsm,
                             (LPTDRMETALIB)a_pstAppData->iLib,
                             a_pstCtx->iBusinessID);
    if (0 > iRet)
    {
        return -1;
    }

    pstMeta = tdr_get_meta_by_name((LPTDRMETALIB)a_pstAppData->iLib,
                                   a_pstAppData->pszMetaName);
    if (!pstMeta)
    {
        fprintf(stderr, "%s: Error: failed to get meta by name '%s'\n",
                a_pstCtx->pszApp, a_pstAppData->pszMetaName);
        return -1;
    }

    snprintf(szMetaUri, URI_MAX_LEN,
             "tsm-meta://%d/%s:%d:%d/%s:%d",
             a_pstCtx->iBusinessID,
             tdr_get_metalib_name((LPTDRMETALIB)a_pstAppData->iLib),
             tdr_get_metalib_version((LPTDRMETALIB)a_pstAppData->iLib),
             tdr_get_metalib_build_version((LPTDRMETALIB)a_pstAppData->iLib),
             a_pstAppData->pszMetaName,
             tdr_get_meta_current_version(pstMeta));
    CLEARTAIL(szMetaUri);
    snprintf(szDataUri, URI_MAX_LEN,
             "tsm-data://%d/%s:%s/%s/default",
             a_pstCtx->iBusinessID,
             a_pstCtx->pszApp,
             NULL == a_pstCtx->pszId ? "" : a_pstCtx->pszId,
             a_pszDomain);
    CLEARTAIL(szDataUri);
    if (tsm_uri_set(&stUri, szDataUri))
    {
        return -1;
    }
    tsm_uri_std_i(&stUri, NULL);

    tsm_tdr_query(gs_stCtxInternal.iTsm, tsm_uri_get(&stUri));
    if (NULL == (pstRow = tsm_tdr_row_only(gs_stCtxInternal.iTsm)))
    {
        fprintf(stdout, "%s: Info: tsm meta(%s) (%s)\n",
                a_pstCtx->pszApp, a_pstAppData->pszMetaName, szDataUri);
        if (tsm_tdr_bind_data(gs_stCtxInternal.iTsm, szDataUri))
        {
            return -1;
        }

        if (tsm_tdr_bind_meta(gs_stCtxInternal.iTsm, szMetaUri))
        {
            return -1;
        }

        if (tsm_tdr_load_data(gs_stCtxInternal.iTsm, NULL, O_CREAT | O_EXCL))
        {
            return -1;
        }

        tsm_tdr_query(gs_stCtxInternal.iTsm, tsm_uri_get(&stUri));
        if (NULL == (pstRow = tsm_tdr_row_only(gs_stCtxInternal.iTsm)))
        {
            return -1;
        }
    }

    if (NULL == (pstPtr = tsm_tdr_row_data_ptr(pstRow)))
    {
        return -1;
    }

    a_pstAppData->iMeta = (intptr_t)pstMeta;
    a_pstAppData->pszBuff = tsm_tdr_at(pstPtr, 0);
    if (!a_pstAppData->pszBuff)
    {
        terror_dump(NULL);
        return -1;
    }
    a_pstAppData->iLen = tsm_tdr_row_size(pstRow);

    if (NULL == (pstPtr = tsm_tdr_row_meta_ptr(pstRow)))
    {
        return -1;
    }

    if (NULL == (a_pstAppData->iLib = tsm_tdr_at(pstPtr, 1)))
    {
        return -1;
    }

    a_pstAppData->iMeta = (intptr_t)tdr_get_meta_by_name(a_pstAppData->iLib,
                                                         a_pstAppData->pszMetaName);

    return 0;
}

static int tapp_init_log_data(TAPPCTX* a_pstCtx, void* a_pvArg)
{
    int iRet = 0;
    TOS_UNUSED_ARG(a_pvArg);
    assert(NULL != a_pstCtx);

    gs_stCtxInternal.stLogConfData.iLen = (int) sizeof(TLOGCONF);
    gs_stCtxInternal.stLogConfData.pszMetaName = "TLOGConf";
    gs_stCtxInternal.stLogConfData.iLib = (void *) tlog_get_meta_data();

    gs_stCtxInternal.stLogConfData.iMeta
        = (intptr_t) tdr_get_meta_by_name((LPTDRMETALIB) (tlog_get_meta_data()),
                                          gs_stCtxInternal.stLogConfData.pszMetaName);

    gs_stCtxInternal.stLogConfPrepareData.iMeta
        = gs_stCtxInternal.stLogConfBackupData.iMeta
        = gs_stCtxInternal.stLogConfData.iMeta;

    gs_stCtxInternal.stLogConfPrepareData.pszMetaName
        = gs_stCtxInternal.stLogConfBackupData.pszMetaName
        = gs_stCtxInternal.stLogConfData.pszMetaName;
    gs_stCtxInternal.stLogConfPrepareData.iLib
        = gs_stCtxInternal.stLogConfBackupData.iLib
        = gs_stCtxInternal.stLogConfData.iLib;

    gs_stCtxInternal.stLogConfPrepareData.iLen
        = gs_stCtxInternal.stLogConfBackupData.iLen
        = gs_stCtxInternal.stLogConfData.iLen;

    if (gs_stCtxInternal.iUseTsm)
    {
        iRet = tsm_tdr_load_meta(gs_stCtxInternal.iTsm,
                                 (LPTDRMETALIB)tlog_get_meta_data(),
                                 a_pstCtx->iBusinessID);
        if (0 > iRet)
        {
            terror_dump(NULL);
            fprintf(stderr, "Failed to load tlog meta data");
            return -1;
        }

        iRet = tapp_tsm_alloc_one_data(a_pstCtx,
                                       &gs_stCtxInternal.stLogConfData,
                                       TAPP_DOMAIN_LOGCFG);
        if (0 > iRet)
        {
            printf("%s: Warning: there is no %s domain in tsm\n",
                   a_pstCtx->pszApp, TAPP_DOMAIN_LOGCFG);
        }

        iRet = tapp_tsm_alloc_one_data(a_pstCtx,
                                       &gs_stCtxInternal.stLogConfPrepareData,
                                       TAPP_DOMAIN_LOGCFGPRE);
        if (0 > iRet)
        {
            printf("%s: Warning: there is no %s domain in tsm\n",
                   a_pstCtx->pszApp, TAPP_DOMAIN_LOGCFGPRE);
        }

        iRet = tapp_tsm_alloc_one_data(a_pstCtx,
                                       &gs_stCtxInternal.stLogConfBackupData,
                                       TAPP_DOMAIN_LOGCFGBAK);
        if (0 > iRet)
        {
            printf("%s: Warning: there is no %s domain in tsm\n",
                   a_pstCtx->pszApp, TAPP_DOMAIN_LOGCFGBAK);
        }
    }
    else
    {
        if (0 <= iRet)
            iRet = tapp_alloc_one_data(&gs_stCtxInternal.stLogConfData);
        if (0 <= iRet)
            iRet = tapp_alloc_one_data(&gs_stCtxInternal.stLogConfPrepareData);
        if (0 <= iRet)
            iRet = tapp_alloc_one_data(&gs_stCtxInternal.stLogConfBackupData);
        if (0 > iRet)
        {
            fprintf(stderr, "%s: Error: tapp_alloc_one_data for log system failed\n", a_pstCtx->pszApp);
            return iRet;
        }
    } /*if( gs_stCtxInternal.iUseTsm )*/

    return iRet;
}

int tapp_init_data(TAPPCTX* a_pstCtx, void* a_pvArg)
{
    int iRet = 0;

    assert(NULL != a_pstCtx);

    gs_stCtxInternal.stBaseData.pszMetaName = "tapp_basedata";
    gs_stCtxInternal.stRunDataTimer.pszMetaName = "tapp_rundata_timer";

    if (gs_stCtxInternal.iUseTsm)
    {
        gs_stCtxInternal.stBaseData.iLib \
            = (void *)(&g_szMetalib_tapp_basedata_def[0]);
        iRet = tapp_tsm_alloc_one_data(a_pstCtx,
                                       &gs_stCtxInternal.stBaseData,
                                       TAPP_DOMAIN_BASE);
        if (0 > iRet)
        {
            printf("%s: Warning: there is no %s domain in tsm\n",
                   a_pstCtx->pszApp, TAPP_DOMAIN_BASE);
        }
        else
        {
            memset(gs_stCtxInternal.stBaseData.pszBuff, 0,
                   gs_stCtxInternal.stBaseData.iLen);
        }

        a_pstCtx->stConfData.iLib = (void *)a_pstCtx->iLib;
        iRet = tapp_tsm_alloc_one_data(a_pstCtx,
                                       &a_pstCtx->stConfData,
                                       TAPP_DOMAIN_CFG);
        if (0 > iRet)
        {
            printf("%s: Warning: there is no %s domain in tsm\n",
                   a_pstCtx->pszApp, TAPP_DOMAIN_CFG);
        }

        a_pstCtx->stConfPrepareData.iLib= (void *)a_pstCtx->iLib;
        iRet = tapp_tsm_alloc_one_data(a_pstCtx,
                                       &a_pstCtx->stConfPrepareData,
                                       TAPP_DOMAIN_CFGPRE);
        if (0 > iRet)
        {
            printf("%s: Warning: there is no %s domain in tsm\n",
                   a_pstCtx->pszApp, TAPP_DOMAIN_CFGPRE);
        }

        gs_stCtxInternal.stConfBackupData .iLib= (void *)a_pstCtx->iLib;
        iRet = tapp_tsm_alloc_one_data(a_pstCtx,
                                       &gs_stCtxInternal.stConfBackupData,
                                       TAPP_DOMAIN_CFGBAK);
        if (0 > iRet)
        {
            printf("%s: Warning: there is no %s domain in tsm\n",
                   a_pstCtx->pszApp, TAPP_DOMAIN_CFGBAK);
        }

        a_pstCtx->stRunDataStatus.iLib = (void *)a_pstCtx->iLib;
        iRet = tapp_tsm_alloc_one_data(a_pstCtx,
                                       &a_pstCtx->stRunDataStatus,
                                       TAPP_DOMAIN_RT_STATUS);
        if (0 > iRet)
        {
            printf("%s: Warning: there is no %s domain in tsm\n",
                   a_pstCtx->pszApp, TAPP_DOMAIN_RT_STATUS);
        }
        else
        {
            memset(a_pstCtx->stRunDataStatus.pszBuff, 0,
                   a_pstCtx->stRunDataStatus.iLen);
        }

        a_pstCtx->stRunDataCumu.iLib  =(void *) a_pstCtx->iLib;
        iRet = tapp_tsm_alloc_one_data(a_pstCtx,
                                       &a_pstCtx->stRunDataCumu,
                                       TAPP_DOMAIN_RT_CUMU);
        if (0 > iRet)
        {
            printf("%s: Warning: there is no %s domain in tsm\n",
                   a_pstCtx->pszApp, TAPP_DOMAIN_RT_CUMU);
        }
        else
        {
            memset(a_pstCtx->stRunDataCumu.pszBuff, 0,
                   a_pstCtx->stRunDataCumu.iLen);
        }

        gs_stCtxInternal.stRunDataTimer.iLib \
            = (void *)(&g_szMetalib_tapp_rundata_timer_def[0]);
        iRet = tapp_tsm_alloc_one_data(a_pstCtx,
                                       &gs_stCtxInternal.stRunDataTimer,
                                       TAPP_DOMAIN_RT_TIMER);
        if (0 > iRet)
        {
            printf("%s: Warning: there is no %s domain in tsm\n",
                   a_pstCtx->pszApp, TAPP_DOMAIN_RT_TIMER);
        }
    }
    else
    {
        gs_stCtxInternal.stBaseData.iLen = sizeof(TAPP_BASEDATA);
        gs_stCtxInternal.stBaseData.pszMetaName = "tapp_basedata";
        gs_stCtxInternal.stBaseData.iMeta
            = (intptr_t) tdr_get_meta_by_name(
                                              (LPTDRMETALIB) (void *)(&g_szMetalib_tapp_basedata_def[0]),
                                              gs_stCtxInternal.stBaseData.pszMetaName);

        gs_stCtxInternal.stRunDataTimer.iLen = sizeof(TAPP_RUNDATA_TIMER);
        gs_stCtxInternal.stRunDataTimer.pszMetaName = "tapp_rundata_timer";
        gs_stCtxInternal.stRunDataTimer.iMeta
            = (intptr_t) tdr_get_meta_by_name(
                                              (LPTDRMETALIB) (void *) (&g_szMetalib_tapp_rundata_timer_def[0]),
                                              gs_stCtxInternal.stRunDataTimer.pszMetaName);

        gs_stCtxInternal.stLogConfPrepareData.iLen
            = gs_stCtxInternal.stLogConfBackupData.iLen
            = gs_stCtxInternal.stLogConfData.iLen;
        gs_stCtxInternal.stLogConfPrepareData.pszMetaName
            = gs_stCtxInternal.stLogConfBackupData.pszMetaName
            = gs_stCtxInternal.stLogConfData.pszMetaName;
        gs_stCtxInternal.stLogConfPrepareData.iMeta
            = gs_stCtxInternal.stLogConfBackupData.iMeta
            = gs_stCtxInternal.stLogConfData.iMeta;

        iRet = tapp_alloc_one_data(&gs_stCtxInternal.stBaseData);
        if (0 == iRet)
        {
            TAPP_BASEDATA *pstBaseData =
                (TAPP_BASEDATA *) gs_stCtxInternal.stBaseData.pszBuff;
            pstBaseData->fProcMinTime = 100000.0;
        }

        if (0 <= iRet)
            iRet = tapp_alloc_one_data(&a_pstCtx->stConfData);
        if (0 <= iRet)
            iRet = tapp_alloc_one_data(&a_pstCtx->stConfPrepareData);
        if (0 <= iRet)
            iRet = tapp_alloc_one_data(&gs_stCtxInternal.stConfBackupData);
        if (0 <= iRet)
            iRet = tapp_alloc_one_data(&a_pstCtx->stRunDataStatus);
        if (0 <= iRet)
            iRet = tapp_alloc_one_data(&a_pstCtx->stRunDataCumu);
        if (0 <= iRet)
            iRet = tapp_alloc_one_data(&gs_stCtxInternal.stRunDataTimer);
        if (0 > iRet)
        {
            fprintf(stderr, "%s: Error: tapp_alloc_one_data() failed\n", a_pstCtx->pszApp);
            return iRet;
        }


    }/*if( gs_stCtxInternal.iUseTsm )*/


    if (!a_pstCtx->iNoLoadConf)
    {
        iRet = tapp_init_data_by_file(a_pstCtx, &a_pstCtx->stConfData, a_pstCtx->pszConfFile,
                                      gs_stCtxInternal.stOption.iConfigFileFormat);
        if (0 > iRet)
        {
            fprintf(stderr, "%s: Error: can't load app conf file %s\n",
                    a_pstCtx->pszApp, a_pstCtx->pszConfFile);
        }
        else
        {
            struct stat stStat;

            if (0 == stat(a_pstCtx->pszConfFile, &stStat))
            {
                gs_stCtxInternal.tConfMod = stStat.st_mtime;
            }
        }
    }


    if (0 == iRet)
    {
        iRet = tapp_init_log_data(a_pstCtx, a_pvArg);
    }

    return iRet;
}

static int tapp_fini_one_data(TAPPDATA* a_pstAppData)
{
    assert(NULL != a_pstAppData);

    if (gs_stCtxInternal.iUseTsm)
    {
        if (a_pstAppData->iLib)
        {
            tsm_tdr_dt(a_pstAppData->iLib);
            a_pstAppData->iLib = NULL;
        }
        if (a_pstAppData->pszBuff)
        {
            tsm_tdr_dt((void*)a_pstAppData->pszBuff);
            a_pstAppData->pszBuff = (intptr_t)NULL;
        }
    }
    else if (a_pstAppData->pszBuff)
    {
        free(a_pstAppData->pszBuff);
        a_pstAppData->pszBuff = NULL;
        a_pstAppData->iLen = 0;
    }

    return 0;
}

int tapp_fini_data(TAPPCTX* a_pstCtx, void* a_pvArg)
{
    assert(NULL != a_pstCtx);
    TOS_UNUSED_ARG(a_pvArg);

    {
        tapp_fini_one_data(&gs_stCtxInternal.stBaseData);
        tapp_fini_one_data(&a_pstCtx->stConfData);
        tapp_fini_one_data(&a_pstCtx->stConfPrepareData);
        tapp_fini_one_data(&gs_stCtxInternal.stConfBackupData);
        tapp_fini_one_data(&a_pstCtx->stRunDataStatus);
        tapp_fini_one_data(&a_pstCtx->stRunDataCumu);
        tapp_fini_one_data(&gs_stCtxInternal.stLogConfData);
        tapp_fini_one_data(&gs_stCtxInternal.stLogConfPrepareData);
        tapp_fini_one_data(&gs_stCtxInternal.stLogConfBackupData);
    }

    return 0;
}

int tapp_def_opt(TAPPCTX* a_pstCtx, void* a_pvArg)
{
    const char TAPP_SUFFIX_XML[] = ".xml";
    int iRet;
    int opt;
    int iOldOptErr;
    char* pszApp;
    char* pszDot;
    int iLen;
    int iOptIdx = 0;
    static int s_iOptChar = 0;
    int iOnlyGerateConfTemple = 0;

    static struct option s_astLongOptions[] =
    {
        { "thread-init-timeout", 1, &s_iOptChar, 'a' },
        { "threadInitTimeOut", 1, &s_iOptChar, 'a' },
        { "use-shm-channel", 0, &s_iOptChar, 'A' },
        { "useShmChannel", 0, &s_iOptChar, 'A' },
        { "use-bus", 0, &s_iOptChar, 'b' },
        { "conf-file", 1, &s_iOptChar, 'C' },
        { "idle-count", 1, &s_iOptChar, 'c' },
        { "daemon", 0, 0, 'D' },
        { "channel-size", 1, &s_iOptChar, 'd' },
        { "threadChannelSize", 1, &s_iOptChar, 'd' },
        { "epoll-wait", 1, &s_iOptChar, 'e' },
        { "resetshm", 0, 0, 'E' },
        { "refresh-bus-timer", 1, &s_iOptChar, 'f' },
        { "tlogconf", 1, &s_iOptChar, 'G' },
        { "generate", 0, &s_iOptChar, 'g' },
        { "help", 0, 0, 'h' },
        { "id", 1, &s_iOptChar, 'i' },
        { "tsm-index", 1, &s_iOptChar, 'I' },
        { "metabase-key", 1, &s_iOptChar, 'k' },
        { "mbase-key", 1, &s_iOptChar, 'k' },
        { "log-file", 1, &s_iOptChar, 'L' },
        { "log-level", 1, &s_iOptChar, 'l' },
        { "meta", 1, &s_iOptChar, 'm' },
        { "conf-meta", 1, &s_iOptChar, 'm' },
        { "use-tsm", 0, &s_iOptChar, 'M' },

        /* comment begin: 下列命令行参数先对用户隐藏，暂不公开到 --help 信息中*/
        { "use-tsm-cfg", 0, &s_iOptChar, TAPP_USE_TSM_CONF },
        { "use-tsm-logcfg", 0, &s_iOptChar, TAPP_USE_TSM_LOG_CONF },
        { "use-tsm-runtimelogcfg", 0, &s_iOptChar, TAPP_USE_TSM_RUNTIMELOG_CONF },
        /* comment end */

        { "noloadconf", 0, &s_iOptChar, 'n' },
        { "no-tbus", 0, &s_iOptChar, 'N' },
        { "pid-file", 1, &s_iOptChar, 'P' },
        { "rundata_timer", 1, &s_iOptChar, 'r' },
        { "nthread", 1, &s_iOptChar, 'R' },
        { "idle-sleep", 1, &s_iOptChar, 's' },
        { "tdr-file", 1, &s_iOptChar, 'T' },
        { "timer", 1, &s_iOptChar, 't' },
        { "business-id", 1, &s_iOptChar, 'u' },
        { "version", 0, 0, 'v' },
        { "wait", 1, &s_iOptChar, 'w' },
        { "conf-format", 1, &s_iOptChar, 'x' },
        { "bus-key", 1, &s_iOptChar, 'y' },
        { "bill-file-length", 1, &s_iOptChar, 'Y' },
        { "bill-precision", 1, &s_iOptChar, 'Z' },
        { "bill-file", 1, &s_iOptChar, 'z' },
        { "ctrl-sock", 1, &s_iOptChar, TAPP_CTRL_ADDR },
        { "no-ctrl", 0, &s_iOptChar, TAPP_CTRL_ON_OR_OFF },
        { "resume", 0, &s_iOptChar, TAPP_START_MODE_OPT },
        { "nostat", 0, &s_iOptChar, TAPP_OUTPUT_STAT_OPT },
        { "bus-timeout-gap", 1, &s_iOptChar, TAPP_BUS_TIMEOUT_GAP },
        { "bus-beat-gap", 1, &s_iOptChar, TAPP_BUS_HEARTBEAT_GAP },
        { "bus-check-gap", 1, &s_iOptChar, TAPP_BUS_CHECK_PEER_GAP },
        { "bus-fresh-time", 0, &s_iOptChar, TAPP_BUS_USE_SYS_TIME },
        { "no-std-output", 0, &s_iOptChar, TAPP_NO_STD_OUTPUT_OPT },

        { 0, 0, 0, 0 } };

        gs_stCtxInternal.iCtrlOn = 1;

        assert(a_pstCtx);

        pszApp = (char *) basename(a_pstCtx->argv[0]);
        a_pstCtx->pszApp = pszApp;

        pszDot = strrchr(pszApp, TOS_DIRDOT);
        if (pszDot)
        {
            iLen = pszDot - pszApp;
        }
        else
        {
            iLen = strlen(pszApp);
        }

        if (iLen + (int) sizeof(TAPP_SUFFIX_XML)
            > (int) sizeof(gs_stTapp.szConfFile))
        {
            iLen = (int) sizeof(gs_stTapp.szConfFile)
                - (int) sizeof(TAPP_SUFFIX_XML);
        }

        memcpy(gs_stTapp.szConfFile, pszApp, iLen);
        strcpy(gs_stTapp.szConfFile + iLen, TAPP_SUFFIX_XML);

        a_pstCtx->iBusTimeOutGap = TAPP_DEFAULT_BUS_TIMEOUT_GAP;
        a_pstCtx->iBusHeartBeatGap = TAPP_DEFAULT_BUS_HEARTBEAT_GAP;
        a_pstCtx->iBusCheckPeerGap = TAPP_DEFAULT_BUS_CHECKPEER_GAP;

        a_pstCtx->pszConfFile = gs_stTapp.szConfFile;

        gs_stCtxInternal.stOption.iConfigFileFormat
            = TDR_XML_DATA_FORMAT_LIST_ENTRY_NAME;

        if (0 == gs_stCtxInternal.iWait)
            gs_stCtxInternal.iWait = TAPP_DEFAULT_KILL_WAIT;

        if (0 == a_pstCtx->iEpollWait)
            a_pstCtx->iEpollWait = TAPP_DEFAULT_EPOLL_WAIT;

        if (0 == a_pstCtx->iIdleCount)
            a_pstCtx->iIdleCount = TAPP_DEFAULT_IDLE_COUNT;

        if (0 == a_pstCtx->iIdleSleep)
            a_pstCtx->iIdleSleep = TAPP_DEFAULT_IDLE_SLEEP;

        if (0 == a_pstCtx->iTimer)
            a_pstCtx->iTimer = TAPP_DEFAULT_TIMER;

        if (0 == a_pstCtx->uiVersion)
            a_pstCtx->uiVersion = TAPP_DEF_VER;

        if (NULL == a_pstCtx->pszMBaseKey)
            a_pstCtx->pszMBaseKey = TAPP_DEFAULT_METABASE_KEY;

        if (NULL == a_pstCtx->pszTsmIdxKey)
            a_pstCtx->pszTsmIdxKey = TSM_TDR_C_INDEX_KEY_STR;

        if (0 == a_pstCtx->pszGCIMKey)
            a_pstCtx->pszGCIMKey = TBUS_DEFAULT_GCIM_KEY;

        if (0 >= gs_stCtxInternal.stOption.iRefeshTbusTimer)
            gs_stCtxInternal.stOption.iRefeshTbusTimer
                = TAPP_REFRESH_TBUS_CONFIGRE_DEFAULT_TIMER;

        gs_stCtxInternal.iResetShm = 0;

        iOldOptErr = opterr;
        opterr = 1;
        while (-1 != (opt = getopt_long(a_pstCtx->argc, a_pstCtx->argv, "Dvh",
                                        s_astLongOptions, &iOptIdx)))
        {
            switch (opt)
            {
                case '\0':
                    /* Long option got. flag is just an index, no othre meanings */
                    switch (*s_astLongOptions[iOptIdx].flag)
                    {
                        case 'a':
                            gs_stCtxInternal.iThreadWaitInitTimeOut = strtol(optarg, NULL, 0);
                            break;

                        case 'A':
                            gs_stCtxInternal.iUseShmChannel = 1;
                            break;
                        case 'b':
                            a_pstCtx->iUseBus = 1;
                            break;

                        case 'C':
                            a_pstCtx->pszConfFile = optarg;
                            break;

                        case 'c':
                            a_pstCtx->iIdleCount = strtol(optarg, NULL, 0);
                            if (INT_MAX == a_pstCtx->iIdleCount && ERANGE == errno)
                            {
                                fprintf(stderr, "%s: Error: bad idle-count value.\n", pszApp);
                                exit(EINVAL);
                            }
                            if (1 > a_pstCtx->iIdleCount)
                            {
                                printf("%s: Warning: bad idle-count %d, use default value %d",
                                       pszApp, a_pstCtx->iIdleCount, TAPP_DEFAULT_IDLE_COUNT);
                                a_pstCtx->iIdleCount = TAPP_DEFAULT_IDLE_COUNT;
                            }
                            break;
                        case 'd':
                            a_pstCtx->iChannelSize = strtol(optarg, NULL, 0);
                            break;

                        case 'e':
                            a_pstCtx->iEpollWait = strtol(optarg, NULL, 0);
                            if (INT_MAX == a_pstCtx->iEpollWait && ERANGE == errno)
                            {
                                fprintf(stderr, "%s: Error: bad epoll-wait time.\n", pszApp);
                                exit(EINVAL);
                            }
                            break;

                        case 'E':
                            gs_stCtxInternal.iResetShm = 1;
                            break;
                        case 'f':
                            {
                                a_pstCtx->iUseBus = 1;
                                if (NULL != optarg)
                                    gs_stCtxInternal.stOption.iRefeshTbusTimer = strtol(optarg,
                                                                                        NULL, 10);
                                if (0 >= gs_stCtxInternal.stOption.iRefeshTbusTimer)
                                {
                                    gs_stCtxInternal.stOption.iRefeshTbusTimer
                                        = TAPP_REFRESH_TBUS_CONFIGRE_DEFAULT_TIMER;
                                }
                            }
                            break;
                        case 'G':
                            a_pstCtx->pszLogConfFile = optarg;
                            break;

                        case 'g':
                            iOnlyGerateConfTemple = 1;
                            break;

                        case 'i':
                            a_pstCtx->pszId = optarg;
                            break;

                        case 'k':
                            a_pstCtx->pszMBaseKey = optarg;
                            break;

                        case 'I':
                            a_pstCtx->pszTsmIdxKey = optarg;
                            break;

                        case 'M':
                            gs_stCtxInternal.iUseTsm = 1;
                            break;

                        case TAPP_USE_TSM_CONF:
                            gs_stCtxInternal.iUseTsmCnf = 1;
                            break;

                        case TAPP_USE_TSM_LOG_CONF:
                            gs_stCtxInternal.iUseTsmLogCfg = 1;
                            break;

                        case TAPP_USE_TSM_RUNTIMELOG_CONF:
                            gs_stCtxInternal.iUseTsmRuntimerLogCfg = 1;
                            break;

                        case 'L':
                            gs_stCtxInternal.pszLogFile = optarg;
                            break;

                        case 'l':
                            gs_stCtxInternal.pszLogLevel = optarg;
                            break;

                        case 'm':
                            a_pstCtx->stConfData.pszMetaName = optarg;
                            a_pstCtx->stConfPrepareData.pszMetaName = optarg;
                            gs_stCtxInternal.stConfBackupData.pszMetaName = optarg;
                            break;

                        case 'n':
                            a_pstCtx->iNoLoadConf = 1;
                            break;

                        case 'N':
                            a_pstCtx->iUseBus = 0;
                            break;

                        case 'P':
                            gs_stCtxInternal.pszPidFile = optarg;
                            break;

                        case 'r':
                            gs_stCtxInternal.pszRundataTimerConf = optarg;
                            break;

                        case 'R':
                            a_pstCtx->iNthreads = strtol(optarg, NULL, 0);
                            break;

                        case 's':
                            a_pstCtx->iIdleSleep = strtol(optarg, NULL, 0);
                            if (INT_MAX == a_pstCtx->iIdleSleep && ERANGE == errno)
                            {
                                fprintf(stderr, "%s: Error: bad idle-sleep value.\n", pszApp);
                                exit(EINVAL);
                            }
                            break;

                        case 't':
                            a_pstCtx->iTimer = strtol(optarg, NULL, 0);
                            if (INT_MAX == a_pstCtx->iTimer && ERANGE == errno)
                            {
                                fprintf(stderr, "%s: Error: bad timer value.\n", pszApp);
                                exit(EINVAL);
                            }
                            break;

                        case 'T':
                            gs_stCtxInternal.pszTdrFile = optarg;
                            break;

                        case 'u':
                            if (NULL != optarg)
                                a_pstCtx->iBusinessID = strtol(optarg, NULL, 10);
                            break;

                        case 'w':
                            gs_stCtxInternal.iWait = strtol(optarg, NULL, 0);
                            if (INT_MAX == gs_stCtxInternal.iWait && ERANGE == errno)
                            {
                                fprintf(stderr, "%s: Error: bad wait time.\n", pszApp);
                                exit(EINVAL);
                            }
                            break;

                        case 'x':
                            gs_stCtxInternal.stOption.iConfigFileFormat = strtol(optarg,
                                                                                 NULL, 0);
                            break;

                        case 'y':
                            if (NULL != optarg)
                                a_pstCtx->pszGCIMKey = optarg;
                            a_pstCtx->iUseBus = 1;
                            break;

                        case 'Y':
                            if (!gs_stCtxInternal.iBillSizeLimit)
                            {
                                gs_stCtxInternal.iBillSizeLimit = strtol(optarg, NULL, 0);
                            }

                            break;

                        case 'Z':
                            if (!gs_stCtxInternal.iBillPrecision)
                            {
                                gs_stCtxInternal.iBillPrecision = strtol(optarg, NULL, 0);
                            }
                            break;
                        case 'z':
                            gs_stCtxInternal.pszBillConfFile = optarg;

                            break;
                        case TAPP_CTRL_ADDR:
                            gs_stCtxInternal.pszCIAddr = optarg;
                            break;

                        case TAPP_CTRL_ON_OR_OFF:
                            gs_stCtxInternal.iCtrlOn = 0;
                            break;

                        case TAPP_START_MODE_OPT:
                            a_pstCtx->iStartMode = TAPP_START_MODE_RESUME;
                            break;

                        case TAPP_OUTPUT_STAT_OPT:
                            a_pstCtx->iNotOutputStat = 1;
                            break;

                        case TAPP_BUS_TIMEOUT_GAP:
                            a_pstCtx->iBusTimeOutGap = strtol(optarg, NULL, 0);
                            break;

                        case TAPP_BUS_USE_SYS_TIME:
                            a_pstCtx->iBusUseFreshTime = 1;
                            break;

                        case TAPP_BUS_HEARTBEAT_GAP:
                            {
                                int iArgGap = strtol(optarg, NULL, 0);
                                if (0 != iArgGap && TAPP_BUS_MIN_HEARTBEAT_GAP > iArgGap)
                                {
                                    printf("%s: Warning: 通过--bus-beat-gap=[NUMBER]指定的心跳时间间隔(%d)太小，"
                                           "将使用最小值(%d)\n", pszApp, iArgGap, TAPP_BUS_MIN_HEARTBEAT_GAP);
                                    iArgGap = TAPP_BUS_MIN_HEARTBEAT_GAP;
                                }
                                a_pstCtx->iBusHeartBeatGap = iArgGap;
                            }
                            break;

                        case TAPP_BUS_CHECK_PEER_GAP:
                            {
                                int iArgGap = strtol(optarg, NULL, 0);
                                if (0 != iArgGap && TAPP_BUS_MIN_CHECK_PEER_GAP > iArgGap)
                                {
                                    printf("%s: Warning: 通过--bus-check-gap=[NUMBER]指定的检查peer状态的时间间隔(%d)太小，"
                                           "将使用最小值(%d)\n", pszApp, iArgGap, TAPP_BUS_MIN_CHECK_PEER_GAP);
                                    iArgGap = TAPP_BUS_MIN_CHECK_PEER_GAP;
                                }
                                a_pstCtx->iBusCheckPeerGap = iArgGap;
                            }
                            break;

#if !defined (_WIN32) && !defined (_WIN64)
                        case TAPP_NO_STD_OUTPUT_OPT:
                            gs_stCtxInternal.iNoStdOutput = 1;
                            break;
#endif

                        default:
                            break;
                    }

                    break;
                case 'D':
                    gs_stCtxInternal.iIsDaemon = 1;
                    break;

                case 'h':
                    /* show help information. */
                    iRet = 0;
                    if (a_pstCtx->pfnUsage)
                        iRet = (*a_pstCtx->pfnUsage)(a_pstCtx->argc, a_pstCtx->argv);

                    if (0 == iRet)
                        tapp_def_usage(a_pstCtx->argc, a_pstCtx->argv);
                    exit(0);
                    break;

                case 'v':
                    /* show version. */

                    if (a_pstCtx->pfnVersionGenerator)
                    {
                        a_pstCtx->pfnVersionGenerator(a_pstCtx, a_pvArg);
                    }else
                    {
                        printf("%s: version %lld.%lld.%lld  build at %lld.\n", pszApp,
                               TAPP_GET_MAJOR(a_pstCtx->uiVersion),
                               TAPP_GET_MINOR(a_pstCtx->uiVersion),
                               TAPP_GET_REV(a_pstCtx->uiVersion),
                               TAPP_GET_BUILD(a_pstCtx->uiVersion));
                    }

                    exit(0);
                    break;
                case '?':
                    break;
                default:
                    break;
            }
        }


        if (1 != gs_stCtxInternal.iUseTsm && 1 == gs_stCtxInternal.iUseTsmCnf)
        {
            gs_stCtxInternal.iUseTsmCnf = 0;
            printf("%s: Warning: use-tsm-cfg is not available\n", a_pstCtx->pszApp);
        }

        if (1 != gs_stCtxInternal.iUseTsm && 1 == gs_stCtxInternal.iUseTsmLogCfg)
        {
            gs_stCtxInternal.iUseTsmLogCfg = 0;
            printf("%s: Warning: use-tsm-logcfg is not available\n", a_pstCtx->pszApp);
        }

        if (1 != gs_stCtxInternal.iUseTsm && 1 == gs_stCtxInternal.iUseTsmRuntimerLogCfg)
        {
            gs_stCtxInternal.iUseTsmRuntimerLogCfg = 0;
            printf("%s: Warning: use-tsm-runtimelogcfg is not available\n", a_pstCtx->pszApp);
        }


        if (iOnlyGerateConfTemple)
        {
            if (tapp_init_tdr(a_pstCtx, a_pvArg) < 0)
            {
                exit(-1);
            }

            if (a_pstCtx->pfnGenerate)
            {
                iRet = (*a_pstCtx->pfnGenerate)(a_pstCtx, a_pvArg);
            }
            else
            {
                iRet = tapp_do_generate(a_pstCtx, a_pvArg);
            }

            tapp_fini_tdr(a_pstCtx, a_pvArg);

            exit(iRet);
        }

        if (0 == gs_stCtxInternal.iThreadWaitInitTimeOut)
        {
            gs_stCtxInternal.iThreadWaitInitTimeOut = 10000;
        }

        if (gs_stCtxInternal.iThreadWaitInitTimeOut < 100)
        {
            gs_stCtxInternal.iThreadWaitInitTimeOut = 10000;
        }

        if (!gs_stCtxInternal.iBillSizeLimit)
        {
            gs_stCtxInternal.iBillSizeLimit = 10000000;
        }

        if (!gs_stCtxInternal.iBillPrecision)
        {
            gs_stCtxInternal.iBillPrecision = 3600;
        }
#if 0
        if(! a_pstCtx->pszBillConfFile)
        {
            a_pstCtx->pszBillConfFile = strdup("bill_%H_%M");
        }
#endif

        if (0 == a_pstCtx->iChannelSize)
        {
            a_pstCtx->iChannelSize = 1024000;
        }

        /* process start/stop/reload */
        gs_stCtxInternal.eOpCmd = TAPP_OPERATE_CMD_START;

        for (opt = optind; opt < a_pstCtx->argc; opt++)
        {
            if (0 == stricmp(a_pstCtx->argv[opt], TAPP_STOP))
            {
                gs_stCtxInternal.eOpCmd = TAPP_OPERATE_CMD_STOP;
                break;
            }
            else if (0 == stricmp(a_pstCtx->argv[opt], TAPP_RELOAD))
            {
                gs_stCtxInternal.eOpCmd = TAPP_OPERATE_CMD_RELOAD;
                break;
            }
            else if (0 == stricmp(a_pstCtx->argv[opt], TAPP_KILL))
            {
                gs_stCtxInternal.eOpCmd = TAPP_OPERATE_CMD_KILL;
                break;
            }
            else if (0 == stricmp(a_pstCtx->argv[opt], TAPP_START))
            {
                gs_stCtxInternal.eOpCmd = TAPP_OPERATE_CMD_START;
                break;
            }
            else if (0 == stricmp(a_pstCtx->argv[opt], TAPP_RESTART))
            {
                gs_stCtxInternal.eOpCmd = TAPP_OPERATE_CMD_RESTART;
                break;
            }
            else if (0 == stricmp(a_pstCtx->argv[opt], TAPP_CONTROL))
            {
                gs_stCtxInternal.eOpCmd = TAPP_OPERATE_CMD_CONTROL;
                break;
            }
#if defined (_WIN32) || defined (_WIN64)
            else if (0 == stricmp(a_pstCtx->argv[opt], TAPP_INSTALL))
            {
                gs_stCtxInternal.eOpCmd = TAPP_OPERATE_CMD_INSTALL;
                break;
            }
            else if (0 == stricmp(a_pstCtx->argv[opt], TAPP_UNINSTALL))
            {
                gs_stCtxInternal.eOpCmd = TAPP_OPERATE_CMD_UNINSTALL;
                break;
            }
#endif

        }/*for( opt=optind; opt<a_pstCtx->argc; opt++ )*/

        /* restore the getopt environment. */
        opterr = iOldOptErr;
        optarg = NULL;
        optind = 1;
        optopt = '?';

        if (TAPP_OPERATE_CMD_NONE == gs_stCtxInternal.eOpCmd)
        {
            fprintf(stderr, "%s: Error: need to specify one operation from:\n\t", pszApp);
#if !defined (_WIN32) && !defined (_WIN64)
            printf("\"%s | %s | %s | %s | %s | %s\"",
                   TAPP_START, TAPP_STOP, TAPP_RESTART,
                   TAPP_KILL, TAPP_RELOAD, TAPP_CONTROL);
#else
            printf("\"%s | %s | %s | %s | %s | %s | %s | %s\"", TAPP_START,
                   TAPP_STOP, TAPP_RESTART, TAPP_KILL, TAPP_RELOAD, TAPP_CONTROL,
                   TAPP_INSTALL, TAPP_UNINSTALL);
#endif
            printf("\n");
            exit(0);
        }

        return 0;
}

/* 控制通道初始化 */
int tapp_init_ctrlchannel(TAPPCTX* a_pstCtx, IN int a_iIsCtrlor)
{
    int iRet = 0;

    assert(NULL != a_pstCtx);

    if (a_iIsCtrlor)
    {
        gs_stTapp.pstAppCatData = NULL; /*不写运行日志*/
    }

    if (NULL == gs_stTapp.pstAppCatText)
    {
        printf("%s: Warning: tapp-control need tlog category named '%s'"
               ", but can't find it\n", a_pstCtx->pszApp, TLOG_DEF_CATEGORY_TEXTROOT);
    }

    if (!gs_stCtxInternal.pszCIAddr)
    {
        iRet = tappctrl_set_ctrlfile_info(&gs_stCtxInternal.pszCIAddr, a_pstCtx->pszApp,
                                          a_pstCtx->pszId, a_pstCtx->iBusinessID);
        if (0 != iRet)
        {
            printf("%s: Warning: failed to generate control channel info"
                   " by ID:%s and businessID:%d\n", a_pstCtx->pszApp,
                   ((NULL== a_pstCtx->pszId) ? "" : a_pstCtx->pszId), a_pstCtx->iBusinessID);
            return -1;
        }

    }

    iRet = tappctrl_init_i(gs_stCtxInternal.pszCIAddr, a_iIsCtrlor,
                           gs_stTapp.pstAppCatText, (void*) g_szMetalib_tapp_ctrl);
    return iRet;
}

/*tapp进程 控制端启动初始化过程*/
int tapp_control_mode_init(TAPPCTX* a_pstCtx, void* a_pvArg)
{
    int iRet = 0;
    char szTappCtrlName[PATH_MAX];
    int iWriteLen = -1;
    void *pvArg = NULL;

    assert(NULL != a_pstCtx);
    TOS_UNUSED_ARG(a_pvArg);

    /*生成tapp ctrl pid和log 文件所需的进程名*/
    iWriteLen = snprintf(szTappCtrlName, sizeof(szTappCtrlName), "%s_ctrl",
                         a_pstCtx->pszApp);
    if ((iWriteLen < 0) || (iWriteLen >= (int) sizeof(szTappCtrlName)))
    {
        fprintf(stderr, "%s: Error: failed to generate service controller's pid file",
                a_pstCtx->pszApp);
        return -1;
    }

    /*控制器的pid文件和 被控制进程的pid文件不同，因此需要重现生成控制器的pidfile*/
    gs_stCtxInternal.pszPidFile = NULL;
    iRet = tapp_set_pid_file(a_pstCtx, szTappCtrlName);
    if (0 != iRet)
    {
        return iRet;
    }

    //初始化控制器的日志系统
    iRet = tapp_init_log_data(a_pstCtx, pvArg);
    if (0 != iRet)
    {
        return iRet;
    }

    gs_stCtxInternal.pszLogFile = NULL;
    iRet = tapp_init_log(a_pstCtx, szTappCtrlName);
    if (0 != iRet)
    {
        fprintf(stderr, "%s: Error: failed to init log, ret %d\n", a_pstCtx->pszApp, iRet);
        return -6;
    }

    //控制通道初始化
    iRet = tapp_init_ctrlchannel(a_pstCtx, 1);
    if (0 != iRet)
    {
        tlog_error(gs_stTapp.pstAppCatText, 0, 0,
                   "Warning: unable to turn on tapp-control");
        fprintf(stderr, "%s: Error: unable to turn on tapp-control\n", a_pstCtx->pszApp);
        return iRet;
    }
    else
    {
        tlog_trace(gs_stTapp.pstAppCatText, 0, 0,
                   "succeed to turn on tapp-control");
    }

    gs_stCtxInternal.stBaseData.iLen = sizeof(TAPP_BASEDATA);
    gs_stCtxInternal.stBaseData.pszMetaName = "tapp_basedata";
    gs_stCtxInternal.stBaseData.iMeta
        = (intptr_t) tdr_get_meta_by_name((LPTDRMETALIB) (void *)(&g_szMetalib_tapp_basedata_def[0]),
                                          gs_stCtxInternal.stBaseData.pszMetaName);

    gs_stCtxInternal.stRunDataTimer.iLen = sizeof(TAPP_RUNDATA_TIMER);
    gs_stCtxInternal.stRunDataTimer.pszMetaName = "tapp_rundata_timer";
    gs_stCtxInternal.stRunDataTimer.iMeta
        = (intptr_t) tdr_get_meta_by_name((LPTDRMETALIB) (void *) (&g_szMetalib_tapp_rundata_timer_def[0]),
                                          gs_stCtxInternal.stRunDataTimer.pszMetaName);
    iRet = tapp_alloc_one_data(&gs_stCtxInternal.stBaseData);
    if (0 == iRet)
    {
        TAPP_BASEDATA *pstBaseData =
            (TAPP_BASEDATA *) gs_stCtxInternal.stBaseData.pszBuff;
        pstBaseData->fProcMinTime = 100000.0;
    }

    /*将进程的回调函数设置成 控制器的回调函数*/
    a_pstCtx->pfnInit = tapp_controller_init;
    a_pstCtx->pfnProc = tapp_controller_proc;
    a_pstCtx->pfnFini = tapp_controller_fini;

    /* 控制终端不能再被控制 */
    gs_stCtxInternal.iCtrlOn = 0;

    a_pstCtx->pfnGenerate = NULL;
    a_pstCtx->pfnGetCtrlUsage = NULL;
    a_pstCtx->pfnIdle = NULL;
    a_pstCtx->pfnQuit = NULL;
    a_pstCtx->pfnReload = NULL;
    a_pstCtx->pfnStop = NULL;
    a_pstCtx->pfnTick = NULL;
    a_pstCtx->pfnUsage = NULL;

    return iRet;
}

#if !defined (_WIN32) && !defined (_WIN64)
static void tapp_surpress_std_output(TAPPCTX* a_pstCtx)
{
    int iNullFd = -1;
    int iStdoutFd = -1;
    int iStderrFd = -1;

    iNullFd = open("/dev/null", O_RDWR);
    if (0 > iNullFd)
    {
        fprintf(stderr, "%s: Error: failed to open /def/null\n", a_pstCtx->pszApp);
        return;
    }

    iStdoutFd = fileno(stdout);
    if (0 > iStdoutFd || 0 > dup2(iNullFd, iStdoutFd))
    {
        fprintf(stderr, "%s: Error: failed to surpress output to stdout\n", a_pstCtx->pszApp);
    }

    iStderrFd = fileno(stderr);
    if (0 > iStderrFd || 0 > dup2(iNullFd, iStderrFd))
    {
        fprintf(stderr, "%s: Error: failed to surpress output to stderr\n", a_pstCtx->pszApp);
    }
}
#endif

/* tapp进程正常启动初始化过程 */
/* 在当前函数中加入逻辑时，如果导致返回失败值(<0), 一定要输出错误原因到标准(错误)输出或日志文件*/
/* 如果错误原因是调用子函数失败，可以选择在相应的子函数中输出错误原因 */
static int tapp_normal_mode_init(TAPPCTX* a_pstCtx, void* a_pvArg)
{
    int iRet = 0;
    int iWriteLen;

    assert(NULL != a_pstCtx);

    iWriteLen = snprintf(gs_stTapp.szConfBakFile,
                         sizeof(gs_stTapp.szConfBakFile), "%s.bak", a_pstCtx->pszConfFile);
    if (iWriteLen < 0 || iWriteLen >= (int) sizeof(gs_stTapp.szConfBakFile))
    {
        fprintf(stderr, "%s: Error: failed to generate configure backup file"
                " by file '%s', maybe filename is too long\n",
                a_pstCtx->pszApp, a_pstCtx->pszConfFile);
        return -1;
    }

    /*mng相关信息初始化*/
    if (tapp_init_tsm(a_pstCtx, a_pvArg) < 0)
    {
        return -3;
    }

    /*初始化tdr数据*/
    iRet = tapp_init_tdr(a_pstCtx, a_pvArg);
    if (0 != iRet)
    {
        return -4;
    }

    if (gs_stCtxInternal.iUseTsm && !a_pstCtx->iLib)
    {
        iRet = tsm_tdr_load_meta(gs_stCtxInternal.iTsm,
                                 (void *) a_pstCtx->iLib,
                                 a_pstCtx->iBusinessID);
        if (0 > iRet)
        {
            terror_dump(NULL);
            return -8;
        }
    }

    if (tapp_init_data(a_pstCtx, a_pvArg) < 0)
    {
        return -5;
    }

    /*可能使用到tsm，所以放到tsm初始化的后面*/
    iRet = tapp_init_log(a_pstCtx, a_pstCtx->pszApp);
    if (0 > iRet)
    {
        fprintf(stderr, "%s: Error: failed to init log, ret %d\n", a_pstCtx->pszApp, iRet);
        return -6;
    }

    /*初始化bus*/
    if (tapp_init_bus(a_pstCtx, a_pvArg) < 0)
    {
        return -7;
    }

    tapp_init_rundata_timer(a_pstCtx, a_pvArg);

    /*控制通道初始化*/
    if (gs_stCtxInternal.iCtrlOn)
    {
        if (0 != tapp_init_ctrlchannel(a_pstCtx, 0))
        {
            printf("%s: Warning: unable to turn on tapp-control\n", a_pstCtx->pszApp);
            tlog_error(gs_stTapp.pstAppCatText, 0, 0,
                       "Warning: unable to turn on tapp-control");
        }
        else
        {
            tlog_trace(gs_stTapp.pstAppCatText, 0, 0,
                       "succeed to turn on tapp-control");
        }
    }

    /*初始化守护进程*/
    if (gs_stCtxInternal.iIsDaemon != 0)
    {
        tos_make_daemon(NULL);
#if !defined (_WIN32) && !defined (_WIN64)
        if (gs_stCtxInternal.iNoStdOutput)
        {
            tapp_surpress_std_output(a_pstCtx);
        }
#endif
    }

    return 0;
}

/* 引入文件锁用于判断是否有前一个进程实例，
 * 如果加锁成功，则需要保持这个文件锁，直到进程退出;
 * 如果加锁失败，则认为前一个进程存在;
 * 由于文件锁是与 fd 相关的，为了保持锁的存在，
 * 该函数在打开文件后没有关闭的操作，依赖于进程退出时系统自动回收
 *
 * 函数返回值 表示是否加锁成功
 */
static int tapp_lock_pid_file(TAPPCTX* a_pstCtx)
{
    HANDLE file;
    int iRet = 0;
    int iIsSuccessful = 0;
    const char* pszPidFile = gs_stCtxInternal.pszPidFile;

    assert(NULL != a_pstCtx);
    assert(NULL != pszPidFile);

    file = tfopen(pszPidFile, TF_MODE_READ | TF_MODE_WRITE | TF_MODE_CREATE);

#if defined (_WIN32) || defined (_WIN64)
    {
        LPTSTR GetLastErrorString(void);

        if (HFILE_ERROR == file)
        {
            fprintf(stderr, "%s: Error: failed to open pid file: %s, for %s",
                    a_pstCtx->pszApp, pszPidFile, GetLastErrorString());
        }
        else
        {
            OVERLAPPED overlap;

            memset(&overlap, 0, sizeof(overlap));
            /* 从4096字节开始加锁，这样是为了保证后继写pid文件时，能够在文件开始处写数据 */
            overlap.Offset = 4096;

            iRet = LockFileEx(file,
                              LOCKFILE_EXCLUSIVE_LOCK | LOCKFILE_FAIL_IMMEDIATELY,
                              0, 4096, 0, &overlap);
            /* 注意：LockFileEx 返回0，表示加锁失败 */
            if (0 != iRet)
            {
                iIsSuccessful = 1;
            }
            else /* 加锁失败，不需要保持打开文件 */
            {
                iIsSuccessful = 0;
                CloseHandle(file);
                if (ERROR_LOCK_VIOLATION != GetLastError())
                {
                    fprintf(stderr, "%s: Error: failed to lock pid file: %s, for %s",
                            a_pstCtx->pszApp, pszPidFile, GetLastErrorString());
                    exit(-1);
                }
            }
        }
    }
#else
    {
        if (-1 == file)
        {
            fprintf(stderr, "%s: Error: failed to open pid file: %s, for %s",
                    a_pstCtx->pszApp, pszPidFile, strerror(errno));
        }
        else
        {
            /* lock */
            iRet = flock(file, LOCK_EX | LOCK_NB);
            if (0 == iRet)
            {
                iIsSuccessful = 1;
            }
            else /* 加锁失败，不需要保持打开文件 */
            {
                iIsSuccessful = 0;
                close(file);
                if (EWOULDBLOCK != errno)
                {
                    fprintf(stderr, "%s: Error: failed to lock pid file: %s, for %s",
                            a_pstCtx->pszApp, pszPidFile, strerror(errno));
                    exit(-1);
                }
            }
        }
    }
#endif

    return iIsSuccessful;
}

static int tapp_operate_init(TAPPCTX* a_pstCtx, void* a_pvArg)
{
    int iRet = 0;

    /* 如果锁 pid 文件成功，表明没有其他进程实例在运行 */
    int iGotLock = 0;

    assert(NULL != a_pstCtx);

    /*生成pid文件的信息*/
    if (tapp_set_pid_file(a_pstCtx, a_pstCtx->pszApp) < 0)
    {
        exit(-1);
    }
    gettimeofday(&a_pstCtx->stCurr, NULL);

    /* 根据要执行的命令，选择性对pid文件加文件锁 */
    switch (gs_stCtxInternal.eOpCmd)
    {
        case TAPP_OPERATE_CMD_STOP:
        case TAPP_OPERATE_CMD_KILL:
        case TAPP_OPERATE_CMD_RELOAD:
        case TAPP_OPERATE_CMD_RESTART:
        case TAPP_OPERATE_CMD_START:
            {
                iGotLock = tapp_lock_pid_file(a_pstCtx);
            }
        default:
            ; /* no operation is needed */
    }

    /*处理各种命令*/
    switch (gs_stCtxInternal.eOpCmd)
    {
        case TAPP_OPERATE_CMD_STOP:
            {
                if (!iGotLock)
                {
                    iRet = tos_send_signal(gs_stCtxInternal.pszPidFile, a_pstCtx->pszApp,
                                           SIGUSR1, NULL);
                }
                exit(iRet);
            }
        case TAPP_OPERATE_CMD_KILL:
            {
                if (!iGotLock)
                {
                    iRet = tos_kill_prev(gs_stCtxInternal.pszPidFile, a_pstCtx->pszApp,
                                         gs_stCtxInternal.iWait);
                    if (0 != iRet || !(iGotLock = tapp_lock_pid_file(a_pstCtx)))
                    {
                        fprintf(stderr, "%s: Error: failed to kill preivous process\n",
                                a_pstCtx->pszApp);
                        tlog_error(gs_stTapp.pstAppCatText, 0, 0,
                                   "failed to kill preprocess(name:%s) by pid file(%s), for %s",
                                   a_pstCtx->pszApp,
                                   gs_stCtxInternal.pszPidFile,
                                   strerror(errno));
                    }
                }
                exit(iRet);
            }
        case TAPP_OPERATE_CMD_RELOAD:
            {
                if (!iGotLock)
                {
                    iRet = tos_send_signal(gs_stCtxInternal.pszPidFile, a_pstCtx->pszApp,
                                           SIGUSR2, NULL);
                }
                exit(iRet);
            }
        case TAPP_OPERATE_CMD_RESTART:
            {
                if (!iGotLock)
                {
                    iRet = tos_kill_prev(gs_stCtxInternal.pszPidFile, a_pstCtx->pszApp,
                                         gs_stCtxInternal.iWait);
                    if (0 != iRet || !(iGotLock = tapp_lock_pid_file(a_pstCtx)))
                    {
                        fprintf(stderr, "%s: Error: failed to kill preivous process\n",
                                a_pstCtx->pszApp);
                        tlog_error(gs_stTapp.pstAppCatText, 0, 0,
                                   "failed to kill preprocess(name:%s) by pid file(%s), for %s",
                                   a_pstCtx->pszApp,
                                   gs_stCtxInternal.pszPidFile,
                                   strerror(errno));
                        exit(-1);
                    }
                }
                iRet = tapp_normal_mode_init(a_pstCtx, a_pvArg);
                if (0 != iRet)
                {
                    exit(iRet);
                }
            }
            break;
        case TAPP_OPERATE_CMD_START:
            {
                /*如果之前已经启动，则直接退出*/
                if (!iGotLock)
                {
                    printf("%s: Warning: service has been started, "
                           "if you want to restart, please  use \'%s restart\' "
                           " or stop the current service at first by using \'%s stop\'\n",
                           a_pstCtx->pszApp, a_pstCtx->pszApp, a_pstCtx->pszApp);
                    iRet = -1;
                }
                else
                {
                    iRet = tapp_normal_mode_init(a_pstCtx, a_pvArg);
                }
                if (0 != iRet)
                {
                    exit(iRet);
                }
            }
            break;
        case TAPP_OPERATE_CMD_CONTROL: /*作为控制端启动*/
            {
                iRet = tapp_control_mode_init(a_pstCtx, a_pvArg);
                if (0 != iRet)
                {
                    exit(iRet);
                }
            }
            break;
#if defined (_WIN32) || defined (_WIN64)
        case TAPP_OPERATE_CMD_INSTALL:
            break;
        case TAPP_OPERATE_CMD_UNINSTALL:
            break;
#endif
        default:
            {
#if defined (_WIN32) || defined (_WIN64)
                printf("Info: to install this service, use \'%s install\'"
                       "(the keyword \'install\' must be appended ).\n", a_pstCtx->pszApp);
                printf("Info: to uninstall this service, use \'%s uninstall\'"
                       "(the keyword \'uninstall\' must be appended ).\n",
                       a_pstCtx->pszApp);
#endif
                printf("Info: to start this service, use \'%s start\'"
                       "(the keyword \'start\' must be appended ).\n", a_pstCtx->pszApp);
                printf("Info: to restart this service, use \'%s restart\'"
                       "(the keyword \'restart\' must be appended ).\n", a_pstCtx->pszApp);
                printf("Info: to control this service, use \'%s control\'"
                       "(the keyword \'control\' must be appended ).\n", a_pstCtx->pszApp);
                exit(-2);
            }
            break;
    } /*switch(gs_stCtxInternal.eOpCmd)*/

    /* 如果前一次加锁失败，需要在这里再一次加锁 */
    if (!iGotLock)
    {
        iGotLock = tapp_lock_pid_file(a_pstCtx);
    }

    return 0;
}

TSF4G_API int tapp_def_init(TAPPCTX* a_pstCtx, void* a_pvArg)
{
    int iRet = 0;
    struct sigaction stAct;
    int iTimeout;

    assert(NULL != a_pstCtx);

    tapp_def_opt(a_pstCtx, a_pvArg);

    /*根据命令行参数信息，执行各个 操作命令的初始化*/
    tapp_operate_init(a_pstCtx, a_pvArg);

    tos_init(NULL);

    /*设置信号处理函数*/
    tos_ignore_pipe();
    memset(&stAct, 0, sizeof(stAct));
    stAct.sa_handler = tapp_on_sigterm;

    sigaction(SIGTERM, &stAct, NULL);
    sigaction(SIGQUIT, &stAct, NULL);
    sigaction(SIGINT, &stAct, NULL);
    sigaction(SIGABRT, &stAct, NULL);

    stAct.sa_handler = tapp_on_sigusr1;
    sigaction(SIGUSR1, &stAct, NULL);

    stAct.sa_handler = tapp_on_sigusr2;
    sigaction(SIGUSR2, &stAct, NULL);

    tos_write_pid(gs_stCtxInternal.pszPidFile);

    iRet = tapp_def_thread_init(a_pstCtx, a_pvArg);
    if (0 != iRet)
    {
        fprintf(stderr, "%s: Error: tapp thread init failed\n", a_pstCtx->pszApp);
        return -1;
    }

    /*执行业务的初始化逻辑*/
    if (a_pstCtx->pfnInit)
    {
        tlog_trace(gs_stTapp.pstAppCatText, 0, 0, "begin to call app_init");
        iRet = (*a_pstCtx->pfnInit)(a_pstCtx, a_pvArg);
        if (0 != iRet)
        {
            fprintf(stderr, "%s: Error: application init function"
                    "(pointed by a_pstCtx->pfnInit) failed\n", a_pstCtx->pszApp);
            return -1;
        }
        tlog_trace(gs_stTapp.pstAppCatText, 0, 0, "call app_init successfully!");
    }

    if (a_pstCtx->iNthreads > 0)
    {
        iTimeout = 0;
        while (tapp_have_threads_in_stats(a_pstCtx, TAPP_THREAD_STATUS_CREATED)
               && iTimeout < gs_stCtxInternal.iThreadWaitInitTimeOut)
        {
            iTimeout += 10;
            usleep(10000);
        }

        /*Some threads may failed to init here.*/
        if (iTimeout >= gs_stCtxInternal.iThreadWaitInitTimeOut)
        {
            fprintf(stderr, "%s: Error: thread initiation timeout, surpass limit:%d ms\n",
                    a_pstCtx->pszApp, gs_stCtxInternal.iThreadWaitInitTimeOut);
            return -1;
        }
        gs_stCtxInternal.stAppThreadCtxInternal.iLastRecvChannelIdx = 0;
        gs_stCtxInternal.stAppThreadCtxInternal.iChannelPkgNumRecved = 0;
    }

    return iRet;
}

TSF4G_API int tapp_def_fini(TAPPCTX* a_pstCtx, void* a_pvArg)
{
    tapp_wait_thread_exit(a_pstCtx);
    if (a_pstCtx->pfnFini)
        a_pstCtx->pfnFini(a_pstCtx, a_pvArg);

    tappctrl_fini();

    if (a_pstCtx->iUseBus)
    {
#ifdef TAPP_TBUS
        // tbus_notify_peer_quit(a_pstCtx->iBus, TBUS_ADDR_ALL, 0);

        tbus_delete(&a_pstCtx->iBus);

        tbus_fini();
#endif /* TAPP_TBUS */
    }

    tapp_fini_data(a_pstCtx, a_pvArg);

    tapp_fini_tsm(a_pstCtx, a_pvArg);

    tos_fini();

    tapp_fini_log(a_pstCtx, a_pvArg);

    tapp_fini_tdr(a_pstCtx, a_pvArg);

    tapp_def_thread_fini(a_pstCtx, a_pvArg);

    return 0;
}

TSF4G_API int tapp_def_reload(TAPPCTX* a_pstCtx, void* a_pvArg)
{
#if defined(TAPP_TDR) && defined(TAPP_TDR_XML)
    TDRDATA stHost;

    TOS_UNUSED_ARG(a_pvArg);

    tlog_trace(gs_stTapp.pstAppCatData, 0, 0, "in tapp_def_reload(...)\n");
    if (a_pstCtx->stConfData.pszBuff && a_pstCtx->stConfData.iMeta)
    {
        // output current conf-data to stdout
        stHost.pszBuff = (char*) a_pstCtx->stConfData.pszBuff;
        stHost.iBuff = (int) a_pstCtx->stConfData.iLen;

        fprintf(stdout,
                "-------------------configuration data-------------------------------\n");
        tdr_output_fp((LPTDRMETA) a_pstCtx->stConfData.iMeta, stdout, &stHost, 0, 0);
        fprintf(stdout,
                "--------------------------------------------------------------------\n");
    }
#endif /* TAPP_TDR */

    return 0;
}

static int tapp_reload_conf(TAPPCTX* a_pstCtx, void* a_pvArg)
{
    int iRet;
    int iNeedCopy = 0;

    assert(NULL != a_pstCtx);

    if (NULL != a_pstCtx->stConfData.pszBuff
        && NULL != gs_stCtxInternal.stConfBackupData.pszBuff)
    {
        memcpy(gs_stCtxInternal.stConfBackupData.pszBuff,
               a_pstCtx->stConfData.pszBuff,
               gs_stCtxInternal.stConfBackupData.iLen);
    }

    if (0 == gs_stCtxInternal.iUseTsmCnf && 0 == a_pstCtx->iNoLoadConf)
    {
        struct stat stStat;

        if (0 == stat(a_pstCtx->pszConfFile, &stStat))
        {
            if (gs_stCtxInternal.tConfMod == stStat.st_mtime)
            {
                /*可能存在多个配置文件,主配置文件间接引用其他配置文件*/
                if (NULL != a_pstCtx->stConfData.pszBuff
                    && NULL != a_pstCtx->stConfPrepareData.pszBuff)
                {
                    memcpy(a_pstCtx->stConfPrepareData.pszBuff,
                           a_pstCtx->stConfData.pszBuff,
                           a_pstCtx->stConfPrepareData.iLen);
                }

                if (NULL != a_pstCtx->pfnReload)
                {
                    iRet = (*a_pstCtx->pfnReload)(a_pstCtx, a_pvArg);
                }
                else
                {
                    iRet = tapp_def_reload(a_pstCtx, a_pvArg);
                }
                return iRet;
            }

            gs_stCtxInternal.tConfMod = stStat.st_mtime;
        }

        //put conf file info to ConfPrepareData
        if (NULL != a_pstCtx->stConfPrepareData.pszBuff
            && a_pstCtx->stConfPrepareData.iMeta)
        {
            iRet = tapp_init_data_by_file(a_pstCtx,
                                          &a_pstCtx->stConfPrepareData,
                                          a_pstCtx->pszConfFile,
                                          gs_stCtxInternal.stOption.iConfigFileFormat);
            if (0 > iRet)
            {
                fprintf(stderr, "%s: Error: reload conf file %s failed\n",
                        a_pstCtx->pszApp, a_pstCtx->pszConfFile);
                tlog_error(gs_stTapp.pstAppCatText, 0, 0,
                           "reload conf file %s failed\n", a_pstCtx->pszConfFile);
                return -1;
            }
        }
        iNeedCopy = 1;
    } /* if (0 == gs_stCtxInternal.iUseTsmCnf && 0 == a_pstCtx->iNoLoadConf) */

    if (NULL != a_pstCtx->pfnReload)
    {
        iRet = (*a_pstCtx->pfnReload)(a_pstCtx, a_pvArg);
    }
    else
    {
        if (iNeedCopy)
        {
            memcpy(a_pstCtx->stConfData.pszBuff,
                   a_pstCtx->stConfPrepareData.pszBuff,
                   a_pstCtx->stConfData.iLen);
        }
        iRet = tapp_def_reload(a_pstCtx, a_pvArg);
    }

    if (NULL != a_pstCtx->stConfData.pszBuff
        && NULL != gs_stCtxInternal.stConfBackupData.pszBuff
        && gs_stCtxInternal.stConfBackupData.iMeta)
    {
        TDRDATA stHost;

        stHost.pszBuff = (char*) gs_stCtxInternal.stConfBackupData.pszBuff;
        stHost.iBuff = (int) gs_stCtxInternal.stConfBackupData.iLen;

        tdr_output_file((LPTDRMETA) gs_stCtxInternal.stConfBackupData.iMeta,
                        gs_stTapp.szConfBakFile, &stHost, 0, 0);
    }

    return iRet;
}

static int tapp_reload_rundata_timer(TAPPCTX* a_pstCtx, void* a_pvArg)
{
    int iRet = 0;

    TAPP_RUNDATA_TIMER stPrepare;
    TAPPDATA stTappData;

    TOS_UNUSED_ARG(a_pstCtx);
    TOS_UNUSED_ARG(a_pvArg);

    if (NULL == gs_stCtxInternal.stRunDataTimer.pszBuff)
    {
        return -1;
    }

    if (0 == gs_stCtxInternal.iUseTsmRuntimerLogCfg)
    {
        struct stat stStat;

        if (0 == stat(gs_stTapp.szRunDataTimerConfFile, &stStat))
        {
            if (gs_stCtxInternal.tRunTimerConfMod == stStat.st_mtime)
            {
                return 0;
            }

            gs_stCtxInternal.tRunTimerConfMod = stStat.st_mtime;
        }

        stTappData.pszBuff = (char *) &stPrepare;
        stTappData.iLen = gs_stCtxInternal.stRunDataTimer.iLen;
        stTappData.iMeta = gs_stCtxInternal.stRunDataTimer.iMeta;
        iRet = tapp_init_data_by_file(a_pstCtx, &stTappData, gs_stTapp.szRunDataTimerConfFile, 0);
        if (0 > iRet)
        {
            fprintf(stderr, "%s: Error: failed to load rundata timer conf file %s\n",
                    a_pstCtx->pszApp, gs_stTapp.szRunDataTimerConfFile);
            tlog_error(gs_stTapp.pstAppCatText, 0, 0,
                       "cant load rundata timer conf file %s\n",
                       gs_stTapp.szRunDataTimerConfFile);
            return -1;
        }

        memcpy(gs_stCtxInternal.stRunDataTimer.pszBuff, &stPrepare,
               gs_stCtxInternal.stRunDataTimer.iLen);
    }
    else
    {
        printf("%s: Warning: use tsm runtime log config is not supported now\n", a_pstCtx->pszApp);
    }

    return 0;
}

static int tapp_reload(TAPPCTX* a_pstCtx, void* a_pvArg)
{
    int iRet;

    tapp_reload_conf(a_pstCtx, a_pvArg);
    tapp_reload_log(a_pstCtx, a_pvArg);
    tapp_reload_rundata_timer(a_pstCtx, a_pvArg);

    iRet = tapp_reload_thread(a_pstCtx);
    if (0 != iRet)
    {
        return -1;
    }

    return 0;
}

int tapp_rundata_log(TAPPDATA *a_pstTappData)
{
    int iRet = 0;

    char szMsg[8192];
    TDRDATA stOut;
    TDRDATA stHost;

    if (0 == a_pstTappData->iMeta
        || 0 == a_pstTappData->iLen
        || NULL == a_pstTappData->pszBuff)
    {
        return -1;
    }

    stOut.iBuff = sizeof(szMsg);
    stOut.pszBuff = szMsg;
    stHost.iBuff = a_pstTappData->iLen;
    stHost.pszBuff = a_pstTappData->pszBuff;

    iRet = tdr_sprintf((LPTDRMETA) (a_pstTappData->iMeta), &stOut, &stHost, 0);
    if (0 > iRet)
    {
        return -1;
    }

    tlog_info(gs_stTapp.pstAppCatData, 0, 0, "%s", szMsg);

    return 0;
}

static void tapp_proc_bus_event(TAPPCTX *a_pstCtx, void *a_pvArg,
                                TBUSADDR a_iPeerAddr, int a_iPeerCurState)
{
    int iRet = 0;

    assert(NULL != a_pstCtx);

    tlog_info(gs_stTapp.pstAppCatText, 0, 0,
              "bus channel<peerAddr:%d> state changed to <%d>", a_iPeerAddr,
              a_iPeerCurState);

    if (NULL != a_pstCtx->pfnBusPeerStateFlipped)
    {
        iRet = a_pstCtx->pfnBusPeerStateFlipped(a_pstCtx, a_pvArg, a_iPeerAddr,
                                                a_iPeerCurState);
        tlog_info(gs_stTapp.pstAppCatText, 0, 0, "pfnBusTimeOut return <%d>",
                  iRet);
    }
}

static void tapp_send_bus_heartbeat(TAPPCTX *a_pstCtx)
{
    int iRet = 0;
    struct timeval stSub;
    static struct timeval stLastHeartBeatTime = { 0, 0 };

    assert(NULL != a_pstCtx);

    if (0 >= a_pstCtx->iBusHeartBeatGap)
    {
        return;
    }

    TV_DIFF(stSub, a_pstCtx->stCurr, stLastHeartBeatTime);
    if (stSub.tv_sec < a_pstCtx->iBusHeartBeatGap)
    {
        return;
    }

    iRet = tbus_send_heartbeat(a_pstCtx->iBus, TBUS_ADDR_ALL, 0);
    if (TBUS_ERR_IS_ERROR(iRet))
    {
        tlog_debug(gs_stTapp.pstAppCatText, 0, 0,
                   "tbus_send_heartbeat ret<%d>, %s", iRet,
                   tbus_error_string(iRet));
    }

    TV_CLONE(stLastHeartBeatTime, a_pstCtx->stCurr);
}

static void tapp_check_bus_event(TAPPCTX *a_pstCtx, void *a_pvArg)
{
    int iRet = 0;
    TBUSADDR iPeerAddr;
    int iPeerPreState;
    int iPeerCurState;
    int i = 0;
    struct timeval stSub;
    static struct timeval stLastCheckPeerTime =
    { 0, 0 };

    assert(NULL != a_pstCtx);

    if (0 >= a_pstCtx->iBusCheckPeerGap)
    {
        return;
    }

    TV_DIFF(stSub, a_pstCtx->stCurr, stLastCheckPeerTime);
    if (stSub.tv_sec < a_pstCtx->iBusCheckPeerGap)
    {
        return;
    }

    iPeerAddr = 0;
    for (i = 0; i < TBUS_MAX_CHANNEL_NUM_PREHOST; i++)
    {
        iRet = tbus_get_next_peer_addr(a_pstCtx->iBus, &iPeerAddr);
        if (TBUS_ERR_IS_ERROR(iRet))
        {
            if (TBUS_ERR_MAKE_ERROR(TBUS_ERROR_NO_MORE_PEER) == iRet)
            {
                break;
            }
            tlog_info(gs_stTapp.pstAppCatText, 0, 0,
                      "tbus_get_next_peer_state<iPeerAddr:%d> failed, for %s",
                      iPeerAddr, tbus_error_string(iRet));
            continue; /* batch mode, single error is skipped */
        }

        iRet = tbus_get_peer_states(&iPeerPreState, &iPeerCurState,
                                    a_pstCtx->iBus, iPeerAddr, a_pstCtx->iBusUseFreshTime);
        if (TBUS_ERR_IS_ERROR(iRet))
        {
            tlog_error(gs_stTapp.pstAppCatText, 0, 0,
                       "tbus_is_peer_timeout failed, for %s", tbus_error_string(iRet));
        }
        else if (iPeerPreState != iPeerCurState)
        {
            tapp_proc_bus_event(a_pstCtx, a_pvArg, iPeerAddr, iPeerCurState);
        }

    }

    TV_CLONE(stLastCheckPeerTime, a_pstCtx->stCurr);
}

int tapp_proc_reload(TAPPCTX* a_pstCtx, void* a_pvArg)
{
    if (gs_stTapp.iReload)
    {
        gs_stTapp.iReload = 0;

        if (0 <= tapp_reload(a_pstCtx, a_pvArg))
        {
            printf("%s: reload succ\n", a_pstCtx->pszApp);
            tlog_info(gs_stTapp.pstAppCatText, 0, 0, "reload succ");
        }
        else
        {
            fprintf(stderr, "%s: Error: reload failed\n", a_pstCtx->pszApp);
            tlog_info(gs_stTapp.pstAppCatText, 0, 0, "reload failed");
        }
    } /* if( gs_stTapp.iReload ) */

    return 0;
}

extern int tappctrl_proc_pkg(TAPPCTX* a_pstCtx, void* a_pvArg);

int tappctrl_query_status()
{
    return gs_stCtxInternal.iCtrlOn;
}

void tappctrl_turn_off()
{
    if (gs_stCtxInternal.iCtrlOn)
    {
        gs_stCtxInternal.iCtrlOn = 0;
    }
}

int tapp_proc_ctrl_msg(TAPPCTX* a_pstCtx, void* a_pvArg)
{
    /*处理控制消息*/
    if (gs_stCtxInternal.iCtrlOn)
    {
        tappctrl_proc_pkg(a_pstCtx, a_pvArg);
    }
    return 0;
}

int tapp_proc_exit_msg(TAPPCTX* a_pstCtx, void* a_pvArg)
{
    int iRet;

    assert(NULL != a_pstCtx);

    switch (gs_stTapp.iIsExit)
    {
        case TAPP_EXIT_QUIT:

            if (a_pstCtx->pfnQuit)
            {
                iRet = (*a_pstCtx->pfnQuit)(a_pstCtx, a_pvArg);
            }
            else
            {
                iRet = -1;
            }

            tapp_thread_set_exit_flag(a_pstCtx);

            if (0 > iRet)
            {
                tlog_info(gs_stTapp.pstAppCatText, 0, 0,
                          "process(%s pid:%d) recv quit signal, so quit",
                          a_pstCtx->pszApp, getpid());
                gs_stTapp.iExitMainloop = 1;
                tapp_wait_thread_exit(a_pstCtx);
            }

            break;

        case TAPP_EXIT_STOP:

            if (a_pstCtx->pfnStop)
            {
                iRet = (*a_pstCtx->pfnStop)(a_pstCtx, a_pvArg);
            }
            else
            {
                iRet = -1;
            }

            tapp_thread_set_exit_flag(a_pstCtx);

            if (0 > iRet)
            {
                tlog_info(gs_stTapp.pstAppCatText, 0, 0,
                          "process(%s pid:%d) recv stop signal, so stop",
                          a_pstCtx->pszApp, getpid());
                gs_stTapp.iExitMainloop = 1;
                tapp_wait_thread_exit(a_pstCtx);
            }

            break;

        default:
            break;
    } /* switch( gs_stTapp.iIsExit ) */

    return 0;
}

int tapp_proc_proc(TAPPCTX* a_pstCtx, void* a_pvArg,
                   TAPP_BASEDATA *a_pstBaseData)
{
    int iRet;
    static int iIdle = 0;

    assert(NULL != a_pstCtx);
    assert(NULL != a_pstBaseData);

    if (a_pstCtx->pfnProc)
    {
        struct timeval stEnd;
        struct timeval stStart;
        struct timeval stSub;
        float fCost;

        gettimeofday(&stStart, NULL);
        iRet = (*a_pstCtx->pfnProc)(a_pstCtx, a_pvArg);
        gettimeofday(&stEnd, NULL);

        TV_DIFF(stSub, stEnd, stStart);
        fCost = (float) (stSub.tv_sec * 1000 + stSub.tv_usec / 1000.0);

        a_pstBaseData->fProcMaxTime = tmax(fCost, a_pstBaseData->fProcMaxTime);
        a_pstBaseData->fProcMinTime = tmin(fCost, a_pstBaseData->fProcMinTime);
        a_pstBaseData->fProcTotalTime += fCost;
    }
    else
        iRet = -1;

    if (0 > iRet)
    {
        iIdle++;
        a_pstBaseData->lIdleTotalNum++;
    }
    else
    {
        /* 参考时间 */
        iIdle = 0;
        a_pstBaseData->lProcTotalNum++;
    }

    if (iIdle >= a_pstCtx->iIdleCount)
    {
        iIdle = 0;
        a_pstBaseData->lSleepTotalNum++;
        if (NULL == a_pstCtx->pfnIdle)
        {
            tapp_def_idle(a_pstCtx);
        }
        else
        {
            iRet = (*a_pstCtx->pfnIdle)(a_pstCtx, a_pvArg);
        }
    }

    return 0;
}

int tapp_def_idle(TAPPCTX* a_pstCtx)
{
    usleep(a_pstCtx->iIdleSleep * 1000);

    return 0;
}

int tapp_proc_tick(TAPPCTX* a_pstCtx, void* a_pvArg,
                   TAPP_BASEDATA *a_pstBaseData)
{
    int iMs;
    float fCost;
    struct timeval stEnd;
    struct timeval stSub;
    static time_t tLastRefreshTbus = 0;

    int iRet = 0;

    assert(NULL != a_pstCtx);
    assert(NULL != a_pstBaseData);

    gettimeofday(&a_pstCtx->stCurr, NULL);

    if (a_pstCtx->iUseBus)
    {
        tbus_set_current_time(&a_pstCtx->stCurr);
    }

    TV_DIFF(stSub, a_pstCtx->stCurr, gs_stCtxInternal.stLastTick);
    TV_TO_MS(iMs, stSub);

    if ((iMs >= 0) && (iMs < a_pstCtx->iTimer))
    {
        return 0;
    }

    /*回调业务tick*/
    if (NULL != a_pstCtx->pfnTick)
    {
        a_pstCtx->iTickCount++;
        a_pstBaseData->lTickTotalNum++;

        (*a_pstCtx->pfnTick)(a_pstCtx, a_pvArg);
        gettimeofday(&stEnd, NULL);

        TV_DIFF(stSub, stEnd, a_pstCtx->stCurr);
        fCost = (float) (stSub.tv_sec * 1000 + stSub.tv_usec / 1000.0);

        a_pstBaseData->fTickMaxTime = tmax(a_pstBaseData->fTickMaxTime , fCost);
        a_pstBaseData->fTickMinTime = tmin(a_pstBaseData->fTickMinTime ,fCost);
        a_pstBaseData->fTickTotalTime += fCost;

        gs_stCtxInternal.iTick++;

        tapp_clear_fini_thread(a_pstCtx,a_pvArg);
    }

    /*tbus 相关定时处理*/
    if (a_pstCtx->iUseBus)
    {
        if ((0 < a_pstCtx->iBus) &&
            ((a_pstCtx->stCurr.tv_sec - tLastRefreshTbus) >= gs_stCtxInternal.stOption.iRefeshTbusTimer))
        {
            tLastRefreshTbus = a_pstCtx->stCurr.tv_sec;
            iRet = tbus_refresh_handle(a_pstCtx->iBus);
        }

        tapp_check_bus_event(a_pstCtx, a_pvArg);

        tapp_send_bus_heartbeat(a_pstCtx);
    }

    /*定时输出运行数据*/
    tapp_proc_rundata(a_pstCtx, a_pvArg);

    /*检查线程状态*/
    if (a_pstCtx->iNthreads > 0)
    {
        tapp_check_threads(a_pstCtx);
    }

    TV_CLONE(gs_stCtxInternal.stLastTick, a_pstCtx->stCurr);

    return 0;
}

static int tapp_proc_rundata(TAPPCTX* a_pstCtx, void* a_pvArg)
{
    struct timeval stSub;
    int iMs;
    static struct timeval stBaseDataLast;
    static struct timeval stRunDataCumuLast;
    static struct timeval stRunDataStatusLast;
    TAPP_RUNDATA_TIMER *pstRunDataTimer = NULL;
    TAPP_BASEDATA *pstBaseData = NULL;

    assert(NULL != a_pstCtx);

    pstRunDataTimer
        = (TAPP_RUNDATA_TIMER *) gs_stCtxInternal.stRunDataTimer.pszBuff;
    if (NULL == pstRunDataTimer)
    {
        return 0;
    }
    if (a_pstCtx->iNotOutputStat)
    {
        return 0;
    }

    pstBaseData = (TAPP_BASEDATA *) gs_stCtxInternal.stBaseData.pszBuff;
    assert(NULL != pstBaseData);
    TV_DIFF(stSub, a_pstCtx->stCurr, stBaseDataLast);
    TV_TO_MS(iMs, stSub);
    if (iMs > pstRunDataTimer->stBasedata_timer.lGlobalTime || iMs < 0)
    {
        int64_t totalDisp = pstBaseData->lDispatchToThreadNum;
        int64_t totalRecv = pstBaseData->lRecvFromThreadNum;

        TV_CLONE(stBaseDataLast, a_pstCtx->stCurr);
        tapp_rundata_log(&gs_stCtxInternal.stBaseData);

        if (a_pstCtx->iNthreads > 0)
            tapp_collect_thread_rundata(a_pstCtx);

        memset(pstBaseData, 0, sizeof(*pstBaseData));
        pstBaseData->fProcMinTime = 1000000.0;
        pstBaseData->fTickMinTime = 1000000.0;

        pstBaseData->lDispatchToThreadNum = totalDisp;
        pstBaseData->lRecvFromThreadNum = totalRecv;
    }

    TV_DIFF(stSub, a_pstCtx->stCurr, stRunDataCumuLast);
    TV_TO_MS(iMs, stSub);
    if (iMs > pstRunDataTimer->stRundata_cumu_timer.lGlobalTime || iMs < 0)
    {
        TV_CLONE(stRunDataCumuLast, a_pstCtx->stCurr);
        tapp_rundata_log(&a_pstCtx->stRunDataCumu);
    }

    TV_DIFF(stSub, a_pstCtx->stCurr, stRunDataStatusLast);
    TV_TO_MS(iMs, stSub);
    if (iMs > pstRunDataTimer->stRundata_status_timer.lGlobalTime || iMs < 0)
    {
        TV_CLONE(stRunDataStatusLast, a_pstCtx->stCurr);
        tapp_rundata_log(&a_pstCtx->stRunDataStatus);
    }

    return 0;
}

TSF4G_API int tapp_def_mainloop(TAPPCTX* a_pstCtx, void* a_pvArg)
{
    TAPP_BASEDATA *pstBaseData = NULL;

    assert(a_pstCtx);

    pstBaseData = (TAPP_BASEDATA *) gs_stCtxInternal.stBaseData.pszBuff;
    assert(NULL != pstBaseData);
    gettimeofday(&a_pstCtx->stCurr, NULL);
    TV_CLONE(gs_stCtxInternal.stLastTick, a_pstCtx->stCurr);

    while (!gs_stTapp.iExitMainloop)
    {
        tapp_proc_ctrl_msg(a_pstCtx, a_pvArg);
        tapp_proc_exit_msg(a_pstCtx, a_pvArg);
        tapp_proc_reload(a_pstCtx, a_pvArg);
        tapp_proc_proc(a_pstCtx, a_pvArg, pstBaseData);
        tapp_proc_tick(a_pstCtx, a_pvArg, pstBaseData);
    }

    return gs_stTapp.iIsExit;
}

TSF4G_API int tapp_is_exit(void)
{
    return gs_stTapp.iIsExit;
}

TSF4G_API void tapp_exit_mainloop(void)
{
    gs_stTapp.iExitMainloop = 1;
}

LPTLOGCATEGORYINST tapp_must_get_category(const char *a_pszName)
{
    assert(NULL != a_pszName);

    if (NULL == gs_stTapp.pstLogBackCtx)
    {
        gs_stTapp.pstLogBackCtx
            = tlog_init_default(gs_stCtxInternal.pszLogFile);
    }

    if (!strcmp(a_pszName, TLOG_DEF_CATEGORY_DATAROOT))
    {
        return tlog_get_category(gs_stTapp.pstLogBackCtx, a_pszName);
    }

    return NULL;
}

TSF4G_API int tapp_get_category(IN const char* a_pszName,
                                OUT LPTLOGCATEGORYINST* a_ppiCatInst)
{
    int iRet = 0;

    TLOGCATEGORYINST* pstGet = NULL;

    if (!a_pszName || '\0' == a_pszName[0])
    {
        a_pszName = TLOG_DEF_CATEGORY_TEXTROOT;
    }

    pstGet = tlog_get_category(&gs_stTapp.stLogCtx, a_pszName);

    if (NULL == pstGet && (strcmp(a_pszName,"data")))
    {
        pstGet = tapp_must_get_category(a_pszName);
    }
    if (NULL != a_ppiCatInst)
        *a_ppiCatInst = pstGet;
    iRet = (NULL != pstGet) ? 0 : -1;
    return iRet;
}

TSF4G_API int tapp_get_bill_category(IN const char* a_pszName,
                           OUT LPTLOGCATEGORYINST* a_ppiCatInst)
{
    int iRet = 0;

    TLOGCATEGORYINST* pstGet = NULL;

    if (!a_pszName || '\0' == a_pszName[0])
    {
        a_pszName = TLOG_DEF_CATEGORY_TEXTROOT;
    }

    pstGet = tlog_get_category(&gs_stTapp.stLogCtx, a_pszName);
    if (NULL != a_ppiCatInst)
    {
        *a_ppiCatInst = pstGet;
    }

    iRet = (NULL != pstGet) ? 0 : -1;
    return iRet;
}

TSF4G_API const char* tapp_get_tdrfile(void)
{
    return gs_stCtxInternal.pszTdrFile;
}

TSF4G_API const char* tapp_get_pidfile(void)
{
    return gs_stCtxInternal.pszPidFile;
}

int tapp_set_log_module_name(char *a_pszName)
{
    assert(a_pszName);
    tlog_set_module_name(&gs_stTapp.stLogCtx, a_pszName);
    return 0;
}
