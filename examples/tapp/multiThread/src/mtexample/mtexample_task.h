#ifndef EXAMPLE_TASK_H
#define EXAMPLE_TASK_H

#include "mtexample.h"
#include "pal/pal.h"
#include "comm/tmempool.h"
#include "comm/tlist.h"

struct tagExampleTask
{
	uint32_t timeOutTick;
	int iSrc;
	MTEXAMPLEMSG stReq;
};

typedef struct tagExampleTask MTEXAMPLETASK;
typedef struct tagExampleTask *LPMTEXAMPLETASK;

struct tagMTExampleTaskQueue;
typedef struct tagMTExampleTaskQueue *LPMTEXAMPLETASKQUEUE;

struct tagMTExampleTaskQueue
{
	LPTMEMPOOL pstTaskpool;
};

typedef struct tagMTExampleTaskQueue MTEXAMPLETASKQUEUE;

/*Create a new task*/
int mtexample_task_init(MTEXAMPLEENV *pstEnv);

int mtexample_task_new(MTEXAMPLEENV *pstEnv);

LPMTEXAMPLETASK mtexample_task_get(MTEXAMPLEENV *pstEnv,int iTaskId);

int mtexample_task_del(MTEXAMPLEENV *pstEnv,int iTaskId);

int mtexample_task_get_first(MTEXAMPLEENV *pstEnv);
#endif
