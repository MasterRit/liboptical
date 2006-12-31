/*
    device.h - Windows optical device driver
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

#ifndef _DEVICE_H
#define _DEVICE_H

#include "list.h"
#include "media.h"
#include "profile.h"
#include "transport.h"


/* Device type codes */
#define DEVICE_TYPE_IMAGE		0
#define DEVICE_TYPE_CDROM		1
#define DEVICE_TYPE_CDR			2
#define DEVICE_TYPE_CDRW		3
#define DEVICE_TYPE_DVDROM		10
#define DEVICE_TYPE_DVDR		11
#define DEVICE_TYPE_DVDRW		12
#define DEVICE_TYPE_DVDRAM		13


/* Device descriptor */
typedef struct tag_device optcl_device;

/* Bind device to an image file */
int optcl_device_bind2file(optcl_device *device, const char *filename);

/* Create new and empty device structure */
int optcl_device_create(optcl_device **device);

/* Clear device structure */
int optcl_device_clear(optcl_device *device);

/* Copy device structure */
int optcl_device_copy(optcl_device *dest, const optcl_device *src);

/* Destroy and deallocate device */
int optcl_device_destroy(optcl_device *device);

/* Get device feature */
int optcl_device_get_feature(const optcl_device *device, 
			     int feature_code, 
			     optcl_feature **feature);

/* Get media count */
int optcl_device_get_media_count(const optcl_device *device,
				 int *count);

/* Get media info */
int optcl_device_get_media_info(const optcl_device *device,
				int media_index,
				optcl_media_info **info);

/* Get device name */
int optcl_device_get_path(const optcl_device *device, char **path);

/* Get product string */
int optcl_device_get_product(const optcl_device *device, char **product);

/* Get revision string */
int optcl_device_get_revision(const optcl_device *device, char **revision);

/* Get device type */
int optcl_device_get_type(const optcl_device *device, int *type);

/* Get device vendor */
int optcl_device_get_vendor(const optcl_device *device, char **vendor);

/* Get vendor string */
int optcl_device_get_vendor_string(const optcl_device *device, 
				   char **vendor_string);

/* Add new media info */
int optcl_device_add_media_info(optcl_device *device, optcl_media_info *info);

/* Set feature */
int optcl_device_set_feature(optcl_device *device, 
			     int feature_number, 
			     optcl_feature *feature);

/* Set device name */
int optcl_device_set_path(optcl_device *device, char *path);

/* Set product */
int optcl_device_set_product(optcl_device *device, char *product);

/* Set revision string */
int optcl_device_set_revision(optcl_device *device, char *revision);

/* Set device type */
int optcl_device_set_type(optcl_device *device, int type);

/* Set device vendor */
int optcl_device_set_vendor(optcl_device *device, char *vendor);

/* Set vendor string */
int optcl_device_set_vendor_string(optcl_device *device, char *vendor_string);

#endif /* _DEVICE_H */
