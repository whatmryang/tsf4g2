/*
**  @file $RCSfile: ut_pal.c,v $
**  general description of this module
**  $Id: ut_pal.c,v 1.1.1.1 2008-05-28 07:35:00 kent Exp $
**  @author $Author: kent $
**  @date $Date: 2008-05-28 07:35:00 $
**  @version $Revision: 1.1.1.1 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#include <CUnit/CUnit.h>
#include <CUnit/TestDB.h>
#include <CUnit/Automated.h>
#include <CUnit/Console.h>
#include <CUnit/Basic.h>
#include <CUnit/TestRun.h>

#include "pal/pal.h"
#include "ut_pal.h"
#include "ut_tstdlib.h"
#include "ut_tstdio.h"
#include "ut_ttime.h"
#include "ut_tsignal.h"
#include "ut_tthread.h"
#include "ut_tstring.h"
#include "ut_os.h"

int default_suite_init(void);
int default_suite_clean(void);

int default_suite_init(void)
{
	return 0;
}

int default_suite_clean(void)
{
	return 0;
}

void add_all_tests(void)
{
	CU_pSuite pSuite = NULL;
	CU_pTest pTest = NULL;

	tos_init(NULL);

	pSuite= CU_add_suite("ut_tstdlib", default_suite_init, default_suite_clean);
	pTest = CU_add_test(pSuite, "ut_rand48", ut_rand48);

	pSuite= CU_add_suite("ut_tstdio", default_suite_init, default_suite_clean);
	pTest = CU_add_test(pSuite, "ut_stdio", ut_stdio);
	pTest = CU_add_test(pSuite, "ut_tmpfile", ut_tmpfile);
	pTest = CU_add_test(pSuite, "ut_tempnam", ut_tempnam);

	pSuite= CU_add_suite("ut_tstring", default_suite_init, default_suite_clean);
	pTest = CU_add_test(pSuite, "ut_strptime", ut_strptime);
	pTest = CU_add_test(pSuite, "ut_strtoll", ut_strtoll);
	pTest = CU_add_test(pSuite, "ut_strtoull", ut_strtoull);

	pSuite= CU_add_suite("ut_ttime", default_suite_init, default_suite_clean);
	pTest = CU_add_test(pSuite, "ut_sleep", ut_sleep);
	pTest = CU_add_test(pSuite, "ut_usleep", ut_usleep);
	pTest = CU_add_test(pSuite, "ut_nanosleep", ut_nanosleep);
	pTest = CU_add_test(pSuite, "ut_utimes", ut_utimes);
	pTest = CU_add_test(pSuite, "ut_itimer", ut_itimer);

	pSuite= CU_add_suite("ut_tsignal", default_suite_init, default_suite_clean);
	pTest = CU_add_test(pSuite, "ut_signal", ut_signal);
	pTest = CU_add_test(pSuite, "ut_sigaction", ut_sigaction);
	pTest = CU_add_test(pSuite, "ut_misc", ut_misc);

	pSuite= CU_add_suite("ut_tthread", default_suite_init, default_suite_clean);
	pTest = CU_add_test(pSuite, "ut_pthread", ut_pthread);
	pTest = CU_add_test(pSuite, "ut_mutex", ut_mutex);
	pTest = CU_add_test(pSuite, "ut_rwlock", ut_rwlock);
	pTest = CU_add_test(pSuite, "ut_semaphore", ut_semaphore);


       pSuite= CU_add_suite("ut_os", default_suite_init, default_suite_clean);
	pTest = CU_add_test(pSuite, "ut_getexe", ut_getexe);


	tos_fini();

	return;
}
