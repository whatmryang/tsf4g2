#ifndef _MTEXAMPLE_H
#define _MTEXAMPLE_H






#define EXAMPLE_MAX_THREADS 256
struct tagMTexampleEnv;
typedef struct tagMTexampleEnv		MTEXAMPLEENV;
typedef struct tagMTexampleEnv		*LPMTEXAMPLEENV;

struct tagMTexampleThreadEnv;
typedef struct tagMTexampleThreadEnv MTEXAMPLETHREADENV;
typedef struct tagMTexampleThreadEnv *LPMTEXAMPLETHREADENV;

#include "tapp/tapp.h"
#include "tdr/tdr.h"
#include "tlog/tlog.h"
#include "tloghelp/tlogload.h"
#include "mtexample_conf_desc.h"
#include "version.h"
#include "mtexample_cs.h"
#include "mtexample_ss.h"

#include "mtexample_task.h"
#include "mtexample_cs.h"
#include "mtexample_proc.h"
#include "mtexample_msgproc.h"
#include "mtexample_control.h"
#include "comm/tmempool.h"

extern TAPPCTX gs_stAppCtx;

struct tagTaskEnv
{
	char szClientBuff[1024];
	size_t  iClientBuff;
	MTEXAMPLEMSG stReq;
	int iSrc;

	MTEXAMPLEMSG stRes;
	char szToClientBuff[1024];
	size_t iToClientBuff;
};

struct tagMTexampleThreadEnv
{
	LPTDRMETA pstMetaMsg;
	LPTDRMETA pstMetaSS;

	MTSSDATA stData;

	char szClientBuff[1024];
	size_t  iClientBuff;
	MTEXAMPLEMSG stReq;
	MTEXAMPLEMSG stRes;
	int iSrc;
	char szToClientBuff[1024];
	size_t iToClientBuff;
};

struct tagMTexampleEnv
{
	MTEXAMPLECONF *pstConf;
	MTEXAMPLERUN_CUMULATE *pstRunCumu;
	MTEXAMPLERUN_STATUS   *pstRunStat;
	TLOGCATEGORYINST* pstLogCat;

	LPTDRMETA pstMetaMsg;
	LPTDRMETA pstMetaSS;

	MTEXAMPLETHREADENV astThreadEnv[EXAMPLE_MAX_THREADS];
	LPTAPPTHREADCTX apstThreadCtx[EXAMPLE_MAX_THREADS];
	int inThread;
	uint32_t iTick;

	char szClientBuff[1024];
	size_t  iClientBuff;
	MTEXAMPLEMSG stReq;
	int iSrc;

	MTEXAMPLEMSG stRes;
	char szToClientBuff[1024];
	size_t iToClientBuff;

	LPTMEMPOOL pstTaskPool;
};

int mtexample_init_env(TAPPCTX *pstAppCtx, MTEXAMPLEENV *pstEnv);

int mtexample_init(TAPPCTX *pstAppCtx, MTEXAMPLEENV *pstEnv);
int mtexample_proc(TAPPCTX *pstAppCtx, MTEXAMPLEENV *pstEnv);
int mtexample_reload(TAPPCTX *pstAppCtx, MTEXAMPLEENV *pstEnv);
int mtexample_fini(TAPPCTX *pstAppCtx, MTEXAMPLEENV *pstEnv);
int mtexample_tick(TAPPCTX *pstAppCtx, MTEXAMPLEENV *pstEnv);
int mtexample_stop(TAPPCTX *pstAppCtx, MTEXAMPLEENV *pstEnv);
int mtexample_create_thread(TAPPCTX *pstAppCtx, MTEXAMPLEENV *pstEnv);
int example_proc_binary_req(TAPPCTX *pstAppCtx,
     void *pstEnv, const char* pszMetaName, const char* pBuf, int iLen);
const char* mtexample_console_help(void);
int mtexample_proc_cmdline_req(TAPPCTX *pstAppCtx, void *pvData, unsigned short argc, const char** argv);

int mtexample_controller_init(TAPPCTX *pstAppCtx, void *argv);
int mtexample_proc_binary_res(const char* pszMetaName, const char* pBuf, int tLen);
int mtexample_proc_stdin(unsigned short argc, const char** argv);
int example_controller_fini(TAPPCTX *pstAppCtx, void *argv);

#endif

