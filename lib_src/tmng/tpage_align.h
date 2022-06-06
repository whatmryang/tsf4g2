#ifndef _TPAGE_ALIGN_H_
#define _TPAGE_ALIGN_H_
//============================================================================
// @Id:       $Id: tpage_align.h 15076 2010-11-23 02:56:24Z flyma $
// @Author:   $Author: flyma $
// @Date:     $Date:: 2010-11-23 10:56:24 +0800 #$
// @Revision: $Revision: 15076 $
// @HeadURL:  $HeadURL:: https://tc-svn.tencent.com/ied/ied_tsf4g_rep/tsf4g_#$
//----------------------------------------------------------------------------


#include <sys/types.h>

#include "pal/tos.h"


//----------------------------------------------------------------------------
//  ���뺯��
//      a_iSize     ��Ҫ����ĳߴ�
//  �ɹ��򷵻ض���֮��ĳߴ�
//  ���򷵻� a_iSize����   (��һ������ʧ����)
//----------------------------------------------------------------------------
#if defined(_WIN32) || defined(_WIN64)

size_t tpage_align(size_t a_iSize);

#else

static __inline__ size_t tpage_align(size_t a_iSize)
{
    int iPageSize = tos_get_pagesize();
    int iSub;

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
#endif
