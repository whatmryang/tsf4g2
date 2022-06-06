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
#ifndef TDR_JSON_PARSER_H
#define TDR_JSON_PARSER_H

#include "tdr/tdr_os.h"
#include "tdr/tdr_define.h"
#include "tdr/tdr_data_io.h"
#include "tdr/tdr_error.h"
#include "tdr/tdr_metalib_init.h"
#include "tdr/tdr_metalib_kernel_i.h"
#include "tdr/tdr_ctypes_info_i.h"
#include "tdr/tdr_define_i.h"
#include "tdr/tdr_iostream_i.h"
#include "tdr/tdr_auxtools.h"
#include "tdr/tdr_metalib_manage_i.h"
#include "JSON_parser.h"

struct tagTdrJsonBuff
{
    char* pszBuff;
    size_t tBuffer;
    size_t tPosition;
};
typedef struct tagTdrJsonBuff     TDRJSONBUF;
typedef struct tagTdrJsonBuff*  LPTDRJSONBUF;

union TdrJsonStreams
{
    TDRJSONBUF stBuff;
    FILE* fp;
};

enum TdrJsonStremmType {
    TDR_JSON_STREAM_TYPE_UNKOWN,
    TDR_JSON_STREAM_TYPE_BUF,
    TDR_JSON_STREAM_TYPE_FP,
};

struct tagTdrJsonStream
{
    enum TdrJsonStremmType eType;
    union TdrJsonStreams unStreams;
};
typedef struct tagTdrJsonStream      TDRJSONSTREAM;
typedef struct tagTdrJsonStream*   LPTDRJSONSTREAM;


enum TdrJsonStateTask
{
    TDR_JSON_TASK_UNKNOWN,
    TDR_JSON_TASK_FIND_STRUCT_BEGIN,
    TDR_JSON_TASK_FIND_STRUCT_END,
    TDR_JSON_TASK_FIND_ARRAY_BEGIN,
    TDR_JSON_TASK_FIND_ARRAY_END,
    TDR_JSON_TASK_FIND_NEXT_KEY,
    TDR_JSON_TASK_FIND_NEXT_KEY_OR_END,
    TDR_JSON_TASK_FIND_SIMPLE_VALUE,
    TDR_JSON_TASK_SKIP_CURRENT_PAIR,
    TDR_JSON_TASK_MAX,
};

enum TdrJsonSimpleValueType
{
    TDR_JSON_TYPE_UNKOWN,
    TDR_JSON_TYPE_INTEGER,
    TDR_JSON_TYPE_FLOAT,
    TDR_JSON_TYPE_NULL,
    TDR_JSON_TYPE_TRUE,
    TDR_JSON_TYPE_FALSE,
    TDR_JSON_TYPE_STRING,
    TDR_JSON_TYPE_MAX,
};

struct tagTdrJsonCtx
{
    char* pszBuf;
    int iBuf;
    int iLevel;
    int iIsTaskDone;
    int iHasError;
    int iStructEndBuf;
    int iArrayEndBuf;
    enum TdrJsonStateTask eTask;
    enum TdrJsonSimpleValueType eType;
};
typedef struct tagTdrJsonCtx      TDRJSONCTX;
typedef struct tagTdrJsonCtx*   LPTDRJSONCTX;

struct tagTdrJsonParser
{
    TDRJSONSTREAM stStream;
    TDRJSONCTX    stCtx;
    struct JSON_parser_struct* pstParser;
};

typedef struct tagTdrJsonParser      TDRJSONPARSER;
typedef struct tagTdrJsonParser*   LPTDRJSONPARSER;

void tdr_json_stream_init_by_buf(LPTDRJSONSTREAM a_pstStream, char* a_pszBuff, size_t a_tBuff);

void tdr_json_stream_init_by_fp(LPTDRJSONSTREAM a_pstStream, FILE* a_fp);

int tdr_json_stream_get_next_char(LPTDRJSONSTREAM a_pstStream);

int tdr_json_parser_init(LPTDRJSONPARSER a_pstParser, JSON_parser_callback a_pfnProc, int a_iLevel);
void tdr_json_parser_fini(LPTDRJSONPARSER a_pstParser);

/*! \brief JSON parser callback

    \param ctx The pointer passed to new_JSON_parser.
    \param type An element of JSON_type but not JSON_T_NONE.
    \param value A representation of the parsed value. This parameter is NULL for
        JSON_T_ARRAY_BEGIN, JSON_T_ARRAY_END, JSON_T_OBJECT_BEGIN, JSON_T_OBJECT_END,
        JSON_T_NULL, JSON_T_TRUE, and JSON_T_FALSE. String values are always returned
        as zero-terminated C strings.

    \return Non-zero if parsing should continue, else zero.
*/
int tdr_json_state_proc(void* ctx, int type, const JSON_value* value);

int tdr_json_parser_match_struct_begin(LPTDRJSONPARSER a_pstParser);
int tdr_json_parser_match_struct_end(LPTDRJSONPARSER a_pstParser);
int tdr_json_parser_match_array_begin(LPTDRJSONPARSER a_pstParser);
int tdr_json_parser_match_array_end(LPTDRJSONPARSER a_pstParser);
int tdr_json_parser_get_next_key(LPTDRJSONPARSER a_pstParser);
int tdr_json_parser_get_simple_value(LPTDRJSONPARSER a_pstParser);
int tdr_json_parser_skip_value(LPTDRJSONPARSER a_pstParser);
int tdr_json_parser_get_next_key_or_end(LPTDRJSONPARSER a_pstParser);

int tdr_json_parser_simple_entry(LPTDRJSONPARSER a_pstParser, LPTDRMETALIB a_pstLib,
                                 LPTDRMETAENTRY a_pstEntry, char** a_ppszHostStart,
                                 const char* a_pszHostEnd);
int tdr_json_parser_match_entry_begin(LPTDRJSONPARSER a_pstParser, LPTDRMETAENTRY a_pstEntry);
int tdr_json_parser_match_entry_end(LPTDRJSONPARSER a_pstParser, LPTDRMETAENTRY a_pstEntry);
int tdr_json_parser_skip_entry(LPTDRJSONPARSER a_pstParser, const char* a_pszEntry);

#endif
