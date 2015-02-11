
#include "unit-test.h"

#include <cflare/handle.h>
#include <cflare/util.h>
#include <cflare/hook.h>
#include <cflare/options.h>

#include <cflare/linkedlist.h>
#include <cflare/hashtable.h>

uint32_t unload_test(const cflare_hookstack* args, cflare_hookstack* rets, void* context)
{
	cflare_debug("Inside Unload hook! args: %p; rets: %p", args, rets);
	//cflare_hookstack_push_integer(rets, 1337);
	cflare_hookstack_push_number(rets, 1337);
	return 0;
}

int main(int argc, char** argv)
{
	cflare_options_load(argc, argv);
	cflare_handle_load();
	cflare_hook_load();
	cflare_hook_call("Load", 0, 0);
	
	int result = 0;
	
	if(strcmp(cflare_options_argument(0), "unit-test") == 0)
	{
		result = unit_test();
	}
	else
	{
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
	}
	
	cflare_hook_unload();
	cflare_handle_unload();
	clfare_options_unload();
	return result;
}
