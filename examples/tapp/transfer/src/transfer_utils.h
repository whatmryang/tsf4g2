/*
 * Copyright (c) 2010, 腾讯科技有限公司互动娱乐研发部
 * All rights reserved
 *
 * File name:       transfer_utils.h
 * Description:     definition for TransferUtil
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

#ifndef _TRANSFER_UTIL_H_
#define _TRANSFER_UTIL_H_

#include "transfer.h"
#include "tdr/tdr.h"
#include "tconnapi/tconnapi.h"
#include "transfer_fore_proto.h"
#include "transfer_backsvr_proto.h"

class TransferUtil
{
    public:
        static int decodeMsg(LPTDRMETA a_pstMeta,
                             char* a_pHostBuff, int* a_piHostBuff,
                             const char* a_pNetBuff, int* a_piNetBuff);

        static int encodeMsg(LPTDRMETA a_pstMeta,
                             char* a_pNetBuff, int* a_piNetBuff,
                             const char* a_pHostBuff, int* a_piHostBuff);

        static int sendPkgToTconnd(TRANSFERENV* a_pstEnv,
                                   const FOREPKG* a_pstPkg,
                                   const TFRAMEHEAD* a_pstFrameHead);

        static int sendPkgToServer(TRANSFERENV* a_pstEnv,
                                   const BACKPKG* a_pstPkg, TBUSADDR a_iDstAddr);
};

#endif
