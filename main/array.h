/*
    array.h - Array
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

#ifndef _ARRAY_H
#define _ARRAY_H

#include "errors.h"
#include "types.h"


/*
 * Compares two list elements
 * Return 0 for equal, -1 for left lesser and 1 for left greater.
 */
typedef int8_t (*optcl_array_equalfn)(const ptr_t left, const ptr_t right);

struct tag_optcl_array;
typedef struct tag_optcl_array optcl_array;


/* Append two arrays */
extern 
RESULT optcl_array_append(optcl_array *dest, const optcl_array *src);

/* Create new array */
extern 
RESULT optcl_array_create(uint32_t element_size,
                          const optcl_array_equalfn equalfn, optcl_array **array);

/* Clear new array */
extern 
RESULT optcl_array_clear(optcl_array *array, bool_t deallocate);

/* Copy one array to another */
extern 
RESULT optcl_array_copy(optcl_array *dest, const optcl_array *src);

/* Destroy list */
extern 
RESULT optcl_array_destroy(optcl_array *array, bool_t deallocate);

/* Find element in the array */
extern 
RESULT optcl_array_find(const optcl_array *array, 
                        const ptr_t element, uint32_t *index);

/* Get element at index */
extern 
RESULT optcl_array_get(const optcl_array *array,
                       uint32_t index, const pptr_t element);

/* Get internal buffer */
extern 
RESULT optcl_array_get_buffer(const optcl_array *array, pptr_t buffer);

/* Get array equalfn function */
extern 
RESULT optcl_array_get_equalfn(const optcl_array *array, 
                               optcl_array_equalfn *equalfn);

/* Get array element size */
extern 
RESULT optcl_array_get_element_size(const optcl_array *array, 
                                    uint32_t *element_size);

/* Get array size */
extern 
RESULT optcl_array_get_size(const optcl_array *array, uint32_t *size);

/* Remove element from the array */
extern 
RESULT optcl_array_remove(optcl_array *array, uint32_t index);

/* Set element at index position - resize list if required. */
extern 
RESULT optcl_array_set(optcl_array *array, uint32_t index, 
                       const ptr_t element);

/* Resize array */
extern 
RESULT optcl_array_set_size(optcl_array *array, uint32_t size, 
                            bool_t deallocate);

/* Set array equalfn function */
extern 
RESULT optcl_array_set_equalfn(optcl_array *array, 
                               optcl_array_equalfn equalfn);

/* Sort array */
extern 
RESULT optcl_array_sort(optcl_array *array);

#endif /* _ARRAY_H */
