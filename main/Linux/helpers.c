/*
    helpers.c - Helper functions implementation for Linux
    Copyright (C) 2007  Aleksandar Dezelin <dezelin@gmail.com>

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

#include "../helpers.h"
#include "../types.h"

#define _GNU_SOURCE
#define __STDC_WANT_LIB_EXT1__

#include <assert.h>
#include <errno.h>
#include <malloc.h>
#include <string.h>


/*
 * Memory allocation 
 */

void* xmalloc_aligned(size_t size, size_t alignment)
{
	ptr_t memptr = 0;
	
	posix_memalign((void**)&memptr, alignment, size);
	
	if (memptr == 0) {
		return(0);
	}
	
	memset(memptr, 0, size);
	
	return(memptr);
}

void* xrealloc_aligned(void *memblock, size_t size, size_t alignment)
{
	return(realloc(memblock, size));
}

void xfree_aligned(void *memoryblock)
{
	free(memoryblock);
}

/*
 * Safe string routines
 */

char* xstrdup(const char *string)
{
	return(strdup(string));
}

errno_t xstrncpy(char *dest, size_t dest_size, const char *src, size_t count)
{
	assert(count <= dest_size);
	
	if (count > dest_size) {
		return(EINVAL);
	}
	
	strncpy(dest, src, count);
	
	return(errno);
}

errno_t xstrcat(char *dest, size_t dest_size, const char *src)
{
	int dest_len = strlen(dest);
	int src_len = strlen(src);
	
	assert(dest_len + src_len < dest_size);
	
	if (dest_len + src_len >= dest_size) {
		return(EINVAL);
	}
	
	strcat(dest, src);
	
	return(errno);
}

/*
 * Safe memory routines
 */

errno_t xmemcpy(void *dest, size_t dest_size, const void *src, size_t count)
{
	assert(count <= dest_size);
	
	if (count > dest_size) {
		return(EINVAL);
	}
	
	memcpy(dest, src, count);

	return(errno);
}
