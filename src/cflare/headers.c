
#include "cflare/cflare.h"
#include "cflare/util.h"
#include "cflare/headers.h"
#include "cflare/hashtable.h"
#include "cflare/linkedlist.h"
#include "cflare/filesystem.h"
#include "cflare/mutex.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <assert.h>

#define max_line_length 1024
#define max_header_length 64
static cflare_hashtable* canonical_headers = 0;
static cflare_mutex* gid_mutex = 0;
size_t gid = 1;

typedef struct loaded_header
{
	int32_t id;
	char name[max_header_length + 1];
	float64_t last_used;
	bool on_disk;
} loaded_header;

static void normalize_header(char* ptr, size_t length)
{
	for(size_t i = 0; i < length + 1; i++) // + 1 to include the null ptr
	{
		if(ptr[i] == '\0')
			return;
		ptr[i] = tolower((unsigned char)ptr[i]);
	}
}

static void free_header(void* data, void* context)
{
	loaded_header** ptr = (loaded_header**)data;
	free(*ptr);
}

static void create_header(char name[max_header_length + 1], loaded_header** output)
{
	size_t len = strlen(name);
	assert(len <= max_header_length);
	
	loaded_header* ptr = malloc(sizeof(loaded_header));
	ptr->last_used = 0;
	ptr->on_disk = false;
	/* ptr->name = name; */ memcpy(ptr->name, name, len + 1);
	
	cflare_mutex_lock(gid_mutex);
		ptr->id = gid;
		gid += 1;
	cflare_mutex_unlock(gid_mutex);
	
	char normal[max_header_length + 1];
	memcpy(normal, name, len + 1);
	normalize_header(normal, len);
	
	cflare_hash hash = cflare_hash_compute(normal, len);
	cflare_hashtable_set(canonical_headers, hash, &ptr, sizeof(loaded_header*));
	
	if(output)
		*output = ptr;
}

static void parse_line(char* line, const char* name)
{
	size_t len = strlen(line);
	
	char buff[max_header_length + 1];
	size_t pos = 0;
	
	// find and remove the comment
	for(size_t i = 0; i < len; i++)
	{
		if(line[i] == '#')
		{
			len = i;
			line[i] = '\0';
			break;
		}
	}
	
	size_t i = 0;
	while(i < len && isspace((unsigned char)line[i]))
		i++;
	
	while(i < len && !isspace((unsigned char)line[i]))
	{
		buff[pos] = line[i];
		pos++;
		i++;
		
		if(pos > max_header_length)
		{
			cflare_warn("headers: header hit length limit (%d): %s",  max_header_length, line);
			return;
		}
	}
	
	if(pos == 0)
		return;
	
	buff[pos] = '\0';
	
	loaded_header* header;
	create_header(buff, &header);
	
	header->on_disk = true;
	header->last_used = 0;
}

static void parse_file(const char* path, const char* name)
{
	cflare_debug("headers: loading %s", name);
	FILE* fp = fopen(path, "r");
	
	if(!fp)
	{
		cflare_warn("headers: can't open %s: %s", path, strerror(errno));
		return;
	}
	
	char buff[max_line_length];
	
	while(fgets(buff, max_line_length, fp))
		parse_line(buff, name);
	fclose(fp);
}

void cflare_headers_load()
{
	gid_mutex = cflare_mutex_new(CFLARE_MUTEX_PLAIN);
	canonical_headers = cflare_hashtable_new();
	cflare_hashtable_ondelete(canonical_headers, &free_header, 0);
	
	char* path = cflare_string_concat(cflare_cfgpath(), "http/headers");
	cflare_linkedlist* files = cflare_filesystem_list(path, 0);
	
	cflare_linkedlist_iterator iter = cflare_linkedlist_get_iterator(files);
	while(cflare_linkedlist_iterator_next(&iter))
	{
		cflare_filesystem_entry* ent = (cflare_filesystem_entry*)iter.value->data;
		parse_file(ent->path, ent->name);
	}
	
	cflare_linkedlist_delete(files);
	free(path);
}

void cflare_headers_unload()
{
	cflare_hashtable_delete(canonical_headers);
	cflare_mutex_delete(gid_mutex);
}

cflare_header cflare_headers_get(const char* name)
{
	size_t len = strlen(name);
	#ifdef _MSC_VER
	char* normal = _alloca(len + 1);
	#else
	char normal[len + 1];
	#endif
	
	memcpy(normal, name, len + 1);
	normalize_header(normal, len);
	
	loaded_header** header;
	size_t header_len;
	
	if(cflare_hashtable_get(
		canonical_headers,
		cflare_hash_compute(normal, len),
		(void**)&header, &header_len))
	{
		cflare_header ret = {(*header)->id, (*header)->name};
		return ret;
	}
	else
	{
		cflare_header null = {0, 0};
		return null;
	}
}

cflare_header cflare_headers_ensure(const char* name)
{
	cflare_header got = cflare_headers_get(name);
	if(cflare_headers_valid(got))
		return got;
	
	size_t len = strlen(name);
	assert(len <= max_header_length);
	char buff[max_header_length + 1];
	memcpy(buff, name, len + 1);
	
	create_header(buff, NULL);
	
	got = cflare_headers_get(name);
	assert(cflare_headers_valid(got));
	return got;
}

bool cflare_headers_valid(cflare_header header)
{
	return header.id > 0 && header.name != 0;
}

bool cflare_headers_equals(cflare_header a, cflare_header b)
{
	if(a.id != 0 && b.id != 0)
		return a.id == b.id;
	return strcmp(a.name, b.name) == 0;
}
