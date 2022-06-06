/*
**  @file $RCSfile: test_tlogd.c,v $
**  general description of this module
**  $Id: test_tlogd.c,v 1.4 2009-03-20 07:35:21 jacky Exp $
**  @author $Author: jacky $
**  @date $Date: 2009-03-20 07:35:21 $
**  @version $Revision: 1.4 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#include "tlog/tlog.h"
#include "tloghelp/tlogload.h"
#include "tdr/tdr.h"
#include "pal/tos.h"

TLOGCATEGORYINST* get_log_cat(char *pszLogConf, char *pszCatName);

LPTLOGCTX g_stLogCtx;
LPTDRMETA g_pstMeta;


TLOGCATEGORYINST* get_log_cat(char *pszLogConf, char *pszCatName)
{

	if ((g_stLogCtx = tlog_init_from_file(pszLogConf)) != NULL)
	{
		printf("log init fail\n");
		return NULL;
	}

	return tlog_get_category(g_stLogCtx, pszCatName);
}

int main(int argc, char* argv[])
{
	TLOGCATEGORYINST* pstCat;
	TOS_UNUSED_ARG(argc);
	TOS_UNUSED_ARG(argv);

	pstCat = get_log_cat("./test_file.xml", "test");
	if (!pstCat)
	{
		printf("get_log_cat from test_file.xml fail\n");
		return -1;
	}
	tlog_info(pstCat, 0, 0, "hello world test_file");
	tlog_fini_ctx(&g_stLogCtx);

	pstCat = get_log_cat("./test_net.xml", "test");
	if (!pstCat)
	{
		printf("get_log_cat from test_net.xml fail\n");
		return -1;
	}
	tlog_info(pstCat, 0, 0, "hello world test_net");
	tlog_fini_ctx(&g_stLogCtx);

	pstCat = get_log_cat("./test_vec.xml", "VecTest");
	if (!pstCat)
	{
		printf("get_log_cat from test_vec.xml fail\n");
		return -1;
	}
	tlog_info(pstCat, 1, 0, "hello world test_vec id=1");
	tlog_info(pstCat, 2, 0, "hello world test_vec id=2");
	tlog_info(pstCat, 101, 0, "hello world test_vec id=101");
	tlog_info(pstCat, 102, 0, "hello world test_vec id=102");
	tlog_fini_ctx(&g_stLogCtx);
	
	return 0;
}


