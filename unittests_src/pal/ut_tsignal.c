/*
**  @file $RCSfile: ut_tsignal.c,v $
**  general description of this module
**  $Id: ut_tsignal.c,v 1.1.1.1 2008-05-28 07:35:00 kent Exp $
**  @author $Author: kent $
**  @date $Date: 2008-05-28 07:35:00 $
**  @version $Revision: 1.1.1.1 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#include "pal/pal.h"
#include <CUnit/CUnit.h>

static int s_iSigCount	=	0;

#define MAX_SIG_COUNT	10

static void ut_sigalarm_handler(int a_iSig);
static void ut_sigalarm_handler_2(int a_iSig);
void ut_signal(void);
void ut_sigaction(void);
void ut_misc(void);

////////////////////////////////////////////
static void ut_sigalarm_handler(int a_iSig)
{
    TOS_UNUSED_ARG(a_iSig);
	printf("in sigalarm handler for signal.\n");
}

static void ut_sigalarm_handler_2(int a_iSig)
{
    TOS_UNUSED_ARG(a_iSig);
	s_iSigCount++;

	printf("in sigalarm handler for sigaction count=%d, max=%d.\n", s_iSigCount, MAX_SIG_COUNT);

	if( s_iSigCount>=MAX_SIG_COUNT	 )
	{
		signal(SIGALRM, SIG_IGN);
	}
}

void ut_signal(void)
{
	sighandler_t pfnDft;

	pfnDft	=	tsignal(SIGALRM, ut_sigalarm_handler);

	CU_ASSERT(pfnDft==SIG_DFL);

	raise(SIGALRM);
}	

void ut_sigaction(void)
{
	struct sigaction stSig;
	int iRet;
	int i;

	memset(&stSig, 0, sizeof(stSig));

	stSig.sa_handler	=	ut_sigalarm_handler_2;

	iRet	=	sigaction(SIGALRM, &stSig, NULL);

	for(i=0; i<MAX_SIG_COUNT; i++)
	{
		raise(SIGALRM);
	}

	CU_ASSERT(MAX_SIG_COUNT==s_iSigCount);

	s_iSigCount	=	0;

	raise(SIGALRM);

	CU_ASSERT(0==s_iSigCount);
}

void ut_misc(void)
{
}

