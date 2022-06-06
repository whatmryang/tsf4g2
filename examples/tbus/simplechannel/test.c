
#include "pal/pal.h"
#include "tbus/tbus.h"


#define SHM_FILE_PATH_LEN	256

struct tagThreadArg
{
	char szShmFile[SHM_FILE_PATH_LEN];
	int iQueueSize;
	int iType;
	char szLocalAddr[SHM_FILE_PATH_LEN];
	char szPeerAddr[SHM_FILE_PATH_LEN];
	int iCount;
	char szBuff[1024];
	size_t iBuff;
	int iShmKey;


};
typedef struct tagThreadArg THREADARG;
typedef struct tagThreadArg *LPTHREADARG;

void * thread_proc(void* pvArg);
void * main_proc(void* pvArg);


/* 原本这个数据是在main中定义的局部变???*/
/* 但是如果是局部变???当主线程退出时 */
/* thread_proc访问这个变量时就会出???*/
/* 所以这个数组定义为全局变量 */

THREADARG stThreadArg[2];

int main(int iagrc, char *argv[])
{
	int i;
	pthread_t iThreadId;

	if (2 >= iagrc)
	{
		printf("Usage: %s shmkey count\n", argv[0]);
        printf("    # shmkey can be any value, only that this value is not used by other programs now.\n");
		return 1;
	}

	printf("i: %d, %p\n", 0,  &stThreadArg[0]);
	printf("i: %d, %p\n", 1,  &stThreadArg[1]);
	tbus_channel_mode_init(TBUS_CHANNEL_MODE_INIT_FLAG_ENABLE_TRACE);
	for(i=0; i<2; i++)
	{
		stThreadArg[i].iShmKey = strtol(argv[1], NULL, 10);
		stThreadArg[i].iType = 1000;
		stThreadArg[i].iQueueSize = 10240;
		stThreadArg[i].iCount = strtol(argv[2], NULL,10);
		if (0 >= stThreadArg[i].iCount )
			stThreadArg[i].iCount  = 10;

		snprintf(stThreadArg[i].szLocalAddr, sizeof(stThreadArg[i].szLocalAddr),
			"1.1.1.%d", i);
		snprintf(stThreadArg[i].szPeerAddr, sizeof(stThreadArg[i].szPeerAddr),
			"1.1.1.%d", 1 - i);

		if (i == 0)
		{
			if( pthread_create(&iThreadId, NULL, thread_proc, &stThreadArg[i])<0 )
			{
				printf("failed to create pthread");
				return 1 ;
			}
		}else
		{
 			main_proc(&stThreadArg[i]);
		}
	}/*for(i=0; i<2; i++)*/

	tbus_channel_mode_fini();
	return 0;
}

void * thread_proc(void* pvArg)
{
	LPTBUSCHANNEL	pstChannel = NULL;

	LPTHREADARG pstThread	=	(LPTHREADARG) pvArg;
	int iRet = 0;
	int iTotol;

	iRet = tbus_channel_open_by_str(&pstChannel, pstThread->szLocalAddr, pstThread->szPeerAddr,
		pstThread->iShmKey, pstThread->iQueueSize);
	if (0 != iRet)
	{
		printf("faild to open channel, iRet :%d, %s\n", iRet, tbus_error_string(iRet));
		getchar();
		return NULL;
	}
	tbus_set_logpriority(TLOG_PRIORITY_TRACE);

	//pstThread->iCount = 3;
	iTotol = pstThread->iCount;
	while( pstThread->iCount )
	{
		int iFailed = 0;
#if defined(_WIN32) || defined(_WIN64)
		snprintf(pstThread->szBuff, sizeof(pstThread->szBuff), "hello, I am pthead %u, this %dth call you",
			(unsigned int)GetCurrentThreadId(), iTotol-pstThread->iCount);
#else
		snprintf(pstThread->szBuff, sizeof(pstThread->szBuff), "hello, I am  pthead %u, this %dth call you",
			(unsigned int)pthread_self(), iTotol-pstThread->iCount);
#endif

		iRet = tbus_channel_send(pstChannel, pstThread->szBuff, strlen(pstThread->szBuff)+1, 0);
		if (0 != iRet)
		{
			printf("tbus_channel_send failed, iRet %d, %s\n", iRet, tbus_error_string(iRet));
		}

		while(1)
		{
			pstThread->iBuff = sizeof(pstThread->szBuff) - 1;
			iRet = tbus_channel_recv(pstChannel, pstThread->szBuff, &pstThread->iBuff, 0);
			if (0 == iRet)
			{
				pstThread->szBuff[pstThread->iBuff] = 0;
				printf("Recv: %s\n", pstThread->szBuff);
				break;
			}
			iFailed++;
			if (10 < iFailed)
			{
				break;
			}
			usleep(10);
		}/*while(1)*/

		pstThread->iCount--;
	}/*while( iCount )*/

	tbus_close_channel(&pstChannel);


	return NULL;
}

void * main_proc(void* pvArg)
{
	LPTBUSCHANNEL	pstChannel = NULL;

	LPTHREADARG pstThread	=	(LPTHREADARG) pvArg;
	int iRet = 0;
	int iTotol;

	iRet = tbus_open_channel_by_str(&pstChannel, pstThread->szLocalAddr, pstThread->szPeerAddr,
		pstThread->iShmKey, pstThread->iQueueSize);
	if (0 != iRet)
	{
		printf("faild to open channel, iRet :%d, %s\n", iRet, tbus_error_string(iRet));
		return NULL;
	}
	tbus_set_logpriority(TLOG_PRIORITY_TRACE);

	//pstThread->iCount = 3;
	iTotol = pstThread->iCount;
	while( pstThread->iCount )
	{
		int iFailed = 0;
#if defined(_WIN32) || defined(_WIN64)
		snprintf(pstThread->szBuff, sizeof(pstThread->szBuff), "hello, I am  pthead %u, this %dth call you",
			(unsigned int)GetCurrentThreadId(), iTotol-pstThread->iCount);
#else
		snprintf(pstThread->szBuff, sizeof(pstThread->szBuff), "hello, I am  pthead %u, this %dth call you",
			(unsigned int)pthread_self(), iTotol-pstThread->iCount);
#endif
		iRet = tbus_channel_send(pstChannel, pstThread->szBuff, strlen(pstThread->szBuff) + 1, 0);
		if (0 != iRet)
		{
			printf("tbus_channel_send failed, iRet %d, %s\n", iRet, tbus_error_string(iRet));
		}

		while(1)
		{
			const char *pszData = NULL;
			size_t iDataLen = 0;
			iRet = tbus_channel_peek_msg(pstChannel, &pszData, &iDataLen, 0);
			if (0 == iRet)
			{
				assert(NULL != pszData);
				printf("Recv: %s\n", pszData);
				tbus_channel_delete_msg(pstChannel);
				break;
			}
			iFailed++;
			if (10 < iFailed)
			{
				break;
			}
			usleep(10);
		}/*while(1)*/

		pstThread->iCount--;
	}/*while( iCount )*/

	tbus_channel_close(&pstChannel);


	return NULL;
}



