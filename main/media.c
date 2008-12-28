/*
    media.c - Optical media
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

#include <assert.h>
#include <malloc.h>

#include "errors.h"
#include "hashtable.h"
#include "media.h"




#ifdef _DEBUG
// Poison value to detect use of deallocated list elements
#define POISON	((void*)0x12345678)
#endif

/* Media info */
struct tag_media_info {
    int type;
    optcl_hashtable *profiles;
};


int optcl_media_info_create(optcl_media_info **media)
{
    assert(media);

    if (!media)
        return E_INVALIDARG;

    *media = (optcl_media_info*)malloc(sizeof(optcl_media_info));

    if (!*media)
        return E_OUTOFMEMORY;

    (*media)->type = -1;

    return optcl_hashtable_create(sizeof(int), 0, &(*media)->profiles);
}

int optcl_media_info_clear(optcl_media_info *media)
{
    assert(media);

    if (!media)
        return E_INVALIDARG;

    media->type = -1;

    return optcl_hashtable_clear(media->profiles, 1);
}

int optcl_media_info_copy(optcl_media_info *dest,
                          const optcl_media_info *src)
{
    assert(src);
    assert(dest);

    if (!src || !dest)
        return E_INVALIDARG;

    dest->type = src->type;

    return optcl_hashtable_copy(dest->profiles, src->profiles);
}

int optcl_media_info_destroy(optcl_media_info *media)
{
    int error = SUCCESS;

    assert(media);

    if (!media)
        return E_INVALIDARG;

    assert(media->profiles);

    if (media->profiles)
        error = optcl_hashtable_destroy(media->profiles, 1);

#ifdef _DEBUG
    media->profiles = (optcl_hashtable*)POISON;
#endif

    free(media);

    return error;
}

int optcl_media_info_get_type(const optcl_media_info *media, int *type)
{
    assert(media);
    assert(type);

    if (!media || !type)
        return E_INVALIDARG;

    *type = media->type;

    return SUCCESS;
}

int optcl_media_info_get_profiles(const optcl_media_info *media,
                                  optcl_list **profiles)
{
    assert(media);
    assert(profiles);

    if (!media || !profiles)
        return E_INVALIDARG;

    return optcl_hashtable_get_pairs(media->profiles, profiles);
}

