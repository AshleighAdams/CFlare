
#include "unit-test.h"

#include <cflare/cflare.h>
#include <cflare/buffer.h>
#include <cflare/hook.h>
#include <cflare/hashtable.h>
#include <cflare/httpstatus.h>
#include <cflare/headers.h>
#include <cflare/thread.h>
#include <cflare/mutex.h>

// if a test fails, it doesn't need to free memory.

static int test_buffer()
{
	cflare_buffer* buff = cflare_buffer_new(CFLARE_BUFFER_NOCOPY | CFLARE_BUFFER_NULLCHAR);
	
	const char* a = "First";
	const char* b = "Second";
	const char* c = "Third";
	
	cflare_buffer_append(buff, (uint8_t*)a, strlen(a));
	cflare_buffer_append(buff, (uint8_t*)b, strlen(b));
	cflare_buffer_append(buff, (uint8_t*)c, strlen(c));
	
	unit_test_part("append");
	if(cflare_buffer_length(buff) != strlen(a) + strlen(b) + strlen(c))
		return 1;
	
	char* result = (char*)cflare_buffer_build(buff);
	
	unit_test_part("build");
	if(strcmp(result, "FirstSecondThird") != 0)
		return 1;
	
	free(result);
	cflare_buffer_delete(buff);
	
	return 0;
}

static bool test_hook_abc(const cflare_hookstack* args, cflare_hookstack* rets, void* context)
{
	int* val = (int*)context;
	*val = 7357;
	return 0;
}
static int test_hook()
{
	int x = 0;
	cflare_hook_add("Abc", "test", 0, &test_hook_abc, &x);
	cflare_hook_call("Abc", 0, 0);
	
	unit_test_part("context");
	if(x !=  7357)
		return 1;
	
	return 0;
}

static int test_linkedlist()
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
		if(cflare_linkedlist_count(list) != 3)
		{
			cflare_linkedlist_delete(list);
			return 1;
		}
		
		unit_test_part("remove");
		// remove the 2nd one
		cflare_linkedlist_iterator iter = cflare_linkedlist_get_iterator(list);
		cflare_linkedlist_iterator_next(&iter);
		cflare_linkedlist_iterator_next(&iter);
		cflare_linkedlist_remove(list, iter.value);
		
		if(cflare_linkedlist_count(list) != 2)
		{
			cflare_linkedlist_delete(list);
			return 1;
		}
	}
	cflare_linkedlist_delete(list);
	return 0;
}

static int test_hashtable()
{
	cflare_hashtable* map = cflare_hashtable_new();
	{
		const char* key = "Content-Length";
		size_t key_len = strlen(key);
		const char* value = "1337";
		size_t value_len = strlen(key);
		
		cflare_hashtable_set(map, cflare_hash_compute(key, key_len), value, value_len);
		unit_test_part("buckets set");
		if(cflare_hashtable_bucketscount(map) == 0)
			return 1;
		unit_test_part("set");
		if(cflare_hashtable_count(map) != 1)
			return 1;
		
		cflare_hashtable_set(map, cflare_hash_compute(key, key_len), value, value_len);
		unit_test_part("double set");
		if(cflare_hashtable_count(map) != 1)
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
		if(cflare_hashtable_bucketscount(map) != 64)
			return 1;
		
		unit_test_part("locate 2");
		if(!cflare_hashtable_get(map, cflare_hash_compute(key, key_len), (void**)&get_value, &get_value_len))
			return 1;
		
		unit_test_part("locate 2: value");
		if(strcmp(get_value, value) != 0)
			return 1;
		
		unit_test_part("remove");
		cflare_hashtable_set(map, cflare_hash_compute(key, key_len), 0, 0);
		if(cflare_hashtable_count(map) != 0)
			return 1;
		
	}
	cflare_hashtable_delete(map);
	return 0;
}

static int test_util()
{
	int64_t val;
	
	unit_test_part("tointeger: base10");
	if(!cflare_tointeger("1337", &val) || val != 1337)
		return 1;
	if(!cflare_tointeger("+123", &val) || val != 123)
		return 1;
	if(!cflare_tointeger("-100", &val) || val != -100)
		return 1;
	
	unit_test_part("tointeger: base16");
	if(!cflare_tointeger("0x1337", &val) || val != 0x1337)
		return 1;
	if(!cflare_tointeger("+0x123", &val) || val != 0x123)
		return 1;
	if(!cflare_tointeger("-0x100", &val) || val != -0x100)
		return 1;
	
	return 0;
}

static bool test_httpstatus_code(uint32_t code, const char* status)
{
	const char* got = cflare_httpstatus_tostring(code);
	if(strcmp(got, status) != 0)
		return false;
	
	if(cflare_httpstatus_fromstring(status) != code)
		return false;
	
	return true;
}
static int test_httpstatus()
{
	#define TEST_CODE(_CODE_, _STATUS_) \
		unit_test_part(#_CODE_" <-> "_STATUS_);\
		if(!test_httpstatus_code(_CODE_, _STATUS_)) \
			return 1;
	
	TEST_CODE(200, "OK");
	TEST_CODE(304, "Not Modified");
	TEST_CODE(404, "Not Found");
	TEST_CODE(500, "Internal Server Error");
	
	return 0;
}

static int test_headers()
{
	unit_test_part("Host");
	cflare_header Host = cflare_headers_get("Host");
	if(Host.id <= 0)
		return 1;
	
	unit_test_part("Host == host");
	cflare_header host = cflare_headers_get("host");
	if(!cflare_headers_equals(Host, host))
		return 1;
	
	
	unit_test_part("Upgrade");
	cflare_header Upgrade = cflare_headers_get("Upgrade");
	if(Upgrade.id <= 0)
		return 1;
	
	unit_test_part("Upgrade == upgrade");
	cflare_header upgrade = cflare_headers_get("upgrade");
	if(!cflare_headers_equals(Upgrade, upgrade))
		return 1;
	
	if(cflare_headers_equals(host, upgrade))
		return 1;
	
	unit_test_part("ensure");
	cflare_header x_nope = cflare_headers_ensure("X-Nope");
	if(x_nope.id <= 0)
		return 1;
	if(strcmp(x_nope.name, "X-Nope") != 0)
		return 1;
	
	return 0;
}

bool inside = false;
void* test_threads_thread(void* context)
{
	inside = true;
	char** selfptr = context;
	*selfptr = strdup("Hello, context!");
	return strdup("Hello, return!");
}

int test_threads()
{
	char* ctx = 0;
	char* ret = 0;
	
	cflare_thread* t = cflare_thread_new(&test_threads_thread, &ctx);
	ret = cflare_thread_join(t);
	
	unit_test_part("entered");
	if(!inside)
		return 1;
	
	unit_test_part("context");
	if(!ctx || strcmp(ctx, "Hello, context!") != 0)
		return 1;
	
	unit_test_part("return");
	if(!ret || strcmp(ret, "Hello, return!") != 0)
		return 1;
	
	cflare_thread_delete(t);
	free(ctx);
	free(ret);
	
	return 0;
}

typedef struct test_mutex_container
{
	cflare_mutex* mutex;
	cflare_thread* thread;
	int64_t* value;
	int64_t add;
} test_mutex_container;
void* test_mutex_adder(void* val)
{
	test_mutex_container* container = val;
	
	for(int64_t i = 0; i < container->add; i++)
	{
		cflare_mutex_lock(container->mutex);
		*container->value += 1;
		cflare_mutex_unlock(container->mutex);
	}
	return 0;
}
bool test_mutexes_normal()
{
	#define num_threads 10
	#define num_add 5000
	test_mutex_container containers[num_threads];
	
	// this will cause a LOT of cache misses (via writes to the same cache line
	// from different threads/cores), but allows us to test that the mutex works.
	// to avoid the cache miss, avoid mutating memory shared by multipul threads.
	// In other words, add the result at the end of the worker thread from a
	// local thread-specific variable.
	int64_t result = 0;
	cflare_mutex* mutex = cflare_mutex_new(CFLARE_MUTEX_PLAIN);
	
	for(size_t i = 0; i < num_threads; i++)
	{
		test_mutex_container* container = containers + i;
		container->add = num_add;
		container->value = &result;
		container->mutex = mutex;
		container->thread = cflare_thread_new(&test_mutex_adder, container);
	}
	
	for(size_t i = 0; i < num_threads; i++)
	{
		test_mutex_container* container = containers + i;
		cflare_thread_join(container->thread);
		cflare_thread_delete(container->thread);
	}
	
	cflare_mutex_delete(mutex);
	
	
		
	if(result != num_threads * num_add)
		return false;
	
	return true;
}
typedef struct test_mutexesrw_container
{
	cflare_rwmutex* mutex;
	int64_t value;
	size_t reads, writes;
	bool running;
} test_mutexesrw_container;
void* test_mutexes_reader(void* arg)
{
	test_mutexesrw_container* data = arg;
	bool okay = true;
	size_t reads = 0;
	while(data->running && okay)
	{
		cflare_rwmutex_read_lock(data->mutex);
		if(data->value == 0) // we failed.
			okay = false;
		//fputc('-', stdout); fflush(stdout);
		cflare_rwmutex_read_unlock(data->mutex);
		reads += 1;
		cflare_thread_sleep(0.01);
	}
	
	cflare_rwmutex_write_lock(data->mutex);
	data->reads += reads;
	cflare_rwmutex_write_unlock(data->mutex);
	return (void*)okay;
}
void* test_mutexes_writer(void* arg)
{
	test_mutexesrw_container* data = arg;
	bool okay = true;
	size_t writes = 0;
	
	while(data->running && okay)
	{
		cflare_rwmutex_write_lock(data->mutex);
		if(data->value == 0)
			okay = false;
		data->value = 0;
		cflare_thread_sleep(0.01);
		data->value = (int64_t)&okay;
		//fputc('#', stdout); fflush(stdout);
		cflare_rwmutex_write_unlock(data->mutex);
		writes += 1;
		cflare_thread_sleep(0.1);
	}
	
	cflare_rwmutex_write_lock(data->mutex);
	data->writes += writes;
	cflare_rwmutex_write_unlock(data->mutex);
	return (void*)okay;
}
bool test_mutexes_rw()
{
	#define num_readers 10
	#define num_writers 10
	test_mutexesrw_container container;
	cflare_thread* readers[num_readers];
	cflare_thread* writers[num_writers];
	
	container.mutex = cflare_rwmutex_new(CFLARE_MUTEX_PLAIN);
	container.value = 1;
	container.reads = 0;
	container.writes = 0;
	container.running = true;
	
	for(size_t i = 0; i < num_readers; i++)
		readers[i] = cflare_thread_new(&test_mutexes_reader, &container);
	for(size_t i = 0; i < num_writers; i++)
		writers[i] = cflare_thread_new(&test_mutexes_writer, &container);
	
	cflare_thread_sleep(1);
	container.running = false;
	
	bool rerr = false, werr = false;
	
	for(size_t i = 0; i < num_readers; i++)
	{
		cflare_thread* t = readers[i];
		if(!(bool)cflare_thread_join(t))
			rerr = true;
		cflare_thread_delete(t);
	}
	
	for(size_t i = 0; i < num_writers; i++)
	{
		cflare_thread* t = writers[i];
		if(!(bool)cflare_thread_join(t))
			werr = true;
		cflare_thread_delete(t);
	}
	
	cflare_rwmutex_delete(container.mutex);
	
	double perc = (double)container.writes / (double)container.reads * 100.0;
	cflare_log("\t\treads: %lu; writes: %lu; writes (%%): %.2lf%%", container.reads, container.writes, perc);
	return !rerr && !werr;
}
int test_mutexes()
{
	unit_test_part("normal");
	if(!test_mutexes_normal())
		return 1;
	unit_test_part("readwrite");
	if(!test_mutexes_rw())
		return 1;
	return 0;
}

static int test_failed;
static const char* msg;
void unit_test_part(const char* location)
{
	msg = location;
	cflare_log("\tchecking %s...", msg);
}
static void test_function(const char* testname, int(*testfunc)())
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
	test_function("util", &test_util);
	test_function("httpstatus", &test_httpstatus);
	test_function("headers", &test_headers);
	test_function("threads", &test_threads);
	test_function("mutexes", &test_mutexes);
	
	if(test_failed)
		cflare_log("One or more unit tests failed.");
	else
		cflare_log("All unit tests passed.");
	
	return test_failed;
}
