// taa.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <time.h>

#include "taa/tagentapi.h"

LPEXCHANGEMNG	pstExcMng;

int agent_open_exc()
{
	int		iRet;
	
	iRet	= agent_api_init(&pstExcMng);
	if (iRet < 0)
	{
		fprintf (stderr, "agent_api_init error\n");
		return -1;
	}
	
	iRet	= agent_api_register(pstExcMng, ID_APPID_TMAC, 0);
	if (iRet < 0)
	{
		fprintf (stderr, "agent_api_register error\n");
		return -1;
	}

	return 0;
}

void regdisplay()
{
	int				i, iRet;
	int 			iBlock;
	
	char			sBuf[64];
	EXCHANGEBLOCK	*astBlock[4096];
	
	if (agent_open_exc() < 0)
	{
		fprintf(stderr, "agent_open_exc error\n");
		return;
	}
	
	iBlock			= sizeof(astBlock) / sizeof(EXCHANGEBLOCK);
	iRet			= agent_api_get_blocks(pstExcMng, astBlock, &iBlock);
	if (iRet < 0)
	{
		printf ("agent_api_get_blocks error\n");
		return;
	}
	
	strftime(sBuf, 64, "%Y-%m-%d %H:%M:%S",		localtime((time_t *)&(pstExcMng->pstExcHead->iCreateTime)));
	printf ("ExchangeSize:\t%d\n",				pstExcMng->pstExcHead->iExchangeSize);
	printf ("BlockSize:\t%d\n",					pstExcMng->pstExcHead->iBlockSize);
	printf ("CreateTime:\t%s\n",				sBuf);
	printf ("Version:\t%x\n",					pstExcMng->pstExcHead->iVersion);
	printf ("Registered:\t%d\n",				pstExcMng->pstExcHead->iUsed);
	printf ("Description:\t%s\n",				pstExcMng->pstExcHead->szDesc);
	
	for (i = 0; i < iBlock; i++)
	{
		fprintf (stderr, "Appid:%u\t\tBusid:%u\n", astBlock[i]->uiAppid, astBlock[i]->uiBusid);
	}
	
	agent_api_destroy(pstExcMng);
}

int main(int argc, char *argv[])
{
	printf ("Test for tagent api\n");

	regdisplay ();

	return 0;
}

