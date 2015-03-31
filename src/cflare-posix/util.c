
#include <cflare/util.h>

#include <time.h>

bool first = true;
float64_t start_time = -1;

float64_t cflare_time()
{
#	ifdef TIME_UTC
		struct timespec ts;
#		ifdef CLOCK_MONOTONIC
			clock_gettime(CLOCK_MONOTONIC, &ts);
#		else
			timespec_get(&ts, TIME_UTC);
#		endif
		
		float64_t t = (float64_t)ts.tv_sec + (float64_t)ts.tv_nsec / 1000.0 / 1000.0 / 1000.0;
		if(first)
		{
			first = false;
			start_time = t;
		}
		return t - start_time;
#	else
		cflare_notimp();
		return -1;
#	endif
}
