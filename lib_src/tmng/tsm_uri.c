//============================================================================
// @Id:       $Id: tsm_uri.c 13846 2010-09-27 02:15:03Z tomaswang $
// @Author:   $Author: tomaswang $
// @Date:     $Date:: 2010-09-27 10:15:03 +0800 #$
// @Revision: $Revision: 13846 $
// @HeadURL:  $HeadURL:: https://tc-svn.tencent.com/ied/ied_tsf4g_rep/tsf4g_#$
//----------------------------------------------------------------------------


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "err_stack.h"
#include "tsm_uri.h"
#include "tsm_uri_i.h"


//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
void *tsm_uri_alloc(const char *a_pszUri)
{
    TsmUri *pstUri;
    TsmUri  stUri;
    int     iRet;

    tassert_r(NULL, a_pszUri && a_pszUri[0], NULL, 1, "参数错误：空 uri");

    iRet = tsm_uri_set(&stUri, a_pszUri);
    tassert_r(NULL, 0 == iRet, NULL, 2, "分析 uristr 出错");

    pstUri = malloc(sizeof(TsmUri));
    tassert_r(NULL, pstUri, NULL, 3, "内存分配失败");

    memcpy(pstUri, &stUri, sizeof(TsmUri));

    return (void *)pstUri;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
const char *tsm_uri_str(void *a_pstUri)
{
    return tsm_uri_get(a_pstUri);
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int tsm_uri_set_bid(void *a_pstUri, int a_iBid)
{
    TsmUri *pstUri = (TsmUri *)a_pstUri;

    tassert(a_pstUri, NULL, 1, "参数错误，空 uri");

    if (TSM_URI_META == pstUri->iType)
    {
        pstUri->s.m.iBid = a_iBid;
    }
    else if (TSM_URI_DATA == pstUri->iType)
    {
        pstUri->s.d.iBid = a_iBid;
    }
    else
    {
        tassert(0, NULL, 2, "URI 类型错误");
    }
    pstUri->iSync = 1;

    return 0;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int tsm_uri_std(void *a_pstUri)
{
    return tsm_uri_std_i((TsmUri *)a_pstUri, NULL);
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
void tsm_uri_free(void *a_pstUri)
{
    if (a_pstUri)
    {
        free(a_pstUri);
    }
}

//----------------------------------------------------------------------------
//  GET functions
//----------------------------------------------------------------------------
#define CHECK_P(P) \
    do \
    { \
        tassert((P), NULL, 1, "参数错误: 空 uri"); \
        tassert((TSM_URI_META == (P)->iType || TSM_URI_DATA == (P)->iType), \
                NULL, 2, "类型不认识"); \
    } \
    while (0)

#define CHECK_PR(P) \
    do \
    { \
        tassert_r(0, (P), NULL, 1, "参数错误: 空 uri"); \
        tassert_r(0, \
                (TSM_URI_META == (P)->iType || TSM_URI_DATA == (P)->iType), \
                NULL, 2, "类型不认识"); \
    } \
    while (0)


#define FUNC_I(N, T, V) \
    int tsm_uri_get_##N(void *a_pstUri) \
    { \
        TsmUri *pstUri = (TsmUri *)a_pstUri; \
        CHECK_P(pstUri); \
        return pstUri->s.T.V; \
    }

#define FUNC_S(N, T, V) \
    const char *tsm_uri_get_##N(void *a_pstUri) \
    { \
        TsmUri *pstUri = (TsmUri *)a_pstUri; \
        CHECK_PR(pstUri); \
        return (const char *)(pstUri->s.T.V); \
    }

#define FUNC_META_I(N, V) FUNC_I(N, m, V)
#define FUNC_DATA_I(N, V) FUNC_I(N, d, V)
#define FUNC_META_S(N, V) FUNC_S(N, m, V)
#define FUNC_DATA_S(N, V) FUNC_S(N, d, V)


//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int tsm_uri_get_type(void *a_pstUri)
{
    TsmUri *pstUri = (TsmUri *)a_pstUri;

    CHECK_P(pstUri);

    return pstUri->iType;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int tsm_uri_get_bid(void *a_pstUri)
{
    TsmUri *pstUri = (TsmUri *)a_pstUri;

    CHECK_P(pstUri);

    return pstUri->s.m.iBid;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
size_t tsm_uri_get_size(void *a_pstUri)
{
    TsmUri *pstUri = (TsmUri *)a_pstUri;

    CHECK_PR(pstUri);

    return pstUri->s.m.iSize;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
FUNC_META_S(libname,       szLib)
FUNC_META_I(lib_ver,       iLibVer)
FUNC_META_I(lib_build_ver, iLibBuildVer)
FUNC_META_S(metaname,      szMeta)
FUNC_META_I(meta_ver,      iMetaVer)
FUNC_META_S(other,         szOther)

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
FUNC_DATA_S(prog,  szProgName)
FUNC_DATA_S(id,    szProgId)
FUNC_DATA_S(func,  szFuncName)
FUNC_DATA_S(value, szValueName)
FUNC_DATA_S(path,  szValuePath)

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
#undef FUNC_DATA_S
#undef FUNC_META_S
#undef FUNC_DATA_I
#undef FUNC_META_I
#undef FUNC_S
#undef FUNC_I
#undef CHECK_PR
#undef CHECK_P


//----------------------------------------------------------------------------
// THE END
//============================================================================
