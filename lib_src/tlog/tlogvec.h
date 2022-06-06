/*
**  @file $RCSfile: tlogvec.h,v $
**  general description of this module
**  $Id: tlogvec.h,v 1.1 2008-08-05 07:15:26 steve Exp $
**  @author $Author: steve $
**  @date $Date: 2008-08-05 07:15:26 $
**  @version $Revision: 1.1 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#ifndef TLOGVEC_H
#define TLOGVEC_H

#include "tlogio.h"
#include "tlogvecdef.h"

#ifdef __cplusplus
extern "C"
{
#endif

union tagTLogFileOrNet
{
	TLOGFILE stFile;
	TLOGNET stNet;
};

typedef union tagTLogFileOrNet		TLOGFILEORNET;
typedef union tagTLogFileOrNet		*LPTLOGFILEORNET;

struct tagTLogElement
{
	int iIndex;
	int iType;
	TLOGFILEORNET stFileOrNet;
};

typedef struct tagTLogElement		TLOGELEMENT;
typedef struct tagTLogElement		*LPTLOGELEMENT;

struct tagTLogDevVecInst
{
	int iCount;
	TLOGELEMENT astElements[TLOGVEC_MAX_ELEMENT];
};

typedef struct tagTLogDevVecInst	TLOGDEVVECINST;
typedef struct tagTLogDevVecInst	*LPTLOGDEVVECINST;

struct tagTLogVec
{
	TLOGDEVVEC* pstDev;
	TLOGDEVVECINST stInst;
};

typedef struct tagTLogVec		TLOGVEC;
typedef struct tagTLogVec		*LPTLOGVEC;

int tlogvec_init(TLOGVEC* a_pstLogVec, TLOGDEVVEC* a_pstDev);
int tlogvec_fini(TLOGVEC* a_pstLogVec);

int tlogvec_write_element(TLOGELEMENT* a_pstLogElement, const char* a_pszBuff, int a_iBuff);
int tlogvec_write_elementv(TLOGELEMENT* a_pstLogElement, const TLOGIOVEC* a_pstIOVec, int a_iCount);

int tlogvec_write(TLOGVEC* a_pstLogVec, int a_iID, int a_iCls, const char* a_pszBuff, int a_iBuff);
int tlogvec_writev(TLOGVEC* a_pstLogVec, int a_iID, int a_iCls, const TLOGIOVEC* a_pstIOVec, int a_iCount);

#ifdef __cplusplus
}
#endif

#endif /* TLOGVEC_H */


