#ifndef CFLARE_FILESYSTEM_H
#define CFLARE_FILESYSTEM_H

#include <cflare/cflare.h>
#include <cflare/linkedlist.h>

typedef enum
{
	CFLARE_FILESYSTEM_UNKNOWN = 0,
	CFLARE_FILESYSTEM_FILE = 1,
	CFLARE_FILESYSTEM_DIRECTORY = 2,
	CFLARE_FILESYSTEM_FIFO = 3,
	CFLARE_FILESYSTEM_SOCKET = 4,
	CFLARE_FILESYSTEM_CHARDEV = 5,
	CFLARE_FILESYSTEM_BLOCKDEV = 6,
	CFLARE_FILESYSTEM_LINK = 7
} cflare_filesystem_entry_type;

typedef struct cflare_filesystem_entry
{
	char* name;
	cflare_filesystem_entry_type type;
	size_t depth;
} cflare_filesystem_entry;

typedef enum {
	CFLARE_FILESYSTEM_LIST_RECURSIVE = 1 << 0, // recurse into directories and add their children
	CFLARE_FILESYSTEM_LIST_EXCLUDE_DIRECTORIES = 1 << 1, // don't add directories to the list
} cflare_filesystem_listoptions;

CFLARE_API cflare_linkedlist* cflare_filesystem_list(const char* path,
	cflare_filesystem_listoptions opts);

#endif /* CFLARE_FILESYSTEM_H */

