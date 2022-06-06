/*
 *  CUnit - A Unit testing framework library for C.
 *  Copyright (C) 2001  Anil Kumar
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <CUnit/CUnit.h>
#include <CUnit/TestDB.h>
#include <CUnit/Automated.h>
#include <CUnit/Console.h>
#include <CUnit/Basic.h>
#include <CUnit/TestRun.h>

#include "pal/pal.h"
#include "ut_pal.h"

#if defined(_WIN32) || defined(_WIN64)
#ifdef _DEBUG
#pragma comment(lib,"..\\debug\\libpal.lib")
#pragma comment(lib,"..\\debug\\libcunit.lib")
#else
#pragma comment(lib,"..\\release\\libpal.lib")
#pragma comment(lib,"..\\release\\libcunit.lib")
#endif /* _DEBUG */
#endif


#define CUT_BASIC		1
#define CUT_AUTO		2
#define CUT_CONSOLE		3
#define CUT_DEFAULT		CUT_BASIC

int main(int argc, char* argv[])
{
	int opt;
	int iType	=	CUT_DEFAULT;

	setvbuf(stdout, NULL, _IONBF, 0);

	CU_set_error_action(CUEA_IGNORE);

	while( -1!= (opt=getopt(argc, argv, "abcAFIh")) )
	{
		switch(opt)
		{
		case 'a':
			iType	=	CUT_AUTO;
			break;
		case 'b':
			iType	=	CUT_BASIC;
			break;
		case 'c':
			iType	=	CUT_CONSOLE;
			break;
		case 'A':
			CU_set_error_action(CUEA_ABORT);
			break;
		case 'F':
			CU_set_error_action(CUEA_FAIL);
			break;
		case 'I':
			CU_set_error_action(CUEA_IGNORE);
			break;
		case 'h':
			printf("\nUsage:  ut_pal [option]\n\n"
               "Options:   -I   Run, ignoring framework errors [default].\n"
               "           -F   Run, failing on framework error.\n"
               "           -A   Run, aborting on framework error.\n"
               "           -a   Use CU_automated_run_tests method.\n"
               "           -b   Use CU_basic_run_tests method.\n"
               "           -c   Use CU_console_run_tests method.\n"
               "           -h   Print this message.\n\n");

			return 0;

		default:
			break;
		}
	}

	if (CU_initialize_registry())
	{
		printf("\nInitialization of Test Registry failed.");
		return -1;
	}

	add_all_tests();

	switch( iType )
	{
	case CUT_AUTO:
		CU_automated_run_tests();
		break;
	case CUT_BASIC:
		CU_basic_run_tests();
		break;
	case CUT_CONSOLE:
		CU_console_run_tests();
		break;
	default:
		break;
	}

	CU_cleanup_registry();

	return 0;

}
