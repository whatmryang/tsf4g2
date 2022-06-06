/******************************************************************************
   ��Ȩ���� (C), 2001-2011, ��Ѹ�����ϵͳ���޹�˾

******************************************************************************
  �ļ���          : dbctrl.cpp
  �汾��          : 1.0
  ����            : ����
  ��������        : 2003-7-8
  ����޸�        : 
  ��������        : �����򣬸����ʼ������ѭ��
  					�����ڴ��ʼ����tcp��ʼ�����ʵ�Ŀ¼��ʼ����
  �����б�        : 
  �޸���ʷ        : 
  DATE			AUTHOR		DESCRIPTION
  --------------------------------------------------------------------------
  2003-7-8		����		����
  2005-5-20		luckylin	����mysql_connect�궨��, �޸ĵ��ţ�dbsvrd_002_20050520
*******************************************************************************/

#ifndef _TLIB_DB_H_
#define _TLIB_DB_H_

#include <stdlib.h>
#include <string.h>
#include "mysql.h"

//add by andyguo 20061027
//����Magic Number
//��ǰ������SQL����Ƿ񷵻�Recordset, 0=��Ҫ��1=select
enum
{
	QUERY_TYPE_SAVE = 0,
	QUERY_TYPE_SELECT = 1,
};


// begin, added by luckylin, �޸ĵ��ţ�dbsvrd_002_20050520
#if MYSQL_VERSION_ID >= 40000
#define mysql_connect(m,h,u,p) mysql_real_connect((m),(h),(u),(p),NULL,0,NULL,0)
#endif
// end, added by luckylin, �޸ĵ��ţ�dbsvrd_002_20050520

typedef struct _TLIB_MYSQL_CONN_
{
	struct _TLIB_MYSQL_CONN_ *pstNext;
	
	char			sHostAddress[50];	//DB Server �ĵ�ַ
	char			sUserName[50];		//�û���
	char			sPassword[50];		//����
	char			sDBName[50];		//Database ����
	
	MYSQL			stMysql;		//��ǰ�򿪵�Mysql����
	
	int			iDBConnect;		//�Ƿ��Ѿ������϶�Ӧ��Database, 0=�Ͽ���1=������	
	
} TLIB_MYSQL_CONN;

typedef struct _TLIB_DB_LINK_
{
	TLIB_MYSQL_CONN		stMysqlConn;		//Mysql��������ĵ�һ��
	TLIB_MYSQL_CONN		*pstCurMysqlConn;	//��ǰ�򿪵�Mysql����ָ��

	MYSQL_RES		*pstRes;		//��ǰ������RecordSet
	MYSQL_ROW		stRow;			//��ǰ������һ��
	  	
	int			iResNotNull;		//��ǰ������RecordSet�Ƿ�Ϊ��,0=�գ�1=�ǿ�
	int			iResNum;		//��ǰ������RecordSet�ļ�¼��Ŀ
	char		sQuery[1024 * 1024 * 2];		//��ǰ������SQL���
	int			iQueryType;		//��ǰ������SQL����Ƿ񷵻�Recordset, 0=��Ҫ��1=select
	int			iMultiDBConn;		//�Ƿ� 0=ֻ��һ��mysql���ӣ�1=���mysql����
}TLIB_DB_LINK;

void TLib_DB_Init(TLIB_DB_LINK *pstDBLink,int iMultiDBConn);
int  TLib_DB_SetDB(TLIB_DB_LINK *pstDBLink, char *sHostAddress, char *sUserName, char *sPassword, char *sDBName, char *sErrMsg);

int  TLib_DB_CloseDatabase(TLIB_DB_LINK *pstDBLink);
int  TLib_DB_CloseCurDB(TLIB_DB_LINK *pstDBLink);
int  TLib_DB_ExecSQL(TLIB_DB_LINK *pstDBLink, char *sErrMsg);
int  TLib_DB_ExecSQL2(TLIB_DB_LINK *pstDBLink, int* iErrNo,char *sErrMsg);
int  TLib_DB_FreeResult(TLIB_DB_LINK  *pstDBLink);
int  TLib_DB_FetchRow(TLIB_DB_LINK  *pstDBLink, char *sErrMsg);
int  TLib_DB_AffectedRow(TLIB_DB_LINK  *pstDBLink, char *sErrMsg);


#endif
