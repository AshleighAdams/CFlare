#ifndef CFLARE_MUTEX_H
#define CFLARE_MUTEX_H

#include <cflare/cflare.h>

typedef struct cflare_mutex cflare_mutex;
typedef struct cflare_rwmutex cflare_rwmutex;
typedef struct cflare_condition cflare_condition;

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

// Conditionals

CFLARE_API cflare_condition* cflare_condition_new();
CFLARE_API void cflare_condition_delete(cflare_condition* cond);
CFLARE_API void cflare_condition_wait(cflare_condition* cond, cflare_mutex* mtx); // put to sleep
CFLARE_API void cflare_condition_signal(cflare_condition* cond, cflare_mutex* mtx); // wake up

#endif /* CFLARE_MUTEX_H */

