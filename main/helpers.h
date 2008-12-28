/*
    helpers.h - Helper functions and macros
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

#ifndef _HELPERS_H
#define _HELPERS_H

#include <stdlib.h>

#ifndef WIN_32

typedef int errno_t;

#endif

/*
 * Memory allocation
 */

extern void* xmalloc_aligned(size_t size, size_t alignment);
extern void* xrealloc_aligned(void *memblock, size_t size, size_t alignment);
extern void xfree_aligned(void *memoryblock);

/*
 * Safe string routines
 */

extern char* xstrdup(const char *string);

extern errno_t
    xstrncpy(char *dest, size_t dest_size, const char *src, size_t count);

extern errno_t xstrcat(char *dest, size_t dest_size, const char *src);

/*
 * Safe memory routines
 */

extern errno_t
    xmemcpy(void *dest, size_t dest_size, const void *src, size_t count);


#endif /* _HELPERS_H */
