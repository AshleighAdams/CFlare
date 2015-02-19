
#include "cflare/httpstatus.h"
#include "cflare/hashtable.h"
#include "cflare/filesystem.h"

typedef struct known_status
{
	uint32_t code;
	char* string;
} known_status;

known_status* known_statuses = 0;

char** cache_code2string;
cflare_hashtable* cache_string2code;

void cflare_httpstatus_load()
{
	cache_string2code = cflare_hashtable_new();
	cflare_notimp();
}

void cflare_httpstatus_unload()
{
	cflare_hashtable_delete(cache_string2code);
	free(cache_code2string);
}

const char* cflare_httpstatus_tostring(uint32_t code)
{
	cflare_notimp();
	return 0;
}

uint32_t cflare_httpstatus_fromstring(const char* part)
{
	cflare_notimp();
	return 0;
}
