/*
    adapter.cs - SCSI adapter.
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

#include <assert.h>
#include <malloc.h>
#include <memory.h>


/* Adapter structure */
typedef struct tag_adapter {
	int bus_type;
	int max_transfer_len;
	int max_physical_pages;
	int alignment_mask;
} optcl_adapter;


/*
 * Adapter functions
 */

int optcl_adapter_create(optcl_adapter **adapter)
{
	assert(adapter);

	if (!adapter)
		return E_INVALIDARG;

	*adapter = malloc(sizeof(optcl_adapter));

	if (!*adapter)
		return E_OUTOFMEMORY;

	memset(*adapter, 0, sizeof(optcl_adapter));

	return SUCCESS;
}

int optcl_adapter_clear(optcl_adapter *adapter)
{
	assert(adapter);

	if (!adapter)
		return E_INVALIDARG;

	memset(adapter, 0, sizeof(optcl_adapter));

	return SUCCESS;
}

int optcl_adapter_copy(optcl_adapter *dest, const optcl_adapter *src)
{
	assert(dest);
	assert(src);

	if (!dest || !src)
		return E_INVALIDARG;

	memcpy(dest, src, sizeof(optcl_adapter));

	return SUCCESS;
}

int optcl_adapter_destroy(optcl_adapter *adapter)
{
	if (adapter)
		free(adapter);

	return SUCCESS;
}

int optcl_adapter_get_bus_type(const optcl_adapter *adapter,
			       int *bus_type)
{
	assert(adapter);
	assert(bus_type);

	if (!adapter || !bus_type)
		return E_INVALIDARG;

	*bus_type = adapter->bus_type;

	return SUCCESS;
}

int optcl_adapter_get_max_transfer_len(const optcl_adapter *adapter,
				       int *max_transfer_len)
{
	assert(adapter);
	assert(max_transfer_len);

	if (!adapter || !max_transfer_len)
		return E_INVALIDARG;

	*max_transfer_len = adapter->max_transfer_len;

	return SUCCESS;
}

int optcl_adapter_get_max_physical_pages(const optcl_adapter *adapter,
					 int *max_physical_pages)
{
	assert(adapter);
	assert(max_physical_pages);

	if (!adapter || !max_physical_pages)
		return E_INVALIDARG;

	*max_physical_pages = adapter->max_physical_pages;

	return SUCCESS;
}

int optcl_adapter_get_alignment_mask(const optcl_adapter *adapter,
				     int *alignment_mask)
{
	assert(adapter);
	assert(alignment_mask);

	if (!adapter || !alignment_mask)
		return E_INVALIDARG;

	*alignment_mask = adapter->alignment_mask;

	return SUCCESS;
}

int optcl_adapter_set_bus_type(optcl_adapter *adapter, int bus_type)
{
	assert(adapter);
	
	if (!adapter)
		return E_INVALIDARG;

	adapter->bus_type = bus_type;

	return SUCCESS;
}

int optcl_adapter_set_max_transfer_length(optcl_adapter *adapter,
					  int max_transfer_len)
{
	assert(adapter);

	if (!adapter)
		return E_INVALIDARG;

	adapter->max_transfer_len = max_transfer_len;

	return SUCCESS;
}

int optcl_adapter_set_max_physical_pages(optcl_adapter *adapter,
					 int max_physical_pages)
{
	assert(adapter);

	if (!adapter)
		return E_INVALIDARG;

	adapter->max_physical_pages = max_physical_pages;

	return SUCCESS;
}

int optcl_adapter_set_max_alignment_mask(optcl_adapter *adapter,
					 int alignment_mask)
{
	assert(adapter);

	if (!adapter)
		return E_INVALIDARG;

	adapter->alignment_mask = alignment_mask;

	return SUCCESS;
}
