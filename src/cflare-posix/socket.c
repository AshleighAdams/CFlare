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
		? flags & ~O_NONBLOCK
		: flags | O_NONBLOCK;
	
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

static int gaierr_to_errno(int error)
{
	switch(error)
	{
	case EAI_ADDRFAMILY:
		return EHOSTUNREACH;
	case EAI_AGAIN:
		return EAGAIN;
	case EAI_BADFLAGS:
		return EIO;
	case EAI_FAIL:
	case EAI_NONAME:
		#ifdef EHOSTNOTFOUND
		return EHOSTNOTFOUND;
		#else
		return ENXIO; // device or address not found
		//return EHOSTUNREACH; // no route to host
		#endif
	case EAI_FAMILY:
		return EAFNOSUPPORT;
	case EAI_MEMORY:
		return ENOMEM;
	case EAI_NODATA:
		return ENODATA;
	case EAI_SERVICE:
		return EPROTONOSUPPORT;
	case EAI_SOCKTYPE:
		return ESOCKTNOSUPPORT;
	case EAI_SYSTEM:
		return errno;
	default:
		return 0;
	}
}

cflare_listener* cflare_socket_listen(const char* address, uint16_t port)
{
	struct addrinfo* resv = 0x0;
	
	if(address && address[0] == '*' && address[1] == '\0')
		address = 0;
	
	struct addrinfo hints = {};
	hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV /*to pass a port*/;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = AF_UNSPEC; // ipv4 or ipv6
	
	/*
		In future, could possibly use a scheme to pass into serv, such as "http" from "http://blah.com", via transforming addr to:
		"http\0//"(address now points here)
	*/
	int error = 0;
	char strport[NI_MAXSERV];
	snprintf(strport, NI_MAXSERV, "%hu", port);
	
	if((error = getaddrinfo(address, strport, &hints, &resv)))
	{
		cflare_warn("listen(): getaddrinfo: %s", gai_strerror(error));
		errno = gaierr_to_errno(error);
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
		cflare_warn("listen(): no suitable address found for %s", address);
		errno = EHOSTUNREACH;
		freeaddrinfo(resv);
		return 0x0;
	}
	
	char ip[NI_MAXHOST];
	{ // get the IP address into a string
		if((error = getnameinfo(addr->ai_addr, addr->ai_addrlen, ip, sizeof(ip), strport, sizeof(strport), NI_NUMERICHOST | NI_NUMERICSERV)))
		{
			cflare_warn("listen(): failed to get IP: %s", gai_strerror(error));
			errno = gaierr_to_errno(error);
			int _errno = errno;
			freeaddrinfo(resv);
			close(fd);
			errno = _errno;
			return 0x0;
		}
		int iport = port;
		sscanf(strport, "%d", &iport);
		port = iport;
	}
	freeaddrinfo(resv);
	
	{ // listen
		if(listen(fd, 0) != 0)
		{
			cflare_warn("listen(): listen: %s", strerror(errno));
			int _errno = errno;
			close(fd);
			errno = _errno;
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

cflare_socket* cflare_socket_new(int fd, const char* ip, uint16_t port);
cflare_socket* cflare_listener_accept(cflare_listener* listener)
{
	struct sockaddr_storage remote_addr;
	socklen_t remote_addrlen = sizeof(remote_addr);
	
	int remote_fd, error;
	
	if((remote_fd = accept(listener->fd, (struct sockaddr*)&remote_addr, &remote_addrlen)) < 0)
	{
		cflare_warn("accept(): accept: %s", strerror(errno));
		return 0x0; // error is left in errno
	}
	
	char ip[NI_MAXHOST];
	char strport[NI_MAXSERV];
	{ // get the IP address into a string
		if((error = getnameinfo((struct sockaddr*)&remote_addr, remote_addrlen, ip, sizeof(ip), strport, sizeof(strport), NI_NUMERICHOST | NI_NUMERICSERV)))
		{
			cflare_warn("accept(): failed to get IP/port: %s", gai_strerror(error));
			errno = gaierr_to_errno(error);
			return 0x0;
		}
	}
	
	int port = 0;
	sscanf(strport, "%d", &port);
	assert(port >= 0/*UINT16_MIN*/ && port <= UINT16_MAX);
	
	return cflare_socket_new(remote_fd, ip, (uint16_t)port);
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
	struct addrinfo* resv = 0x0;
	
	struct addrinfo hints = {};
	hints.ai_flags = AI_NUMERICSERV /*to pass a port*/;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = AF_UNSPEC; // ipv4 or ipv6
	
	int error = 0;
	char strport[NI_MAXSERV];
	snprintf(strport, NI_MAXSERV, "%hu", port);
	
	if((error = getaddrinfo(host, strport, &hints, &resv)))
	{
		cflare_warn("connect(): getaddrinfo: %s", gai_strerror(error));
		errno = gaierr_to_errno(error);
		if(resv)
			freeaddrinfo(resv);
		return 0x0;
	}
	
	char ip[NI_MAXHOST];
	{ // get the IP address into a string
		if((error = getnameinfo(resv->ai_addr, resv->ai_addrlen, ip, sizeof(ip), 0, 0, NI_NUMERICHOST)))
		{
			cflare_warn("connect(): failed to get IP: %s", gai_strerror(error));
			errno = gaierr_to_errno(error);
			freeaddrinfo(resv);
			return 0x0;
		}
	}
	
	int fd;
	if((fd = socket(resv->ai_family, resv->ai_socktype, 0)) < 0)
	{
		cflare_warn("connect(): socket: %s", strerror(errno));
		freeaddrinfo(resv);
		return 0x0;
	}
	
	set_socket_timeout(fd, timeout);
	
	if(connect(fd, resv->ai_addr, resv->ai_addrlen) != 0)
	{
		// connect() should always block, so this error is a timeout...
		if(errno == EINPROGRESS)
			errno = ETIMEDOUT;
		
		int _errno = errno;
		cflare_warn("connect(): connect: %s", strerror(errno));
		freeaddrinfo(resv);
		close(fd);
		errno = _errno;
		return 0x0;
	}
	
	freeaddrinfo(resv);
	
	return cflare_socket_new(fd, ip, port);
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

bool cflare_socket_read(cflare_socket* socket, uint8_t* buffer, size_t buffer_length, size_t* read)
{
	if(!socket->connected)
	{
		if(read) *read = 0;
		errno = ENOTCONN;
		return false;
	}
	ssize_t read_count = recv(socket->fd, buffer, buffer_length, 0);
	
	if(read_count < 0)
	{
		// these 2 just mean a timeout
		if(errno != EAGAIN && errno != EWOULDBLOCK)
			socket->connected = false;
		if(read) *read = 0;
		// errno from recv is kept
		return false;
	}
	
	*read = read_count;
	return true;
}

// this is very inefficient, need to do it in blocks with peak in the future, rather than reading char by char...
bool cflare_socket_readline(cflare_socket* socket, char* buffer, size_t buffer_length, size_t* read)
{
	assert(socket && buffer && buffer_length > 0);
	if(!socket->connected)
	{
		if(read) *read = 0;
		errno = ENOTCONN;
		return false;
	}
	
	char* ptr = buffer;
	size_t len = 0;
	errno = 0;
	
	while(true)
	{
		ssize_t read_count = recv(socket->fd, ptr, 1, 0);
		if(read_count < 0)
		{
			if(errno != EAGAIN && errno != EWOULDBLOCK)
				socket->connected = false;
			break;
		}
		
		if(ptr[0] == '\n')
			break;
		else if(ptr[0] == '\r') // this char is ignored
			continue;
		
		len           += 1;
		ptr           += 1;
		buffer_length -= 1;
		
		if(buffer_length <= 1) // we need at least 1 for the null-char
		{
			errno = ENOBUFS;
			break;
		}
	}
	
	*read = len; // don't include the null-pointer
	buffer[len] = '\0';
	return errno == 0;
}

bool cflare_socket_write(cflare_socket* socket, const uint8_t* buffer, size_t buffer_length)
{
	assert(socket && buffer);
	if(!socket->connected)
	{
		errno = ENOTCONN;
		return false;
	}
	
	// todo track this
	send(socket->fd, buffer, buffer_length, 0);
	return true;
}

bool cflare_socket_writeline(cflare_socket* socket, const char* buffer, size_t buffer_length)
{
	assert(socket && buffer);
	if(!socket->connected)
	{
		errno = ENOTCONN;
		return false;
	}
	// should we error if a \n is in the buffer?
	
	char newline = {'\n'};
	send(socket->fd, buffer, buffer_length, MSG_MORE);
	send(socket->fd, &newline, sizeof(newline), 0);
	return true;
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

void cflare_listener_timeout(cflare_listener* listener, double64_t timeout)
{
	set_socket_timeout(listener->fd, timeout);
}
