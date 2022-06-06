
#include "pal/pal.h"
#include "tbus/tbus.h"


int main(int argc, char *argv[])
{
	int iRet = TBUS_SUCCESS;
	int iBusSendHandle = 0;
	unsigned int dwID;
    TBUSADDR  iSrcAddr;
	TBUSADDR  iTrsfAddr;
	char szBuf[1024] = "ybgg send\n";
    int iInFlux =0;
    int iOutFlux = 0;
	size_t iRecvLen = sizeof(szBuf);

	if (2 > argc)
	{
		printf("Usage: %s GCIMShmkey\n", argv[0]);
        printf("    # GCIMShmKey's value should be the same with that in tbus_mgr.xml\n");
		return -1;
	}

    iRet = tbus_init_ex(argv[1], 0);
    if (TBUS_SUCCESS != iRet)
    {
        printf("tbus init error, %s\n", tbus_error_string(iRet));
        return -1;
    }

    iRet = tbus_new(&iBusSendHandle);
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
    iRet = tbus_bind(iBusSendHandle, iSrcAddr);
    if (TBUS_SUCCESS != iRet)
    {
        printf("tbus bind error:%s\n", tbus_error_string(iRet));
        return -1;
    }

	iRet = tbus_addr_aton("0.0.0.2", &iTrsfAddr);
	if (TBUS_SUCCESS != iRet)
	{
		printf("failed to convert %s to addr, %s", "0.0.0.1", tbus_error_string(iRet));
		return iRet;
	}
	iRet = tbus_send(iBusSendHandle, &iSrcAddr, &iTrsfAddr, (void*)szBuf, strlen(szBuf),  TBUS_FLAG_START_DYE_MSG);
	if (TBUS_SUCCESS != iRet)
	{
		printf("tbus send error\n");
		return -1;
	}
	dwID = tbus_get_dyedmsgid();
	printf("trace send msg, Id : %u\n", dwID);

    iRet = tbus_get_channel_flux(iBusSendHandle, iSrcAddr,iTrsfAddr,&iInFlux, &iOutFlux);
    if (0 == iRet)
    {
        printf("address(%s) influx:%d outflux:%d\n", tbus_addr_ntoa(iSrcAddr),iInFlux,iOutFlux);
    }

	memset(szBuf, 0, sizeof(szBuf));
	do {
        iRet = tbus_recv(iBusSendHandle, &iTrsfAddr, &iSrcAddr, szBuf, &iRecvLen, TBUS_FLAG_START_DYE_MSG);
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

	printf("==========================\n");
	printf(szBuf);
	printf("==========================\n");

	tbus_delete(&iBusSendHandle);
    tbus_fini();

	return 0;
}
