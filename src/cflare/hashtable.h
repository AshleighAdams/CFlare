#ifndef CFLARE_HASHMAP_H
#define CFLARE_HASHMAP_H

#include <cflare/cflare.h>
#include <cflare/linkedlist.h>
#include <cflare/hash.h>
#include <cflare/mutex.h>

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
	cflare_rwmutex* mutex;
} cflare_hashtable_bucket;

typedef struct cflare_hashtable
{
	cflare_hashtable_bucket* buckets;
	size_t buckets_count;
	size_t count;
	cflare_rwmutex* mutex;
	cflare_deleter* deleter;
	void* deleter_context;
} cflare_hashtable;

// TODO: maybe add _key_ondelete and _key_setcompare

CFLARE_API cflare_hashtable* cflare_hashtable_new();
CFLARE_API void cflare_hashtable_delete(cflare_hashtable* map);

CFLARE_API void cflare_hashtable_ondelete(cflare_hashtable* list, cflare_deleter* func, void* context);

CFLARE_API void cflare_hashtable_rebuild(cflare_hashtable* map, size_t count);

CFLARE_API void cflare_hashtable_set(cflare_hashtable* map, cflare_hash hash, const void* value, size_t len);
CFLARE_API bool cflare_hashtable_get(cflare_hashtable* map, cflare_hash hash, void** out, size_t* len);

#endif /* CFLARE_HASHMAP_H */

