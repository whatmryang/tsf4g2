#ifndef TBUS_VIEW_CHANNEL_H
#define TBUS_VIEW_CHANNEL_H

#include <stdio.h>
#include "tdr/tdr.h"
#include "tbus/tbus_macros.h"
#include "tbus_mgr.h"
#include "tbus/tbus_config_mng.h"

#ifdef __cplusplus
extern "C" {
#endif


struct tagViewChannelTools
{
	LPTDRMETALIB pstLib;
	LPTDRMETA	pstMsgMeta;
	char *pszBuff;	/*�洢�������ݵĻ�����*/
	int iSize;	/*��������С*/
	FILE *fpOut;
	int iMaxSeeMsg;
	int iCurSeeMsg;
    int iDumpBinary;
    int iWithSep;
    int iStartMsgIndex;
};

typedef struct tagViewChannelTools  VIEWCHANNELTOOLS;
typedef struct tagViewChannelTools  *LPVIEWCHANNELTOOLS;

int ViewSimpleChannel(LPTBUSMGROPTIONS a_pstOption);
int ViewChannels(IN LPTBUSSHMGCIM a_pShmGCIM, LPTBUSMGROPTIONS a_pstOption);
int ViewAllChannels(IN LPTBUSSHMGCIM a_pShmGCIM, LPTBUSMGROPTIONS a_pstOption);
#if !defined(_WIN32) && !defined(_WIN64)
int ViewChannelByShmID(LPTBUSMGROPTIONS a_pstOption);
#endif


#ifdef __cplusplus
}
#endif

#endif
