/**
 *
 * @file    tlogany.h
 * @brief   日志设备记录模块
 *
 * @author steve
 * @version 1.0
 * @date 2007-04-05
 *
 *
 * Copyright (c)  2007, 腾讯科技有限公司互动娱乐研发部
 * All rights reserved.
 *
 */

#ifndef TLOGANY_H
#define TLOGANY_H

#include "tlog_i.h"
#include "tlogio.h"
#include "tlogmem.h"
#include "tlogfiledef.h"
#include "tlogmemdef.h"
#include "tlognetdef.h"
#include "tlogvecdef.h"
#include "tloganydef.h"

#ifdef __cplusplus
extern "C"
{
#endif

union tagTLogDevSelectorInst
{
    TLOGFILE stFile;
    TLOGNET stNet;
    TLOGVEC stVec;
    TLOGMEM stMem;
};

typedef union tagTLogDevSelectorInst TLOGDEVSELECTORINST;
typedef union tagTLogDevSelectorInst *LPTLOGDEVSELECTORINST;

struct tagTLogDevAnyInst
{
    int iType;
    TLOGDEVSELECTORINST stAny;

};

typedef struct tagTLogDevAnyInst TLOGDEVANYINST;
typedef struct tagTLogDevAnyInst *LPTLOGDEVANYINST;

struct tagTLogAny
{
    TLOGDEVANY* pstDev;
    //TLOGDEVANY stDev;

    //int iNeedReload;
    //TLOGDEVANY stTlogAnyNewConf;
    TLOGDEVANYINST stInst;
};

typedef struct tagTLogAny TLOGANY;
typedef struct tagTLogAny *LPTLOGANY;

/**
 * 写入初始化tlog文件结构
 * @param[in,out] a_pstLogAny tlogAny结构
 * @param[in] a_pstDev     文件配置
 *
 * @return 0 成功
 *		非0 失败
 */
int tlogany_init(TLOGANY* a_pstLogAny, TLOGDEVANY* a_pstDev);

/**
 * 清空tlog设备结构
 * @param[in,out] a_pstLogAny tlogAny结构
 *
 * @return 0 成功
 *		非0 失败
 */
int tlogany_fini(TLOGANY* a_pstLogAny);

/**
 * 清空tlog设备结构
 * @param[in,out] a_pstLogAny tlogAny结构
 * @param[in] a_iID 日志的ID，通常为UIN
 * @param[in] a_iCls 日志的CLS，用户自己定义，可以过滤
 * @param[in] a_pszBuff 日志字符串内容
 * @param[in] a_iBuff 日志字符串内容长度
 *
 * @return 0 成功
 *		非0 失败
 */
int tlogany_write(TLOGANY* a_pstLogAny, int a_iID, int a_iCls,
        const char* a_pszBuff, int a_iBuff);

/**
 * 清空tlog设备结构
 * @param[in,out] a_pstLogAny tlogAny结构
 * @param[in] a_iID 日志的ID，通常为UIN
 * @param[in] a_iCls 日志的CLS，用户自己定义，可以过滤
 * @param[in] a_pstIOVec 日志字符串数组
 * @param[in] a_iCount 日志字符串数组长度
 *
 * @return 0 成功
 *		非0 失败
 */
int tlogany_writev(TLOGANY* a_pstLogAny, int iId, int iCls,
        const TLOGIOVEC* a_pstIOVec, int a_iCount);

#ifdef __cplusplus
}
#endif

#endif /* TLOGANY_H */

