/*
**  @file $RCSfile: tlogfilter.c,v $
**  general description of this module
**  $Id: tlogfilter.c,v 1.1 2008-08-05 07:04:59 steve Exp $
**  @author $Author: steve $
**  @date $Date: 2008-08-05 07:04:59 $
**  @version $Revision: 1.1 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#include "tlogfilter.h"

int tlogfilter_match(TLOGFILTER* a_pstFilter, int a_iID, int a_iCls)
{
	int fMatch1;
	int fMatch2;

	TLOGFILTER_MATCH_INT(fMatch1, &a_pstFilter->stIDFilter, a_iID);
	TLOGFILTER_MATCH_INT(fMatch2, &a_pstFilter->stClsFilter, a_iCls);

	if( fMatch1 && fMatch2 )
		return 1;
	else
		return 0;
}

int tlogfilter_match_vec(TLOGFILTERVEC* a_pstFilterVec, int a_iID, int a_iCls)
{
	int i;
	// Mickey  by default it is all pass
	if(0 == a_pstFilterVec->iCount)
	{
		return 1;
	}
	// End

	for(i=0; i<a_pstFilterVec->iCount; i++)
	{
		if( tlogfilter_match(a_pstFilterVec->astFilters+i, a_iID, a_iCls) )
			return 1;
	}	

	return 0;
}



