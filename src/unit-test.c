
#include "unit-test.h"

#include <cflare/buffer.h>
#include <cflare/hook.h>
#include <cflare/hashtable.h>

// if a test fails, it doesn't need to free memory.

int test_buffer()
{
	cflare_buffer* buff = cflare_buffer_new(CFLARE_BUFFER_NOCOPY | CFLARE_BUFFER_NULLCHAR);
	
	const char* a = "First";
	const char* b = "Second";
	const char* c = "Third";
	
	cflare_buffer_append(buff, (uint8_t*)a, strlen(a));
	cflare_buffer_append(buff, (uint8_t*)b, strlen(b));
	cflare_buffer_append(buff, (uint8_t*)c, strlen(c));
	
	unit_test_part("append");
	if(buff->length != strlen(a) + strlen(b) + strlen(c))
		return 1;
	
	char* result = (char*)cflare_buffer_build(buff);
	
	unit_test_part("build");
	if(strcmp(result, "FirstSecondThird") != 0)
		return 1;
	
	free(result);
	cflare_buffer_delete(buff);
	
	return 0;
}

uint32_t test_hook_abc(const cflare_hookstack* args, cflare_hookstack* rets, void* context)
{
	int* val = (int*)context;
	*val = 7357;
	return 0;
}
int test_hook()
{
	int x = 0;
	cflare_hook_add("Abc", "test", 0, &test_hook_abc, &x);
	cflare_hook_call("Abc", 0, 0);
	
	unit_test_part("context");
	if(x !=  7357)
		return 1;
	
	return 0;
}

int test_linkedlist()
{
	cflare_linkedlist* list = cflare_linkedlist_new(32);
	{
		char* ptr;

		cflare_linkedlist_insert_last(list, (void**)&ptr);
		strncpy(ptr, "Hello, ", 32);

		cflare_linkedlist_insert_last(list, (void**)&ptr);
		strncpy(ptr, "world; ", 32);

		cflare_linkedlist_insert_last(list, (void**)&ptr);
		strncpy(ptr, "how're you?", 32);
		
		unit_test_part("add");
		if(list->count != 3)
		{
			cflare_linkedlist_delete(list);
			return 1;
		}
		
		unit_test_part("remove");
		// remove the 2nd one
		cflare_linkedlist_remove(list, list->first->next);
		
		if(list->count != 2)
		{
			cflare_linkedlist_delete(list);
			return 1;
		}
	}
	cflare_linkedlist_delete(list);
	return 0;
}

int test_hashtable()
{
	cflare_hashtable* map = cflare_hashtable_new();
	{
		const char* key = "Content-Length";
		size_t key_len = strlen(key);
		const char* value = "1337";
		size_t value_len = strlen(key);
		
		cflare_hashtable_set(map, cflare_hash_compute(key, key_len), value, value_len);
		unit_test_part("buckets set");
		if(map->buckets_count == 0)
			return 1;
		unit_test_part("set");
		if(map->count != 1)
			return 1;
		
		cflare_hashtable_set(map, cflare_hash_compute(key, key_len), value, value_len);
		unit_test_part("double set");
		if(map->count != 1)
			return 1;
		
		unit_test_part("locate 1");
		char* get_value;
		size_t get_value_len;
		if(!cflare_hashtable_get(map, cflare_hash_compute(key, key_len), (void**)&get_value, &get_value_len))
			return 1;
		
		unit_test_part("locate 1: value");
		if(strcmp(get_value, value) != 0)
			return 1;
		
		unit_test_part("rebuild");
		cflare_hashtable_rebuild(map, 64);
		if(map->buckets_count != 64)
			return 1;
		
		unit_test_part("locate 2");
		if(!cflare_hashtable_get(map, cflare_hash_compute(key, key_len), (void**)&get_value, &get_value_len))
			return 1;
		
		unit_test_part("locate 2: value");
		if(strcmp(get_value, value) != 0)
			return 1;
		
		unit_test_part("remove");
		cflare_hashtable_set(map, cflare_hash_compute(key, key_len), 0, 0);
		if(map->count != 0)
			return 1;
		
	}
	cflare_hashtable_delete(map);
	return 0;
}

int test_failed;
const char* msg;
void unit_test_part(const char* location)
{
	msg = location;
	cflare_log("\tchecking %s...", msg);
}
void test_function(const char* testname, int(*testfunc)())
{
	msg = 0;
	cflare_log("Testing %s...", testname);
	
	int result = testfunc();
	
	if(result)
	{
		test_failed = 1;
		cflare_log("Test %s failed", testname);
	}
	else
		cflare_log("Test %s passed", testname);
}

int unit_test()
{
	test_failed = 0;
	
	test_function("buffer", &test_buffer);
	test_function("hook", &test_hook);
	test_function("linkedlist", &test_linkedlist);
	test_function("hashtable", &test_hashtable);
	
	if(test_failed)
		cflare_log("One or more unit tests failed.");
	else
		cflare_log("All unit tests passed.");
	
	return test_failed;
}