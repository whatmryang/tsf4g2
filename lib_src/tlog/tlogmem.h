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
 * ��ʼ���ڴ���־
 * */
int tlogmem_init(TLOGMEM *pstMem, TLOGDEVMEM *pstDev);

/*
 * ͨ���ڴ���־�ӿڷ���buffer������iBuffΪ�����õ� ��buffer��С
 * */
int tlogmem_get_buff(TLOGMEM *pstMemHandle, char **ppszBuff, size_t iBuff);
/*
 * ��tlogmem_get_buff֮����ã�ȷ��ǰ��д������ݿ��Թ���һ����¼
 * */
int tlogmem_make_frame(TLOGMEM *pstMem, TLOGEVENTBASE *pstEvt, size_t iUsed);

int tmemlog_dump_memdev(TLOGDEVMEM *pstDev);
int tmemlog_dump_tlog_conf(const char *pszConfFileName);
/*
 * �ͷ��ڴ���־ռ�õ������Դ
 * */
int tlogmem_fini(TLOGMEM *pstMem);
#ifdef __cplusplus
}
#endif

#endif /* TLOGMEM_H_ */
