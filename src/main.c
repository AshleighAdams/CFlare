
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

#include <cflare/coroutine.h>
#include <cflare/thread.h>
#include <cflare/mutex.h>

#include <errno.h>
#include <signal.h>

/*
static bool unload_test(const cflare_hookstack* args, cflare_hookstack* rets, void* context)
{
	cflare_debug("Inside Unload hook! args: %p; rets: %p", (void*)args, (void*)rets);
	//cflare_hookstack_push_integer(rets, 1337);
	cflare_hookstack_push_number(rets, 1337);
	return 0;
}
*/

typedef struct workers_msg {
	cflare_socket* socket;
	cflare_coroutinecondition* read_cond;
	cflare_coroutinecondition* write_cond;
} workers_msg;

static int64_t workers = 0;
static void* worker_thread(void* context)
{
	cflare_coroutine_autoname();
	cflare_coroutine_detach();
	
	workers_msg* msg = context;
	cflare_request* req = cflare_request_new();
	
	workers++;
	cflare_debug("total workers: %"FMT_INT64, workers);
	
	while(true)
	{
		int why = cflare_coroutinecondition_wait(msg->read_cond, 10);
		if(why != 0) // timeout
			break;
		cflare_socket* sock = msg->socket;
		cflare_coroutinecondition_signal(msg->write_cond); // signal we've got the socket
		
		if(!sock) // our thread is terminated
			break;
		
		cflare_socket_timeout(sock, 600);
		if(cflare_socket_wait_write(sock, 0) == -1) // sockets are broken, abandon this thread
			break;

		while(cflare_request_process_socket(req, sock))
			;
		
		cflare_socket_close(sock);
		cflare_socket_delete(sock);
	}
	cflare_request_delete(req);
	
	workers--;
	cflare_debug("total workers: %"FMT_INT64, workers);
	
	return 0x0;
}

static cflare_listener* listener;

static void* main_listen_thread(void* _)
{
	cflare_coroutine_autoname();
	listener = cflare_socket_listen(CFLARE_SOCKET_HOST_ANY, 1025, CFLARE_SOCKET_OPT_DEFAULT);
	
	workers_msg msg; // the msg that's passed to coroutines
	msg.socket = 0x0;
	msg.read_cond = cflare_coroutinecondition_new();
	msg.write_cond = cflare_coroutinecondition_new();
	
	while(true)
	{
		msg.socket = cflare_listener_accept(listener);
		if(!msg.socket)
			break;
		
		
		cflare_coroutinecondition_signal(msg.read_cond);
		if(cflare_coroutinecondition_wait(msg.write_cond, 0.001) == -2)
		{
			cflare_debug("no thread to accept, creating new");
			cflare_coroutine_new(&worker_thread, &msg);
			
			while(cflare_coroutinecondition_wait(msg.write_cond, 0) == -2)
				cflare_coroutinecondition_signal(msg.read_cond);
			// timed out
		}
		else
			msg.socket = 0x0; // prevent further threads accidentally taking it
	}
	
	cflare_listener_delete(listener);
	return 0x0;
}

static void on_signal(int sig)
{
	if(sig == SIGINT)
	{
		if(listener)
		{
			cflare_log("received SIGINT, shutting down...");
			cflare_listener_close(listener);
			exit(1);
		}
		else
		{
			cflare_fatal("unknown how to handle signal");
		}
	}
}

int main(int argc, char** argv)
{
	if(signal(SIGINT, on_signal) == SIG_ERR)
		cflare_warn("couldn't install SIGINT handler");
	
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
	else if(cflare_options_argument(0) && strcmp(cflare_options_argument(0), "listen") == 0)
	{
		cflare_coroutine* co = cflare_coroutine_new(&main_listen_thread, 0x0);
		cflare_coroutine_detach2(co);
		
		cflare_coroutine_scheduler_run();
	}
	else
	{
		/*
		cflare_hook_add("Unload", "unload test", 0, &unload_test, 0);
	
		cflare_handle hd = cflare_handle_new("test", 0, 0, 0);
		printf("hd = %zu\n", hd);
		cflare_handle_unreference(hd);
		*/
		/*{
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
		}*/
		
		{ // socket test
			/*
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
			*/
			
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
