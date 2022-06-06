/**
*
* @file     tbusd.h
* @brief    tbusd������ģ��
*
* @author jackyai
* @version 1.0
* @date 2008-05-16
*
*
* Copyright (c)  2008, ��Ѷ�Ƽ����޹�˾���������з����ܹ���
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

#define  RELAY_CONFIG_DATA_META_NAME  "Tbusd" /** ������������Ϣ�洢���ݽṹԪ���������� */
#define RELAY_RUN_DATA_META_NAME	"TbusdStat" /** ������ͳ����Ϣ�洢���ݽṹԪ���������� */

#define RELAY_CONN_RECV_BUFF_SIZE	sizeof(TBUSPKG)		/*relayͨ�����ջ���������*/

#define TBUSD_DEFAULT_LOGCONF_FILE	"./tlog.xml"		/*tbusd loge�����ļ�Ŀ¼*/

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
	RELAY_FD_LISTEN_MODE = 1,	/**< ��������*/
	RELAY_FD_SERVER_MODE = 2, /**< ������ģʽ���������򿪵�����*/
	RELAY_FD_CLIENT_MODE = 3, /**< �ͻ���ģʽ�����ӣ��������򿪵�����*/
};

enum tagTbusdDataType
{
	TBUSD_DATA_TYPE_UNKNOWN = 0,	/**< ���Ͳ���*/
	TBUSD_DATA_TYPE_SYN = 1, /**< ͬ�����кŵȿ�������*/
	TBUSD_DATA_TYPE_BUSINESS = 2, /**< ҵ������*/
};

struct tagSendDataMng
{
	int iSendPos;  /*��stSendPkg���Ѿ��Ѿ����͵�����ƫ�ƣ��ٴη���ʣ������ʱ�������λ�ÿ�ʼ����*/
	int iSendBuff;	/*��Ҫ�������ݵĳ���*/
	LPTBUSHEAD pstSendHead;  /*�������ݰ�ͷ��*/
	TBUSDDATATYPE enRemainDataType;  /*stSendPkg�а�����ʣ��δ�������ݵ�����*/
	union tagData
	{
		char szData[TBUS_HEAD_MAX_LEN];	/*syn���ݰ�������*/
		char *pszPkg;	/*ҵ�����ݻ�����ָ�룬ҵ������ֱ�ӱ���������ͨ����*/
	}stData;
#define	pszBussiData	stData.pszPkg
#define pszSynData	stData.szData
};
typedef struct tagSendDataMng SENDDATAMNG;
typedef struct tagSendDataMng *LPSENDDATAMNG;


struct tagConnNode
{
	TBUSDBOOLEAN bIsConnected;	/*�Ƿ��Ѿ���������*/
	int idx;			/*�����ӽڵ������ӹ������е�����*/
	TBUSDBOOLEAN bNeedFree;		/*��¼�Ƿ���Ҫ�ͷŴ����ӽڵ㣬��ֵΪ����ֵ������Ҫ�ͷ�*/
	RELAYFDMODE dwType;      /*����ģʽ*/
	int iFd;                 /*����������*/
	time_t tCreate;		/*���ӽ�����ʱ��*/
#if defined(_WIN32) || defined(_WIN64)
	ULONG_PTR ulEpollKey;		/* ��windows�£�epoll_ctl deleteһ��fdʱ��Ҫ��fd��epoll_event��ulKey ֵ*/
#endif

	LPRELAY pstBindRelay;			/*�����Ӱ󶨵�relayͨ����ָ��*/

	int iRecvPkg;   /*�յ������ݰ�������������ÿ�յ�10�����ݰ�ʱ���ᷢ��һ��ͬ�������Է�*/
	int iRecvByte;   /*�յ������ݰ���С��������ÿ�յ�һ���ֽ���ʱ���ᷢ��һ��ͬ�������Է�*/
    TBUSDBOOLEAN bNeedSynSeq;  /*�Ƿ���Ҫ����seqͬ�����ݰ�*/
	time_t tLastSynSeq;			/*���һ��ȷ�����кŵ�ʱ��*/
	int iLastAckSeq;		/*���һ��ȷ�ϵ����к�*/

	SENDDATAMNG stSendMng;	/*�������ݹ������ݽṹ*/

	int iBuff;			/*���ջ��������ѽ��յ��ֽ���*/
	char *szRecvBuff;		 /*�������ݵĻ���������ռ��ڳ�ʼ���˽ڵ�����ʱ����*/
	int iRecvBuffSize;	 /*�˽��ܻ������Ĵ�С*/
	struct sockaddr_in stAddr;


};

struct tagRelay
{
	unsigned int dwFlag;                        	/*   relay node enable */
	int iID;                             	/*   relay id */
	TBUSSHMRELAYCNF stRelayInfo;                     	/*   relay information node */
	TBUSCHANNEL stChl;                           	/*   ��relay�������ͨ�� */
	time_t tLastConnectPeer;                	/*  ���һ�����ӶԷ���ʱ�� */

	LPCONNNODE pConnNode;                       	/*   ���ӹ�������ָ�� */
	unsigned int dwRelayStatus;                     /*   0 -- not connected, 1 -- connected but hand shake package pending, 2 -- connected and hand shake package sent, but no response, 3 -- auth */
	time_t tBeginDisabled;	/*����תͨ��ʱЧ����ʼʱ��*/
	TNETADDR	stConnectAddr;	/*�Է� tbusd���ӵ�ַ*/

	time_t tLastHeartbeat;		/*�ϴ��յ���������ʱ�䣬Ŀǰ��ʵ�֣���������� �շ����ݾ���Ϊ������*/
};


struct tagRelayEnv
{
	int iEpoolFd;	/* epool handle*/
	LPTBUSD pstConf ;  /*relay ����������������Ϣ�ṹ */
	LPTBUSDSTAT pstStat; /** relay������ͳ����Ϣ���ݽṹָ�� */
	TMEMPOOL *ptMemPool;   /*����������Ϣ���ڴ�ڵ�*/
	LPTBUSSHMGCIM pstShmGCIM;	/* ȫ��·�ɱ���Ϣ */
	LPTBUSSHMGRM pstShmGRM;  /* ȫ��relay�ڵ���Ϣ */
	unsigned int dwRelayCnt;                      	/*   tbus relay information count */
	RELAY *pastTbusRelay[TBUS_MAX_RELAY_NUM_PREHOST];      	/*   tbus relay interface */

	time_t tLastStat;			/*���һ��дͳ����Ϣ��ʱ��*/
	time_t tLastCheckHandShake;  /*���һ�μ���������ӵ�ʱ��*/

	time_t tLastRefreshConf;	/*���һ�μ��ˢ�����õ�ʱ��*/
	volatile unsigned int dwGCIMVersion; /*��¼�ϴ�����*/
	TBUSDBOOLEAN bNeedRefresh;	//�Ƿ���Ҫˢϴ����

	TBUSSHMGRM stShmGRMBackup;  /* ����GRM��˵,tbusd��ֻ���ķ�ʽ����,���ÿ����������ʱ����һ�ݳ���*/

	int iEpollWaitTime;		/*epoll_wait�ȴ���ʱ��,��λΪ����*/
	int iMaxSendPkgPerLoop;	/*ÿ��tick��������ĵ���ͨ����������ݰ�*/

	TBUSHEAD stSendHead;  /*�������ݰ�ͷ��*/
	TNETADDR	stListenAddr;	/*tbusd ������ַ*/
};


#define TBUSD_CONN_HAVE_REMAIN_DATA(a_pstConn)		((a_pstConn)->stSendMng.iSendPos < (a_pstConn)->stSendMng.iSendBuff)


#define TBUSD_RELAY_IS_ENABLE(pstRelay)     ((pstRelay)->dwFlag & TBUSD_FLAG_ENABLE)
#define TBUSD_RELAY_SET_ENABLE(pstRelay)       ((pstRelay)->dwFlag |= TBUSD_FLAG_ENABLE)
#define TBUSD_RELAY_CLR_ENABLE(pstRelay)		((pstRelay)->dwFlag &= ~TBUSD_FLAG_ENABLE)

#define TBUSD_RELAY_IS_NOT_IN_CONF(pstRelay)     ((pstRelay)->dwFlag & TBUSD_FLAG_NOT_IN_CONF)
#define TBUSD_RELAY_SET_NOT_IN_CONF(pstRelay)       ((pstRelay)->dwFlag |= TBUSD_FLAG_NOT_IN_CONF)
#define TBUSD_RELAY_CLR_NOT_IN_CONF(pstRelay)		((pstRelay)->dwFlag &= ~TBUSD_FLAG_NOT_IN_CONF)


#endif
