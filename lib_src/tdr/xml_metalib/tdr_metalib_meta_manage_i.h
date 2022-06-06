/**
*
* @file     tdr_metalib_meta_manage_i.h 
* @brief    TSF4G-Ԫ����������meta���Ĺ���ģ��
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


#ifndef TDR_METALIB_META_MANAGE_I_H
#define TDR_METALIB_META_MANAGE_I_H

#include <stddef.h>
#include <stdio.h>

#include <scew/scew.h>
#include "tdr/tdr_metalib_kernel_i.h"

/** ���궨��Ԫ���ӵ�ԭ����������
*@param[in,out] a_pstLib ��Ҫ���µ�Ԫ���ݿ�
*@param[in] a_pstElement �궨����Ԫ��
*@param[in] a_fpError ������¼���������Ϣ���ļ����
*
*@return �ɹ�����TDR_SUCCESS,���򷵻ش����
*
*@pre \e a_pstLib ����ΪNULL
*@pre \e a_pstElement ����ΪNULL
*@pre \e a_fpError ����ΪNULL
*/
int tdr_add_macro_i(TDRMETALIB* a_pstLib, scew_element *a_pstElement, FILE *a_fpError, INOUT LPTDRMACRO *a_ppMacro);

/** ��struct/union ��metaԪ�صĻ�����Ϣ�ӵ�ԭ���ݿ�
*@param[inout] a_pstLib ��Ҫ���µ�Ԫ���ݿ�
*@param[in] a_pstElement �궨����Ԫ��
*@param[in] a_fpError ������¼���������Ϣ���ļ����
*
*@return �ɹ�����TDR_SUCCESS,���򷵻ش����
*
*@pre \e a_pstLib ����ΪNULL
*@pre \e a_pstElement ����ΪNULL
*@pre \e a_fpError ����ΪNULL
*/
int tdr_add_meta_base_i(TDRMETALIB* a_pstLib, scew_element *a_pstElement, scew_element *a_pstRoot, FILE *a_fpError);

/**��metaԪ�ص��������Զ�ȡ��Ԫ���ݿ��� 
*@param[inout] a_pstMeta ��Ҫ���µ�meta
*@param[in] a_pstElement metaԪ��
*@param[in] a_fpError ������¼���������Ϣ���ļ����
*/
int tdr_get_meta_specail_attribute_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError);


/**��metaԪ���µ�index��Ԫ����ӵ�meta�ṹ����
*@param[out] a_pstMeta ���ڴ����meta
*@param[in] a_stElement meta��ָ��Ԫ��
*/
int tdr_add_meta_indexes_i(LPTDRMETA a_pstMeta, scew_element *a_stElement,
                           scew_element *a_pstRoot, FILE *a_fpError);

/**��metaԪ���µ�entry��Ԫ����ӵ�meta�ṹ����
*@param[out] a_pstMeta ���ڴ����meta
*@param[in] a_stElement meta��ָ��Ԫ��
*/
int tdr_add_meta_entries_i(LPTDRMETA a_pstMeta, scew_element *a_stElement, FILE *a_fpError);

/**���meta���ݵ���Ч��
*/
int tdr_check_meta_i(LPTDRMETA a_pstMeta, FILE *a_fpError);

/** ��tagsetversion=0�汾�ĺ궨�����µĺ�ӵ�ԭ���ݿ�
*@param[inout] a_pstLib ��Ҫ���µ�Ԫ���ݿ�
*@param[in] a_pstElement �궨����Ԫ��
*@param[in] a_fpError ������¼���������Ϣ���ļ����
*
*@return �ɹ�����TDR_SUCCESS,���򷵻ش����
*
*@pre \e a_pstLib ����ΪNULL
*@pre \e a_pstElement ����ΪNULL
*@pre \e a_fpError ����ΪNULL
*/
int tdr_add_macros_i(TDRMETALIB* a_pstLib, scew_element *a_pstElement, FILE *a_fpError);



/** ����cname����
*@param[out] a_ptrName cname�ַ������ַ����������е�ָ��
*@param[out] a_shNameLen cname�ַ����ĳ���
*@param[in] a_pstLib ��Ҫ���µ�Ԫ���ݿ�
*@param[in] a_pstElement Ҫ�����Ԫ��
*@param[in] a_fpError ���������Ϣ���ļ����
*
*@return �ɹ�����meta��ָ�룬���򷵻�NULL
*
*@pre \e a_ptrName ����ΪNULL
*@pre \e a_shNameLen ����С�ڵ���0
*@pre \e a_pstLib ����ΪNULL
*@pre \e a_pstElement ����ΪNULL
*/
int tdr_get_cname_attribute_i(TDRPTR *a_pptrName, LPTDRMETALIB a_pstLib, scew_element *a_pstElement, FILE *a_fpError);



/** ����Ԫ�ص�name������
*@param[out] pszName    �������ֵĻ�����
*@param[in] iNameSize name�������Ĵ�С
*@param[in] pstElement ����ӵ�Ԫ��
*
*@return �ɹ�����TDR_SUCCESS,���򷵻ش����
*
*@pre \e pszName ����ΪNULL
*@pre \e iNameSize �������0
*@pre \e pstElement ����ΪNULL
*/
int tdr_get_name_attribute_i(char *pszName, int iNameSize, scew_element *pstElement);


/**��ȡԪ�ص�id����
*@retval <0  ʧ��
*@retval 0  �ɹ���ȡ��id����
*@retval >0 û��id����
*/
int tdr_get_id_attribute_i(int *a_piID, int *a_piIdx, LPTDRMETALIB a_pstLib, scew_element *a_pstElement, FILE *a_fpError);

/**��ȡԪ�ص�desc����
*/
int tdr_get_desc_attribute_i(TDRPTR *a_pptrDesc, LPTDRMETALIB a_pstLib, scew_element *a_pstElement, FILE *a_fpError);




/**��ȡversion����
*/
int tdr_get_version_attribute_i(int *a_piVersion, int *a_piIdxVersion, LPTDRMETALIB a_pstLib, scew_element *a_pstElement, FILE *a_fpError);


/** ���궨����ӵ�Ԫ������������
*@param[in,out] a_pstLib ��Ҫ���µ�Ԫ���ݿ�
*@param[in] a_pstElement �궨����Ԫ��
*@param[in] a_fpError ������¼���������Ϣ���ļ����
*
*@return �ɹ�����TDR_SUCCESS,���򷵻ش����
*
*@pre \e a_pstLib ����ΪNULL
*@pre \e a_pstElement ����ΪNULL
*@pre \e a_fpError ����ΪNULL
*/
int tdr_add_macrosgroup_i(TDRMETALIB* a_pstLib, scew_element *a_pstElement, FILE *a_fpError);


/**���ݺ궨��������Ʋ��Ҵ˺궨�������ݽṹ
*/
LPTDRMACROSGROUP tdr_find_macrosgroup_i(IN const LPTDRMETALIB a_pstLib, IN const char *a_pszName);

#endif /* TDR_METALIB_META_MANAGE_I_H */
