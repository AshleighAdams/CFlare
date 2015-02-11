
#include "cflare/options.h"

#include "cflare/hashtable.h"

int original_argc;
char** original_argv;

cflare_hashtable* opt_hashtable;
cflare_hashtable* env_hashtable;
char**            args;
size_t            args_count;
char*             self;

CFLARE_API void cflare_options_load(int argc, char** argv)
{
	opt_hashtable  = cflare_hashtable_new();
	env_hashtable  = cflare_hashtable_new();
	
	self = argv[0];
	
	args_count = 0;
	args = malloc(sizeof(char**) * argc); // this is the abs max
	
	for(int i = 1; i < argc; i++)
	{
		char* arg = argv[i];
		if(arg[0] == '-')
		{
			cflare_fatal("option parsing not implimented");
		}
		else
		{
			args[args_count] = argv[i];
			args_count += 1;
		}
	}
}

CFLARE_API void clfare_options_unload()
{
	cflare_hashtable_delete(opt_hashtable);
	cflare_hashtable_delete(env_hashtable);
	
	free(args);
}

CFLARE_API int64_t cflare_options_integer(const char** name, int64_t fallback)
{
	cflare_notimp();
}

CFLARE_API double64_t cflare_options_number(const char** name, double64_t fallback)
{
	cflare_notimp();
}

CFLARE_API const char* cflare_options_string(const char** name, const char* fallback)
{
	cflare_notimp();
}

CFLARE_API size_t cflare_options_argument_count()
{
	return args_count;
}

CFLARE_API const char* cflare_options_executable()
{
	return self;
}

CFLARE_API const char* cflare_options_argument(size_t position)
{
	if(position >= args_count)
		return 0;
	return (const char*)args[position];
}

