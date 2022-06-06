/*
**  @file $RCSfile: ut_tstdio.c,v $
**  general description of this module
**  $Id: ut_tstdio.c,v 1.1.1.1 2008-05-28 07:35:00 kent Exp $
**  @author $Author: kent $
**  @date $Date: 2008-05-28 07:35:00 $
**  @version $Revision: 1.1.1.1 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#include "pal/pal.h"
#include <stdio.h>
#include <stdio_ext.h>
#include <CUnit/CUnit.h>

#if defined(_WIN32) || defined(_WIN64)

#pragma warning(disable:4996)

#endif

void ut_stdio(void);
void ut_tmpfile(void);
void ut_tempnam(void);

////////////////////////////////////////////
void ut_stdio(void)
{
	FILE* fp;
	int iRet;
	int i;

	fp	=	fopen("ut_stdio.txt", "wb+");
	CU_ASSERT( NULL!=fp );

	iRet	=	putw('a', fp);

	rewind(fp);
	iRet	=	getw(fp);
	CU_ASSERT( 'a'==iRet );

	iRet	=	fileno(fp);
	CU_ASSERT( -1!=iRet );

	iRet	=	fflush(fp);
	CU_ASSERT( 0==iRet );

	__fpurge(fp);

	iRet	=	fclose(fp);
	CU_ASSERT( 0==iRet );

	printf("test errno list:-----------\n");

	for(i=0; i<sys_nerr; i++)
	{
		printf("%d:\t %s\n", i, sys_errlist[i]);
	}
}

void ut_tmpfile(void)
{
	FILE* fp;

	fp	=	tmpfile();
	CU_ASSERT( NULL!=fp );

	fclose(fp);

}

void ut_tempnam(void)
{
	char* pszName;
	int i;

	for(i=0; i<5; i++)
	{
		pszName	=	tempnam(NULL, NULL);

		if( !pszName )
		{
			CU_FAIL("tempnam returan failed");
		}
		else
		{
			printf("tempnam:\t %s\n", pszName);
		}
	}

	for(i=0; i<5; i++)
	{
		pszName	=	tempnam(NULL, "tstdio");

		if( !pszName )
		{
			CU_FAIL("tempnam returan failed");
		}
		else
		{
			printf("tempnam:\t %s\n", pszName);
		}
	}
}
