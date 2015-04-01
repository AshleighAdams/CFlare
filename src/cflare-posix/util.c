
#include <cflare/util.h>

#include <stdarg.h>
#include <time.h>

#define SUB_TIMESPEC(_A_, _B_, _R_) \
	do \
	{ \
		_R_.tv_sec = _A_.tv_sec - _B_.tv_sec; \
		_R_.tv_nsec = _A_.tv_nsec - _B_.tv_nsec; \
		if (_R_.tv_nsec < 0) \
		{ \
			_R_.tv_sec  -= 1; \
			_R_.tv_nsec += 1000 * 1000 * 1000; \
		} \
	} while (0)

#if defined(CLOCK_MONOTONIC) // try the posix one first, as it has monotonic time
	#define FILL_TIMESPEC(_SPEC_) clock_gettime(CLOCK_MONOTONIC, _SPEC_)
#elif defined(TIME_UTC) // then try the ISO C one
	#define FILL_TIMESPEC(_SPEC_) timespec_get(_SPEC_, TIME_UTC)
#else // one doesn't exist, make a dummy struct, and ensure it notimp()s
	struct timespec { long int tv_sec, tv_usec; };
	#define FILL_TIMESPEC(_SPEC_) cflare_notimp()
#endif

struct timespec start;
bool first = true;

float64_t cflare_time()
{
		if(first)
		{
			first = false;
			FILL_TIMESPEC(&start);
		}
		
		struct timespec ts;
		FILL_TIMESPEC(&ts);
		SUB_TIMESPEC(ts, start, ts);
		
		return (float64_t)ts.tv_sec + (float64_t)ts.tv_nsec / 1000.0 / 1000.0 / 1000.0;
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
