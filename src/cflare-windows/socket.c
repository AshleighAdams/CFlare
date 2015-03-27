#include "cflare/socket.h"

typedef struct cflare_socket
{
} cflare_socket;

typedef struct cflare_listener
{
} cflare_listener;

cflare_listener* cflare_socket_listen(const char* addr, uint16_t port)
{
	cflare_notimp();
	return 0;
}

void cflare_listener_delete(cflare_listener* listener)
{
	cflare_notimp();
}

cflare_socket* cflare_listener_accept(cflare_listener* listener)
{
	cflare_notimp();
	return 0;
}
uint16_t cflare_listener_port(cflare_listener* listener)
{
	cflare_notimp();
	return 0;
}
const char* cflare_listener_address(cflare_listener* listener)
{
	cflare_notimp();
	return 0;
}
void cflare_listener_close(cflare_listener* listener)
{
	cflare_notimp();
}


cflare_socket* cflare_socket_connect(const char* host, uint16_t port, float64_t timeout)
{
	cflare_notimp();
	return 0x0;
}
void cflare_socket_delete(cflare_socket* socket)
{
	cflare_notimp();
}

const char* cflare_socket_ip(cflare_socket* socket)
{
	cflare_notimp();
	return 0;
}
uint16_t cflare_socket_port(cflare_socket* socket)
{
	cflare_notimp();
	return 0;
}

bool cflare_socket_connected(cflare_socket* socket)
{
	cflare_notimp();
	return false;
}

bool cflare_socket_read(cflare_socket* socket, uint8_t* buffer, size_t buffer_length, size_t* read)
{
	cflare_notimp();
	*read = 0;
	return false;
}

bool cflare_socket_write(cflare_socket* socket, const uint8_t* buffer, size_t buffer_length)
{
	cflare_notimp();
	return false;
}

bool cflare_socket_read_line(cflare_socket* socket, char* buffer, size_t buffer_length, size_t* read)
{
	cflare_notimp();
	*read = 0;
	return false;
}

bool cflare_socket_write_line(cflare_socket* socket, const char* buffer, size_t buffer_length)
{
	cflare_notimp();
	return false;
}

void cflare_socket_flush(cflare_socket* socket)
{
	cflare_notimp();
}

void cflare_socket_close(cflare_socket* socket)
{
	cflare_notimp();
}

void cflare_socket_timeout(cflare_socket* socket, float64_t timeout)
{
	cflare_notimp();
}

void cflare_listener_timeout(cflare_listener* listener, float64_t timeout)
{
	cflare_notimp();
}
