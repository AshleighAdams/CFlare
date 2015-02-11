#ifndef CFLARE_MUTEX_H
#define CFLARE_MUTEX_H

#include <cflare/cflare.h>

typedef void* cflare_mutex;
typedef void* cflare_rwmutex;

typedef enum
{
	CFLARE_MUTEX_PLAIN      = 1 << 0,
	CFLARE_MUTEX_RECURSIVE  = 1 << 1,
	CFLARE_MUTEX_TIMED      = 1 << 2,
	CFLARE_MUTEX_TRY        = 1 << 3
} cflare_mutex_type;

CFLARE_API cflare_mutex* cflare_mutex_new(cflare_mutex_type type);
CFLARE_API void cflare_mutex_delete(cflare_mutex* mtx);

CFLARE_API void cflare_mutex_lock(cflare_mutex* mtx);
CFLARE_API void cflare_mutex_unlock(cflare_mutex* mtx);

// RW mutex, writes block reads, many reads don't block themselves

CFLARE_API cflare_rwmutex* cflare_rwmutex_new(cflare_mutex_type type);
CFLARE_API void cflare_rwmutex_delete(cflare_rwmutex* mtx);

CFLARE_API void cflare_rwmutex_read_lock(cflare_rwmutex* mtx);
CFLARE_API void cflare_rwmutex_read_unlock(cflare_rwmutex* mtx);

CFLARE_API void cflare_rwmutex_write_lock(cflare_rwmutex* mtx);
CFLARE_API void cflare_rwmutex_write_unlock(cflare_rwmutex* mtx);

#endif /* CFLARE_MUTEX_H */

