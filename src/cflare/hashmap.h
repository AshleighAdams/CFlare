#ifndef CFLARE_HASHMAP_H
#define CFLARE_HASHMAP_H

#include "cflare/linkedlist2.h"
#include "cflare/hash.h"

#include <pthread.h>

typedef struct cflare_hashmap_container
{
	uint32_t hash;
	void* key;
	size_t key_size;
	void* data;
	size_t data_size;
} cflare_hashmap_container;

typedef struct cflare_hashmap_bucket
{
	cflare_linkedlist* list;
	pthread_rwlock_t mutex;
} cflare_hashmap_bucket;

typedef struct cflare_hashmap
{
	cflare_hashmap_bucket* buckets;
	size_t buckets_count;
	pthread_rwlock_t mutex;
} cflare_hashmap;


cflare_hashmap* cflare_hashmap_new();
void cflare_hashmap_delete(cflare_hashmap* map);

void cflare_hashmap_rebuild(cflare_hashmap* map, size_t count);

void cflare_hashmap_set(cflare_hashmap* map, cflare_hash hash, const void* value, size_t len);
uint8_t cflare_hashmap_get(cflare_hashmap* map, cflare_hash hash, void** out, size_t* len);

#endif /* CFLARE_HASHMAP_H */

