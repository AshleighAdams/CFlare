
#include "cflare/request.h"
#include "cflare/headers.h"
#include "cflare/util.h"

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
	char* path;
	char* query;
	
	float64_t version;
	float64_t time_start, time_start_parse, time_start_postparse;
	
	struct
	{
		cflare_header header;
		char* value;
	} headers[CFLARE_REQUEST_MAX_HEADERS];
	size_t headers_count;
	
	cflare_socket* socket;
} cflare_request;

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

bool cflare_request_process_socket(cflare_request* req, cflare_socket* socket)
{
	req->socket = socket;
	req->buffer_position = req->buffer;
	req->buffer_freespace = sizeof(req->buffer);
	req->time_start = cflare_time();
	
	size_t read;
	size_t total_read = 0;
	
	if(!cflare_socket_read_line(req->socket, req->buffer_position, req->buffer_freespace, &read))
		return false;
	
	req->time_start_parse = cflare_time();
	total_read += read;
	
	{ // parse method
		req->method = req->buffer_position;
		ssize_t pos = first_of(req->buffer_position, read, ' ');
		
		if(pos < 0)
		{
			cflare_warn("request line invalid (method)");
			return false;
		}
		else
			req->method[pos] = '\0';
		
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
			cflare_warn("request line invalid (path)");
			return false;
		}
		else
			req->path[pos] = '\0';
		
		ssize_t query_pos = first_of(req->path, read, '?');
		if(query_pos < 0)
			req->query = 0x0;
		else
		{
			req->path[query_pos] = '\0';
			req->query = req->path + query_pos + 1;
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
			cflare_warn("request line invalid (version)");
			return false;
		}
		req->version = (float64_t)version_major + (float64_t)version_minor / 10.0;
	}
	
	{ // read the headers
		req->headers_count = 0;
		bool firstheader = true;
		bool ignored_header = false;
		while(true)
		{
			if(!cflare_socket_read_line(req->socket, req->buffer_position, req->buffer_freespace, &read))
			{
				cflare_warn("request headers never finished");
				return false;
			}
			total_read += read;
			
			if(total_read >= CFLARE_REQUEST_MAX_LENGTH)
			{
				cflare_warn("request too long %zu", total_read);
				return false;
			}
			
			if(read == 0) // the blank line
				break;
			else if(req->buffer_position[0] == ' ' || req->buffer_position[0] == '\t')
			{
				if(firstheader)
				{
					cflare_warn("request header invalid: nothing to continue");
					return false;
				}
				if(ignored_header) // we didn't want this, so we can skip
					continue;
				// a continuation...
				// "previous0 continue0"
				//           ^
				// "previous continue0"
				req->buffer_position[0] = ' ';
				memmove(req->buffer_position - 1, req->buffer_position, read + 1 /*null*/);
				
				req->buffer_position += read;
				req->buffer_freespace -= read;
			}
			else
			{
				ssize_t pos = first_of(req->buffer_position, read, ':');
				if(pos < 0)
				{
					cflare_debug("invalid header line: %s", req->buffer_position);
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
						cflare_debug("reached max number of headers");
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
	
	cflare_debug("method: %s; path: %s; query: %s; %lf", req->method, req->path, req->query, req->version);
	
	for(size_t i = 0; i < req->headers_count; i++)
	{
		cflare_debug("header: %s = %s", req->headers[i].header.name, req->headers[i].value);
	}
	
	cflare_debug("there are %lu free bytes in the request buffer; used %lu",
		req->buffer_freespace,
		CFLARE_REQUEST_MAX_LENGTH - req->buffer_freespace);
	
	cflare_debug("request parsed in %lfms", (req->time_start_postparse - req->time_start_parse) * 1000.0);
	
	return true;
}
