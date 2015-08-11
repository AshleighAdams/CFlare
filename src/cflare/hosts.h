
#ifndef cflare_host_H
#define cflare_host_H

#include <cflare/cflare.h>
#include <cflare/util.h>
#include <cflare/request.h>
#include <cflare/response.h>


typedef struct cflare_host cflare_host;
typedef bool(cflare_host_callback)(cflare_request*, cflare_response*, void* context, uint64_t argc, char** argv);

extern cflare_host* cflare_hosts_any; // global variable
extern cflare_host* cflare_hosts_dev;

CFLARE_API void cflare_hosts_load();
CFLARE_API void cflare_hosts_unload();

// < 0 = error; >= 0 = matched hosts
CFLARE_API ssize_t cflare_hosts_match(const char* host, cflare_host*** out_array);

CFLARE_API cflare_host* cflare_host_new(const char* domain_glob); // a glob
CFLARE_API void cflare_host_delete(cflare_host* site);

CFLARE_API void cflare_host_map (cflare_host* site, const char* path,    cflare_host_callback* callback, void* context);
CFLARE_API void cflare_host_mapf(cflare_host* site, const char* pattern, cflare_host_callback* callback, void* context);


#endif

