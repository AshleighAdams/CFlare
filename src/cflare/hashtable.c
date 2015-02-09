
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
	ret->buckets = 0;
	ret->deleter = 0;
	ret->deleter_context = 0;
	
	// setup the RW mutex
	pthread_rwlockattr_t attr;
	pthread_rwlockattr_init(&attr);
	assert(pthread_rwlock_init(&ret->mutex, &attr) == 0);
	pthread_rwlockattr_destroy(&attr);
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
			pthread_rwlock_destroy(&b->mutex);
		}
	}
	
	if(map->buckets)
	{
		free(map->buckets);
		map->buckets = 0;
	}
	
	free(map);
	
	pthread_rwlock_destroy(&map->mutex);
}

void cflare_hashtable_ondelete(cflare_hashtable* map, cflare_hashtable_deleter* func, void* context)
{
	map->deleter = func;
	map->deleter_context = context;
}

void cflare_hashtable_rebuild(cflare_hashtable* map, size_t count)
{
	pthread_rwlock_wrlock(&map->mutex);
	
	size_t size = sizeof(cflare_hashtable_bucket) * count;
	
	cflare_hashtable_bucket* old_buckets = map->buckets;
	size_t           old_buckets_count = map->buckets_count;
	
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
			
			// copy to the new one
			cflare_linkedlist_iter iter = cflare_linkedlist_iterator(b->list);
			while(cflare_linkedlist_iterator_next(&iter))
			{
				cflare_hashtable_container* cont = (cflare_hashtable_container*)iter.value->data;
				cflare_hash hash;
				hash.hash = cont->hash;
				hash.pointer = cont->key;
				hash.pointer_size = cont->key_size;
				
				cflare_hashtable_set(map, hash, cont->data, cont->data_size);
				
				// this data is being moved, not removed, so make sure the deleter isn't called.
				free(cont->data);
				cont->data = 0;
			}
			
			cflare_linkedlist_delete(b->list);
			pthread_rwlock_destroy(&b->mutex);
		}
		free(old_buckets);
	}
	
	pthread_rwlock_unlock(&map->mutex);
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
	if(!map->buckets)
		cflare_hashtable_rebuild(map, start_size);
	
	pthread_rwlock_wrlock(&map->mutex);
	
	assert(map->buckets && map->buckets_count > 0);
	
	uint32_t pos = hash.hash % map->buckets_count;
	cflare_hashtable_bucket* bucket = map->buckets + pos;
	
	// the bucket has not yet been setup
	if(!bucket->list)
	{
		bucket->list = cflare_linkedlist_new(sizeof(cflare_hashtable_container));
		pthread_rwlockattr_t attr;
		pthread_rwlockattr_init(&attr);
		assert(pthread_rwlock_init(&bucket->mutex, &attr) == 0);
		pthread_rwlockattr_destroy(&attr);
		
		cflare_linkedlist_ondelete(bucket->list, &free_container, map);
	}
	
	// lock the bucket for writing
	cflare_linkedlist* list = bucket->list;
	pthread_rwlock_wrlock(&bucket->mutex);
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
		
		cflare_hashtable_container* container;
		if(iter.value)
			free(container->data);
		else
		{
			cflare_linkedlist_insert_last(list, (void**)&container);
			container->hash = hash.hash;
			container->key_size = hash.pointer_size;
			container->key = malloc(hash.pointer_size);
			memcpy(container->key, hash.pointer, hash.pointer_size);
		}
		
		container->data_size = len;
		container->data = malloc(len);
		memcpy(container->data, value, len);
	}
	pthread_rwlock_unlock(&bucket->mutex);
	
	pthread_rwlock_unlock(&map->mutex);
}

uint8_t cflare_hashtable_get(cflare_hashtable* map, cflare_hash hash,
	void** out, size_t* len)
{
	uint8_t status = 0;
	pthread_rwlock_rdlock(&map->mutex);
	
	if(map->buckets_count > 0)
	{
		uint32_t pos = hash.hash % map->buckets_count;
		cflare_hashtable_bucket* bucket = map->buckets + pos;
		
		if(bucket->list && bucket->list->count > 0)
		{
			cflare_linkedlist* list = bucket->list;
			
			pthread_rwlock_rdlock(&bucket->mutex);
			
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
			
			pthread_rwlock_unlock(&bucket->mutex);
		}
	}
	
	pthread_rwlock_unlock(&map->mutex);
	return status;
}
