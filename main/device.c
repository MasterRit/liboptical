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

#include <stdafx.h>

#include "adapter.h"
#include "errors.h"
#include "device.h"
#include "hashtable.h"
#include "list.h"
#include "media.h"

#include <assert.h>
#include <malloc.h>
#include <string.h>

#ifdef _DEBUG
// Poison value to detect use of deallocated list elements
#define POISON	((void*)0x12345678)
#endif


/* Device info */
typedef struct tag_device_info {
	char *product;
	char *revision;
	char *vendor;
	char *vendor_string;
	optcl_hashtable *features;
} optcl_device_info;

/* Device info */
typedef struct tag_device_info optcl_device_info;

/* Device descriptor */
typedef struct tag_device {
	int type;
	char *path;
	optcl_list *medias;
	optcl_adapter *adapter;
	optcl_device_info *info;
} optcl_device;


/*
 * Helpers
 */

static int clear_media_info_list(optcl_list *infos)
{
	int error;
	optcl_list_iterator it;
	optcl_media_info *media;

	assert(infos);

	if (!infos)
		return E_INVALIDARG;

	error = optcl_list_get_head_pos(infos, &it);

	if (FAILED(error))
		return error;

	while(it) {
		error = optcl_list_get_at_pos(infos, it, &media);

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

static int copy_media_info_list(optcl_list *dest, const optcl_list *src)
{
	int error;
	optcl_list_iterator it;
	optcl_media_info *media;
	optcl_media_info *nmedia;

	assert(dest);
	assert(src);

	if (!dest || !src)
		return E_INVALIDARG;

	error = clear_media_info_list(dest);

	if (FAILED(error))
		return error;

	error = optcl_list_get_head_pos(src, &it);

	if (FAILED(error))
		return error;

	while(it) {
		error = optcl_list_get_at_pos(src, it, &media);

		if (FAILED(error))
			break;

		if (media) {
			error = optcl_media_info_create(&nmedia);

			if (FAILED(error))
				break;

			error = optcl_media_info_copy(nmedia, media);

			if (FAILED(error)) {
				optcl_media_info_destroy(nmedia);
				break;
			}

			error = optcl_list_add_tail(dest, nmedia);

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

static int copy_features_hashtable(optcl_hashtable *dest, const optcl_hashtable *src)
{
	int error;
	optcl_list *pairs;
	optcl_list_iterator it;
	struct pair *pair;
	optcl_feature_descriptor *nfeature;

	assert(dest);
	assert(src);

	if (!dest || !src)
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

	while(it) {
		error = optcl_list_get_at_pos(pairs, it, &pair);

		if (FAILED(error))
			break;

		assert(pair);

		if (!pair) {
			error = E_UNEXPECTED;
			break;
		}

		assert(pair->key);
		assert(pair->value);

		if (!pair->key || !pair->value) {
			error = E_UNEXPECTED;
			break;
		}

		error = optcl_feature_copy(&nfeature, pair->value);

		if (FAILED(error))
			break;

		if (!nfeature) {
			error = E_OUTOFMEMORY;
			break;
		}

		error = optcl_hashtable_set(dest, pair->key, nfeature);

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

static uint32_t joaat_hash_for_device(const void *key, size_t len)
{
     size_t i;
     uint32_t hash = 0;
     
     for (i = 0; i < len; i++) {
         hash += ((unsigned char*)&key)[i];
         hash += (hash << 10);
         hash ^= (hash >> 6);
     }

     hash += (hash << 3);
     hash ^= (hash >> 11);
     hash += (hash << 15);

     return hash;
}

/*
 * Device functions
 */

int optcl_device_bind2file(optcl_device *device, const char *filename)
{
	assert(device);
	assert(filename);

	if (!device || !filename)
		return E_INVALIDARG;

	optcl_device_clear(device);
	optcl_device_set_path(device, _strdup(filename));
	optcl_device_set_type(device, DEVICE_TYPE_IMAGE);

	return SUCCESS;
}

int optcl_device_clear(optcl_device *device)
{
	int error;

	assert(device);

	if (!device)
		return E_INVALIDARG;

	device->type = 0;

	free(device->path);
	device->path = 0;

	if (device->adapter) {
		error = optcl_adapter_clear(device->adapter);

		if (FAILED(error))
			return error;
	}

	if (device->info) {
		if (device->info->features) {
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

	if (device->medias) {
		error = clear_media_info_list(device->medias);

		if (FAILED(error))
			return error;
	}

	return SUCCESS;
}

int optcl_device_copy(optcl_device *dest, const optcl_device *src)
{
	int error;

	assert(dest);
	assert(src);

	if (!dest || !src)
		return E_INVALIDARG;

	error = optcl_device_clear(dest);

	if (FAILED(error))
		return error;

	dest->type = src->type;
	dest->path = _strdup(src->path);

	if (src->path && !dest->path)
		return E_OUTOFMEMORY;

	assert(src->adapter);
	assert(dest->adapter);

	if (!src->adapter || !dest->adapter)
		return E_UNEXPECTED;

	error = optcl_adapter_copy(dest->adapter, src->adapter);

	if (FAILED(error))
		return error;

	assert(src->info);
	assert(dest->info);

	if (!src->info || !dest->info)
		return E_UNEXPECTED;

	dest->info->product = _strdup(src->info->product);

	if (src->info->product && !dest->info->product) {
		optcl_device_clear(dest);
		return E_OUTOFMEMORY;
	}

	dest->info->revision = _strdup(src->info->revision);

	if (src->info->revision && !dest->info->revision) {
		optcl_device_clear(dest);
		return E_OUTOFMEMORY;
	}

	dest->info->vendor = _strdup(src->info->vendor);

	if (src->info->vendor && !dest->info->vendor) {
		optcl_device_clear(dest);
		return E_OUTOFMEMORY;
	}

	dest->info->vendor_string = _strdup(src->info->vendor_string);

	if (src->info->vendor_string != dest->info->vendor_string) {
		optcl_device_clear(dest);
		return E_OUTOFMEMORY;
	}

	assert(src->info->features);
	assert(dest->info->features);

	if (!src->info->features || !dest->info->features) {
		optcl_device_clear(dest);
		return E_UNEXPECTED;
	}

	error = copy_features_hashtable(dest->info->features, src->info->features);

	if (FAILED(error)) {
		optcl_device_clear(dest);
		return error;
	}

	assert(src->medias);
	assert(dest->medias);

	if (!src->medias || !dest->medias) {
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

int optcl_device_create(optcl_device **device)
{
	int error;
	optcl_device *newdev;

	assert(device);

	if (!device)
		return E_INVALIDARG;

	newdev = (optcl_device*)malloc(sizeof(optcl_device));

	if (!newdev)
		return E_OUTOFMEMORY;

	memset(newdev, 0, sizeof(optcl_device));

	error = optcl_adapter_create(&newdev->adapter);

	if (FAILED(error)) {
		free(newdev);
		return error;
	}

	newdev->info = (optcl_device_info*)malloc(sizeof(optcl_device_info));

	if (!newdev->info) {
		optcl_device_destroy(newdev);
		return E_OUTOFMEMORY;
	}

	memset(newdev->info, 0, sizeof(optcl_device_info));

	error = optcl_hashtable_create(
		sizeof(int), 
		&joaat_hash_for_device, 
		&newdev->info->features
		);

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

int optcl_device_destroy(optcl_device *device)
{
	int error;

	assert(device);

	if (!device)
		return E_INVALIDARG;

	optcl_device_clear(device);

	if (device->adapter) {
		error = optcl_adapter_destroy(device->adapter);

		if (FAILED(error))
			return error;
	}

#ifdef _DEBUG
	device->adapter = POISON;
#endif

	if (device->info) {
		if (device->info->features) {
			error = optcl_hashtable_destroy(device->info->features, 1);

			if (FAILED(error))
				return error;
		}

#ifdef _DEBUG
		device->info->product = POISON;
		device->info->features = POISON;
		device->info->revision = POISON;
		device->info->vendor = POISON;
		device->info->vendor_string = POISON;
#endif
		free(device->info);
	}

	if (device->medias) {
		error = optcl_list_destroy(device->medias, 1);

		if (FAILED(error))
			return error;
	}

#ifdef _DEBUG
	device->path = POISON;
#endif

	free(device);

	return SUCCESS;
}

int optcl_device_get_adapter(const optcl_device *device,
			     optcl_adapter **adapter)
{
	int error;

	assert(device);
	assert(adapter);

	if (!device || !adapter)
		return E_INVALIDARG;

	assert(device->adapter);

	if (!device->adapter)
		return E_UNEXPECTED;


	error = optcl_adapter_create(adapter);

	if (FAILED(error))
		return error;

	error = optcl_adapter_copy(*adapter, device->adapter);

	if (FAILED(error))
		optcl_adapter_destroy(*adapter);
	
	return error;
}

int optcl_device_get_feature(const optcl_device *device, 
			     int feature_code, 
			     optcl_feature **feature)
{
	assert(device);
	assert(feature);

	if (!device || !feature)
		return E_INVALIDARG;

	assert(device->info);

	if (!device->info)
		return E_UNEXPECTED;

	assert(device->info->features);

	if (!device->info->features)
		return E_UNEXPECTED;

	/*
	 * NOTE:
	 * feature_code is intentionly cast to size_t 
	 * to resolve 64-bit portability warning.
	 */
	return optcl_hashtable_lookup(
		device->info->features, 
		(void*)(size_t)feature_code, 
		feature
		);
}

int optcl_device_get_media_count(const optcl_device *device,
				 int *count)
{
	assert(device);
	assert(count);

	if (!device || !count)
		return E_INVALIDARG;

	assert(device->medias);

	if (!device->medias)
		return E_UNEXPECTED;

	return optcl_list_get_count(device->medias, count);
}

int optcl_get_media_info(const optcl_device *device,
			 int media_index,
			 optcl_media_info **media)
{
	int error;
	int count;
	optcl_media_info *info;
	optcl_media_info *nmedia;

	assert(device);
	assert(media);
	assert(media_index >= 0);

	if (!device || !media)
		return E_INVALIDARG;

	error = optcl_device_get_media_count(device, &count);

	if (FAILED(error))
		return error;

	if (media_index < 0 || media_index >= count)
		return E_OUTOFRANGE;

	assert(device->medias);

	if (!device->medias)
		return E_UNEXPECTED;

	error = optcl_list_get_at_index(device->medias, media_index, &info);

	if (FAILED(error))
		return error;

	if (!info)
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

int optcl_device_get_path(const optcl_device *device, char **path)
{
	assert(device);
	assert(path);

	if (!device || !path)
		return E_INVALIDARG;

	*path = _strdup(device->path);

	if (!*path && device->path)
		return E_OUTOFMEMORY;
	
	return SUCCESS;
}

int optcl_device_get_product(const optcl_device *device, char **product)
{
	assert(device);
	assert(product);

	if (!device || !product)
		return E_INVALIDARG;

	assert(device->info);

	if (!device->info)
		return E_UNEXPECTED;

	*product = _strdup(device->info->product);

	if (!*product && device->info->product)
		return E_OUTOFMEMORY;

	return SUCCESS;
}

int optcl_device_get_revision(const optcl_device *device, char **revision)
{
	assert(device);
	assert(revision);

	if (!device || !revision)
		return E_INVALIDARG;

	assert(device->info);

	if (!device->info)
		return E_UNEXPECTED;

	*revision = _strdup(device->info->revision);

	if (!*revision && device->info->revision)
		return E_OUTOFMEMORY;

	return SUCCESS;
}

int optcl_device_get_type(const optcl_device *device, int *type)
{
	assert(device);
	assert(type);

	if (!device || !type)
		return E_INVALIDARG;

	*type = device->type;

	return SUCCESS;
}

int optcl_device_get_vendor(const optcl_device *device, char **vendor)
{
	assert(device);
	assert(vendor);

	if (!device || !vendor)
		return E_INVALIDARG;

	assert(device->info);

	if (!device->info)
		return E_UNEXPECTED;

	*vendor = _strdup(device->info->vendor);

	if (!*vendor && device->info->vendor)
		return E_OUTOFMEMORY;

	return SUCCESS;
}

int optcl_device_get_vendor_string(const optcl_device *device, 
				   char **vendor_string)
{
	assert(device);
	assert(vendor_string);

	if (!device || !vendor_string)
		return E_INVALIDARG;

	assert(device->info);

	if (!device->info)
		return E_UNEXPECTED;

	*vendor_string = _strdup(device->info->vendor_string);

	if (!*vendor_string && device->info->vendor_string)
		return E_OUTOFMEMORY;

	return SUCCESS;
}

int optcl_device_add_media_info(optcl_device *device,
				optcl_media_info *info)
{
	assert(device);
	assert(info);

	if (!device || !info)
		return E_INVALIDARG;

	assert(device->medias);

	if (!device->medias)
		return E_UNEXPECTED;

	return optcl_list_add_tail(device->medias, info);
}

int optcl_device_set_adapter(optcl_device *device, optcl_adapter *adapter)
{
	int error;

	assert(device);
	assert(adapter);

	if (!device || !adapter)
		return E_INVALIDARG;

	assert(device->adapter);

	if (!device->adapter)
		return E_UNEXPECTED;

	error = optcl_adapter_destroy(device->adapter);

	if (FAILED(error))
		return error;

	device->adapter = adapter;

	return SUCCESS;	
}

int optcl_device_set_feature(optcl_device *device, 
			     int feature_code, 
			     optcl_feature *feature)
{
	assert(device);
	assert(feature);

	if (!device || !feature)
		return E_INVALIDARG;

	assert(device->info);

	if (!device->info)
		return E_UNEXPECTED;

	assert(device->info->features);

	if (!device->info->features)
		return E_INVALIDARG;

	/*
	 * NOTE:
	 * feature_code is intentionly cast to size_t 
	 * to resolve 64-bit portability warning.
	 */
	return optcl_hashtable_set(
		device->info->features, 
		(void*)(size_t)feature_code, 
		feature
		);
}

int optcl_device_set_path(optcl_device *device, char *path)
{
	assert(device);
	
	if (!device)
		return E_INVALIDARG;

	free(device->path);

	device->path = path;

	return SUCCESS;
}

int optcl_device_set_product(optcl_device *device, char *product)
{
	assert(device);

	if (!device)
		return E_INVALIDARG;

	assert(device->info);

	if (!device->info)
		return E_UNEXPECTED;

	free(device->info->product);

	device->info->product = product;

	return SUCCESS;
}

int optcl_device_set_revision(optcl_device *device, char *revision)
{
	assert(device);

	if (!device)
		return E_INVALIDARG;

	assert(device->info);

	if (!device->info)
		return E_UNEXPECTED;

	free(device->info->revision);

	device->info->revision = revision;

	return SUCCESS;
}

int optcl_device_set_type(optcl_device *device, int type)
{
	assert(device);

	if (!device)
		return E_INVALIDARG;

	device->type = type;

	return SUCCESS;
}

int optcl_device_set_vendor(optcl_device *device, char *vendor)
{
	assert(device);

	if (!device)
		return E_INVALIDARG;

	assert(device->info);

	if (!device->info)
		return E_UNEXPECTED;

	free(device->info->vendor);

	device->info->vendor = vendor;

	return SUCCESS;
}

int optcl_device_set_vendor_string(optcl_device *device, char *vendor_string)
{
	assert(device);

	if (!device)
		return E_INVALIDARG;

	assert(device->info);

	if (!device->info)
		return E_UNEXPECTED;

	free(device->info->vendor_string);

	device->info->vendor_string = vendor_string;

	return SUCCESS;
}
