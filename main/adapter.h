/*
    adapter.h - SCSI adapter.
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

#ifndef _ADAPTER_H
#define _ADAPTER_H

#include "errors.h"


/* Bustype constants */
#define ADAPTER_BUSTYPE_UNKNOWN		0x00
#define ADAPTER_BUSTYPE_SCSI		0x01
#define ADAPTER_BUSTYPE_ATAPI		0x02
#define ADAPTER_BUSTYPE_ATA		0x03
#define ADAPTER_BUSTYPE_IEEE1394	0x04
#define ADAPTER_BUSTYPE_SSA		0x05
#define ADAPTER_BUSTYPE_FIBRE		0x06
#define ADAPTER_BUSTYPE_USB		0x07
#define ADAPTER_BUSTYPE_RAID		0x08


/* Adapter structure */
typedef struct tag_adapter optcl_adapter;


/* Create new adapter structure */
extern RESULT optcl_adapter_create(optcl_adapter **adapter);

/* Clear adapter structure */
extern RESULT optcl_adapter_clear(optcl_adapter *adapter);

/* Copy adapter structure */
extern RESULT optcl_adapter_copy(optcl_adapter *dest, 
				 const optcl_adapter *src);

/* Destroy adapter structure */
extern RESULT optcl_adapter_destroy(optcl_adapter *adapter);

/* Get bus type */
extern RESULT optcl_adapter_get_bus_type(const optcl_adapter *adapter, 
					 uint32_t *bus_type);

/* Get maximal transfer length */
extern RESULT optcl_adapter_get_max_transfer_len(const optcl_adapter *adapter, 
						 uint32_t *max_transfer_len);

/* Get maximum physical pages */
extern RESULT optcl_adapter_get_max_physical_pages(const optcl_adapter *adapter, 
						   uint32_t *max_physical_pages);

/* Get alignment mask */
extern RESULT optcl_adapter_get_alignment_mask(const optcl_adapter *adapter,
					       uint32_t *alignment_mask);

/* Set bus type */
extern RESULT optcl_adapter_set_bus_type(optcl_adapter *adapter, 
					 uint32_t bus_type);

/* Set maximal transfer length */
extern RESULT optcl_adapter_set_max_transfer_length(optcl_adapter *adapter,
						    uint32_t max_transfer_len);

/* Set physical pages */
extern RESULT optcl_adapter_set_max_physical_pages(optcl_adapter *adapter,
						   uint32_t max_physical_pages);

/* Set alignment mask */
extern RESULT optcl_adapter_set_max_alignment_mask(optcl_adapter *adapter,
						   uint32_t alignment_mask);


#endif /* _ADAPTER_H */
