/**
*
* @file     tbusd_controller.h 
* @brief    ���ƶ˴���
* 
* @author jackyai  
* @version 1.0
* @date 2010-1-27 
*
*
* Copyright (c)  2008-2010, ��Ѷ�Ƽ����޹�˾���������з����ܹ���
* All rights reserved.
*
*/
#ifndef TBUS_CONTROLLER_H
#define TBUS_CONTROLLER_H

#include "tapp/tapp.h"

#ifdef __cplusplus
extern "C" 
{
#endif 

//tbusd ���ƶ˳�ʼ��
int tbusd_contorller_init(struct tagTAPPCTX *pstCtx, void* pvArg);

//�����tbusd���صĿ�����Ӧ
int tbusd_proc_ctrlres(const char* pszMetaName, const char* pBuf, int tLen);
#ifdef __cplusplus
}
#endif 
#endif /*#ifndef TBUS_CONTROLLER_H*/
