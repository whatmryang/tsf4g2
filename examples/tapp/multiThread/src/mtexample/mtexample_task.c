#include "mtexample_task.h"
#include "mtexample.h"

int mtexample_task_init(MTEXAMPLEENV *pstEnv)
{
	//Init the task queue for example.
	int iRet;

	iRet = tmempool_new(&pstEnv->pstTaskPool,1000,sizeof(MTEXAMPLETASK));
	if(iRet)
	{
		return -1;
	}
	return 0;
}

int mtexample_task_new(MTEXAMPLEENV *pstEnv)
{
	return tmempool_alloc(pstEnv->pstTaskPool);
}

LPMTEXAMPLETASK mtexample_task_get(MTEXAMPLEENV *pstEnv,int iTaskId)
{
	return tmempool_get(pstEnv->pstTaskPool,iTaskId);
}

int mtexample_task_del(MTEXAMPLEENV *pstEnv,int iTaskId)
{
	return tmempool_free(pstEnv->pstTaskPool,iTaskId);
}

#if 0
int mtexample_task_get_first(MTEXAMPLEENV *pstEnv)
{
	return tmempool_get_used_first(pstEnv->pstTaskPool);
}

#endif





