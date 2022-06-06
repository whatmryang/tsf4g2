
#define MAX_SOCKET	1000

#include "oi_net.h"
#include "og_bus.h"
#include "og_ini.h"
#include "og_term.h"


#define MAX_TMP_SOCK	10
#define BUSCONF_DIR "/usr/local/mmog/bus/conf/"

#define BUSRELAYSVR_VER_MAJOR		1
#define BUSRELAYSVR_VER_MINOR		0
#define BUSRELAYSVR_VER_BUILD		0

#define GETSTRVER(version , prestr ,major , minor , build)	\
	do{	sprintf(version,"%s %d.%d.%04hd" , prestr , major ,minor , build) ;  }while(0) 	

	
#if defined(__TIME__) && defined(__DATE__)

#define GETCOMPILEDATE(date)		\
	do	{ sprintf(date , "Last Compiling Date : %s %s" , __DATE__ , __TIME__) ; }while(0)							
	
#else 

#define GETCOMPILEDATE(date)		\
	do{ sprintf(date , "Last Compiling Date : Unknow") ; }while(0)						

#endif	

typedef struct {
	int iNum;
	int aiSock[MAX_TMP_SOCK];
	time_t alConnectTime[MAX_TMP_SOCK];
} TmpSocks;

typedef struct {
	int iProcID;
	char sBusDir[80];
	char sBusConfigFile[80];
	char sIP[80];
	char sPort[80];
	int iListenSock;
	int fBusLog;
	BusInterface stBus;
	TmpSocks stTmpSocks;
	BusRecvBuf *astRecv;
	CONNINFO astConns[MAX_ATTACH_CHANNEL];
}CONFIG;

TypeDesc astConfigDesc[] = {
	{"ProcID", CFG_IP, offsetof(CONFIG, iProcID)}, 
	{"BusDir", CFG_STRING, offsetof(CONFIG, sBusDir), "/usr/local/mmog/bus/conf/"},
	{"BusConfig", CFG_STRING, offsetof(CONFIG, sBusConfigFile), "bus.ini"}, 
	{"BindIP", CFG_STRING, offsetof(CONFIG, sIP), "0.0.0.0"},
	{"ListenPort", CFG_STRING, offsetof(CONFIG, sPort), "0"},
	{"BusLog", CFG_INT, offsetof(CONFIG, fBusLog), "0"},
	{""}
};

int iNfds;
fd_set Rfds, Wfds, AllFds;
int iPkgCount = 0, iByteCount = 0;

int g_iRemoteLog = 0;

int CheckTmpSocks(CONFIG *pstConfig)
{
time_t lCurr;
int i, iNum;

	time(&lCurr);
	for (i = 0; i < pstConfig->stTmpSocks.iNum; i++) {
		if (pstConfig->stTmpSocks.aiSock[i] < 0) continue;
		if (lCurr - pstConfig->stTmpSocks.alConnectTime[i] < 20) continue;
		CloseSock(&pstConfig->stTmpSocks.aiSock[i], &AllFds);
	}
	for (i = iNum = 0; i < pstConfig->stTmpSocks.iNum; i++) {
		if (pstConfig->stTmpSocks.aiSock[i] < 0) continue;
		if (i != iNum) {
			pstConfig->stTmpSocks.aiSock[iNum] = pstConfig->stTmpSocks.aiSock[i];
			pstConfig->stTmpSocks.alConnectTime[iNum] = pstConfig->stTmpSocks.alConnectTime[i];
		}
		iNum++;
	}
	pstConfig->stTmpSocks.iNum = iNum;
	return 0;
}

int AcceptConnect(CONFIG *pstConfig)
{
int i, iFlags, iNewSock, iSinSize;
struct sockaddr_in stSin;

	iSinSize = sizeof(stSin);
	iNewSock = accept(pstConfig->iListenSock, (struct sockaddr *) &stSin, &iSinSize);
	if (iNewSock < 0) {
		return -1;
	}
	CheckTmpSocks(pstConfig);
	if (pstConfig->stTmpSocks.iNum >= MAX_TMP_SOCK) {
		close(iNewSock);
		return -2;
	}
	iFlags = fcntl(iNewSock, F_GETFL, 0);
	iFlags |= O_NDELAY;
	fcntl(iNewSock, F_SETFL, iFlags);
	FD_SET(iNewSock, &AllFds);
	if (iNewSock >= iNfds) iNfds = iNewSock + 1;
	pstConfig->stTmpSocks.aiSock[pstConfig->stTmpSocks.iNum] = iNewSock;
	time(&pstConfig->stTmpSocks.alConnectTime[pstConfig->stTmpSocks.iNum]);
	pstConfig->stTmpSocks.iNum++;
	return 0;
}

AttachChannel* FindRelayChannel(BusInterface *pstBus, int iSrc, int iDst, int* iSet)
{
int i;
Channel *pstChannel;

	printf("%s -->", inet_ntoa(*(struct in_addr *)&iSrc));
	printf("%s\n", inet_ntoa(*(struct in_addr *)&iDst));
	printf("attach: %d\n", pstBus->iAttach);
	for (i = 0; i < pstBus->iAttach; i++) {
		pstChannel = &pstBus->astAttach[i].stChannel;
		if (pstChannel->iSide & CHANNEL_SIDE_UP) {
			if (pstChannel->pstHead->iDown == iDst && pstChannel->pstHead->iUp == iSrc) 
			{
				*iSet = i;
				return &pstBus->astAttach[i];
			}
		} 
		else {
			printf("%s -->", inet_ntoa(*(struct in_addr *)&pstChannel->pstHead->iUp));
			printf("%s\n", inet_ntoa(*(struct in_addr *)&pstChannel->pstHead->iDown));
			if (pstChannel->pstHead->iDown == iSrc && pstChannel->pstHead->iUp == iDst) 
			{
				*iSet = i;
				return &pstBus->astAttach[i];
			}
		}
	}

	*iSet = 0;
	return NULL;
}

int RecvTmpSock(CONFIG *pstConfig, int iIndex)
{
	BusPkgHead stHead;
	int iSock;
	AttachChannel *pstAch;
	CONNINFO* pstConnInfo;
	int i = 0;

	iSock = pstConfig->stTmpSocks.aiSock[iIndex];
	LogBus(&pstConfig->stBus, "recv tmp sock %d\n", iSock);
	if (ReadSynHead(iSock, &stHead) < 0) goto close;
	LogBus(&pstConfig->stBus, "HandShake:\n%s", HexString((char *)&stHead, sizeof(stHead)));
	pstAch = FindRelayChannel(&pstConfig->stBus, *(int *)stHead.sSrc, *(int *)stHead.sDst, &i);
	if (!pstAch) printf("no channel?\n");
	if (!pstAch) goto close;

	if( pstAch->iDst > 0 && pstAch->iDst != iSock )
	{
		pstConnInfo	= &pstConfig->astConns[i];		
		pstConnInfo->iSend	=	0;
		pstConnInfo->iTry	=	0;
		pstConnInfo->iPkgValid	= 	0;

		pstConfig->astRecv[i].iGetLen = 0;
		
		CloseSock(&pstAch->iDst, &AllFds);
		pstAch->iDst = -1;
		goto close;
	}

	switch (DealSynHead(pstAch, &stHead, iSock, &AllFds)) {
		case 3:
		case 5:
			pstConfig->stTmpSocks.aiSock[iIndex] = -1;
			CheckTmpSocks(pstConfig);
			break;
		case 2:
		case 4:
			SynSend(iSock, &pstAch->stChannel);
			break;
		default:
			goto close;
	}

	return 0;

close:
	LogBus(&pstConfig->stBus, "Close tmp sock: %d\n", pstConfig->stTmpSocks.aiSock[iIndex]);
	CloseSock(&pstConfig->stTmpSocks.aiSock[iIndex], &AllFds);
	CheckTmpSocks(pstConfig);
	return -1;
}

int StatisticMinLog(CONFIG *pstConfig, int iErrorNum)
{
	int iPkgStat = 0;
	int iByteStat = 0;
	int i;

	for (i = 0; i < pstConfig->stBus.iAttach; i++)
	{
		iPkgStat += pstConfig->astConns[i].iPkgTotal;
		iByteStat += pstConfig->astConns[i].iByteTotal;
	}
	
	LogBus(&pstConfig->stBus, "Min log: recv pkg %u, byte %u, error %d", iPkgStat, iByteStat, iErrorNum);
	return 0;
}

int StatisticDayLog(CONFIG *pstConfig, int iErrorNum)
{
	int iPkgStat = 0;
	int iByteStat = 0;
	int i;

	for (i = 0; i < pstConfig->stBus.iAttach; i++)
	{
		iPkgStat += pstConfig->astConns[i].iPkgTotal;
		iByteStat += pstConfig->astConns[i].iByteTotal;
	}
	
	LogBus(&pstConfig->stBus, "Day log: recv pkg %u, byte %u, error %d", iPkgStat, iByteStat, iErrorNum);
	return 0;
}

int main(int argc, char* argv[])
{
	CONFIG stConfig;
	struct timeval stTimeVal;
	int i;
	CONNINFO* pstConnInfo;

	int iOpt ;
	int iQurVer = 0 ;

	char szVersion[64] ;
	char szProcName[32] ;
	char szCompileTime[128] ;
	int iErrorNum = 0;
	time_t tNow, tBase;
	struct tm *pstTMCurr;
	
	while( -1 != (iOpt=getopt(argc, argv, "vV")) )
	{
		switch(iOpt)
		{
			case 'v' :
			case 'V' :
				iQurVer = 1 ;
				break ;
			default :
				break ;
				
		}
	}

	if( iQurVer)
	{
		memset( szVersion , 0 , sizeof(szVersion)) ;
		memset( szProcName , 0 , sizeof(szProcName)) ;
		memset( szCompileTime , 0 , sizeof(szCompileTime)) ;
		
		sprintf(szProcName , "%s : Version " , basename(argv[0]) ) ;
		GETSTRVER(szVersion, szProcName , BUSRELAYSVR_VER_MAJOR , BUSRELAYSVR_VER_MINOR , BUSRELAYSVR_VER_BUILD) ;
		GETCOMPILEDATE(szCompileTime) ;
		fprintf(stdout ,"%s\n" , szVersion) ;
		fprintf(stdout , "%s\n" , szCompileTime) ;
		return 0 ;
	}
	

	if (argc != 2) {
		printf("Usage: %s config_file\n", argv[0]);
		exit(0);
	}
	if( optind<argc && !strcasecmp("stop", argv[optind]) ) {
		system("killall -15 bus_relay_server");
	}
	
	if (ReadCfg(argv[1], &stConfig, astConfigDesc) < 0) exit(0);
	if (BusConnectRelayServerDir(&stConfig.stBus, stConfig.sBusDir, stConfig.iProcID) < 0) {
		printf("Can not connect bus.\n");
		exit(0);
	}
	if (stConfig.stBus.iAttach == 0) {
		printf("No attach channel!\n");
		exit(0);
	}
	stConfig.astRecv = calloc(stConfig.stBus.iAttach, sizeof(BusRecvBuf));
	if (!stConfig.astRecv) {
		printf("Can not allocate RecvBuf.\n");
		exit(0);
	}

	for (i = 0; i < stConfig.stBus.iAttach; i++) {
		stConfig.astRecv[i].iGetLen = 0;
		stConfig.astConns[i].iByteCount	= 0;
		stConfig.astConns[i].iPkgCount	= 0;
	}

	DaemonInit();
	KillPre(stConfig.stBus.iProcID);

	BusSetLog(stConfig.fBusLog);
	g_iRemoteLog = stConfig.fBusLog;

	printf("listen: %s\n", stConfig.sPort);
	stConfig.iListenSock = 0;

	CreateSockAddr(SOCK_STREAM, stConfig.sIP, stConfig.sPort, &stConfig.iListenSock);
	if (stConfig.iListenSock < 0) {
		printf("Listen fail\n");
		exit(0);
	}

	SetNBlock(stConfig.iListenSock);
	stConfig.stTmpSocks.iNum = 0;
	tBase = time(NULL);

	FD_ZERO(&AllFds);
	FD_SET(stConfig.iListenSock, &AllFds);
	iNfds = stConfig.iListenSock + 1;
	while (1) 
	{
		CheckTerm(argv[0]);
		tNow = time(NULL);
		if (tNow - tBase >= 60)
		{
			tBase = tNow;
			StatisticMinLog(&stConfig, iErrorNum);
			pstTMCurr = localtime(&tNow);
			if (pstTMCurr->tm_min == 0 && pstTMCurr->tm_hour == 0)
			{
				StatisticDayLog(&stConfig, iErrorNum);
			}
			
		}
		
		memcpy(&Rfds, &AllFds, sizeof(Rfds));
		stTimeVal.tv_sec = 0;
		stTimeVal.tv_usec = 10000;

		if (select(iNfds, &Rfds, NULL, NULL, &stTimeVal) <= 0)
		{
			if( RecvBus(&stConfig.stBus, stConfig.astConns, 2*stConfig.stBus.iAttach)<0 )
			{
				pstConnInfo	=	&stConfig.astConns[stConfig.stBus.iLast];
				
				pstConnInfo->iSend	=	0;
				pstConnInfo->iTry	=	0;
				pstConnInfo->iPkgValid	=	0;
				iErrorNum++;
				
				stConfig.astRecv[stConfig.stBus.iLast].iGetLen = 0;
				
				LogBus(&stConfig.stBus, "Closed after RecvBus. %d\n", stConfig.stBus.astAttach[stConfig.stBus.iLast].iDst);
				
				CloseSock(&stConfig.stBus.astAttach[stConfig.stBus.iLast].iDst, &AllFds);
			}
				
			continue;
		}

		if (FD_ISSET(stConfig.iListenSock, &Rfds)) {
			printf("nfds %d %d\n", iNfds, stConfig.iListenSock);
			AcceptConnect(&stConfig);
			printf("nfds %d\n", iNfds);
		}

		for (i = 0; i < stConfig.stBus.iAttach; i++) {
			if (stConfig.stBus.astAttach[i].iDst >= 0 && 
				FD_ISSET(stConfig.stBus.astAttach[i].iDst, &Rfds) && 
				ReadRemotePkg(&stConfig.stBus, &stConfig.stBus.astAttach[i], &stConfig.astRecv[i], &stConfig.astConns[i]) < 0) 
			{
				pstConnInfo	=	&stConfig.astConns[i];
				
				pstConnInfo->iSend	=	0;
				pstConnInfo->iTry	=	0;
				pstConnInfo->iPkgValid	=	0;
				iErrorNum++;
				
				stConfig.astRecv[i].iGetLen = 0;

				LogBus(&stConfig.stBus, "Closed after ReadRemotePkg. %d\n", stConfig.stBus.astAttach[i].iDst);
				
				CloseSock(&stConfig.stBus.astAttach[i].iDst, &AllFds);
			}
		}

		if (stConfig.stTmpSocks.iNum) {
			for (i = 0; i < stConfig.stTmpSocks.iNum; i++) {
				if (FD_ISSET(stConfig.stTmpSocks.aiSock[i], &Rfds)) RecvTmpSock(&stConfig, i);
			}
		}

		if( RecvBus(&stConfig.stBus, stConfig.astConns, 5*stConfig.stBus.iAttach)<0 )
		{
			pstConnInfo	=	&stConfig.astConns[stConfig.stBus.iLast];
			
			pstConnInfo->iSend	=	0;
			pstConnInfo->iTry	=	0;
			pstConnInfo->iPkgValid	=	0;
			iErrorNum++;
			
			stConfig.astRecv[stConfig.stBus.iLast].iGetLen = 0;
			
			LogBus(&stConfig.stBus, "Closed after RecvBus. %d\n", stConfig.stBus.astAttach[stConfig.stBus.iLast].iDst);
			
			CloseSock(&stConfig.stBus.astAttach[stConfig.stBus.iLast].iDst, &AllFds);
			
		}
	}
}
