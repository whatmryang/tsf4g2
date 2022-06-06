/**
*
* @file     tdr_XMLMetalib_i.h 
* @brief    Ԫ������������XML�����໥ת�� �ڲ�����
* 
* @author steve jackyai  
* @version 1.0
* @date 2007-03-22 
*
*
* Copyright (c)  2007, ��Ѷ�Ƽ����޹�˾���������з���
* All rights reserved.
*
*/

#ifndef TDR_XMLMETALIB_I_H
#define TDR_XMLMETALIB_I_H


#include <stdio.h>
#include "tdr/tdr_define_i.h"
#include "tdr/tdr_types.h"


/**���ݲ���ΪԪ���ݿ����ռ䣬����ʼ��
*@param[out] pstLib ������Ԫ���ݿ�
*@Param[in]	stLibParam ��ʼ��Ԫ���ݵĲ���
*@return �ɹ�����0�����򷵻ش������
*/
int tdr_init_metalib_i(LPTDRMETALIB *ppstLib, LPTDRLIBPARAM pstLibParam);





#endif /*TDR_XMLMETALIB_I_H*/
