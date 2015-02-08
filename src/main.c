#include "cflare/handle.h"
#include "cflare/util.h"
#include "cflare/linkedlist2.h"

int main(int argc, char** argv)
{
	cflare_linkedlist* list = cflare_linkedlist_new(32);
	{
		char* ptr;
	
		cflare_linkedlist_insert_after(list, list->last, (void**)&ptr);
		strncpy(ptr, "Hello, ", 32);
	
		cflare_linkedlist_insert_after(list, list->last, (void**)&ptr);
		strncpy(ptr, "world; ", 32);
	
		cflare_linkedlist_insert_after(list, list->last, (void**)&ptr);
		strncpy(ptr, "how're you?", 32);
	
		cflare_linkedlist_iter iter = cflare_linkedlist_iterator(list);
		while(cflare_linkedlist_iterator_next(&iter))
			printf("%s", (char*)iter.value->data);
		printf("\n");
	
		// remove the 2nd one
		cflare_linkedlist_remove(list, list->first->next);
	
		iter.started = 0;
		while(cflare_linkedlist_iterator_next(&iter))
			printf("%s", (char*)iter.value->data);
		printf("\n");
	}
	cflare_linkedlist_delete(list);
	
	cflare_handle_load();
	
	cflare_handle hd = cflare_handle_new("test", 0, 0);
	printf("hd = %lu\n", hd);
	cflare_handle_unreference(hd);
	
	cflare_handle_unload();
	return 0;
}
