/*
 * Copyright (c) 2010, 腾讯科技有限公司互动娱乐研发部
 * All rights reserved
 *
 * File name:       transfer_tconnd_handle.h
 * Description:     definition for class TconndHandler
 * Version:         1.0
 *
 * Author:          flyma.IERD.IED.tencent
 * Creation time:   2010-06-03 17:49
 * Last modified:   2010-06-04 14:24
 *
 * History:
 * <date>        <author>      <version>   <description>
 * 2010-06-03     flyma         1.0         creation
 *
 */

#ifndef _TRANSFER_TCONND_HANDLER_
#define _TRANSFER_TCONND_HANDLER_

#include <assert.h>
#include "tdr/tdr.h"
#include "tlog/tlog.h"
#include "tbus/tbus.h"
#include "tconnapi/tconnapi.h"

class TconndHandler
{
    public:

        int init(const char* a_pszGCIMKey, const int a_iBusineessID,
                 TBUSADDR a_iLocalAddr, TBUSADDR a_iTconndAddr);

        int sendMsg(const char* a_pBuff, const int a_iLen, const TFRAMEHEAD* a_pstFrameHead);
        int peekMsg(const char** a_ppBuff, int* a_piLen);

        int deleteMsg();

        const TFRAMEHEAD* getCurMsgFrame() { return &m_stFrameHead; }

        static TconndHandler* getInstance()
        {
            if (NULL == s_instance)
            {
                s_instance = new TconndHandler();
            }
            return s_instance;
        }

        void keepBusMsgUntouched() { m_deleteBusMsg = false; }


    private:

        TconndHandler();
        ~TconndHandler();

        static TconndHandler* s_instance;

        bool m_deleteBusMsg;
        bool m_hasInited;
        bool m_hasPeeked;

        LPTLOGCATEGORYINST m_pstLogCat;

        int m_pstHandler;

        int m_iBusHandler;
        TBUSADDR m_iLocalAddr;
        TBUSADDR m_iPeerAddr;
        TBUSADDR m_iTopMsgSrcAddr;
        TBUSADDR m_iTopMsgDstAddr;

        TFRAMEHEAD m_stFrameHead;
};
#endif
