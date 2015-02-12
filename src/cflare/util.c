
#include "cflare/util.h"

#include "cflare/hook.h"

void call_log_hook(const char* level, const char* str)
{
	cflare_hookstack* args = cflare_hookstack_new();
	cflare_hookstack* rets = 0;
	
	cflare_hookstack_push_string(args, str);
	cflare_hookstack_push_string(args, level);
	cflare_hook_call("Log", args, rets);
	
	cflare_hookstack_delete(args);
}

void cflare_debug_c(const char* str)
{
	call_log_hook("debug", str);
	fprintf(stderr, "debug: %s\n", str);
	fflush(stderr);
}

void cflare_info_c(const char* str)
{
	call_log_hook("info", str);
	fprintf(stdout, "info: %s\n", str);
	fflush(stdout);
}

void cflare_log_c(const char* str)
{
	call_log_hook("log", str);
	fprintf(stdout, "%s\n", str);
	fflush(stdout);
}

void cflare_warn_c(const char* str)
{
	call_log_hook("warn", str);
	fprintf(stderr, "warning: %s\n", str);
	fflush(stderr);
}

void cflare_fatal_c(const char* str)
{
	call_log_hook("fatal", str);
	fprintf(stderr, "fatal: %s\n", str);
	fflush(stderr);
	abort();
}
