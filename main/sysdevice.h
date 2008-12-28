/*
    sysdevice.h - Platform dependent device functions.
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

#ifndef _SYSDEVICE_H
#define _SYSDEVICE_H

#include "device.h"
#include "errors.h"
#include "list.h"
#include "types.h"


/* Enumerates all supported optical devices */
extern RESULT optcl_device_enumerate(optcl_list **devices);

/* Execute SCSI command */
extern RESULT optcl_device_command_execute(const optcl_device *device,
            const uint8_t cdb[],
            uint32_t cdb_size,
            uint8_t param[],
            uint32_t param_size);


#endif /* _SYSDEVICE_H */
