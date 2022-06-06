/*
 * tlogmem.h
 *
 *  Created on: 2010-7-7
 *      Author: mickeyxu
 */

#ifndef TLOGMEM_H_
#define TLOGMEM_H_

#include "tlogmemdef.h"
#include "tlog/tlog_loc_info.h"
#include "tlog/tlog_event.h"
#include "tlog_event_i.h"

struct tagTlogMem
{
    TLOGDEVMEM *pstDev;
    void *pstTsmHandle;
    void *pvMemHead;
};

typedef struct tagTlogMem TLOGMEM;
typedef struct tagTlogMem *LPTLOGMEM;
#ifdef __cplusplus
extern "C"
{
#endif

int tlogmem_dump(TLOGMEM *pstMem, size_t iSize);
int tlogmem_attach(TLOGMEM *pstMem, TLOGDEVMEM *pstDev);
/*
 * 初始化内存日志
 * */
int tlogmem_init(TLOGMEM *pstMem, TLOGDEVMEM *pstDev);

/*
 * 通过内存日志接口分配buffer，输入iBuff为期望得到 的buffer大小
 * */
int tlogmem_get_buff(TLOGMEM *pstMemHandle, char **ppszBuff, size_t iBuff);
/*
 * 在tlogmem_get_buff之后调用，确认前面写入的数据可以构成一条记录
 * */
int tlogmem_make_frame(TLOGMEM *pstMem, TLOGEVENTBASE *pstEvt, size_t iUsed);

int tmemlog_dump_memdev(TLOGDEVMEM *pstDev);
int tmemlog_dump_tlog_conf(const char *pszConfFileName);
/*
 * 释放内存日志占用的相关资源
 * */
int tlogmem_fini(TLOGMEM *pstMem);
#ifdef __cplusplus
}
#endif

#endif /* TLOGMEM_H_ */
