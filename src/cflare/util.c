
#include "cflare/util.h"

#include "cflare/hook.h"

#include <math.h>

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

int64_t char_to_number(char x)
{
	if(x >= '0' && x <= '9')
		return (int64_t)(x - '0');
	else if(x >= 'a' && x <= 'z')
		return 10 + (int64_t)(x - 'a');
	else if(x >= 'A' && x <= 'Z')
		return 10 + (int64_t)(x - 'A');
	else
		return -1;
}

uint8_t cflare_tointeger(const char* str, int64_t* out)
{
	size_t len = strlen(str);
	const char* ptr = str;
	int64_t sign = 1;
	int64_t base = 10;
	
	if(len == 0)
		return 0;
	
	// look for the sign: +1337 or -1337
	if(ptr[0] == '-' || ptr[0] == '+')
	{
		sign = ptr[0] == '+' ? 1 : -1;
		ptr += 1;
		len -= 1;
	}
	
	// find the base
	if(len > 2 && ptr[0] == '0') // posible for a base denotion (0x... 0b... 0o...)
	{
		char cb = ptr[1];
		switch(cb)
		{
		case 'x':
			ptr += 2;
			len -= 2;
			base = 16;
			break;
		case 'b':
			ptr += 2;
			len -= 2;
			base = 2;
			break;
		default:
			break;
		}
	}
	
	if(!len)
		return 0;
	
	int64_t ret = 0;
	
	for(int64_t i = 0, b = len - 1; i < len; i++, b--)
	{
		int64_t n = char_to_number(ptr[i]);
		if(i < 0 || n >= base) // check if it's outside our base range
			return 0;
		ret += pow(base, b) * n;
	}
	
	*out = sign * ret;
	return 1;
}

uint8_t cflare_tonumber(const char* str, double64_t* out)
{
	cflare_notimp();
	return 0;
}

