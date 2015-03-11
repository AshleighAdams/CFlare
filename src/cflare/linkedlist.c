
#include "cflare/linkedlist.h"

typedef struct cflare_linkedlist
{
	uint64_t count;
	size_t element_size;
	cflare_deleter* deleter;
	void* deleter_context;
	cflare_linkedlist_node* first;
	cflare_linkedlist_node* last;
} cflare_linkedlist;

cflare_linkedlist* cflare_linkedlist_new(size_t element_size)
{
	cflare_linkedlist* ret = malloc(sizeof(cflare_linkedlist));
	ret->count = 0;
	ret->element_size = element_size;
	ret->first = 0;
	ret->last = 0;
	ret->deleter = 0;
	ret->deleter_context = 0;
	
	return ret;
}

void cflare_linkedlist_delete(cflare_linkedlist* list)
{
	while(list->count)
		cflare_linkedlist_remove(list, list->first);
	
	free(list);
}

void cflare_linkedlist_ondelete(cflare_linkedlist* list, cflare_deleter* func, void* context)
{
	list->deleter = func;
	list->deleter_context = context;
}

static void insert_between(cflare_linkedlist* list, cflare_linkedlist_node* pre,
	cflare_linkedlist_node* post, void** output)
{
	cflare_linkedlist_node* node = malloc(sizeof(cflare_linkedlist_node));
	node->data = malloc(list->element_size);
	memset(node->data, 0, list->element_size);
	node->prev = pre;
	node->next = post;
	
	if(pre)
		pre->next = node;
	else
		list->first = node;
	
	if(post)
		post->prev = node;
	else
		list->last = node;
	
	list->count += 1;
	*output = node->data;
}

void cflare_linkedlist_insert_before(cflare_linkedlist* list,
	cflare_linkedlist_node* node, void** output)
{
	assert(list);
	assert(output);
	insert_between(list, node ? node->prev : 0, node, output);
}

void cflare_linkedlist_insert_after(cflare_linkedlist* list,
	cflare_linkedlist_node* node, void** output)
{
	assert(list);
	assert(output);
	insert_between(list, node, node ? node->next : 0, output);
}

void cflare_linkedlist_remove(cflare_linkedlist* list,
	cflare_linkedlist_node* node)
{
	assert(list);
	assert(node);
	
	cflare_linkedlist_node* pre = node->prev;
	cflare_linkedlist_node* post = node->next;
	
	if(pre)
		pre->next = post;
	else // removing first elm
		list->first = post;
	
	if(post)
		post->prev = pre;
	else // removing last elm
		list->last = pre;
	
	if(list->deleter && node->data)
		list->deleter(node->data, list->deleter_context);
	free(node->data);
	free(node);
	list->count -= 1;
}

size_t cflare_linkedlist_count(cflare_linkedlist* list)
{
	return list->count;
}

cflare_linkedlist_iter cflare_linkedlist_iterator(cflare_linkedlist* list)
{
	cflare_linkedlist_iter ret;
	ret.started = false;
	ret.list = list;
	ret.prev = 0;
	ret.next = 0;
	
	return ret;
}

cflare_linkedlist_node* cflare_linkedlist_iterator_next(cflare_linkedlist_iter* iter)
{
	if(!iter->started)
	{
		iter->next = iter->list->first;
		iter->prev = 0;
		iter->started = true;
		iter->value = 0;
	}
	
	if(!iter->next)
	{
		iter->value = 0;
		return 0;
	}
	
	cflare_linkedlist_node* ret = iter->next;
	iter->next = ret->next;
	iter->prev = iter->value;
	iter->value = ret;
	return ret;
}

cflare_linkedlist_node* cflare_linkedlist_iterator_prev(cflare_linkedlist_iter* iter)
{
	if(!iter->started)
	{
		iter->prev = iter->list->last;
		iter->next = 0;
		iter->started = true;
		iter->value = 0;
	}
	
	if(!iter->prev)
	{
		iter->value = 0;
		return 0;
	}
	
	cflare_linkedlist_node* ret = iter->prev;
	iter->prev = ret->prev;
	iter->next = iter->value;
	iter->value = ret;
	return ret;
}


/// Shortcuts

void cflare_linkedlist_insert_first(cflare_linkedlist* list, void** output)
{
	cflare_linkedlist_insert_before(list, list->first, output);
}

void cflare_linkedlist_insert_last(cflare_linkedlist* list, void** output)
{
	cflare_linkedlist_insert_after(list, list->last, output);
}
