#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <stdint.h>

typedef struct cflare_linkedlist_node
{
	size_t size;

	void* p_data;
	size_t data_size;

	struct cflare_linkedlist_node* p_next;
} cflare_linkedlist_node;

typedef struct cflare_linkedlist
{
	size_t size;

	struct cflare_linkedlist_node* p_head;
	uint32_t count;
} cflare_linkedlist;

static void cflare_linkedlist_initialize(cflare_linkedlist* p_linkedlist);

/*
Gets
*/

//Get the firts element, for internal use only
//\param[out] p_curnode current node pointer
static void cflare_linkedlist_internal_get_first(cflare_linkedlist* p_linkedlist, cflare_linkedlist_node* p_curnode);
//Get the last and current node of the last element, for internal use only
//\param[out] p_lastnode last node pointer
//\param[out] p_curnode current node pointer
static void cflare_linkedlist_internal_get_last(cflare_linkedlist* p_linkedlist, cflare_linkedlist_node* p_lastnode, cflare_linkedlist_node* p_curnode);
//Get the last and current node at the location, for internal use only
//\param location The location of the element, 0 is the first element in the list
//\param[out] p_lastnode last node pointer
//\param[out] p_curnode current node pointer
static void cflare_linkedlist_internal_get(cflare_linkedlist* p_linkedlist, int location, cflare_linkedlist_node* p_lastnode, cflare_linkedlist_node* p_curnode);
//Get the last and current node that first cotains this data, for internal use only
//\param[out] p_lastnode last node pointer
//\param[out] p_curnode current node pointer
static void cflare_linkedlist_internal_get(cflare_linkedlist* p_linkedlist, void* p_data, cflare_linkedlist_node* p_lastnode, cflare_linkedlist_node* p_curnode);

/*
Adds
*/

//Add an element between the nodes, for internal use only
//\param[in] p_lastnode last node pointer
//\param[in] p_curnode current node pointer
static int32_t cflare_linkedlist_internal_add(cflare_linkedlist* p_linkedlist, cflare_linkedlist_node* p_lastnode, cflare_linkedlist_node* p_curnode, void* p_data, size_t data_size);
//Add before the first element of the list
int32_t cflare_linkedlist_add_first(cflare_linkedlist* p_linkedlist, void* p_data, size_t data_size);
//Add after the last element of the list
int32_t cflare_linkedlist_add_last(cflare_linkedlist* p_linkedlist, void* p_data, size_t data_size);
//Add one element ahead of the location
//\param location The location of the element, 0 is the first element in the list
int32_t cflare_linkedlist_add(cflare_linkedlist* p_linkedlist, int location, void* p_data, size_t data_size);

/*
Removes
*/

//Remove the element p_curnode, for internal use only
//\param[out] p_lastnode last node pointer
//\param[out] p_curnode current node pointer
static int32_t cflare_linkedlist_internal_remove(cflare_linkedlist* p_linkedlist, cflare_linkedlist_node* p_lastnode, cflare_linkedlist_node* p_curnode);
//Remove the first element
int32_t cflare_linkedlist_remove_first(cflare_linkedlist* p_linkedlist);
//Remove the last element
int32_t cflare_linkedlist_remove_last(cflare_linkedlist* p_linkedlist);
//Remove the element at the location
//\param location The location of the element, 0 is the first element in the list
int32_t cflare_linkedlist_remove(cflare_linkedlist* p_linkedlist, int location);
//Remove the element that first cotains this data, for internal use only
int32_t cflare_linkedlist_remove(cflare_linkedlist* p_linkedlist, void* p_data);

#endif