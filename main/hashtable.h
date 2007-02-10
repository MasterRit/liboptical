/*
    hashtable.h - Hashtable
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

#ifndef _HASHTABLE_H
#define _HASHTABLE_H

#include "errors.h"
#include "list.h"
#include "types.h"


/* Hashtable */
typedef struct tag_optcl_hashtable optcl_hashtable;

/* Key/value pair */
typedef struct pair {
	ptr_t key;
	ptr_t value;
} optcl_hashtable_pair;

/* User hash function */
typedef unsigned int (*optcl_hashtable_hashfn)(const uint8_t key[], uint32_t len);



/* Clear hashtable */
extern RESULT optcl_hashtable_clear(optcl_hashtable *hashtable, 
				    bool_t deallocate);

/* Copy hashtable */
extern RESULT optcl_hashtable_copy(optcl_hashtable *dest, 
				   const optcl_hashtable *src);

/* Create new hashtable */
extern RESULT optcl_hashtable_create(uint32_t keysize,
				     optcl_hashtable_hashfn hashfn,
				     optcl_hashtable **hashtable);

/* Destroy hashtable */
extern RESULT optcl_hashtable_destroy(optcl_hashtable *hashtable, 
				      bool_t deallocate);

/* Get list of all key/value pairs */
extern RESULT optcl_hashtable_get_pairs(const optcl_hashtable *hashtable,
					optcl_list **pairs);

/* Lookup key in the hashtable */
extern RESULT optcl_hashtable_lookup(const optcl_hashtable *hashtable, 
				     const ptr_t key, 
				     const pptr_t value);

/* Set key/value pair */
extern RESULT optcl_hashtable_set(optcl_hashtable *hashtable, 
				  const ptr_t key, 
				  const ptr_t value);

/* Rehash hashtable */
extern RESULT optcl_hashtable_rehash(optcl_hashtable *hashtable);


#endif /* _HASHTABLE_H */
