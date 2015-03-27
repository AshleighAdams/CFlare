#ifndef CFLARE_HOOK_H
#define CFLARE_HOOK_H

#include <cflare/cflare.h>
#include <cflare/util.h>
#include <cflare/handle.h>
#include <cflare/linkedlist.h>

typedef enum
{
	CFLARE_HOOKSTACK_INTEGER,
	CFLARE_HOOKSTACK_NUMBER,
	CFLARE_HOOKSTACK_STRING,
	CFLARE_HOOKSTACK_POINTER,
	CFLARE_HOOKSTACK_HANDLE
	
} cflare_hookstack_type;

typedef struct cflare_hookstack cflare_hookstack;

const char* cflare_hookstack_type_tostring(cflare_hookstack_type input);

typedef bool(cflare_hookfunction)(const cflare_hookstack*, cflare_hookstack*,
	void* context);

CFLARE_API void cflare_hook_load();
CFLARE_API void cflare_hook_unload();
CFLARE_API void cflare_hook_add(const char* name, const char* id, float64_t priority,
	cflare_hookfunction* func, void* context);
CFLARE_API void cflare_hook_remove(const char* name, const char* id);
CFLARE_API bool cflare_hook_call(const char* name, const cflare_hookstack* args,
	cflare_hookstack* returns);

CFLARE_API cflare_hookstack* cflare_hookstack_new();
CFLARE_API void cflare_hookstack_delete(cflare_hookstack* stack);

CFLARE_API void cflare_hookstack_push_integer(cflare_hookstack* stack, int64_t value);
CFLARE_API bool cflare_hookstack_get_integer(const cflare_hookstack* stack, int32_t index,
	int64_t* out);

CFLARE_API void cflare_hookstack_push_number(cflare_hookstack* stack, float64_t value);
CFLARE_API bool cflare_hookstack_get_number(const cflare_hookstack* stack, int32_t index,
	float64_t* out);

CFLARE_API void cflare_hookstack_push_string(cflare_hookstack* stack, const char* value);
CFLARE_API bool cflare_hookstack_get_string(const cflare_hookstack* stack, int32_t index,
	const char** out);

CFLARE_API void cflare_hookstack_push_pointer(cflare_hookstack* stack, const char* type,
	void* ptr, cflare_deleter* deleter, void* context);
CFLARE_API bool cflare_hookstack_get_pointer(const cflare_hookstack* stack, int32_t index,
	const char* type, void** out);

CFLARE_API void cflare_hookstack_push_handle(cflare_hookstack* stack, cflare_handle hd);
CFLARE_API bool cflare_hookstack_get_handle(const cflare_hookstack* stack, int32_t index,
	cflare_handle* out);


#endif /* CFLARE_HOOK_H */

