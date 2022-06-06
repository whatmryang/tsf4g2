/***********************************************************
 * FileName: tapp_i.h
 * Author: flyma
 * Version: 1.0
 * Data: 2009-9-27
 * Function List: none
 * History:
 * <author>     <time>      <version>   <desc>
 * flyma        2009-9-27   1.0         create this file
***********************************************************/

#ifndef TAPP_I_H
#define TAPP_I_H



#define TAPP_DEF_TIMER		50

#if defined(_WIN32) || defined(_WIN64)
#define TAPP_DEF_BUSDIR		"C:\\Temp"
#else
#define TAPP_DEF_BUSDIR		"/usr/local/bus"
#endif

#define TAPP_DEFAULT_METABASE_KEY "22001"


#define TAPP_DEFAULT_TIMER			10
#define TAPP_DEFAULT_KILL_WAIT		10
#define TAPP_DEFAULT_EPOLL_WAIT		10
#define TAPP_DEFAULT_IDLE_COUNT		10
#define TAPP_DEFAULT_IDLE_SLEEP		10

#define TAPP_DEFAULT_BUS_HEARTBEAT_GAP 10
#define TAPP_DEFAULT_BUS_TIMEOUT_GAP 3 * TAPP_DEFAULT_BUS_HEARTBEAT_GAP
#define TAPP_DEFAULT_BUS_CHECKPEER_GAP 3

#define TAPP_BUS_MIN_HEARTBEAT_GAP  1
#define TAPP_BUS_MIN_CHECK_PEER_GAP 1

//tapp进程启动、停止、控制及重载配置相关命令
#define TAPP_START             "start"	/**<启动进程命令，如果之前已启动相同程序，则直接退出*/
#define TAPP_RESTART           "restart"	/**<重启进程，停掉前一个进程，再启动*/
#define TAPP_STOP              "stop"		/**<正常停止进程*/
#define TAPP_KILL              "kill"		/**<强制退出进程*/
#define TAPP_RELOAD            "reload" /**<进程重现加载配置*/
#define TAPP_CONTROL           "control"	/**<作为控制端启动，通过控制通道向进程发出控制命令*/
#if defined(_WIN32) || defined(_WIN64)
#define TAPP_INSTALL           "install"
#define TAPP_UNINSTALL         "uninstall"
#endif

#include "tlog/tlog.h"
#include "../tlog/tlog_i.h"
#include "tbus/tbus.h"
#include "tapp/tapp.h"
#include "../../lib_src/tbus/tbus_config_mng.h"
#include "comm/tmempool.h"
#include "../lib_src/comm/comm_i.h"

#define TAPP_EXIT_QUIT		0x01
#define TAPP_EXIT_STOP		0x02

#define TAPP_INJECT_PRO_UNIX        0x01

/* if you use related module, you need to define the relative macro */

/* libtbus : TAPP_TBUS */
/* libtdr : TAPP_TDR */
/* libtdr_xml: TAPP_TDR_XML */

#define TAPP_TDR

#define TAPP_TBUS
#define TAPP_TDR_XML
#define TAPP_TLOG

#include "pal/pal.h"

#ifdef TAPP_TDR
#include "tdr/tdr.h"
#endif /* TAPP_TDR */

#ifdef TAPP_TBUS
#include "tbus/tbus.h"
#endif /* TAPP_TBUS */

#include "../lib_src/tmng/err_stack.h"
#include "../lib_src/tmng/tsm_tdr.h"

#ifdef TAPP_TLOG
#include "tlog/tlog.h"
#include "tloghelp/tlogload.h"
#endif

#include "taa/tagentapi.h"
#include "tapp_basedata_def.h"
#include "tapp_rundata_timer_def.h"

#include "../lib_src/comm/tcommshm.h"

#define TAPP_DOMAIN_BASE     "base"
#define TAPP_DOMAIN_CFG     "cfg"
#define TAPP_DOMAIN_CFGPRE   "cfgpre"
#define TAPP_DOMAIN_CFGBAK     "cfgbak"
#define TAPP_DOMAIN_RT_STATUS        "runtime_status"
#define TAPP_DOMAIN_RT_CUMU    "runtime_cumulate"
#define TAPP_DOMAIN_RT_TIMER "runtime_timer"
#define TAPP_DOMAIN_LOGCFG     "logcfg"
#define TAPP_DOMAIN_LOGCFGPRE    "logcfg_pre"
#define TAPP_DOMAIN_LOGCFGBAK     "logcfg_bak"

#define TAPP_MAJOR            0x00
#define TAPP_MINOR            0x00
#define TAPP_REV            0x01
#define TAPP_BUILD            0x01

#define TAPP_DEF_VER        TAPP_MAKE_VERSION(TAPP_MAJOR, TAPP_MINOR, TAPP_REV, TAPP_BUILD)

#define CLEARTAIL(str) (str)[sizeof(str)-1]='\0'

#define TAPP_BUILD            0x01

#define TAPP_CTRL_ON_OR_OFF       532
#define TAPP_CTRL_ADDR     		  542
#define TAPP_START_MODE_OPT       552
#define TAPP_OUTPUT_STAT_OPT      562
#define TAPP_BUS_TIMEOUT_GAP      572
#define TAPP_BUS_USE_SYS_TIME     582
#define TAPP_BUS_HEARTBEAT_GAP    592
#define TAPP_BUS_CHECK_PEER_GAP   602
#define TAPP_NO_STD_OUTPUT_OPT    612

#define TAPP_USE_TSM_CONF         701
#define TAPP_USE_TSM_LOG_CONF         702
#define TAPP_USE_TSM_RUNTIMELOG_CONF         703


#define TAPP_CTRL_MSG_VERSION 1

#define TAPP_BASEDATA_TIMER 300000
#define TAPP_RUNDATA_STATUS_TIMER     300000
#define TAPP_RUNDATA_CUMU_TIMER     300000

#define TAPP_DEF_INITCALLED 1
#define TAPP_THREAD_INIT_CALLED 2

enum tapgTappOperateCmd
{
	TAPP_OPERATE_CMD_NONE	= 0,	/**<无效操作命令*/
	TAPP_OPERATE_CMD_START,	/**<启动命令*/
	TAPP_OPERATE_CMD_RESTART,	/**<重启命令*/
	TAPP_OPERATE_CMD_STOP,	/**<停止命令*/
	TAPP_OPERATE_CMD_KILL,	/**<强制停止命令*/
	TAPP_OPERATE_CMD_RELOAD,	/**<重载配置命令*/
	TAPP_OPERATE_CMD_CONTROL,	/**<作为tapp控制端启动*/
#if defined(_WIN32) || defined(_WIN64)
    TAPP_OPERATE_CMD_INSTALL,
    TAPP_OPERATE_CMD_UNINSTALL,
#endif
};
typedef enum tapgTappOperateCmd TAPPOPERATECMD;


#ifdef __cplusplus
extern "C"
{
#endif

struct tagTappOption
{
	int iRefeshTbusTimer;	/*timer for refresh tbus channel configure*/
	int iConfigFileFormat;	/*xml File format*/
};
typedef struct tagTappOption	TAPPOPTION;
typedef struct tagTappOption	*LPTAPPOPTION;

#define TAPP_META_NAME_LEN	128

struct tagTappGs
{
    int iIsExit;
    int iReload;
    int iExitMainloop;
    char szPidFile[PATH_MAX];
    char szCIAddr[PATH_MAX];
    char szLogFile[PATH_MAX];
    char szConfFile[PATH_MAX];
    char szConfBakFile[PATH_MAX];
    char szLogConfFile[PATH_MAX];
    char szLogConfBakFile[PATH_MAX];
    char szRunDataTimerConfFile[PATH_MAX];
    char szDefRunDataStatusMeta[TAPP_META_NAME_LEN];
    char szDefRunDataCumuMeta[TAPP_META_NAME_LEN];

#ifdef TAPP_TLOG
    TLOGCTX stLogCtx;
    LPTLOGCTX pstLogBackCtx;
    TLOGCATEGORYINST *pstAppCatText;
    TLOGCATEGORYINST *pstAppCatData;
#endif
};
typedef struct tagTappGs TAPP_GS;
typedef struct tagTappGs *LPTAPP_GS;

#define TAPP_THREAD_MAX_THREAD_NUM 65535
#define TAPP_MAIN_THREAD_ID TAPP_THREAD_MAX_THREAD_NUM


struct tappThreadInnerCtx
{
	//This should be the first element of the Ctx.
	TAPPTHREADCTX stThreadCtx;
	void *pvData;
	int iWatchDogEnable;
	int iWatchDogCounter;
	TAPPTHREADCONF stConf;
	LPTBUSCHANNEL	pstChannelMainToThread;
	void *pvRingBuff;
	int iExitMainLoop;
	TAPP_THREAD_STATUS iFsm;

	int iNeedReload;


	int iNeedThreadRunData;
	//
	TAPP_THREAD_BASEDATA stInnerRunData;
	TAPP_THREAD_BASEDATA stToMainRunData;
	char szRunDataMetaName[128];
	TAPPDATA stMainRunData;
	LPTAPPCTX pstAppCtx;
	int iLastTick;
	int iIdleCount;


};

typedef struct tappThreadInnerCtx TAPPTHREADINNERCTX;
typedef struct tappThreadInnerCtx *LPTAPPTHREADINNERCTX;


struct tappThreadInternelData
{
	LPTMEMPOOL pstThreadPool;
	void *pvChannelBuff;
	int iBlockSize;
	int iNeedFree;
	int iNeedInitChannel;
	LPTCOMMSHM pstShm;

	int iTbusHeadLen;
	int szTbusHead[1024];

    int iLastRecvChannelIdx;
    int iChannelPkgNumRecved;
};

typedef struct tappThreadInternelData TAPPTHREADINTERNELDATA;
typedef struct tappThreadInternelData *LPTAPPTHREADINTERNELDATA;


struct tagTAPPCTX_INTERNAL
{
	int iWait;		/* the seconds to wait the previous to exit. */

	struct timeval stLastTick;

	const char* pszLogFile;
	const char* pszLogLevel;
	const char* pszPidFile;
	const char* pszTdrFile;
	const char* pszRundataTimerConf;    /*run data timer cfg file path*/

    LPTDRMETA pstCtrlMeta; /* record tapp-ctrl protocol's meta */
    const char* pszCIAddr;
    int iCtrlOn; /* with tapp-ctrl on or off */


    int iIsDaemon;

#if !defined(_WIN32) && !defined(_WIN64)
    int iNoStdOutput;
#endif

	int iNeedFreeLib;

	int iUseTsmCnf;	/*whether load cfg from tsm*/
	int iUseTsmLogCfg; /*whether load log cfg from tsm*/
	int iUseTsmRuntimerLogCfg; /*whether load runtimer log cfg from tsm*/

	int iUseTsm;
	void* iTsm;

	time_t tLogConfMod;
	time_t tConfMod; /* last modifed time of conf */
	time_t tRunTimerConfMod;

	TAPPDATA stBaseData; /*base run time data*/
	TAPPDATA stConfBackupData; /*after reload , old conf data in here*/
	TAPPDATA stRunDataTimer; /*run time data timer*/
	TAPPDATA stLogConfData; /*log conf data*/
	TAPPDATA stLogConfPrepareData; /*when reload, can use this for prepare load log conf */
	TAPPDATA stLogConfBackupData; /*after reload , old log conf data in here*/
	TAPPOPTION stOption;	/*the option by command line*/

	TAPPOPERATECMD	eOpCmd;	/*进程控制命令*/

	/*I have to get the cmd meta and allocate the buffer for it*/
	int iIsInJect;
	char *pszInJectCmd;
	size_t iInJectCmdLen;

	uint32_t iTick;

	/* The timeout for waiting threads init.*/
	int iThreadWaitInitTimeOut;

	/* Whether to use shm as the channel between main thread and worker thread.*/
    int iUseShmChannel;
	// Default we haveto reset all the shm. and reinit all the data
	// From ti.
	int iResetShm;

	const char *pszBillConfFile;
	int iBillSizeLimit;
	int iBillPrecision;

	TAPPTHREADINTERNELDATA stAppThreadCtxInternal;
};
typedef struct tagTAPPCTX_INTERNAL TAPPCTX_INTERNAL;
typedef struct tagTAPPCTX_INTERNAL *LPTAPPCTX_INTERNAL;



//extern TAPPCTX_INTERNAL gs_stCtxInternal;


extern TAPPCTX_INTERNAL gs_stCtxInternal;
extern TAPP_GS gs_stTapp ;


extern unsigned char g_szMetalib_tapp_basedata_def[];
extern unsigned char g_szMetalib_tapp_rundata_timer_def[];
extern unsigned char g_szMetalib_tapp_ctrl[];



/**
*	@brief reload conf data
*	@param pstCtx, pvArg.
*	@retval 0 no error happened.
*	@retval !0 error happend.
*/
TSF4G_API int tapp_def_reload(TAPPCTX* pstCtx, void* pvArg);

//控制通道初始化
int tapp_wait_thread_exit(TAPPCTX *pstCtx);
int tapp_check_threads(TAPPCTX *a_pstCtx);
int tapp_clear_fini_thread(TAPPCTX *pstCtx,void *pvArgMain);
int tapp_init_thread_conf(TAPPCTX *a_pstCtx);

int tapp_collect_thread_rundata(TAPPCTX *pstCtx);

int tapp_init_ctrlchannel(TAPPCTX* a_pstCtx, IN int a_iIsCtrlor);
int tapp_thread_init_done(TAPPCTX *pstCtx);
int tapp_thread_watchdog_check(TAPPCTX *pstCtx);
int tapp_controller_init(TAPPCTX  *a_pstCtx, void *a_pvArg);
int tapp_controller_fini(TAPPCTX  *a_pstCtx, void *a_pvArg);
int tapp_fini_data(TAPPCTX* a_pstCtx, void* a_pvArg);
int tapp_thread_proc_watchdog(TAPPTHREADINNERCTX *pstInnerCtx);
LPTLOGCATEGORYINST tapp_must_get_category(const char *a_pszName);
int tapp_def_opt(TAPPCTX* a_pstCtx, void* a_pvArg);
int tapp_init_tsm(TAPPCTX* a_pstCtx, void* a_pvArg);
int tapp_set_pid_file(TAPPCTX* a_pstCtx, const char *a_pszFileBase);
int tapp_init_bus(TAPPCTX* a_pstCtx, void* a_pvArg);
int tapp_reload_log(TAPPCTX* a_pstCtx, void* a_pvArg);
void tapp_on_sigusr1(int a_iSig);
void tapp_on_sigterm(int a_iSig);
int tapp_fini_log(TAPPCTX* a_pstCtx, void* a_pvArg);
void tapp_on_sigusr2(int a_iSig);
int tapp_def_usage(int argc, char* argv[] );
int tapp_init_tdr(TAPPCTX* a_pstCtx, void* a_pvArg);
int tapp_do_generate(TAPPCTX* a_pstCtx, void* a_pvArg);
int tapp_fini_tdr(TAPPCTX* a_pstCtx, void* a_pvArg);
int tapp_fini_tsm(TAPPCTX* a_pstCtx, void* a_pvArg);
int tapp_init_log(TAPPCTX* a_pstCtx, const char* a_pszConfFileBase);
int tapp_init_rundata_timer(TAPPCTX* a_pstCtx, void* a_pvArg);
int tapp_init_data(TAPPCTX* a_pstCtx, void* a_pvArg);
int tapp_init_threads(TAPPCTX* a_pstCtx, void* a_pvArg);
void *tapp_thread_function(void *pvData);
int tapp_theads_init_conf(TAPPCTX *a_pstCtx);
int tapp_create_thread_single(TAPPCTX *pstCtx,TAPPTHREADCONF *pstThreadConf,void *a_pvArg,LPTAPPTHREADCTX *ppstThread);
int tapp_thread_proc_reload(TAPPTHREADINNERCTX *pstInnerCtx);
int tapp_thread_proc_proc(TAPPTHREADINNERCTX *pstInnerCtx);
int tapp_reload_thread(TAPPCTX *pstCtx);
int tapp_def_thread_init(TAPPCTX* pstCtx, void* pvArg);

void *tapp_get_thread_channel_buffer(TAPPCTX* pstCtx,int iThreadIdx);
int tapp_have_threads_in_stats(TAPPCTX *pstCtx,int iStatus);
int tapp_are_threads_in_stats(TAPPCTX *pstCtx,int iStatus);
int tapp_rundata_log(TAPPDATA *a_pstTappData);
int tapp_thread_proc_rundata(TAPPTHREADINNERCTX *pstInner);

int tapp_thread_fini_user_rundata(TAPPDATA *pstTappData);
int tapp_def_thread_fini(TAPPCTX* pstCtx, void* pvArg);
int tapp_thread_set_exit_flag(TAPPCTX *pstCtx);

void tapp_on_reload();
#ifdef __cplusplus
}
#endif

#endif /* TAPP_I_H */
