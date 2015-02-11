
#include <cflare/handle.h>
#include <cflare/util.h>
#include <cflare/hook.h>

#include <cflare/linkedlist.h>
#include <cflare/hashtable.h>
#include <cflare/buffer.h>

uint32_t unload_test(const cflare_hookstack* args, cflare_hookstack* rets, void* context)
{
	cflare_debug("Inside Unload hook! args: %p; rets: %p", args, rets);
	//cflare_hookstack_push_integer(rets, 1337);
	cflare_hookstack_push_number(rets, 1337);
	return 0;
}

void test_buffer()
{
	cflare_buffer* buff = cflare_buffer_new(CFLARE_BUFFER_NOCOPY | CFLARE_BUFFER_NULLCHAR);
	
	const char* a = "First";
	const char* b = "Second";
	const char* c = "Third";
	
	cflare_buffer_append(buff, (uint8_t*)a, strlen(a));
	cflare_buffer_append(buff, (uint8_t*)b, strlen(b));
	cflare_buffer_append(buff, (uint8_t*)c, strlen(c));
	
	char* result = (char*)cflare_buffer_build(buff);
	
	cflare_debug("buffer: %s", result);
	
	free(result);
	cflare_buffer_delete(buff);
}

int main(int argc, char** argv)
{
	test_buffer();
	
	// linked list test
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
	
			cflare_linkedlist_iter iter = cflare_linkedlist_iterator(list);
			while(cflare_linkedlist_iterator_next(&iter))
				printf("%s", (char*)iter.value->data);
			printf("\n");
	
			// remove the 2nd one
			cflare_linkedlist_remove(list, list->first->next);
	
			iter = cflare_linkedlist_iterator(list);
			while(cflare_linkedlist_iterator_next(&iter))
				printf("%s", (char*)iter.value->data);
			printf("\n");
		}
		cflare_linkedlist_delete(list);
	}
	
	// hashtable test
	{
		const char* test = "Hello, world!";
		size_t len = strlen(test);
		
		cflare_hash hash = cflare_hash_compute(test, len);
		printf("hash(\"%s\") = %u\n", test, hash.hash);
		
		cflare_hashtable* map = cflare_hashtable_new();
		{
			const char* key = "Content-Length";
			size_t key_len = strlen(key);
			const char* value = "1337";
			size_t value_len = strlen(key);
			
			cflare_debug("hashtable test: pre-set: %lu buckets", map->buckets_count);
			cflare_hashtable_set(map, cflare_hash_compute(key, key_len), value, value_len);
			cflare_debug("hashtable test: post-set: %lu buckets", map->buckets_count);
			
			cflare_hashtable_set(map, cflare_hash_compute(key, key_len), value, value_len);
			if(map->count == 1)
				cflare_debug("hashtable test: replace: double-set replaced old.");
			else
				cflare_warn("hashtable test: replace: double-set did not replace old.");
			
			char* get_value;
			size_t get_value_len;
			if(!cflare_hashtable_get(map, cflare_hash_compute(key, key_len), (void**)&get_value, &get_value_len))
				cflare_warn("hashtable test [fail]: %s not located", key);
			else
				cflare_debug("hashtable test [okay]: %s = %s", key, get_value);
			
			cflare_hashtable_rebuild(map, 64);
			cflare_debug("hashtable test: post-rebuild: %lu buckets", map->buckets_count);
			
			if(!cflare_hashtable_get(map, cflare_hash_compute(key, key_len), (void**)&get_value, &get_value_len))
				cflare_debug("hashtable test rebuild [fail]: %s not located", key);
			else
				cflare_debug("hashtable test rebuild [okay]: %s = %s", key, get_value);
			
			cflare_hashtable_set(map, cflare_hash_compute(key, key_len), 0, 0);
			if(map->count == 1)
				cflare_warn("hashtable test: remove: set null did not remove.");
			else
				cflare_debug("hashtable test: remove: set null removed.");
			
		}
		cflare_hashtable_delete(map);
	}
	
	cflare_handle_load();
	cflare_hook_load();
	cflare_hook_call("Load", 0, 0);
	
	cflare_hook_add("Unload", "unload test", 0, &unload_test, 0);
	
	cflare_handle hd = cflare_handle_new("test", 0, 0, 0);
	printf("hd = %lu\n", hd);
	cflare_handle_unreference(hd);
	
	{
		cflare_hookstack* args = cflare_hookstack_new();
		cflare_hookstack* rets = cflare_hookstack_new();
			cflare_hook_call("Unload", args, rets);
			int64_t val;
			if(cflare_hookstack_get_integer(rets, 0, &val))
				cflare_debug("Unload: return 0: %li", val);
			else
				cflare_debug("Unload: return 0: (nil)");
		cflare_hookstack_delete(args);
		cflare_hookstack_delete(rets);
	}
	
	cflare_hook_unload();
	cflare_handle_unload();
	return 0;
}
