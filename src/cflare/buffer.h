#ifndef CFLARE_BUFFER_H
#define CFLARE_BUFFER_H

#include <cflare/cflare.h>
#include <cflare/util.h>

typedef enum
{
	CFLARE_BUFFER_NOCOPY   = 1 << 0, // no need to copy the data
	CFLARE_BUFFER_REVERSE  = 1 << 1, // the buffer should be built in the reverse order
	CFLARE_BUFFER_NULLCHAR = 1 << 2  // append to the end a null char on build
} cflare_buffer_options;

// forward declaration.
struct cflare_buffer_part;
typedef struct cflare_buffer_part cflare_buffer_part;

struct cflare_buffer;
typedef struct cflare_buffer cflare_buffer;

CFLARE_API cflare_buffer* cflare_buffer_new(cflare_buffer_options opts);
CFLARE_API void cflare_buffer_delete(cflare_buffer* buff);

CFLARE_API size_t cflare_buffer_length(cflare_buffer* buff);
CFLARE_API size_t cflare_buffer_parts(cflare_buffer* buff);

CFLARE_API void cflare_buffer_append(cflare_buffer* buff, const uint8_t* data, size_t len);
CFLARE_API uint8_t* cflare_buffer_build(cflare_buffer* buff);

#endif /* CFLARE_BUFFER_H */

