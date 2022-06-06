/**
*
* @file     tdr_data_sort.c
* @brief    TDR��������
*
* @author steve jackyai
* @version 1.0
* @date 2007-05-18
*
*
* Copyright (c)  2007, ��Ѷ�Ƽ����޹�˾���������з���
* All rights reserved.
*
*/

#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "tdr_auxtools.h"
#include "tdr/tdr_data_sort.h"
#include "tdr/tdr_error.h"
#include "tdr_define_i.h"
#include "tdr_metalib_kernel_i.h"
#include "tdr_ctypes_info_i.h"
#include "tdr_metalib_manage_i.h"
#include "tdr/tdr_ctypes_info.h"

#if defined (_WIN32) || defined (_WIN64)
#pragma warning(disable:4996)
#endif

struct tagTDRSortMataInfo
{
	int iVersion;		/**<�������ݼ��ð汾*/
	int idxSelectEntry;		/**<���metaΪunion���ͣ���˳�Ա����ѡ�еĳ�Ա*/
	LPTDRMETA pstMeta;	/**<meta Ԫ����ָ��*/
	int chOrder;		/**<���򷽷�*/
};
typedef struct tagTDRSortMataInfo TDRSORTMETAINFO;
typedef struct tagTDRSortMataInfo *LPTDRSORTMETAINFO;

struct tagTDRSortEntryInfo
{
	int iCount1;			/**<��һ��entry���ݵ�countֵ*/
	int iCount2;			/**<�ڶ���entry���ݵ�countֵ*/
	int idxSelectEntry;		/**<entry��selectֵ����˳�Ա����ѡ�еĳ�Ա*/
	int iVersion;
	LPTDRMETAENTRY pstEntry;	/**<meta Ԫ����ָ��*/
};
typedef struct tagTDRSortEntryInfo TDRSORTENTRYINFO;
typedef struct tagTDRSortEntryInfo *LPTDRSORTENTRYINFO;

typedef int(* LPCOMPAREFUNC)(const void *, const void *);


/**
* �Ը����������������������
*/
static int tdr_sort_composize_entry_array_i(IN LPTDRSORTMETAINFO a_pstMetaInfo, IN int a_iCount, IN char *a_pszHostBase,
											IN char *a_pszHostEnd);

/**
* �Լ��������������������
*/
static int tdr_sort_simple_entry_array_i(IN LPTDRMETAENTRY a_pstEntry, IN int a_iSortType, IN int a_iCount, IN char *a_pszHostBase,
											IN char *a_pszHostEnd);

static LPCOMPAREFUNC tdr_get_entry_compare_function_i(IN int a_iEntryType,  IN int a_chOrder);

static int tdr_string_asc_comparefunc_i(const void *pCompare1, const void *pCompare2);
static int tdr_string_desc_comparefunc_i(const void *pCompare1, const void *pCompare2);
static int tdr_wstring_asc_comparefunc_i(const void *pCompare1, const void *pCompare2);
static int tdr_wstring_desc_comparefunc_i(const void *pCompare1, const void *pCompare2);
static int tdr_char_asc_comparefunc_i(const void *pCompare1, const void *pCompare2);
static int tdr_char_desc_comparefunc_i(const void *pCompare1, const void *pCompare2);
static int tdr_uchar_asc_comparefunc_i(const void *pCompare1, const void *pCompare2);
static int tdr_uchar_desc_comparefunc_i(const void *pCompare1, const void *pCompare2);
static int tdr_short_desc_comparefunc_i(const void *pCompare1, const void *pCompare2);
static int tdr_short_asc_comparefunc_i(const void *pCompare1, const void *pCompare2);
static int tdr_ushort_desc_comparefunc_i(const void *pCompare1, const void *pCompare2);
static int tdr_ushort_asc_comparefunc_i(const void *pCompare1, const void *pCompare2);
static int tdr_int_asc_comparefunc_i(const void *pCompare1, const void *pCompare2);
static int tdr_int_desc_comparefunc_i(const void *pCompare1, const void *pCompare2);
static int tdr_uint_asc_comparefunc_i(const void *pCompare1, const void *pCompare2);
static int tdr_uint_desc_comparefunc_i(const void *pCompare1, const void *pCompare2);
static int tdr_long_asc_comparefunc_i(const void *pCompare1, const void *pCompare2);
static int tdr_long_desc_comparefunc_i(const void *pCompare1, const void *pCompare2);
static int tdr_ulong_asc_comparefunc_i(const void *pCompare1, const void *pCompare2);
static int tdr_ulong_desc_comparefunc_i(const void *pCompare1, const void *pCompare2);
static int tdr_longlong_asc_comparefunc_i(const void *pCompare1, const void *pCompare2);
static int tdr_longlong_desc_comparefunc_i(const void *pCompare1, const void *pCompare2);
static int tdr_ulonglong_asc_comparefunc_i(const void *pCompare1, const void *pCompare2);
static int tdr_ulonglong_desc_comparefunc_i(const void *pCompare1, const void *pCompare2);
static int tdr_float_asc_comparefunc_i(const void *pCompare1, const void *pCompare2);
static int tdr_float_desc_comparefunc_i(const void *pCompare1, const void *pCompare2);
static int tdr_double_asc_comparefunc_i(const void *pCompare1, const void *pCompare2);
static int tdr_double_desc_comparefunc_i(const void *pCompare1, const void *pCompare2);

static int tdr_date_desc_comparefunc_i(const void *pCompare1, const void *pCompare2);

static int tdr_datetime_desc_comparefunc_i(const void *pCompare1, const void *pCompare2);

static int tdr_time_desc_comparefunc_i(const void *pCompare1, const void *pCompare2);
static int tdr_ip_asc_comparefunc_i(const void *pCompare1, const void *pCompare2);
static int tdr_ip_desc_comparefunc_i(const void *pCompare1, const void *pCompare2);
static int tdr_wchar_asc_comparefunc_i(const void *pCompare1, const void *pCompare2);
static int tdr_wchar_desc_comparefunc_i(const void *pCompare1, const void *pCompare2);

/**����ԭ���������Ƚ��������ݽṹ�Ĵ�С
*@note �����Ԫ����ָ��������ؼ���sortkey���ԣ�������ؼ��ֽ������򣻷������Ԫ���ݸ���Ա����˳��
*���αȽϸ���Ա���Ӷ��õ���Ԫ���ݽṹ�Ĵ�С�����ĳ��ԱΪ���飬����˳�Աû��ָ�����򷽷�������Ϊ�˳�Ա��ȣ�
*�������αȽϸ������Ա�������Ա���鳤�Ȳ��ȣ���������ǰ��Ԫ�ض��ȵ�����£����鳤�ȴ�ĳ�Ա��Ϊ��
*@retval >0 data1��data2��
*@retval 0 �����������
*@retval <0 data1��data2С
*/
static int tdr_compare_struct_metadata_i(IN LPTDRSORTMETAINFO a_pstMetaInfo, IN char *a_pszHostData1, char *a_pszHostData2);

/**����ԭ���������Ƚ��������ݽṹ�Ĵ�С
*@note �����Ԫ����ָ��������ؼ���sortkey���ԣ�������ؼ��ֽ������򣻷������Ԫ���ݸ���Ա����˳��
*���αȽϸ���Ա���Ӷ��õ���Ԫ���ݽṹ�Ĵ�С�����ĳ��ԱΪ���飬����˳�Աû��ָ�����򷽷�������Ϊ�˳�Ա��ȣ�
*�������αȽϸ������Ա�������Ա���鳤�Ȳ��ȣ���������ǰ��Ԫ�ض��ȵ�����£����鳤�ȴ�ĳ�Ա��Ϊ��
*@retval >0 data1��data2��
*@retval 0 �����������
*@retval <0 data1��data2С
*/
static int tdr_compare_union_metadata_i(IN LPTDRSORTMETAINFO a_pstMetaInfo, IN char *a_pszHostData1, char *a_pszHostData2);

typedef int (*LPTDRCMPMETADATAFUNC)(IN LPTDRSORTMETAINFO a_pstMetaInfo, IN char *a_pszHostData1, char *a_pszHostData2);

/**����ԭ���������Ƚ�����Ԫ�صĴ�С
*@note �����Ԫ����ָ��������ؼ���sortkey���ԣ�������ؼ��ֽ������򣻷������Ԫ���ݸ���Ա����˳��
*���αȽϸ���Ա���Ӷ��õ���Ԫ���ݽṹ�Ĵ�С�����ĳ��ԱΪ���飬����˳�Աû��ָ�����򷽷�������Ϊ�˳�Ա��ȣ�
*�������αȽϸ������Ա�������Ա���鳤�Ȳ��ȣ���������ǰ��Ԫ�ض��ȵ�����£����鳤�ȴ�ĳ�Ա��Ϊ��
*@retval >0 data1��data2��
*@retval 0 �����������
*@retval <0 data1��data2С
*/
static int tdr_compare_entry_data_i(LPTDRMETALIB a_pstLib, LPTDRSORTENTRYINFO a_pstSortEntryInfo, char *a_pszHostEntry1,
									char *a_pszHostEntry2);

/**ʹ�ÿ��������㷨����meta���ݽ�������
*���ǵ�meta���ݿ�Ĵ洢�ռ䲻ȷ��������ʱ��ȡ�������򣬲��ƶ�ʵ�����ݿ飬ֻ��������ֵ
*@param[in,out] pidxMetas ��������Ĳ���
*	- ��� ������meta���ݵ�����ֵ
*   - ���� ����ǰ��meta���ݵ�����ֵ
*@param[in] a_iBegin ������ʼ����
*@param[in] a_iEnd ������ֹ����
*@param[in] a_pstMetaInfo ��Ҫ�����meta���ݵ���Ϣ
* @param a_pszHostStart���������ݵ���ʵ��ַ
*/
static int tdr_qsort_composize_entry_array_i(INOUT int *pidxMetas, IN int a_iBegin, IN int a_iEnd,
                                             LPTDRSORTMETAINFO a_pstMetaInfo, char *a_pszHostStart, LPTDRCMPMETADATAFUNC a_pfnCmp);

/** ����Ԫ����������װ��������Ԫ��������
*/
static int tdr_swap_meta_data_i(IN LPTDRMETA a_pstMeta, IN int *a_pidxMetas, IN int a_iCount, char *a_pszHostStart);

/**�������Ԫ�صļ�ֵԼ��
*@note ���ô˷���ǰ,�����ȶ������������,���������ָ�������򷽷�,��Ĭ���Ѿ�����������
*/
static int tdr_check_simple_entry_array_unique_i(LPTDRMETAENTRY a_pstEntry, int a_iCount, char *a_pszHostStart);

static int tdr_check_composize_entry_array_unique_i(LPTDRSORTMETAINFO a_pstMetaInfo, int a_iCount,char *a_pszHostStart, char *a_pszHostEnd);

static int tdr_check_meta_entry_unique_i(LPTDRMETA a_pstMeta, int a_iCount, LPTDRMETAENTRY a_pstEntry, char *a_pszHostStart);


#define TDR_GET_UNION_ENTRY_INDEX(a_idxSubEntry, a_pstEntry, a_pszHostBase) \
{																			\
	if (TDR_INVALID_OFFSET != a_pstEntry->stSelector.iHOff)					\
	{																		\
		tdr_longlong lVal;													\
		char *pszPtr = a_pszHostBase + a_pstEntry->stSelector.iHOff;	\
		TDR_GET_INT(lVal, a_pstEntry->stSelector.iUnitSize, pszPtr);	\
		a_idxSubEntry = (int)lVal;										\
	}else																	\
	{																		\
		a_idxSubEntry = TDR_INVALID_INDEX;									\
	}																		\
}


TDR_API int tdr_sort(IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstHost, IN int a_iVersion)
{
	int iRet = TDR_SUCCESS;
	LPTDRMETALIB pstLib;
	LPTDRMETA pstCurMeta;
	TDRSTACK  stStack;
	LPTDRSTACKITEM pstStackTop;
	int iStackItemCount;
	int iChange;
	int iCutOffVersion;
	char *pszHostStart;
	char *pszHostEnd;

	/*assert(NULL != a_pstMeta);
	assert(NULL != a_pstHost);
	assert(NULL != a_pstHost->pszBuff);
	assert(0 < a_pstHost->iBuff);*/
	if ((NULL == a_pstMeta) || (NULL == a_pstHost))
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
	}
	if ((NULL == a_pstHost->pszBuff)||(0 >= a_pstHost->iBuff))
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
	}

	if (0 == a_iVersion)
	{
		a_iVersion = TDR_MAX_VERSION;
	}
	if (a_pstMeta->iBaseVersion > a_iVersion)
	{
		a_pstHost->iBuff = 0;
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
	}
	if (TDR_TYPE_UNION == a_pstMeta->iType)
	{
		return TDR_SUCCESS;
	}

	pszHostStart = a_pstHost->pszBuff;
	pszHostEnd = a_pstHost->pszBuff + a_pstHost->iBuff;
	pstLib = TDR_META_TO_LIB(a_pstMeta);

	pstCurMeta = a_pstMeta;
	pstStackTop = &stStack[0];
	pstStackTop->pszHostBase = pszHostStart;
	pstStackTop->pszNetBase = pszHostStart;  /*meta���׵�ַ*/
	pstStackTop->pstMeta = pstCurMeta;
	pstStackTop->iCount = 1;
	pstStackTop->idxEntry = 0;
	pstStackTop->iMetaSizeInfoUnit = 1;			/*count ����ֵ*/
	pstStackTop->pszMetaSizeInfoTarget = NULL;  /*��meta���ϲ�ṹ�ж��ڵ�entryָ��*/
	pstStackTop->iChange = 0;
	iCutOffVersion = a_iVersion;
	if (TDR_ERR_IS_ERROR(iRet))
	{
		a_pstHost->iBuff = 0;
		return iRet;
	}
	pstStackTop->iCutOffVersion = iCutOffVersion;
	iStackItemCount = 1;

	iChange = 0;
	while (0 < iStackItemCount)
	{
		LPTDRMETAENTRY pstEntry;
		int iArrayRealCount ;

		if (0 >= pstStackTop->iCount)
		{
			/*��ǰmeta����Ա�Ѿ�������ϣ����meta���鱾���������*/
			pstEntry = (LPTDRMETAENTRY)pstStackTop->pszMetaSizeInfoTarget;
			if ((1 < pstStackTop->iMetaSizeInfoUnit) && (TDR_SORTMETHOD_NONE_SORT != pstEntry->chOrder))
			{
				TDRSORTMETAINFO stMetaInfo;
				stMetaInfo.idxSelectEntry = (int)pstStackTop->iEntrySizeInfoOff;
				stMetaInfo.iVersion = pstStackTop->iCutOffVersion;
				stMetaInfo.pstMeta = pstCurMeta;
				stMetaInfo.chOrder = pstEntry->chOrder;
				tdr_sort_composize_entry_array_i(&stMetaInfo, (int)pstStackTop->iMetaSizeInfoUnit,
					pstStackTop->pszNetBase, pszHostEnd);
			}
			pstStackTop--;
			iStackItemCount--;
			if (0 < iStackItemCount)
			{
				pstCurMeta = pstStackTop->pstMeta;
				TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			}
			continue;
		}

		pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;
		if (pstEntry->iVersion > pstStackTop->iCutOffVersion)
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}

		/*ָ�벻����*/
		if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry))
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}
		if (TDR_ENTRY_IS_REFER_TYPE(pstEntry))
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}
		pszHostStart = pstStackTop->pszHostBase + pstEntry->iHOff;


		/*ȡ����entry�����������Ϣ*/
		TDR_GET_ARRAY_REAL_COUNT(iArrayRealCount, pstEntry, pstStackTop->pszHostBase, a_iVersion);
		if ((iArrayRealCount < 0) ||
			((0 < pstEntry->iCount) && (pstEntry->iCount < iArrayRealCount)))
		{/*ʵ����ĿΪ�������������󳤶�Ҫ������Ч*/
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_REFER_VALUE);
			break;
		}
		if (0 >= iArrayRealCount)
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}


		if (TDR_TYPE_COMPOSITE >= pstEntry->iType)
		{/*������������*/
			int idxSubEntry;
			LPTDRMETA pstTypeMeta;

			if (0 >= iArrayRealCount)
			{
				TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
				continue;
			}
			if (TDR_STACK_SIZE <=  iStackItemCount)
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_TOO_COMPLIEX_META);
				break;
			}

			if (TDR_TYPE_UNION == pstEntry->iType)
			{
				TDR_GET_UNION_ENTRY_TYPE_META_INFO(pstStackTop->pszHostBase, pstLib, pstEntry, pstStackTop->iCutOffVersion, pstTypeMeta, idxSubEntry);
				if (NULL == pstTypeMeta)
				{
					TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
					continue;
				}
			}else
			{
				pstTypeMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
				idxSubEntry = 0;
			}

			pstCurMeta = pstTypeMeta;
			iStackItemCount++;
			pstStackTop++;
			pstStackTop->iCount = iArrayRealCount;
            pstStackTop->pstMeta = pstCurMeta;
			pstStackTop->idxEntry = idxSubEntry;
			pstStackTop->iMetaSizeInfoUnit = iArrayRealCount;		/*count ����ֵ*/
			pstStackTop->pszMetaSizeInfoTarget = (char *)pstEntry;  /*��meta���ϲ�ṹ�ж��ڵ�entryָ��*/
			pstStackTop->pszHostBase = pszHostStart;
			pstStackTop->pszNetBase = pszHostStart;  /*meta���׵�ַ*/
			pstStackTop->iEntrySizeInfoOff = idxSubEntry;  /*selcet id*/
			iCutOffVersion = a_iVersion;
			if (TDR_ERR_IS_ERROR(iRet))
			{
				break;
			}
			pstStackTop->iCutOffVersion = iCutOffVersion;
			pstStackTop->iChange = 0;
		}else if (TDR_TYPE_WSTRING >= pstEntry->iType)
		{
			if ((1 < iArrayRealCount) && (TDR_SORTMETHOD_NONE_SORT != pstEntry->chOrder))
			{
				iRet = tdr_sort_simple_entry_array_i(pstEntry, pstEntry->chOrder, iArrayRealCount, pszHostStart,
					pszHostEnd);
				if (TDR_ERR_IS_ERROR(iRet))
				{
					break;
				}
			}

			/*�������ƶ�pszHostStart���Ա�����������ݵ��ֽ���*/
			if (0 < pstEntry->iCustomHUnitSize)
			{
				pszHostStart += pstEntry->iCustomHUnitSize * iArrayRealCount;
			}else
			{
				pszHostStart += pstEntry->iHUnitSize * iArrayRealCount;
			}
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
		}
	}/*while (0 < iStackItemCount)*/

	a_pstHost->iBuff = pszHostStart - a_pstHost->pszBuff;

	return iRet;
}

TDR_API int tdr_sort_metas(IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstHost, IN int a_iCount, IN TDRSORTMETHOD a_enSortMethod, IN int a_iVersion)
{
	int iRet = TDR_SUCCESS;
	int i;
	char *pszHostStart;
	char *pszHostEnd;
	TDRDATA stMetaHost;
	int iCutOffVersion;

	/*assert(NULL != a_pstMeta);
	assert(NULL != a_pstHost);
	assert(NULL != a_pstHost->pszBuff);
	assert(0 < a_pstHost->iBuff);
	assert(0 < a_iCount);*/
	if ((NULL == a_pstMeta) || (NULL == a_pstHost)||(0 >= a_iCount))
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
	}
	if ((NULL == a_pstHost->pszBuff)||(0 >= a_pstHost->iBuff))
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
	}

	if (TDR_SORTMETHOD_NONE_SORT == a_enSortMethod)
	{
		return TDR_SUCCESS;
	}
	if (0 == a_iVersion)
	{
		a_iVersion = TDR_MAX_VERSION;
	}
	if (a_pstMeta->iBaseVersion > a_iVersion)
	{
		a_pstHost->iBuff = 0;
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
	}
	if (TDR_TYPE_UNION == a_pstMeta->iType)
	{
		return TDR_SUCCESS;
	}
	if (a_pstHost->iBuff < a_pstMeta->iHUnitSize * a_iCount)
	{
		a_pstHost->iBuff = 0;
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);
	}

	pszHostStart = a_pstHost->pszBuff;
	pszHostEnd = a_pstHost->pszBuff + a_pstHost->iBuff;

	iCutOffVersion = a_iVersion;
	if (TDR_ERR_IS_ERROR(iRet))
	{
		a_pstHost->iBuff = 0;
		return iRet;
	}

	for (i = 0; i < a_iCount; i++)
	{
		stMetaHost.iBuff = a_pstMeta->iHUnitSize;
		stMetaHost.pszBuff = pszHostStart + a_pstMeta->iHUnitSize * i;
		iRet = tdr_sort(a_pstMeta, &stMetaHost, iCutOffVersion);
		if (TDR_ERR_IS_ERROR(iRet))
		{
			break;
		}
	}

	if (!TDR_ERR_IS_ERROR(iRet))
	{
		TDRSORTMETAINFO stMetaInfo;
		TDRSORTKEYINFO stSortKey; /**<sortkey����ֵ*/

		stSortKey.idxSortEntry = TDR_INVALID_INDEX;
		stSortKey.iSortKeyOff = TDR_INVALID_OFFSET;
		stSortKey.ptrSortKeyMeta = TDR_INVALID_PTR;
		stMetaInfo.idxSelectEntry = TDR_INVALID_OFFSET;
		stMetaInfo.iVersion = iCutOffVersion;
		stMetaInfo.pstMeta = a_pstMeta;
		stMetaInfo.chOrder = a_enSortMethod;
		iRet = tdr_sort_composize_entry_array_i(&stMetaInfo, a_iCount, pszHostStart, pszHostEnd);
	}

	a_pstHost->iBuff = a_pstMeta->iHUnitSize * a_iCount;

	return iRet;
}

int tdr_sort_composize_entry_array_i(IN LPTDRSORTMETAINFO a_pstMetaInfo, IN int a_iCount, IN char *a_pszHostStart,
									 IN char *a_pszHostEnd)
{
	LPTDRMETA pstMeta;
	int *piIdxMeta = NULL;
	int i;

	assert(NULL != a_pstMetaInfo);
	assert( 1 < a_iCount);
	assert(NULL != a_pszHostStart);
	assert(NULL != a_pszHostEnd);
	assert(NULL != a_pstMetaInfo->pstMeta);
	assert(TDR_SORTMETHOD_NONE_SORT != a_pstMetaInfo->chOrder);


	pstMeta = a_pstMetaInfo->pstMeta;
	if (pstMeta->iBaseVersion > a_pstMetaInfo->iVersion)
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
	}

	if ((a_pszHostStart + pstMeta->iHUnitSize * a_iCount) > a_pszHostEnd)
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);
	}

	/*��¼������meta���ݵ�����*/
	piIdxMeta = (int *)calloc(1, (a_iCount*sizeof(int)));
	if (NULL == piIdxMeta)
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_MEMORY);
	}

	for (i = 0; i < a_iCount; i++)
	{
		piIdxMeta[i] = i;
	}

	/*�������meta���ݽ�����������ʱ��ʵ���ƶ�meta���ݣ�ֻ�ƶ�����ֵ��piIdxMeta�еõ��ź����meta����*/
    if (TDR_TYPE_STRUCT == a_pstMetaInfo->pstMeta->iType)
    {
        tdr_qsort_composize_entry_array_i(piIdxMeta, 0, a_iCount - 1, a_pstMetaInfo,
            a_pszHostStart, tdr_compare_struct_metadata_i);
    }else
    {
        tdr_qsort_composize_entry_array_i(piIdxMeta, 0, a_iCount - 1, a_pstMetaInfo,
            a_pszHostStart, tdr_compare_union_metadata_i);
    }



	/*���ָ������������תһ��piIdxMeta�����е�����*/
	if (TDR_SORTMETHOD_DSC_SORT == a_pstMetaInfo->chOrder)
	{
		int iTempCount = a_iCount / 2;
		for (i = 0; i < iTempCount; i++)
		{
			int iTemp = piIdxMeta[i];
			piIdxMeta[i] = piIdxMeta[a_iCount -i -1];
			piIdxMeta[a_iCount -i -1] = iTemp;
		}
	}

	/*�����ź��������һ���Ե�������*/
	tdr_swap_meta_data_i(pstMeta, piIdxMeta, a_iCount, a_pszHostStart);

	free(piIdxMeta);

	return TDR_SUCCESS;
}

int tdr_sort_simple_entry_array_i(IN LPTDRMETAENTRY a_pstEntry, IN int a_iSortType, IN int a_iCount, IN char *a_pszHostStart,
										 IN char *a_pszHostEnd)
{
	size_t iNeedSize;
	size_t iUnitSize;
	LPCOMPAREFUNC fnCompare = NULL;

	assert(NULL != a_pstEntry);
	assert(TDR_INVALID_PTR == a_pstEntry->ptrMeta);
	assert( 1 < a_iCount);
	assert(NULL != a_pszHostStart);
	assert(NULL != a_pszHostEnd);
	assert(TDR_SORTMETHOD_NONE_SORT != a_iSortType);

	if (0 < a_pstEntry->iCustomHUnitSize)
	{
		iUnitSize = a_pstEntry->iCustomHUnitSize;
	}else
	{
		iUnitSize = a_pstEntry->iHUnitSize;
	}
	iNeedSize = iUnitSize * a_iCount;

	if ((a_pszHostStart + iNeedSize) > a_pszHostEnd)
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);
	}

	if (((TDR_TYPE_STRING == a_pstEntry->iType)|| (TDR_TYPE_WSTRING == a_pstEntry->iType)) && (0 >= a_pstEntry->iCustomHUnitSize))
	{
		/*����string���ͣ��������Ԫ�ش洢�ռ䲻�̶��򲻽�������*/
		return TDR_SUCCESS;
	}

	fnCompare = tdr_get_entry_compare_function_i(a_pstEntry->iType, a_iSortType);
	if (NULL == fnCompare)
	{
		/*�Ҳ������ʵıȽϷ���������������*/
		return TDR_SUCCESS;
	}


	tdr_qsort(a_pszHostStart, a_iCount, iUnitSize, fnCompare);

	return TDR_SUCCESS;
}

static LPCOMPAREFUNC tdr_get_entry_compare_function_i(IN int a_iEntryType,  IN int a_chOrder)
{
	LPCOMPAREFUNC fnCompare = NULL;


	if (TDR_SORTMETHOD_ASC_SORT == a_chOrder)
	{
		switch(a_iEntryType)
		{
		case TDR_TYPE_STRING:
			fnCompare = tdr_string_asc_comparefunc_i;
			break;
		case TDR_TYPE_WSTRING:
			fnCompare = tdr_wstring_asc_comparefunc_i;
			break;
		case TDR_TYPE_CHAR:
            fnCompare = tdr_char_asc_comparefunc_i;
            break;
		case TDR_TYPE_UCHAR:
			fnCompare = tdr_uchar_asc_comparefunc_i;
			break;
		case TDR_TYPE_SMALLINT:
            fnCompare = tdr_short_asc_comparefunc_i;
            break;
		case TDR_TYPE_SMALLUINT:
			fnCompare = tdr_ushort_asc_comparefunc_i;
			break;
		case TDR_TYPE_INT:
            fnCompare = tdr_int_asc_comparefunc_i;
            break;
		case TDR_TYPE_UINT:
			fnCompare = tdr_uint_asc_comparefunc_i;
			break;
		case TDR_TYPE_LONG:
            fnCompare = tdr_long_asc_comparefunc_i;
            break;
		case TDR_TYPE_ULONG:
			fnCompare = tdr_ulong_asc_comparefunc_i;
			break;
		case TDR_TYPE_LONGLONG:
            fnCompare = tdr_longlong_asc_comparefunc_i;
            break;
		case TDR_TYPE_ULONGLONG:
			fnCompare = tdr_ulonglong_asc_comparefunc_i;
			break;
		case TDR_TYPE_FLOAT:
			fnCompare = tdr_float_asc_comparefunc_i;
			break;
		case TDR_TYPE_DOUBLE:
			fnCompare = tdr_double_asc_comparefunc_i;
			break;
		case TDR_TYPE_DATETIME:
			fnCompare = (LPCOMPAREFUNC)tdr_compare_datetime;
			break;
		case TDR_TYPE_DATE:
			fnCompare = (LPCOMPAREFUNC)tdr_compare_date;
			break;
		case TDR_TYPE_TIME:
			fnCompare = (LPCOMPAREFUNC)tdr_compare_time;
			break;
		case TDR_TYPE_IP:
			fnCompare = tdr_ip_asc_comparefunc_i;
			break;
		case TDR_TYPE_WCHAR:
			fnCompare = tdr_wchar_asc_comparefunc_i;
			break;
		default:
			break;
		}
	}else if (TDR_SORTMETHOD_DSC_SORT == a_chOrder)
	{
		switch(a_iEntryType)
		{
		case TDR_TYPE_STRING:
			fnCompare = tdr_string_desc_comparefunc_i;
			break;
		case TDR_TYPE_WSTRING:
			fnCompare = tdr_wstring_desc_comparefunc_i;
			break;
		case TDR_TYPE_CHAR:
            fnCompare = tdr_char_desc_comparefunc_i;
            break;
		case TDR_TYPE_UCHAR:
			fnCompare = tdr_uchar_desc_comparefunc_i;
			break;
		case TDR_TYPE_SMALLINT:
            fnCompare = tdr_short_desc_comparefunc_i;
            break;
		case TDR_TYPE_SMALLUINT:
			fnCompare = tdr_ushort_desc_comparefunc_i;
			break;
		case TDR_TYPE_INT:
            fnCompare = tdr_int_desc_comparefunc_i;
            break;
		case TDR_TYPE_UINT:
			fnCompare = tdr_uint_desc_comparefunc_i;
			break;
		case TDR_TYPE_LONG:
            fnCompare = tdr_long_desc_comparefunc_i;
            break;
		case TDR_TYPE_ULONG:
			fnCompare = tdr_ulong_desc_comparefunc_i;
			break;
		case TDR_TYPE_LONGLONG:
            fnCompare = tdr_longlong_desc_comparefunc_i;
            break;
		case TDR_TYPE_ULONGLONG:
			fnCompare = tdr_ulonglong_desc_comparefunc_i;
			break;
		case TDR_TYPE_FLOAT:
			fnCompare = tdr_float_desc_comparefunc_i;
			break;
		case TDR_TYPE_DOUBLE:
			fnCompare = tdr_double_desc_comparefunc_i;
			break;
		case TDR_TYPE_DATETIME:
			fnCompare = (LPCOMPAREFUNC)tdr_datetime_desc_comparefunc_i;
			break;
		case TDR_TYPE_DATE:
			fnCompare = (LPCOMPAREFUNC)tdr_date_desc_comparefunc_i;
			break;
		case TDR_TYPE_TIME:
			fnCompare = (LPCOMPAREFUNC)tdr_time_desc_comparefunc_i;
			break;
		case TDR_TYPE_IP:
			fnCompare = tdr_ip_desc_comparefunc_i;
			break;
		case TDR_TYPE_WCHAR:
			fnCompare = tdr_wchar_desc_comparefunc_i;
			break;
		default:
			break;
		}
	}

	return fnCompare;
}

int tdr_string_asc_comparefunc_i(const void *pCompare1, const void *pCompare2)
{
	int ret = 0 ;
	unsigned char *pszSrc = (unsigned char *)pCompare1;
	unsigned char *pszDst = (unsigned char *)pCompare2;

	while( ! (ret = *pszSrc - *pszDst) && *pszDst)
	{
		++pszSrc;
		++pszDst;
	}

	if ( ret < 0 )
		ret = -1 ;
	else if ( ret > 0 )
		ret = 1 ;

	return( ret );
}

int tdr_wstring_asc_comparefunc_i(const void *pCompare1, const void *pCompare2)
{
	int ret = 0 ;
	tdr_wchar_t *pszSrc = (tdr_wchar_t *)pCompare1;
	tdr_wchar_t *pszDst = (tdr_wchar_t *)pCompare2;

	while( ! (ret = *pszSrc - *pszDst) && *pszDst)
	{
		++pszSrc;
		++pszDst;
	}

	if ( ret < 0 )
		ret = -1 ;
	else if ( ret > 0 )
		ret = 1 ;

	return( ret );
}

int tdr_wstring_desc_comparefunc_i(const void *pCompare1, const void *pCompare2)
{
	int ret = 0 ;
	tdr_wchar_t *pszSrc = (tdr_wchar_t *)pCompare1;
	tdr_wchar_t *pszDst = (tdr_wchar_t *)pCompare2;

	while( ! (ret = *pszSrc - *pszDst) && *pszDst)
	{
		++pszSrc;
		++pszDst;
	}

	if ( ret < 0 )
		ret = 1 ;
	else if ( ret > 0 )
		ret = -1 ;

	return( ret );
}


int tdr_string_desc_comparefunc_i(const void *pCompare1, const void *pCompare2)
{
	int ret = 0 ;
	unsigned char *pszSrc = (unsigned char *)pCompare1;
	unsigned char *pszDst = (unsigned char *)pCompare2;

	while( ! (ret = *pszSrc - *pszDst) && *pszDst)
	{
		++pszSrc;
		++pszDst;
	}

	if ( ret < 0 )
		ret = 1 ;
	else if ( ret > 0 )
		ret = -1 ;

	return( ret );
}

int tdr_char_asc_comparefunc_i(const void *pCompare1, const void *pCompare2)
{
	char *pch1 = (char *)pCompare1;
	char *pch2 = (char *)pCompare2;

	return (int)(*pch1 - *pch2);
}

int tdr_char_desc_comparefunc_i(const void *pCompare1, const void *pCompare2)
{
	char *pch1 = (char *)pCompare1;
	char *pch2 = (char *)pCompare2;

	return (int)(*pch2 - *pch1);
}

int tdr_uchar_asc_comparefunc_i(const void *pCompare1, const void *pCompare2)
{
    unsigned char *pch1 = (unsigned  char *)pCompare1;
    unsigned char *pch2 = (unsigned char *)pCompare2;

    return (int)(*pch1 - *pch2);
}

int tdr_uchar_desc_comparefunc_i(const void *pCompare1, const void *pCompare2)
{
    unsigned char *pch1 = (unsigned char *)pCompare1;
    unsigned char *pch2 = (unsigned char *)pCompare2;

    return (int)(*pch2 - *pch1);
}


int tdr_short_asc_comparefunc_i(const void *pCompare1, const void *pCompare2)
{
	short *pnValue1 = (short *)pCompare1;
	short *pnValue2 = (short *)pCompare2;

	return (*pnValue1 - *pnValue2);
}

int tdr_short_desc_comparefunc_i(const void *pCompare1, const void *pCompare2)
{
	short *pnValue1 = (short *)pCompare1;
	short *pnValue2 = (short *)pCompare2;

	return (int)(*pnValue2 - *pnValue1);
}

int tdr_ushort_asc_comparefunc_i(const void *pCompare1, const void *pCompare2)
{
    unsigned short *pnValue1 = (unsigned short *)pCompare1;
    unsigned short *pnValue2 = (unsigned short *)pCompare2;

    return (*pnValue1 - *pnValue2);
}

int tdr_ushort_desc_comparefunc_i(const void *pCompare1, const void *pCompare2)
{
    unsigned short *pnValue1 = (unsigned short *)pCompare1;
    unsigned short *pnValue2 = (unsigned short *)pCompare2;

    return (int)(*pnValue2 - *pnValue1);
}

int tdr_int_desc_comparefunc_i(const void *pCompare1, const void *pCompare2)
{
	int *pnValue1 = (int *)pCompare1;
	int *pnValue2 = (int *)pCompare2;

	if (*pnValue1 < *pnValue2)
	{
		return 1;
	}

	if (*pnValue1 > *pnValue2)
	{
		return -1;
	}
	return 0;
}

int tdr_int_asc_comparefunc_i(const void *pCompare1, const void *pCompare2)
{
	int *pnValue1 = (int *)pCompare1;
	int *pnValue2 = (int *)pCompare2;

	if (*pnValue1 < *pnValue2)
	{
		return -1;
	}

	if (*pnValue1 > *pnValue2)
	{
		return 1;
	}
	return 0;
}

int tdr_uint_desc_comparefunc_i(const void *pCompare1, const void *pCompare2)
{
    uint32_t *pnValue1 = (uint32_t *)pCompare1;
    uint32_t *pnValue2 = (uint32_t *)pCompare2;

    if (*pnValue1 < *pnValue2)
    {
        return 1;
    }

    if (*pnValue1 > *pnValue2)
    {
        return -1;
    }
    return 0;
}



int tdr_date_desc_comparefunc_i(const void *pCompare1, const void *pCompare2)
{
	LPTDRDATE pnValue1 = (LPTDRDATE )pCompare1;
	LPTDRDATE pnValue2 = (LPTDRDATE )pCompare2;

	if (pnValue1->nYear > pnValue2->nYear)
	{
		return -1;
	}else if (pnValue1->nYear < pnValue2->nYear)
	{
		return 1;
	}

	if (pnValue1->bMonth > pnValue2->bMonth)
	{
		return -1;
	}else if (pnValue1->bMonth < pnValue2->bMonth)
	{
		return 1;
	}

	if (pnValue1->bDay > pnValue2->bDay)
	{
		return -1;
	}else if (pnValue1->bDay < pnValue2->bDay)
	{
		return 1;
	}

	return 0;
}


int tdr_uint_asc_comparefunc_i(const void *pCompare1, const void *pCompare2)
{
    uint32_t *pnValue1 = (uint32_t *)pCompare1;
    uint32_t *pnValue2 = (uint32_t *)pCompare2;

    if (*pnValue1 < *pnValue2)
    {
        return -1;
    }

    if (*pnValue1 > *pnValue2)
    {
        return 1;
    }
    return 0;
}

int tdr_long_desc_comparefunc_i(const void *pCompare1, const void *pCompare2)
{
	long *pnValue1 = (long *)pCompare1;
	long *pnValue2 = (long *)pCompare2;

	if (*pnValue1 < *pnValue2)
	{
		return 1;
	}

	if (*pnValue1 > *pnValue2)
	{
		return -1;
	}
	return 0;
}

int tdr_long_asc_comparefunc_i(const void *pCompare1, const void *pCompare2)
{
	long *pnValue1 = (long *)pCompare1;
	long *pnValue2 = (long *)pCompare2;

	if (*pnValue1 < *pnValue2)
	{
		return -1;
	}

	if (*pnValue1 > *pnValue2)
	{
		return 1;
	}
	return 0;
}

int tdr_ulong_desc_comparefunc_i(const void *pCompare1, const void *pCompare2)
{
    unsigned long *pnValue1 = (unsigned long *)pCompare1;
    unsigned long *pnValue2 = (unsigned long *)pCompare2;

    if (*pnValue1 < *pnValue2)
    {
        return 1;
    }

    if (*pnValue1 > *pnValue2)
    {
        return -1;
    }
    return 0;
}

int tdr_ulong_asc_comparefunc_i(const void *pCompare1, const void *pCompare2)
{
    unsigned long *pnValue1 = (unsigned long *)pCompare1;
    unsigned long *pnValue2 = (unsigned long *)pCompare2;

    if (*pnValue1 < *pnValue2)
    {
        return -1;
    }

    if (*pnValue1 > *pnValue2)
    {
        return 1;
    }
    return 0;
}

int tdr_longlong_desc_comparefunc_i(const void *pCompare1, const void *pCompare2)
{
	tdr_longlong *pnValue1 = (tdr_longlong *)pCompare1;
	tdr_longlong *pnValue2 = (tdr_longlong *)pCompare2;

	if (*pnValue1 < *pnValue2)
	{
		return 1;
	}

	if (*pnValue1 > *pnValue2)
	{
		return -1;
	}
	return 0;
}

int tdr_longlong_asc_comparefunc_i(const void *pCompare1, const void *pCompare2)
{
	tdr_longlong *pnValue1 = (tdr_longlong *)pCompare1;
	tdr_longlong *pnValue2 = (tdr_longlong *)pCompare2;

	if (*pnValue1 < *pnValue2)
	{
		return -1;
	}

	if (*pnValue1 > *pnValue2)
	{
		return 1;
	}
	return 0;
}

int tdr_ulonglong_desc_comparefunc_i(const void *pCompare1, const void *pCompare2)
{
    uint64_t *pnValue1 = (uint64_t *)pCompare1;
    uint64_t *pnValue2 = (uint64_t *)pCompare2;

    if (*pnValue1 < *pnValue2)
    {
        return 1;
    }

    if (*pnValue1 > *pnValue2)
    {
        return -1;
    }
    return 0;
}

int tdr_ulonglong_asc_comparefunc_i(const void *pCompare1, const void *pCompare2)
{
    uint64_t *pnValue1 = (uint64_t *)pCompare1;
    uint64_t *pnValue2 = (uint64_t *)pCompare2;

    if (*pnValue1 < *pnValue2)
    {
        return -1;
    }

    if (*pnValue1 > *pnValue2)
    {
        return 1;
    }
    return 0;
}

int tdr_float_desc_comparefunc_i(const void *pCompare1, const void *pCompare2)
{
	float *pnValue1 = (float *)pCompare1;
	float *pnValue2 = (float *)pCompare2;

	if (fabs(*pnValue1 - *pnValue2) < 0.00001)
	{
		return 0;
	}else if (*pnValue1 < *pnValue2)
	{
		return 1;
	}

	return -1;
}

int tdr_float_asc_comparefunc_i(const void *pCompare1, const void *pCompare2)
{
	float *pnValue1 = (float *)pCompare1;
	float *pnValue2 = (float *)pCompare2;

	if (fabs(*pnValue1 - *pnValue2) < 0.00001)
	{
		return 0;
	}else if (*pnValue1 < *pnValue2)
	{
		return -1;
	}

	return 1;
}

int tdr_double_desc_comparefunc_i(const void *pCompare1, const void *pCompare2)
{
	double *pnValue1 = (double *)pCompare1;
	double *pnValue2 = (double *)pCompare2;

	if (fabs(*pnValue1 - *pnValue2) < 0.000001)
	{
		return 0;
	}else if (*pnValue1 < *pnValue2)
	{
		return 1;
	}

	return -1;
}

int tdr_double_asc_comparefunc_i(const void *pCompare1, const void *pCompare2)
{
	double *pnValue1 = (double *)pCompare1;
	double *pnValue2 = (double *)pCompare2;

	if (fabs(*pnValue1 - *pnValue2) < 0.000001)
	{
		return 0;
	}else if (*pnValue1 < *pnValue2)
	{
		return -1;
	}

	return 1;
}


int tdr_time_desc_comparefunc_i(const void *pCompare1, const void *pCompare2)
{
	LPTDRTIME pnValue1 = (LPTDRTIME )pCompare1;
	LPTDRTIME pnValue2 = (LPTDRTIME )pCompare2;

	if (pnValue1->nHour > pnValue2->nHour)
	{
		return -1;
	}else if (pnValue1->nHour < pnValue2->nHour)
	{
		return 1;
	}

	if (pnValue1->bMin > pnValue2->bMin)
	{
		return -1;
	}else if (pnValue1->bMin < pnValue2->bMin)
	{
		return 1;
	}

	if (pnValue1->bSec > pnValue2->bSec)
	{
		return -1;
	}else if (pnValue1->bSec < pnValue2->bSec)
	{
		return 1;
	}

	return 0;
}


int tdr_datetime_desc_comparefunc_i(const void *pCompare1, const void *pCompare2)
{
	int iRet;

	LPTDRDATETIME pnValue1 = (LPTDRDATETIME )pCompare1;
	LPTDRDATETIME pnValue2 = (LPTDRDATETIME )pCompare2;

	iRet = tdr_date_desc_comparefunc_i(&pnValue1->stDate, &pnValue2->stDate);
	if (0 != iRet)
	{
		return iRet;
	}

	iRet = tdr_time_desc_comparefunc_i(&pnValue1->stTime, &pnValue2->stTime);

	return iRet;
}

int tdr_ip_asc_comparefunc_i(const void *pCompare1, const void *pCompare2)
{
	uint32_t i;
	int iRet = 0;

	unsigned char *pnValue1 = (unsigned char *)pCompare1;
	unsigned char *pnValue2 = (unsigned char *)pCompare2;

	for (i = 0; i < sizeof(tdr_ip_t); i++)
	{
		iRet = *pnValue1 - *pnValue2;
		if (0 != iRet)
		{
			break;
		}
		pnValue1++;
		pnValue2++;
	}

	return iRet;
}

int tdr_ip_desc_comparefunc_i(const void *pCompare1, const void *pCompare2)
{
	uint32_t i;
	int iRet = 0;

	unsigned char *pnValue1 = (unsigned char *)pCompare1;
	unsigned char *pnValue2 = (unsigned char *)pCompare2;

	for (i = 0; i < sizeof(tdr_ip_t); i++)
	{
		iRet = *pnValue2 - *pnValue1;
		if (0 != iRet)
		{
			break;
		}
		pnValue1++;
		pnValue2++;
	}

	return iRet;
}

int tdr_wchar_asc_comparefunc_i(const void *pCompare1, const void *pCompare2)
{
	tdr_wchar_t *pnValue1 = (tdr_wchar_t *)pCompare1;
	tdr_wchar_t *pnValue2 = (tdr_wchar_t *)pCompare2;

	return (*pnValue1 - *pnValue2);
}

int tdr_wchar_desc_comparefunc_i(const void *pCompare1, const void *pCompare2)
{
	tdr_wchar_t *pnValue1 = (tdr_wchar_t *)pCompare1;
	tdr_wchar_t *pnValue2 = (tdr_wchar_t *)pCompare2;

	return (*pnValue2 - *pnValue1);
}



int tdr_compare_struct_metadata_i(IN LPTDRSORTMETAINFO a_pstMetaInfo, IN char *a_pszHostData1, char *a_pszHostData2)
{
	char *pszHostEntry1;
	char *pszHostEntry2;
	LPTDRMETAENTRY pstComparedEntry;
	LPTDRMETALIB pstLib;
	int iRet = 0;
	LPTDRMETA pstMeta;
	LPTDRSORTKEYINFO pstSortkey;
	TDRSORTENTRYINFO stSortEntryInfo;
	int iCount1;
	int iCount2;
	int idxSelect1;
	int idxSelect2;

	assert(NULL != a_pstMetaInfo);
	assert(NULL != a_pszHostData1);
	assert(NULL != a_pszHostData2);
	assert(NULL != a_pstMetaInfo->pstMeta);
	assert(TDR_TYPE_STRUCT == a_pstMetaInfo->pstMeta->iType);

	pstMeta = a_pstMetaInfo->pstMeta;
	pstLib = TDR_META_TO_LIB(pstMeta);

    if (a_pszHostData1 == a_pszHostData2)
    {
        return 0;
    }

	/*�Ƚ�struct���͵�����*/
	pstSortkey = &pstMeta->stSortKey;
	stSortEntryInfo.iVersion = a_pstMetaInfo->iVersion;
	if ((TDR_INVALID_OFFSET != pstSortkey->iSortKeyOff) && (TDR_INVALID_PTR != pstSortkey->ptrSortKeyMeta) &&
		(TDR_INVALID_INDEX != pstSortkey->idxSortEntry))
	{
		/*���ָ������ؼ���,������ؼ��ֽ��бȽ�*/
		LPTDRMETA pstSortMeta;

		pstSortMeta = TDR_PTR_TO_META(pstLib, pstSortkey->ptrSortKeyMeta);
		pstComparedEntry = pstSortMeta->stEntries + pstSortkey->idxSortEntry;
		if (pstComparedEntry->iVersion > a_pstMetaInfo->iVersion)
		{
			iRet = 0;
		}else
		{
			pszHostEntry1 = a_pszHostData1 + pstSortkey->iSortKeyOff;
			pszHostEntry2 = a_pszHostData2 + pstSortkey->iSortKeyOff;
			if (TDR_ENTRY_IS_REFER_TYPE(pstComparedEntry))
			{
				pszHostEntry1 = *(char **)pszHostEntry1;
				pszHostEntry2 = *(char **)pszHostEntry2;
			}

			TDR_GET_ARRAY_REAL_COUNT(iCount1, pstComparedEntry, a_pszHostData1, stSortEntryInfo.iVersion);
			TDR_GET_ARRAY_REAL_COUNT(iCount2, pstComparedEntry, a_pszHostData1, stSortEntryInfo.iVersion);
			TDR_GET_UNION_ENTRY_INDEX(idxSelect1, pstComparedEntry, a_pszHostData1);
			TDR_GET_UNION_ENTRY_INDEX(idxSelect2, pstComparedEntry, a_pszHostData2);
			stSortEntryInfo.iCount1 = iCount1;
			stSortEntryInfo.iCount2 = iCount2;
			if (idxSelect1 != idxSelect2)
			{
				stSortEntryInfo.idxSelectEntry = TDR_INVALID_INDEX;
			}else
			{
				stSortEntryInfo.idxSelectEntry = idxSelect1;
			}
			stSortEntryInfo.pstEntry = pstComparedEntry;
			iRet = tdr_compare_entry_data_i(pstLib, &stSortEntryInfo, pszHostEntry1, pszHostEntry2);
		}/*if (pstComparedEntry->iVersion > a_iVersion)*/
	}else
	{
		/*����Ƚϸ���Ա*/
		int i;

		for (i = 0; i < pstMeta->iEntriesNum; i++)
		{
			pstComparedEntry = pstMeta->stEntries + i;
			if (pstComparedEntry->iVersion > a_pstMetaInfo->iVersion)
			{
				continue;
			}
			if (TDR_ENTRY_IS_POINTER_TYPE(pstComparedEntry))
			{
				continue;
			}

			pszHostEntry1 = a_pszHostData1 + pstComparedEntry->iHOff;
			pszHostEntry2 = a_pszHostData2 + pstComparedEntry->iHOff;
			if (TDR_ENTRY_IS_REFER_TYPE(pstComparedEntry))
			{
				pszHostEntry1 = *(char **)pszHostEntry1;
				pszHostEntry2 = *(char **)pszHostEntry2;
			}

			TDR_GET_ARRAY_REAL_COUNT(iCount1, pstComparedEntry, a_pszHostData1, stSortEntryInfo.iVersion);
			TDR_GET_ARRAY_REAL_COUNT(iCount2, pstComparedEntry, a_pszHostData1, stSortEntryInfo.iVersion);
			TDR_GET_UNION_ENTRY_INDEX(idxSelect1, pstComparedEntry, a_pszHostData1);
			TDR_GET_UNION_ENTRY_INDEX(idxSelect2, pstComparedEntry, a_pszHostData2);
			stSortEntryInfo.iCount1 = iCount1;
			stSortEntryInfo.iCount2 = iCount2;
			if (idxSelect1 != idxSelect2)
			{
				stSortEntryInfo.idxSelectEntry = TDR_INVALID_INDEX;
			}else
			{
				stSortEntryInfo.idxSelectEntry = idxSelect1;
			}
			stSortEntryInfo.pstEntry = pstComparedEntry;
			iRet = tdr_compare_entry_data_i(pstLib, &stSortEntryInfo, pszHostEntry1, pszHostEntry2);
			if (0 != iRet)
			{
				break;
			}
		}/*for (i = 0; i < a_pstMeta->iEntriesNum; i++)*/
	}/*if ((TDR_INVALID_OFFSET != pstMeta->iSortKeyOff) && (TDR_INVALID_PTR != a_pstSortKey->ptrSortKeyMeta) &&*/

	return iRet;
}

int tdr_compare_union_metadata_i(IN LPTDRSORTMETAINFO a_pstMetaInfo, IN char *a_pszHostData1, char *a_pszHostData2)
{
	char *pszHostEntry1;
	char *pszHostEntry2;
	LPTDRMETAENTRY pstComparedEntry;
	LPTDRMETALIB pstLib;
	int iRet = 0;
	LPTDRMETA pstMeta;
	int iCount1;
	int iCount2;
	TDRSORTENTRYINFO stSortEntryInfo;

	assert(NULL != a_pstMetaInfo);
	assert(NULL != a_pszHostData1);
	assert(NULL != a_pszHostData2);
	assert(NULL != a_pstMetaInfo->pstMeta);
	assert((TDR_TYPE_UNION == a_pstMetaInfo->pstMeta->iType));

	pstMeta = a_pstMetaInfo->pstMeta;
	pstLib = TDR_META_TO_LIB(pstMeta);

    if (a_pszHostData1 == a_pszHostData2)
    {
        return 0;
    }

	/*�Ƚ�union*/
	if ((0 > a_pstMetaInfo->idxSelectEntry) || (a_pstMetaInfo->idxSelectEntry >= pstMeta->iEntriesNum))
	{
		return 0;
	}

	pstComparedEntry = pstMeta->stEntries + a_pstMetaInfo->idxSelectEntry;
	if (pstComparedEntry->iVersion > a_pstMetaInfo->iVersion)
	{
		iRet = 0;
	}else
	{
		pszHostEntry1 = a_pszHostData1 + pstComparedEntry->iHOff;
		pszHostEntry2 = a_pszHostData2 + pstComparedEntry->iHOff;
		if (TDR_ENTRY_IS_REFER_TYPE(pstComparedEntry))
		{
			pszHostEntry1 = *(char **)pszHostEntry1;
			pszHostEntry2 = *(char **)pszHostEntry2;
		}

		TDR_GET_ARRAY_REAL_COUNT(iCount1, pstComparedEntry, a_pszHostData1, a_pstMetaInfo->iVersion);
		TDR_GET_ARRAY_REAL_COUNT(iCount2, pstComparedEntry, a_pszHostData1, a_pstMetaInfo->iVersion);
		stSortEntryInfo.iCount1 = iCount1;
		stSortEntryInfo.iCount2 = iCount2;
		stSortEntryInfo.idxSelectEntry = TDR_INVALID_INDEX;
		stSortEntryInfo.pstEntry = pstComparedEntry;
		stSortEntryInfo.iVersion = a_pstMetaInfo->iVersion;
		iRet = tdr_compare_entry_data_i(pstLib, &stSortEntryInfo, pszHostEntry1, pszHostEntry2);
	}/*if (pstComparedEntry->iVersion > a_iVersion)*/
	return iRet;
}

int tdr_compare_entry_data_i(LPTDRMETALIB a_pstLib, LPTDRSORTENTRYINFO a_pstSortEntryInfo, char *a_pszHostEntry1,
							 char *a_pszHostEntry2)
{
	int iRet = 0;
	int iMinCount;
	char *pszHostEntry1;
	char *pszHostEntry2;
	int i;
	LPTDRMETAENTRY pstComparedEntry;

	assert(NULL != a_pstLib);
	assert(NULL != a_pstSortEntryInfo);
	assert(NULL != a_pszHostEntry1);
	assert(NULL != a_pszHostEntry2);
	assert(NULL != a_pstSortEntryInfo->pstEntry);

    if (a_pszHostEntry1 == a_pszHostEntry2)
    {
        return 0;
    }

	pstComparedEntry = a_pstSortEntryInfo->pstEntry;
	iMinCount = TDR_MIN(a_pstSortEntryInfo->iCount1, a_pstSortEntryInfo->iCount2);
	if ((1 < iMinCount) && (TDR_SORTMETHOD_NONE_SORT == pstComparedEntry->chOrder))
	{
		/*û����������鲻���бȽϣ���Ϊ���ֱ�ӷ���*/
		return 0;
	}


	pszHostEntry1 = a_pszHostEntry1;
	pszHostEntry2 = a_pszHostEntry2;
	if (TDR_INVALID_PTR != pstComparedEntry->ptrMeta)
	{
		/*������������*/

		TDRSORTMETAINFO stMetaInfo;

		stMetaInfo.pstMeta = TDR_PTR_TO_META(a_pstLib, pstComparedEntry->ptrMeta);
		stMetaInfo.chOrder = pstComparedEntry->chOrder;
		stMetaInfo.idxSelectEntry = a_pstSortEntryInfo->idxSelectEntry;
		stMetaInfo.iVersion = a_pstSortEntryInfo->iVersion;

		if (TDR_TYPE_STRUCT == pstComparedEntry->iType)
		{
			for (i = 0; i < iMinCount; i++)
			{
				iRet = tdr_compare_struct_metadata_i(&stMetaInfo, pszHostEntry1, pszHostEntry2);
				if (0 != iRet)
				{
					break;
				}
				pszHostEntry1 += pstComparedEntry->iHUnitSize;
				pszHostEntry2 += pstComparedEntry->iHUnitSize;
			}
		}else
		{
			for (i = 0; i < iMinCount; i++)
			{
				iRet = tdr_compare_union_metadata_i(&stMetaInfo, pszHostEntry1, pszHostEntry2);
				if (0 != iRet)
				{
					break;
				}
				pszHostEntry1 += pstComparedEntry->iHUnitSize;
				pszHostEntry2 += pstComparedEntry->iHUnitSize;
			}
		}/*if (TDR_TYPE_STRUCT == pstComparedEntry->iType)*/
	}else
	{
		/*����������*/
		LPCOMPAREFUNC fnCompare = NULL;
		fnCompare = tdr_get_entry_compare_function_i(pstComparedEntry->iType, TDR_SORTMETHOD_ASC_SORT);
		if (NULL == fnCompare)
		{
			return 0;
		}

		for (i = 0; i < iMinCount; i++)
		{
			iRet = fnCompare(pszHostEntry1, pszHostEntry2);
			if (0 != iRet)
			{
				break;
			}
			pszHostEntry1 += pstComparedEntry->iHUnitSize;
			pszHostEntry2 += pstComparedEntry->iHUnitSize;
		}
	}/*if (TDR_INVALID_PTR != a_pstComparedEntry->ptrMeta)*/

	/*ǰiMinCount�����ȣ������鳤�ȳ�����Ϊֵ��һЩ*/
	if (0 == iRet)
	{
		if (a_pstSortEntryInfo->iCount1 > iMinCount)
		{
			iRet = 1;
		}else if (a_pstSortEntryInfo->iCount2 > iMinCount)
		{
			iRet = -1;
		}
	}

	return iRet;
}

int tdr_qsort_composize_entry_array_i(INOUT int *pidxMetas, IN int a_iBegin, IN int a_iEnd, LPTDRSORTMETAINFO a_pstMetaInfo,
                                      char *a_pszHostStart, LPTDRCMPMETADATAFUNC a_pfnCmp)
{
    int i;
    int j;
    int iMid;
    int iRet ;
    int iTemp;
    char *pszHostVot;
    char *pszHostCompare;
    char *pszBegin;
    char *pszMid;
    char *pszEnd;

    assert(NULL != a_pstMetaInfo);
    assert(NULL != pidxMetas);
    assert(NULL != a_pstMetaInfo->pstMeta);
    assert(0 <= a_iBegin);
    assert(NULL != a_pszHostStart);
    assert(TDR_TYPE_COMPOSITE >= a_pstMetaInfo->pstMeta->iType);
    assert(NULL != a_pfnCmp);

    if (a_iBegin >= a_iEnd)
    {
        return TDR_SUCCESS;
    }

    /**ѡ��һ���м�ֵ��Ϊ ��������
    */
    iMid = (a_iBegin + a_iEnd) / 2;
    pszBegin =  a_pszHostStart + a_pstMetaInfo->pstMeta->iHUnitSize * pidxMetas[a_iBegin];
    pszMid = a_pszHostStart + a_pstMetaInfo->pstMeta->iHUnitSize * pidxMetas[iMid];
    pszEnd = a_pszHostStart + a_pstMetaInfo->pstMeta->iHUnitSize * pidxMetas[a_iEnd];
    if ((*a_pfnCmp)(a_pstMetaInfo, pszBegin, pszMid) > 0)
    {
        iTemp = pidxMetas[a_iBegin];
        pidxMetas[a_iBegin] = pidxMetas[iMid];
        pidxMetas[iMid] =  iTemp;
        pszHostVot = pszBegin;
        pszBegin =  pszMid;
        pszMid =  pszHostVot;
    }
    if ((*a_pfnCmp)(a_pstMetaInfo, pszMid, pszEnd) > 0)
    {
        iTemp = pidxMetas[a_iEnd];
        pidxMetas[a_iEnd] = pidxMetas[iMid];
        pidxMetas[iMid] =  iTemp;
        pszHostVot = pszMid;
        pszMid =  pszEnd;
        pszEnd =  pszHostVot;
    }
    if ((*a_pfnCmp)(a_pstMetaInfo, pszBegin, pszMid) < 0)
    {
        iTemp = pidxMetas[a_iBegin];
        pidxMetas[a_iBegin] = pidxMetas[iMid];
        pidxMetas[iMid] =  iTemp;
    }


    pszHostVot = a_pszHostStart + a_pstMetaInfo->pstMeta->iHUnitSize * pidxMetas[a_iBegin];
    i = a_iBegin -1 ;
    j = a_iEnd + 1;
    while (1)
    {
        /*�Ӻ���ǰ�ҵ���һ��С��pvot��ֵ*/
        do
        {
            j--;
            pszHostCompare = a_pszHostStart + a_pstMetaInfo->pstMeta->iHUnitSize * pidxMetas[j];
            iRet = (*a_pfnCmp)(a_pstMetaInfo, pszHostVot, pszHostCompare);
        }while (0 > iRet);


        /*��ǰ�����һ������pvotֵ��Ԫ�طŵ�λ��*/
        do
        {
            i++;
            pszHostCompare = a_pszHostStart + a_pstMetaInfo->pstMeta->iHUnitSize * pidxMetas[i];
            iRet = (*a_pfnCmp)(a_pstMetaInfo, pszHostVot, pszHostCompare);
        }while (0 < iRet);

        if (i < j)
        {
            iTemp = pidxMetas[i];
            pidxMetas[i] = pidxMetas[j];
            pidxMetas[j] =  iTemp;
        }else
        {
            break;
        }
    }/*while (1)*/

    if (j == a_iEnd)
    {
        j--;
    }
    tdr_qsort_composize_entry_array_i(pidxMetas, a_iBegin, j, a_pstMetaInfo, a_pszHostStart,a_pfnCmp);

    tdr_qsort_composize_entry_array_i(pidxMetas, j + 1, a_iEnd, a_pstMetaInfo, a_pszHostStart,a_pfnCmp);

    return TDR_SUCCESS;
}

int tdr_swap_meta_data_i(IN LPTDRMETA a_pstMeta, IN int *a_pidxMetas, IN int a_iCount, char *a_pszHostStart)
{
    TDRBOOLEAN *pbVisits;
    int i,j;
    char  *pszTempMeta;
    char *pszMetaData1;
    char *pszMetaData2;

    assert(NULL != a_pstMeta);
    assert(NULL != a_pidxMetas);
    assert(0 < a_iCount);
    assert(NULL != a_pszHostStart);

    pbVisits = (TDRBOOLEAN *)calloc(1, (a_iCount*sizeof(TDRBOOLEAN)));
    if (NULL == pbVisits)
    {
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_MEMORY);
    }
    pszTempMeta = (char *)calloc(1, a_pstMeta->iHUnitSize);
    if (NULL == pszTempMeta)
    {
        return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_MEMORY);
    }

    for (i = 0; i < a_iCount; i++)
    {
        pbVisits[i] = TDR_FALSE;
    }

    for (i = 0; i < a_iCount; i++)
    {
        if (TDR_TRUE == pbVisits[i])
        {
            continue;
        }
        if (i == a_pidxMetas[i])
        {
            pbVisits[i] = TDR_TRUE;
            continue;
        }

        pszMetaData1 = a_pszHostStart + a_pstMeta->iHUnitSize * a_pidxMetas[i];
        memcpy(pszTempMeta, pszMetaData1, a_pstMeta->iHUnitSize);
        for (j = a_pidxMetas[i]; j != i;)
        {
           pszMetaData1 = a_pszHostStart + a_pstMeta->iHUnitSize * a_pidxMetas[j];
           pszMetaData2 = a_pszHostStart + a_pstMeta->iHUnitSize * j;
           memcpy(pszMetaData2, pszMetaData1, a_pstMeta->iHUnitSize);
           pbVisits[j] = TDR_TRUE;
           j = a_pidxMetas[j];
        }
        pszMetaData2 = a_pszHostStart + a_pstMeta->iHUnitSize * i;
        memcpy(pszMetaData2, pszTempMeta, a_pstMeta->iHUnitSize);
        pbVisits[i] = TDR_TRUE;
    }/*for (i = 0; i < a_iCount; i++)*/

    return TDR_SUCCESS;
}


int tdr_sort_spec(IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstHost, IN int a_iSortType, IN const char* a_pszSortEntry, IN int a_iVersion)
{
	TDRSORTKEYINFO stSortKey;
	int iRet = TDR_SUCCESS;
	LPTDRMETALIB pstLib;
	LPTDRMETAENTRY pstSortEntry;
	LPTDRMETA pstSortMeta;
	char *pszSortEntryHost;
	char *pszSortMetaHost;
	char *pszHostEnd;
	int iArrayRealCount;

    if ((NULL == a_pstMeta)
        || (NULL == a_pstHost)
        || (NULL == a_pstHost->pszBuff)
        || (0 >= a_pstHost->iBuff)
        || (NULL == a_pszSortEntry))
    {
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
    }


	if (0 == a_iVersion)
	{
		a_iVersion = TDR_MAX_VERSION;
	}
	if (a_pstMeta->iBaseVersion > a_iVersion)
	{
		a_pstHost->iBuff = 0;
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
	}
	if (TDR_SORTMETHOD_NONE_SORT == a_iSortType)
	{
		return TDR_SUCCESS;
	}
	if ((TDR_SORTMETHOD_ASC_SORT != a_iSortType) && (TDR_SORTMETHOD_DSC_SORT != a_iSortType))
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_ENTRY_INVALID_SORTMETHOD_VALUE);
	}


	iRet = tdr_parse_meta_sortkey_info_i(&stSortKey, a_pstMeta, a_pszSortEntry);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		return iRet;
	}

	pstLib = TDR_META_TO_LIB(a_pstMeta);
	pstSortMeta = TDR_PTR_TO_META(pstLib, stSortKey.ptrSortKeyMeta);
	pstSortEntry = pstSortMeta->stEntries + stSortKey.idxSortEntry;
	if (pstSortEntry->iVersion > a_iVersion)
	{
		return TDR_SUCCESS;
	}
	if (TDR_ENTRY_IS_POINTER_TYPE(pstSortEntry))
	{	/*ָ���Ա������*/
		return TDR_SUCCESS;
	}

	pszHostEnd = a_pstHost->pszBuff + a_pstHost->iBuff;
	pszSortEntryHost = a_pstHost->pszBuff + stSortKey.iSortKeyOff;
	pszSortMetaHost = pszSortEntryHost - pstSortEntry->iHOff;
	if (TDR_ENTRY_IS_REFER_TYPE(pstSortEntry))
	{
		pszSortEntryHost = *(char **)pszSortEntryHost;
	}


	/*ȡ����entry�����������Ϣ*/
	TDR_GET_ARRAY_REAL_COUNT(iArrayRealCount, pstSortEntry, pszSortMetaHost, a_iVersion);
	if ((iArrayRealCount < 0) ||
		((0 < pstSortEntry->iCount) && (pstSortEntry->iCount < iArrayRealCount)))
	{/*ʵ����ĿΪ�������������󳤶�Ҫ������Ч*/
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_REFER_VALUE);
	}
	if (0 >= iArrayRealCount)
	{
		return TDR_SUCCESS;
	}

	/*����������*/
	if ((TDR_TYPE_COMPOSITE < pstSortEntry->iType) && (TDR_TYPE_WSTRING >= pstSortEntry->iType))
	{
		if (1 < iArrayRealCount)
		{
			iRet = tdr_sort_simple_entry_array_i(pstSortEntry, a_iSortType, iArrayRealCount, pszSortEntryHost,
				pszHostEnd);
		}/*if (1 < iArrayRealCount)*/
	}/*if ((TDR_TYPE_COMPOSITE < pstSortEntry->iType) && (TDR_TYPE_STRING >= pstSortEntry->iType))*/

	/*����������������*/
	if (TDR_TYPE_COMPOSITE >= pstSortEntry->iType)
	{
		int i;
		LPTDRMETA pstTypeMeta;
		char *pszMetaHost;
		TDRDATA stData;
		tdr_longlong idxSelectID = TDR_INVALID_INDEX;

		/*�ȶԸ�meta�����������*/
		pszMetaHost = pszSortEntryHost;
		pstTypeMeta = TDR_PTR_TO_META(pstLib, pstSortEntry->ptrMeta);
		stData.iBuff = pstTypeMeta->iHUnitSize;

		if (TDR_TYPE_UNION == pstSortEntry->iType)
		{
			char *pszPtr = pszSortMetaHost + pstSortEntry->stSelector.iHOff;
			TDR_GET_INT(idxSelectID, pstSortEntry->stSelector.iUnitSize, pszPtr);
		}

		for (i = 0; i < iArrayRealCount; i++)
		{
			if (TDR_TYPE_STRUCT == pstTypeMeta->iType)
			{
				stData.pszBuff = pszMetaHost;
				iRet = tdr_sort(pstTypeMeta, &stData, a_iVersion);
			}

			if (TDR_TYPE_UNION == pstTypeMeta->iType)
			{
				LPTDRMETAENTRY pstSubEntry = pstTypeMeta->stEntries + idxSelectID;
				int iCount;
				char *pszSubEntryHost;

				pszSubEntryHost = pszMetaHost + pstSubEntry->iHOff;
				TDR_GET_ARRAY_REAL_COUNT(iCount, pstSubEntry, pszSortMetaHost, a_iVersion);
				if ((pstSubEntry->iVersion <= a_iVersion) && (TDR_SORTMETHOD_NONE_SORT != pstSubEntry->chOrder) &&
					(1 < iCount) && (iCount <= pstSubEntry->iCount))
				{
					iRet = tdr_sort_simple_entry_array_i(pstSubEntry, pstSubEntry->chOrder, iCount,
						pszSubEntryHost, pszHostEnd);
				}
			}/*if (TDR_TYPE_STRUCT == pstTypeMeta->iType)*/

			if (TDR_ERR_IS_ERROR(iRet))
			{
				break;
			}

			pszMetaHost += pstTypeMeta->iHUnitSize;
		}/*for (i = 0; i < iArrayRealCount; i++)*/

		if (1 < iArrayRealCount)
		{
			TDRSORTMETAINFO stMetaInfo;

			stMetaInfo.idxSelectEntry = (int)idxSelectID;
			stMetaInfo.iVersion = a_iVersion;
			stMetaInfo.pstMeta = pstTypeMeta;
			stMetaInfo.chOrder = a_iSortType;
			iRet = tdr_sort_composize_entry_array_i(&stMetaInfo, iArrayRealCount,
				pszSortEntryHost, pszHostEnd);
		}
	}/*if (TDR_TYPE_COMPOSITE >= pstSortEntry->iType)*/

	return iRet;
}

int tdr_normalize(IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstHost, IN int a_iVersion)
{
	int iRet = TDR_SUCCESS;
	LPTDRMETALIB pstLib;
	LPTDRMETA pstCurMeta;
	TDRSTACK  stStack;
	LPTDRSTACKITEM pstStackTop;
	int iStackItemCount;
	int iChange;
	int iCutOffVersion;
	char *pszHostStart;
	char *pszHostEnd;

	/*assert(NULL != a_pstMeta);
	assert(NULL != a_pstHost);
	assert(NULL != a_pstHost->pszBuff);
	assert(0 < a_pstHost->iBuff);*/
	if ((NULL == a_pstMeta) || (NULL == a_pstHost))
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
	}
	if ((NULL == a_pstHost->pszBuff)||(0 >= a_pstHost->iBuff))
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
	}


	if (0 == a_iVersion)
	{
		a_iVersion = TDR_MAX_VERSION;
	}
	if (a_pstMeta->iBaseVersion > a_iVersion)
	{
		a_pstHost->iBuff = 0;
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
	}
	if (TDR_TYPE_UNION == a_pstMeta->iType)
	{
		return TDR_SUCCESS;
	}

	pszHostStart = a_pstHost->pszBuff;
	pszHostEnd = a_pstHost->pszBuff + a_pstHost->iBuff;
	pstLib = TDR_META_TO_LIB(a_pstMeta);

	pstCurMeta = a_pstMeta;
	pstStackTop = &stStack[0];
	pstStackTop->pszHostBase = pszHostStart;
	pstStackTop->pszNetBase = pszHostStart;  /*meta���׵�ַ*/
	pstStackTop->pstMeta = pstCurMeta;
	pstStackTop->iCount = 1;
	pstStackTop->idxEntry = 0;
	pstStackTop->iMetaSizeInfoUnit = 1;			/*count ����ֵ*/
	pstStackTop->pszMetaSizeInfoTarget = NULL;  /*��meta���ϲ�ṹ�ж��ڵ�entryָ��*/
	iCutOffVersion = a_iVersion;
	if (TDR_ERR_IS_ERROR(iRet))
	{
		a_pstHost->iBuff = 0;
		return iRet;
	}
	pstStackTop->iCutOffVersion = iCutOffVersion;
	pstStackTop->iChange = 0;
	iStackItemCount = 1;

	iChange = 0;
	while (0 < iStackItemCount)
	{
		LPTDRMETAENTRY pstEntry;
		int iArrayRealCount ;

		if (0 >= pstStackTop->iCount)
		{
			/*��ǰmeta����Ա�Ѿ�������ϣ����meta���鱾���������*/
			pstEntry = (LPTDRMETAENTRY)pstStackTop->pszMetaSizeInfoTarget;
			if (1 < pstStackTop->iMetaSizeInfoUnit)
			{
				TDRSORTMETAINFO stMetaInfo;

				stMetaInfo.idxSelectEntry = (int)pstStackTop->iEntrySizeInfoOff;
				stMetaInfo.iVersion = pstStackTop->iCutOffVersion;
				stMetaInfo.pstMeta = pstCurMeta;
				stMetaInfo.chOrder = pstEntry->chOrder;
				if (TDR_SORTMETHOD_NONE_SORT != pstEntry->chOrder)
				{
					tdr_sort_composize_entry_array_i(&stMetaInfo, (int)pstStackTop->iMetaSizeInfoUnit,
						pstStackTop->pszNetBase, pszHostEnd);
				}

				iRet = tdr_check_composize_entry_array_unique_i(&stMetaInfo, (int)pstStackTop->iMetaSizeInfoUnit,
					pstStackTop->pszNetBase, pszHostEnd);
				if (TDR_ERR_IS_ERROR(iRet))
				{
					break;
				}
			}

			pstStackTop--;
			iStackItemCount--;
			if (0 < iStackItemCount)
			{
				pstCurMeta = pstStackTop->pstMeta;
				TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			}
			continue;
		}

		pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;
		if (pstEntry->iVersion > pstStackTop->iCutOffVersion)
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}

		/*ָ�벻����*/
		if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry))
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}

		pszHostStart = pstStackTop->pszHostBase + pstEntry->iHOff;
		if (TDR_ENTRY_IS_REFER_TYPE(pstEntry))
		{
			pszHostStart = *(char **)pszHostStart;
		}


		/*ȡ����entry�����������Ϣ*/
		TDR_GET_ARRAY_REAL_COUNT(iArrayRealCount, pstEntry, pstStackTop->pszHostBase, a_iVersion);
		if ((iArrayRealCount < 0) ||
			((0 < pstEntry->iCount) && (pstEntry->iCount < iArrayRealCount)))
		{/*ʵ����ĿΪ�������������󳤶�Ҫ������Ч*/
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_REFER_VALUE);
			break;
		}
		if (0 >= iArrayRealCount)
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}


		if (TDR_TYPE_COMPOSITE >= pstEntry->iType)
		{/*������������*/
			int idxSubEntry;
			LPTDRMETA pstTypeMeta;

			if (0 >= iArrayRealCount)
			{
				TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
				continue;
			}
			if (TDR_STACK_SIZE <=  iStackItemCount)
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_TOO_COMPLIEX_META);
				break;
			}

			if (TDR_TYPE_UNION == pstEntry->iType)
			{
				TDR_GET_UNION_ENTRY_TYPE_META_INFO(pstStackTop->pszHostBase, pstLib, pstEntry, pstStackTop->iCutOffVersion, pstTypeMeta, idxSubEntry);
				if (NULL == pstTypeMeta)
				{
					TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
					continue;
				}
			}else
			{
				pstTypeMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
				idxSubEntry = 0;
			}

			pstCurMeta = pstTypeMeta;
			iStackItemCount++;
			pstStackTop++;
			pstStackTop->iCount = iArrayRealCount;
			pstStackTop->pstMeta = pstCurMeta;
			pstStackTop->idxEntry = idxSubEntry;
			pstStackTop->iMetaSizeInfoUnit = iArrayRealCount;		/*count ����ֵ*/
			pstStackTop->pszMetaSizeInfoTarget = (char *)pstEntry;  /*��meta���ϲ�ṹ�ж��ڵ�entryָ��*/
			pstStackTop->pszHostBase = pszHostStart;
			pstStackTop->pszNetBase = pszHostStart;  /*meta���׵�ַ*/
			pstStackTop->iEntrySizeInfoOff = idxSubEntry;  /*selcet id*/
			iCutOffVersion = a_iVersion;
			if (TDR_ERR_IS_ERROR(iRet))
			{
				break;
			}
			pstStackTop->iCutOffVersion = iCutOffVersion;
			pstStackTop->iChange = 0;
		}else if (TDR_TYPE_WSTRING >= pstEntry->iType)
		{
			if ((1 < iArrayRealCount) && (TDR_SORTMETHOD_NONE_SORT != pstEntry->chOrder))
			{
				iRet = tdr_sort_simple_entry_array_i(pstEntry, pstEntry->chOrder, iArrayRealCount, pszHostStart,
					pszHostEnd);
				if (TDR_ERR_IS_ERROR(iRet))
				{
					break;
				}
			}

			/*����ֵԼ��*/
			if ((1 < iArrayRealCount) && (TDR_ENTRY_IS_UNIQUE(pstEntry)))
			{
				iRet = tdr_check_simple_entry_array_unique_i(pstEntry, iArrayRealCount, pszHostStart);
				if (TDR_ERR_IS_ERROR(iRet))
				{
					break;
				}
			}

			/*�ƶ�pszHostStart���Ա�����Ѵ������ݵ��ֽ���*/
			if (0 < pstEntry->iCustomHUnitSize)
			{
				pszHostStart += pstEntry->iCustomHUnitSize * iArrayRealCount;
			}else
			{
				pszHostStart += pstEntry->iHUnitSize * iArrayRealCount;
			}
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
		}
	}/*while (0 < iStackItemCount)*/

	a_pstHost->iBuff = pszHostStart - a_pstHost->pszBuff;

	return iRet;
}


int tdr_check_simple_entry_array_unique_i(LPTDRMETAENTRY a_pstEntry, int a_iCount, char *a_pszHostStart)
{
	LPCOMPAREFUNC fnComp;
	int iRet = TDR_SUCCESS;
	char *pszValue1Host;
	char *pszValue2Host;
	int i;

	assert(NULL != a_pstEntry);
	assert(NULL != a_pszHostStart);
	assert(0 < a_iCount);


	fnComp = tdr_get_entry_compare_function_i(a_pstEntry->iType, TDR_SORTMETHOD_ASC_SORT);
	if (TDR_SORTMETHOD_NONE_SORT != a_pstEntry->chOrder)
	{
		for (i = 0; i < a_iCount -1; i++)
		{
			pszValue1Host = a_pszHostStart + i * a_pstEntry->iHUnitSize;
			pszValue2Host = pszValue1Host + a_pstEntry->iHUnitSize;
			if (0 == fnComp(pszValue1Host, pszValue2Host))
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_BREACH_KEY_RESTRICTION);
				break;
			}
		}/*for (i = 0; i < a_iCount -1; i++)*/
	}else
	{
		/*û�����������,��ȡð�ݵķ������м��*/
		for (i = 0; i < a_iCount - 1; i++)
		{
			int j;
			pszValue1Host = a_pszHostStart + i * a_pstEntry->iHUnitSize;
			for (j = i + 1; j < a_iCount; j++)
			{
				pszValue2Host = pszValue1Host + j * a_pstEntry->iHUnitSize;
				if (0 == fnComp(pszValue1Host, pszValue2Host))
				{
					iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_BREACH_KEY_RESTRICTION);
					break;
				}
			}/*for (j = i + 1; j < a_iCount; j++)*/
			if (TDR_SUCCESS != iRet)
			{
				break;
			}
		}/*for (i = 0; i < a_iCount - 1; i++)*/
	}

	return iRet;
}

int tdr_check_composize_entry_array_unique_i(LPTDRSORTMETAINFO a_pstMetaInfo, int a_iCount,char *a_pszHostStart, char *a_pszHostEnd)
{
	LPTDRMETA pstMeta;
	LPTDRMETAENTRY pstEntry;
	int iRet = TDR_SUCCESS;

	assert(NULL != a_pstMetaInfo);
	assert(NULL != a_pszHostStart);
	assert(NULL != a_pszHostEnd);
	assert(NULL != a_pstMetaInfo->pstMeta);


	pstMeta = a_pstMetaInfo->pstMeta;
	if ((a_pszHostStart + a_iCount * pstMeta->iHUnitSize) > a_pszHostEnd)
	{
		return TDR_SUCCESS;
	}

	if ((TDR_TYPE_UNION == pstMeta->iType) && (TDR_INVALID_INDEX != a_pstMetaInfo->idxSelectEntry))
	{
		pstEntry = pstMeta->stEntries + a_pstMetaInfo->idxSelectEntry;
		if (TDR_ENTRY_IS_UNIQUE(pstEntry))
		{
			iRet = tdr_check_meta_entry_unique_i(pstMeta, a_iCount, pstEntry, a_pszHostStart);
		}/*if (TDR_ENTRY_IS_UNIQUE(pstEntry)*/
	}else if (TDR_TYPE_STRUCT == pstMeta->iType)
	{
		int i;

		for (i = 0; i < pstMeta->iEntriesNum; i++)
		{
			pstEntry = pstMeta->stEntries + i;
			if (TDR_ENTRY_IS_UNIQUE(pstEntry))
			{
				iRet = tdr_check_meta_entry_unique_i(pstMeta, a_iCount, pstEntry, a_pszHostStart);
				if (TDR_SUCCESS != iRet)
				{
					break;
				}
			}
		}/*for (i = 0; i < pstMeta->iEntriesNum; i++)*/

	}/*if ((TDR_TYPE_UNION == pstMeta->iType) && (TDR_INVALID_INDEX != a_pstMetaInfo->idxSelectEntry))*/

	return iRet;
}

int tdr_check_meta_entry_unique_i(LPTDRMETA a_pstMeta, int a_iCount, LPTDRMETAENTRY a_pstEntry, char *a_pszHostStart)
{
	int i;
	int iRet = TDR_SUCCESS;
	char *pszValue1Host;
	char *pszValue2Host;

	assert(NULL != a_pstMeta);
	assert(NULL != a_pstEntry);
	assert(NULL != a_pszHostStart);

	/*û�����������,��ȡð�ݵķ������м��*/
	for (i = 0; i < a_iCount - 1; i++)
	{
		int j;
		pszValue1Host = a_pszHostStart + i * a_pstMeta->iHUnitSize + a_pstEntry->iHOff;
		for (j = i + 1; j < a_iCount; j++)
		{
			pszValue2Host = pszValue1Host + j * a_pstMeta->iHUnitSize + a_pstEntry->iHOff;
			if (0 == memcmp(pszValue1Host, pszValue2Host, a_pstEntry->iHUnitSize))
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_BREACH_KEY_RESTRICTION);
				break;
			}
		}/*for (j = i + 1; j < a_iCount; j++)*/
		if (TDR_SUCCESS != iRet)
		{
			break;
		}
	}/*for (i = 0; i < a_iCount - 1; i++)*/

	return iRet;
}
