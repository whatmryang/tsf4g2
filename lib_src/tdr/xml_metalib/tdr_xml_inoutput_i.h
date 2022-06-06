/**
*
* @file     tdr_xml_inoutput_i.h 
* @brief    xml�����ļ����������ؽӿ�
* 
* @author steve jackyai  
* @version 1.0
* @date 2009-04-30
*
*
* Copyright (c)  2007, ��Ѷ�Ƽ����޹�˾���������з���
* All rights reserved.
*
*/

#ifndef TDR_XML_INOUTPUT_I_H
#define TDR_XML_INOUTPUT_I_H


#include <stdio.h>
#include <scew/scew.h>
#include "tdr/tdr_define.h"
#include "tdr/tdr_types.h"

/**����XML�ļ��������XMLԪ���� 
*/
int tdr_create_XMLParser_tree_byFileName(scew_tree **a_ppstTree, const char *a_pszFileName, FILE *a_fpError);

/**����XML������Ϣ����XMLԪ����
*@param[out] a_ppstTree ������Ԫ������ָ��
*@Param[in]	a_pszXml ����XML��Ϣ�Ļ�������ַ
*@Param[in]	a_iXml ����XML��Ϣ�Ļ�������С
*@param[in]	a_fpError	�������������Ϣ���ļ����
*@return �ɹ�����0�����򷵻ش������
*/
int tdr_create_XMLParser_tree_byBuff_i(scew_tree **a_ppstTree, IN const char* a_pszXml, IN size_t a_iXml, FILE* a_fpError);

/**����XML�ļ��������XMLԪ���� 
*/
int tdr_create_XMLParser_tree_byfp(scew_tree **a_ppstTree, FILE *a_fpXML, FILE *a_fpError);


int tdr_input_i(IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstHost, IN scew_element *a_pstRoot, 
				IN int a_iCutOffVersion, IN int a_iIOVersion);


/**����TDR_XML_DATA_FORMAT_ATTR_ENTRY_NAME��ʽ�洢��xml�ļ��ж�ȡĳ���ڵ��������Ϣ
*@param[in] a_pstMeta ����������Ϣ�����ݽṹ���������
*@param[in] a_pstNode xml�ڵ�
*@param[in] a_iFirstEntry ָ����ʼ��Ա������a_iFirstEntryָ���ĳ�Ա��ʼ��ȡ
*@param[in] a_pszHostStart ����������Ϣ���ڴ滺������ʼ��ַ
*@param[in] a_iSize	����������Ϣ���ڴ滺�������ô�С����λΪ�ֽ�
*@param[in] a_iVersion ���ݵİ汾
@return succ: zero, failed: nonzero
*/
int tdr_input_meta_attrs_i(IN LPTDRMETA a_pstMeta, IN scew_element *a_pstNode, IN int a_iFirstEntry,
						   IN char *a_pszHostStart,  IN ptrdiff_t a_iSize, int a_iVersion);

/**����TDR_XML_DATA_FORMAT_ATTR_ENTRY_NAME��ʽ�洢��xml�ļ��ж�ȡ��Ϣ
*@param[in] a_pstMeta ����������Ϣ�����ݽṹ���������
*@param[in] a_pstRoot xml�ڵ�
*@param[in,out] a_pstHost �������ݵĻ�������Ϣ
	-	in	a_pstHost->iBuff	���ݻ������Ŀ��ÿռ��С
	-	out a_pstHost->iBuff	���ݻ�������ʵ��ʹ�ô�С
*@param[in] a_iCutOffVersion ���ݵİ汾
@return succ: zero, failed: nonzero
*/
int tdr_input_by_attr_xml_i(IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstHost, IN scew_element *a_pstRoot, 
							IN int a_iCutOffVersion);

#endif /* TDR_XML_INOUTPUT_I_H*/
