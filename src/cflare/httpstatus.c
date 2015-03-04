
#include "cflare/httpstatus.h"
#include "cflare/hashtable.h"
#include "cflare/filesystem.h"

#include <errno.h>
#include <stdio.h>
#include <ctype.h>

typedef struct known_status
{
	uint32_t code;
	char* string;
} known_status;

static cflare_hashtable* cache_code2string;
static cflare_hashtable* cache_string2code;

static void free_knownstatus(void* data, void* ctx)
{
	known_status* status = (known_status*)data;
	free(status->string);
}

#define max_line_length 1024

// returns new length
static size_t normalize_status(char* ptr)
{
	size_t pos = 0, curpos = 0;
	while(true)
	{
		char x = ptr[curpos];
		if(x == '\0')
			break;
		
		if(isalpha((unsigned char)x))
		{
			ptr[pos] = (char)tolower((unsigned char)x);
			pos++;
		}
		
		curpos++;
	}
	ptr[pos] = '\0';
	return pos;
}

static void parse_line(char* line, const char* name)
{
	for(size_t i = 0; i < max_line_length; i++)
	{
		if(line[i] == '#')
		{
			line[i] = '\0';
			break;
		}
		else if(line[i] == '\0')
		{
			break;
		}
	}
	
	// remove starting whitespace
	while(true)
	{
		char x = line[0];
		if(x == '\0')
			return;
		else if(!isspace((unsigned char)x))
			break;
		else
			line++;
	}
	
	size_t digit_end = 0;
	while(true) // find where digits end
	{
		char x = line[digit_end];
		if(x == '\0')
			return;
		else if(isdigit((unsigned char)x))
			digit_end++;
		else
			break;
	}
	
	if(digit_end == 0)
		return; // digit code not found
	
	line[digit_end] = '\0';
	
	int64_t tmp;
	if(!cflare_tointeger(line, &tmp))
		return;
	line += digit_end + 1;
	uint32_t code = (uint32_t)tmp;
	
	// remove ending whitespace
	size_t lastpos = 0;
	size_t pos = 0;
	while(true)
	{
		char x = line[pos];
		if(x == '\0')
			break;
		else if(!isspace((unsigned char)x))
			lastpos = pos;
		pos++;
	}
	
	if(lastpos == 0)
		return;
	
	line[lastpos + 1] = '\0';
	size_t len = lastpos;
	
	{ // add to code2string cache
		known_status status;
		memset(&status, 0, sizeof(known_status)); // because compilers don't set the whole struct to 0, only it's fields...
		status.code = code;
		status.string = strdup(line);
		
		cflare_hash code_hash = cflare_hash_compute(&code, sizeof(uint32_t));
		cflare_hashtable_set(cache_code2string, code_hash, &status, sizeof(known_status));
	}
	
	{ // add to string2code cache
		known_status status;
		memset(&status, 0, sizeof(known_status));
		status.code = code;
		status.string = strdup(line);
		
		len = normalize_status(line); // modifies line
		
		cflare_hash string_hash = cflare_hash_compute(line, sizeof(char) * len);
		cflare_hashtable_set(cache_string2code, string_hash, &status, sizeof(known_status));
	}
}

static void parse_file(const char* path, const char* name)
{
	cflare_debug("httpstatus: loading %s", name);
	FILE* fp = fopen(path, "r");
	
	if(!fp)
	{
		cflare_warn("httpstatus: can't open %s: %s", path, strerror(errno));
		return;
	}
	
	char buff[max_line_length];
	
	while(fgets(buff, max_line_length, fp))
		parse_line(buff, name);
	fclose(fp);
}

void cflare_httpstatus_load()
{
	cache_code2string = cflare_hashtable_new();
	cache_string2code = cflare_hashtable_new();
	
	cflare_hashtable_ondelete(cache_code2string, &free_knownstatus, 0);
	cflare_hashtable_ondelete(cache_string2code, &free_knownstatus, 0);
	
	char* path =  cflare_string_concat(cflare_cfgpath(), "http/statuses");
	cflare_linkedlist* list = cflare_filesystem_list(path, 0);
	
	cflare_linkedlist_iter iter = cflare_linkedlist_iterator(list);
	while(cflare_linkedlist_iterator_next(&iter))
	{
		cflare_filesystem_entry* ent = (cflare_filesystem_entry*)iter.value->data;
		parse_file(ent->path, ent->name);
	}
	
	cflare_linkedlist_delete(list);
	free(path);
}

void cflare_httpstatus_unload()
{
	cflare_hashtable_delete(cache_string2code);
	cflare_hashtable_delete(cache_code2string);
}

const char* cflare_httpstatus_tostring(uint32_t code)
{
	cflare_hash hash = cflare_hash_compute(&code, sizeof(uint32_t));
	
	known_status* status;
	size_t ptr_len;
	if(cflare_hashtable_get(cache_code2string, hash, (void**)&status, &ptr_len))
		return status->string;
	
	return 0;
}

uint32_t cflare_httpstatus_fromstring(const char* stat)
{
	char* key = strdup(stat);
	size_t len = normalize_status(key);
	
	cflare_hash hash = cflare_hash_compute(key, sizeof(char) * len);
	
	uint32_t ret = 0;
	known_status* status;
	size_t ptr_len;
	if(cflare_hashtable_get(cache_string2code, hash, (void**)&status, &ptr_len))
		ret = status->code;
	
	free(key);
	return ret;
}
