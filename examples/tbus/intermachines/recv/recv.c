
#include "pal/pal.h"
#include "tbus/tbus.h"


int main()
{
	int iRet = TBUS_SUCCESS;
	int iBusRecvHandle = 0;
	TBUSADDR iTrsfAddr;
	TBUSADDR iDstAddr;
	char szBuf[1024] = {0};
	int iD = 0;
	int iS = 0;

	iRet = tbus_initialize("7000", 0);
	if (TBUS_SUCCESS != iRet)
	{
		printf("tbus init error\n");
		return -1;
	}

	iRet = tbus_new(&iBusRecvHandle);
	if (TBUS_SUCCESS != iRet)
	{
		printf("tbus new error\n");
		return -1;
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

	iRet = tbus_bind(iBusRecvHandle, iDstAddr);
	if (TBUS_SUCCESS != iRet)
	{
		printf("tbus bind error\n");
		return -1;
	}

	tbus_set_logpriority(TLOG_PRIORITY_TRACE);
	do {
		size_t iRecvLen = sizeof(szBuf);
		iRet = tbus_recv(iBusRecvHandle, &iTrsfAddr, &iDstAddr, szBuf, &iRecvLen, 0);
        if (TBUS_ERR_CHANNEL_EMPTY == (unsigned)iRet)
		{
			printf("%s\n", tbus_error_string(iRet));
			sleep(1);
			continue;
		}
        else if (0 != iRet)
        {
            printf("tbus recv error isrc:%d iDst:%d\n", iTrsfAddr, iDstAddr);
        }
		szBuf[iRecvLen] = 0;
		break;
	}while(1);



	printf("[recver]\nrecv: ");
	puts(szBuf);
	printf("==========================\n");


	iRet = tbus_backward(iBusRecvHandle, &iS, &iD, szBuf, strlen(szBuf), 0);
	if (TBUS_SUCCESS != iRet)
	{
		printf("tbus backward error\n");
		return -1;
	}
	printf("iD[%d], iS[%d]\n", iD, iS);



	printf("[recver]\nbackward: ");
	puts(szBuf);
	printf("==========================\n");

	tbus_delete(&iBusRecvHandle);
    tbus_fini();

	return 0;
}


