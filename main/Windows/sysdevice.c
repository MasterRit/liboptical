/*
    sysdevice.c - Platform dependent device functions.
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

#include "device.h"
#include "errors.h"
#include "sysdevice.h"
#include "transport.h"

#include <assert.h>


int optcl_device_enumerate(optcl_list **devices)
{
	assert(0);
	assert(devices);

	if (!devices)
		return E_INVALIDARG;

			

	return SUCCESS;
}

int optcl_device_getinfo(const char *device_name, optcl_device **device)
{
	assert(0);
	assert(device_name);
	assert(device);

	if (!device_name || !device)
		return E_INVALIDARG;

	return SUCCESS;
}