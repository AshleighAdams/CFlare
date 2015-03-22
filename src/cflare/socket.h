#ifndef CFLARE_SOCKET_H
#define CFLARE_SOCKET_H

#include <cflare/cflare.h>
#include <cflare/util.h>

typedef struct cflare_socket cflare_socket;
typedef struct cflare_listener cflare_listener;

CFLARE_API cflare_listener* cflare_socket_listen(const char* addr, uint16_t port);
CFLARE_API void cflare_listener_delete(cflare_listener* listener);

CFLARE_API cflare_socket* cflare_listener_accept(cflare_listener* listener);
CFLARE_API uint64_t cflare_listener_port(cflare_listener* listener);
CFLARE_API const char* cflare_listener_address(cflare_listener* listener);
CFLARE_API void cflare_listener_close(cflare_listener* listener);

CFLARE_API cflare_socket* cflare_socket_connect(const char* host, uint16_t port);
CFLARE_API void cflare_socket_delete(cflare_socket* socket);

CFLARE_API const char* cflare_socket_ip(cflare_socket* socket);
CFLARE_API uint16_t cflare_socket_port(cflare_socket* socket);
CFLARE_API bool cflare_socket_connected(cflare_socket* socket);

// these will return false on either connection error, or timeout; if it's a timeout and a read, the partial data will should still be readable.
CFLARE_API bool cflare_socket_read(cflare_socket* socket, uint8_t* buffer, size_t* read, size_t buffer_length);
CFLARE_API bool cflare_socket_readline(cflare_socket* socket, uint8_t* buffer, size_t* read, size_t buffer_length);
CFLARE_API bool cflare_socket_write(cflare_socket* socket, const uint8_t* buffer, size_t buffer_length);
CFLARE_API bool cflare_socket_writeline(cflare_socket* socket, const uint8_t* buffer, size_t buffer_length); // same as write, but also appends \n

CFLARE_API void cflare_socket_flush(cflare_socket* socket);
CFLARE_API void cflare_socket_close(cflare_socket* socket);

// set a timeout for all operations
// -1 = forever, 0 = no delay, >0 = seconds
CFLARE_API void cflare_socket_timeout(cflare_socket* socket, double64_t timeout);

#endif /* CFLARE_SOCKET_H */

