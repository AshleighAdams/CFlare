#include "cflare/mutex.h"

#include "cflare/util.h"

cflare_mutex* cflare_mutex_new(cflare_mutex_type type)
{
	cflare_warn("mutex: Windows mutexes are not implemented");
	return 0;
}

void cflare_mutex_delete(cflare_mutex* mtx)
{
	cflare_warn("mutex: Windows mutexes are not implemented");
}

void cflare_mutex_lock(cflare_mutex* mtx)
{
}

void cflare_mutex_unlock(cflare_mutex* mtx)
{
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

