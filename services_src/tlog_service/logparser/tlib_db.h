/******************************************************************************
   版权所有 (C), 2001-2011, 腾迅计算机系统有限公司

******************************************************************************
  文件名          : dbctrl.cpp
  版本号          : 1.0
  作者            : 曾宇
  生成日期        : 2003-7-8
  最近修改        : 
  功能描述        : 主程序，负责初始化、主循环
  					共享内存初始化、tcp初始化、帐单目录初始化、
  函数列表        : 
  修改历史        : 
  DATE			AUTHOR		DESCRIPTION
  --------------------------------------------------------------------------
  2003-7-8		曾宇		生成
  2005-5-20		luckylin	增加mysql_connect宏定义, 修改单号：dbsvrd_002_20050520
*******************************************************************************/

#ifndef _TLIB_DB_H_
#define _TLIB_DB_H_

#include <stdlib.h>
#include <string.h>
#include "mysql.h"

//add by andyguo 20061027
//不用Magic Number
//当前操作的SQL语句是否返回Recordset, 0=不要，1=select
enum
{
	QUERY_TYPE_SAVE = 0,
	QUERY_TYPE_SELECT = 1,
};


// begin, added by luckylin, 修改单号：dbsvrd_002_20050520
#if MYSQL_VERSION_ID >= 40000
#define mysql_connect(m,h,u,p) mysql_real_connect((m),(h),(u),(p),NULL,0,NULL,0)
#endif
// end, added by luckylin, 修改单号：dbsvrd_002_20050520

typedef struct _TLIB_MYSQL_CONN_
{
	struct _TLIB_MYSQL_CONN_ *pstNext;
	
	char			sHostAddress[50];	//DB Server 的地址
	char			sUserName[50];		//用户名
	char			sPassword[50];		//密码
	char			sDBName[50];		//Database 名字
	
	MYSQL			stMysql;		//当前打开的Mysql连接
	
	int			iDBConnect;		//是否已经连接上对应的Database, 0=断开，1=连接上	
	
} TLIB_MYSQL_CONN;

typedef struct _TLIB_DB_LINK_
{
	TLIB_MYSQL_CONN		stMysqlConn;		//Mysql连接链表的第一项
	TLIB_MYSQL_CONN		*pstCurMysqlConn;	//当前打开的Mysql连接指针

	MYSQL_RES		*pstRes;		//当前操作的RecordSet
	MYSQL_ROW		stRow;			//当前操作的一行
	  	
	int			iResNotNull;		//当前操作的RecordSet是否为空,0=空，1=非空
	int			iResNum;		//当前操作的RecordSet的记录数目
	char		sQuery[1024 * 1024 * 2];		//当前操作的SQL语句
	int			iQueryType;		//当前操作的SQL语句是否返回Recordset, 0=不要，1=select
	int			iMultiDBConn;		//是否 0=只有一个mysql连接，1=多个mysql连接
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
