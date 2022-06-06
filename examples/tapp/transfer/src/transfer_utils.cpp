/*
 * Copyright (c) 2010, 腾讯科技有限公司互动娱乐研发部
 * All rights reserved
 *
 * File name:       transfer_utils.cpp
 * Description:     definition for utility functions
 * Version:         1.0
 *
 * Author:          flyma.IERD.IED.tencent
 * Creation time:   2010-06-07 17:44
 * Last modified:   2010-06-07 17:44
 *
 * History:
 * <date>        <author>      <version>   <description>
 * 2010-06-07     flyma         1.0         creation
 *
 */

#include "transfer.h"
#include "tlog/tlog.h"
#include "transfer_utils.h"

int TransferUtil::decodeMsg(LPTDRMETA a_pstMeta,
                            char* a_pHostBuff, int* a_piHostBuff,
                            const char* a_pNetBuff, int* a_piNetBuff)
{
    assert(NULL != a_pstMeta);
    assert(NULL != a_pHostBuff);
    assert(NULL != a_pNetBuff);
    assert(NULL != a_piHostBuff);
    assert(NULL != a_piNetBuff);

    int iRet = 0;

    TDRDATA stNetData;
    stNetData.pszBuff  = (char*)a_pNetBuff;
    stNetData.iBuff    = (size_t)*a_piNetBuff;
    TDRDATA stHostData;
    stHostData.pszBuff = (char*)a_pHostBuff;
    stHostData.iBuff   = (size_t)*a_piHostBuff;

    iRet = tdr_ntoh(a_pstMeta, &stHostData, &stNetData, 0);
    tlog_debug_dr(g_stEnv.pstLogCat, 0, 0, a_pstMeta,stHostData.pszBuff ,stHostData.iBuff ,0);
    if (TDR_ERR_IS_ERROR(iRet))
    {
        tlog_error(g_stEnv.pstLogCat, 0, 0, "tdr_hton failed, iHBuff<%d>, iNBuff<%d> for %s",
                   *a_piHostBuff, *a_piNetBuff, tdr_error_string(iRet));
        return -1;
    }
    tlog_debug_dr(g_stEnv.pstLogCat, 0, 0, a_pstMeta,stHostData.pszBuff ,stHostData.iBuff ,0);
    *a_piNetBuff  = stNetData.iBuff;
    *a_piHostBuff = stHostData.iBuff;

    return iRet;
}

int TransferUtil::encodeMsg(LPTDRMETA a_pstMeta,
                            char* a_pNetBuff, int* a_piNetBuff,
                            const char* a_pHostBuff, int* a_piHostBuff)
{
    assert(NULL != a_pstMeta);
    assert(NULL != a_pHostBuff);
    assert(NULL != a_pNetBuff);
    assert(NULL != a_piHostBuff);
    assert(NULL != a_piNetBuff);

    int iRet = 0;

    TDRDATA stNetData;
    stNetData.pszBuff  = (char*)a_pNetBuff;
    stNetData.iBuff    = (size_t)*a_piNetBuff;
    TDRDATA stHostData;
    stHostData.pszBuff = (char*)a_pHostBuff;
    stHostData.iBuff   = (size_t)*a_piHostBuff;

    iRet = tdr_hton(a_pstMeta, &stNetData, &stHostData, 0);
    tlog_debug_dr(g_stEnv.pstLogCat, 0, 0, a_pstMeta,stHostData.pszBuff ,stHostData.iBuff ,0);
    if (TDR_ERR_IS_ERROR(iRet))
    {
        tlog_error(g_stEnv.pstLogCat, 0, 0, "tdr_ntoh failed, iHBuff<%d>, iNBuff<%d> for %s",
                   *a_piHostBuff, *a_piNetBuff, tdr_error_string(iRet));
        return -1;
    }
    *a_piNetBuff  = stNetData.iBuff;
    *a_piHostBuff = stHostData.iBuff;

    return iRet;
}


int TransferUtil::sendPkgToTconnd(TRANSFERENV* a_pstEnv, const FOREPKG* a_pstPkg,
                                  const TFRAMEHEAD* a_pstFrameHead)
{
    assert(NULL != a_pstEnv);
    assert(NULL != a_pstPkg);
    assert(NULL != a_pstFrameHead);

    int iRet = 0;

    /* encode msg */
    char szNetBuff[sizeof(FOREPKG)*2];
    int iNetBuff     = (int)sizeof(szNetBuff);
    int iHostBuff    = (int)sizeof(FOREPKG);
    iRet = TransferUtil::encodeMsg(a_pstEnv->pstRequestMeta,
                                   (char*)szNetBuff, &iNetBuff,
                                   (char*)a_pstPkg, &iHostBuff);
    if (TDR_ERR_IS_ERROR(iRet))
    {
        tlog_error(a_pstEnv->pstLogCat, 0, 0, "encodeMsg to tconnd failed");
        return -1;
    }

    /* send to tconnd */
    iRet = TconndHandler::getInstance()->sendMsg(szNetBuff, iNetBuff, a_pstFrameHead);
    if (0 > iRet)
    {
        tlog_error(a_pstEnv->pstLogCat, 0, 0, "sendMsg to tconnd failed");
        return -1;
    }

    return iRet;
}

int TransferUtil::sendPkgToServer(TRANSFERENV* a_pstEnv,
                                  const BACKPKG* a_pstPkg, TBUSADDR a_iDstAddr)
{
    assert(NULL != a_pstEnv);
    assert(NULL != a_pstPkg);

    int iRet = 0;

    /* encode msg */
    char szNetBuff[sizeof(BACKPKG)*2];
    int iNetBuff     = (int)sizeof(szNetBuff);
    int iHostBuff    = (int)sizeof(BACKPKG);
    iRet = TransferUtil::encodeMsg(a_pstEnv->pstResponseMeta,
                                   (char*)szNetBuff, &iNetBuff,
                                   (char*)a_pstPkg, &iHostBuff);
    if (TDR_ERR_IS_ERROR(iRet))
    {
        tlog_error(a_pstEnv->pstLogCat, 0, 0, "encodeMsg to dirtycheck-svr failed");
        return -1;
    }

    /* send to bus channel */
    TBUSADDR iSrcAddr = a_pstEnv->iBusAddr;
    TBUSADDR iDstAddr = (TBUSADDR)a_iDstAddr;
    iRet = tbus_send(a_pstEnv->iBusHandle, &iSrcAddr, &iDstAddr, szNetBuff, iNetBuff, 0);
    if (TBUS_ERR_IS_ERROR(iRet))
    {
        tlog_error(a_pstEnv->pstLogCat, 0, 0, "tbus_send failed, src<%d>, dst<%d>, for %s",
                   iSrcAddr, iDstAddr, tbus_error_string(iRet));
        return -1;
    }

    return iRet;
}
