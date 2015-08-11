#include "cflare/socket.h"
#include "cflare/coroutine.h"

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


#include <lthread.h>

typedef struct cflare_socket
{
	int fd;
	bool connected;
	char* ip;
	int16_t port;
	uint64_t timeout;
	
	uint8_t* requeue_buffer;
	uint8_t* requeue;
	size_t requeue_length;
} cflare_socket;

typedef struct cflare_listener
{
	int fd;
	bool listening;
	char* addr;
	int16_t port;
	uint64_t timeout;
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

static void set_socket_timeout(int fd, float64_t timeout)
{
	struct timeval to;
	to.tv_sec = 0;
	to.tv_usec = 0;
	
	set_blocking(fd, timeout != 0.0);
	
	if(timeout > 0.0)
	{
		to.tv_sec = floor(timeout);
		to.tv_usec = (timeout - (float64_t)to.tv_sec) * 1000.0 /*ms*/ * 1000.0 /*us*/;
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

cflare_listener* cflare_socket_listen(const char* address, uint16_t port, uint64_t opts)
{
	struct addrinfo* resv = 0x0;
	
	if(address && address[0] == '*' && address[1] == '\0')
		address = 0;
	
	struct addrinfo hints = {0};
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
		if((fd = lthread_socket(info->ai_family, info->ai_socktype, 0)) < 0)
			continue;
		
		
		if(opts & CFLARE_SOCKET_OPT_REUSEADDR)
			setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &so_reuseaddr, sizeof(so_reuseaddr));
		if(opts & CFLARE_SOCKET_OPT_REUSEPORT)
		#ifndef SO_REUSEPORT // this is a new feature, might not be in older kernels
			cflare_warn("socket option: SO_REUSEPORT not supported");
		#else
			setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &so_reuseaddr, sizeof(so_reuseaddr));
		#endif
		
		if(bind(fd, info->ai_addr, info->ai_addrlen) != 0)
		{
			cflare_warn("listen(): bind: %s", strerror(errno));
			lthread_close(fd);
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
			lthread_close(fd);
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
			lthread_close(fd);
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
	
	if((remote_fd = lthread_accept(listener->fd, (struct sockaddr*)&remote_addr, &remote_addrlen)) < 0)
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

void cflare_listener_timeout(cflare_listener* listener, float64_t timeout)
{
	set_socket_timeout(listener->fd, timeout);
}

void cflare_listener_close(cflare_listener* listener)
{
	if(!listener->listening)
		return;
	lthread_close(listener->fd);
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
	//set_socket_timeout(fd, -1); // block forever by default
	
	int so_keepalive = 1;
	if(setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &so_keepalive, sizeof(so_keepalive)) < 0)
		cflare_warn("socket: failed to enable heartbeat for %s: %s", ip, strerror(errno));
	
	cflare_socket* sock = malloc(sizeof(cflare_socket));
	sock->fd = fd;
	sock->ip = strdup(ip);
	sock->port = port;
	sock->connected = true;
	sock->timeout = UINT64_MAX;
	sock->requeue = sock->requeue_buffer = 0x0;
	sock->requeue_length = 0;
	return sock;
}

cflare_socket* cflare_socket_connect(const char* host, uint16_t port, float64_t timeout)
{
	struct addrinfo* resv = 0x0;
	
	struct addrinfo hints = {0};
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
	if((fd = lthread_socket(resv->ai_family, resv->ai_socktype, 0)) < 0)
	{
		cflare_warn("connect(): socket: %s", strerror(errno));
		freeaddrinfo(resv);
		return 0x0;
	}
	
	if(lthread_connect(fd, resv->ai_addr, resv->ai_addrlen, timeout * 1000) != 0)
	{
		// connect() should always block, so this error is a timeout...
		if(errno == EINPROGRESS)
			errno = ETIMEDOUT;
		
		int _errno = errno;
		cflare_warn("connect(): connect: %s", strerror(errno));
		freeaddrinfo(resv);
		lthread_close(fd);
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
	free(socket->requeue_buffer);
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


static inline ssize_t requeue_read(cflare_socket* socket, uint8_t* buffer, size_t buffer_length)
{
	if(!socket->requeue)
		return 0;
	
	size_t can_read = buffer_length < socket->requeue_length ? buffer_length : socket->requeue_length;
	memcpy(buffer, socket->requeue, can_read);
	socket->requeue_length -= can_read;
	socket->requeue += can_read;
	
	if(socket->requeue_length == 0)
	{
		free(socket->requeue_buffer);
		socket->requeue = socket->requeue_buffer = 0x0;
	}
	
	return can_read;
}


bool cflare_socket_read(cflare_socket* socket, uint8_t* buffer, size_t buffer_length, size_t* read)
{
	if(!socket->connected)
	{
		if(read) *read = 0;
		errno = ENOTCONN;
		return false;
	}
	
	// what should be done if we have a buffer_length of 0?
	assert(buffer_length > 0);
	
	ssize_t read_count;
	
	if(socket->requeue_length > 0)
	{
		// read from the requeue and then fix the buffer values
		read_count = requeue_read(socket, (uint8_t*)buffer, buffer_length);
		buffer += read_count;
		buffer_length -= read_count;
		
		ssize_t r = lthread_recv(socket->fd, buffer, buffer_length, 0, socket->timeout);
		if(r > 0) // ignore the error for now, it will fail in a subsequent read call
			read_count += r;
	}
	else
	{
		read_count = lthread_recv(socket->fd, buffer, buffer_length, 0, socket->timeout);
	}
	
	if(read_count <= 0)
	{
		switch(read_count)
		{
		case 0:
			errno = ECONNRESET;
			socket->connected = false;
			// fall through
		case -1:
			errno = EAGAIN;
			break;
		case -2:
			errno = ETIMEDOUT;
			break;
		default:
			socket->connected = false;
			break;
		}
		
		if(read) *read = 0;
		return false;
	}
	
	if(read) *read = read_count;
	return true;
}

int cflare_socket_wait_write(cflare_socket* socket, float64_t timeout)
{
	uint64_t to;
	if(timeout < 0)
		to = 0;
	else if(timeout == 0)
		to = 1;
	else
		to = timeout * 1000;
	return lthread_wait_write(socket->fd, to);
}
int cflare_socket_wait_read(cflare_socket* socket, float64_t timeout)
{
	uint64_t to;
	if(timeout < 0)
		to = 0;
	else if(timeout == 0)
		to = 1;
	else
		to = timeout * 1000;
	return lthread_wait_write(socket->fd, to);
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
	// use MSG_NOSIGNAL to prevent us from being terminated in the event the remote end hangs up
	int error; // = lthread_send(socket->fd, buffer, buffer_length, MSG_NOSIGNAL);
	//float64_t to = cflare_time();
	
	while(true)
	{
		error = lthread_send(socket->fd, buffer, buffer_length, 0);
		
		if(error == buffer_length) // we wrote it all
			break;
		else if(error >= 0)
		{
			buffer += error;
			buffer_length -= error;
			cflare_coroutine_yield();
		}
		else
		{
			if(errno != EWOULDBLOCK && errno != EAGAIN)
			{
				cflare_warn("write: 1: send: %s (%d)", strerror(errno), errno);
				cflare_socket_close(socket);
				socket->connected = false;
				return false;
			}
			else
			{
				int wait = lthread_wait_write(socket->fd, socket->timeout);
				if(wait < 0)
				{
					cflare_warn("write: 2: send: %s (%d)", strerror(errno), wait);
					cflare_socket_close(socket);
					socket->connected = false;
					return false;
				}
			}
		}
	}
	
	return true;
}

bool cflare_socket_read_line(cflare_socket* socket, char* buffer, size_t buffer_length, size_t* read)
{
	assert(socket && buffer && buffer_length > 0);
	if(!socket->connected)
	{
		if(read) *read = 0;
		errno = ENOTCONN;
		return false;
	}
	
	bool gotnewline = false;
	size_t len = 0;
	
	while(len < buffer_length - 1)
	{
		ssize_t r;
		
		if(socket->requeue_length > 0)
			r = requeue_read(socket, (uint8_t*)buffer + len, 1);
		else
			r = lthread_recv(socket->fd, buffer + len, 1, 0, socket->timeout);
		
		if(r <= 0)
		{
			switch(r)
			{
			case 0:
				errno = ECONNRESET;
				socket->connected = false;
				// fall through
			case -1:
				errno = EAGAIN;
				break;
			case -2:
				errno = ETIMEDOUT;
				break;
			default:
				socket->connected = false;
				break;
			}
			
			break;
		}
		
		len++;
		if(buffer[len - 1] == '\n')
		{
			gotnewline = true;
			len--;
			break;
		}
	}
	
	// now fix the buffer to remove all \r s
	size_t src, dst;
	for(src = 0, dst = 0; src < len; src++)
	{
		if(buffer[src] == '\r')
			continue;
		buffer[dst] = buffer[src];
		dst++;
	}
	buffer[dst] = '\0';
	if(read) *read = dst;
	return gotnewline;
}

bool cflare_socket_write_line(cflare_socket* socket, const char* buffer, size_t buffer_length)
{
	assert(socket && buffer);
	if(!socket->connected)
	{
		errno = ENOTCONN;
		return false;
	}
	
	char newline = {'\n'};
	
	if(!cflare_socket_write(socket, (uint8_t*)buffer, buffer_length) || !cflare_socket_write(socket, (uint8_t*)&newline, sizeof(newline)))
		return false;
	
	return socket->connected;
}

void cflare_socket_flush(cflare_socket* socket)
{
	int flag = 1; 
	setsockopt(socket->fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));
	flag = 0;
	setsockopt(socket->fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));
}

void cflare_socket_timeout(cflare_socket* socket, float64_t timeout)
{
	if(timeout == -1)
		socket->timeout = UINT64_MAX;
	else if(timeout == 0)
		socket->timeout = 0;
	else
		socket->timeout = timeout * 1000;
}

void cflare_socket_disconnect(cflare_socket* socket)
{
	if(!socket->connected)
		return;
	socket->connected = false;
	
	int error = shutdown(socket->fd, /*SHUT_RDWR*/SHUT_WR);
	if(error == -1)
	{
		cflare_warn("disconnect: shutdown: %s", strerror(errno));
	}
	
	char buff[1024];
	ssize_t read = lthread_recv(socket->fd, buff, sizeof(buff), 0, socket->timeout);
	
	if(read < 0)
		cflare_warn("disconnect: read: %s", strerror(errno));
	/*
	while(true)
	{
		read = recv(socket->fd, buff, sizeof(buff), 0);
		cflare_debug("disconnect: read: %i errno: (%s) %i ", (int)read, strerror(errno), errno);
		
		if(read <= 0)
			break;
		if(timeout == -1)
			continue;
		else if(timeout == 0)
			break;
		else if(cflare_time() > to)
			break;
	}
	*/
}

void cflare_socket_close(cflare_socket* socket)
{
	if(socket->fd < 0)
		return;
	
	lthread_close(socket->fd);
	socket->fd = -1;
	socket->connected = false;
}

void cflare_socket_requeue(cflare_socket* socket, const uint8_t* buffer, size_t buffer_length)
{
	uint8_t* new_ptr = malloc(socket->requeue_length + buffer_length);
	memcpy(new_ptr, buffer, buffer_length);
	memcpy(new_ptr + buffer_length, socket->requeue, socket->requeue_length);
	
	free(socket->requeue_buffer);
	socket->requeue_buffer = new_ptr;
	socket->requeue = new_ptr;
	socket->requeue_length = buffer_length + socket->requeue_length;
	cflare_debug("requeueing data"); // to see if it's actually done
}
