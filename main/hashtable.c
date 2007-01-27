/*
    hashtable.c - Hashtable
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
#include "hashtable.h"
#include "types.h"

#include <assert.h>
#include <memory.h>
#include <stdlib.h>


/*
 * Helper constants
 */

#ifdef _DEBUG
// Poison value to detect use of deallocated pointers
#define POISON	((void*)0x12345678)
#endif

static const unsigned int primes[] = {
	53, 97, 193, 389,
	769, 1543, 3079, 6151,
	12289, 24593, 49157, 98317,
	196613, 393241, 786433, 1572869,
	3145739, 6291469, 12582917, 25165843,
	50331653, 100663319, 201326611, 402653189,
	805306457, 1610612741
};

const float max_load_factor = 0.65f;
const uint32_t prime_table_length = sizeof(primes) / sizeof(primes[0]);


/*
 * Internal hashtable data structures
 */

struct entry {
	uint32_t hash;
	struct pair pair;
	optcl_list *bucket;
};

struct tag_optcl_hashtable {
	uint32_t keycount;
	uint32_t keysize;
	uint32_t primeindex;
	optcl_array *entries;
	optcl_hashtable_hashfn hashfn;
};

/*
 * Helpers
 */

static uint32_t joaat_hash(const void *key, uint32_t len)
{
     uint32_t i;
     uint32_t hash = 0;
     
     for (i = 0; i < len; i++) {
         hash += ((uint8_t*)key)[i];
         hash += (hash << 10);
         hash ^= (hash >> 6);
     }

     hash += (hash << 3);
     hash ^= (hash >> 11);
     hash += (hash << 15);

     return hash;
}

/*
 *  Hashtable functions implementation
 */

RESULT optcl_hashtable_clear(optcl_hashtable *hashtable, bool_t deallocate)
{
	uint32_t i;
	RESULT error;
	uint32_t entrycount;
	struct entry *entry;

	assert(hashtable);

	if (!hashtable) {
		return E_INVALIDARG;
	}

	if (!hashtable->entries) {
		return SUCCESS;
	}

	error = optcl_array_get_size(hashtable->entries, &entrycount);

	if (FAILED(error)) {
		return error;
	}

	for(i = 0; i < entrycount; ++i) {
		error = optcl_array_get(hashtable->entries, i, &entry);

		if (FAILED(error)) {
			break;
		}

		if (entry->bucket) {
			error = optcl_list_destroy(entry->bucket, deallocate);

			if (FAILED(error)) {
				break;
			}
		}
	}

	if (FAILED(error)) {
		return error;
	}

	error = optcl_array_destroy(hashtable->entries, 0);

#ifdef _DEBUG
	if (SUCCEEDED(error)) {
		hashtable->entries = POISON;
	}
#endif

	return error;
}

RESULT optcl_hashtable_copy(optcl_hashtable *dest, const optcl_hashtable *src)
{
	uint32_t i;
	RESULT error;
	struct entry *entry;
	optcl_list *nbucket;
	optcl_list_equalfn equalfn;

	assert(src);
	assert(dest);

	if (!dest || !src) {
		return E_INVALIDARG;
	}

	error = optcl_hashtable_clear(dest, 0);

	if (FAILED(error)) {
		return error;
	}

	dest->hashfn = src->hashfn;
	dest->keysize = src->keysize;
	dest->primeindex = src->primeindex;

	error = optcl_array_copy(dest->entries, src->entries);

	if (FAILED(error)) {
		return error;
	}

	error = optcl_array_get_size(dest->entries, &dest->keycount);

	if (FAILED(error)) {
		return error;
	}

	for(i = 0; i < dest->keycount; ++i) {
		error = optcl_array_get(dest->entries, i, &entry);

		if (FAILED(error)) {
			break;
		}

		error = optcl_list_get_equalfn(entry->bucket, &equalfn);

		if (FAILED(error)) {
			break;
		}

		error = optcl_list_create(equalfn, &nbucket);

		if (FAILED(error)) {
			break;
		}

		error = optcl_list_copy(nbucket, entry->bucket);

		if (FAILED(error)) {
			optcl_list_destroy(nbucket, 0);
			break;
		}

		entry->bucket = nbucket;
	}

	return error;
}

RESULT optcl_hashtable_create(uint32_t keysize,
			      optcl_hashtable_hashfn hashfn,
			      optcl_hashtable **hashtable)
{
	optcl_hashtable *nhashtable;

	assert(hashtable);
	assert(keysize >0);

	if (!hashtable || keysize <= 0) {
		return E_INVALIDARG;
	}

	nhashtable = (optcl_hashtable*)malloc(sizeof(optcl_hashtable));

	if (!nhashtable) {
		return E_OUTOFMEMORY;
	}

	memset(nhashtable, 0, sizeof(optcl_hashtable));

	nhashtable->hashfn = (hashfn) ? hashfn : joaat_hash;
	nhashtable->keysize = keysize;

	*hashtable = nhashtable;

	return SUCCESS;
}

RESULT optcl_hashtable_destroy(optcl_hashtable *hashtable, bool_t deallocate)
{
	RESULT error;

	assert(hashtable);

	if (!hashtable) {
		return E_INVALIDARG;
	}

	error = optcl_hashtable_clear(hashtable, deallocate);

	if (SUCCEEDED(error)) {
		free(hashtable);
	}

	return SUCCESS;
}

RESULT optcl_hashtable_get_pairs(const optcl_hashtable *hashtable,
				 optcl_list **pairs)
{
	uint32_t i;
	RESULT error;
	uint32_t entries_count;
	struct pair *pair;
	struct pair *bucket_pair;
	struct entry *entry;
	optcl_list *pair_list;
	optcl_list_iterator it;

	assert(hashtable);
	assert(pairs);

	if (!hashtable || !pairs) {
		return E_INVALIDARG;
	}

	*pairs = 0;

	if (!hashtable->entries) {
		return SUCCESS;
	}

	error = optcl_array_get_size(hashtable->entries, &entries_count);

	if (FAILED(error)) {
		return error;
	}

	if (entries_count < 1) {
		return SUCCESS;
	}

	error = optcl_list_create(0, &pair_list);

	if (FAILED(error)) {
		return error;
	}

	for(i = 0; i < entries_count; ++i) {
		error = optcl_array_get(hashtable->entries, i, &entry);

		if (FAILED(error)) {
			break;
		}

		if (!entry) {
			continue;
		}

		pair = (struct pair*)malloc(sizeof(struct pair));

		if (!pair) {
			error = E_OUTOFMEMORY;
			break;
		}

		pair->key = entry->pair.key;
		pair->value = entry->pair.value;

		error = optcl_list_add_tail(pair_list, pair);

		if (FAILED(error)) {
			free(pair);
			break;
		}

		if (!entry->bucket) {
			continue;
		}

		error = optcl_list_get_head_pos(entry->bucket, &it);

		if (FAILED(error)) {
			break;
		}

		while (it) {
			error = optcl_list_get_at_pos(entry->bucket, it, &bucket_pair);

			if (FAILED(error)) {
				break;
			}

			if (!bucket_pair) {
				error = E_COLLINVLDHASHTABLE;
				break;
			}

			pair = (struct pair*)malloc(sizeof(struct pair));

			if (!pair) {
				error = E_OUTOFMEMORY;
				break;
			}

			pair->key = bucket_pair->key;
			pair->value = bucket_pair->value;

			error = optcl_list_add_tail(pair_list, pair);

			if (FAILED(error)) {
				free(pair);
				break;
			}

			error = optcl_list_get_next(entry->bucket, it, &it);

			if (FAILED(error)) {
				break;
			}
		}

		if (FAILED(error)) {
			break;
		}
	}

	if (FAILED(error)) {
		/* Destroy created list of pairs on error */
		optcl_list_destroy(pair_list, 1);
		pair_list = 0;
	}

	*pairs = pair_list;

	return error;
}

RESULT optcl_hashtable_lookup(const optcl_hashtable *hashtable, 
			      const void *key, 
			      void **value)
{
	RESULT error;
	uint32_t hash;
	uint32_t entrycount;
	struct pair *pair;
	struct entry *entry;
	optcl_list_iterator it;
	optcl_hashtable_hashfn hashfn;

	assert(key);
	assert(value);
	assert(hashtable);

	if (!hashtable || !key || !value) {
		return E_INVALIDARG;
	}

	*value = 0;

	assert(hashtable->hashfn);
	assert(hashtable->keysize > 0);

	if (!hashtable->hashfn || hashtable->keysize <= 0) {
		return E_COLLINVLDHASHTABLE;
	}

	hashfn = hashtable->hashfn;
	hash = hashfn(key, hashtable->keysize);

	if (!hashtable->entries) {
		return SUCCESS;
	}

	error = optcl_array_get_size(hashtable->entries, &entrycount);

	if (FAILED(error)) {
		return error;
	}

	assert(entrycount > 0);

	if (entrycount <= 0) {
		return E_COLLINVLDHASHTABLE;
	}

	error = optcl_array_get(hashtable->entries, hash % entrycount, &entry);

	if (FAILED(error)) {
		return error;
	}

	assert(entry);

	if (!entry) {
		return E_COLLINVLDHASHTABLE;
	}

	if (hashfn(entry->pair.key, hashtable->keysize) == hash) {
		assert(!entry->bucket);
		*value = entry->pair.value;
		return SUCCESS;
	}

	error = optcl_list_get_head_pos(entry->bucket, &it);

	if (FAILED(error)) {
		return error;
	}

	assert(it);

	if (!it) {
		return E_COLLINVLDHASHTABLE;
	}

	while (it) {
		error = optcl_list_get_at_pos(entry->bucket, it, &pair);

		if (FAILED(error)) {
			break;
		}

		if (hashfn(pair->key, hashtable->keysize) == hash) {
			*value = pair->value;
			break;
		}

		error = optcl_list_get_next(entry->bucket, it, &it);

		if (FAILED(error)) {
			break;
		}
	}

	return SUCCESS;
}

RESULT optcl_hashtable_set(optcl_hashtable *hashtable, 
			   void *key, 
			   void *value)
{
	uint32_t hash;
	uint32_t size;
	RESULT error;
	float loadfactor;
	struct pair *pair;
	struct entry *entry;

	assert(key);
	assert(hashtable);

	if (!key || !hashtable) {
		return E_INVALIDARG;
	}

	assert(hashtable->hashfn);
	assert(hashtable->keysize > 0);

	if (hashtable->keysize <= 0) {
		return E_COLLINVLDHASHTABLE;
	}

	assert(hashtable->entries);

	if (!hashtable->entries) {
		size = 0;
	} else {
		error = optcl_array_get_size(hashtable->entries, &size);

		if (FAILED(error)) {
			return error;
		}
	}

	loadfactor =(size * 1.0f) / (hashtable->keycount * 1.0f);

	if (loadfactor >= max_load_factor) {
		error = optcl_hashtable_rehash(hashtable);

		if (FAILED(error)) {
			return error;
		}

		error = optcl_array_get_size(hashtable->entries, &size);

		if (FAILED(error)) {
			return error;
		}
	}

	hash = hashtable->hashfn(key, hashtable->keysize);

	error = optcl_array_get(hashtable->entries, hash % size, &entry);

	if (FAILED(error)) {
		return error;
	}

	if (entry) {
		pair = (struct pair*)malloc(sizeof(struct pair));

		if (!pair) {
			return E_OUTOFMEMORY;
		}

		pair->key = key;
		pair->value = value;

		if (!entry->bucket) {
			error = optcl_list_create(0, &entry->bucket);

			if (FAILED(error)) {
				free(pair);
				return error;
			}
		}

		error = optcl_list_add_head(entry->bucket, pair);

		if (FAILED(error)) {
			free(pair);
			return error;
		}
	} else {
		entry = (struct entry*)malloc(sizeof(struct entry));

		if (!entry) {
			return E_OUTOFMEMORY;
		}

		memset(entry, 0, sizeof(struct entry));

		entry->hash = hash;
		entry->pair.key = key;
		entry->pair.value = value;

		error = optcl_array_set(hashtable->entries, hash % size, entry);

		if (FAILED(error)) {
			return error;
		}
	}
	
	return error;
}

RESULT optcl_hashtable_rehash(optcl_hashtable *hashtable)
{
	uint32_t i;
	uint32_t hash;
	uint32_t size;
	uint32_t nsize;
	RESULT error;
	struct entry *entry;
	optcl_array *nentries;

	assert(hashtable);

	if (!hashtable) {
		return E_INVALIDARG;
	}

	if (hashtable->primeindex + 1 > prime_table_length - 1) {
		return E_OUTOFRANGE;
	}

	error = optcl_array_create(sizeof(struct entry), 0, &nentries);

	if (FAILED(error)) {
		return error;
	}

	if (!hashtable->entries) {
		assert(hashtable->keycount == 0);
		assert(hashtable->primeindex == 0);

		hashtable->entries = nentries;

		return optcl_array_set_size(nentries, primes[0], 0);
	}

	nsize = primes[hashtable->primeindex + 1];

	error = optcl_array_set_size(nentries, nsize, 0);

	if (FAILED(error)) {
		optcl_array_destroy(nentries, 0);
		return error;
	}

	error = optcl_array_get_size(hashtable->entries, &size);

	if (FAILED(error)) {
		optcl_array_destroy(nentries, 0);
		return error;
	}

	for(i = 0; i < size; ++i) {
		error = optcl_array_get(hashtable->entries, i, &entry);

		if (FAILED(error)) {
			break;
		}

		if (!entry) {
			continue;
		}

		hash = hashtable->hashfn(entry->pair.key, hashtable->keysize);

		error = optcl_array_set(nentries, hash % nsize, entry);

		if (FAILED(error)) {
			break;
		}
	}

	if (FAILED(error)) {
		optcl_array_destroy(nentries, 0);
	} else {
		error = optcl_array_destroy(hashtable->entries, 0);
		hashtable->entries = nentries;
	}

	return error;
}
