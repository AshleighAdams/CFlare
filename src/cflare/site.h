
#ifndef CFLARE_SITE_H
#define CFLARE_SITE_H

#include <cflare/cflare.h>
#include <cflare/util.h>
#include <cflare/request.h>
#include <cflare/response.h>


typedef struct cflare_site cflare_site;
typedef bool(cflare_site_add_callback)(cflare_request*, cflare_response*);

cflare_site* cflare_site_any; // global variable
cflare_site* cflare_site_dev;

CFLARE_API cflare_site* cflare_site_new(const char* domain);
CFLARE_API void cflare_site_delete(cflare_site* site);

CFLARE_API void cflare_site_add(cflare_site* site, const char* path, cflare_site_add_callback* callback);
CFLARE_API void cflare_site_add_pattern(cflare_site* site, const char* path, void* callback);

#endif

