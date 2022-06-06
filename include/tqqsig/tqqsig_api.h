#ifndef _TQQSIGAUTH_H
#define _TQQSIGAUTH_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef IN
#define  IN
#endif

#ifndef OUT
#define  OUT
#endif

#ifndef INOUT
#define INOUT
#endif


#define TQQSIG_REASON_NONE 0

enum tagTQQSIG_AUTH 
{
    TQQSIG_OX82 = 1, 
    TQQSIG_0XDE = 2
};

enum tagTQQSIG_LOGTYPE
{
    
    TQQSIG_UIN = 1,
    TQQSIG_TIMESTAMP = 2,
    TQQSIG_IP = 3,
    TQQSIG_SERVICEID = 4,
    TQQSIG_CLTVER = 5	
};


enum tagTQQSIG_IP_REASON
{
    TQQSIG_IP_TIME = 1,
    TQQSIG_IP_NETMASK = 2
};

struct tagTQQSigCtx;

typedef struct tagTQQSigCtx		TQQSIGCTX;
typedef struct tagTQQSigCtx		*LPTQQSIGCTX;

struct tagTQQSigCheckList;
typedef struct tagTQQSigCheckList TQQSIGCHECKLIST;
typedef struct tagTQQSigCheckList *LPTQQSIGCHECKLIST;

struct tagTQQConnInfo;
typedef struct tagTQQConnInfo TQQConnInfo;
typedef struct tagTQQConnInfo *LPTQQConnInfo;

struct tagTQQConnInfo
{
	unsigned int uin;              //������,�û�QQ 
	unsigned int uCltIP;           //������,�ͻ���IP
	unsigned int uAppID;           //��ѡ��,�������,ҵ��ID
	unsigned int uCltVer;          //��ѡ��,�û��ͻ��˰汾,����ҵ���Ƿ���Ҫ�԰汾��֤�����Ƿ���д����ʹ���֤����
};

struct tagTQQSigCheckList
{
	unsigned int uin;              //������,ǩ�����û�QQ 
	unsigned int utimestamp;       //������,ǩ����ʱ����Ϣ
	unsigned int uCltIP;           //������,ǩ���пͻ���IP
	unsigned int uAppID;           //��ѡ��,ǩ����ҵ������,����0x82ǩ���ṹ���ֶο��Բ���,����0xDE��Ҫ��д
	unsigned int uCltVer;          //��ѡ��,ǩ���пͻ��˰汾,����0x82ǩ���ṹ���Բ���,����0xDE������д,�Ƿ���֤����ҵ������������
};

 /**
*@brief:����,��ʼ���������ȡ����
*@param pSigCtx[IN]:							�ṹ���ָ���ַ
*@param szConfilePath[IN]:     		�����ļ�
*@retval
*         = 0 for success
*         <0 failed,���ش�����
*         -1�������������
*         -2: ��ȡCheckSigTimeʧ��
*         -3: ��ȡTimeOutValueʧ��
*         -4: ��ȡCheckUinʧ��
*         -5: ��ȡCheckCltIPʧ��
*         -6: ��ȡAllowedTimeGapʧ��
*         -7: ��ȡNetMaskBitCountʧ��
*	    -8: ��ȡCheckServiceIDʧ��
*	    -9: ��ȡCheckCltverʧ��
*	    -10:��ȡlogdirʧ��
*	    -11:����logdirʧ��
*         -12:�������ʧ��
*/
int tsigapi_init(IN LPTQQSIGCTX *ppSigCtx,IN const char *szConfilePath);

 /**
*@brief:reload,ˢ�������ļ�
*@param pSigCtx[IN]:							�ṹ���ָ��
*@param szConfilePath[IN]:     		�����ļ�
*@retval
*         = 0 for success
*         <0 failed,���ش�����,������ͬtsigapi_init
*/
int tsigapi_reload(IN TQQSIGCTX *pSigCtx,IN const char *szConfilePath);




 /**
*@brief:��ǩ���е���Ϣ�͵�ǰ��Ϣ���ж�����֤
*@param  pSigCtx[IN]:			  �ṹ���ָ��
*@param  pstCltInfo[IN]                ��ǰ���ӵ���Ϣ
*@param  pstSigInfo[IN]                ǩ���е���Ϣ
*@retval
*         = 0 for success
*         <0 failed,���ش�����
*         -1:   ���������֤���Ϸ�
*         -2:   uin ��֤��һ��
*         -3:   ʱ����ʱ
*         -4:   IP��֤���Ϸ�
*         -5:   serviceid��һ��
*         -6:   �汾��һ��
*/
int tsigapi_check(IN TQQSIGCTX *pSigCtx,IN TQQConnInfo *pstConnInfo,IN TQQSIGCHECKLIST *pstSigInfo);


 /**
*@brief:�ͷž��
*@param pSigCtx[IN]:							  �ṹ���ָ��
*@retval
*         = 0 for success
*         <0 failed,���ش�����
*         -1:�����������
*/
int tsigapi_fini(IN LPTQQSIGCTX *ppSigCtx);


#ifdef __cplusplus
}
#endif






#endif 
