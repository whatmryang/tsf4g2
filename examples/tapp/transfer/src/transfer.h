/*
 * Copyright (c) 2010, 腾讯科技有限公司互动娱乐研发部
 * All rights reserved
 *
 * File name:       transfer.h
 * Description:     include, declaration
 * Version:         1.0
 *
 * Author:          flyma.IERD.IED.tencent
 * Creation time:   2010-06-03 09:11
 * Last modified:   2010-06-09 10:24
 *
 * History:
 * <date>        <author>      <version>   <description>
 * 2010-06-03     flyma         1.0         creation
 *
 */

#ifndef _TAPP_TRANSFER_H_
#define _TAPP_TRANSFER_H_

#include "tdr/tdr.h"
#include "tapp/tapp.h"
#include "tlog/tlog.h"
#include "tbus/tbus.h"
#include "tconnapi/tconnapi.h"
#include "transfer_conf_desc.h"
#include "transfer_tconnd_handle.h"

#define TCONNDPROTOMETA   "ForePkg"
#define BACKSVRPROTOMETA  "BackPkg"
#define BACKPROTOHEADMETA "BackPkgHead"
#define BACKPROTOBODYMETA "BackPkgBody"

struct tagtransferEnv
{
    TRANSFERCONF* pstConf;
    TRANSFERRUN_CUMULATE* pstRunCumu;
    TRANSFERRUN_STATUS* pstRunStat;

    TLOGCATEGORYINST* pstLogCat;

    LPTDRMETA pstRequestMeta;
    LPTDRMETA pstResponseMeta;

    int iBusHandle;
    TBUSADDR iBusAddr;

    TBUSADDR iDirtyCheckAddr;
    /* add other background servers' tbus addresses here */
};

typedef struct tagtransferEnv TRANSFERENV;
typedef struct tagtransferEnv* LPTRANSFERENV;

extern TRANSFERENV g_stEnv;
extern unsigned char g_szMetalib_transfer[];
extern unsigned char g_szMetalib_ForeProto[];
extern unsigned char g_szMetalib_BackProto[];

int transfer_init(TAPPCTX* a_pstTappCtx, TRANSFERENV* a_pstEnv);
int transfer_proc(TAPPCTX* a_pstTappCtx, TRANSFERENV* a_pstEnv);
int transfer_reload(TAPPCTX* a_pstTappCtx, TRANSFERENV* a_pstEnv);
int transfer_fini(TAPPCTX* a_pstTappCtx, TRANSFERENV* a_pstEnv);
int transfer_tick(TAPPCTX* a_pstTappCtx, TRANSFERENV* a_pstEnv);
int transfer_stop(TAPPCTX* a_pstTappCtx, TRANSFERENV* a_pstEnv);

int transfer_service_init(TAPPCTX *a_pstAppCtx, TRANSFERENV *a_pstEnv, int argc, char* argv[]);

#endif
