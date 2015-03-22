#ifndef CFLARE_UTIL_H
#define CFLARE_UTIL_H

#include <cflare/cflare.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
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

CFLARE_API bool cflare_tointeger(const char* str, int64_t* out);
CFLARE_API bool cflare_tonumber(const char* str, double64_t* out);

CFLARE_API char* cflare_string_concat_n_c(size_t count, size_t* length, ...);

// from 0 to 8 args
#define CFLARE_VA_NUM_ARGS_IMPL(_1_, _2_, _3_, _4_, _5_, _6_, _7_, _8_, _N_, ...) _N_
#define CFLARE_VA_NUM_ARGS(...) \
	(sizeof(#__VA_ARGS__) == sizeof("")) ? \
		0 \
	: \
		CFLARE_VA_NUM_ARGS_IMPL(__VA_ARGS__, 8, 7, 6, 5, 4, 3, 2, 1)


#define cflare_string_concat_n(_LEN_, ...) \
        cflare_string_concat_n_c(CFLARE_VA_NUM_ARGS(__VA_ARGS__), _LEN_, __VA_ARGS__)

#define cflare_string_concat(...) \
        cflare_string_concat_n(0, __VA_ARGS__)

#ifdef _MSC_VER
	CFLARE_API extern int vasprintf(char** strp, const char* format, va_list ap);
	CFLARE_API extern int asprintf(char** strp, const char* format, ...);
#define CFLARE_ASPRINTF_NEEDS_IMPLIMENT_WINDOWS
#endif

#define cflare_notimp() \
	cflare_fatal("%s:%d: %s(): not implemented", __FILE__, __LINE__, __func__)
#define cflare_notimpf(_FMT_, ...) \
	cflare_fatal("%s:%d: %s(): not implemented: " _FMT_,  __FILE__, __LINE__, __func__, __VA_ARGS__)

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


CFLARE_API char* cflare_format(const char* str, ...);

#endif /* CFLARE_UTIL_H */

