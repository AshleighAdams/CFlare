#ifndef cflare_HANDLE_H
#define cflare_HANDLE_H

#include <stdint.h>
#include <stddef.h>

#include <cflare/cflare.h>
#include <cflare/util.h>

typedef size_t cflare_handle;

CFLARE_API void cflare_handle_load();
CFLARE_API void cflare_handle_unload();

CFLARE_API cflare_handle cflare_handle_new(const char* type, void* data,
	cflare_deleter* deleter, void* context);
CFLARE_API void cflare_handle_reference(cflare_handle hd);
CFLARE_API void cflare_handle_unreference(cflare_handle hd);

CFLARE_API void* cflare_handle_data(cflare_handle hd);

#endif /* cflare_HANDLE_H */

