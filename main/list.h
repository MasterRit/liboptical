/*
    list.h - Double-linked list
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

#ifndef _LIST_H
#define _LIST_H


typedef struct tag_optcl_list optcl_list;

/* List iterator type */
typedef struct tag_optcl_list_node* optcl_list_iterator;

/*
 * Compare two list elements 
 * Return 0 for equal, -1 for left lesser and 1 for left greater.
 */
typedef int (*optcl_list_equalfn)(const void *left, const void *right);


/* Add new element to head of the list */
extern int optcl_list_add_head(optcl_list *list, const void *data);

/* Add new element to tail of the list */
extern int optcl_list_add_tail(optcl_list *list, const void *data);

/* Append list to another */
extern int optcl_list_append(optcl_list *dest, const optcl_list *src);

/* Create new list */
extern int optcl_list_create(const optcl_list_equalfn equalfn, 
			     optcl_list **list);

/* Clear list (removes all elements) */
extern int optcl_list_clear(optcl_list *list, int deallocate);

/* Copy all elements from src to dest list */
extern int optcl_list_copy(optcl_list *dest, const optcl_list *src);

/* Destroy list */
extern int optcl_list_destroy(optcl_list *list, int deallocate);

/* Find element position */
extern int optcl_list_find(const optcl_list *list, 
			   const void *data, 
			   optcl_list_iterator *pos);

/* Get element at index */
extern int optcl_list_get_at_index(const optcl_list *list, 
				   int index, 
				   void **data);

/* Get element at iterator position */
extern int optcl_list_get_at_pos(const optcl_list *list, 
				 const optcl_list_iterator pos, 
				 const void **data);

/* Get list element equalfn function */
extern int optcl_list_get_equalfn(const optcl_list *list, 
				  optcl_list_equalfn *equalfn);

/* Get element count */
extern int optcl_list_get_count(const optcl_list *list, int *count);

/* Get start position iterator */
extern int optcl_list_get_head_pos(const optcl_list *list, 
				   optcl_list_iterator *pos);

/* Get end position iterator */
extern int optcl_list_get_tail_pos(const optcl_list *list, 
				   optcl_list_iterator *pos);

// Get next element in the list
extern int optcl_list_get_next(const optcl_list *list,
			       const optcl_list_iterator pos,
			       optcl_list_iterator *next);

// Get previous element in the list
extern int optcl_list_get_previous(const optcl_list *list,
				   const optcl_list_iterator pos,
				   optcl_list_iterator *previous);

/* Insert new element after iterator position */
extern int optcl_list_insert_after(const optcl_list *list, 
				   const optcl_list_iterator pos, 
				   const void *data);

/* Insert new element before iterator position */
extern int optcl_list_insert_before(const optcl_list *list, 
				    const optcl_list_iterator pos, 
				    const void *data);

/* Remove element from the list */
extern int optcl_list_remove(optcl_list *list, optcl_list_iterator pos);

/* Set element at iterator position */
extern int optcl_list_set_at_pos(const optcl_list *list, 
				 optcl_list_iterator pos, 
				 const void *element
			  );

/* Set list element equalfn */
extern int optcl_list_set_equalfn(optcl_list *list, 
				  optcl_list_equalfn equalfn);

/* Sort */
extern int optcl_list_sort(optcl_list *list);

#endif /* _LIST_H */
