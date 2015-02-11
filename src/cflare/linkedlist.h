#ifndef CFLARE_LINKEDLIST2_H
#define CFLARE_LINKEDLIST2_H


#include <cflare/cflare.h>
#include <cflare/util.h>

struct cflare_linkedlist;

typedef struct cflare_linkedlist_node
{
	void* data;
	struct cflare_linkedlist_node*  next;
	struct cflare_linkedlist_node*  prev;
} cflare_linkedlist_node;

typedef struct cflare_linkedlist
{
	uint64_t count;
	size_t element_size;
	cflare_deleter* deleter;
	void* deleter_context;
	struct cflare_linkedlist_node* first;
	struct cflare_linkedlist_node* last;
	
} cflare_linkedlist;

typedef struct cflare_linkedlist_iter
{
	uint8_t started;
	struct cflare_linkedlist* list;
	struct cflare_linkedlist_node* prev;
	struct cflare_linkedlist_node* value;
	struct cflare_linkedlist_node* next;
} cflare_linkedlist_iter;

CFLARE_API cflare_linkedlist* cflare_linkedlist_new(size_t element_size);
CFLARE_API void cflare_linkedlist_delete(cflare_linkedlist* list);

CFLARE_API void cflare_linkedlist_ondelete(cflare_linkedlist* list, cflare_deleter* func, void* context);

CFLARE_API void cflare_linkedlist_insert_before(cflare_linkedlist* list, cflare_linkedlist_node* node, void** output);
CFLARE_API void cflare_linkedlist_insert_after(cflare_linkedlist* list, cflare_linkedlist_node* node, void** output);
CFLARE_API void cflare_linkedlist_remove(cflare_linkedlist* list, cflare_linkedlist_node* node);

CFLARE_API cflare_linkedlist_iter cflare_linkedlist_iterator(cflare_linkedlist* list);
CFLARE_API cflare_linkedlist_node* cflare_linkedlist_iterator_next(cflare_linkedlist_iter* iter);
CFLARE_API cflare_linkedlist_node* cflare_linkedlist_iterator_prev(cflare_linkedlist_iter* iter);

// short cuts, but not neccessary required for implimentation
CFLARE_API void cflare_linkedlist_insert_first(cflare_linkedlist* list, void** output);
CFLARE_API void cflare_linkedlist_insert_last(cflare_linkedlist* list, void** output);

#endif /* CFLARE_LINKEDLIST2_H */

