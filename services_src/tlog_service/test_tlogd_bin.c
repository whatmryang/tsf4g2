/*
**  @file $RCSfile: test_tlogd_bin.c,v $
**  general description of this module
**  $Id: test_tlogd_bin.c,v 1.1 2009-01-20 02:57:01 kent Exp $
**  @author $Author: kent $
**  @date $Date: 2009-01-20 02:57:01 $
**  @version $Revision: 1.1 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#include "tlog/tlog.h"
#include "tloghelp/tlogload.h"
#include <string.h>

TLOGCONF g_stLogConf;
TLOGCTX  g_stLogCtx;

int main(int argc, char* argv[])
{
	TLOGCATEGORYINST* pstCat;
	char sBuff[1024];

	if (0 > tlog_init_cfg_from_file(&g_stLogConf, "./test_net.xml"))
	{
		printf("get_log_cat from test_net.xml fail\n");
		return -1;
	}
	if (0 > tlog_init(&g_stLogCtx, &g_stLogConf))
	{
		printf("log init fail\n");
		return -1;
	}
	pstCat = tlog_get_category(&g_stLogCtx, "test");
	if (!pstCat)
	{
		printf("get_log_cat from test_file.xml fail\n");
		return -1;
	}

	strcpy(sBuff, "hello world");
	tlog_info_bin(pstCat, 2, 3, 0, 1, sBuff, strlen(sBuff));
	
	tlog_fini(&g_stLogCtx);

	return 0;
}


