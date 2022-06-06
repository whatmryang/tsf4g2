/**
*
* @file     tbusd_ctrl_kernel.h 
* @brief    ���ƺ��Ĵ���
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
#ifndef TBUS_CTRL_KERNEL_H
#define TBUS_CTRL_KERNEL_H

#include <stdio.h>
#include "pal/pal.h"
#include "tbusd_ctrl_proto.h"

#define TBUSD_CTRL_PKG_META_NAME    "TbusCtrlPkg"

#ifdef __cplusplus
extern "C" 
{
#endif 


    /** ��ʼ��������Ϣ������ ���ӿ��Ƿ��̰߳�ȫ�ġ�
    */
    int tbusdctrl_init();

    /*�������ͷ������Դ��Դ
    */
    void tbusdctrl_fini();


    /** ���������Ϣ�������͵�����ͨ���С����ӿ��Ƿ��̰߳�ȫ�ġ�
    @param[in] a_pstPkg �����͵�����
    @return 0 �ɹ� ��0��ʧ��
    */
    int tbusdctrl_send_pkg(IN  LPTBUSCTRLPKG a_pstPkg);

    /** ���������Ϣ
    @param[out] a_pstPkg �����������Ϣ
    @param[in] a_pszNet ������Ϣ������
    @param[in] a_iLen ������Ϣ����
    @return 0 �ɹ� ��0��ʧ��
    */
    int tbusdctrl_unpack_pkg(OUT  LPTBUSCTRLPKG a_pstPkg, const char *a_pszNet, int a_iLen);


    /** ��ӡ������Ϣ
    @param[in] a_pstPkg �����������Ϣ
    @return 0 �ɹ� ��0��ʧ��
    */
    int tbusdctrl_print_pkg(OUT  LPTBUSCTRLPKG a_pstPkg, FILE *fpOut);


    //��ʼ��������Ϣͷ��
    void tbusdctrl_fill_pkghead(OUT TBUSCTRLHEAD *a_pstHead, unsigned int a_dwCmd, unsigned int a_dwVer);

#ifdef __cplusplus
}
#endif 
#endif /*#ifndef TBUS_CTRL_KERNEL_H*/
