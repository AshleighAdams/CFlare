#include "cflare/mutex.h"

#include "cflare/util.h"
#include <Windows.h>

/*
Some code taken from the windows pthreads
wrapper, found at: http://locklessinc.com/articles/pthreads_on_windows/
*/

cflare_mutex* cflare_mutex_new(cflare_mutex_type type)
{
	CRITICAL_SECTION* mtx = malloc(sizeof(CRITICAL_SECTION));
	InitializeCriticalSection(mtx);
	return (cflare_mutex*)mtx;
}

void cflare_mutex_delete(cflare_mutex* mtx)
{
	// by the way, API funcs now have CFLARE_API prepended in their defs
	// as win32/64 needs __declspec(dllexport) to make .libs for .dlls
	CRITICAL_SECTION* mutex = (CRITICAL_SECTION*)mtx;
	DeleteCriticalSection(mutex);
	free(mutex);
}

void cflare_mutex_lock(cflare_mutex* mtx)
{
	CRITICAL_SECTION* mutex = (CRITICAL_SECTION*)mtx;
	EnterCriticalSection(mutex);
}

void cflare_mutex_unlock(cflare_mutex* mtx)
{
	CRITICAL_SECTION* mutex = (CRITICAL_SECTION*)mtx;
	LeaveCriticalSection(mutex);
}

// for when I add trylock
// return TryEnterCriticalSection(mutex) ? success : fail;

cflare_rwmutex* cflare_rwmutex_new(cflare_mutex_type type)
{
	SRWLOCK* mutex = malloc(sizeof(SRWLOCK));
	InitializeSRWLock(mutex);
	return (cflare_rwmutex*)mutex;
}

void cflare_rwmutex_delete(cflare_rwmutex* mtx)
{
	SRWLOCK* mutex = (SRWLOCK*)mtx;
	// apparently you can't destroy it? wow
	cflare_warn("readwrite mutex: unable to free on Windows platform.");
	// is it safe to free() it?
	//free(mutex);
}

void cflare_rwmutex_read_lock(cflare_rwmutex* mtx)
{
	SRWLOCK* mutex = (SRWLOCK*)mtx;
	AcquireSRWLockShared(mutex);
}

void cflare_rwmutex_read_unlock(cflare_rwmutex* mtx)
{
	SRWLOCK* mutex = (SRWLOCK*)mtx;
	ReleaseSRWLockShared(mutex);
}

void cflare_rwmutex_write_lock(cflare_rwmutex* mtx)
{
	SRWLOCK* mutex = (SRWLOCK*)mtx;
	AcquireSRWLockExclusive(mutex);
}

void cflare_rwmutex_write_unlock(cflare_rwmutex* mtx)
{
	SRWLOCK* mutex = (SRWLOCK*)mtx;
	ReleaseSRWLockExclusive(mutex);
}

