
#include "cflare/hashtable.h"

const size_t start_size = 32;

void free_container(void* data, void* hashtable)
{
	cflare_hashtable_container* cont = (cflare_hashtable_container*)data;
	cflare_hashtable* self = (cflare_hashtable*)hashtable;
	free(cont->key);
	
	if(self->deleter && cont->data)
		self->deleter(cont->data, self->deleter_context);
	free(cont->data);
}

cflare_hashtable* cflare_hashtable_new()
{
	cflare_hashtable* ret = malloc(sizeof(cflare_hashtable));
	
	ret->buckets_count = 0;
	ret->count = 0;
	ret->buckets = 0;
	ret->deleter = 0;
	ret->deleter_context = 0;
	
	// setup the RW mutex
	ret->mutex = cflare_rwmutex_new(CFLARE_MUTEX_PLAIN);
	return ret;
}

void cflare_hashtable_delete(cflare_hashtable* map)
{
	if(map->buckets)
	{
		for(size_t i = 0; i < map->buckets_count; i++)
		{
			cflare_hashtable_bucket* b = map->buckets + i;
			
			if(!b->list)
				continue;
			
			cflare_linkedlist_delete(b->list);
			cflare_rwmutex_delete(b->mutex);
		}
	}
	
	if(map->buckets)
	{
		free(map->buckets);
		map->buckets = 0;
	}
	
	free(map);
	
	cflare_rwmutex_delete(map->mutex);
}

void cflare_hashtable_ondelete(cflare_hashtable* map, cflare_deleter* func, void* context)
{
	map->deleter = func;
	map->deleter_context = context;
}

void cflare_hashtable_rebuild(cflare_hashtable* map, size_t count)
{
	cflare_rwmutex_write_lock(map->mutex);
	
	size_t size = sizeof(cflare_hashtable_bucket) * count;
	
	cflare_hashtable_bucket* old_buckets = map->buckets;
	size_t             old_buckets_count = map->buckets_count;
	
	map->buckets = malloc(size);
	memset(map->buckets, 0, size);
	map->buckets_count = count;
	
	// free the linked lists
	if(old_buckets)
	{
		for(size_t i = 0; i < old_buckets_count; i++)
		{
			cflare_hashtable_bucket* b = old_buckets + i;
			
			if(!b->list)
				continue;
			
			// disable the delete hook
			cflare_linkedlist_ondelete(b->list, 0, 0);
			
			// copy to the new one
			cflare_linkedlist_iter iter = cflare_linkedlist_iterator(b->list);
			while(cflare_linkedlist_iterator_next(&iter))
			{
				cflare_hashtable_container* cont = (cflare_hashtable_container*)iter.value->data;
				
				size_t pos = cont->hash % map->buckets_count;
				cflare_hashtable_bucket* nb = map->buckets + pos;
				
				if(!nb->list)
				{
					nb->list = cflare_linkedlist_new(sizeof(cflare_hashtable_container));
					assert(nb->list);
					nb->mutex = cflare_rwmutex_new(CFLARE_MUTEX_PLAIN);
		
					cflare_linkedlist_ondelete(nb->list, &free_container, map);
				}
				
				cflare_hashtable_container* newcont;
				cflare_linkedlist_insert_last(nb->list, (void**)&newcont);
				
				memcpy(newcont, cont, sizeof(cflare_hashtable_container));
				cflare_linkedlist_remove(b->list, iter.value);
			}
			
			cflare_linkedlist_delete(b->list);
			cflare_rwmutex_delete(b->mutex);
		}
		free(old_buckets);
	}
	
	cflare_rwmutex_write_unlock(map->mutex);
}

uint8_t memory_equals(size_t len, const void* a, const void* b)
{
	if(a == b)
		return 1;
	
	const uint8_t* A = (const uint8_t*)a;
	const uint8_t* B = (const uint8_t*)b;
	for(size_t i = 0; i < len; i++)
	{
		if(*A != *B)
			return 0;
		A++;
		B++;
	}
	return 1;
}

void cflare_hashtable_set(cflare_hashtable* map, cflare_hash hash, const void* value, size_t len)
{
	if(value)
	{
		//   32 buckets = resizes at   64 elements to  128 buckets
		//  128 buckets = resizes at  256 elements to  512 buckets
		//  512 buckets = resizes at 1024 elements to 2048 buckets
		// 2048 buckets = resizes at 4096 elements to 8192 buckets
		if(map->count + 1 > map->buckets_count * 2)
		{
			size_t newsize = map->buckets_count * 4;
			if(newsize < start_size)
				newsize = start_size;
		
			cflare_debug("automatically growing a hashtable to %lu buckets", newsize);
			cflare_hashtable_rebuild(map, newsize);
		}
	}
	else
	{
		// 8192 buckets = resizes at 1024 elements to 2048 buckets
		// 2048 buckets = resizes at  256 elements to  512 buckets
		//  512 buckets = resizes at   64 elements to  128 buckets
		//  128 buckets = resizes at   16 elements to   32 buckets
		if(map->count - 1 < map->buckets_count / 8)
		{
			size_t newsize = map->buckets_count / 4;
			if(newsize > start_size)
			{
				cflare_debug("automatically shrinking a hashtable to %lu buckets", newsize);
				cflare_hashtable_rebuild(map, newsize);
			}
		}
	}
	
	assert(map->buckets && map->buckets_count > 0);
	
	cflare_rwmutex_write_lock(map->mutex);
	
	uint32_t pos = hash.hash % map->buckets_count;
	cflare_hashtable_bucket* bucket = map->buckets + pos;
	
	// the bucket has not yet been setup
	if(!bucket->list)
	{
		// only create it if we're not attempting to remove the value
		if(value)
		{
			bucket->list = cflare_linkedlist_new(sizeof(cflare_hashtable_container));
			bucket->mutex = cflare_rwmutex_new(CFLARE_MUTEX_PLAIN);
		
			cflare_linkedlist_ondelete(bucket->list, &free_container, map);
		}
	}
	
	// lock the bucket for writing
	cflare_linkedlist* list = bucket->list;
	
	if(list)
	{
		cflare_rwmutex_write_lock(bucket->mutex);
		{
			cflare_linkedlist_iter iter = cflare_linkedlist_iterator(list);
			while(cflare_linkedlist_iterator_next(&iter))
			{
				cflare_hashtable_container* cont = (cflare_hashtable_container*)iter.value->data;
				if(hash.pointer_size != cont->key_size || hash.hash != cont->hash)
					continue;
			
				if(memory_equals(hash.pointer_size, hash.pointer, cont->key))
					break;
			}
		
			if(!value)
			{
				// we're removing it
				if(iter.value)
				{
					cflare_linkedlist_remove(list, iter.value);
					map->count -= 1;
				}
			}
			else
			{
				cflare_hashtable_container* container;
				if(iter.value)
				{
					container = (cflare_hashtable_container*)iter.value->data;
					free(container->data);
				}
				else
				{
					cflare_linkedlist_insert_last(list, (void**)&container);
					container->hash = hash.hash;
					container->key_size = hash.pointer_size;
					container->key = malloc(hash.pointer_size);
					memcpy(container->key, hash.pointer, hash.pointer_size);
			
					// increase the count too
					map->count += 1;
				}
		
				container->data_size = len;
				container->data = malloc(len);
				memcpy(container->data, value, len);
			}
		}
		cflare_rwmutex_write_unlock(bucket->mutex);
	}
	
	cflare_rwmutex_write_unlock(map->mutex);
}

uint8_t cflare_hashtable_get(cflare_hashtable* map, cflare_hash hash,
	void** out, size_t* len)
{
	uint8_t status = 0;
	cflare_rwmutex_read_lock(map->mutex);
	
	if(map->buckets_count > 0)
	{
		uint32_t pos = hash.hash % map->buckets_count;
		cflare_hashtable_bucket* bucket = map->buckets + pos;
		
		if(bucket->list && bucket->list->count > 0)
		{
			cflare_linkedlist* list = bucket->list;
			
			cflare_rwmutex_read_lock(bucket->mutex);
			
			cflare_linkedlist_iter iter = cflare_linkedlist_iterator(list);
			while(cflare_linkedlist_iterator_next(&iter))
			{
				cflare_hashtable_container* cont = (cflare_hashtable_container*)iter.value->data;
				if(hash.pointer_size != cont->key_size || hash.hash != cont->hash)
					continue;
				
				if(memory_equals(hash.pointer_size, hash.pointer, cont->key))
				{
					status = 1;
					*out = cont->data;
					*len = cont->data_size;
					break;
				}
			}
			
			cflare_rwmutex_read_unlock(bucket->mutex);
		}
	}
	
	cflare_rwmutex_read_unlock(map->mutex);
	return status;
}
