/**
*
* @file     tdr_dbORMapping.h 
* @brief    TDR数据库对象关系映射示例程序
*
* 
* @author jackyai  
* @version 1.0
* @date 2007-07-31 
*
*
* Copyright (c)  2007, 腾讯科技有限公司互动娱乐研发部
* All rights reserved.
*
*/

#ifndef TDR_DBORMAPPING_H
#define TDR_DBORMAPPING_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "tdr/tdr.h"
#include "tdr_database.h"

#define TDR_EX_MAN_SQL_LEN	100000	/**<SQL操作语句最大长度*/

/**根据数据库系统的配置，打开DB处理会话句柄 
*/
int tdrex_open_dbhandle(OUT TDRDBHANDLE *a_phDHandle, IN const char *a_pszDBMSConf);

/**插入数据 
*/
int insert_data(IN TDRDBHANDLE a_hDBHandle, IN LPTDRMETALIB a_pstLib, LPTESTEXTENDTABLE a_pstData);

/**更新数据 
*/
int update_data(IN TDRDBHANDLE a_hDBHandle, IN LPTDRMETALIB a_pstLib, LPTESTEXTENDTABLE a_pstData);


/**检索数据 
*/
int select_data(IN TDRDBHANDLE a_hDBHandle, IN LPTDRMETALIB a_pstLib, LPTESTEXTENDTABLE a_pstData);

/**检索数据 
*/
int select_data_quickly(IN TDRDBHANDLE a_hDBHandle, IN LPTDRMETALIB a_pstLib, LPTESTEXTENDTABLE a_pstData);

/**删除数据 
*/
int delete_data(IN TDRDBHANDLE a_hDBHandle, IN LPTDRMETALIB a_pstLib, LPTESTEXTENDTABLE a_pstData);



#endif
