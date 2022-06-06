/**
*
* @file    tlogfile.h  
* @brief   日志文件记录模块
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

#ifndef TLOGFILE_H

#define TLOGFILE_H

#include <stdlib.h>
#include <stdio.h>
#include "tlogio.h"
#include "tlogfiledef.h"

#define TLOGFILE_PATH_LEN	(TLOGFILE_PATTERN_LEN + 64)
#define TLOGFILE_MAX_SIZE   4200000000UL


#ifdef __cplusplus
extern "C"
{
#endif


struct tagTLogDevFileInst
{
	FILE* fp;
	char* pszBuff;		/* if filebuff is set, here is file buffer pointer */
	time_t tLastMsg;
	int fDirty;
	time_t tLastSync;
	int iCurRotate;
	time_t tCreate;	                 /* the time for create file. */
	uint32_t iTail;
	int iRes;
	char szPath[TLOGFILE_PATH_LEN]; // 当前写入文件的路径
};

typedef struct tagTLogDevFileInst	TLOGDEVFILEINST;
typedef struct tagTLogDevFileInst	*LPTLOGDEVFILEINST;

struct tagTLogFile
{
	TLOGDEVFILE* pstDev;
	TLOGDEVFILEINST stInst;
};

typedef struct tagTLogFile			TLOGFILE;
typedef struct tagTLogFile			*LPTLOGFILE;


/**
* 写入初始化tlog文件结构
* @param[in,out] a_pstLogFile tlog文件结构
* @param[in] a_pstDev     文件配置
*
* @return 0 成功
*		非0 失败
*/
int tlogfile_init(TLOGFILE* a_pstLogFile, TLOGDEVFILE* a_pstDev);


/**
* 向文件中写入二进制串
* @param[in] a_pstLogFile tlog文件结构
* @param[in] a_pszBuff     二进制串指针
* @param[in] a_iBuff     二进制串长度
*
* @return 0 成功
*		非0 失败
*/
int tlogfile_write(TLOGFILE* a_pstLogFile, const char* a_pszBuff, int a_iBuff);

/**
* 释放文件结构
* @param[in,out] a_pstLogFile tlog文件结构
*
* @return 0 成功
*		非0 失败
*/
int tlogfile_fini(TLOGFILE* a_pstLogFile);

/**
*将缓冲区中数据写入文件
* @param[in,out] a_pstLogFile tlog文件结构
*
* @return 0 成功
*		非0 失败
*/
int tlogfile_sync(TLOGFILE* a_pstLogFile);


/**
*将多个缓冲区中数据写入文件
* @param[in,out] a_pstLogFile tlog文件结构
* @param[in,out] a_pstIOVec 需要写入的数据数组基地址
* @param[in,out] a_iCount 需要写入的数据数组元素个数
* @return 0 成功
*		非0 失败
*/
int tlogfile_writev(TLOGFILE* a_pstLogFile, const TLOGIOVEC* a_pstIOVec, int a_iCount);


// 将文件配置修改成bill的配置
int tlogfile_is_bill(TLOGFILE *a_pstLogFile);

#ifdef __cplusplus
}
#endif

#endif /* TLOGFILE_H */
