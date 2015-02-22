
#include "unit-test.h"

#include <cflare/cflare.h>
#include <cflare/handle.h>
#include <cflare/util.h>
#include <cflare/hook.h>
#include <cflare/options.h>

#include <cflare/linkedlist.h>
#include <cflare/hashtable.h>
#include <cflare/filesystem.h>


bool unload_test(const cflare_hookstack* args, cflare_hookstack* rets, void* context)
{
	cflare_debug("Inside Unload hook! args: %p; rets: %p", (void*)args, (void*)rets);
	//cflare_hookstack_push_integer(rets, 1337);
	cflare_hookstack_push_number(rets, 1337);
	return 0;
}

int main(int argc, char** argv)
{
	cflare_load(argc, argv);
	cflare_hook_call("Load", 0, 0);
	
	int result = 0;
	
	if(cflare_options_boolean("version", false))
	{
		cflare_log("cflare %s", cflare_version());
	}
	else if(cflare_options_argument(0) && strcmp(cflare_options_argument(0), "unit-test") == 0)
	{
		result = unit_test();
	}
	else
	{
		cflare_hook_add("Unload", "unload test", 0, &unload_test, 0);
	
		cflare_handle hd = cflare_handle_new("test", 0, 0, 0);
		printf("hd = %zu\n", hd);
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
		
		/*{
			cflare_linkedlist* files = cflare_filesystem_list(".", CFLARE_FILESYSTEM_LIST_RECURSIVE | CFLARE_FILESYSTEM_LIST_EXCLUDE_DIRECTORIES);
			
			cflare_linkedlist_iter iter = cflare_linkedlist_iterator(files);
			while(cflare_linkedlist_iterator_next(&iter))
			{
				cflare_filesystem_entry* ent = (cflare_filesystem_entry*)iter.value->data;
				cflare_log("ls: %lu %s", ent->depth, ent->name);
			}
			
			cflare_linkedlist_delete(files);
		}*/
	}
	
	cflare_unload();
	return result;
}
