
#include "tdr/tdr.h"
#include "pal/pal.h"
#include "tbus/tbus.h"

struct tagThreadArg
{
	int iChannelKey;
	int iBusHandle;
};
typedef struct tagThreadArg  THREADARG;
typedef struct tagThreadArg  *LPTHREADARG;

void trsf_thread(void* pvArg);
void recv_thread(void* pvArg);

int main(int argc, char *argv[])
{
	int iRet = TBUS_SUCCESS;
	int iBusSendHandle = 0;
	int iBusTrsfHandle;
	pthread_t iThreadId;
	unsigned int dwID;
	THREADARG threadarg;
	THREADARG threadarg2;
	int iTrsfAddr;
	int iSrcAddr;


	if (2 > argc)
	{
		printf("Usage: test GCIMShmkey\n");
		return -1;
	}

	iRet = tbus_init_ex(argv[1], 0);
	if (TBUS_SUCCESS != iRet)
	{
		printf("tbus init error\n");
		return -1;
	}
	tbus_channel_mode_init(0);

	iRet = tbus_new(&iBusSendHandle);
	if (TBUS_SUCCESS != iRet)
	{
		printf("tbus new error\n");
		return -1;
	}
	iRet = tbus_new(&iBusTrsfHandle);
	if (TBUS_SUCCESS != iRet)
	{
		printf("tbus new error\n");
		return -1;
	}






	 tbus_set_logpriority(TLOG_PRIORITY_TRACE);
	iRet = tbus_addr_aton("0.0.1", &iSrcAddr);
	if (TBUS_SUCCESS != iRet)
	{
		printf("failed to convert %s to addr, %s", "0.0.1", tbus_error_string(iRet));
		return iRet;
	}
	iRet = tbus_bind(iBusSendHandle, iSrcAddr);
	if (TBUS_SUCCESS != iRet)
	{
		printf("tbus bind error:%s\n", tbus_error_string(iRet));
		return -1;
	}


	 tbus_set_logpriority(TLOG_PRIORITY_TRACE);
	 iRet = tbus_addr_aton("0.0.2", &iTrsfAddr);
	 if (TBUS_SUCCESS != iRet)
	 {
		 printf("failed to convert %s to addr, %s", "0.0.2", tbus_error_string(iRet));
		 return iRet;
	 }
	iRet = tbus_connect(iBusSendHandle, iTrsfAddr);
	if (TBUS_SUCCESS != iRet)
	{
		printf("tbus connect error\n");
		return -1;
	}
	iRet = tbus_bind(iBusTrsfHandle, iTrsfAddr);
	if (TBUS_SUCCESS != iRet)
	{
		printf("tbus bind error:%s\n", tbus_error_string(iRet));
		return -1;
	}

	threadarg.iChannelKey = ftok(argv[0],0);
	threadarg.iBusHandle = iBusTrsfHandle;
	pthread_create(&iThreadId, NULL, (void*)trsf_thread, &threadarg);

	threadarg2.iChannelKey = threadarg.iChannelKey;
	threadarg2.iBusHandle = -1;
	pthread_create(&iThreadId, NULL, (void*)recv_thread, &threadarg2);


	//send
	tbus_set_logpriority(TLOG_PRIORITY_TRACE);
	char szBuf[1024] = "ybgg send\n";
	iRet = tbus_send(iBusSendHandle, &iSrcAddr, &iTrsfAddr, (void*)szBuf, strlen(szBuf),  TBUS_FLAG_START_DYE_MSG);
	if (TBUS_SUCCESS != iRet)
	{
		printf("tbus send error\n");
		return -1;
	}
	dwID = tbus_get_dyedmsgid();
	printf("trace send msg:%s Id : %u\n", szBuf,dwID);

    int iInFlux =0;
    int iOutFlux = 0;
    iRet = tbus_get_channel_flux(iBusSendHandle, iSrcAddr,iTrsfAddr,&iInFlux, &iOutFlux);
    if (0 == iRet)
    {
        printf("address(%s) influx:%d outflux:%d\n", tbus_addr_ntoa(iSrcAddr),iInFlux,iOutFlux);
    }


	tbus_set_logpriority(TLOG_PRIORITY_TRACE);
	//recv msg from  trsf thread
	while(1)
	{
		size_t iRecvLen = sizeof(szBuf);

		iRet = tbus_recv(iBusSendHandle, &iTrsfAddr,&iSrcAddr, &szBuf[0], &iRecvLen, 0);
		if (TBUS_SUCCESS == iRet)
		{
			break;
		}else if (TBUS_ERR_CHANNEL_EMPTY != (unsigned)iRet)
		{
			printf("tbus_peek_msg error\n");
			return -1;
		}
		usleep(10000);
	}


	printf("==========================\n");
	printf(szBuf);
	printf("==========================\n");

	tbus_delete(&iBusSendHandle);
	tbus_delete(&iBusTrsfHandle);

	return 0;

}

void trsf_thread(void* pvArg)
{
	TBUSADDR iTrsfAddr ;
	int iRet;
	LPTHREADARG pstArg = (LPTHREADARG)pvArg;
	const char *pszBuff = NULL;
	size_t iRecvLen = 0;
	TBUSADDR iSrcAddr = -1;
	LPTBUSCHANNEL pstChannel;
	char szTbusHead[1024];
	int iTbusHeadLen;

	//init
	if (NULL == pstArg)
	{
		printf("param is null");
		return;
	}
	iRet = tbus_addr_aton("0.0.2", &iTrsfAddr);
	if (TBUS_SUCCESS != iRet)
	{
		printf("failed to convert %s to addr, %s", "0.0.2", tbus_error_string(iRet));
		return;
	}
	iRet = tbus_channel_open(&pstChannel, 1,2, pstArg->iChannelKey, 10240);
	if (0 != iRet)
	{
		printf("failed to open channel by key(%d):%s", pstArg->iChannelKey, tbus_error_string(iRet));
		return;
	}
	printf("recv_thread: channelkey:%d]\n", pstArg->iChannelKey);


	tbus_set_logpriority(TLOG_PRIORITY_TRACE);
	//recv msg from  send thread
	while(1)
	{
		iSrcAddr = -1;
		iRet = tbus_peek_msg(pstArg->iBusHandle, &iSrcAddr, &iTrsfAddr, &pszBuff, &iRecvLen, 0);
		if (TBUS_SUCCESS == iRet)
		{
			printf("trsf_thread  recv msg(len:%d)\n", (int)iRecvLen);
			break;
		}else if (TBUS_ERR_CHANNEL_EMPTY != (unsigned)iRet)
		{
			printf("tbus_peek_msg error\n");
			return;
		}
		usleep(10000);
		//printf("tbus_peek_msg error:%s \n",tbus_error_string(iRet));
	}

	//forward msg to recv_thread
	iTbusHeadLen = sizeof(szTbusHead);
	iRet = tbus_get_bus_head(pstArg->iBusHandle, &szTbusHead[0], &iTbusHeadLen);
	if (0 != iRet)
	{
		printf("failed to get bus head: %s", tdr_error_string(iRet));
		return;
	}
	iRet = tbus_channel_set_bus_head(pstChannel, &szTbusHead[0], iTbusHeadLen);
	if (0 != iRet)
	{
		printf("failed to set channel bus head: %s", tdr_error_string(iRet));
		return;
	}
	iRet = tbus_channel_forward(pstChannel, pszBuff, iRecvLen, 0);
	if (0 != iRet)
	{
		printf("failed to forward by channcel: %s", tdr_error_string(iRet));
		return;
	}
	printf("trsf_thread  channel_forward msg(len:%d)\n", (int)iRecvLen);
	iRet = tbus_delete_msg(pstArg->iBusHandle, iSrcAddr, iTrsfAddr);
	if (0 != iRet)
	{
		printf("tbus_delete_msg failed: %s\n", tbus_error_string(iRet));
		return;
	}


	//recv msg from recv_thread
	while(1)
	{
		iSrcAddr = -1;
		iRet = tbus_channel_peek_msg(pstChannel, &pszBuff, &iRecvLen, 0);
		if (TBUS_SUCCESS == iRet)
		{
			printf("trsf_thread  channel_peek msg(len:%d)\n", (int)iRecvLen);
			break;
		}else if (TBUS_ERR_CHANNEL_EMPTY != (unsigned)iRet)
		{
			printf("tbus_peek_msg error\n");
			return;
		}
		usleep(10000);
		printf("trsf_thread tbus_channel_peek_msg error:%s \n",tbus_error_string(iRet));
	}


	//backward to send_thread
	iTbusHeadLen = sizeof(szTbusHead);
	iRet = tbus_channel_get_bus_head(pstChannel, &szTbusHead[0], &iTbusHeadLen);
	if (0 != iRet)
	{
		printf("failed to get channel bus head: %s", tdr_error_string(iRet));
		return;
	}
	iRet = tbus_set_bus_head(pstArg->iBusHandle, &szTbusHead[0], iTbusHeadLen);
	if (0 != iRet)
	{
		printf("failed to set  bus head: %s", tdr_error_string(iRet));
		return;
	}

	iSrcAddr = 0;
	iTrsfAddr = 0;
	iRet = tbus_backward(pstArg->iBusHandle, &iSrcAddr, &iTrsfAddr, pszBuff, iRecvLen, 0);
	if (0 != iRet)
	{
		printf("failed to bacckward, iret:0x%0x  %s", iRet, tdr_error_string(iRet));
		return;
	}
	printf("trsf_thread  backward msg(len:%d)\n", (int)iRecvLen);
	iRet = tbus_channel_delete_msg(pstChannel);
	if (0 != iRet)
	{
		printf("tbus_delete_msg failed: %s\n", tbus_error_string(iRet));
		return;
	}


}
void recv_thread(void* pvArg)
{
	int iRet;
	LPTHREADARG pstArg = (LPTHREADARG)pvArg;
	LPTBUSCHANNEL pstChannel;
	char szData[1024];
	size_t iData;

	//init
	if (NULL == pstArg)
	{
		printf("param is null");
		return;
	}
	iRet = tbus_channel_open(&pstChannel, 2,1, pstArg->iChannelKey, 10240);
	if (0 != iRet)
	{
		printf("failed to open channel by key(%d):%s", pstArg->iChannelKey, tbus_error_string(iRet));
		return;
	}
	printf("recv_thread: channelkey:%d]\n", pstArg->iChannelKey);

	tbus_set_logpriority(TLOG_PRIORITY_TRACE);
	//recv msg from  send thread
	while(1)
	{
		iData = sizeof(szData);
		iRet = tbus_channel_recv(pstChannel, &szData[0], &iData, 0);
		if (TBUS_SUCCESS == iRet)
		{
			printf("recv_thread  recv msg(len:%d)\n", (int)iData);
			break;
		}else if (TBUS_ERR_CHANNEL_EMPTY != (unsigned)iRet)
		{
			printf("tbus_peek_msg error\n");
			return;
		}
		usleep(10000);
		printf("recv_thread tbus_channel_recv error:%s \n",tbus_error_string(iRet));
	}

	//forward msg to trsf_thread
	tbus_channel_backward(pstChannel, &szData[0], iData, 0);
	if (0 != iRet)
	{
		printf("tbus_channel_backward failed: %s\n", tbus_error_string(iRet));
		return;
	}
	printf("recv_thread  backward msg(len:%d)\n", (int)iData);
}


