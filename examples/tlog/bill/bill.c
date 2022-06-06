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
#include "pal/pal.h"
#include "pal/tos.h"
#include "tlog/tlog_bill.h"
#include "tlog/tlog.h"

int main(int argc, char* argv[])
{
	LPTLOGBILL pstBill = NULL;
	int iRet;
	int i;
	iRet = tlog_bill_init(&pstBill, 100, 100,"testbill.log");
	assert(iRet==0);

	for(i = 0;i < 30;i++)
	{
		iRet = tlog_bill_log(pstBill,"First bill log %d",i);
		assert(iRet==0);
	}

	iRet = tlog_bill_fini(&pstBill);
	assert(iRet==0);


	return 0;
}


