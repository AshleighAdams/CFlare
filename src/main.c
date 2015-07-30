
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

typedef struct thread_data
{
	size_t id;
	bool running;
	cflare_socket* socket;
	cflare_thread* thread;
	
	cflare_condition* wait_cond;
	cflare_mutex* wait_mutex;
} thread_data;
thread_data* thread_datas;


static void* worker_thread(void* context)
{
	thread_data* td = context;
	cflare_request* req = cflare_request_new();
	
	while(td->running)
	{
		cflare_mutex_lock(td->wait_mutex);
			while(td->socket == 0x0)
			{
				if(td->running)
					cflare_condition_wait(td->wait_cond, td->wait_mutex);
				else
				{
					cflare_mutex_unlock(td->wait_mutex);
					goto end_root_while;
				}
			}
		cflare_mutex_unlock(td->wait_mutex);
		
		cflare_socket* sock = td->socket;
		cflare_socket_timeout(sock, 60);
	
		while(cflare_socket_connected(sock))
			cflare_request_process_socket(req, sock);
	
		cflare_socket_delete(sock);
		td->socket = 0x0;
	} end_root_while:
	
	cflare_request_delete(req);
	
	return 0x0;
}

static cflare_listener* listener;

static void main_listen_thread()
{
	// setup the threads
	size_t threads = 10;
	
	thread_datas = malloc(sizeof(thread_data) * threads);
	
	for(size_t tid = 0; tid < threads; tid++)
	{
		thread_data* data = thread_datas + tid;
		data->running = true;
		data->id = tid;
		data->socket = 0x0;
		data->wait_cond = cflare_condition_new();
		data->wait_mutex = cflare_mutex_new(CFLARE_MUTEX_PLAIN);
		data->thread = cflare_thread_new(&worker_thread, data);
	}
	
	listener = cflare_socket_listen(CFLARE_SOCKET_HOST_ANY, 1025);
	
	size_t current_thread = 0;
	
	while(true)
	{
		size_t tries = 0;
		while((thread_datas + current_thread)->socket != 0)
		{
			current_thread = (current_thread + 1) % threads;
			tries++;
			
			if(tries > threads)
			{
				tries = 0;
				cflare_thread_sleep(0.01);
			}
		}
		
		cflare_socket* sock = cflare_listener_accept(listener);
		if(!sock)
			break;
		
		thread_data* td = thread_datas + current_thread;
		cflare_mutex_lock(td->wait_mutex);
			td->socket = sock;
			cflare_condition_signal(td->wait_cond, td->wait_mutex);
		cflare_mutex_unlock(td->wait_mutex);
	}
	
	cflare_listener_delete(listener);
	
	for(size_t tid = 0; tid < threads; tid++)
	{
		thread_data* data = thread_datas + tid;
		
		data->running = false;
		
		cflare_mutex_lock(data->wait_mutex);
			cflare_condition_signal(data->wait_cond, data->wait_mutex);
		cflare_mutex_unlock(data->wait_mutex);
		
		cflare_thread_join(data->thread);
		
		cflare_thread_delete(data->thread);
		cflare_mutex_delete(data->wait_mutex);
		cflare_condition_delete(data->wait_cond);
	}
	free(thread_datas);
}

static void on_signal(int sig)
{
	if(sig == SIGINT)
	{
		if(listener)
		{
			cflare_log("received SIGINT, shutting down...");
			cflare_listener_close(listener);
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
		main_listen_thread();
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
