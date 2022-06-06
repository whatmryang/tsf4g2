#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cutest/CuTest.h"


#include "tlog/tlog.h"
#include "tlog/tlog_layout.h"

void TestTlogFormateLayout(CuTest* tc);

void TestTlogFormateLayout(CuTest* tc)
{

	TLOGEVENTBASE stEvt;
	const TLOGLOCINFO locinfo = TLOG_LOC_INFO_INITIALIZER(NULL);
	char buff[] = "Hello";
	int bufflen = strlen(buff);

	char outBuff[1024];
	int outBuffLen = sizeof(outBuff);

	memset(&stEvt,0,sizeof(stEvt));

	stEvt.evt_priority = TLOG_PRIORITY_DEBUG;
	stEvt.evt_id = 2;
	stEvt.evt_cls  = 3;
	stEvt.evt_is_msg_binary = 0;
	stEvt.evt_msg = buff;
	stEvt.evt_msg_len = bufflen;
	stEvt.evt_loc = &locinfo;

	stEvt.evt_rendered_msg=outBuff;
	stEvt.evt_rendered_msg_len = outBuffLen;


	stEvt.evt_buffer.buf_size= 0;
	stEvt.evt_buffer.buf_maxsize = outBuffLen;
	stEvt.evt_buffer.buf_data=outBuff;

	stEvt.evt_moduleName ="moduleName";
	stEvt.evt_hostName ="hostName";

	tlog_layout_format(&stEvt, "%c%d%u%m%p%f%F%l%n%h%M");

	CuAssertStrEquals(tc,"(null)19700101 08:00:00000000HelloDEBUG   unittests_src/tlog/tlogcutest.cTestTlogFormateLayout19\nhostNamemoduleName",outBuff);

}



CuSuite* CuGetSuite(void)
{
	CuSuite* suite = CuSuiteNew();
	SUITE_ADD_TEST(suite,TestTlogFormateLayout);

	return suite;
}
