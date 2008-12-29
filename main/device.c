/*
    device.c - Windows optical device driver
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

#include "adapter.h"
#include "errors.h"
#include "device.h"
#include "hashtable.h"
#include "helpers.h"
#include "list.h"
#include "media.h"
#include "types.h"

#include <assert.h>
#include <malloc.h>
#include <string.h>


/*
 * Internal device structures
 */

/* Device info */
typedef struct tag_device_info {
    char *product;
    char *revision;
    char *vendor;
    char *vendor_string;
    optcl_hashtable *features;
} optcl_device_info;

/* Device descriptor */
struct tag_device {
    char *path;
    uint16_t type;
    optcl_list *medias;
    optcl_adapter *adapter;
    optcl_device_info *info;
};


/*
 * Helper functions
 */

static RESULT clear_media_info_list(optcl_list *infos)
{
    RESULT error;
    optcl_list_iterator it;
    optcl_media_info *media;

    assert(infos != 0);
    if (infos == 0)
        return E_INVALIDARG;

    error = optcl_list_get_head_pos(infos, &it);
    if (FAILED(error))
        return error;

    while (it != 0) {
        error = optcl_list_get_at_pos(infos, it, (const pptr_t)&media);
        if (SUCCEEDED(error)) {
            error = optcl_media_info_destroy(media);
            if (FAILED(error))
                break;
        }

        error = optcl_list_get_next(infos, it, &it);
        if (FAILED(error))
            break;
    }

    return optcl_list_clear(infos, 0);
}

static RESULT copy_media_info_list(optcl_list *dest, const optcl_list *src)
{
    RESULT error;
    optcl_list_iterator it;
    optcl_media_info *media;
    optcl_media_info *nmedia;

    assert(dest != 0);
    assert(src != 0);
    if (dest == 0 || src == 0)
        return E_INVALIDARG;

    error = clear_media_info_list(dest);
    if (FAILED(error))
        return error;

    error = optcl_list_get_head_pos(src, &it);
    if (FAILED(error))
        return error;

    while (it != 0) {
        error = optcl_list_get_at_pos(src, it, (const pptr_t)&media);
        if (FAILED(error))
            break;

        if (media != 0) {
            error = optcl_media_info_create(&nmedia);
            if (FAILED(error))
                break;

            error = optcl_media_info_copy(nmedia, media);
            if (FAILED(error)) {
                optcl_media_info_destroy(nmedia);
                break;
            }

            error = optcl_list_add_tail(dest, (const ptr_t)nmedia);
            if (FAILED(error)) {
                optcl_media_info_destroy(nmedia);
                break;
            }
        }

        error = optcl_list_get_next(src, it, &it);
        if (FAILED(error))
            break;
    }

    if (FAILED(error))
        clear_media_info_list(dest);

    return error;
}

static RESULT copy_features_hashtable(optcl_hashtable *dest,
                                      const optcl_hashtable *src)
{
    RESULT error;
    optcl_list *pairs;
    optcl_list_iterator it;
    struct pair *pair;
    optcl_feature_descriptor *nfeature;

    assert(dest != 0);
    assert(src != 0);
    if (dest == 0 || src == 0)
        return E_INVALIDARG;

    error = optcl_hashtable_get_pairs(src, &pairs);
    if (FAILED(error))
        return error;

    error = optcl_hashtable_clear(dest, 1);
    if (FAILED(error))
        return error;

    error = optcl_list_get_head_pos(pairs, &it);
    if (FAILED(error)) {
        optcl_list_destroy(pairs, 1);
        return error;
    }

    while (it != 0) {
        error = optcl_list_get_at_pos(pairs, it, (const pptr_t)&pair);
        if (FAILED(error))
            break;

        assert(pair != 0);
        if (pair == 0) {
            error = E_UNEXPECTED;
            break;
        }

        assert(pair->key != 0);
        assert(pair->value != 0);
        if (pair->key == 0 || pair->value == 0) {
            error = E_UNEXPECTED;
            break;
        }

        error = optcl_feature_copy(&nfeature, (optcl_feature*)pair->value);
        if (FAILED(error))
            break;

        if (nfeature == 0) {
            error = E_OUTOFMEMORY;
            break;
        }

        error = optcl_hashtable_set(dest, pair->key, (const ptr_t)nfeature);
        if (FAILED(error)) {
            optcl_feature_destroy(nfeature);
            break;
        }
    }

    if (FAILED(error)) {
        optcl_hashtable_clear(dest, 1);
        optcl_list_destroy(pairs, 1);
        return error;
    }

    return optcl_list_destroy(pairs, 1);
}

/*
 * Device functions
 */

RESULT optcl_device_bind2file(optcl_device *device, const char *filename)
{
    RESULT error;
    char *devicepath;

    assert(device != 0);
    assert(filename != 0);
    if (device == 0 || filename == 0)
        return E_INVALIDARG;

    error = optcl_device_clear(device);
    if (FAILED(error))
        return error;

    devicepath = xstrdup(filename);
    if (devicepath == 0)
        return E_OUTOFMEMORY;

    error = optcl_device_set_path(device, devicepath);
    if (FAILED(error))
        return error;

    return optcl_device_set_type(device, DEVICE_TYPE_IMAGE);
}

RESULT optcl_device_clear(optcl_device *device)
{
    RESULT error;

    assert(device != 0);
    if (device == 0)
        return E_INVALIDARG;

    device->type = 0;
    free(device->path);
    device->path = 0;
    if (device->adapter != 0) {
        error = optcl_adapter_clear(device->adapter);
        if (FAILED(error))
            return error;
    }

    if (device->info != 0) {
        if (device->info->features != 0) {
            error = optcl_hashtable_clear(device->info->features, 1);
            if (FAILED(error))
                return error;
        }

        free(device->info->product);
        free(device->info->revision);
        free(device->info->vendor);
        free(device->info->vendor_string);
        device->info->product = 0;
        device->info->revision = 0;
        device->info->vendor = 0;
        device->info->vendor_string = 0;
    }

    if (device->medias != 0) {
        error = clear_media_info_list(device->medias);
        if (FAILED(error))
            return error;
    }

    return SUCCESS;
}

RESULT optcl_device_copy(optcl_device *dest, const optcl_device *src)
{
    RESULT error;

    assert(dest != 0);
    assert(src != 0);
    if (dest == 0 || src == 0)
        return E_INVALIDARG;

    error = optcl_device_clear(dest);
    if (FAILED(error))
        return error;

    dest->type = src->type;
    dest->path = xstrdup(src->path);
    if (src->path != 0 && dest->path == 0)
        return E_OUTOFMEMORY;

    assert(src->adapter != 0);
    assert(dest->adapter != 0);
    if (src->adapter == 0 || dest->adapter == 0)
        return E_UNEXPECTED;

    error = optcl_adapter_copy(dest->adapter, src->adapter);
    if (FAILED(error))
        return error;

    assert(src->info != 0);
    assert(dest->info != 0);
    if (src->info == 0 || dest->info == 0)
        return E_UNEXPECTED;

    dest->info->product = xstrdup(src->info->product);
    if (src->info->product != 0 && dest->info->product == 0) {
        optcl_device_clear(dest);
        return E_OUTOFMEMORY;
    }

    dest->info->revision = xstrdup(src->info->revision);
    if (src->info->revision != 0 && dest->info->revision == 0) {
        optcl_device_clear(dest);
        return E_OUTOFMEMORY;
    }

    dest->info->vendor = xstrdup(src->info->vendor);
    if (src->info->vendor && !dest->info->vendor) {
        optcl_device_clear(dest);
        return E_OUTOFMEMORY;
    }

    dest->info->vendor_string = xstrdup(src->info->vendor_string);
    if (src->info->vendor_string != dest->info->vendor_string) {
        optcl_device_clear(dest);
        return E_OUTOFMEMORY;
    }

    assert(src->info->features != 0);
    assert(dest->info->features != 0);
    if (src->info->features == 0 || dest->info->features == 0) {
        optcl_device_clear(dest);
        return E_UNEXPECTED;
    }

    error = copy_features_hashtable(dest->info->features, src->info->features);
    if (FAILED(error)) {
        optcl_device_clear(dest);
        return error;
    }

    assert(src->medias != 0);
    assert(dest->medias != 0);
    if (src->medias == 0 || dest->medias == 0) {
        optcl_device_clear(dest);
        return E_UNEXPECTED;
    }

    error = copy_media_info_list(dest->medias, src->medias);
    if (FAILED(error)) {
        optcl_device_clear(dest);
        return error;
    }

    return error;
}

RESULT optcl_device_create(optcl_device **device)
{
    RESULT error;
    optcl_device *newdev;

    assert(device != 0);
    if (device == 0)
        return E_INVALIDARG;

    newdev = (optcl_device*)malloc(sizeof(optcl_device));
    if (newdev == 0)
        return E_OUTOFMEMORY;

    memset(newdev, 0, sizeof(optcl_device));
    error = optcl_adapter_create(&newdev->adapter);
    if (FAILED(error)) {
        free(newdev);
        return error;
    }

    newdev->info = (optcl_device_info*)malloc(sizeof(optcl_device_info));
    if (newdev->info == 0) {
        optcl_device_destroy(newdev);
        return E_OUTOFMEMORY;
    }

    memset(newdev->info, 0, sizeof(optcl_device_info));
    error = optcl_hashtable_create(sizeof(int), 0, &newdev->info->features);
    if (FAILED(error)) {
        optcl_device_destroy(newdev);
        return error;
    }

    error = optcl_list_create(0, &newdev->medias);
    if (FAILED(error)) {
        optcl_device_destroy(newdev);
        return error;
    }

    *device = newdev;
    return error;
}

RESULT optcl_device_destroy(optcl_device *device)
{
    RESULT error;

    assert(device != 0);
    if (device == 0) {
        return E_INVALIDARG;
    }

    optcl_device_clear(device);
    if (device->adapter != 0) {
        error = optcl_adapter_destroy(device->adapter);
        if (FAILED(error))
            return error;
    }

    if (device->info != 0) {
        if (device->info->features != 0) {
            error = optcl_hashtable_destroy(device->info->features, 1);
            if (FAILED(error))
                return error;
        }

        free(device->info);
    }

    if (device->medias != 0) {
        error = optcl_list_destroy(device->medias, 1);
        if (FAILED(error))
            return error;
    }

    free(device);
    return SUCCESS;
}

RESULT optcl_device_get_adapter(const optcl_device *device,
                                optcl_adapter **adapter)
{
    RESULT error;

    assert(device != 0);
    assert(adapter != 0);
    if (device == 0 || adapter == 0)
        return E_INVALIDARG;

    assert(device->adapter != 0);
    if (device->adapter == 0)
        return E_UNEXPECTED;

    error = optcl_adapter_create(adapter);
    if (FAILED(error))
        return error;

    error = optcl_adapter_copy(*adapter, device->adapter);
    if (FAILED(error))
        optcl_adapter_destroy(*adapter);

    return error;
}

RESULT optcl_device_get_feature(const optcl_device *device,
                                uint16_t feature_code,
                                optcl_feature **feature)
{
    assert(device != 0);
    assert(feature != 0);
    if (device == 0 || feature == 0)
        return E_INVALIDARG;

    assert(device->info != 0);
    if (device->info == 0)
        return E_UNEXPECTED;

    assert(device->info->features != 0);
    if (device->info->features == 0)
        return E_UNEXPECTED;

    /*
     * NOTE:
     * feature_code is intentionly cast to size_t
     * to resolve 64-bit portability warning.
     */
    return optcl_hashtable_lookup(device->info->features, 
        (const ptr_t)&feature_code, (const pptr_t)feature);
}

RESULT optcl_device_get_media_count(const optcl_device *device,
                                    uint32_t *count)
{
    assert(device != 0);
    assert(count != 0);
    if (device == 0 || count == 0)
        return E_INVALIDARG;

    assert(device->medias != 0);
    if (device->medias == 0)
        return E_UNEXPECTED;

    return optcl_list_get_count(device->medias, count);
}

RESULT optcl_get_media_info(const optcl_device *device,
                            uint32_t media_index,
                            optcl_media_info **media)
{
    RESULT error;
    uint32_t count;
    optcl_media_info *info;
    optcl_media_info *nmedia;

    assert(device != 0);
    assert(media != 0);
    assert(media_index >= 0);
    if (device == 0 || media == 0)
        return E_INVALIDARG;

    error = optcl_device_get_media_count(device, &count);
    if (FAILED(error))
        return error;

    if (media_index < 0 || media_index >= count)
        return E_OUTOFRANGE;

    assert(device->medias != 0);
    if (device->medias == 0)
        return E_UNEXPECTED;

    error = optcl_list_get_at_index(device->medias, media_index, 
        (const pptr_t)&info);
    if (FAILED(error))
        return error;

    if (info == 0)
        return E_UNEXPECTED;

    error = optcl_media_info_create(&nmedia);
    if (FAILED(error))
        return error;

    error = optcl_media_info_copy(nmedia, info);
    if (FAILED(error)) {
        optcl_media_info_destroy(nmedia);
        return error;
    }

    *media = nmedia;
    return SUCCESS;
}

RESULT optcl_device_get_path(const optcl_device *device, char **path)
{
    assert(device != 0);
    assert(path != 0);
    if (device == 0 || path == 0)
        return E_INVALIDARG;

    *path = xstrdup(device->path);
    if (*path == 0 && device->path != 0)
        return E_OUTOFMEMORY;

    return SUCCESS;
}

RESULT optcl_device_get_product(const optcl_device *device, char **product)
{
    assert(device != 0);
    assert(product != 0);
    if (device == 0 || product == 0)
        return E_INVALIDARG;

    assert(device->info != 0);
    if (device->info == 0)
        return E_UNEXPECTED;

    *product = xstrdup(device->info->product);
    if (*product == 0 && device->info->product != 0)
        return E_OUTOFMEMORY;

    return SUCCESS;
}

RESULT optcl_device_get_revision(const optcl_device *device, char **revision)
{
    assert(device != 0);
    assert(revision != 0);
    if (device == 0 || revision == 0)
        return E_INVALIDARG;

    assert(device->info != 0);
    if (device->info == 0)
        return E_UNEXPECTED;

    *revision = xstrdup(device->info->revision);
    if (*revision == 0 && device->info->revision != 0)
        return E_OUTOFMEMORY;

    return SUCCESS;
}

RESULT optcl_device_get_type(const optcl_device *device, uint16_t *type)
{
    assert(device != 0);
    assert(type != 0);
    if (device == 0 || type == 0)
        return E_INVALIDARG;

    *type = device->type;
    return SUCCESS;
}

RESULT optcl_device_get_vendor(const optcl_device *device, char **vendor)
{
    assert(device != 0);
    assert(vendor != 0);
    if (device == 0 || vendor == 0)
        return E_INVALIDARG;

    assert(device->info != 0);
    if (device->info == 0)
        return E_UNEXPECTED;

    *vendor = xstrdup(device->info->vendor);
    if (*vendor == 0 && device->info->vendor != 0)
        return E_OUTOFMEMORY;

    return SUCCESS;
}

RESULT optcl_device_get_vendor_string(const optcl_device *device,
                                      char **vendor_string)
{
    assert(device != 0);
    assert(vendor_string != 0);
    if (device == 0 || vendor_string == 0)
        return E_INVALIDARG;

    assert(device->info != 0);
    if (device->info == 0)
        return E_UNEXPECTED;

    *vendor_string = xstrdup(device->info->vendor_string);
    if (*vendor_string == 0 && device->info->vendor_string != 0)
        return E_OUTOFMEMORY;

    return SUCCESS;
}

RESULT optcl_device_add_media_info(optcl_device *device,
                                   optcl_media_info *info)
{
    assert(device != 0);
    assert(info != 0);
    if (device == 0 || info == 0)
        return E_INVALIDARG;

    assert(device->medias != 0);
    if (device->medias == 0)
        return E_UNEXPECTED;

    return optcl_list_add_tail(device->medias, (const ptr_t)info);
}

RESULT optcl_device_set_adapter(optcl_device *device, optcl_adapter *adapter)
{
    RESULT error;

    assert(device != 0);
    assert(adapter != 0);
    if (device == 0 || adapter == 0)
        return E_INVALIDARG;

    assert(device->adapter != 0);
    if (device->adapter == 0)
        return E_UNEXPECTED;

    error = optcl_adapter_destroy(device->adapter);
    if (FAILED(error))
        return error;

    device->adapter = adapter;
    return SUCCESS;
}

RESULT optcl_device_set_feature(optcl_device *device,
                                uint16_t feature_code,
                                optcl_feature *feature)
{
    uint16_t *key = 0;

    assert(device != 0);
    assert(feature != 0);
    if (device == 0 || feature == 0)
        return E_INVALIDARG;

    assert(device->info != 0);
    if (device->info == 0)
        return E_UNEXPECTED;

    assert(device->info->features != 0);
    if (device->info->features == 0)
        return E_INVALIDARG;

    key = (uint16_t*)malloc(sizeof(uint16_t));
    if (key == 0)
        return E_OUTOFMEMORY;

    *key = feature_code;
    return optcl_hashtable_set(device->info->features, 
        (const ptr_t)key, (const ptr_t)feature);
}

RESULT optcl_device_set_path(optcl_device *device, char *path)
{
    assert(device != 0);
    if (device == 0)
        return E_INVALIDARG;

    free(device->path);
    device->path = path;
    return SUCCESS;
}

RESULT optcl_device_set_product(optcl_device *device, char *product)
{
    assert(device != 0);
    if (device == 0)
        return E_INVALIDARG;

    assert(device->info != 0);
    if (device->info == 0)
        return E_UNEXPECTED;

    free(device->info->product);
    device->info->product = product;
    return SUCCESS;
}

RESULT optcl_device_set_revision(optcl_device *device, char *revision)
{
    assert(device != 0);
    if (device == 0)
        return E_INVALIDARG;

    assert(device->info != 0);
    if (device->info == 0)
        return E_UNEXPECTED;

    free(device->info->revision);
    device->info->revision = revision;
    return SUCCESS;
}

RESULT optcl_device_set_type(optcl_device *device, uint16_t type)
{
    assert(device != 0);
    if (device == 0)
        return E_INVALIDARG;

    device->type = type;
    return SUCCESS;
}

RESULT optcl_device_set_vendor(optcl_device *device, char *vendor)
{
    assert(device != 0);
    if (device == 0)
        return E_INVALIDARG;

    assert(device->info != 0);
    if (device->info == 0)
        return E_UNEXPECTED;

    free(device->info->vendor);
    device->info->vendor = vendor;
    return SUCCESS;
}

RESULT optcl_device_set_vendor_string(optcl_device *device,
                                      char *vendor_string)
{
    assert(device != 0);
    if (device == 0)
        return E_INVALIDARG;

    assert(device->info != 0);
    if (device->info == 0)
        return E_UNEXPECTED;

    free(device->info->vendor_string);
    device->info->vendor_string = vendor_string;
    return SUCCESS;
}
