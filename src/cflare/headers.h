#ifndef CFLARE_HEADERS_H
#define CFLARE_HEADERS_H

#include <cflare/cflare.h>
#include <cflare/util.h>

typedef struct cflare_header
{
	int32_t id;
	const char* name;
} cflare_header;

CFLARE_API void cflare_headers_load();
CFLARE_API void cflare_headers_unload();

CFLARE_API cflare_header cflare_headers_get(const char* name);
CFLARE_API cflare_header cflare_headers_ensure(const char* name); // same as get, but will create it if not exists
CFLARE_API bool cflare_headers_valid(cflare_header header);
CFLARE_API bool cflare_headers_equals(cflare_header a, cflare_header b);


#endif /* CFLARE_HEADERS_H */

