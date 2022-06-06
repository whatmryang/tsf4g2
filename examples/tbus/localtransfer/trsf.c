
#include "pal/pal.h"
#include "tbus/tbus.h"


int main(int argc, char *argv[])
{
	int iRet = TBUS_SUCCESS;
	int iBusTrsfHandle = 0;
	TBUSADDR  iSrcAddr;
	TBUSADDR iTrsfAddr ;
	TBUSADDR  iDstAddr ;
	const char *pszBuff = NULL;
	size_t iRecvLen = 0;
	char szBuf[1024];
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

	iRet = tbus_new(&iBusTrsfHandle);
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
	iRet = tbus_bind(iBusTrsfHandle, iTrsfAddr);
	if (TBUS_SUCCESS != iRet)
	{
		printf("tbus bind error\n");
		return -1;
	}

	iRet = tbus_addr_aton("0.0.0.3" ,&iDstAddr);
	if (TBUS_SUCCESS != iRet)
	{
		printf("failed to convert %s to addr, %s", "0.0.0.3",  tbus_error_string(iRet));
		return iRet;
	}


	do {
        iRet = tbus_peek_msg(iBusTrsfHandle, &iSrcAddr, &iTrsfAddr, &pszBuff, &iRecvLen, 0);
        if (TBUS_ERR_CHANNEL_EMPTY == (unsigned)iRet)
		{
			printf("%s\n", tbus_error_string(iRet));
			sleep(1);
			continue;
		}
        else if (0 != iRet)
        {
            printf("tbus peek error isrc:%d iDst:%d\n", iSrcAddr, iTrsfAddr);
        }
		break;
	}while (1);

    memset(szBuf, 0, sizeof(szBuf));
    memcpy(szBuf, pszBuff, iRecvLen);
	//pszBuff[iRecvLen] = 0;
	iRet = tbus_delete_msg(iBusTrsfHandle, iSrcAddr, iTrsfAddr);
	if (0 != iRet)
	{
		printf("tbus_delete_msg failed: %s\n", tbus_error_string(iRet));
		return iRet;
	}

	iRet = tbus_forward(iBusTrsfHandle, &iTrsfAddr, &iDstAddr, szBuf, strlen(szBuf), 0);
	if (TBUS_SUCCESS != iRet)
	{
		printf("tbus send error\n");
		return -1;
	}

	memset(szBuf, 0, sizeof(szBuf));
	iRecvLen = sizeof(szBuf);
	do {
        iRet = tbus_recv(iBusTrsfHandle, &iDstAddr, &iTrsfAddr, szBuf, &iRecvLen, 0);
        if (TBUS_ERR_CHANNEL_EMPTY == (unsigned)iRet)
		{
			printf("%s\n", tbus_error_string(iRet));
			sleep(1);
			continue;
		}
        else if (0 != iRet)
        {
            printf("tbus recv error isrc:%d iDst:%d\n", iDstAddr, iTrsfAddr);
        }
		szBuf[iRecvLen] = 0;
		break;
	}while (1);

	iRet = tbus_backward(iBusTrsfHandle, &iS, &iD, szBuf, iRecvLen, 0);
	if (TBUS_SUCCESS != iRet)
	{
		printf("tbus forward error\n");
		return -1;
	}
	printf("iD[%d], iS[%d]\n", iD, iS);

	tbus_delete(&iBusTrsfHandle);

	return 0;
}
