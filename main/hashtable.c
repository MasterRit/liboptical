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

static uint32_t joaat_hash(const uint8_t key[], uint32_t len)
{
    uint32_t i;
    uint32_t hash = 0U;

    assert(key != 0);
    if (key == 0)
        return(0);

    for (i = 0; i < len; ++i) {
        hash += key[i];
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
    struct entry *entry = 0;

    assert(hashtable != 0);
    if (hashtable == 0)
        return E_INVALIDARG;

    if (hashtable->entries == 0)
        return SUCCESS;

    error = optcl_array_get_size(hashtable->entries, &entrycount);
    if (FAILED(error))
        return error;

    i = 0;
    while (i < entrycount) {
        error = optcl_array_get(hashtable->entries, i, (pptr_t)&entry);
        if (FAILED(error))
            break;

        if (entry == 0) {
            ++i;
            continue;
        }

        if (deallocate == True) {
            if (entry->pair.key != entry->pair.value)
                free((void*)entry->pair.value);

            free((void*)entry->pair.key);
        }

        if (entry->bucket != 0) {
            error = optcl_list_destroy(entry->bucket, deallocate);
            if (FAILED(error))
                break;
        }

        ++i;
    }

    if (FAILED(error))
        return error;

    error = optcl_array_destroy(hashtable->entries, deallocate);
    return error;
}

RESULT optcl_hashtable_copy(optcl_hashtable *dest, const optcl_hashtable *src)
{
    uint32_t i;
    RESULT error;
    RESULT destroy_error;
    struct entry *entry = 0;
    optcl_list *nbucket = 0;
    optcl_list_equalfn equalfn = 0;

    assert(src != 0);
    assert(dest != 0);
    if (dest == 0 || src == 0)
        return E_INVALIDARG;

    error = optcl_hashtable_clear(dest, 0);
    if (FAILED(error))
        return error;

    dest->hashfn = src->hashfn;
    dest->keysize = src->keysize;
    dest->primeindex = src->primeindex;
    error = optcl_array_copy(dest->entries, src->entries);
    if (FAILED(error))
        return error;

    error = optcl_array_get_size(dest->entries, &dest->keycount);
    if (FAILED(error))
        return error;

    destroy_error = SUCCESS;
    i = 0;
    while (i < dest->keycount) {
        error = optcl_array_get(dest->entries, i, (pptr_t)&entry);
        if (FAILED(error))
            break;

        error = optcl_list_get_equalfn(entry->bucket, &equalfn);
        if (FAILED(error))
            break;

        error = optcl_list_create(equalfn, &nbucket);
        if (FAILED(error))
            break;

        error = optcl_list_copy(nbucket, entry->bucket);
        if (FAILED(error)) {
            destroy_error = optcl_list_destroy(nbucket, 0);
            break;
        }

        entry->bucket = nbucket;
        ++i;
    }

    return SUCCEEDED(destroy_error) ? error : destroy_error;
}

RESULT optcl_hashtable_create(uint32_t keysize,
                              optcl_hashtable_hashfn hashfn,
                              optcl_hashtable **hashtable)
{
    RESULT error;
    optcl_hashtable *nhashtable = 0;

    assert(hashtable != 0);
    assert(keysize > 0);
    if (hashtable == 0 || keysize <= 0)
        return E_INVALIDARG;

    nhashtable = (optcl_hashtable*)malloc(sizeof(optcl_hashtable));
    if (nhashtable == 0)
        return E_OUTOFMEMORY;

    memset(nhashtable, 0, sizeof(optcl_hashtable));
    nhashtable->hashfn = (hashfn) ? hashfn : joaat_hash;
    nhashtable->keysize = keysize;
    error = optcl_array_create(sizeof(struct entry*), 0, &nhashtable->entries);
    if (FAILED(error)) {
        free(nhashtable);
        return error;
    }

    *hashtable = nhashtable;
    return SUCCESS;
}

RESULT optcl_hashtable_destroy(optcl_hashtable *hashtable, bool_t deallocate)
{
    RESULT error;

    assert(hashtable != 0);
    if (hashtable == 0)
        return E_INVALIDARG;

    error = optcl_hashtable_clear(hashtable, deallocate);
    if (SUCCEEDED(error))
        free(hashtable);

    return SUCCESS;
}

RESULT optcl_hashtable_get_pairs(const optcl_hashtable *hashtable,
                                 optcl_list **pairs)
{
    uint32_t i;
    RESULT error;
    RESULT destroy_error;
    uint32_t entries_count;
    struct pair *pair = 0;
    struct entry *entry = 0;
    struct pair *bucket_pair = 0;
    optcl_list *pair_list = 0;
    optcl_list_iterator it = 0;

    assert(hashtable != 0);
    assert(pairs != 0);
    if (hashtable == 0 || pairs == 0)
        return E_INVALIDARG;

    *pairs = 0;
    if (hashtable->entries == 0)
        return SUCCESS;

    error = optcl_array_get_size(hashtable->entries, &entries_count);
    if (FAILED(error))
        return error;

    if (entries_count < 1)
        return SUCCESS;

    error = optcl_list_create(0, &pair_list);
    if (FAILED(error))
        return error;

    i = 0;
    while (i < entries_count) {
        error = optcl_array_get(hashtable->entries, i, (const pptr_t)&entry);
        if (FAILED(error))
            break;

        if (entry == 0) {
            ++i;
            continue;
        }

        pair = (struct pair*)malloc(sizeof(struct pair));
        if (pair == 0) {
            error = E_OUTOFMEMORY;
            break;
        }

        pair->key = entry->pair.key;
        pair->value = entry->pair.value;
        error = optcl_list_add_tail(pair_list, (const ptr_t)pair);
        if (FAILED(error)) {
            free(pair);
            break;
        }

        if (entry->bucket == 0) {
            ++i;
            continue;
        }

        error = optcl_list_get_head_pos(entry->bucket, &it);
        if (FAILED(error)) {
            break;
        }

        while (it != 0) {
            error = optcl_list_get_at_pos(entry->bucket, it, (const pptr_t)&bucket_pair);
            if (FAILED(error))
                break;

            if (bucket_pair == 0) {
                error = E_COLLINVLDHASHTABLE;
                break;
            }

            pair = (struct pair*)malloc(sizeof(struct pair));
            if (pair == 0) {
                error = E_OUTOFMEMORY;
                break;
            }

            pair->key = bucket_pair->key;
            pair->value = bucket_pair->value;
            error = optcl_list_add_tail(pair_list, (const ptr_t)pair);
            if (FAILED(error)) {
                free(pair);
                break;
            }

            error = optcl_list_get_next(entry->bucket, it, &it);
            if (FAILED(error))
                break;
        }

        if (FAILED(error))
            break;

        ++i;
    }

    destroy_error = SUCCESS;

    if (FAILED(error)) {
        /* Destroy created list of pairs on error */
        destroy_error = optcl_list_destroy(pair_list, 1);
        pair_list = 0;
    }

    *pairs = pair_list;
    return SUCCEEDED(destroy_error) ? error : destroy_error;
}

RESULT optcl_hashtable_lookup(const optcl_hashtable *hashtable,
                              const ptr_t key,
                              const pptr_t value)
{
    RESULT error;
    uint32_t hash;
    uint32_t entrycount;
    struct pair *pair = 0;
    struct entry *entry = 0;
    optcl_list_iterator it = 0;
    optcl_hashtable_hashfn hashfn = 0;

    assert(key != 0);
    assert(value != 0);
    assert(hashtable != 0);
    if (hashtable == 0 || key == 0 || value == 0)
        return E_INVALIDARG;

    *value = 0;

    assert(hashtable->hashfn != 0);
    assert(hashtable->keysize > 0);
    if (hashtable->hashfn == 0 || hashtable->keysize <= 0)
        return E_COLLINVLDHASHTABLE;

    hashfn = hashtable->hashfn;
    hash = hashfn(key, hashtable->keysize);
    if (hashtable->entries == 0)
        return SUCCESS;

    error = optcl_array_get_size(hashtable->entries, &entrycount);
    if (FAILED(error))
        return error;

    assert(entrycount > 0);
    if (entrycount <= 0)
        return E_COLLINVLDHASHTABLE;

    error = optcl_array_get(hashtable->entries, hash % entrycount, 
        (const pptr_t)&entry);
    if (FAILED(error))
        return error;

    assert(entry != 0);
    if (entry == 0)
        return E_COLLINVLDHASHTABLE;

    if (hashfn(entry->pair.key, hashtable->keysize) == hash) {
        *value = entry->pair.value;
        return SUCCESS;
    }

    error = optcl_list_get_head_pos(entry->bucket, &it);
    if (FAILED(error))
        return error;

    assert(it != 0);
    if (it == 0)
        return E_COLLINVLDHASHTABLE;

    while (it != 0) {
        error = optcl_list_get_at_pos(entry->bucket, it, (const pptr_t)&pair);
        if (FAILED(error))
            break;

        if (hashfn(pair->key, hashtable->keysize) == hash) {
            *value = pair->value;
            break;
        }

        error = optcl_list_get_next(entry->bucket, it, &it);
        if (FAILED(error))
            break;
    }

    return SUCCESS;
}

RESULT optcl_hashtable_set(optcl_hashtable *hashtable,
                           const ptr_t key,
                           const ptr_t value)
{
    uint32_t hash;
    uint32_t size;
    RESULT error;
    float loadfactor;
    struct pair *pair = 0;
    struct entry *entry = 0;

    assert(key != 0);
    assert(hashtable != 0);
    if (key == 0 || hashtable == 0)
        return E_INVALIDARG;

    assert(hashtable->hashfn != 0);
    assert(hashtable->keysize > 0);
    if (hashtable->keysize <= 0)
        return E_COLLINVLDHASHTABLE;

    assert(hashtable->entries != 0);
    if (hashtable->entries == 0)
        return E_POINTER;

    error = optcl_array_get_size(hashtable->entries, &size);
    if (FAILED(error))
        return error;

    loadfactor = (hashtable->keycount > 0 && size > 0) ? 
        (hashtable->keycount * 1.0f) / size : 100.0f;

    if (loadfactor >= max_load_factor) {
        error = optcl_hashtable_rehash(hashtable);
        if (FAILED(error))
            return error;

        error = optcl_array_get_size(hashtable->entries, &size);
        if (FAILED(error))
            return error;
    }

    hash = hashtable->hashfn(key, hashtable->keysize);
    error = optcl_array_get(hashtable->entries, hash % size, 
        (const pptr_t)&entry);
    if (FAILED(error))
        return error;

    if (entry != 0) {
        pair = (struct pair*)malloc(sizeof(struct pair));
        if (pair == 0)
            return E_OUTOFMEMORY;

        pair->key = key;
        pair->value = value;
        if (entry->bucket == 0) {
            error = optcl_list_create(0, &entry->bucket);
            if (FAILED(error)) {
                free(pair);
                return error;
            }
        }

        error = optcl_list_add_head(entry->bucket, (const ptr_t)pair);
        if (FAILED(error)) {
            free(pair);
            return error;
        }
    } else {
        entry = (struct entry*)malloc(sizeof(struct entry));
        if (entry == 0)
            return E_OUTOFMEMORY;

        memset(entry, 0, sizeof(struct entry));
        entry->hash = hash;
        entry->pair.key = key;
        entry->pair.value = value;
        error = optcl_array_set(hashtable->entries, hash % size, 
            (const ptr_t)entry);
        if (FAILED(error))
            return error;
    }

    hashtable->keycount++;
    return error;
}

RESULT optcl_hashtable_rehash(optcl_hashtable *hashtable)
{
    RESULT error;
    RESULT destroy_error;
    uint32_t i;
    uint32_t hash;
    uint32_t size;
    uint32_t nsize;
    struct entry *entry = 0;
    optcl_array *nentries = 0;

    assert(hashtable != 0);
    if (hashtable == 0)
        return E_INVALIDARG;

    if (hashtable->primeindex + 1 > prime_table_length - 1)
        return E_OUTOFRANGE;

    error = optcl_array_create(sizeof(struct entry*), 0, &nentries);
    if (FAILED(error))
        return error;

    assert(hashtable->entries != 0);
    if (hashtable->entries == 0) {
        destroy_error = optcl_array_destroy(nentries, False);
        return SUCCEEDED(destroy_error) ? E_POINTER : destroy_error;
    }

    nsize = primes[hashtable->primeindex++];
    error = optcl_array_set_size(nentries, nsize, 0);
    if (FAILED(error)) {
        destroy_error = optcl_array_destroy(nentries, False);
        return SUCCEEDED(destroy_error) ? error : destroy_error;
    }

    error = optcl_array_get_size(hashtable->entries, &size);
    if (FAILED(error)) {
        destroy_error = optcl_array_destroy(nentries, False);
        return SUCCEEDED(destroy_error) ? error : destroy_error;
    }

    i = 0;
    while (i < size) {
        error = optcl_array_get(hashtable->entries, i, (const pptr_t)&entry);
        if (FAILED(error))
            break;

        if (entry == 0) {
            ++i;
            continue;
        }

        hash = hashtable->hashfn(entry->pair.key, hashtable->keysize);
        error = optcl_array_set(nentries, hash % nsize, (const ptr_t)entry);
        if (FAILED(error))
            break;

        ++i;
    }

    destroy_error = SUCCESS;
    if (FAILED(error))
        destroy_error = optcl_array_destroy(nentries, False);
    else {
        error = optcl_array_destroy(hashtable->entries, False);
        hashtable->entries = nentries;
    }

    return SUCCEEDED(destroy_error) ? error : destroy_error;
}
