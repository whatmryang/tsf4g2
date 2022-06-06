/**
*
* @file     tdr_dbdriver_mysql.h
* @brief    TSF-G-DR mysql���ݿ����ϵͳ����
*
* @author jackyai
* @version 1.0
* @date 2007-07-24
*
*
* Copyright (c)  2007, ��Ѷ�Ƽ����޹�˾���������з���
* All rights reserved.
*
*/

#ifndef TDR_DBDRIVER_MYSQL_H
#define TDR_DBDRIVER_MYSQL_H

#include "tdr/tdr_os.h"
#include <my_global.h>
#include <mysql.h>
#include "tdr/tdr_iostream_i.h"

#if defined(_WIN32) || defined(_WIN64)
#pragma comment(lib, "libmysql.lib")
#pragma comment(lib, "mysqlclient.lib")
#pragma comment(lib, "libzlib.lib")
#endif

#include "tdr/tdr_types.h"

#define  ZLIB_INTERNAL

#define TDR_DBMS_DEFAULT_SWAP_BUFF_SIZE		0x100000	/**<��������ת��ʱ�Ľ����ռ��С,ȱʡΪ1M */
#define TDR_DBMS_SWAP_BUFF_MIN_SIZE		(int)(sizeof(uint32_t) *2 + sizeof(uint8_t)*4)    /*��������С��С,���ڴ洢�ṹ��İ汾�ͳ���*/
#define TDR_DBMS_MAX_BLOB_STAT_COUNT 16

struct tagTDRDBMysqlConn
{
	TDRDBMS  stDbmsInfo;
	int			iIsDBConnected;			/**<�Ƿ��Ѿ������϶�Ӧ��Database, 0=�Ͽ���1=������*/
	MYSQL			stMysql;		/**<��ǰ�򿪵�Mysql����*/
};
typedef struct tagTDRDBMysqlConn TDRDBMYSQLCONN;
typedef struct tagTDRDBMysqlConn *LPTDRDBMYSQLCONN;

enum tagTDRDBResultSetStat
{
	TDR_DBRESULT_NONE = 0,	/**<Ϊ֪״̬*/
	TDR_DBRESULT_STORE,		/**<ͨ��mysql_strore_result��ȡ�����*/
	TDR_DBRESULT_USE,		/**<ͨ��mysql_use_result��ȡ�����*/
};
typedef enum tagTDRDBResultSetStat TDRDBRESULTSETSTAT;

struct tagTDRDBMysqlResult
{
	int			iIsResNotNull;	/**<��ǰ������RecordSet�Ƿ�Ϊ��,0=�գ�1=�ǿ�*/
	TDRDBRESULTSETSTAT enStat;	/**<�������״̬*/

	uint32_t dwFieldsNum;		/**<�����еĸ���*/
	MYSQL_FIELD *astFields;			/**<������и��е��ֶζ���*/
	unsigned long *aulFieldsLength; /**<������и��еĳ���*/
	uint32_t *aSortedFieldIdx;

	MYSQL_RES		*pstRes;		/**<��ǰ������RecordSet*/
	MYSQL_ROW		stRow;			/**<��ǰ������һ��*/
	MYSQL			*pstConn;		/**<��ǰ�򿪵�Mysql����*/
};
typedef struct tagTDRDBMysqlResult TDRDBMYSQLRESULT;
typedef struct tagTDRDBMysqlResult *LPTDRDBMYSQLRESULT;


struct tagTDRBlobStatInfo
{
	char szEntryName[64];
	uint32_t dwRawSize;
	uint32_t dwCompressedSize;
};


typedef struct tagTDRBlobStatInfo TDRBLOBSTATINFO;
typedef struct tagTDRBlobStatInfo *LPTDRBLOBSTATINFO;


struct tagTDRDBMysqlLink
{
	TDRDBMYSQLCONN		stMysqlConn;
	TDRDBMYSQLRESULT stRes;
	char *szSwapBuff;
	size_t iSwapSize;	/*�������Ĵ�С*/
	char *szSwapBuffCompress;
	size_t iSwapSizeCompress;	/*ѹ���������Ĵ�С*/

	uint32_t dwBlobStatCount;	/*��¼blob����ѹ��ͳ������*/
	TDRBLOBSTATINFO stBlobStat[TDR_DBMS_MAX_BLOB_STAT_COUNT];
};
typedef struct tagTDRDBMysqlLink TDRDBMYSQLLINK;
typedef struct tagTDRDBMysqlLink *LPTDRDBMYSQLLINK;





/**����mysql���Ӿ��
*/
int tdr_dbdriver_open_handle(OUT LPTDRDBHANDLE a_phDBHandle, IN LPTDRDBMS a_pstDBMS, IN char *a_pszErrMsg);

/**�ر�mysql���Ӿ��
*/
void tdr_dbdriver_close_handle(TDRDBHANDLE a_phDBHandle);


/**�ӽ������ȡ��һ����¼
*/
int tdr_dbms_fetch_row(IN TDRDBRESULTHANDLE a_hDBResult);

int tdr_set_table_options(IN LPTDRDBMS a_pstDBMS, IN LPTDRMETA a_pstMeta, INOUT LPTDRIOSTREAM a_pstIOStream);

#endif
