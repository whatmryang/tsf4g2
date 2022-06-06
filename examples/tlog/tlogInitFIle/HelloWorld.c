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

LPTLOGCTX g_pstLogCtx;

int main(int argc, char* argv[])
{

	LPTLOGCATEGORYINST pstCat;
	TOS_UNUSED_ARG(argc);
	TOS_UNUSED_ARG(argv);

	g_pstLogCtx = tlog_init_file_ctx("test",0,"test_file.log",0,10000000,1);
	pstCat = tlog_get_category(g_pstLogCtx, "test");

	if (NULL == pstCat)
	{
		return -1;
	}

	tlog_log(pstCat,TLOG_PRIORITY_CRIT , 0, 0, "hello world info");
	/*
	int tlog_fatal(TLOGCATEGORYINST *a_pstCatInst,int id,int cls , const char* a_pszFmt, ...);
    int tlog_alert(TLOGCATEGORYINST *a_pstCatInst,int id,int cls , const char* a_pszFmt, ...);
    int tlog_crit(TLOGCATEGORYINST *a_pstCatInst,int id,int cls , const char* a_pszFmt, ...);
    int tlog_error(TLOGCATEGORYINST *a_pstCatInst,int id,int cls , const char* a_pszFmt, ...);
    int tlog_warn(TLOGCATEGORYINST *a_pstCatInst,int id,int cls , const char* a_pszFmt, ...);
    int tlog_notice(TLOGCATEGORYINST *a_pstCatInst,int id,int cls , const char* a_pszFmt, ...);
    int tlog_info(TLOGCATEGORYINST *a_pstCatInst,int id,int cls , const char* a_pszFmt, ...);
    int tlog_debug(TLOGCATEGORYINST *a_pstCatInst,int id,int cls , const char* a_pszFmt, ...);
	*/
	tlog_fatal(pstCat, 0, 0, TLOG_FMT("hello world warn"));
	tlog_alert(pstCat, 0, 0, "hello world warn");
	tlog_crit(pstCat, 0, 0, "hello world warn");
	tlog_error(pstCat, 0, 0, "hello world warn");
	tlog_warn(pstCat, 0, 0, "hello world warn");
	tlog_notice(pstCat, 0, 0, "hello world warn");
	tlog_info(pstCat, 0, 0, "hello world warn");
	tlog_debug(pstCat, 0, 0, "hello world warn");
	tlog_trace(pstCat, 0, 0, "hello world warn");


#if 0
	//struct timeval xxbegin, xxend;
	for(i = 0;i < 100000000;i++)
	{
        long long llInter = 0;
        gettimeofday(&xxbegin, NULL);
	tlog_warn(pstCat, 0, 0, "hello world warn");
#endif
	tlog_fini_ctx(&g_pstLogCtx);

	return 0;
}


