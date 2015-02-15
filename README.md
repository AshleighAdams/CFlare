CFlare [![Build Status](https://travis-ci.org/KateAdams/CFlare.svg?branch=master)](https://travis-ci.org/KateAdams/CFlare)
========

Build depends:

 - POSIX/UNIX (Linux/OS X/Cygwin):
	 - Lua >= 5.1
	 - gcc >= 4.8 | clang >= 3.4 (C11 support)
	 - pthreads
	 - GNU Make
 - Windows:
 	 - Windows NT >= 6.0
 		Required by the implimentation of cflare_mutex, using the newer and
 		faster SRWLock rather than CriticalSection.
	 - MSVC 2013
	 - MSVC 2013 Redist

