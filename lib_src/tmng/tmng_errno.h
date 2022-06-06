#ifndef _TMNG_ERRNO_H
#define _TMNG_ERRNO_H


#include "pal/pal.h"

#if defined(_WIN32) || defined(_WIN64)
#define __inline__
#endif

typedef enum
{
    TMNG_ERR_OK = 0,    // 没有错误发生
//-->error_table tmng code begin
    TMNG_ERR_ARGS,  // 参数检查没有通过
    TMNG_ERR_COMP,  // 不匹配的数据
    TMNG_ERR_FULL,  // 容量已满
    TMNG_ERR_LOCKOPEN,  // 打开锁失败
    TMNG_ERR_MEMORY,  // 内存分配失败
    TMNG_ERR_NULLMETA,  // tdr 中没有找到匹配的元数据
    TMNG_ERR_SHMAT,  // 共享内存获取失败
    TMNG_ERR_SHMDEL,  // 共享内存删除失败
    TMNG_ERR_SHMOPEN,  // 共享内存打开失败
    TMNG_ERR_SIZE,  // 超出预定尺寸
    TMNG_ERR_TDR_LOAD_METALIB,  // tdr_load_metalib 函数调用失败
    TMNG_ERR_VERSION,  // 版本不匹配
    TMNG_ERR_TMB_FIND_UNLOCKED,  // 共享内存中没有找到匹配的元数据
    TMNG_ERR_ALIGN,  // 数据对其错误
    TMNG_ERR_TDR_FPRINTF,  // tdr_printf 函数调用失败
    TMNG_ERR_TMIB_GET_DATA,  // 没有找到匹配的 tmib 数据
    TMNG_ERR_CANCEL,  // 操作取消
    TMNG_ERR_HELP,  // 可以执行 -h 获取更多信息
    TMNG_ERR_SET_ENTRY_VALUE,  // 设置变量出错
    TMNG_ERR_TDR_CREATE_LIB_FILE,  // tdr_create_lib_file 函数调用出错
    TMNG_ERR_TDR_DATA_TYPE,  // tdr 数据类型错误
    TMNG_ERR_TDR_ENTRY_PATH_TO_OFF,  // tdr_entry_path_to_off 函数调用出错
    TMNG_ERR_TDR_INPUT_FILE,  // tdr_input_file 函数调用出错
//-->error_table tmng code end
} TMNG_ERRNO;

static const char *g_ppszErrStr[] = {
    "没有错误发生",
//-->error_table tmng string begin
    "参数检查没有通过",
    "不匹配的数据",
    "容量已满",
    "打开锁失败",
    "内存分配失败",
    "tdr 中没有找到匹配的元数据",
    "共享内存获取失败",
    "共享内存删除失败",
    "共享内存打开失败",
    "超出预定尺寸",
    "tdr_load_metalib 函数调用失败",
    "版本不匹配",
    "共享内存中没有找到匹配的元数据",
    "数据对其错误",
    "tdr_printf 函数调用失败",
    "没有找到匹配的 tmib 数据",
    "操作取消",
    "可以执行 -h 获取更多信息",
    "设置变量出错",
    "tdr_create_lib_file 函数调用出错",
    "tdr 数据类型错误",
    "tdr_entry_path_to_off 函数调用出错",
    "tdr_input_file 函数调用出错",
//-->error_table tmng string end
};

static __inline__ const char *tmng_errstr(int a_iErrNo)
{
    if (0 > a_iErrNo)
    {
        a_iErrNo = -a_iErrNo;
    }

    if (sizeof(g_ppszErrStr) / sizeof(*g_ppszErrStr) <= (size_t)a_iErrNo)
    {
        return "错误号超出范围";
    }

    return g_ppszErrStr[a_iErrNo];
}

#endif




