#ifndef CFLARE_REQUEST_H
#define CFLARE_REQUEST_H

#include <cflare/cflare.h>
#include <cflare/socket.h>

typedef struct cflare_request cflare_request;

CFLARE_API cflare_request* cflare_request_new();
CFLARE_API void cflare_request_delete(cflare_request* req);

CFLARE_API bool cflare_request_process_socket(cflare_request* req, cflare_socket* socket);

#endif /* CFLARE_REQUEST_H */

