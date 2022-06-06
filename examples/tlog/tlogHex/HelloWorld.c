/*
**  @file $RCSfile: HelloWorld.c,v $
**  general description of this module
**  $Id: HelloWorld.c,v 1.1 2009-03-30 02:07:59 kent Exp $
**  @author $Author: kent $
**  @date $Date: 2009-03-30 02:07:59 $
**  @version $Revision: 1.1 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#include "pal/tos.h"
#include "tdr/tdr.h"
#include "tlog/tlog.h"
#include "tloghelp/tlogload.h"

int main(int argc, char* argv[])
{
	LPTLOGCTX g_pstLogCtx;
	LPTLOGCATEGORYINST pstCat;
	g_pstLogCtx = tlog_init_from_file("tlogconf.xml");

	pstCat = tlog_get_category(g_pstLogCtx, "test");
	if (NULL == pstCat)
	{
		return -1;
	}

	while(1)
	{
	tlog_log(pstCat,TLOG_PRIORITY_CRIT , 0, 0, "hello world info");
	tlog_fatal_hex(pstCat,0,0,"abcdefghijklmn",10);
	tlog_fatal(pstCat, 0, 0, TLOG_FMT("hello world warn"));
	tlog_alert(pstCat, 0, 0, "hello world warn");
	tlog_crit(pstCat, 0, 0, "hello world warn");
	tlog_error(pstCat, 0, 0, "hello world warn");
	tlog_warn(pstCat, 0, 0, "hello world warn");
	tlog_notice(pstCat, 0, 0, "hello world warn");
	tlog_info(pstCat, 0, 0, "hello world warn");
	tlog_debug(pstCat, 0, 0, "hello world warn");
	tlog_trace(pstCat, 0, 0, "hello world warn");
	break;//shleep(1);
	}
	tlog_fini_ctx(&g_pstLogCtx);
	return 0;
}


