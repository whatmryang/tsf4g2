#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../lib_src/comm/comm_i.h"
#include "comm/comm.h"
#include "cutest/CuTest.h"
#include "comm/tlist.h"
#include "../lib_src/comm/tlist_i.h"

struct uttlist
{
	int iId;
	TLISTNODE stList1;
	TLISTNODE stList2;
};

typedef struct uttlist UTTLIST;

void tlist(CuTest* tc);
void tlist2(CuTest* tc);

#define ARRAYSIZE 10
#if 0
int tlist1_for_each(LPTLIST pstList,LPTLISTNODE pElement,void* pvData)
{


	if(pstData->iId == 5)
	{
		tlist_remove(pstList,pElement);
		return -1;
	}
	
	printf("The data is %d\n",pstData->iId);
	return 0;
}

int tlist1_for_each2(LPTLIST pstList,LPTLISTNODE pElement,void* pvData)
{
	UTTLIST *pstData;
	pstData = container(UTTLIST,stList1,pElement);
	tlist_remove(pstList,pElement);
	printf("The data is %d\n",pstData->iId);
	return 0;
}
#endif


void tlist(CuTest* tc)
{
//	int iRet;
	int i;

	LPTLISTNODE pstNode,pstTmp;
	
	UTTLIST astList[ARRAYSIZE]; 
	TLISTNODE stList;

	memset(astList,0,sizeof(astList));

	TLIST_INIT(&stList);

	
	//CuAssertPtrEquals(tc,sizeof(TLIST),tlist_get_size(NULL));

	for(i = 0;i < ARRAYSIZE;i++)
	{
		astList[i].iId = i;
		TLIST_INSERT_NEXT(&stList,&astList[i].stList1);
	}

	TLIST_FOR_EACH_SAFE(pstNode,pstTmp,&stList)
	{
		UTTLIST *pstData;
		pstData = container(UTTLIST,stList1,pstNode);
		printf("The value is %d\n",pstData->iId);		
	}

	TLIST_FOR_EACH_SAFE(pstNode,pstTmp,&stList)
	{
		UTTLIST *pstData;
		pstData = container(UTTLIST,stList1,pstNode);

		if(pstData->iId == 5)
			TLIST_DEL(pstNode);
		
		printf("The value is %d\n",pstData->iId);		
	}


	

	TLIST_FOR_EACH_PREV_SAFE(pstNode,pstTmp,&stList)
	{
		UTTLIST *pstData;
		pstData = container(UTTLIST,stList1,pstNode);
		printf("The value is %d\n",pstData->iId);		
	}

	
}

#if 0
void tlist2(CuTest* tc)
{
	int iRet;
	int i;
	TLIST stList;
	UTTLIST astList[ARRAYSIZE]; 
	CuAssertPtrEquals(tc,sizeof(TLIST),tlist_get_size(NULL));


	tlist_init_from_mem(&stList,astList);
	memset(astList,0,sizeof(astList));

	for(i = 0;i < ARRAYSIZE;i++)
	{
		astList[i].iId = i;
		tlist_insert_next(&stList,&astList[i].stList1);
	}

	tlist_for_each(&stList,tlist1_for_each,NULL);
	tlist_for_each(&stList,tlist1_for_each,NULL);
}
#endif

CuSuite* CuGetSuite(void)
{
	CuSuite* suite = CuSuiteNew();
	SUITE_ADD_TEST(suite,tlist);
	//SUITE_ADD_TEST(suite,tlist2);
	return suite;
}

