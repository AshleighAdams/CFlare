#ifndef CFLARE_UTIL_H
#define CFLARE_UTIL_H

#include <cflare/cflare.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>

typedef float double32_t;
typedef double double64_t;
typedef long double double128_t;

typedef void(cflare_deleter)(void* data, void* context);

CFLARE_API void cflare_debug_c(const char* str);
CFLARE_API void cflare_info_c(const char* str);
CFLARE_API void cflare_log_c(const char* str);
CFLARE_API void cflare_warn_c(const char* str);
CFLARE_API CFLARE_API_NORETURN void cflare_fatal_c(const char* str);

CFLARE_API uint8_t cflare_tointeger(const char* str, int64_t* out);
CFLARE_API uint8_t cflare_tonumber(const char* str, double64_t* out);

CFLARE_API char* cflare_string_add_n(size_t count, size_t* length, ...);

#ifdef _MSC_VER
	CFLARE_API extern int vasprintf(char** strp, const char* format, va_list ap);
	CFLARE_API extern int asprintf(char** strp, const char* format, ...);
#define CFLARE_ASPRINTF_NEEDS_IMPLIMENT_WINDOWS
#endif

#define cflare_notimp() \
	cflare_fatal("%s(): not implemented", __func__)

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

#endif /* CFLARE_UTIL_H */

