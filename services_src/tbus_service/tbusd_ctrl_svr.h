/**
*
* @file     tbusd_ctrl_svr.h  
* @brief    tbusd�����п��ƴ���ģ��
* 
* @author jackyai  
* @version 1.0
* @date 2010-1-26 
*
*
* Copyright (c)  2008-2010, ��Ѷ�Ƽ����޹�˾���������з����ܹ���
* All rights reserved.
*
*/
#ifndef TBUS_PROC_CTRLPKG_H
#define TBUS_PROC_CTRLPKG_H


#ifdef __cplusplus
extern "C" 
{
#endif 

/*��ȡ���̿���ʹ�ð���*/
const char* tbusd_get_ctrl_usage(void);

/**������������� 
*/
int tbusd_proc_ctrl_cmdline(struct tagTAPPCTX *a_pstCtx, void* a_pvArg, unsigned short argc, const char** argv);



#ifdef __cplusplus
}
#endif 
#endif /*#ifndef TBUS_PROC_CTRLPKG_H*/
