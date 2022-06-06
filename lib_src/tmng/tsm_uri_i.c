//============================================================================
// @Id:       $Id: tsm_uri_i.c 13810 2010-09-26 03:24:17Z tomaswang $
// @Author:   $Author: tomaswang $
// @Date:     $Date:: 2010-09-26 11:24:17 +0800 #$
// @Revision: $Revision: 13810 $
// @HeadURL:  $HeadURL:: https://tc-svn.tencent.com/ied/ied_tsf4g_rep/tsf4g_#$
//----------------------------------------------------------------------------


#include <stdlib.h>

#include "pal/tstdio.h"

#include "err_stack.h"
#include "tsm_uri_i.h"


//============================================================================
//
//----------------------------------------------------------------------------
#define STR_N_CPY(T, S, L) \
    do \
    { \
        if ((L) <= 0 || 0 == (S)[0]) \
        { \
            T[0] = '\0'; \
        } \
        else \
        { \
            int i = 0; \
            do \
            { \
                (T)[i] = S[i]; \
            } while (++i < L && S[i]); \
            T[i == (L) ? (L) - 1 : i] = '\0'; \
        } \
    } \
    while (0)


//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
#define TSM_URI_META_FORMAT "?/::/:/"
#define TSM_URI_DATA_FORMAT "?/:///"


#define TSM_URI_SET_INT(P, T, N, V) \
    do \
    { \
        ((P)->s.T.N) = (V); \
        ((P)->iSync) = 1; \
    } \
    while (0)

#define TSM_URI_SET_STR(P, T, N, V) \
    do \
    { \
        if (0 == V[0]) \
        { \
            memset(((P)->s.T.N), 0, sizeof((P)->s.T.N)); \
        } \
        else \
        { \
            STR_N_CPY(((P)->s.T.N), V, sizeof((P)->s.T.N)); \
        } \
        ((P)->iSync) = 1; \
    } \
    while (0)


//============================================================================
//  static functions
//----------------------------------------------------------------------------
static int tsm_uri_init(TsmUri *a_pstUri);
static int tsm_uri_init(TsmUri *a_pstUri)
{
    tassert(a_pstUri, NULL, 1, "参数错误: 句柄为空");

    memset(a_pstUri, 0, sizeof(TsmUri));

    return 0;
}


//============================================================================
//  APIs
//----------------------------------------------------------------------------
//  
//----------------------------------------------------------------------------
int tsm_uri_sync(TsmUri *a_pstUri)
{
    if (NULL == a_pstUri || 0 == a_pstUri->iType)
    {
        return 0;
    }
    tassert1(TSM_URI_META == a_pstUri->iType || TSM_URI_DATA == a_pstUri->iType,
             NULL, 2, "错误的 Uri Type(%d)", a_pstUri->iType);

    if (a_pstUri->iSync)
    {
        int i = 0;

#define STR_P(I) (a_pstUri->szUri + (I))
#define STR_S(I) (URI_MAX_LEN - (I) - 1)
#define STR_CAT_S(C, V) \
    do \
    { \
        i += snprintf(STR_P(i), STR_S(i), "%c%s", C, V); \
    } \
    while (0)

#define STR_CAT_I(C, V) \
    do \
    { \
        if (V < 0) \
        { \
            i += snprintf(STR_P(i), STR_S(i), "%c*", C); \
        } \
        else \
        { \
            i += snprintf(STR_P(i), STR_S(i), "%c%d", C, V); \
        } \
    } \
    while (0)

        if (TSM_URI_META == a_pstUri->iType)
        {
            TsmTdrMetaInfo *pstMeta = &a_pstUri->s.m;

            STR_CAT_S('t', "sm-meta:/");
            STR_CAT_I('/', pstMeta->iBid);
            STR_CAT_S('/', pstMeta->szLib);
            STR_CAT_I(':', pstMeta->iLibVer);
            STR_CAT_I(':', pstMeta->iLibBuildVer);
            STR_CAT_S('/', pstMeta->szMeta);
            STR_CAT_I(':', pstMeta->iMetaVer);
            STR_CAT_S('/', pstMeta->szOther);
        }
        else
        {
            TsmTdrDataInfo *pstData = &a_pstUri->s.d;

            STR_CAT_S('t', "sm-data:/");
            STR_CAT_I('/', pstData->iBid);
            STR_CAT_S('/', pstData->szProgName);
            STR_CAT_S(':', pstData->szProgId);
            STR_CAT_S('/', pstData->szFuncName);
            STR_CAT_S('/', pstData->szValueName);
            //STR_CAT_S('/', pstData->szValuePath);
        }

#undef STR_CAT_I
#undef STR_CAT_S
#undef STR_S
#undef STR_P

        a_pstUri->iSync = 0;
    }

    return 0;
}

//----------------------------------------------------------------------------
//  Uri 解析状态表
//----------------------------------------------------------------------------
int tsm_uri_set(TsmUri *a_pstUri, const char *a_pszUri)
{
    const char *pszHead = a_pszUri;
    const char *pszTail = strstr(a_pszUri, "://");
    const char *pszFormat;
    TsmUri      stUri;
    int         i;
    char        c;
    char        s;  // star

    tassert(a_pszUri, NULL, 1, "参数错误: 空字符串");
    tassert(pszTail && pszTail - pszHead < URI_MAX_LEN - 5,
            NULL, 2, "Uri 格式错误: 找不到 ://");

    //-----------------------------------------------
    //  填充 scheme 部分
    //-----------------------------------------------
    tsm_uri_init(&stUri);
    for (i = 0, c = *pszHead; '/' != c; c = *++pszHead)
    {
        stUri.szUri[i++] = c >= 'A' && c <= 'Z' ? c - 'A' + 'a' : c;
    }
    stUri.szUri[i++] = '/';
    stUri.szUri[i++] = '/';
    stUri.szUri[i++] = '\0';
    pszTail += 2;

    //-----------------------------------------------
    //  分析
    //-----------------------------------------------
#define GET_VAL(F) \
    do \
    { \
        char X; \
        if (F && c == F) \
        { \
            ++pszTail; \
        } \
        c = *(pszHead = pszTail); \
        X = pszFormat[++i]; \
        for (; c && (!X || (':' != c && '/' != c)); c = *++pszTail) \
        { \
        } \
        if (':' == c && '/' == X) \
        { \
            return terror_push(NULL, 3, "Uri 格式错误: 分隔错误"); \
        } \
        if ('*' == *pszHead && pszHead + 1 == pszTail) \
        { \
            s = 1; \
        } \
    } \
    while (0)

#define GET_INT(T, N) \
    do \
    { \
        GET_VAL(pszFormat[i]); \
        if (1 == pszTail - pszHead) \
        { \
            stUri.s.T.N = ('*' == (*pszHead) ? -1 : atoi(pszHead)); \
        } \
        else if (0 == c && s) \
        { \
            stUri.s.T.N = -1; \
        } \
        else \
        { \
            stUri.s.T.N = atoi(pszHead); \
        } \
    } \
    while (0)

#define GET_STR(T, N) \
    do \
    { \
        GET_VAL(pszFormat[i]); \
        if (pszTail - pszHead >= sizeof(stUri.s.T.N)) \
        { \
            return terror_push(NULL, 4, "Uri 格式错误: 字段太长"); \
        } \
        if (0 == c && s) \
        { \
            STR_N_CPY(stUri.s.T.N, "*", 2); \
        } \
        else \
        { \
            STR_N_CPY(stUri.s.T.N, pszHead, pszTail - pszHead + 1); \
        } \
    } \
    while (0)

    i = 0;
    s = 0;
    c = '?';
    if (0 == strncmp("tsm-meta://", stUri.szUri, 12))
    {
        stUri.iType = TSM_URI_META;
        pszFormat   = TSM_URI_META_FORMAT;

        GET_INT(m, iBid);
        GET_STR(m, szLib);
        GET_INT(m, iLibVer);
        GET_INT(m, iLibBuildVer);
        GET_STR(m, szMeta);
        GET_INT(m, iMetaVer);
        GET_STR(m, szOther);
    }
    else if (0 == strncmp("tsm-data://", stUri.szUri, 12))
    {
        stUri.iType = TSM_URI_DATA;
        pszFormat   = TSM_URI_DATA_FORMAT;

        GET_INT(d, iBid);
        GET_STR(d, szProgName);
        GET_STR(d, szProgId);
        GET_STR(d, szFuncName);
        GET_STR(d, szValueName);
        GET_STR(d, szValuePath);
    }
    else
    {
        return terror_push(NULL, 5, "Uri 格式错误: 必须是 tsm-(meta|data)");
    }

#undef GET_STR
#undef GET_INT
#undef GET_VAL

    //-----------------------------------------------
    //  成功
    //-----------------------------------------------
    stUri.iSync = 1;
    tsm_uri_sync(&stUri);

    if (NULL != a_pstUri)
    {
        memcpy(a_pstUri, &stUri, sizeof(TsmUri));
    }

    return 0;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int tsm_uri_set_meta(TsmUri         *a_pstUri,
                     TsmTdrMetaInfo *a_pstMeta)
{
    tassert(a_pstUri,  NULL, 1, "参数错误: 空句柄");
    tassert(a_pstMeta, NULL, 2, "参数错误: 空Meta");

    a_pstUri->iType = TSM_URI_META;
    a_pstUri->iSync = 1;
    memcpy(&a_pstUri->s.m, a_pstMeta, sizeof(TsmTdrMetaInfo));

    tsm_uri_sync(a_pstUri);

    return 0;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int tsm_uri_set_data(TsmUri         *a_pstUri,
                     TsmTdrDataInfo *a_pstData)
{
    tassert(a_pstUri,  NULL, 1, "参数错误: 空句柄");
    tassert(a_pstData, NULL, 2, "参数错误: 空Data");

    a_pstUri->iType = TSM_URI_DATA;
    a_pstUri->iSync = 1;
    memcpy(&a_pstUri->s.d, a_pstData, sizeof(TsmTdrDataInfo));

    tsm_uri_sync(a_pstUri);

    return 0;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int tsm_uri_is_null(TsmUri *a_pstUri)
{
    tassert(a_pstUri,  NULL, 1, "参数错误: 空句柄");

    if (TSM_URI_META == a_pstUri->iType)
    {
        return tsm_tdr_meta_is_null(&a_pstUri->s.m);
    }

    if (TSM_URI_DATA == a_pstUri->iType)
    {
        return tsm_tdr_data_is_null(&a_pstUri->s.d);
    }

    tassert(0 == a_pstUri->iType, NULL, 2, "错误的 URI类型");

    return 1;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
const char *tsm_uri_get(TsmUri *a_pstUri)
{
    if (NULL == a_pstUri)
    {
        return NULL;
    }

    tsm_uri_sync(a_pstUri);

    return (const char *)a_pstUri->szUri;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int tsm_uri_std_i(TsmUri *a_pstStd,
                  TsmUri *a_pstUri)
{
    tassert(a_pstStd, NULL, 1, "参数错误: 标准Uri 空指针");

    if (a_pstUri)
    {
        memcpy(a_pstStd, a_pstUri, sizeof(TsmUri));
    }

    if (TSM_URI_META == a_pstStd->iType)
    {
        TSM_URI_SET_STR(a_pstStd, m, szMeta,   "");
        TSM_URI_SET_INT(a_pstStd, m, iMetaVer, 0);
        TSM_URI_SET_STR(a_pstStd, m, szOther,  "");
    }
    else if (TSM_URI_DATA == a_pstStd->iType)
    {
        TSM_URI_SET_STR(a_pstStd, d, szValuePath, "");
    }
    else
    {
        return terror_push1(NULL, 3, "错误的 URI类型 (%d)", a_pstStd->iType);
    }

    return tsm_uri_sync(a_pstStd);
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
int tsm_uri_compare(TsmUri *a_pstUri1,
                    TsmUri *a_pstUri2)
{
    int i;

    tassert(a_pstUri1, NULL, 2, "参数错误: uri_1 为空");
    tassert(a_pstUri2, NULL, 3, "参数错误: uri_2 为空");

    tsm_uri_sync(a_pstUri1);
    tsm_uri_sync(a_pstUri2);

    for (i = 0; i < URI_MAX_LEN; ++i)
    {
        if (a_pstUri1->szUri[i] != a_pstUri2->szUri[i])
        {
            return -1;
        }

        if ('\0' == a_pstUri1->szUri[i])
        {
            break;
        }
    }

    return 0;
}


//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
#undef STR_N_CPY

//----------------------------------------------------------------------------
// THE END
//============================================================================
