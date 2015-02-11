#include "cflare/util.h"

#include <stdio.h>
#include <stdarg.h>

#ifdef CFLARE_ASPRINTF_NEEDS_IMPLIMENT_WINDOWS
int vasprintf(char** strp, const char* format, va_list ap)
{
	int count;
	// Find out how long the resulting string is
	count = _vscprintf(format, ap);
	if (count == 0)
	{
		return strdup("");
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
	return vsprintf(*strp, format, ap);
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