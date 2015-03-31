
#include "unit-test.h"

#include <cflare/cflare.h>
#include <cflare/handle.h>
#include <cflare/util.h>
#include <cflare/hook.h>
#include <cflare/options.h>

#include <cflare/linkedlist.h>
#include <cflare/hashtable.h>
#include <cflare/filesystem.h>

#include <cflare/socket.h>
#include <cflare/request.h>

#include <errno.h>

static bool unload_test(const cflare_hookstack* args, cflare_hookstack* rets, void* context)
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
		
		{ // socket test
			cflare_socket* sock;
			if(!(sock = cflare_socket_connect("kateadams.eu", 80, CFLARE_SOCKET_TIMEOUT_FOREVER)))
				cflare_log("failed to connect: %s", strerror(errno));
			else
			{
				cflare_log("socket: %s %hu", cflare_socket_ip(sock), cflare_socket_port(sock));
				
				const char* request = 
					"GET / HTTP/1.1\n"
					"Host: kateadams.eu\n";
				
				cflare_socket_write_line(sock, request, strlen(request));
				
				char linebuff[1024];
				while(true)
				{
					size_t read = 0;
					if(!cflare_socket_read_line(sock, linebuff, sizeof(linebuff), &read))
						break;
					if(read == 0)
						break;
					cflare_log("%s", linebuff);
				}
				
				cflare_socket_delete(sock);
			}
			
			cflare_request* req = cflare_request_new();
			
			cflare_listener* listener = cflare_socket_listen(CFLARE_SOCKET_HOST_ANY, 1025);
			assert(listener);
			cflare_log("listener: %s %hu", cflare_listener_address(listener), cflare_listener_port(listener));
			char response[] = "HTTP/1.1 200 OK\nContent-Length: 13\nContent-Type: text/plain\nConnection: keep-alive\n\nHello, world!";
			
			while(true)
			{
				sock = cflare_listener_accept(listener);
					assert(sock);
					cflare_socket_timeout(sock, 5);
					cflare_log("client: %s %hu", cflare_socket_ip(sock), cflare_socket_port(sock));
					cflare_request_process_socket(req, sock);
				cflare_socket_delete(sock);
			}
			cflare_listener_delete(listener);
			cflare_request_delete(req);
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
