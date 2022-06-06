/**
*
* @file     tdr_metalib_entry_manage_i.h   
* @brief    TSF4G-Ԫ����������entryԪ�ع���ģ��
* 
* @author steve jackyai  
* @version 1.0
* @date 2007-04-16 
*
*
* Copyright (c)  2007, ��Ѷ�Ƽ����޹�˾���������з���
* All rights reserved.
*
*/

#ifndef TDR_METALIB_ENTRY_MANAGE_H
#define TDR_METALIB_ENTRY_MANAGE_H

#include <scew/scew.h>

#include "tdr/tdr_types.h"
#include "tdr_XMLtags_i.h"

/**< string wstring sizeinfo����ȱʡֵ*/
#define  TDR_STRING_DEFULT_SIZEINFO_VALUE  TDR_TAG_INT

/** ��pstElementָ���entryԪ����Ϊ��Ա�ӵ�meta�ṹ��
*@param[out] a_pstMeta ����˳�Ա���Զ������ݽṹ
*@param[in] a_pstElement Ҫ�����Ԫ��
*@param[in] a_fpError ���������Ϣ���ļ����
*
*@return �ɹ�����TDR_SUCCESS�����򷵻ش������
*
*@pre \e a_pstMeta ����ΪNULL
*@pre \e a_pstElement ����ΪNULL
*@pre \e a_fpError ����ΪNULL
*/
int tdr_add_entry_to_meta_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError);


/** ��pstElementָ���indexԪ����Ϊ��Ա�ӵ�meta�ṹ��
*@param[out] a_pstMeta ����˳�Ա���Զ������ݽṹ
*@param[in] [out] a_piColumnNum ����˽ڵ�ǰ�����нڵ��column���Եĸ����������ۻ���ǰ�Ľڵ�ĸ���
*@param[in] a_pstElement Ҫ�����Ԫ��
*@param[in] a_fpError ���������Ϣ���ļ����
*
*@return �ɹ�����TDR_SUCCESS�����򷵻ش������
*
*@pre \e a_pstMeta ����ΪNULL
*@pre \e a_piColumnNum ����ΪNULL
*@pre \e a_pstElement ����ΪNULL
*@pre \e a_fpError ����ΪNULL
*/
int tdr_add_index_to_meta_i(LPTDRMETA a_pstMeta, int *a_piColumnNum, scew_element *a_pstElement, FILE *a_fpError);

/**���ݱ���ƫ��ֲ����meta�ж�λ��ƫ��ֵ��Ӧ��·�� 
*@param[in] a_pstLib ��Ҫ��λ��meta
*@param[in] a_pstSortKey    ��Ҫ��λ��ƫ��
*/
int tdr_sortkeyinfo_to_path_i(LPTDRMETALIB a_pstLib, LPTDRSORTKEYINFO a_pstSortKey, char* a_pszBuff, int a_iBuff);






#endif /*TDR_METALIB_ENTRY_MANAGE_H*/
