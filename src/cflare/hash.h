#ifndef CFLARE_HASH_H
#define CFLARE_HASH_H

#include "cflare/util.h"

typedef struct cflare_hash
{
	const void* pointer;
	size_t pointer_size;
	uint32_t hash;
} cflare_hash;

// be aware, this doesn't allocate any memory/nor does it take ownership of it
cflare_hash cflare_hash_compute(const void* data, size_t len);

#endif /* CFLARE_HASH_H */
