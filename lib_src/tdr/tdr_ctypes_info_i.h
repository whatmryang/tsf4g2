/**
*
* @file     Tdr_ctypes_info.h
* @brief    DR֧�ֵ�C/C++��������������Ϣ
*
* @author steve jackyai
* @version 1.0
* @date 2007-03-26
*
* ����DR֧�ֵ�c/c++�������������͵���ϸ��Ϣ�����ṩ���ʷ���
*
* Copyright (c)  2007, ��Ѷ�Ƽ����޹�˾���������з���
* All rights reserved.
*
*/

#ifndef TDR_CTYPES_INFO_H
#define TDR_CTYPES_INFO_H

#include <stdio.h>
#include "tdr/tdr_types.h"

#define TDR_MAX_PREFIX_LEN 32

/**
* This is the type info for c/c++ base date type info.
*/
typedef struct tagTDRCTypeInfo	TDRCTYPEINFO;

/**
*brief struct to store c/c++ base type info
*/
struct tagTDRCTypeInfo
{
    char* pszName;		///< type name
    char* pszCName;		///< name used in c/c++ head file. */
    char* pszSQLName;   /* name used in sql */
    char pszSPrefix[TDR_MAX_PREFIX_LEN];	/* single prefix  in c/c++ head file. */
    char pszMPrefix[TDR_MAX_PREFIX_LEN];	/* multiple prefix(for array). */

    int iType;
    int iSize;
    unsigned int uiFlags;
};


#pragma pack(1)

/**tdr��ȡdate�����������͵Ľṹ
*/
struct tagTDRDate
{
	short nYear;
	unsigned char bMonth;
	unsigned char bDay;
};
typedef struct tagTDRDate TDRDATE;
typedef struct tagTDRDate *LPTDRDATE;

/**tdr��ȡtime�����������͵Ľṹ
*/
struct tagTDRTime
{
	short nHour;  /**<Сʱ*/
	unsigned char bMin; /**<����*/
	unsigned char bSec; /**<��*/
};
typedef struct tagTDRTime TDRTIME;
typedef struct tagTDRTime *LPTDRTIME;

/**tdr��ȡdatetime�����������͵Ľṹ
*/
struct tagTDRDateTime
{
	TDRDATE stDate;  /**<����*/
	TDRTIME stTime;   /**<ʱ��*/
};
typedef struct tagTDRDateTime TDRDATETIME;
typedef struct tagTDRDateTime *LPTDRDATETIME;

#pragma pack()

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
*@brief ������������ȡ��������Ϣ������ϸ��Ϣ�ṹ�е�����
*
*@param name ������
*@param iXMLTagVer Ԫ�����ݵ�XML��ǩ����汾
*@retval >=0 ��Ч�Ĵ洢����
*@retval TDR_INVALID_INDEX ��Ч����ֵ����û�д����Ͷ���
*/
int tdr_typename_to_idx(IN const char *name);

/**����������ȡ������Ϣ��ָ��
*/
LPTDRCTYPEINFO tdr_idx_to_typeinfo(IN int iIdx);


/**
 * @brief �������Ͷ�Ӧ�ĵ�������ǰ׺�ַ���
 *
 * @param a_pszName ������
 * @param a_pszPrefix ǰ׺�ַ���
 * @param a_fpError ��������ļ�ָ��
 * @retval 0 ���óɹ�
 * @retval others ����
 */

int tdr_set_sprefix_by_name(IN const char *a_pszName,
                           IN const char *a_pszPrefix,
                           IN FILE* a_fpError);

/**
 * @brief �������Ͷ�Ӧ���������ǰ׺�ַ���
 *
 * @param a_pszName ������
 * @param a_pszPrefix ǰ׺�ַ���
 * @param a_fpError ��������ļ�ָ��
 * @retval 0 ���óɹ�
 * @retval others ����
 */

int tdr_set_mprefix_by_name(IN const char *a_pszName,
                           IN const char *a_pszPrefix,
                           IN FILE* a_fpError);

/**
 * @brief ȡ�ñ���ǰ׺�ַ�����Ĵ�С
 *
 * @retval unsigned ǰ׺��Ĵ�С
 */
unsigned int tdr_get_prefix_table_size(void);

/**
 * @brief ȡ������ı���ǰ׺�ַ�����󳤶�
 *
 * @retval unsigned ��󳤶�
 */
unsigned int tdr_get_prefix_max_len(void);

/**
 * @brief ȡ�����Ͷ�Ӧ��xml������
 *
 * @param a_idx ���ͱ������
 * @param a_fpError ��������ļ�ָ��
 * @retval NULL ��ȡʧ��
 * @retval others ����ǰ׺�ַ���
 */
const char* tdr_get_typename_by_index(IN unsigned int a_iIndex, IN FILE* a_fpError);

/**
 * @brief ȡ�����Ͷ�Ӧ�ĵ�������ǰ׺�ַ���
 *
 * @param a_idx ���ͱ������
 * @param a_fpError ��������ļ�ָ��
 * @retval NULL ��ȡʧ��
 * @retval others ����ǰ׺�ַ���
 */
const char* tdr_get_sprefix_by_index(IN unsigned int a_iIndex, IN FILE* a_fpError);

/**
 * @brief ȡ�����Ͷ�Ӧ���������ǰ׺�ַ���
 *
 * @param a_idx ���ͱ������
 * @param a_fpError ��������ļ�ָ��
 * @retval NULL ��ȡʧ��
 * @retval others ����ǰ׺�ַ���
 */
const char* tdr_get_mprefix_by_index(IN unsigned int a_iIndex, IN FILE* a_fpError);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TDR_CTYPES_INFO_H */
