/**
*
* @file     tdr_sql.c
* @brief    TSF-G-DR DB��ϴ�����
*
* @author steve jackyai
* @version 1.0
* @date 2007-04-28
*
*
* Copyright (c)  2007, ��Ѷ�Ƽ����޹�˾���������з���
* All rights reserved.
*
*/

#include <assert.h>
#include <string.h>

#include "tdr/tdr_sql.h"
#include "tdr_dbdriver_mysql.h"
#include "tdr/tdr_iostream_i.h"
#include "tdr/tdr_error.h"
#include "tdr/tdr_os.h"
#include "tdr/tdr_metalib_kernel_i.h"
#include "tdr/tdr_define_i.h"
#include "tdr/tdr_ctypes_info_i.h"
#include "tdr/tdr_metalib_manage.h"
#include "tdr/tdr_auxtools.h"
#include "tdr_dbms_comm.h"

#if defined (_WIN32) || defined (_WIN64)
#pragma warning(disable:4996)
#endif


/*add by vinsonzuo 20110620, ���ݿ�blob�ֶ�Ԥ���ռ�*/
#define TDR_DBMS_BLOB_BUFF_RESERVE 4096


/************************************************************************/
/* ���º����ӿ���DBMS������ʵ��                                         */
/************************************************************************/
extern int tdr_dbms_escape_string(IN TDRDBHANDLE a_hDBHandle, OUT char *a_pszTo, IN char *a_pszFrom, int a_iLen);
extern int tdr_query(INOUT LPTDRDBRESULTHANDLE a_phDBResult, IN TDRDBHANDLE a_hDBHandle, IN LPTDRDATA a_pstSql);
extern int tdr_query_quick(INOUT TDRDBRESULTHANDLE *a_phDBResult, IN TDRDBHANDLE a_hDBHandle, IN LPTDRDATA a_pstSql);
extern void tdr_close_dbhanlde(IN LPTDRDBHANDLE a_phDBHandle);
extern void tdr_free_dbresult(IN LPTDRDBRESULTHANDLE a_phDBResult);
extern int tdr_dbms_errno(IN TDRDBHANDLE a_hDBHandle);
extern const char *tdr_dbms_error(IN TDRDBHANDLE a_hDBHandle);
extern unsigned long tdr_num_rows(IN TDRDBRESULTHANDLE a_hDBResult);
extern long tdr_affected_rows(IN TDRDBRESULTHANDLE a_hDBResult);




static int tdr_obj2sql_insert_or_replace_i(IN TDRDBHANDLE a_hDBHandle, INOUT LPTDRDATA a_pstSql, INOUT  LPTDRDBOBJECT a_pstObj, IN char* a_pszOper);

static int tdr_create_table_i(IN LPTDRDBMS a_pstDBMS, IN LPTDRMETA a_pstMeta, INOUT LPTDRIOSTREAM a_pstIOStream, IN int a_iVersion);



/**�������ݳ�Ա�����������ݿ�������Ϣ
*/
static int tdr_create_table_columns_i(INOUT LPTDRIOSTREAM a_pstIOStream, IN LPTDRMETA a_pstMeta, IN int a_iVersion);


static void tdr_get_db_string_type_i(OUT char *pszColType, IN size_t a_iSize);


static void tdr_get_entry_defaultvalue_i(OUT char *a_pszDefault, IN LPTDRMETALIB a_pstLib, IN LPTDRMETAENTRY a_pstEntry);

/**�ڽ������������������Ϣ
*/
static int tdr_create_table_primarykey_i(INOUT LPTDRIOSTREAM a_pstIOStream, IN LPTDRMETA a_pstMeta, int a_iVersion);

static int tdr_create_table_uniqueinfo_i(INOUT LPTDRIOSTREAM a_pstIOStream, IN LPTDRMETA a_pstMeta,  int a_iVersion);

static int tdr_create_table_index_i(INOUT LPTDRIOSTREAM a_pstIOStream, IN LPTDRMETA a_pstMeta, int a_iVersion);

/**���ݴ洢�ռ��Сȷ�������BLOB����
*@return ����BLOB�����ַ���
*/
static char *tdr_get_db_blob_type_i(IN size_t a_iSize);

/**����Ԫ���������޸����ݱ�
*/
static int tdr_alter_table_i(IN LPTDRMETA a_pstMeta, INOUT LPTDRIOSTREAM a_pstIOStream, IN int a_iVersion);

/**����Ԫ���������޸����ݱ����
*/
static int tdr_alter_table_columns_i(INOUT LPTDRIOSTREAM a_pstIOStream, IN LPTDRMETA a_pstMeta, IN int a_iVersion);

static int tdr_alter_table_primarykey_i(INOUT LPTDRIOSTREAM a_pstIOStream, IN LPTDRMETA a_pstMeta, int a_iVersion);

static int tdr_alter_table_uniqueinfo_i(INOUT LPTDRIOSTREAM a_pstIOStream, IN LPTDRMETA a_pstMeta, int a_iVersion);

static int tdr_alter_table_indexes_i(INOUT LPTDRIOSTREAM a_pstIOStream, IN LPTDRMETA a_pstMeta, IN int a_iVersion);

/**���ɳ�Ա����������
*/
/*add a_iMetaVer by vinsonzuo 20110620, for computing BLOB type*/
static void tdr_gen_entry_sql_typename(OUT char *a_szTypeBuff, IN int a_iBuffLen, IN LPTDRMETAENTRY a_pstEntry, IN int a_iMetaVer);

/** the auto column  must be defined as a key , so check this column whether is in primary
, if not gen then key for auto column, otherwise return ""
*@note ��Ϊͬһ������ֻ���ܳ���һ��auto increment��Ա,��˲��迼��������������˲��ÿ���
* ����ǰ׺
*/
static   int tdr_gen_autoincrement_key(IN LPTDRMETA a_pstMeta, IN LPTDRMETAENTRY a_pstEntry, char *pszKey, int iKeySize);


int tdr_create_table(IN LPTDRDBMS a_pstDBMS, IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstSql, IN int a_iVersion)
{
	int iRet = TDR_SUCCESS;
	TDRIOSTREAM stIOStream;

	/*assert(NULL != a_pstDBMS);
	assert(NULL != a_pstMeta);
	assert(NULL != a_pstSql);
	assert(NULL != a_pstSql->pszBuff);
	assert(0 < a_pstSql->iBuff);*/
	if ((NULL == a_pstDBMS)||(NULL == a_pstMeta)||(NULL == a_pstSql)||
		(NULL == a_pstSql->pszBuff)||(0 >= a_pstSql->iBuff))
	{
		return TDR_ERRIMPLE_INVALID_PARAM;
	}

	if (0 == a_iVersion)
	{
		a_iVersion = TDR_MAX_VERSION;
	}
	if (a_pstMeta->iBaseVersion > a_iVersion)
	{
		a_pstSql->iBuff = 0;
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
	}

	stIOStream.emIOStreamType = TDR_IOSTREAM_STRBUF;
	stIOStream.pszTDRIOBuff = a_pstSql->pszBuff;
	stIOStream.iTDRIOBuffLen = a_pstSql->iBuff;
	if (0 == tdr_stricmp(a_pstDBMS->szDBMSName, TDR_DBMS_MYSQL))
	{
		iRet = tdr_create_table_i(a_pstDBMS, a_pstMeta, &stIOStream, a_iVersion);
	}else
	{
		iRet =  TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_DB_UNSUPPORTED_DBMS);
	}

	a_pstSql->iBuff = stIOStream.pszTDRIOBuff - a_pstSql->pszBuff;

	return iRet;
}

int tdr_create_table_fp(IN LPTDRDBMS a_pstDBMS, IN LPTDRMETA a_pstMeta, IN FILE *a_fpSql, IN int a_iVersion)
{
	int iRet = TDR_SUCCESS;
	TDRIOSTREAM stIOStream;

	/*assert(NULL != a_pstDBMS);
	assert(NULL != a_pstMeta);
	assert(NULL != a_fpSql);*/
	if ((NULL == a_pstDBMS)||(NULL == a_pstMeta)||(NULL == a_fpSql))
	{
		return TDR_ERRIMPLE_INVALID_PARAM;
	}

	if (0 == a_iVersion)
	{
		a_iVersion = TDR_MAX_VERSION;
	}
	if (a_pstMeta->iBaseVersion > a_iVersion)
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
	}

	stIOStream.emIOStreamType = TDR_IOSTREAM_FILE;
	stIOStream.fpTDRIO = a_fpSql;

	if (0 == tdr_stricmp(a_pstDBMS->szDBMSName, TDR_DBMS_MYSQL))
	{
		iRet = tdr_create_table_i(a_pstDBMS, a_pstMeta, &stIOStream, a_iVersion);
	}else
	{
		iRet =  TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_DB_UNSUPPORTED_DBMS);
	}

	return iRet;
}

int tdr_create_table_file(IN LPTDRDBMS a_pstDBMS, IN LPTDRMETA a_pstMeta, IN const char *a_pszSqlFile, IN int a_iVersion)
{
	int iRet = TDR_SUCCESS;
	FILE *fp;

	/*assert(NULL != a_pszSqlFile);*/
	if (NULL == a_pszSqlFile)
	{
		return TDR_ERRIMPLE_INVALID_PARAM;
	}

	fp = fopen(a_pszSqlFile, "w");
	if (NULL == fp)
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_FAILED_OPEN_FILE_TO_WRITE);
	}
    iRet = tdr_create_table_fp(a_pstDBMS, a_pstMeta, fp, a_iVersion);

	fclose(fp);

	return iRet;
}

int tdr_create_table_i(IN LPTDRDBMS a_pstDBMS, IN LPTDRMETA a_pstMeta, IN LPTDRIOSTREAM a_pstIOStream, IN int a_iVersion)
{
	int iRet = TDR_SUCCESS;
	int i;
    LPTDRMETALIB pstLib;

	assert(NULL != a_pstDBMS);
	assert(NULL != a_pstMeta);
	assert(NULL != a_pstIOStream);
	assert(a_iVersion >= a_pstMeta->iBaseVersion);

    pstLib = TDR_META_TO_LIB(a_pstMeta);
	if (TDR_META_IS_VARIABLE(a_pstMeta))
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_DB_UNSUPPORTED_VARIABLE_META);
	}

	if (0 < a_pstMeta->iSplitTableFactor)
	{
		/*�ֶ����д洢*/
		for (i = 0; i < a_pstMeta->iSplitTableFactor; i++)
		{
			/*���ɱ�ͷ*/
			iRet = tdr_iostream_write(a_pstIOStream, "CREATE TABLE IF NOT EXISTS %s_%d (\n",
				TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), i);
			if (TDR_ERR_IS_ERROR(iRet))
			{
				break;
			}

			/*�������ݿ��ĸ���*/
			iRet = tdr_create_table_columns_i(a_pstIOStream, a_pstMeta, a_iVersion);


			/*����������Ϣ*/
			if (0 < a_pstMeta->nPrimayKeyMemberNum)
			{
				iRet = tdr_create_table_primarykey_i(a_pstIOStream, a_pstMeta, a_iVersion);
			}

			/*UNIQUE ��Ϣ*/
			iRet = tdr_create_table_uniqueinfo_i(a_pstIOStream, a_pstMeta, a_iVersion);

            /*INDEX ��Ϣ*/
            if (TDR_INDEX_VERSION <= pstLib->nBuild)
            {
                if (0 < a_pstMeta->iIndexesNum)
                {
                    iRet = tdr_create_table_index_i(a_pstIOStream, a_pstMeta, a_iVersion);
                }
            }

			iRet = tdr_iostream_write(a_pstIOStream, ")\n");

			/*���ñ�ѡ��*/
			iRet = tdr_set_table_options(a_pstDBMS, a_pstMeta, a_pstIOStream);

			iRet = tdr_iostream_write(a_pstIOStream, ";\n");
		}/*for (i = 1; i <= a_pstMeta->iSplitTableFactor; i++)*/
	}else
	{
		/*���ɱ�ͷ*/
		iRet = tdr_iostream_write(a_pstIOStream, "CREATE TABLE IF NOT EXISTS %s (\n", TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName));

		/*�������ݿ��ĸ���*/
		iRet = tdr_create_table_columns_i(a_pstIOStream, a_pstMeta, a_iVersion);

		/*����������Ϣ*/
		if (0 < a_pstMeta->nPrimayKeyMemberNum)
		{
			iRet = tdr_create_table_primarykey_i(a_pstIOStream, a_pstMeta, a_iVersion);
		}

		/*UNIQUE ��Ϣ*/
		iRet = tdr_create_table_uniqueinfo_i(a_pstIOStream, a_pstMeta, a_iVersion);

        /*INDEX ��Ϣ*/
        if (TDR_INDEX_VERSION <= pstLib->nBuild)
        {
            if (0 < a_pstMeta->iIndexesNum)
            {
                iRet = tdr_create_table_index_i(a_pstIOStream, a_pstMeta, a_iVersion);
            }
        }

		iRet = tdr_iostream_write(a_pstIOStream, ")\n");

		/*���ñ�ѡ��*/
		iRet = tdr_set_table_options(a_pstDBMS, a_pstMeta, a_pstIOStream);

		iRet = tdr_iostream_write(a_pstIOStream, ";\n");
	}/*if (0 < a_pstMeta->iSplitTableFactor)*/

	return iRet;
}



int tdr_create_table_columns_i(INOUT LPTDRIOSTREAM a_pstIOStream, IN LPTDRMETA a_pstMeta, IN int a_iVersion)
{
	int iRet = TDR_SUCCESS;
	TDRSTACK stStack;
	LPTDRSTACKITEM pstStackTop;
	int iStackItemCount;
	LPTDRMETA pstCurMeta;
	LPTDRMETAENTRY pstEntry;
	LPTDRMETALIB pstLib;
	char *pszColType;
	char *pszNotNull;
	int iChange;
	int i;
	char szDefault[TDR_DEFAULT_VALUE_LEN + 16];

	char szColType[128];
	TDRBOOLEAN bFirstColumm = TDR_FALSE;
	char *pszPreName="";
	char *pszAutoIncrement = "";
	char szAutoIncrementKey[TDR_NAME_LEN * 2] = {0};

	assert(NULL != a_pstIOStream);
	assert(NULL != a_pstMeta);
	assert(a_iVersion >= a_pstMeta->iBaseVersion);

	pstLib = TDR_META_TO_LIB(a_pstMeta);

	pstStackTop = &stStack[0];
	pstStackTop->pstMeta = a_pstMeta;
	pstStackTop->iCount = 1;
	pstStackTop->idxEntry = 0;
	pstCurMeta = a_pstMeta;
	iStackItemCount = 1;
	pstStackTop->iRealCount = 1;
	pstStackTop->pstEntry = NULL;

	while (0 < iStackItemCount)
	{
		if (0 >= pstStackTop->iCount)
		{
			iStackItemCount--;
			if (0 < iStackItemCount)
			{
				pstStackTop--;
				pstCurMeta = pstStackTop->pstMeta;
				TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			}
			continue;
		}/*if (0 >= pstStackTop->iCount)*/

		pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;
		if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry) || TDR_ENTRY_IS_REFER_TYPE(pstEntry))
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}
		if (pstEntry->iVersion > a_iVersion)
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}

		/*������������*/
		if ((TDR_TYPE_COMPOSITE >= pstEntry->iType) && (TDR_ENTRY_DO_EXTENDABLE(pstEntry)))
		{
			if (TDR_STACK_SIZE <=  iStackItemCount)
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_TOO_COMPLIEX_META);
				break;
			}

			pstStackTop++;
			iStackItemCount++;
			pstCurMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
			pstStackTop->pstMeta = pstCurMeta;
			pstStackTop->iRealCount = pstEntry->iCount;
			pstStackTop->iCount = pstStackTop->iRealCount;
			pstStackTop->idxEntry = 0;
			pstStackTop->pstEntry = pstEntry; /*count of entry*/
			continue;
		}/*if (TDR_TYPE_COMPOSITE >= pstEntry->iType)*/

		/*������������*/
		strncpy(szColType, "", sizeof(szColType));
		tdr_gen_entry_sql_typename(szColType, sizeof(szColType), pstEntry, 0);
		pszColType = &szColType[0];

		/*����ǰ׺*/
		pszPreName = tdr_gen_entry_columnname_prefix(&stStack[0], iStackItemCount);




		/*Not Null */
		if (TDR_ENTRY_IS_NOT_NULL(pstEntry))
		{
			pszNotNull = "NOT NULL";
		}else
		{
			pszNotNull = "";
		}

		/*default value, if the entry is auto increment, it do not have deafault*/
		TDR_STRNCPY(szDefault, "", sizeof(szDefault));
		pszAutoIncrement = "";
		if (TDR_ENTRY_IS_AUTOINCREMENT(pstEntry))
		{
			pszAutoIncrement= "AUTO_INCREMENT";
			tdr_gen_autoincrement_key(a_pstMeta, pstEntry, &szAutoIncrementKey[0],
				sizeof(szAutoIncrementKey));
		}else if (TDR_INVALID_PTR != pstEntry->ptrDefaultVal)
		{
			char szDefaultval[TDR_DEFAULT_VALUE_LEN] = {0};

			tdr_get_entry_defaultvalue_i(szDefaultval, pstLib, pstEntry);
			if (szDefaultval[0] != '\0')
			{
				tdr_snprintf(szDefault, sizeof(szDefault), "DEFAULT %s", szDefaultval);
			}
		}/*if (TDR_INVALID_PTR != a_pstEntry->ptrDefaultVal)*/


		/*д����Ϣ*/
		if ((1 == pstEntry->iCount) ||
			((TDR_TYPE_CHAR <= pstEntry->iType) && (TDR_TYPE_BYTE >= pstEntry->iType)))
		{
			if (TDR_FALSE != bFirstColumm)
			{
				iRet = tdr_iostream_write(a_pstIOStream, ",\n");
			}else
			{
				bFirstColumm = TDR_TRUE;
			}
			iRet = tdr_iostream_write(a_pstIOStream, " %s%s %s %s %s %s",
				pszPreName, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName), pszColType, pszNotNull,
				pszAutoIncrement, szDefault);
		}else if (TDR_TYPE_WSTRING >= pstEntry->iType)
		{
			for (i = 1; i <= pstEntry->iCount; i++)
			{
				if (TDR_FALSE != bFirstColumm)
				{
					iRet = tdr_iostream_write(a_pstIOStream, ",\n");
				}else
				{


					bFirstColumm = TDR_TRUE;
				}
				iRet = tdr_iostream_write(a_pstIOStream, " %s%s_%d %s %s %s",
					pszPreName, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName), i, pszColType, pszNotNull, szDefault);
			}
		}/*if ((TDR_TYPE_CHAR <= pstEntry->iType) && (TDR_TYPE_BYTE >= pstEntry->iType))*/

		TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
	}/*while (0 < iStackItemCount)*/

	if ('\0' != szAutoIncrementKey[0])
	{
		iRet = tdr_iostream_write(a_pstIOStream, ",\n%s", szAutoIncrementKey);
	}

	return iRet;
}

void tdr_gen_entry_sql_typename(OUT char *a_szTypeBuff, IN int a_iBuffLen, IN LPTDRMETAENTRY a_pstEntry, IN int a_iMetaVer)
{
	size_t iSize;

	size_t iMaxSize = 0;
	LPTDRMETA pstMeta = NULL;
	LPTDRMETALIB pstMetaLib = NULL;
	int iRet = 0;

	assert(NULL != a_szTypeBuff);
	assert(NULL != a_pstEntry);
	assert(0 < a_iBuffLen);

	//fprintf(stdout, "a_pstEntry->iType(%d)\n\n", a_pstEntry->iType);

	if (TDR_TYPE_COMPOSITE >= a_pstEntry->iType)
	{
		/*changed by vinsonzuo 20110620, ͨ������Ԫ���ݴ��������С��Ԥ���ռ��С��������*/
                pstMeta = TDR_ENTRY_TO_META(a_pstEntry);
		pstMetaLib = TDR_META_TO_LIB(pstMeta);
		pstMeta = tdr_get_entry_type_meta(pstMetaLib, a_pstEntry);

		iRet = tdr_get_meta_max_net_size(pstMeta, a_iMetaVer, &iMaxSize);
		if (iRet != 0)
		{
			a_szTypeBuff[0] = '\0';
			//printf("Error: get max net size of meta:name(%s)ver(%d) fail(%s)\n\n", 
			//		tdr_get_meta_name(pstMeta), a_iMetaVer, tdr_error_string(iRet));
			return;
		}

		//printf("info: get max net size(%d) of meta:name(%s)ver(%d)\n\n", 
		//		iMaxSize, tdr_get_meta_name(pstMeta), a_iMetaVer);
		
		//assert(iMaxSize > 0);
		iMaxSize += TDR_DBMS_BLOB_BUFF_RESERVE;
		TDR_STRNCPY(a_szTypeBuff, tdr_get_db_blob_type_i(iMaxSize), a_iBuffLen);
		//TDR_STRNCPY(a_szTypeBuff, tdr_get_db_blob_type_i(a_pstEntry->iHUnitSize + sizeof(uint32_t)*2), a_iBuffLen);
	}else if (TDR_TYPE_STRING == a_pstEntry->iType)
	{
		if (0 < a_pstEntry->iCustomHUnitSize)
		{
			iSize = a_pstEntry->iCustomHUnitSize;
		}else
		{
			iSize = a_pstEntry->iHUnitSize;
		}
		tdr_get_db_string_type_i(a_szTypeBuff, iSize);
	}else if (TDR_TYPE_WSTRING == a_pstEntry->iType)
	{
		if (0 < a_pstEntry->iCustomHUnitSize)
		{
			iSize = a_pstEntry->iCustomHUnitSize;
		}else
		{
			iSize = a_pstEntry->iHUnitSize;
		}
		TDR_STRNCPY(a_szTypeBuff, tdr_get_db_blob_type_i(iSize), a_iBuffLen);
	}else if ((TDR_TYPE_CHAR <= a_pstEntry->iType) && (TDR_TYPE_BYTE >= a_pstEntry->iType) && (1 < a_pstEntry->iCount))
	{
		TDR_STRNCPY(a_szTypeBuff, tdr_get_db_blob_type_i(a_pstEntry->iHRealSize), a_iBuffLen);
	}else
	{
		LPTDRCTYPEINFO pstTypeInfo = tdr_idx_to_typeinfo(a_pstEntry->idxType);
		TDR_STRNCPY(a_szTypeBuff, pstTypeInfo->pszSQLName, a_iBuffLen);
	}
}

char *tdr_get_db_blob_type_i(IN size_t a_iSize)
{
	char *pszType;

	assert(0 < a_iSize);

	if (a_iSize <= 0xFFFF)
	{
		pszType = "BLOB";
	}else if (a_iSize <= 0xFFFFFF)
	{
		pszType = "MEDIUMBLOB ";
	}else
	{
		pszType = "LONGBLOB";
	}

	return pszType;
}


void tdr_get_db_string_type_i(OUT char *pszColType, IN size_t a_iSize)
{
	assert(NULL != pszColType);
	assert(0 < a_iSize);

	if (0xFF >= a_iSize)
	{
		sprintf(pszColType, "VARCHAR(%"TDRPRI_SIZET")", a_iSize);
	}else if (0xFFFF >= a_iSize)
	{
		sprintf(pszColType, "TEXT");
	}else if (0xFFFFFF >= a_iSize)
	{
		sprintf(pszColType, "MEDIUMTEXT");
	}else
	{
		sprintf(pszColType, "LONGTEXT");
	}

}

void tdr_get_entry_defaultvalue_i(OUT char *a_pszDefault, IN LPTDRMETALIB a_pstLib, IN LPTDRMETAENTRY a_pstEntry)
{
	char *pszDefault;

	assert(NULL != a_pszDefault);
	assert(NULL != a_pstLib);
	assert(NULL != a_pstEntry);
	assert(TDR_INVALID_PTR != a_pstEntry->ptrDefaultVal);

	pszDefault = TDR_GET_STRING_BY_PTR(a_pstLib, a_pstEntry->ptrDefaultVal);
	switch(a_pstEntry->iType)
	{
	case TDR_TYPE_STRING:
		sprintf(a_pszDefault, "\'%s\'", pszDefault);
		break;
	case TDR_TYPE_CHAR:
		sprintf(a_pszDefault, "\'%d\'", (int)pszDefault[0]);
		break;
	case TDR_TYPE_UCHAR:
	case TDR_TYPE_BYTE:
		sprintf(a_pszDefault, "\'%d\'", (int)(unsigned char )pszDefault[0]);
		break;
	case TDR_TYPE_SMALLINT:
		sprintf(a_pszDefault, "\'%d\'", (int)*(short *)pszDefault);
		break;
	case TDR_TYPE_SMALLUINT:
		sprintf(a_pszDefault, "\'%d\'", (int)*(unsigned short *)pszDefault);
		break;
	case TDR_TYPE_INT:
	case TDR_TYPE_LONG:
		sprintf(a_pszDefault, "\'%d\'", (int)*(int *)pszDefault);
		break;
	case TDR_TYPE_UINT:
	case TDR_TYPE_ULONG:
		sprintf(a_pszDefault, "\'%u\'", (uint32_t)*(uint32_t *)pszDefault);
		break;
	case TDR_TYPE_LONGLONG:
#if (defined(_WIN32) || defined(_WIN64)) &&  _MSC_VER < 1400  /*vc7,vc6,,*/
		sprintf(a_pszDefault, "\'%I64i\'", (int64_t)*(int64_t *)pszDefault);
#else
		sprintf(a_pszDefault, "\'%"TDRPRId64"\'", (int64_t)*(int64_t *)pszDefault);
#endif
		break;
	case TDR_TYPE_ULONGLONG:
#if (defined(_WIN32) || defined(_WIN64)) &&  _MSC_VER < 1400  /*vc7,vc6,,*/
		sprintf(a_pszDefault, "\'%I64u\'", (uint64_t)*(uint64_t *)pszDefault);
#else
		sprintf(a_pszDefault, "\'%"TDRPRIud64"\'", (uint64_t)*(uint64_t *)pszDefault);
#endif
		break;
	case TDR_TYPE_FLOAT:
		sprintf(a_pszDefault, "\'%f\'", *(float *)pszDefault);
		break;
	case TDR_TYPE_DOUBLE:
		sprintf(a_pszDefault, "\'%f\'", *(double *)pszDefault);
		break;
	case TDR_TYPE_DATETIME:
		{
			char szDateTime[32]={0};
			sprintf(a_pszDefault, "\'%s\'", tdr_tdrdatetime_to_str_r((tdr_datetime_t *)pszDefault,szDateTime,sizeof(szDateTime)));
		}
		break;
	case TDR_TYPE_DATE:
		{
			char szDate[16] = {0};
			sprintf(a_pszDefault, "\'%s\'", tdr_tdrdate_to_str_r((tdr_date_t *)pszDefault, szDate,sizeof(szDate)));
		}
		break;
	case TDR_TYPE_TIME:
		{
			char szTime[16]={0};
			sprintf(a_pszDefault, "\'%s\'", tdr_tdrtime_to_str_r((tdr_time_t *)pszDefault,szTime,sizeof(szTime)));
		}
		break;
	case TDR_TYPE_IP:
		sprintf(a_pszDefault, "\'%u\'", (uint32_t)*(tdr_ip_t *)pszDefault);
		break;
	case TDR_TYPE_WCHAR:
		{
			sprintf(a_pszDefault, "\'%u\'", (uint32_t)*(tdr_wchar_t *)pszDefault);
			break;
		}
	default:
		break;
	}
}

int tdr_create_table_index_i(INOUT LPTDRIOSTREAM a_pstIOStream, IN LPTDRMETA a_pstMeta, int a_iVersion)
{
    int iRet = TDR_SUCCESS;
    int i;
    int k;
    LPTDRMETALIB pstLib;

    assert(NULL != a_pstIOStream);
    assert(NULL != a_pstMeta);

	pstLib = TDR_META_TO_LIB(a_pstMeta);

    for (i = 0; i  < a_pstMeta->iIndexesNum; i++)
    {
        LPTDRMETAINDEX pstIndex = TDR_GET_META_INDEX_PTR(a_pstMeta) + i;
        iRet = tdr_iostream_write(a_pstIOStream, ",\n");
        iRet = tdr_iostream_write(a_pstIOStream, "INDEX %s (",
                                  TDR_GET_STRING_BY_PTR(pstLib, pstIndex->ptrName));
        for (k = 0; k < pstIndex->iColumnNum; k++)
        {
            LPTDRCOLUMNINFO pstColumn = TDR_GET_COLUMN_PTR(a_pstMeta, pstIndex) + k;
            LPTDRMETAENTRY  pstEntry  = TDR_PTR_TO_ENTRY(pstLib, pstColumn->ptrEntry);
            if (0 != k)
            {
                iRet = tdr_iostream_write(a_pstIOStream, ", ");
            }
            iRet = tdr_iostream_write(a_pstIOStream, "%s", TDR_GET_STRING_BY_PTR(pstLib, pstEntry->ptrName));
        }
        iRet = tdr_iostream_write(a_pstIOStream, ")");
    }


    return iRet;
}

int tdr_create_table_primarykey_i(INOUT LPTDRIOSTREAM a_pstIOStream, IN LPTDRMETA a_pstMeta,  int a_iVersion)
{
	int iRet = TDR_SUCCESS;
	int i;
	LPTDRDBKEYINFO pstKey;
	LPTDRMETAENTRY pstEntry;
	LPTDRMETALIB pstLib;

	assert(NULL != a_pstIOStream);
	assert(NULL != a_pstMeta);
	assert(TDR_INVALID_PTR != a_pstMeta->ptrPrimayKeyBase);

	if (0 >= a_pstMeta->nPrimayKeyMemberNum)
	{
		return TDR_SUCCESS;
	}

	pstKey = TDR_GET_PRIMARYBASEPTR(a_pstMeta);
	pstLib = TDR_META_TO_LIB(a_pstMeta);

	/*�ҳ���һ�����������*/
	for (i = 0; i < a_pstMeta->nPrimayKeyMemberNum; i++)
	{
		pstEntry = TDR_PTR_TO_ENTRY(pstLib, pstKey->ptrEntry);
		if (pstEntry->iVersion <= a_iVersion)
		{
			break;
		}
		pstKey++;
	}
	if (i >= a_pstMeta->nPrimayKeyMemberNum)
	{
		return TDR_SUCCESS;
	}
	pstEntry = TDR_PTR_TO_ENTRY(pstLib, pstKey->ptrEntry);
	iRet = tdr_iostream_write(a_pstIOStream, ",\nPRIMARY KEY(%s", TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName));

	for (i++; i < a_pstMeta->nPrimayKeyMemberNum; i++)
	{
		pstKey++;
		pstEntry = TDR_PTR_TO_ENTRY(pstLib, pstKey->ptrEntry);
		if (pstEntry->iVersion > a_iVersion)
		{
			continue;
		}
		iRet = tdr_iostream_write(a_pstIOStream, ", %s", TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName));
	}

	iRet = tdr_iostream_write(a_pstIOStream, ")", TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName));

	return iRet;
}

int tdr_create_table_uniqueinfo_i(INOUT LPTDRIOSTREAM a_pstIOStream, IN LPTDRMETA a_pstMeta, int a_iVersion)
{
	int iRet = TDR_SUCCESS;
	TDRSTACK stStack;
	LPTDRSTACKITEM pstStackTop;
	int iStackItemCount;
	int iUniqueKeyCount;
	LPTDRMETA pstCurMeta;
	LPTDRMETAENTRY pstEntry;
	LPTDRMETALIB pstLib;
	int iChange;
	TDRBOOLEAN bHaveUnique = TDR_FALSE;
	TDRBOOLEAN bFirst = TDR_TRUE;

	assert(NULL != a_pstIOStream);
	assert(NULL != a_pstMeta);

	pstStackTop = &stStack[0];
	pstStackTop->iCount = 1;
	pstStackTop->idxEntry = 0;
	pstStackTop->pstMeta = a_pstMeta;
	pstCurMeta = a_pstMeta;
	iStackItemCount = 1;
	iUniqueKeyCount = 0;
	pstLib = TDR_META_TO_LIB(a_pstMeta);
	pstStackTop->iRealCount = 1;
	pstStackTop->pstEntry = NULL;

	while (0 < iStackItemCount)
	{
		if (0 >= pstStackTop->iCount)
		{
			iStackItemCount--;
			if (0 < iStackItemCount)
			{
				pstStackTop--;
				pstCurMeta = pstStackTop->pstMeta;
				TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			}
			continue;
		}/*if (0 >= pstStackTop->iCount)*/

		pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;
		if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry) || TDR_ENTRY_IS_REFER_TYPE(pstEntry))
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}
		if (pstEntry->iVersion > a_iVersion)
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}

		if ((TDR_TYPE_STRUCT == pstEntry->iType) && TDR_ENTRY_DO_EXTENDABLE(pstEntry))
		{
			if (TDR_STACK_SIZE <=  iStackItemCount)
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_TOO_COMPLIEX_META);
				break;
			}

			pstStackTop++;
			iStackItemCount++;
			pstCurMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
			pstStackTop->pstMeta = pstCurMeta;
			pstStackTop->iCount = pstEntry->iCount;
			pstStackTop->idxEntry = 0;
			pstStackTop->iRealCount = pstStackTop->iCount;
			pstStackTop->pstEntry = pstEntry;
			continue;
		}/*if ((TDR_TYPE_STRUCT == pstEntry->iType) && TDR_ENTRY_DO_EXTENDABLE(pstEntry))*/

		if (TDR_ENTRY_IS_UNIQUE(pstEntry))
		{
			int i;
			char *pszPreName;
			if (!bHaveUnique)
			{
				bHaveUnique = TDR_TRUE;
				iRet = tdr_iostream_write(a_pstIOStream, ",\nUNIQUE (");
			}
			pszPreName = tdr_gen_entry_columnname_prefix(&stStack[0], iStackItemCount);
			if ((1 == pstEntry->iCount) ||
				((TDR_TYPE_CHAR <= pstEntry->iType) && (TDR_TYPE_BYTE >= pstEntry->iType)))
			{
				if (bFirst)
				{
					iRet = tdr_iostream_write(a_pstIOStream, "%s%s", pszPreName, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName));
					bFirst = TDR_FALSE;
				}else
				{
					iRet = tdr_iostream_write(a_pstIOStream, ",%s%s", pszPreName, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName));
				}
			}else if (TDR_TYPE_WSTRING >= pstEntry->iType)
			{
				for (i = 1; i <= pstEntry->iCount; i++)
				{
					if (bFirst)
					{
						iRet = tdr_iostream_write(a_pstIOStream, "%s%s_d", pszPreName, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName), i);
						bFirst = TDR_FALSE;
					}else
					{
						iRet = tdr_iostream_write(a_pstIOStream, ",%s%s_d", pszPreName, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName), i);
					}
				}
			}/*if ((TDR_TYPE_CHAR <= pstEntry->iType) && (TDR_TYPE_BYTE >= pstEntry->iType))*/

			iUniqueKeyCount++;
			if (iUniqueKeyCount >= TDR_MAX_UNIQUE_KEY_IN_TABLE)
			{
				break;
			}
		}/*if (TDR_ENTRY_IS_UNIQUE(pstEntry))*/
		TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
	}/*while (0 < iStackItemCount)*/

	if (bHaveUnique)
	{
		iRet = tdr_iostream_write(a_pstIOStream, ")");
	}

	return iRet;
}

int tdr_alter_table(IN LPTDRDBMS a_pstDBMS, IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstSql, IN int a_iVersion)
{
	int iRet = TDR_SUCCESS;
	TDRIOSTREAM stIOStream;

	/*assert(NULL != a_pstDBMS);
	assert(NULL != a_pstMeta);
	assert(NULL != a_pstSql);
	assert(NULL != a_pstSql->pszBuff);
	assert(0 < a_pstSql->iBuff);*/
	if ((NULL == a_pstDBMS)||(NULL == a_pstMeta)||(NULL == a_pstSql)||
		(NULL == a_pstSql->pszBuff)||(0 >= a_pstSql->iBuff))
	{
		return TDR_ERRIMPLE_INVALID_PARAM;
	}


	if (a_pstMeta->iBaseVersion > a_iVersion)
	{
		a_pstSql->iBuff = 0;
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
	}
	if (a_pstMeta->iCurVersion <= a_iVersion)
	{
		a_pstSql->iBuff = 0;
		return TDR_SUCCESS;
	}

	stIOStream.emIOStreamType = TDR_IOSTREAM_STRBUF;
	stIOStream.pszTDRIOBuff = a_pstSql->pszBuff;
	stIOStream.iTDRIOBuffLen = a_pstSql->iBuff;
	if (0 == tdr_stricmp(a_pstDBMS->szDBMSName, TDR_DBMS_MYSQL))
	{
		iRet = tdr_alter_table_i(a_pstMeta, &stIOStream, a_iVersion);
	}else
	{
		iRet =  TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_DB_UNSUPPORTED_DBMS);
	}

	a_pstSql->iBuff = stIOStream.pszTDRIOBuff - a_pstSql->pszBuff;

	return iRet;
}

int tdr_alter_table_fp(IN LPTDRDBMS a_pstDBMS, IN LPTDRMETA a_pstMeta, IN FILE *a_fpSql, IN int a_iVersion)
{
	int iRet = TDR_SUCCESS;
	TDRIOSTREAM stIOStream;

	/*assert(NULL != a_pstDBMS);
	assert(NULL != a_pstMeta);
	assert(NULL != a_fpSql);*/
	if ((NULL == a_pstDBMS)||(NULL == a_pstMeta)||(NULL == a_fpSql))
	{
		return TDR_ERRIMPLE_INVALID_PARAM;
	}


	if (a_pstMeta->iBaseVersion > a_iVersion)
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
	}
	if (a_pstMeta->iCurVersion <= a_iVersion)
	{
		return TDR_SUCCESS;
	}

	stIOStream.emIOStreamType = TDR_IOSTREAM_FILE;
	stIOStream.fpTDRIO = a_fpSql;

	if (0 == tdr_stricmp(a_pstDBMS->szDBMSName, TDR_DBMS_MYSQL))
	{
		iRet = tdr_alter_table_i(a_pstMeta, &stIOStream, a_iVersion);
	}else
	{
		iRet =  TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_DB_UNSUPPORTED_DBMS);
	}

	return iRet;
}

int tdr_alter_table_file(IN LPTDRDBMS a_pstDBMS, IN LPTDRMETA a_pstMeta, IN const char *a_pszSqlFile, IN int a_iVersion)
{
	int iRet = TDR_SUCCESS;
	FILE *fp;

	/*assert(NULL != a_pszSqlFile);*/
	if (NULL == a_pszSqlFile)
	{
		return TDR_ERRIMPLE_INVALID_PARAM;
	}

	fp = fopen(a_pszSqlFile, "w");
	if (NULL == fp)
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_FAILED_OPEN_FILE_TO_WRITE);
	}

    iRet = tdr_alter_table_fp(a_pstDBMS, a_pstMeta, fp, a_iVersion);

	fclose(fp);

	return iRet;
}

int tdr_alter_table_i(IN LPTDRMETA a_pstMeta, INOUT LPTDRIOSTREAM a_pstIOStream, IN int a_iVersion)
{
	int iRet = TDR_SUCCESS;
	int i;
    LPTDRMETALIB pstLib;

	assert(NULL != a_pstMeta);
	assert(NULL != a_pstIOStream);
	assert(a_iVersion >= a_pstMeta->iBaseVersion);

    pstLib = TDR_META_TO_LIB(a_pstMeta);
	if (TDR_META_IS_VARIABLE(a_pstMeta))
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_DB_UNSUPPORTED_VARIABLE_META);
	}

	if (0 < a_pstMeta->iSplitTableFactor)
	{
		/*�ֶ����д洢*/
		for (i = 0; i < a_pstMeta->iSplitTableFactor; i++)
		{
			/*���ɱ�ͷ*/
			iRet = tdr_iostream_write(a_pstIOStream, "ALTER TABLE  %s_%d \n",
				TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName), i);
			if (TDR_ERR_IS_ERROR(iRet))
			{
				break;
			}

			/*�޸����ݿ��ĸ���*/
			iRet = tdr_alter_table_columns_i(a_pstIOStream, a_pstMeta, a_iVersion);

			/*�޸�������Ϣ*/
			if (0 < a_pstMeta->nPrimayKeyMemberNum)
			{
				iRet = tdr_alter_table_primarykey_i(a_pstIOStream, a_pstMeta, a_iVersion);
			}

			/*UNIQUE ��Ϣ*/
			iRet = tdr_alter_table_uniqueinfo_i(a_pstIOStream, a_pstMeta, a_iVersion);

            /*INDEX ��Ϣ*/
            if (TDR_INDEX_VERSION <= pstLib->nBuild)
            {
                if (0 < a_pstMeta->iIndexesNum)
                {
                    iRet = tdr_alter_table_indexes_i(a_pstIOStream, a_pstMeta, a_iVersion);
                }
            }

			iRet = tdr_iostream_write(a_pstIOStream, ";\n", TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName));
			if (TDR_ERR_IS_ERROR(iRet))
			{
				break;
			}
		}/*for (i = 1; i <= a_pstMeta->iSplitTableFactor; i++)*/
	}else
	{
		/*���ɱ�ͷ*/
		iRet = tdr_iostream_write(a_pstIOStream, "ALTER TABLE %s \n", TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName));

		/*�޸����ݿ��ĸ���*/
		iRet = tdr_alter_table_columns_i(a_pstIOStream, a_pstMeta, a_iVersion);

		/*�޸�������Ϣ*/
		if (0 < a_pstMeta->nPrimayKeyMemberNum)
		{
			iRet = tdr_alter_table_primarykey_i(a_pstIOStream, a_pstMeta, a_iVersion);
		}

		/*UNIQUE ��Ϣ*/
		iRet = tdr_alter_table_uniqueinfo_i(a_pstIOStream, a_pstMeta, a_iVersion);

        /*INDEX ��Ϣ*/
        if (TDR_INDEX_VERSION <= pstLib->nBuild)
        {
            if (0 < a_pstMeta->iIndexesNum)
            {
                iRet = tdr_alter_table_indexes_i(a_pstIOStream, a_pstMeta, a_iVersion);
            }
        }

		iRet = tdr_iostream_write(a_pstIOStream, ";\n", TDR_GET_STRING_BY_PTR(pstLib,a_pstMeta->ptrName));
	}/*if (0 < a_pstMeta->iSplitTableFactor)*/

	return iRet;
}

int tdr_alter_table_indexes_i(INOUT LPTDRIOSTREAM a_pstIOStream, IN LPTDRMETA a_pstMeta, IN int a_iVersion)
{
    int iRet = TDR_SUCCESS;
    int i;
    int k;
    LPTDRMETALIB pstLib;

    assert(NULL != a_pstIOStream);
    assert(NULL != a_pstMeta);

	pstLib = TDR_META_TO_LIB(a_pstMeta);

    for (i = 0; i  < a_pstMeta->iIndexesNum; i++)
    {
        LPTDRMETAINDEX pstIndex = TDR_GET_META_INDEX_PTR(a_pstMeta) + i;
        if (a_iVersion < pstIndex->iVersion)
        {
            iRet = tdr_iostream_write(a_pstIOStream, ",\n");
            iRet = tdr_iostream_write(a_pstIOStream, "ADD INDEX %s (",
                                      TDR_GET_STRING_BY_PTR(pstLib, pstIndex->ptrName));
            for (k = 0; k < pstIndex->iColumnNum; k++)
            {
                LPTDRCOLUMNINFO pstColumn = TDR_GET_COLUMN_PTR(a_pstMeta, pstIndex) + k;
                LPTDRMETAENTRY  pstEntry  = TDR_PTR_TO_ENTRY(pstLib, pstColumn->ptrEntry);
                if (0 != k)
                {
                    iRet = tdr_iostream_write(a_pstIOStream, ", ");
                }
                iRet = tdr_iostream_write(a_pstIOStream, "%s", TDR_GET_STRING_BY_PTR(pstLib, pstEntry->ptrName));
            }
            iRet = tdr_iostream_write(a_pstIOStream, ")");
        }
    }


	return iRet;
}

int tdr_alter_table_columns_i(INOUT LPTDRIOSTREAM a_pstIOStream, IN LPTDRMETA a_pstMeta, IN int a_iVersion)
{
	int iRet = TDR_SUCCESS;
	TDRSTACK stStack;
	LPTDRSTACKITEM pstStackTop;
	int iStackItemCount;
	LPTDRMETA pstCurMeta;
	LPTDRMETAENTRY pstEntry;
	LPTDRMETALIB pstLib;
	char *pszColType;
	char *pszNotNull;
	int iChange;
	int i;
	char szDefault[TDR_DEFAULT_VALUE_LEN + 16];
	char szColType[128];
	char szColTypeBefore[128];
	char *pszColTypeBefor;
	LPTDRMETA pstEntryMeta;
	TDRBOOLEAN bFristCol = TDR_FALSE;
	char *pszPreName="";
	char *pszAutoIncrement;
	char szAutoIncrementKey[TDR_NAME_LEN *2] = {0};

	assert(NULL != a_pstIOStream);
	assert(NULL != a_pstMeta);
	assert(a_iVersion >= a_pstMeta->iBaseVersion);


	pstLib = TDR_META_TO_LIB(a_pstMeta);

	pstStackTop = &stStack[0];
	pstStackTop->pstMeta = a_pstMeta;
	pstStackTop->iCount = 1;
	pstStackTop->idxEntry = 0;
	pstCurMeta = a_pstMeta;
	iStackItemCount = 1;
	pstStackTop->iCutOffVersion = 0; /*�����洢�ṹ��Ա�İ汾��Ϣ*/

	while (0 < iStackItemCount)
	{
		if (0 >= pstStackTop->iCount)
		{
			iStackItemCount--;
			if (0 < iStackItemCount)
			{
				pstStackTop--;
				pstCurMeta = pstStackTop->pstMeta;
				TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			}
			continue;
		}/*if (0 >= pstStackTop->iCount)*/

		pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;
		if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry) || TDR_ENTRY_IS_REFER_TYPE(pstEntry))
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}

		if (pstStackTop->iCutOffVersion <= a_iVersion)
		{
			/*�ֶ��Ѿ���ԭ���汾�д���*/
			if ((pstEntry->iVersion <= a_iVersion) && !TDR_ENTRY_DO_EXTENDABLE(pstEntry))
			{
				/*add by vinsonzuo 20110620, �ж��������С��û�б仯����blob���͵ı仯*/
				if (TDR_TYPE_COMPOSITE >= pstEntry->iType)
				{
					pstEntryMeta = TDR_ENTRY_TO_META(pstEntry);
					if (pstEntryMeta->iCurVersion > a_iVersion)
					{
						/*��õ�ǰ����*/
						strncpy(szColType, "", sizeof(szColType));
						tdr_gen_entry_sql_typename(szColType, sizeof(szColType), pstEntry, 0);
						pszColType = &szColType[0];

						/*���֮ǰ�汾����*/
						strncpy(szColTypeBefore, "", sizeof(szColTypeBefore));
						tdr_gen_entry_sql_typename(szColTypeBefore, sizeof(szColTypeBefore), pstEntry, a_iVersion);
						pszColTypeBefor = &szColTypeBefore[0];

						/*�Ƚ�����*/
						if (strcmp(pszColType, pszColTypeBefor) != 0)
						{
							/*�����޸����*/
							pszPreName = tdr_gen_entry_columnname_prefix(&stStack[0], iStackItemCount);
							
							/*default value, if the entry is auto increment, it do not have deafault*/
							TDR_STRNCPY(szDefault, "", sizeof(szDefault));
							pszAutoIncrement = "";
							if (TDR_ENTRY_IS_AUTOINCREMENT(pstEntry))
							{
								pszAutoIncrement= "AUTO_INCREMENT";
								tdr_gen_autoincrement_key(a_pstMeta, pstEntry, &szAutoIncrementKey[0],
									sizeof(szAutoIncrementKey));
							}else if (TDR_INVALID_PTR != pstEntry->ptrDefaultVal)
							{
								char szDefaultval[TDR_DEFAULT_VALUE_LEN] = {0};
							
								tdr_get_entry_defaultvalue_i(szDefaultval, pstLib, pstEntry);
								if (szDefaultval[0] != '\0')
								{
									tdr_snprintf(szDefault, sizeof(szDefault), "DEFAULT %s", szDefaultval);
								}
							}/*if (TDR_INVALID_PTR != a_pstEntry->ptrDefaultVal)*/
							
							/*Not Null */
							if (TDR_ENTRY_IS_NOT_NULL(pstEntry))
							{
								pszNotNull = "NOT NULL";
							}else
							{
								pszNotNull = "";
							}
							
							/*д����Ϣ*/
							if ((1 == pstEntry->iCount) ||
								((TDR_TYPE_CHAR <= pstEntry->iType) && (TDR_TYPE_BYTE >= pstEntry->iType)))
							{
								if (TDR_FALSE != bFristCol)
								{
									iRet = tdr_iostream_write(a_pstIOStream, ",\n");
								}else
								{
									bFristCol = TDR_TRUE;
								}
								iRet = tdr_iostream_write(a_pstIOStream, "CHANGE %s%s %s%s %s %s %s %s",
									pszPreName, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName),
									pszPreName, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName),
									szColType, pszNotNull,
									pszAutoIncrement, szDefault);
							}else if (TDR_TYPE_WSTRING >= pstEntry->iType)
							{
							
								for (i = 1; i <= pstEntry->iCount; i++)
								{
									if (TDR_FALSE != bFristCol)
									{
										iRet = tdr_iostream_write(a_pstIOStream, ",\n");
									}else
									{
										bFristCol = TDR_TRUE;
									}
									iRet = tdr_iostream_write(a_pstIOStream, "CHANGE %s%s_%d %s%s_%d %s %s %s",
										pszPreName, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName), i,
										pszPreName, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName), i,
										pszColType, pszNotNull, szDefault);
								}
							}/*if ((TDR_TYPE_CHAR <= pstEntry->iType) && (TDR_TYPE_BYTE >= pstEntry->iType))*/
						}
					}
				}
				/*end add by vinsonzuo 20110620*/

				TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
				continue;
			}
		}/*if ((NULL == pstStackTop->pstEntry) || (pstStackTop->pstEntry->iVersion > a_iVersion))*/



		/*������������*/
		if ((TDR_TYPE_COMPOSITE >= pstEntry->iType) && (TDR_ENTRY_DO_EXTENDABLE(pstEntry)))
		{
			if (TDR_STACK_SIZE <=  iStackItemCount)
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_TOO_COMPLIEX_META);
				break;
			}

			pstStackTop++;
			iStackItemCount++;
			pstCurMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
			pstStackTop->pstMeta = pstCurMeta;
			pstStackTop->iCount = pstEntry->iCount;
			pstStackTop->iRealCount = pstStackTop->iCount;
			pstStackTop->pstEntry = pstEntry;
			pstStackTop->idxEntry = 0;
			if (pstEntry->iVersion > a_iVersion)
			{
				pstStackTop->iCutOffVersion = TDR_MAX_VERSION;
			}else
			{
				pstStackTop->iCutOffVersion = (pstStackTop -1)->iCutOffVersion;
			}

			continue;
		}/*if (TDR_TYPE_COMPOSITE >= pstEntry->iType)*/


		/*������������*/
		strncpy(szColType, "", sizeof(szColType));
		tdr_gen_entry_sql_typename(szColType, sizeof(szColType), pstEntry, 0);
		pszColType = &szColType[0];

		pszPreName = tdr_gen_entry_columnname_prefix(&stStack[0], iStackItemCount);

		/*default value, if the entry is auto increment, it do not have deafault*/
		TDR_STRNCPY(szDefault, "", sizeof(szDefault));
		pszAutoIncrement = "";
		if (TDR_ENTRY_IS_AUTOINCREMENT(pstEntry))
		{
			pszAutoIncrement= "AUTO_INCREMENT";
			tdr_gen_autoincrement_key(a_pstMeta, pstEntry, &szAutoIncrementKey[0],
				sizeof(szAutoIncrementKey));
		}else if (TDR_INVALID_PTR != pstEntry->ptrDefaultVal)
		{
			char szDefaultval[TDR_DEFAULT_VALUE_LEN] = {0};

			tdr_get_entry_defaultvalue_i(szDefaultval, pstLib, pstEntry);
			if (szDefaultval[0] != '\0')
			{
				tdr_snprintf(szDefault, sizeof(szDefault), "DEFAULT %s", szDefaultval);
			}
		}/*if (TDR_INVALID_PTR != a_pstEntry->ptrDefaultVal)*/

		/*Not Null */
		if (TDR_ENTRY_IS_NOT_NULL(pstEntry))
		{
			pszNotNull = "NOT NULL";
		}else
		{
			pszNotNull = "";
		}

		/*д����Ϣ*/
		if ((1 == pstEntry->iCount) ||
			((TDR_TYPE_CHAR <= pstEntry->iType) && (TDR_TYPE_BYTE >= pstEntry->iType)))
		{
			if (TDR_FALSE != bFristCol)
			{
				iRet = tdr_iostream_write(a_pstIOStream, ",\n");
			}else
			{
				bFristCol = TDR_TRUE;
			}
			iRet = tdr_iostream_write(a_pstIOStream, "ADD %s%s %s %s %s %s",
				pszPreName, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName), szColType, pszNotNull,
				pszAutoIncrement, szDefault);
		}else if (TDR_TYPE_WSTRING >= pstEntry->iType)
		{

			for (i = 1; i <= pstEntry->iCount; i++)
			{
				if (TDR_FALSE != bFristCol)
				{
					iRet = tdr_iostream_write(a_pstIOStream, ",\n");
				}else
				{
					bFristCol = TDR_TRUE;
				}
				iRet = tdr_iostream_write(a_pstIOStream, "ADD %s%s_%d %s %s %s",
					pszPreName, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName), i, pszColType, pszNotNull, szDefault);
			}
		}/*if ((TDR_TYPE_CHAR <= pstEntry->iType) && (TDR_TYPE_BYTE >= pstEntry->iType))*/

		TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
	}/*while (0 < iStackItemCount)*/

	if ('\0' != szAutoIncrementKey[0])
	{
		iRet = tdr_iostream_write(a_pstIOStream, ",\nADD %s", szAutoIncrementKey);
	}

	return iRet;
}

/** the auto column  must be defined as a key , so check this column whether is in primary
, if not gen then key for auto column, otherwise return ""
*@note ��Ϊͬһ������ֻ���ܳ���һ��auto increment��Ա,��˲��迼��������������˲��ÿ���
* ����ǰ׺
*/
int tdr_gen_autoincrement_key(IN LPTDRMETA a_pstMeta, IN LPTDRMETAENTRY a_pstEntry, char *pszKey, int iKeySize)
{
	int iWrite;
    LPTDRMETALIB pstLib;

	assert(NULL != a_pstMeta);
	assert(NULL != a_pstEntry);
	assert(NULL != pszKey);

	if (!TDR_ENTRY_IS_AUTOINCREMENT(a_pstEntry))
	{
		*pszKey = '\0';
		return TDR_SUCCESS;
	}
#if 0
	pstKey = TDR_GET_PRIMARYBASEPTR(a_pstMeta);
	pstLib = TDR_META_TO_LIB(a_pstMeta);
	for (i = 0; i < a_pstMeta->nPrimayKeyMemberNum; i++)
	{
		pstEntry = TDR_PTR_TO_ENTRY(pstLib, pstKey->ptrEntry);
		if (pstEntry == a_pstEntry)
		{
			break;
		}
		pstKey++;
	}/*for (i = 0; i < a_pstMeta->nPrimayKeyMemberNum; i++)*/
	if (i < a_pstMeta->nPrimayKeyMemberNum)
	{
		*pszKey = '\0';
		return TDR_SUCCESS;
	}
#endif


	/*not in primary , so generate key information for this column*/
    pstLib = TDR_META_TO_LIB(a_pstMeta);
	iWrite = tdr_snprintf(pszKey, iKeySize, " KEY(%s)", TDR_GET_STRING_BY_PTR(pstLib,a_pstEntry->ptrName));
	if ((0 > iWrite) || (iWrite >= iKeySize))
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_SPACE_TO_WRITE);
	}

	return TDR_SUCCESS;
}

int tdr_alter_table_primarykey_i(INOUT LPTDRIOSTREAM a_pstIOStream, IN LPTDRMETA a_pstMeta, int a_iVersion)
{
	int iRet = TDR_SUCCESS;
	int i;
	LPTDRDBKEYINFO pstKey;
	LPTDRMETAENTRY pstEntry;
	LPTDRMETALIB pstLib;

	assert(NULL != a_pstIOStream);
	assert(NULL != a_pstMeta);
	assert(TDR_INVALID_PTR != a_pstMeta->ptrPrimayKeyBase);

	if (0 >= a_pstMeta->nPrimayKeyMemberNum)
	{
		return TDR_SUCCESS;
	}

	pstKey = TDR_GET_PRIMARYBASEPTR(a_pstMeta);
	pstLib = TDR_META_TO_LIB(a_pstMeta);

	/*�ҳ���֮ǰ�汾���primary key�Ƿ�᲻ͬ*/
	for (i = 0; i < a_pstMeta->nPrimayKeyMemberNum; i++)
	{
		pstEntry = TDR_PTR_TO_ENTRY(pstLib, pstKey->ptrEntry);
		if (pstEntry->iVersion > a_iVersion)
		{
			break;
		}
		pstKey++;
	}
	if (i >= a_pstMeta->nPrimayKeyMemberNum)
	{
		return TDR_SUCCESS;
	}

	/*��֮ǰ�汾���primary key��ͬ*/
	iRet = tdr_iostream_write(a_pstIOStream, ",\nDROP PRIMARY KEY");  /*ɾ��֮ǰ������*/

	pstKey = TDR_GET_PRIMARYBASEPTR(a_pstMeta);
	pstEntry = TDR_PTR_TO_ENTRY(pstLib, pstKey->ptrEntry);
	iRet = tdr_iostream_write(a_pstIOStream, ",\nADD PRIMARY KEY(%s", TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName));
	for (i = 1; i < a_pstMeta->nPrimayKeyMemberNum; i++)
	{
		pstKey++;
		pstEntry = TDR_PTR_TO_ENTRY(pstLib, pstKey->ptrEntry);
		iRet = tdr_iostream_write(a_pstIOStream, ", %s", TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName));
	}

	iRet = tdr_iostream_write(a_pstIOStream, ")", TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName));

	return iRet;
}

int tdr_alter_table_uniqueinfo_i(INOUT LPTDRIOSTREAM a_pstIOStream, IN LPTDRMETA a_pstMeta, int a_iVersion)
{
	int iRet = TDR_SUCCESS;
	TDRSTACK stStack;
	LPTDRSTACKITEM pstStackTop;
	int iStackItemCount;
	int iUniqueKeyCount;
	LPTDRMETA pstCurMeta;
	LPTDRMETAENTRY pstEntry;
	LPTDRMETALIB pstLib;
	int iChange;
	TDRBOOLEAN bHaveUnique = TDR_FALSE;
	TDRBOOLEAN bFirst = TDR_TRUE;


	assert(NULL != a_pstIOStream);
	assert(NULL != a_pstMeta);

	pstStackTop = &stStack[0];
	pstStackTop->iCount = 1;
	pstStackTop->idxEntry = 0;
	pstStackTop->pstMeta = a_pstMeta;
	pstCurMeta = a_pstMeta;
	iStackItemCount = 1;
	iUniqueKeyCount = 0;
	pstLib = TDR_META_TO_LIB(a_pstMeta);
	pstStackTop->iCutOffVersion = 0;

	while (0 < iStackItemCount)
	{
		if (0 >= pstStackTop->iCount)
		{
			iStackItemCount--;
			if (0 < iStackItemCount)
			{
				pstStackTop--;
				pstCurMeta = pstStackTop->pstMeta;
				TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			}
			continue;
		}/*if (0 >= pstStackTop->iCount)*/

		pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;
		if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry) || TDR_ENTRY_IS_REFER_TYPE(pstEntry))
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}
		if (pstStackTop->iCutOffVersion <= a_iVersion)
		{
			if ((pstEntry->iVersion <= a_iVersion) && !TDR_ENTRY_DO_EXTENDABLE(pstEntry))
			{
				TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
				continue;
			}
		}/*if ((NULL == pstStackTop->pstEntry) || (pstStackTop->pstEntry->iVersion > a_iVersion))*/


		if ((TDR_TYPE_STRUCT == pstEntry->iType) && TDR_ENTRY_DO_EXTENDABLE(pstEntry))
		{
			if (TDR_STACK_SIZE <=  iStackItemCount)
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_TOO_COMPLIEX_META);
				break;
			}

			pstStackTop++;
			iStackItemCount++;
			pstCurMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
			pstStackTop->pstMeta = pstCurMeta;
			pstStackTop->iCount = pstEntry->iCount;
			pstStackTop->iRealCount = pstStackTop->iCount;
			pstStackTop->pstEntry = pstEntry;
			pstStackTop->idxEntry = 0;
			if (pstEntry->iVersion > a_iVersion)
			{
				pstStackTop->iCutOffVersion = TDR_MAX_VERSION;
			}else
			{
				pstStackTop->iCutOffVersion = (pstStackTop -1)->iCutOffVersion;
			}
			continue;
		}/*if ((TDR_TYPE_STRUCT == pstEntry->iType) && TDR_ENTRY_DO_EXTENDABLE(pstEntry))*/

		if (TDR_ENTRY_IS_UNIQUE(pstEntry))
		{
			int i;
			char *pszPreName;
			if (!bHaveUnique)
			{
				bHaveUnique = TDR_TRUE;
				iRet = tdr_iostream_write(a_pstIOStream, ",\nADD UNIQUE (");
			}
			pszPreName = tdr_gen_entry_columnname_prefix(&stStack[0], iStackItemCount);
			if ((1 == pstEntry->iCount) ||
				((TDR_TYPE_CHAR <= pstEntry->iType) && (TDR_TYPE_BYTE >= pstEntry->iType)))
			{
				if (bFirst)
				{
					iRet = tdr_iostream_write(a_pstIOStream, "%s%s", pszPreName, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName));
					bFirst = TDR_FALSE;
				}else
				{
					iRet = tdr_iostream_write(a_pstIOStream, ",%s%s", pszPreName, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName));
				}
			}else if (TDR_TYPE_WSTRING >= pstEntry->iType)
			{
				for (i = 1; i <= pstEntry->iCount; i++)
				{
					if (bFirst)
					{
						iRet = tdr_iostream_write(a_pstIOStream, "%s%s_d", pszPreName, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName), i);
						bFirst = TDR_FALSE;
					}else
					{
						iRet = tdr_iostream_write(a_pstIOStream, ",%s%s_d", pszPreName, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName), i);
					}
				}
			}/*if ((TDR_TYPE_CHAR <= pstEntry->iType) && (TDR_TYPE_BYTE >= pstEntry->iType))*/

			iUniqueKeyCount++;
			if (iUniqueKeyCount >= TDR_MAX_UNIQUE_KEY_IN_TABLE)
			{
				break;
			}
		}/*if (TDR_ENTRY_IS_UNIQUE(pstEntry))*/
		TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
	}/*while (0 < iStackItemCount)*/

	if (bHaveUnique)
	{
		iRet = tdr_iostream_write(a_pstIOStream, ")");
	}


	return iRet;
}

int tdr_open_dbhanlde(OUT LPTDRDBHANDLE a_phDBHandle, IN LPTDRDBMS a_pstDBMS, IN char *a_pszErrMsg)
{
	int iRet = TDR_SUCCESS;

	/*assert(NULL != a_phDBHandle);
	assert(NULL != a_pstDBMS);*/
	if ((NULL == a_phDBHandle)||(NULL == a_pstDBMS))
	{
		return TDR_ERRIMPLE_INVALID_PARAM;
	}


	if (0 == tdr_stricmp(a_pstDBMS->szDBMSName, TDR_DBMS_MYSQL))
	{
		iRet = tdr_dbdriver_open_handle(a_phDBHandle, a_pstDBMS, a_pszErrMsg);
	}else
	{
		iRet =  TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_DB_UNSUPPORTED_DBMS);
	}

	return iRet;
}


int tdr_obj2sql(IN TDRDBHANDLE a_hDBHandle, INOUT LPTDRDATA a_pstSql, IN int a_iOPType,
				INOUT  LPTDRDBOBJECT a_pstObj, IN const char *a_pszWhereDefinition)
{
	int iRet = TDR_SUCCESS;

	/*assert(NULL != a_hDBHandle);
	assert(NULL != a_pstSql);
	assert(NULL != a_pstObj);
	assert(NULL != a_pstSql->pszBuff);
	assert(0 < a_pstSql->iBuff);
	assert(NULL != a_pstObj->pstMeta);*/
	if ((NULL == a_hDBHandle)||(NULL == a_pstSql)||(NULL == a_pstObj)||
		(NULL == a_pstSql->pszBuff)||(0 >= a_pstSql->iBuff)||(NULL == a_pstObj->pstMeta)||
		(NULL == a_pstObj->pszObj))
	{
		return TDR_ERRIMPLE_INVALID_PARAM;
	}


	if (0 == a_pstObj->iVersion)
	{
		a_pstObj->iVersion = TDR_MAX_VERSION;
	}
	if (a_pstObj->pstMeta->iBaseVersion > a_pstObj->iVersion)
	{
		a_pstSql->iBuff = 0;
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
	}

	switch(a_iOPType)
	{
	case TDR_DBOP_INSERT:
		iRet = tdr_obj2sql_insert_i(a_hDBHandle, a_pstSql, a_pstObj);
		break;
	case TDR_DBOP_UPDATE:
		iRet = tdr_obj2sql_update_i(a_hDBHandle, a_pstSql, a_pstObj, a_pszWhereDefinition);
		break;
	case TDR_DBOP_DELETE:
		iRet = tdr_obj2sql_delete_i(a_hDBHandle, a_pstSql, a_pstObj, a_pszWhereDefinition);
		break;
	case TDR_DBOP_SELECT:
		iRet = tdr_obj2sql_select_i(a_hDBHandle, a_pstSql, a_pstObj, a_pszWhereDefinition);
		break;
    case TDR_DBOP_REPLACE:
        iRet = tdr_obj2sql_replace_i(a_hDBHandle, a_pstSql, a_pstObj);
        break;
	default:
		iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_DB_UNSUPPORTED_OPER);
	}

	return iRet;
}

int tdr_obj2sql_insert_i(IN TDRDBHANDLE a_hDBHandle, INOUT LPTDRDATA a_pstSql, INOUT  LPTDRDBOBJECT a_pstObj)
{
    return tdr_obj2sql_insert_or_replace_i(a_hDBHandle, a_pstSql, a_pstObj, "INSERT");
}

int tdr_obj2sql_replace_i(IN TDRDBHANDLE a_hDBHandle, INOUT LPTDRDATA a_pstSql, INOUT  LPTDRDBOBJECT a_pstObj)
{
    return tdr_obj2sql_insert_or_replace_i(a_hDBHandle, a_pstSql, a_pstObj, "REPLACE");
}

/** INSERT or REPLACE  INTO tbl_name (col_name,...) VALUES (col_val,...)
*������ߴ˺�����Ч�ʣ�����С�ú�����װģ��
*/
int tdr_obj2sql_insert_or_replace_i(IN TDRDBHANDLE a_hDBHandle, INOUT LPTDRDATA a_pstSql,
                                    INOUT  LPTDRDBOBJECT a_pstObj, IN char* a_pszOper)
{
	int iRet = TDR_SUCCESS;
	char *pchSql;
	char *pszHostStart;
	size_t iLeftLen;
	int iWriteLen;
	LPTDRMETA pstMeta;
	int iCutVersion;
	char szTableName[TDR_NAME_LEN*2];
	TDRDATA stTempSql;


	assert(NULL != a_hDBHandle);
	assert(NULL != a_pstSql);
	assert(NULL != a_pstObj);
	assert(NULL != a_pstSql->pszBuff);
	assert(0 < a_pstSql->iBuff);
	assert(NULL != a_pstObj->pstMeta);
	assert(NULL != a_pstObj->pszObj);
	assert(0 < a_pstObj->iObjSize);
    assert(NULL != a_pszOper);

	pchSql = a_pstSql->pszBuff;
	iLeftLen = a_pstSql->iBuff;
	pstMeta = a_pstObj->pstMeta;
	pszHostStart = a_pstObj->pszObj;
	iCutVersion = a_pstObj->iVersion;

	/*insert or replace into table*/
	iRet = tdr_get_dbtable_name(szTableName, sizeof(szTableName), pstMeta, pszHostStart);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		return iRet;
	}
	iWriteLen = tdr_snprintf(pchSql, iLeftLen, "%s INTO %s ", a_pszOper, szTableName);
	TDR_CHECK_BUFF(pchSql, iLeftLen, iWriteLen, iRet);


	/*(col_name,...)*/
	if (0 >= iLeftLen)
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);
	}
	*pchSql++ = '(';
	stTempSql.pszBuff = pchSql;
	stTempSql.iBuff = iLeftLen;
	iRet = tdr_gen_columm_name_lists_i(&stTempSql, a_pstObj, TDR_TRUE);
	pchSql = stTempSql.pszBuff + stTempSql.iBuff;
	iLeftLen -= stTempSql.iBuff;
	if (0 >= iLeftLen)
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);
	}
	*pchSql++ = ')';

	/*VALUES (col_val,...)*/
	iWriteLen = tdr_snprintf(pchSql, iLeftLen, " VALUES (");
	TDR_CHECK_BUFF(pchSql, iLeftLen, iWriteLen, iRet);
	stTempSql.pszBuff = pchSql;
	stTempSql.iBuff = iLeftLen;
	iRet = tdr_gen_columm_value_lists_i(&stTempSql, a_hDBHandle, a_pstObj);
	pchSql = stTempSql.pszBuff + stTempSql.iBuff;
	iLeftLen -= stTempSql.iBuff;
	if (0 >= iLeftLen)
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);
	}
	*pchSql++ = ')';


	if (0 < iLeftLen)
	{
		*pchSql++ = '\0';
	}

	a_pstSql->iBuff = pchSql - a_pstSql->pszBuff;
	return iRet;
}


int tdr_gen_where_definition_i(INOUT LPTDRDATA a_pstSql, IN TDRDBHANDLE a_hDBHandle, IN  LPTDRDBOBJECT a_pstObj, IN const char *a_pszWhereDefinition)
{
	int iRet = TDR_SUCCESS;
	int iWriteLen = 0;
	char *pChSql;
	LPTDRMETA pstMeta;
	LPTDRDBKEYINFO pstDBKey;
	LPTDRMETAENTRY pstEntry;
	char *pszHostStart;
	char *pszHostEnd;
	int i;
	int iKeyNum;
	size_t iLeftLen;
	LPTDRMETALIB pstLib;

	assert(NULL != a_pstSql);
	assert(NULL != a_pstObj);
	assert(NULL != a_pstSql->pszBuff);
	assert(0 < a_pstSql->iBuff);
	assert(NULL != a_pstObj->pstMeta);
	assert(NULL != a_pstObj->pszObj);
	assert(0 < a_pstObj->iObjSize);

	pChSql = a_pstSql->pszBuff;
	iLeftLen = a_pstSql->iBuff;
	if ((NULL != a_pszWhereDefinition) && ('\0' != *a_pszWhereDefinition))
	{
		/*�Զ���where����*/
		iWriteLen = tdr_snprintf(pChSql, iLeftLen, " %s", a_pszWhereDefinition);
		TDR_CHECK_BUFF(pChSql, iLeftLen, iWriteLen, iRet);
		a_pstSql->iBuff = iWriteLen;
		return iRet;
	}


	pstMeta = a_pstObj->pstMeta;
	pstLib = TDR_META_TO_LIB(pstMeta);
	if (0 == pstMeta->nPrimayKeyMemberNum)
	{
		a_pstSql->iBuff = 0;
		return iRet;
	}

	/*ͨ����������where����*/
	iWriteLen = tdr_snprintf(pChSql, iLeftLen, " WHERE ");
	TDR_CHECK_BUFF(pChSql, iLeftLen, iWriteLen, iRet);

	pstDBKey = TDR_GET_PRIMARYBASEPTR(pstMeta);
	iKeyNum = 0;
	pszHostEnd = a_pstObj->pszObj + a_pstObj->iObjSize;



	for (i = 0; i < pstMeta->nPrimayKeyMemberNum; i++)
	{
		int iArrayRealCount = 1;
		pstEntry = TDR_PTR_TO_ENTRY(pstLib, pstDBKey->ptrEntry);

		if (pstEntry->iVersion > a_pstObj->iVersion)
		{
			continue;
		}

		iKeyNum++;
		pszHostStart = a_pstObj->pszObj + pstDBKey->iHOff;

		if ((TDR_TYPE_CHAR <= pstEntry->iType) && (TDR_TYPE_BYTE >= pstEntry->iType))
		{
			char *pszMetaBase = pszHostStart - pstEntry->iHOff;
			TDR_GET_ARRAY_REAL_COUNT(iArrayRealCount, pstEntry, pszMetaBase, a_pstObj->iVersion);
			if (((0 < pstEntry->iCount) && (pstEntry->iCount < iArrayRealCount))||(0 > iArrayRealCount))
			{
				/*ʵ����ĿΪ�������������󳤶�Ҫ������Ч*/
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_REFER_VALUE);
				break;
			}
		}

		/*columm name*/
		iWriteLen = tdr_snprintf(pChSql, iLeftLen, "%s=", TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName));
		TDR_CHECK_BUFF(pChSql, iLeftLen, iWriteLen, iRet);

		/*value*/
		TDR_ENTRY_VALUE2SQL_VALUE(iRet, a_hDBHandle, pstLib, pstEntry, iArrayRealCount, TDR_INVALID_INDEX, a_pstObj->iVersion, pChSql, iLeftLen,
			pszHostStart, pszHostEnd);
		i++;
		break;

	}/*for (i = 0; i < pstMeta->nPrimayKeyMemberNum; i++)*/

	for (; i < pstMeta->nPrimayKeyMemberNum; i++)
	{
		int iArrayRealCount = 1;

		pstDBKey++;
		pstEntry = TDR_PTR_TO_ENTRY(pstLib, pstDBKey->ptrEntry);
		if (pstEntry->iVersion > a_pstObj->iVersion)
		{
			continue;
		}

		iKeyNum++;
		pszHostStart = a_pstObj->pszObj + pstDBKey->iHOff;
		if ((TDR_TYPE_CHAR <= pstEntry->iType) && (TDR_TYPE_BYTE >= pstEntry->iType))
		{
			char *pszMetaBase = pszHostStart - pstEntry->iHOff;
			TDR_GET_ARRAY_REAL_COUNT(iArrayRealCount, pstEntry, pszMetaBase, a_pstObj->iVersion);
			if (((0 < pstEntry->iCount) && (pstEntry->iCount < iArrayRealCount))||(0 > iArrayRealCount))
			{
				/*ʵ����ĿΪ�������������󳤶�Ҫ������Ч*/
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_REFER_VALUE);
				break;
			}
		}

		/*columm name*/
		iWriteLen = tdr_snprintf(pChSql, iLeftLen, " AND %s=", TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName));
		TDR_CHECK_BUFF(pChSql, iLeftLen, iWriteLen, iRet);

		/*value*/
		TDR_ENTRY_VALUE2SQL_VALUE(iRet, a_hDBHandle, pstLib, pstEntry, iArrayRealCount, TDR_INVALID_INDEX, a_pstObj->iVersion, pChSql, iLeftLen,
			pszHostStart, pszHostEnd);
	}/*for (i = 0; i < pstMeta->nPrimayKeyMemberNum; i++)*/

	if ((0 == iKeyNum) || (iKeyNum < pstMeta->nPrimayKeyMemberNum))
	{
		iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_DB_NO_PRIMARYKEY);
	}

	a_pstSql->iBuff = pChSql - a_pstSql->pszBuff;

	return iRet;
}


/** delete���ĸ�ʽ�� DELETE FROM tbl_name WHERE where_definition
*/
int tdr_obj2sql_delete_i(IN TDRDBHANDLE a_hDBHandle, INOUT LPTDRDATA a_pstSql, INOUT  LPTDRDBOBJECT a_pstObj, IN const char *a_pszWhere)
{
	int iRet = TDR_SUCCESS;
	char *pchSql;
	char *pszHostStart;
	size_t iLeftLen;
	int iWriteLen;
	LPTDRMETA pstMeta;
	int iCutVersion;
	char szTableName[TDR_NAME_LEN*2];
	TDRDATA stTempSql;


	assert(NULL != a_hDBHandle);
	assert(NULL != a_pstSql);
	assert(NULL != a_pstObj);
	assert(NULL != a_pstSql->pszBuff);
	assert(0 < a_pstSql->iBuff);
	assert(NULL != a_pstObj->pstMeta);
	assert(NULL != a_pstObj->pszObj);
	assert(0 < a_pstObj->iObjSize);

	pchSql = a_pstSql->pszBuff;
	iLeftLen = a_pstSql->iBuff;
	pstMeta = a_pstObj->pstMeta;
	pszHostStart = a_pstObj->pszObj;
	iCutVersion = a_pstObj->iVersion;

	/*delete from table */
	iRet = tdr_get_dbtable_name(szTableName, sizeof(szTableName), pstMeta, pszHostStart);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		return iRet;
	}
	iWriteLen = tdr_snprintf(pchSql, iLeftLen, "DELETE FROM %s ", szTableName);
	TDR_CHECK_BUFF(pchSql, iLeftLen, iWriteLen, iRet);


	/*where definition*/
	stTempSql.iBuff = iLeftLen;
	stTempSql.pszBuff = pchSql;
	iRet = tdr_gen_where_definition_i(&stTempSql, a_hDBHandle, a_pstObj, a_pszWhere);
	pchSql += stTempSql.iBuff;


	if (0 < iLeftLen)
	{
		*pchSql++ = '\0';
	}

	a_pstSql->iBuff = pchSql - a_pstSql->pszBuff;
	return iRet;
}

int tdr_obj2sql_update_i(IN TDRDBHANDLE a_hDBHandle, INOUT LPTDRDATA a_pstSql, INOUT  LPTDRDBOBJECT a_pstObj, IN const char *a_pszWhereDefinition)
{
	int iRet = TDR_SUCCESS;
	char *pchSql;
	size_t iLeftLen;
    int iWriteLen;
	LPTDRMETA pstCurMeta;
	TDRDATA stTempSql;
	char szTableName[TDR_NAME_LEN*2];
	char *pszHostStart;



	assert(NULL != a_hDBHandle);
	assert(NULL != a_pstSql);
	assert(NULL != a_pstObj);
	assert(NULL != a_pstSql->pszBuff);
	assert(0 < a_pstSql->iBuff);
	assert(NULL != a_pstObj->pstMeta);
	assert(NULL != a_pstObj->pszObj);
	assert(0 < a_pstObj->iObjSize);

	pchSql = a_pstSql->pszBuff;
	iLeftLen = a_pstSql->iBuff;
	pstCurMeta = a_pstObj->pstMeta;
	pszHostStart = a_pstObj->pszObj;

	/*UPDATE table SET*/
	iRet = tdr_get_dbtable_name(szTableName, sizeof(szTableName), pstCurMeta, pszHostStart);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		a_pstSql->iBuff = 0;
		return iRet;
	}
	iWriteLen = tdr_snprintf(pchSql, iLeftLen, "UPDATE %s SET ", szTableName);
	TDR_CHECK_BUFF(pchSql, iLeftLen, iWriteLen, iRet);

	stTempSql.iBuff = iLeftLen;
	stTempSql.pszBuff = pchSql;
	iRet = tdr_gen_update_columm_lists_i(a_hDBHandle, &stTempSql, a_pstObj);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		a_pstSql->iBuff = pchSql - a_pstSql->pszBuff;
		return iRet;
	}
	pchSql += stTempSql.iBuff;
	iLeftLen -= stTempSql.iBuff;

	/*where definition*/
	stTempSql.iBuff = iLeftLen;
	stTempSql.pszBuff = pchSql;
	iRet = tdr_gen_where_definition_i(&stTempSql, a_hDBHandle, a_pstObj, a_pszWhereDefinition);
	pchSql += stTempSql.iBuff;


	if (0 < iLeftLen)
	{
		*pchSql++ = '\0';
	}

	a_pstSql->iBuff = pchSql - a_pstSql->pszBuff;
	return iRet;
}

int tdr_gen_update_columm_lists_i(IN TDRDBHANDLE a_hDBHandle, INOUT LPTDRDATA a_pstSql, INOUT  LPTDRDBOBJECT a_pstObj)
{
	int iRet = TDR_SUCCESS;
	char *pchSql;
	size_t iLeftLen;
    int iWriteLen;
	LPTDRMETA pstCurMeta;
	int iCutVersion;
	TDRSTACK stStack;
	LPTDRSTACKITEM pstStackTop;
	int iStackItemCount;
	LPTDRMETAENTRY pstEntry;
	LPTDRMETALIB pstLib;
	int iArrayRealCount;
	char *pszHostStart,*pszHostEnd;
	int iChange;
	int i;
	char *pszPreName = "";

	assert(NULL != a_hDBHandle);
	assert(NULL != a_pstSql);
	assert(NULL != a_pstObj);
	assert(NULL != a_pstSql->pszBuff);
	assert(0 < a_pstSql->iBuff);
	assert(NULL != a_pstObj->pstMeta);
	assert(NULL != a_pstObj->pszObj);
	assert(0 < a_pstObj->iObjSize);

	pchSql = a_pstSql->pszBuff;
	iLeftLen = a_pstSql->iBuff;
	pstCurMeta = a_pstObj->pstMeta;
	pszHostStart = a_pstObj->pszObj;
	iCutVersion = a_pstObj->iVersion;


	/*col_name1=expr1 [, col_name2=expr2 ...]*/
	pstStackTop = &stStack[0];
	pstStackTop->pstMeta = pstCurMeta;
	pstStackTop->iCount = 1;
	pstStackTop->idxEntry = 0;
	pstStackTop->pszHostBase = a_pstObj->pszObj;
	iStackItemCount = 1;
	pstLib = TDR_META_TO_LIB(pstCurMeta);
	pszHostEnd = a_pstObj->pszObj + a_pstObj->iObjSize;
	pstStackTop->iRealCount = 1;
	pstStackTop->pstEntry = NULL;

	while (0 < iStackItemCount)
	{
		int idxSelEntry = TDR_INVALID_INDEX;

		if (0 >= pstStackTop->iCount)
		{
			iStackItemCount--;
			if (0 < iStackItemCount)
			{
				pstStackTop--;
				pstCurMeta = pstStackTop->pstMeta;
				TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			}
			continue;
		}/*if (0 >= pstStackTop->iCount)*/

		pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;
		if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry) || TDR_ENTRY_IS_REFER_TYPE(pstEntry))
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}
		if (pstEntry->iVersion > iCutVersion)
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}

		/*�����в��ظ���*/
		if (TDR_ENTRY_IS_PRIMARYKEY(pstEntry))
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}


		/*ȡ����entry�����������Ϣ*/
		TDR_GET_ARRAY_REAL_COUNT(iArrayRealCount, pstEntry, pstStackTop->pszHostBase, iCutVersion);
		if (((0 < pstEntry->iCount) && (pstEntry->iCount < iArrayRealCount))||(0 > iArrayRealCount))
		{
			/*ʵ����ĿΪ�������������󳤶�Ҫ������Ч*/
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_REFER_VALUE);
			break;
		}
		if (0 == iArrayRealCount)
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}
		pszHostStart = pstStackTop->pszHostBase + pstEntry->iHOff;

		/*union ����*/
		if (TDR_TYPE_UNION == pstEntry->iType)
		{
			LPTDRMETA pstSelMeta;
			TDR_GET_UNION_ENTRY_TYPE_META_INFO(pstStackTop->pszHostBase, pstLib, pstEntry, iCutVersion, pstSelMeta, idxSelEntry);
			if (TDR_INVALID_INDEX == idxSelEntry)
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
				break;
			}
			if (NULL == pstSelMeta)
			{
				continue;
			}
		}/*if (TDR_TYPE_UNION == pstEntry->iType)*/


		/*������������*/
		if ((TDR_TYPE_COMPOSITE >= pstEntry->iType) && (TDR_ENTRY_DO_EXTENDABLE(pstEntry)))
		{
			if (TDR_STACK_SIZE <=  iStackItemCount)
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_TOO_COMPLIEX_META);
				break;
			}

			pszHostStart = pstStackTop->pszHostBase + pstEntry->iHOff;
			pstStackTop++;
			iStackItemCount++;
			pstCurMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
			pstStackTop->pstMeta = pstCurMeta;
			pstStackTop->iCount = iArrayRealCount;
			pstStackTop->iRealCount = iArrayRealCount;
			pstStackTop->pstEntry = pstEntry;
			pstStackTop->idxEntry = 0;
			pstStackTop->pszHostBase = pszHostStart;

			continue;
		}/*if (TDR_TYPE_COMPOSITE >= pstEntry->iType)*/

		pszPreName = tdr_gen_entry_columnname_prefix(&stStack[0], iStackItemCount);
		/*������������*/
		if ((1 == pstEntry->iCount) ||
			((TDR_TYPE_CHAR <= pstEntry->iType) && (TDR_TYPE_BYTE >= pstEntry->iType)))
		{
			iWriteLen = tdr_snprintf(pchSql, iLeftLen, "%s%s=", pszPreName, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName));
			TDR_CHECK_BUFF(pchSql, iLeftLen, iWriteLen, iRet);
			TDR_ENTRY_VALUE2SQL_VALUE(iRet, a_hDBHandle, pstLib, pstEntry, iArrayRealCount, idxSelEntry, a_pstObj->iVersion, pchSql, iLeftLen,
				pszHostStart, pszHostEnd);
			if (0 < iLeftLen)
			{
				iLeftLen--;
				*pchSql++ = ',';
			}
		}else if (TDR_TYPE_WSTRING >= pstEntry->iType)
		{
			for (i = 1; i <= iArrayRealCount; i++)
			{
				iWriteLen = tdr_snprintf(pchSql, iLeftLen, "%s%s_%d=", pszPreName, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName), i);
				TDR_CHECK_BUFF(pchSql, iLeftLen, iWriteLen, iRet);
				TDR_ENTRY_VALUE2SQL_VALUE(iRet, a_hDBHandle, pstLib, pstEntry, 1, idxSelEntry, a_pstObj->iVersion, pchSql, iLeftLen,
					pszHostStart, pszHostEnd);
				if (0 < iLeftLen)
				{
					iLeftLen--;
					*pchSql++ = ',';
				}
			}
		}/*if ((TDR_TYPE_CHAR <= pstEntry->iType) && (TDR_TYPE_BYTE >= pstEntry->iType))*/

		TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
	}/*while (0 < iStackItemCount)*/

	/*ȥ�����һ��','�ַ�*/
	pchSql--;

	a_pstSql->iBuff = pchSql - a_pstSql->pszBuff;
	return iRet;
}


/** SELECT col_name,... FROM  tbl_name  WHERE where_definition��
*/
int tdr_obj2sql_select_i(IN TDRDBHANDLE a_hDBHandle, INOUT LPTDRDATA a_pstSql, INOUT  LPTDRDBOBJECT a_pstObj, IN const char *a_pszWhereDefinition)
{
	int iRet = TDR_SUCCESS;
	char *pchSql;
	char *pszHostStart;
	size_t iLeftLen;
	int iWriteLen;
	LPTDRMETA pstMeta;
	int iCutVersion;
	char szTableName[TDR_NAME_LEN*2];
	TDRDATA stTempSql;


	assert(NULL != a_hDBHandle);
	assert(NULL != a_pstSql);
	assert(NULL != a_pstObj);
	assert(NULL != a_pstSql->pszBuff);
	assert(0 < a_pstSql->iBuff);
	assert(NULL != a_pstObj->pstMeta);
	assert(NULL != a_pstObj->pszObj);
	assert(0 < a_pstObj->iObjSize);

	pchSql = a_pstSql->pszBuff;
	iLeftLen = a_pstSql->iBuff;
	pstMeta = a_pstObj->pstMeta;
	pszHostStart = a_pstObj->pszObj;
	iCutVersion = a_pstObj->iVersion;

	/*SELECT*/
	iWriteLen = tdr_snprintf(pchSql, iLeftLen, "SELECT ");
	TDR_CHECK_BUFF(pchSql, iLeftLen, iWriteLen, iRet);


	/*col_name,...*/
	stTempSql.pszBuff = pchSql;
	stTempSql.iBuff = iLeftLen;
	iRet = tdr_gen_columm_name_lists_i(&stTempSql, a_pstObj, TDR_FALSE);
	pchSql = stTempSql.pszBuff + stTempSql.iBuff;
	iLeftLen -= stTempSql.iBuff;


	/*FROM table_name*/
	iRet = tdr_get_dbtable_name(szTableName, sizeof(szTableName), pstMeta, pszHostStart);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		return iRet;
	}
	iWriteLen = tdr_snprintf(pchSql, iLeftLen, " FROM %s ", szTableName);
	TDR_CHECK_BUFF(pchSql, iLeftLen, iWriteLen, iRet);

	/*where definition*/
	stTempSql.iBuff = iLeftLen;
	stTempSql.pszBuff = pchSql;
	iRet = tdr_gen_where_definition_i(&stTempSql, a_hDBHandle, a_pstObj, a_pszWhereDefinition);
	pchSql += stTempSql.iBuff;


	if (0 < iLeftLen)
	{
		*pchSql++ = '\0';
	}

	a_pstSql->iBuff = pchSql - a_pstSql->pszBuff;
	return iRet;
}

/** SELECT col_name,... FROM  tbl_name  WHERE where_definition��
*/
int tdr_obj2sql_select_table_i(IN TDRDBHANDLE a_hDBHandle, INOUT LPTDRDATA a_pstSql, 
		INOUT  LPTDRDBOBJECT a_pstObj, IN const char *a_pszWhereDefinition, IN char* a_pszTable)
{
	int iRet = TDR_SUCCESS;
	char *pchSql;
	char *pszHostStart;
	size_t iLeftLen;
	int iWriteLen;
	LPTDRMETA pstMeta;
	int iCutVersion;
	//char szTableName[TDR_NAME_LEN*2];
	TDRDATA stTempSql;


	assert(NULL != a_hDBHandle);
	assert(NULL != a_pstSql);
	assert(NULL != a_pstObj);
	assert(NULL != a_pstSql->pszBuff);
	assert(0 < a_pstSql->iBuff);
	assert(NULL != a_pstObj->pstMeta);
	assert(NULL != a_pstObj->pszObj);
	assert(0 < a_pstObj->iObjSize);

	pchSql = a_pstSql->pszBuff;
	iLeftLen = a_pstSql->iBuff;
	pstMeta = a_pstObj->pstMeta;
	pszHostStart = a_pstObj->pszObj;
	iCutVersion = a_pstObj->iVersion;

	/*SELECT*/
	iWriteLen = tdr_snprintf(pchSql, iLeftLen, "SELECT ");
	TDR_CHECK_BUFF(pchSql, iLeftLen, iWriteLen, iRet);


	/*col_name,...*/
	stTempSql.pszBuff = pchSql;
	stTempSql.iBuff = iLeftLen;
	iRet = tdr_gen_columm_name_lists_i(&stTempSql, a_pstObj, TDR_FALSE);
	pchSql = stTempSql.pszBuff + stTempSql.iBuff;
	iLeftLen -= stTempSql.iBuff;


	/*FROM table_name*/
	/*iRet = tdr_get_dbtable_name(szTableName, sizeof(szTableName), pstMeta, pszHostStart);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		return iRet;
	}*/
	iWriteLen = tdr_snprintf(pchSql, iLeftLen, " FROM %s ", a_pszTable);
	TDR_CHECK_BUFF(pchSql, iLeftLen, iWriteLen, iRet);

	/*where definition*/
	stTempSql.iBuff = iLeftLen;
	stTempSql.pszBuff = pchSql;
	iRet = tdr_gen_where_definition_i(&stTempSql, a_hDBHandle, a_pstObj, a_pszWhereDefinition);
	pchSql += stTempSql.iBuff;


	if (0 < iLeftLen)
	{
		*pchSql++ = '\0';
	}

	a_pstSql->iBuff = pchSql - a_pstSql->pszBuff;
	return iRet;
}


int tdr_fetch_row(IN  TDRDBRESULTHANDLE a_hDBResult, INOUT  LPTDRDBOBJECT a_pstObj)
{
	int iRet = TDR_SUCCESS;

	/*assert(NULL != a_hDBResult);
	assert(NULL != a_pstObj);
	assert(NULL != a_pstObj->pstMeta);
	assert(NULL != a_pstObj->pszObj);
	assert(0 < a_pstObj->iObjSize);
	assert(a_pstObj->iObjSize >= a_pstObj->pstMeta->iHUnitSize);*/
	if ((NULL == a_hDBResult)||(NULL == a_pstObj)||(NULL == a_pstObj->pstMeta)||
		(NULL == a_pstObj->pszObj)||(a_pstObj->iObjSize < a_pstObj->pstMeta->iHUnitSize))
	{
		return TDR_ERRIMPLE_INVALID_PARAM;
	}

	if (0 == a_pstObj->iVersion)
	{
		a_pstObj->iVersion = TDR_MAX_VERSION;
	}
	if (a_pstObj->pstMeta->iBaseVersion > a_pstObj->iVersion)
	{
		a_pstObj->iObjSize = 0;
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
	}

	/*�ȴӽ������ȡ��һ����¼*/
	iRet = tdr_dbms_fetch_row(a_hDBResult);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		return iRet;
	}

	iRet = tdr_fetch_all_fields(a_hDBResult, a_pstObj);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		return iRet;
	}

	return iRet;
}

int tdr_fetch_all_fields(IN  TDRDBRESULTHANDLE a_hDBResult, INOUT  LPTDRDBOBJECT a_pstObj)
{
	int iRet = TDR_SUCCESS;
	LPTDRMETA pstCurMeta;
	TDRSTACK stStack;
	LPTDRSTACKITEM pstStackTop;
	int iStackItemCount;
	LPTDRMETAENTRY pstEntry;
	LPTDRMETALIB pstLib;
	int iArrayRealCount;
	char *pszHostStart,*pszHostEnd;
	int iChange;
	int i;
	char *pszFieldVal;
	unsigned long dwFieldLen;
	char *pszPreName;
	char szFieldName[TDR_NAME_LEN *2];

	assert(NULL != a_pstObj);
	assert(NULL != a_pstObj->pszObj);
	assert(NULL != a_pstObj->pstMeta);

	pstCurMeta = a_pstObj->pstMeta;
	pstLib = TDR_META_TO_LIB(pstCurMeta);
	pstStackTop = &stStack[0];
	pstStackTop->pstMeta = pstCurMeta;
	pstStackTop->iCount = 1;
	pstStackTop->idxEntry = 0;
	pstStackTop->pszHostBase = a_pstObj->pszObj;
	iStackItemCount = 1;
	pszHostEnd = a_pstObj->pszObj + a_pstObj->iObjSize;
	pszHostStart = a_pstObj->pszObj;
	pstStackTop->iRealCount = 1;
	pstStackTop->pstEntry = NULL;

	while (0 < iStackItemCount)
	{
		int idxSelEntry = TDR_INVALID_INDEX;

		if (0 >= pstStackTop->iCount)
		{
			iStackItemCount--;
			if (0 < iStackItemCount)
			{
				pstStackTop--;
				pstCurMeta = pstStackTop->pstMeta;
				TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			}
			continue;
		}/*if (0 >= pstStackTop->iCount)*/

		pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;
		if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry) || TDR_ENTRY_IS_REFER_TYPE(pstEntry))
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}


		/*ȡ����entry�����������Ϣ*/
		TDR_GET_ARRAY_REAL_COUNT(iArrayRealCount, pstEntry, pstStackTop->pszHostBase, a_pstObj->iVersion);
		if (((0 < pstEntry->iCount) && (pstEntry->iCount < iArrayRealCount))||(0 > iArrayRealCount))
		{
			/*ʵ����ĿΪ�������������󳤶�Ҫ������Ч*/
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_REFER_VALUE);
			break;
		}
		if (0 == iArrayRealCount)
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}
		pszHostStart = pstStackTop->pszHostBase + pstEntry->iHOff;
		if (pstEntry->iVersion > a_pstObj->iVersion)
		{
			TDR_SET_DEFAULT_VALUE(iRet, pszHostStart, pszHostEnd, pstLib, pstEntry, 1);
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}

		/*union ����*/
		if (TDR_TYPE_UNION == pstEntry->iType)
		{
			LPTDRMETA pstSelMeta;
			TDR_GET_UNION_ENTRY_TYPE_META_INFO(pstStackTop->pszHostBase, pstLib, pstEntry, a_pstObj->iVersion, pstSelMeta, idxSelEntry);
			if (TDR_INVALID_INDEX == idxSelEntry)
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
				break;
			}
			if (NULL == pstSelMeta)
			{
				continue;
			}
		}/*if (TDR_TYPE_UNION == pstEntry->iType)*/


		/*������������*/
		if ((TDR_TYPE_COMPOSITE >= pstEntry->iType) && (TDR_ENTRY_DO_EXTENDABLE(pstEntry)))
		{
			if (TDR_STACK_SIZE <=  iStackItemCount)
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_TOO_COMPLIEX_META);
				break;
			}

			pszHostStart = pstStackTop->pszHostBase + pstEntry->iHOff;
			pstStackTop++;
			iStackItemCount++;
			pstCurMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
			pstStackTop->pstMeta = pstCurMeta;
			pstStackTop->iCount = iArrayRealCount;
			pstStackTop->iRealCount = iArrayRealCount;
			pstStackTop->pstEntry = pstEntry;
			pstStackTop->idxEntry = 0;
			pstStackTop->pszHostBase = pszHostStart;

			continue;
		}/*if (TDR_TYPE_COMPOSITE >= pstEntry->iType)*/


		/*������������������*/
		pszPreName = tdr_gen_entry_columnname_prefix(&stStack[0], iStackItemCount);
		if ((1 == pstEntry->iCount) ||
			((TDR_TYPE_CHAR <= pstEntry->iType) && (TDR_TYPE_BYTE >= pstEntry->iType)))
		{
			tdr_snprintf(szFieldName, sizeof(szFieldName), "%s%s", pszPreName, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName));
			iRet = tdr_dbms_fetch_field(&pszFieldVal, &dwFieldLen, a_hDBResult, szFieldName);
			if (TDR_ERR_IS_ERROR(iRet))
			{
				break;
			}
			if (NULL != pszFieldVal)
			{
				/*������ΪNULL*/
				TDR_SQL_VALUE2_ENTRY_VALUE(iRet, pstLib,pstEntry, iArrayRealCount,idxSelEntry, a_pstObj->iVersion, pszHostStart, pszHostEnd, pszFieldVal, dwFieldLen);
			}else
			{
				TDR_SET_DEFAULT_VALUE(iRet, pszHostStart, pszHostEnd, pstLib, pstEntry, 1);
			}

            /*add by vinson 20110319*/
            if (TDR_ERR_IS_ERROR(iRet))
            {
                break;
            }

		}else if (TDR_TYPE_WSTRING >= pstEntry->iType)
		{
			for (i = 1; i <= iArrayRealCount; i++)
			{
				tdr_snprintf(szFieldName, sizeof(szFieldName), "%s%s_%d", pszPreName, TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrName), i);
				iRet = tdr_dbms_fetch_field(&pszFieldVal, &dwFieldLen, a_hDBResult, szFieldName);
				if (TDR_ERR_IS_ERROR(iRet))
				{
					break;
				}

				if (NULL != pszFieldVal)
				{
					/*������ΪNULL*/
					TDR_SQL_VALUE2_ENTRY_VALUE(iRet, pstLib,pstEntry, 1, idxSelEntry, a_pstObj->iVersion, pszHostStart, pszHostEnd, pszFieldVal, dwFieldLen);
				}else
				{
					TDR_SET_DEFAULT_VALUE(iRet, pszHostStart, pszHostEnd, pstLib, pstEntry, 1);
				}

                /*add by vinson 20110319*/
                if (TDR_ERR_IS_ERROR(iRet))
                {
                    break;
                }

			}
		}/*if ((TDR_TYPE_CHAR <= pstEntry->iType) && (TDR_TYPE_BYTE >= pstEntry->iType))*/

		TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
	}/*while (0 < iStackItemCount)*/

	a_pstObj->iObjSize = pszHostStart - a_pstObj->pszObj;

	return iRet;
}

