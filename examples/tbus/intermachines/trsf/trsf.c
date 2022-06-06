
#include "pal/pal.h"
#include "tbus/tbus.h"

int main()
{
	int iRet = TBUS_SUCCESS;
	int iBusTrsfHandle = 0;
	size_t iRecvLen ;
	TBUSADDR iSrcAddr ;
	TBUSADDR iTrsfAddr;
	TBUSADDR iDstAddr;
	char szBuf[1024];
	int iD = 0;
	int iS = 0;

	iRet = tbus_init_ex("6000", 0);
	if (TBUS_SUCCESS != iRet)
	{
		printf("tbus init error\n");
		return -1;
	}

	iRet = tbus_new(&iBusTrsfHandle);
	if (TBUS_SUCCESS != iRet)
	{
		printf("tbus new error\n");
		return -1;
	}

	iRet = tbus_addr_aton("0.0.0.1", &iSrcAddr);
	if (0 != iRet )
	{
		printf("failed to convert %s to tbus addr, iRet %x\n", "0.0.0.1", iRet);
	}
	iRet = tbus_addr_aton("0.0.0.2", &iTrsfAddr);
	if (0 != iRet )
	{
		printf("failed to convert %s to tbus addr, iRet %x\n", "0.0.0.2", iRet);
	}
	iRet = tbus_addr_aton("0.0.0.3", &iDstAddr);
	if (0 != iRet )
	{
		printf("failed to convert %s to tbus addr, iRet %x\n", "0.0.0.3", iRet);
	}
	iRet = tbus_bind(iBusTrsfHandle, iTrsfAddr);
	if (TBUS_SUCCESS != iRet)
	{
		printf("tbus bind error\n");
		return -1;
	}

	tbus_set_logpriority(TLOG_PRIORITY_TRACE);
	do {
		memset(szBuf, 0, sizeof(szBuf));
		iRecvLen = sizeof(szBuf);
		iRet = tbus_recv(iBusTrsfHandle, &iSrcAddr, &iTrsfAddr, (void*)szBuf, &iRecvLen, 0);
        if (TBUS_ERR_CHANNEL_EMPTY == (unsigned)iRet)
		{
			printf("%s\n", tbus_error_string(iRet));
			sleep(1);
			continue;
		}
        else if (0 != iRet)
        {
            printf("tbus recv error isrc:%d iDst:%d\n", iSrcAddr, iTrsfAddr);
            break;
        }
		szBuf[iRecvLen] = 0;

		printf("[trsfer]\nrecv: ");
		puts(szBuf);
		printf("==========================\n");
		break;
	}while(1);

	iRet = tbus_forward(iBusTrsfHandle, &iTrsfAddr, &iDstAddr, szBuf, strlen(szBuf), 0);
	if (TBUS_SUCCESS != iRet)
	{
		printf("tbus send error\n");
		return -1;
	}

	printf("[trsfer]\nforward: ");
	puts(szBuf);
	printf("==========================\n");
	do {
		memset(szBuf, 0, sizeof(szBuf));
		iRecvLen = sizeof(szBuf);
		iRet = tbus_recv(iBusTrsfHandle, &iDstAddr, &iTrsfAddr, szBuf, &iRecvLen, 0);
        if (TBUS_ERR_CHANNEL_EMPTY == (unsigned)iRet)
		{
			printf("%s\n", tbus_error_string(iRet));
			sleep(1);
			continue;
		}
        else if (0 != iRet)
        {
            printf("tbus send error isrc:%d iDst:%d\n", iDstAddr, iTrsfAddr);
            break;
        }
		szBuf[iRecvLen] = '\0';
		printf("[trsfer]\nrecv: ");
		puts(szBuf);
		printf("==========================\n");

		break;
	}while(1);

	iRet = tbus_backward(iBusTrsfHandle, &iS, &iD, szBuf, iRecvLen, 0);
	if (TBUS_SUCCESS != iRet)
	{
		printf("tbus forward error\n");
		return -1;
	}
	printf("iD[%d], iS[%d]\n", iD, iS);


	printf("[trsfer]\nbackward: ");
	puts(szBuf);
	printf("==========================\n");

	tbus_delete(&iBusTrsfHandle);
    tbus_fini();

	return 0;
}


