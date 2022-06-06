/**
*
* @file     tdr_data_io.c
* @brief    TDR元数据库IO操作模块
*
* @author steve jackyai
* @version 1.0
* @date 2007-03-26
*
*
* Copyright (c)  2007, 腾讯科技有限公司互动娱乐研发部
* All rights reserved.
*
*/
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "tdr_os.h"
//#include "scew/scew.h"
#include "tdr/tdr_define.h"
#include "tdr/tdr_data_io.h"
#include "tdr/tdr_error.h"
#include "tdr/tdr_metalib_init.h"
#include "tdr_metalib_kernel_i.h"
#include "tdr_ctypes_info_i.h"
#include "tdr_define_i.h"
#include "tdr_iostream_i.h"
#include "tdr/tdr_XMLtags.h"
#include "tdr_auxtools.h"
#include "tdr_md5.h"
#include "tdr_metalib_manage_i.h"
#include "pal/ttypes.h"


#if defined (_WIN32) || defined (_WIN64)
#pragma warning(disable:4996)
#endif

#define CHECK_AND_RETURN(iRet)\
if (TDR_ERR_IS_ERROR((iRet)))\
{\
    return iRet;\
}

#define CHECK_AND_BREAK(iRet)\
if (TDR_ERR_IS_ERROR((iRet)))\
{\
    break;\
}


char g_szEncoding[128]="GBK";


static void tdr_dump_entry_defaultval_i(FILE *a_fp, LPTDRMETAENTRY a_pstEntry, LPTDRMETALIB a_pstLib);



static int tdr_ioprintf_i(IN LPTDRMETA a_pstMeta, IN LPTDRIOSTREAM a_pstIOStream,
                          INOUT LPTDRDATA a_pstHost, IN int a_iCutOffVersion,
                          IN LPTDRPRINTFORMAT a_pstFormat);


static void tdr_dump_metalib_macrosgroup_i(IN LPTDRMETALIB a_pstLib, IN FILE *a_fp);

static void tdr_dump_id_index_mapping_i(IN FILE *a_fp, IN LPTDRMETALIB a_pstLib);

///////////////////////////////////////////////////////////////////////////////////////////////////
void tdr_dump_metalib(IN LPTDRMETALIB a_pstLib, IN FILE* a_fp)
{
	LPTDRMACRO pstMacro = NULL;
	int i;
	int j;
    int k;
	LPTDRMETA pstMeta = NULL;
	LPTDRMETAENTRY pstEntry = NULL;
	LPTDRNAMEENTRY pstNameMappingEntry;

	//assert(NULL != a_pstLib);
	//assert(NULL != a_fp);
	if ((NULL == a_pstLib)||(NULL == a_fp))
	{
		return ;
	}


	fprintf(a_fp, "\nMetalib(\"%s\"): Magic: %x Build: %d PlatArch=%u ID=%d ver=%d Metas %d/%d Macros:%d/%d \n", a_pstLib->szName,
		a_pstLib->wMagic, a_pstLib->nBuild, a_pstLib->dwPlatformArch, a_pstLib->iID, a_pstLib->iVersion, a_pstLib->iCurMetaNum, a_pstLib->iMaxMetaNum,
		a_pstLib->iCurMacroNum, a_pstLib->iMaxMacroNum);

    fprintf(a_fp, "\t StringBuf<BeginPtr= %"TDRPRI_INTPTRT" EndPtr= %"TDRPRI_INTPTRT" FreeSize=%"TDRPRI_SIZET"  defaultalign:%d\n\n",
        a_pstLib->ptrStrBuf, a_pstLib->ptrFreeStrBuf, a_pstLib->iFreeStrBufSize, TDR_DEFAULT_ALIGN_VALUE);

    if (TDR_BUILD != a_pstLib->nBuild)
    {
        return ;
    }

	/*macro*/
	pstMacro = TDR_GET_MACRO_TABLE(a_pstLib);
	for(i=0; i < a_pstLib->iCurMacroNum; i++)
	{
        fprintf(a_fp, "Macro Name ptr:%"TDRPRI_INTPTRT" Name=\"%s\" id=%d Desciption: %s\n",
            pstMacro[i].ptrMacro,
            TDR_GET_STRING_BY_PTR(a_pstLib,pstMacro[i].ptrMacro), pstMacro[i].iValue,
			((TDR_INVALID_PTR != pstMacro[i].ptrDesc)?TDR_GET_STRING_BY_PTR(a_pstLib, pstMacro[i].ptrDesc):""));
	}

	/*macrosgroup*/
	tdr_dump_metalib_macrosgroup_i(a_pstLib, a_fp);

	/*meta*/
	for(i=0; i< a_pstLib->iCurMetaNum; i++)
	{
		pstMeta	= TDR_IDX_TO_META(a_pstLib, i);

        fprintf(a_fp, "\nMeta(name ptr:%"TDRPRI_INTPTRT" name=\"%s\" ) memsize<%"TDRPRI_SIZET"> version<idx:%d, %d, CurVer:%d> ID<idx:%d, %d> cnamePtr: %"TDRPRI_INTPTRT
            " descPtr: %s\n",
			pstMeta->ptrName,TDR_GET_STRING_BY_PTR(a_pstLib,pstMeta->ptrName), pstMeta->iMemSize,
			pstMeta->idxVersion, pstMeta->iBaseVersion, pstMeta->iCurVersion,
			pstMeta->idxID, pstMeta->iID, pstMeta->ptrChineseName,
			((TDR_INVALID_PTR != pstMeta->ptrDesc)?TDR_GET_STRING_BY_PTR(a_pstLib, pstMeta->ptrDesc):""));

		fprintf(a_fp, "\t\t iCustomAlign: %d iValidAign:%d  EntriesNum: %d Type<idx: %d %d> UnitSize<HSize: %"TDRPRI_SIZET" NSize: %"TDRPRI_SIZET"> Flag: %08x\n",
			pstMeta->iCustomAlign, pstMeta->iValidAlign,
            pstMeta->iEntriesNum, pstMeta->idxType, pstMeta->iType,
			pstMeta->iHUnitSize, pstMeta->iNUnitSize,
			pstMeta->uFlags);

		fprintf(a_fp, "\t\t idxSizeType: %d SizeType<NOff: %"TDRPRI_INTPTRT
            " HOff: %"TDRPRI_INTPTRT" Unit: %"TDRPRI_SIZET"> ptr: %"TDRPRI_INTPTRT"\n",
			pstMeta->stSizeType.idxSizeType, pstMeta->stSizeType.iNOff, pstMeta->stSizeType.iHOff, pstMeta->stSizeType.iUnitSize,
			pstMeta->ptrMeta);

		fprintf(a_fp, "\t\t sortKey<Off: %"TDRPRI_INTPTRT" ptrMeta: %"TDRPRI_INTPTRT" idxEntry: %d> \n",
			pstMeta->stSortKey.iSortKeyOff, pstMeta->stSortKey.ptrSortKeyMeta, pstMeta->stSortKey.idxSortEntry);

		if (TDR_TYPE_STRUCT == pstMeta->iType)
		{
            fprintf(a_fp, "\t\t VersionIndicator<NOff: %"TDRPRI_INTPTRT" HOff: %"TDRPRI_INTPTRT
                " Unit: %"TDRPRI_SIZET"> Size<idx: %d"" %"TDRPRI_SIZET">\n",
				pstMeta->stVersionIndicator.iNOff, pstMeta->stVersionIndicator.iHOff, pstMeta->stVersionIndicator.iUnitSize,
				pstMeta->idxCustomHUnitSize, pstMeta->iCustomHUnitSize);
			fprintf(a_fp, "\t\t uniqueentryname: %s\n",
				((TDR_META_DO_NEED_PREFIX(pstMeta))?TDR_TAG_FALSE:TDR_TAG_TRUE));
		}

		if (0 < pstMeta->nPrimayKeyMemberNum)
		{
			LPTDRDBKEYINFO pstDBKey = TDR_GET_PRIMARYBASEPTR(pstMeta);
			fprintf(a_fp, "\t\t primarykey: %d enties: \n", pstMeta->nPrimayKeyMemberNum);
			for (j = 0; j < pstMeta->nPrimayKeyMemberNum; j++)
			{
				pstEntry = TDR_PTR_TO_ENTRY(a_pstLib, pstDBKey->ptrEntry);
				fprintf(a_fp, "\t\t\t <ptr：%"TDRPRI_INTPTRT" name: %s, HOff : %"TDRPRI_INTPTRT">",
                    pstDBKey->ptrEntry, TDR_GET_STRING_BY_PTR(a_pstLib,pstEntry->ptrName), pstDBKey->iHOff);
				pstDBKey++;
			}
			fprintf(a_fp, "\n");
		}
		fprintf(a_fp, "\t\t splittablefactor: %d splittablekey<HOff: %"TDRPRI_INTPTRT", ptr: %"TDRPRI_INTPTRT"> rule: %d\n",
            pstMeta->iSplitTableFactor,
				pstMeta->stSplitTableKey.iHOff, pstMeta->stSplitTableKey.ptrEntry, pstMeta->nSplitTableRuleID);


		if (TDR_INVALID_PTR != pstMeta->ptrDependonStruct)
		{
			LPTDRMETA pstTemp = TDR_PTR_TO_META(a_pstLib, pstMeta->ptrDependonStruct);
			fprintf(a_fp, "\t\t dependonstruct: <%"TDRPRI_INTPTRT" %s>\n", pstMeta->ptrDependonStruct,
                TDR_GET_STRING_BY_PTR(a_pstLib,pstTemp->ptrName));
		}


		for(j=0; j < pstMeta->iEntriesNum; j++)
		{
			pstEntry	=	pstMeta->stEntries + j;

            fprintf(a_fp, "\tEntry:\t Name ptr:%"TDRPRI_INTPTRT" Name=\"%s\"  ID<idx:%d %d> type<idx: %d %d> version<idx:%d %d> iOffToMeta%"TDRPRI_INTPTRT"\n",
				pstEntry->ptrName,TDR_GET_STRING_BY_PTR(a_pstLib,pstEntry->ptrName), pstEntry->idxID, pstEntry->iID,
				pstEntry->idxType,pstEntry->iType,
				pstEntry->idxVersion, pstEntry->iVersion, pstEntry->iOffToMeta);

			fprintf(a_fp, "\t\t RealSize<HSize: %"TDRPRI_SIZET" NSize: %"TDRPRI_SIZET
                "> CustomUnitSize: %d"" Unit<HSize: %"TDRPRI_SIZET" NSize: %"TDRPRI_SIZET"> count<idx: %d %d>\n",
				pstEntry->iHRealSize, pstEntry->iNRealSize,
				pstEntry->idxCustomHUnitSize, pstEntry->iHUnitSize,
				pstEntry->iNUnitSize, pstEntry->idxCount, pstEntry->iCount);

			fprintf(a_fp, "\t\t HOff: %"TDRPRI_PTRDIFFT" NOff: %"TDRPRI_PTRDIFFT" wFlag: 0x%x chDBFlag：0x%x chOrder: %x ptrmacrosgroup: %"TDRPRI_INTPTRT"\n",
				pstEntry->iHOff, pstEntry->iNOff, pstEntry->wFlag, pstEntry->chDBFlag,
				pstEntry->chOrder, pstEntry->ptrMacrosGroup);

			fprintf(a_fp, "\t\t Refer<ptrEntry: %"TDRPRI_INTPTRT" HOff: %"TDRPRI_INTPTRT
                " Unit: %"TDRPRI_SIZET"> SizeInfo<idxType: %d Noff: %"TDRPRI_INTPTRT" HOff: %"TDRPRI_INTPTRT" Unit: %"TDRPRI_SIZET">\n",
				pstEntry->stRefer.ptrEntry, pstEntry->stRefer.iHOff, pstEntry->stRefer.iUnitSize,
				pstEntry->stSizeInfo.idxSizeType, pstEntry->stSizeInfo.iNOff, pstEntry->stSizeInfo.iHOff, pstEntry->stSizeInfo.iUnitSize);

			fprintf(a_fp, "\t\t IO<idx: %d %d> ptrMeta: %"TDRPRI_INTPTRT" cnamePtr: %"TDRPRI_INTPTRT
                " descPtr: %"TDRPRI_INTPTRT" defualt<Ptr: %"TDRPRI_INTPTRT" len %"TDRPRI_SIZET">\n",
				pstEntry->idxIO, pstEntry->iIO, pstEntry->ptrMeta,
				pstEntry->ptrChineseName, pstEntry->ptrDesc,
				pstEntry->ptrDefaultVal, pstEntry->iDefaultValLen);

			if (TDR_TYPE_UNION == pstEntry->iType)
			{
                fprintf(a_fp, "\t\t Select<ptrEntry: %"TDRPRI_INTPTRT" HOff: %"TDRPRI_INTPTRT
                    " Unit: %"TDRPRI_SIZET"> maxID<idx: %d %d> minid<idx: %d %d>\n",
					pstEntry->stSelector.ptrEntry, pstEntry->stSelector.iHOff, pstEntry->stSelector.iUnitSize,
                    pstEntry->iMaxIdIdx,
					pstEntry->iMaxId, pstEntry->iMinIdIdx,pstEntry->iMinId);
			}
			if (TDR_INVALID_PTR != pstEntry->ptrDefaultVal)
			{
				tdr_dump_entry_defaultval_i(a_fp, pstEntry, a_pstLib);
			}
			if (TDR_INVALID_PTR != pstEntry->ptrChineseName)
			{
				fprintf(a_fp, "\t\t chinesename: %s\n", TDR_GET_STRING_BY_PTR(a_pstLib, pstEntry->ptrChineseName));
			}
			if (TDR_INVALID_PTR != pstEntry->ptrDesc)
			{
				fprintf(a_fp, "\t\t desc: %s\n", TDR_GET_STRING_BY_PTR(a_pstLib, pstEntry->ptrDesc));
			}
			if (TDR_INVALID_PTR != pstEntry->ptrCustomAttr)
			{
				fprintf(a_fp, "\t\t customattr: %s\n", TDR_GET_STRING_BY_PTR(a_pstLib, pstEntry->ptrCustomAttr));
			}
		}/*for(j=0; j < pstMeta->iEntriesNum; j++)*/

        for (j = 0; j < pstMeta->iIndexesNum; j++)
        {
            LPTDRMETAINDEX pstIndex = TDR_GET_META_INDEX_PTR(pstMeta) + j;
            fprintf(a_fp, "\tIndex:\t Name ptr: %"TDRPRI_INTPTRT" name: %s, DB table columns: %d\n",
                    pstIndex->ptrName, TDR_GET_STRING_BY_PTR(a_pstLib, pstIndex->ptrName), pstIndex->iColumnNum);
            for (k = 0; k < pstIndex->iColumnNum; k++)
            {
                LPTDRCOLUMNINFO pstColumn = TDR_GET_COLUMN_PTR(pstMeta, pstIndex) + k;
                LPTDRMETAENTRY  pstEntry  = TDR_PTR_TO_ENTRY(a_pstLib, pstColumn->ptrEntry);
                fprintf(a_fp, "\t\t <ptr：%"TDRPRI_INTPTRT" name: %s HOff: %"PRIdPTR"\n",
                        pstColumn->ptrEntry, TDR_GET_STRING_BY_PTR(a_pstLib, pstEntry->ptrName), pstColumn->iHOff);
            }
        }
	}/*for(i=0; i< a_pstLib->iCurMetaNum; i++)*/

	/*ID-index mapping*/
	tdr_dump_id_index_mapping_i(a_fp, a_pstLib);

	/*meta name-index mapping*/
	pstNameMappingEntry = TDR_GET_META_NAME_MAP_TABLE(a_pstLib);
	for(i=0; i< a_pstLib->iCurMetaNum; i++)
	{
		if ((i % 2) == 0)
		{
			fprintf(a_fp, "\n");
		}
        fprintf(a_fp, "<nameptr:%-10"TDRPRI_INTPTRT" name:%-16s ptr:%-10"TDRPRI_INTPTRT"  > ",
            pstNameMappingEntry->ptrName,
            TDR_GET_STRING_BY_PTR(a_pstLib,pstNameMappingEntry->ptrName),
            pstNameMappingEntry->ptrMeta);
		pstNameMappingEntry++;
	}

}

void tdr_dump_metalib_macrosgroup_i(IN LPTDRMETALIB a_pstLib, IN FILE *a_fp)
{
	LPTDRMAPENTRY pstMap;

	int i,j;
	LPTDRMACRO pstMacro;
	TDRIDX *pNameIdxTab;
	TDRIDX *pValueIdxTab;

	assert(NULL != a_pstLib);
	assert(NULL != a_fp);

	pstMap = TDR_GET_MACROSGROUP_MAP_TABLE(a_pstLib);
	pstMacro = TDR_GET_MACRO_TABLE(a_pstLib);
	for (i = 0; i < a_pstLib->iCurMacrosGroupNum; i++)
	{
		LPTDRMACROSGROUP pstTmpGroup = TDR_PTR_TO_MACROSGROUP(a_pstLib, pstMap[i].iPtr);
		fprintf(a_fp, "macrosgroup[%d]: name=%s desc= %s ptr:%"TDRPRI_INTPTRT" size:%"TDRPRI_SIZET" macrosnum:%d >\n ",
			i, pstTmpGroup->szName,
			((TDR_INVALID_PTR != pstTmpGroup->ptrDesc)?TDR_GET_STRING_BY_PTR(a_pstLib, pstTmpGroup->ptrDesc):""),
			pstMap[i].iPtr, pstMap[i].iSize, pstTmpGroup->iCurMacroCount);

		pNameIdxTab = TDR_GET_MACROSGROUP_NAMEIDXMAP_TAB(pstTmpGroup);
		pValueIdxTab = TDR_GET_MACROSGROUP_VALUEIDXMAP_TAB(pstTmpGroup);
		fprintf(a_fp, "name index macro:\n");
		for(j=0; j < pstTmpGroup->iCurMacroCount; j++)
		{
			fprintf(a_fp, "\t idx: %d Macro Name=\"%s\" id=%d \n", pNameIdxTab[j],
				TDR_GET_STRING_BY_PTR(a_pstLib,pstMacro[pNameIdxTab[j]].ptrMacro), pstMacro[pNameIdxTab[j]].iValue);
		}
		fprintf(a_fp, "value index macro:\n");
		for(j=0; j < pstTmpGroup->iCurMacroCount; j++)
		{
			fprintf(a_fp, "\t idx: %d  id=%d Macro Name=\"%s\"\n", pValueIdxTab[j],
				pstMacro[pValueIdxTab[j]].iValue, TDR_GET_STRING_BY_PTR(a_pstLib,pstMacro[pValueIdxTab[j]].ptrMacro));
		}
	}/*for (i = 0; i < a_pstLib->iCurMacrosGroupNum; i++)*/
}

void tdr_dump_id_index_mapping_i(IN FILE *a_fp, IN LPTDRMETALIB a_pstLib)
{
	LPTDRIDENTRY pstIDMappingEntry = NULL;
	int i;

	assert(NULL != a_fp);
	assert(NULL != a_pstLib);

	pstIDMappingEntry = TDR_GET_META_ID_MAP_TABLE(a_pstLib);
	for(i=0; i< a_pstLib->iCurMetaNum; i++)
	{
        LPTDRMETA pstMeta = TDR_PTR_TO_META(a_pstLib, pstIDMappingEntry->ptrMeta);
		if ((i % 8) == 0)
		{
			fprintf(a_fp, "\n");
		}
        fprintf(a_fp, "<meta:%s ID:%d metaptr:%"TDRPRI_INTPTRT"  >",
            TDR_GET_STRING_BY_PTR(a_pstLib, pstMeta->ptrName),
            pstIDMappingEntry->iID,
            pstIDMappingEntry->ptrMeta);
		pstIDMappingEntry++;
	}
}


void tdr_dump_metalib_file(IN LPTDRMETALIB a_pstLib, IN const char* a_pszFile)
{
	FILE *fp;

	//assert(NULL != a_pstLib);
	//assert(NULL != a_pszFile);
	if (NULL == a_pszFile)
	{
		return ;
	}

	fp = fopen(a_pszFile, "w");
	if (NULL != fp)
	{
		tdr_dump_metalib(a_pstLib, fp);
		fclose(fp);
	}
}

TDR_API int tdr_load_verify_metalib(INOUT LPTDRMETALIB* a_ppstLib, IN const char* a_pszBinFile, IN const char *a_pszHash)
{
	int iRet = TDR_SUCCESS;
	unsigned char szBinHash[TDR_MD5_DIGEST_LENGTH];
	char szHash[TDR_MD5_DIGEST_LENGTH * 2 + 1] = {0};

	//assert(NULL != a_ppstLib);
	//assert(NULL != a_pszBinFile);


	iRet = tdr_load_metalib(a_ppstLib, a_pszBinFile);
	if (TDR_ERR_IS_ERROR(iRet) || (NULL == a_pszHash))
	{
		return iRet;
	}

	/*验证hash值*/
	tdr_md5hash_buffer(szBinHash, (const unsigned char *)*a_ppstLib, (uint32_t)(*a_ppstLib)->iSize);
	tdr_md5hash2str(szBinHash, &szHash[0], sizeof(szHash));
	if (strncmp(szHash, a_pszHash, sizeof(szBinHash)) != 0)
	{
		tdr_free_lib(a_ppstLib);
		iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_DIFF_METALIB_HASH);
	}

	return iRet;
}

TDR_API int tdr_verify_metalib(IN LPTDRMETALIB a_pstLib, IN const char *a_pszHash)
{
	int iRet = TDR_SUCCESS;
	unsigned char szBinHash[TDR_MD5_DIGEST_LENGTH];
	char szHash[TDR_MD5_DIGEST_LENGTH * 2 + 1] = {0};

	if (NULL == a_pstLib || NULL == a_pszHash)
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
	}


	/*验证hash值*/
	tdr_md5hash_buffer(szBinHash, (const unsigned char *)a_pstLib, (uint32_t)(a_pstLib)->iSize);
	tdr_md5hash2str(szBinHash, &szHash[0], sizeof(szHash));
	if (strncmp(szHash, a_pszHash, sizeof(szBinHash)) != 0)
	{
		iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_DIFF_METALIB_HASH);
	}

	return iRet;
}

TDR_API int tdr_load_metalib(INOUT LPTDRMETALIB* a_ppstLib, IN const char* a_pszBinFile)
{
	FILE *fp = NULL;
	int iRet = TDR_SUCCESS;

	//assert(NULL != a_ppstLib);
	//assert(NULL != a_pszBinFile);
	if (NULL == a_pszBinFile)
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
	}

	fp = fopen(a_pszBinFile, "rb");
	if (NULL == fp)
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_FAILED_OPEN_FILE_TO_READ);
	}

	iRet = tdr_load_metalib_fp(a_ppstLib, fp);

	fclose(fp);

	return iRet;
}

TDR_API int tdr_load_metalib_fp(INOUT LPTDRMETALIB* a_ppstLib, IN FILE* a_fpBin)
{
	long lLen;
	size_t lSize;
	LPTDRMETALIB pstLib;
	int iRet = TDR_SUCCESS;

	//assert(NULL != a_ppstLib);
	//assert(NULL != a_fpBin);
	if ((NULL == a_ppstLib) || (NULL == a_fpBin))
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
	}

	fseek(a_fpBin, 0, SEEK_END);
	lLen = ftell(a_fpBin);
	if (0 >= lLen)
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_METALIB_FILE);
	}
    if (offsetof(TDRMETALIB, iSize) + sizeof(pstLib->iSize) > lLen)
    {
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_METALIB_FILE);
    }
	rewind(a_fpBin);

	pstLib = (LPTDRMETALIB) calloc(1, (size_t)lLen);
	if (NULL == pstLib)
	{
		return TDR_ERRIMPLE_NO_MEMORY;
	}

	lSize =	fread(pstLib, 1, lLen, a_fpBin);
	if (pstLib->iSize != lSize)
	{
		iRet =  TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_METALIB_FILE);
	}else if( (TDR_MAGIC!=pstLib->wMagic) ||	(TDR_BUILD!=pstLib->nBuild) )
	{
		iRet =  TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_BUILD_VERSION_CONFLICT);
    }else if (TDR_FLATFORM_MAGIC != pstLib->dwPlatformArch)
    {
        iRet =  TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_PLATFORM_CONFLICT);
    }

    if (!(TDR_ERR_IS_ERROR(iRet)))
    {
        if (TDR_CHECKSUM_VERSION <= pstLib->nBuild)
        {
            if (0 != tdr_check_checksum_i(pstLib))
            {
                iRet = TDR_ERRIMPLE_INVALID_METALIB_CHECKSUM;
            }
        }
    }

	if (TDR_ERR_IS_ERROR(iRet))
	{
		tdr_free_lib(&pstLib);
	}else
	{
		*a_ppstLib	=	pstLib;
	}

	return iRet;
}

TDR_API int tdr_load_metalib_buf(INOUT LPTDRMETALIB* a_ppstLib, IN const char* a_pBuf, IN size_t* a_ptLen)
{
	LPTDRMETALIB pstLib;
	int iRet = TDR_SUCCESS;

	if ((NULL == a_ppstLib) || (NULL == a_pBuf) || (NULL == a_ptLen))
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
	}

    pstLib = (LPTDRMETALIB)a_pBuf;

    if (offsetof(TDRMETALIB, iSize) + sizeof(pstLib->iSize) > *a_ptLen)
    {
		iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_SHORT_METALIB_BUF);
    } else if (pstLib->iSize > *a_ptLen)
	{
		iRet =  TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_SHORT_METALIB_BUF);
	}else if((TDR_MAGIC!=pstLib->wMagic) ||	(TDR_BUILD!=pstLib->nBuild))
	{
		iRet =  TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_BUILD_VERSION_CONFLICT);
    }else if (TDR_FLATFORM_MAGIC != pstLib->dwPlatformArch)
    {
        iRet =  TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_PLATFORM_CONFLICT);
    }
    if (TDR_ERR_IS_ERROR(iRet))
    {
        return iRet;
    }

    if (TDR_CHECKSUM_VERSION <= pstLib->nBuild)
    {
        if (0 != tdr_check_checksum_i(pstLib))
        {
            return TDR_ERRIMPLE_INVALID_METALIB_CHECKSUM;
        }
    }

    pstLib = (LPTDRMETALIB) calloc(1, ((LPTDRMETALIB)a_pBuf)->iSize);
    if (NULL == pstLib)
    {
        return TDR_ERRIMPLE_NO_MEMORY;
    }

    memcpy(pstLib, a_pBuf, ((LPTDRMETALIB)a_pBuf)->iSize);

    *a_ppstLib	=	pstLib;
    *a_ptLen = pstLib->iSize;

	return iRet;
}


TDR_API int tdr_save_metalib(IN LPTDRMETALIB a_pstLib, IN const char* a_pszBinFile)
{
	FILE *fp;
	int iRet;

	//assert(NULL != a_pstLib);
	//assert(NULL != a_pszBinFile);
	if (NULL == a_pszBinFile)
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
	}

	fp = fopen(a_pszBinFile, "wb");
	if (NULL == fp)
	{
		return TDR_ERRIMPLE_FAILED_OPEN_FILE_TO_WRITE;
	}

	iRet = tdr_save_metalib_fp(a_pstLib, fp);

	fclose(fp);

	return iRet;
}



TDR_API int tdr_save_metalib_fp(IN LPTDRMETALIB a_pstLib, IN FILE* a_fpBin)
{
	size_t lSize;

	//assert(NULL != a_pstLib);
	//assert(NULL != a_fpBin);
	if ((NULL == a_pstLib) || (NULL == a_fpBin))
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
	}

    if (TDR_CHECKSUM_VERSION <= a_pstLib->nBuild)
    {
        tdr_calc_checksum_i(a_pstLib);
    }

	lSize = fwrite(a_pstLib, 1, a_pstLib->iSize, a_fpBin);
	if (lSize != (size_t)a_pstLib->iSize)
	{
		return TDR_ERRIMPLE_FAILED_TO_WRITE_FILE;
	}

	return TDR_SUCCESS;
}

void tdr_dump_entry_defaultval_i(FILE *a_fp, LPTDRMETAENTRY a_pstEntry, LPTDRMETALIB a_pstLib)
{
	char *pszDefault;
	char *pstHostEnd;
	TDRIOSTREAM stIOStream;

	assert(NULL != a_fp);
	assert(NULL != a_pstEntry);
	assert(NULL != a_pstEntry);

	if (TDR_INVALID_PTR == a_pstEntry->ptrDefaultVal)
	{
		return ;
	}

	if (TDR_ENTRY_IS_REFER_TYPE(a_pstEntry) || TDR_ENTRY_IS_POINTER_TYPE(a_pstEntry))
	{
		return ;
	}

	if ((TDR_TYPE_COMPOSITE >= a_pstEntry->iType) || (TDR_TYPE_WSTRING < a_pstEntry->iType))
	{
		return ;
	}

	stIOStream.emIOStreamType = TDR_IOSTREAM_FILE;
	stIOStream.fpTDRIO = a_fp;

	pszDefault = TDR_GET_STRING_BY_PTR(a_pstLib, a_pstEntry->ptrDefaultVal);
	pstHostEnd = pszDefault + a_pstEntry->iDefaultValLen;
	tdr_iostream_write(&stIOStream, "\t\t defalutval:");
	tdr_ioprintf_basedtype_i(&stIOStream, a_pstLib, a_pstEntry, &pszDefault, pstHostEnd);
	tdr_iostream_write(&stIOStream, "\n");

}

int tdr_get_default_format(INOUT LPTDRPRINTFORMAT a_pstFormat)
{
    int iRet = TDR_SUCCESS;

    if (NULL == a_pstFormat)
    {
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
    }

    a_pstFormat->iNoVarName = 0;
    a_pstFormat->pszSepStr = "\n";
    a_pstFormat->chIndentChar = ' ';

    return iRet;
}

int tdr_fprintf_ex(IN LPTDRMETA a_pstMeta, IN FILE  *a_fp, IN LPTDRDATA a_pstHost,
                   IN int a_iCutOffVersion, IN LPTDRPRINTFORMAT a_pstFormat)
{
	int iRet = TDR_SUCCESS;
	TDRIOSTREAM stIOStream;

	/*assert(NULL != a_pstMeta);
	assert(NULL != a_fp);
	assert(NULL != a_pstHost);
	assert(NULL != a_pstHost->pszBuff);*/
	if ((NULL == a_pstMeta) || (NULL == a_fp)|| (NULL == a_pstHost))
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
	}
	if ((NULL == a_pstHost->pszBuff)||(0 >= a_pstHost->iBuff))
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
	}


	if (0 == a_iCutOffVersion)
	{
		a_iCutOffVersion = a_pstMeta->iCurVersion;;
	}
	if (a_pstMeta->iBaseVersion > a_iCutOffVersion)
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
	}

	stIOStream.emIOStreamType = TDR_IOSTREAM_FILE;
	stIOStream.fpTDRIO = a_fp;

	iRet = tdr_ioprintf_i(a_pstMeta, &stIOStream, a_pstHost, a_iCutOffVersion, a_pstFormat);

	return iRet;
}


TDR_API int tdr_fprintf(IN LPTDRMETA a_pstMeta, IN FILE  *a_fp, IN LPTDRDATA a_pstHost, IN int a_iCutOffVersion)
{
	int iRet = TDR_SUCCESS;
	TDRIOSTREAM stIOStream;
    TDRPRINTFORMAT stFormat;

	/*assert(NULL != a_pstMeta);
	assert(NULL != a_fp);
	assert(NULL != a_pstHost);
	assert(NULL != a_pstHost->pszBuff);*/
	if ((NULL == a_pstMeta) || (NULL == a_fp)|| (NULL == a_pstHost))
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
	}
	if ((NULL == a_pstHost->pszBuff)||(0 >= a_pstHost->iBuff))
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
	}


	if (0 == a_iCutOffVersion)
	{
		a_iCutOffVersion = a_pstMeta->iCurVersion;;
	}
	if (a_pstMeta->iBaseVersion > a_iCutOffVersion)
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
	}

	stIOStream.emIOStreamType = TDR_IOSTREAM_FILE;
	stIOStream.fpTDRIO = a_fp;

    tdr_get_default_format(&stFormat);

	iRet = tdr_ioprintf_i(a_pstMeta, &stIOStream, a_pstHost, a_iCutOffVersion, &stFormat);

	return iRet;
}

TDR_API int tdr_sprintf(IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstOut, INOUT LPTDRDATA a_pstHost, IN int a_iCutOffVersion)
{
	int iRet = TDR_SUCCESS;
	TDRIOSTREAM stIOStream;
    TDRPRINTFORMAT stFormat;

	/*assert(NULL != a_pstMeta);
	assert(NULL != a_pstOut);
	assert(NULL != a_pstOut->pszBuff);
	assert(0 < a_pstOut->iBuff);
	assert(NULL != a_pstHost);
	assert(NULL != a_pstHost->pszBuff);*/
	if ((NULL == a_pstMeta) || (NULL == a_pstOut)|| (NULL == a_pstHost))
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
	}
	if ((NULL == a_pstHost->pszBuff)||(0 >= a_pstHost->iBuff)||
		(NULL == a_pstHost->pszBuff)||(0 >= a_pstHost->iBuff))
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
	}

	if (0 == a_iCutOffVersion)
	{
		a_iCutOffVersion = a_pstMeta->iCurVersion;
	}
	if (a_pstMeta->iBaseVersion > a_iCutOffVersion)
	{
		a_pstOut->iBuff = 0;
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
	}

	stIOStream.emIOStreamType = TDR_IOSTREAM_STRBUF;
	stIOStream.pszTDRIOBuff = a_pstOut->pszBuff;
	stIOStream.iTDRIOBuffLen = a_pstOut->iBuff;

    tdr_get_default_format(&stFormat);

	iRet = tdr_ioprintf_i(a_pstMeta, &stIOStream, a_pstHost, a_iCutOffVersion, &stFormat);
	a_pstOut->iBuff = stIOStream.pszTDRIOBuff - a_pstOut->pszBuff;
	return iRet;
}

TDR_API int tdr_sprintf_ex(IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstOut, IN LPTDRDATA a_pstHost,
                           IN int a_iCutOffVersion, IN LPTDRPRINTFORMAT a_pstFormat)
{
	int iRet = TDR_SUCCESS;
	TDRIOSTREAM stIOStream;

	/*assert(NULL != a_pstMeta);
	assert(NULL != a_pstOut);
	assert(NULL != a_pstOut->pszBuff);
	assert(0 < a_pstOut->iBuff);
	assert(NULL != a_pstHost);
	assert(NULL != a_pstHost->pszBuff);*/
	if ((NULL == a_pstMeta) || (NULL == a_pstOut)|| (NULL == a_pstHost))
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
	}
	if ((NULL == a_pstHost->pszBuff)||(0 >= a_pstHost->iBuff)||
		(NULL == a_pstHost->pszBuff)||(0 >= a_pstHost->iBuff))
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
	}

	if (0 == a_iCutOffVersion)
	{
		a_iCutOffVersion = a_pstMeta->iCurVersion;
	}
	if (a_pstMeta->iBaseVersion > a_iCutOffVersion)
	{
		a_pstOut->iBuff = 0;
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
	}

	stIOStream.emIOStreamType = TDR_IOSTREAM_STRBUF;
	stIOStream.pszTDRIOBuff = a_pstOut->pszBuff;
	stIOStream.iTDRIOBuffLen = a_pstOut->iBuff;

	iRet = tdr_ioprintf_i(a_pstMeta, &stIOStream, a_pstHost, a_iCutOffVersion, a_pstFormat);
	a_pstOut->iBuff = stIOStream.pszTDRIOBuff - a_pstOut->pszBuff;
	return iRet;
}

int tdr_ioprintf_i(IN LPTDRMETA a_pstMeta, IN LPTDRIOSTREAM a_pstIOStream,
                   INOUT LPTDRDATA a_pstHost, IN int a_iCutOffVersion,
                   IN LPTDRPRINTFORMAT a_pstFormat)
{
	int iRet = TDR_SUCCESS;
	LPTDRMETALIB pstLib;
	LPTDRMETA pstCurMeta;
	TDRSTACK  stStack;
	LPTDRSTACKITEM pstStackTop;
	int iStackItemCount;
	int iCutOffVersion;
	char *pszHostStart;
	char *pszHostEnd;
	char szSpace[TDR_STACK_SIZE*TDR_TAB_SIZE+1];
	int i;
	int iChange = 0;


	assert(NULL != a_pstMeta);
	assert(NULL != a_pstIOStream);
	assert(NULL != a_pstHost);
    assert(NULL != a_pstFormat);
	assert(a_pstMeta->iBaseVersion <= a_iCutOffVersion);

    if (NULL == a_pstFormat->pszSepStr)
    {
        a_pstFormat->pszSepStr = "\n";
    }

	pszHostStart = a_pstHost->pszBuff;
	pszHostEnd = a_pstHost->pszBuff + a_pstHost->iBuff;
	pstCurMeta = a_pstMeta;
	pstLib = TDR_META_TO_LIB(a_pstMeta);

	pstStackTop = &stStack[0];
	pstStackTop->pstMeta = pstCurMeta;
	pstStackTop->pszHostBase = pszHostStart;
	pstStackTop->iCount = 1;
	pstStackTop->idxEntry = 0;
	iCutOffVersion = a_iCutOffVersion;
	if (TDR_ERR_IS_ERROR(iRet))
	{
		a_pstHost->iBuff = 0;
		return iRet;
	}
	pstStackTop->iCutOffVersion = iCutOffVersion;
	iStackItemCount = 1;
	pstStackTop->iEntrySizeInfoOff = 1;	/*结构层次*/
	pstStackTop->iChange = 0;
	pstStackTop->pszNetBase = NULL;

    memset(szSpace, a_pstFormat->chIndentChar, sizeof(szSpace));
	szSpace[sizeof(szSpace) -1 ] = '\0';

    if (!a_pstFormat->iNoVarName)
    {
        iRet = tdr_iostream_write(a_pstIOStream, "[%s version=\"%d\"]:",
                                  TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), pstStackTop->iCutOffVersion);
        CHECK_AND_RETURN(iRet);
    }

	while (0 < iStackItemCount)
	{
		LPTDRMETAENTRY pstEntry;
		int iArrayRealCount ;

		if ((0 != iChange) && ((0 < pstStackTop->iCount)))
		{
			szSpace[(pstStackTop->iEntrySizeInfoOff-1)*TDR_TAB_SIZE] = '\0';
            if (!a_pstFormat->iNoVarName)
            {
                iRet = tdr_iostream_write(a_pstIOStream, "%s%s[%s]", a_pstFormat->pszSepStr,
                                          szSpace, pstStackTop->pszNetBase);
                CHECK_AND_BREAK(iRet);
			}
			szSpace[(pstStackTop->iEntrySizeInfoOff-1)*TDR_TAB_SIZE] = a_pstFormat->chIndentChar;
			iChange = 0;
		}
		iChange = 0;

		if (0 >= pstStackTop->iCount)
		{
			/*当前元数据数组已经处理完毕*/
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

		/*取出此entry的数组计数信息*/
		TDR_GET_ARRAY_REAL_COUNT(iArrayRealCount, pstEntry, pstStackTop->pszHostBase,a_iCutOffVersion);
		if ((iArrayRealCount < 0) ||
			((0 < pstEntry->iCount) && (pstEntry->iCount < iArrayRealCount)))
		{/*实际数目为负数或比数组最大长度要大，则无效*/
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_REFER_VALUE);
			break;
		}
		if (0 == iArrayRealCount)
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}


		if (TDR_TYPE_COMPOSITE >= pstEntry->iType)
		{/*复合数据类型*/
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

			szSpace[pstStackTop->iEntrySizeInfoOff*TDR_TAB_SIZE] = '\0';
            if (!a_pstFormat->iNoVarName)
            {
                iRet = tdr_iostream_write(a_pstIOStream, "%s%s[%s]", a_pstFormat->pszSepStr,
                                          szSpace, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName));
                CHECK_AND_BREAK(iRet);
			}
			szSpace[pstStackTop->iEntrySizeInfoOff*TDR_TAB_SIZE] = a_pstFormat->chIndentChar;

			pstCurMeta = pstTypeMeta;
			iStackItemCount++;
			pstStackTop++;
			pstStackTop->pstMeta = pstCurMeta;
			pstStackTop->iCount = iArrayRealCount;
			pstStackTop->idxEntry = idxSubEntry;
			pstStackTop->pszHostBase = pszHostStart;
			iCutOffVersion = a_iCutOffVersion;
			pstStackTop->iCutOffVersion = iCutOffVersion;
			pstStackTop->iEntrySizeInfoOff = (pstStackTop - 1)->iEntrySizeInfoOff + 1;
			pstStackTop->pszNetBase = TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName);
			pstStackTop->iChange = 1;
			continue;
		}

		/*简单数据类型成员输出*/
		switch(pstEntry->iType)
		{
		case TDR_TYPE_STRING:
		case TDR_TYPE_WSTRING:
			{
				for (i = 0; i < iArrayRealCount; i++)
				{
					szSpace[pstStackTop->iEntrySizeInfoOff*TDR_TAB_SIZE] = '\0';
                    if (a_pstFormat->iNoVarName)
                    {
                        iRet = tdr_iostream_write(a_pstIOStream, "%s%s", a_pstFormat->pszSepStr, szSpace);
                    } else
                    {
                        iRet = tdr_iostream_write(a_pstIOStream, "%s%s%s=", a_pstFormat->pszSepStr,
                                                  szSpace, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName));
                    }
                    CHECK_AND_BREAK(iRet);
                    iRet = tdr_ioprintf_basedtype_i(a_pstIOStream, pstLib, pstEntry, &pszHostStart, pszHostEnd);
                    CHECK_AND_BREAK(iRet);
					szSpace[pstStackTop->iEntrySizeInfoOff*TDR_TAB_SIZE] = a_pstFormat->chIndentChar;
				}/*for (i = 0; i < iArrayRealCount; i++)	*/
				break;
			}
		case TDR_TYPE_DATETIME:
			{
				if((pszHostStart + pstEntry->iHUnitSize*iArrayRealCount) > pszHostEnd)
				{
					iRet =	TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);
					break;
				}
				for (i = 0; i < iArrayRealCount; i++)
				{
					szSpace[pstStackTop->iEntrySizeInfoOff*TDR_TAB_SIZE] = '\0';
                    if (a_pstFormat->iNoVarName)
                    {
                        iRet = tdr_iostream_write(a_pstIOStream, "%s%s", a_pstFormat->pszSepStr, szSpace);
                    } else
                    {
                        iRet = tdr_iostream_write(a_pstIOStream, "%s%s%s=", a_pstFormat->pszSepStr,
                                                  szSpace, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName));
                    }
                    CHECK_AND_BREAK(iRet);
					iRet = tdr_ioprintf_basedtype_i(a_pstIOStream, pstLib, pstEntry, &pszHostStart, pszHostEnd);
                    CHECK_AND_BREAK(iRet);
					szSpace[pstStackTop->iEntrySizeInfoOff*TDR_TAB_SIZE] = a_pstFormat->chIndentChar;
				}/*for (i = 0; i < iArrayRealCount; i++)	*/
			}
			break;
		default:
			{
				if((pszHostStart + pstEntry->iHUnitSize*iArrayRealCount) > pszHostEnd)
				{
					iRet =	TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);
					break;
				}
				szSpace[pstStackTop->iEntrySizeInfoOff*TDR_TAB_SIZE] = '\0';
                if (a_pstFormat->iNoVarName)
                {
                    if (1 < iArrayRealCount)
                    {
                        iRet = tdr_iostream_write(a_pstIOStream, "%s%s", a_pstFormat->pszSepStr, szSpace);
                    }
                    else
                    {
                        iRet = tdr_iostream_write(a_pstIOStream, "%s%s", a_pstFormat->pszSepStr, szSpace);
                    }
                } else
                {
                    if (1 < iArrayRealCount)
                    {
                        iRet = tdr_iostream_write(a_pstIOStream, "%s%s%s[%d]=", a_pstFormat->pszSepStr,
                                                  szSpace, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName),
                                                  iArrayRealCount);
                    }
                    else
                    {
                        iRet = tdr_iostream_write(a_pstIOStream, "%s%s%s=", a_pstFormat->pszSepStr,
                                                  szSpace, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName));
                    }
                }
                CHECK_AND_BREAK(iRet);
                szSpace[pstStackTop->iEntrySizeInfoOff*TDR_TAB_SIZE] = a_pstFormat->chIndentChar;
                /* 非最后一个元素打印时，尾部需要一个 ' ' */
                for (i = 0; i < iArrayRealCount - 1; i++)
                {
                    iRet = tdr_ioprintf_basedtype_i(a_pstIOStream, pstLib, pstEntry, &pszHostStart, pszHostEnd);
                    CHECK_AND_BREAK(iRet);
                    iRet = tdr_iostream_write(a_pstIOStream, " ");
                    CHECK_AND_BREAK(iRet);
                }/*for (i = 0; i < iArrayRealCount; i++)	*/
                iRet = tdr_ioprintf_basedtype_i(a_pstIOStream, pstLib, pstEntry, &pszHostStart, pszHostEnd);
                CHECK_AND_BREAK(iRet);
            }
			break;
		}/*switch(pstEntry->iType)*/

        CHECK_AND_BREAK(iRet);

		TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
	}/*while (0 < iStackItemCount)*/



    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_iostream_write(a_pstIOStream, "\n");
    }

	return iRet;
}

void tdr_set_encoding(IN const char *a_pszEncoding)
{
	if ((NULL != a_pszEncoding) && ('\0' != a_pszEncoding))
	{
		TDR_STRNCPY(g_szEncoding, a_pszEncoding, sizeof(g_szEncoding));
	}
}



