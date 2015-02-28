#ifndef CFLARE_HEADERS_H
#define CFLARE_HEADERS_H

#include <cflare/cflare.h>
#include <cflare/util.h>

typedef struct cflare_header
{
	// if this is < 0, then it's a non-standard header; == 0, then completley unknown
	//the ids are not persistant, and not preserved across CFlare instances.
	int32_t id;
	const char* name;
} cflare_header;

CFLARE_API void cflare_headers_load();
CFLARE_API void cflare_headers_unload();

CFLARE_API cflare_header cflare_headers_get(const char* buffer);
CFLARE_API bool cflare_headers_equals(cflare_header a, cflare_header b);

#endif /* CFLARE_HEADERS_H */

