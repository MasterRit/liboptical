/*
    helpers.c - Helper functions implementation for Win32
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

#include "helpers.h"
#include "types.h"

#include <assert.h>
#include <malloc.h>
#include <memory.h>
#include <string.h>


/*
 * Memory allocation 
 */

void* xmalloc_aligned(size_t size, size_t alignment)
{
	ptr_t memptr = 0;
	
	memptr = (ptr_t)_aligned_malloc(size, alignment);
	
	if (memptr == 0) {
		return(0);
	}
	
	memset(memptr, 0, size);
	
	return(memptr);
}

void* xrealloc_aligned(void *memblock, size_t size, size_t alignment)
{
	return _aligned_realloc(memblock, size, alignment);
}

void xfree_aligned(void *memoryblock)
{
	_aligned_free(memoryblock);
}

/*
 * Safe string routines
 */

char* xstrdup(const char *string)
{
	return _strdup(string);
}

errno_t xstrncpy(char *dest, size_t dest_size, const char *src, size_t count)
{
	errno_t err = strncpy_s(dest, dest_size, src, count);
	assert(err == 0);
	return(err);
}

errno_t xstrcat(char *dest, size_t dest_size, const char *src)
{
	errno_t err = strcat_s(dest, dest_size, src);
	assert(err == 0);
	return(err);
}

/*
 * Safe memory routines
 */

errno_t xmemcpy(void *dest, size_t dest_size, const void *src, size_t count)
{
	errno_t err = memcpy_s(dest, dest_size, src, count);
	assert(err == 0);
	return(err);
}
