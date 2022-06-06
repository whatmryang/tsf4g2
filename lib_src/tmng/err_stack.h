#ifndef _ERR_STACK_H_
#define _ERR_STACK_H_
//============================================================================
// @Id:       $Id: err_stack.h 15076 2010-11-23 02:56:24Z flyma $
// @Author:   $Author: flyma $
// @Date:     $Date:: 2010-11-23 10:56:24 +0800 #$
// @Revision: $Revision: 15076 $
// @HeadURL:  $HeadURL:: https://tc-svn.tencent.com/ied/ied_tsf4g_rep/tsf4g_#$
//----------------------------------------------------------------------------


#include "tsingleton.h"

#include "pal/tstring.h"

#ifdef __cplusplus
extern "C"
{
#endif


//============================================================================
// TError Structs: TErrorFrame, TErrorSet
//----------------------------------------------------------------------------
#define TERROR_MSG_SIZE 1024
#define TERROR_LSIZE    256

typedef struct
{
    const char *pszFile;
    int         iLine;
    const char *pszFunc;
    int         iErrNo;
    char        szErrStr[TERROR_LSIZE + 1];
} TErrorFrame;

typedef struct
{
    int          iSize;
    int          iCount;
    int          iCurrent;
    int          iTerator;
    int          iMaxFile;
    int          iMaxFunc;
    TErrorFrame *pstFrame;
} TErrorSet;

TSINGLETON_H(TErrorSet, _);


//============================================================================
// terror apis: clear, init, close, push, pop, top
//----------------------------------------------------------------------------
void terror_clear(TErrorSet *a_Error);

TErrorSet *terror_init(int a_iSize);
void terror_destroy(TErrorSet *a_Error);

#if defined(_WIN32) || defined(_WIN64)
#if _MSC_VER < 1400
    #define __FUNCTION__ "vc{6,7}func"
#endif
#endif

#define terror_push(E, N, S) \
    terror_push_func(E, __FILE__, __LINE__, __FUNCTION__, N, S)

#define terror_push1(E, N, S, A1) \
    terror_push_func(E, __FILE__, __LINE__, __FUNCTION__, N, S, A1)

#define terror_push2(E, N, S, A1, A2) \
    terror_push_func(E, __FILE__, __LINE__, __FUNCTION__, N, S, A1, A2)

#define terror_push3(E, N, S, A1, A2, A3) \
    terror_push_func(E, __FILE__, __LINE__, __FUNCTION__, N, S, A1, A2, A3)

#define terror_push4(E, N, S, A1, A2, A3, A4) \
    terror_push_func(E, __FILE__, __LINE__, __FUNCTION__, N, S, A1, A2, A3, A4)

#define terror_push5(E, N, S, A1, A2, A3, A4, A5) \
    terror_push_func(E, __FILE__, __LINE__, __FUNCTION__, N, \
                     S, A1, A2, A3, A4, A5)


int terror_push_func(TErrorSet  *a_Error,
                     const char *a_pszFile,
                     int         a_iLine,
                     const char *a_pszFunc,
                     int         a_iErrNo,
                     const char *a_pszFormat,
                     ...);
TErrorFrame *terror_pop(TErrorSet *a_Error);
TErrorFrame *terror_top(TErrorSet *a_Error);

//============================================================================
// iterator: count, begin, end, next, dump
//----------------------------------------------------------------------------
int terror_count(TErrorSet *a_Error);
TErrorFrame *terror_begin(TErrorSet *a_Error);
TErrorFrame *terror_end(TErrorSet *a_Error);
TErrorFrame *terror_next(TErrorSet *a_Error);
void terror_dump(TErrorSet *e);

//============================================================================
// reverse_iterator: rbegin, rend, rnext, rdump
//----------------------------------------------------------------------------
TErrorFrame *terror_rbegin(TErrorSet *a_Error);
TErrorFrame *terror_rend(TErrorSet *a_Error);
TErrorFrame *terror_rnext(TErrorSet *a_Error);
void terror_rdump(TErrorSet *e);
#ifdef __cplusplus
}
#endif

//============================================================================
//
//----------------------------------------------------------------------------

#define tassert_r(R, O, E, I, S) \
    do \
    { \
        if (!(O)) \
        { \
            terror_push(E, I, S); \
            return (R); \
        } \
    } \
    while (0)

#define tassert_r1(R, O, E, I, S, A1) \
    do \
    { \
        if (!(O)) \
        { \
            terror_push1(E, I, S, A1); \
            return (R); \
        } \
    } \
    while (0)

#define tassert_r2(R, O, E, I, S, A1, A2) \
    do \
    { \
        if (!(O)) \
        { \
            terror_push2(E, I, S, A1, A2); \
            return (R); \
        } \
    } \
    while (0)

#define tassert_r3(R, O, E, I, S, A1, A2, A3) \
    do \
    { \
        if (!(O)) \
        { \
            terror_push3(E, I, S, A1, A2, A3); \
            return (R); \
        } \
    } \
    while (0)

#define tassert_r4(R, O, E, I, S, A1, A2, A3, A4) \
    do \
    { \
        if (!(O)) \
        { \
            terror_push4(E, I, S, A1, A2, A3, A4); \
            return (R); \
        } \
    } \
    while (0)

#define tassert_r5(R, O, E, I, S, A1, A2, A3, A4, A5) \
    do \
    { \
        if (!(O)) \
        { \
            terror_push5(E, I, S, A1, A2, A3, A4, A5); \
            return (R); \
        } \
    } \
    while (0)


#define tassert(O, E, I, S) \
    tassert_r((-(I)), O, E, I, S)

#define tassert1(O, E, I, S, A1) \
    tassert_r1((-(I)), O, E, I, S, A1)

#define tassert2(O, E, I, S, A1, A2) \
    tassert_r2((-(I)), O, E, I, S, A1, A2)

#define tassert3(O, E, I, S, A1, A2, A3) \
    tassert_r3((-(I)), O, E, I, S, A1, A2, A3)

#define tassert4(O, E, I, S, A1, A2, A3, A4) \
    tassert_r4((-(I)), O, E, I, S, A1, A2, A3, A4, A5)

#define tassert5(O, E, I, S, A1, A2, A3, A4) \
    tassert_r5((-(I)), O, E, I, S, A1, A2, A3, A4, A5)


//----------------------------------------------------------------------------
// THE END
//============================================================================
#endif
