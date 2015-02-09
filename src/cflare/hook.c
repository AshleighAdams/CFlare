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

void cflare_hook_call(const char* name, const cflare_hook_stack* args,
	cflare_hook_stack** returns)
{
	hook_table* tbl;
	size_t len;
	
	if(!cflare_hashtable_get(hook_tables, cflare_hash_compute(name, strlen(name)), (void**)&tbl, &len))
		return;
	
	if(returns)
		*returns = 0;
	
	cflare_linkedlist_iter iter = cflare_linkedlist_iterator(tbl->funcs);
	while(cflare_linkedlist_iterator_next(&iter))
	{
		hook* val = (hook*)iter.value->data;
		
		uint32_t rets = val->func(args, 0, val->context);
		
		if(rets != 0)
			break;
	}
}







