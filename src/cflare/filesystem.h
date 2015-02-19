#ifndef CFLARE_FILESYSTEM_H
#define CFLARE_FILESYSTEM_H

#include "cflare/cflare.h"

typedef enum
{
	CFLARE_FILESYSTEM_FILE = 1,
	CFLARE_FILESYSTEM_DIRECTORY = 2,
	CFLARE_FILESYSTEM_LINK = 3,
	CFLARE_FILESYSTEM_FIFO = 4,
} cflare_filesystem_entry_type;

typedef struct cflare_filesystem_entry
{
	const char* name;
	cflare_filesystem_entry_type type;
} cflare_filesystem_entry;

typedef enum {
	CFLARE_FILESYSTEM_LIST_RECURSIVE = 1 << 0
} cflare_filesystem_listoptions;

CFLARE_API cflare_linkedlist* cflare_filesystem_list(const char* path, cflare_filesystem_listoptions opts);

#endif /* CFLARE_FILESYSTEM_H */

