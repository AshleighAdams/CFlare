
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
	
	char* path =  cflare_string_concat(cflare_cfgpath(), "http/statuses");
	cflare_linkedlist* list = cflare_filesystem_list(path, 0);
	
	cflare_linkedlist_iter iter = cflare_linkedlist_iterator(list);
	while(cflare_linkedlist_iterator_next(&iter))
	{
		cflare_filesystem_entry* ent = (cflare_filesystem_entry*)iter.value->data;
		cflare_debug("httpstatus: not loading %s (not imp)", ent->name);
	}
	
	cflare_linkedlist_delete(list);
	free(path);
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
