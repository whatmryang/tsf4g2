/**
*
* @file    tlog_layout.h  
* @brief   ��־��ʽ��ģ��
* 
* @author steve
* @version 1.0
* @date 2007-04-05 
*
*
* Copyright (c)  2007, ��Ѷ�Ƽ����޹�˾���������з���
* All rights reserved.
*
*/


#ifndef TLOG_LAYOUT_H
#define TLOG_LAYOUT_H

#include "tlog_event_i.h"

#ifdef __cplusplus
extern "C"
{
#endif


#define TLOG_LAYOUT_DEF_FORMAT  "[%d] %m"

/**
* ��ʽ����־����
* @param[in,out] a_pstEvt ��־�¼��ṹ��
* @param[in] a_pszFmt ��־��ʽ����
*

*/
void tlog_layout_format(TLOGEVENTBASE* a_pstEvt, const char* a_pszFmt);

#ifdef __cplusplus
}
#endif

#endif /* TLOG_LAYOUT_H */


