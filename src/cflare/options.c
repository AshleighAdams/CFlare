
#include "cflare/options.h"

#include "cflare/hashtable.h"

int original_argc;
char** original_argv;

cflare_hashtable* opt_hashtable;
cflare_hashtable* env_hashtable;
char**            args;
size_t            args_count;
char*             self;

void cflare_options_load(int argc, char** argv)
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
			size_t len = strlen(arg);
			if(arg[1] != '-')
			{
				for(size_t n = 1; n < len; n++)
					cflare_hashtable_set(opt_hashtable,
						cflare_hash_compute(arg + n, 1), "", strlen(""));
			}
			else
			{
				len -= 2;
				arg += 2;
				char* name = arg;
				size_t name_len = 0;
				
				while(name_len < len)
				{
					if(name[name_len] == '=')
						break;
					name_len += 1;
				}
				
				char* value;
				size_t value_len;
				
				if(len == name_len)
				{
					value_len = 0;
				}
				else
				{
					value = name + name_len + 1/*=*/;
					value_len = len - name_len - 1/*=*/;
				}
				
				cflare_hashtable_set(opt_hashtable,
					cflare_hash_compute(name, name_len),
					value, value_len);
			}
		}
		else
		{
			args[args_count] = argv[i];
			args_count += 1;
		}
	}
	
	// TODO: load enviroment variables
}

void clfare_options_unload()
{
	cflare_hashtable_delete(opt_hashtable);
	cflare_hashtable_delete(env_hashtable);
	
	free(args);
}

int64_t cflare_options_integer(const char** name, int64_t fallback)
{
	cflare_notimp();
}

double64_t cflare_options_number(const char** name, double64_t fallback)
{
	cflare_notimp();
}

const char* cflare_options_string(const char** name, const char* fallback)
{
	cflare_notimp();
}

size_t cflare_options_argument_count()
{
	return args_count;
}

const char* cflare_options_executable()
{
	return self;
}

const char* cflare_options_argument(size_t position)
{
	if(position >= args_count)
		return 0;
	return (const char*)args[position];
}

