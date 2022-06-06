/**
*
* @file     tdr_dbms_comm.h
* @brief    TSF-G-DR �����ṩ���ϲ��һ����װ��DB��ϴ�����
*
* @author jackyai
* @version 1.0
* @date 2007-12-13
*
*
* Copyright (c)  2007, ��Ѷ�Ƽ����޹�˾���������з���
* All rights reserved.
*
*/

#ifndef TDR_DBMS_COMM_H
#define TDR_DBMS_COMM_H

#include "tdr/tdr_types.h"
#include "tdr/tdr_net.h"
#include "tdr/tdr_metalib_kernel_i.h"


#ifdef __cplusplus
extern "C"
{
#endif


/**
�������ӵĵ�ǰ�ַ���������from���е��ַ�������Ϊת��SQL�ַ�������������ڡ�to���У������1���ս���NULL�ֽڡ�
����ֵ�Ǳ����ַ����ĳ��ȣ��������ս���Null�ַ���

*/
int tdr_dbms_escape_string(IN TDRDBHANDLE a_hDBHandle, OUT char *a_pszTo, IN char *a_pszFrom, int a_iLen);


/**��struct�ṹ�����ݴ����sql�ַ������Ա����͸��´�ֵ
*����Ĺ����ǣ�meta�汾�ţ�4�ֽڣ�����Ϣ���ȣ�4�ֽڣ�������hton��������Ϣ��
*@param[in,out] a_pstSql �洢ת���������
*	- ����	a_pstSql.pszBuff ָ�����汾�ؽṹ��Ϣ�Ļ�������ʼ��ַ
*	- ����  a_pstSql.iBuff	ָ���������Ĵ�С
*	- ���  a_pstSql.iBuff  ʵ�ʴ�������ݵ��ܳ���
*@param[in] a_hDBHandle ���ݿ⴦����
*@param[in] a_pstMeta ���ݵ��������
*@param[in] a_pstData �����ת�����ݵĻ�����
*	- ����	a_pstData.pszBuff �����ת�����ݵĻ�������ʼ��ַ
*	- ����  a_pstData.iBuff	ָ���������Ĵ�С
*@param[in] a_iVersion	���ð汾
*
*@retval 0   ����ɹ�
*@retval >0  ����ɹ���������ĳЩ�������
*@retval <0  ����ʧ��
*/
int tdr_dbms_meta2sql(INOUT LPTDRDATA a_pstSql, IN TDRDBHANDLE a_hDBHandle, IN LPTDRMETAENTRY a_pstMetaEntry, IN LPTDRMETA a_pstMeta, IN LPTDRDATA a_pstData, IN int a_iVersion);



/**��union�����������ݴ����sql�ַ������Ա����͸��´�ֵ
*����Ĺ����ǣ�meta�汾�ţ�4�ֽڣ�����Ϣ���ȣ�4�ֽڣ�������hton��������Ϣ��
*@param[in,out] a_pstSql �洢ת���������
*	- ����	a_pstSql.pszBuff ָ�����汾�ؽṹ��Ϣ�Ļ�������ʼ��ַ
*	- ����  a_pstSql.iBuff	ָ���������Ĵ�С
*	- ���  a_pstSql.iBuff  ʵ�ʴ�������ݵ��ܳ���
*@param[in] a_hDBHandle ���ݿ⴦����
*@param[in] a_pstMeta ���ݵ�Ԫ�����������
*@param[in] a_idxEntry ѡ����Ա����
*@param[in] a_pstData �����ת�����ݵĻ�����
*	- ����	a_pstData.pszBuff �����ת�����ݵĻ�������ʼ��ַ
*	- ����  a_pstData.iBuff	ָ���������Ĵ�С
*@param[in] a_iVersion	���ð汾
*
*@retval 0   ����ɹ�
*@retval >0  ����ɹ���������ĳЩ�������
*@retval <0  ����ʧ��
*/
int tdr_dbms_union2sql(IN TDRDBHANDLE a_hDBHandle, INOUT LPTDRDATA a_pstSql, IN LPTDRMETAENTRY a_pstMetaEntry, IN LPTDRMETA a_pstMeta, IN int  a_idxEntry, IN LPTDRDATA a_pstData, IN int a_iVersion);


/**��sql��ѯ��������struct�ṹ
*����Ĺ����ǣ�meta�汾�ţ�4�ֽڣ�����Ϣ���ȣ�4�ֽڣ�������ntoh��������Ϣ��
*@param[in] a_pstMeta ���ݵ��������
*@param[in,out] a_pstSql �洢�����������
*	- ����	a_pstSql.pszBuff ָ�������Ϣ�Ļ�������ʼ��ַ
*	- ����  a_pstSql.iBuff	ָ���������Ĵ�С
*@param[in] a_pstData ����ת�������ݵĻ�����
*	- ����	a_pstData.pszBuff ����ת�����ݵĻ�������ʼ��ַ
*	- ����  a_pstData.iBuff	ָ���������Ĵ�С
*	- ���  a_pstData.iBuff  ʵ�ʽ�������ݵ��ܳ���
*@param[in] a_iVersion	���ð汾
*
*@retval 0   ����ɹ�
*@retval >0  ����ɹ���������ĳЩ�������
*@retval <0  ����ʧ��
*/
int tdr_dbms_sql2meta(IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstData, IN LPTDRDATA a_pstSql, IN int a_iVersion);

/**��sql��ѯ��������union�ṹ
*����Ĺ����ǣ�meta�汾�ţ�4�ֽڣ�����Ϣ���ȣ�4�ֽڣ�������ntoh��������Ϣ��
*@param[in] a_pstMeta ���ݵ�Ԫ�����������
*@param[in] a_idxEntry ѡ����Ա����
*@param[in,out] a_pstSql �洢�����������
*	- ����	a_pstSql.pszBuff ָ�������Ϣ�Ļ�������ʼ��ַ
*	- ����  a_pstSql.iBuff	ָ���������Ĵ�С
*@param[in] a_pstData ����ת�������ݵĻ�����
*	- ����	a_pstData.pszBuff ����ת�����ݵĻ�������ʼ��ַ
*	- ����  a_pstData.iBuff	ָ���������Ĵ�С
*	- ���  a_pstData.iBuff  ʵ�ʽ�������ݵ��ܳ���
*@param[in] a_iVersion	���ð汾
*
*@retval 0   ����ɹ�
*@retval >0  ����ɹ���������ĳЩ�������
*@retval <0  ����ʧ��
*/
int tdr_dbms_sql2union(IN LPTDRMETA a_pstMeta, IN int  a_idxEntry,  INOUT LPTDRDATA a_pstData, IN LPTDRDATA a_pstSql, IN int a_iVersion);


#if (defined(_WIN32) || defined(_WIN64)) &&  _MSC_VER < 1400
#define TDR_LONGLONG_ENTRY_VALUE2SQL_VALUE(a_iRet, a_pstEntry, a_pszSql, a_iSize, a_iWriteLen, a_pszHostStart) \
{																												\
	a_iWriteLen = tdr_snprintf(a_pszSql, a_iSize, "%I64i", (int64_t)*(int64_t *)a_pszHostStart);	\
	TDR_CHECK_BUFF(a_pszSql, a_iSize, a_iWriteLen, a_iRet);												\
	a_pszHostStart += a_pstEntry->iHUnitSize;															\
}
#else
#define TDR_LONGLONG_ENTRY_VALUE2SQL_VALUE(a_iRet, a_pstEntry, a_pszSql, a_iSize, a_iWriteLen, a_pszHostStart) \
{																											\
	a_iWriteLen = tdr_snprintf(a_pszSql, a_iSize, "%"TDRPRId64, (int64_t)*(int64_t *)a_pszHostStart);	\
	TDR_CHECK_BUFF(a_pszSql, a_iSize, a_iWriteLen, a_iRet);												\
	a_pszHostStart += a_pstEntry->iHUnitSize;															\
}
#endif

#if (defined(_WIN32) || defined(_WIN64)) &&  _MSC_VER < 1400
#define TDR_ULONGLONG_ENTRY_VALUE2SQL_VALUE(a_iRet, a_pstEntry, a_pszSql, a_iSize, a_iWriteLen, a_pszHostStart) \
{																											\
	a_iWriteLen = tdr_snprintf(a_pszSql, a_iSize, "%I64u", (uint64_t)*(uint64_t *)a_pszHostStart);	\
	TDR_CHECK_BUFF(a_pszSql, a_iSize, a_iWriteLen, a_iRet);												\
	a_pszHostStart += a_pstEntry->iHUnitSize;															\
}
#else
#define TDR_ULONGLONG_ENTRY_VALUE2SQL_VALUE(a_iRet, a_pstEntry, a_pszSql, a_iSize, a_iWriteLen, a_pszHostStart) \
{																											\
	a_iWriteLen = tdr_snprintf(a_pszSql, a_iSize, "%"TDRPRIud64, (uint64_t)*(uint64_t *)a_pszHostStart);	\
	TDR_CHECK_BUFF(a_pszSql, a_iSize, a_iWriteLen, a_iRet);												\
	a_pszHostStart += a_pstEntry->iHUnitSize;															\
}
#endif


/**��entry��ֵת����SQL��������ֵ
*/
#define TDR_ENTRY_VALUE2SQL_VALUE(a_iRet, a_hDBHandle, a_pstLib, a_pstEntry, a_iCount, a_idxEntry, a_iVersion, a_pszSql, a_iSize, a_pszHostStart, a_pszHostEnd) \
{																													\
	int iWriteLen;																									\
	switch(a_pstEntry->iType)																						\
	{																												\
	case TDR_TYPE_UNION:																							\
		{																											\
			TDRDATA stSql,stData;																					\
			LPTDRMETA pstType = TDR_PTR_TO_META(a_pstLib, a_pstEntry->ptrMeta);										\
			stSql.iBuff = a_iSize;																					\
			stSql.pszBuff = a_pszSql;																				\
			stData.iBuff = a_pszHostEnd - a_pszHostStart;																	\
			stData.pszBuff = a_pszHostStart;																		\
			a_iRet = tdr_dbms_union2sql(a_hDBHandle, &stSql,a_pstEntry, pstType, a_idxEntry, &stData, a_iVersion);										\
			a_pszSql += stSql.iBuff;																				\
			a_iSize -= stSql.iBuff;																					\
			if (0 < a_pstEntry->iCustomHUnitSize)												\
			{																					\
				a_pszHostStart += a_pstEntry->iCustomHUnitSize;											\
			}else																				\
			{																					\
				a_pszHostStart += a_pstEntry->iHUnitSize;											\
			}																					\
		}																											\
		break;																										\
	case TDR_TYPE_STRUCT:																						\
		{																											\
			TDRDATA stSql,stData;																					\
			LPTDRMETA pstMeta;																						\
			assert(a_pstEntry->ptrMeta);																			\
			stSql.iBuff = a_iSize;																					\
			stSql.pszBuff = a_pszSql;																				\
			stData.iBuff = a_pszHostEnd - a_pszHostStart;																	\
			stData.pszBuff = a_pszHostStart;																		\
			pstMeta = TDR_PTR_TO_META(a_pstLib, a_pstEntry->ptrMeta);												\
			a_iRet = tdr_dbms_meta2sql(&stSql, a_hDBHandle, a_pstEntry, pstMeta, &stData, a_iVersion);										\
			a_pszSql += stSql.iBuff;																				\
			a_iSize -= stSql.iBuff;																					\
			if (0 < a_pstEntry->iCustomHUnitSize)												\
			{																					\
				a_pszHostStart += a_pstEntry->iCustomHUnitSize;											\
			}else																				\
			{																					\
				a_pszHostStart += a_pstEntry->iHUnitSize;											\
			}																					\
		}																											\
		break;																										\
	case TDR_TYPE_CHAR:																								\
		{																											\
		if (1 == a_iCount)																						\
			{																										\
			iWriteLen = tdr_snprintf(a_pszSql, a_iSize, "%d", (int)(int8_t)a_pszHostStart[0]);						\
			TDR_CHECK_BUFF(a_pszSql, a_iSize, iWriteLen, a_iRet);												\
			}else																									\
			{																										\
			if (a_iSize < (size_t)(2*a_iCount + 2))	/*2*a_iCount�ǿ��ǵ������ַ�������Ҫת��*/															\
				{																										\
				a_iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);										\
				break;																								\
				}																										\
				*a_pszSql++ = '\'';																						\
				a_iSize--;																								\
				iWriteLen = tdr_dbms_escape_string(a_hDBHandle, a_pszSql, a_pszHostStart, a_iCount);		\
				a_pszSql += iWriteLen;																					\
				a_iSize -= (iWriteLen + 1);																					\
				*a_pszSql++ = '\'';																						\
			}																										\
		}																											\
		break;																										\
	case TDR_TYPE_UCHAR:																							\
	case TDR_TYPE_BYTE:																								\
		{																											\
		if (1 == a_iCount)																						\
			{																										\
			iWriteLen = tdr_snprintf(a_pszSql, a_iSize, "%d", (int)(unsigned char)a_pszHostStart[0]);						\
			TDR_CHECK_BUFF(a_pszSql, a_iSize, iWriteLen, a_iRet);												\
			}else																									\
			{																										\
			if (a_iSize < (size_t)(2*a_iCount + 2))																\
				{																										\
				a_iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);										\
				break;																								\
				}																										\
				*a_pszSql++ = '\'';																						\
				a_iSize--;																								\
				iWriteLen = tdr_dbms_escape_string(a_hDBHandle, a_pszSql, a_pszHostStart, a_iCount);		\
				a_pszSql += iWriteLen;																					\
				a_iSize -= (iWriteLen + 1);																					\
				*a_pszSql++ = '\'';																						\
			}																										\
		}																											\
		break;																										\
	case TDR_TYPE_SMALLINT:																							\
		{																											\
		iWriteLen = tdr_snprintf(a_pszSql, a_iSize, "%d", (short)*(short *)a_pszHostStart);					\
		TDR_CHECK_BUFF(a_pszSql, a_iSize, iWriteLen, a_iRet);												\
		a_pszHostStart += a_pstEntry->iHUnitSize;															\
		}																											\
		break;																										\
	case TDR_TYPE_SMALLUINT:																							\
	case TDR_TYPE_WCHAR:																							\
		{																										\
		iWriteLen = tdr_snprintf(a_pszSql, a_iSize, "%d", (unsigned short)*(unsigned short *)a_pszHostStart);	\
		TDR_CHECK_BUFF(a_pszSql, a_iSize, iWriteLen, a_iRet);													\
		a_pszHostStart += a_pstEntry->iHUnitSize;															\
		}																											\
		break;																										\
	case TDR_TYPE_INT:																								\
	case TDR_TYPE_LONG:																								\
		{																											\
		iWriteLen = tdr_snprintf(a_pszSql, a_iSize, "%d", (int)*(int *)a_pszHostStart);						\
		TDR_CHECK_BUFF(a_pszSql, a_iSize, iWriteLen, a_iRet);												\
		a_pszHostStart += a_pstEntry->iHUnitSize;															\
		}																											\
		break;																										\
	case TDR_TYPE_UINT:																								\
	case TDR_TYPE_ULONG:																								\
	case TDR_TYPE_IP:																								\
		{																											\
		iWriteLen = tdr_snprintf(a_pszSql, a_iSize, "%u", (uint32_t)*(uint32_t *)a_pszHostStart);						\
		TDR_CHECK_BUFF(a_pszSql, a_iSize, iWriteLen, a_iRet);												\
		a_pszHostStart += a_pstEntry->iHUnitSize;															\
		}																											\
		break;																										\
	case TDR_TYPE_LONGLONG:																								\
		{																											\
		TDR_LONGLONG_ENTRY_VALUE2SQL_VALUE(a_iRet, a_pstEntry, a_pszSql, a_iSize, iWriteLen, a_pszHostStart);	\
		}																											\
		break;																										\
	case TDR_TYPE_ULONGLONG:																								\
		{																											\
		TDR_ULONGLONG_ENTRY_VALUE2SQL_VALUE(a_iRet, a_pstEntry, a_pszSql, a_iSize, iWriteLen, a_pszHostStart);	\
		}																											\
		break;																										\
	case TDR_TYPE_FLOAT:																								\
		{																											\
		iWriteLen = tdr_snprintf(a_pszSql, a_iSize, "%f", (float)*(float *)a_pszHostStart);		\
		TDR_CHECK_BUFF(a_pszSql, a_iSize, iWriteLen, a_iRet);												\
		a_pszHostStart += a_pstEntry->iHUnitSize;															\
		}																											\
		break;																										\
	case TDR_TYPE_DOUBLE:																								\
		{																											\
		iWriteLen = tdr_snprintf(a_pszSql, a_iSize, "%f", (double)*(double *)a_pszHostStart);		\
		TDR_CHECK_BUFF(a_pszSql, a_iSize, iWriteLen, a_iRet);												\
		a_pszHostStart += a_pstEntry->iHUnitSize;															\
		}																											\
		break;																										\
	case TDR_TYPE_DATETIME:																								\
		{																											\
		char szDateTime[32]={0};																					\
		iWriteLen = tdr_snprintf(a_pszSql, a_iSize, "'%s'", tdr_tdrdatetime_to_str_r((tdr_datetime_t *)a_pszHostStart,szDateTime,sizeof(szDateTime)));		\
		TDR_CHECK_BUFF(a_pszSql, a_iSize, iWriteLen, a_iRet);												\
		a_pszHostStart += a_pstEntry->iHUnitSize;															\
		}																											\
		break;																										\
	case TDR_TYPE_DATE:																								\
		{																											\
		char szDate[16]={0};																						\
		iWriteLen = tdr_snprintf(a_pszSql, a_iSize, "'%s'", tdr_tdrdate_to_str_r((tdr_date_t *)a_pszHostStart, szDate,sizeof(szDate)));		\
		TDR_CHECK_BUFF(a_pszSql, a_iSize, iWriteLen, a_iRet);												\
		a_pszHostStart += a_pstEntry->iHUnitSize;															\
		}																											\
		break;																										\
	case TDR_TYPE_TIME:																								\
		{																											\
		char szTime[16] ={0};																						\
		iWriteLen = tdr_snprintf(a_pszSql, a_iSize, "'%s'", tdr_tdrtime_to_str_r((tdr_time_t *)a_pszHostStart, szTime,sizeof(szTime)));		\
		TDR_CHECK_BUFF(a_pszSql, a_iSize, iWriteLen, a_iRet);												\
		a_pszHostStart += a_pstEntry->iHUnitSize;															\
		}																											\
		break;																										\
	case TDR_TYPE_STRING:																								\
		{																											\
		int iSize;																			\
		int iLen;																			\
		if (0 < a_pstEntry->iCustomHUnitSize)												\
				{																					\
				iSize = (int)a_pstEntry->iCustomHUnitSize;											\
				}else																				\
				{																					\
				iSize = (int)(a_pszHostEnd - a_pszHostStart) ;											\
				}																					\
				iLen = (int)(tdr_strnlen(a_pszHostStart, iSize));												\
				if ((int)a_iSize < 2*iLen + 2)																\
				{																										\
				a_iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);										\
				break;																								\
				}																										\
				*a_pszSql++ = '\'';																						\
				a_iSize--;																								\
				iWriteLen = tdr_dbms_escape_string(a_hDBHandle, a_pszSql, a_pszHostStart, iLen);		\
				a_pszSql += iWriteLen;																					\
				a_iSize -= (iWriteLen + 1);																					\
				*a_pszSql++ = '\'';																						\
				a_pszHostStart += iSize;															\
		}																											\
		break;																										\
	case TDR_TYPE_WSTRING:																								\
			{																											\
			int iSize;																			\
			int iLen;																			\
			if (0 < a_pstEntry->iCustomHUnitSize)												\
					{																					\
					iSize = (int)a_pstEntry->iCustomHUnitSize;											\
					}else																				\
					{																					\
					iSize = (int)(a_pszHostEnd - a_pszHostStart ) ;											\
					}																					\
					iLen = (int)tdr_wcsnlen((wchar_t*)pszHostStart, iSize);										\
					iLen *= sizeof(tdr_wchar_t);														\
					if ((int)a_iSize < iLen + 2)																\
					{																										\
					a_iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);										\
					break;																								\
					}																										\
					*a_pszSql++ = '\'';																						\
					a_iSize--;																								\
					iWriteLen = tdr_dbms_escape_string(a_hDBHandle, a_pszSql, a_pszHostStart, iLen);		\
					a_pszSql += iWriteLen;																					\
					a_iSize -= (iWriteLen + 1);																					\
					*a_pszSql++ = '\'';																						\
					a_pszHostStart += iSize;															\
			}																											\
			break;																										\
	default:																											\
	break;																											\
	}																													\
}

#define TDR_SQL_VALUE2_ENTRY_VALUE(a_iRet, a_pstLib, a_pstEntry, a_iCount, a_idxEntry, a_iVersion, a_pszHostStart, a_pszHostEnd, a_pszFieldVal, a_dwFieldLen)\
do{																													\
	size_t iLen;																										\
	long lVal;																										\
	int64_t llVal;																								\
    if (a_pszHostEnd < a_pszHostStart)                                                                              \
    {                                                                                                               \
        a_iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);                                              \
        break;                                                                                                      \
    }                                                                                                               \
	switch(a_pstEntry->iType)																						\
	{																												\
	case TDR_TYPE_UNION:																							\
		{																											\
			TDRDATA stSql,stData;																					\
			LPTDRMETA pstType = TDR_PTR_TO_META(a_pstLib, a_pstEntry->ptrMeta);										\
			stSql.iBuff = a_dwFieldLen;																					\
			stSql.pszBuff = a_pszFieldVal;																				\
			stData.iBuff = a_pszHostEnd - a_pszHostStart;																	\
			stData.pszBuff = a_pszHostStart;																		\
			a_iRet = tdr_dbms_sql2union(pstType, a_idxEntry, &stData, &stSql, a_iVersion);										\
			if (0 < a_pstEntry->iCustomHUnitSize)												\
			{																					\
				a_pszHostStart += a_pstEntry->iCustomHUnitSize;											\
			}else																				\
			{																					\
				a_pszHostStart += a_pstEntry->iHUnitSize;											\
			}																					\
		}																											\
		break;																										\
	case TDR_TYPE_STRUCT:																						\
	{																											\
		TDRDATA stSql,stData;																					\
		LPTDRMETA pstMeta;																						\
		assert(a_pstEntry->ptrMeta);																			\
		stSql.iBuff = a_dwFieldLen;																					\
		stSql.pszBuff = a_pszFieldVal;																				\
		stData.iBuff = a_pszHostEnd - a_pszHostStart;																	\
		stData.pszBuff = a_pszHostStart;																		\
		pstMeta = TDR_PTR_TO_META(a_pstLib, a_pstEntry->ptrMeta);												\
		a_iRet = tdr_dbms_sql2meta(pstMeta, &stData, &stSql, a_iVersion);										\
		if (0 < a_pstEntry->iCustomHUnitSize)												\
		{																					\
			a_pszHostStart += a_pstEntry->iCustomHUnitSize;											\
		}else																				\
		{																					\
			a_pszHostStart += a_pstEntry->iHUnitSize;											\
		}																					\
	}																											\
	break;																										\
	case TDR_TYPE_CHAR:																								\
	{																											\
	if (1 == a_iCount)																						\
		{																										\
		    lVal = strtol(a_pszFieldVal, NULL, 0);																	\
		    *(int8_t *)a_pszHostStart = (int8_t)lVal;																	\
		    a_pszHostStart += a_pstEntry->iHUnitSize;															\
		}else																									\
		{																										\
		    iLen = (size_t)a_dwFieldLen;																				\
		    if ((int)iLen > a_iCount)																		\
			{																										\
			    iLen = a_iCount;																		\
			    a_iRet = TDR_SUCCEESS_WARNING(TDR_SUCWARN_TRUNCATE_DATE);											\
			}																										\
			if (a_pszHostStart + iLen > a_pszHostEnd )																\
			{																										\
			a_iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);										\
			break;																								\
			}																										\
			TDR_MEMCPY(a_pszHostStart, a_pszFieldVal, iLen, TDR_MIN_COPY);											\
			a_pszHostStart += (a_pstEntry->iHRealSize - iLen);														\
		}																										\
	}																											\
	break;																										\
	case TDR_TYPE_UCHAR:																							\
	case TDR_TYPE_BYTE:																								\
	{																											\
	if (1 == a_iCount)																						\
		{																										\
		    lVal = strtol(a_pszFieldVal, NULL, 0);																	\
		    *(uint8_t *)a_pszHostStart = (uint8_t)lVal;																	\
		    a_pszHostStart += a_pstEntry->iHUnitSize;															\
		}else																									\
		{																										\
		    iLen = (size_t)a_dwFieldLen;																				\
		    if ((int)iLen > a_iCount)																		\
			{																										\
			iLen = a_iCount;																		\
			a_iRet = TDR_SUCCEESS_WARNING(TDR_SUCWARN_TRUNCATE_DATE);											\
			}																										\
			if (a_pszHostStart + iLen > a_pszHostEnd )																\
			{																										\
			a_iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);										\
			break;																								\
			}																										\
			TDR_MEMCPY(a_pszHostStart, a_pszFieldVal, iLen, TDR_MIN_COPY);											\
			a_pszHostStart += (a_pstEntry->iHRealSize - iLen);														\
		}																										\
	}																											\
	break;																										\
	case TDR_TYPE_SMALLINT:																							\
	{																											\
	lVal = strtol(a_pszFieldVal, NULL, 0);																	\
	*(int16_t *)a_pszHostStart = (int16_t)lVal;																	\
	a_pszHostStart += a_pstEntry->iHUnitSize;															\
	}																											\
	break;																										\
	case TDR_TYPE_SMALLUINT:																							\
	case TDR_TYPE_WCHAR:																							\
	{																										\
	lVal = strtol(a_pszFieldVal, NULL, 0);																	\
	*(uint16_t *)a_pszHostStart = (uint16_t)lVal;																	\
	a_pszHostStart += a_pstEntry->iHUnitSize;															\
	}																											\
	break;																										\
	case TDR_TYPE_INT:																								\
	case TDR_TYPE_LONG:																								\
	{																											\
	lVal = strtol(a_pszFieldVal, NULL, 0);																	\
	*(int32_t *)a_pszHostStart = (int32_t)lVal;																	\
	a_pszHostStart += a_pstEntry->iHUnitSize;															\
	}																											\
	break;																										\
	case TDR_TYPE_UINT:																								\
	case TDR_TYPE_ULONG:																								\
	case TDR_TYPE_IP:																								\
	{																											\
	llVal = TDR_ATOLL(a_pszFieldVal);																	\
	*(uint32_t *)a_pszHostStart = (uint32_t)llVal;																	\
	a_pszHostStart += a_pstEntry->iHUnitSize;															\
	}																											\
	break;																										\
	case TDR_TYPE_LONGLONG:																								\
	{																											\
	llVal = TDR_ATOLL(a_pszFieldVal);																	\
	*(int64_t *)a_pszHostStart = llVal;																	\
	a_pszHostStart += a_pstEntry->iHUnitSize;															\
	}																											\
	break;																										\
	case TDR_TYPE_ULONGLONG:																								\
	{																											\
	llVal = TDR_ATOULL(a_pszFieldVal);																	\
	*(uint64_t *)a_pszHostStart = (uint64_t)llVal;																	\
	a_pszHostStart += a_pstEntry->iHUnitSize;															\
	}																											\
	break;																										\
	case TDR_TYPE_FLOAT:																								\
	{																											\
	float  fVal;																							\
	fVal = (float)atof(a_pszFieldVal);																			\
	*(float *)a_pszHostStart = fVal;																			\
	a_pszHostStart += a_pstEntry->iHUnitSize;																\
	}																											\
	break;																										\
	case TDR_TYPE_DOUBLE:																								\
	{																											\
	double  dVal;																							\
	dVal = strtod(a_pszFieldVal, NULL);																			\
	*(double *)a_pszHostStart = dVal;																			\
	a_pszHostStart += a_pstEntry->iHUnitSize;																\
	}																											\
	break;																										\
	case TDR_TYPE_DATETIME:																								\
	{																											\
	a_iRet = tdr_str_to_tdrdatetime((tdr_datetime_t *)a_pszHostStart, &a_pszFieldVal[0]);						\
	a_pszHostStart += a_pstEntry->iHUnitSize;															\
	}																											\
	break;																										\
	case TDR_TYPE_DATE:																								\
	{																											\
	a_iRet = tdr_str_to_tdrdate((tdr_date_t *)a_pszHostStart, &a_pszFieldVal[0]);						\
	a_pszHostStart += a_pstEntry->iHUnitSize;															\
	}																											\
	break;																										\
	case TDR_TYPE_TIME:																								\
	{																											\
	a_iRet = tdr_str_to_tdrtime((tdr_time_t *)a_pszHostStart, &a_pszFieldVal[0]);						\
	a_pszHostStart += a_pstEntry->iHUnitSize;															\
	}																											\
	break;																										\
	case TDR_TYPE_STRING:																								\
	{																											\
	size_t iSize;																			\
	if (0 < a_pstEntry->iCustomHUnitSize)												\
		{																					\
		iSize = a_pstEntry->iCustomHUnitSize;											\
		}else																				\
		{																					\
		iSize = a_pszHostStart - a_pszHostEnd;											\
		}																					\
		iLen = (int)a_dwFieldLen + 1;														\
		if (iLen > iSize) /* the string has no space for null. */						\
		{																				\
		a_iRet = TDR_SUCCEESS_WARNING(TDR_SUCWARN_TRUNCATE_DATE);					\
		iLen = iSize;															\
		}																				\
		if( a_pszHostStart + iLen > a_pszHostEnd )											\
		{																				\
		a_iRet =	TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);			\
		}																				\
		TDR_STRNCPY(a_pszHostStart, a_pszFieldVal, iLen);								\
		a_pszHostStart += iSize;															\
	}																											\
	break;																										\
	case TDR_TYPE_WSTRING:																								\
	{																											\
	    size_t iSize;																			\
	    if (0 < a_pstEntry->iCustomHUnitSize)												\
		{																					\
		    iSize = a_pstEntry->iCustomHUnitSize;											\
		}else																				\
		{																					\
		    iSize = a_pszHostStart - a_pszHostEnd;											\
		}																					\
		iLen = (int)a_dwFieldLen;														\
		if (iLen > iSize - 2) /* the string has no space for null. */							\
		{																					\
		a_iRet = TDR_SUCCEESS_WARNING(TDR_SUCWARN_TRUNCATE_DATE);						\
		iLen = iSize - 2;																	\
		}																					\
		if( a_pszHostStart + iLen > a_pszHostEnd )											\
		{																				\
		a_iRet =	TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);			\
		}																				\
		TDR_MEMCPY(a_pszHostStart, a_pszFieldVal, iLen, TDR_MIN_COPY);						\
		*a_pszHostStart++ = '\0';															\
		*a_pszHostStart++ = '\0';															\
		a_pszHostStart += (iSize - iLen -2);												\
	}																											\
	break;																										\
	default:																											\
	break;																											\
	}																													\
}while (0)




/**�������ݿ����
*@param[out] a_pszName �������ɺõ����ݿ������ֵĻ�����
*@param[in] a_iNameSize �����������ռ�
*@param[in] a_pstMeta ������ݵ�Ԫ�����������
*@param[in] a_pszMetaHost �������ݵĻ������׵�ַ
*@return �ɹ�����0�����򷵻ظ���
*/
int tdr_get_dbtable_name(OUT char *a_pszName, IN int a_iNameSize, IN LPTDRMETA a_pstMeta, IN char *a_pszMetaHost);

/**����SQL���������б�
*@param[in,out] a_pstSql ��¼SQL��Ϣ
*@param[in] a_pstObj ���ݶ�����Ϣ
*@param[in] bCalcRefer ���������б�ʱ�Ƿ�ʹ��refer����,��ֵΪtrue,��ʹ��,����ʹ��
*@return ����ɹ�����0,���򷵻ظ���
*/
int tdr_gen_columm_name_lists_i(INOUT LPTDRDATA a_pstSql, INOUT  LPTDRDBOBJECT a_pstObj, IN TDRBOOLEAN bCalcRefer);

/**����SQL���ĳ�Ա��ֵ�б�
*@param[in,out] a_pstSql ��¼SQL��Ϣ
*@param[in] a_hDBHandle ���ݿ⴦����
*@param[in] a_pstObj ���ݶ�����Ϣ
*@return ����ɹ�����0,���򷵻ظ���
*/
int tdr_gen_columm_value_lists_i(INOUT LPTDRDATA a_pstSql, IN TDRDBHANDLE a_hDBHandle, INOUT  LPTDRDBOBJECT a_pstObj);


/**����insert���
*/
int tdr_obj2sql_insert_i(IN TDRDBHANDLE a_hDBHandle, INOUT LPTDRDATA a_pstSql, INOUT  LPTDRDBOBJECT a_pstObj);

/**����update���
*/
int tdr_obj2sql_update_i(IN TDRDBHANDLE a_hDBHandle, INOUT LPTDRDATA a_pstSql, INOUT  LPTDRDBOBJECT a_pstObj, IN const char *a_pszWhereDefinition);

/**
*/
int tdr_gen_update_columm_lists_i(IN TDRDBHANDLE a_hDBHandle, INOUT LPTDRDATA a_pstSql, INOUT  LPTDRDBOBJECT a_pstObj);

/**����select���
*/
int tdr_obj2sql_select_i(IN TDRDBHANDLE a_hDBHandle, INOUT LPTDRDATA a_pstSql, INOUT  LPTDRDBOBJECT a_pstObj, IN const char *a_pszWhereDefinition);

int tdr_obj2sql_select_table_i(IN TDRDBHANDLE a_hDBHandle, INOUT LPTDRDATA a_pstSql, 
		INOUT  LPTDRDBOBJECT a_pstObj, IN const char *a_pszWhereDefinition, IN char* a_pszTable);

/*����WHERE����*/
int tdr_gen_where_definition_i(INOUT LPTDRDATA a_pstSql, IN TDRDBHANDLE a_hDBHandle, INOUT  LPTDRDBOBJECT a_pstObj, IN const char *a_pszWhereDefinition);

/**����delete���
*/
int tdr_obj2sql_delete_i(IN TDRDBHANDLE a_hDBHandle, INOUT LPTDRDATA a_pstSql, INOUT  LPTDRDBOBJECT a_pstObj, IN const char *a_pszWhereDefinition);

/**����replace���
*/
int tdr_obj2sql_replace_i(IN TDRDBHANDLE a_hDBHandle, INOUT LPTDRDATA a_pstSql, INOUT  LPTDRDBOBJECT a_pstObj);


int tdr_gen_columm_name_lists_i(INOUT LPTDRDATA a_pstSql, IN  LPTDRDBOBJECT a_pstObj, IN TDRBOOLEAN bCalcRefer);

int tdr_gen_columm_value_lists_i(INOUT LPTDRDATA a_pstSql, IN TDRDBHANDLE a_hDBHandle, INOUT  LPTDRDBOBJECT a_pstObj);


int tdr_dbms_fetch_row(IN TDRDBRESULTHANDLE a_hDBResult);

/** �ӵ�ǰ�������л�ȡָ�����ֵ�������
*@param[out] a_ppszFieldVal �����������ַָ���ָ��
*@param[out] a_pulLen �����򳤶ȵ�ָ��
*@param[in] a_hDBResult ����DB����������ľ��
*@param[in] a_szName �����������
*
* @pre \e a_ppszFieldVal ����ΪNULL
* @pre \e a_pulLen ����ΪNULL
* @pre \e	a_szName ����ΪNULL
*
*@retval 0   ����ɹ�
*@retval >0  ����ɹ���������ĳЩ�������
*@retval <0  ����ʧ��
*/
int tdr_dbms_fetch_field(OUT char **a_ppszFieldVal, OUT unsigned long *a_pulLen, IN TDRDBRESULTHANDLE a_hDBResult, IN const char *a_szName);

int tdr_fetch_all_fields(IN  TDRDBRESULTHANDLE a_hDBResult, INOUT  LPTDRDBOBJECT a_pstObj);

/** ���ݽṹ���ڶ�ջ�е���Ϣ�����ɳ�Ա�б�����ǰ׺ ��֧�ֽṹ���ӳ�Ա����չ���洢
*/
char *tdr_gen_entry_columnname_prefix(TDRSTACKITEM	a_pstStack[], int a_iStackCount);


#ifdef __cplusplus
}
#endif


#endif

