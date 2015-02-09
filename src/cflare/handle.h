#ifndef cflare_HANDLE_H
#define cflare_HANDLE_H

#include <stdint.h>
#include <stddef.h>

#include "cflare/util.h"

typedef size_t cflare_handle;

void cflare_handle_load();
void cflare_handle_unload();

cflare_handle cflare_handle_new(const char* type, void* data,
	cflare_deleter* deleter, void* context);
void cflare_handle_reference(cflare_handle hd);
void cflare_handle_unreference(cflare_handle hd);

void* cflare_handle_data(cflare_handle hd);

#endif /* cflare_HANDLE_H */

