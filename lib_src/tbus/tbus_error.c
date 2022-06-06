
#include <assert.h>
#include <stdio.h>

#include "tbus/tbus_error.h"
#include "tbus/tbus.h"

#if defined (_WIN32) || defined (_WIN64)
#pragma warning(disable:4996)
#endif



const char * tbus_error_string(IN int iErrorCode)
{
	int iTdrErrno = TBUS_ERR_GET_ERROR_CODE(iErrorCode);
	const char *pchStr = NULL;

	static const char *message[] = {
		"没有错误",
		"分配内存失败",
		"获取tbus头部的元数据描述失败",
		"初始化日志系统失败",
		"解析通信地址十进制点分表示模板失败",
		"点分十进制通信地址串不正确",
		"向代理系统注册业务ID失败",
		"通过shmget分配共享内存失败",
		"通过shmat加载共享内存失败",
		"生成共享内存的key值失败",
		"GCIM共享内存校验失败",
		"传递给接口的参数不对",
		"Tbus系统还没有初始化",
		"已分配tbus句柄数据已经达到设定的最大数目，不能在分配",
		"tbus处理句柄无效",
		"一个tbus处理句柄所管理的通道数已经到达上限",
		"通过指定地址绑定不到任何通道",
		"tbus句柄没有绑定任何通信通道",
		"通道共享内存中记录的地址信息与GCIM中记录的地址信息不一致",
		"与tbus句柄管理的任何通道的对端地址都不匹配",
		"没有与指定源地址和目的地址匹配的通道",
		"tbus头部打包失败",
		"tbus头部打包信息长度超过了最大预定长度",
		"tbus发送消息通道已满，不能再发送数据",
		"tbus消息接收通道已空，没有任何消息可接收",
		"通道中数据长度错误，只有丢弃此数据才能恢复正常",
		"接收数据的缓冲区太小，不足以接收整个数据包",
		"tbus消息头部解包失败",
		"受最大转发路由跳点限制，不能继续转发",
		"路由信息无效",
		"GCIM的配置不正确",
		"shmctl操作共享内存失败",
		"管理的中转通道数已经到达最大数目限制",
		"校验数据包头部失败",
		"数据通道正在使用，不能删除此通道的共享内存",
		"不支持的option",
		"选项值不正确",
		"tbus channel保存核心数据的头部结构指针为NULL",
		"TBus channel共享内存数据中记录的Magic匹配不上",
		"Tbus 数据通道中的字节对齐设置不正确",
		"TBUS  通道所需大小与共享内存实际大小不一致",
		"TBUS 数据通道的大小与 指定配置的值不一致",
		"TBus 通道队列中的参数不正确",
        "不应该使用特殊的TBUS地址",
        "没有匹配的对端进程",
        "没有更多的peer进程",
	    "接收地址的缓冲区太小，不足以接收所有的peer地址",
	};

	assert((sizeof(message) / sizeof(message[0])) == TBUS_ERROR_COUNT);

	if (!TBUS_ERR_IS_ERROR(iErrorCode))
	{
		pchStr = "没有错误";
	}else  if ((iTdrErrno < 0) || (iTdrErrno > TBUS_ERROR_COUNT))
	{
		pchStr =  "未知错误";
	}
	else
	{
		pchStr = message[iTdrErrno];
	}

	return pchStr;
}
