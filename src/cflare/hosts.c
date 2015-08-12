
#include "cflare/hosts.h"
#include "cflare/hashtable.h"

static cflare_host* hosts = 0x0;
static size_t hosts_len = 0;
static size_t hosts_alloc = 32;

cflare_host* cflare_hosts_any;
cflare_host* cflare_hosts_dev;

typedef struct cflare_host_staticroute
{
	char* path;
	cflare_host_callback* callback;
	void* context;
} cflare_host_staticroute;

typedef struct cflare_host
{
	char* name;
	cflare_hashtable* static_routes;
	size_t index;
} cflare_host;

void cflare_hosts_load()
{
	assert(hosts == 0x0 && hosts_len == 0);
	
	size_t len = sizeof(cflare_host) * hosts_alloc;
	hosts = malloc(len);
	memset(hosts, 0, len);
}

void cflare_hosts_unload()
{
	for(size_t i = 0; i < hosts_len; i++)
	{
		// these should be unloaded, throw a warning and delete() them
		cflare_host* host = hosts + i;
		cflare_warn("host not deleted on unload: %s", host->name);
		cflare_host_delete(host);
	}
	
	free(hosts);
	hosts = 0x0;
	hosts_len = 0;
	hosts_alloc = 32;
}

static void free_staticroute(void* ptr, void* context)
{
	cflare_host_staticroute* rt = ptr;
	free(rt->path);
}

#define DESCRIPTOR_TO_PTR(__ARG__, __ARR__) __ARG__ = __ARR__ + (((uintptr_t)__ARG__) - 1)

// TODO: WARNING: this is not thread safe!
cflare_host* cflare_host_new(const char* domain)
{
	size_t i;
	for(i = 0; i < hosts_alloc; i++)
	{
		if(!hosts[i].name)
			break;
	}

	if(!i < hosts_alloc) // wasn't a free one, let's grow the array
	{
		size_t half = hosts_alloc; // this will be both the old position and the new length to memset to 0
		hosts_alloc *= 2; // double the space
		size_t len = sizeof(cflare_host) * hosts_alloc;

		void* newptr = realloc(hosts, len);
		assert(newptr);
		hosts = newptr;

		memset(hosts + half, 0, half * sizeof(cflare_host));
		// i is already pointing at the new place
	}

	cflare_host* hst = hosts + i;
	hst->name = strdup(domain);
	hst->static_routes = cflare_hashtable_new();
	cflare_hashtable_ondelete(hst->static_routes, &free_staticroute, 0x0);

	return (void*)(i + 1); // + 1 so that the first entry isn't a nullptr
}

void cflare_host_delete(cflare_host* host)
{
  	DESCRIPTOR_TO_PTR(host, hosts);
	cflare_hashtable_delete(host->static_routes);
	free(host->name);
	host->name = 0x0; // mark it as free
}

void cflare_host_map(cflare_host* host, const char* path, cflare_host_callback* callback, void* context)
{
  	DESCRIPTOR_TO_PTR(host, hosts);
  	cflare_host_staticroute route;
	route.path = strdup(path);
	route.callback = callback;
	route.context = context;

	cflare_hashtable_set(host->static_routes,
	                     cflare_hash_compute(path, strlen(path)),
	                     &route, sizeof(route));
}

void cflare_host_mapf(cflare_host* host, const char* pattern, cflare_host_callback* callback, void* context)
{
  	DESCRIPTOR_TO_PTR(host, hosts);
	cflare_notimp();
}

