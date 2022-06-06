//============================================================================
// @Id:       $Id: tsm_base.c 10522 2010-07-20 02:22:08Z tomaswang $
// @Author:   $Author: tomaswang $
// @Date:     $Date:: 2010-07-20 10:22:08 +0800 #$
// @Revision: $Revision: 10522 $
// @HeadURL:  $HeadURL:: https://tc-svn.tencent.com/ied/ied_tsf4g_rep/tsf4g_#$
//----------------------------------------------------------------------------


#include <stdio.h>

#include "tsm_base.h"
#include "err_stack.h"


//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
static int tsm_base_init_i(TsmBase *a_pstBase)
{
    a_pstBase->iIndexKey   = TSM_TDR_C_INDEX_KEY;

    a_pstBase->iLibHash    = 8;
    a_pstBase->iLibCap     = 3000;
    a_pstBase->iUriHash    = 8;
    a_pstBase->iUriCap     = 10000;

    a_pstBase->iHashCap    = 0;

    a_pstBase->iOffsetLib  = 0;
    a_pstBase->iOffsetUri  = 0;
    a_pstBase->iOffsetHash = 0;

    a_pstBase->iIndexSize  = 0;

    return 0;
}

TSINGLETON_C(TsmBase, I, tsm_base_init_i);


//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int tsm_base_index(TsmBaseVar *a_pstVar)
{
    TsmBase *pstBase = TSINGLETON(TsmBase, I);

    tassert(a_pstVar, NULL, 1, "参数错误: 句柄为空");

    if (a_pstVar->iIndex >= 0)
    {
        int iIndex  = a_pstVar->iIndex;
        int iOffset = pstBase->iLibHash * pstBase->iLibCap;

        if (iIndex < iOffset)
        {
            a_pstVar->iType = TSM_URI_META;
            a_pstVar->iRow  = iIndex / pstBase->iLibHash;
            a_pstVar->iCol  = iIndex % pstBase->iLibHash;

            return 0;
        }

        iIndex -= iOffset;

        if (iIndex < pstBase->iUriHash * pstBase->iUriCap)
        {
            a_pstVar->iType  = TSM_URI_DATA;
            a_pstVar->iRow   = iIndex / pstBase->iUriHash;
            a_pstVar->iCol   = iIndex % pstBase->iUriHash;

            return 0;
        }
    }
    else if (a_pstVar->iRow >= 0 && a_pstVar->iCol >= 0)
    {
        int iRow = a_pstVar->iRow;
        int iCol = a_pstVar->iCol;

        if (TSM_URI_META == a_pstVar->iType && \
                iRow < pstBase->iLibCap && \
                iCol < pstBase->iLibHash)
        {
            a_pstVar->iIndex = iRow * pstBase->iLibHash + iCol;

            return 0;
        }

        if (TSM_URI_DATA == a_pstVar->iType && \
                iRow < pstBase->iUriCap && \
                iCol < pstBase->iUriHash)
        {
            a_pstVar->iIndex  = pstBase->iLibCap * pstBase->iLibHash;
            a_pstVar->iIndex += iRow * pstBase->iUriHash + iCol;

            return 0;
        }
    }

    return terror_push(NULL, 2, "索引上越界"); 
}


//----------------------------------------------------------------------------
// THE END
//============================================================================
