#ifndef MEMLOG_H_
#define MEMLOG_H_

#include "pal/pal.h"
#include "tlog/tlog_loc_info.h"
#include "comm/comm.h"
#include "tlog/tlog_bill.h"
#include "tlog/tlog.h"

struct tagMemLog;
typedef struct tagMemLog *LPMEMLOG;

#if !defined(_WIN32) && !defined(_WIN64)

#define tmemlog_log(pstMemLog,fmt,args...) \
	do{\
	tmemlog_print(pstMemLog,"%s():"__FILE__"[%d]"fmt,__FUNCTION__,__LINE__, ##args);\
	}while(0)

#else

#define tmemlog_log(pstMemLog,fmt,...) \
	do{\
	tmemlog_print(pstMemLog,"%s():%s[%d]"fmt,__FUNCTION__,basename(__FILE__),__LINE__, __VA_ARGS__);\
	}while(0)

#endif

int tmemlog_new(LPMEMLOG *ppstMemLog,int inMaxItem,char *outFileName);
int tmemlog_print(LPMEMLOG pstMemLog, const char* a_pszFmt, ...);
int tmemlog_append(LPMEMLOG pstMemLog,char *pszBuff,size_t iBuff);
int tmemlog_dump_to_fd(LPMEMLOG pstMemLog,FILE *fd);
int tmemlog_fini(LPMEMLOG pstMemLog,int isWrite);
int tmempool_dump(LPMEMLOG pstMemLog);
#endif

