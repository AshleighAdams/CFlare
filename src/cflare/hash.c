
#include "cflare/hash.h"

#include "cflare/options.h"

static uint32_t murmur3_32(const void* buff, uint32_t len)
{
	const char* key = (const char*)buff;
	uint32_t seed = 0;
	
	static const uint32_t c1 = 0xcc9e2d51;
	static const uint32_t c2 = 0x1b873593;
	static const uint32_t r1 = 15;
	static const uint32_t r2 = 13;
	static const uint32_t m = 5;
	static const uint32_t n = 0xe6546b64;
 
	uint32_t hash = seed;
 
	const int nblocks = len / 4;
	const uint32_t *blocks = (const uint32_t *) key;
	int i;
	for (i = 0; i < nblocks; i++)
	{
		uint32_t k = blocks[i];
		k *= c1;
		k = (k << r1) | (k >> (32 - r1));
		k *= c2;
 
		hash ^= k;
		hash = ((hash << r2) | (hash >> (32 - r2))) * m + n;
	}
 
	const uint8_t *tail = (const uint8_t *) (key + nblocks * 4);
	uint32_t k1 = 0;
 
	switch (len & 3) {
	case 3:
		k1 ^= tail[2] << 16;
	case 2:
		k1 ^= tail[1] << 8;
	case 1:
		k1 ^= tail[0];
 
		k1 *= c1;
		k1 = (k1 << r1) | (k1 >> (32 - r1));
		k1 *= c2;
		hash ^= k1;
	}
 
	hash ^= len;
	hash ^= (hash >> 16);
	hash *= 0x85ebca6b;
	hash ^= (hash >> 13);
	hash *= 0xc2b2ae35;
	hash ^= (hash >> 16);
 
	return hash;
}

static uint32_t lua_hash(const void* buff, uint32_t len)
{
	#define HASHLIMIT 5

	unsigned int seed = 0;
	char* str = (char*)buff;
	unsigned int computed = seed ^ (unsigned int)len;
	size_t l1;
	size_t step = (len >> HASHLIMIT) + 1;
	for (l1 = len; l1 >= step; l1 -= step)
		computed = computed ^ ((computed<<5) + (computed>>2) + (unsigned int)(str[l1 - 1]));
	
	return (uint32_t)computed;
}

static uint32_t orig(const void* data, size_t len)
{
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

	return computed;
}

#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
  || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get16bits(d) (*((const uint16_t *) (d)))
#endif

#if !defined (get16bits)
#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
                       +(uint32_t)(((const uint8_t *)(d))[0]) )
#endif

// http://www.azillionmonkeys.com/qed/hash.html
static uint32_t SuperFastHash(const void* arg_data, size_t arg_len)
{
	const char * data = (const char*)arg_data;;
	int len = (int)arg_len;
	
	uint32_t hash = len, tmp;
	int rem;
	
    rem = len & 3;
    len >>= 2;

    /* Main loop */
    for (;len > 0; len--)
    {
        hash  += get16bits (data);
        tmp    = (get16bits (data+2) << 11) ^ hash;
        hash   = (hash << 16) ^ tmp;
        data  += 2*sizeof (uint16_t);
        hash  += hash >> 11;
    }

    /* Handle end cases */
    switch (rem)
    {
        case 3: hash += get16bits (data);
                hash ^= hash << 16;
                hash ^= ((signed char)data[sizeof (uint16_t)]) << 18;
                hash += hash >> 11;
                break;
        case 2: hash += get16bits (data);
                hash ^= hash << 11;
                hash += hash >> 17;
                break;
        case 1: hash += (signed char)*data;
                hash ^= hash << 10;
                hash += hash >> 1;
    }

    /* Force "avalanching" of final 127 bits */
    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash += hash >> 17;
    hash ^= hash << 25;
    hash += hash >> 6;

    return hash;
}

static uint32_t fnv_hash_1a_32(const void *key, size_t len)
{
	const uint8_t *p = key;
	uint32_t h = 0x811c9dc5;
	int i;

	for ( i = 0; i < len; i++ )
		h = ( h ^ p[i] ) * 0x01000193;

	return h;
}

/* // this needs to be linked to libssl, it's slow, used as just a test
#include <openssl/sha.h>
uint32_t sha1(const void *key, size_t len)
{
	const unsigned char* str = (const unsigned char*)key;
	unsigned char hash[SHA_DIGEST_LENGTH];
	
	SHA1(str, len, hash);
	
	return *hash;
}
*/

cflare_hash cflare_hash_compute(const void* data, size_t len)
{
	cflare_hash ret;
	ret.pointer = data;
	ret.pointer_size = len;
	
	// the graphs are in the format:
	//       |
	// total |
	//       |______________________
	//              elements/bucket
	// the more compressed to the left, the better the hashing function.
	// when building these graphs, make sure the number of buckets = the number of elements
	
	// number: distro: █▇▄▁_____; percentile (|99th): ▃▆▇██|███
	// string: distro: █▇▃▂_____; percentile (|99th): ▃▆▇|█████
	//ret.hash = murmur3_32(data, len);
	
	// number: distro: ▄█▄______; percentile (|99th): ▂▆|██████
	// string: distro: ██▃▂▁____; percentile (|99th): ▃▆▇█|████
	//ret.hash = lua_hash(data, len); // basically fnv_hash_1a_32
	
	// number: distro: ██▄▂_____; percentile (|99th): ▃▆▇|█████
	// string: distro: ▆█▄▁_____; percentile (|99th): ▃▆██|████
	//ret.hash = SuperFastHash(data, len);
	
	// number: distro: ▂█▂______; percentile (|99th): ▂▇|██████
	// string: distro: █▇▄▁_____; percentile (|99th): ▃▆▇█|████
	ret.hash = fnv_hash_1a_32(data, len);
	
	// number: distro: ▇█▄▁_____; percentile (|99th): ▃▆▇█|████
	// string: distro: █▇▅▁_____; percentile (|99th): ▃▆▇|█████
	//ret.hash = orig(data, len);
	
	// number: distro: █▇▄▁▁____; percentile (|99th): ▃▆▇█|████
	// string: distro: █▆▃▁▁____; percentile (|99th): ▃▆▇█|████
	//ret.hash = sha1(data, len);
	
	// example of a perfect hash:  █________  |████████
	
	return ret;
}

