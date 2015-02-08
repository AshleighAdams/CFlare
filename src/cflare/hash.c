
#include "cflare/hash.h"

cflare_hash cflare_hash_compute(const void* data, size_t len)
{
	cflare_hash ret;
	ret.pointer = data;
	ret.pointer_size = len;
	ret.hash = 0;

	const char* ptr = (const char*)data;
	uint32_t computed, i;
	for(computed = i = 0; i < len; i++)
	{
		computed += ptr[i];
		computed += (computed << 10);
		computed ^= (computed >> 6);
	}
	computed += (computed << 3);
	computed ^= (computed >> 11);
	computed += (computed << 15);

	ret.hash = computed;
	return ret;
}
