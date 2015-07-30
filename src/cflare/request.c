
#include "cflare/request.h"
#include "cflare/headers.h"
#include "cflare/httpstatus.h"
#include "cflare/util.h"
#include "cflare/url.h"

#ifndef CFLARE_REQUEST_MAX_LENGTH
#define CFLARE_REQUEST_MAX_LENGTH 4096
#endif

#ifndef CFLARE_REQUEST_MAX_HEADERS
#define CFLARE_REQUEST_MAX_HEADERS 32
#endif

typedef struct cflare_request
{
	char buffer[CFLARE_REQUEST_MAX_LENGTH];
	size_t buffer_freespace;
	
	char* buffer_position;
	char* method;
	size_t method_len;
	char* path;
	size_t path_len;
	char* query;
	size_t query_len;
	char* version;
	size_t version_len;
	
	float64_t time_start, time_start_parse, time_start_postparse;
	
	struct
	{
		cflare_header header;
		char* value;
		size_t value_len;
	} headers[CFLARE_REQUEST_MAX_HEADERS];
	size_t headers_count;
	
	cflare_socket* socket;
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
		status, status_string, msglen, keep_alive ? "keep-alive" : "close"
	);
	
	cflare_socket_write_line(sock, response_headers, strlen(response_headers));
	cflare_socket_write(sock, (uint8_t*)message, msglen);
	
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

static void print_query_param(const char* key, size_t keylen, const char* value, size_t valuelen, void* context)
{
	cflare_log("%s = %s", key, value);
}

bool cflare_request_process_socket(cflare_request* req, cflare_socket* socket)
{
	req->socket = socket;
	req->buffer_position = req->buffer;
	req->buffer_freespace = sizeof(req->buffer);
	req->time_start = cflare_time();
	
	size_t nbytes;
	if(!cflare_socket_read(socket, req->buffer, req->buffer_freespace, &nbytes) || nbytes == req->buffer_freespace)
	{
		quick_response_socket(socket, 413, "request too long", false);
		return false;
	}
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
	const char* content_length = 0x0;
	const char* content_type = 0x0;
	
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
			
			if(pos < 0 || pos == nbytes)
			{
				quick_response_socket(socket, 400, "invalid header", false);
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
				ssize_t pos_col = first_of(line, nbytes, ':');
				
				if(pos_col < 0)
				{
					quick_response_socket(socket, 400, "invalid header", false);
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
				}
			}
		}
	}
	
	
	//cflare_debug("%s %s %s", req->method, req->path, req->version);
	quick_response_socket(socket, 200, "all is good", true);
	
	return true;
	/*
	size_t read;
	size_t total_read = 0;
	
	if(!cflare_socket_read_line(req->socket, req->buffer_position, req->buffer_freespace, &read))
	{
		if(pos < 0 || read == nbytes)
			quick_response_socket(socket, 414, "request URI too long", false);
		return false;
	}
	
	req->time_start_parse = cflare_time();
	total_read += read;
	
	{ // parse method
		req->method = req->buffer_position;
		ssize_t pos = first_of(req->buffer_position, read, ' ');
		
		if(pos < 0)
		{
			quick_response_socket(socket, 400, "request line invalid (method)", false);
			return false;
		}
		
		req->method[pos] = '\0';
		req->method_len = pos;
		
		pos++;
		req->buffer_freespace -= pos;
		req->buffer_position  += pos;
		read                  -= pos;
	}
	
	{ // parse path
		req->path = req->buffer_position;
		ssize_t pos = first_of(req->buffer_position, read, ' ');
		if(pos < 0)
		{
			quick_response_socket(socket, 400, "request line invalid (path)", false);
			return false;
		}
		
		req->path[pos] = '\0';
		req->path_len = pos;
		
		ssize_t query_pos = first_of(req->path, read, '?');
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
			
			cflare_debug("p = %s q = %s %"FMT_SIZE" %"FMT_SIZE,
					req->path, req->query, req->query_len, req->path_len);
		}
		
		pos++;
		req->buffer_freespace -= pos;
		req->buffer_position  += pos;
		read                  -= pos;
	}
	
	{ // parse "HTTP/major.minor"
		int version_major, version_minor;
		if(sscanf(req->buffer_position, "HTTP/%d.%d", &version_major, &version_minor) != 2)
		{
			quick_response_socket(socket, 400, "request line invalid (version)", false);
			return false;
		}
		req->version = (float64_t)version_major + (float64_t)version_minor / 10.0;
	}
	
	size_t content_length = 0;
	char* content_type = "";
	
	{ // read the headers
		req->headers_count = 0;
		bool firstheader = true;
		bool ignored_header = false;
		while(true)
		{
			if(!cflare_socket_read_line(req->socket, req->buffer_position, req->buffer_freespace, &read))
			{
				quick_response_socket(socket, 400, "request headers never finished", false);
				return false;
			}
			total_read += read;
			
			if(total_read >= CFLARE_REQUEST_MAX_LENGTH)
			{
				quick_response_socket(socket, 413, "request too long", false);
				return false;
			}
			
			if(read == 0) // the blank line
				break;
			else if(req->buffer_position[0] == ' ' || req->buffer_position[0] == '\t')
			{
				if(firstheader)
				{
					quick_response_socket(socket, 400, "request header invalid: nothing to continue", false);
					return false;
				}
				if(ignored_header) // we didn't want this, so we can skip
					continue;
				// a continuation...
				// "previous0 continue0"
				//           ^
				// "previous continue0"
				req->buffer_position[0] = ' ';
				memmove(req->buffer_position - 1, req->buffer_position, read + 1);
				
				req->buffer_position += read;
				req->buffer_freespace -= read;
			}
			else
			{
				ssize_t pos = first_of(req->buffer_position, read, ':');
				if(pos < 0)
				{
					quick_response_socket(socket, 400, "invalid header", false);
					return false;
				}
				
				firstheader = false;
				req->buffer_position[pos] = '\0';
				
				cflare_header hdr = cflare_headers_get(req->buffer_position);
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
					
					req->headers[req->headers_count].header = hdr;
					req->headers[req->headers_count].value = req->buffer_position + pos + 1; // +1 for the space between header and the value
					
					read++; // for the null byte
					req->buffer_position += read;
					req->buffer_freespace -= read;
					req->headers_count++;
				}
			}
		}
	}
	
	// METHODnullPATHnull
	req->time_start_postparse = cflare_time();
	
	/*
	cflare_debug("method: %s; path: %s; query: %s; %lf", req->method, req->path, req->query, req->version);
	
	for(size_t i = 0; i < req->headers_count; i++)
	{
		cflare_debug("header: %s = %s", req->headers[i].header.name, req->headers[i].value);
	}
	
	cflare_debug("there are %lu free bytes in the request buffer; used %lu",
		req->buffer_freespace,
		CFLARE_REQUEST_MAX_LENGTH - req->buffer_freespace);
	
	cflare_debug("request parsed in %lfms", (req->time_start_postparse - req->time_start_parse) * 1000.0);
	*/
	
	cflare_url_parse_query(req->query, req->query_len, &print_query_param, 0x0);
	
	//cflare_debug("request parsed in %lfms", (req->time_start_postparse - req->time_start_parse) * 1000.0);
	
	bool keep_alive = false;
	quick_response_socket(socket, 200, "all is good", keep_alive);
	
	return true;
}
