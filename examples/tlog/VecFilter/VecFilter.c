/*
**  @file $RCSfile: VecFilter.c,v $
**  general description of this module
**  $Id: VecFilter.c,v 1.1 2009-03-30 03:40:40 kent Exp $
**  @author $Author: kent $
**  @date $Date: 2009-03-30 03:40:40 $
**  @version $Revision: 1.1 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#include "tdr/tdr.h"
#include "tlog/tlog.h"
#include "tloghelp/tlogload.h"

LPTLOGCTX g_stLogCtx;

int main(int argc, char* argv[])
{
	LPTLOGCATEGORYINST pstCat;

	g_stLogCtx = tlog_init_from_file( "tlogconf.xml");

	pstCat = tlog_get_category(g_stLogCtx, "VecTest");
	if (NULL == pstCat)
	{
		return -1;
	}

	tlog_info(pstCat, 1, 0, "hello world test_vec id=1");
	tlog_info(pstCat, 2, 0, "hello world test_vec id=2");
	tlog_info(pstCat, 101, 0, "hello world test_vec id=101");
	tlog_info(pstCat, 102, 0, "hello world test_vec id=102");
	
	tlog_fini_ctx(&g_stLogCtx);
	
	return 0;
}


