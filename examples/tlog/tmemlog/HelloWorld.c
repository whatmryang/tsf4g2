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
#include "../lib_src/tmng/err_stack.h"
#include "tloghelp/tlogload.h"

int main(int argc, char* argv[])
{
    LPTLOGCTX g_pstLogCtx;
    LPTLOGCATEGORYINST pstCat;

    int i;
    if(argc != 2)
    {
        printf("Error:usage:%s 10\n",argv[0]);
        i = 10;
    }
    else
    i = atoi(argv[1]);
    g_pstLogCtx = tlog_init_from_file("tlogconf.xml");

    pstCat = tlog_get_category(g_pstLogCtx, "test");
    if (NULL == pstCat)
    {
        printf("Error Failed to get tlog category\n");
        terror_dump(NULL);
        return -1;
    }

    while (i--)
    {

        tlog_log(pstCat,TLOG_PRIORITY_CRIT , 0, 0,"%c" "I am a char:""%d" "I am a short:""%d""I am a int:" "I am a long:""%d" "I am a string:" "%s"
                ,'a',
                123,
                123456789,
                123456789,
                "Hello World");
       // break;
#if 0
        tlog_fatal_bin(pstCat,0,0, 0, 0, "hello world warn",2);
        tlog_fatal(pstCat, 0, 0, TLOG_FMT("hello world warn"));
        tlog_alert(pstCat, 0, 0, "hello world warn");
        tlog_crit(pstCat, 0, 0, "hello world warn");
        tlog_error(pstCat, 0, 0, "hello world warn");
        tlog_warn(pstCat, 0, 0, "hello world warn");
        tlog_notice(pstCat, 0, 0, "hello world warn");
        tlog_info(pstCat, 0, 0, "hello world warn");
        tlog_debug(pstCat, 0, 0, "hello world warn");
        tlog_trace(pstCat, 0, 0, "hello world warn");
        //break;//shleep(1);
#endif
    }
    tlog_fini_ctx(&g_pstLogCtx);
    return 0;
}

