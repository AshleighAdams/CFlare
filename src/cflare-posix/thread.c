
#include "cflare/thread.h"

#include "cflare/util.h"

#include <pthread.h>
#include <time.h> // for sleep

typedef struct cflare_thread
{
	pthread_t native;
	bool owns; // should we call detatch?
} cflare_thread;

cflare_thread* cflare_thread_new(cflare_thread_entrypoint* function, void* context)
{
	cflare_thread* ret = malloc(sizeof(cflare_thread));
	ret->owns = true;
	
	if(pthread_create(&ret->native, 0, function, context))
		cflare_fatal("failed to create thread!");
	
	return ret;
}

void cflare_thread_delete(cflare_thread* thread)
{
	if(thread->owns)
	{
		cflare_warn("thread_delete(): thread not explicitly joined or detached; detaching implicitly...");
		cflare_thread_detach(thread);
	}
	
	free(thread);
}

void* cflare_thread_join(cflare_thread* thread)
{
	thread->owns = false;
	
	void* ret = 0;
	if(pthread_join(thread->native, &ret))
	{
		cflare_warn("failed to join %p", (void*)thread);
		return 0;
	}
	
	return ret;
}

void cflare_thread_detach(cflare_thread* thread)
{
	thread->owns = false;
	if(pthread_detach(thread->native))
	{
		cflare_warn("failed to detatch %p", (void*)thread);
	}
}

size_t cflare_thread_id(cflare_thread* thread)
{
	return (size_t)thread->native;
}

void cflare_thread_sleep(double64_t seconds)
{
	struct timespec t = {0, 0};
	t.tv_sec = (time_t)seconds;
	t.tv_nsec = (long)((t.tv_sec - seconds) * 1000.0 /*ms*/ * 1000.0 /*us*/ * 1000.0 /*ns*/);
	
	nanosleep(&t, 0);
}
