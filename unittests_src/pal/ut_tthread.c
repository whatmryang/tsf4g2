/*
**  @file $RCSfile: ut_tthread.c,v $
**  general description of this module
**  $Id: ut_tthread.c,v 1.1.1.1 2008-05-28 07:35:00 kent Exp $
**  @author $Author: kent $
**  @date $Date: 2008-05-28 07:35:00 $
**  @version $Revision: 1.1.1.1 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#include "pal/pal.h"
#include <CUnit/CUnit.h>

void ut_pthread(void);
void ut_mutex(void);
void ut_rwlock(void);
void ut_semaphore(void);

//////////////////////////////////////
static void* ut_new_thread(void* pvArg)
{
	int iSec;

	iSec	=	*(int*)pvArg;

	printf("pthread_create success, in new thread, wait %d sec to exit\n", iSec);

	usleep(iSec*1000000);

	return NULL;
}

void ut_pthread(void)
{
	pthread_t th;
	pthread_attr_t attr;
	void* pvRet;
	int iSec;
	int iRet;

	iSec	=	2;

	pthread_attr_init(&attr);

	iRet	=	pthread_create(&th, &attr, ut_new_thread, &iSec);

	CU_ASSERT(0==iRet);

	pthread_attr_destroy(&attr);

	iRet	=	pthread_join(th, &pvRet);

	CU_ASSERT( 0==iRet );

	printf("pthread_join returned\n");
}

void ut_mutex(void)
{
	int iRet;

	pthread_mutexattr_t attr;

	pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;

	iRet	=	pthread_mutex_init(&mutex, NULL);
	CU_ASSERT( 0==iRet );

	iRet	=	pthread_mutex_lock(&mutex);
	CU_ASSERT( 0==iRet );

	iRet	=	pthread_mutex_unlock(&mutex);
	CU_ASSERT( 0==iRet );

/*
	iRet	=	pthread_mutex_trylock(&mutex);
	CU_ASSERT( 0==iRet );

	iRet	=	pthread_mutex_unlock(&mutex);
	CU_ASSERT( 0==iRet );
*/

	iRet	=	pthread_mutex_destroy(&mutex);
	CU_ASSERT( 0==iRet );

	return;
}

void ut_rwlock(void)
{
	/*
	int iRet;
	pthread_rwlock_t lock;
	pthread_rwlockattr_t attr;

	iRet	=	pthread_rwlockattr_init(&attr);
	CU_ASSERT( 0==iRet );

	iRet	=	phtread_rwlock_init(&lock, &attr);
	CU_ASSERT( 0==iRet );

	iRet	=	pthread_rwlock_rdlock(&lock);
	CU_ASSERT( 0==iRet );

	iRet	=	pthread_rwlock_unlock(&lock);
	CU_ASSERT( 0==iRet );

	iRet	=	pthread_rwlock_wrlock(&lock);
	CU_ASSERT( 0==iRet );

	iRet	=	pthread_rwlock_unlock(&lock);
	CU_ASSERT( 0==iRet );

	iRet	=	pthread_rwlock_destroy(&lock);
	CU_ASSERT( 0==iRet );
	*/
}

void ut_semaphore(void)
{
	sem_t sem;
	int iRet;
	int iValue;

	iRet	=	sem_init(&sem, 0, 100);
	CU_ASSERT( 0==iRet );

	iRet	=	sem_post(&sem);
	CU_ASSERT( 0==iRet );

	iRet	=	sem_getvalue(&sem, &iValue);
	CU_ASSERT( 0==iRet );
	
	iRet	=	sem_destroy(&sem);
	CU_ASSERT( 0==iRet );
}

