/**
*
* @file     tdr_dbdriver_mysql_ext.h 
* @brief    TSF-G-DR mysql数据库管理系统驱动接口扩展
* 
* @author vinsonzuo  
* @version 1.0
* @date 2010-11-09 
*
*
* Copyright (c)  2007, 腾讯科技有限公司互动娱乐研发部
* All rights reserved.
*
*/

#ifndef TDR_DBDRIVER_MYSQL_EXT_H
#define TDR_DBDRIVER_MYSQL_EXT_H

#include "tdr/tdr_types.h"

#define TDR_COMPRESS_METHOD_NO 0
#define TDR_COMPRESS_METHOD_ZLIB 1

#define TDR_COMPRESS_ZLIB_VERSION125 1

#define TDR_BLOB_VERSION_OLD 0
#define TDR_BLOB_VERSION_ADD_ZLIB_COMPRESS 1




/**清除统计信息
*/
void tdr_dbdriver_clear_statinfo(TDRDBHANDLE a_phDBHandle);

/*输出统计信息*/
void tdr_dbdriver_output_statinfo(TDRDBHANDLE a_hDBHandle, char* a_pszOut, int a_iLen);


#endif
