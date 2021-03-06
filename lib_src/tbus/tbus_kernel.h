#ifndef TBUS_KERNEL_H
#define TBUS_KERNEL_H

#include "tdr/tdr.h"
#include "tbus/tbus.h"
#include "tbus_config_mng.h"
#include "tbus_head.h"
#include "tbus_macros.h"



#define TBUS_VERSION                             	1 	/* Tbus版本 */


#define HANDLE_COUNT                             	2049 	/* Only support 2048 handle max at one process,the first not use */
#define TUBS_MAX_CHANNEL_COUNT_PREHANDLE         	TBUS_MAX_CHANNEL_NUM_PREHOST 	/*max channel at one handle */
#define TBUS_MAX_BIND_ADDR_NUM_PREHANDLE			32		/*单个句柄上可以绑定的最大地址数*/

enum tagTbusHandleFlag
{
	TBUS_HANDLE_FLAG_NONE	= 0x000000000,
	TBUS_HANDLE_FLAG_ENABLE		=	0x00000001,	/**<表示此通道句柄是否可用*/
	TBUS_HANDLE_FLAG_EXCLUSIVE_CHANNELS = 0x00000002, /**<此通道设置了互斥模式*/
    TBUS_HANDLE_FLAG_CHANNELS_TIMESTAMP = 0x00000004, /**<所有通道发送消息时携带时间戳*/
};

#define TBUS_HANDLE_WITH_TIMESTAMP(pstHandle)   ((pstHandle)->dwFlag & TBUS_HANDLE_FLAG_CHANNELS_TIMESTAMP)
#define TBUS_HANDLE_ENABLE_TIMESTAMP(pstHandle)   ((pstHandle)->dwFlag |= TBUS_HANDLE_FLAG_CHANNELS_TIMESTAMP)
#define TBUS_HANDLE_DISABLE_TIMESTAMP(pstHandle)   ((pstHandle)->dwFlag &= ~TBUS_HANDLE_FLAG_CHANNELS_TIMESTAMP)

#define TBUS_HANDLE_IS_EXCLUSIVE(pstHandle)     ((pstHandle)->dwFlag & TBUS_HANDLE_FLAG_EXCLUSIVE_CHANNELS)
#define TBUS_HANDLE_SET_EXCLUSIVE(pstHandle)       ((pstHandle)->dwFlag |= TBUS_HANDLE_FLAG_EXCLUSIVE_CHANNELS)
#define TBUS_HANDLE_CLR_EXCLUSIVE(pstHandle)		((pstHandle)->dwFlag &= ~TBUS_HANDLE_FLAG_EXCLUSIVE_CHANNELS)



#define	TBUS_CHANNEL_FLAG_ENABLE		0x00000001	/**<设置了此标识位表示此通道可用*/
#define TBUS_CHANNEL_FLAG_NOT_IN_GCIM		0X00000002	/**<设置了此标识位表示此通道已经从全局配置中移除*/
#define TBUS_CHANNEL_FLAG_DISABLE_BY_CTRL	0x00000004	/**<设置了此标识位表示调用tbus_peer_ctrl关闭了此通道*/
#define TBUS_CHANNEL_FLAG_CONNECTED	0x00000008	/**<通过调用tbus_connect绑定了此通道*/
#define TBUS_CHANNEL_FLAG_WITH_TIMESTAMP   0x00000010 /*此通道发送消息时记录时间戳*/

#define TBUS_CHANNEL_WITH_TIMESTAMP(pstChannel)   ((pstChannel)->dwFlag & TBUS_CHANNEL_FLAG_WITH_TIMESTAMP)
#define TBUS_CHANNEL_ENABLE_TIMESTAMP(pstChannel) ((pstChannel)->dwFlag |= TBUS_CHANNEL_FLAG_WITH_TIMESTAMP)
#define TBUS_CHANNEL_DISABLE_TIMESTAMP(pstChannel) ((pstChannel)->dwFlag &=~ TBUS_CHANNEL_FLAG_WITH_TIMESTAMP)

#define TBUS_CHANNEL_IS_ENABLE(pstChannel)     ((pstChannel)->dwFlag & TBUS_CHANNEL_FLAG_ENABLE)
#define TBUS_CHANNEL_SET_ENABLE(pstChannel)       ((pstChannel)->dwFlag |= TBUS_CHANNEL_FLAG_ENABLE)
#define TBUS_CHANNEL_CLR_ENABLE(pstChannel)		((pstChannel)->dwFlag &= ~TBUS_CHANNEL_FLAG_ENABLE)

#define TBUS_CHANNEL_IS_NOT_IN_GCIM(pstChannel)     ((pstChannel)->dwFlag & TBUS_CHANNEL_FLAG_NOT_IN_GCIM)
#define TBUS_CHANNEL_SET_NOT_IN_GCIM(pstChannel)       ((pstChannel)->dwFlag |= TBUS_CHANNEL_FLAG_NOT_IN_GCIM)
#define TBUS_CHANNEL_CLR_NOT_IN_GCIM(pstChannel)		((pstChannel)->dwFlag &= ~TBUS_CHANNEL_FLAG_NOT_IN_GCIM)

#define TBUS_CHANNEL_IS_DISABLE_BY_CTRL(pstChannel)     ((pstChannel)->dwFlag & TBUS_CHANNEL_FLAG_DISABLE_BY_CTRL)
#define TBUS_CHANNEL_SET_DISABLE_BY_CTRL(pstChannel)       ((pstChannel)->dwFlag |= TBUS_CHANNEL_FLAG_DISABLE_BY_CTRL)
#define TBUS_CHANNEL_CLR_DISABLE_BY_CTRL(pstChannel)		((pstChannel)->dwFlag &= ~TBUS_CHANNEL_FLAG_DISABLE_BY_CTRL)

#define TBUS_CHANNEL_IS_CONNECTED(pstChannel)     ((pstChannel)->dwFlag & TBUS_CHANNEL_FLAG_CONNECTED)
#define TBUS_CHANNEL_SET_CONNECTED(pstChannel)       ((pstChannel)->dwFlag |= TBUS_CHANNEL_FLAG_CONNECTED)
#define TBUS_CHANNEL_CLR_CONNECTED(pstChannel)		((pstChannel)->dwFlag &= ~TBUS_CHANNEL_FLAG_CONNECTED)


#ifdef _DEBUG_
/**<当从检测到通道不在配置中到将通道设置成不可用之间的时间间隔，单位为秒，目前为2天*/
#define TBUS_CHANNEL_SET_DISABLE_TIMEOUTGAP		20

/**<当从检测到通道不在配置中到将不可用通道通道清除之间的时间间隔，单位为秒，目前为7天*/
#define TBUS_DISABLE_CHANNEL_CLEAR_DISABLE_TIMEOUTGAP		40
#else
/**<当从检测到通道不在配置中到将通道设置成不可用之间的时间间隔，单位为秒，目前为2天*/
#define TBUS_CHANNEL_SET_DISABLE_TIMEOUTGAP		172800

/**<当从检测到通道不在配置中到将不可用通道通道清除之间的时间间隔，单位为秒，目前为7天*/
#define TBUS_DISABLE_CHANNEL_CLEAR_DISABLE_TIMEOUTGAP		604800
#endif


#define TBUS_INVALID_HANDLE	-1

#define TBUS_RELAY_DISABLE                       	0 	/*  */
#define TBUS_RELAY_ENABLE                        	1 	/*  */


enum enTbusInitStatus
{
	TBUS_MODULE_NOT_INITED  = 0,  /*系统没有初始化*/
	TBUS_MODULE_INITING,	/*系统正在初始化*/
	TBUS_MODULE_INITED,	/*系统已经初始化*/
};


#define TBUS_CHANNEL_SIDE_UP                     	1 	/*  */
#define TBUS_CHANNEL_SIDE_DOWN                   	2 	/*  */

#define TBUS_IDX_INVALID                         	-1 	/*  */
#define TBUS_RECV_POS_INVALID                    	-1 	/*  */


#define TBUS_MAX_NOMSG_CNT                       	3 	/*  */
#define TBUS_DEFAULT_SENT_CNT                    	10 	/*  */
#define TBUS_STEP_SENT_CNT                       	5 	/* how sent count added at one time */
#define TBUS_MAX_SENT_CNT                        	40 	/*  */
#define TBUS_MAX_RECV_CNT                        	10 	/*  */

#ifdef TSF4G_THREAD_SAFE
#define TBUS_MUTEX_LOCK(tMutex)		pthread_mutex_lock(&(tMutex))
#define TBUS_MUTEX_UNLOCK(tMutex)		pthread_mutex_unlock(&(tMutex))
#else
#define TBUS_MUTEX_LOCK(tMutex)
#define TBUS_MUTEX_UNLOCK(tMutex)
#endif /*#ifdef TSF4G_THREAD_SAFE*/


/*快速计算按指定方式对齐的长度值的方法，当iLen大于0x7FFFFF时可以溢出*/
#define	TBUS_CALC_ALIGN(iLen, iAlignLevel)\
		(int)(( ((tdr_longlong)(iLen)>>(iAlignLevel)) + 1) <<(iAlignLevel))


/*计算按指定方式对齐的长度值*/
#define	TBUS_CALC_ALIGN_VALUE(iLen, iAlign)\
	{										\
		int iTmp = (iLen) % (iAlign);		\
		if (0 != iTmp)						\
		{									\
			iLen += (iAlign) - iTmp;		\
		}									\
	}

/*根据对其值计算其对数值*/
#define TBUS_CALC_ALIGN_LEVEL(iAlignLevel, iAlign) \
{												\
	int iTmp = 1;								\
	iAlignLevel = 0;							\
	for(; iTmp < iAlign; iAlignLevel++)			\
		iTmp <<=1;								\
}

/*将对其值设置为2的幂数*/
#define TBUS_NORMALIZE_ALIGN(iAlign)            \
{												\
	int i;										\
	if (0 >= (iAlign))							\
	{											\
		iAlign = TBUS_DEFAULT_CHANNEL_DATA_ALIGN;\
	}else if (TBUS_MAX_CHANNEL_DATA_ALIGN < (iAlign))\
	{											\
		iAlign = TBUS_MAX_CHANNEL_DATA_ALIGN;	\
	}											\
	for(i = 1; i < (iAlign); i <<=1);			\
	iAlign = i;									\
}

#ifdef __cplusplus
extern	"C"
{
#endif


struct tagTbusHandle;
typedef struct tagTbusHandle                        	TBUSHANDLE;
typedef struct tagTbusHandle                        	*LPTBUSHANDLE;

typedef struct tagTbusProcessState                      TBUSPROCESSSTATE;
typedef struct tagTbusProcessState                   *LPTBUSPROCESSSTATE;

typedef struct tagTbusChannel                          	TBUSCHANNEL;


typedef struct tagTbusGlobal                       	TBUSGLOBAL;
typedef struct tagTbusGlobal                       	*LPTBUSGLOBAL;

typedef struct tagChannelVar                       	CHANNELVAR;
typedef struct tagChannelVar                       	*LPCHANNELVAR;

typedef struct tagTbusPkg                       	TBUSPKG;
typedef struct tagTbusPkg                       	*LPTBUSPKG;

struct tagTbusPkg
{
	TBUSHEAD stHead;
	char *szData;
};





#define TBUS_SHM_CHANNEL_VERSION    2 /**共享内存中通道数据结构的版本信息信息*/
#define TBUS_SHM_WITH_CTRL_CHNNL    2 /**引入控制通道时共享内存中通道数据结构的版本*/
#define TBUS_SHM_CTRL_CHNNL_SIZE    1024 /*控制通道的大小*/

/**共享内存中通道消息队列参数数据结构
*/
struct tagChannelVar
{
	unsigned int dwSize;                   	/*数据队列的可容纳的字节数 */
	volatile unsigned int dwHead;			/*消息队列的头指针*/
	volatile unsigned int dwTail;			/*消息队列的尾指针*/
	int iSeq;								/*消息序列号*/
	unsigned int dwGet;						/*消息读取起始位置*/
	int iRecvSeq;                       	/*表示当前收到的数据包的流水号，该数值在握手过程中起到了重要的同步作用 */
	char chAuth;                          	/*值为1时表示，当前的内存channel是由relay模块来处理的,目前已不再使用 */
	char chGStart;                        	/*值为1时表示，当前的channel已经开始向对端发送数据 */
	char chPStart;                        	/*值为1时表示，当前的channel已经开始接收对端发送过来的数据 */
    char chPadding;                       	/*为保证对齐加入的填充字节*/
    unsigned int dwCtrlChnnlSize;           /*控制通道缓冲的大小*/
    volatile unsigned int dwCtrlChnnlHead;  /*控制通道缓冲的头指针*/
    volatile unsigned int dwCtrlChnnlTail;  /*控制通道缓冲的尾指针*/
    int iCtrlMsgSeq;                        /*控制消息的序列号*/
	char sReserve[256 - sizeof(int)*11];	/*保证tagChannelVar的size为256字节*/
};

#define  TBUS_CHANNEL_HEAD_RESERVE_NUM   4  /*通道头部预留数据区*/


/*共享内存中通道头部信息结构*/
struct tagChannelHead
{
	TBUSADDR astAddr[2];          /*保存通道两端的地址*/
	CHANNELVAR astQueueVar[2];    /*通道的读，写两个消息队列的参数信息*/
	unsigned int dwAlignLevel;    /*通道数据对齐方式,这里记录的是对齐值的对数值,即如果对齐方式为8.则dwAlignLevel为3*/
	HANDLE iShmID;	              /*id of share memory for channel*/
	unsigned int dwMagic;		  /*tbus 数据通道的magic*/
    unsigned int dwVersion;       /**/
	char sReserve[TBUS_CHANNEL_HEAD_RESERVE_NUM]; /*预留数据区*/
};

struct tagTbusProcessState
{
    long long llLastBeatTime;
    int iIsAlive;
    int iIsAvailable;
    int iTimeOutGap;
    TBUSADDR iPeerAddr;
    int iBindedChnnlNum;
};

/*bus通道*/
struct tagTbusChannel
{
	unsigned int dwFlag;		/**<标志信息位*/
	int iRecvSide;              /*通道数据接收端，其对应地址为对端的地址其值为0或1*/
	int iSendSide;               /*通道数据发送端，其对应地址为对端的地址，其值为(iRecvSide+1)%2*/
	HANDLE iShmID;	/*id of share memory for channel*/

	/*通道信息头部，本成员指向共享内存中通道头部，pstHead->astAddr[iRecvSide]为本地地址*/
	CHANNELHEAD *pstHead;

	/*读写消息队列, 本成员指向共享内存中通道的两个队列*/
    /*pszQueues[iRecvSide]为读队列，pszQueues[iSendSide]为写队列*/
	char *pszQueues[2];

    /*控制消息读写队列，本成员指向共享内存中通道的两个控制消息队列*/
    /*pszCtrlQueues[iRecvSide]为读队列，pszCtrlQueues[iSendSide]为写队列*/
    char *pszCtrlQueues[2];

    LPTBUSPROCESSSTATE pstPeerState;

	time_t tBeginDisable;		/*开始失效的起始时间*/
    int iHandle;                /*保存所属的句柄索引*/

    TBUSPROCESSSTATE stPeerState; /*用于Channel模式，防止访问pstPeerState时程序core*/
	TBUSHEAD stHead;            /*用于Channel模式 支持forward和backward操作的数据结构*/
};




struct tagTbusHandle
{
	unsigned int dwFlag;                        	/* 标志位 */
	unsigned int dwChannelCnt;                       	/* 此句柄管理的通道数据 */
	TBUSCHANNEL *pastChannelSet[TUBS_MAX_CHANNEL_COUNT_PREHANDLE]; 	/*通道信息数据 */

    unsigned int dwPeerCnt;
    TBUSPROCESSSTATE astPeerState[TUBS_MAX_CHANNEL_COUNT_PREHANDLE];

	int iRecvPos;                        	/*   new field, current received address position */
	unsigned int dwRecvPkgCnt;                    	/*   received packages count in one channel */
	TBUSHEAD stRecvHead;                          	/*   head of recv Pkg  package */
	TBUSHEAD stHeadBak;                          	/*  backup of the head of send Pkg  package */


	volatile unsigned int dwGCIMVersion;						/**< version of GCIM in share memory*/
	int iBindAddrNum;		/*此句柄上已经绑定的地址数*/
	TBUSADDR aiBindAddr[TBUS_MAX_BIND_ADDR_NUM_PREHANDLE];  /*此句柄上已经bind的地址数*/
};

struct tagTbusGlobal
{
	unsigned int dwInitStatus;                        	/*   全局信息结构是否 */
	pthread_mutex_t tMutex;						/**<互斥读写锁进行互斥访问本全局数据去和实现一些线程安全操作*/
	int	iBussId;									/**<业务ID*/
	int iFlag;
	unsigned int dwDyedMsgID;                     	/*   染色消息的ID */
	unsigned int dwHandleCnt;                     	/*   new field, tbus handle count */

	TBUSHANDLE *pastTbusHandle[HANDLE_COUNT];    	/*   new field, tbus interface */
	volatile unsigned int dwGCIMVersion;						/**< version of GCIM in share memory*/
	TBUSSHMGCIM *pstGCIM;                     	/*   global channel info map */
	LPTDRMETA	pstHeadMeta;					/*meta of tbus head*/

	TBUSHEAD stCounterfeitPkg;		/*为方便tbus处理，而在数据通道尾部添加的伪造数据包*/
	int iCounterfeitPkgLen;		/*伪造数据包的的长度*/
    void *pvAgentMng;           /* save agent mng data ptr<malloced> */

    struct timeval stCurTime;   /* 记录当前时间，用于发送消息时记录时间戳 */
};

#define TBUS_SET_LAST_ROUTE(stHead, a_iSrc) \
{												\
	if (0 == (stHead).stExtHead.stDataHead.bRoute) \
	{													\
		(stHead).stExtHead.stDataHead.bRoute = 1;	\
		(stHead).bFlag |= TBUS_HEAD_FLAG_WITH_ROUTE;	\
	}												\
    (stHead).stExtHead.stDataHead.routeAddr[(stHead).stExtHead.stDataHead.bRoute -1] = a_iSrc;\
}

#define TBUS_IS_SPECIFICAL_ADDRESS(addr)  ((TBUS_ADDR_DEFAULT == (addr)) || (TBUS_ADDR_ALL == (addr)))

#define TBUS_COMPRESS_HEAD_LEN_VERSION	0	/*In this tbus head version, the net length of tbus head is divide 4 */


#define TBUS_HEAD_CODE_BUFFER_SIZE		1024	/*size of buffer to coding tbus head */

/*offset of head.headlen in network, pack by one byte */
#define TBUS_HEAD_LEN_NET_OFFSET	offsetof(TBUSHEAD, bHeadLen)

#define TBUS_MIN_NET_LEN_TO_GET_HEADLEN	 (int)(TBUS_HEAD_LEN_NET_OFFSET + TDR_SIZEOF_BYTE)

/*offset of head.seq in network msg, pack by one byte*/
#define TBUS_HEAD_SEQ_NET_OFFSET	offsetof(TBUSHEAD, iSeq)
#define TBUS_MIN_NET_LEN_TO_GET_HEADSEQ	 (int)(TBUS_HEAD_SEQ_NET_OFFSET + TDR_SIZEOF_INT)

#define TBUS_MIN_HEAD_VERSION	1

#define TBUS_HEAD_EXTHEAD_DATA_OFFSET	offsetof(TBUSHEAD, stExtHead)

#define TBUS_HEAD_MIN_SIZE TBUS_HEAD_EXTHEAD_DATA_OFFSET

#define TBUS_CHANNEL_VAR_PUSH(a_ptChannel)  (CHANNELVAR *)&((a_ptChannel)->pstHead->astQueueVar[(a_ptChannel)->iSendSide])

#define TBUS_CHANNEL_VAR_GET(a_ptChannel)  (CHANNELVAR *)&((a_ptChannel)->pstHead->astQueueVar[(a_ptChannel)->iRecvSide])

#define TBUS_CHANNEL_QUEUE_GET(a_ptChannel)	(a_ptChannel)->pszQueues[(a_ptChannel)->iRecvSide]

#define TBUS_CHANNEL_QUEUE_PUSH(a_ptChannel)	(a_ptChannel)->pszQueues[(a_ptChannel)->iSendSide]

#define TBUS_CHANNEL_LOCAL_ADDR(a_ptChannel)  ((a_ptChannel)->pstHead->astAddr[(a_ptChannel)->iRecvSide])

#define TBUS_CHANNEL_PEER_ADDR(a_ptChannel)  ((a_ptChannel)->pstHead->astAddr[(a_ptChannel)->iSendSide])




/*检查数据队列相关参数是否需要调整*/
extern TBUSGLOBAL g_stBusGlobal;
#define TBUS_CHECK_QUEUE_HEAD_VAR(a_pszQueue, a_iSize, a_iHead) \
{															 \
	int iTailLen = (a_iSize) - (a_iHead);					 \
	if (iTailLen <= g_stBusGlobal.iCounterfeitPkgLen)		 \
	{														 \
		tbus_log(TLOG_PRIORITY_DEBUG, "the data taillen(%d) is not bigger than the counterfeit pkglen(%d)," \
			 "so change head pointer to zero", iTailLen, g_stBusGlobal.iCounterfeitPkgLen);						\
		 a_iHead = 0;												\
	}else															\
	{																\
		LPTBUSHEAD pstTmpHead = (LPTBUSHEAD)((a_pszQueue) + (a_iHead));\
		if (pstTmpHead->bFlag == TBUS_HEAD_FLAG_COUNTERFEIT_DATA)		\
		{																\
			tbus_log(TLOG_PRIORITY_DEBUG, "the pkg in the data tail(len:%d) is a counterfeit pkg,"\
			"so change head pointer to zero",iTailLen);							\
			a_iHead = 0;													\
		}/*if (a_pstHead->bFlag == TBUS_HEAD_FLAG_COUNTERFEIT_DATA)*/	\
	}/*if (iTailLen <= g_stBusGlobal.iCounterfeitPkgLen)*/				\
}


#define  TBUS_GET_HANDLE(a_iHandle)  	((a_iHandle) < 1 || ((a_iHandle) >= HANDLE_COUNT) ?\
                                        NULL : g_stBusGlobal.pastTbusHandle[(a_iHandle)])

#define TBUS_GET_INDEX(a_piHandle, pstHandle)\
{\
    if (NULL == pstHandle)\
    {\
        *a_piHandle = -1;\
    } else\
    {\
        int iCaledIndex = pstHandle - &g_stBusGlobal.pastTbusHandle[0];\
        if (1 > iCaledIndex || g_stBusGlobal.dwHandleCnt < iCaledIndex)\
        {\
            *a_piHandle = -1;\
        } else\
        {\
            *a_piHandle = iCaledIndex;\
        }\
    }\
}


#define TBUS_CALC_CHECKSUM_SIZE offsetof(TBUSHEAD,bFlag)

#define TBUS_CALC_HEAD_CHECKSUM(shSum, a_pstHead)  {	\
	int i = 0 ;											\
	short *pTemp = (short *)a_pstHead;					\
	shSum = 0 ;											\
	for (i = 0; i < (int)(TBUS_CALC_CHECKSUM_SIZE/sizeof(short)); i++)	\
	{														\
		shSum ^= *pTemp++;									\
	}														\
}



//判断指定地址是否匹配指定通道
#define TBUS_SELECT_CHANNEL(a_bSelected, a_pstChannel, a_iSrc, a_iDst) \
do {																	\
	int _iSrcAddr ;														\
	int _iDstAddr;														\
	CHANNELHEAD *pstHead = a_pstChannel->pstHead;						\
	a_bSelected = 0;													\
	if ( !TBUS_CHANNEL_IS_ENABLE(a_pstChannel))							\
	{																	\
		break;															\
	}																	\
	_iSrcAddr = pstHead->astAddr[a_pstChannel->iRecvSide];				\
	_iDstAddr = pstHead->astAddr[a_pstChannel->iSendSide];				\
	if (TBUS_IS_SPECIFICAL_ADDRESS(a_iSrc))								\
	{																	\
		if (TBUS_IS_SPECIFICAL_ADDRESS(a_iDst))							\
		{																\
			a_iSrc = _iSrcAddr;											\
			a_iDst = _iDstAddr;											\
			a_bSelected = 1;											\
		}else															\
		{																\
			if (_iDstAddr == a_iDst)										\
			{															\
				a_iSrc = _iSrcAddr;										\
				a_bSelected = 1;										\
			}															\
		}/*if (TBUS_IS_SPECIFICAL_ADDRESS(a_iDst))*/					\
	}else																\
	{																	\
		if (TBUS_IS_SPECIFICAL_ADDRESS(a_iDst))							\
		{																\
			if (a_iSrc == _iSrcAddr)										\
			{															\
				a_iDst = _iDstAddr;										\
				a_bSelected = 1;										\
			}															\
		}else															\
		{																\
			if ((a_iSrc == _iSrcAddr) && (a_iDst == _iDstAddr))		\
			{															\
				a_bSelected = 1;										\
			}															\
		}/*if (TBUS_IS_SPECIFICAL_ADDRESS(a_iDst))*/					\
	}/*if (TBUS_IS_SPECIFICAL_ADDRESS(a_iSrc))*/						\
}while (0)



int tbus_encode_head(IN LPTBUSHEAD a_pstHead, IN char *a_pszNet, INOUT int *a_piLen, IN int a_iVersion);

int tbus_decode_head(OUT LPTBUSHEAD a_pstHead, IN char *a_pszNet, IN int a_iLen, IN int a_iVersion);

int tbus_get_pkghead(OUT LPTBUSHEAD a_pstHead, const char* a_pszQueue, unsigned int dwSize,
					 INOUT unsigned int *a_pdwHead, unsigned int dwTail);


#define TBUS_CALC_TIME_DELTA(a_dwDelta, stBegin, stEnd) \
{\
    if ((stEnd).llSec > (stBegin).llSec)\
    {\
        a_dwDelta = (((stEnd).llSec - (stBegin).llSec) * 1000000);\
        if ((stEnd).dwUSec > (stBegin).dwUSec)\
        {\
            a_dwDelta += ((stEnd).dwUSec - (stBegin).dwUSec);\
        } else if ((stEnd).dwUSec < (stBegin).dwUSec)\
        {\
            a_dwDelta -= ((stBegin).dwUSec - (stEnd).dwUSec);\
        }\
    } else if (((stEnd).llSec == (stBegin).llSec)\
               && ((stEnd).dwUSec > (stBegin).dwUSec))\
    {\
        a_dwDelta = ((stEnd).dwUSec - (stBegin).dwUSec);\
    } else\
    {\
        (a_dwDelta) = 0;\
    }\
}


#ifdef __cplusplus
}
#endif
#endif /*TBUS_KERNEL_H*/
