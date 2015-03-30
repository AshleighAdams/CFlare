
#include <cflare/util.h>

#include <time.h>

float64_t cflare_time()
{
#	ifdef TIME_UTC
		struct timespec ts;
#		ifdef CLOCK_MONOTONIC
			clock_gettime(CLOCK_MONOTONIC, &ts);
#		else
			timespec_get(&ts, TIME_UTC);
#		endif
		return (float64_t)ts.tv_sec + (float64_t)ts.tv_nsec / 1000.0 / 1000.0 / 1000.0;
#	else
		cflare_notimp();
#	endif
}
