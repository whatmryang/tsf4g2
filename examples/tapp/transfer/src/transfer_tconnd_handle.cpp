/*
 * Copyright (c) 2010, 腾讯科技有限公司互动娱乐研发部
 * All rights reserved
 *
 * File name:       transfer_tconnd_handle.cpp
 * Description:     definition for functions of class TconndHandler
 * Version:         1.0
 *
 * Author:          flyma.IERD.IED.tencent
 * Creation time:   2010-06-03 17:49
 * Last modified:   2010-06-04 14:22
 *
 * History:
 * <date>        <author>      <version>   <description>
 * 2010-06-03     flyma         1.0         creation
 *
 */

#include "transfer.h"
#include "tbus/tbus.h"
#include "transfer_tconnd_handle.h"


TconndHandler* TconndHandler::s_instance = NULL;

TconndHandler::TconndHandler()
{
    m_deleteBusMsg    = true;
    m_hasInited       = false;
    m_hasPeeked       = false;
}

TconndHandler::~TconndHandler()
{
    tbus_delete(&m_iBusHandler);
}

int TconndHandler::init(const char* a_pszGCIMKey,
                        const int a_iBusineessID,
                        TBUSADDR a_iLocalAddr,
                        TBUSADDR a_iTconndAddr)
{
    int iRet = 0;

    m_pstLogCat = g_stEnv.pstLogCat;
    assert(NULL != m_pstLogCat);

    m_iLocalAddr   = a_iLocalAddr;
    m_iPeerAddr    = a_iTconndAddr;

    iRet = tconnapi_initialize(a_pszGCIMKey, a_iBusineessID);
    if (0 > iRet)
    {
        tlog_error(m_pstLogCat, 0, 0, "tconnapi_initialize failed");
        return -1;
    }

    iRet = tconnapi_create((int)a_iLocalAddr, &m_pstHandler);
    if (0 > iRet)
    {
        tlog_error(m_pstLogCat, 0, 0, "tconnapi_create failed");
        return -1;
    }
    tconnapi_connect(m_pstHandler,m_iPeerAddr);

    /* by design, we first peek msg from tconnd-tbus-channel */
    /* but, tconnapi have no 'peekMsg' interface */
    /* so we have to implement a 'peekMsg' interface */
    /* that means, we need to get a tbus-handler */
    /* sadly, we can't get this handler from TCONNAPIHANDLE */
    /* so, we create a tbus-handler here */

    /* THIS SECTION OF CODE IS NOT SUGGESTED */

    iRet = tbus_new(&m_iBusHandler);
    if (TBUS_ERR_IS_ERROR(iRet))
    {
        tlog_error(m_pstLogCat, 0, 0, "tbus_new failed, for %s",
                   tbus_error_string(iRet));
        return -1;
    }

    iRet = tbus_bind(m_iBusHandler, a_iLocalAddr);
    if (TBUS_ERR_IS_ERROR(iRet))
    {
        tlog_error(m_pstLogCat, 0, 0, "tbus_bind<%d> failed, for %s",
                   a_iLocalAddr, tbus_error_string(iRet));
        return -1;
    }

    m_hasInited = true;

    return iRet;
}

int TconndHandler::sendMsg(const char* a_pBuff, const int a_iLen,
                           const TFRAMEHEAD* a_pstFrameHead)
{
    assert(NULL != a_pBuff);
    assert(NULL != a_pstFrameHead);

    int iRet = 0;

    assert(m_hasInited);

    if (0 >= a_iLen)
    {
        tlog_error(m_pstLogCat, 0, 0, "parameter error, a_iLen<%d>", a_iLen);
        return -1;
    }

    TBUSADDR iDstAddr = m_iPeerAddr;
    iRet = tconnapi_send(m_pstHandler, iDstAddr, (char*)a_pBuff,
                         a_iLen, (TFRAMEHEAD*)a_pstFrameHead);
    if (0 > iRet)
    {
        tlog_error(m_pstLogCat, 0, 0, "tconnapi_send failed, iRet<%d>", iRet);
        return -1;
    }

    return iRet;
}

int TconndHandler::peekMsg(const char** a_ppBuff, int* a_piLen)
{
    assert(NULL != a_ppBuff);
    assert(NULL != a_piLen);

    int iRet = 0;

    assert(m_hasInited);

    m_iTopMsgSrcAddr = m_iPeerAddr;
    m_iTopMsgDstAddr = m_iLocalAddr;
    const char* pszMsg;
    size_t iMsgLen;
    iRet = tbus_peek_msg(m_iBusHandler, &m_iTopMsgSrcAddr, &m_iTopMsgDstAddr, &pszMsg, &iMsgLen, 0);
    if (TBUS_ERR_IS_ERROR(iRet))
    {
        if (TBUS_ERR_CHANNEL_EMPTY == (unsigned)iRet)
        {
            tlog_trace(m_pstLogCat, 0, 0, "tbus channel empty");
        } else
        {
            tlog_error(m_pstLogCat, 0, 0, "tbus_peek_msg failed, srcAddr<%d>, dstAddr<%d>, for %s",
                       m_iTopMsgSrcAddr, m_iTopMsgDstAddr, tbus_error_string(iRet));
        }
        return -1;
    }

    int iHeadNetLen = 0;
    iRet = tconnapi_decode(pszMsg, iMsgLen, &m_stFrameHead, &iHeadNetLen);
    if (0 > iRet)
    {
        tlog_error(m_pstLogCat, 0, 0, "failed to decode framehaed, iRet<%d>", iRet);
        return -1;
    }

    *a_piLen = (int)(iMsgLen - iHeadNetLen);
    *a_ppBuff = pszMsg + iHeadNetLen;


    m_deleteBusMsg = true;

    return iRet;
}

int TconndHandler::deleteMsg()
{
    assert(m_hasInited);

    int iRet = 0;



    if (!m_deleteBusMsg)
    {
        return iRet;
    }

    iRet = tbus_delete_msg(m_iBusHandler, m_iTopMsgSrcAddr, m_iTopMsgDstAddr);
    if (TBUS_ERR_IS_ERROR(iRet))
    {
        tlog_error(m_pstLogCat, 0, 0, "tbus_delete_msg failed, srcAddr<%d>, dstAddr<%d>, for %s",
                   m_iTopMsgSrcAddr, m_iTopMsgDstAddr, tbus_error_string(iRet));
        return -1;
    } else
    {
        tlog_trace(m_pstLogCat, 0, 0, "tbus_delete_msg, srcAddr<%d>, dstAddr<%d>",
                   m_iTopMsgSrcAddr, m_iTopMsgDstAddr);
    }

    g_stEnv.pstRunCumu->ullTotalRequestNum++;



    return iRet;
}
