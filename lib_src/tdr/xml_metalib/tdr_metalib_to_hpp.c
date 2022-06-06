/**
*
* @file     tdr_define_i.h
* @brief    �ڲ�ʹ�õĺ궨��
*
* @author steve jackyai
* @version 1.0
* @date 2007-04-24
*
*
* Copyright (c)  2007, ��Ѷ�Ƽ����޹�˾���������з���
* All rights reserved.
*
*/

#include <assert.h>
#include <string.h>
#include <errno.h>


#include "tdr/tdr_os.h"
#include "tdr/tdr_error.h"
#include "tdr/tdr_define.h"
#include "tdr/tdr_define_i.h"
#include "tdr/tdr_metalib_kernel_i.h"
#include "tdr/tdr_metalib_to_hpp.h"
#include "tdr/tdr_ctypes_info_i.h"
#include "tdr/tdr_XMLtags.h"
#include "tdr_XMLMetalib_i.h"
#include "tdr/tdr_metalib_manage.h"
#include "tdr_metalib_param_i.h"
#include "tdr/tdr_metalib_manage_i.h"
#include "tdr_metalib_entry_manage_i.h"
#include "tdr/tdr_md5.h"
#include "tdr_metalib_meta_manage_i.h"
#include "tdr_xml_inoutput_i.h"
#include "tdr/tdr_auxtools.h"

#if defined (_WIN32) || defined (_WIN64)
#pragma warning(disable:4996)
#endif

#define TDR_DIRECT_RENAME 1

#define TDR_TAIL_CMP(a_iResult, a_pszLong, a_pszShort)\
{\
    if (strlen(a_pszShort) > strlen(a_pszLong))\
        a_iResult = 1;\
    a_iResult = strcmp(a_pszLong + strlen(a_pszLong) - strlen(a_pszShort), a_pszShort);\
}

/**����entry��c����ͷ�ļ��е�����
*@param[out] a_szHppName ����c�������ֵĻ�������ַ
*@param[in] a_iHppNameSize �������Ĵ�С
*@param[in] a_pstEntry Ҫ�������ֵĳ�Ա��ָ��
*@param[in] a_pstLib Ԫ���ݿ�ָ��
*@param[in] a_stRule ����ͷ�ļ��Ĺ���
*@pre \e a_szHppName ����ΪNULL
*@pre \e a_iHppNameSize ����0
*@pre \e a_pstEntry ����ΪNULL
*@pre \e a_pstLib ����ΪNULL
*@pre \e a_stRule ����ΪNULL
*/
static int tdr_generate_entry_hpp_name(OUT char *a_szHppName, IN int a_iHppNameSize, IN LPTDRMETAENTRY a_pstEntry,
									   IN LPTDRMETALIB a_pstLib, IN LPTDRHPPRULE a_pstRule);
/**���ļ��м���汾��Ϣ
*/
static void tdr_decl_version(FILE* a_fp, LPTDRMETALIB a_pstLib);

static void tdr_decl_metalib_hash(FILE* a_fp, LPTDRMETALIB a_pstLib);

static void tdr_decl_file_head(FILE* a_fp);

static void tdr_decl_mutex_start(FILE* a_fp, const char* a_szName);
static void tdr_decl_mutex_end(FILE* a_fp, const char* a_szName);

static void tdr_decl_comment(FILE* a_fp, const char* a_pszComment, int a_iLine);

static void tdr_decl_line(FILE* a_fp, int a_iLine);

static int tdr_decl_macro(FILE* a_fp, LPTDRMETALIB a_pstLib, LPTDRMACRO a_pstMacro, int a_iWidth);

static void tdr_decl_enum_entry(FILE* a_fp, LPTDRMETALIB a_pstLib, LPTDRMACRO a_pstMacro);

static void tdr_decl_cpp_start(FILE* a_fp);

static void tdr_decl_cpp_end(FILE* a_fp);

static void tdr_decl_type(FILE* a_fp, LPTDRMETA a_pstMeta, int a_iWidth);

static void tdr_decl_pack(FILE* a_fp, int a_iPack);

/**
*���ض�XMLԪ����ת����c����ͷ�ļ�
*��XMLԪ�����������ĺ��union/struct������Ԫ���ݿ��еĲ��֣�����c���Խṹ����Ԫ���ݿ���û�ж�������ݽ�
*����������
*@param[in] a_pstLib: Ԫ���ݿ�ָ��
*@param[in] a_pstTree Ԫ����
*@param[in] a_pszHppFile ͷ�ļ����ļ���
*@param[in] a_pstRule ���������Ĺ�����Ϣָ��
*@param[in] a_fpError ���������Ϣ���ļ����
*@retval <0  ����ʧ�ܣ����ر�ʾ������Ϣ�Ĵ������
*@retval 0   ����ɹ�
*@retval >0  ����ɹ���������ĳЩ�������
*
*@pre \e a_pstLib ����Ϊ NULL
*@pre \e a_pstRule ����Ϊ NULL
*@pre \e a_pstTree ����ΪNULL
*@pre \e a_pszHppFile ����ΪNULL
*/
int tdr_tree_to_hpp_i(IN LPTDRMETALIB a_pstLib, IN scew_tree *a_pstTree,
        IN int a_iTagSetVersion, IN const char* a_pszXmlConf,
        IN const char* a_pszHppFile, IN LPTDRHPPRULE a_pstRule, IN FILE* a_fpError);

/**����a_pstRootΪ���ĺ궨��Ԫ�ط��ǳ�����a_pstLib�е�ת����cͷ�ļ���
*/
static int tdr_tree_macros_to_hpp_i(FILE* a_fp, LPTDRMETALIB a_pstLib, scew_element *a_pstRoot, IN int a_iTagSetVersion);

/**����a_pstRootΪ����includeԪ��ת����cͷ�ļ���
*/
static int tdr_tree_includes_to_hpp_i(FILE* a_fp, scew_element *a_pstRoot, IN const char* a_pszXmlConf, IN int a_iFileNameRule);

/**��ĳ�궨��Ԫ��ת����c����ͷ�ļ��ṹ
*/
static int tdr_macro_element_to_hpp_i(FILE* a_fp, LPTDRMETALIB a_pstLib, scew_element *a_pstMacro);

/**����a_pstRootΪ���ĺ궨��Ԫ�ط��ǳ�����a_pstLib�е�,��cͷ�ļ�������ԭ������
*/
static int tdr_tree_metas_to_hpp_prototype_i(FILE* a_fp, LPTDRMETALIB a_pstLib, scew_element *a_pstRoot, IN int a_iTagSetVersion);

/**����a_pstRootΪ���ĺ궨��Ԫ�ط��ǳ�����a_pstLib�е�,��cͷ�ļ������ɶ���
*/
static int tdr_tree_metas_to_hpp_i(FILE* a_fp, LPTDRMETALIB a_pstLib, scew_element *a_pstRoot, IN int a_iTagSetVersion, LPTDRHPPRULE a_pstRule);

/**��meta�ĵ�idxEntry����Ա�Ķ���ӵ�C����ͷ�ļ���
*@return �ɹ�����0 ��ʧ�ܷ��ش������
*/
static int tdr_entry_to_hpp_i(FILE* a_fp, LPTDRMETA a_pstMeta, int idxEntry, LPTDRHPPRULE a_pstRule);

static void tdr_decl_ctypes(FILE *a_fp);

static TDRBOOLEAN tdr_tree_have_metas_i(LPTDRMETALIB a_pstLib, scew_element *a_pstRoot, IN int a_iTagSetVersion);

static void tdr_add_meta_comment_i(FILE *a_fp, LPTDRMETA a_pstMeta);

static int tdr_decl_all_macros(FILE* a_fp, LPTDRMETALIB a_pstLib);

static int tdr_tree_macros_to_hppV0_i(FILE* a_fp, LPTDRMETALIB a_pstLib, scew_element *a_pstRoot);

////////////////////////////////////////////////////////////////////////////////////////
void tdr_add_meta_comment_i(FILE *a_fp, LPTDRMETA a_pstMeta)
{
	assert(NULL != a_fp);
	assert(NULL != a_pstMeta);

	if (TDR_INVALID_PTR != a_pstMeta->ptrDesc)
	{
		LPTDRMETALIB pstLib = TDR_META_TO_LIB(a_pstMeta);
		fprintf(a_fp, "/* %s */\n", TDR_GET_STRING_BY_PTR(pstLib, a_pstMeta->ptrDesc));
	}
}


int tdr_meta_to_hpp(IN FILE* a_fp, IN LPTDRMETA a_pstMeta,  IN LPTDRHPPRULE a_pstRule)
{
	int iRet = TDR_SUCCESS;
	LPTDRMETALIB pstLib = NULL;
	LPTDRMETAENTRY pstEntry = NULL;
	const char *pszClass = NULL;
	int i = 0;
	TDROFF iCurHOff;
	int iPaddingCount;


	/*assert(NULL != a_fp);
	assert(NULL != a_pstMeta);
	assert(NULL != a_pstRule);*/
	if ((NULL == a_fp)||(NULL == a_pstMeta)||(NULL == a_pstRule))
	{
		return TDR_ERRIMPLE_INVALID_PARAM;
	}

	pstLib = TDR_META_TO_LIB(a_pstMeta);


	if (TDR_TYPE_UNION == a_pstMeta->iType)
	{
		pszClass = TDR_TAG_UNION;
	}else
	{
		pszClass = TDR_TAG_STRUCT;
	}
	tdr_add_meta_comment_i(a_fp, a_pstMeta);
	fprintf(a_fp, "%s tag%s\n", pszClass, TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName));
	fprintf(a_fp, "{\n");

	iCurHOff = 0;
	iPaddingCount = 0;
	for(i=0; i< a_pstMeta->iEntriesNum; i++)
	{
		int iEntryAlign;
		int iEntryValidAlign;

		pstEntry = a_pstMeta->stEntries + i;
		if (TDR_INVALID_PTR != pstEntry->ptrMeta)
		{
			LPTDRMETALIB pstLib = TDR_META_TO_LIB(a_pstMeta);
			LPTDRMETA pstType = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);

			iEntryAlign = pstType->iValidAlign;
		}else
		{
			LPTDRCTYPEINFO pstType = tdr_idx_to_typeinfo(pstEntry->idxType);
			iEntryAlign = pstType->iSize;
		}
		iEntryValidAlign = TDR_MIN(iEntryAlign, a_pstMeta->iCustomAlign);

		if ((0 < iEntryValidAlign ) && (0 != (iCurHOff % iEntryValidAlign)))
		{/*û�ж���,����ֽ�*/
			int iPadding = iEntryValidAlign - (int)(iCurHOff % iEntryValidAlign);

			iPaddingCount++;
			if (1 == iPadding)
			{
				fprintf(a_fp, "    char __chPadding%d__;\n", iPaddingCount);
			}else
			{
				fprintf(a_fp, "    char __szPadding%d__[%d];\n", iPaddingCount, iPadding);
			}
			iCurHOff += iPadding;
		}/*if ((0 < iEntryValidAlign ) && (0 != (iCurHOff % iEntryValidAlign))*/

		/*union �ṹ����ͬ���ĳ�Ա*/
		if ((TDR_TYPE_UNION == a_pstMeta->iType) &&
			(TDR_INVALID_INDEX != tdr_get_entry_by_name_i(&a_pstMeta->stEntries[0], i, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName))))
		{
			continue;
		}

		iRet = tdr_entry_to_hpp_i(a_fp, a_pstMeta, i, a_pstRule);
		if (TDR_ERR_IS_ERROR(iRet))
		{
			break;
		}

		iCurHOff += pstEntry->iHRealSize;
	}/*for(i=0; i< a_pstMeta->iEntriesNum; i++)*/

	/*�ṹ�崦��size���Ժ�������ɶ����*/
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		size_t iPadding = 0;

		/*size���*/
		if ((0 < a_pstMeta->iCustomHUnitSize) && (iCurHOff < (TDROFF)a_pstMeta->iCustomHUnitSize))
		{
			iPadding = a_pstMeta->iCustomHUnitSize - iCurHOff;
			iCurHOff += iPadding;
		}

		/*�ṹ�����*/
		if ((0 < a_pstMeta->iValidAlign ) && (0 != (iCurHOff % a_pstMeta->iValidAlign)))
		{
			/*û�ж���,����ֽ�*/
			iPadding += (a_pstMeta->iValidAlign - (iCurHOff % a_pstMeta->iValidAlign));
		}/*if ((0 < iEntryValidAlign ) && (0 != (iCurHOff % iEntryValidAlign))*/

		if (0 < iPadding)
		{
			iPaddingCount++;
			if (1 == iPadding)
			{
				fprintf(a_fp, "    char __chPadding%d__;\n", iPaddingCount);
			}else
			{
				fprintf(a_fp, "    char __szPadding%d__[%"TDRPRI_SIZET"];\n", iPaddingCount, iPadding);
			}
		}/*if (0 < iPadding)*/

	}/*if (!TDR_ERR_IS_ERROR(iRet))*/

	fprintf(a_fp, "};\n\n");

	return iRet;
}

int tdr_entry_to_hpp_i(FILE* a_fp, LPTDRMETA a_pstMeta, int idxEntry, LPTDRHPPRULE a_pstRule)
{
	int iRet = TDR_SUCCESS;
	char szType[TDR_NAME_LEN] = {0};
	char szBuff[TDR_NAME_LEN + TDR_MACRO_LEN*2 + TDR_MAX_CUSTOM_NAME_PREFIX_LEN + 20];
	char szVersion[TDR_MACRO_LEN + 20] = {0};
	char szID[TDR_MACRO_LEN + 20] = {0};
	char szMacrosGroup[TDR_NAME_LEN*3] = {0};
	char *pszEntryDesc = "";
	LPTDRCTYPEINFO pstTypeInfo = NULL;
	LPTDRMETAENTRY pstEntry = NULL;
	LPTDRMETALIB pstLib;
	LPTDRMACRO pstMacro = NULL;
	char *pszPoint = "";
    int iWidth;

	assert(NULL != a_fp);
	assert(NULL != a_pstRule);
	assert(NULL != a_pstMeta);
	assert((0 <= idxEntry) && (idxEntry < a_pstMeta->iEntriesNum));

	pstEntry = a_pstMeta->stEntries + idxEntry;
	pstLib = TDR_META_TO_LIB(a_pstMeta);
	pstMacro = TDR_GET_MACRO_TABLE(pstLib);

	/*type*/
	pstTypeInfo = tdr_idx_to_typeinfo(pstEntry->idxType);
	if( TDR_INVALID_PTR != pstEntry->ptrMeta )
	{
		LPTDRMETA pstType =	TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
		TDR_STRNCPY(szType, TDR_GET_STRING_BY_PTR(pstLib,pstType->ptrName), sizeof(szType));
		tdr_strupr(szType);
	} else
	{
		TDR_STRNCPY(szType, pstTypeInfo->pszCName, sizeof(szType));
	}

	if (TDR_ENTRY_IS_REFER_TYPE(pstEntry) || TDR_ENTRY_IS_POINTER_TYPE(pstEntry))
	{
		pszPoint = "*";
	}

	/*name*/
	iRet = tdr_generate_entry_hpp_name(szBuff, sizeof(szBuff), pstEntry, pstLib, a_pstRule);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		return iRet;
	}

	/*commonet infomation*/
	if ( (TDR_INVALID_VERSION != pstEntry->iVersion) &&
		(pstEntry->iVersion != a_pstMeta->iBaseVersion ))
	{
		tdr_snprintf(szVersion, sizeof(szVersion)-1, "Ver.%d", pstEntry->iVersion);
	}
	if (TDR_INVALID_ID != pstEntry->idxID )
	{
		tdr_snprintf(szID, sizeof(szID)-1,"%s,", TDR_GET_STRING_BY_PTR(pstLib,pstMacro[pstEntry->idxID].ptrMacro));
	}
	if (TDR_INVALID_PTR != pstEntry->ptrDesc)
	{
		pszEntryDesc = TDR_GET_STRING_BY_PTR(pstLib, pstEntry->ptrDesc);
	}
	if (TDR_INVALID_PTR != pstEntry->ptrMacrosGroup)
	{
		LPTDRMACROSGROUP pstGroup = TDR_PTR_TO_MACROSGROUP(pstLib, pstEntry->ptrMacrosGroup);
		tdr_snprintf(szMacrosGroup, sizeof(szMacrosGroup)-1,"Bind Macrosgroup:%s,", pstGroup->szName);
	}


	/*���ɳ�Ա�������*/
    iWidth = (int)(TDR_MAX_HPP_STRING_WIDTH_LEN - tdr_strnlen(szType, sizeof(szType)) - tdr_strnlen(pszPoint,1));
	if( ('\0' != pszEntryDesc[0]) || ('\0' != szVersion[0]) || ('\0' != szID[0]) || ('\0' != szMacrosGroup[0]) )
	{
		iRet = fprintf(a_fp, "    %s %s%-*s \t/* %s %s %s %s*/\n", szType, pszPoint, iWidth,
			szBuff, szID, szVersion, pszEntryDesc, szMacrosGroup);
	} else
	{
		iRet = fprintf(a_fp, "    %s %s%-*s\n", szType, pszPoint, iWidth, szBuff);
	}

	if (0 > iRet)
	{
		iRet = TDR_ERRIMPLE_FAILED_TO_WRITE_FILE;
	} else
    {
        iRet = 0;
    }

	return iRet;
}

int tdr_generate_entry_hpp_name(OUT char *a_szHppName, IN int a_iHppNameSize, IN LPTDRMETAENTRY a_pstEntry,
								IN LPTDRMETALIB a_pstLib, IN LPTDRHPPRULE a_pstRule)
{
	char szName[TDR_NAME_LEN] = {0};
	char szMacro[TDR_MACRO_LEN] = {0};
	char szArray[TDR_MACRO_LEN + 4] = {0};
	char szSize[TDR_MACRO_LEN + 4] = {0};
	char *pszTypePrefix = "";
	char *pszCustomPrefix = "";
	char *pszPointPrefix = "";
	int iRet = TDR_SUCCESS;

	assert(NULL != a_szHppName);
	assert(0 < a_iHppNameSize);
	assert(NULL != a_pstEntry);
	assert(NULL != a_pstRule);


	/*main part of name*/
	TDR_STRNCPY(szName, TDR_GET_STRING_BY_PTR(a_pstLib,a_pstEntry->ptrName), sizeof(szName));
	//added by layne supporting resource-transfer tools
	if ( TDR_TYPE_INT== a_pstEntry->iType && TDR_ENTRY_IS_STR_COMPRESS(a_pstEntry))
	{
		TDR_STRNCPY(szName+strlen(szName),"ID",sizeof("ID"));
	}
	//end added by layne supporting resource-transfer tools
	

	/*custom prefix*/
	if (TDR_HPPRULE_ADD_CUSTOM_PREFIX & a_pstRule->iRule)
	{
		pszCustomPrefix = a_pstRule->szCustomNamePrefix;
	}

	/*type prefix*/
	if (!(TDR_HPPRULE_NO_TYPE_PREFIX & a_pstRule->iRule))
	{
		LPTDRCTYPEINFO pstTypeInfo = tdr_idx_to_typeinfo(a_pstEntry->idxType);

		if ( 1 == a_pstEntry->iCount )
		{
			pszTypePrefix =	pstTypeInfo->pszSPrefix;
		} else
		{
			pszTypePrefix = pstTypeInfo->pszMPrefix;
		}
		if ( strlen(TDR_GET_STRING_BY_PTR(a_pstLib,a_pstEntry->ptrName)) <= 1 )
		{/*����������ǰ׺*/
			pszTypePrefix = "";
		}

		if (TDR_ENTRY_IS_REFER_TYPE(a_pstEntry) || TDR_ENTRY_IS_POINTER_TYPE(a_pstEntry))
		{
			pszPointPrefix = "p";
		}
	}/*!(TDR_HPPRULE_NO_TYPE_PREFIX & a_pstRule->iRule)*/

	/*�Ƿ�ı��Ա�����ַ���Сд*/
	if (!(TDR_HPPRULE_NO_LOWERCASE_PREFIX & a_pstRule->iRule))
	{
		if ( '\0' != pszTypePrefix[0] || ('\0' != pszPointPrefix[0]))
		{
			szName[0] =	(char )toupper(szName[0]);
		}else
		{
			szName[0]	=	(char)tolower(szName[0]);
		}
	}

	/*suffix array[]*/
	if( TDR_INVALID_INDEX != a_pstEntry->idxCount )
	{
		LPTDRMACRO pstMacro = TDR_GET_MACRO_TABLE(a_pstLib);
		TDR_STRNCPY(szMacro, TDR_GET_STRING_BY_PTR(a_pstLib,pstMacro[a_pstEntry->idxCount].ptrMacro), sizeof(szMacro));
		tdr_strupr(szMacro);
		sprintf(szArray, "[%s]", szMacro);
	}else if( 1 == a_pstEntry->iCount )
	{
		szArray[0] = '\0';
	}else if ( 0 == a_pstEntry->iCount )
	{
		sprintf(szArray, "[1]");
	} else
	{
		sprintf(szArray, "[%d]", a_pstEntry->iCount);
	}

	/*size*/
	if ((TDR_TYPE_STRING == a_pstEntry->iType) || (TDR_TYPE_WSTRING == a_pstEntry->iType))
	{
		if( TDR_INVALID_INDEX != a_pstEntry->idxCustomHUnitSize )
		{
			LPTDRMACRO pstMacro = TDR_GET_MACRO_TABLE(a_pstLib);
			TDR_STRNCPY(szMacro, TDR_GET_STRING_BY_PTR(a_pstLib,pstMacro[a_pstEntry->idxCustomHUnitSize].ptrMacro), sizeof(szMacro));
			tdr_strupr(szMacro);
			sprintf(szSize, "[%s]", szMacro);
		}else if( 0 < a_pstEntry->iCustomHUnitSize )
		{
			LPTDRCTYPEINFO pstTypeInfo = tdr_idx_to_typeinfo(a_pstEntry->idxType);
			sprintf(szSize, "[%"TDRPRI_SIZET"]", a_pstEntry->iCustomHUnitSize/pstTypeInfo->iSize);

		}else if ( 0 == a_pstEntry->iCount )
		{
			szSize[0] = '\0';
		}
	}/*if (TDR_TYPE_STRING == a_pstEntry->iType)*/

	iRet = tdr_snprintf(a_szHppName, a_iHppNameSize -1, "%s%s%s%s%s%s;", pszCustomPrefix, pszPointPrefix, pszTypePrefix, szName, szArray, szSize);
	if ((0 > iRet) || (iRet >= (a_iHppNameSize -1)))
	{
		iRet = TDR_ERRIMPLE_FAILED_TO_WRITE_FILE;
	}else
	{
		a_szHppName[a_iHppNameSize -1]	=	'\0';
	}

	return iRet;
}

int tdr_metalib_to_hpp(IN LPTDRMETALIB a_pstLib, IN const char* a_pszHppFile, IN LPTDRHPPRULE a_pstRule)
{
    int iRet = TDR_SUCCESS;
    FILE *fp = NULL;
    LPTDRMETA pstMeta = NULL;
    char szFileMacro[TDR_MAX_PATH] = {0};
    int iBaseWidth = TDR_MAX_HPP_STRING_WIDTH_LEN;
    int i;

    /*assert(NULL != a_pstLib);
    assert(NULL != a_pszHppFile);
    assert(NULL != a_pstRule);*/
	if ((NULL == a_pstLib)||(NULL == a_pszHppFile)||(NULL == a_pstRule))
	{
		return TDR_ERRIMPLE_INVALID_PARAM;
	}

    fp = fopen(a_pszHppFile, "w");
    if (NULL == fp)
    {
        return TDR_ERRIMPLE_FAILED_OPEN_FILE_TO_WRITE;
    }

	tdr_decl_file_head(fp);

    tdr_os_file_to_macro_i(szFileMacro, sizeof(szFileMacro), a_pszHppFile);

    tdr_decl_mutex_start(fp, szFileMacro);

	tdr_decl_version(fp, a_pstLib);

	tdr_decl_metalib_hash(fp, a_pstLib);

    tdr_decl_comment(fp, "\nUser Defined Macros.", 2);

    /* begin define macros. */
    tdr_decl_all_macros(fp, a_pstLib);


	/*����ṹ*/
	if (0 < a_pstLib->iCurMetaNum)
	{
		tdr_decl_line(fp, 2);

		/* begin define extern c. */
		tdr_decl_cpp_start(fp);

		tdr_decl_comment(fp, "Define c types.", 2);
		tdr_decl_ctypes(fp);

		/* define prototype of struct or union. */
		if (!(TDR_HPPRULE_NO_TYPE_DECLARE & a_pstRule->iRule))
		{
			tdr_decl_comment(fp, "Structs/unions prototype.", 2);
			for(i=0; i< a_pstLib->iCurMetaNum; i++)
			{
				pstMeta	= TDR_IDX_TO_META(a_pstLib, i);
				tdr_decl_type(fp, pstMeta, iBaseWidth);
			}

			tdr_decl_line(fp, 2);
		}/*if (0 == a_pstRule->iNoTypeDecl)*/


		tdr_decl_comment(fp, "Define structs/unions.", 2);

		tdr_decl_pack(fp, TDR_DEFAULT_ALIGN_VALUE);

		/* now define the struct or union. */
		for(i=0; i< a_pstLib->iCurMetaNum; i++)
		{
			pstMeta	=TDR_IDX_TO_META(a_pstLib, i);
			iRet = tdr_meta_to_hpp(fp, pstMeta, a_pstRule);
			if (TDR_ERR_IS_ERROR(iRet))
			{
				break;
			}
			tdr_decl_line(fp, 1);
		}

		tdr_decl_line(fp, 1);

		tdr_decl_pack(fp, 0);

		tdr_decl_cpp_end(fp);

	}/*if (0 < a_pstLib->iCurMetaNum)*/

    tdr_decl_mutex_end(fp, szFileMacro);

    fclose(fp);

    return iRet;
}

void tdr_decl_file_head(FILE* a_fp)
{
	assert(NULL != a_fp);

    fprintf(a_fp, "/********************************************************************\n");
    fprintf(a_fp, "**       This head file is generated by program,                   **\n");
    fprintf(a_fp, "**            Please do not change it directly.                    **\n");
    fprintf(a_fp, "********************************************************************/\n");
    fprintf(a_fp, "\n");

}

void tdr_decl_version(FILE* a_fp, LPTDRMETALIB a_pstLib)
{
	assert(NULL != a_fp);
	assert(NULL != a_pstLib);

    if( a_pstLib->iVersion >0 )
    {
		char szMetalib[TDR_NAME_LEN];

		TDR_STRNCPY(szMetalib, a_pstLib->szName, sizeof(szMetalib));
		tdr_strupr(szMetalib);
        fprintf(a_fp, "\n#ifndef TDR_METALIB_%s_VERSION \n", szMetalib);
		fprintf(a_fp, "#define TDR_METALIB_%s_VERSION \t%d /*version of metalib*/\n", szMetalib, a_pstLib->iVersion);
        fprintf(a_fp, "#endif\n");
    }
}

void tdr_decl_metalib_hash(FILE* a_fp, LPTDRMETALIB a_pstLib)
{
	char szMetalib[TDR_NAME_LEN];
	unsigned char szMetalibHash[TDR_MD5_DIGEST_LENGTH];
	char szHash[TDR_MD5_DIGEST_LENGTH * 2 + 1] = {0};

	assert(NULL != a_fp);
	assert(NULL != a_pstLib);

	tdr_md5hash_buffer(szMetalibHash, (const unsigned char *)a_pstLib, (unsigned int)a_pstLib->iSize);
	tdr_md5hash2str(szMetalibHash, &szHash[0], sizeof(szHash));

	TDR_STRNCPY(szMetalib, a_pstLib->szName, sizeof(szMetalib));
	tdr_strupr(szMetalib);
	fprintf(a_fp, "\n#ifndef TDR_METALIB_%s_HASH \n", szMetalib);
	fprintf(a_fp, "#define TDR_METALIB_%s_HASH \t\"%s\" /*hash of metalib*/\n", szMetalib, szHash);
	fprintf(a_fp, "#endif\n");
}

void tdr_decl_mutex_start(FILE* a_fp, const char* a_szName)
{
    assert(NULL != a_fp);
    assert(NULL != a_szName);

    fprintf(a_fp, "#ifndef %s\n", a_szName);
    fprintf(a_fp, "#define %s\n\n", a_szName);
}

void tdr_decl_mutex_end(FILE* a_fp, const char* a_szName)
{
    assert(NULL != a_fp);
    assert(NULL != a_szName);

    fprintf(a_fp, "\n#endif /* %s */\n", a_szName);
}

void tdr_decl_comment(FILE* a_fp, const char* a_pszComment, int a_iLine)
{
    assert(NULL != a_fp);
    assert(NULL != a_pszComment);

    fprintf(a_fp, "/*   %s   */", a_pszComment);

    tdr_decl_line(a_fp, a_iLine);
}


void tdr_decl_line(FILE* a_fp, int a_iLine)
{
    int i;

    assert(NULL != a_fp);

    for(i=0; i< a_iLine; i++)
    {
        fprintf(a_fp, "\n");
    }
}

int tdr_decl_all_macros(FILE* a_fp, LPTDRMETALIB a_pstLib)
{
	int iRet = TDR_SUCCESS;
	TDRBOOLEAN *pastIsGroupNember = NULL;
	int i,j;
	LPTDRMAPENTRY pstMap;
	LPTDRMACROSGROUP pstGroup;
	TDRIDX *pValueTable;
	LPTDRMACRO pstMacroTable;
	LPTDRMACRO pstMacro;

	assert(NULL != a_pstLib);
	assert(NULL != a_fp);

	if (0 >= a_pstLib->iCurMacroNum)
	{
		return TDR_SUCCESS;
	}

	/*����macro��macrosgroupӳ��*/
	pastIsGroupNember = (TDRBOOLEAN  *)malloc(sizeof(TDRBOOLEAN)*(a_pstLib->iCurMacroNum));
	if (NULL == pastIsGroupNember)
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_MEMORY);
	}
	for (i = 0; i < a_pstLib->iCurMacroNum; i++)
	{
		pastIsGroupNember[i] = TDR_FALSE;
	}
	pstMap = TDR_GET_MACROSGROUP_MAP_TABLE(a_pstLib);
	for (i = 0; i < a_pstLib->iCurMacrosGroupNum; i++)
	{
		pstGroup = TDR_PTR_TO_MACROSGROUP(a_pstLib, pstMap[i].iPtr);
		pValueTable = TDR_GET_MACROSGROUP_VALUEIDXMAP_TAB(pstGroup);
		for (j = 0; j < pstGroup->iCurMacroCount; j++)
		{
			pastIsGroupNember[pValueTable[j]] = TDR_TRUE;
		}
	}/*for (i = 0; i < a_pstLib->iCurMacrosGroupNum; i++)*/

	/*������������κκ궨����ĺ�*/
	pstMacroTable = TDR_GET_MACRO_TABLE(a_pstLib);
	for (i = 0; i < a_pstLib->iCurMacroNum; i++)
	{
		if (TDR_FALSE == pastIsGroupNember[i])
		{
			pstMacro = pstMacroTable + i;
			iRet = tdr_decl_macro(a_fp, a_pstLib, pstMacro, TDR_MAX_HPP_STRING_WIDTH_LEN);
		}
		if (TDR_ERR_IS_ERROR(iRet))
		{
			break;
		}
	}/*for (i = 0; i < a_pstLib->iCurMacroNum; i++)*/

	/*��enum��������궨�����еĺ�*/
	for (i = 0; i < a_pstLib->iCurMacrosGroupNum; i++)
	{
		pstGroup = TDR_PTR_TO_MACROSGROUP(a_pstLib, pstMap[i].iPtr);
		fprintf(a_fp, "\n/*%s	%s*/\n", pstGroup->szName,
			((TDR_INVALID_PTR != pstGroup->ptrDesc)? TDR_GET_STRING_BY_PTR(a_pstLib, pstGroup->ptrDesc):""));
		fprintf(a_fp, "enum tag%s \n{\n", pstGroup->szName);

		/*����궨��*/
		pValueTable = TDR_GET_MACROSGROUP_VALUEIDXMAP_TAB(pstGroup);
		for (j = 0; j < pstGroup->iCurMacroCount; j++)
		{
			pstMacro = pstMacroTable + pValueTable[j];
			tdr_decl_enum_entry(a_fp, a_pstLib, pstMacro);
		}
		fprintf(a_fp, "};\n");
	}/*for (i = 0; i < a_pstLib->iCurMacrosGroupNum; i++)*/

	free(pastIsGroupNember);

	return iRet;
}


int tdr_decl_macro(FILE* a_fp, LPTDRMETALIB a_pstLib, LPTDRMACRO a_pstMacro, int a_iWidth)
{
    char szMacro[TDR_MACRO_LEN];
	int iRet = TDR_SUCCESS;
	int iWrite;

    assert(NULL != a_fp);
    assert(NULL != a_pstMacro);
    assert(0 < a_iWidth);
	assert(NULL != a_pstLib);

    TDR_STRNCPY(szMacro, TDR_GET_STRING_BY_PTR(a_pstLib,a_pstMacro->ptrMacro), sizeof(szMacro));
    tdr_strupr(szMacro);

	if (TDR_INVALID_PTR != a_pstMacro->ptrDesc)
	{
		iWrite = fprintf(a_fp, "#define %-*s \t%d \t/* %s */\n", a_iWidth, szMacro, a_pstMacro->iValue,
			TDR_GET_STRING_BY_PTR(a_pstLib, a_pstMacro->ptrDesc));
	}else
	{
		iWrite = fprintf(a_fp, "#define %-*s \t%d\n", a_iWidth, szMacro, a_pstMacro->iValue);
	}

	if (0 > iWrite)
	{
		iRet = TDR_ERRIMPLE_FAILED_TO_WRITE_FILE;
	}

	return iRet;
}

void tdr_decl_enum_entry(FILE* a_fp, LPTDRMETALIB a_pstLib, LPTDRMACRO a_pstMacro)
{
	char szMacro[TDR_MACRO_LEN];
	char *pszDesc;

	assert(NULL != a_fp);
	assert(NULL != a_pstMacro);
	assert(NULL != a_pstLib);

	TDR_STRNCPY(szMacro, TDR_GET_STRING_BY_PTR(a_pstLib,a_pstMacro->ptrMacro), sizeof(szMacro));
	tdr_strupr(szMacro);

	pszDesc = (TDR_INVALID_PTR != a_pstMacro->ptrDesc)? TDR_GET_STRING_BY_PTR(a_pstLib, a_pstMacro->ptrDesc):"";
	if ('\0' != *pszDesc )
	{
		fprintf(a_fp, "    %s = %d, \t/* %s */\n", szMacro, a_pstMacro->iValue, pszDesc);
	}else
	{
		fprintf(a_fp, "    %s = %d, \n", szMacro, a_pstMacro->iValue);
	}

}



void tdr_decl_cpp_start(FILE* a_fp)
{
    assert(NULL != a_fp);

    fprintf(a_fp, "#ifdef __cplusplus\n");
    fprintf(a_fp, "extern \"C\" {\n");
    fprintf(a_fp, "#endif\n\n");
}

void tdr_decl_cpp_end(FILE* a_fp)
{
    assert(NULL != a_fp);

    fprintf(a_fp, "#ifdef __cplusplus\n");
    fprintf(a_fp, "}\n");
    fprintf(a_fp, "#endif\n\n");
}

void tdr_decl_type(FILE* a_fp, LPTDRMETA a_pstMeta, int a_iWidth)
{
    char szName[TDR_NAME_LEN];
    LPTDRMETALIB pstLib = TDR_META_TO_LIB(a_pstMeta);

    TDR_STRNCPY(szName, TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), sizeof(szName));
    tdr_strupr(szName);

    if ( TDR_TYPE_UNION == a_pstMeta->iType )
    {
        fprintf(a_fp, "%s tag%s;\n", TDR_TAG_UNION,
            TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName));
        fprintf(a_fp, "typedef %s  tag%-*s \t%s;\n",
            TDR_TAG_UNION, a_iWidth, TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), szName);
        fprintf(a_fp, "typedef %s  tag%-*s \t*LP%s;\n",
            TDR_TAG_UNION, a_iWidth, TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), szName);
    }
    else
    {
        fprintf(a_fp, "%s tag%s;\n", TDR_TAG_STRUCT,
            TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName));
        fprintf(a_fp, "typedef %s tag%-*s \t%s;\n",
            TDR_TAG_STRUCT, a_iWidth, TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), szName);
        fprintf(a_fp, "typedef %s tag%-*s \t*LP%s;\n",
            TDR_TAG_STRUCT, a_iWidth, TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), szName);
    }

    fprintf(a_fp, "\n");
}

void tdr_decl_ctypes(FILE *a_fp)
{
    static const char s_szCtypesDes[] = "\
#ifndef TDR_CUSTOM_C_TYPES\n\
#define TDR_CUSTOM_C_TYPES\n\
    #include <stddef.h>\n\
    #include <time.h>\n\
    #include <sys/types.h>\n\n\
    #if !defined(_WIN32) && !defined(_WIN64)\n\
        #include <stdint.h>\n\
        #include <inttypes.h>\n\n\
    #else /*if !defined(_WIN32) && !defined(_WIN64)*/\n\n\
        //The stdint declaras\n\
        typedef  signed char  int8_t;\n\
        typedef  short int16_t;\n\
        typedef  int   int32_t;\n\
        typedef unsigned char  uint8_t;\n\
        typedef unsigned short uint16_t;\n\
        typedef unsigned int   uint32_t;\n\
        #if _MSC_VER >= 1300\n\
            typedef unsigned long long 	uint64_t;\n\
            typedef long long 	int64_t;\n\
        #else /* _MSC_VER */\n\
            typedef unsigned __int64	uint64_t;\n\
            typedef __int64	int64_t;\n\
        #endif /* _MSC_VER */\n\n\
    #endif /*if !defined(_WIN32) && !defined(_WIN64)*/\n\n\n\
    typedef int64_t tdr_longlong;\n\
    typedef uint64_t tdr_ulonglong;\n\
    typedef uint16_t tdr_wchar_t;  /**<Wchar������������*/\n\
    typedef uint32_t tdr_date_t;	/**<data������������*/\n\
    typedef uint32_t tdr_time_t;	/**<time������������*/\n\
    typedef uint64_t tdr_datetime_t; /**<datetime������������*/\n\
    typedef uint32_t tdr_ip_t;  /**<IPv4��������*/\n\
#endif /*TDR_CUSTOM_C_TYPES*/\n";

	fprintf(a_fp, "\n%s\n", s_szCtypesDes);
}


void tdr_decl_pack(FILE* a_fp, int a_iPack)
{
    if( a_iPack>0 )
        fprintf(a_fp, "#pragma pack(%d)\n\n", a_iPack);
    else
        fprintf(a_fp, "#pragma pack()\n\n");
}


int tdr_metalib_to_hpp_spec(IN LPTDRMETALIB a_pstLib, IN const char* a_pszXmlConf, IN int a_iTagSetVersion,
                        IN const char* a_pszHppFile, IN LPTDRHPPRULE a_pstRule, IN FILE* a_fpError)
{
    int iRet = TDR_SUCCESS;
    scew_tree* pstTree = NULL;

    /*assert(NULL != a_pstLib);
    assert(NULL != a_pszXmlConf);
    assert(NULL != a_pszHppFile);
    assert(NULL != a_pstRule);*/
	if ((NULL == a_pstLib)||(NULL == a_pszXmlConf)||(NULL == a_pszHppFile)||(NULL == a_pstRule))
	{
		return TDR_ERRIMPLE_INVALID_PARAM;
	}


    iRet =	tdr_create_XMLParser_tree_byFileName(&pstTree, a_pszXmlConf, a_fpError);
    if( TDR_ERR_IS_ERROR(iRet))
    {
        return iRet;
    }

    iRet = tdr_tree_to_hpp_i(a_pstLib, pstTree, a_iTagSetVersion,
            a_pszXmlConf, a_pszHppFile, a_pstRule, a_fpError);


    /*�ͷ���Դ*/
    scew_tree_free( pstTree );

    return iRet;
}


int tdr_tree_to_hpp_i(IN LPTDRMETALIB a_pstLib, IN scew_tree *a_pstTree,
        IN int a_iTagSetVersion, IN const char* a_pszXmlConf,
        IN const char* a_pszHppFile, IN LPTDRHPPRULE a_pstRule, IN FILE* a_fpError)
{
    int iRet = TDR_SUCCESS;
    char szFileMacro[TDR_MAX_PATH] = {0};
    scew_element *pstRoot = NULL;
    int iTagsVersion = TDR_INVALID_VERSION;
    FILE *fp = NULL;
	scew_attribute *pstAttr = NULL;

    assert(NULL != a_pstLib);
    assert(NULL != a_pstTree);
    assert(NULL != a_pszHppFile);
    assert(NULL != a_pstRule);


    fp = fopen(a_pszHppFile, "w");
    if (( NULL == fp ) && (NULL != a_fpError))
    {
       fprintf(a_fpError, "ERR: Can\'t open file \"%s\" for write, error desc: %s.\n", a_pszHppFile, strerror(errno));
       return TDR_ERRIMPLE_FAILED_OPEN_FILE_TO_WRITE;
    }

    pstRoot = scew_tree_root(a_pstTree);
    if ((NULL == pstRoot) && (NULL != a_fpError))
    {
        fprintf(a_fpError, "\nerror:\t��XMLԪ������û�и�Ԫ��");
        return TDR_ERRIMPLE_NO_XML_ROOT;
    }

	/*��ȡԪ���������汾*/
	tdr_get_metalib_tagsversion_i(&iTagsVersion, pstRoot, a_iTagSetVersion);
	if (((TDR_SUPPORTING_MIN_XMLTAGSET_VERSION > iTagsVersion) || (iTagsVersion > TDR_SUPPORTING_MAX_XMLTAGSET_VERSION)) &&
		(NULL != a_fpError))
	{
		fprintf(a_fpError, "\nerror:\t��֧�ֵ�Ԫ��������XML��ǩ���汾<%d>��Ŀǰ֧�ֵ���С�汾��: %d, ���汾����: %d,",
			iTagsVersion, TDR_SUPPORTING_MIN_XMLTAGSET_VERSION, TDR_SUPPORTING_MAX_XMLTAGSET_VERSION);
		fprintf(a_fpError, "���鴫��İ汾������XML��Ϣ��Ԫ�ص�tagsetversion����\n");
		return TDR_ERRIMPLE_INVALID_TAGSET_VERSION;
	}

	/*�ļ�ͷ*/
    tdr_decl_file_head(fp);

    tdr_os_file_to_macro_i(szFileMacro, sizeof(szFileMacro), a_pszHppFile);
    tdr_decl_mutex_start(fp, szFileMacro);

	pstAttr = scew_attribute_by_name(pstRoot, TDR_TAG_NAME);
	if ((NULL != pstAttr) && (tdr_strnlen(scew_attribute_value(pstAttr), TDR_NAME_LEN) > 0))
	{
		tdr_decl_version(fp, a_pstLib);
		tdr_decl_metalib_hash(fp, a_pstLib);
	}

    /* include ���ļ� */
    tdr_decl_comment(fp, "\n User Define include[s].", 2);
    iRet = tdr_tree_includes_to_hpp_i(fp, pstRoot, a_pszXmlConf, TDR_DIRECT_RENAME);
    if (iRet < 0)
    {
        fprintf(a_fpError, "error: �����ļ�%sʱ�������û������include���ʧ�ܡ�\n", a_pszXmlConf);
        return iRet;
    }

	/*�����*/
    tdr_decl_comment(fp, "\n User Define Macros.", 2);
    iRet = tdr_tree_macros_to_hpp_i(fp, a_pstLib, pstRoot, iTagsVersion);

	if (tdr_tree_have_metas_i(a_pstLib, pstRoot, iTagsVersion))
	{
		tdr_decl_line(fp, 2);
		/* begin define extern c. */
		tdr_decl_cpp_start(fp);

		tdr_decl_comment(fp, "Define c types.", 2);
		tdr_decl_ctypes(fp);

		tdr_decl_line(fp, 2);

		/* define prototype of struct or union. */
		if (!(TDR_HPPRULE_NO_TYPE_DECLARE & a_pstRule->iRule))
		{
			tdr_decl_comment(fp, "Structs/unions prototype.", 2);
			iRet = tdr_tree_metas_to_hpp_prototype_i(fp, a_pstLib, pstRoot, iTagsVersion);
			tdr_decl_line(fp, 2);
		}



		tdr_decl_comment(fp, "Define structs/unions.", 2);

		tdr_decl_pack(fp, TDR_DEFAULT_ALIGN_VALUE);

		/* now define the struct or union. */
		iRet = tdr_tree_metas_to_hpp_i(fp, a_pstLib, pstRoot, iTagsVersion, a_pstRule);

		tdr_decl_line(fp, 1);

		tdr_decl_pack(fp, 0);

		tdr_decl_cpp_end(fp);
	}

    tdr_decl_mutex_end(fp, szFileMacro);

    fclose(fp);

    return iRet;
}

TDRBOOLEAN tdr_tree_have_metas_i(LPTDRMETALIB a_pstLib, scew_element *a_pstRoot, IN int a_iTagSetVersion)
{
	TDRBOOLEAN bHaveMetas = TDR_FALSE;
	scew_element *pstSubItem = NULL;
	const char *pszName = NULL;

	assert(NULL != a_pstLib);
	assert(NULL != a_pstRoot);
	assert((TDR_SUPPORTING_MIN_XMLTAGSET_VERSION <= a_iTagSetVersion) &&
		(a_iTagSetVersion <= TDR_SUPPORTING_MAX_XMLTAGSET_VERSION));

	if (TDR_XML_TAGSET_VERSION_0 == a_iTagSetVersion)
	{
		pstSubItem = scew_element_next(a_pstRoot, NULL);
		while (NULL != pstSubItem)
		{
			if (0 == tdr_stricmp(scew_element_name(pstSubItem), TDR_TAG_TYPE ) )
			{
				scew_attribute *pstAttr = NULL;

				pstAttr = scew_attribute_by_name(pstSubItem, TDR_TAG_NAME);
				if (NULL != pstAttr)
				{
                    char szName[TDR_NAME_LEN];
					pszName = scew_attribute_value(pstAttr);
                    tdr_normalize_string(szName, sizeof(szName), pszName);
					if (NULL != tdr_get_meta_by_name(a_pstLib, szName))
					{
						bHaveMetas = TDR_TRUE;
						break;
					}
				}
			}/*if (0 == tdr_stricmp(scew_element_name(pstSubItem), TDR_TAG_TYPE ) )*/

			pstSubItem = scew_element_next( a_pstRoot, pstSubItem );
		}/*while (NULL != pstSubItem)*/
	}/*if (TDR_XML_TAGSET_VERSION_0 == a_iTagSetVersion)*/


	if (TDR_XML_TAGSET_VERSION_0 < a_iTagSetVersion)
	{
		pstSubItem = scew_element_next(a_pstRoot, NULL);
		while (NULL != pstSubItem)
		{
			const char *pszElementName = scew_element_name(pstSubItem);
			if (0 == tdr_stricmp(pszElementName, TDR_TAG_STRUCT) ||
				(0 == tdr_stricmp(pszElementName, TDR_TAG_UNION)))
			{
				scew_attribute *pstAttr = NULL;

				pstAttr = scew_attribute_by_name(pstSubItem, TDR_TAG_NAME);
				if (NULL != pstAttr)
				{
                    char szName[TDR_NAME_LEN];
                    pszName = scew_attribute_value(pstAttr);
                    tdr_normalize_string(szName, sizeof(szName), pszName);
					if (NULL != tdr_get_meta_by_name(a_pstLib, szName))
					{
						bHaveMetas = TDR_TRUE;
						break;
					}
				}
			}/*if (0 == tdr_stricmp(scew_element_name(pstSubItem), TDR_TAG_TYPE ) )*/

			pstSubItem = scew_element_next( a_pstRoot, pstSubItem );
		}/*while (NULL != pstSubItem)*/
	}/*if (TDR_XML_TAGSET_VERSION_0 == a_iTagSetVersion)*/

	return bHaveMetas;
}

static int tdr_tree_macros_to_hppV0_i(FILE* a_fp, LPTDRMETALIB a_pstLib, scew_element *a_pstRoot)
{
	scew_element *pstSubItem = NULL;
	const char *pszName = NULL;
	int iRet = TDR_SUCCESS;
	scew_element *pstMacro;

	assert(NULL != a_fp);
	assert(NULL != a_pstLib);
	assert(NULL != a_pstRoot);

	pstSubItem = scew_element_next(a_pstRoot, NULL);
	while (NULL != pstSubItem)
	{
		pszName = scew_element_name(pstSubItem);
		if (0!=tdr_stricmp( scew_element_name(pstSubItem), TDR_TAG_MACROS))
		{
			pstSubItem = scew_element_next(a_pstRoot, pstSubItem);
			continue;
		}

		pstMacro = scew_element_next(pstSubItem, NULL);
		while (NULL != pstMacro)
		{
			if (0!=tdr_stricmp(scew_element_name(pstMacro), TDR_TAG_MACRO))
			{
				pstMacro = scew_element_next(pstSubItem, pstMacro);
				continue;
			}

			iRet = tdr_macro_element_to_hpp_i(a_fp, a_pstLib, pstMacro);
			if (TDR_ERR_IS_ERROR(iRet))
			{
				break;
			}
			pstMacro = scew_element_next(pstSubItem, pstMacro);
		}
		if (TDR_ERR_IS_ERROR(iRet))
		{
			break;
		}
		pstSubItem = scew_element_next(a_pstRoot, pstSubItem);
	}/*while (NULL != pstSubItem)*/

	return iRet;
}

int tdr_tree_includes_to_hpp_i(FILE* a_fp, scew_element *a_pstRoot,
        IN const char* a_pszXmlConf, IN int a_iFileNameRule)
{
	int iRet = TDR_SUCCESS;
    scew_attribute* pstAttr;
    const char* pszElemName;
    const char* pszIncludeFile;
#ifndef TDR_MAX_FILE_LEN
#define TDR_MAX_FILE_LEN	    1024
#endif
    char szHFile[TDR_MAX_FILE_LEN] = {0};
	scew_element *pstSubItem = NULL;

	assert(NULL != a_fp);
	assert(NULL != a_pstRoot);

	pstSubItem = NULL;
    while (NULL != (pstSubItem = scew_element_next(a_pstRoot, pstSubItem)))
    {
        int iResult;
        int iWriteLen;
        char* pchFileNameSuffix;

        pszElemName = scew_element_name(pstSubItem);
        if (!pszElemName || strcmp(pszElemName, TDR_TAG_INCLUDE))
        {
            continue;
        }
        pstAttr = scew_attribute_by_name(pstSubItem, TDR_TAG_INCLUDE_FILE);
        if (!pstAttr)
        {
            continue;
        }
        pszIncludeFile = scew_attribute_value(pstAttr);
        if (!pszIncludeFile)
        {
            continue;
        }
        TDR_TAIL_CMP(iResult, a_pszXmlConf, pszIncludeFile);
        if (!iResult)
        {
            continue;
        }

#define TDR_HPP_SUFFIX_LEN 3 /* strlen(".h") + 1 */
        switch (a_iFileNameRule)
        {
            case TDR_DIRECT_RENAME:
                {
                    /* sizeof(szHFile) - TDR_HPP_SUFFIX_LEN guarantee .h can be appended */
                    iWriteLen = tdr_snprintf(szHFile, sizeof(szHFile)-TDR_HPP_SUFFIX_LEN, "%s", pszIncludeFile);
                    if (0 > iWriteLen)
                    {
                        fprintf(stderr, "error: about included file:\n \"%s\"\n"
                                "faild to transer it into .h file.\n", pszIncludeFile);
                        return -1;
                    }
                    if (iWriteLen >= (int)sizeof(szHFile)-TDR_HPP_SUFFIX_LEN)
                    {
                        fprintf(stderr, "error: about included file:\n \"%s\"\n filepath is too long[%d],"
                                " max permitted length is %d.\n", pszIncludeFile, (int)strlen(pszIncludeFile),
                                (int)sizeof(szHFile)-TDR_HPP_SUFFIX_LEN);
                        return -1;
                    }
                    pchFileNameSuffix = (char*)tdr_get_filename_suffix(szHFile, strlen(szHFile));
                    if (pchFileNameSuffix) /* szHFile has suffix */
                    {
                        strcpy(pchFileNameSuffix+1, "h");
                    }else /* szHFile has no suffix */
                    {
                        strcat(szHFile, ".h");
                    }
                }
                break;
            default:
                break;
        }
        fprintf(a_fp, "#include \"%s\"\n", szHFile);
    } /* while ((pstSubItem = scew_element_next(a_pstRoot, pstSubItem))) */
    fprintf(a_fp, "\n");

	return iRet;
}

int tdr_tree_macros_to_hpp_i(FILE* a_fp, LPTDRMETALIB a_pstLib, scew_element *a_pstRoot, IN int a_iTagSetVersion)
{
	scew_element *pstSubItem = NULL;

	int iRet = TDR_SUCCESS;
	scew_element *pstMacro;

	assert(NULL != a_fp);
	assert(NULL != a_pstLib);
	assert(NULL != a_pstRoot);
	assert((TDR_SUPPORTING_MIN_XMLTAGSET_VERSION <= a_iTagSetVersion) &&
		(a_iTagSetVersion <= TDR_SUPPORTING_MAX_XMLTAGSET_VERSION));


	if (TDR_XML_TAGSET_VERSION_0 == a_iTagSetVersion)
	{
		return tdr_tree_macros_to_hppV0_i(a_fp, a_pstLib, a_pstRoot);
	}/*if (TDR_XML_TAGSET_VERSION_0 == a_iTagSetVersion)*/

	/*�°汾Ԫ��������*/
	pstSubItem = scew_element_next(a_pstRoot, NULL);
	while (NULL != pstSubItem)
	{
		if (0 == tdr_stricmp(scew_element_name(pstSubItem), TDR_TAG_MACRO))
		{
			iRet = tdr_macro_element_to_hpp_i(a_fp, a_pstLib, pstSubItem);
			if (TDR_ERR_IS_ERROR(iRet))
			{
				break;
			}
		}/*if (0 == tdr_stricmp(scew_element_name(pstSubItem), TDR_TAG_MACRO))*/

		if (0 == tdr_stricmp(scew_element_name(pstSubItem), TDR_TAG_MACROSGROUP))
		{
			char szName[TDR_NAME_LEN] = {0};

			/*��enum�ĸ�ʽд�궨����ע��*/
			tdr_get_name_attribute_i(szName, sizeof(szName), pstSubItem);
			fprintf(a_fp, "\n/* %s*/\n", szName);
			fprintf(a_fp, "enum tag%s \n{\n", szName);


			pstMacro = scew_element_next(pstSubItem, NULL);
			while (NULL != pstMacro)
			{
				scew_attribute *pstNameAttr;
				const char *pszName;
				int idx;

				if (0!=tdr_stricmp(scew_element_name(pstMacro), TDR_TAG_MACRO))
				{
					pstMacro = scew_element_next(pstSubItem, pstMacro);
					continue;
				}
				pstNameAttr = scew_attribute_by_name(pstMacro, TDR_TAG_NAME);
				if ((NULL == pstNameAttr) )
				{
					pstMacro = scew_element_next(pstSubItem, pstMacro);
					continue;
				}
				pszName = scew_attribute_value(pstNameAttr);
				if (NULL == pszName)
				{
					pstMacro = scew_element_next(pstSubItem, pstMacro);
					continue;
				}

				idx = tdr_get_macro_index_by_name_i(a_pstLib, pszName);
				if( TDR_INVALID_INDEX != idx)
				{
					LPTDRMACRO pstMacroTab = TDR_GET_MACRO_TABLE(a_pstLib);
					tdr_decl_enum_entry(a_fp, a_pstLib, &pstMacroTab[idx]);
				}


				pstMacro = scew_element_next(pstSubItem, pstMacro);
			}
			fprintf(a_fp, "};\n");
			if (TDR_ERR_IS_ERROR(iRet))
			{
				break;
			}
		}/*if (0 == tdr_stricmp(scew_element_name(pstSubItem), TDR_TAG_MACROSGROUP))*/

		pstSubItem = scew_element_next(a_pstRoot, pstSubItem);
	}/*while (NULL != pstMacro)*/


	return iRet;
}

int tdr_macro_element_to_hpp_i(FILE* a_fp, LPTDRMETALIB a_pstLib, scew_element *a_pstMacro)
{
	int iRet = TDR_SUCCESS;
	const char *pszName = NULL;
	int idx = TDR_INVALID_INDEX;
	LPTDRMACRO pstMacro = NULL;
	scew_attribute *pstNameAttr = NULL;

	assert(NULL != a_fp);
	assert(NULL != a_pstLib);
	assert(NULL != a_pstMacro);

	pstNameAttr = scew_attribute_by_name(a_pstMacro, TDR_TAG_NAME);
	if ((NULL == pstNameAttr) )
	{
		return TDR_SUCCESS;
	}

	pszName = scew_attribute_value(pstNameAttr);
	if (NULL == pszName)
	{
		return TDR_SUCCESS;
	}

	idx = tdr_get_macro_index_by_name_i(a_pstLib, pszName);
	if( TDR_INVALID_INDEX != idx)
	{
		pstMacro = TDR_GET_MACRO_TABLE(a_pstLib);
		iRet = tdr_decl_macro(a_fp, a_pstLib, &pstMacro[idx], TDR_MAX_HPP_STRING_WIDTH_LEN);
	}

	return iRet;
}

int tdr_tree_metas_to_hpp_prototype_i(FILE* a_fp, LPTDRMETALIB a_pstLib, scew_element *a_pstRoot, IN int a_iTagSetVersion)
{
	scew_element *pstSubItem = NULL;
	const char *pszName = NULL;
	int iRet = TDR_SUCCESS;

	assert(NULL != a_fp);
	assert(NULL != a_pstLib);
	assert(NULL != a_pstRoot);
	assert((TDR_SUPPORTING_MIN_XMLTAGSET_VERSION <= a_iTagSetVersion) &&
		(a_iTagSetVersion <= TDR_SUPPORTING_MAX_XMLTAGSET_VERSION));


	if (TDR_XML_TAGSET_VERSION_0 == a_iTagSetVersion)
	{
		pstSubItem = scew_element_next(a_pstRoot, NULL);
		while (NULL != pstSubItem)
		{
			if (0 == tdr_stricmp(scew_element_name(pstSubItem), TDR_TAG_TYPE ) )
			{
				LPTDRMETA pstMeta = NULL;
				scew_attribute *pstAttr = NULL;

				pstAttr = scew_attribute_by_name(pstSubItem, TDR_TAG_NAME);
				if (NULL != pstAttr)
				{
                    char szName[TDR_NAME_LEN];
                    pszName = scew_attribute_value(pstAttr);
                    tdr_normalize_string(szName, sizeof(szName), pszName);
					if (NULL != (pstMeta = tdr_get_meta_by_name(a_pstLib, szName)))
					{
						tdr_decl_type(a_fp, pstMeta, TDR_MAX_HPP_STRING_WIDTH_LEN);
					}
				}
			}/*if (0 == tdr_stricmp(scew_element_name(pstSubItem), TDR_TAG_TYPE ) )*/

			pstSubItem = scew_element_next( a_pstRoot, pstSubItem );
		}/*while (NULL != pstSubItem)*/
	}/*if (TDR_XML_TAGSET_VERSION_0 == a_iTagSetVersion)*/


	if (TDR_XML_TAGSET_VERSION_0 < a_iTagSetVersion)
	{
		pstSubItem = scew_element_next(a_pstRoot, NULL);
		while (NULL != pstSubItem)
		{
			const char *pszElementName = scew_element_name(pstSubItem);
			if (0 == tdr_stricmp(pszElementName, TDR_TAG_STRUCT) ||
				(0 == tdr_stricmp(pszElementName, TDR_TAG_UNION)))
			{
				LPTDRMETA pstMeta = NULL;
				scew_attribute *pstAttr = NULL;

				pstAttr = scew_attribute_by_name(pstSubItem, TDR_TAG_NAME);
				if (NULL != pstAttr)
				{
                    char szName[TDR_NAME_LEN];
                    pszName = scew_attribute_value(pstAttr);
                    tdr_normalize_string(szName, sizeof(szName), pszName);
					if (NULL != (pstMeta = tdr_get_meta_by_name(a_pstLib, szName)))
					{
						tdr_decl_type(a_fp, pstMeta, TDR_MAX_HPP_STRING_WIDTH_LEN);
					}
				}
			}/*if (0 == tdr_stricmp(scew_element_name(pstSubItem), TDR_TAG_TYPE ) )*/

			pstSubItem = scew_element_next( a_pstRoot, pstSubItem );
		}/*while (NULL != pstSubItem)*/
	}/*if (TDR_XML_TAGSET_VERSION_0 == a_iTagSetVersion)*/

	return iRet;
}

int tdr_tree_metas_to_hpp_i(FILE* a_fp, LPTDRMETALIB a_pstLib, scew_element *a_pstRoot, IN int a_iTagSetVersion, LPTDRHPPRULE a_pstRule)
{
	scew_element *pstSubItem = NULL;
	const char *pszName = NULL;
	int iRet = TDR_SUCCESS;

	assert(NULL != a_fp);
	assert(NULL != a_pstLib);
	assert(NULL != a_pstRoot);
	assert((TDR_SUPPORTING_MIN_XMLTAGSET_VERSION <= a_iTagSetVersion) &&
		(a_iTagSetVersion <= TDR_SUPPORTING_MAX_XMLTAGSET_VERSION));


	if (TDR_XML_TAGSET_VERSION_0 == a_iTagSetVersion)
	{
		pstSubItem = scew_element_next(a_pstRoot, NULL);
		while (NULL != pstSubItem)
		{
			if (0 == tdr_stricmp(scew_element_name(pstSubItem), TDR_TAG_TYPE ) )
			{
				LPTDRMETA pstMeta = NULL;
				scew_attribute *pstAttr = NULL;

				pstAttr = scew_attribute_by_name(pstSubItem, TDR_TAG_NAME);
				if (NULL != pstAttr)
				{
                    char szName[TDR_NAME_LEN];
                    pszName = scew_attribute_value(pstAttr);
                    tdr_normalize_string(szName, sizeof(szName), pszName);
					if (NULL != (pstMeta = tdr_get_meta_by_name(a_pstLib, szName)))
					{
						tdr_meta_to_hpp(a_fp, pstMeta, a_pstRule);
					}
				}
			}/*if (0 == tdr_stricmp(scew_element_name(pstSubItem), TDR_TAG_TYPE ) )*/

			pstSubItem = scew_element_next( a_pstRoot, pstSubItem );
		}/*while (NULL != pstSubItem)*/
	}/*if (TDR_XML_TAGSET_VERSION_0 == a_iTagSetVersion)*/


	if (TDR_XML_TAGSET_VERSION_0 < a_iTagSetVersion)
	{
		pstSubItem = scew_element_next(a_pstRoot, NULL);
		while (NULL != pstSubItem)
		{
			const char *pszElementName = scew_element_name(pstSubItem);
			if (0 == tdr_stricmp(pszElementName, TDR_TAG_STRUCT) ||
				(0 == tdr_stricmp(pszElementName, TDR_TAG_UNION)))
			{
				LPTDRMETA pstMeta = NULL;
				scew_attribute *pstAttr = NULL;

				pstAttr = scew_attribute_by_name(pstSubItem, TDR_TAG_NAME);
				if (NULL != pstAttr)
				{
                    char szName[TDR_NAME_LEN];
                    pszName = scew_attribute_value(pstAttr);
                    tdr_normalize_string(szName, sizeof(szName), pszName);
					if (NULL != (pstMeta = tdr_get_meta_by_name(a_pstLib, szName)))
					{
						tdr_meta_to_hpp(a_fp, pstMeta, a_pstRule);
					}
				}
			}/*if (0 == tdr_stricmp(scew_element_name(pstSubItem), TDR_TAG_TYPE ) )*/

			pstSubItem = scew_element_next( a_pstRoot, pstSubItem );
		}/*while (NULL != pstSubItem)*/
	}/*if (TDR_XML_TAGSET_VERSION_0 == a_iTagSetVersion)*/

	return iRet;
}

int tdr_metalib_to_cfile(IN LPTDRMETALIB a_pstLib, IN const char* a_pszCFile)
{
	FILE *fp;
	size_t i;
	size_t iLibSize;
	unsigned char *pch;
	char szFileMacro[TDR_MAX_PATH] = {0};

#define TDR_MAX_BYTE_DATE_PRE_LINE	32

	//assert(NULL != a_pstLib);
	//assert(NULL != a_pszCFile);
	if ((NULL == a_pstLib)||(NULL == a_pszCFile))
	{
		return TDR_ERRIMPLE_INVALID_PARAM;
	}

	fp = fopen(a_pszCFile, "w");
	if (NULL == fp)
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_FAILED_OPEN_FILE_TO_WRITE);
	}

	fprintf(fp, "/********************************************************************\n");
	fprintf(fp, "**       This c file is generated by program,                      **\n");
	fprintf(fp, "**            Please do not change it directly.                    **\n");
	fprintf(fp, "********************************************************************/\n");
	fprintf(fp, "\n");

	fprintf(fp, "/*      Metalib Version Number: %d          */\n", a_pstLib->iVersion);
	fprintf(fp, "\n");

	tdr_os_file_to_macro_i(szFileMacro, sizeof(szFileMacro), a_pszCFile);

	tdr_decl_mutex_start(fp, szFileMacro);

	fprintf(fp, "/*      Metalib %s content          */\n", a_pstLib->szName);
	fprintf(fp, "unsigned char g_szMetalib_%s[]={", a_pstLib->szName);

	iLibSize = a_pstLib->iSize - 1;
	pch = (unsigned char*)a_pstLib;
	for (i = 0; i < iLibSize; i++)
	{
		if ((i % TDR_MAX_BYTE_DATE_PRE_LINE) == 0)
		{
			fprintf(fp, "\\\n    ");
		}
		fprintf(fp, "0x%02x, ", *pch);
		pch++;

	}/*for (i = 0; i < iLibSize; i++)*/
	fprintf(fp, "0x%x};\n", *pch);

	tdr_decl_mutex_end(fp, szFileMacro);

	fclose(fp);

	return TDR_SUCCESS;
}
