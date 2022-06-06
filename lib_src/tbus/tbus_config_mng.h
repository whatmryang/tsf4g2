#ifndef TBUS_CONFIG_MNG_H
#define TBUS_CONFIG_MNG_H

#include "pal/pal.h"
#include "tbus_addr_templet.h"
#include "tbus/tbus_macros.h"
#include "tbus_comm.h"
#include "tbus_desc.h"

#define TBUS_SHM_GCIM_HEAD_RESERV_NUM                 	7 	/* reserve*/
#define TBUS_SHM_GRM_HEAD_RESERV_NUM                 	8 	/* reserve*/

#define TBUS_CHANNEL_RESERVER_NUM						4

#define TBUS_DEFAULT_ADDRESS_TEMPLET	"8.8.8.8"  /*tbusϵͳȱʡ��ַģ��*/

/**<timeout for attaching shm by milli seconds*/
#define TBUS_ATTACH_SHM_TIMEOUT						30000

/*timeout for lock by milli seconds*/
#define TBUS_RWLOCK_TIMEOUT			30000

/**<sleep time by micro seconds per loop*/
#define TBUS_SLEEP_PRELOOP						100


#define TBUS_CHECK_CHNNL_FLAG_KEEPOPEN      1  /* keep channel shm open after check */

#define TBUS_CHANNEL_SIDE_INDEX_1				0	/*ͨ��ĳһ�˵�����ֵ*/
#define TBUS_CHANNEL_SIDE_INDEX_2				1	/*ͨ��ĳһ�˵�����ֵ*/

#define TBUS_MAX_PATH_LEN					1024    /*path·������*/
#define TBUS_APPDATA_PATH					"SystemRoot" /*appdata ��������*/
#define TBUS_SHM_MMAP_PATH					"tbus" /*appdata ��������*/

#if defined(_WIN32) || defined(_WIN64)
#define TBUS_DEFAULT_MMAP_FILE_DIR			"C:"
#else
#define TBUS_DEFAULT_MMAP_FILE_DIR			"/tmp"
#endif

#define TBUS_GCIM_SHM_ACCESS	0664	//GCIM�����ڴ�ķ���Ȩ��
#define TBUS_GRM_SHM_ACCESS	0664	//GRM�����ڴ�ķ���Ȩ��

#define TBUS_GCIM_CHANNEL_FLAG_NONE                 0X00000000
#define TBUS_GCIM_CHANNEL_FLAG_ENABLE               0X00000001 	/*�������Ƿ���Ч*/
#define TBUS_GCIM_CHANNEL_IS_ENABLE(pstChannel)     ((pstChannel)->dwFlag & TBUS_GCIM_CHANNEL_FLAG_ENABLE)
#define TBUS_GCIM_CHANNEL_SET_ENABLE(pstChannel)       ((pstChannel)->dwFlag |= TBUS_GCIM_CHANNEL_FLAG_ENABLE)
#define TBUS_GCIM_CHANNEL_CLR_ENABLE(pstChannel)		((pstChannel)->dwFlag &= ~TBUS_GCIM_CHANNEL_FLAG_ENABLE)

#define TBUS_SET_GCIM_FLAG_RELATIVE  0x00000001 /* ���ģʽ�����ڲ��������ļ��е�ͨ������������ */
                                                /* ����ھ���ģʽ(���ڲ��������ļ��е�ͨ�����ӹ����ڴ���ɾ��) */
#define TBUS_SET_GCIM_FLAG_RECREATE  0x00000002 /* �ؽ�ģʽ�������Ѿ����ڵ�ͨ������ɾ���ٴ��� */
                                                /* ����ڷ��ؽ�ģʽ(�����Ѿ����ڵ�ͨ���������κβ���) */

#ifdef __cplusplus
extern "C"
{
#endif


typedef struct tagTbusShmGRMHead                        	TBUSSHMGRMHEAD;
typedef struct tagTbusShmGRMHead                        	*LPTBUSSHMGRMHEAD;

typedef struct tagTbusShmGCIMHead TBUSSHMGCIMHEAD;
typedef struct tagTbusShmGCIMHead *LPTBUSSHMGCIMHEAD;

typedef struct tagTbusShmGCIM	TBUSSHMGCIM;
typedef struct tagTbusShmGCIM	*LPTBUSSHMGCIM;

typedef struct tagTbusShmRelayCnf		TBUSSHMRELAYCNF;
typedef struct tagTbusShmRelayCnf	*LPTBUSSHMRELAYCNF;




typedef struct tagShmChannelCnf                         	TBUSSHMCHANNELCNF;
typedef struct tagShmChannelCnf                         	*LPTBUSSHMCHANNELCNF;

typedef struct tagChannelHead                      	CHANNELHEAD;
typedef struct tagChannelHead                      	*LPCHANNELHEAD;

typedef struct tagTbusShmGRM  TBUSSHMGRM;
typedef struct tagTbusShmGRM *LPTBUSSHMGRM;

struct tagTbusShmGRMHead
{
	pthread_rwlock_t stRWLock;						/**< TBUS���ù����ڴ��У�ͨ�������д�����л������*/
	volatile unsigned int dwVersion;                       	/*   ��shm�е����ݸ��º���Ҫ���¸���ֵ���Է���Ӧ��ģ����н������¼��� */
	unsigned int dwShmKey;                        	/*   GCIM�����ڴ�key����Ҫ��У���� */
	unsigned int dwShmSize;                       	/*   �����ڴ��С */
	unsigned int dwMaxCnt;                        	/*   �����ڴ����������ɵ�channel��Ϣ��slot�� */
	unsigned int dwUsedCnt;                       	/*   �����ڴ����õ�slot�� */
	time_t dwCreateTime;                    	/*  �����ڴ洴��ʱ�� */
	unsigned int dwLastStamp;                     	/*  �����ڴ�����޸�ʱ��� */
	unsigned int dwBusiID;	/**<ʹ��tbusϵͳ��ҵ���ID*/
	TBUSADDRTEMPLET	stAddrTemplet;					/**< ��busϵͳ��ʹ�õĵ��ʮ���Ƶ�ַģ��*/
	unsigned int dwCRC;                           	/*  �����ڴ������ */
	unsigned int aiReserved[TBUS_SHM_GRM_HEAD_RESERV_NUM]; 	/*Ԥ��������*/
};

#define TBUS_GCIM_BUILD     1
struct tagTbusShmGCIMHead
{
	volatile unsigned int dwVersion;                       	/*   ��shm�е����ݸ��º���Ҫ���¸���ֵ���Է���Ӧ��ģ����н������¼��� */
	unsigned int dwShmKey;                        	/*   GCIM�����ڴ�key����Ҫ��У���� */
	unsigned int dwShmSize;                       	/*   �����ڴ��С */
	unsigned int dwMaxCnt;                        	/*   �����ڴ����������ɵ�channel��Ϣ��slot�� */
	unsigned int dwUsedCnt;                       	/*   �����ڴ����õ�slot�� */
	time_t dwCreateTime;                    	/*  �����ڴ洴��ʱ�� */
	time_t dwLastStamp;                     	/*  �����ڴ�����޸�ʱ��� */
	unsigned int dwBusiID;	/**<ʹ��tbusϵͳ��ҵ���ID*/
	unsigned int dwCRC;                           	/*  �����ڴ������ */
	int iAlign;				/*ͨ�����ݶ��뷽ʽ*/
	int iAlignLevel;				/*ͨ�����ݶ���ֵ�Ķ���ֵ,��ֵͨ��iAlign�������*/
	pthread_rwlock_t stRWLock;						/**< TBUS���ù����ڴ��У�ͨ�������д�����л������*/
	TBUSADDRTEMPLET	stAddrTemplet;					/**< ��busϵͳ��ʹ�õĵ��ʮ���Ƶ�ַģ��*/
    unsigned int dwBuild;   /*GCIM���ݽṹ�İ汾*/
	unsigned int aiReserved[TBUS_SHM_GCIM_HEAD_RESERV_NUM]; 	/*Ԥ��������*/
};

struct tagShmChannelCnf
{
	unsigned int dwFlag;                        	/*   bit set: 0001 -- route valid */
	unsigned int dwPriority;
	HANDLE iShmID;	/*id of share memory for channel*/
	TBUSADDR astAddrs[2];                        	/* ͨ�����˵ĵ�ַ  original ip format: xxx.xxx.xxx.xxx, converted by tbus_addr_aton() */

	unsigned int dwRecvSize;                      	/*����astAddrs[0]��˵����ն��еĴ�С, default is 2M */
	unsigned int dwSendSize;                      	/*����astAddrs[0]��˵�䷢�Ͷ��еĴ�С, default is 2M */


	time_t dwCTime;
	time_t dwInvalidTime;	/*��ͨ��������ΪʧЧ����ʼʱ��*/
	int aiReserved[TBUS_CHANNEL_RESERVER_NUM];
};

/** �����ڴ��е�ͨ����Ϣ���ñ�
*/
struct tagTbusShmGCIM
{
	TBUSSHMGCIMHEAD	stHead;
	TBUSSHMCHANNELCNF astChannels[TBUS_MAX_CHANNEL_NUM_PREHOST]; /*����ͨ����������Ϣ*/
};



struct tagTbusShmRelayCnf
{
	unsigned int dwFlag;                        	/*   bit set: 0001 -- relay node valid */
	TBUSADDR dwAddr; 	 /*  Message sent to this addr would be forwarded via network */
	unsigned int dwPriority;
	char szMConn[TBUS_MAX_HOST_URL_LEN];  	/*   master connection information */
	char szSConn[TBUS_MAX_HOST_URL_LEN];  	/*   slave connection information */
	unsigned int dwStrategy;                      	/*   strategy between master connection and slave connection */
};


/*�����ڴ��е�ͨ����Ϣ���ñ�*/
struct tagTbusShmGRM
{
	TBUSSHMGRMHEAD	stHead;
	TBUSSHMRELAYCNF astRelays[TBUS_MAX_RELAY_NUM_PREHOST]; /*������תͨ����������Ϣ*/
};

/**����GCIM�����ڴ�
*@param[in,out] ppstGCIM ����GCIM�����ڴ�ṹָ���ָ��
*@param[in] a_pszShmkey ���ɹ����ڴ�key�������Ϣ��
*@param[in]	a_iBussId ҵ��id
*@param[in]	a_iShmSize ָ�������ڴ��С�����ڼ����Ѵ��ڵĹ����ڴ�˲�����������Ϊ0
*@param[in]	iSHmFlag ���Ʊ�־λ������ָʾshm����Ȩ�ޣ��Ƿ���Ҫ��������Ϣ
*@param[out] a_phShmHandle, if not NULL, save mmapID
*@retval 0 �ɹ�
*@retval <0 ʧ��
*/
int tbus_create_gcimshm(INOUT LPTBUSSHMGCIM *a_ppstGCIM, IN const char *a_pszShmkey,
							   IN int a_iBussId, IN unsigned int a_iShmSize,
                               OUT HANDLE *a_phShmHandle);

/**����GCIM�����ڴ�
*@param[in,out] ppstGCIM ����GCIM�����ڴ�ṹָ���ָ��
*@param[in] a_pszShmkey ���ɹ����ڴ�key�������Ϣ��
*@param[in]	a_iBussId ҵ��id
*@param[in]	a_iShmSize ָ�������ڴ��С�����ڼ����Ѵ��ڵĹ����ڴ�˲�����������Ϊ0
*@param[in] a_iTimeout If the gcim share memory do not exist, try to attach again, until time out
*@param[out] a_phShmHandle, if not NULL, save mmapID
*@retval 0 �ɹ�
*@retval <0 ʧ��
*/
int tbus_get_gcimshm(INOUT LPTBUSSHMGCIM *a_ppstGCIM, IN const char *a_pszShmkey,
					 IN int a_iBussId, IN unsigned int a_iShmSize,
                     IN int a_iTimeout, OUT HANDLE *a_phShmHandle);


/**��GCIM����д�������ڴ���
*@param[in] a_pstShmGCIM ָ��GCIM�����ڴ����ݽṹָ��
*@param[in] a_pstGCIM GCIM������Ϣ�ṹָ��
*@retval 0 �ɹ�
*@retval <0 ʧ��
*/
int tbus_set_gcim(IN LPTBUSSHMGCIM a_pstShmGCIM, IN LPTBUSGCIM a_pstGCIM);

/**��GCIM����д�������ڴ���
*@param[in] a_pstShmGCIM ָ��GCIM�����ڴ����ݽṹָ��
*@param[in] a_pstGCIM GCIM������Ϣ�ṹָ��
*@param[in] a_iFlag ������־λ, ����ȡ����ֵ(����|����)
*-TBUS_SET_GCIM_FLAG_RELATIVE ���ģʽ�����ڲ��������ļ��е�ͨ������������
                              ����ھ���ģʽ(���ڲ��������ļ��е�ͨ�����ӹ����ڴ���ɾ��)
*-TBUS_SET_GCIM_FLAG_RECREATE �����Ѿ����ڵ�ͨ������ɾ�������´���
*@retval 0 �ɹ�
*@retval <0 ʧ��
*/
int tbus_set_gcim_ex(IN LPTBUSSHMGCIM a_pstShmGCIM, IN LPTBUSGCIM a_pstGCIM, IN int a_iFlag);

void tbus_detach_gcim(IN LPTBUSSHMGCIM a_pstShmGCIM, IN HANDLE a_hShmHandle);

/**ɾ��ָ��������ͨ������
*@param[in] a_pstShmGCIM ָ��GCIM�����ڴ����ݽṹָ��
*@param[in] a_idx ָ��ͨ����GCIM�е�����
*@retval 0 �ɹ�
*@retval <0 ʧ��
*/
int tbus_delete_channel_by_index(IN LPTBUSSHMGCIM a_pstShmGCIM, IN int a_idx);

/**enableָ��������ͨ������
*@param[in] a_pstShmGCIM ָ��GCIM�����ڴ����ݽṹָ��
*@param[in] a_idx ָ��ͨ����GCIM�е�����
*@retval 0 �ɹ�
*@retval <0 ʧ��
*/
int tbus_enable_channel_by_index(IN LPTBUSSHMGCIM a_pstShmGCIM, IN int a_idx);

/**disableָ��������ͨ������
*@param[in] a_pstShmGCIM ָ��GCIM�����ڴ����ݽṹָ��
*@param[in] a_idx ָ��ͨ����GCIM�е�����
*@retval 0 �ɹ�
*@retval <0 ʧ��
*/
int tbus_disable_channel_by_index(IN LPTBUSSHMGCIM a_pstShmGCIM, IN int a_idx);

/** Register business id to Senderd-Agent system
*/
int tbus_register_bussid(void);

/** free memory that agent_api_init malloced
 */
void tbus_agent_api_destroy(void);
void tbus_set_shmgcimheader_stamp(LPTBUSSHMGCIMHEAD pstHeader);

/**���·�ɱ����ڴ�ļ�����,������гɹ�����0,���򷵻ط���ֵ
*/
int tbus_check_shmgcimheader_stamp(LPTBUSSHMGCIMHEAD pstHeader);

void tbus_set_shmgrmheader_stamp(LPTBUSSHMGRMHEAD pstHeader);


int tbus_check_shmgrmheader_stamp(LPTBUSSHMGRMHEAD pstHeader);

/**����GRM�����ڴ�
*@param[in,out] a_ppstGRM ����GRM�����ڴ�ṹָ���ָ��
*@param[in] a_pszShmkey ���ɹ����ڴ�key�������Ϣ��
*@param[in]	a_iBussId ҵ��id
*@param[in]	a_iShmSize ָ�������ڴ��С�����ڼ����Ѵ��ڵĹ����ڴ�˲�����������Ϊ0
*@retval 0 �ɹ�
*@retval <0 ʧ��
*/
int tbus_create_grmshm(INOUT LPTBUSSHMGRM *a_ppstGRM, IN const char *a_pszShmkey,
						IN int a_iBussId, IN unsigned int a_iShmSize, HANDLE* a_phShmHandle);

/**����GRM�����ڴ�
*@param[in,out] a_ppstGRM ����GRM�����ڴ�ṹָ���ָ��
*@param[in] a_pszShmkey ���ɹ����ڴ�key�������Ϣ��
*@param[in]	a_iBussId ҵ��id
*@param[in]	a_iShmSize ָ�������ڴ��С�����ڼ����Ѵ��ڵĹ����ڴ�˲�����������Ϊ0
*@retval 0 �ɹ�
*@retval <0 ʧ��
*/
int tbus_get_grmshm(INOUT LPTBUSSHMGRM *a_ppstGRM, IN const char *a_pszShmkey,
					 IN int a_iBussId, IN unsigned int a_iShmSize, HANDLE* a_phShmHandle);


void tbus_detach_grmshm(IN LPTBUSSHMGRM a_pstGRM, HANDLE a_phShmHandle);

/**��GRM����д�������ڴ���
*@param[in] a_pstShmGRM ָ��GCIM�����ڴ����ݽṹָ��
*@param[in] a_pstRelayConf GRM������Ϣ�ṹָ��
*@retval 0 �ɹ�
*@retval <0 ʧ��
*/
int tbus_set_grm(IN LPTBUSSHMGRM a_pstShmGRM, IN LPRELAYMNGER a_pstRelayConf);

int tbus_check_channel_shm_i(LPTBUSSHMCHANNELCNF a_pstShmChl, LPTBUSSHMGCIMHEAD a_pstHead, int a_iFlag);

int tbus_delete_channel_shm_i(LPTBUSSHMCHANNELCNF a_pstShmChl, LPTBUSSHMGCIMHEAD a_pstHead);

int tbus_get_attached_num(const char* a_pszShmKey, int a_iBussId, int *a_piNum, HANDLE *a_piShmID);
int tbus_wrlock(pthread_rwlock_t *a_pstRWLock);

int tbus_rdlock(pthread_rwlock_t *a_pstRWLock);

void tbus_unlock(pthread_rwlock_t *a_pstRWLock);

int tbus_gen_shmkey(OUT key_t *ptShmkey, IN const char *a_pszShmkey,
					IN int a_iBussId);

int tbus_is_channel_in_use(IN LPTBUSSHMCHANNELCNF a_pstShmChl,
        LPTBUSSHMGCIMHEAD a_pstHead, INOUT int* a_iIsInUse);

#ifdef __cplusplus
}
#endif /*__cplusplus*/
#endif/*TBUS_CONFIG_MNG_H*/

