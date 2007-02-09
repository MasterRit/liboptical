/*
    array.c - Array
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

#include "stdafx.h"

#include "array.h"
#include "errors.h"

#include <assert.h>
#include <memory.h>
#include <stdlib.h>


/*
 * Array internal structures
 */

struct tag_optcl_array {
	void *buffer;
	uint32_t count;
	uint32_t element_size;
	optcl_array_equalfn equalfn;
};

typedef struct tag_optcl_array optcl_array;


/*
 * Helper data types
 */

/* qsort comparator function pointer */
typedef int (*qsort_equalfn)(const void *, const void *);


/*
 * Helpers functions
 */

static int8_t compare_ints(const void *left, const void *right)
{
	int8_t result;
	uint32_t larg;
	uint32_t rarg;

	assert(left != 0);
	assert(right != 0);

	if (left == 0 || right == 0) {
		return(E_INVALIDARG);
	}

	memcpy(&larg, left, sizeof(int));
	memcpy(&rarg, right, sizeof(int));

	if (larg == rarg) {
		result = 0;
	} else if (larg < rarg) {
		result = -1;
	} else {
		result = 1;
	}

	return(result);
}


/*
 * Array functions implementation
 */

RESULT optcl_array_append(optcl_array *dest, const optcl_array *src)
{
	uint32_t i;
	RESULT error;
	uint32_t src_size;
	uint32_t dest_size;
	void *element = 0;

	assert(src != 0);
	assert(dest != 0);

	if (dest == 0 || src == 0) {
		return(E_INVALIDARG);
	}

	if (dest->element_size != src->element_size) {
		return(E_SIZEMISMATCH);
	}

	error = optcl_array_get_size(dest, &dest_size);

	if (FAILED(error)) {
		return(error);
	}

	error = optcl_array_get_size(src, &src_size);
		
	if (FAILED(error)) {
		return(error);
	}

	error = optcl_array_set_size(dest, dest_size + src_size, 0);

	if (FAILED(error)) {
		return(error);
	}

	i = 0;

	while (i < src_size) {

		error = optcl_array_get(src, i, &element);

		if (FAILED(error)) {
			break;
		}

		error = optcl_array_set(dest, dest_size + i, element);

		if (FAILED(error)) {
			break;
		}

		++i;
	}

	return(error);
}

RESULT optcl_array_create(uint32_t element_size, 
			  const optcl_array_equalfn equalfn, 
			  optcl_array **array)
{
	optcl_array *narray = 0;

	assert(array != 0);

	if (array == 0) {
		return(E_INVALIDARG);
	}

	narray = (optcl_array*)malloc(sizeof(optcl_array));

	if (narray == 0) {
		return(E_OUTOFMEMORY);
	}

	memset(narray, 0, sizeof(optcl_array));

	narray->element_size = element_size;
	narray->equalfn = (equalfn) ? equalfn : &compare_ints;

	*array = narray;

	return(SUCCESS);
}

RESULT optcl_array_clear(optcl_array *array, bool_t deallocate)
{
	RESULT error;

	assert(array != 0);

	if (array == 0) {
		return(E_INVALIDARG);
	}

	error = optcl_array_set_size(array, 0, deallocate);

	if (FAILED(error)) {
		return(error);
	}

	array->equalfn = compare_ints;

	return(SUCCESS);
}

RESULT optcl_array_copy(optcl_array *dest, const optcl_array *src)
{
	uint32_t i;
	RESULT error;
	uint32_t src_count;
	void *element = 0;

	assert(src != 0);
	assert(dest != 0);

	if (dest == 0 || src == 0) {
		return(E_INVALIDARG);
	}

	if (dest->element_size != src->element_size) {
		return(E_SIZEMISMATCH);
	}

	error = optcl_array_get_size(src, &src_count);

	if (FAILED(error)) {
		return(error);
	}

	error = optcl_array_set_size(dest, src_count, 0);

	if (FAILED(error)) {
		return(error);
	}

	i = 0;

	while (i < src_count) {

		error = optcl_array_get(src, i, &element);

		if (FAILED(error)) {
			break;
		}

		error = optcl_array_set(dest, i, element);

		if (FAILED(error)) {
			break;
		}

		++i;
	}

	return(error);
}

RESULT optcl_array_destroy(optcl_array *array, bool_t deallocate)
{
	RESULT error;
	
	assert(array != 0);

	if (array == 0) {
		return(E_INVALIDARG);
	}

	 error = optcl_array_set_size(array, 0, deallocate);

	 if (FAILED(error)) {
		 return(error);
	 }

	 free(array);

	 return(error);
}

RESULT optcl_array_find(const optcl_array *array, 
			const void *element, 
			uint32_t *index)
{
	uint32_t i;
	RESULT error;
	uint32_t array_size;
	void *current_element = 0;
	optcl_array_equalfn equalfn = 0;

	assert(array != 0);
	assert(element != 0);
	assert(index != 0);

	if (array == 0 || element == 0 || index == 0) {
		return(E_INVALIDARG);
	}

	error = optcl_array_get_size(array, &array_size);

	if (FAILED(error)) {
		return(error);
	}

	equalfn = array->equalfn;
	
	assert(equalfn != 0);

	if (equalfn == 0) {
		return(E_UNEXPECTED);
	}

	i = 0;

	while (i < array_size) {

		error = optcl_array_get(array, i, &current_element);

		if (FAILED(error)) {
			break;
		}

		if (equalfn(current_element, element) != 0) {
			continue;
		}

		*index = i;

		++i;

		break;
	}

	return(error);
}

RESULT optcl_array_get(const optcl_array *array, 
		       uint32_t index, 
		       const void **element)
{
	RESULT error;
	ptr_t buffer = 0;

	assert(array != 0);
	assert(element != 0);

	if (array == 0 || element == 0) {
		return(E_INVALIDARG);
	}

	assert(index >= 0 && index < array->count);

	if (index < 0 || index >= array->count) {
		return(E_OUTOFRANGE);
	}

	error = optcl_array_get_buffer(array, &buffer);

	if (FAILED(error)) {
		return(error);
	}

	assert(buffer != 0);

	if (buffer == 0) {
		return(E_POINTER);
	}

	*element = *(ptr_t**)&buffer[index * sizeof(ptr_t)];

	return(SUCCESS);
}

RESULT optcl_array_get_buffer(const optcl_array *array, void **buffer)
{
	assert(array != 0);
	assert(buffer != 0);

	if (array == 0 || buffer == 0) {
		return(E_INVALIDARG);
	}

	*buffer = array->buffer;

	return(SUCCESS);
}

RESULT optcl_array_get_equalfn(const optcl_array *array,
			       optcl_array_equalfn *equalfn)
{
	assert(array != 0);
	assert(equalfn != 0);

	if (array == 0 || equalfn == 0) {
		return(E_INVALIDARG);
	}

	*equalfn = array->equalfn;

	return(SUCCESS);
}

RESULT optcl_array_get_element_size(const optcl_array *array, 
				    uint32_t *element_size)
{
	assert(array != 0);
	assert(element_size != 0);

	if (array == 0 || element_size == 0) {
		return(E_INVALIDARG);
	}

	*element_size = array->element_size;

	return(SUCCESS);
}

RESULT optcl_array_get_size(const optcl_array *array, uint32_t *size)
{
	assert(array != 0);
	assert(size != 0);

	if (array == 0 || size == 0) {
		return(E_INVALIDARG);
	}

	*size = array->count;

	return(SUCCESS);
}

RESULT optcl_array_remove(optcl_array *array, uint32_t index)
{
	RESULT error;
	ptr_t buffer = 0;
	uint32_t array_size;
	uint32_t element_size;

	assert(array != 0);
	assert(index >= 0);

	if (array == 0) {
		return(E_INVALIDARG);
	}

	error = optcl_array_get_size(array, &array_size);

	if (FAILED(error)) {
		return(error);
	}

	if (index < 0 || index >= array_size) {
		return(E_OUTOFRANGE);
	}

	error = optcl_array_get_element_size(array, &element_size);

	if (FAILED(error)) {
		return(error);
	}

	error = optcl_array_get_buffer(array, &buffer);

	if (FAILED(error)) {
		return(error);
	}

	if (index < array_size - 1) {
		memcpy(
			&buffer[index * sizeof(ptr_t)],
			&buffer[(index + 1) * sizeof(ptr_t)],
			(array_size - index - 1) * sizeof(ptr_t)
			);
	}

	return(optcl_array_set_size(array, array_size - 1, 0));
}

RESULT optcl_array_set(optcl_array *array, uint32_t index, const void *element)
{
	RESULT error;
	ptr_t buffer = 0;

	assert(array != 0);
	assert(element != 0);

	if (array == 0 || element == 0) {
		return(E_INVALIDARG);
	}

	assert(index >= 0 && index < array->count);

	if (index < 0 || index >= array->count) {
		return(E_OUTOFRANGE);
	}

	error = optcl_array_get_buffer(array, &buffer);

	if (FAILED(error)) {
		return(error);
	}

	memcpy(&buffer[index * sizeof(ptr_t)], &element, sizeof(ptr_t));

	return(SUCCESS);
}

RESULT optcl_array_set_equalfn(optcl_array *array, 
			       optcl_array_equalfn equalfn)
{
	assert(array != 0);
	assert(equalfn != 0);

	if (array == 0 || equalfn == 0) {
		return(E_INVALIDARG);
	}

	array->equalfn = equalfn;

	return(SUCCESS);
}

RESULT optcl_array_set_size(optcl_array *array, 
			    uint32_t size, 
			    bool_t deallocate)
{
	RESULT error;
	uint32_t i;
	uint32_t index;
	ptr_t buffer = 0;
	ptr_t nbuffer = 0;

	assert(array != 0);
	assert(size >= 0);

	if (array == 0 || size < 0) {
		return(E_INVALIDARG);
	}

	if (array->count == size) {
		return(SUCCESS);
	}

	error = optcl_array_get_buffer(array, &buffer);

	if (FAILED(error)) {
		return(error);
	}

	if (deallocate == True && array->count > size) {
	
		/* Deallocate pointers */
		for(i = 0; i < array->count - size; ++i) {
			index = array->count - size + i;
			free(*(ptr_t**)&buffer[index * sizeof(ptr_t)]);
		}
	}

	nbuffer = realloc(buffer, size * sizeof(ptr_t));

	if (nbuffer == 0 && size > 0) {
		return(E_OUTOFMEMORY);
	}

	if (array->count < size) {
		memset(
			&nbuffer[array->count * sizeof(ptr_t)],
			0, 
			(size - array->count) * sizeof(ptr_t)
			);
	}

	array->count = size;
	array->buffer = nbuffer;

	return(SUCCESS);
}

RESULT optcl_array_sort(optcl_array *array)
{
	RESULT error;
	ptr_t buffer = 0;
	uint32_t array_size;
	uint32_t element_size;
	optcl_array_equalfn equalfn = 0;

	assert(array != 0);

	if (array == 0) {
		return(E_INVALIDARG);
	}

	error = optcl_array_get_size(array, &array_size);

	if (FAILED(error)) {
		return(error);
	}

	error = optcl_array_get_element_size(array, &element_size);

	if (FAILED(error)) {
		return(error);
	}

	error = optcl_array_get_buffer(array, &buffer);

	if (FAILED(error)) {
		return(error);
	}

	equalfn = array->equalfn;

	assert(equalfn != 0);

	if (equalfn == 0) {
		return(E_UNEXPECTED);
	}

	qsort(buffer, array_size, element_size, (qsort_equalfn)equalfn);

	return(SUCCESS);
}
