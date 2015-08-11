
#ifndef CFLARE_RESPONSE_H
#define CFLARE_RESPONSE_H

#include <cflare/cflare.h>
#include <cflare/util.h>

typedef struct cflare_response cflare_response;

CFLARE_API cflare_response* cflare_response_new();

CFLARE_API void cflare_response_init(cflare_response* res, cflare_socket* sock);

CFLARE_API cflare_socket* cflare_response_socket(cflare_response* req);
CFLARE_API void cflare_response_status(cflare_response* res, uint32_t status);
CFLARE_API void cflare_response_cookie(cflare_response* res, const char* domain, const char* key, const char* value, float64_t valid_for, bool https_only);
CFLARE_API void cflare_response_header(cflare_response* res, cflare_header hdr, const char* value);
CFLARE_API void cflare_response_keepalive(cflare_response* res, bool keepalive);

// finalizers
// these cause the reply to be sent
// these will interpret the response's headers, like using HEAD insteadof GET, or Range
// any one of these more than once is invalid
// returns if the data was sent okay
CFLARE_API bool cflare_response_data(cflare_response* res, const uint8_t* buffer, size_t buffer_len);
CFLARE_API bool cflare_response_file(cflare_response* res, const char* path);

#endif
