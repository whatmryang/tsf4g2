/*
 * Copyright (c) 2010, 腾讯科技有限公司互动娱乐研发部
 * All rights reserved
 *
 * File name:       tdr_json_parser.h
 * Description:     在JSON_parser.h的基础上抽象出更容易使用的JSON解析器
 * Version:         1.0
 *
 * Author:          flyma.IERD.IED.tencent
 * Creation time:   2011-02-17 17:31
 * Last modified:   2011-02-17 17:31
 *
 * History:
 * <date>        <author>      <version>   <description>
 * 2011-02-17     flyma         1.0         creation
 *
 */

#include "tdr_json_parser.h"
#include <assert.h>

#define PARSER_SET_TASK(a_pstParser, TASK)\
{\
    (a_pstParser)->stCtx.eTask = (TASK);\
    (a_pstParser)->stCtx.iHasError = 0;\
    (a_pstParser)->stCtx.iIsTaskDone = 0;\
}

#define PARSER_CLEAR_TASK(a_pstParser)\
{\
    (a_pstParser)->stCtx.eTask = TDR_JSON_TASK_UNKNOWN;\
    (a_pstParser)->stCtx.eType = TDR_JSON_TYPE_UNKOWN;\
    (a_pstParser)->stCtx.iHasError = 0;\
    (a_pstParser)->stCtx.iIsTaskDone = 0;\
}

#define PARSER_TASK_HAS_STOP(a_pstParser)\
    (0 != (a_pstParser)->stCtx.iHasError || 0 != (a_pstParser)->stCtx.iIsTaskDone)

#define PARSER_TASK_CHECK_AND_RETURN(a_pstParser, ch)\
{\
    if (0 == ch)\
    {\
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_JSON_SHORT_OF_INPUT);\
    }\
    if (0 != (a_pstParser)->stCtx.iHasError)\
    {\
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_JSON_INVALIED_FORMAT);\
    } else if (0 == (a_pstParser)->stCtx.iIsTaskDone)\
    {\
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_JSON_INVALIED_FORMAT);\
    } else\
    {\
        return TDR_SUCCESS;\
    }\
}

void tdr_json_stream_init_by_buf(LPTDRJSONSTREAM a_pstStream, char* a_pszBuff, size_t a_tBuff)
{
    assert(NULL != a_pstStream);
    assert(NULL != a_pszBuff);

    a_pstStream->eType = TDR_JSON_STREAM_TYPE_BUF;
    a_pstStream->unStreams.stBuff.pszBuff = a_pszBuff;
    a_pstStream->unStreams.stBuff.tBuffer = a_tBuff;
    a_pstStream->unStreams.stBuff.tPosition = 0;
}

void tdr_json_stream_init_by_fp(LPTDRJSONSTREAM a_pstStream, FILE* a_fp)
{
    assert(NULL != a_pstStream);
    assert(NULL != a_fp);

    a_pstStream->eType = TDR_JSON_STREAM_TYPE_FP;
    a_pstStream->unStreams.fp = a_fp;
}

int tdr_json_stream_get_next_char(LPTDRJSONSTREAM a_pstStream)
{
    int ch = 0;

    assert(NULL != a_pstStream);

    switch (a_pstStream->eType)
    {
        case TDR_JSON_STREAM_TYPE_BUF:
            assert(NULL != a_pstStream->unStreams.stBuff.pszBuff);
            if (a_pstStream->unStreams.stBuff.tPosition < a_pstStream->unStreams.stBuff.tBuffer)
            {
                ch = *(a_pstStream->unStreams.stBuff.pszBuff + a_pstStream->unStreams.stBuff.tPosition++);
            }
            break;
        case TDR_JSON_STREAM_TYPE_FP:
            assert(NULL != a_pstStream->unStreams.fp);
            ch = fgetc(a_pstStream->unStreams.fp);
            break;
        default:
            break;
    }

    return ch;
}

int tdr_json_parser_match_struct_begin(LPTDRJSONPARSER a_pstParser)
{
    char ch;

    assert(NULL != a_pstParser);

    PARSER_SET_TASK(a_pstParser, TDR_JSON_TASK_FIND_STRUCT_BEGIN);

    while ((ch = tdr_json_stream_get_next_char(&a_pstParser->stStream)))
    {
        JSON_parser_char(a_pstParser->pstParser, ch);
        if (PARSER_TASK_HAS_STOP(a_pstParser))
        {
            break;
        }
    }

    PARSER_TASK_CHECK_AND_RETURN(a_pstParser, ch);
}

int tdr_json_parser_match_struct_end(LPTDRJSONPARSER a_pstParser)
{
    char ch;

    assert(NULL != a_pstParser);

    if (a_pstParser->stCtx.iStructEndBuf)
    {
        a_pstParser->stCtx.iStructEndBuf = 0;
        a_pstParser->stCtx.iHasError = 0;
        a_pstParser->stCtx.iIsTaskDone = 1;
        return TDR_SUCCESS;
    }

    PARSER_SET_TASK(a_pstParser, TDR_JSON_TASK_FIND_STRUCT_END);

    while ((ch = tdr_json_stream_get_next_char(&a_pstParser->stStream)))
    {
        JSON_parser_char(a_pstParser->pstParser, ch);
        if (PARSER_TASK_HAS_STOP(a_pstParser))
        {
            break;
        }
    }

    PARSER_TASK_CHECK_AND_RETURN(a_pstParser, ch);
}

int tdr_json_parser_match_array_begin(LPTDRJSONPARSER a_pstParser)
{
    char ch;

    assert(NULL != a_pstParser);

    PARSER_SET_TASK(a_pstParser, TDR_JSON_TASK_FIND_ARRAY_BEGIN);

    while ((ch = tdr_json_stream_get_next_char(&a_pstParser->stStream)))
    {
        JSON_parser_char(a_pstParser->pstParser, ch);
        if (PARSER_TASK_HAS_STOP(a_pstParser))
        {
            break;
        }
    }

    PARSER_TASK_CHECK_AND_RETURN(a_pstParser, ch);
}

int tdr_json_parser_match_array_end(LPTDRJSONPARSER a_pstParser)
{
    char ch;

    assert(NULL != a_pstParser);

    if (a_pstParser->stCtx.iArrayEndBuf)
    {
        a_pstParser->stCtx.iArrayEndBuf = 0;
        a_pstParser->stCtx.iHasError = 0;
        a_pstParser->stCtx.iIsTaskDone = 1;
        return TDR_SUCCESS;
    }

    PARSER_SET_TASK(a_pstParser, TDR_JSON_TASK_FIND_ARRAY_END);

    while ((ch = tdr_json_stream_get_next_char(&a_pstParser->stStream)))
    {
        JSON_parser_char(a_pstParser->pstParser, ch);
        if (PARSER_TASK_HAS_STOP(a_pstParser))
        {
            break;
        }
    }

    PARSER_TASK_CHECK_AND_RETURN(a_pstParser, ch);
}

int tdr_json_parser_get_next_key_or_end(LPTDRJSONPARSER a_pstParser)
{
    char ch;

    assert(NULL != a_pstParser);

    if (0 != a_pstParser->stCtx.pszBuf[0])
    {
        a_pstParser->stCtx.iIsTaskDone = 1;
        return TDR_SUCCESS;
    }

    if (a_pstParser->stCtx.iStructEndBuf)
    {
        a_pstParser->stCtx.iIsTaskDone = 1;
        return TDR_SUCCESS;
    }

    PARSER_SET_TASK(a_pstParser, TDR_JSON_TASK_FIND_NEXT_KEY_OR_END);

    while ((ch = tdr_json_stream_get_next_char(&a_pstParser->stStream)))
    {
        JSON_parser_char(a_pstParser->pstParser, ch);
        if (PARSER_TASK_HAS_STOP(a_pstParser))
        {
            break;
        }
    }

    PARSER_TASK_CHECK_AND_RETURN(a_pstParser, ch);
}

int tdr_json_parser_get_next_key(LPTDRJSONPARSER a_pstParser)
{
    char ch;

    assert(NULL != a_pstParser);

    if (0 != a_pstParser->stCtx.pszBuf[0])
    {
        a_pstParser->stCtx.iIsTaskDone = 1;
        return TDR_SUCCESS;
    }

    PARSER_SET_TASK(a_pstParser, TDR_JSON_TASK_FIND_NEXT_KEY);

    while ((ch = tdr_json_stream_get_next_char(&a_pstParser->stStream)))
    {
        JSON_parser_char(a_pstParser->pstParser, ch);
        if (PARSER_TASK_HAS_STOP(a_pstParser))
        {
            break;
        }
    }

    PARSER_TASK_CHECK_AND_RETURN(a_pstParser, ch);
}

int tdr_json_parser_get_simple_value(LPTDRJSONPARSER a_pstParser)
{
    char ch;

    assert(NULL != a_pstParser);

    PARSER_SET_TASK(a_pstParser, TDR_JSON_TASK_FIND_SIMPLE_VALUE);

    while ((ch = tdr_json_stream_get_next_char(&a_pstParser->stStream)))
    {
        JSON_parser_char(a_pstParser->pstParser, ch);
        if (PARSER_TASK_HAS_STOP(a_pstParser))
        {
            break;
        }
    }

    PARSER_TASK_CHECK_AND_RETURN(a_pstParser, ch);
}

int tdr_json_parser_skip_value(LPTDRJSONPARSER a_pstParser)
{
    char ch;

    assert(NULL != a_pstParser);

    PARSER_SET_TASK(a_pstParser, TDR_JSON_TASK_SKIP_CURRENT_PAIR);
    a_pstParser->stCtx.pszBuf[0] = 0;
    a_pstParser->stCtx.iLevel = 0;

    while ((ch = tdr_json_stream_get_next_char(&a_pstParser->stStream)))
    {
        JSON_parser_char(a_pstParser->pstParser, ch);
        if (PARSER_TASK_HAS_STOP(a_pstParser))
        {
            break;
        }
    }

    PARSER_TASK_CHECK_AND_RETURN(a_pstParser, ch);
}

int tdr_json_parser_skip_entry(LPTDRJSONPARSER a_pstParser, const char* a_pszEntry)
{
    int iRet = TDR_SUCCESS;

    assert(NULL != a_pstParser);
    assert(NULL != a_pszEntry);

    if (a_pstParser->stCtx.iStructEndBuf)
    {
        return TDR_SUCCESS;
    }

    iRet = tdr_json_parser_get_next_key(a_pstParser);
    if (TDR_ERR_IS_ERROR(iRet))
    {
        return iRet;
    }

    if (strcmp(a_pstParser->stCtx.pszBuf, a_pszEntry))
    {
        return TDR_SUCCESS;
    }

    iRet = tdr_json_parser_skip_value(a_pstParser);
//
//    if (!TDR_ERR_IS_ERROR(iRet))
//    {
//        a_pstParser->stCtx.pszBuf[0] = 0;
//    }

    return iRet;
}

int tdr_json_parser_match_entry_begin(LPTDRJSONPARSER a_pstParser, LPTDRMETAENTRY a_pstEntry)
{
    int iRet = TDR_SUCCESS;

    assert(NULL != a_pstParser);
    assert(NULL != a_pstEntry);

    /*
    iRet = tdr_json_parser_get_next_key(a_pstParser);
    if (TDR_ERR_IS_ERROR(iRet))
    {
        return iRet;
    }

    if (strcmp(a_pstParser->stCtx.pszBuf, tdr_get_entry_name(a_pstEntry)))
    {
        return TDR_SUCCESS;
    }
    */

    if (1 != a_pstEntry->iCount)
    {
        iRet = tdr_json_parser_match_array_begin(a_pstParser);
    }

    if (!TDR_ERR_IS_ERROR(iRet) && TDR_TYPE_COMPOSITE >= a_pstEntry->iType)
    {
        iRet = tdr_json_parser_match_struct_begin(a_pstParser);
    }

    if (!TDR_ERR_IS_ERROR(iRet))
    {
        a_pstParser->stCtx.pszBuf[0] = 0;
    }

    return iRet;
}

int tdr_json_parser_match_entry_end(LPTDRJSONPARSER a_pstParser, LPTDRMETAENTRY a_pstEntry)
{
    int iRet = TDR_SUCCESS;

    assert(NULL != a_pstParser);
    assert(NULL != a_pstEntry);

    if (TDR_TYPE_COMPOSITE >= a_pstEntry->iType)
    {
        iRet = tdr_json_parser_match_struct_end(a_pstParser);
    }

    if (!TDR_ERR_IS_ERROR(iRet) && 1 != a_pstEntry->iCount)
    {
        iRet = tdr_json_parser_match_array_end(a_pstParser);
    }

    if (!TDR_ERR_IS_ERROR(iRet))
    {
        a_pstParser->stCtx.pszBuf[0] = 0;
    }

    return iRet;
}

/**
 * - TDR float/double                          对应 JSON float
 * - TDR string/wstring/date/time/datetime/ip  对应 JSON string
 * - TDR wchar 等其他数据类型　                对应 JSON integer
 */
int tdr_json_parser_simple_entry(LPTDRJSONPARSER a_pstParser, LPTDRMETALIB a_pstLib,
                                 LPTDRMETAENTRY a_pstEntry, char** a_ppszHostStart,
                                 const char* a_pszHostEnd)
{
    int iRet = TDR_SUCCESS;
    size_t iSize = 0;

    assert(NULL != a_pstParser);
    assert(NULL != a_pstLib);
    assert(NULL != a_pstEntry);
    assert(NULL != a_ppszHostStart);
    assert(NULL != *a_ppszHostStart);
    assert(NULL != a_pszHostEnd);

    assert(TDR_TYPE_COMPOSITE < a_pstEntry->iType);

    iRet = tdr_json_parser_get_simple_value(a_pstParser);
    if (TDR_ERR_IS_ERROR(iRet))
    {
        return iRet;
    }

    switch (a_pstParser->stCtx.eType)
    {
        case TDR_JSON_TYPE_FLOAT:
        case TDR_JSON_TYPE_STRING:
        case TDR_JSON_TYPE_INTEGER:
            break;
        default:
            iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_JSON_INVALIED_FORMAT);
            return iRet;
            break;
    }

    iSize = a_pszHostEnd - *a_ppszHostStart;
    iRet = tdr_ioscanf_basedtype_i(a_pstLib, a_pstEntry, *a_ppszHostStart,
                                   &iSize, a_pstParser->stCtx.pszBuf);
    if (TDR_ERR_IS_ERROR(iRet))
    {
        return iRet;
    }
    *a_ppszHostStart += iSize;


    return 0;
}

/*! \brief JSON parser callback

    \param ctx The pointer passed to new_JSON_parser.
    \param type An element of JSON_type but not JSON_T_NONE.
    \param value A representation of the parsed value. This parameter is NULL for
        JSON_T_ARRAY_BEGIN, JSON_T_ARRAY_END, JSON_T_OBJECT_BEGIN, JSON_T_OBJECT_END,
        JSON_T_NULL, JSON_T_TRUE, and JSON_T_FALSE. String values are always returned
        as zero-terminated C strings.

    \return Non-zero if parsing should continue, else zero.
*/
int tdr_json_state_proc(void* ctx, int type, const JSON_value* value)
{
    TDRJSONCTX* lpCtx = (TDRJSONCTX*)ctx;

    assert(NULL != lpCtx);

    switch (lpCtx->eTask)
    {
        case TDR_JSON_TASK_FIND_STRUCT_BEGIN:
            {
                switch (type)
                {
                    case JSON_T_OBJECT_BEGIN:
                        lpCtx->iIsTaskDone = 1;
                        return 1;
                        break;
                    default:
                        lpCtx->iHasError = 1;
                        return 0;
                        break;
                }
            }
            break;
        case TDR_JSON_TASK_FIND_STRUCT_END:
            {
                switch (type)
                {
                    case JSON_T_OBJECT_END:
                        lpCtx->iIsTaskDone = 1;
                        return 1;
                        break;
                    default:
                        lpCtx->iHasError = 1;
                        return 0;
                        break;
                }
            }
            break;
        case TDR_JSON_TASK_FIND_ARRAY_BEGIN:
            {
                switch (type)
                {
                    case JSON_T_ARRAY_BEGIN:
                        lpCtx->iIsTaskDone = 1;
                        return 1;
                        break;
                    default:
                        lpCtx->iHasError = 1;
                        return 0;
                        break;
                }
            }
            break;
        case TDR_JSON_TASK_FIND_ARRAY_END:
            {
                switch (type)
                {
                    case JSON_T_ARRAY_END:
                        lpCtx->iIsTaskDone = 1;
                        return 1;
                        break;
                    default:
                        lpCtx->iHasError = 1;
                        return 0;
                        break;
                }
            }
            break;
        case TDR_JSON_TASK_FIND_NEXT_KEY:
            {
                switch (type)
                {
                    case JSON_T_KEY:
                        lpCtx->iIsTaskDone = 1;
                        strncpy(lpCtx->pszBuf, value->vu.str.value, lpCtx->iBuf - 1);
                        return 1;
                        break;
                    default:
                        lpCtx->iHasError = 1;
                        return 0;
                        break;
                }
            }
        case TDR_JSON_TASK_FIND_NEXT_KEY_OR_END:
            {
                switch (type)
                {
                    case JSON_T_KEY:
                        lpCtx->iIsTaskDone = 1;
                        strncpy(lpCtx->pszBuf, value->vu.str.value, lpCtx->iBuf - 1);
                        return 1;
                        break;
                    case JSON_T_OBJECT_END:
                        lpCtx->iIsTaskDone = 1;
                        lpCtx->iStructEndBuf = 1;
                        return 1;
                        break;
                    default:
                        lpCtx->iHasError = 1;
                        return 0;
                        break;
                }
            }
            break;
        case TDR_JSON_TASK_FIND_SIMPLE_VALUE:
            {
                switch (type)
                {
                    case JSON_T_INTEGER:
                        lpCtx->eType = TDR_JSON_TYPE_INTEGER;
                        strncpy(lpCtx->pszBuf, value->vu.str.value, lpCtx->iBuf - 1);
                        lpCtx->iIsTaskDone = 1;
                        return 1;
                        break;

                    case JSON_T_FLOAT:
                        lpCtx->eType = TDR_JSON_TYPE_FLOAT;
                        strncpy(lpCtx->pszBuf, value->vu.str.value, lpCtx->iBuf - 1);
                        lpCtx->iIsTaskDone = 1;
                        return 1;
                        break;

                    case JSON_T_NULL:
                        lpCtx->eType = TDR_JSON_TYPE_NULL;
                        lpCtx->iIsTaskDone = 1;
                        return 1;
                        break;

                    case JSON_T_TRUE:
                        lpCtx->eType = TDR_JSON_TYPE_TRUE;
                        lpCtx->iIsTaskDone = 1;
                        return 1;
                        break;

                    case JSON_T_FALSE:
                        lpCtx->eType = TDR_JSON_TYPE_FALSE;
                        lpCtx->iIsTaskDone = 1;
                        return 1;
                        break;

                    case JSON_T_STRING:
                        lpCtx->eType = TDR_JSON_TYPE_STRING;
                        strncpy(lpCtx->pszBuf, value->vu.str.value, lpCtx->iBuf - 1);
                        lpCtx->iIsTaskDone = 1;
                        return 1;
                        break;

                    case JSON_T_ARRAY_END:
                        lpCtx->iArrayEndBuf = 1;
                        return 1;
                        break;

                    case JSON_T_OBJECT_END:
                        lpCtx->iStructEndBuf = 1;
                        return 1;
                        break;

                    case JSON_T_ARRAY_BEGIN:
                    case JSON_T_OBJECT_BEGIN:
                    default:
                        lpCtx->iHasError = 1;
                        return 0;
                        break;
                }
            }
            break;
        case TDR_JSON_TASK_SKIP_CURRENT_PAIR:
            {
                switch(type) {
                    case JSON_T_ARRAY_BEGIN:
                        ++lpCtx->iLevel;
                        break;
                    case JSON_T_ARRAY_END:
                        if (lpCtx->iLevel > 0)
                            --lpCtx->iLevel;
                        else
                            lpCtx->iArrayEndBuf = 1;
                        break;
                    case JSON_T_OBJECT_BEGIN:
                        ++lpCtx->iLevel;
                        break;
                    case JSON_T_OBJECT_END:
                        if (lpCtx->iLevel > 0)
                            --lpCtx->iLevel;
                        else
                            lpCtx->iStructEndBuf = 1;
                        break;
                    case JSON_T_INTEGER:
                    case JSON_T_FLOAT:
                    case JSON_T_NULL:
                    case JSON_T_TRUE:
                    case JSON_T_FALSE:
                    case JSON_T_STRING:
                    case JSON_T_KEY:
                        break;
                    default:
                        {
                            lpCtx->iHasError = 1;
                            return 0;
                        }
                        break;
                }

                if (0 == lpCtx->iLevel)
                {
                    lpCtx->iIsTaskDone = 1;
                }
                return 1;
            }
            break;
        default:
            return 0;
            break;
    }

    return 1;
}

int tdr_json_parser_init(LPTDRJSONPARSER a_pstParser, JSON_parser_callback a_pfnProc, int a_iLevel)
{
    JSON_config config;

    assert(NULL != a_pstParser);
    assert(NULL != a_pfnProc);
    assert(0 < a_iLevel);

    a_pstParser->stCtx.iLevel = 0;

    init_JSON_config(&config);

    config.depth                  = a_iLevel;
    config.callback               = a_pfnProc;
    config.allow_comments         = 1;
    config.handle_floats_manually = 0;
    config.callback_ctx           = &a_pstParser->stCtx;

    a_pstParser->pstParser = new_JSON_parser(&config);
    if (NULL == a_pstParser->pstParser)
    {
        return -1;
    }

    /* TODO: decide iBuf size */
    a_pstParser->stCtx.iBuf = 2000000;
    a_pstParser->stCtx.pszBuf = malloc(a_pstParser->stCtx.iBuf);
    if (NULL == a_pstParser->stCtx.pszBuf)
    {
        delete_JSON_parser(a_pstParser->pstParser);
        a_pstParser->pstParser = NULL;
        return -1;
    }

    a_pstParser->stCtx.pszBuf[0] = 0;
    a_pstParser->stCtx.pszBuf[a_pstParser->stCtx.iBuf - 1] = 0;

    a_pstParser->stCtx.eTask = TDR_JSON_TASK_UNKNOWN;
    a_pstParser->stCtx.eType = TDR_JSON_TYPE_UNKOWN;

    a_pstParser->stCtx.iStructEndBuf = 0;
    a_pstParser->stCtx.iArrayEndBuf = 0;

    return 0;
}

void tdr_json_parser_fini(LPTDRJSONPARSER a_pstParser)
{
    assert(NULL != a_pstParser);

    if (NULL != a_pstParser->pstParser)
    {
        delete_JSON_parser(a_pstParser->pstParser);
        a_pstParser->pstParser = NULL;
    }

    if (NULL != a_pstParser->stCtx.pszBuf)
    {
        free(a_pstParser->stCtx.pszBuf);
        a_pstParser->stCtx.pszBuf = NULL;
        a_pstParser->stCtx.iBuf = 0;
    }
}
