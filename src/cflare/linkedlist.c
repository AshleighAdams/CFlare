#include "linkedlist.h"

#include <stdint.h>

static void cflare_linkedlist_internal_get_first(cflare_linkedlist* p_linkedlist, cflare_linkedlist_node* p_curnode)
{
	assert(p_linkedlist);

	p_curnode = p_linkedlist->p_head;
}

static void cflare_linkedlist_internal_get_last(cflare_linkedlist* p_linkedlist, cflare_linkedlist_node* p_lastnode, cflare_linkedlist_node* p_curnode)
{
	assert(p_linkedlist);

	p_curnode = p_linkedlist->p_head;
	while (1)
	{
		if (p_curnode->p_next == 0x0)
			break;

		p_lastnode = p_curnode;
		p_curnode = p_curnode->p_next;
	}
}

static void cflare_linkedlist_internal_get(cflare_linkedlist* p_linkedlist, int location, cflare_linkedlist_node* p_lastnode, cflare_linkedlist_node* p_curnode)
{
	assert(p_linkedlist);

	int curlocation = 0;
	p_curnode = p_linkedlist->p_head;

	while (1)
	{
		if (curlocation == location || p_curnode == 0x0)
			break;

		p_lastnode = p_curnode;
		p_curnode = p_curnode->p_next;
		curlocation++;
	}

	//out of range
	assert(p_curnode);
}

static void cflare_linkedlist_internal_get(cflare_linkedlist* p_linkedlist, void* p_data, cflare_linkedlist_node* p_lastnode, cflare_linkedlist_node* p_curnode)
{
	assert(p_linkedlist);

	p_curnode = p_linkedlist->p_head;

	while (1)
	{
		if (p_curnode == 0x0)
			break;

		p_lastnode = p_curnode;
		p_curnode = p_curnode->p_next;
	}

	//not found
	assert(p_curnode);
}



static int32_t cflare_linkedlist_internal_add(cflare_linkedlist* p_linkedlist, cflare_linkedlist_node* p_lastnode, cflare_linkedlist_node* p_curnode, void* p_data, size_t data_size)
{
	cflare_linkedlist_node* p_newnode = 0x0;

	assert(p_linkedlist);
	
	p_newnode = (cflare_linkedlist_node*)malloc(sizeof(cflare_linkedlist_node));
	assert(p_newnode);

	p_newnode->size = 1;//fixme
	p_newnode->p_data = p_data;
	p_newnode->data_size = data_size;
	p_newnode->p_next = 0x0;

	//first node
	if (p_lastnode == 0x0 && p_curnode == 0x0)
	{
		p_linkedlist->p_head = p_newnode;
		p_linkedlist->count++;
		
		return 1;
	}

	//last node
	if (p_curnode->p_next == 0x0)
	{
		p_curnode->p_next = p_newnode;
		p_linkedlist->count++;

		return 1;
	}

	//somewhere in the middle node
	p_newnode->p_next = p_curnode;
	p_lastnode->p_next = p_newnode;
	p_linkedlist->count++;

	return 1;
}

int32_t cflare_linkedlist_add_first(cflare_linkedlist* p_linkedlist, void* p_data, size_t data_size)
{
	cflare_linkedlist_node* p_curnode = 0x0;

	assert(p_linkedlist);

	cflare_linkedlist_internal_get_first(p_linkedlist, p_curnode);

	return cflare_linkedlist_internal_add(p_linkedlist, 0x0, p_curnode, p_data, data_size);
}

int32_t cflare_linkedlist_add_last(cflare_linkedlist* p_linkedlist, void* p_data, size_t data_size)
{
	cflare_linkedlist_node* p_curnode = 0x0;
	cflare_linkedlist_node* p_lastnode = 0x0;

	assert(p_linkedlist);

	cflare_linkedlist_internal_get_last(p_linkedlist, p_lastnode, p_curnode);

	return cflare_linkedlist_internal_add(p_linkedlist, p_lastnode, p_curnode, p_data, data_size);
}

int32_t cflare_linkedlist_add(cflare_linkedlist* p_linkedlist, int location, void* p_data, size_t data_size)
{
	cflare_linkedlist_node* p_curnode = 0x0;
	cflare_linkedlist_node* p_lastnode = 0x0;

	assert(p_linkedlist);

	cflare_linkedlist_internal_get(p_linkedlist, location, p_lastnode, p_curnode);

	return cflare_linkedlist_internal_add(p_linkedlist, p_lastnode, p_curnode, p_data, data_size);
}


static int32_t cflare_linkedlist_internal_remove(cflare_linkedlist* p_linkedlist, cflare_linkedlist_node* p_lastnode, cflare_linkedlist_node* p_curnode)
{
	assert(p_linkedlist);
	assert(p_curnode);

	//first node
	if (p_lastnode == 0x0)
	{
		p_linkedlist->p_head = p_curnode->p_next;

		//fix me - free the data in the node? if not just free the node
		free(p_curnode->p_data);
		free(p_curnode);
		p_linkedlist->count--;

		return 1;
	}

	//last node
	if (p_curnode->p_next == 0x0)
	{
		p_lastnode->p_next = 0x0;

		free(p_curnode->p_data);
		free(p_curnode);
		p_linkedlist->count--;

		return 1;
	}

	//somewhere in the middle node
	p_lastnode->p_next = p_curnode->p_next;
	free(p_curnode->p_data);
	free(p_curnode);
	p_linkedlist->count--;

	return 1;
}

int32_t cflare_linkedlist_remove_first(cflare_linkedlist* p_linkedlist)
{
	cflare_linkedlist_node* p_curnode = 0x0;

	assert(p_linkedlist);

	cflare_linkedlist_internal_get_first(p_linkedlist, p_curnode);

	return cflare_linkedlist_remove(p_linkedlist, 0x0, p_curnode);
}

int32_t cflare_linkedlist_remove_last(cflare_linkedlist* p_linkedlist)
{
	cflare_linkedlist_node* p_curnode = 0x0;
	cflare_linkedlist_node* p_lastnode = 0x0;

	assert(p_linkedlist);

	cflare_linkedlist_internal_get_last(p_linkedlist, p_lastnode, p_curnode);

	return cflare_linkedlist_remove(p_linkedlist, p_lastnode, p_curnode);
}

int32_t cflare_linkedlist_remove(cflare_linkedlist* p_linkedlist, int location)
{
	cflare_linkedlist_node* p_curnode = 0x0;
	cflare_linkedlist_node* p_lastnode = 0x0;

	assert(p_linkedlist);

	cflare_linkedlist_internal_get(p_linkedlist, location, p_lastnode, p_curnode);

	return cflare_linkedlist_remove(p_linkedlist, p_lastnode, p_curnode);
}

int32_t cflare_linkedlist_remove(cflare_linkedlist* p_linkedlist, void* p_data)
{
	cflare_linkedlist_node* p_curnode = 0x0;
	cflare_linkedlist_node* p_lastnode = 0x0;

	assert(p_linkedlist);

	cflare_linkedlist_internal_get(p_linkedlist, p_data, p_lastnode, p_curnode);

	return cflare_linkedlist_remove(p_linkedlist, p_lastnode, p_curnode);
}
