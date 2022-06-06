#include <stdio.h>
#include "comm/comm.h"

#define MAX_PLAYER_NUM 1000
typedef struct
{
	 int iIdx; /*idx in player mempool*/
	 int iLevel;
	 int  iHP;
}Player;

int main(int argc, char* argv[])
{
	int iRet;
	intptr_t iIdx;
	Player *pstPlayer;
	TMEMPOOL *pstMemPool;

	(void) argc;
	(void)argv;

	iRet = tmempool_new(&pstMemPool, MAX_PLAYER_NUM, sizeof(Player));
	if (0 > iRet)
	{
		return -1;
	}

	iIdx =	tmempool_alloc(pstMemPool);
	if (0 > iIdx)
	{
		return -1;
	}
	pstPlayer = (Player *)tmempool_get(pstMemPool, iIdx);
	if (NULL == pstPlayer)
	{
		return -1;
	}

	pstPlayer->iIdx = iIdx;
	pstPlayer->iLevel = 1;
	pstPlayer->iHP = 100;

	//Player logout , free the mempool
	tmempool_free(pstMemPool, pstPlayer->iIdx);
	tmempool_destroy(&pstMemPool);

	return 0;
}

