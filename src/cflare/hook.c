#include "cflare/util.h"
#include "cflare/hashtable.h"
#include "cflare/linkedlist.h"

#include "cflare/hook.h"

typedef struct hook
{
	char* id;
	double32_t priority;
	void* context;
	hook_function* func;
} hook;

void free_hook(void* ptr, void* unused)
{
	hook* tbl = (hook*)ptr;
	free(tbl->id);
}

typedef struct hook_table
{
	char* name;
	cflare_linkedlist* funcs;
} hook_table;

cflare_hashtable* hook_tables;

void free_hooktable(void* ptr, void* unused)
{
	hook_table* tbl = (hook_table*)ptr;
	free(tbl->name);
	cflare_linkedlist_delete(tbl->funcs);
}

void cflare_hook_load()
{
	hook_tables = cflare_hashtable_new();
	cflare_hashtable_ondelete(hook_tables, &free_hooktable, 0);
}

void cflare_hook_unload()
{
	 cflare_hashtable_delete(hook_tables);
}

void cflare_hook_add(const char* name, const char* id, double64_t priority,
	hook_function* func, void* context)
{
	cflare_hook_remove(name, id);
	
	hook_table* tbl;
	size_t len;
	size_t namelen = strlen(name);
	
	cflare_hash hash = cflare_hash_compute(name, namelen);
	
	if(!cflare_hashtable_get(hook_tables, hash, (void**)&tbl, &len))
	{
		hook_table newtbl;
		newtbl.funcs = cflare_linkedlist_new(sizeof(hook));
		newtbl.name = malloc(namelen);
		
		cflare_linkedlist_ondelete(newtbl.funcs, &free_hook, 0);
		strncpy(newtbl.name, name, namelen);
		
		cflare_hashtable_set(hook_tables, hash, &newtbl, sizeof(hook_table));
		
		assert(cflare_hashtable_get(hook_tables, hash, (void**)&tbl, &len));
	}
	
	cflare_linkedlist_iter iter = cflare_linkedlist_iterator(tbl->funcs);
	while(cflare_linkedlist_iterator_next(&iter))
	{
		hook* val = (hook*)iter.value->data;
		if(val->priority > priority)
			break;
	}
	
	hook* new_tbl;
	cflare_linkedlist_insert_before(tbl->funcs, iter.value, (void**)&new_tbl);
	
	new_tbl->id = malloc(strlen(id));
	strncpy(new_tbl->id, id, strlen(id));
	
	new_tbl->priority = priority;
	new_tbl->context = context;
	
	new_tbl->func = func;
}

void cflare_hook_remove(const char* name, const char* id)
{
	hook_table* tbl;
	size_t len;
	
	if(!cflare_hashtable_get(hook_tables, cflare_hash_compute(name, strlen(name)), (void**)&tbl, &len))
		return;
	
	cflare_linkedlist_iter iter = cflare_linkedlist_iterator(tbl->funcs);
	while(cflare_linkedlist_iterator_next(&iter))
	{
		hook* val = (hook*)iter.value->data;
		if(strcmp(val->id, id) == 0)
			cflare_linkedlist_remove(tbl->funcs, iter.value);
	}
}

int32_t cflare_hook_call(const char* name, const cflare_hookstack* args,
	cflare_hookstack* returns)
{
	hook_table* tbl;
	size_t len;
	
	if(!cflare_hashtable_get(hook_tables, cflare_hash_compute(name, strlen(name)), (void**)&tbl, &len))
		return 0;
	
	
	cflare_linkedlist_iter iter = cflare_linkedlist_iterator(tbl->funcs);
	while(cflare_linkedlist_iterator_next(&iter))
	{
		hook* val = (hook*)iter.value->data;
		
		uint32_t rets = val->func(args, returns, val->context);
		
		if(rets != 0)
			return rets;
	}
	
	return 0;
}

// HookStack functions

void free_element(void* data, void* context)
{
	cflare_hookstack_elm* elm = (cflare_hookstack_elm*)data;
	
	if(elm->deleter && elm->data)
		elm->deleter(elm->data, elm->deleter_context);
	free(elm->data);
}

cflare_hookstack* cflare_hookstack_new()
{
	cflare_hookstack* ret = malloc(sizeof(cflare_hookstack));
	ret->elements = cflare_linkedlist_new(sizeof(cflare_hookstack_elm));
	cflare_linkedlist_ondelete(ret->elements, &free_element, 0);
	return ret;
}

void cflare_hookstack_delete(cflare_hookstack* stack)
{
	if(!stack)
		return;
	cflare_linkedlist_delete(stack->elements);
	free(stack);
}

const char* cflare_hookstack_type_tostring(cflare_hookstack_type input)
{
	switch(input)
	{
	case CFLARE_HOOKSTACK_INTEGER:
		return "integer";
	case CFLARE_HOOKSTACK_NUMBER:
		return "number";
	case CFLARE_HOOKSTACK_STRING:
		return "string";
	case CFLARE_HOOKSTACK_POINTER:
		return "pointer";
	case CFLARE_HOOKSTACK_HANDLE:
		return "handle";
	default:
		return "unknown";
	}
}

// push

cflare_hookstack_elm* get_elm(cflare_hookstack* stack, int32_t index, cflare_hookstack_type type)
{
	if(!stack)
		return 0;
	
	cflare_linkedlist_iter iter = cflare_linkedlist_iterator(stack->elements);
	
	if(index >= 0)
	{
		for(int32_t i = 0; i < index + 1; i++)
			if(!cflare_linkedlist_iterator_next(&iter))
				return 0;
	}
	else
	{
		for(int32_t i = 0; i > index - 1; i--)
			if(!cflare_linkedlist_iterator_prev(&iter))
				return 0;
	}
	
	// we found one
	cflare_hookstack_elm* ret = (cflare_hookstack_elm*)iter.value->data;
	
	if(ret->type != type)
	{
		cflare_warn("cflare_hookstack_get_*(): type mismatch: at index %i; have %s, requested %s.",
			index, cflare_hookstack_type_tostring(ret->type), cflare_hookstack_type_tostring(type));
		return 0;
	}
	
	return ret;
}

void cflare_hookstack_push_integer(cflare_hookstack* stack, int64_t value)
{
	if(!stack)
		return;
	
	cflare_hookstack_elm* elm;
	cflare_linkedlist_insert_last(stack->elements, (void**)&elm);
	
	elm->type = CFLARE_HOOKSTACK_INTEGER;
	elm->data = malloc(sizeof(int64_t));
	elm->deleter = 0;
	elm->deleter_context = 0;
	
	*(int64_t*)elm->data = value;
}
int32_t cflare_hookstack_get_integer(cflare_hookstack* stack, int32_t index,
	int64_t* out)
{
	cflare_hookstack_elm* elm = get_elm(stack, index, CFLARE_HOOKSTACK_INTEGER);
	if(!elm)
		return 0;
	
	*out = *(int64_t*)elm->data;
	return 1;
}


void cflare_hookstack_push_number(cflare_hookstack* stack, double64_t value)
{
	if(!stack)
		return;
	
	cflare_hookstack_elm* elm;
	cflare_linkedlist_insert_last(stack->elements, (void**)&elm);
	
	elm->type = CFLARE_HOOKSTACK_NUMBER;
	elm->data = malloc(sizeof(double64_t));
	elm->deleter = 0;
	elm->deleter_context = 0;
	
	*(double64_t*)elm->data = value;
}
int32_t cflare_hookstack_get_number(cflare_hookstack* stack, int32_t index,
	double64_t* out)
{
	cflare_hookstack_elm* elm = get_elm(stack, index, CFLARE_HOOKSTACK_NUMBER);
	if(!elm)
		return 0;
	
	*out = *(double64_t*)elm->data;
	return 1;
}