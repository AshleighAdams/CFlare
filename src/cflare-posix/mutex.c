
#include "cflare/mutex.h"
#include "cflare/util.h"

#include <stdlib.h>
#include <assert.h>

#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <sys/time.h>

typedef struct cflare_mutex
{
	pthread_mutex_t* mutex;
} cflare_mutex;

typedef struct cflare_rwmutex
{
	pthread_rwlock_t* mutex;
} cflare_rwmutex;

cflare_mutex* cflare_mutex_new(cflare_mutex_type type)
{
	cflare_mutex* mutex = malloc(sizeof(cflare_mutex));
	mutex->mutex = malloc(sizeof(pthread_mutex_t));
	
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	
	if(type & CFLARE_MUTEX_TRY)
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);
	if(type & CFLARE_MUTEX_TIMED)
	{
		#ifdef PTHREAD_MUTEX_TIMED_NP // this is commonly not availible.
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_TIMED_NP);
		#else
		cflare_warn("this version of pthreads does not support MUTEX_TIMED_NP!");
		#endif
	}
	if(type & CFLARE_MUTEX_RECURSIVE)
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	
	
	assert(pthread_mutex_init(mutex->mutex, &attr) == 0);
	pthread_mutexattr_destroy(&attr);
	
	return mutex;
}

void cflare_mutex_delete(cflare_mutex* mtx)
{
	pthread_mutex_destroy(mtx->mutex);
	free(mtx->mutex);
	free(mtx);
}

void cflare_mutex_lock(cflare_mutex* mtx)
{
	pthread_mutex_lock(mtx->mutex);
}

void cflare_mutex_unlock(cflare_mutex* mtx)
{
	pthread_mutex_unlock(mtx->mutex);
}

cflare_rwmutex* cflare_rwmutex_new(cflare_mutex_type type)
{
	cflare_rwmutex* mutex = malloc(sizeof(cflare_rwmutex));
	mutex->mutex = malloc(sizeof(pthread_rwlock_t));
	
	pthread_rwlockattr_t attr;
	pthread_rwlockattr_init(&attr);
	assert(pthread_rwlock_init(mutex->mutex, &attr) == 0);
	pthread_rwlockattr_destroy(&attr);
	
	return mutex;
}

void cflare_rwmutex_delete(cflare_rwmutex* mtx)
{
	pthread_rwlock_destroy(mtx->mutex);
	free(mtx->mutex);
	free(mtx);
}

void cflare_rwmutex_read_lock(cflare_rwmutex* mtx)
{
	pthread_rwlock_rdlock(mtx->mutex);
}

void cflare_rwmutex_read_unlock(cflare_rwmutex* mtx)
{
	pthread_rwlock_unlock(mtx->mutex);
}

void cflare_rwmutex_write_lock(cflare_rwmutex* mtx)
{
	pthread_rwlock_wrlock(mtx->mutex);
}

void cflare_rwmutex_write_unlock(cflare_rwmutex* mtx)
{
	pthread_rwlock_unlock(mtx->mutex);
}


