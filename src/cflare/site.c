
#include "cflare/site.h"

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <dlfcn.h>


typedef struct cflare_site
{
	char* name;
} cflare_site;

/*

typedef _Bool(fn)(void*, void*, int,const char*);

_Bool forward(void* r1, void* r2, fn* func, void** args)
{
	int arg0 = *(int*)args[0];
	char* arg1 = *(char**)args[1];
	return func(r1, r2, arg0, arg1);
}


// cc -nostdlib -shared tmp.c -o tmp.so

*/


cflare_site* cflare_site_new(const char* domain)
{
	cflare_site* ret = malloc(sizeof(cflare_site));
	ret->name = strdup(domain);
	return ret;
}

void cflare_site_delete(cflare_site* site)
{
	free(site->name);
	free(site);
}

#define MAX_ARGS 16

static inline void write_part(int fd, const char* str)
{
	write(fd, str, strlen(str));
}

void cflare_site_add_pattern(cflare_site* site, const char* path, void* callback)
{
	// find out the argument types
	char types[MAX_ARGS + 1];
	size_t types_count = 0;
	bool finalized = false;
	
	for(size_t n = 0; n < strlen(path); n++)
	{
		if(path[n] == '%')
		{
			n++;
			char type = path[n];
			
			if(path[n] == '%')
				continue;
			
			if(types_count >= MAX_ARGS)
			{
				cflare_warn("site: %s: add_pattern: too many arguments", site->name);
				return;
			}
			else if(finalized)
			{
				cflare_warn("site: %s: add_pattern: argument cannot come after %%*", site->name);
				return;
			}
			
			switch(type)
			{
			case '*': // until the end, must be last
				finalized = true;
				type = 's';
			case 's': // anything but /
			case 'i': // int64: (+|-)?[0-9]*
			case 'u': // uint64: [0-9]*
			case 'f': // float64: (+|-)?[0-9]*\.[0-9]*
				break;
			default:
				cflare_warn("site: %s: add_pattern: unknown argument type '%c'", site->name, type);
				return;
			}
			
			types[types_count] = type;
			types_count++;
		}
	}
	types[types_count] = '\0';
	
	// make the temp files
	int fd;
	char src[] = "/tmp/cflare-site-XXXXXX";
	char lib[] = "/tmp/cflare-site-XXXXXX.so";
	
	if((fd = mkstemp(src)) < 0)
	{
		cflare_warn("site: %s: add_pattern: couldn't create temporary source file", site->name);
		return;
	}
	
	size_t start = strlen("/tmp/cflare-site-");
	size_t len = strlen("XXXXXX");
	memcpy(lib + start, src + start, len);
	
	// generate the code
	char symbol[] = "_cflare_siteforward_XXXXXXXXXXXXXXXX";
	strcpy(symbol + strlen("_cflare_siteforward_"), types);
	
	write_part(fd, "#include <stdint.h>\n");
	write_part(fd, "_Bool ");
	write_part(fd, symbol);
	
	write_part(fd, "(_Bool(*ptr)(void*, void*");
	for(size_t i = 0; i < types_count; i++)
	{
		switch(types[i])
		{
		case 's':
			write_part(fd, ", const char*");
			break;
		case 'u':
			write_part(fd, ", uint64_t");
			break;
		case 'i':
			write_part(fd, ", int64_t");
			break;
		case 'f':
			write_part(fd, ", double");
			break;
		default:
			assert(false);
			return;
		}
	}
	write_part(fd, "), void* req, void* res, void** args)\n{\n");
	write_part(fd, "\treturn ptr(req, res");
	for(size_t i = 0; i < types_count; i++)
	{
		char buff[] = "*(typetypetypetype_t*)args[0000]";
		const char* typestr;
		
		switch(types[i])
		{
		case 's':
			typestr = "const char*";
			break;
		case 'u':
			typestr = "uint64_t";
			break;
		case 'i':
			typestr = "int64_t";
			break;
		case 'f':
			typestr = "double";
			break;
		default:
			assert(false);
			return;
		}
		
		snprintf(buff, sizeof(buff), ", *(%s*)args[%"FMT_SIZE"]", typestr, i);
		write_part(fd, buff);
	}
	write_part(fd, ");\n}\n");
	
	close(fd);
	
	// compile it
	{
		char cmd[50] = "";
		strcat(cmd, "cc -x c -nostdlib -shared ");
		strcat(cmd, src);
		strcat(cmd, " -o ");
		strcat(cmd, lib);
		
		cflare_debug("%s", cmd);
		int ret = system(cmd);
		unlink(src);
		if(ret != 0)
		{
			cflare_warn("site: %s: add_pattern: couldn't compile temporary source file", site->name);
			return;
		}
	}
	
	
	void* handle = dlopen(lib, RTLD_LAZY | RTLD_LOCAL);
	unlink(lib);
	
	if(!handle)
	{
		cflare_warn("site: %s: add_pattern: cannot open generated library", site->name);
		return;
	}
	
	bool(*cb)(void*,void*,void*,void**) = dlsym(handle, symbol);
	
	if(!cb)
	{
		cflare_warn("site: %s: add_pattern: cannot find generated symbol %s", site->name, symbol);
		return;
	}
	
	union {
		const char* string;
		int64_t integer;
		uint64_t unsigned_integer;
		float64_t real;
	} args[types_count];
	
	void* args_ptrs[types_count];
	
	for(size_t i = 0; i < types_count; i++)
	{
		args_ptrs[i] = args + i;
		
		switch(types[i])
		{
		case 's': args[i].string = "1337"; break;
		case 'i': args[i].integer = -1337; break;
		case 'u': args[i].unsigned_integer = 1337; break;
		case 'f': args[i].real = 1337.1337; break;
		default:
			assert(false);
			return;
		}
	}
	
	bool ret = cb(callback, 0x0, 0x0, args_ptrs);
	cflare_debug("ret = %s", ret ? "true":"false");
	
	dlclose(handle);
}

