#ifndef CFLARE_HASHMAP_H
#define CFLARE_HASHMAP_H

#include <cflare/cflare.h>
#include <cflare/linkedlist.h>
#include <cflare/hash.h>
#include <cflare/mutex.h>

typedef struct cflare_hashtable cflare_hashtable;

// TODO: maybe add _key_ondelete and _key_setcompare

CFLARE_API cflare_hashtable* cflare_hashtable_new();
CFLARE_API void cflare_hashtable_delete(cflare_hashtable* map);

CFLARE_API void cflare_hashtable_ondelete(cflare_hashtable* list, cflare_deleter* func, void* context);

CFLARE_API void cflare_hashtable_rebuild(cflare_hashtable* map, size_t count);

CFLARE_API void cflare_hashtable_set(cflare_hashtable* map, cflare_hash hash, const void* value, size_t len);
CFLARE_API bool cflare_hashtable_get(const cflare_hashtable* map, cflare_hash hash, void** out, size_t* len);

CFLARE_API void cflare_hashtable_printdebug(const cflare_hashtable* map);
CFLARE_API size_t cflare_hashtable_count(const cflare_hashtable* map);
CFLARE_API size_t cflare_hashtable_bucketscount(const cflare_hashtable* map);

#endif /* CFLARE_HASHMAP_H */

