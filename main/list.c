/*
    list.c - Double-linked list implementation
    Copyright (C) 2006  Aleksandar Dezelin <dezelin@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <stdafx.h>

#include "array.h"
#include "errors.h"
#include "list.h"

#include <assert.h>
#include <malloc.h>
#include <memory.h>
#include <stdlib.h>

#ifdef _DEBUG
// Poison value to detect use of deallocated list elements
#define POISON	((void*)0x12345678)
#endif

/* List node type */
struct tag_optcl_list_node {
	struct tag_optcl_list_node *prev;
	struct tag_optcl_list_node *next;
	void *data;
};

typedef struct tag_optcl_list_node optcl_list_node;

/* List type */
struct tag_optcl_list {
	int node_count;
	optcl_list_node *first_node;
	optcl_list_node *last_node;
	optcl_list_equalfn equalfn;
};


int optcl_list_add_head(optcl_list *list, const void *data)
{
	optcl_list_node *nnode;

	assert(list);

	if (!list)
		return E_INVALIDARG;


	nnode = (optcl_list_node*)malloc(sizeof(optcl_list_node));

	if (!nnode)
		return E_OUTOFMEMORY;

	nnode->data = (void*)data;
	nnode->prev = 0;
	nnode->next = list->first_node;

	if (list->first_node)
		list->first_node->prev = nnode;

	list->first_node = nnode;

	if (!list->last_node)
		list->last_node = nnode;

	return SUCCESS;
}

int optcl_list_add_tail(optcl_list *list, const void *data)
{
	optcl_list_node *nnode;

	assert(list);

	if (!list)
		return E_INVALIDARG;

	nnode = (optcl_list_node*)malloc(sizeof(optcl_list_node));

	if (!nnode)
		return E_OUTOFMEMORY;

	nnode->data = (void*)data;
	nnode->prev = list->last_node;
	nnode->next = 0;

	if (list->last_node)
		list->last_node->next = nnode;

	list->last_node = nnode;

	if (!list->first_node)
		list->first_node = nnode;

	return SUCCESS;
}

int optcl_list_append(optcl_list *dest, const optcl_list *src)
{
	int error;
	optcl_list_iterator it;

	assert(src);
	assert(dest);

	if (!dest || !src)
		return E_INVALIDARG;

	error = optcl_list_get_head_pos(src, &it);

	if (FAILED(error))
		return error;

	while (it) {
		error = optcl_list_add_tail(dest, it->data);

		if (FAILED(error))
			break;

		error = optcl_list_get_next(src, it, &it);

		if (FAILED(error))
			break;
	}

	return error;
}

static int compare_data_ptrs(const void *left, const void *right)
{
	/* NOTE: There are no relations '<' and '>' for pointers. */
	return (left != right);
}

int optcl_list_create(const optcl_list_equalfn equalfn, 
		      optcl_list **list)
{
	optcl_list *newlist;

	assert(list);

	if (!list)
		return E_INVALIDARG;

	newlist = (optcl_list*)malloc(sizeof(optcl_list));

	if (!newlist)
		return E_OUTOFMEMORY;

	memset(newlist, 0, sizeof(optcl_list));
	newlist->equalfn = (equalfn) ? equalfn : compare_data_ptrs;

	*list = newlist;

	return SUCCESS;
}

int optcl_list_destroy(optcl_list *list, int deallocate)
{
	assert(list);

	if (!list)
		return E_INVALIDARG;

	optcl_list_clear(list, deallocate);
	
	memset(list, 0, sizeof(optcl_list));

#ifdef _DEBUG
	list->first_node = POISON;
	list->last_node = POISON;
#endif

	free(list);

	return SUCCESS;
}

int optcl_list_clear(optcl_list *list, int deallocate)
{
	optcl_list_iterator current = 0;
	optcl_list_iterator next = 0;

	assert(list);

	if (!list)
		return E_INVALIDARG;

	current = list->first_node;

	while(current) {
		next = current->next;

		if (deallocate)
			free(current->data);
#ifdef _DEBUG		
	current->data = POISON;
	current->next = POISON;
	current->prev = POISON;
#endif
		free(current);
		current = next;
	}

	list->node_count = 0;
	list->first_node = 0;
	list->last_node = 0;

	return SUCCESS;
}

int optcl_list_copy(optcl_list *dest, const optcl_list *src)
{
	int error;

	assert(src);
	assert(dest);

	if (!dest || !src)
		return E_INVALIDARG;

	error = optcl_list_clear(dest, 0);

	if (FAILED(error))
		return error;

	dest->equalfn = src->equalfn;

	return optcl_list_append(dest, src);
}

int optcl_list_find(const optcl_list *list, 
		    const void *data,
		    optcl_list_iterator *pos)
{
	int error;
	void *element;
	optcl_list_iterator it;

	assert(list);
	assert(pos);

	if (!list || !pos)
		return E_INVALIDARG;

	assert(list->equalfn);

	if (!list->equalfn)
		return E_UNEXPECTED;

	error = optcl_list_get_head_pos(list, &it);

	if (FAILED(error))
		return error;

	while (it) {
		error = optcl_list_get_at_pos(list, it, &element);

		if (FAILED(error))
			break;

		/* Break if equal */
		if (!list->equalfn(element, data))
			break;

		error = optcl_list_get_next(list, it, &it);

		if (FAILED(error))
			break;
	}

	if (SUCCEEDED(error))
		*pos = it;

	return error;
}

int optcl_list_get_equalfn(const optcl_list *list, 
			   optcl_list_equalfn *equalfn)
{
	assert(list);
	assert(equalfn);

	if (!list || !equalfn)
		return E_INVALIDARG;

	*equalfn = list->equalfn;

	return SUCCESS;
}

int optcl_list_get_at_index(const optcl_list *list, int index, void**data)
{
	int i = 0;
	int count;
	int error;
	int forward;
	optcl_list_iterator it;

	assert(data);
	assert(list);

	if (!data || !list)
		return E_INVALIDARG;

	if (index < 0)
		return E_OUTOFRANGE;

	error = optcl_list_get_count(list, &count);

	if (FAILED(error))
		return error;

	if (index >= count)
		return E_OUTOFRANGE;

	forward = (count - index < index);

	if (forward) {
		error = optcl_list_get_head_pos(list, &it);
		i = 0;
	} else {
		error = optcl_list_get_tail_pos(list, &it);
		i = count - 1;
	}

	if (FAILED(error))
		return error;

	while (it && i != index) {
		if (forward) {
			error = optcl_list_get_next(list, it, &it);
			i++;
		} else {
			error = optcl_list_get_previous(list, it, &it);
			i--;
		}
	}

	if (!it) 
		return error;
	
	return optcl_list_get_at_pos(list, it, data);
}

int optcl_list_get_at_pos(const optcl_list *list, 
			  const optcl_list_iterator pos, 
			  void **data)
{
	assert(pos);
	assert(list);
	assert(data);

	if (!list || !pos || !data)
		return E_INVALIDARG;

	*data = pos->data;

	return SUCCESS;
}

int optcl_list_get_count(const optcl_list *list, int *count)
{
	assert(list);
	assert(count);

	if (!list || !count)
		return E_INVALIDARG;

	*count = list->node_count;

	return SUCCESS;
}

int optcl_list_get_head_pos(const optcl_list *list, 
			    optcl_list_iterator *pos)
{
	assert(list);
	assert(pos);

	if (!list || !pos)
		return E_INVALIDARG;

	*pos = list->first_node;

	return SUCCESS;
}

int optcl_list_get_tail_pos(const optcl_list *list,
			    optcl_list_iterator *pos)
{
	assert(list);
	assert(pos);

	if (!list || !pos)
		return E_INVALIDARG;

	*pos = list->last_node;

	return SUCCESS;
}

int optcl_list_get_next(const optcl_list *list,
			const optcl_list_iterator pos,
			optcl_list_iterator *next)
{
	assert(list);
	assert(pos);
	assert(next);

	if (!list || !pos || !next)
		return E_INVALIDARG;

	*next = pos->next;

	return SUCCESS;
}

int optcl_list_get_previous(const optcl_list *list,
			    const optcl_list_iterator pos,
			    optcl_list_iterator *previous)
{
	assert(list);
	assert(pos);
	assert(previous);

	if (!list || !pos || !previous)
		return E_INVALIDARG;

	*previous = pos->prev;

	return SUCCESS;
}

int optcl_list_insert_after(optcl_list *list, 
			    const optcl_list_iterator pos, 
			    const void *data)
{
	optcl_list_node *nnode;

	assert(pos);
	assert(list);
	assert(data);

	if (!pos || !list || !data)
		return E_INVALIDARG;

	nnode = (optcl_list_node*)malloc(sizeof(optcl_list_node));

	if (!nnode)
		return E_OUTOFMEMORY;

	nnode->data = (void*)data;

	if (pos->next) {
		pos->next->prev = nnode;
		nnode->next = pos->next->prev;
	}

	pos->next = nnode;
	nnode->prev = pos;

	return SUCCESS;
}

int optcl_list_insert_before(optcl_list *list, 
			     const optcl_list_iterator pos, 
			     const void *data)
{
	optcl_list_node *nnode;

	assert(pos);
	assert(list);
	assert(data);

	if (!pos || !list || !data)
		return E_INVALIDARG;

	nnode = (optcl_list_node*)malloc(sizeof(optcl_list_node));

	if (!nnode)
		return E_OUTOFMEMORY;

	nnode->data = (void*)data;

	if (pos->prev) {
		pos->prev->next = nnode;
		nnode->prev = pos->prev->next;
	}

	pos->prev = nnode;
	nnode->next = pos;

	return SUCCESS;
}

int optcl_list_remove(optcl_list *list, optcl_list_iterator pos)
{
	assert(list);
	assert(pos);

	if (!list || !pos)
		return E_INVALIDARG;

	if (!pos->prev) {
		assert(pos == list->first_node);

		if (pos != list->first_node)
			return E_UNEXPECTED;

		list->first_node = pos->next;
		
		if (pos->next)
			pos->next->prev = 0;
		else
			list->last_node = 0;
	} else if (!pos->next) {
		assert(pos == list->last_node);

		if (pos != list->last_node)
			return E_UNEXPECTED;

		list->last_node = pos->prev;

		if (pos->prev)
			pos->prev->next = 0;
		else
			list->first_node = 0;
	} else {
		pos->prev->next = pos->next;
		pos->next->prev = pos->prev;
	}

	list->node_count--;

#ifdef _DEBUG
	pos->data = POISON;
	pos->next = POISON;
	pos->prev = POISON;
#endif

	free(pos);

	return SUCCESS;
}

int optcl_list_set_at_pos(const optcl_list *list, 
			  optcl_list_iterator pos, 
			  void *element
			  )
{
	assert(list);
	assert(pos);

	if (!list || !pos)
		return E_INVALIDARG;

	pos->data = element;

	return SUCCESS;
}

int optcl_list_set_equalfn(optcl_list *list, 
			   optcl_list_equalfn equalfn)
{
	assert(list);
	assert(equalfn);

	if (!list || !equalfn)
		return E_INVALIDARG;

	list->equalfn = equalfn;

	return SUCCESS;
}

int optcl_list_sort(optcl_list *list)
{
	int i;
	int error;
	int count;
	void *element;
	optcl_array *array;
	optcl_list_iterator it;
	
	assert(list);

	if (!list)
		return E_INVALIDARG;

	assert(list->equalfn);

	if (!list->equalfn)
		return E_UNEXPECTED;

	error = optcl_array_create(sizeof(int*), list->equalfn, &array);

	if (FAILED(error))
		return error;

	error = optcl_list_get_count(list, &count);

	if (FAILED(error)) {
		optcl_array_destroy(array, 0);
		return error;
	}

	error = optcl_array_set_size(array, count, 0);

	if (FAILED(error)) {
		optcl_array_destroy(array, 0);
		return error;
	}

	error = optcl_list_get_head_pos(list, &it);

	if (FAILED(error)) {
		optcl_array_destroy(array, 0);
		return error;
	}

	i = 0;

	while (it) {
		error = optcl_list_get_at_pos(list, it, &element);

		if (FAILED(error))
			break;

		error = optcl_array_set(array, i++, element);

		if (FAILED(error))
			break;

		error = optcl_list_get_next(list, it, &it);

		if (FAILED(error))
			break;
	}

	if (FAILED(error)) {
		optcl_array_destroy(array, 0);
		return error;
	}

	assert(list->equalfn);

	if (!list->equalfn) {
		optcl_array_destroy(array, 0);
		return E_UNEXPECTED;
	}

	error = optcl_array_set_equalfn(array, list->equalfn);

	if (FAILED(error)) {
		optcl_array_destroy(array, 0);
		return error;
	}

	error = optcl_array_sort(array);

	if (FAILED(error)) {
		optcl_array_destroy(array, 0);
		return error;
	}

	error = optcl_list_get_head_pos(list, &it);

	if (FAILED(error)) {
		optcl_array_destroy(array, 0);
		return error;
	}

	i = 0;

	while (it) {
		error = optcl_array_get(array, i, &element);

		if (FAILED(error))
			break;

		error = optcl_list_set_at_pos(list, it, element);

		if (FAILED(error))
			break;

		error = optcl_list_get_next(list, it, &it);

		if (FAILED(error))
			break;
	}

	optcl_array_destroy(array, 0);

	return error;
}
