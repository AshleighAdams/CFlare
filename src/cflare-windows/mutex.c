#include "cflare/mutex.h"

#include "cflare/util.h"
#include <Windows.h>

cflare_mutex* cflare_mutex_new(cflare_mutex_type type)
{
	//cflare_warn("mutex: Windows mutexes are not implemented");
	CRITICAL_SECTION* mtx = malloc(sizeof(CRITICAL_SECTION));
	InitializeCriticalSection(mtx);
	return (cflare_mutex*)mtx;
}

void cflare_mutex_delete(cflare_mutex* mtx)
{
	//cflare_warn("mutex: Windows mutexes are not implemented");
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

cflare_rwmutex* cflare_rwmutex_new(cflare_mutex_type type)
{
	cflare_warn("mutex: Windows readwrite mutexes are not implemented");
	return 0;
}

void cflare_rwmutex_delete(cflare_rwmutex* mtx)
{
	cflare_warn("mutex: Windows readwrite mutexes are not implemented");
}

void cflare_rwmutex_read_lock(cflare_rwmutex* mtx)
{
}

void cflare_rwmutex_read_unlock(cflare_rwmutex* mtx)
{
}

void cflare_rwmutex_write_lock(cflare_rwmutex* mtx)
{
}

void cflare_rwmutex_write_unlock(cflare_rwmutex* mtx)
{
}

