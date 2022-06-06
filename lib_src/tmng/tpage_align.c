//============================================================================
// @Id:       $Id: tpage_align.c 15076 2010-11-23 02:56:24Z flyma $
// @Author:   $Author: flyma $
// @Date:     $Date:: 2010-11-23 10:56:24 +0800 #$
// @Revision: $Revision: 15076 $
// @HeadURL:  $HeadURL:: https://tc-svn.tencent.com/ied/ied_tsf4g_rep/tsf4g_#$
//----------------------------------------------------------------------------


#include "tpage_align.h"


//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
#if defined (_WIN32) || defined (_WIN64)

size_t tpage_align(size_t a_iSize)
{
    int    iPageSize = tos_get_pagesize();
    size_t iSub;

    if (iPageSize <= 0)
    {
        return a_iSize;
    }

    iSub = a_iSize % iPageSize;
    if (iSub)
    {
        a_iSize += iPageSize - iSub;
    }

    return a_iSize;
}

#endif


//----------------------------------------------------------------------------
// THE END
//============================================================================
