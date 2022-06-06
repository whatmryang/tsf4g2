/*
  * changed by jessiexu
  * 新增两个ConnectStatus 结构表示内网和外网连接: stInnerConn, stOuterConn
  * 四个连接状态: DISCONNECTION, CONNECTING, HANDSHAKE, CONNECTED
  * 通过轮询方式检查当前两个连接的连接状态, 当内网确认
  * 连接成功后关闭外网连接
  */

#include "obus/oi_net.h"
#include "obus/og_ini.h"
#include "obus/og_bus.h"
#include "obus/og_term.h"
#include "obus/og_channel.h"
#include <stdarg.h>
#include <sys/procfs.h>
#include <errno.h>

#define BUSRELAYCLT_VER_MAJOR		1
#define BUSRELAYCLT_VER_MINOR		0
#define BUSRELAYCLT_VER_BUILD		0

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
	int iDst;
	int fBusLog;
	struct sockaddr_in stAddr;
//	struct sockaddr_in stAgent;
//	struct sockaddr_in stLocal;
} BusRoute;

#define MAX_BUS_ROUTE	100

typedef struct {
	int iProcID;
	int iKeepTime;		//心跳间隔
	char sBusDir[80];
	char sBusConfigFile[80];
	char sRouteConfigFile[80];
	int iRoute;
	BusRoute astRoute[MAX_BUS_ROUTE];
	BusInterface stBus;
	CONNINFO astConns[MAX_ATTACH_CHANNEL];
} CONFIG;

typedef struct {
	int iSock;
	int iStep;
	time_t tActive;
	char sDstIP[20];
	char sDstPort[6];
//	char sLocalIP[20];
//	char sLocalPort[6];
	time_t tKeepAlive;		//上次发送心跳时间
} ConnectStatus;


#define DISCONNECTION 0
#define CONNECTING 1
#define HANDSHAKE 2
#define CONNECTED 3

#define BUS_KEEP_INTERVAL			1200	//心跳间隔20分钟
#define BUS_KEEP_INTERVAL_MIN		300		//最小间隔5分钟

TypeDesc astConfigDesc[] = {
	{"ProcID", CFG_IP, offsetof(CONFIG, iProcID)}, 
	{"BusDir", CFG_STRING, offsetof(CONFIG, sBusDir), "/usr/local/mmog/bus/conf/"},
	{"BusConfig", CFG_STRING, offsetof(CONFIG, sBusConfigFile), "bus.ini"}, 
	{"RouteConfig", CFG_STRING, offsetof(CONFIG, sRouteConfigFile), "route.ini"}, 
	{"KeepTime", CFG_INT, offsetof(CONFIG, iKeepTime), "1200"}, 
	{""}
};

SectionDesc astRouteConfigDesc[] = {
	{"Route", CFG_SECTION_ARRAY, offsetof(CONFIG, astRoute), {
		{"Dst", CFG_IP, offsetof(BusRoute, iDst), "0"}, 
		{"Addr", CFG_ADDR, offsetof(BusRoute, stAddr), "127.0.0.1:0"}, 
//		{"Agent", CFG_ADDR, offsetof(BusRoute, stAgent), ""},
//		{"Local", CFG_ADDR, offsetof(BusRoute, stLocal), ""},
		{"BusLog", CFG_INT, offsetof(BusRoute, fBusLog), "0"}, 
	}, offsetof(CONFIG, iRoute), sizeof(BusRoute), MAX_BUS_ROUTE},
	{""}
};

int g_iRemoteLog = 0;

int connect_server(char* sLocalIP, char* sLocalPort, char* sServerIP, char* sServerPort)
{
	int iRet, flags;
	int iSock;
	struct sockaddr_in stServer;

	if( sServerIP==NULL || sServerPort==NULL )
		return -1;

	iSock = socket(AF_INET, SOCK_STREAM, 0);
	if( iSock <= 0 )
		return -1;
	
/*	if( sLocalIP!=NULL && sLocalPort!=NULL )
	{
		bzero(&stClient, sizeof(stClient));
		stClient.sin_family = AF_INET;
		stClient.sin_port = htons(atoi(sLocalPort));
		inet_aton(sLocalIP, &stClient.sin_addr);

		iRet = bind(iSock, (struct sockaddr*)&stClient, sizeof(stClient));
		if( iRet < 0 )
		{
			close(iSock);
			return -1;
		}
	}*/

	bzero(&stServer, sizeof(stServer));
	stServer.sin_family = AF_INET;
	stServer.sin_port = htons(atoi(sServerPort));
	inet_aton(sServerIP, &stServer.sin_addr);
	
	flags = fcntl(iSock, F_GETFL, 0);
	fcntl(iSock, F_SETFL, flags|O_NONBLOCK);

	errno = 0;
	iRet = connect(iSock, (struct sockaddr*)&stServer, sizeof(stServer));
	if( iRet < 0 )
	{
		if( errno == EINPROGRESS )
			return iSock;
		else
		{
			close(iSock);
			return -1;
		}
	}

	return iSock;
}

int DealHandShake(CONFIG *pstConfig, ConnectStatus *pstConn, AttachChannel *pstAch, fd_set *pfds, LogFile* pstLogFile)
{
	BusPkgHead stHead;
	
	if (pstConfig == NULL || pstConn == NULL || pstAch == NULL || pfds == NULL)
		return -1;
		
	if (ReadSynHead(pstConn->iSock, &stHead) < 0) {
		close(pstConn->iSock);
		pstConn->iSock = -1;
		pstConn->iStep = DISCONNECTION;
		return -1;
	}
	LogBus(&pstConfig->stBus, "HandShake\n%s", HexString((char *)&stHead, sizeof(stHead)));
	LogBus(&pstConfig->stBus, "recv %d\n", stHead.cRoute);
	switch (DealSynHead(pstAch, &stHead, pstConn->iSock, pfds)) {
		case 3:
		case 5:
			break;
		case 2:
		case 4:
			pstConn->iStep = CONNECTED;
			LogBus(&pstConfig->stBus, "Handshake is Finished\n");
			Log(pstLogFile, 1, "Handshake is Finished\n");
			break;
		default:
			close(pstConn->iSock);
			pstConn->iSock = -1;
			pstConn->iStep = DISCONNECTION;
			return -1;
	}
	
	return 0;
}


int ClearConnStat(ConnectStatus *pConn)
{
	if (pConn == NULL)
		return -1;
		
	pConn->iStep = DISCONNECTION;
	close(pConn->iSock);
	pConn->iSock = -1;
}

int CloseConnect(ConnectStatus *pInner, ConnectStatus *pOuter)
{
	if (pInner->iStep == CONNECTED)
		ClearConnStat(pInner);

	if (pOuter->iStep == CONNECTED)
		ClearConnStat(pOuter);
	
	return 0;
}

int CheckConnection(ConnectStatus *pConn, time_t tNow)
{
	if (pConn == NULL)
		return -1;
		
	if ((pConn->iStep == CONNECTING || pConn->iStep == HANDSHAKE) && tNow - pConn->tActive > 20)
	{
		ClearConnStat(pConn);
	}
	
	return 0;
}

int StatisticMinLog(CONFIG *pstConfig, int iErrorNum)
{
	
	LogBus(&pstConfig->stBus, "Min log: recv pkg %u, byte %u, error %d", 
		pstConfig->astConns[0].iPkgTotal, pstConfig->astConns[0].iByteTotal, iErrorNum);
	return 0;
}

int StatisticDayLog(CONFIG *pstConfig, int iErrorNum)
{
	LogBus(&pstConfig->stBus, "Day log: recv pkg %u, byte %u, error %d", 
		pstConfig->astConns[0].iPkgTotal, pstConfig->astConns[0].iByteTotal, iErrorNum);
	return 0;
}

int MyKillPrevious(char* sPidFilePath, LogFile* pstLog)
{
	FILE* pPidFile;
	char sPid[10], sMyName[50], sKillName[50];
	char sFile[80];
	int i, iPid, iCurPid;
	prpsinfo_t prpsinfo;

	if ((pPidFile = fopen(sPidFilePath, "r")) == NULL) {
		Log(pstLog, 1, "open File %s error: %s\n", sPidFilePath, strerror(errno));
		perror(sPidFilePath);
		return 0;
	}
	memset(sPid, 0, sizeof(sPid));
	if (!fgets(sPid, sizeof(sPid) - 1, pPidFile)) {
		Log(pstLog, 1, "File %s has no content\n", sPidFilePath);
		fclose(pPidFile);
		return -1;
	}
	fclose(pPidFile);
	iPid = atoi(sPid);
	iCurPid = getpid();

	Log(pstLog, 1, "curproc id: %d, killproc id: %d\n", iCurPid, iPid);
	sprintf(sFile, "/proc/%d/status",  iPid);
	if(access(sFile, F_OK) == 0)
		Log(pstLog, 1, "open killfile %s successful.\n", sFile);
	else
		Log(pstLog, 1, "open killfile %s error.\n", sFile);

	sprintf(sFile, "/proc/%d/status", iCurPid);
	if(access(sFile, F_OK) == 0)
		Log(pstLog, 1, "open curfile %s successful.\n", sFile);
	else
		Log(pstLog, 1, "open curfile %s error.\n", sFile);
	
	if (GetPrpsinfo(iCurPid, &prpsinfo) < 0) 
	{
		Log(pstLog, 1, "proc:%d GetPrpsinfo error\n", iCurPid);
		return -1;
	}
	memset(sMyName, 0, sizeof(sMyName));
	strncpy(sMyName, prpsinfo.pr_fname, sizeof(sMyName) - 1);

	if (GetPrpsinfo(iPid, &prpsinfo) < 0) 
	{
		Log(pstLog, 1, "proc:%d GetPrpsinfo error\n", iPid);
		return 0;
	}
	memset(sKillName, 0, sizeof(sKillName));
	strncpy(sKillName, prpsinfo.pr_fname, sizeof(sKillName) - 1);
	Log(pstLog, 1, "curporc name: %s, killproc name: %s.\n", sMyName, sKillName);

	if (strcmp(sMyName, sKillName)) return 0;
	// if (kill(iPid, SIGKILL)) return -1;

	return KillProcByPid(iPid);
// return 0;
}

int MyKillPre(int iProcID, LogFile* pstLog) 
{
char sFile[80];
struct in_addr in;

	in.s_addr = iProcID;
	sprintf(sFile, "/tmp/%s.pid", inet_ntoa(in));
	if (MyKillPrevious(sFile, pstLog) < 0) {
		Log(pstLog, 1, "can not kill previous proc. exit.\n" );
		printf("Can not kill previous process. exit\n");
		exit(0);
	}
	WritePid(sFile);
	return 0;
}

int iNfds;
fd_set Rfds, Wfds, AllFds;

int main(int argc, char* argv[])
{
	int i, iSock, iProcID, iDst;
	int error, len;
	AttachChannel *pstAch;
	CONFIG stConfig;
	time_t lCurr, tBase;
	int iErrorNum = 0;
	BusPkgHead stHead;
	struct timeval stSelect;
	BusRecvBuf stRecv;
	CONNINFO* pstConnInfo;

	int iOpt ;
	struct timeval tv;
	struct tm *pstTMCurr;
	int iQurVer = 0 ;

	char szVersion[64] ;
	char szProcName[32] ;
	char szCompileTime[128] ;
	ConnectStatus stInnerConn/*, stOuterConn*/;
	LogFile stLogFile;
	char sLogFile[80];

	memset(sLogFile, 0, sizeof(sLogFile));
	bzero(&stInnerConn, sizeof(stInnerConn));
//	bzero(&stOuterConn, sizeof(stOuterConn));
	
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
		GETSTRVER(szVersion, szProcName , BUSRELAYCLT_VER_MAJOR , BUSRELAYCLT_VER_MINOR , BUSRELAYCLT_VER_BUILD) ;
		GETCOMPILEDATE(szCompileTime) ;
		fprintf(stdout ,"%s\n" , szVersion) ;
		fprintf(stdout , "%s\n" , szCompileTime) ;
		return 0 ;
	}

	iProcID = 0;
	if (argc == 3) {
		inet_aton(argv[2], (struct in_addr *)&iProcID);
	} else if (argc != 2) {
		printf("Usage: %s config_file\n", argv[0]);
		exit(0);
	}
	if( optind<argc && !strcasecmp("stop", argv[optind]) ){
		system("killall -15 bus_relay_client");
	}

	if (ReadCfg(argv[1], &stConfig, astConfigDesc) < 0) exit(0);
	
	if ( stConfig.iKeepTime < BUS_KEEP_INTERVAL_MIN )
		stConfig.iKeepTime = BUS_KEEP_INTERVAL_MIN;
	
	strcpy(stConfig.sRouteConfigFile, argv[1]);
	if (!iProcID) iProcID = stConfig.iProcID = 0;
	if (ReadIni(stConfig.sRouteConfigFile, &stConfig, astRouteConfigDesc) < 0) {
		printf("Can not load route info.\n");
		exit(0);
	}
	if (stConfig.iRoute == 0) {
		printf("read route info error.\n");
		exit(0);
	}
	if (BusConnectRelayClientDir(&stConfig.stBus, stConfig.sBusDir, iProcID) < 0) exit(0);
	pstAch = &stConfig.stBus.astAttach[0];
	if (pstAch->stChannel.iSide & CHANNEL_SIDE_UP) iDst = pstAch->stChannel.pstHead->iUp;
	else iDst = pstAch->stChannel.pstHead->iDown;

	for (i = 0; i < stConfig.iRoute; i++) {
		printf("%s  ", inet_ntoa(*(struct in_addr *)&stConfig.astRoute[i].iDst));
		printf("%s:%d\n", inet_ntoa(stConfig.astRoute[i].stAddr.sin_addr), ntohs(stConfig.astRoute[i].stAddr.sin_port));
		if (iDst == stConfig.astRoute[i].iDst) break;
	}
	printf("route %d\n", stConfig.iRoute);
	if (i >= stConfig.iRoute) {
		printf("No route for %s\n", inet_ntoa(*(struct in_addr *)&iDst));
		exit(0);
	} 
	
	sprintf(stInnerConn.sDstIP, "%s", inet_ntoa(stConfig.astRoute[i].stAddr.sin_addr));
	sprintf(stInnerConn.sDstPort, "%d", ntohs(stConfig.astRoute[i].stAddr.sin_port));
/*	if(stConfig.astRoute[i].stAgent.sin_addr.s_addr != 0)
	{
		sprintf(stOuterConn.sDstIP, "%s", inet_ntoa(stConfig.astRoute[i].stAgent.sin_addr));
		sprintf(stOuterConn.sDstPort, "%d", ntohs(stConfig.astRoute[i].stAgent.sin_port));
		Log(&pstLogFile, 1, "AgentIP: %s AgentPort %s\n", stOuterConn.sDstIP, stOuterConn.sDstPort);
	}*/
/*	if(stConfig.astRoute[i].stLocal.sin_addr.s_addr != 0)
	{
		sprintf(stOuterConn.sLocalIP, "%s", inet_ntoa(stConfig.astRoute[i].stLocal.sin_addr));
		sprintf(stOuterConn.sLocalPort, "%d", ntohs(stConfig.astRoute[i].stLocal.sin_port));
		Log(&pstLogFile, 1, "LocalIP: %s LocalPort %s\n", stOuterConn.sLocalIP, stOuterConn.sLocalPort );
	}*/

	memset(stConfig.astConns, 0, sizeof(stConfig.astConns));

	DaemonInit();

	sprintf(sLogFile, "/tmp/busclient.%d", stConfig.stBus.iProcID);
	InitLogFile(&stLogFile, sLogFile, 0, 5, 1000000);
	MyKillPre(stConfig.stBus.iProcID, &stLogFile);

	BusSetLog(stConfig.astRoute[i].fBusLog);
	g_iRemoteLog = stConfig.astRoute[i].fBusLog;

	stInnerConn.iSock = -1;
	stInnerConn.iStep = DISCONNECTION;
	stInnerConn.tActive = 0;
	stInnerConn.tKeepAlive = time(NULL);

/*	stOuterConn.iSock = -1;
	stOuterConn.iStep = DISCONNECTION;
	stOuterConn.tActive = 0;*/

	tv.tv_sec = 5;
	tv.tv_usec = 0;

	time(&tBase);
	stRecv.iGetLen = 0;
	iNfds = 0;
	
	while (1) 
	{
		CheckTerm(argv[0]);
		time(&lCurr);
		if (lCurr - tBase >= 60)
		{
			tBase = lCurr;
			StatisticMinLog(&stConfig, iErrorNum);
			pstTMCurr = localtime(&tBase);
			if (pstTMCurr->tm_min == 0 && pstTMCurr->tm_hour == 0)
			{
				StatisticDayLog(&stConfig, iErrorNum);
			}
			
		}

		CheckConnection(&stInnerConn, lCurr);
//		CheckConnection(&stOuterConn, lCurr);

		if( stInnerConn.iStep==DISCONNECTION && lCurr-stInnerConn.tActive>1 )
		{
			stInnerConn.tActive = lCurr;
			iSock = connect_server(NULL, NULL, stInnerConn.sDstIP, stInnerConn.sDstPort);
			if( iSock > 0 )
			{
				stInnerConn.iSock = iSock;
				stInnerConn.iStep = CONNECTING;
				Log(&stLogFile, 1, "Inner Connect is Successful, iSock = %d\n", iSock );
			}
		}
		
/*		if( stOuterConn.iStep==DISCONNECTION && lCurr-stOuterConn.tActive>1 )
		{
			stOuterConn.tActive = lCurr;
			iSock = connect_server(NULL, NULL, stOuterConn.sDstIP, stOuterConn.sDstPort);
			if( iSock > 0 )
			{
				stOuterConn.iSock = iSock;
				stOuterConn.iStep = CONNECTING;
				Log(&pstLogFile, 1, "Outer Connect is Successful iSock = %d\n", iSock );
			}
		}*/

		if( stInnerConn.iSock<=0/* && stOuterConn.iSock<=0*/ ) 
		{
			sleep(1);
			continue;
		}

		FD_ZERO(&Rfds);
		FD_ZERO(&Wfds);

		if( stInnerConn.iStep == CONNECTING )	FD_SET(stInnerConn.iSock, &Wfds);
//		if( stOuterConn.iStep == CONNECTING )	FD_SET(stOuterConn.iSock, &Wfds);
		if( stInnerConn.iStep > CONNECTING )	FD_SET(stInnerConn.iSock, &Rfds);
//		if( stOuterConn.iStep > CONNECTING )	FD_SET(stOuterConn.iSock, &Rfds);
/*		if( stInnerConn.iSock > stOuterConn.iSock )
			iNfds = stInnerConn.iSock + 1;
		else
			iNfds = stOuterConn.iSock + 1;*/
		iNfds = stInnerConn.iSock + 1;

		stSelect.tv_sec = 0;
		stSelect.tv_usec = 10000;
		if( select(iNfds, &Rfds, &Wfds, NULL, &stSelect) <= 0 )
		{
			if( stInnerConn.iStep<CONNECTED /*&& stOuterConn.iStep<CONNECTED */) //
				continue;

			if( RecvBus(&stConfig.stBus, stConfig.astConns, 2)<0 )
			{
				pstConnInfo	=	&stConfig.astConns[0];

				pstConnInfo->iSend	=	0;
				pstConnInfo->iTry	=	0;
				pstConnInfo->iPkgValid	=	0;
				iErrorNum++;

				stRecv.iGetLen = 0;

				LogBus(&stConfig.stBus, "Closed After RecvBus.\n");
//				Log(&pstLogFile, 1, "Closed After RecvBus\n");

				ClearConnStat(&stInnerConn);
//				CloseConnect(&stInnerConn, &stOuterConn);
			}
			else if( lCurr - stInnerConn.tKeepAlive >= stConfig.iKeepTime )
			{	//发送保持心跳包
				KeepSend(stInnerConn.iSock, &pstAch->stChannel);
				stInnerConn.tKeepAlive = lCurr;
			}

			continue;
		}
		
		if( stInnerConn.iStep==CONNECTING && FD_ISSET(stInnerConn.iSock, &Wfds) )
		{
		 	len = sizeof(error);
			error = 0;
			if( getsockopt(stInnerConn.iSock, SOL_SOCKET, SO_ERROR, &error, &len) < 0 || error != 0 )
			{
//				Log(&pstLogFile, 1, "Inner Connect Closed After getsockopt\n" );
				ClearConnStat(&stInnerConn);
			}
			else
			{
				stInnerConn.iStep = HANDSHAKE;
				time(&lCurr);
				stInnerConn.tActive = lCurr;
				SynSend(stInnerConn.iSock, &pstAch->stChannel);
				pstAch->iDst =-1;
				setsockopt(stInnerConn.iSock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
				setsockopt(stInnerConn.iSock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));				
/*				if( stOuterConn.iSock > 0 )
				{
					ClearConnStat(&stOuterConn);
					Log(&pstLogFile, 1, "Outer Connect is Clear\n");
				}*/
			}
		}

/*		if( stOuterConn.iStep==CONNECTING && FD_ISSET(stOuterConn.iSock, &Wfds) )
		{
		 	len = sizeof(error);
			error = 0;
			if( getsockopt(stOuterConn.iSock, SOL_SOCKET, SO_ERROR, &error, &len) < 0 || error != 0 )
			{
//				Log(&pstLogFile, 1, "Outer Connect Closed After getsockopt\n" );
				ClearConnStat(&stOuterConn);
				continue;
			}
			else
			{
				stOuterConn.iStep = HANDSHAKE;
				time(&lCurr);
				stOuterConn.tActive = lCurr;
				SynSend(stOuterConn.iSock, &pstAch->stChannel);
				pstAch->iDst =-1;
				setsockopt(stOuterConn.iSock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
				setsockopt(stOuterConn.iSock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
			}
		}*/
		
		if( stInnerConn.iStep>=HANDSHAKE && FD_ISSET(stInnerConn.iSock, &Rfds) )
		{
			time(&lCurr);
			stInnerConn.tActive = lCurr;
			if( stInnerConn.iStep == HANDSHAKE )
			{
				Log(&stLogFile, 1, "Inner Connect is going to HandShake\n" );
				DealHandShake(&stConfig, &stInnerConn, pstAch, &Rfds, &stLogFile);
			}
			else if( stInnerConn.iStep==CONNECTED && ReadRemotePkg(&stConfig.stBus, pstAch, &stRecv, &stConfig.astConns[0]) < 0 )
			{
				pstConnInfo	=	&stConfig.astConns[0];
				
				pstConnInfo->iSend	=	0;
				pstConnInfo->iTry	=	0;
				pstConnInfo->iPkgValid	=	0;
				iErrorNum++;
				
				stRecv.iGetLen = 0;

				LogBus(&stConfig.stBus, "Closed after ReadRemotePkg. %d\n", iSock);
//				Log(&pstLogFile, 1, "Inner Connect Closed after ReadRemotePkg. %d\n", iSock);

				ClearConnStat(&stInnerConn);
//				CloseConnect(&stInnerConn, &stOuterConn);

				continue;
			}
		}

/*		if( stOuterConn.iStep>=HANDSHAKE && FD_ISSET(stOuterConn.iSock, &Rfds) )
		{
			time(&lCurr);
			stOuterConn.tActive = lCurr;
			if( stOuterConn.iStep == HANDSHAKE )
			{
				Log(&pstLogFile, 1, "Outer Connect is going to HandShake\n" );
				DealHandShake(&stConfig, &stOuterConn, pstAch, &Rfds, &pstLogFile);
			}
			else if( stOuterConn.iStep==CONNECTED && ReadRemotePkg(&stConfig.stBus, pstAch, &stRecv, &stConfig.astConns[0]) < 0 )
			{
				pstConnInfo	=	&stConfig.astConns[0];
				
				pstConnInfo->iSend	=	0;
				pstConnInfo->iTry	=	0;
				pstConnInfo->iPkgValid	=	0;
				iErrorNum++;
				
				stRecv.iGetLen = 0;

				LogBus(&stConfig.stBus, "Closed after ReadRemotePkg. %d\n", iSock);
//				Log(&pstLogFile, 1, "Outer Connect Closed after ReadRemotePkg. %d\n", iSock);

				CloseConnect(&stInnerConn, &stOuterConn);

				continue;
			}
		}*/

		if( stInnerConn.iStep < CONNECTED/* && stOuterConn.iStep <CONNECTED */) //
			continue;

		if( RecvBus(&stConfig.stBus, stConfig.astConns, 10)<0 )
		{
			pstConnInfo	=	&stConfig.astConns[0];
			
			pstConnInfo->iSend	=	0;
			pstConnInfo->iTry	=	0;
			pstConnInfo->iPkgValid	=	0;
			iErrorNum++;
			
			stRecv.iGetLen = 0;
			
			LogBus(&stConfig.stBus, "Closed after RecvBus.\n");
//			Log(&pstLogFile, 1, "Connect Closed after RecvBus.\n");

			ClearConnStat(&stInnerConn);
//			CloseConnect(&stInnerConn, &stOuterConn);
		}
		else if( lCurr - stInnerConn.tKeepAlive >= stConfig.iKeepTime )
		{	//发送保持心跳包
			KeepSend(stInnerConn.iSock, &pstAch->stChannel);
			stInnerConn.tKeepAlive = lCurr;
		}
		
	}
}
