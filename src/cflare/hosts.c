
#include "cflare/hosts.h"

static cflare_host** hosts = 0x0;
static size_t hosts_len = 0;
static size_t hosts_alloc = 32;

cflare_host* cflare_hosts_any;
cflare_host* cflare_hosts_dev;

typedef struct cflare_host
{
	char* name;
} cflare_host;

void cflare_hosts_load()
{
	assert(hosts == 0x0 && hosts_len == 0);
	
	hosts = malloc(sizeof(cflare_host*) * hosts_alloc);
	
}

void cflare_hosts_unload()
{
	for(size_t i = 0; i < hosts_len; i++)
	{
		// these should be unloaded, throw a warning and delete() them
		cflare_host* host = hosts[i];
		cflare_warn("host not deleted on unload: %s", host->name);
		cflare_host_delete(host);
	}
	
	free(hosts);
	hosts = 0x0;
	hosts_len = 0;
	hosts_alloc = 32;
}

cflare_host* cflare_host_new(const char* domain)
{
	cflare_host* ret = malloc(sizeof(cflare_host));
	ret->name = strdup(domain);
	return ret;
}

void cflare_host_delete(cflare_host* host)
{
	free(host->name);
	free(host);
}

void cflare_host_map(cflare_host* site, const char* path, cflare_host_callback* callback, void* context)
{
}

void cflare_host_mapf(cflare_host* site, const char* pattern, cflare_host_callback* callback, void* context)
{
	
}
