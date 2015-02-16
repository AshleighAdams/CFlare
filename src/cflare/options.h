#ifndef CFLARE_OPTIONS_H
#define CFLARE_OPTIONS_H

#include <cflare/cflare.h>
#include <cflare/util.h>

/*
Options are in the form of:
	option name
Which will check the command line argument:
	--option-name
And default to the enviroment variable:
	OPTION_NAME
If that evan fails, then the default passed to the get argument will be returned.

Everything after "--" will be interpreted as an argument, and not an option

`main`'s 0th argument is not included in the argument, and is instead read via _executable();

*/

CFLARE_API void cflare_options_load(int argc, char** argv);
CFLARE_API void clfare_options_unload();

CFLARE_API uint8_t cflare_options_boolean(const char* name, uint8_t fallback);
CFLARE_API int64_t cflare_options_integer(const char* name, int64_t fallback);
CFLARE_API double64_t cflare_options_number(const char* name, double64_t fallback);
CFLARE_API const char* cflare_options_string(const char* name, const char* fallback);

CFLARE_API size_t cflare_options_argument_count();
CFLARE_API const char* cflare_options_executable();
CFLARE_API const char* cflare_options_argument(size_t position);

#endif /* CFLARE_OPTIONS_H */

