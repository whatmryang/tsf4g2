#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "comm/comm.h"

#include "cutest/CuTest.h"

#include "../lib_src/comm/tmempool_i.h"

#define MAX_PLAYER_NUM 1000

typedef struct
{
	 int iIdx; /*idx in player mempool*/
	 int iLevel;
	 int  iHP;
	 TLISTNODE stNode;
}Player;

void TestTmempoolAllocDel(CuTest* tc);
void TestTmempoolFreeChain(CuTest* tc);
void TestTmempoolList(CuTest* tc);
void TestTmempoolAttach(CuTest* tc);

#if 0
void TestTTimer(CuTest* tc);

int ttimer_call_back(TTIMER *pstTimer)
{
	printf("TTimer call back called timeoutTime %d\n",pstTimer->timeOut);
	return 0;
}

void TestTTimer(CuTest* tc)
{
	int iRet;

	LPTTIMERHANDLE pstTimer;
	TTIMER stTimer;
	int i;

	iRet = ttimer_init(&pstTimer,100);
	CuAssertIntEquals(tc, 0, iRet);

	iRet = ttimer_register_callback( pstTimer,5, ttimer_call_back);
	CuAssertIntEquals(tc, 0, iRet);

	stTimer.callBackData = 0;
	stTimer.iCallback = 5;
	stTimer.timeOut = 5;


	iRet = ttimer_add(pstTimer,&stTimer);
	CuAssertIntEquals(tc, 0, iRet);

	stTimer.timeOut = 4;


	iRet = ttimer_add(pstTimer,&stTimer);
	CuAssertIntEquals(tc, 0, iRet);

	stTimer.timeOut = 1023;

	iRet = ttimer_add(pstTimer,&stTimer);
	CuAssertIntEquals(tc, 0, iRet);

	stTimer.timeOut = 1023*1000;

	iRet = ttimer_add(pstTimer,&stTimer);
	CuAssertIntEquals(tc, 0, iRet);


	stTimer.timeOut = 1023*1000 *200;

	iRet = ttimer_add(pstTimer,&stTimer);
	CuAssertIntEquals(tc, 0, iRet);

	//200M
	for(i = 0;i < 1024*1024 ;i++)
		ttimer_tick( pstTimer,0);

	stTimer.callBackData = 0;
	stTimer.iCallback = 5;
	stTimer.timeOut = 5;


	iRet = ttimer_add(pstTimer,&stTimer);
	CuAssertIntEquals(tc, 0, iRet);

	stTimer.timeOut = 4;


	iRet = ttimer_add(pstTimer,&stTimer);
	CuAssertIntEquals(tc, 0, iRet);

	stTimer.timeOut = 3;


	iRet = ttimer_add(pstTimer,&stTimer);
	CuAssertIntEquals(tc, 0, iRet);



	for(i = 0;i < 100;i++)
		ttimer_tick( pstTimer,0);

}

#endif

void TestTmempoolList(CuTest* tc)
{
#if 0
	int iRet;
	int iIdx;
	Player *pstPlayer1;
	TMEMPOOL *pstMemPool;


	iRet = tmempool_new(&pstMemPool, MAX_PLAYER_NUM, sizeof(Player));
	CuAssertIntEquals(tc, 0, iRet);

	iRet = tlist_new(&pstList,pstMemPool);
	CuAssertIntEquals(tc, 0, iRet);

	iIdx = tmempool_alloc(pstMemPool);
	pstPlayer1 = tmempool_get(pstMemPool,iIdx);
	tlist_insert_next(pstList,&(pstPlayer1->stNode));

	iIdx = tmempool_alloc(pstMemPool);
	pstPlayer1 = tmempool_get(pstMemPool,iIdx);
	tlist_insert_next(pstList,&(pstPlayer1->stNode));

	iIdx = tmempool_alloc(pstMemPool);
	pstPlayer1 = tmempool_get(pstMemPool,iIdx);
	tlist_insert_next(pstList,&(pstPlayer1->stNode));


	iIdx = tmempool_alloc(pstMemPool);
	pstPlayer1 = tmempool_get(pstMemPool,iIdx);
	tlist_insert_prev(pstList,&(pstPlayer1->stNode));

	iIdx = tmempool_alloc(pstMemPool);
	pstPlayer1 = tmempool_get(pstMemPool,iIdx);
	tlist_insert_prev(pstList,&(pstPlayer1->stNode));

	iIdx = tmempool_alloc(pstMemPool);
	pstPlayer1 = tmempool_get(pstMemPool,iIdx);
	tlist_insert_prev(pstList,&(pstPlayer1->stNode));



	iRet = tlist_fini(&pstList);
	CuAssertIntEquals(tc, 0, iRet);

	tmempool_destroy(&pstMemPool);
#endif
}


void TestTmempoolAttach(CuTest* tc)
{

	int iRet;
    int iIdx;
	int iIdx1,iIdx2;
	Player *pstPlayer1;
	TMEMPOOL *pstMemPool;
	TMEMPOOL *pstNewPool;

	iRet = tmempool_new(&pstMemPool, MAX_PLAYER_NUM, sizeof(Player));
	CuAssertIntEquals(tc, 0, iRet);

	iIdx1 = tmempool_alloc(pstMemPool);
	iIdx2 = tmempool_alloc(pstMemPool);
	iIdx2 = tmempool_free(pstMemPool,iIdx2);
	iIdx = tmempool_alloc(pstMemPool);
	iIdx = tmempool_alloc(pstMemPool);
	iIdx = tmempool_alloc(pstMemPool);
	iIdx = tmempool_alloc(pstMemPool);

	iRet = tmempool_attach(&pstNewPool,0,0,pstMemPool,tmempool_get_size(pstMemPool));
	CuAssertIntEquals(tc, -1, iRet);

	iRet = tmempool_attach(&pstNewPool,MAX_PLAYER_NUM, sizeof(Player),pstMemPool,tmempool_get_size(pstMemPool));
	CuAssertIntEquals(tc, 0, iRet);

	pstPlayer1 = tmempool_get(pstNewPool,iIdx1);
	CuAssertPtrNotNull(tc, pstPlayer1);

	tmempool_free(pstNewPool,iIdx1);
	pstPlayer1 = tmempool_get(pstMemPool,iIdx1);


	iRet = tmempool_attach(&pstNewPool,MAX_PLAYER_NUM, sizeof(Player),pstMemPool,tmempool_get_size(pstMemPool));
	CuAssertIntEquals(tc, 0, iRet);


	CuAssert(tc, "Pst player is not NULL", NULL == pstPlayer1);

	tmempool_destroy(&pstMemPool);

}

void TestCreateOnUserMem(CuTest *tc)
{
	int iRet;
	int iIdx;
	//Player *pstPlayer1;
	TMEMPOOL *pstMemPool;
	//TMEMPOOL *pstNewPool;
	void *pvBuff;
	size_t iSize;

	iSize = tmempool_calc_size(MAX_PLAYER_NUM, sizeof(Player));

	pvBuff = calloc(1,iSize);
	CuAssertPtrNotNull(tc, pvBuff);

	iRet =tmempool_init(&pstMemPool,MAX_PLAYER_NUM, sizeof(Player), pvBuff, iSize);
	CuAssertIntEquals(tc, 0, iRet);

	iIdx = tmempool_alloc(pstMemPool);

#if 0
	iRet = tmempool_attach_to(&pstNewPool,pstMemPool,tmempool_get_size(pstMemPool));
	CuAssertIntEquals(tc, 0, iRet);
#endif
	//pstPlayer1 = tmempool_get(pstNewPool,iIdx);
	//CuAssertPtrNotNull(tc, pstPlayer1);

	//tmempool_free(pstNewPool,iIdx);
	//pstPlayer1 = tmempool_get(pstMemPool,iIdx);

	//CuAssert(tc, "Pst player is not NULL", NULL == pstPlayer1);

	tmempool_destroy(&pstMemPool);
}


void TestTmempoolFreeChain(CuTest* tc)
{
	int iRet;
	int iIdx;
//	Player *pstPlayer;
	TMEMPOOL *pstMemPool;
	int iPos;
//	TMEMPOOL *pstMemPool2;
	iRet = tmempool_new(&pstMemPool, MAX_PLAYER_NUM, sizeof(Player));
	CuAssertIntEquals(tc, 0, iRet);


	iIdx =	tmempool_alloc(pstMemPool);
	CuAssertIntEquals(tc, 1000L, iIdx);

	//tmempool_free(pstMemPool,iIdx);

	iIdx =	tmempool_alloc(pstMemPool);
	CuAssertIntEquals(tc, 1001L, iIdx);


	tmempool_dump_used(pstMemPool,stdout);

	iIdx =	tmempool_alloc(pstMemPool);
	CuAssertIntEquals(tc, 1002L, iIdx);

	iIdx =	tmempool_alloc(pstMemPool);
	CuAssertIntEquals(tc, 1003L, iIdx);

	iIdx =	tmempool_alloc(pstMemPool);
	CuAssertIntEquals(tc, 1004L, iIdx);


	iIdx =	tmempool_alloc(pstMemPool);
	CuAssertIntEquals(tc, 1005L, iIdx);


	iIdx =	tmempool_alloc(pstMemPool);
	CuAssertIntEquals(tc, 1006L, iIdx);

	tmempool_dump_used(pstMemPool,stdout);

	iRet = tmempool_find_used_first(pstMemPool,&iPos);
	CuAssertIntEquals(tc, 0, iRet);
	CuAssertIntEquals(tc, 0, iPos);

	iIdx = tmempool_find_used_next(pstMemPool,&iPos);
	CuAssertIntEquals(tc, 1000, iIdx);
	CuAssertIntEquals(tc, 1, iPos);


	iIdx = tmempool_find_used_next(pstMemPool,&iPos);
	CuAssertIntEquals(tc, 1001, iIdx);
	CuAssertIntEquals(tc, 2, iPos);

	iIdx = tmempool_find_used_next(pstMemPool,&iPos);
	CuAssertIntEquals(tc, 1002, iIdx);
	CuAssertIntEquals(tc, 3, iPos);

	iIdx = tmempool_find_used_next(pstMemPool,&iPos);
	CuAssertIntEquals(tc, 1003L, iIdx);
	CuAssertIntEquals(tc,4, iPos);

	iIdx = tmempool_find_used_next(pstMemPool,&iPos);
	CuAssertIntEquals(tc, 1004L, iIdx);
	CuAssertIntEquals(tc, 5, iPos);

	iIdx = tmempool_find_used_next(pstMemPool,&iPos);
	CuAssertIntEquals(tc, 1005L, iIdx);
	CuAssertIntEquals(tc,6, iPos);


	iIdx = tmempool_find_used_next(pstMemPool,&iPos);
	CuAssertIntEquals(tc, 1006, iIdx);
	CuAssertIntEquals(tc, -1, iPos);

	//This should fail
	iIdx = tmempool_find_used_next(pstMemPool,&iPos);
	CuAssertIntEquals(tc, -1, iIdx);
	CuAssertIntEquals(tc, -1, iPos);

/*
	iRet = tmempool_reset_iterator(pstMemPool);
	CuAssertIntEquals(tc, 0, iRet);

	iIdx = tmempool_iterator_next(pstMemPool);
	CuAssertIntEquals(tc, 1000L, iIdx);

	iIdx = tmempool_iterator_next(pstMemPool);
	CuAssertIntEquals(tc, 1001L, iIdx);

	iIdx = tmempool_iterator_next(pstMemPool);
	CuAssertIntEquals(tc, 1002L, iIdx);
*/
	tmempool_dump_used(pstMemPool,stdout);

	//pstPlayer = (Player *)tmempool_get(pstMemPool, iIdx);
	//CuAssertTrue(tc,pstPlayer != NULL);

	tmempool_free(pstMemPool,1000L);

	tmempool_free(pstMemPool,1006L);


	tmempool_free(pstMemPool,1003L);

	tmempool_dump_used(pstMemPool,stdout);

	iRet = tmempool_free(pstMemPool,1003L);
	CuAssertIntEquals(tc, -2, iRet);


	tmempool_destroy(&pstMemPool);
}

void TestProtect(CuTest *tc);
void TestProtect(CuTest *tc)
{
	int iRet;
	int i;
	int iIdx;
	Player *pstPlayer;
	TMEMPOOL *pstMemPool;
//	TMEMPOOL *pstMemPool2;
	iRet = tmempool_new(&pstMemPool, MAX_PLAYER_NUM, sizeof(Player));
	CuAssertIntEquals(tc, 0, iRet);

	//iRet = tmempool_ctrl(pstMemPool, TMEMPOOL_SET_PAGE_PROTECT, NULL , 0);
	//CuAssertIntEquals(tc, 0, iRet);

	for(i = 0;i < 7;i++)
	{
		iIdx =	tmempool_alloc(pstMemPool);
		pstPlayer = tmempool_get(pstMemPool,iIdx);
		pstPlayer->iIdx = i+1000;
		CuAssertIntEquals(tc, i+1000, iIdx);
	}
	/*
	iRet = tmempool_reset_iterator(pstMemPool);
	CuAssertIntEquals(tc, 0, iRet);

	iIdx = tmempool_iterator_next(pstMemPool);
	CuAssertIntEquals(tc, 1000L, iIdx);

	iRet = tmempool_free(pstMemPool,iIdx);
	CuAssertIntEquals(tc, 0, iRet);

	iIdx = tmempool_iterator_next(pstMemPool);
	CuAssertIntEquals(tc, 1001L, iIdx);

	iIdx = tmempool_iterator_next(pstMemPool);
	CuAssertIntEquals(tc, 1002L, iIdx);

	iIdx = tmempool_iterator_next(pstMemPool);
	CuAssertIntEquals(tc, 1003L, iIdx);

	iIdx = tmempool_iterator_next(pstMemPool);
	CuAssertIntEquals(tc, 1004L, iIdx);

	iIdx = tmempool_iterator_next(pstMemPool);
	CuAssertIntEquals(tc, 1005L, iIdx);

	iIdx = tmempool_iterator_next(pstMemPool);
	CuAssertIntEquals(tc, 1006L, iIdx);

	iIdx = tmempool_iterator_next(pstMemPool);
	CuAssertIntEquals(tc, -1, iIdx);
	*/
	pstPlayer = (Player *)tmempool_get(pstMemPool, 1006L);
	CuAssertTrue(tc,pstPlayer != NULL);

	tmempool_free(pstMemPool,1000L);
	tmempool_free(pstMemPool,1006L);
	tmempool_free(pstMemPool,1003L);

	tmempool_dump_used(pstMemPool,stdout);

	iRet = tmempool_free(pstMemPool,1003L);
	CuAssertIntEquals(tc, -2, iRet);
	tmempool_destroy(&pstMemPool);
}

void TestIterator(CuTest *tc);

void TestIterator(CuTest *tc)
{
	int iRet;
	int i;
	int iIdx;
	Player *pstPlayer;
	TMEMPOOL *pstMemPool;
//	TMEMPOOL *pstMemPool2;
	iRet = tmempool_new(&pstMemPool, MAX_PLAYER_NUM, sizeof(Player));
	CuAssertIntEquals(tc, 0, iRet);

	for(i = 0;i < 7;i++)
	{
		iIdx =	tmempool_alloc(pstMemPool);
		pstPlayer = tmempool_get(pstMemPool,iIdx);
		pstPlayer->iIdx = i+1000;
		CuAssertIntEquals(tc, i+1000, iIdx);
	}
	/*
	iRet = tmempool_reset_iterator(pstMemPool);
	CuAssertIntEquals(tc, 0, iRet);

	iIdx = tmempool_iterator_next(pstMemPool);
	CuAssertIntEquals(tc, 1000L, iIdx);

	iRet = tmempool_free(pstMemPool,iIdx);
	CuAssertIntEquals(tc, 0, iRet);

	iIdx = tmempool_iterator_next(pstMemPool);
	CuAssertIntEquals(tc, 1001L, iIdx);

	iIdx = tmempool_iterator_next(pstMemPool);
	CuAssertIntEquals(tc, 1002L, iIdx);

	iIdx = tmempool_iterator_next(pstMemPool);
	CuAssertIntEquals(tc, 1003L, iIdx);

	iIdx = tmempool_iterator_next(pstMemPool);
	CuAssertIntEquals(tc, 1004L, iIdx);

	iIdx = tmempool_iterator_next(pstMemPool);
	CuAssertIntEquals(tc, 1005L, iIdx);

	iIdx = tmempool_iterator_next(pstMemPool);
	CuAssertIntEquals(tc, 1006L, iIdx);

	iIdx = tmempool_iterator_next(pstMemPool);
	CuAssertIntEquals(tc, -1, iIdx);
	*/
	pstPlayer = (Player *)tmempool_get(pstMemPool, 1006L);
	CuAssertTrue(tc,pstPlayer != NULL);

	tmempool_free(pstMemPool,1000L);
	tmempool_free(pstMemPool,1006L);
	tmempool_free(pstMemPool,1003L);

	tmempool_dump_used(pstMemPool,stdout);

	iRet = tmempool_free(pstMemPool,1003L);
	CuAssertIntEquals(tc, -2, iRet);
	tmempool_destroy(&pstMemPool);
}


void TestTmempoolAllocDel(CuTest* tc)
{
	int iRet;
	int iIdx;
	int i;
	Player *pstPlayer;
	TMEMPOOL *pstMemPool;

	#undef MAX_PLAYER_NUM
	#ifdef __LP64__
		#define MAX_PLAYER_NUM (300*1024*1024/sizeof(Player))
	#else
		#define MAX_PLAYER_NUM (20*1024*1024/sizeof(Player))
	#endif

	iRet = tmempool_new(&pstMemPool, MAX_PLAYER_NUM, sizeof(Player));
	CuAssertIntEquals(tc, 0, iRet);

	for(i = 0;i < MAX_PLAYER_NUM;i++)
	{
		iIdx = tmempool_alloc(pstMemPool);
		CuAssert(tc, "Failed to alloc", iIdx != -1);
		pstPlayer = tmempool_get(pstMemPool,iIdx);
		CuAssertPtrNotNull(tc, pstPlayer);
		memset(pstPlayer,0,sizeof(Player));
	}

	tmempool_destroy(&pstMemPool);
	CuAssertPtrEquals(tc, NULL, pstMemPool);

}

void mempoolOverAlloc(CuTest* tc)
{
	int iRet;
	int iIdx;
	int i = 10000000;
	//Player *pstPlayer;
	TMEMPOOL *pstMemPool;
    void *pvData;

	iRet = tmempool_new(&pstMemPool, 10, sizeof(Player));
	CuAssertIntEquals(tc, 0, iRet);

	for(i = 0; i < 20;i++)
	{
		iIdx = tmempool_alloc(pstMemPool);
		if(i < 10)
		{
			CuAssertIntEquals(tc, 10+i, iIdx);
		}
		else
		{
			CuAssertIntEquals(tc, -1, iIdx);
		}
	}

    i = 100000000;
    while(i--)
    {
#if 0
        TMEMBLOCK* pstBlock;
        pstBlock=TMEMPOOL_GET_PTR(pstMemPool, i);
        if(pstBlock->fValid)
        {
            pvData = pvData;
           // pvData = TMEMPOOL_GET_DATA(pstMemPool,pstBlock->iIdx);
        }
#endif
        tmempool_get_bypos(pstMemPool,i);
        pvData = pvData;
    }

	for(i = 0; i < 10;i++)
	{
		iRet = tmempool_free_bypos(pstMemPool,i);
		CuAssertIntEquals(tc, 0, iRet);
	}

	for(i = 0; i < 20;i++)
	{
		iIdx = tmempool_alloc(pstMemPool);
		if(i < 10)
		{
			CuAssert(tc,"Allocate failed",iIdx >= 0);
		}
		else
		{
			CuAssertIntEquals(tc, -1, iIdx);
		}
	}

	tmempool_destroy(&pstMemPool);
	CuAssertPtrEquals(tc, NULL, pstMemPool);
}


CuSuite* CuGetSuite(void)
{
	CuSuite* suite = CuSuiteNew();

	SUITE_ADD_TEST(suite,TestTmempoolAllocDel);
	SUITE_ADD_TEST(suite,TestTmempoolAttach);
	SUITE_ADD_TEST(suite,mempoolOverAlloc);

	SUITE_ADD_TEST(suite,TestTmempoolFreeChain);

	SUITE_ADD_TEST(suite,TestIterator);
	SUITE_ADD_TEST(suite,TestProtect);
	SUITE_ADD_TEST(suite,TestTmempoolAttach);

	return suite;
}

