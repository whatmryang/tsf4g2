#ifndef _TLIB_DB_C_
#define _TLIB_DB_C_

#include <stdio.h>
#include "tlib_db.h"

void TLib_DB_Init(TLIB_DB_LINK *pstDBLink,int iMultiDBConn)
{
	memset(pstDBLink, 0, sizeof(TLIB_DB_LINK));
	
	//初始化mysql的连接
	mysql_init(&(pstDBLink->stMysqlConn.stMysql));
	
	pstDBLink->iResNotNull = 0;
	pstDBLink->iMultiDBConn = iMultiDBConn;	
	pstDBLink->stMysqlConn.pstNext=NULL;
	pstDBLink->pstCurMysqlConn = &(pstDBLink->stMysqlConn); //设置当前连接指针
	pstDBLink->pstCurMysqlConn->iDBConnect = 0;
}

int TLib_DB_CloseCurDB(TLIB_DB_LINK *pstDBLink)
{

	mysql_close(&(pstDBLink->pstCurMysqlConn->stMysql));
	pstDBLink->pstCurMysqlConn->iDBConnect = 0;
	
	return 0;
}

int TLib_DB_CloseDatabase(TLIB_DB_LINK *pstDBLink)
{
	if (pstDBLink->iResNotNull == 1)
	{
		mysql_free_result(pstDBLink->pstRes);
		pstDBLink->iResNotNull= 0;
	}		
	
	//如果是多连接则关闭除第一个的mysql连接，并释放链表中除第一个结构的所有项目
	if (pstDBLink->iMultiDBConn!=0)
	{
		TLIB_MYSQL_CONN *pstMyConn,*pstMysqlConn;
		pstMysqlConn=pstDBLink->stMysqlConn.pstNext;
		while (pstMysqlConn!=NULL)
		{
			//关闭mysql的连接
			if (pstMysqlConn->iDBConnect == 1)
			{
				mysql_close(&(pstMysqlConn->stMysql));
			}
			pstMyConn = pstMysqlConn;
			pstMysqlConn = pstMysqlConn->pstNext;
			free(pstMyConn);
		}
	}	
		
	//关闭链表中第一个mysql连接
	if (pstDBLink->stMysqlConn.iDBConnect == 1)
		mysql_close(&(pstDBLink->stMysqlConn.stMysql));
	
		
	//设置当前连接指针
	pstDBLink->stMysqlConn.iDBConnect = 0;
	pstDBLink->stMysqlConn.pstNext = NULL;

	pstDBLink->pstCurMysqlConn = &(pstDBLink->stMysqlConn);		
	pstDBLink->pstCurMysqlConn->sHostAddress[0] = '\0';
	
	return 0;
}

int TLib_DB_SetDB(TLIB_DB_LINK *pstDBLink, char *sHostAddress, char *sUserName, char *sPassword, char *sDBName, char *sErrMsg)
{
	int iSelectDB;
	
	
	iSelectDB = 0;
	
	//判断是否设置了只要一个mysql的连接
	if (pstDBLink->iMultiDBConn == 0)
	{
		//如果要连接的地址和当前的地址不是同一台机器则先close当前的连接,再重新建立连接
		if (strcmp(pstDBLink->pstCurMysqlConn->sHostAddress, sHostAddress) != 0)
		{
			if (pstDBLink->pstCurMysqlConn->iDBConnect==1)
			{
				TLib_DB_CloseDatabase(pstDBLink);
			}
		
#ifndef _MYSQL40_
			if (mysql_connect(&(pstDBLink->pstCurMysqlConn->stMysql), sHostAddress, sUserName, sPassword) == 0)
#else
		    if (mysql_real_connect(&(pstDBLink->pstCurMysqlConn->stMysql), sHostAddress, sUserName, sPassword, sDBName, 0,
 NULL, CLIENT_FOUND_ROWS) == 0)

#endif

			{
				sprintf(sErrMsg, "Fail To Connect To Mysql: %s", mysql_error(&(pstDBLink->pstCurMysqlConn->stMysql)));
				return -1;
			}	
			pstDBLink->pstCurMysqlConn->iDBConnect=1;
			iSelectDB = 1;
		}
		else  //如果当前没有连接，则连接mysql 
		{
			if (pstDBLink->pstCurMysqlConn->iDBConnect == 0)
			{
#ifndef  _MYSQL40_
				if (mysql_connect(&(pstDBLink->pstCurMysqlConn->stMysql), sHostAddress, sUserName, sPassword) == 0)
#else
				if (mysql_real_connect(&(pstDBLink->pstCurMysqlConn->stMysql), sHostAddress, sUserName, sPassword, sDBName, 0,
 NULL, CLIENT_FOUND_ROWS) == 0)
#endif
				{
					sprintf(sErrMsg, "Fail To Connect To Mysql: %s", mysql_error(&(pstDBLink->pstCurMysqlConn->stMysql)));
					return -1;
				}	
				pstDBLink->pstCurMysqlConn->iDBConnect=1;
			}	
		}
	}
	//多个mysql连接的
	else
	{
		//如果要连接的地址和当前的地址不同
		if (strcmp(pstDBLink->pstCurMysqlConn->sHostAddress, sHostAddress) != 0)
		{
			
			TLIB_MYSQL_CONN *pstMysqlConn;
			TLIB_MYSQL_CONN *pstMysqlConnTail;//pstMysqlConnTail是尾指针
			
			//先释放现在的Res
			if (pstDBLink->iResNotNull == 1)
			{
				mysql_free_result(pstDBLink->pstRes);
				pstDBLink->iResNotNull=0;
			}
			
			//先判断当前是否已经连接上Db,如果没有就可以利用此连接connect sHostAddress的DB,
			if (pstDBLink->pstCurMysqlConn->iDBConnect == 1)
			{
				//试图从其他连接找出已有相同地址的mysql连接
				pstMysqlConn=&(pstDBLink->stMysqlConn);	
				pstMysqlConnTail=&(pstDBLink->stMysqlConn);
				pstDBLink->pstCurMysqlConn=NULL;
				
				while (pstMysqlConn!=NULL)
				{
					if (strcmp(pstMysqlConn->sHostAddress,sHostAddress) == 0)
					{
						pstDBLink->pstCurMysqlConn=pstMysqlConn;
						break;
					}
					pstMysqlConnTail=pstMysqlConn;
					pstMysqlConn=pstMysqlConn->pstNext;
				}
				
			
				//没有相同的则创建一个新的连接
				if (pstDBLink->pstCurMysqlConn==NULL)
				{
					pstMysqlConn = (TLIB_MYSQL_CONN *)malloc(sizeof(TLIB_MYSQL_CONN));
					memset(pstMysqlConn, 0, sizeof(TLIB_MYSQL_CONN));
					mysql_init(&(pstMysqlConn->stMysql));
					pstMysqlConnTail->pstNext = pstMysqlConn;
					pstMysqlConn->pstNext = NULL;
					pstDBLink->pstCurMysqlConn = pstMysqlConn;
				}
			}	
			
			if (pstDBLink->pstCurMysqlConn->iDBConnect == 0)
			{
				
#ifndef  _MYSQL40_
				if (mysql_connect(&(pstDBLink->pstCurMysqlConn->stMysql), sHostAddress, sUserName, sPassword) == 0)
#else
				if (mysql_real_connect(&(pstDBLink->pstCurMysqlConn->stMysql), sHostAddress, sUserName, sPassword, sDBName, 0,
 NULL, CLIENT_FOUND_ROWS) == 0)
#endif

				{
					sprintf(sErrMsg, "Fail To Connect To Mysql: %s", mysql_error(&(pstDBLink->pstCurMysqlConn->stMysql)));
					return -1;
				}	
				//设置当前连接指针
				pstDBLink->pstCurMysqlConn->iDBConnect=1;
				iSelectDB = 1;
			}
		}
		else if (pstDBLink->pstCurMysqlConn->iDBConnect==0)
		{
#ifndef _MYSQL40_
			if (mysql_connect(&(pstDBLink->pstCurMysqlConn->stMysql), sHostAddress, sUserName, sPassword) == 0)
#else
			if (mysql_real_connect(&(pstDBLink->pstCurMysqlConn->stMysql), sHostAddress, sUserName, sPassword, sDBName, 0,
 NULL, CLIENT_FOUND_ROWS) == 0)
#endif
			{
				sprintf(sErrMsg, "Fail To Connect To Mysql: %s", mysql_error(&(pstDBLink->pstCurMysqlConn->stMysql)));
				return -1;
			}	
			pstDBLink->pstCurMysqlConn->iDBConnect=1;
			iSelectDB = 1;
		}
		
	}

	if ((iSelectDB != 0) || (strcmp(pstDBLink->pstCurMysqlConn->sDBName, sDBName) != 0))
	{
		int iSelectRet = mysql_select_db(&(pstDBLink->pstCurMysqlConn->stMysql), sDBName);
		if(iSelectRet != 0)
		{
			//TLib_DB_CloseDatabase(pstDBLink);
			sprintf(sErrMsg, "Cannot(%d) Select Database %s: %s",
				iSelectRet, sDBName, mysql_error(&(pstDBLink->pstCurMysqlConn->stMysql)));
			return -1;
		}
	}	
	
	strcpy(pstDBLink->pstCurMysqlConn->sHostAddress, sHostAddress);
	strcpy(pstDBLink->pstCurMysqlConn->sDBName, sDBName);	
	strcpy(sErrMsg, "");
	
	return 0;
}

int TLib_DB_ExecSQL(TLIB_DB_LINK *pstDBLink, char *sErrMsg) 
{
	int   iRetCode;
	
	// 检查参数是否正确
	if (pstDBLink->iQueryType != 0)
	{
		if ((pstDBLink->sQuery[0]!='s') && (pstDBLink->sQuery[0]!='S'))
		{
			sprintf(sErrMsg, "QueryType=1, But SQL is not select");
			return -1;
		}
	}

	// 是否需要关闭原来RecordSet
	if (pstDBLink->iResNotNull==1)
	{
		mysql_free_result(pstDBLink->pstRes);
		pstDBLink->iResNotNull=0;
	}		
	
	if ((pstDBLink->pstCurMysqlConn ==NULL) || (pstDBLink->pstCurMysqlConn->iDBConnect == 0))
	{
		strcpy(sErrMsg, "Has Not Connect To DB Server Yet");
		return -1;
	}

	// 执行相应的SQL语句
	iRetCode =mysql_query(&(pstDBLink->pstCurMysqlConn->stMysql), pstDBLink->sQuery);
	if (iRetCode != 0)
	{
		//出错信息
		sprintf(sErrMsg, "Fail To Execute SQL: %s", mysql_error(&(pstDBLink->pstCurMysqlConn->stMysql)));
		return -1;
	}
	
	// 保存结果
	if (pstDBLink->iQueryType == 1)
	{
		pstDBLink->pstRes = mysql_store_result(&(pstDBLink->pstCurMysqlConn->stMysql));
		pstDBLink->iResNum = mysql_num_rows(pstDBLink->pstRes);
		pstDBLink->iResNotNull = 1;
	}

	return 0;
}

int TLib_DB_ExecSQL2(TLIB_DB_LINK *pstDBLink, int *iErrNo,char *sErrMsg) 
{
	int   iRetCode;
	
	// 检查参数是否正确
	if (pstDBLink->iQueryType != 0)
	{
		if ((pstDBLink->sQuery[0]!='s') && (pstDBLink->sQuery[0]!='S'))
		{
			sprintf(sErrMsg, "QueryType=1, But SQL is not select");
			return -1;
		}
	}

	// 是否需要关闭原来RecordSet
	if (pstDBLink->iResNotNull==1)
	{
		mysql_free_result(pstDBLink->pstRes);
		pstDBLink->iResNotNull=0;
	}		
	
	if ((pstDBLink->pstCurMysqlConn ==NULL) || (pstDBLink->pstCurMysqlConn->iDBConnect == 0))
	{
		strcpy(sErrMsg, "Has Not Connect To DB Server Yet");
		return -1;
	}

	// 执行相应的SQL语句
	iRetCode =mysql_query(&(pstDBLink->pstCurMysqlConn->stMysql), pstDBLink->sQuery);
	if (iRetCode != 0)
	{
		//出错信息
		*iErrNo=mysql_errno(&(pstDBLink->pstCurMysqlConn->stMysql));
		sprintf(sErrMsg, "Fail To Execute SQL: %s", mysql_error(&(pstDBLink->pstCurMysqlConn->stMysql)));
		return -1;
	}
	
	// 保存结果
	if (pstDBLink->iQueryType == 1)
	{
		pstDBLink->pstRes = mysql_store_result(&(pstDBLink->pstCurMysqlConn->stMysql));
		pstDBLink->iResNum = mysql_num_rows(pstDBLink->pstRes);
		pstDBLink->iResNotNull = 1;
	}

	return 0;
}

int TLib_DB_FetchRow(TLIB_DB_LINK  *pstDBLink, char *sErrMsg)
{
	if (pstDBLink->iResNotNull == 0)
	{
		sprintf(sErrMsg, "Recordset is Null");
		return -1;	
	}		
	
	if (pstDBLink->iResNum == 0)
	{
		sprintf( sErrMsg, "Recordset count=0");
		return -1;	
	}		

	pstDBLink->stRow = mysql_fetch_row(pstDBLink->pstRes);
	return 0;
}

int TLib_DB_FreeResult(TLIB_DB_LINK  *pstDBLink)
{
	if (pstDBLink->iResNotNull==1)
	{
		mysql_free_result(pstDBLink->pstRes);
		pstDBLink->iResNotNull=0;
	}		
	
	return 0;
}

int TLib_DB_AffectedRow(TLIB_DB_LINK  *pstDBLink, char *sErrMsg)
{
	if ((pstDBLink->pstCurMysqlConn ==NULL) || (pstDBLink->pstCurMysqlConn->iDBConnect == 0))
	{
		strcpy(sErrMsg, "Has Not Connect To DB Server Yet");
		return -1;
	}

	return mysql_affected_rows(&(pstDBLink->pstCurMysqlConn->stMysql));
}

#endif
