#ifndef CFLARE_REQUEST_H
#define CFLARE_REQUEST_H

#include <cflare/cflare.h>
#include <cflare/socket.h>
#include <cflare/headers.h>

typedef struct cflare_request cflare_request;

typedef struct cflare_request_header
{
	cflare_header header;
	char* value;
	size_t value_len;
} cflare_request_header;


CFLARE_API cflare_request* cflare_request_new();
CFLARE_API void cflare_request_delete(cflare_request* req);

CFLARE_API bool cflare_request_process_socket(cflare_request* req, cflare_socket* socket);

//

CFLARE_API cflare_socket* cflare_request_socket(cflare_request* req);
CFLARE_API const char* cflare_request_method(cflare_request* req);
CFLARE_API const char* cflare_request_path(cflare_request* req);
CFLARE_API const char* cflare_request_query(cflare_request* req);
CFLARE_API const char* cflare_request_version(cflare_request* req);

CFLARE_API const char* cflare_request_ip(cflare_request* req);
CFLARE_API uint16_t cflare_request_port(cflare_request* req);
CFLARE_API cflare_request_header* cflare_request_headers(cflare_request* req);

CFLARE_API const char* cflare_request_host(cflare_request* req);

// does the request have any content that needs to be read? + related funcs to read it
CFLARE_API bool cflare_request_content_has(cflare_request* req);
CFLARE_API const char* cflare_request_content_type(cflare_request* req);
CFLARE_API size_t cflare_request_content_length(cflare_request* req);
CFLARE_API bool cflare_request_content_chunk(cflare_request* req, char* buffer, size_t buffer_len, size_t* read); // returns true if read succesfully

#endif /* CFLARE_REQUEST_H */

