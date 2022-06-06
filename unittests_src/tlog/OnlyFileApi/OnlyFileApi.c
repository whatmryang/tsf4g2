/*
**  @file $RCSfile: OnlyFileApi.c,v $
**  general description of this module
**  $Id: OnlyFileApi.c,v 1.1 2009-03-30 03:55:35 kent Exp $
**  @author $Author: kent $
**  @date $Date: 2009-03-30 03:55:35 $
**  @version $Revision: 1.1 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/
#include <string.h>
#include "tlog/tlog.h"
#include "../lib_src/tlog/tlogfile.h"

int main(int argc, char* argv[])
{
	int iRet;
	TLOGFILE stLogFile;
	TLOGDEVFILE stLogDev;
	char szMsg[128];

	
	memset(&stLogDev, 0, sizeof(stLogDev));
	strcpy(stLogDev.szPattern, "./test.log");
	stLogDev.iSizeLimit = 256;
	stLogDev.iMaxRotate = 3;	
	
	iRet = tlogfile_init(&stLogFile, &stLogDev);
	if (0 > iRet)
	{
		printf("tlog init err:%s\n", tlog_error_string(iRet));
		return -1;
	}
	
	strcpy(szMsg, "hello world\n");
	tlogfile_write(&stLogFile, szMsg, strlen(szMsg));
	
	tlogfile_fini(&stLogFile);

	return 0;
}


