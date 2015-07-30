#ifndef CFLARE_SOCKET_H
#define CFLARE_SOCKET_H

#include <cflare/cflare.h>
#include <cflare/util.h>

typedef struct cflare_socket cflare_socket;
typedef struct cflare_listener cflare_listener;

enum
{
	CFLARE_SOCKET_TIMEOUT_FOREVER = -1,
	CFLARE_SOCKET_TIMEOUT_NOBLOCK = 0
};

#define CFLARE_SOCKET_HOST_ANY "*"
#define CFLARE_SOCKET_PORT_ANY 0

enum
{
	CFLARE_SOCKET_OPT_REUSEADDR = 1 << 0,
	CFLARE_SOCKET_OPT_REUSEPORT = 1 << 1,
	
	CFLARE_SOCKET_OPT_DEFAULT = CFLARE_SOCKET_OPT_REUSEADDR
};

CFLARE_API cflare_listener* cflare_socket_listen(const char* addr, uint16_t port, uint64_t ops);
CFLARE_API void cflare_listener_delete(cflare_listener* listener);

CFLARE_API cflare_socket* cflare_listener_accept(cflare_listener* listener);
CFLARE_API uint16_t cflare_listener_port(cflare_listener* listener);
CFLARE_API const char* cflare_listener_address(cflare_listener* listener);
CFLARE_API void cflare_listener_timeout(cflare_listener* listener, float64_t timeout);
CFLARE_API void cflare_listener_close(cflare_listener* listener);

CFLARE_API cflare_socket* cflare_socket_connect(const char* host, uint16_t port, float64_t timeout);
CFLARE_API void cflare_socket_delete(cflare_socket* socket);

CFLARE_API const char* cflare_socket_ip(cflare_socket* socket);
CFLARE_API uint16_t cflare_socket_port(cflare_socket* socket);
CFLARE_API bool cflare_socket_connected(cflare_socket* socket);

// for coroutines
CFLARE_API int cflare_socket_wait_write(cflare_socket* socket, float64_t timeout);
CFLARE_API int cflare_socket_wait_read(cflare_socket* socket, float64_t timeout);

// these will return false on either connection error, or timeout; if it's a timeout and a read, the partial data will should still be readable.
CFLARE_API bool cflare_socket_read(cflare_socket* socket, uint8_t* buffer, size_t buffer_length, size_t* read_length);
CFLARE_API bool cflare_socket_write(cflare_socket* socket, const uint8_t* buffer, size_t buffer_length);

// same as write, but also appends \n.  Does not stop at a null-byte.
CFLARE_API bool cflare_socket_write_line(cflare_socket* socket, const char* buffer, size_t buffer_length);
// getting to the end of a buffer is considered partial data, and should return false. should ignore '\r's, \n is the char to look for.
// does not stop at a null byte.
CFLARE_API bool cflare_socket_read_line(cflare_socket* socket, char* buffer, size_t buffer_length, size_t* read_length);

CFLARE_API void cflare_socket_flush(cflare_socket* socket);
CFLARE_API void cflare_socket_timeout(cflare_socket* socket, float64_t timeout);
CFLARE_API void cflare_socket_close(cflare_socket* socket);

#endif /* CFLARE_SOCKET_H */

