#include "../lib_src/comm/tcommshm.h"
#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "comm/tlist.h"


#include "cutest/CuTest.h"
#include "comm/tmempool.h"
#include "comm/ttimer.h"



void testcommshm(CuTest* tc);

void testcommshm(CuTest* tc)
{
	//LPTCOMMSHM pstShm;
	int iRet = 0;
	//iRet = tcommshm_remove("test");
	//iRet = tcommshm_init(&pstShm,"test",101,NULL);
	CuAssertIntEquals(tc, 0, iRet);
}

CuSuite* CuGetSuite(void)
{
	CuSuite* suite = CuSuiteNew();
	SUITE_ADD_TEST(suite,testcommshm);
	return suite;
}


