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
#include "types.h"

#include <assert.h>
#include <malloc.h>
#include <memory.h>
#include <stdlib.h>


/*
 * List internal structures
 */

/* List node type */
struct tag_optcl_list_node {
	struct tag_optcl_list_node *prev;
	struct tag_optcl_list_node *next;
	const void *data;
};

typedef struct tag_optcl_list_node optcl_list_node;

/* List type */
struct tag_optcl_list {
	uint32_t node_count;
	optcl_list_node *first_node;
	optcl_list_node *last_node;
	optcl_list_equalfn equalfn;
};


/*
 * Helper functions
 */

/* NOTE: There are no relations '<' and '>' for pointers. */
static int8_t compare_data_ptrs(const void *left, const void *right)
{
	int8_t result;

	if (left == right) {
		result = 0;
	} else {
		result = 1;
	}

	return(result);
}


/*
 * List functions implementation
 */

RESULT optcl_list_add_head(optcl_list *list, const void *data)
{
	optcl_list_node *nnode = 0;

	assert(list != 0);

	if (list == 0) {
		return(E_INVALIDARG);
	}

	nnode = (optcl_list_node*)malloc(sizeof(optcl_list_node));

	if (nnode == 0) {
		return(E_OUTOFMEMORY);
	}

	nnode->data = data;
	nnode->prev = 0;
	nnode->next = list->first_node;

	if (list->first_node != 0) {
		list->first_node->prev = nnode;
	}

	list->first_node = nnode;

	if (list->last_node == 0) {
		list->last_node = nnode;
	}

	list->node_count++;

	return(SUCCESS);
}

RESULT optcl_list_add_tail(optcl_list *list, const void *data)
{
	optcl_list_node *nnode = 0;

	assert(list != 0);

	if (list == 0) {
		return(E_INVALIDARG);
	}

	nnode = (optcl_list_node*)malloc(sizeof(optcl_list_node));

	if (nnode == 0) {
		return(E_OUTOFMEMORY);
	}

	nnode->data = data;
	nnode->prev = list->last_node;
	nnode->next = 0;

	if (list->last_node != 0) {
		list->last_node->next = nnode;
	}

	list->last_node = nnode;

	if (list->first_node == 0) {
		list->first_node = nnode;
	}

	list->node_count++;

	return(SUCCESS);
}

RESULT optcl_list_append(optcl_list *dest, const optcl_list *src)
{
	RESULT error;
	optcl_list_iterator it = 0;

	assert(src != 0);
	assert(dest != 0);

	if (dest == 0 || src == 0) {
		return(E_INVALIDARG);
	}

	error = optcl_list_get_head_pos(src, &it);

	if (FAILED(error)) {
		return(error);
	}

	while (it != 0) {
		error = optcl_list_add_tail(dest, it->data);

		if (FAILED(error)) {
			break;
		}

		error = optcl_list_get_next(src, it, &it);

		if (FAILED(error)) {
			break;
		}
	}

	return(error);
}

RESULT optcl_list_create(const optcl_list_equalfn equalfn, 
			 optcl_list **list)
{
	optcl_list *newlist = 0;

	assert(list != 0);

	if (list == 0) {
		return(E_INVALIDARG);
	}

	newlist = (optcl_list*)malloc(sizeof(optcl_list));

	if (newlist == 0) {
		return(E_OUTOFMEMORY);
	}

	memset(newlist, 0, sizeof(optcl_list));
	newlist->equalfn = (equalfn) ? equalfn : compare_data_ptrs;

	*list = newlist;

	return(SUCCESS);
}

RESULT optcl_list_destroy(optcl_list *list, bool_t deallocate)
{
	RESULT error;

	assert(list != 0);

	if (list == 0) {
		return(E_INVALIDARG);
	}

	error = optcl_list_clear(list, deallocate);

	if (FAILED(error)) {
		return(error);
	}
	
	memset(list, 0, sizeof(optcl_list));

	free(list);

	return(SUCCESS);
}

RESULT optcl_list_clear(optcl_list *list, bool_t deallocate)
{
	optcl_list_iterator next = 0;
	optcl_list_iterator current = 0;

	assert(list != 0);

	if (list == 0) {
		return(E_INVALIDARG);
	}

	current = list->first_node;

	while (current != 0) {
		next = current->next;

		if (deallocate == True) {
			free((void*)current->data);
		}

		free(current);
		current = next;
	}

	list->node_count = 0;
	list->first_node = 0;
	list->last_node = 0;

	return(SUCCESS);
}

RESULT optcl_list_copy(optcl_list *dest, const optcl_list *src)
{
	RESULT error;

	assert(src != 0);
	assert(dest != 0);

	if (dest == 0 || src == 0) {
		return(E_INVALIDARG);
	}

	error = optcl_list_clear(dest, 0);

	if (FAILED(error)) {
		return(error);
	}

	dest->equalfn = src->equalfn;

	return(optcl_list_append(dest, src));
}

RESULT optcl_list_find(const optcl_list *list, 
		       const void *data,
		       optcl_list_iterator *pos)
{
	RESULT error;
	void *element = 0;
	optcl_list_iterator it = 0;

	assert(list != 0);
	assert(pos != 0);

	if (list == 0 || pos == 0) {
		return(E_INVALIDARG);
	}

	assert(list->equalfn != 0);

	if (list->equalfn == 0) {
		return(E_UNEXPECTED);
	}

	error = optcl_list_get_head_pos(list, &it);

	if (FAILED(error)) {
		return(error);
	}

	while (it != 0) {
		error = optcl_list_get_at_pos(list, it, &element);

		if (FAILED(error)) {
			break;
		}

		/* Break if equal */
		if (list->equalfn(element, data) == 0) {
			break;
		}

		error = optcl_list_get_next(list, it, &it);

		if (FAILED(error)) {
			break;
		}
	}

	if (SUCCEEDED(error)) {
		*pos = it;
	}

	return(error);
}

RESULT optcl_list_get_equalfn(const optcl_list *list, 
			      optcl_list_equalfn *equalfn)
{
	assert(list != 0);
	assert(equalfn != 0);

	if (list == 0 || equalfn == 0) {
		return(E_INVALIDARG);
	}

	*equalfn = list->equalfn;

	return(SUCCESS);
}

RESULT optcl_list_get_at_index(const optcl_list *list, 
			       uint32_t index, 
			       const void **data)
{
	int forward;
	RESULT error;
	uint32_t count;
	uint32_t i = 0;
	optcl_list_iterator it = 0;

	assert(data != 0);
	assert(list != 0);

	if (data == 0|| list == 0) {
		return(E_INVALIDARG);
	}

	if (index < 0) {
		return(E_OUTOFRANGE);
	}

	error = optcl_list_get_count(list, &count);

	if (FAILED(error)) {
		return(error);
	}

	if (index >= count) {
		return(E_OUTOFRANGE);
	}

	forward = (count - index < index);

	if (forward != 0) {
		error = optcl_list_get_head_pos(list, &it);
		i = 0;
	} else {
		error = optcl_list_get_tail_pos(list, &it);
		i = count - 1;
	}

	if (FAILED(error)) {
		return(error);
	}

	while (it != 0 && i != index) {
		if (forward != 0) {
			error = optcl_list_get_next(list, it, &it);
			++i;
		} else {
			error = optcl_list_get_previous(list, it, &it);
			--i;
		}
	}

	if (it == 0) {
		return(error);
	}
	
	return(optcl_list_get_at_pos(list, it, data));
}

RESULT optcl_list_get_at_pos(const optcl_list *list, 
			     const optcl_list_iterator pos, 
			     const void **data)
{
	assert(pos != 0);
	assert(list != 0);
	assert(data != 0);

	if (list == 0 || pos == 0 || data == 0) {
		return(E_INVALIDARG);
	}

	*data = pos->data;

	return(SUCCESS);
}

RESULT optcl_list_get_count(const optcl_list *list, uint32_t *count)
{
	assert(list != 0);
	assert(count != 0);

	if (list == 0 || count == 0) {
		return(E_INVALIDARG);
	}

	*count = list->node_count;

	return(SUCCESS);
}

RESULT optcl_list_get_head_pos(const optcl_list *list, 
			       optcl_list_iterator *pos)
{
	assert(list != 0);
	assert(pos != 0);

	if (list == 0 || pos == 0) {
		return(E_INVALIDARG);
	}

	*pos = list->first_node;

	return(SUCCESS);
}

RESULT optcl_list_get_tail_pos(const optcl_list *list,
			       optcl_list_iterator *pos)
{
	assert(list != 0);
	assert(pos != 0);

	if (list == 0 || pos == 0) {
		return(E_INVALIDARG);
	}

	*pos = list->last_node;

	return(SUCCESS);
}

RESULT optcl_list_get_next(const optcl_list *list,
			   const optcl_list_iterator pos,
			   optcl_list_iterator *next)
{
	assert(list != 0);
	assert(pos != 0);
	assert(next != 0);

	if (list == 0 || pos == 0 || next == 0) {
		return(E_INVALIDARG);
	}

	*next = pos->next;

	return(SUCCESS); 
}

RESULT optcl_list_get_previous(const optcl_list *list,
			       const optcl_list_iterator pos,
			       optcl_list_iterator *previous)
{
	assert(list != 0);
	assert(pos != 0);
	assert(previous != 0);

	if (list == 0 || pos == 0 || previous == 0) {
		return(E_INVALIDARG);
	}

	*previous = pos->prev;

	return(SUCCESS);
}

RESULT optcl_list_insert_after(optcl_list *list, 
			       const optcl_list_iterator pos, 
			       const void *data)
{
	optcl_list_node *nnode = 0;

	assert(pos != 0);
	assert(list != 0);
	assert(data != 0);

	if (pos == 0 || list == 0 || data == 0) {
		return(E_INVALIDARG);
	}

	nnode = (optcl_list_node*)malloc(sizeof(optcl_list_node));

	if (nnode == 0) {
		return(E_OUTOFMEMORY);
	}

	nnode->data = data;

	if (pos->next != 0) {
		pos->next->prev = nnode;
		nnode->next = pos->next->prev;
	}

	pos->next = nnode;
	nnode->prev = pos;

	list->node_count++;

	return(SUCCESS);
}

RESULT optcl_list_insert_before(optcl_list *list, 
				const optcl_list_iterator pos, 
				const void *data)
{
	optcl_list_node *nnode = 0;

	assert(pos != 0);
	assert(list != 0);
	assert(data != 0);

	if (pos == 0 || list == 0 || data == 0) {
		return(E_INVALIDARG);
	}

	nnode = (optcl_list_node*)malloc(sizeof(optcl_list_node));

	if (nnode == 0) {
		return(E_OUTOFMEMORY);
	}

	nnode->data = data;

	if (pos->prev != 0) {
		pos->prev->next = nnode;
		nnode->prev = pos->prev->next;
	}

	pos->prev = nnode;
	nnode->next = pos;

	list->node_count++;

	return(SUCCESS);
}

RESULT optcl_list_remove(optcl_list *list, optcl_list_iterator pos)
{
	assert(list != 0);
	assert(pos != 0);

	if (list == 0 || pos == 0) {
		return(E_INVALIDARG);
	}

	if (pos->prev == 0) {
		assert(pos == list->first_node);

		if (pos != list->first_node) {
			return(E_UNEXPECTED);
		}

		list->first_node = pos->next;
		
		if (pos->next != 0) {
			pos->next->prev = 0;
		} else {
			list->last_node = 0;
		}
	} else if (pos->next == 0) {
		assert(pos == list->last_node);

		if (pos != list->last_node) {
			return(E_UNEXPECTED);
		}

		list->last_node = pos->prev;

		if (pos->prev != 0) {
			pos->prev->next = 0;
		} else {
			list->first_node = 0;
		}
	} else {
		pos->prev->next = pos->next;
		pos->next->prev = pos->prev;
	}

	list->node_count--;

	free(pos);

	return(SUCCESS);
}

RESULT optcl_list_set_at_pos(const optcl_list *list, 
			     optcl_list_iterator pos, 
			     const void *element)
{
	assert(list != 0);
	assert(pos != 0);

	if (list == 0 || pos == 0) {
		return(E_INVALIDARG);
	}

	pos->data = element;

	return(SUCCESS);
}

RESULT optcl_list_set_equalfn(optcl_list *list, 
			      optcl_list_equalfn equalfn)
{
	assert(list != 0);
	assert(equalfn != 0);

	if (list == 0 || equalfn == 0) {
		return(E_INVALIDARG);
	}

	list->equalfn = equalfn;

	return(SUCCESS);
}

RESULT optcl_list_sort(optcl_list *list)
{
	int i;
	RESULT error;
	RESULT destroy_error;
	uint32_t count;
	void *element = 0;
	optcl_array *array = 0;
	optcl_list_iterator it = 0;
	
	assert(list != 0);

	if (list == 0) {
		return(E_INVALIDARG);
	}

	assert(list->equalfn != 0);

	if (list->equalfn == 0) {
		return(E_UNEXPECTED);
	}

	error = optcl_array_create(sizeof(int*), list->equalfn, &array);

	if (FAILED(error)) {
		return(error);
	}

	error = optcl_list_get_count(list, &count);

	if (FAILED(error)) {
		destroy_error = optcl_array_destroy(array, 0);
		return(SUCCEEDED(destroy_error)) ? error : destroy_error;
	}

	error = optcl_array_set_size(array, count, 0);

	if (FAILED(error)) {
		destroy_error = optcl_array_destroy(array, 0);
		return(SUCCEEDED(destroy_error)) ? error : destroy_error;
	}

	error = optcl_list_get_head_pos(list, &it);

	if (FAILED(error)) {
		destroy_error = optcl_array_destroy(array, 0);
		return(SUCCEEDED(destroy_error)) ? error : destroy_error;
	}

	i = 0;

	while (it != 0) {
		error = optcl_list_get_at_pos(list, it, &element);

		if (FAILED(error)) {
			break;
		}

		error = optcl_array_set(array, i, element);

		++i;

		if (FAILED(error)) {
			break;
		}

		error = optcl_list_get_next(list, it, &it);

		if (FAILED(error)) {
			break;
		}
	}

	if (FAILED(error)) {
		destroy_error = optcl_array_destroy(array, 0);
		return(SUCCEEDED(destroy_error)) ? error : destroy_error;
	}

	assert(list->equalfn != 0);

	if (list->equalfn == 0) {
		destroy_error = optcl_array_destroy(array, 0);
		return(SUCCEEDED(destroy_error)) ? E_UNEXPECTED : destroy_error;
	}

	error = optcl_array_set_equalfn(array, list->equalfn);

	if (FAILED(error)) {
		destroy_error = optcl_array_destroy(array, 0);
		return(SUCCEEDED(destroy_error)) ? error : destroy_error;
	}

	error = optcl_array_sort(array);

	if (FAILED(error)) {
		destroy_error = optcl_array_destroy(array, 0);
		return(SUCCEEDED(destroy_error)) ? error : destroy_error;
	}

	error = optcl_list_get_head_pos(list, &it);

	if (FAILED(error)) {
		destroy_error = optcl_array_destroy(array, 0);
		return(SUCCEEDED(destroy_error)) ? error : destroy_error;
	}

	i = 0;

	while (it != 0) {
		error = optcl_array_get(array, i, &element);

		if (FAILED(error)) {
			break;
		}

		error = optcl_list_set_at_pos(list, it, element);

		if (FAILED(error)) {
			break;
		}

		error = optcl_list_get_next(list, it, &it);

		if (FAILED(error)) {
			break;
		}
	}

	destroy_error = optcl_array_destroy(array, 0);

	return(SUCCEEDED(destroy_error)) ? error : destroy_error;
}
