/**
*
* @file     tbusd.h
* @brief    tbusd主程序模块
*
* @author jackyai
* @version 1.0
* @date 2008-05-16
*
*
* Copyright (c)  2008, 腾讯科技有限公司互动娱乐研发部架构组
* All rights reserved.
*
*/
#ifndef TBUSD_H
#define TBUSD_H

#include <time.h>
#include "comm/tmempool.h"
#include "pal/pal.h"
#include "tbus/tbus_macros.h"
#include "tbus/tbus_route.h"
#include "tbusd_desc.h"
#include "tbus/tbus_config_mng.h"
#include "tbus/tbus_kernel.h"


#define TBUSD_MAX_FD                                     	TBUS_MAX_RELAY_NUM_PREHOST + 16 	/* relay module max epoll handle */
#define TBUSD_MAX_EVENTS                                 	TBUSD_MAX_FD 	/* relay module max epoll events */

#define  RELAY_CONFIG_DATA_META_NAME  "Tbusd" /** 服务器配置信息存储数据结构元数据描述名 */
#define RELAY_RUN_DATA_META_NAME	"TbusdStat" /** 服务器统计信息存储数据结构元数据描述名 */

#define RELAY_CONN_RECV_BUFF_SIZE	sizeof(TBUSPKG)		/*relay通道接收缓冲区容量*/

#define TBUSD_DEFAULT_LOGCONF_FILE	"./tlog.xml"		/*tbusd loge配置文件目录*/

#if defined(_WIN32) || defined(_WIN64)
#define TBUSD_EX_LOCK_FILE	"/tmp/.tbusdlock"
#else
#define TBUSD_EX_LOCK_FILE	"C:\tbusdlock"
#endif

typedef struct tagRelayEnv  RELAYENV;
typedef struct tagRelayEnv  *LPRELAYENV;

typedef enum tagRelayFdMode RELAYFDMODE;
typedef enum tagRelayFdMode *LPRELAYFDMODE;

typedef struct tagConnNode CONNNODE;
typedef struct tagConnNode *LPCONNNODE;

typedef enum tagTbusdBoolean TBUSDBOOLEAN;

typedef enum tagTbusdDataType TBUSDDATATYPE;

struct tagRelay;
typedef struct tagRelay                            	RELAY;
typedef struct tagRelay                            	*LPRELAY;

extern RELAYENV gs_stRelayEnv;

enum tagTbusdBoolean
{
	TBUSD_FALSE = 0,
	TBUSD_TRUE = 1,
};

enum tagRelayFdMode
{
	RELAY_FD_LISTEN_MODE = 1,	/**< 监听连接*/
	RELAY_FD_SERVER_MODE = 2, /**< 服务器模式，即被动打开的连接*/
	RELAY_FD_CLIENT_MODE = 3, /**< 客户端模式的连接，即主动打开的连接*/
};

enum tagTbusdDataType
{
	TBUSD_DATA_TYPE_UNKNOWN = 0,	/**< 类型不明*/
	TBUSD_DATA_TYPE_SYN = 1, /**< 同步序列号等控制数据*/
	TBUSD_DATA_TYPE_BUSINESS = 2, /**< 业务数据*/
};

struct tagSendDataMng
{
	int iSendPos;  /*在stSendPkg中已经已经发送的数据偏移，再次发送剩余数据时，从这个位置开始发送*/
	int iSendBuff;	/*需要发生数据的长度*/
	LPTBUSHEAD pstSendHead;  /*发送数据包头部*/
	TBUSDDATATYPE enRemainDataType;  /*stSendPkg中包含的剩余未发送数据的类型*/
	union tagData
	{
		char szData[TBUS_HEAD_MAX_LEN];	/*syn数据包缓冲区*/
		char *pszPkg;	/*业务数据缓冲区指针，业务数据直接保存在数据通道中*/
	}stData;
#define	pszBussiData	stData.pszPkg
#define pszSynData	stData.szData
};
typedef struct tagSendDataMng SENDDATAMNG;
typedef struct tagSendDataMng *LPSENDDATAMNG;


struct tagConnNode
{
	TBUSDBOOLEAN bIsConnected;	/*是否已经建立连接*/
	int idx;			/*此链接节点在链接管理器中的索引*/
	TBUSDBOOLEAN bNeedFree;		/*记录是否需要释放此连接节点，其值为非零值，则需要释放*/
	RELAYFDMODE dwType;      /*描述模式*/
	int iFd;                 /*连接描述符*/
	time_t tCreate;		/*连接建立的时间*/
#if defined(_WIN32) || defined(_WIN64)
	ULONG_PTR ulEpollKey;		/* 在windows下，epoll_ctl delete一个fd时需要此fd的epoll_event的ulKey 值*/
#endif

	LPRELAY pstBindRelay;			/*此连接绑定的relay通道的指针*/

	int iRecvPkg;   /*收到的数据包个数记数器，每收到10个数据包时，会发送一个同步包给对方*/
	int iRecvByte;   /*收到的数据包大小记数器，每收到一定字节数时，会发送一个同步包给对方*/
    TBUSDBOOLEAN bNeedSynSeq;  /*是否需要发送seq同步数据包*/
	time_t tLastSynSeq;			/*最近一次确认序列号的时间*/
	int iLastAckSeq;		/*最近一次确认的序列号*/

	SENDDATAMNG stSendMng;	/*发送数据管理数据结构*/

	int iBuff;			/*接收缓冲区中已接收的字节数*/
	char *szRecvBuff;		 /*接收数据的缓冲区，其空间在初始化此节点数据时分配*/
	int iRecvBuffSize;	 /*此接受缓冲区的大小*/
	struct sockaddr_in stAddr;


};

struct tagRelay
{
	unsigned int dwFlag;                        	/*   relay node enable */
	int iID;                             	/*   relay id */
	TBUSSHMRELAYCNF stRelayInfo;                     	/*   relay information node */
	TBUSCHANNEL stChl;                           	/*   此relay所管理的通道 */
	time_t tLastConnectPeer;                	/*  最近一次连接对方的时间 */

	LPCONNNODE pConnNode;                       	/*   连接管理器的指针 */
	unsigned int dwRelayStatus;                     /*   0 -- not connected, 1 -- connected but hand shake package pending, 2 -- connected and hand shake package sent, but no response, 3 -- auth */
	time_t tBeginDisabled;	/*此中转通道时效的起始时间*/
	TNETADDR	stConnectAddr;	/*对方 tbusd链接地址*/

	time_t tLastHeartbeat;		/*上次收到心跳包的时间，目前简单实现：如果能正常 收发数据就认为有心跳*/
};


struct tagRelayEnv
{
	int iEpoolFd;	/* epool handle*/
	LPTBUSD pstConf ;  /*relay 服务器的主配置信息结构 */
	LPTBUSDSTAT pstStat; /** relay服务器统计信息数据结构指针 */
	TMEMPOOL *ptMemPool;   /*保存链接信息的内存节点*/
	LPTBUSSHMGCIM pstShmGCIM;	/* 全局路由表信息 */
	LPTBUSSHMGRM pstShmGRM;  /* 全局relay节点信息 */
	unsigned int dwRelayCnt;                      	/*   tbus relay information count */
	RELAY *pastTbusRelay[TBUS_MAX_RELAY_NUM_PREHOST];      	/*   tbus relay interface */

	time_t tLastStat;			/*最近一次写统计信息的时间*/
	time_t tLastCheckHandShake;  /*最近一次检查握手连接的时间*/

	time_t tLastRefreshConf;	/*最近一次检查刷新配置的时间*/
	volatile unsigned int dwGCIMVersion; /*记录上次生成*/
	TBUSDBOOLEAN bNeedRefresh;	//是否需要刷洗配置

	TBUSSHMGRM stShmGRMBackup;  /* 对于GRM来说,tbusd以只读的方式访问,因此每次生成配置时拷贝一份出来*/

	int iEpollWaitTime;		/*epoll_wait等待的时间,单位为毫秒*/
	int iMaxSendPkgPerLoop;	/*每个tick计算出来的单个通道最大发送数据包*/

	TBUSHEAD stSendHead;  /*发送数据包头部*/
	TNETADDR	stListenAddr;	/*tbusd 监听地址*/
};


#define TBUSD_CONN_HAVE_REMAIN_DATA(a_pstConn)		((a_pstConn)->stSendMng.iSendPos < (a_pstConn)->stSendMng.iSendBuff)


#define TBUSD_RELAY_IS_ENABLE(pstRelay)     ((pstRelay)->dwFlag & TBUSD_FLAG_ENABLE)
#define TBUSD_RELAY_SET_ENABLE(pstRelay)       ((pstRelay)->dwFlag |= TBUSD_FLAG_ENABLE)
#define TBUSD_RELAY_CLR_ENABLE(pstRelay)		((pstRelay)->dwFlag &= ~TBUSD_FLAG_ENABLE)

#define TBUSD_RELAY_IS_NOT_IN_CONF(pstRelay)     ((pstRelay)->dwFlag & TBUSD_FLAG_NOT_IN_CONF)
#define TBUSD_RELAY_SET_NOT_IN_CONF(pstRelay)       ((pstRelay)->dwFlag |= TBUSD_FLAG_NOT_IN_CONF)
#define TBUSD_RELAY_CLR_NOT_IN_CONF(pstRelay)		((pstRelay)->dwFlag &= ~TBUSD_FLAG_NOT_IN_CONF)


#endif
