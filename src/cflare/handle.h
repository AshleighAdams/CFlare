#ifndef cflare_HANDLE_H
#define cflare_HANDLE_H

#include <stdint.h>
#include <stddef.h>

typedef size_t cflare_handle;
typedef void(*cflare_handle_deleter)(void* ptr);

void cflare_handle_load();
void cflare_handle_unload();

cflare_handle cflare_handle_new(const char* type, void* data, cflare_handle_deleter deleter);
void cflare_handle_reference(cflare_handle hd);
void cflare_handle_unreference(cflare_handle hd);

void* cflare_handle_data(cflare_handle hd);

#endif /* cflare_HANDLE_H */

