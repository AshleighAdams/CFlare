#ifndef CFLARE_HOOK_H
#define CFLARE_HOOK_H

typedef struct cflare_hook_stack_elm
{
	size_t type;
	void* data;
} cflare_hook_stack_elm;

typedef struct cflare_hook_stack
{
	size_t count;
	cflare_hook_stack_elm* elements;
} cflare_hook_stack;

typedef uint32_t(hook_function)(const cflare_hook_stack*, cflare_hook_stack*, void* context);

void cflare_hook_load();
void cflare_hook_unload();
void cflare_hook_add(const char* name, const char* id, double64_t priority, hook_function* func, void* context);
void cflare_hook_remove(const char* name, const char* id);
void cflare_hook_call(const char* name, const cflare_hook_stack* args, cflare_hook_stack** returns);

#endif /* CFLARE_HOOK_H */

