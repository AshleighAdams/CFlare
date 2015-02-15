CFlare [![Build Status](https://travis-ci.org/KateAdams/CFlare.svg?branch=master)](https://travis-ci.org/KateAdams/CFlare)
========

Build depends:

 - POSIX/UNIX (Linux/OS X/Cygwin):
	 - Lua >= 5.1
	 - gcc >= 4.8 | clang >= 3.4 (C11 support)
	 - GNU Make
 - Windows (MSVC)
	 - MSVC 2013
	 - MSVC 2013 Redist

cflare_mutex:
	Windows:
		Depends on Windows Vista or higher, due to SRWLock.
	Posix:
		Depends on pthreads
