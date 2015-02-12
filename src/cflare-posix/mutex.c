
#include "cflare/mutex.h"
#include "cflare/util.h"

#include <stdlib.h>
#include <assert.h>

#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <sys/time.h>

cflare_mutex* cflare_mutex_new(cflare_mutex_type type)
{
	pthread_mutex_t* mutex = malloc(sizeof(pthread_mutex_t));
	
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
	
	
	assert(pthread_mutex_init(mutex, &attr) == 0);
	pthread_mutexattr_destroy(&attr);
	
	return (cflare_mutex*)mutex;
}

void cflare_mutex_delete(cflare_mutex* mtx)
{
	pthread_mutex_t* mutex = (pthread_mutex_t*)mtx;
	pthread_mutex_destroy(mutex);
	free(mutex);
}

void cflare_mutex_lock(cflare_mutex* mtx)
{
	pthread_mutex_t* mutex = (pthread_mutex_t*)mtx;
	pthread_mutex_lock(mutex);
}

void cflare_mutex_unlock(cflare_mutex* mtx)
{
	pthread_mutex_t* mutex = (pthread_mutex_t*)mtx;
	pthread_mutex_unlock(mutex);
}

cflare_rwmutex* cflare_rwmutex_new(cflare_mutex_type type)
{
	pthread_rwlock_t* mutex = malloc(sizeof(pthread_rwlock_t));
	
	pthread_rwlockattr_t attr;
	pthread_rwlockattr_init(&attr);
	assert(pthread_rwlock_init(mutex, &attr) == 0);
	pthread_rwlockattr_destroy(&attr);
	
	return (cflare_rwmutex*)mutex;
}

void cflare_rwmutex_delete(cflare_rwmutex* mtx)
{
	pthread_rwlock_t* mutex = (pthread_rwlock_t*)mtx;
	pthread_rwlock_destroy(mutex);
	free(mutex);
}

void cflare_rwmutex_read_lock(cflare_rwmutex* mtx)
{
	pthread_rwlock_t* mutex = (pthread_rwlock_t*)mtx;
	pthread_rwlock_rdlock(mutex);
}

void cflare_rwmutex_read_unlock(cflare_rwmutex* mtx)
{
	pthread_rwlock_t* mutex = (pthread_rwlock_t*)mtx;
	pthread_rwlock_unlock(mutex);
}

void cflare_rwmutex_write_lock(cflare_rwmutex* mtx)
{
	pthread_rwlock_t* mutex = (pthread_rwlock_t*)mtx;
	pthread_rwlock_wrlock(mutex);
}

void cflare_rwmutex_write_unlock(cflare_rwmutex* mtx)
{
	pthread_rwlock_t* mutex = (pthread_rwlock_t*)mtx;
	pthread_rwlock_unlock(mutex);
}


