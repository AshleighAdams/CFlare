// TODO MUTEXES IMPORTANT

#include "cflare/handle.h"
#include "cflare/util.h"

#include "threads.h"

struct reference
{
	char type[32];
	cflare_handle id;
	void* data;
	size_t count;
	cflare_handle_deleter deleter;
};

#define MAX_REFS 255
struct reference refs[MAX_REFS];
cflare_handle curid = 0;
int loaded = 0;
mtx_t mutex;

int32_t find_handle(cflare_handle id, struct reference** out)
{
	for(size_t i = 0; i < MAX_REFS; i++)
	{
		struct reference* ref = refs + i;
		
		if(ref->id == id)
		{
			*out = ref;
			return 1;
		}
	}
	
	return 0;
}

cflare_handle new_handle(struct reference** out)
{
	*out = 0;
	
	for(size_t i = 0; i < MAX_REFS; i++)
	{
		struct reference* ref = refs + i;
		if(!ref->id)
		{
			curid += 1;
			assert(curid); // make sure it hasn't rolled over...
			ref->id = curid;
			*out = ref;
			return curid;
		}
	}
	return 0;
}

void cflare_handle_load()
{
	assert(!loaded);
	for(size_t i = 0; i < MAX_REFS; i++)
	{
		struct reference* ref = refs + i;
		ref->type[0] = '\0';
		ref->id = 0;
		ref->data = 0;
		ref->count = 0;
		ref->deleter = 0;
	}
	
	if(mtx_init(&mutex, mtx_plain) != thrd_success)
		cflare_fatal("handle: failed to create mutex");
	
	loaded = 1;
}

void cflare_handle_unload()
{
	assert(loaded);
	for(size_t i = 0; i < MAX_REFS; i++)
	{
		struct reference* ref = refs + i;
		if(ref->id)
		{
			cflare_warn("handle: %s (hd %lu) still has %lu references!", ref->type, ref->id, ref->count);
		}
	}
	
	mtx_destroy(&mutex);
	
	loaded = 0;
}

cflare_handle cflare_handle_new(const char* type, void* data, cflare_handle_deleter deleter)
{
	assert(loaded);
	struct reference* ref;
	
	mtx_lock(&mutex);
	{
		if(!new_handle(&ref))
		{
			mtx_unlock(&mutex);
			return 0;
		}
	
		strcpy(ref->type, type);
		ref->data = data;
		ref->deleter = deleter;
		ref->count = 1;
	
		cflare_debug("handle: created %lu (%s)", ref->id, ref->type);
	}
	mtx_unlock(&mutex);
	
	return ref->id;
}

void cflare_handle_reference(cflare_handle hd)
{
	assert(hd);
	assert(loaded);
	
	struct reference* ref;
	
	mtx_lock(&mutex);
	{
		if(!find_handle(hd, &ref))
		{
			mtx_unlock(&mutex);
			cflare_fatal("handle: invalid handle: %lu", hd);
		}
	
		ref->count++;
	}
	mtx_unlock(&mutex);
}

void cflare_handle_unreference(cflare_handle hd)
{
	assert(hd);
	assert(loaded);
	
	struct reference* ref;
	
	mtx_lock(&mutex);
	{
		if(!find_handle(hd, &ref))
		{
			mtx_unlock(&mutex);
			cflare_fatal("handle: invalid handle: %lu", hd);
		}
	
		ref->count--;
	
		if(ref->count == 0)
		{
			cflare_log("handle: destroyed %lu (%s)", ref->id, ref->type);
			ref->id = 0;
			if(ref->deleter)
				ref->deleter(ref->data);
		}
	}
	mtx_unlock(&mutex);
}

void* cflare_handle_data(cflare_handle hd)
{
	assert(hd);
	assert(loaded);
	
	struct reference* ref;
	
	mtx_lock(&mutex);
	if(!find_handle(hd, &ref))
	{
		mtx_unlock(&mutex);
		cflare_fatal("handle: invalid handle: %lu", hd);
	}
	mtx_unlock(&mutex);
	
	return ref->data;
}

