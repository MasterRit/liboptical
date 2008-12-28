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

#include "adapter.h"
#include "feature.h"
#include "list.h"
#include "media.h"
#include "transport.h"
#include "types.h"


/* Device type codes */
#define DEVICE_TYPE_IMAGE		0
#define DEVICE_TYPE_CD_DVD		5


/* Device descriptor */
struct tag_device;
typedef struct tag_device optcl_device;

/* Bind device to an image file */
extern RESULT optcl_device_bind2file(optcl_device *device,
                                         const char *filename);

/* Create new and empty device structure */
extern RESULT optcl_device_create(optcl_device **device);

/* Clear device structure */
extern RESULT optcl_device_clear(optcl_device *device);

/* Copy device structure */
extern RESULT optcl_device_copy(optcl_device *dest, const optcl_device *src);

/* Destroy and deallocate device */
extern RESULT optcl_device_destroy(optcl_device *device);

/* Get device adapter info */
extern RESULT optcl_device_get_adapter(const optcl_device *device,
                                           optcl_adapter **adapter);

/* Get device feature */
extern RESULT optcl_device_get_feature(const optcl_device *device,
                                           uint16_t feature_code,
                                           optcl_feature **feature);

/* Get media count */
extern RESULT optcl_device_get_media_count(const optcl_device *device,
            uint32_t *count);

/* Get media info */
extern RESULT optcl_device_get_media_info(const optcl_device *device,
            uint32_t media_index,
            optcl_media_info **info);

/* Get device name */
extern RESULT optcl_device_get_path(const optcl_device *device, char **path);

/* Get product string */
extern RESULT optcl_device_get_product(const optcl_device *device,
                                           char **product);

/* Get revision string */
extern RESULT optcl_device_get_revision(const optcl_device *device,
                                            char **revision);

/* Get device type */
extern RESULT optcl_device_get_type(const optcl_device *device,
                                        uint16_t *type);

/* Get device vendor */
extern RESULT optcl_device_get_vendor(const optcl_device *device,
                                          char **vendor);

/* Get vendor string */
extern RESULT optcl_device_get_vendor_string(const optcl_device *device,
            char **vendor_string);

/* Add new media info */
extern RESULT optcl_device_add_media_info(optcl_device *device,
            optcl_media_info *info);

/* Set feature */
extern RESULT optcl_device_set_feature(optcl_device *device,
                                           uint16_t feature_number,
                                           optcl_feature *feature);

/* Set device adapter */
extern RESULT optcl_device_set_adapter(optcl_device *device,
                                           optcl_adapter *adapter);

/* Set device name */
extern RESULT optcl_device_set_path(optcl_device *device, char *path);

/* Set product */
extern RESULT optcl_device_set_product(optcl_device *device, char *product);

/* Set revision string */
extern RESULT optcl_device_set_revision(optcl_device *device, char *revision);

/* Set device type */
extern RESULT optcl_device_set_type(optcl_device *device, uint16_t type);

/* Set device vendor */
extern RESULT optcl_device_set_vendor(optcl_device *device, char *vendor);

/* Set vendor string */
extern RESULT optcl_device_set_vendor_string(optcl_device *device,
            char *vendor_string);

#endif /* _DEVICE_H */
