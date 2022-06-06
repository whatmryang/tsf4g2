#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "comm/comm.h"
#include "comm/tlist.h"


#include "cutest/CuTest.h"
#include "comm/tmempool.h"
#include "comm/ttimer.h"


#include "../lib_src/comm/shtable_i.h"

#define MAX_APP_ITEM  20
#define MAX_APP_BUCKET  5*MAX_APP_ITEM
#define SCAN_NUM_ONE_LOOP 10

typedef struct tagUinShtData 
{
	long lUin;	 /*uin of player*/
	int  iIdx;	/*index of player in memory pool*/
}UinShtData;

int UinHashCmp( const void * pv1 , const void *pv2 )
{
	UinShtData *pstUinHashData1 = (UinShtData *)pv1;
	UinShtData *pstUinHashData2 = (UinShtData *)pv2;
	return pstUinHashData1->lUin - pstUinHashData2->lUin;
}

unsigned int UinHashCode(const void* pvData)
{
	UinShtData *pstUinShtData = (UinShtData *)pvData;
	return (unsigned int)pstUinShtData->lUin;
}

void testShtableAlloc(CuTest* tc);

void testShtableAlloc(CuTest* tc)
{
	int iRet;
	LPSHTABLE pstHashTab;
	size_t iHashTabSize;
	UinShtData stUinHashData;
	UinShtData *pstUinHashData;
	int i;

	pstHashTab = sht_create(MAX_APP_BUCKET, MAX_APP_ITEM, sizeof(UinShtData), &iHashTabSize);
	CuAssertPtrNotNull(tc, pstHashTab);

	for(i = 0;i < 19;i++)
	{
		stUinHashData.lUin = 267478100+i;	
		stUinHashData.iIdx = -1;
		pstUinHashData = (UinShtData *)   sht_insert_unique(pstHashTab,&stUinHashData,UinHashCmp,UinHashCode);
		CuAssertPtrNotNull(tc, pstUinHashData);
		memcpy(pstUinHashData, &stUinHashData,
		sizeof(*pstUinHashData));	
	}


	stUinHashData.lUin = 267478100;	
	stUinHashData.iIdx = -1;
	pstUinHashData = (UinShtData *)   sht_insert_unique(pstHashTab,&stUinHashData,UinHashCmp,UinHashCode);
	CuAssertPtrEquals(tc, NULL,pstUinHashData);

	stUinHashData.lUin = 267478100+19;	
	stUinHashData.iIdx = -1;
	pstUinHashData = (UinShtData *)   sht_insert_unique(pstHashTab,&stUinHashData,UinHashCmp,UinHashCode);
	CuAssertPtrNotNull(tc, pstUinHashData);
	memcpy(pstUinHashData, &stUinHashData,
	sizeof(*pstUinHashData));	

	stUinHashData.lUin = 267478100+i+10;	
	stUinHashData.iIdx = -1;
	pstUinHashData = (UinShtData *)   sht_insert_unique(pstHashTab,&stUinHashData,UinHashCmp,UinHashCode);
	CuAssertPtrEquals(tc, NULL,pstUinHashData);
	


	for(i = 0;i < 20;i++)
	{
		stUinHashData.lUin = 267478100+i;	
		stUinHashData.iIdx = -1;
		pstUinHashData = (UinShtData *) sht_find(pstHashTab,&stUinHashData,UinHashCmp,UinHashCode);
		CuAssertPtrNotNull(tc, pstUinHashData);
		CuAssert(tc, "Data ont right.", !memcmp(&stUinHashData,pstUinHashData,sizeof(stUinHashData)));
	}

	CuAssert(tc, "Sht full", sht_is_full(pstHashTab));

	sht_rebuild(pstHashTab);

	for(i = 0;i < 20;i++)
	{
		stUinHashData.lUin = 267478100+i;	
		stUinHashData.iIdx = -1;
		pstUinHashData = (UinShtData *) sht_remove(pstHashTab,&stUinHashData,UinHashCmp,UinHashCode);
		CuAssertPtrNotNull(tc, pstUinHashData);
		CuAssert(tc, "Data ont right.", !memcmp(&stUinHashData,pstUinHashData,sizeof(stUinHashData)));
	}



	stUinHashData.lUin = 267478100;	
	stUinHashData.iIdx = -1;
	pstUinHashData = (UinShtData *)   sht_insert_unique(pstHashTab,&stUinHashData,UinHashCmp,UinHashCode);
	CuAssertPtrNotNull(tc, pstUinHashData);
	memcpy(pstUinHashData, &stUinHashData,
	sizeof(*pstUinHashData));	


	stUinHashData.lUin = 267478100;	
	stUinHashData.iIdx = -1;
	pstUinHashData = (UinShtData *)   sht_insert_force(pstHashTab,UinHashCode(&stUinHashData));
	CuAssertPtrNotNull(tc, pstUinHashData);	
	memcpy(pstUinHashData, &stUinHashData,
	sizeof(*pstUinHashData));	

	sht_dump_all(pstHashTab,stdout,NULL);
	sht_dump_valid(pstHashTab,stdout,NULL);


	stUinHashData.lUin = 267478100;	
	stUinHashData.iIdx = -1;
	pstUinHashData = (UinShtData *) sht_remove(pstHashTab,&stUinHashData,UinHashCmp,UinHashCode);
	CuAssertPtrNotNull(tc, pstUinHashData);
	CuAssert(tc, "Data ont right.", !memcmp(&stUinHashData,pstUinHashData,sizeof(stUinHashData)));



	stUinHashData.lUin = 267478100;	
	stUinHashData.iIdx = -1;
	pstUinHashData = (UinShtData *) sht_remove(pstHashTab,&stUinHashData,UinHashCmp,UinHashCode);
	CuAssertPtrNotNull(tc, pstUinHashData);
	CuAssert(tc, "Data ont right.", !memcmp(&stUinHashData,pstUinHashData,sizeof(stUinHashData)));

	stUinHashData.lUin = 267478100;
	pstUinHashData = (UinShtData *) sht_find(pstHashTab,&stUinHashData,UinHashCmp,UinHashCode);
	CuAssertPtrEquals(tc, NULL,pstUinHashData);

	
	iRet = sht_is_empty(pstHashTab);
	CuAssertIntEquals(tc, 1, iRet);


	stUinHashData.lUin = 267478100;	
	pstUinHashData =  (UinShtData *)sht_remove(pstHashTab,&stUinHashData,UinHashCmp,UinHashCode);
	CuAssertPtrEquals(tc, NULL,pstUinHashData);
	
	sht_destroy(&pstHashTab);	
	CuAssertPtrEquals(tc, NULL,pstHashTab);
}

void testShtable_attach(CuTest* tc);
void testShtable_attach(CuTest* tc)
{
	int iRet;
	LPSHTABLE pstHashTab;
	size_t iHashTabSize;
	UinShtData stUinHashData;
	UinShtData *pstUinHashData;
	char *pszBuff;
	
	iHashTabSize = sht_get_size(MAX_APP_BUCKET, MAX_APP_ITEM, sizeof(UinShtData));

	pszBuff = calloc(1,iHashTabSize);
	CuAssertPtrNotNull(tc, pszBuff);

	pstHashTab = sht_init(pszBuff, iHashTabSize, MAX_APP_BUCKET, MAX_APP_ITEM, sizeof(UinShtData));
	CuAssertPtrNotNull(tc, pstHashTab);


	pstHashTab = sht_attach(pszBuff, iHashTabSize, MAX_APP_BUCKET, MAX_APP_ITEM, sizeof(UinShtData));
	CuAssertPtrNotNull(tc, pstHashTab);	

	stUinHashData.lUin = 267478100;	
	stUinHashData.iIdx = -1;
	pstUinHashData = (UinShtData *)   sht_insert_unique(pstHashTab,&stUinHashData,UinHashCmp,UinHashCode);
	CuAssertPtrNotNull(tc, pstUinHashData);
	
	memcpy(pstUinHashData, &stUinHashData,
			sizeof(*pstUinHashData));	

	stUinHashData.lUin = 267478100;
	pstUinHashData = (UinShtData *) sht_find(pstHashTab,&stUinHashData,UinHashCmp,UinHashCode);

	iRet = sht_is_empty(pstHashTab);
	CuAssertIntEquals(tc, 0, iRet);

	CuAssertPtrNotNull(tc, pstUinHashData);

	stUinHashData.lUin = 267478100;	
	sht_remove(pstHashTab,&stUinHashData,UinHashCmp,UinHashCode);
	
	sht_destroy(&pstHashTab);

	free(pszBuff);
}

void testShtableOverAlloc(CuTest* tc)
{
	int iRet;
	LPSHTABLE pstHashTab;
	size_t iHashTabSize;
	UinShtData stUinHashData;
	UinShtData *pstUinHashData;


	pstHashTab = sht_create(MAX_APP_BUCKET, MAX_APP_ITEM, sizeof(UinShtData), &iHashTabSize);
	CuAssertPtrNotNull(tc, pstHashTab);

	stUinHashData.lUin = 267478100;	
	stUinHashData.iIdx = -1;
	pstUinHashData = (UinShtData *)   sht_insert_unique(pstHashTab,&stUinHashData,UinHashCmp,UinHashCode);
	CuAssertPtrNotNull(tc, pstUinHashData);
	
	memcpy(pstUinHashData, &stUinHashData,
			sizeof(*pstUinHashData));	

	stUinHashData.lUin = 267478100;
	pstUinHashData = (UinShtData *) sht_find(pstHashTab,&stUinHashData,UinHashCmp,UinHashCode);

	iRet = sht_is_empty(pstHashTab);
	CuAssertIntEquals(tc, 0, iRet);

	CuAssertPtrNotNull(tc, pstUinHashData);

	stUinHashData.lUin = 267478100;	
	sht_remove(pstHashTab,&stUinHashData,UinHashCmp,UinHashCode);
	
	sht_destroy(&pstHashTab);	
}

int pfnCompare(const void *p1,const void *p2)
{
	return strncmp(p1,p2,100);
}

void testShtableCode(CuTest* tc);
void testShtableCode(CuTest* tc)
{

	LPSHTABLE pstHashTab;
	size_t iHashTabSize;
	UinShtData *pstUinHashData;

	char szConst[100];

	memset(szConst,0,sizeof(szConst));

	pstHashTab = sht_create(MAX_APP_BUCKET, MAX_APP_ITEM, sizeof(szConst), &iHashTabSize);
	CuAssertPtrNotNull(tc, pstHashTab);

	strncpy(szConst,"Hello",sizeof(szConst));
	pstUinHashData = (UinShtData *)   sht_insert_unique(pstHashTab,szConst,pfnCompare,(SHT_HASHCODE)sht_get_code);
	CuAssertPtrNotNull(tc, pstUinHashData);
	
	memcpy(pstUinHashData,szConst,
			sizeof(szConst));	

	strncpy(szConst,"Hello",sizeof(szConst));
	pstUinHashData = (UinShtData *)   sht_insert_unique(pstHashTab,szConst,pfnCompare,(SHT_HASHCODE)sht_get_code);
	CuAssertPtrEquals(tc, NULL,pstUinHashData);

	sht_destroy(&pstHashTab);	
}



void testShtablePos(CuTest* tc);
void testShtablePos(CuTest* tc)
{
	int iRet;
	LPSHTABLE pstHashTab;
	size_t iHashTabSize;
	UinShtData stUinHashData;
	UinShtData *pstUinHashData;
	int i;
	int fValid;

	pstHashTab = sht_create(MAX_APP_BUCKET, MAX_APP_ITEM, sizeof(UinShtData), &iHashTabSize);
	CuAssertPtrNotNull(tc, pstHashTab);

	stUinHashData.lUin = 267478100;	
	stUinHashData.iIdx = -1;
	pstUinHashData = (UinShtData *)   sht_insert_unique(pstHashTab,&stUinHashData,UinHashCmp,UinHashCode);
	CuAssertPtrNotNull(tc, pstUinHashData);
	
	memcpy(pstUinHashData, &stUinHashData,
			sizeof(*pstUinHashData));	

	for(i = 0;i < MAX_APP_ITEM;i++)
	{
		pstUinHashData = sht_pos(pstHashTab,i,&fValid);
		if(fValid)
		{
			pstUinHashData = sht_remove_by_pos(pstHashTab,i);
			CuAssertPtrNotNull(tc, pstUinHashData);
		}
	}
	
	iRet = sht_is_empty(pstHashTab);
	CuAssertIntEquals(tc, 1, iRet);

	CuAssertPtrNotNull(tc, pstUinHashData);

	stUinHashData.lUin = 267478100;	
	sht_remove(pstHashTab,&stUinHashData,UinHashCmp,UinHashCode);
	
	sht_destroy(&pstHashTab);	
}


CuSuite* CuGetSuite(void)
{
	CuSuite* suite = CuSuiteNew();
	SUITE_ADD_TEST(suite,testShtableAlloc);
	SUITE_ADD_TEST(suite,testShtable_attach);
	SUITE_ADD_TEST(suite,testShtableOverAlloc);
	SUITE_ADD_TEST(suite,testShtableCode);
	SUITE_ADD_TEST(suite,testShtablePos);

	
	return suite;
}

