/*
**  @file $RCSfile: ut_tstdlib.c,v $
**  general description of this module
**  $Id: ut_tstdlib.c,v 1.1.1.1 2008-05-28 07:35:00 kent Exp $
**  @author $Author: kent $
**  @date $Date: 2008-05-28 07:35:00 $
**  @version $Revision: 1.1.1.1 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#include "pal/pal.h"
#include <CUnit/CUnit.h>

void ut_rand48(void);

////////////////////////////////
void ut_rand48(void)
{
	long lValue;
	long lOld;
	int i;

	srand48(time(NULL));
	lOld	=	lrand48();

	for(i=0; i<1000000; i++)
	{
		lValue	=	lrand48();
		if( lValue==lOld )
			CU_FAIL("conflict in 1000000 cycle");

		lOld	=	lValue;
	}
}

