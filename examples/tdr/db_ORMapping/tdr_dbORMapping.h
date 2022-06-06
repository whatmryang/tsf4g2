/**
*
* @file     tdr_dbORMapping.h 
* @brief    TDR���ݿ�����ϵӳ��ʾ������
*
* 
* @author jackyai  
* @version 1.0
* @date 2007-07-31 
*
*
* Copyright (c)  2007, ��Ѷ�Ƽ����޹�˾���������з���
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

#define TDR_EX_MAN_SQL_LEN	100000	/**<SQL���������󳤶�*/

/**�������ݿ�ϵͳ�����ã���DB����Ự��� 
*/
int tdrex_open_dbhandle(OUT TDRDBHANDLE *a_phDHandle, IN const char *a_pszDBMSConf);

/**�������� 
*/
int insert_data(IN TDRDBHANDLE a_hDBHandle, IN LPTDRMETALIB a_pstLib, LPTESTEXTENDTABLE a_pstData);

/**�������� 
*/
int update_data(IN TDRDBHANDLE a_hDBHandle, IN LPTDRMETALIB a_pstLib, LPTESTEXTENDTABLE a_pstData);


/**�������� 
*/
int select_data(IN TDRDBHANDLE a_hDBHandle, IN LPTDRMETALIB a_pstLib, LPTESTEXTENDTABLE a_pstData);

/**�������� 
*/
int select_data_quickly(IN TDRDBHANDLE a_hDBHandle, IN LPTDRMETALIB a_pstLib, LPTESTEXTENDTABLE a_pstData);

/**ɾ������ 
*/
int delete_data(IN TDRDBHANDLE a_hDBHandle, IN LPTDRMETALIB a_pstLib, LPTESTEXTENDTABLE a_pstData);



#endif
