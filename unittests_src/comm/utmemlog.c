#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cutest/CuTest.h"
#include "comm/comm.h"


#include "../lib_src/comm/memlog.h"
#if 0
void testMemlog(CuTest* tc);

void testMemlog(CuTest* tc)
{
	LPMEMLOG pstMemlog;
	int iRet;
	//int i;	
	char *t;
	iRet = tmemlog_new(&pstMemlog,1000000,"t");
	CuAssertIntEquals(tc, 0, iRet);
	t = malloc(1024);
	memset(t, 'a', 1024);
	t[1023] = '\n';
	//for(i = 0;i < 10000;i++)
	//{
	//	iRet = tmemlog_append(pstMemlog,t,1024);
	//}

	tmemlog_log(pstMemlog,"Hello%d",1);
	tmemlog_log(pstMemlog,"Hello%d",2);

	tmemlog_dump_to_fd(pstMemlog, stdout);
	
	CuAssertIntEquals(tc, 0, iRet);
	tmemlog_fini(pstMemlog,1);
}
#endif
CuSuite* CuGetSuite(void)
{
	CuSuite* suite = CuSuiteNew();
	//SUITE_ADD_TEST(suite,testMemlog);
	return suite;
}

