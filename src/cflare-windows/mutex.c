#include "cflare/mutex.h"

#include "cflare/util.h"
#include <Windows.h>

/*
Some code taken from the windows pthreads
wrapper, found at: http://locklessinc.com/articles/pthreads_on_windows/
*/

cflare_mutex* cflare_mutex_new(cflare_mutex_type type)
{
	return cflare_rwmutex_new(type);
}

void cflare_mutex_delete(cflare_mutex* mtx)
{
	cflare_rwmutex_delete(mtx);
}

void cflare_mutex_lock(cflare_mutex* mtx)
{
	cflare_rwmutex_write_lock(mtx);
}

void cflare_mutex_unlock(cflare_mutex* mtx)
{
	cflare_rwmutex_write_unlock(mtx);
}

// for when I add trylock
// return TryEnterCriticalSection(mutex) ? success : fail;

typedef struct lockinfo
{
	bool is_used;
	size_t position; // position inside the array
	SRWLOCK lock;
	SRWLOCK internallock; // this lock us used internally for recursiveness.
	HANDLE write_thread; // so we know when recursive writes can continue.
	size_t writelock_depth;
	size_t readlock_depth;
} lockinfo;

static lockinfo* infos = 0;
static size_t infos_len = 0;
static size_t infos_free = 0;
static size_t infos_curpos = 0;

static SRWLOCK infos_lock;

static void realloc_locks(size_t newsize)
{
	size_t oldsize = infos_len;
	size_t newcount = newsize - infos_len;

	infos = realloc(infos, sizeof(lockinfo) * newsize);
	infos_len = newsize;
	infos_free += newcount; // keep track of how many are free

	for (size_t i = oldsize; i < infos_len; i++)
	{
		// initialize the new ones
		lockinfo* info = infos + i;
		InitializeSRWLock(&info->lock);
		InitializeSRWLock(&info->internallock);
		info->is_used = false;
		info->position = i;
		info->write_thread = 0;
		info->writelock_depth = 0;
		info->readlock_depth = 0;
	}
}

static size_t alloc_lock()
{
	if (!infos)
	{
		infos = malloc(sizeof(lockinfo) * 1); // to get the OS to alloc atleast 1, will be realloc'ed right after
		infos_len = 0;
		infos_free = 0;
		InitializeSRWLock(&infos_lock);
		AcquireSRWLockExclusive(&infos_lock);
			realloc_locks(16);
		ReleaseSRWLockExclusive(&infos_lock);
	}

	AcquireSRWLockExclusive(&infos_lock);

	if (infos_free == 0)
		realloc_locks(infos_len * 2);

	lockinfo* info = 0;
	while(1)
	{
		info = infos + infos_curpos;
		if (!info->is_used)
			break;
		infos_curpos = (infos_curpos + 1) % infos_len;
	}
	info->is_used = true;
	infos_free -= 1;

	ReleaseSRWLockExclusive(&infos_lock);
	return info->position + 1; // so 0 is never used, and passes if checks
}

static lockinfo* get_info(cflare_rwmutex* mtx)
{
	size_t id = (size_t)mtx;
	if (id == 0)
		cflare_fatal("mutex: is null.");
	id -= 1; // 'cause we +1'ed on alloc
	if (id > infos_len)
		cflare_fatal("mutex: outside range of allocated locks.");
	return infos + id;
}

static void free_lock(size_t lock)
{
	AcquireSRWLockExclusive(&infos_lock);

	lockinfo* info = get_info((cflare_mutex*)lock);
	if (!info->is_used)
		cflare_fatal("mutex: freeing already free mutex.");

	info->is_used = 0;
	infos_free += 1;
	
	ReleaseSRWLockExclusive(&infos_lock);
}

cflare_rwmutex* cflare_rwmutex_new(cflare_mutex_type type)
{
	cflare_rwmutex* mutex = (cflare_rwmutex*)alloc_lock();
	cflare_debug("mutex: +1: %lu/%lu", infos_len - infos_free, infos_len);
	return mutex;
}

void cflare_rwmutex_delete(cflare_rwmutex* mtx)
{
	free_lock((size_t)mtx);
	cflare_debug("mutex: -1: %lu/%lu", infos_len - infos_free, infos_len);
}

void cflare_rwmutex_read_lock(cflare_rwmutex* mtx)
{
	lockinfo* mutex = get_info(mtx);
	AcquireSRWLockExclusive(&mutex->internallock);
	if (mutex->write_thread == GetCurrentThread()) // so READ locks don't block under the same WRITE lock
	{
		mutex->readlock_depth++;
		ReleaseSRWLockExclusive(&mutex->internallock);
		return;
	}
	ReleaseSRWLockExclusive(&mutex->internallock);

	AcquireSRWLockShared(&mutex->lock);
}

void cflare_rwmutex_read_unlock(cflare_rwmutex* mtx)
{
	lockinfo* mutex = get_info(mtx);
	AcquireSRWLockExclusive(&mutex->internallock);
	if (mutex->write_thread == GetCurrentThread())
	{
		mutex->readlock_depth--;
		ReleaseSRWLockExclusive(&mutex->internallock);
		return;
	}
	ReleaseSRWLockExclusive(&mutex->internallock);

	ReleaseSRWLockShared(&mutex->lock);
}

void cflare_rwmutex_write_lock(cflare_rwmutex* mtx)
{
	lockinfo* mutex = get_info(mtx);
	AcquireSRWLockExclusive(&mutex->internallock);
	if (mutex->write_thread == GetCurrentThread())
	{
		mutex->writelock_depth++;
		ReleaseSRWLockExclusive(&mutex->internallock);
		return;
	}
	ReleaseSRWLockExclusive(&mutex->internallock);

	AcquireSRWLockExclusive(&mutex->lock);
	AcquireSRWLockExclusive(&mutex->internallock);
	mutex->write_thread = GetCurrentThread();
	mutex->writelock_depth = 1;
	ReleaseSRWLockExclusive(&mutex->internallock);
}

void cflare_rwmutex_write_unlock(cflare_rwmutex* mtx)
{
	lockinfo* mutex = get_info(mtx);
	AcquireSRWLockExclusive(&mutex->internallock);
	if (mutex->write_thread == GetCurrentThread())
	{
		mutex->writelock_depth--;

		if (mutex->writelock_depth > 0)
		{
			ReleaseSRWLockExclusive(&mutex->internallock);
			return;
		}
	}
	
	mutex->write_thread = 0;
	ReleaseSRWLockExclusive(&mutex->lock);
	ReleaseSRWLockExclusive(&mutex->internallock);
}

