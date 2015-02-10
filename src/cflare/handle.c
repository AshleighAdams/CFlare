#include "cflare/handle.h"
#include "cflare/util.h"
#include "cflare/mutex.h"

struct reference
{
	char type[32];
	cflare_handle id;
	void* data;
	size_t count;
	cflare_deleter* deleter;
	void* deleter_context;
};

#define MAX_REFS 255
struct reference refs[MAX_REFS];
cflare_handle curid = 0;
int loaded = 0;
cflare_mutex* mutex;

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
		ref->deleter_context = 0;
	}
	
	mutex = cflare_mutex_new(CFLARE_MUTEX_PLAIN);
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
			cflare_warn("handle: %s (hd %lu) still has %lu references!",
				ref->type, ref->id, ref->count);
		}
	}
	
	cflare_mutex_delete(mutex);
	loaded = 0;
}

cflare_handle cflare_handle_new(const char* type, void* data,
	cflare_deleter* deleter, void* context)
{
	assert(loaded);
	struct reference* ref;
	
	cflare_mutex_lock(mutex);
	{
		if(!new_handle(&ref))
		{
			cflare_mutex_unlock(mutex);
			return 0;
		}
	
		strcpy(ref->type, type);
		ref->data = data;
		ref->deleter = deleter;
		ref->deleter_context = context;
		ref->count = 1;
	
		cflare_debug("handle: created %lu (%s)", ref->id, ref->type);
	}
	cflare_mutex_unlock(mutex);
	
	return ref->id;
}

void cflare_handle_reference(cflare_handle hd)
{
	assert(hd);
	assert(loaded);
	
	struct reference* ref;
	
	cflare_mutex_lock(mutex);
	{
		if(!find_handle(hd, &ref))
		{
			cflare_mutex_unlock(mutex);
			cflare_fatal("handle: invalid handle: %lu", hd);
		}
	
		ref->count++;
	}
	cflare_mutex_unlock(mutex);
}

void cflare_handle_unreference(cflare_handle hd)
{
	assert(hd);
	assert(loaded);
	
	struct reference* ref;
	
	cflare_mutex_lock(mutex);
	{
		if(!find_handle(hd, &ref))
		{
			cflare_mutex_unlock(mutex);
			cflare_fatal("handle: invalid handle: %lu", hd);
		}
	
		ref->count--;
	
		if(ref->count == 0)
		{
			cflare_log("handle: destroyed %lu (%s)", ref->id, ref->type);
			ref->id = 0;
			if(ref->deleter)
				ref->deleter(ref->data, ref->deleter_context);
		}
	}
	cflare_mutex_unlock(mutex);
}

void* cflare_handle_data(cflare_handle hd)
{
	assert(hd);
	assert(loaded);
	
	struct reference* ref;
	
	cflare_mutex_lock(mutex);
	if(!find_handle(hd, &ref))
	{
		cflare_mutex_unlock(mutex);
		cflare_fatal("handle: invalid handle: %lu", hd);
	}
	cflare_mutex_unlock(mutex);
	
	return ref->data;
}

