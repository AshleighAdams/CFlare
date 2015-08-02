
#include "cflare/request.h"
#include "cflare/response.h"

#include "cflare/headers.h"
#include "cflare/httpstatus.h"
#include "cflare/util.h"
#include "cflare/url.h"
#include "cflare/hook.h"

#ifndef CFLARE_REQUEST_MAX_LENGTH
#define CFLARE_REQUEST_MAX_LENGTH 4096
#endif

#ifndef CFLARE_REQUEST_MAX_HEADERS
#define CFLARE_REQUEST_MAX_HEADERS 32
#endif

#ifndef CFLARE_RESPONSE_MAX_HEADERS
#define CFLARE_RESPONSE_MAX_HEADERS 32
#endif

#ifndef CFLARE_RESPONSE_MAX_LENGTH
#define CFLARE_RESPONSE_MAX_LENGTH 4096
#endif


typedef struct kv_pair
{
	const char* key;
	const char* value;
} kv_pair;

typedef struct cflare_response
{
	cflare_socket* socket;
	uint32_t status_code;
	bool sent;
	
	size_t buffer_pos;
	char buffer[CFLARE_RESPONSE_MAX_LENGTH];
} cflare_response;

typedef struct cflare_request
{
	char buffer[CFLARE_REQUEST_MAX_LENGTH];
	size_t buffer_freespace;
	char* buffer_position;
	size_t buffer_unread; // amount of data on the buffer that's unread
	
	char* method;
	size_t method_len;
	char* path;
	size_t path_len;
	char* query;
	size_t query_len;
	char* version;
	size_t version_len;
	const char* ip;
	uint16_t port;
	
	float64_t time_start, time_start_parse, time_start_postparse;
	
	cflare_request_header headers[CFLARE_REQUEST_MAX_HEADERS];
	size_t headers_count;
	
	const char* host;
	size_t host_len;
	size_t content_length_unread;
	size_t content_length;
	const char* content_type;
	
	cflare_socket* socket;
	cflare_response response;
} cflare_request;

static void quick_response_socket(cflare_socket* sock, uint32_t status, const char* message, bool keep_alive)
{
	const char* status_string = cflare_httpstatus_tostring(status);
	
	size_t msglen = strlen(message);
	
	char* response_headers = cflare_format(
		"HTTP/1.1 %"FMT_UINT32" %"FMT_STRING"\n"
		"Content-Length: %"FMT_INT"\n"
		"Content-Type: text/plain\n"
		"Server: cflare\n"
		"Connection: %"FMT_STRING"\n",
		status, status_string, msglen + 1, keep_alive ? "keep-alive" : "close"
	);
	
	cflare_socket_write_line(sock, response_headers, strlen(response_headers));
	cflare_socket_write_line(sock, message, msglen);
	
	if(!keep_alive)
		cflare_socket_close(sock);
	
	free(response_headers);
}

cflare_request* cflare_request_new()
{
	cflare_request* ret = malloc(sizeof(cflare_request));
	memset(ret, 0, sizeof(cflare_request));
	return ret;
}

void cflare_request_delete(cflare_request* req)
{
	free(req);
}

static inline ssize_t first_of(const char* ptr, size_t len, char value)
{
	for(size_t x = 0; x < len; x++)
		if(ptr[x] == value)
			return x;
	return -1;
}

/*
static void print_query_param(const char* key, size_t keylen, const char* value, size_t valuelen, void* context)
{
	cflare_log("%s = %s", key, value);
}
*/

bool hdr_cache_loaded = false;
struct
{
	cflare_header
		host, connection, content_type, content_length; // cached host
} hdr_cache;

bool cflare_request_process_socket(cflare_request* req, cflare_socket* socket)
{
	req->socket = socket;
	req->buffer_position = req->buffer;
	req->buffer_freespace = sizeof(req->buffer);
	req->time_start = cflare_time();
	
	cflare_response_init(&req->response, socket);
	
	// keep reading until we get \r?\n\r?\n
	size_t nbytes = 0;
	int newlines = 0;
	while(true)
	{
		size_t frag_size;
		char* frag_ptr = req->buffer + nbytes;
		
		if(!cflare_socket_read(socket, (uint8_t*)req->buffer + nbytes, req->buffer_freespace - nbytes, &frag_size))
		{
			quick_response_socket(socket, 400, "recv error", false);
			return false;
		}
		
		nbytes += frag_size;
		
		if(nbytes == req->buffer_freespace)
		{
			quick_response_socket(socket, 413, "request too long", false);
			return false;
		}
		
		// look for the \r\n
		for(size_t i = 0; i < frag_size; i++)
		{
			switch( frag_ptr[i] )
			{
			case '\r':
				break;
			case '\n':
				newlines++;
				if(newlines == 2)
					goto got_part;
				break;
			default:
				newlines = 0; // reset it
				break;
			}
		}
	} got_part:
	
	req->buffer_freespace -= nbytes;
	
	{ // parse method
		req->method = req->buffer_position;
		ssize_t pos = first_of(req->buffer_position, nbytes, ' ');
		
		if(pos < 0 || pos == nbytes)
		{
			quick_response_socket(socket, 400, "invalid request", false);
			return false;
		}
		
		req->method[pos] = '\0';
		req->method_len = pos;
		
		nbytes -= pos + 1;
		req->buffer_position += pos + 1;
	}
	
	{ // parse path
		req->path = req->buffer_position;
		ssize_t pos = first_of(req->buffer_position, nbytes, ' ');
		
		if(pos < 0 || pos == nbytes)
		{
			quick_response_socket(socket, 414, "request URI too long", false);
			return false;
		}
		
		req->path[pos] = '\0';
		req->path_len = pos;
		
		ssize_t query_pos = first_of(req->path, req->path_len, '?');
		if(query_pos < 0)
		{
			req->query = 0x0;
			req->query_len = 0;
		}
		else
		{
			req->path[query_pos] = '\0';
			req->query = req->path + query_pos + 1;
			
			req->path_len = query_pos;
			req->query_len = pos - query_pos - 1;
		}
		
		
		nbytes -= pos + 1;
		req->buffer_position += pos + 1;
	}
	
	{ // parse version
		req->version = req->buffer_position;
		ssize_t pos = first_of(req->buffer_position, nbytes, '\n');
		
		if(pos < 0 || pos == nbytes)
		{
			quick_response_socket(socket, 400, "invalid version", false);
			return false;
		}
		
		if(req->version[pos - 1] == '\r') // look for \r\n
		{
			req->version[pos - 1] = '\0';
			req->version_len = pos - 1;
		}
		else
		{
			req->version[pos] = '\0';
			req->version_len = pos;
		}
		
		nbytes -= pos + 1;
		req->buffer_position += pos + 1;
	}
	
	// some headers we might be interested in
	int64_t content_length = 0x0;
	
	const char* host = 0x0;
	size_t host_len = 0;
	
	const char* content_type = 0x0;
	size_t content_type_len = 0;

	const char* connection = 0x0;
	size_t connection_len = 0;
	
	{ // read the headers
		req->headers_count = 0;
		bool ignored_header = false;
		char* prev_header = 0x0;
		size_t prev_header_len = 0;
		
		while(true)
		{
			char* line = req->buffer_position;
			size_t line_len;
			ssize_t pos = first_of(line, nbytes, '\n');
			
			if(pos < 0)
			{
				quick_response_socket(socket, 400, "invalid header 1", false);
				return false;
			}
			
			nbytes -= pos;
			req->buffer_position += pos;
			
			if(line[pos - 1] == '\r')
			{
				line[pos - 1] = '\0';
				line_len = pos - 1;
			}
			else
			{
				line[pos] = '\0';
				line_len = pos;
			}
			
			if(line_len == 0) // the blank line
				break;
			
			else if(line[0] == ' ' || line[0] == '\t')
			{
				if(!prev_header)
				{
					quick_response_socket(socket, 400, "request header invalid: nothing to continue", false);
					return false;
				}
				if(ignored_header) // we didn't want this header anyway, so we can skip
					continue;
				
				prev_header[prev_header_len] = ' '; // this was a null ptr, but now we need to memmove it
				
				memmove(prev_header + prev_header_len, line + 1, line_len - 1);
				prev_header_len += line_len - 1;
				req->headers[req->headers_count].value_len = prev_header_len; // update the length here
			}
			else
			{
				ssize_t pos_col = first_of(line, line_len, ':');
				
				if(pos_col < 0)
				{
					quick_response_socket(socket, 400, "invalid header 2", false);
					return false;
				}
				
				line[pos_col] = '\0';
				char* key = line;
				//size_t key_len = pos_col;
				
				line += pos_col + 1 + 1; // point the line at the value, one for ':' and one for ' '
				line_len -= pos_col + 1 + 1;
				
				char* value = line;
				size_t value_len = line_len;
				value[value_len] = '\0';
				
				cflare_header hdr = cflare_headers_get(key);
				if(!cflare_headers_valid(hdr))
				{
					ignored_header = true;
					continue;
				}
				else
				{
					if(req->headers_count >= CFLARE_REQUEST_MAX_HEADERS)
					{
						quick_response_socket(socket, 413, "reached max number of headers", false);
						return false;
					}
					
					//cflare_debug("%s = '%s'", key, value);
					req->headers[req->headers_count].header = hdr;
					req->headers[req->headers_count].value = value;
					req->headers[req->headers_count].value_len = value_len;
					req->headers_count++;
					
					// update our important ones
					
					if(cflare_headers_equals(hdr, cflare_headers->host))
					{
						host = value;
						host_len = value_len;
					}
					else if(cflare_headers_equals(hdr, cflare_headers->connection))
					{
						connection = value;
						connection_len = value_len;
					}
					else if(cflare_headers_equals(hdr, cflare_headers->content_type))
					{
						content_type = value;
						content_type_len = value_len;
					}
					else if(cflare_headers_equals(hdr, cflare_headers->content_length))
					{
						if(!cflare_tointeger(value, &content_length))
						{
							quick_response_socket(socket, 400, "bad content-length", false);
							return false;
						}
					}
				}
			}
		}
	}
	
	char major = req->version[5], minor = req->version[7]; // HTTP/5.7
	
	if(major != '1')
	{
		quick_response_socket(socket, 400, "HTTP version not supported", false);
		return false;
	}
	
	switch(minor)
	{
	default:
	case '0':
		break;
	case '1':
		// host is required
		if(!host)
		{
			quick_response_socket(socket, 400, "HTTP version requires host", false);
			return false;
		}
		break;
	case '2':
		// host may be present in the URL authority
		{
			char* path = req->path;
			
			// http://localhost/path
			char* protocol = strstr(path, "://");
			if(protocol)
			{
				char* host_part = protocol + 3;
				char* path_part = strstr(host_part, "/");
				size_t host_part_len = path_part - host_part;
				
				// we need to make some space for the host's null byte
				memmove(host_part - 1, host_part, host_len);
				host_part -= 1;
				host_part[host_len] = '\0';
				
				host = host_part;
				host_len = host_part_len;
				
				req->path_len = (req->path + req->path_len) - path_part;
				req->path = path_part;
			}
		}
	}
	
	req->host = host;
	req->host_len = host_len;
	req->ip = cflare_socket_ip(socket);
	req->port = cflare_socket_port(socket);
	req->buffer_unread = nbytes; // the number of bytes still in the buffer
	req->content_length = req->content_length_unread = content_length; // the number of bytes of content left to deliver
	
	//cflare_debug("%s %s %s", req->method, req->path, req->version);
	//cflare_url_parse_query(req->query, req->query_len, &print_query_param, 0x0);
	
	//bool keep_alive = true;
	//quick_response_socket(socket, 200, "all is good", keep_alive);
	
	
	cflare_hookstack* args = cflare_hookstack_new();
	cflare_hookstack* rets = cflare_hookstack_new();
	cflare_hookstack_push_pointer(args, "cflare_request", req, 0x0/*del*/, 0x0/*del ctx*/);
	cflare_hookstack_push_pointer(args, "cflare_response", &req->response, 0x0/*del*/, 0x0/*del ctx*/);
	
		cflare_hook_call("Request", args, rets);
		
		int64_t result;
		bool got = cflare_hookstack_get_integer(rets, 0, &result);
	
	cflare_hookstack_delete(rets);
	cflare_hookstack_delete(args);
	
	if(!got)
	{
		quick_response_socket(socket, 500, "no request handler could handle this request!", false);
		return false;
	}
	
	return result == 0;
}


// properties

cflare_socket* cflare_request_socket(cflare_request* req)
{
	return req->socket;
}

const char* cflare_request_method(cflare_request* req)
{
	return req->method;
}

const char* cflare_request_path(cflare_request* req)
{
	return req->path;
}

const char* cflare_request_query(cflare_request* req)
{
	return req->query;
}

const char* cflare_request_version(cflare_request* req)
{
	return req->version;
}

const char* cflare_request_ip(cflare_request* req)
{
	return req->ip;
}

uint16_t cflare_request_port(cflare_request* req)
{
	return req->port;
}

const char* cflare_request_host(cflare_request* req)
{
	return req->host;
}

cflare_request_header* cflare_request_headers(cflare_request* req)
{
	return req->headers;
}

// for reading the content part

bool cflare_request_content_has(cflare_request* req)
{
	return req->content_length != 0;
}

const char* cflare_request_content_type(cflare_request* req)
{
	return req->content_type;
}

size_t cflare_request_content_length(cflare_request* req)
{
	return req->content_length;
}

bool cflare_request_content_chunk(cflare_request* req, char* buffer, size_t buffer_len, size_t* read)
{
	*read = 0;
	
	// see if data is in the buffers we've already read
	if(req->buffer_unread > 0)
	{
		size_t readable = buffer_len > req->buffer_unread ? buffer_len : req->buffer_unread; // find the smallest
		
		memcpy(buffer, req->buffer_position, readable);
		req->buffer_position += readable;
		req->buffer_unread -= readable;
		
		*read = readable;
		
		buffer += readable;
		buffer_len -= readable;
		
		if(buffer_len == 0) // ran out of buffer room
			return true;
	}
	
	size_t readsock;
	bool ret = cflare_socket_read(req->socket, (uint8_t*)buffer, buffer_len, &readsock);
	*read += readsock;
	
	return ret;
}

// response part


cflare_response* cflare_response_new()
{
	cflare_response* ret = malloc(sizeof(cflare_response));
	ret->socket = 0x0;
	ret->status_code = 0x0;
	ret->buffer_pos = 0;
	
	return ret;
}

void cflare_response_init(cflare_response* r, cflare_socket* sock)
{
	r->socket = sock;
	r->status_code = 0;
	r->buffer_pos = 0;
	r->sent = false;
}

cflare_socket* cflare_response_socket(cflare_response* req)
{
	return req->socket;
}

// headers—including cookies—must last until the finializer
void cflare_response_status(cflare_response* res, uint32_t status)
{
	res->status_code = status;
}

void cflare_response_cookie(cflare_response* res, const char* domain, const char* key, const char* value, float64_t valid_for, bool https_only)
{
	cflare_notimp();
}

void cflare_response_header(cflare_response* res, cflare_header hdr, const char* value)
{
	const char* key = hdr.name;
	
	// todo: canonicalize headers?
	size_t key_len = strlen(key);
	size_t val_len = strlen(value);
	
	size_t req = key_len + val_len + 3/*: \n*/;
	
	if(req > CFLARE_RESPONSE_MAX_LENGTH - res->buffer_pos - 1) // we need one for the newline too
	{
		cflare_warn("header: response buffer too small");
		return;
	}
	
	char* ptr = res->buffer + res->buffer_pos;
	
	memcpy(ptr, key, key_len);
	ptr += key_len;
	
	ptr[0] = ':';
	ptr[1] = ' ';
	ptr += 2;
	
	memcpy(ptr, value, val_len);
	ptr += val_len;
	ptr[val_len] = '\n';
}

static inline bool send_headers(cflare_response* res)
{
	if(res->sent)
	{
		cflare_warn("attempted to send more than one response");
		return false;
	}
	res->sent = true;
	
	const char* status_string = cflare_httpstatus_tostring(res->status_code);
	if(!status_string)
	{
		cflare_warn("status code %"FMT_UINT32" is unknown", res->status_code);
		status_string = "Unknown";
	}
	
	char status[128];
	ssize_t status_len = snprintf(status, sizeof(status), "HTTP/1.1 %"FMT_UINT32" %s\n", res->status_code, status_string);
	assert(status_len >= 0);
	
	if(!cflare_socket_write(res->socket, (uint8_t*)status, status_len))
		return false;
	
	res->buffer[res->buffer_pos] = '\n';
	res->buffer_pos += 1;
	
	return cflare_socket_write(res->socket, (uint8_t*)res->buffer, res->buffer_pos);
}

bool cflare_response_data(cflare_response* res, const uint8_t* buffer, size_t buffer_len)
{
	char len[32]; len[0] = '\0';
	
	snprintf(len, sizeof(len), "%"FMT_SIZE, buffer_len);
	cflare_response_header(res, cflare_headers->content_length, len);
	
	if(!send_headers(res))
		return false;
	return cflare_socket_write(res->socket, buffer, buffer_len);
}

bool cflare_response_file(cflare_response* res, const char* path)
{
	return false;
}
