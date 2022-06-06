#ifndef TBUS_MGR_H
#define TBUS_MGR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "tdr/tdr.h"
#include "tbus/tbus_macros.h"
#include "tbus/tbus_comm.h"
#include "tbus/tbus_desc.h"

#define TBUS_MGR_PATH_LEN 512
#define TBUS_MGR_DEFAULT_SEE_MAX_NUM  10

enum tagTbusMgrOp
{
	TBUS_MGR_OP_WRITE = 1,
	TBUS_MGR_OP_VIEW = 2,
	TBUS_MGR_OP_DELETE=3,
	TBUS_MGR_OP_LIST= 4,
    TBUS_MGR_OP_DELETE_ALL=5,
    TBUS_MGR_OP_VIEW_ALL=6,
    TBUS_MGR_OP_CLEAN=7,
    TBUS_MGR_OP_SEND_TEST_MSG=8,
    TBUS_MGR_OP_SIMPLE_CHNNL=9,
    TBUS_MGR_OP_STRING_ERROR=10,
#if !defined(_WIN32) && !defined(_WIN64)
    TBUS_MGR_OP_VIEW_CHNNL=11,
    TBUS_MGR_OP_CAL_CHNNL_SIZE=12,
#endif
    TBUS_MGR_OP_DISABLE_CHNNL=13,
    TBUS_MGR_OP_ENABLE_CHNNL=14,
};

struct tagTbusmgrOptions
{
	char szConfFile[TBUS_MGR_PATH_LEN];
	int iMgrOP;
	char szProcID[TBUS_MGR_PATH_LEN];
	char szMetalibFile[TBUS_MGR_PATH_LEN];
	char szMetaName[TDR_NAME_LEN];
	int iDelID;
    int iBusiID;
    int iTestDataLen;
	char szTbusSrcAddr[TBUS_MGR_PATH_LEN];
	char szTbusDstAddr[TBUS_MGR_PATH_LEN];
	char szShmKey[TBUS_MAX_KEY_STRING_LEN];
	char szOutFile[TBUS_MGR_PATH_LEN];
	int iMaxSeeMsg;	/*相信察看的最大消息数*/
	int iIsOldCnf;	/*是否是老格式的配置文件，其值为非零值表示老格式的配置文件，否则为新格式*/
	int iIsDebug; /*如果值不是0，打印调试信息*/
    int iShmID;
    int iSendBufSize;
    int iRecvBufSize;
    int iAlignSize;
    int iDumpBinary;
    int iWithSep;
    int iStartMsgIndex;
    int iSetGCIMFlag;
    const char* pszBackFile;
};



typedef struct tagTbusmgrOptions  TBUSMGROPTIONS;
typedef struct tagTbusmgrOptions  *LPTBUSMGROPTIONS;

#ifdef __cplusplus
}
#endif


#endif
