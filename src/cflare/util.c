
#include "cflare/util.h"

void cflare_debug_c(const char* str)
{
	fprintf(stderr, "debug: %s\n", str);
}

void cflare_info_c(const char* str)
{
	fprintf(stdout, "info: %s\n", str);
}

void cflare_log_c(const char* str)
{
	fprintf(stdout, "%s\n", str);
}

void cflare_warn_c(const char* str)
{
	fprintf(stderr, "warning: %s\n", str);
}

void cflare_fatal_c(const char* str)
{
	fprintf(stderr, "fatal: %s\n", str);
	abort();
}
