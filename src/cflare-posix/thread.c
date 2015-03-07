
#include "cflare/thread.h"

#include "cflare/util.h"

cflare_thread* cflare_thread_new(cflare_thread_entrypoint* function, void* context)
{
	cflare_notimp();
}

void cflare_thread_delete(cflare_thread* thread)
{
	cflare_notimp();
}

void* cflare_thread_join(cflare_thread* thread)
{
	cflare_notimp();
	return 0;
}

size_t cflare_thread_id(cflare_thread* thread)
{
	cflare_notimp();
	return 0;
}

void cflare_thread_sleep(double64_t seconds)
{
	cflare_notimp();
}
