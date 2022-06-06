//============================================================================
// @Id:       $Id: err_stack.c 12118 2010-08-09 05:16:23Z tomaswang $
// @Author:   $Author: tomaswang $
// @Date:     $Date:: 2010-08-09 13:16:23 +0800 #$
// @Revision: $Revision: 12118 $
// @HeadURL:  $HeadURL:: https://tc-svn.tencent.com/ied/ied_tsf4g_rep/tsf4g_#$
//----------------------------------------------------------------------------


#include <stdarg.h>
#include <stdlib.h>

#include "pal/tstdio.h"

#include "err_stack.h"


//============================================================================
//
//----------------------------------------------------------------------------
static void *terror_make_frame_i(int a_iSize, int a_iHead);
static void *terror_make_frame_i(int a_iSize, int a_iHead)
{
    void   *pRet = NULL;
    size_t  iSize;

    if (a_iSize < 0)
    {
        return pRet;
    }

    iSize = a_iSize * sizeof(TErrorFrame);
    if (a_iHead)
    {
        iSize += sizeof(TErrorSet);
    }

    pRet = malloc(iSize);
    if (pRet)
    {
        memset(pRet, 0, iSize);
    }

    return pRet;
}

static int terrorset_init_i(TErrorSet *a_pstErr);
static int terrorset_init_i(TErrorSet *a_pstErr)
{
    TErrorFrame *pstFrame = (TErrorFrame *)terror_make_frame_i(20, 0);

    if (NULL == pstFrame)
    {
        return -1;
    }

    a_pstErr->pstFrame   = pstFrame;
    a_pstErr->iSize      = 20;

    terror_clear(a_pstErr);

    return 0;
}

TSINGLETON_C(TErrorSet, _, terrorset_init_i);

//============================================================================
// terror apis: clear, init, close, push, pop, top
//----------------------------------------------------------------------------
void terror_clear(TErrorSet *a_Error)
{
    if (NULL == a_Error)
    {
        a_Error = TSINGLETON(TErrorSet, _);
    }

    if (NULL != a_Error)
    {
        a_Error->iCount   = 0;
        a_Error->iCurrent = 0;
        a_Error->iTerator = 0;
        a_Error->iMaxFile = 0;
        a_Error->iMaxFunc = 0;
    }
}

TErrorSet *terror_init(int a_iSize)
{
    TErrorSet *pRet = (TErrorSet *)terror_make_frame_i(a_iSize, 1);

    if (pRet)
    {
        pRet->pstFrame   = (TErrorFrame *)((char *)pRet + sizeof(TErrorSet));
        pRet->iSize      = a_iSize;

        terror_clear(pRet);
    }

    return pRet;
}

void terror_destroy(TErrorSet *a_Error)
{
    if (a_Error)
    {
        free(a_Error);
    }
}

int terror_push_func(TErrorSet  *a_Error,
                     const char *a_pszFile,
                     int         a_iLine,
                     const char *a_pszFunc,
                     int         a_iErrNo,
                     const char *a_pszFormat,
                     ...)
{
    va_list args;

    TErrorFrame *pstFrame = NULL;
    size_t       iLen;

    if (NULL == a_Error)
    {
        a_Error = TSINGLETON(TErrorSet, _);
    }

    if (NULL == a_Error || a_Error->iCount >= a_Error->iSize)
    {
        return -a_iErrNo;
    }

#define TERROR_VAR(V) ((NULL == (V) || '\0' == *(V)) ? "" : (V))
    pstFrame = a_Error->pstFrame + a_Error->iCount++;
    pstFrame->pszFile     = TERROR_VAR(a_pszFile);
    pstFrame->iLine       = a_iLine;
    pstFrame->pszFunc     = TERROR_VAR(a_pszFunc);
    pstFrame->iErrNo      = a_iErrNo;
    pstFrame->szErrStr[0] = '\0';
#undef TERROR_VAR

#define TERROR_SET_MAX(V) do \
    { \
        iLen = (int)strlen(pstFrame->psz##V); \
        if (iLen > a_Error->iMax##V) \
        { \
            a_Error->iMax##V = iLen; \
        } \
    } while (0)
    TERROR_SET_MAX(File);
    TERROR_SET_MAX(Func);
#undef TERROR_SET_MAX

    if (a_pszFormat && *a_pszFormat)
    {
        va_start(args, a_pszFormat);
        vsnprintf(pstFrame->szErrStr, 255, a_pszFormat, args);
        va_end(args);

        pstFrame->szErrStr[255] = '\0';
    }

    return -a_iErrNo;
}

TErrorFrame *terror_pop(TErrorSet *a_Error)
{
    TErrorFrame *pRet = NULL;

    if (NULL == a_Error)
    {
        a_Error = TSINGLETON(TErrorSet, _);
    }

    if (NULL != a_Error && a_Error->iCount > 0)
    {
        pRet = a_Error->pstFrame + a_Error->iCount - 1;
        a_Error->iCount--;
    }

    return pRet;
}

TErrorFrame *terror_top(TErrorSet *a_Error)
{
    if (NULL == a_Error)
    {
        a_Error = TSINGLETON(TErrorSet, _);
    }

    return NULL == a_Error || a_Error->iCount <= 0
        ? NULL
        : a_Error->pstFrame + a_Error->iCount - 1;
}


//============================================================================
// iterator: count, begin, end, next, dump
//----------------------------------------------------------------------------
int terror_count(TErrorSet *a_Error)
{
    if (NULL == a_Error)
    {
        a_Error = TSINGLETON(TErrorSet, _);
    }

    return NULL == a_Error ? -1 : a_Error->iCount;
}

TErrorFrame *terror_begin(TErrorSet *a_Error)
{
    if (NULL == a_Error)
    {
        a_Error = TSINGLETON(TErrorSet, _);
    }

    if (NULL == a_Error)
    {
        return NULL;
    }

    a_Error->iTerator = a_Error->iCount - 1;

    return a_Error->pstFrame + a_Error->iTerator;
}

TErrorFrame *terror_end(TErrorSet *a_Error)
{
    if (NULL == a_Error)
    {
        a_Error = TSINGLETON(TErrorSet, _);
    }

    if (NULL == a_Error)
    {
        return NULL;
    }

    return a_Error->pstFrame - 1;
}

TErrorFrame *terror_next(TErrorSet *a_Error)
{
    if (NULL == a_Error)
    {
        a_Error = TSINGLETON(TErrorSet, _);
    }

    if (NULL == a_Error)
    {
        return NULL;
    }

    a_Error->iTerator = a_Error->iTerator < 0
        ? -1
        : a_Error->iTerator - 1;

    return a_Error->pstFrame + a_Error->iTerator;
}

void terror_dump(TErrorSet *a_Error)
{
    int          iIterator;
    TErrorFrame *i;
    char         szFormat[80];

    if (NULL == a_Error)
    {
        a_Error = TSINGLETON(TErrorSet, _);
    }

    if (NULL == a_Error)
    {
        return;
    }

    snprintf(szFormat,
             80,
             "%% %ds:%%-4d %% %ds -> (%%d) (%%s)\n",
             a_Error->iMaxFile,
             a_Error->iMaxFunc);

    iIterator = a_Error->iTerator;
    i = terror_begin(a_Error);
    while (i != terror_end(a_Error))
    {
        printf(szFormat,
               i->pszFile,
               i->iLine,
               i->pszFunc,
               i->iErrNo,
               i->szErrStr);
        i = terror_next(a_Error);
    }
    a_Error->iTerator = iIterator;
}


//============================================================================
// reverse_iterator: rbegin, rend, rnext, rdump
//----------------------------------------------------------------------------
TErrorFrame *terror_rbegin(TErrorSet *a_Error)
{
    if (NULL == a_Error)
    {
        a_Error = TSINGLETON(TErrorSet, _);
    }

    if (NULL == a_Error)
    {
        return NULL;
    }

    a_Error->iTerator = 0;

    return a_Error->pstFrame;
}

TErrorFrame *terror_rend(TErrorSet *a_Error)
{
    if (NULL == a_Error)
    {
        a_Error = TSINGLETON(TErrorSet, _);
    }

    if (NULL == a_Error)
    {
        return NULL;
    }

    return a_Error->pstFrame + a_Error->iCount;
}

TErrorFrame *terror_rnext(TErrorSet *a_Error)
{
    if (NULL == a_Error)
    {
        a_Error = TSINGLETON(TErrorSet, _);
    }

    if (NULL == a_Error)
    {
        return NULL;
    }

    a_Error->iTerator = a_Error->iTerator < a_Error->iCount
        ? a_Error->iTerator + 1
        : a_Error->iCount;

    return a_Error->pstFrame + a_Error->iTerator;
}

void terror_rdump(TErrorSet *a_Error)
{
    int          iIterator;
    TErrorFrame *i;
    char         szFormat[80];

    if (NULL == a_Error)
    {
        a_Error = TSINGLETON(TErrorSet, _);
    }

    if (NULL == a_Error)
    {
        return;
    }

    snprintf(szFormat,
             80,
             "%%d (%%s)\n%% %ds:%%-4d %% %ds\n",
             a_Error->iMaxFile,
             a_Error->iMaxFunc);

    iIterator = a_Error->iTerator;
    i = terror_rbegin(a_Error);
    while (i != terror_rend(a_Error))
    {
        printf(szFormat,
               i->iErrNo,
               i->szErrStr,
               i->pszFile,
               i->iLine,
               i->pszFunc);
        i = terror_rnext(a_Error);
    }
    a_Error->iTerator = iIterator;
}


//----------------------------------------------------------------------------
// THE END
//============================================================================
