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

#include "list.h"


/* Hashtable */
typedef struct tag_optcl_hashtable optcl_hashtable;

/* Key/value pair */
typedef struct pair {
	void *key;
	void *value;
} optcl_hashtable_pair;

/* User hash function */
typedef unsigned int (*optcl_hashtable_hashfn)(const void *key, size_t len);



/* Clear hashtable */
int optcl_hashtable_clear(optcl_hashtable *hashtable, int deallocate);

/* Copy hashtable */
int optcl_hashtable_copy(optcl_hashtable *dest, const optcl_hashtable *src);

/* Create new hashtable */
int optcl_hashtable_create(size_t keysize,
			   optcl_hashtable_hashfn hashfn,
			   optcl_hashtable **hashtable);

/* Destroy hashtable */
int optcl_hashtable_destroy(optcl_hashtable *hashtable, int deallocate);

/* Get list of all key/value pairs */
int optcl_hashtable_get_pairs(const optcl_hashtable *hashtable,
			      optcl_list **pairs);

/* Lookup key in the hashtable */
int optcl_hashtable_lookup(const optcl_hashtable *hashtable, 
			   const void *key, 
			   void **value);

/* Set key/value pair */
int optcl_hashtable_set(optcl_hashtable *hashtable, 
			void *key, 
			void *value);

/* Rehash hashtable */
int optcl_hashtable_rehash(optcl_hashtable *hashtable);


#endif /* _HASHTABLE_H */
