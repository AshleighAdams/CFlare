#ifndef CFLARE_HASHMAP_H
#define CFLARE_HASHMAP_H

#include "cflare/linkedlist.h"
#include "cflare/hash.h"

#include <pthread.h>

typedef struct cflare_hashtable_container
{
	uint32_t hash;
	void* key;
	size_t key_size;
	void* data;
	size_t data_size;
} cflare_hashtable_container;

typedef struct cflare_hashtable_bucket
{
	cflare_linkedlist* list;
	pthread_rwlock_t mutex;
} cflare_hashtable_bucket;

typedef struct cflare_hashtable
{
	cflare_hashtable_bucket* buckets;
	size_t buckets_count;
	pthread_rwlock_t mutex;
	cflare_deleter* deleter;
	void* deleter_context;
} cflare_hashtable;

// TODO: maybe add _key_ondelete and _key_setcompare

cflare_hashtable* cflare_hashtable_new();
void cflare_hashtable_delete(cflare_hashtable* map);

void cflare_hashtable_ondelete(cflare_hashtable* list, cflare_deleter* func, void* context);

void cflare_hashtable_rebuild(cflare_hashtable* map, size_t count);

void cflare_hashtable_set(cflare_hashtable* map, cflare_hash hash, const void* value, size_t len);
uint8_t cflare_hashtable_get(cflare_hashtable* map, cflare_hash hash, void** out, size_t* len);

#endif /* CFLARE_HASHMAP_H */

