/** @file $RCSfile: cunit_tsec.c,v $
  general description of this module
  $Id: cunit_tsec.c,v 1.1.1.1 2008-05-28 07:35:00 kent Exp $
@author $Author: kent $
@date $Date: 2008-05-28 07:35:00 $
@version $Revision: 1.1.1.1 $
@note Editor: Vim 6.3, Gcc 4.0.2, tab=4
@note Platform: Linux
*/


#include "cunit_tsec.h"


/* The main() function for setting up and running the tests.
 * Returns a CUE_SUCCESS on successful running, another
 * CUnit error code on failure.
 */
int main()
{
   /* initialize the CUnit test registry */
   if (CUE_SUCCESS != CU_initialize_registry())
      return CU_get_error();

   /* CU_get_registry */
   assert(NULL != CU_get_registry());

   /* is running */
   assert(!CU_is_test_running());

   /* add test suite */
   AddTestInit() ;
   AddTestCrypto() ;
   AddTestHash() ;
   AddTestAcrypto() ;
   AddTestIP() ;

   /* Run all tests using the CUnit Basic interface */
   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();

   CU_cleanup_registry();

   return CU_get_error();
}

