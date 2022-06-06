/** @file $RCSfile: tbus_test.c,v $
  general description of this module
  $Id: tbus_test.c,v 1.15 2009-08-12 06:12:15 jacky Exp $
@author $Author: jacky $
@date $Date: 2009-08-12 06:12:15 $
@version $Revision: 1.15 $
@note Editor: Vim 6.3, Gcc 4.0.2, tab=4
@note Platform: Linux
*/

//#include <sys/wait.h> ///< for wait3() */
//#include <sys/time.h> ///< for wait3() */
//#include <sys/resource.h> ///< for wait3() */
#include <stdlib.h>
#include "pal/pal.h"
#include "tlog/tlog.h"

#include "tbus/tbus_macros.h"
#include "tbus/tbus_route.h"
#include "tbus/tbus.h"


#include "tdr/tdr.h"
#include "tdr/tdr_metalib_manage.h"
#include "tbus_test_desc.h"

#include "tbus_test_net.h"

#define TBUSTEST_CONFINFO "TbusTest"
#define TBUSTEST_ADDR_ALL "all"

typedef struct tagTOption
{
	char szGCIMKey[256]; //keep the GCIMkey info
}TOPTION;

static char s_szConf[512] = { 0 } ;
static char s_szSrcAddr[32] = { 0 },
			s_szDstAddr[32] = { 0 } ;
static int  s_iLoopTries = 1,
			s_iBatchCnt = 1,
			s_iSentOnly = 0,
			s_iVerbose = 0,
			s_iDataLen = 100,
			s_iTrace = 0,
			s_iRecvOnly = 0;
TOPTION	gs_stOption;

static int  s_iRun = 0,
			s_iReload = 0 ;
static void *g_ptBusLog = NULL ;

static TBUSTEST *s_ptTestConf = NULL ;

extern unsigned char g_szMetalib_TbusTest[] ;

static int s_iBusHandle = -1 ;
static int s_iIndex = 0 ;
static TBUSTESTPKG *s_ptBatchPkgs = NULL ;
#define log4c_category_debug //
#define log4c_category_fatal  //
#define log4c_category_error //


/**
  various internal function defined
*/
static int commandLine ( int argc, char * const * argv ) ;
static int initialize ( void ) ;
static void printHelp ( ) ;

static int run ( void ) ;
static int pack ( void ) ;
static int unpack ( const void *a_pvBuffer, const size_t a_iLen ) ;
static int dataSend ( void ) ;
static int dataRecv ( void ) ;
static unsigned int getRandom() ;


/**
  @brief
  @retval 0 --
  @retval !0 --
  @param
  @note
*/
void printHelp ( )
{
  printf ( "\n" ) ;
  printf ( "Test tool of bus module for IERD. Version 1.0\n" ) ;
  printf ( "\n" ) ;
  printf ( "tbus_test -g gcimkey [-s source address] [-r remote address] [-l loop tries] [-b batch package counts] [-h] [-v]\n" );
  printf ( " Usage: -g [gcimkey]       # GCIM key  \n" ) ;
  printf ( "        -s [source address]        # source bus address, format xxx.xxx.xxx.xxx\n" ) ;
  printf ( "        -r [remote address]        # remote bus address, format xxx.xxx.xxx.xxx\n" ) ;
  printf ( "        -l [loop tries]            # loop times, default is 1\n" ) ;
  printf ( "        -b [batch package counts]  # batch package counts, send -b packages before receiving, default is 1\n" ) ;
  printf ( "        -k                         # only send data and not receive from source address\n" ) ;
  printf ( "        -n                         # only recvdata and not send data\n" ) ;
  printf ( "        -d                         # data len\n" ) ;
  printf ( "        -t                         # enable trace \n" ) ;
  printf ( "        -v                         # verbose output\n" ) ;
  printf ( "        -h                         # print this screen\n" ) ;
  printf ( "\n" ) ;
  printf ( "\n" ) ;
  printf ( "Authors: ARCH,IERD,Tencent\n" );
  //printf ( "Bugs:    huwu@tencent.com\n" );
  printf ( "\n" ) ;
  return;
}


void test_on_sigint(int iSig)
{
    switch( iSig )
    {
    default:
        s_iRun = 0;
        break;
    }
}


/**
  @brief
  @retval 0 --
  @retval !0 --
  @param
  @note
*/
int main ( int argc, char ** argv )
{

	memset(&gs_stOption, 0, sizeof(gs_stOption));
	/* get options from command line */
	if ( commandLine ( argc, argv ) != 0 ) return -1 ;

	/* initialize global arguments */
	if ( 0 != initialize () ) return -1 ;

	return run() ; /* run the program */
}





/**
  @brief
  @retval 0 --
  @retval !0 --
  @param
  @note
*/
int commandLine ( int argc, char * const * argv )
{
	extern char *optarg ;
	int c ;
	while ( (c = getopt ( argc, argv, "d:g:s:r:l:b:tnkvh" )) != EOF ) {
		switch ( c ) {
			case 'g' : /* test tool configuration file */
				STRNCPY(gs_stOption.szGCIMKey, optarg, sizeof(gs_stOption.szGCIMKey) ) ;
				break ;
			case 's' : /* source bus address */
				STRNCPY ( s_szSrcAddr, optarg, sizeof(s_szSrcAddr) ) ;
				break ;
			case 'r' : /* remote bus address */
				STRNCPY ( s_szDstAddr, optarg, sizeof(s_szDstAddr) ) ;
				break ;
			case 'l' : /* loop tries */
				s_iLoopTries = atoi ( optarg ) ;
				break ;
			case 'b' : /* batch sent counts */
				s_iBatchCnt = atoi ( optarg ) ;
				break ;
			case 'k' : /* only send data and not receive from source address */
				s_iSentOnly = 1 ;
				break ;
			case 'n' :
				s_iRecvOnly = 1 ;
				break ;
			case 'd' :
				s_iDataLen= atoi(optarg) ;
				break ;
			case 'v' : /* verbose output */
				s_iVerbose = 1 ;
				break ;
			case 't' : /* verbose output */
				s_iTrace = 1 ;
				break ;
			case 'h' : /* print help screen */
			case '?' :
				printHelp() ;
				return -1 ;
				break ;
			default :
				printHelp() ;
				return -1 ;
				break ;
		} //end switch
	} //end while
	return 0 ;
}


/**
  @brief
  @retval 0 --
  @retval !0 --
  @param
  @note
*/
int initialize ( void )
{
	int iRet = TBUS_SUCCESS ;
	unsigned int iSize = 0 ;
    struct sigaction stAct;

	LPTDRMETALIB ptLib = NULL ;
	LPTDRMETA ptMeta = NULL ;
	TDRDATA stDataTemp ;

	if (0 >= s_iBatchCnt)
	{
		printHelp() ;
		return -1 ;
	}

	printf("CALL: tool initialize...\n" ) ;

    memset(&stAct, 0, sizeof(stAct));
    stAct.sa_handler    =    test_on_sigint;

    sigaction(SIGINT, &stAct, NULL);


	/* init bus handle */
	{
		iRet = tbus_init_ex(&gs_stOption.szGCIMKey[0], 0) ;
		if ( TBUS_SUCCESS != iRet )
		{
			printf("ERROR: initialize, tbus_init() failed %i\n", iRet ) ;
			return iRet ;
		}

		iRet = tbus_new ( &s_iBusHandle ) ;
		if ( 0 > iRet )
		{
			printf("ERROR: initialize, tbus_new() failed %i\n", iRet ) ;
			return iRet ;
		}

		if ( '\0' != *s_szSrcAddr )
		{
			iRet = tbus_bind_by_str ( s_iBusHandle, s_szSrcAddr ) ;
			if ( TBUS_SUCCESS != iRet )
			{
				printf("ERROR: initialize, tbus_bind() failed %i\n", iRet ) ;
				return iRet ;
			}
		}

		if ( ('\0' != *s_szDstAddr) && (0 != strcasecmp(s_szDstAddr, TBUSTEST_ADDR_ALL)) )
		{
			iRet = tbus_connect_by_str ( s_iBusHandle, s_szDstAddr ) ;
			if ( TBUS_SUCCESS != iRet )
			{
				printf("ERROR: initialize, tbus_connect() failed %i\n", iRet ) ;
				return iRet ;
			}
		}
		if (0  != s_iTrace)
		{
			tbus_set_logpriority(TLOG_PRIORITY_TRACE);
		}else
		{
			tbus_set_logpriority(TLOG_PRIORITY_INFO);
		}
	}

	/* init packages buffer */
	{
		int i, j;
		if (0 > s_iDataLen)
		{
			printf("invalid datalen:%d\n", s_iDataLen);
			return -1;
		}
		s_ptBatchPkgs = (TBUSTESTPKG *) malloc ( s_iBatchCnt * s_iDataLen ) ;
		if ( NULL == s_ptBatchPkgs )
		{
			printf("ERROR: initialize, malloc %i bytes for s_ptBatchPkgs failed\n",
				s_iBatchCnt * s_iDataLen ) ;
			return TBUS_ERROR ;
		}
		for (i = 0; i < s_iBatchCnt; i++)
		{
			char *pch = (char *)&s_ptBatchPkgs[i];
			for (j = 0; j < s_iDataLen; j++)
			{
				*pch = i + j;
				pch++;
			}
		}

//		memset ( s_ptBatchPkgs, 0, s_iBatchCnt * s_iDataLen ) ;
	}

	s_iRun = 1 ; /* process running */
	s_iReload = 0 ;

	printf("RETN: tool initialize...ok\n" ) ;

	return iRet ;
}


/**
  @brief
  @retval 0 --
  @retval !0 --
  @param
  @note
*/
int run ( void )
{
	int iRet = TBUS_SUCCESS,
        i = 0,
		j = 0 ;
	unsigned long long ui = 0;
	unsigned long long iSend = 0,
                  iRecv = 0 ;
	struct timeval stBegin, stEnd, stTempTime ;
	double iInterval = 0 ;
	double iPkgNum = 0 ;
	double iFlux = 0 ;
	char *pdata;

	gettimeofday ( &stBegin, NULL ) ;

    for ( i=0; i<s_iLoopTries; i++ )
	{
        if (!s_iRun)
            break;
		if ( s_iVerbose )
		{
			fprintf ( stderr, "The %llu loops #################################\n", ui+1 ) ;
		}

		if (( '\0' != *s_szDstAddr ) &&(0 == s_iRecvOnly))
		{
			for ( j=0; j<s_iBatchCnt;j++)
			{
				s_iIndex = j ;
				iRet = dataSend() ;
				if ( TBUS_SUCCESS == iRet )
				{
					iSend ++ ;
				}
			}
		}

		if ( ('\0' != *s_szSrcAddr) && (0 == s_iSentOnly) )
		{
			for ( j=0; j<s_iBatchCnt; j++)
			{
				s_iIndex = j ;
				iRet = dataRecv() ;
				if (TBUS_SUCCESS == iRet)
				{
					iRecv++;
				}

			}
		}

		//usleep ( 2 * 1000 );
        ui++;
	}

	gettimeofday ( &stEnd, NULL ) ;
	TV_DIFF(stTempTime, stEnd, stBegin);
	iInterval = (stTempTime.tv_sec)*1000.0 + (stTempTime.tv_usec)/1000.0 ;
	iPkgNum = ((iSend + iRecv) / iInterval)*1000;
	iFlux = (iPkgNum / (1024*128))*s_iDataLen;
	fprintf ( stderr, "Summary: total sent %llu, total received %llu, time %fms datalen: %d pkg:%f/s flux:%f Mbps \n",
		iSend, iRecv, iInterval, s_iDataLen, iPkgNum, iFlux) ;


	pdata = malloc(s_iDataLen);
	if (NULL == pdata)
	{
		return 0;
	}
	iSend=0;
	gettimeofday ( &stBegin, NULL ) ;
	for ( i=0; i<s_iLoopTries; i++ )
	{

		for ( j=0; j<s_iBatchCnt; j++ )
		{
			s_iIndex = j ;
			memcpy(pdata, (void *)&s_ptBatchPkgs[s_iIndex], s_iDataLen) ;
			iSend ++ ;
		}
	}
	gettimeofday ( &stEnd, NULL ) ;
	TV_DIFF(stTempTime, stEnd, stBegin);
	iInterval = (stTempTime.tv_sec)*1000.0 + (stTempTime.tv_usec)/1000.0 ;
	iPkgNum = ((iSend) / iInterval *1000);
	iFlux = (iPkgNum / (1024*128))*s_iDataLen;
	fprintf ( stderr, "Summary: memcpu total %llu, time %fms datalen: %d pkg:%f/s flux:%f Mbps \n",
		iSend, iInterval, s_iDataLen, iPkgNum, iFlux) ;

	return iRet ;
}


/**
  @brief
  @retval 0 --
  @retval !0 --
  @param
  @note
*/
int pack ( void )
{
	int iRet = TBUS_SUCCESS ;

	s_ptBatchPkgs[s_iIndex].stHead.bVersion += PKG_VERSION ;
	s_ptBatchPkgs[s_iIndex].stHead.bHeadLen = sizeof(PKGHEAD) ;
	s_ptBatchPkgs[s_iIndex].stHead.wBodyLen = sizeof(PKGBODY) ;
	s_ptBatchPkgs[s_iIndex].stHead.wCmdID = PKG_CS_CMD ;
	s_ptBatchPkgs[s_iIndex].stHead.dwSequence += 1 ;
	s_ptBatchPkgs[s_iIndex].stHead.wMagic = PKG_MAGIC ;
	/* s_ptBatchPkgs[s_iIndex].stHead.szReserved = */
	/* s_ptBatchPkgs[s_iIndex].stHead.wHeadChkSum = */

	s_ptBatchPkgs[s_iIndex].stBody.dwUin = getRandom() ;
	s_ptBatchPkgs[s_iIndex].stBody.dwCount = getRandom() ;
	s_ptBatchPkgs[s_iIndex].stBody.ullBigint = getRandom() ;
	snprintf ( s_ptBatchPkgs[s_iIndex].stBody.szBuffer, sizeof(s_ptBatchPkgs[s_iIndex].stBody.szBuffer), "%i-%i",
			s_ptBatchPkgs[s_iIndex].stHead.dwSequence, s_ptBatchPkgs[s_iIndex].stBody.dwUin ) ;
	s_ptBatchPkgs[s_iIndex].stBody.szBuffer[sizeof(s_ptBatchPkgs[s_iIndex].stBody.szBuffer)-2] = 'a' ;
	/* s_ptBatchPkgs[s_iIndex].stBody.szReserved */

	if ( s_iVerbose )
	{
		fprintf ( stderr, "Sent package(length %i):\n", sizeof(TBUSTESTPKG) ) ;
		fprintf ( stderr, "\tHead.Version -- %i\n", s_ptBatchPkgs[s_iIndex].stHead.bVersion ) ;
		fprintf ( stderr, "\tHead.HeadLen -- %i\n", s_ptBatchPkgs[s_iIndex].stHead.bHeadLen ) ;
		fprintf ( stderr, "\tHead.BodyLen -- %i\n", s_ptBatchPkgs[s_iIndex].stHead.wBodyLen ) ;
		fprintf ( stderr, "\tHead.CmdID -- %i\n", s_ptBatchPkgs[s_iIndex].stHead.wCmdID ) ;
		fprintf ( stderr, "\tHead.Sequence -- %i\n", s_ptBatchPkgs[s_iIndex].stHead.dwSequence ) ;
		fprintf ( stderr, "\tHead.Magic -- %i\n", s_ptBatchPkgs[s_iIndex].stHead.wMagic ) ;
		fprintf ( stderr, "\tBody.dwUin -- %i\n", s_ptBatchPkgs[s_iIndex].stBody.dwUin ) ;
		fprintf ( stderr, "\tBody.dwCount -- %i\n", s_ptBatchPkgs[s_iIndex].stBody.dwCount ) ;
		fprintf ( stderr, "\tBody.ullBigint -- %llu\n", s_ptBatchPkgs[s_iIndex].stBody.ullBigint ) ;
		fprintf ( stderr, "\tBody.szBuffer -- '%s'\n", s_ptBatchPkgs[s_iIndex].stBody.szBuffer ) ;
	}

	return iRet ;
}


/**
  @brief
  @retval 0 --
  @retval !0 --
  @param
  @note
*/
int unpack ( const void *a_pvBuffer, const size_t a_iLen )
{
	int iRet = TBUS_SUCCESS ;
	TBUSTESTPKG *ptRecvPkg = NULL ;

	if ( a_iLen != sizeof(TBUSTESTPKG) )
	{
		printf("ERROR: package lenth no match %i != %i\n", a_iLen, sizeof(TBUSTESTPKG) ) ;
	}

	ptRecvPkg = (TBUSTESTPKG *) a_pvBuffer ;

	if ( s_iVerbose )
	{
		fprintf ( stderr, "Received package(length %i):\n", a_iLen ) ;
		fprintf ( stderr, "\tHead.Version -- %i\n", ptRecvPkg->stHead.bVersion ) ;
		fprintf ( stderr, "\tHead.HeadLen -- %i\n", ptRecvPkg->stHead.bHeadLen ) ;
		fprintf ( stderr, "\tHead.BodyLen -- %i\n", ptRecvPkg->stHead.wBodyLen ) ;
		fprintf ( stderr, "\tHead.CmdID -- %i\n", ptRecvPkg->stHead.wCmdID ) ;
		fprintf ( stderr, "\tHead.Sequence -- %i\n", ptRecvPkg->stHead.dwSequence ) ;
		fprintf ( stderr, "\tHead.Magic -- %i\n", ptRecvPkg->stHead.wMagic ) ;
		fprintf ( stderr, "\tBody.dwUin -- %i\n", ptRecvPkg->stBody.dwUin ) ;
		fprintf ( stderr, "\tBody.dwCount -- %i\n", ptRecvPkg->stBody.dwCount ) ;
		fprintf ( stderr, "\tBody.ullBigint -- %llu\n", ptRecvPkg->stBody.ullBigint ) ;
		fprintf ( stderr, "\tBody.szBuffer -- '%s'\n", ptRecvPkg->stBody.szBuffer ) ;
	}

	/* check result */
#if 0
	/* assert ( 0 == memcmp(&s_ptBatchPkgs[s_iIndex], ptRecvPkg, sizeof(TBUSTESTPKG)) ) ; */
	if ( 0 != memcmp(&s_ptBatchPkgs[s_iIndex], ptRecvPkg, sizeof(TBUSTESTPKG)) )
	{
		fprintf ( stderr, "Ooo....weird, data not match, a bug or a mistake?\n" ) ;
	}
#endif

	return iRet ;
}


/**
  @brief
  @retval 0 --
  @retval !0 --
  @param
  @note
*/
int dataSend ( void )
{
	int iRet = TBUS_SUCCESS ;
	int iSrc = 0,
		iDst = 0 ;
	static iError = 0;
	static int iSend = 0;

	iSrc = inet_addr ( s_szSrcAddr ) ;

	if ( 0 != strcasecmp(s_szDstAddr, TBUSTEST_ADDR_ALL) )
	{
		iDst = inet_addr ( s_szDstAddr ) ;
	}
	else
	{
		iDst = TBUS_ADDR_ALL ;
	}

	//pack() ;

	iRet = tbus_send ( s_iBusHandle, &iSrc, &iDst, (void *)&s_ptBatchPkgs[s_iIndex], s_iDataLen,  0 ) ;
	if (0 != iRet)
	{
		iError++;
		/*if (iRet == TBUS_ERR_CHANNEL_FULL)
		{
			usleep(10);
		}*/

		//if (iError % 10)
		//	fprintf ( stderr, "dataSend result %i\n", iRet) ;
	}else
	{
		iSend++;
		log4c_category_fatal( g_ptBusLog, "dyed msg ID: %u\n",  tbus_get_dyedmsgid() ) ;
		//if ((0 < iSend) &&((iSend % 500) == 0))
		//	printf("Send pkg count: %d\n", iSend);
	}

	if ( s_iVerbose )
	{
		fprintf ( stderr, "data sending....., result %i\n", iRet ) ;
	}

	return iRet ;
}


/**
  @brief
  @retval 0 --
  @retval !0 --
  @param
  @note
*/
int dataRecv ( void )
{
	int iRet = TBUS_ERROR;
	int iSrc = 0,
		iDst = 0 ;
	size_t iLen = 0 ;
    char sBuffer[MAX_PKG_LEN] = { 0 } ;
    static iRecv = 0;
    struct timeval stBegin, stEnd ;
	double iInterval = 0 ;
	static iError = 0;

	gettimeofday ( &stBegin, NULL ) ;

	if ( 0 != strcasecmp(s_szDstAddr, TBUSTEST_ADDR_ALL) )
	{
		iSrc = inet_addr ( s_szDstAddr ) ;
	}
	else
	{
		iSrc = TBUS_ADDR_ALL ;
	}

	iDst = inet_addr ( s_szSrcAddr ) ;
	iLen = MAX_PKG_LEN ;

	iRet = tbus_recv ( s_iBusHandle, &iSrc, &iDst, (void *)&sBuffer, &iLen, 0 ) ;
	/*if (iRet == TBUS_ERR_CHANNEL_EMPTY)
	{
		usleep(10);
	}*/


	if ( s_iVerbose )
	{
		fprintf ( stderr, "data receiving.....result %i, len %i\n", iRet, iLen ) ;
	}

	if ( TBUS_SUCCESS == iRet )
	{
	//	unpack ( sBuffer, iLen ) ;
	}

	return iRet;
}


/**
  @brief
  @retval 0 --
  @retval !0 --
  @param
  @note
*/
unsigned int getRandom()
{
	unsigned int seed;
	struct timeval now;
	gettimeofday(&now, NULL);
	seed = now.tv_sec ^ now.tv_usec;

	//return ( rand_r(&seed)/RAND_MAX * 1688 + MIN_UIN ) ;
	return ( (rand_r(&seed) % RAND_MAX)/2 + MIN_UIN ) ;
}

