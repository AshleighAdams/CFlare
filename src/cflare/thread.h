#ifndef CFLARE_THREAD_H
#define CFLARE_THREAD_H

#include <cflare/cflare.h>
#include <cflare/util.h>

typedef struct cflare_thread cflare_thread;
typedef void*(cflare_thread_entrypoint)(void* context);

CFLARE_API cflare_thread* cflare_thread_new(cflare_thread_entrypoint* function, void* context);
CFLARE_API void cflare_thread_delete(cflare_thread* thread);

CFLARE_API void* cflare_thread_join(cflare_thread* thread); // return value is the return value of the entrypoint
CFLARE_API size_t cflare_thread_id(cflare_thread* thread);

CFLARE_API void cflare_thread_sleep(double64_t seconds);

#endif /* CFLARE_THREAD_H */

