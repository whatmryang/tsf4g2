/**
*
* @file     tdr_metaleb_param_i.h  
* @brief    Ԫ���������������ȡ
* 
* @author steve jackyai  
* @version 1.0
* @date 2007-04-05 
*
*
* Copyright (c)  2007, ��Ѷ�Ƽ����޹�˾���������з���
* All rights reserved.
*
*/

#ifndef TDR_METALIB_PARAM_I_H
#define TDR_METALIB_PARAM_I_H


#include <stdio.h>
#include <scew/scew.h>

#include "tdr/tdr_define.h"
#include "tdr/tdr_types.h"


/**��ʼ��metalib������
*�����ṹ��Ŀռ��ں��������
* @pre \e a_pstParam ����Ϊ��
*/
void tdr_init_lib_param_i(LPTDRLIBPARAM a_pstParam);

/** 
*����XML����������Ϣ���metalib�Ĳ����������������з��ִ���
*��¼��fpErrorָ����ļ���
*@param[out] a_pstParam ��Ҫ��Ӹ��µĲ����ṹ
*@param[in] a_pstTree XML������
*@param[in] a_iTagsVersion XML��ǩ���汾
*@param[in] a_fpError ��¼���������Ϣ���ļ����
*@retval <0 �������
*@retval 0 �ɹ�
*@retval >0 �ɹ����������������
*
*@pre \e a_pstParam ����ΪNULL
*@pre \e a_pstTree ����ΪNULL
*@pre \e a_fpError ����ΪNULL
*@pre \e a_iTagsVersion Ϊ�Ϸ��İ汾��
*/
int tdr_add_lib_param_i(LPTDRLIBPARAM a_pstParam, IN scew_tree* a_pstTree, IN int a_iTagsVersion, IN FILE *a_fpError);


/** 
*����XML������������metalib�Ĳ����������������з��ִ���
*��¼��fpErrorָ����ļ���
*@param[out] a_pstParam ��Ҫ��Ӹ��µĲ����ṹ
*@param[in] a_pstTree XML������
*@param[in] a_iTagsVersion XML��ǩ���汾
*@param[in] a_fpError ��¼���������Ϣ���ļ����
*@retval <0 �������
*@retval 0 �ɹ�
*@retval >0 �ɹ����������������
*
*@pre \e a_pstParam ����ΪNULL
*@pre \e a_pstTree ����ΪNULL
*@pre \e a_fpError ����ΪNULL
*/
int tdr_get_lib_param_i(LPTDRLIBPARAM a_pstParam, IN scew_tree* a_pstTree, IN int a_iTagsVersion, IN FILE *a_fpError);


/**��ȡ�����ṹ�������ڴ�ռ�
*@param[out] a_piEntirsNum ��ȡ��meta������entry����Ŀ
*@param[out] a_piMetaSize ��ȡ��metaԪ�ش洢��ռ�õ��ֽ���
*@param[out] a_piStrBufSize ��ȡ��metaԪ�����ַ�������������ռ���ֽ���
*@param[in] a_iTagversion ��meta������ǩ�İ汾
*@param[in] a_pstElement �迼���Ԫ��
*@param[in] a_pstRoot ��Ԫ��
*/
int tdr_get_meta_size_i(int *a_piEntirsNum, size_t *a_piMetaSize, size_t *a_piStrBufSize, IN int a_iTagversion,
						scew_element *a_pstElement, scew_element *a_pstRoot);

/**��ȡstruct�ṹ�嶨���indexԪ�صĸ���/����indexԪ����column����ָ���������еĸ���*/
int tdr_get_meta_index_info_i(int *a_piIndexNum, int *a_piColumnNum, int *a_piStrLen,
                              scew_element *a_pstElement, scew_element *a_pstRoot);

/**��ȡ�����궨���������ڴ�ռ�
*@param[out] a_piMacrosNum ��ȡ�˺궨���������macro��Ŀ
*@param[out] a_piGroupSize ��ȡ�˺궨����Ԫ�ش洢��ռ�õ��ֽ���
*@param[out] a_piStrBufSize ��ȡ��metaԪ�����ַ�������������ռ���ֽ���
*@param[in] a_pstElement �迼���Ԫ��
*/
int tdr_get_macrosgroup_size_i(int *a_piMacrosNum, size_t *a_piGroupSize, size_t *a_piStrBufSize, scew_element *a_pstElement);



/** 
*ȡ��XML��������Ԫ�ص�version����ֵ�������������з��ִ���
*��¼��fpErrorָ����ļ���
*@param[out] a_plVersion �汾ֵ
*@param[in] pstTree XML������
*@param[in] fpError ��¼���������Ϣ���ļ����
*@retval <0 �������
*@retval 0 �ɹ�
*@retval >0 �ɹ����������������
*
*@pre \e a_plVersion ����ΪNULL
*@pre \e pstTree ����ΪNULL
*@pre \e fpError ����ΪNULL
*/
int tdr_get_metalib_version_i(OUT int *a_plVersion, IN scew_element* a_pstRoot);

/**��ȡmetalib��tagsversion����
*/
int tdr_get_metalib_tagsversion_i(OUT int *a_piTagVersion, IN scew_element *pstRoot, IN int iTagsVersion);

#endif /* TDR_METALIB_PARAM_I_H*/
