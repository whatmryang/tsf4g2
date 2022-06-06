/*
**  @file $RCSfile: tconnmgr.c,v $
**  general description of this module
**  $Id: tconnmgr.c,v 1.2 2008-08-07 08:53:30 steve Exp $
**  @author $Author: steve $
**  @date $Date: 2008-08-07 08:53:30 $
**  @version $Revision: 1.2 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#include "pal/pal.h"
#include "tdr/tdr.h"
/*
#include "obus/oi_misc.h"
#include "obus/oi_log.h"
#include "obus/oi_cfg.h"
#include "obus/oi_cistr.h"
#include "obus/oi_str.h"
#include "obus/og_ini.h"
#include "obus/og_bus.h"
*/
#include "tdr/tdr_metalib_manage_i.h"
#include "tlogdmgr.h"

#if defined(_WIN32) || defined(_WIN64)
	#pragma warning(disable:4996)
#endif

#define DECLA
int tlogd_tconnd_init_pdulen_noneparser(PDUINST* pstPDUInst, PDU* pstPDU);
int tlogd_tconnd_init_pdulen_tlogparser(PDUINST* pstPDUInst, PDU* pstPDU);
int tlogd_tconnd_init_pdulen_thttpparser(PDUINST* pstPDUInst, PDU* pstPDU);


int tlogd_tconnd_init_pdulen_tdrparser(TDRINSTLIST* pstTDRInstList, PDUINST* pstPDUInst, PDU* pstPDU);

extern unsigned char g_szMetalib_tlogd[];

extern int tlogd_tconnd_get_pkglen(TCONNINST* pstInst, TLOGDTHREAD* pstThread);
extern int tlogd_tconnd_get_thttp_pkglen(TCONNINST* pstInst, TLOGDTHREAD* pstThread);
extern int tlogd_tconnd_get_none_pkglen(TCONNINST* pstInst, TLOGDTHREAD* pstThread);

int tlogd_tconnd_fini_tdrinstlist(TDRINSTLIST* pstTDRInstList)
{
	int i;

	for(i=0; i<pstTDRInstList->iCount; i++)
	{
		tdr_free_lib(&pstTDRInstList->astInsts[i].pstLib);
	}

	pstTDRInstList->iCount	=	0;

	return 0;
}

int tlogd_tconnd_init_tdrinstlist(TDRINSTLIST* pstTDRInstList, TDRLIST* pstTDRList)
{
	TDR* pstTDR;
	TDRINST* pstTDRInst;
	int iIsFail	=	0;

	pstTDRInstList->iCount	=	0;

	while( pstTDRInstList->iCount < pstTDRList->iCount )
	{
		if( pstTDRInstList->iCount >= (int) (sizeof(pstTDRInstList->astInsts)/sizeof(pstTDRInstList->astInsts[0])) )
		{
			iIsFail	=	1;
			break;
		}

		pstTDR		=	pstTDRList->astTDRs + pstTDRInstList->iCount;
		pstTDRInst	=	pstTDRInstList->astInsts + pstTDRInstList->iCount;

		if( '\0'!=pstTDR->szPath[0] && tdr_load_metalib(&pstTDRInst->pstLib, pstTDR->szPath)<0 )
		{
			iIsFail	=	1;
			break;
		}

		STRNCPY(pstTDRInst->szName, pstTDR->szName, sizeof(pstTDRInst->szName));

		pstTDRInstList->iCount++;
	}

	if( !iIsFail )
		return 0;

	tlogd_tconnd_fini_tdrinstlist(pstTDRInstList);

	return -1;
}

int tlogd_tconnd_find_lib(LPTDRINSTLIST pstTDRInstList, const char* pszName, LPTDRMETALIB* ppstLib)
{
	int i;
	int iIsFind = 0;
	LPTDRMETALIB pstFind = NULL;

	if( !pszName || !pszName[0] )
	{
		if( ppstLib )
			*ppstLib	=	NULL;

		return 0;
	}

	for(i=0; i<pstTDRInstList->iCount; i++)
	{
		if( !strcasecmp(pszName, pstTDRInstList->astInsts[i].szName) )
		{
			pstFind	=	pstTDRInstList->astInsts[i].pstLib;
			iIsFind	=	1;
			break;
		}
	}

	if( !iIsFind )
	{
		return -1;
	}

	if( ppstLib )
		*ppstLib	=	pstFind;

	return 0;
}

int tlogd_tconnd_find_meta(TDRINSTLIST* pstTDRInstList, const char* pszName, LPTDRMETALIB pstPrefer, LPTDRMETA* ppstFind)
{
	LPTDRMETALIB pstLib;
	LPTDRMETA pstMeta;
	int iIsFind;
	int i;

	if( !pszName || !pszName[0] )
		return 0;

	if( pstPrefer )
	{
		pstMeta	=	tdr_get_meta_by_name(pstPrefer, pszName);

		if( !pstMeta )
		{
			return -1;
		}
		else
		{
			*ppstFind	=	pstMeta;
			return 0;
		}
	}

	iIsFind	=	0;

	for(i=0; i<pstTDRInstList->iCount; i++)
	{
		pstLib	=	pstTDRInstList->astInsts[i].pstLib;

		if( pstLib==pstPrefer )
			continue;

		pstMeta	=	tdr_get_meta_by_name(pstLib, pszName);

		if( pstMeta )
		{
			iIsFind	=	1;
			break;
		}
	}

	if( iIsFind )
	{
		*ppstFind	=	pstMeta;
		return 0;
	}
	else
	{
		return -1;
	}
}

static int tlogd_tconnd_init_pdulen_tdrparser_i(LPTDRMETALIB pstLib, PDUINST* pstPDUInst, PDU* pstPDU)
{
	TDRSIZEINFO stSizeInfo;
	int iRet;
	int iLen;
	LPPDULENTDRPARSERINST pstTDRParserInst;
	LPPDULENTDRPARSER pstTDRParser ;

	pstTDRParserInst = &pstPDUInst->stLenParser.stTDRParser;
	pstTDRParser = &pstPDU->stLenParser.stTDRParser;

	pstPDUInst->iMinLen	=	0;

	pstTDRParserInst->pstPkg	=	tdr_get_meta_by_name(pstLib, pstTDRParser->szPkg);
	if( !pstTDRParserInst->pstPkg )
		return -1;

	iRet = tdr_sizeinfo_to_off(&stSizeInfo, pstTDRParserInst->pstPkg, TDR_INVALID_INDEX, pstTDRParser->szPkgLen);
	if (TDR_SUCCESS == iRet)
	{
		pstTDRParserInst->iPkgLenNOff = stSizeInfo.iNOff;
		pstTDRParserInst->iPkgLenUnitSize = stSizeInfo.iUnitSize;

		iLen	=	stSizeInfo.iNOff + stSizeInfo.iUnitSize;
		if( pstPDUInst->iMinLen < iLen )  pstPDUInst->iMinLen	=	iLen;
	}
	iRet = tdr_sizeinfo_to_off(&stSizeInfo, pstTDRParserInst->pstPkg, TDR_INVALID_INDEX, pstTDRParser->szHeadLen);
	if (TDR_SUCCESS == iRet)
	{
		pstTDRParserInst->iHeadLenNOff = stSizeInfo.iNOff;
		pstTDRParserInst->iHeadLenUnitSize = stSizeInfo.iUnitSize;

		iLen	=	stSizeInfo.iNOff + stSizeInfo.iUnitSize;
		if( pstPDUInst->iMinLen < iLen )  pstPDUInst->iMinLen	=	iLen;
	}
	iRet = tdr_sizeinfo_to_off(&stSizeInfo, pstTDRParserInst->pstPkg, TDR_INVALID_INDEX, pstTDRParser->szBodyLen);
	if (TDR_SUCCESS == iRet)
	{
		pstTDRParserInst->iBodyLenNOff = stSizeInfo.iNOff;
		pstTDRParserInst->iBodyLenUnitSize = stSizeInfo.iUnitSize;

		iLen	=	stSizeInfo.iNOff + stSizeInfo.iUnitSize;
		if( pstPDUInst->iMinLen < iLen )  pstPDUInst->iMinLen	=	iLen;
	}
	if ((0 >= pstTDRParserInst->iPkgLenUnitSize) && (
		(0 >= pstTDRParserInst->iHeadLenUnitSize) || (0 >= pstTDRParserInst->iBodyLenUnitSize)))
	{
		printf("failed to get the entry contained length of PDU<%s>", pstTDRParser->szPkg);
		return -1;
	}

	iRet = tdr_sizeinfo_to_off(&stSizeInfo, pstTDRParserInst->pstPkg, TDR_INVALID_INDEX, pstTDRParser->szMsgID);
	if (TDR_SUCCESS == iRet)
	{
		pstTDRParserInst->iMsgIDNOff = stSizeInfo.iNOff;
		pstTDRParserInst->iMsgIDUnitSize = stSizeInfo.iUnitSize;

		iLen	=	stSizeInfo.iNOff + stSizeInfo.iUnitSize;
		if( pstPDUInst->iMinLen < iLen )  pstPDUInst->iMinLen	=	iLen;
	}

	iRet = tdr_sizeinfo_to_off(&stSizeInfo, pstTDRParserInst->pstPkg, TDR_INVALID_INDEX, pstTDRParser->szMsgCls);
	if (TDR_SUCCESS == iRet)
	{
		pstTDRParserInst->iMsgClsNOff = stSizeInfo.iNOff;
		pstTDRParserInst->iMsgClsUnitSize = stSizeInfo.iUnitSize;

		iLen	=	stSizeInfo.iNOff + stSizeInfo.iUnitSize;
		if( pstPDUInst->iMinLen < iLen )  pstPDUInst->iMinLen	=	iLen;
	}

	pstPDUInst->iUnit	=	tdr_get_meta_size(pstTDRParserInst->pstPkg);

	if( pstPDUInst->iUnit<pstPDU->iSize )
		pstPDUInst->iUnit	=	pstPDU->iSize;

	pstPDUInst->iUnit	=	(pstPDUInst->iUnit + 0x400 - 1)/0x400*0x400;

	pstTDRParserInst->iHeadLenMultiplex	=	pstTDRParser->iHeadLenMultiplex;
	pstTDRParserInst->iBodyLenMultiplex	=	pstTDRParser->iBodyLenMultiplex;
	pstTDRParserInst->iPkgLenMultiplex	=	pstTDRParser->iPkgLenMultiplex;

	pstPDUInst->pfnGetPkgLen = tlogd_tconnd_get_pkglen;

	return 0;
}


int tlogd_tconnd_init_pdulen_tdrparser(TDRINSTLIST* pstTDRInstList, PDUINST* pstPDUInst, PDU* pstPDU)
{
	LPTDRMETALIB pstPrefer=NULL;
	LPPDULENTDRPARSER pstTDRParser ;

	pstTDRParser = &pstPDU->stLenParser.stTDRParser;

	if( tlogd_tconnd_find_lib(pstTDRInstList, pstTDRParser->szTDR, &pstPrefer)<0 )
		return -1;

	return tlogd_tconnd_init_pdulen_tdrparser_i(pstPrefer, pstPDUInst, pstPDU);
}


int tlogd_tconnd_init_pdulen_tlogparser(PDUINST* pstPDUInst, PDU* pstPDU)
{
	LPTDRMETALIB pstPrefer=(LPTDRMETALIB)g_szMetalib_tlogd;
	LPPDULENTDRPARSER pstTDRParser ;

	pstTDRParser = &pstPDU->stLenParser.stTDRParser;

	STRNCPY(pstTDRParser->szPkg, "TLOGBinPkg", sizeof(pstTDRParser->szPkg) );
	STRNCPY(pstTDRParser->szHeadLen, "Head.HeadLen", sizeof(pstTDRParser->szHeadLen) );
	STRNCPY(pstTDRParser->szBodyLen, "Head.BodyLen", sizeof(pstTDRParser->szBodyLen) );

	STRNCPY(pstTDRParser->szMsgID, "Head.ID", sizeof(pstTDRParser->szMsgID) );
	STRNCPY(pstTDRParser->szMsgCls, "Head.Cls", sizeof(pstTDRParser->szMsgCls) );

	return tlogd_tconnd_init_pdulen_tdrparser_i(pstPrefer, pstPDUInst, pstPDU);
}


int tlogd_tconnd_init_pdulen_thttpparser(PDUINST* pstPDUInst, PDU* pstPDU)
{

	LPPDULENTHTTPPARSERINST pstThttpParserInst;
	LPPDULENTHTTPARSER pstThttpParser ;

	pstThttpParserInst = &pstPDUInst->stLenParser.stThttpParser;
	pstThttpParser = &pstPDU->stLenParser.stThttpParser;

	if (pstThttpParser->iMaxUpPkgLen > pstPDU->iSize && pstPDU->iSize > 0)
	{
		pstThttpParser->iMaxUpPkgLen = pstPDU->iSize;
	}
	if (0 >= pstThttpParser->iMaxUpPkgLen)
	{
		printf("invalid max up pkglen<%d>, please check configure of PDU<name=%s>",
			pstThttpParser->iMaxUpPkgLen, pstPDU->szName);
		return -1;
	}
	pstThttpParserInst->iMaxUpPkgLen = pstThttpParser->iMaxUpPkgLen;
	pstPDUInst->iUnit =	pstThttpParser->iMaxUpPkgLen;
	if( pstPDUInst->iUnit<pstPDU->iSize )
		pstPDUInst->iUnit	=	pstPDU->iSize;

	pstPDUInst->iUnit	=	(pstPDUInst->iUnit + 0x400 - 1)/0x400*0x400;

	pstPDUInst->pfnGetPkgLen = tlogd_tconnd_get_thttp_pkglen;
	return 0;
}

int tlogd_tconnd_init_pdulen_noneparser(PDUINST* pstPDUInst, PDU* pstPDU)
{

	LPPDULENNONEPARSERINST pstNoneParserInst;
	LPPDULENNONEPARSER pstNoneParser ;

	pstNoneParserInst = &pstPDUInst->stLenParser.stNoneParser;
	pstNoneParser = &pstPDU->stLenParser.stNoneParser;

/*
	if (pstNoneParser->iMaxPkgLen > pstPDU->iSize)
	{
		pstNoneParser->iMaxPkgLen = pstPDU->iSize;
	}

*/
	if (0 >= pstNoneParser->iMaxPkgLen)
	{
		printf("invalid max up pkglen<%d>, please check configure of PDU<name=%s>",
			pstNoneParser->iMaxPkgLen, pstPDU->szName);
		return -1;
	}
	pstNoneParserInst->iMaxPkgLen = pstNoneParser->iMaxPkgLen;
	pstPDUInst->iUnit =	pstNoneParser->iMaxPkgLen;
	if( pstPDUInst->iUnit<pstPDU->iSize )
		pstPDUInst->iUnit	=	pstPDU->iSize;

	pstPDUInst->iUnit	=	(pstPDUInst->iUnit + 0x400 - 1)/0x400*0x400;

	pstPDUInst->pfnGetPkgLen = tlogd_tconnd_get_none_pkglen;

	return 0;
}



int tlogd_tconnd_init_pduinstlist(TDRINSTLIST* pstTDRInstList, PDUINSTLIST* pstPDUInstList, PDULIST* pstPDUList)
{

	int iRet = 0;

	pstPDUInstList->iCount	=	0;
	while( pstPDUInstList->iCount<pstPDUList->iCount)
	{
		PDUINST* pstPDUInst;
		PDU* pstPDU;

		if( pstPDUInstList->iCount >= (int) (sizeof(pstPDUInstList->astInsts)/sizeof(pstPDUInstList->astInsts[0]) )  )
		{
			iRet = -1;
			break;
		}

		pstPDU = pstPDUList->astPDUs + pstPDUInstList->iCount;
		pstPDUInst = pstPDUInstList->astInsts + pstPDUInstList->iCount;
		memset(pstPDUInst, 0, sizeof(PDUINST));
		STRNCPY(pstPDUInst->szName, pstPDU->szName, sizeof(pstPDUInst->szName));
		pstPDUInst->iLenParsertype = pstPDU->iLenParsertype;


		switch(pstPDUInst->iLenParsertype)
		{
		case PDULENPARSERID_BY_TDR:
			{
				LPPDULENTDRPARSER pstTDRParser ;
				pstTDRParser = &pstPDU->stLenParser.stTDRParser;

				if( '\0'==pstTDRParser->szPkgLen[0] && '\0'==pstTDRParser->szHeadLen[0] && '\0'==pstTDRParser->szBodyLen[0] )
					iRet = tlogd_tconnd_init_pdulen_tlogparser(pstPDUInst, pstPDU);
				else
					iRet = tlogd_tconnd_init_pdulen_tdrparser(pstTDRInstList, pstPDUInst, pstPDU);

				break;
			}
		case PDULENPARSERID_BY_NULL:
			iRet = tlogd_tconnd_init_pdulen_thttpparser(pstPDUInst, pstPDU);
			break;

		case PDULENPARSERID_BY_NONE:
			iRet = tlogd_tconnd_init_pdulen_noneparser(pstPDUInst, pstPDU);
			break;

		default:
			iRet = -1;
			break;
		}/*switch(pstPDUInst->iLenParsertype)*/

		if(0 != iRet)
		{
			break;
		}
		pstPDUInstList->iCount++;
	}/*while( pstPDUInstList->iCount<pstPDUList->iCount)*/

	return iRet;
}

int tlogd_tconnd_init_lisinstlist(LISINSTLIST* pstLisInstList, LISTENERLIST* pstListenerList)
{
	LISINST* pstLisInst;
	LISTENER* pstListener;

	pstLisInstList->iCount	=	0;

	while( pstLisInstList->iCount < pstListenerList->iCount )
	{
		pstLisInst	=	pstLisInstList->astInsts + pstLisInstList->iCount;
		pstListener	=	pstListenerList->astListeners + pstLisInstList->iCount;

		memcpy(pstLisInst->szName, pstListener->szName, sizeof(pstLisInst->szName));

		pstLisInst->iRef	=	0;

		pstLisInstList->iCount++;
	}

	return 0;
}

int tlogd_tconnd_fini_lisinstlist(LISINSTLIST* pstLisInstList)
{
	LISINST* pstLisInst;
	int i;

	for(i=0; i<pstLisInstList->iCount; i++)
	{
		pstLisInst	=	pstLisInstList->astInsts + i;
		pstLisInst->iRef	=	0;
	}

	return 0;
}

int tlogd_tconnd_init_serinstlist(SERINSTLIST* pstSerInstList, SERIALIZERLIST* pstSerializerList)
{
	SERINST* pstSerInst;
	SERIALIZER* pstSerializer;
	int iRet;

	pstSerInstList->iCount	=	0;

	while( pstSerInstList->iCount < pstSerializerList->iCount )
	{
		pstSerInst	=	pstSerInstList->astInsts + pstSerInstList->iCount;
		pstSerializer=	pstSerializerList->astSerializers + pstSerInstList->iCount;

		iRet	=	tlogany_init(&pstSerInst->stLogAny, &pstSerializer->stLogger);

		memcpy(pstSerInst->szName, pstSerializer->szName, sizeof(pstSerInst->szName));

		pstSerInst->iRef	=	0;

		pstSerInstList->iCount++;
	}

	return 0;
}

int tlogd_tconnd_fini_serinstlist(SERINSTLIST* pstSerInstList)
{
	SERINST* pstSerInst;
	int i;

	for(i=0; i<pstSerInstList->iCount; i++)
	{
		pstSerInst	=	pstSerInstList->astInsts + i;

		tlogany_fini(&pstSerInst->stLogAny);

		pstSerInst->iRef	=	0;
	}

	return 0;
}

int tlogd_tconnd_load_conffile(TLOGD_TCONND* pstConnd, LPTDRMETA pstMeta, const char* pszPath)
{
	TDRDATA stInput;

	stInput.pszBuff	=	(char*)pstConnd;
	stInput.iBuff	=	(int) sizeof(*pstConnd);

	if( !pstMeta || !pszPath )
		return -1;

	if( tdr_input_file(pstMeta, &stInput, pszPath, 0, 0)<0 )
		return -1;

	return 0;
}

int tlogd_tconnd_init_transinstlist(TRANSINSTLIST* pstTransInstList, TLOGD_TCONND* pstConnd, LISINSTLIST* pstLisInstList, SERINSTLIST* pstSerInstList)
{
	NETTRANSLIST* pstNetTransList;
	NETTRANS* pstNetTrans;
	TRANSINST* pstTransInst;
	LISTENERLIST* pstListenerList;
	SERIALIZERLIST* pstSerializerList;
	TDRLIST* pstTDRList;
	PDULIST* pstPDUList;
	int iIsFind;
	int i;
	int j;

	pstNetTransList		=	&pstConnd->stNetTransList;
	pstTDRList			=	&pstConnd->stTDRList;
	pstPDUList			=	&pstConnd->stPDUList;
	pstListenerList		=	&pstConnd->stListenerList;
	pstSerializerList	=	&pstConnd->stSerializerList;

	if( pstNetTransList->iCount>(int)(sizeof(pstTransInstList->astInsts)/sizeof(pstTransInstList->astInsts[0])) )
		return -1;

	/* first, check the tdrs. */
	for(i=0; i<pstNetTransList->iCount; i++)
	{
		pstNetTrans	=	pstNetTransList->astNetTrans + i;
		pstTransInst=	pstTransInstList->astInsts + i;

		iIsFind		=	0;

		for(j=0; j<pstPDUList->iCount; j++)
		{
			if( !strcasecmp(pstNetTrans->szPDU, pstPDUList->astPDUs[j].szName) )
			{
				pstTransInst->iPDULoc	=	j;
				iIsFind	=	1;
				break;
			}
		}

		if( !iIsFind )
		{
			printf("Error: Find PDU \'%s\' failed.\n", pstNetTrans->szPDU);
			return -1;
		}

		iIsFind	=	0;

		pstTransInst->iLisCount	=	0;

		for(j=0; j<pstListenerList->iCount; j++)
		{
			if( !strcasecmp(pstNetTrans->szListener, pstListenerList->astListeners[j].szName) )
			{
				iIsFind	=	1;

				if( pstTransInst->iLisCount>=(int)(sizeof(pstTransInst->aiLisLoc)/sizeof(pstTransInst->aiLisLoc[0])) )
					break;

				if( pstLisInstList->astInsts[j].iRef>=1 )
				{
					printf("Error: Listener \'%s\' be used more than 1 times.\n", pstNetTrans->szListener);
					return -1;
				}

				pstLisInstList->astInsts[j].iRef++;

				pstTransInst->aiLisLoc[pstTransInst->iLisCount]		=	j;
				pstTransInst->iLisCount++;
			}
		}

		if( !iIsFind )
		{
			printf("Error: Find Listener \'%s\' failed.\n", pstNetTrans->szListener);
			return -1;
		}


		iIsFind	=	0;

		pstTransInst->iSerCount	=	0;

		for(j=0; j<pstSerializerList->iCount; j++)
		{
			if( !strcasecmp(pstNetTrans->szSerializer, pstSerializerList->astSerializers[j].szName) )
			{
				iIsFind	=	1;

				if( pstTransInst->iSerCount>=(int)(sizeof(pstTransInst->aiSerLoc)/sizeof(pstTransInst->aiSerLoc[0])) )
					break;

				pstSerInstList->astInsts[j].iRef++;

				pstTransInst->aiSerLoc[pstTransInst->iSerCount]		=	j;
				pstTransInst->iSerCount++;
			}
		}

		if( !iIsFind )
		{
			printf("Error: Find Serializer \'%s\' failed.\n", pstNetTrans->szSerializer);
			return -1;
		}

		pstTransInst->iLoadRatio	=	pstNetTrans->iLoadRatio;
	}

	pstTransInstList->iCount	=	pstNetTransList->iCount;

	return 0;

}

int tlogd_tconnd_fini_confinst(CONFINST* pstConfInst)
{
	assert(pstConfInst);

	return 0;
}

int tlogd_tconnd_init_confinst(CONFINST* pstConfInst, TLOGD_TCONND* pstConnd)
{
	if( tlogd_tconnd_init_tdrinstlist(&pstConfInst->stTDRInstList, &pstConnd->stTDRList)<0 )
		return -1;

	if( tlogd_tconnd_init_pduinstlist(&pstConfInst->stTDRInstList, &pstConfInst->stPDUInstList, &pstConnd->stPDUList)<0 )
		return -1;

	if( tlogd_tconnd_init_lisinstlist(&pstConfInst->stLisInstList, &pstConnd->stListenerList)<0 )
		return -1;

	if( tlogd_tconnd_init_serinstlist(&pstConfInst->stSerInstList, &pstConnd->stSerializerList)<0 )
		return -1;

	if( tlogd_tconnd_init_transinstlist(&pstConfInst->stTransInstList, pstConnd, &pstConfInst->stLisInstList, &pstConfInst->stSerInstList)<0 )
		return -1;

	return 0;
}

int tlogd_tconnd_init_tlogd_tconndrun(TLOGD_TCONNDRUN* pstRun, TLOGD_TCONND* pstConnd)
{
	int i;

	memset(pstRun, 0, sizeof(*pstRun));

	for(i=0; i<pstConnd->stListenerList.iCount;i++)
	{
		strcpy(pstRun->stListenerRunList.astListeners[i].szName, pstConnd->stListenerList.astListeners[i].szName);
	}

	for(i=0; i<pstConnd->stSerializerList.iCount;i++)
	{
		strcpy(pstRun->stSerializerRunList.astSerializers[i].szName, pstConnd->stSerializerList.astSerializers[i].szName);
	}

	for(i=0; i<pstConnd->stNetTransList.iCount;i++)
	{
		strcpy(pstRun->stNetTransRunList.astNetTrans[i].szName, pstConnd->stNetTransList.astNetTrans[i].szName);
	}

	pstRun->stNetTransRunList.iCount	=	pstConnd->stNetTransList.iCount;
	pstRun->stListenerRunList.iCount	=	pstConnd->stListenerList.iCount;
	pstRun->stSerializerRunList.iCount	=	pstConnd->stSerializerList.iCount;

	return 0;
}
