#include "cflare/handle.h"
#include "cflare/util.h"

#include "cflare/linkedlist2.h"
#include "cflare/hashmap.h"

int main(int argc, char** argv)
{
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
	
	// hashmap test
	{
		const char* test = "Hello, world!";
		size_t len = strlen(test);
		
		cflare_hash hash = cflare_hash_compute(test, len);
		printf("hash(\"%s\") = %u\n", test, hash.hash);
		
		cflare_hashmap* map = cflare_hashmap_new();
		{
			const char* key = "Content-Length";
			size_t key_len = strlen(key);
			const char* value = "1337";
			size_t value_len = strlen(key);
			
			cflare_debug("hashmap test: pre-set: %lu buckets", map->buckets_count);
			cflare_hashmap_set(map, cflare_hash_compute(key, key_len), value, value_len);
			cflare_debug("hashmap test: post-set: %lu buckets", map->buckets_count);
			
			char* get_value;
			size_t get_value_len;
			if(!cflare_hashmap_get(map, cflare_hash_compute(key, key_len), (void**)&get_value, &get_value_len))
				cflare_debug("hashmap test [fail]: %s not located", key);
			else
				cflare_debug("hashmap test [okay]: %s = %s", key, get_value);
			
			cflare_hashmap_rebuild(map, 16);
			cflare_debug("hashmap test: post-rebuild: %lu buckets", map->buckets_count);
			
			if(!cflare_hashmap_get(map, cflare_hash_compute(key, key_len), (void**)&get_value, &get_value_len))
				cflare_debug("hashmap test rebuild [fail]: %s not located", key);
			else
				cflare_debug("hashmap test rebuild [okay]: %s = %s", key, get_value);
		}
		cflare_hashmap_delete(map);
	}
	
	cflare_handle_load();
	
	cflare_handle hd = cflare_handle_new("test", 0, 0);
	printf("hd = %lu\n", hd);
	cflare_handle_unreference(hd);
	
	cflare_handle_unload();
	return 0;
}
