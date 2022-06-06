#ifndef TBUS_KERNEL_H
#define TBUS_KERNEL_H

#include "tdr/tdr.h"
#include "tbus/tbus.h"
#include "tbus_config_mng.h"
#include "tbus_head.h"
#include "tbus_macros.h"



#define TBUS_VERSION                             	1 	/* Tbus�汾 */


#define HANDLE_COUNT                             	2049 	/* Only support 2048 handle max at one process,the first not use */
#define TUBS_MAX_CHANNEL_COUNT_PREHANDLE         	TBUS_MAX_CHANNEL_NUM_PREHOST 	/*max channel at one handle */
#define TBUS_MAX_BIND_ADDR_NUM_PREHANDLE			32		/*��������Ͽ��԰󶨵�����ַ��*/

enum tagTbusHandleFlag
{
	TBUS_HANDLE_FLAG_NONE	= 0x000000000,
	TBUS_HANDLE_FLAG_ENABLE		=	0x00000001,	/**<��ʾ��ͨ������Ƿ����*/
	TBUS_HANDLE_FLAG_EXCLUSIVE_CHANNELS = 0x00000002, /**<��ͨ�������˻���ģʽ*/
    TBUS_HANDLE_FLAG_CHANNELS_TIMESTAMP = 0x00000004, /**<����ͨ��������ϢʱЯ��ʱ���*/
};

#define TBUS_HANDLE_WITH_TIMESTAMP(pstHandle)   ((pstHandle)->dwFlag & TBUS_HANDLE_FLAG_CHANNELS_TIMESTAMP)
#define TBUS_HANDLE_ENABLE_TIMESTAMP(pstHandle)   ((pstHandle)->dwFlag |= TBUS_HANDLE_FLAG_CHANNELS_TIMESTAMP)
#define TBUS_HANDLE_DISABLE_TIMESTAMP(pstHandle)   ((pstHandle)->dwFlag &= ~TBUS_HANDLE_FLAG_CHANNELS_TIMESTAMP)

#define TBUS_HANDLE_IS_EXCLUSIVE(pstHandle)     ((pstHandle)->dwFlag & TBUS_HANDLE_FLAG_EXCLUSIVE_CHANNELS)
#define TBUS_HANDLE_SET_EXCLUSIVE(pstHandle)       ((pstHandle)->dwFlag |= TBUS_HANDLE_FLAG_EXCLUSIVE_CHANNELS)
#define TBUS_HANDLE_CLR_EXCLUSIVE(pstHandle)		((pstHandle)->dwFlag &= ~TBUS_HANDLE_FLAG_EXCLUSIVE_CHANNELS)



#define	TBUS_CHANNEL_FLAG_ENABLE		0x00000001	/**<�����˴˱�ʶλ��ʾ��ͨ������*/
#define TBUS_CHANNEL_FLAG_NOT_IN_GCIM		0X00000002	/**<�����˴˱�ʶλ��ʾ��ͨ���Ѿ���ȫ���������Ƴ�*/
#define TBUS_CHANNEL_FLAG_DISABLE_BY_CTRL	0x00000004	/**<�����˴˱�ʶλ��ʾ����tbus_peer_ctrl�ر��˴�ͨ��*/
#define TBUS_CHANNEL_FLAG_CONNECTED	0x00000008	/**<ͨ������tbus_connect���˴�ͨ��*/
#define TBUS_CHANNEL_FLAG_WITH_TIMESTAMP   0x00000010 /*��ͨ��������Ϣʱ��¼ʱ���*/

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
/**<���Ӽ�⵽ͨ�����������е���ͨ�����óɲ�����֮���ʱ��������λΪ�룬ĿǰΪ2��*/
#define TBUS_CHANNEL_SET_DISABLE_TIMEOUTGAP		20

/**<���Ӽ�⵽ͨ�����������е���������ͨ��ͨ�����֮���ʱ��������λΪ�룬ĿǰΪ7��*/
#define TBUS_DISABLE_CHANNEL_CLEAR_DISABLE_TIMEOUTGAP		40
#else
/**<���Ӽ�⵽ͨ�����������е���ͨ�����óɲ�����֮���ʱ��������λΪ�룬ĿǰΪ2��*/
#define TBUS_CHANNEL_SET_DISABLE_TIMEOUTGAP		172800

/**<���Ӽ�⵽ͨ�����������е���������ͨ��ͨ�����֮���ʱ��������λΪ�룬ĿǰΪ7��*/
#define TBUS_DISABLE_CHANNEL_CLEAR_DISABLE_TIMEOUTGAP		604800
#endif


#define TBUS_INVALID_HANDLE	-1

#define TBUS_RELAY_DISABLE                       	0 	/*  */
#define TBUS_RELAY_ENABLE                        	1 	/*  */


enum enTbusInitStatus
{
	TBUS_MODULE_NOT_INITED  = 0,  /*ϵͳû�г�ʼ��*/
	TBUS_MODULE_INITING,	/*ϵͳ���ڳ�ʼ��*/
	TBUS_MODULE_INITED,	/*ϵͳ�Ѿ���ʼ��*/
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


/*���ټ��㰴ָ����ʽ����ĳ���ֵ�ķ�������iLen����0x7FFFFFʱ�������*/
#define	TBUS_CALC_ALIGN(iLen, iAlignLevel)\
		(int)(( ((tdr_longlong)(iLen)>>(iAlignLevel)) + 1) <<(iAlignLevel))


/*���㰴ָ����ʽ����ĳ���ֵ*/
#define	TBUS_CALC_ALIGN_VALUE(iLen, iAlign)\
	{										\
		int iTmp = (iLen) % (iAlign);		\
		if (0 != iTmp)						\
		{									\
			iLen += (iAlign) - iTmp;		\
		}									\
	}

/*���ݶ���ֵ���������ֵ*/
#define TBUS_CALC_ALIGN_LEVEL(iAlignLevel, iAlign) \
{												\
	int iTmp = 1;								\
	iAlignLevel = 0;							\
	for(; iTmp < iAlign; iAlignLevel++)			\
		iTmp <<=1;								\
}

/*������ֵ����Ϊ2������*/
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





#define TBUS_SHM_CHANNEL_VERSION    2 /**�����ڴ���ͨ�����ݽṹ�İ汾��Ϣ��Ϣ*/
#define TBUS_SHM_WITH_CTRL_CHNNL    2 /**�������ͨ��ʱ�����ڴ���ͨ�����ݽṹ�İ汾*/
#define TBUS_SHM_CTRL_CHNNL_SIZE    1024 /*����ͨ���Ĵ�С*/

/**�����ڴ���ͨ����Ϣ���в������ݽṹ
*/
struct tagChannelVar
{
	unsigned int dwSize;                   	/*���ݶ��еĿ����ɵ��ֽ��� */
	volatile unsigned int dwHead;			/*��Ϣ���е�ͷָ��*/
	volatile unsigned int dwTail;			/*��Ϣ���е�βָ��*/
	int iSeq;								/*��Ϣ���к�*/
	unsigned int dwGet;						/*��Ϣ��ȡ��ʼλ��*/
	int iRecvSeq;                       	/*��ʾ��ǰ�յ������ݰ�����ˮ�ţ�����ֵ�����ֹ�����������Ҫ��ͬ������ */
	char chAuth;                          	/*ֵΪ1ʱ��ʾ����ǰ���ڴ�channel����relayģ���������,Ŀǰ�Ѳ���ʹ�� */
	char chGStart;                        	/*ֵΪ1ʱ��ʾ����ǰ��channel�Ѿ���ʼ��Զ˷������� */
	char chPStart;                        	/*ֵΪ1ʱ��ʾ����ǰ��channel�Ѿ���ʼ���նԶ˷��͹��������� */
    char chPadding;                       	/*Ϊ��֤������������ֽ�*/
    unsigned int dwCtrlChnnlSize;           /*����ͨ������Ĵ�С*/
    volatile unsigned int dwCtrlChnnlHead;  /*����ͨ�������ͷָ��*/
    volatile unsigned int dwCtrlChnnlTail;  /*����ͨ�������βָ��*/
    int iCtrlMsgSeq;                        /*������Ϣ�����к�*/
	char sReserve[256 - sizeof(int)*11];	/*��֤tagChannelVar��sizeΪ256�ֽ�*/
};

#define  TBUS_CHANNEL_HEAD_RESERVE_NUM   4  /*ͨ��ͷ��Ԥ��������*/


/*�����ڴ���ͨ��ͷ����Ϣ�ṹ*/
struct tagChannelHead
{
	TBUSADDR astAddr[2];          /*����ͨ�����˵ĵ�ַ*/
	CHANNELVAR astQueueVar[2];    /*ͨ���Ķ���д������Ϣ���еĲ�����Ϣ*/
	unsigned int dwAlignLevel;    /*ͨ�����ݶ��뷽ʽ,�����¼���Ƕ���ֵ�Ķ���ֵ,��������뷽ʽΪ8.��dwAlignLevelΪ3*/
	HANDLE iShmID;	              /*id of share memory for channel*/
	unsigned int dwMagic;		  /*tbus ����ͨ����magic*/
    unsigned int dwVersion;       /**/
	char sReserve[TBUS_CHANNEL_HEAD_RESERVE_NUM]; /*Ԥ��������*/
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

/*busͨ��*/
struct tagTbusChannel
{
	unsigned int dwFlag;		/**<��־��Ϣλ*/
	int iRecvSide;              /*ͨ�����ݽ��նˣ����Ӧ��ַΪ�Զ˵ĵ�ַ��ֵΪ0��1*/
	int iSendSide;               /*ͨ�����ݷ��Ͷˣ����Ӧ��ַΪ�Զ˵ĵ�ַ����ֵΪ(iRecvSide+1)%2*/
	HANDLE iShmID;	/*id of share memory for channel*/

	/*ͨ����Ϣͷ��������Աָ�����ڴ���ͨ��ͷ����pstHead->astAddr[iRecvSide]Ϊ���ص�ַ*/
	CHANNELHEAD *pstHead;

	/*��д��Ϣ����, ����Աָ�����ڴ���ͨ������������*/
    /*pszQueues[iRecvSide]Ϊ�����У�pszQueues[iSendSide]Ϊд����*/
	char *pszQueues[2];

    /*������Ϣ��д���У�����Աָ�����ڴ���ͨ��������������Ϣ����*/
    /*pszCtrlQueues[iRecvSide]Ϊ�����У�pszCtrlQueues[iSendSide]Ϊд����*/
    char *pszCtrlQueues[2];

    LPTBUSPROCESSSTATE pstPeerState;

	time_t tBeginDisable;		/*��ʼʧЧ����ʼʱ��*/
    int iHandle;                /*���������ľ������*/

    TBUSPROCESSSTATE stPeerState; /*����Channelģʽ����ֹ����pstPeerStateʱ����core*/
	TBUSHEAD stHead;            /*����Channelģʽ ֧��forward��backward���������ݽṹ*/
};




struct tagTbusHandle
{
	unsigned int dwFlag;                        	/* ��־λ */
	unsigned int dwChannelCnt;                       	/* �˾�������ͨ������ */
	TBUSCHANNEL *pastChannelSet[TUBS_MAX_CHANNEL_COUNT_PREHANDLE]; 	/*ͨ����Ϣ���� */

    unsigned int dwPeerCnt;
    TBUSPROCESSSTATE astPeerState[TUBS_MAX_CHANNEL_COUNT_PREHANDLE];

	int iRecvPos;                        	/*   new field, current received address position */
	unsigned int dwRecvPkgCnt;                    	/*   received packages count in one channel */
	TBUSHEAD stRecvHead;                          	/*   head of recv Pkg  package */
	TBUSHEAD stHeadBak;                          	/*  backup of the head of send Pkg  package */


	volatile unsigned int dwGCIMVersion;						/**< version of GCIM in share memory*/
	int iBindAddrNum;		/*�˾�����Ѿ��󶨵ĵ�ַ��*/
	TBUSADDR aiBindAddr[TBUS_MAX_BIND_ADDR_NUM_PREHANDLE];  /*�˾�����Ѿ�bind�ĵ�ַ��*/
};

struct tagTbusGlobal
{
	unsigned int dwInitStatus;                        	/*   ȫ����Ϣ�ṹ�Ƿ� */
	pthread_mutex_t tMutex;						/**<�����д�����л�����ʱ�ȫ������ȥ��ʵ��һЩ�̰߳�ȫ����*/
	int	iBussId;									/**<ҵ��ID*/
	int iFlag;
	unsigned int dwDyedMsgID;                     	/*   Ⱦɫ��Ϣ��ID */
	unsigned int dwHandleCnt;                     	/*   new field, tbus handle count */

	TBUSHANDLE *pastTbusHandle[HANDLE_COUNT];    	/*   new field, tbus interface */
	volatile unsigned int dwGCIMVersion;						/**< version of GCIM in share memory*/
	TBUSSHMGCIM *pstGCIM;                     	/*   global channel info map */
	LPTDRMETA	pstHeadMeta;					/*meta of tbus head*/

	TBUSHEAD stCounterfeitPkg;		/*Ϊ����tbus������������ͨ��β����ӵ�α�����ݰ�*/
	int iCounterfeitPkgLen;		/*α�����ݰ��ĵĳ���*/
    void *pvAgentMng;           /* save agent mng data ptr<malloced> */

    struct timeval stCurTime;   /* ��¼��ǰʱ�䣬���ڷ�����Ϣʱ��¼ʱ��� */
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




/*������ݶ�����ز����Ƿ���Ҫ����*/
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



//�ж�ָ����ַ�Ƿ�ƥ��ָ��ͨ��
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
