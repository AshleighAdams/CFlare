#ifndef CFLARE_H
#define CFLARE_H

#include <stddef.h>

#ifndef CFLARE_API
	#ifdef _MSC_VER
		#define CFLARE_API __declspec(dllexport)
	#else
		#define CFLARE_API // not needed
	#endif
#endif

#ifndef CFLARE_API_NORETURN
	#ifdef _MSC_VER
		#define CFLARE_API_NORETURN
	#else
		#define CFLARE_API_NORETURN __attribute__((noreturn))
	#endif
#endif

#ifdef _MSC_VER
	#define __func__ __FUNCTION__
#endif

CFLARE_API size_t cflare_version_major();
CFLARE_API size_t cflare_version_minor();
CFLARE_API size_t cflare_version_patch();
CFLARE_API const char* cflare_version();

CFLARE_API void cflare_load(int argc, char** argv);
CFLARE_API void cflare_unload();

CFLARE_API const char* cflare_cfgpath();
CFLARE_API const char* cflare_libpath();

#endif /* CFLARE_H */

