#include "cflare/util.h"

#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#include <windows.h>

#ifdef CFLARE_ASPRINTF_NEEDS_IMPLIMENT_WINDOWS
int vasprintf(char** strp, const char* format, va_list ap)
{
	int count;
	// Find out how long the resulting string is
	count = _vscprintf(format, ap);
	if (count == 0)
	{
		*strp = _strdup("");
		return 0;
	}
	else if (count < 0)
	{
		// Something went wrong, so return the error code (probably still requires checking of "errno" though)
		return count;
	}

	assert(strp != NULL);

	// Allocate memory for our string
	*strp = malloc(count + 1);
	if (*strp == NULL)
	{
		cflare_fatal_c("vasprintf: Out of memory!");
		return -1;
	}
	// Do the actual printing into our newly created string
	return vsprintf_s(*strp, count + 1, format, ap);
}

int asprintf(char** strp, const char* format, ...)
{
	va_list ap;
	int count;
	va_start(ap, format);
	count = vasprintf(strp, format, ap);
	va_end(ap);
	return count;
}
#endif

static LARGE_INTEGER freq, start;
static bool time_first = true;

float64_t cflare_time()
{
	LARGE_INTEGER count;
	if(!QueryPerformanceCounter(&count))
		cflare_fatal("QueryPerformanceCounter");
	if(time_first)
	{
		time_first = false;
		if (!QueryPerformanceFrequency(&freq))
			cflare_fatal("QueryPerformanceFrequency");
		start = count;
	}
	return (float64_t)(count.QuadPart - start.QuadPart) / freq.QuadPart;
}

char* cflare_format(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	
	char* ret;
	vasprintf(&ret, fmt, args);
	
	va_end(args);
	return ret;
}
