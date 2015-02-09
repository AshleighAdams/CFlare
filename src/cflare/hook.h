#ifndef CFLARE_HOOK_H
#define CFLARE_HOOK_H

#include "cflare/util.h"
#include "cflare/handle.h"
#include "cflare/linkedlist.h"

typedef enum
{
	CFLARE_HOOKSTACK_INTEGER,
	CFLARE_HOOKSTACK_NUMBER,
	CFLARE_HOOKSTACK_STRING,
	CFLARE_HOOKSTACK_POINTER,
	CFLARE_HOOKSTACK_HANDLE
	
} cflare_hookstack_type;

const char* cflare_hookstack_type_tostring(cflare_hookstack_type input);

typedef struct cflare_hookstack_elm
{
	cflare_hookstack_type type;
	void* data;
	cflare_deleter* deleter;
	void* deleter_context;
} cflare_hookstack_elm;

typedef struct cflare_hookstack
{
	cflare_linkedlist* elements;
} cflare_hookstack;

typedef uint32_t(hook_function)(const cflare_hookstack*, cflare_hookstack*,
	void* context);

void cflare_hook_load();
void cflare_hook_unload();
void cflare_hook_add(const char* name, const char* id, double64_t priority,
	hook_function* func, void* context);
void cflare_hook_remove(const char* name, const char* id);
int32_t cflare_hook_call(const char* name, const cflare_hookstack* args,
	cflare_hookstack* returns);

cflare_hookstack* cflare_hookstack_new();
void cflare_hookstack_delete(cflare_hookstack* stack);

void cflare_hookstack_push_integer(cflare_hookstack* stack, int64_t value);
int32_t cflare_hookstack_get_integer(cflare_hookstack* stack, int32_t index,
	int64_t* out);

void cflare_hookstack_push_number(cflare_hookstack* stack, double64_t value);
int32_t cflare_hookstack_get_number(cflare_hookstack* stack, int32_t index,
	double64_t* out);

void cflare_hookstack_push_string(cflare_hookstack* stack, const char* value);
int32_t cflare_hookstack_get_string(cflare_hookstack* stack, int32_t index,
	char** out);

void cflare_hookstack_push_pointer(cflare_hookstack* stack, const char* type,
	void* ptr, cflare_deleter* deleter, void* context);
int32_t cflare_hookstack_get_pointer(cflare_hookstack* stack, int32_t index,
	const char* type, void** out);

void cflare_hookstack_push_handle(cflare_hookstack* stack, cflare_handle hd);
int32_t cflare_hookstack_get_handle(cflare_hookstack* stack, int32_t index,
	cflare_handle* out);


#endif /* CFLARE_HOOK_H */

