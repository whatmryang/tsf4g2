/** @file $RCSfile: cunit_tsec.h,v $
  general description of this module
  $Id: cunit_tsec.h,v 1.1.1.1 2008-05-28 07:35:00 kent Exp $
@author $Author: kent $
@date $Date: 2008-05-28 07:35:00 $
@version $Revision: 1.1.1.1 $
@note Editor: Vim 6.3, Gcc 4.0.2, tab=4
@note Platform: Linux
*/


#ifndef _CUNIT_TSEC_H
#define _CUNIT_TSEC_H

#include <assert.h>

#include "CUnit/CUnit.h"
#include "CUnit/TestDB.h"
#include "CUnit/Console.h"
#include "CUnit/Basic.h"


/**
  @brief
  @retval
  @retval
  @param
  @note
*/
int AddTestInit() ;
int AddTestCrypto() ;
int AddTestHash() ;
int AddTestAcrypto() ;
int AddTestIP() ;



#endif /**< _CUNIT_TSEC_H */

