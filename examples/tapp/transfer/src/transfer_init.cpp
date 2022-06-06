/*
 * Copyright (c) 2010, 腾讯科技有限公司互动娱乐研发部
 * All rights reserved
 *
 * File name:       transfer_init.cpp
 * Description:     definition for functions about initiation
 * Version:         1.0
 *
 * Author:          flyma.IERD.IED.tencent
 * Creation time:   2010-06-03 14:26
 * Last modified:   2010-06-09 10:09
 *
 * History:
 * <date>        <author>      <version>   <description>
 * 2010-06-03     flyma         1.0         creation
 *
 */

#include "transfer.h"
#include "transfer_conf_desc.h"
#include "tloghelp/tlogload.h"
#include "tconnapi/tconnapi.h"
#include "transfer_session_mgr.h"
#include "transfer_session_base.h"
#include "transfer_session_dirtycheck.h"

static int transfer_init_env(TAPPCTX* a_pstTappCtx, TRANSFERENV* a_pstEnv);
static int transfer_init_connection(TAPPCTX* a_pstTappCtx, TRANSFERENV* a_pstEnv);



int transfer_init(TAPPCTX* a_pstTappCtx, TRANSFERENV* a_pstEnv)
{
    assert(NULL != a_pstTappCtx);
    assert(NULL != a_pstEnv);

    int iRet = 0;

    iRet = transfer_init_env(a_pstTappCtx, a_pstEnv);
    if (0 > iRet)
    {
        return -1;
    }

    iRet = transfer_init_connection(a_pstTappCtx, a_pstEnv);
    if (0 > iRet)
    {
        tlog_error(a_pstEnv->pstLogCat, 0, 0, "transfer_init_connection failed");
        return -1;
    }

    //TODO 时间服务器可能存在多种功能不同的事务处理，这里以
    //DirtyCheckSession来说明事务对象的使用
    iRet = SessionMgr<DirtyCheckSession>::getInstance()->
        init(a_pstEnv->pstConf->dwMaxDirtyCheckSessionNum,
             a_pstEnv->pstConf->dwCheckNumPerTick,
             a_pstEnv->pstConf->dwSessionTimeout);
    if (0 > iRet)
    {
        tlog_error(a_pstEnv->pstLogCat, 0, 0, "failed to init sesseion pool");
        return -1;
    }

    // TODO
    // add code here to do any other initiation

    printf("transfer start\n");
    tlog_info(a_pstEnv->pstLogCat, 0, 0, "transfer start");

    return iRet;
}

int transfer_init_env(TAPPCTX* a_pstTappCtx, TRANSFERENV* a_pstEnv)
{
    assert(NULL != a_pstTappCtx);
    assert(NULL != a_pstEnv);

    int iRet = 0;

    if (0 > tapp_get_category(TLOG_DEF_CATEGORY_TEXTROOT,
                              &a_pstEnv->pstLogCat))
    {
        printf("tapp_get_category run fail\n");
        return -1;
    }

    if (NULL == a_pstTappCtx->pszConfFile )
    {
        printf("no configure file\n");
        tlog_error(a_pstEnv->pstLogCat, 0, 0, "no configure file");
        return -1;
    }

    a_pstEnv->pstConf = (TRANSFERCONF* )a_pstTappCtx->stConfData.pszBuff;
    if (NULL == a_pstEnv->pstConf)
    {
        printf("a_pstEnv->pstConf is NULL\n");
        tlog_error(a_pstEnv->pstLogCat, 0, 0, "a_pstEnv->pstConf is NULL\n");
        return -1;
    }

    a_pstEnv->pstRunCumu = (TRANSFERRUN_CUMULATE* )a_pstTappCtx->stRunDataCumu.pszBuff;
    if (NULL == a_pstEnv->pstRunCumu)
    {
        printf("a_pstEnv->pstRunCumu is NULL\n");
        tlog_error(a_pstEnv->pstLogCat, 0, 0, "a_pstEnv->pstRunCumu is NULL\n");
        return -1;
    }

    a_pstEnv->pstRunStat = (TRANSFERRUN_STATUS* )a_pstTappCtx->stRunDataStatus.pszBuff;
    if (NULL == a_pstEnv->pstRunStat)
    {
        printf("a_pstEnv->pstRunStat is NULL\n");
        tlog_error(a_pstEnv->pstLogCat, 0, 0, "a_pstEnv->pstRunStat is NULL\n");
        return -1;
    }

    LPTDRMETALIB pstMetaLib = NULL;
    pstMetaLib = (LPTDRMETALIB)g_szMetalib_ForeProto;
    a_pstEnv->pstRequestMeta = tdr_get_meta_by_name(pstMetaLib, TCONNDPROTOMETA);
    if (NULL == a_pstEnv->pstRequestMeta)
    {
        tlog_error(a_pstEnv->pstLogCat, 0, 0, "failed to get meta for tconnd-protocol");
        return -1;
    }

    pstMetaLib = (LPTDRMETALIB)g_szMetalib_BackProto;
    a_pstEnv->pstResponseMeta = tdr_get_meta_by_name(pstMetaLib, BACKSVRPROTOMETA);
    if (NULL == a_pstEnv->pstResponseMeta)
    {
        tlog_error(a_pstEnv->pstLogCat, 0, 0, "failed to get meta for backsvr-protocol");
        return -1;
    }

    a_pstEnv->iBusHandle = a_pstTappCtx->iBus;
    a_pstEnv->iBusAddr   = a_pstTappCtx->iId;

    return iRet;
}

int transfer_init_connection(TAPPCTX* a_pstTappCtx, TRANSFERENV* a_pstEnv)
{
    assert(NULL != a_pstTappCtx);
    assert(NULL != a_pstEnv);

    int iRet = 0;

    TBUSADDR iTconndAddr;
    iRet = tbus_addr_aton(a_pstEnv->pstConf->stTconnd.szBusAddr, &iTconndAddr);
    if (TBUS_ERR_IS_ERROR(iRet))
    {
        tlog_error(a_pstEnv->pstLogCat, 0, 0, "tbus_addr_aton failed, addr<%s>",
                   a_pstEnv->pstConf->stTconnd.szBusAddr);
        return -1;
    }

    iRet = TconndHandler::getInstance()->init(a_pstTappCtx->pszGCIMKey,
                                              a_pstTappCtx->iBusinessID,
                                              (TBUSADDR)a_pstTappCtx->iId, iTconndAddr);
    if (0 > iRet)
    {
        tlog_error(a_pstEnv->pstLogCat, 0, 0, "tconndHandler.init() failed");
        return -1;
    }

    iRet = tbus_peer_ctrl(a_pstTappCtx->iBus, iTconndAddr, TBUS_MODE_DISABLE, TBUS_PEER_CTRL_ALL);
    if (TBUS_ERR_IS_ERROR(iRet))
    {
        tlog_error(a_pstEnv->pstLogCat, 0, 0, "tbus_peer_ctrl failed, for %s",
                   tbus_error_string(iRet));
        return -1;
    }

    return iRet;
}
