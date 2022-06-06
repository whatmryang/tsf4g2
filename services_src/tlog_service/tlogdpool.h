/*
**  @file $RCSfile: tconnpool.h,v $
**  general description of this module
**  $Id: tconnpool.h,v 1.1 2008-08-07 02:02:49 steve Exp $
**  @author $Author: steve $
**  @date $Date: 2008-08-07 02:02:49 $
**  @version $Revision: 1.1 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#ifndef TCONNPOOL_H
#define TCONNPOOL_H

#include "pal/pal.h"
#include "comm/tmempool.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct tagTConnInst		TCONNINST;
typedef struct tagTConnInst		*LPTCONNINST;



struct tagTConnInst
{
	int fValid;
	short fListen;
	short fStream;
	int iIdx;
	int iID;

	int s;
	int iLisLoc;
	int iTransLoc;
	int iNeedFree;

	int iBuff;
	int iOff;
	int iData;
	int fWaitFirstPkg;

	int iMinHeadLen;
	int iHeadLen;
	int iPkgLen;
	int iFailedMsg;

	int iSendMsg;
	int iRecvMsg;
	int iSendByte;
	int iRecvByte;

	struct sockaddr stAddr;
	char szRes1[32-sizeof(struct sockaddr)];

	time_t tLastRecv;
	char szRes2[16-sizeof(time_t)];
	
	int iMsgID;
	int iMsgCls;
	int iRes3;
	int iRes4;

	char szBuff[1];
};



typedef TMEMPOOL				TCONNPOOL;
typedef TMEMPOOL				*LPTCONNPOOL;

#define tlogd_tconnd_init_pool(ppstPool, iMax, iUnit)	tmempool_new(ppstPool, iMax, iUnit)
#define tlogd_tconnd_fini_pool(ppstPool)				tmempool_destroy(ppstPool)
#define tlogd_tconnd_get_inst(pstPool, iIdx)			(TCONNINST*)tmempool_get(pstPool, iIdx)
#define tlogd_tconnd_alloc_inst(pstPool)				tmempool_alloc(pstPool)
#define tlogd_tconnd_free_inst(pstPool, iIdx)			tmempool_free(pstPool, iIdx)

#ifdef __cplusplus
}
#endif

#endif /* TCONNPOOL_H */

