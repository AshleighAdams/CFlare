
#ifndef CFLARE_COROUTINE_H
#define CFLARE_COROUTINE_H

#include <lthread.h>

#include <cflare/cflare.h>
#include <cflare/util.h>

typedef struct cflare_coroutine cflare_coroutine;
typedef struct cflare_coroutinecondition cflare_coroutinecondition;
typedef void*(cflare_coroutine_entrypoint)(void* context);

CFLARE_API void cflare_coroutine_scheduler_run();

CFLARE_API cflare_coroutine* cflare_coroutine_new(cflare_coroutine_entrypoint* fn, void* context);
CFLARE_API void cflare_coroutine_delete(cflare_coroutine* co);


CFLARE_API void cflare_coroutine_yield();
CFLARE_API void cflare_coroutine_sleep(float64_t seconds);
CFLARE_API void* cflare_coroutine_join(cflare_coroutine* co);

// self-call
CFLARE_API void cflare_coroutine_detach();
CFLARE_API void cflare_coroutine_detach2(cflare_coroutine* co);
//CFLARE_API void cflare_coroutine_exit(void* returns); // use the return value
CFLARE_API void cflare_coroutine_wakeup(cflare_coroutine* co);


// these arn't bound, _current() uses them.
//CFLARE_API void cflare_coroutine_set_data(void* data);
//CFLARE_API void* cflare_coroutine_get_data();

CFLARE_API cflare_coroutine* cflare_coroutine_current();
CFLARE_API void cflare_coroutine_name(const char* name);
#define cflare_coroutine_autoname() cflare_coroutine_name(__func__)

CFLARE_API cflare_coroutinecondition* cflare_coroutinecondition_new();
CFLARE_API void cflare_coroutinecondition_delete(cflare_coroutinecondition* co);
CFLARE_API int cflare_coroutinecondition_wait(cflare_coroutinecondition* co, float64_t timeout);
CFLARE_API void cflare_coroutinecondition_signal(cflare_coroutinecondition* co); // wakeup one
CFLARE_API void cflare_coroutinecondition_broadcast(cflare_coroutinecondition* co); // wakeup all

CFLARE_API int cflare_coroutine_compute_begin();
CFLARE_API void cflare_coroutine_compute_end();

#endif
