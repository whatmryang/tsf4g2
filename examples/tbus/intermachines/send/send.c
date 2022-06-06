
#include "pal/pal.h"
#include "tbus/tbus.h"


int main()
{
	int iRet = TBUS_SUCCESS;
	int iBusSendHandle = 0;
	TBUSADDR iSrcAddr;
	TBUSADDR iTrsfAddr;
	char szBuf[1024] = "a test message\n";

	iRet = tbus_init_ex("5000", 0);
	if (TBUS_SUCCESS != iRet)
	{
		printf("tbus init error\n");
		return -1;
	}

	iRet = tbus_new(&iBusSendHandle);
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
	iRet = tbus_bind(iBusSendHandle, iSrcAddr);
	if (TBUS_SUCCESS != iRet)
	{
		printf("tbus bind error\n");
		return -1;
	}

	iRet = tbus_addr_aton("0.0.0.2", &iTrsfAddr);
	if (0 != iRet )
	{
		printf("failed to convert %s to tbus addr, iRet %x\n", "0.0.0.2", iRet);
	}

	tbus_set_logpriority(TLOG_PRIORITY_TRACE);
	iRet = tbus_send(iBusSendHandle, &iSrcAddr, &iTrsfAddr, (void*)szBuf, strlen(szBuf), TBUS_FLAG_START_DYE_MSG);
	if (TBUS_SUCCESS != iRet)
	{
		printf("tbus send error\n");
		return -1;
	}

	printf("\n[sender]\nsend: ");
	puts(szBuf);
	printf("==========================\n");

	memset(szBuf, 0, sizeof(szBuf));
	do {
		size_t iRecvLen = sizeof(szBuf);
		iRet = tbus_recv(iBusSendHandle, &iTrsfAddr, &iSrcAddr, (void*)szBuf, &iRecvLen, 0);
        if (TBUS_ERR_CHANNEL_EMPTY == (unsigned)iRet)
		{
			printf("%s\n", tbus_error_string(iRet));
			sleep(1);
			continue;
		}
        else if (0 != iRet)
        {
            printf("tbus recv error isrc:%d iDst:%d\n", iTrsfAddr, iSrcAddr);
        }
		szBuf[iRecvLen] = 0;
		break;
	}while (1);



	printf("[sender]\nrecv: ");
	puts(szBuf);
	printf("==========================\n");

	tbus_delete(&iBusSendHandle);
    tbus_fini();

	return 0;
}
