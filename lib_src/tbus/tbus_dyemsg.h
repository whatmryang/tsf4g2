/**
*
* @file     tbus_dyemsg.h 
* @brief    tbusȾɫ��ؽӿ�
* 
* @author jackyai  
* @version 1.0
* @date 2008-05-16 
*
*
* Copyright (c)  2008, ��Ѷ�Ƽ����޹�˾���������з����ܹ���
* All rights reserved.
*
*/

#ifndef TBUS_DYEMSG_H
#define TBUS_DYEMSG_H

#include "tbus/tbus.h"
#include "tbus_head.h"


/** ����Ⱦɫ��־λ��ǰһ����Ϣ����Ϣ����Ⱦɫ
*/
int tbus_dye_pkg(IN LPTBUSHEAD a_pstHead, IN LPTBUSHEAD a_pstPreHead, int a_iFlag);



/**
@brief ��Ⱦɫ����Ϣ���м�¼
@param
@note
*/
void tbus_log_dyedpkg(IN LPTBUSHEAD a_pstHead, IN const char *a_pszOperation) ;

#endif
