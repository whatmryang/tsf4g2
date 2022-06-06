/**
*
* @file     tdr_dbdriver_mysql_ext.h 
* @brief    TSF-G-DR mysql���ݿ����ϵͳ�����ӿ���չ
* 
* @author vinsonzuo  
* @version 1.0
* @date 2010-11-09 
*
*
* Copyright (c)  2007, ��Ѷ�Ƽ����޹�˾���������з���
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




/**���ͳ����Ϣ
*/
void tdr_dbdriver_clear_statinfo(TDRDBHANDLE a_phDBHandle);

/*���ͳ����Ϣ*/
void tdr_dbdriver_output_statinfo(TDRDBHANDLE a_hDBHandle, char* a_pszOut, int a_iLen);


#endif
