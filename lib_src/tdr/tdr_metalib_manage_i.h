/**
*
* @file     tdr_metalib_manage_i.h
* @brief    TSF4G-Ԫ�������������ģ��
*
* @author steve jackyai
* @version 1.0
* @date 2007-07-19
*
*
* Copyright (c)  2007, ��Ѷ�Ƽ����޹�˾���������з���
* All rights reserved.
*
*/

#ifndef TDR_METALIB_MANAGE_I_H
#define TDR_METALIB_MANAGE_I_H


#include "tdr/tdr_types.h"
#include "tdr_metalib_kernel_i.h"
#include "tdr_iostream_i.h"
#include "tdr/tdr_metalib_manage.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**��������ƫ��ֲ����meta�ж�λ��ƫ��ֵ��Ӧ��·��
*@param[in] a_pstMeta ��Ҫ��λ��meta
*@param[in iEntry   ��meta�е�iEntry��entry��ƫ����iOffƥ��ʱ������Ӧ���ּ�¼Ϊthis
*@param[in] a_iNetOff    ��Ҫ��λ��ƫ��
*/
int tdr_netoff_to_path_i(LPTDRMETA a_pstMeta, int a_iEntry, TDROFF a_iNetOff, char* a_pszBuff, int a_iBuff);


/**���ݱ���ƫ��ֲ����meta�ж�λ��ƫ��ֵ��Ӧ��·��
*@param[in] a_pstMeta ��Ҫ��λ��meta
*@param[in] iEntry   ��meta�е�iEntry��entry��ƫ����iOffƥ��ʱ������Ӧ���ּ�¼Ϊthis
*@param[in] a_iHostOff    ��Ҫ��λ��ƫ��
*/
int tdr_hostoff_to_path_i(LPTDRMETA a_pstMeta, int a_iEntry, TDROFF a_iHostOff, char* a_pszBuff, int a_iBuff);



/**��DR·����ȡ����1���ڵ������
*@return ������һ���ڵ�ָ��
*/
char *tdr_get_first_node_name_from_path_i(char *pszName, int iNameSize, const char *pszPath);




/**����entry������entry��meta�е�����
*/
int tdr_get_entry_by_name_i(TDRMETAENTRY pstEntry[], int iMax, const char* pszName);


/**����index������index��meta�е�����
*/
int tdr_get_index_by_name_i(LPTDRMETAINDEX pstIndex, int iMax, const char* pszName);




/** ����mata������Ԫ���ݿ��в���Ԫ��
*@param[in] pstLib ��Ҫ���µ�Ԫ���ݿ�
*@param[in] pszName Ҫ���ҵ�meta������
*
*@return �ɹ�����meta��ָ�룬���򷵻�NULL
*
*@pre \e pstLib ����ΪNULL
*@pre \e pszName ����ΪNULL
*/
LPTDRMETA tdr_get_meta_by_name_i(TDRMETALIB* pstLib, const char* pszName);




/** ���ݺ����������ֵ
*@param[out] a_piID �������ֵ
*@param[out] a_piIdx �����macro���������
*@param[in] a_pstLib ��Ҫ���µ�Ԫ���ݿ�
*@param[in] a_pszValue ����
*
*@return �ɹ�����0�����򷵻ط�0
*
*@pre \e a_piID ����ΪNULL
*@pre \e a_piIdx ����ΪNULL
*@pre \e a_pstLib ����ΪNULL
*@pre \e a_pszValue ����ΪNULL
*/
int tdr_get_macro_int_i(int* a_piID, int* a_piIdx, TDRMETALIB* a_pstLib, const  char *a_pszValue);

/** ���ݺ궨��������Ԫ���ݿ��в��Һ궨����Ϣ������
*@param[inout] pstLib Ԫ���ݿ�
*@param[in] pszName �궨������
*
*@return �ɹ����غ궨������������򷵻�TDR_INVALID_INDEX
*
*@pre \e pstLib ����ΪNULL
*@pre \e pszName ����ΪNULL
*/
int tdr_get_macro_index_by_name_i(TDRMETALIB* pstLib, const char* pszName);


int tdr_parse_meta_sortkey_info_i(IN LPTDRSORTKEYINFO a_pstSortKey, LPTDRMETA a_pstMeta, const char *a_pszSortKeyPath);

/** ����Ԫ��·����ȡ��Ԫ����meta�е�ƫ��
*@param[in] a_pstMeta pstEntry���ڵ��Զ���ṹ
*@param[out] a_pstRedirector ��Ԫ�ص�ƫ��
*@param[in] a_iEntry ��ǰiEntry��Ԫ���в���
*@param[in] a_pszName Ҫ���������
*@note ���a_iEntry����TDR_INVALID_INDEX,���ʾΪmeta�����ԣ�����ȫ����Ԫ���в���
*
*@return �ɹ�����TDR_SUCCESS�����򷵻ش������
*
*@pre \e a_piOff ����ΪNULL
*@pre \e a_pstMeta ����ΪNULL
*@pre \e a_piUnit ����ΪNULL
*@pre \e a_pszName ����ΪNULL
*/
int tdr_name_to_off_i(IN LPTDRMETA a_pstMeta, OUT LPTDRSelector a_pstSelector, IN int a_iEntry, IN const char* a_pszName);

/*���ݳ�Ա��ֵ��ȡ��ֵ�󶨵ĺ궨�壬ֻ�е���Ա������macrosgroup����ʱ����Ч*/
LPTDRMACRO tdr_get_bindmacro_by_value(IN LPTDRMETALIB a_pstLib, IN LPTDRMETAENTRY a_pstEntry, IN int a_iValue);

/*����ָ�����ֻ�ȡ��ֵ�󶨵ĺ궨�壬ֻ�е���Ա������macrosgroup����ʱ����Ч*/
LPTDRMACRO tdr_get_bindmacro_by_name(IN LPTDRMETALIB a_pstLib, IN LPTDRMETAENTRY a_pstEntry, IN const char *a_pszName);

/**��������������ͳ�Ա��ֵ
*@param[in] a_pstIOStream �����Ϣ�������
*@param[in] a_pstLib Ԫ����������ָ��
*@param[in] a_pstEntry ��Ա�������ṹָ��
*@param[in,out] a_ppszHostStart �˳�Աֵ�洢�ռ����ʼ��ַ
*@param[in] a_pszHostEnd �˳�Աֵ�洢�ռ�Ľ�����ַ
*@return �ɹ�����0�����򷵻ط���ֵ
*/
int tdr_ioprintf_basedtype_i(IN LPTDRIOSTREAM a_pstIOStream, IN LPTDRMETALIB a_pstLib, IN LPTDRMETAENTRY a_pstEntry,
									INOUT char **a_ppszHostStart, IN const char *a_pszHostEnd);

/**���json��ʽ�Ļ����������ͳ�Ա��ֵ
*@param[in] a_pstIOStream �����Ϣ�������
*@param[in] a_pstEntry ��Ա�������ṹָ��
*@param[in,out] a_ppszHostStart �˳�Աֵ�洢�ռ����ʼ��ַ
*@param[in] a_pszHostEnd �˳�Աֵ�洢�ռ�Ľ�����ַ
*@return �ɹ�����0�����򷵻ط���ֵ
*/
int tdr_json_ioprintf_basedtype_i(IN LPTDRIOSTREAM a_pstIOStream, IN LPTDRMETAENTRY a_pstEntry,
                             INOUT const char **a_ppszHostStart, IN const char *a_pszHostEnd);

/**���ݴ��ַ���ֵ����������������ͳ�Ա��ֵ
*@param[in] a_pstLib Ԫ����������ָ��
*@param[in] a_pstEntry ��Ա�������ṹָ��
*@param[in] a_pszHostStart �˳�Աֵ�洢�ռ����ʼ��ַ
*@param[in,out] a_piSize �������������ڴ�ռ��С��ָ��
*	-	����	��ʹ�õ��ڴ�ռ��С
*	-	���	ʵ����ʹ�õ��ڴ�ռ��С
*@return �ɹ�����0�����򷵻ط���ֵ
*/
int tdr_ioscanf_basedtype_i(IN LPTDRMETALIB a_pstLib, IN LPTDRMETAENTRY a_pstEntry,
							IN char *a_pszHostStart, INOUT size_t *a_piSize, IN const char *a_pszValue);

void tdr_calc_checksum_i(IN LPTDRMETALIB a_pstLib);

unsigned int tdr_check_checksum_i(IN LPTDRMETALIB a_pstLib);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*TDR_METALIB_MANAGE_I_H*/
