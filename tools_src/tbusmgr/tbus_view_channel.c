#include <assert.h>
#include "tbus_mgr.h"
#include "tbus/tbus.h"
#include "tbus/tbus_addr_templet.h"
#include "tbus/tbus_kernel.h"
#include "tbus/tbus_misc.h"
#include "tbus/tbus_comm.h"
#include "tbus_view_channel.h"
#include "tbus/tbus_channel.h"

#if defined(_WIN32) || defined(_WIN64)
#define PTR_OR_INT "%p"
#else
#define PTR_OR_INT "%d"
#endif

int toHexStr(FILE* fpOut, const char* beginPtr, size_t dataLen, int iWithSep);
void DumpBusInterface(int iBusHandle, int iProcID, LPVIEWCHANNELTOOLS pstTool);
int DumpBusChannel(LPTBUSCHANNEL pstChannel, LPVIEWCHANNELTOOLS pstTool);
int DumpSimpleChannel(LPCHANNELHEAD pstHead, LPVIEWCHANNELTOOLS pstTool);
void DumpBusVar(CHANNELHEAD *a_pstHead ,LPCHANNELVAR pstVar, char *pszData, LPVIEWCHANNELTOOLS pstTool);
int PrintBuffMsg(CHANNELHEAD *a_pstHead ,LPCHANNELVAR a_pstVar, const char* pszData, LPVIEWCHANNELTOOLS pstTool);
int AttachChannel( IN TBUSSHMCHANNELCNF *a_pstShmChl, INOUT TBUSCHANNEL *a_ptChannel, IN LPTBUSSHMGCIMHEAD a_pstHead);
int tbus_open_mmap_i(INOUT void **ppShm, IN key_t a_tKey, IN unsigned a_uAddr1, IN unsigned a_uAddr2,
        IN int a_iUseOnlyKey, IN unsigned int *a_piSize, IN int a_iFlag, OUT int *a_piCreate, OUT HANDLE* a_piMMapID);

static int constructViewTool(LPVIEWCHANNELTOOLS a_pstViewTool, LPTBUSMGROPTIONS a_pstOption);


///////////////////////////////////////////////////
int ViewSimpleChannel(LPTBUSMGROPTIONS a_pstOption)
{
	int iRet =0;
	VIEWCHANNELTOOLS stTools;
    int iShmkey;
    TBUSCHANNEL stChannel;


    memset(&stChannel, 0, sizeof(stChannel));

	assert(NULL != a_pstOption);

    if ('\0' == a_pstOption->szShmKey[0])
    {
        printf("Error: shm key not specified, please add --bus-key=shm_key\n");
        return -1;
    }

	memset(&stTools, 0, sizeof(stTools));

    stTools.iDumpBinary = a_pstOption->iDumpBinary;
    stTools.iWithSep = a_pstOption->iWithSep;
    stTools.iStartMsgIndex = a_pstOption->iStartMsgIndex;

	if ('\0' != a_pstOption->szMetalibFile[0])
	{
		iRet = tdr_load_metalib(&stTools.pstLib, a_pstOption->szMetalibFile);
		if (TDR_SUCCESS != iRet)
		{
			stTools.pstLib = NULL;
			printf("warnsing: failed to load metalib %s, for %s\n", a_pstOption->szMetalibFile, tdr_error_string(iRet));
		}
	}

	if (('\0' != a_pstOption->szMetaName[0]) && (NULL != stTools.pstLib))
	{
		stTools.pstMsgMeta = tdr_get_meta_by_name(stTools.pstLib, a_pstOption->szMetaName);
		if (NULL == stTools.pstMsgMeta)
		{
			printf("warnning: failed to get meta by name %s\n", a_pstOption->szMetaName);
		}else
		{
			stTools.iSize = tdr_get_meta_size(stTools.pstMsgMeta);
			stTools.pszBuff = (char *)malloc(stTools.iSize);
		}
	}/*if (('\0' != a_pstOption->szMetaName[0]) && (NULL != stTools.pstLib))*/
	if ('\0' != a_pstOption->szOutFile[0])
	{
		stTools.fpOut = fopen(a_pstOption->szOutFile, "w");
		if (NULL == stTools.fpOut)
		{
			stTools.fpOut = stdout;
		}
	}else
	{
		stTools.fpOut = stdout;
	}
	stTools.iMaxSeeMsg = a_pstOption->iMaxSeeMsg;

    iRet = tbus_gen_shmkey(&iShmkey, a_pstOption->szShmKey, a_pstOption->iBusiID);
    if (0 > iRet)
    {
        printf("Error: failed to gen shm key by \'%s\' and businessid<%d>\n",
                a_pstOption->szShmKey, a_pstOption->iBusiID);
        return -1;
    }

    iRet = tbus_channel_get(iShmkey, &stChannel);
	if (0 > iRet)
	{
        printf("Error: failed to get shm by key <%d>\n", iShmkey);
		return -1;
	}

    stChannel.iRecvSide = 0;
    stChannel.iSendSide = 1;
    printf("\nKey: %d\n", iShmkey);
    DumpBusChannel(&stChannel, &stTools);
    printf("\n");

	if (stdout != stTools.fpOut)
	{
		fclose(stTools.fpOut);
	}
	if (NULL != stTools.pszBuff)
	{
		free(stTools.pszBuff);
	}

	return 0;
}

#if !defined(_WIN32) && !defined(_WIN64)
int ViewChannelByShmID(LPTBUSMGROPTIONS a_pstOption)
{
    int iRet = 0;
    int iHeadLen = 0;
    char *sShm = NULL;
    TBUSCHANNEL stChannel;
    VIEWCHANNELTOOLS stViewTool;

    assert(NULL != a_pstOption);

    sShm = shmat(a_pstOption->iShmID, NULL , 0);
    if (NULL == sShm)
    {
        printf("error: failed to attach share momory(id:%d), for %s",
                 a_pstOption->iShmID, strerror(errno));
        return -1;
    }

    memset(&stChannel, 0, sizeof(stChannel));
    stChannel.pstHead = (CHANNELHEAD *)sShm;
    if (TBUS_CHANNEL_MAGIC != stChannel.pstHead->dwMagic)
    {
        fprintf(stderr, "error: magic<%x> in shared memory is NOT equal to expected value<%d>\n",
                stChannel.pstHead->dwMagic, TBUS_CHANNEL_MAGIC);
        return -1;
    }

    stChannel.iRecvSide = 0;
    stChannel.iSendSide = 1;

	iHeadLen = sizeof(CHANNELHEAD);
	TBUS_CALC_ALIGN_VALUE(iHeadLen, a_pstOption->iAlignSize);
    stChannel.pszQueues[0] = (char*)stChannel.pstHead + iHeadLen;
    stChannel.pszQueues[1] = stChannel.pszQueues[0] + stChannel.pstHead->astQueueVar[0].dwSize;

    constructViewTool(&stViewTool, a_pstOption);

	fprintf(stViewTool.fpOut, "================================================\n");
	fprintf(stViewTool.fpOut, "Channel<ShmID:%d>\n", a_pstOption->iShmID);

    DumpBusChannel(&stChannel, &stViewTool);

    return iRet;
}
#endif

int ViewChannels(IN LPTBUSSHMGCIM a_pShmGCIM, LPTBUSMGROPTIONS a_pstOption)
{
	int iRet =0;
	int iBusHandle;
	TBUSADDR iSrcAddr;
	VIEWCHANNELTOOLS stTools;

	assert(NULL != a_pShmGCIM);
	assert(NULL != a_pstOption);

	if ('\0' == a_pstOption->szProcID[0])
	{
		printf("Error: processid not specified, use --view=proc option.\n");
		return -1;
	}

	iRet = tbus_initialize(a_pstOption->szShmKey, a_pShmGCIM->stHead.dwBusiID);
	if (TBUS_SUCCESS != iRet)
	{
		printf("tbus init error\n");
		return -1;
	}

	iRet = tbus_new(&iBusHandle);
	if (TBUS_SUCCESS != iRet)
	{
        tbus_fini();
		printf("tbus new error\n");
		return -1;
	}

	iRet = tbus_addr_aton(&a_pstOption->szProcID[0], &iSrcAddr);
	if (TBUS_SUCCESS != iRet)
	{
        tbus_delete(&iBusHandle);
        tbus_fini();
		printf("failed to convert %s to tbus address\n", a_pstOption->szProcID);
		return -1;
	}
	iRet = tbus_bind(iBusHandle, iSrcAddr);
	if (TBUS_SUCCESS != iRet)
	{
        tbus_delete(&iBusHandle);
        tbus_fini();
		printf("tbus bind error by addr %s\n", a_pstOption->szProcID);
		return -1;
	}

	memset(&stTools, 0, sizeof(stTools));

    stTools.iDumpBinary = a_pstOption->iDumpBinary;
    stTools.iWithSep = a_pstOption->iWithSep;
    stTools.iStartMsgIndex = a_pstOption->iStartMsgIndex;

	if ('\0' != a_pstOption->szMetalibFile[0])
	{
		iRet = tdr_load_metalib(&stTools.pstLib, a_pstOption->szMetalibFile);
		if (TDR_SUCCESS != iRet)
		{
			stTools.pstLib = NULL;
			printf("warnsing: failed to load metalib %s, for %s\n", a_pstOption->szMetalibFile, tdr_error_string(iRet));
		}
	}

	if (('\0' != a_pstOption->szMetaName[0]) && (NULL != stTools.pstLib))
	{
		stTools.pstMsgMeta = tdr_get_meta_by_name(stTools.pstLib, a_pstOption->szMetaName);
		if (NULL == stTools.pstMsgMeta)
		{
			printf("warnning: failed to get meta by name %s\n", a_pstOption->szMetaName);
		}else
		{
			stTools.iSize = tdr_get_meta_size(stTools.pstMsgMeta);
			stTools.pszBuff = (char *)malloc(stTools.iSize);
		}
	}/*if (('\0' != a_pstOption->szMetaName[0]) && (NULL != stTools.pstLib))*/

	if ('\0' != a_pstOption->szOutFile[0])
	{
		stTools.fpOut = fopen(a_pstOption->szOutFile, "w");
		if (NULL == stTools.fpOut)
		{
			stTools.fpOut = stdout;
		}
	}else
	{
		stTools.fpOut = stdout;
	}
	stTools.iMaxSeeMsg = a_pstOption->iMaxSeeMsg;

	DumpBusInterface(iBusHandle, iSrcAddr, &stTools);

	if (stdout != stTools.fpOut)
	{
		fclose(stTools.fpOut);
	}
	if (NULL != stTools.pszBuff)
	{
		free(stTools.pszBuff);
	}

    tbus_delete(&iBusHandle);
    tbus_fini();

	return 0;
}

void DumpBusInterface(int iBusHandle, int iProcID, LPVIEWCHANNELTOOLS pstTool)
{
	int i;
	TBUSHANDLE *pstHandle;

	assert(NULL != pstTool);

	pstHandle = TBUS_GET_HANDLE( iBusHandle ) ;
	if ( NULL == pstHandle)
	{
		printf("DumpBusInterface failed, invalid handle  %d\n", iBusHandle);
		return  ;
	}

	fprintf(pstTool->fpOut, "================================================\n");
	fprintf(pstTool->fpOut, "Total bind channel: %d, self:[%s].\n", pstHandle->dwChannelCnt, inet_ntoa(*(struct in_addr*)&iProcID));

	for(i=0; i< (int)pstHandle->dwChannelCnt; i++)
	{
		DumpBusChannel(pstHandle->pastChannelSet[i], pstTool);
	}
}


int toHexStr(FILE* fpOut, const char* beginPtr, size_t dataLen, int iWithSep)
{
    size_t i = 0;
    static char hexTab[16] =
    {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
    };

    if (NULL == fpOut || NULL == beginPtr)
    {
        printf("Error: toHexStr failed, for fpOut or beginPtr is NULL\n");

        return -1;
    }

    if (iWithSep)
    {
        for (i = 0; i < dataLen; i++)
        {
            unsigned char low  = *(beginPtr + i) & 0X0F;
            unsigned char high = *(unsigned char*)(beginPtr + i) >> 4;
            fprintf(fpOut, "%c", hexTab[high]);
            fprintf(fpOut, "%c", hexTab[low]);
            fprintf(fpOut, " ");
        }
    } else
    {
        for (i = 0; i < dataLen; i++)
        {
            unsigned char low  = *(beginPtr + i) & 0X0F;
            unsigned char high = *(unsigned char*)(beginPtr + i) >> 4;
            fprintf(fpOut, "%c", hexTab[high]);
            fprintf(fpOut, "%c", hexTab[low]);
        }
    }
    fprintf(fpOut, "\n");

    return 0;
}


int PrintBuffMsg(CHANNELHEAD *a_pstHead ,LPCHANNELVAR a_pstVar, const char* pszData, LPVIEWCHANNELTOOLS pstTool)
{
	TBUSHEAD stHead;
	int iData;
	TDRDATA stHostInfo;
	TDRDATA stNetInfo;
	int iPkg;
	int iMsg=0;
	int iRet =0;
	const char *pszPkg = NULL;
	int iSize;
	int iHead;
	int iTail;

#define  TBUS_PRINT_MSG_SLEEP_GAP	20

	assert(NULL != pszData);
	assert(NULL != pstTool);
	assert(NULL != a_pstVar);
	assert(NULL != a_pstHead);

	iHead = a_pstVar->dwHead;
	iTail = a_pstVar->dwTail;
	iSize = a_pstVar->dwSize;
	if( iHead==iTail )
		return 0;

	iData = iTail - iHead;
	if (iData< 0) iData	+=	iSize;



	while( iData>0 )
	{
		if (TBUS_SUCCESS != tbus_get_pkghead(&stHead, pszData, iSize, (unsigned int *)&iHead, iTail))
		{
			break;
		}


		iPkg	=	stHead.bHeadLen + stHead.iBodyLen;
		if( iData < iPkg )
			break;

		assert(0 <= iHead);
		pszPkg = pszData + iHead + stHead.bHeadLen;

        if (iMsg + 1 < pstTool->iStartMsgIndex)
        {
            TBUS_MOVETO_NEXT_PKG(iRet, iPkg, a_pstHead, iSize, iHead, iTail);
            iMsg++;
            continue;
        }


		/*解析数据包*/
		if (pstTool->iCurSeeMsg >= pstTool->iMaxSeeMsg)
		{
			break;
		}
		fprintf(pstTool->fpOut, "Msg[%d]:\n", iMsg+1);
        if (pstTool->iDumpBinary)
        {
            toHexStr(pstTool->fpOut, pszPkg, stHead.iBodyLen, pstTool->iWithSep);
        }
        if (NULL != pstTool->pstMsgMeta)
        {
            stHostInfo.iBuff = pstTool->iSize;
            stHostInfo.pszBuff = pstTool->pszBuff;
            stNetInfo.pszBuff = (char *)pszPkg;
            stNetInfo.iBuff = stHead.iBodyLen;
            iRet = tdr_ntoh(pstTool->pstMsgMeta, &stHostInfo, &stNetInfo, 0);
            if (0 < stHostInfo.iBuff)
            {
                tdr_fprintf(pstTool->pstMsgMeta, pstTool->fpOut, &stHostInfo, 0);
            }else
            {
                printf("failed to unpack pkg(len:%d):%s\n", stHead.iBodyLen, tdr_error_string(iRet));
            }
        }
		pstTool->iCurSeeMsg++;

		TBUS_MOVETO_NEXT_PKG(iRet, iPkg, a_pstHead, iSize, iHead, iTail);
		iData	-=	iPkg;
		iMsg++;
		if ((iMsg % TBUS_PRINT_MSG_SLEEP_GAP) == 0)
		{
			usleep(1000);
		}
	}



	return iMsg;
}

void DumpBusVar(CHANNELHEAD *a_pstHead ,LPCHANNELVAR pstVar, char *pszData, LPVIEWCHANNELTOOLS pstTool)
{
	int iMsg=0;
	int iDataLen=0;
	int iHSeq = 0;
	int iGSeq = 0;
	TBUSHEAD stHead;

	assert(NULL != pstVar);
	assert(NULL != pszData);
	assert(NULL != pstTool);
	assert(NULL != a_pstHead);

	iDataLen = pstVar->dwTail - pstVar->dwHead;
	if (iDataLen < 0)
	{
		iDataLen += pstVar->dwSize;
	}

	if ( (0 < iDataLen) &&
		(TBUS_SUCCESS == tbus_get_pkghead(&stHead, pszData, pstVar->dwSize, (unsigned int*)&pstVar->dwHead, pstVar->dwTail)))
	{
		iHSeq = stHead.iSeq;
	}
	if ((0 != pstVar->chGStart) && (0 < iDataLen) &&
		(TBUS_SUCCESS == tbus_get_pkghead(&stHead, pszData, pstVar->dwSize, &pstVar->dwGet, pstVar->dwTail)))
	{
		iGSeq = stHead.iSeq;
	}

	/*设计更改：统计通道上的消息数只需将队列上的序列号减去队列头部第一个消息的序列号即可，因为队列中的
	每一个消息都有序列号，且序列号是递增的*/
	//iMsg = CalcBuffMsg(pszData, pstVar->dwSize, pstVar->dwHead, pstVar->dwTail);
	if (0 < iDataLen)
	{
		iMsg = pstVar->iSeq - iHSeq;
	}else
	{
		iMsg = 0;
	}

	fprintf(pstTool->fpOut, "{MsgNum:%d, Bytes:%d H:%u/T:%u G: %u Size:%u HSeq: %u GSeq: %u} seq=%u rseq=%u Align:%d\n",
		iMsg, iDataLen, pstVar->dwHead, pstVar->dwTail, pstVar->dwGet, pstVar->dwSize,
		iHSeq, iGSeq, pstVar->iSeq, pstVar->iRecvSeq,
		1<<a_pstHead->dwAlignLevel);

	/*尝试打印通道中的数据*/
	if ((0 < iMsg) &&
        ((pstTool->iDumpBinary)
         || ((NULL != pstTool->pstMsgMeta) && (NULL != pstTool->pszBuff) && (0 < pstTool->iSize))
         )
        )
	{
		PrintBuffMsg(a_pstHead, pstVar, pszData, pstTool);
	}

}

//void DumpOnlyBusVar(CHANNELHEAD *a_pstHead ,LPCHANNELVAR pstVar, LPVIEWCHANNELTOOLS pstTool)
//{
//	int iDataLen=0;
//
//	assert(NULL != pstVar);
//	assert(NULL != pstTool);
//	assert(NULL != a_pstHead);
//
//	iDataLen = pstVar->dwTail - pstVar->dwHead;
//	if (iDataLen < 0)
//	{
//		iDataLen += pstVar->dwSize;
//	}
//
//	/*设计更改：统计通道上的消息数只需将队列上的序列号减去队列头部第一个消息的序列号即可，因为队列中的
//	每一个消息都有序列号，且序列号是递增的*/
//	//iMsg = CalcBuffMsg(pszData, pstVar->dwSize, pstVar->dwHead, pstVar->dwTail);
//	if (0 < iDataLen)
//	{
//        fprintf(pstTool->fpOut, "{ MsgNum: unknown, Bytes:%d H:%u/T:%u G: %u Size:%u } seq=%u rseq=%u Align:%d\n",
//                iDataLen, pstVar->dwHead, pstVar->dwTail, pstVar->dwGet, pstVar->dwSize,
//                pstVar->iSeq, pstVar->iRecvSeq, 1<<a_pstHead->dwAlignLevel);
//	}
//    else
//    {
//        fprintf(pstTool->fpOut, "{ MsgNum:0, Bytes:%d H:%u/T:%u G: %u Size:%u } seq=%u rseq=%u Align:%d\n",
//                iDataLen, pstVar->dwHead, pstVar->dwTail, pstVar->dwGet, pstVar->dwSize,
//                pstVar->iSeq, pstVar->iRecvSeq, 1<<a_pstHead->dwAlignLevel);
//    }
//
//}

//int DumpSimpleChannel(LPCHANNELHEAD pstHead, LPVIEWCHANNELTOOLS pstTool)
//{
//	LPCHANNELVAR pstGet;
//	LPCHANNELVAR pstPut;
//	char szSrc[128] = {0};
//	char szDst[128] = {0};
//
//	assert(NULL != pstHead);
//	assert(NULL != pstTool);
//
//	pstGet	=	&pstHead->astQueueVar[0];
//	pstPut	=	&pstHead->astQueueVar[1];
//	STRNCPY(szSrc,tbus_addr_ntoa(pstHead->astAddr[0]), sizeof(szSrc));
//	STRNCPY(szDst,tbus_addr_ntoa(pstHead->astAddr[1]), sizeof(szDst));
//
//
//	fprintf(pstTool->fpOut, "[%10s <-- %10s]: ", szSrc, szDst);
//	DumpOnlyBusVar(pstHead, pstGet, pstTool);
//
//	fprintf(pstTool->fpOut, "[%10s --> %10s]: ", szSrc, szDst);
//	DumpOnlyBusVar(pstHead, pstPut, pstTool);
//
//	return 0;
//
//}

int DumpBusChannel(LPTBUSCHANNEL pstChannel, LPVIEWCHANNELTOOLS pstTool)
{
	LPCHANNELVAR pstGet;
	char* pszGet;
	LPCHANNELVAR pstPut;
	char* pszPut;
	char szSrc[128] = {0};
	char szDst[128] = {0};
	CHANNELHEAD *pstHead ;

	assert(NULL != pstChannel);
	assert(NULL != pstTool);

	pstHead = pstChannel->pstHead;
	pstGet	=	&pstHead->astQueueVar[pstChannel->iRecvSide];
	pszGet	=	pstChannel->pszQueues[pstChannel->iRecvSide];
	pstPut	=	&pstHead->astQueueVar[pstChannel->iSendSide];
	pszPut	=	pstChannel->pszQueues[pstChannel->iSendSide];
    tbus_addr_ntop(pstHead->astAddr[pstChannel->iRecvSide], szSrc, sizeof(szSrc));
    tbus_addr_ntop(pstHead->astAddr[pstChannel->iSendSide], szDst, sizeof(szDst));


	fprintf(pstTool->fpOut, "[%10s <-- %10s]: ", szSrc, szDst);

	DumpBusVar(pstHead, pstGet, pszGet, pstTool);

	fprintf(pstTool->fpOut, "[%10s --> %10s]: ", szSrc, szDst);
	DumpBusVar(pstHead, pstPut, pszPut, pstTool);

	return 0;

}

int AttachChannel( IN TBUSSHMCHANNELCNF *a_pstShmChl,
                   INOUT TBUSCHANNEL *a_ptChannel,
                   IN LPTBUSSHMGCIMHEAD a_pstHead
                   )
{
	int iRet = TBUS_SUCCESS;
	char *sShm = NULL ;

	CHANNELHEAD *pstHead;
	int iChannelHeadLen;

	assert(NULL != a_pstShmChl);
	assert(NULL != a_ptChannel);
	assert(NULL != a_pstHead);

	iRet = tbus_check_channel_shm_i(a_pstShmChl, a_pstHead, TBUS_CHECK_CHNNL_FLAG_KEEPOPEN);
	if (TBUS_SUCCESS != iRet)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed to check share memory of channel, iRet %x", iRet);
		return iRet;
	}

#if defined(_WIN32) || defined(_WIN64)
	{
        int iSize;
        int iFlags;
        int iCreate;
        int iChannelHeadLen;

        iChannelHeadLen = sizeof(CHANNELHEAD);
        TBUS_CALC_ALIGN_VALUE(iChannelHeadLen, a_pstHead->iAlign);
        iSize = iChannelHeadLen + a_pstShmChl->dwRecvSize + a_pstShmChl->dwSendSize;
        iFlags = TMMAPF_READ | TMMAPF_WRITE;
        iRet = tbus_open_mmap_i(&sShm, a_pstHead->dwShmKey,
                (unsigned int)a_pstShmChl->astAddrs[0],
                (unsigned int)a_pstShmChl->astAddrs[1],
                0, &iSize, iFlags, &iCreate, &a_pstShmChl->iShmID);
        if (0 > iRet)
        {
            tbus_log(TLOG_PRIORITY_ERROR, "tbus_open_mmap_by_key_and_addr failed, by "
                    "shmkey(%d), addr1(%u), addr2(%u), size(%d)", a_pstHead->dwShmKey,
                    (unsigned int)a_pstShmChl->astAddrs[0], (unsigned int)a_pstShmChl->astAddrs[1],
                    iSize);
            sShm = (void*)-1;
        }
	}
#else
	sShm =	shmat(a_pstShmChl->iShmID, NULL, 0 ) ;
#endif
	if ((void*)-1 == sShm)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"shmat failed by id "PTR_OR_INT","
                 " for %s\n", a_pstShmChl->iShmID, strerror(errno));
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_SHMAT_FAILED);
	}


	iChannelHeadLen = sizeof(CHANNELHEAD);
	TBUS_CALC_ALIGN_VALUE(iChannelHeadLen, a_pstHead->iAlign);
#if !defined(_WIN32) && !defined(_WIN64)
	/*check size */
	{
		struct shmid_ds stShmStat ;
		int iSize;
		iSize = iChannelHeadLen + a_pstShmChl->dwRecvSize + a_pstShmChl->dwSendSize;
		memset(&stShmStat, 0, sizeof(stShmStat));
		if ( 0 != shmctl(a_pstShmChl->iShmID, IPC_STAT, &stShmStat))
		{
			tbus_log(TLOG_PRIORITY_ERROR,"shmctl stat failed by id %d,"
                     " for %s\n", a_pstShmChl->iShmID, strerror(errno));
			return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_SHMAT_FAILED);
		}

		if (iSize > (int)stShmStat.shm_segsz)
		{
			tbus_log(TLOG_PRIORITY_ERROR,
                     "shm size(%d) is smaller than the channel size(%d) expected\n",
                     (int)stShmStat.shm_segsz, iSize);
			return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_SHMAT_FAILED);
		}
	}

#endif

	a_ptChannel->pstHead = (CHANNELHEAD *)sShm;
	a_ptChannel->pszQueues[TBUS_CHANNEL_SIDE_INDEX_1] = sShm + iChannelHeadLen;
	a_ptChannel->pszQueues[TBUS_CHANNEL_SIDE_INDEX_2] = sShm + iChannelHeadLen + a_pstShmChl->dwRecvSize;
	pstHead = a_ptChannel->pstHead;
	if (pstHead->astAddr[0] != a_pstShmChl->astAddrs[0] || pstHead->astAddr[1] != a_pstShmChl->astAddrs[1]) {
		tbus_log(TLOG_PRIORITY_ERROR,"Channel config ERROR!\n");
		tbus_log(TLOG_PRIORITY_ERROR,"address1 in Channel Shm : %s   ", tbus_addr_ntoa(pstHead->astAddr[0]));
		tbus_log(TLOG_PRIORITY_ERROR,"address2 in Channel Shm : %s", tbus_addr_ntoa(pstHead->astAddr[1]));
		tbus_log(TLOG_PRIORITY_ERROR,"address1 in GCIM : %s   ", tbus_addr_ntoa(a_pstShmChl->astAddrs[0]));
		tbus_log(TLOG_PRIORITY_ERROR,"address1 in GCIM : %s", tbus_addr_ntoa(a_pstShmChl->astAddrs[1]));
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_CHANNEL_ADDRESS_CONFLICT);
	}

    a_ptChannel->iRecvSide = TBUS_CHANNEL_SIDE_INDEX_1 ;
    a_ptChannel->iSendSide = TBUS_CHANNEL_SIDE_INDEX_2 ;
	a_ptChannel->iShmID = pstHead->iShmID;

	tbus_log(TLOG_PRIORITY_DEBUG,"attach to channel(shmid:"PTR_OR_INT") successfully, localside:%d, peerside:%d",
		a_pstShmChl->iShmID, a_ptChannel->iRecvSide, a_ptChannel->iSendSide);


    return iRet;
}


int DetachChannel(     IN TBUSCHANNEL *a_ptChannel)
{
    int iRet = TBUS_SUCCESS;


    shmdt(a_ptChannel->pstHead);
#if defined(_WIN32) || defined(_WIN64)
    CloseHandle((HANDLE)a_ptChannel->iShmID);
#endif


    return iRet;
}


int constructViewTool(LPVIEWCHANNELTOOLS a_pstViewTool, LPTBUSMGROPTIONS a_pstOption)
{
    int iRet = 0;

    assert(NULL != a_pstViewTool);
    assert(NULL != a_pstOption);

	memset(a_pstViewTool, 0, sizeof(*a_pstViewTool));
	if ('\0' != a_pstOption->szMetalibFile[0])
	{
		iRet = tdr_load_metalib(&a_pstViewTool->pstLib, a_pstOption->szMetalibFile);
		if (TDR_SUCCESS != iRet)
		{
			a_pstViewTool->pstLib = NULL;
			printf("warnsing: failed to load metalib %s, for %s\n", a_pstOption->szMetalibFile, tdr_error_string(iRet));
		}
	}

	if (('\0' != a_pstOption->szMetaName[0]) && (NULL != a_pstViewTool->pstLib))
	{
		a_pstViewTool->pstMsgMeta = tdr_get_meta_by_name(a_pstViewTool->pstLib, a_pstOption->szMetaName);
		if (NULL == a_pstViewTool->pstMsgMeta)
		{
			printf("warnning: failed to get meta by name %s\n", a_pstOption->szMetaName);
		}else
		{
			a_pstViewTool->iSize = tdr_get_meta_size(a_pstViewTool->pstMsgMeta);
			a_pstViewTool->pszBuff = (char *)malloc(a_pstViewTool->iSize);
		}
	}/*if (('\0' != a_pstOption->szMetaName[0]) && (NULL != a_pstViewTool->pstLib))*/

	if ('\0' != a_pstOption->szOutFile[0])
	{
		a_pstViewTool->fpOut = fopen(a_pstOption->szOutFile, "w");
		if (NULL == a_pstViewTool->fpOut)
		{
			a_pstViewTool->fpOut = stdout;
		}
	}else
	{
		a_pstViewTool->fpOut = stdout;
	}
	a_pstViewTool->iMaxSeeMsg = a_pstOption->iMaxSeeMsg;

    return iRet;
}

int ViewAllChannels(IN LPTBUSSHMGCIM a_pShmGCIM, LPTBUSMGROPTIONS a_pstOption)
{
	int iRet = TBUS_SUCCESS;
    LPTBUSSHMCHANNELCNF pChannelConf = NULL;
    TBUSCHANNEL stTbusChannel;
	VIEWCHANNELTOOLS stTools;
    unsigned int i = 0;

	assert(NULL != a_pShmGCIM);
	assert(NULL != a_pstOption);

	iRet = tbus_initialize(a_pstOption->szShmKey, a_pShmGCIM->stHead.dwBusiID);
	if (TBUS_SUCCESS != iRet)
	{
		printf("tbus init error\n");
		return -1;
	}

	memset(&stTools, 0, sizeof(stTools));

    stTools.iDumpBinary = a_pstOption->iDumpBinary;
    stTools.iWithSep = a_pstOption->iWithSep;
    stTools.iStartMsgIndex = a_pstOption->iStartMsgIndex;

	if ('\0' != a_pstOption->szMetalibFile[0])
	{
		iRet = tdr_load_metalib(&stTools.pstLib, a_pstOption->szMetalibFile);
		if (TDR_SUCCESS != iRet)
		{
			stTools.pstLib = NULL;
			printf("warnsing: failed to load metalib %s, for %s\n", a_pstOption->szMetalibFile, tdr_error_string(iRet));
		}
	}

	if (('\0' != a_pstOption->szMetaName[0]) && (NULL != stTools.pstLib))
	{
		stTools.pstMsgMeta = tdr_get_meta_by_name(stTools.pstLib, a_pstOption->szMetaName);
		if (NULL == stTools.pstMsgMeta)
		{
			printf("warnning: failed to get meta by name %s\n", a_pstOption->szMetaName);
		}else
		{
			stTools.iSize = tdr_get_meta_size(stTools.pstMsgMeta);
			stTools.pszBuff = (char *)malloc(stTools.iSize);
		}
	}/*if (('\0' != a_pstOption->szMetaName[0]) && (NULL != stTools.pstLib))*/

	if ('\0' != a_pstOption->szOutFile[0])
	{
		stTools.fpOut = fopen(a_pstOption->szOutFile, "w");
		if (NULL == stTools.fpOut)
		{
			stTools.fpOut = stdout;
		}
	}else
	{
		stTools.fpOut = stdout;
	}
	stTools.iMaxSeeMsg = a_pstOption->iMaxSeeMsg;

	fprintf(stTools.fpOut, "================================================\n");
	fprintf(stTools.fpOut, "All channels about shmkey<%s>:\n", a_pstOption->szShmKey);

    for (i= 0; i< a_pShmGCIM->stHead.dwUsedCnt; i++)
    {
        memset(&stTbusChannel, 0, sizeof(TBUSCHANNEL));
        pChannelConf = &a_pShmGCIM->astChannels[i];
        iRet = AttachChannel(pChannelConf, &stTbusChannel, &a_pShmGCIM->stHead);
        if (TBUS_SUCCESS != iRet)
            break;
        fprintf(stTools.fpOut, "\n");
		DumpBusChannel(&stTbusChannel, &stTools);
        DetachChannel(&stTbusChannel);
    }

	if (stdout != stTools.fpOut)
	{
		fclose(stTools.fpOut);
	}
	if (NULL != stTools.pszBuff)
	{
		free(stTools.pszBuff);
	}
    tbus_fini();

    return iRet;
}
