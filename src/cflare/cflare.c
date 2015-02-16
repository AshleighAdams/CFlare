#include "cflare/cflare.h"

#include <cflare/util.h>
#include <cflare/handle.h>
#include <cflare/hook.h>
#include <cflare/options.h>



size_t cflare_version_major()
{
	#ifdef CFLARE_VERSION_MAJOR
	return CFLARE_VERSION_MAJOR;
	#else
	return 0;
	#endif
}

size_t cflare_version_minor()
{
	#ifdef CFLARE_VERSION_MINOR
	return CFLARE_VERSION_MINOR;
	#else
	return 0;
	#endif
}

size_t cflare_version_patch()
{
	#ifdef CFLARE_VERSION_PATCH
	return CFLARE_VERSION_PATCH;
	#else
	return 0;
	#endif
}

const char* cflare_version()
{
	#if defined CFLARE_VERSION_MAJOR && defined CFLARE_VERSION_MINOR && defined CFLARE_VERSION_PATCH
		#define CF_VAL(x) #x
		#define CF_STRINGIFY(x) CF_VAL(x)
			return CF_STRINGIFY(CFLARE_VERSION_MAJOR) "." CF_STRINGIFY(CFLARE_VERSION_MINOR) "." CF_STRINGIFY(CFLARE_VERSION_PATCH);
		#undef CF_STRINGIFY
		#undef CF_VAL
	#else
		return "unknown";
	#endif
}

void cflare_load(int argc, char** argv)
{
	cflare_handle_load();
	cflare_hook_load();
	cflare_options_load(argc, argv);
}

void cflare_unload()
{
	clfare_options_unload();
	cflare_hook_unload();
	cflare_handle_unload();
}
