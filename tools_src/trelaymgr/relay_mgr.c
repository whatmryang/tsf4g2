/** @file $RCSfile: relay_mgr.c,v $
  general description of this module
  $Id: relay_mgr.c,v 1.12 2009-03-31 01:43:13 jacky Exp $
@author $Author: jacky $
@date $Date: 2009-03-31 01:43:13 $
@version $Revision: 1.12 $
@note Editor: Vim 6.3, Gcc 4.0.2, tab=4
@note Platform: Linux
*/


#include "pal/pal.h"
#include "tlog/tlog.h"
#include "tdr/tdr.h"
#include "tbus/tbus_macros.h"
#include "tbus/tbus_comm.h"
#include "tbus/tbus_desc.h"
#include "relay_mgr.h"
#include "version.h"
#include "tbus/tbus_config_mng.h"
#include "tbus/tbus_log.h"
#include "tbus/tbus_kernel.h"


#define RELAY_CONFINFO "RelayMnger"

#define TRELAY_MGR_CONF_XML "trelaymgr.xml"



TRELAYMGROPTIONS g_stOption;
extern unsigned char g_szMetalib_TbusConf[] ;
static RELAYMNGER *s_ptToolConf = NULL ;
LPTBUSSHMGRM s_ptRelays = NULL;


/**
  various internal function defined
*/
static int commandLine ( int argc, char * const * argv ) ;
static int initialize ( void ) ;

static void printHelp ( ) ;
static int run ( void ) ;
static int list ( void ) ;
static int set ( void ) ;
static int del ( void ) ;
static void printHelp (void);


void printHelp (void)
{
	printf ( "\n" ) ;
	printf ( "- tbus·�ɹ���\n");

	printf ( "\n�÷�:\n" ) ;
	printf ( "relaymgr ([-W,--write] | [-D,--delete=ID] |[-L,--list]) [-C,--conf-file=FILE][-k,--bus-key=KEY] [-v] [-h]\n" );

	printf ( "\n��Ҫ����ѡ��:\n" ) ;
	printf ( "-W,--write\n"
		"\t�������ý�Tbusϵͳ����ת·�ɱ���Ϣд�������ڴ��С�\n"
        "\t����������ͨ��--conf-fileָ��Tbusϵͳ����ת·����Ϣ�������ļ���\n" );
	printf ( "-D,--delete=INDEX\n"
		"\tɾ�������ڴ���Tbus·�ɱ���ָ������ֵ��ͨ��������ֵINDEX��1��ʼ���롣\n"
		"\t����������ָ��--conf-file��--bus-keyѡ�\n");
	printf ( "-L,--list\n\t�鿴�����ڴ��е���ת·�ɱ�\n"
		"\t����������ָ��--conf-file��--bus-keyѡ�\n");

	printf ( "\n����������:\n");
	printf ( "-C,--conf-file=FILE\n"
		"\tָ������Tbus��ת·����Ϣ�������ļ���\n"
        "\t���û��ָ�������ļ���Ĭ��ֵΪ%s��\n", TRELAY_MGR_CONF_XML) ;
	printf ( "-k,--bus-key=key\n"
		"\tָ���洢Tbus��ת·�ɱ���Ϣ�Ĺ����ڴ�keyֵ��\n"
        "\t���ͬʱҲָ����--conf-fileѡ������ڴ�key�Ա�ѡ��Ϊ׼��\n");
	printf ( "-v,--version, -v\n"
		"\t�鿴���߰汾��\n" ) ;
	printf ( "-h,--help, -h\n"
		"\t�鿴�����ߵİ�����\n" ) ;

	printf("\n����ʾ��:\n");
	printf("relaymgr --conf-file=./relaymgr.xml --write\n");
	printf("tbus_mgr --delete=2\n");
	printf("tbus_mgr --list --conf-file=./relaymgr.xml\n");

	printf ( "\nAuthors: ARCH,IERD,Tencent\n" );
	printf ( "Bugs:    g_IERD_Rnd_Architecture@tencent.com\n" );
	printf ( "\n" ) ;

}


int main ( int argc, char ** argv )
{

	/* get options from command line */
	if ( commandLine ( argc, argv ) != 0 )
	{
		printf("failed to parse commandline, please check your args\n");
		return -1 ;
	}

	/* initialize global arguments */
	if ( 0 != initialize () ) return -1 ;

	return run() ; /* run the program */
}



int commandLine ( int argc, char * const * argv )
{
	static struct option s_astLongOptions[] = {
		{"conf-file", 1, NULL, 'C'},
		{"write", 0, NULL, 'W'},
		{"delete", 1, NULL, 'D'},
		{"list", 0, NULL, 'L'},
		{"bus-key", 1, NULL, 'k'},
		{"version", 0, 0, 'v'},
		{"help", 0, 0, 'h'},

		{0, 0, 0, 0}
	};

	int opt = 0;
	char* pszApp = NULL ;

	pszApp = basename ( argv[0] ) ;
	memset(&g_stOption, 0, sizeof(g_stOption));
	g_stOption.iMgrOP = TRELAY_MGR_OP_LIST;
	STRNCPY(g_stOption.szConfFile, TRELAY_MGR_CONF_XML, sizeof(g_stOption.szConfFile) ) ;


	while (1)
	{
		int option_index = 0;

		opt = getopt_long (argc, argv, "C:c:D:d:k:K:LlWwvh",
			s_astLongOptions, &option_index);

		if (opt == -1)
			break;

		switch(opt)
		{
		case 'C':
		case 'c':
			STRNCPY(g_stOption.szConfFile, optarg, sizeof(g_stOption.szConfFile) ) ;
			break;
		case 'W':
		case 'w':
			g_stOption.iMgrOP = TRELAY_MGR_OP_WRITE ;
			break;
		case 'D':
		case 'd':
			g_stOption.iMgrOP = TRELAY_MGR_OP_DELETE;
			g_stOption.iDelID = atoi(optarg) ;
			break;
		case 'L':
		case 'l':
			{
				g_stOption.iMgrOP = TRELAY_MGR_OP_LIST ;
				break;
			}
		case 'k':
		case 'K':
			{
				STRNCPY(g_stOption.szShmKey, optarg, sizeof(g_stOption.szShmKey)) ;
				break;
			}
		case 'v': /* show version. */
			printf("%s:version %d.%d.%d, build at %d.\n", pszApp, MAJOR, MINOR, REV, BUILD);
			exit(0);
			break;

		case 'h':
		case '?':
			printHelp() ;
			exit(0);
		default:
			break;
		}/*switch(opt)*/
	}



	return 0 ;
}


int initialize ( void )
{
	int iRet = TBUS_SUCCESS ;
	unsigned int iSize = 0 ;

	LPTDRMETALIB ptLib = NULL ;
	LPTDRMETA ptMeta = NULL ;
	TDRDATA stDataTemp ;



	/*��ʼ����־ϵͳ*/
	tbus_init_log();
	tbus_set_logpriority(TLOG_PRIORITY_TRACE);

	/* malloc memory */
	s_ptToolConf = ( RELAYMNGER * ) malloc ( sizeof(RELAYMNGER) ) ;
	if ( NULL == s_ptToolConf )
	{
		fprintf ( stdout, "FATAL: malloc memory failed %"PRIdPTR" bytes\n", sizeof(RELAYMNGER) ) ;
		return TBUS_ERROR ;
	}
	memset ( s_ptToolConf, 0, sizeof(RELAYMNGER) ) ;


	/* load global settings */
	 if (('\0' == g_stOption.szShmKey[0]) ||(TRELAY_MGR_OP_WRITE == g_stOption.iMgrOP))
	 {
		 ptLib = (LPTDRMETALIB) g_szMetalib_TbusConf ;
		 ptMeta = tdr_get_meta_by_name ( ptLib, RELAY_CONFINFO ) ;
		 if ( NULL == ptMeta )
		 {
			 fprintf ( stdout, "ERROR: tool initialize, tdr_get_meta_by_name() failed, error %s\n", tdr_error_string(iRet) ) ;
			 return -2 ;
		 }

		 /* tdr_dump_metalib_file ( ptLib, "/tmp/relay_mgr.metalib" ) ; */

		 stDataTemp.iBuff = sizeof (RELAYMNGER) ;
		 stDataTemp.pszBuff = (char *)s_ptToolConf ;

		 iRet = tdr_input_file ( ptMeta, &stDataTemp, g_stOption.szConfFile, 0, TDR_IO_NEW_XML_VERSION ) ;
		 if ( 0 > iRet )
		 {
			 fprintf ( stdout, "ERROR: initialize, tdr_input_file() failed %s, error %s\n", g_stOption.szConfFile, tdr_error_string(iRet) ) ;
			 return -3 ;
		 }
		 STRNCPY(g_stOption.szShmKey, s_ptToolConf->szRelayShmKey, sizeof(g_stOption.szShmKey));;
	 }



	 /* attach routes share memory */

	 if (TRELAY_MGR_OP_WRITE == g_stOption.iMgrOP)
	 {
		 iSize = sizeof(TBUSSHMGRM);
		 iRet = tbus_create_grmshm(&s_ptRelays, g_stOption.szShmKey, s_ptToolConf->iBussinessID, iSize, NULL);
	 }else
	 {
		 iRet = tbus_get_grmshm(&s_ptRelays, g_stOption.szShmKey, s_ptToolConf->iBussinessID, iSize, NULL);
	 }

	return iRet ;
}


int run ( void )
{
	int iRet = TBUS_SUCCESS ;

	switch(g_stOption.iMgrOP)
	{
	case TRELAY_MGR_OP_WRITE:
		{
			iRet = set() ;
		}
		break;
	case TRELAY_MGR_OP_DELETE:
		{
			if ( 0 < g_stOption.iDelID )
			{
				iRet = del() ;
				if ( TBUS_SUCCESS == iRet )
				{
					list() ;
				}
			}
		}
		break;
	default:
		iRet = list() ;
	}


	return iRet ;

}


int list ( void )
{
	int iRet = TBUS_SUCCESS ;
	unsigned int i = 0 ;
	LPTBUSSHMGRMHEAD pstHead;



	fprintf ( stdout, "\n========================\n" ) ;
	pstHead = &s_ptRelays->stHead;

	tbus_rdlock(&pstHead->stRWLock);
	/* share memory header list */
	{
		fprintf ( stdout, "Shm version:\t%i\n", pstHead->dwVersion ) ;
		fprintf ( stdout, "Shm key:\t%i\n", pstHead->dwShmKey ) ;
		fprintf ( stdout, "Shm size:\t%i\n", pstHead->dwShmSize ) ;
		fprintf ( stdout, "Routes max count:\t%i\n", pstHead->dwMaxCnt ) ;
		fprintf ( stdout, "Routes used count:\t%i\n", pstHead->dwUsedCnt ) ;
		tbus_dump_addrtemplet(&pstHead->stAddrTemplet, stdout);
	}

	fprintf ( stdout, "\n" ) ;

	/* share memory routes information list */
	for ( i=0; i<pstHead->dwUsedCnt; i++ )
	{
		LPTBUSSHMRELAYCNF pstShmRelay = &s_ptRelays->astRelays[i];
		fprintf ( stdout, "Route %i information:\n", i+1 ) ;

		if ( pstShmRelay->dwFlag & TBUS_RELAY_ENABLE)
		{
			fprintf ( stdout, "\tStatus %x-- enable\n", pstShmRelay->dwFlag ) ;
		}
		else
		{
			fprintf ( stdout, "\tStatus %x-- disable\n" , pstShmRelay->dwFlag) ;
		}

		fprintf ( stdout, "\tAddr -- %s\n",
			tbus_addr_nota_by_addrtemplet(&pstHead->stAddrTemplet, pstShmRelay->dwAddr)) ;
		fprintf ( stdout, "\tPriority -- %i\n", pstShmRelay->dwPriority ) ;
		fprintf ( stdout, "\tMaster connection -- %s\n", pstShmRelay->szMConn ) ;
		fprintf ( stdout, "\tSlave connection -- %s\n", pstShmRelay->szSConn ) ;
		fprintf ( stdout, "\tStrategy -- %i\n", pstShmRelay->dwStrategy ) ;
	}/*for ( i=0; i<pstHead->dwUsedCnt; i++ )*/


	tbus_unlock(&pstHead->stRWLock);

	return iRet ;
}


int set ( void )
{
	int iRet = TBUS_SUCCESS;


	iRet = tbus_set_grm(s_ptRelays, s_ptToolConf);
	if (TBUS_SUCCESS != iRet)
	{
		fprintf ( stdout, "\nfailed to write Relay information \n\n" ) ;
	}else
	{
		fprintf ( stdout, "\nRelay information writing...... done\n\n" ) ;
		list();
	}

	return iRet ;
}

int del ( void )
{
	int iRet = TBUS_SUCCESS ;
	LPTBUSSHMGRMHEAD pstHead;



	pstHead = &s_ptRelays->stHead;

	tbus_wrlock(&pstHead->stRWLock);
	/* delete specified route information */
	{
		if ( g_stOption.iDelID > (signed int)s_ptRelays->stHead.dwUsedCnt )
		{
			fprintf ( stdout, "ERROR: Illegal route id %i, current max id is %i\n",
				g_stOption.iDelID, s_ptRelays->stHead.dwUsedCnt ) ;
			return TBUS_ERR_ARG ;
		}

		memset ( &(s_ptRelays->astRelays[g_stOption.iDelID-1]), 0, sizeof(SHMRELAY) ) ;

		if ( g_stOption.iDelID != (signed int)s_ptRelays->stHead.dwUsedCnt )
		{
			/* not the last one, data moved */
			memcpy( &(s_ptRelays->astRelays[g_stOption.iDelID-1]), &(s_ptRelays->astRelays[s_ptRelays->stHead.dwUsedCnt -1]),
				sizeof(TBUSSHMRELAYCNF)) ;
		}

		s_ptRelays->stHead.dwUsedCnt -= 1 ;
	}

	s_ptRelays->stHead.dwVersion ++ ; /* version upgraded */

	tbus_set_shmgrmheader_stamp(&s_ptRelays->stHead);

	tbus_unlock(&pstHead->stRWLock);
	fprintf ( stdout, "\nSpecified 'Route %i' deleted...... done\n\n", g_stOption.iDelID ) ;


	return iRet ;
}




