#ifndef CFLARE_H
#define CFLARE_H

#ifndef CFLARE_API
	#ifdef _MSC_VER
		#define CFLARE_API __dllspec(dllexport)
	#else
		#define CFLARE_API // not needed
	#endif
#endif

#ifndef CFLARE_API_NORETURN
	#ifdef _MSC_VER
		#define CFLARE_API_NORETURN
	#else
		#define CFLARE_API_NORETURN __attribute__((noreturn))
	#endif
#endif

#endif /* CFLARE_H */

