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


#ifdef _DEBUG
// Poison value to detect use of deallocated list elements
#define POISON	((void*)0x12345678)
#endif

struct tag_optcl_array {
	int count;
	int element_size;
	void *buffer;
	optcl_array_equalfn equalfn;
};

typedef struct tag_optcl_array optcl_array;


int optcl_array_append(optcl_array *dest, const optcl_array *src)
{
	int i;
	int error;
	int src_size;
	int dest_size;
	void *element;

	assert(src);
	assert(dest);

	if (!dest || !src)
		return E_INVALIDARG;

	if (dest->element_size != src->element_size)
		return E_SIZEMISMATCH;

	error = optcl_array_get_size(dest, &dest_size);

	if (FAILED(error))
		return error;

	error = optcl_array_get_size(src, &src_size);
		
	if (FAILED(error))
		return error;

	error = optcl_array_set_size(dest, dest_size + src_size, 0);

	if (FAILED(error))
		return error;

	for(i = 0; i < src_size; ++i) {
		error = optcl_array_get(src, i, &element);

		if (FAILED(error))
			break;

		error = optcl_array_set(dest, dest_size + i, element);

		if (FAILED(error))
			break;
	}

	return error;
}

static int compare_ints(const void *left, const void *right)
{
	int larg;
	int rarg;

	assert(left);
	assert(right);

	if (!left || !right)
		return E_INVALIDARG;

	larg = *((int*)left);
	rarg = *((int*)right);

	if (larg == rarg)
		return 0;
	else if (larg < rarg)
		return -1;
	else
		return 1;
}

int optcl_array_create(int element_size, 
		       const optcl_array_equalfn equalfn, 
		       optcl_array **array)
{
	optcl_array *narray;

	assert(array);

	if (!array)
		return E_INVALIDARG;

	
	narray = (optcl_array*)malloc(sizeof(optcl_array));

	if (!narray)
		return E_OUTOFMEMORY;

	memset(narray, 0, sizeof(optcl_array));

	narray->element_size = element_size;
	narray->equalfn = (equalfn) ? equalfn : &compare_ints;

	return SUCCESS;
}

int optcl_array_clear(optcl_array *array, int deallocate)
{
	int error;
	optcl_array_equalfn equalfn;

	assert(array);

	if (!array)
		return E_INVALIDARG;

	equalfn = array->equalfn;

	error = optcl_array_set_size(array, 0, deallocate);

	if (FAILED(error))
		return error;

	array->equalfn = equalfn;

	return SUCCESS;
}

int optcl_array_copy(optcl_array *dest, const optcl_array *src)
{
	int i;
	int error;
	int src_count;
	void *element;

	assert(src);
	assert(dest);

	if (!dest || !src)
		return E_INVALIDARG;

	if (dest->element_size != src->element_size)
		return E_SIZEMISMATCH;

	error = optcl_array_get_size(src, &src_count);

	if (FAILED(error))
		return error;

	error = optcl_array_set_size(dest, src_count, 0);

	if (FAILED(error))
		return error;

	for(i = 0; i < src_count; ++i) {
		error = optcl_array_get(src, i, &element);

		if (FAILED(error))
			break;

		error = optcl_array_set(dest, i, element);

		if (FAILED(error))
			break;
	}

	return error;
}

int optcl_array_destroy(optcl_array *array, int deallocate)
{
	int error;
	assert(array);

	if (!array)
		return E_INVALIDARG;

	 error = optcl_array_set_size(array, 0, deallocate);

	 if (FAILED(error))
		 return error;

#ifdef _DEBUG
	 array->buffer = POISON;
#endif

	 free(array);

	 return error;
}

int optcl_array_find(const optcl_array *array, const void *element, int *index)
{
	int i;
	int error;
	int array_size;
	void *current_element;
	optcl_array_equalfn equalfn;

	assert(array);
	assert(element);
	assert(index);

	if (!array || !element || !index)
		return E_INVALIDARG;

	error = optcl_array_get_size(array, &array_size);

	if (FAILED(error))
		return error;

	equalfn = array->equalfn;
	assert(equalfn);

	if (!equalfn)
		return E_UNEXPECTED;

	for(i = 0; i < array_size; ++i) {
		error = optcl_array_get(array, i, &current_element);

		if (FAILED(error))
			break;

		if (equalfn(current_element, element) != 0)
			continue;

		*index = i;
		break;
	}

	return error;
}

int optcl_array_get(const optcl_array *array, int index, void **element)
{
	int error;
	int element_size;
	void *buffer;

	assert(array);
	assert(element);
	assert(index >= 0);

	if (!array || !element)
		return E_INVALIDARG;

	if (index < 0 || index >= array->count)
		return E_OUTOFRANGE;

	error = optcl_array_get_buffer(array, &buffer);

	if (FAILED(error))
		return error;

	error = optcl_array_get_element_size(array, &element_size);

	if (FAILED(error))
		return error;

	*element = (char*)buffer + element_size * index;

	return SUCCESS;
}

int optcl_array_get_buffer(const optcl_array *array, void **buffer)
{
	assert(array);
	assert(buffer);

	if (!array || !buffer)
		return E_INVALIDARG;

	*buffer = array->buffer;

	return SUCCESS;
}

int optcl_array_get_equalfn(const optcl_array *array, 
			    optcl_array_equalfn *equalfn)
{
	assert(array);
	assert(equalfn);

	if (!array || !equalfn)
		return E_INVALIDARG;

	*equalfn = array->equalfn;

	return SUCCESS;
}

int optcl_array_get_element_size(const optcl_array *array, int *element_size)
{
	assert(array);
	assert(element_size);

	if (!array || !element_size)
		return E_INVALIDARG;

	*element_size = array->element_size;

	return SUCCESS;
}

int optcl_array_get_size(const optcl_array *array, int *size)
{
	assert(array);
	assert(size);

	if (!array || !size)
		return E_INVALIDARG;

	*size = array->count;

	return SUCCESS;
}

int optcl_array_remove(optcl_array *array, int index)
{
	int error;
	int array_size;
	int element_size;
	void *buffer;

	assert(array);
	assert(index >= 0);

	if (!array)
		return E_INVALIDARG;

	error = optcl_array_get_size(array, &array_size);

	if (FAILED(error))
		return error;

	if (index < 0 || index >= array_size)
		return E_OUTOFRANGE;

	error = optcl_array_get_element_size(array, &element_size);

	if (FAILED(error))
		return error;

	error = optcl_array_get_buffer(array, &buffer);

	if (FAILED(error))
		return error;

	if (index < array_size - 1) {
		memcpy(
			(char*)buffer + element_size * index,
			(char*)buffer + element_size * (index + 1),
			element_size * (array_size - index - 1)
			);
	}

	error = optcl_array_set_size(array, array_size - 1, 0);

	return error;
}

int optcl_array_set(optcl_array *array, int index, void *element)
{
	int error;
	int element_size;
	void *buffer;

	assert(array);
	assert(element);
	assert(index >= 0);

	if (!array || !element)
		return E_INVALIDARG;

	if (index < 0 || index >= array->count)
		return E_OUTOFRANGE;

	error = optcl_array_get_element_size(array, &element_size);

	if (FAILED(error))
		return error;

	error = optcl_array_get_buffer(array, &buffer);

	if (FAILED(error))
		return error;

	memcpy((char*)buffer + element_size * index, element, element_size);

	return SUCCESS;
}

int optcl_array_set_equalfn(optcl_array *array, 
			    optcl_array_equalfn equalfn)
{
	assert(array);
	assert(equalfn);

	if (!array || !equalfn)
		return E_INVALIDARG;

	array->equalfn = equalfn;

	return SUCCESS;
}

int optcl_array_set_size(optcl_array *array, int size, int deallocate)
{
	int i;
	int index;
	int error;
	void *buffer;
	void *nbuffer;
	char **dispose;
	int element_size;

	assert(array);
	assert(size >= 0);

	if (!array || size < 0)
		return E_INVALIDARG;

	if (array->count == size)
		return SUCCESS;

	error = optcl_array_get_element_size(array, &element_size);

	if (FAILED(error))
		return error;

	error = optcl_array_get_buffer(array, &buffer);

	if (FAILED(error))
		return error;

	if (deallocate && array->count > size) {
		dispose = (char**)buffer;

		/* Deallocate pointers */
		for(i = 0; i < array->count - size; ++i) {
			index = array->count - size + i;
			free(dispose[index]);
#ifdef _DEBUG
			dispose[index] = POISON;
#endif
		}
	}

	nbuffer = realloc(buffer, size * element_size);

	if (!nbuffer && size > 0)
		return E_OUTOFMEMORY;

	array->count = size;

	return SUCCESS;
}

int optcl_array_sort(optcl_array *array)
{
	int error;
	int array_size;
	int element_size;
	void *buffer;
	optcl_array_equalfn equalfn;

	assert(array);

	if (!array)
		return E_INVALIDARG;

	error = optcl_array_get_size(array, &array_size);

	if (FAILED(error))
		return error;

	error = optcl_array_get_element_size(array, &element_size);

	if (FAILED(error))
		return error;

	error = optcl_array_get_buffer(array, &buffer);

	if (FAILED(error))
		return error;

	equalfn = array->equalfn;
	assert(equalfn);

	if (!equalfn)
		return E_UNEXPECTED;

	qsort(buffer, array_size, element_size, equalfn);

	return SUCCESS;
}
