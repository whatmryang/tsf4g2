/*
**  @file $RCSfile: tconnmgr.h,v $
**  general description of this module
**  $Id: tconnmgr.h,v 1.1 2008-08-07 02:02:49 steve Exp $
**  @author $Author: steve $
**  @date $Date: 2008-08-07 02:02:49 $
**  @version $Revision: 1.1 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#ifndef TCONNMGR_H
#define TCONNMGR_H
#include "../lib_src/tlog/tlog_i.h"
#include "tdr/tdr.h"
#include "tapp/tapp.h"
#include "tlog/tlog.h"
#include "tlogddef.h"
#include "tlogdpool.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct tagTDRInstList	TDRINSTLIST;
typedef struct tagTDRInstList	*LPTDRINSTLIST;

typedef struct tagPDULenTDRParserInst	PDULENTDRPARSERINST;
typedef struct tagPDULenTDRParserInst	*LPPDULENTDRPARSERINST;

typedef struct tagPDULenTHTTParserInst	PDULENTHTTPPARSERINST;
typedef struct tagPDULenTHTTParserInst	*LPPDULENTHTTPPARSERINST;

typedef struct tagPDULenNoneParserInst	PDULENNONEPARSERINST;
typedef struct tagPDULenNoneParserInst	*LPPDULENNONEPARSERINST;

typedef union tagPDULenParserInst	PDULENPARSERINST;
typedef union tagPDULenParserInst	*LPPDULENPARSERINST;

typedef struct tagConfInst		CONFINST;
typedef struct tagConfInst		*LPCONFINST;

typedef struct tagTConnThread	TLOGDTHREAD;
typedef struct tagTConnThread	*LPTLOGDTHREAD;

typedef int (*PFNTCONND_GET_PKGLEN)(TCONNINST* pstInst, LPTLOGDTHREAD pstThread);


struct tagTDRInst
{
	char szName[TCONND_NAME_LEN];
	LPTDRMETALIB pstLib;
};

typedef struct tagTDRInst		TDRINST;
typedef struct tagTDRInst		*LPTDRINST;

struct tagTDRInstList
{
	int iCount;
	TDRINST astInsts[TCONND_MAX_TDR];
};




/* ʹ��TDR�������������ݰ��ĳ��� */
struct tagPDULenTDRParserInst
{
	LPTDRMETA pstPkg;

	int iPkgLenNOff;	/*��¼PDU���ȳ�Ա���PDU�ܽṹ��ƫ��*/
	int iPkgLenUnitSize;	/*��¼PDU���ȳ�Ա�Ĵ洢�ռ�*/
	int iHeadLenNOff;	/*��¼PDUͷ�����ȳ�Ա���PDU�ܽṹ��ƫ��*/
	int iHeadLenUnitSize;	/*��¼PDUͷ�����ȳ�Ա�Ĵ洢�ռ�*/
	int iBodyLenNOff;	/*��¼PDU��Ϣ�峤�ȳ�Ա���PDU�ܽṹ��ƫ��*/
	int iBodyLenUnitSize;	/*��¼PDU��Ϣ�峤�ȳ�Ա�Ĵ洢�ռ�*/

	int iMsgIDNOff;
	int iMsgIDUnitSize;
	int iMsgClsNOff;
	int iMsgClsUnitSize;

	int iHeadLenMultiplex;
	int iBodyLenMultiplex;
	int iPkgLenMultiplex;           
};



/* ͨ�������г���NULL('\0')�ַ��������� */
struct tagPDULenTHTTParserInst
{
	int iMaxUpPkgLen;                    	/*   ����PDU���ݰ���󳤶� */
};

struct tagPDULenNoneParserInst
{
	int iMaxPkgLen;                    	/*   ����PDU���ݰ���󳤶� */
};


/* ����Э�����ݵ�Ԫ(PDU)������Ϣ�����ݽṹ�����ڽ��������ֽ��Ӧ�ö��������ͨ����Ϣ */
union tagPDULenParserInst
{
	PDULENTDRPARSERINST stTDRParser;                     	/* PDULENPARSERID_BY_TDR,  ʹ��TDR�������з��� */
	PDULENTHTTPPARSERINST stThttpParser;                   	/* PDULENPARSERID_BY_NULL,  ͨ�������г���NULL('\0')�ַ��������� */
	PDULENNONEPARSERINST stNoneParser;
};

struct tagPDUInst
{
	char szName[TCONND_NAME_LEN];
	int iUnit;
	int iMinLen;
	int iLenParsertype;                  	/*  Ver.11  Bind Macrosgroup:PDULenParserID,*/
	PDULENPARSERINST stLenParser;                     	/*  Ver.11 ����Э�����ݵ�Ԫ(PDU)������Ϣ�ĳ�Ա */
	PFNTCONND_GET_PKGLEN pfnGetPkgLen; /*�������ݰ����ȵĻص�����*/
};

typedef struct tagPDUInst		PDUINST;
typedef struct tagPDUInst		*LPPDUINST;

struct tagPDUInstList
{
	int iCount;
	PDUINST astInsts[TCONND_MAX_PDU];
};

typedef struct tagPDUInstList	PDUINSTLIST;
typedef struct tagPDUInstList	*LPPDUINSTLIST;

struct tagTransInst
{
	char szName[TCONND_NAME_LEN];
	int iLoadRatio;
	int iPDULoc;
	int iLisCount;
	int aiLisLoc[TCONND_MAX_NETTRANS];
	int iSerCount;
	int aiSerLoc[TCONND_MAX_NETTRANS];
};

typedef struct tagTransInst		TRANSINST;
typedef struct tagTransInst		*LPTRANSINST;

struct tagTransInstList
{
	int iCount;
	TRANSINST astInsts[TCONND_MAX_NETTRANS];
};

typedef struct tagTransInstList	TRANSINSTLIST;
typedef struct tagTransInstList	*LPTRANSINSTLIST;

struct tagLisInst
{
	char szName[TCONND_NAME_LEN];
	int iRef;
	int iRes;
};

typedef struct tagLisInst		LISINST;
typedef struct tagLisInst		*LPLISINST;

struct tagLisInstList
{
	int iCount;
	LISINST astInsts[TCONND_MAX_NETTRANS];
};

typedef struct tagLisInstList	LISINSTLIST;
typedef struct tagLisInstList	*LPLISINSTLIST;


struct tagSerInst
{
	char szName[TCONND_NAME_LEN];
	TLOGANY stLogAny;
	int iRef;
};

typedef struct tagSerInst		SERINST;
typedef struct tagSerInst		*LPSERINST;

struct tagSerInstList
{
	int iCount;
	SERINST astInsts[TCONND_MAX_NETTRANS];
};

typedef struct tagSerInstList	SERINSTLIST;
typedef struct tagSerInstList	*LPSERINSTLIST;

struct tagConfInst
{
	TDRINSTLIST stTDRInstList;
	PDUINSTLIST stPDUInstList;
	LISINSTLIST	stLisInstList;
	SERINSTLIST	stSerInstList;
	TRANSINSTLIST stTransInstList;
};


struct tagTConnThread
{
	int iID;
	int iIsValid;
	void *iIsExit;
	int epfd;
	pthread_t tid;
	CONFINST* pstConfInst;
	TCONNPOOL* pstPool;
	int iTickCount;
	int iPkgUnit;
	int iPoolUnit;
	TAPPCTX* pstAppCtx;
	int iScanPos;
	int iMsRecv;	/* the milli-second usecd to receive message. */
	int iMsSend;	/* the milli-second usecd to send message. */
	char* pszBuff;
	int iBuff;
};



int tlogd_tconnd_fini_tdrinstlist(TDRINSTLIST* pstTDRInstList);
int tlogd_tconnd_init_tdrinstlist(TDRINSTLIST* pstTDRInstList, TDRLIST* pstTDRList);
int tlogd_tconnd_find_lib(TDRINSTLIST* pstTDRInstList, const char* pszName, LPTDRMETALIB* ppstLib);
int tlogd_tconnd_find_meta(TDRINSTLIST* pstTDRInstList, const char* pszName, LPTDRMETALIB pstPrefer, LPTDRMETA* ppstFind);
int tlogd_tconnd_init_pduinst(TDRINSTLIST* pstTDRInstList, PDUINST* pstPDUInst, PDU* pstPDU);
int tlogd_tconnd_init_pduinstlist(TDRINSTLIST* pstTDRInstList, PDUINSTLIST* pstPDUInstList, PDULIST* pstPDUList);
int tlogd_tconnd_load_conffile(TLOGD_TCONND* pstConnd, LPTDRMETA pstMeta, const char* pszPath);
int tlogd_tconnd_init_lisinstlist(LISINSTLIST* pstLisInstList, LISTENERLIST* pstListenerList);
int tlogd_tconnd_fini_lisinstlist(LISINSTLIST* pstLisInstList);
int tlogd_tconnd_init_serinstlist(SERINSTLIST* pstSerInstList, SERIALIZERLIST* pstSerializerList);
int tlogd_tconnd_fini_serinstlist(SERINSTLIST* pstSerInstList);
int tlogd_tconnd_init_transinstlist(TRANSINSTLIST* pstTransInstList, TLOGD_TCONND* pstConnd, LISINSTLIST* pstLisInstList, SERINSTLIST* pstSerInstList);
int tlogd_tconnd_fini_confinst(CONFINST* pstConfInst);
int tlogd_tconnd_init_confinst(CONFINST* pstConfInst, TLOGD_TCONND* pstConnd);

int tlogd_tconnd_init_tlogd_tconndrun(TLOGD_TCONNDRUN* pstRun, TLOGD_TCONND* pstConnd);

#ifdef __cplusplus
}
#endif

#endif /* TCONNMGR_H */
