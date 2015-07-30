#include "cflare/coroutine.h"

typedef struct cflare_coroutine
{
	bool running, joined, detached;
	lthread_t* lt;
	cflare_coroutine_entrypoint* entry_point;
	void* entry_context;
	void* return_data;
} cflare_coroutine;

typedef struct cflare_coroutinecondition
{
	lthread_cond_t* c;
} cflare_coroutinecondition;

static void enter_entrypoint(void* ctx)
{
	cflare_coroutine* co = ctx;
	//lthread_t* lt = co->lt;
	
	lthread_set_data(co);
	co->running = true;
		co->return_data = co->entry_point(co->entry_context);
	co->running = false;
	
	if(co->detached || co->joined)
		free(co);
	//lthread_exit(return_data);
}

void cflare_coroutine_scheduler_run()
{
	lthread_run();
}

cflare_coroutine* cflare_coroutine_new(cflare_coroutine_entrypoint* fn, void* context)
{
	cflare_coroutine* ret = malloc(sizeof(cflare_coroutine));
	
	ret->entry_point = fn;
	ret->entry_context = context;
	ret->running = false;
	ret->joined = false;
	ret->detached = false;
	ret->return_data = 0x0;
	
	lthread_create(&ret->lt, enter_entrypoint, ret);
	
	return ret;
}

void cflare_coroutine_halt(cflare_coroutine* co)
{
	lthread_cancel(co->lt);
	free(co); // mem leak p.much always here...
}

void cflare_coroutine_yield(cflare_coroutine* co)
{
	lthread_sleep(0);
}

void cflare_coroutine_sleep(float64_t seconds)
{
	lthread_sleep(seconds * 1000);
}

void* cflare_coroutine_join(cflare_coroutine* co)
{
	void* ret;
	
	int reason;
	do
	{
		reason = lthread_join(co->lt, &ret, 1000);
	} while(reason == -2);
	
	return ret;
}

void cflare_coroutine_detach()
{
	cflare_coroutine* self = cflare_coroutine_current();
	self->detached = true;
	lthread_detach();
}

void cflare_coroutine_detach2(cflare_coroutine* co)
{
	co->detached = true;
	lthread_detach2(co->lt);
}


void cflare_coroutine_wakeup(cflare_coroutine* co)
{
	lthread_wakeup(co->lt);
}


cflare_coroutine* cflare_coroutine_current()
{
	return lthread_get_data();
}

void cflare_coroutine_name(const char* name)
{
	lthread_set_funcname(name);
}


cflare_coroutinecondition* cflare_coroutinecondition_new()
{
	cflare_coroutinecondition* ret = malloc(sizeof(cflare_coroutinecondition));
	
	int r = lthread_cond_create(&ret->c);
	assert(r == 0);
	
	return ret;
}

void cflare_coroutinecondition_delete(cflare_coroutinecondition* co)
{
	free(co->c);
	free(co);
}

int cflare_coroutinecondition_wait(cflare_coroutinecondition* co, float64_t timeout)
{
	uint64_t to;
	if(timeout == 0.0)
		to = 1;
	else if(timeout < 0)
		to = 0;
	else
		to = timeout * 1000;
	
	return lthread_cond_wait(co->c, to); // returns -2 = timeout, 0 = signal
}

void cflare_coroutinecondition_signal(cflare_coroutinecondition* co)
{
	lthread_cond_signal(co->c);
}

void cflare_coroutinecondition_broadcast(cflare_coroutinecondition* co)
{
	lthread_cond_broadcast(co->c);
}


int cflare_coroutine_compute_begin()
{
	return lthread_compute_begin();
}

void cflare_coroutine_compute_end()
{
	lthread_compute_end();
}

