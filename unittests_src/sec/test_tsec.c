/** @file $RCSfile: test_tsec.c,v $
  general description of this module
  $Id: test_tsec.c,v 1.2 2008-08-13 03:56:50 jackyai Exp $
@author $Author: jackyai $
@date $Date: 2008-08-13 03:56:50 $
@version $Revision: 1.2 $
@note Editor: Vim 6.3, Gcc 4.0.2, tab=4
@note Platform: Linux
*/


#include "cunit_tsec.h"
#include "sec/tsec.h"

#if defined(_WIN32) || defined(_WIN64)
#include "applink.c"
#include "crypto/bio/bss_file.c"
#endif

#if defined(_WIN32) || defined(_WIN64)
#define RUN_PATH "T:\\tsf4g\\src\\sec\\unittest\\"
#define TEST_FILE "ut_tsec.exe"
#else
#define RUN_PATH "./"
#define TEST_FILE "ut_tsec"
#endif

#define TEST_ACRYPTO_KEY_FILE "ut_tsec.rsa"

char *g_pchSrc ;
char *g_pchDst ;
ssize_t g_iSrcLen ;
ssize_t g_iDstLen ;
ssize_t g_iRealLen ;

int init_suite(void) ;
int clean_suite(void) ;

void test_tsec_init(void) ;
void test_tsec_crypto(void) ;
void test_tsec_hash(void) ;
void test_tsec_acrypto(void) ;
void test_tsec_ip(void) ;

int readfile ( const char *apszFile, char **appszSrc, ssize_t *apiLen ) ;
int writefile ( const char *apszFile, const char *apszBuffer, const ssize_t aiLen ) ;
void freepair() ;


/* The suite tsec hash test function.
 * Returns zero on success, non-zero otherwise.
 */
int AddTestInit()
{
   CU_pSuite pSuite = NULL;

   /* add a suite to the registry */
   pSuite = CU_add_suite("TestInit", init_suite, clean_suite);
   if (NULL == pSuite) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* add the tests to the suite */
   /* NOTE - ORDER IS IMPORTANT */
   if ( NULL == CU_add_test(pSuite, "test of tsec init", test_tsec_init) )
   {
      CU_cleanup_registry();
      return CU_get_error();
   }

   return CU_get_error();
}


/* The suite tsec crypto test function.
 * Returns zero on success, non-zero otherwise.
 */
int AddTestCrypto()
{
   CU_pSuite pSuite = NULL;

   /* add a suite to the registry */
   pSuite = CU_add_suite("TestCrypto", init_suite, clean_suite);
   if (NULL == pSuite) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* add the tests to the suite */
   /* NOTE - ORDER IS IMPORTANT */
   if ( NULL == CU_add_test(pSuite, "test of tsec crypto", test_tsec_crypto) )
   {
      CU_cleanup_registry();
      return CU_get_error();
   }

   return CU_get_error();
}


/* The suite tsec hash test function.
 * Returns zero on success, non-zero otherwise.
 */
int AddTestHash()
{
   CU_pSuite pSuite = NULL;

   /* add a suite to the registry */
   pSuite = CU_add_suite("TestHash", init_suite, clean_suite);
   if (NULL == pSuite) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* add the tests to the suite */
   /* NOTE - ORDER IS IMPORTANT */
   if ( NULL == CU_add_test(pSuite, "test of tsec hash", test_tsec_hash) )
   {
      CU_cleanup_registry();
      return CU_get_error();
   }

   return CU_get_error();
}


/* The suite tsec acrypto test function.
 * Returns zero on success, non-zero otherwise.
 */
int AddTestAcrypto()
{
   CU_pSuite pSuite = NULL;

   /* add a suite to the registry */
   pSuite = CU_add_suite("TestAcrypto", init_suite, clean_suite);
   if (NULL == pSuite) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* add the tests to the suite */
   /* NOTE - ORDER IS IMPORTANT */
   if ( NULL == CU_add_test(pSuite, "test of tsec Acrypto", test_tsec_acrypto) )
   {
      CU_cleanup_registry();
      return CU_get_error();
   }

   return CU_get_error();
}


/* The suite tsec ip test function.
 * Returns zero on success, non-zero otherwise.
 */
int AddTestIP()
{
   CU_pSuite pSuite = NULL;

   /* add a suite to the registry */
   pSuite = CU_add_suite("TestIP", init_suite, clean_suite);
   if (NULL == pSuite) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* add the tests to the suite */
   /* NOTE - ORDER IS IMPORTANT */
   if ( NULL == CU_add_test(pSuite, "test of tsec IP", test_tsec_ip) )
   {
      CU_cleanup_registry();
      return CU_get_error();
   }

   return CU_get_error();
}


/* The suite initialization function.
 * Returns zero on success, non-zero otherwise.
 */
int init_suite(void)
{
    g_pchSrc = g_pchDst = NULL ;
    g_iSrcLen = g_iDstLen = 0 ;

    return 0;
}


/* The suite cleanup function.
 * Returns zero on success, non-zero otherwise.
 */
int clean_suite(void)
{
    freepair() ;

    g_iSrcLen = g_iDstLen = 0 ;

    return 0;
}


void test_tsec_init(void)
{
    int iRet = 0 ;

    /* init */
#if defined(_WIN32) || defined(_WIN64)
    iRet = tsec_init ( "T:\\tsf4g\\src\\sec\\tsec_conf.win32.xml" , NULL) ;
#else
    iRet = tsec_init ( "../tsec_conf.linux.xml", NULL ) ;
#endif

    CU_ASSERT ( 0 == iRet ) ;

    return ;
}


void test_tsec_crypto()
{
    int iRet = 0,
        iHandler = 0 ;
    char sSrcFile[256] = { 0 },
         sDstFile[256] = { 0 } ;

    /* get handler */
    iRet = tsec_open_handler ( "qqgame", &iHandler ) ;
    CU_ASSERT ( 0 == iRet ) ;
    CU_ASSERT ( 0 <= iHandler ) ;

    snprintf ( sSrcFile, sizeof(sSrcFile), "%s%s", RUN_PATH, TEST_FILE ) ;

#define TEST_CRYPTO_KEY "this is a crypto key for tsf4g-sec module"
    {
      /* encrypt */
      readfile ( sSrcFile, &g_pchSrc, &g_iSrcLen ) ;
      g_iRealLen = g_iSrcLen ;


      g_iDstLen = g_iSrcLen * 4 ;
      g_pchDst = ( char * ) malloc ( g_iDstLen ) ;
      memset ( g_pchDst, 0, g_iDstLen ) ;

      iRet = tsec_encrypt ( iHandler, TEST_CRYPTO_KEY, g_pchSrc, g_iSrcLen, g_pchDst, &g_iDstLen ) ;
      CU_ASSERT ( 0 == iRet ) ;
      CU_ASSERT ( 0 < g_iDstLen ) ;

      snprintf ( sDstFile, sizeof(sDstFile), "%sut_tsec.encrypt", RUN_PATH ) ;

      writefile ( sDstFile, g_pchDst, g_iDstLen ) ;

      freepair() ;
    }

    {
      /* descrypt */
      readfile ( sDstFile, &g_pchSrc, &g_iSrcLen ) ;

      g_iDstLen = g_iSrcLen * 4 ;
      g_pchDst = ( char * ) malloc ( g_iDstLen ) ;
      memset ( g_pchDst, 0, g_iDstLen ) ;

      iRet = tsec_descrypt ( iHandler, TEST_CRYPTO_KEY, g_pchSrc, g_iSrcLen, g_pchDst, &g_iDstLen ) ;
      CU_ASSERT ( 0 == iRet ) ;
      CU_ASSERT ( 0 < g_iDstLen ) ;
      CU_ASSERT ( g_iRealLen == g_iDstLen ) ;

      snprintf ( sDstFile, sizeof(sDstFile), "%sut_tsec.descrypt", RUN_PATH ) ;

      writefile ( sDstFile, g_pchDst, g_iDstLen ) ;

      freepair() ;
    }

    /* return handler */
    iRet = tsec_close_handler ( &iHandler ) ;
    CU_ASSERT ( 0 == iRet ) ;
    CU_ASSERT ( 0 > iHandler ) ;

    return ;
}


void test_tsec_hash()
{
    int iRet = 0,
        iHandler = 0 ;
    char sSrcFile[256] = { 0 } ;

    /* get handler */
    iRet = tsec_open_handler ( "qqpet", &iHandler ) ;
    CU_ASSERT ( 0 == iRet ) ;
    CU_ASSERT ( 0 <= iHandler ) ;

    snprintf ( sSrcFile, sizeof(sSrcFile), "%s%s", RUN_PATH, TEST_FILE ) ;

    {
      /* hash */
      char sValue1[128] = { 0 },
           sValue2[128] = { 0 } ;
      int iLen1 = 128,
          iLen2 = 128 ;

      readfile ( sSrcFile, &g_pchSrc, &g_iSrcLen ) ;

      iRet = tsec_hash ( iHandler, g_pchSrc, g_iSrcLen, sValue1, &iLen1 ) ;
      CU_ASSERT ( 0 == iRet ) ;
      CU_ASSERT ( 0 < iLen1 ) ;

      iRet = tsec_hash ( iHandler, g_pchSrc, g_iSrcLen, sValue2, &iLen2 ) ;
      CU_ASSERT ( 0 == iRet ) ;
      CU_ASSERT ( 0 < iLen2 ) ;

      CU_ASSERT ( iLen1 == iLen2 ) ;
      CU_ASSERT ( 0 == memcmp ( sValue1, sValue2, iLen1 ) ) ;

      freepair() ;
    }

    /* return handler */
    iRet = tsec_close_handler ( &iHandler ) ;
    CU_ASSERT ( 0 == iRet ) ;
    CU_ASSERT ( 0 > iHandler ) ;

    return ;
}


void test_tsec_acrypto()
{
    int iRet = 0,
        iHandler = 0 ;
    char sSrcFile[256] = { 0 },
         sDstFile[256] = { 0 } ;
    char sKeyFile[256] = { 0 } ;

    snprintf ( sKeyFile, sizeof(sKeyFile), "%s%s", RUN_PATH, TEST_ACRYPTO_KEY_FILE ) ;

    {
      /* generate key function: RSA */
      char sValue1[128] = { 0 },
           sValue2[128] = { 0 } ;
      int iLen1 = 128,
          iLen2 = 128 ;

      /* get handler */
      iRet = tsec_open_handler ( "qqtang", &iHandler ) ;
      CU_ASSERT ( 0 == iRet ) ;
      CU_ASSERT ( 0 <= iHandler ) ;

      iRet = tsec_aencrypt ( iHandler, sKeyFile, sValue1, iLen1, sValue2, &iLen2 ) ;
      CU_ASSERT ( 0 == iRet ) ;

      /* return handler */
      iRet = tsec_close_handler ( &iHandler ) ;
      CU_ASSERT ( 0 == iRet ) ;
      CU_ASSERT ( 0 > iHandler ) ;
    }

    {
      /* asymmetrical crypto function: RSA */

      /* get handler */
      iRet = tsec_open_handler ( "qqfo", &iHandler ) ;
      CU_ASSERT ( 0 == iRet ) ;
      CU_ASSERT ( 0 <= iHandler ) ;

      /* asymmetrical encrypt */
      snprintf ( sSrcFile, sizeof(sSrcFile), "%s%s", RUN_PATH, TEST_FILE ) ;

      readfile ( sSrcFile, &g_pchSrc, &g_iSrcLen ) ;
      g_iRealLen = g_iSrcLen ;

      g_iDstLen = g_iSrcLen * 4 ;
      g_pchDst = ( char * ) malloc ( g_iDstLen ) ;
      memset ( g_pchDst, 0, g_iDstLen ) ;

      iRet = tsec_aencrypt ( iHandler, sKeyFile, g_pchSrc, g_iSrcLen, g_pchDst, &g_iDstLen ) ;
      CU_ASSERT ( 0 == iRet ) ;
      CU_ASSERT ( 0 < g_iDstLen ) ;

      snprintf ( sDstFile, sizeof(sDstFile), "%sut_tsec.aencrypt", RUN_PATH ) ;

      writefile ( sDstFile, g_pchDst, g_iDstLen ) ;

      freepair() ;

      /* asymmetrical descrypt */
      readfile ( sDstFile, &g_pchSrc, &g_iSrcLen ) ;

      g_iDstLen = g_iSrcLen * 4 ;
      g_pchDst = ( char * ) malloc ( g_iDstLen ) ;
      memset ( g_pchDst, 0, g_iDstLen ) ;

      iRet = tsec_adescrypt ( iHandler, sKeyFile, g_pchSrc, g_iSrcLen, g_pchDst, &g_iDstLen ) ;
      CU_ASSERT ( 0 == iRet ) ;
      CU_ASSERT ( 0 < g_iDstLen ) ;

      CU_ASSERT ( g_iRealLen == g_iDstLen ) ;

      snprintf ( sDstFile, sizeof(sDstFile), "%sut_tsec.adescrypt", RUN_PATH ) ;

      writefile ( sDstFile, g_pchDst, g_iDstLen ) ;

      freepair() ;

      /* return handler */
      iRet = tsec_close_handler ( &iHandler ) ;
      CU_ASSERT ( 0 == iRet ) ;
      CU_ASSERT ( 0 > iHandler ) ;
    }


    return ;
}


void test_tsec_ip()
{
    int iRet = 0,
        iHandler = 0 ;

    /* get handler */
    iRet = tsec_open_handler ( "qqsg", &iHandler ) ;
    CU_ASSERT ( 0 == iRet ) ;
    CU_ASSERT ( 0 <= iHandler ) ;

    {
      /* ip */
      iRet = tsec_ip_reg ( iHandler, "127.0.0.1" ) ;
      CU_ASSERT ( 0 == iRet ) ;

      iRet = tsec_ip_unreg ( iHandler, "127.0.0.1" ) ;
      CU_ASSERT ( 0 == iRet ) ;
    }

    /* return handler */
    iRet = tsec_close_handler ( &iHandler ) ;
    CU_ASSERT ( 0 == iRet ) ;
    CU_ASSERT ( 0 > iHandler ) ;

    return ;
}


int readfile ( const char *apszFile, char **appszSrc, ssize_t *apiLen )
{
  int liRet = 0 ;
  TFSTAT ltFileStat ;
  int liFile = 0 ;

  if ( ( liFile = tfopen ( apszFile, TF_MODE_READ ) ) != -1 ) {
    if ( tflstat( apszFile, &ltFileStat ) == 0 ) {
      *appszSrc = ( char* )malloc( ltFileStat.st_size + 8 );
      if ( *appszSrc != NULL ) {
        memset ( *appszSrc, 0, ltFileStat.st_size + 8 ) ;
        if ( tfread ( liFile, *appszSrc, ltFileStat.st_size ) != ltFileStat.st_size ) {
          printf ( "GET content from file failed" ) ;
          liRet = -1 ;
        } else {
          /* get content from file successfully */
          *apiLen = ltFileStat.st_size ;

        }
      } else {
        printf ( "Malloc memory failed\n" ) ;
        liRet = -1 ;
      }
    } else {
      printf ( "Can not get ( %s ) file stat", apszFile ) ;
      liRet = -1 ;
    }
    tfclose ( liFile ) ;
  } else { //open file failed
    printf ( "Odd, open %s failed\n", apszFile ) ;
    liRet = -1 ;
  }

  return liRet ;
}


int writefile ( const char *apszFile, const char *apszBuffer, const ssize_t aiLen )
{
  int liRet = 0 ;
  int liFile ;

  if ( ( liFile = tfopen ( apszFile, TF_MODE_CREATE|TF_MODE_WRITE ) ) != -1 ) {
    if ( tfwrite ( liFile, apszBuffer, aiLen ) != aiLen ) {
      printf ( "Write content to file=%s failed\n", apszFile ) ;
      liRet = -1 ;
    } else {
      /* Write content to file OK */
    }
    //ftruncate( liFile, lseek( liFile, 0, SEEK_CUR ) );
    tfclose ( liFile ) ;
  } else {
    printf ( "Oh..., open %s for write failed\n", apszFile ) ;
    liRet = -1 ;
  }
  return liRet ;
}


void freepair()
{
    if ( NULL != g_pchSrc ) free ( g_pchSrc ) ;
    g_pchSrc = NULL ;

    if ( NULL != g_pchDst ) free ( g_pchDst ) ;
    g_pchDst = NULL ;

    return ;
}

