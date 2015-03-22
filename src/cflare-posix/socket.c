#include "cflare/socket.h"

#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#include <math.h>
#include <errno.h>

typedef struct cflare_socket
{
	int fd;
	bool connected;
	char* ip;
	int16_t port;
} cflare_socket;

typedef struct cflare_listener
{
	int fd;
	bool listening;
	char* addr;
	int16_t port;
} cflare_listener;

static void set_blocking(int fd, bool block)
{
	int flags = fcntl(fd, F_GETFL, 0);
	assert(flags != -1);
	
	int newflags = block
		? flags | O_NONBLOCK
		: flags & ~O_NONBLOCK;
	
	if(newflags != flags) // we can avoid a syscall
	{
		int ret = fcntl(fd, F_SETFL, newflags);
		assert(ret != -1);
	}
}

static void set_socket_timeout(int fd, double64_t timeout)
{
	struct timeval to;
	to.tv_sec = 0;
	to.tv_usec = 0;
	
	set_blocking(fd, timeout != 0.0);
	
	if(timeout > 0.0)
	{
		to.tv_sec = floor(timeout);
		to.tv_usec = (timeout - (double64_t)to.tv_sec) * 1000.0 /*ms*/ * 1000.0 /*us*/;
	}
	setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &to, sizeof(to));
	setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &to, sizeof(to));
}

cflare_listener* cflare_socket_listen(const char* address, uint16_t port)
{
	struct addrinfo* resv = 0x0;
	
	if(address && address[0] == '*' && address[1] == '\0')
		address = 0;
	
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV /*to pass a port*/;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = AF_UNSPEC; // ipv4 or ipv6
	
	int error = 0;
	char strport[10]; // 65k max, so "65535\0"
	snprintf(strport, 10, "%hu", port);
	
	if((error = getaddrinfo(address, strport, &hints, &resv)))
	{
		cflare_warn("listen(): getaddrinfo: %s", gai_strerror(error));
		if(resv)
			freeaddrinfo(resv);
		return 0;
	}
	
	int fd;
	struct addrinfo* addr = 0x0;
	int so_reuseaddr = true;
	
	for(struct addrinfo* info = resv; info != 0; info = info->ai_next)
	{
		if((fd = socket(info->ai_family, info->ai_socktype, 0)) < 0)
			continue;
		
		setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &so_reuseaddr, sizeof(so_reuseaddr));
		if(bind(fd, info->ai_addr, info->ai_addrlen) != 0)
		{
			cflare_warn("listen(): bind: %s", strerror(errno));
			close(fd);
			continue;
		}
		
		addr = info;
		break;
	}
	
	if(!addr)
	{
		cflare_warn("listen(): no suitable address found", address);
		freeaddrinfo(resv);
		return 0x0;
	}
	
	
	char ip[INET6_ADDRSTRLEN];
	{ // get the IP address into a string
		if((error = getnameinfo(addr->ai_addr, addr->ai_addrlen, ip, sizeof(ip), 0, 0, NI_NUMERICHOST)))
		{
			cflare_warn("listen(): failed to get IP: %s", gai_strerror(error));
			freeaddrinfo(resv);
			close(fd);
			return 0x0;
		}
	}
	freeaddrinfo(resv);
	
	{ // listen
		if(listen(fd, 0) != 0)
		{
			cflare_warn("listen(): listen: %s", strerror(errno));
			close(fd);
			return 0x0;
		}
	}
	
	cflare_listener* ret = malloc(sizeof(cflare_listener));
	ret->fd = fd;
	ret->listening = true;
	ret->addr = strdup(ip);
	ret->port = port;
	
	return ret;
}

void cflare_listener_delete(cflare_listener* listener)
{
	cflare_listener_close(listener);
	free(listener->addr);
	free(listener);
}

cflare_socket* cflare_listener_accept(cflare_listener* listener)
{
	cflare_notimp();
	return 0;
}
uint16_t cflare_listener_port(cflare_listener* listener)
{
	return listener->port;
}
const char* cflare_listener_address(cflare_listener* listener)
{
	return listener->addr;
}

void cflare_listener_close(cflare_listener* listener)
{
	if(!listener->listening)
		return;
	close(listener->fd);
	listener->fd = -1;
	listener->listening = false;
}

// this function isn't in the header because you really shouldn't be calling it...
// the assumtion is made the filedescriptor is already connected
cflare_socket* cflare_socket_new(int fd, const char* ip, uint16_t port)
{
	assert(fd >= 0);
	struct stat statbuf;
	fstat(fd, &statbuf);
	assert(S_ISSOCK(statbuf.st_mode));
	
	// attempt to do some default socket options
	set_socket_timeout(fd, -1); // block forever by default
	
	cflare_socket* sock = malloc(sizeof(cflare_socket));
	sock->fd = fd;
	sock->ip = strdup(ip);
	sock->port = port;
	sock->connected = true;
	return sock;
}

cflare_socket* cflare_socket_connect(const char* host, uint16_t port, double64_t timeout)
{
	cflare_notimp();
	return 0;
}
void cflare_socket_delete(cflare_socket* socket)
{
	cflare_socket_close(socket);
	free(socket->ip);
	free(socket);
}

const char* cflare_socket_ip(cflare_socket* socket)
{
	return socket->ip;
}
uint16_t cflare_socket_port(cflare_socket* socket)
{
	return socket->port;
}

bool cflare_socket_connected(cflare_socket* socket)
{
	return socket->connected;
}

bool cflare_socket_read(cflare_socket* socket, uint8_t* buffer, size_t* read, size_t buffer_length)
{
	cflare_notimp();
	*read = 0;
	return false;
}

bool cflare_socket_readline(cflare_socket* socket, uint8_t* buffer, size_t* read, size_t buffer_length)
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

bool cflare_socket_writeline(cflare_socket* socket, const uint8_t* buffer, size_t buffer_length)
{
	cflare_notimp();
	return false;
}

void cflare_socket_flush(cflare_socket* socket)
{
	int flag = 1; 
	setsockopt(socket->fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));
	flag = 0;
	setsockopt(socket->fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));
}

void cflare_socket_close(cflare_socket* socket)
{
	if(socket->connected)
		return;
	close(socket->fd);
	socket->connected = false;
	socket->fd = -1;
}

void cflare_socket_timeout(cflare_socket* socket, double64_t timeout)
{
	 set_socket_timeout(socket->fd, timeout);
}
