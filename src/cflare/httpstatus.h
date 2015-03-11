#ifndef CFLARE_HTTPSTATUS_H
#define CFLARE_HTTPSTATUS_H

#include "cflare/cflare.h"
#include "cflare/util.h"

CFLARE_API void cflare_httpstatus_load();
CFLARE_API void cflare_httpstatus_unload();

CFLARE_API const char* cflare_httpstatus_tostring(uint32_t code);
CFLARE_API uint32_t cflare_httpstatus_fromstring(const char* status);

#endif /* CFLARE_HTTPSTATUS_H */

