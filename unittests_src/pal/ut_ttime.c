/*
**  @file $RCSfile: ut_ttime.c,v $
**  general description of this module
**  $Id: ut_ttime.c,v 1.1.1.1 2008-05-28 07:35:00 kent Exp $
**  @author $Author: kent $
**  @date $Date: 2008-05-28 07:35:00 $
**  @version $Revision: 1.1.1.1 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#include "pal/pal.h"
#include <CUnit/CUnit.h>

#if defined(_WIN32) || defined(_WIN64)
	#pragma warning(disable:4996)
#endif

void ut_itimer(void);
void ut_utimes(void);
void ut_sleep(void);
void ut_usleep(void);
void ut_nanosleep(void);

static int s_iReal	=	0;
static int s_iVirtual	=	0;
static int s_iProf	=	0;
//////////////////////////////////////////
static void ut_it_real_handler(int a_iSig)
{
    TOS_UNUSED_ARG(a_iSig);
	printf("in it_real handler.\n");
	s_iReal++;
}

static void ut_it_virtual_handler(int a_iSig)
{
    TOS_UNUSED_ARG(a_iSig);
	printf("in it_virtual handler.\n");
	s_iVirtual++;
}

static void ut_it_prof_handler(int a_iSig)
{
    TOS_UNUSED_ARG(a_iSig);
	printf("in it_prof handler.\n");
	s_iProf++;
}

void ut_itimer(void)
{
	struct itimerval stIt;
	struct sigaction stSig;
	int iRet;

	memset(&stSig, 0, sizeof(stSig));

	stSig.sa_handler	=	ut_it_real_handler;
	iRet	=	sigaction(SIGALRM, &stSig, NULL);
	CU_ASSERT(0==iRet);

	stSig.sa_handler	=	ut_it_virtual_handler;
	iRet	=	sigaction(SIGVTALRM, &stSig, NULL);
	CU_ASSERT(0==iRet);

	stSig.sa_handler	=	ut_it_prof_handler;
	iRet	=	sigaction(SIGPROF, &stSig, NULL);
	CU_ASSERT(0==iRet);

	stIt.it_interval.tv_sec	=	0;
	stIt.it_interval.tv_usec=	200000;

	stIt.it_value.tv_sec	=	0;
	stIt.it_value.tv_usec	=	200000;

	iRet	=	setitimer(ITIMER_REAL, &stIt, NULL);
	CU_ASSERT(0==iRet);

	iRet	=	setitimer(ITIMER_VIRTUAL, &stIt, NULL);
	CU_ASSERT(0==iRet);

	iRet	=	setitimer(ITIMER_PROF, &stIt, NULL);
	CU_ASSERT(0==iRet);


	while( !s_iProf || !s_iVirtual || !s_iReal )
	{
		s_iProf		=	0;
		s_iVirtual	=	0;
		s_iReal		=	0;
		break;
//		usleep(1000000);
	}

	memset(&stIt, 0, sizeof(stIt));

	setitimer(ITIMER_REAL, &stIt, NULL);
	setitimer(ITIMER_VIRTUAL, &stIt, NULL);
	setitimer(ITIMER_PROF, &stIt, NULL);

	signal(SIGALRM, SIG_DFL);
	signal(SIGVTALRM, SIG_DFL);
	signal(SIGPROF, SIG_DFL);
}

#define UT_TIMEFILE		"ut_pal.dat"

void ut_utimes(void)
{
	struct timeval astTv[2];
	int iRet;
	FILE* fp;

	gettimeofday(astTv, NULL);

	memcpy(astTv+1, astTv, sizeof(astTv[0]));

	fp	=	fopen(UT_TIMEFILE, "wt");
	CU_ASSERT( NULL!=fp );

	if( fp )
		fclose(fp);

	iRet	=	utimes(UT_TIMEFILE, astTv);

	CU_ASSERT(0==iRet);
}

void ut_sleep(void)
{
	struct timeval stStart;
	struct timeval stEnd;
	int iMs;

	gettimeofday(&stStart, NULL);
	sleep(1);
	gettimeofday(&stEnd, NULL);

	stEnd.tv_sec	-=	stStart.tv_sec;
	stEnd.tv_usec	-=	stStart.tv_usec;

	TV_TO_MS(iMs, stEnd);

	CU_ASSERT(iMs>=1000 && iMs<=1100);
}

void ut_usleep(void)
{
	struct timeval stStart;
	struct timeval stEnd;
	int iMs;

	gettimeofday(&stStart, NULL);
	usleep(50000);
	gettimeofday(&stEnd, NULL);

	stEnd.tv_sec	-=	stStart.tv_sec;
	stEnd.tv_usec	-=	stStart.tv_usec;

	TV_TO_MS(iMs, stEnd);

	CU_ASSERT(iMs>=40 && iMs<=100);
}

void ut_nanosleep(void)
{
	struct timeval stStart;
	struct timeval stEnd;
	struct timespec stReq;
	struct timespec stRem;
	int iMs;

	stReq.tv_sec	=	0;
	stReq.tv_nsec	=	36000000;

	gettimeofday(&stStart, NULL);
	nanosleep(&stReq, &stRem);
	gettimeofday(&stEnd, NULL);

	stEnd.tv_sec	-=	stStart.tv_sec;
	stEnd.tv_usec	-=	stStart.tv_usec;

	TV_TO_MS(iMs, stEnd);

	CU_ASSERT(iMs>=26 && iMs<=86);
}
