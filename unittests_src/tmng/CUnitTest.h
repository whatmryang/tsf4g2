#ifndef _CUNIT_TEST_H20070330
#define _CUNIT_TEST_H20070330

#if defined(_WIN32) || defined(_WIN64)

# ifdef _DEBUG
#  define LIB_D    "_d"
# else
#  define LIB_D
# endif /* _DEBUG */

# if defined(LIB_D)
# pragma comment( lib, "libcunit"  LIB_D ".lib" )
# else
# pragma comment( lib, "libcunit.lib" )
# endif

#endif

int AddMIBTestSuites();
#endif
