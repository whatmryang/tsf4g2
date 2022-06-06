
#include "pal/pal.h"
#include "tbus/tbus.h"


int main(int argc, char *argv[])
{
	int iRet = TBUS_SUCCESS;
	int iBusRecvHandle = 0;
	TBUSADDR  iSrcAddr;
	TBUSADDR iTrsfAddr ;
	TBUSADDR  iDstAddr ;
	char szBuf[1024];
	size_t iRecvLen = sizeof(szBuf);
	int iD = 0;
	int iS = 0;

	if (2 > argc)
	{
		printf("Usage: %s GCIMShmkey\n", argv[0]);
        printf("    # GCIMShmKey's value should be the same with that in tbus_mgr.xml\n");
		return -1;
	}

	iRet = tbus_init_ex(argv[1], 0);
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

    tbus_set_logpriority(TLOG_PRIORITY_TRACE);


	iRet = tbus_addr_aton("0.0.0.1", &iSrcAddr);
	if (TBUS_SUCCESS != iRet)
	{
		printf("failed to convert %s to addr, %s", "0.0.0.1", tbus_error_string(iRet));
		return iRet;
	}

	iRet = tbus_addr_aton("0.0.0.2", &iTrsfAddr);
	if (TBUS_SUCCESS != iRet)
	{
		printf("failed to convert %s to addr, %s", "0.0.0.2", tbus_error_string(iRet));
		return iRet;
	}


	iRet = tbus_addr_aton("0.0.0.3" ,&iDstAddr);
	if (TBUS_SUCCESS != iRet)
	{
		printf("failed to convert %s to addr, %s", "0.0.0.3",  tbus_error_string(iRet));
		return iRet;
	}
	iRet = tbus_bind(iBusRecvHandle, iDstAddr);
	if (TBUS_SUCCESS != iRet)
	{
		printf("tbus bind error\n");
		return -1;
	}



	memset(szBuf, 0, sizeof(szBuf));
	do {
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
	}while (1);

	printf("==========================\n");
	printf(szBuf);
	printf("==========================\n");

	memset(szBuf, 0, sizeof(szBuf));
	strcpy(szBuf, "ybgg backward\n");

	iRet = tbus_backward(iBusRecvHandle, &iS, &iD, szBuf, strlen(szBuf), 0);
	if (TBUS_SUCCESS != iRet)
	{
		printf("tbus backward error\n");
		return -1;
	}
	printf("iD[%d], iS[%d]\n", iD, iS);

	tbus_delete(&iBusRecvHandle);
	return 0;
}
