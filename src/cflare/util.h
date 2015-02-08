#ifndef LUAFLARE_UTL_H
#define LUAFLARE_UTL_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

typedef float double32_t;
typedef double double64_t;
typedef long double double128_t;

void cflare_debug_c(const char* str);
void cflare_info_c(const char* str);
void cflare_log_c(const char* str);
void cflare_warn_c(const char* str);
void cflare_fatal_c(const char* str);

#define cflare_debug(...) \
	do \
	{ \
		char* __fmt_string; \
		asprintf(&__fmt_string, __VA_ARGS__); \
		if(0) printf(__VA_ARGS__); \
		cflare_debug_c(__fmt_string); \
		free(__fmt_string); \
	} while(0)

#define cflare_info(...) \
	do \
	{ \
		char* __fmt_string; \
		asprintf(&__fmt_string, __VA_ARGS__); \
		if(0) printf(__VA_ARGS__); \
		cflare_info_c(__fmt_string); \
		free(__fmt_string); \
	} while(0)

#define cflare_log(...) \
	do \
	{ \
		char* __fmt_string; \
		asprintf(&__fmt_string, __VA_ARGS__); \
		if(0) printf(__VA_ARGS__); \
		cflare_log_c(__fmt_string); \
		free(__fmt_string); \
	} while(0)

#define cflare_warn(...) \
	do \
	{ \
		char* __fmt_string; \
		asprintf(&__fmt_string, __VA_ARGS__); \
		if(0) printf(__VA_ARGS__); \
		cflare_warn_c(__fmt_string); \
		free(__fmt_string); \
	} while(0)

#define cflare_fatal(...) \
	do \
	{ \
		char* __fmt_string; \
		asprintf(&__fmt_string, __VA_ARGS__); \
		if(0) printf(__VA_ARGS__); \
		cflare_fatal_c(__fmt_string); \
		free(__fmt_string); \
	} while(0)

#endif /* LUAFLARE_UTL_H */

