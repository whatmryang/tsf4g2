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
	unsigned int uin;              //必填项,用户QQ 
	unsigned int uCltIP;           //必填项,客户端IP
	unsigned int uAppID;           //可选项,最好填上,业务ID
	unsigned int uCltVer;          //可选项,用户客户端版本,根据业务是否需要对版本验证决定是否填写此项和打开验证开关
};

struct tagTQQSigCheckList
{
	unsigned int uin;              //必填项,签名中用户QQ 
	unsigned int utimestamp;       //必填项,签名中时戳信息
	unsigned int uCltIP;           //必填项,签名中客户端IP
	unsigned int uAppID;           //可选项,签名中业务类型,对于0x82签名结构该字段可以不填,对于0xDE需要填写
	unsigned int uCltVer;          //可选项,签名中客户端版本,对于0x82签名结构可以不填,对于0xDE可以填写,是否验证根据业务具体情况配置
};

 /**
*@brief:创建,初始化句柄并读取配置
*@param pSigCtx[IN]:							结构句柄指针地址
*@param szConfilePath[IN]:     		配置文件
*@retval
*         = 0 for success
*         <0 failed,返回错误码
*         -1：输入参数错误
*         -2: 读取CheckSigTime失败
*         -3: 读取TimeOutValue失败
*         -4: 读取CheckUin失败
*         -5: 读取CheckCltIP失败
*         -6: 读取AllowedTimeGap失败
*         -7: 读取NetMaskBitCount失败
*	    -8: 读取CheckServiceID失败
*	    -9: 读取CheckCltver失败
*	    -10:读取logdir失败
*	    -11:创建logdir失败
*         -12:创建句柄失败
*/
int tsigapi_init(IN LPTQQSIGCTX *ppSigCtx,IN const char *szConfilePath);

 /**
*@brief:reload,刷新配置文件
*@param pSigCtx[IN]:							结构句柄指针
*@param szConfilePath[IN]:     		配置文件
*@retval
*         = 0 for success
*         <0 failed,返回错误码,错误码同tsigapi_init
*/
int tsigapi_reload(IN TQQSIGCTX *pSigCtx,IN const char *szConfilePath);




 /**
*@brief:对签名中的信息和当前信息进行二次验证
*@param  pSigCtx[IN]:			  结构句柄指针
*@param  pstCltInfo[IN]                当前连接的信息
*@param  pstSigInfo[IN]                签名中的信息
*@retval
*         = 0 for success
*         <0 failed,返回错误码
*         -1:   输入参数验证不合法
*         -2:   uin 验证不一致
*         -3:   时戳超时
*         -4:   IP验证不合法
*         -5:   serviceid不一致
*         -6:   版本不一致
*/
int tsigapi_check(IN TQQSIGCTX *pSigCtx,IN TQQConnInfo *pstConnInfo,IN TQQSIGCHECKLIST *pstSigInfo);


 /**
*@brief:释放句柄
*@param pSigCtx[IN]:							  结构句柄指针
*@retval
*         = 0 for success
*         <0 failed,返回错误码
*         -1:输入参数错误
*/
int tsigapi_fini(IN LPTQQSIGCTX *ppSigCtx);


#ifdef __cplusplus
}
#endif






#endif 
