/*
    command.c - Multi-Media Commands - 5 (MMC-5).
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
#include "command.h"
#include "errors.h"
#include "feature.h"
#include "parsers.h"
#include "sysdevice.h"
#include "types.h"

#include <assert.h>
#include <malloc.h>
#include <memory.h>
#include <string.h>

/*
 * MMC opcodes
 */

#define MMC_OPCODE_INQUIRY	0x0012
#define MMC_OPCODE_GET_CONFIG	0x0046

/*
 * Constants used throughout the code
 */

#define MAX_GET_CONFIG_TRANSFER_LEN	65530

/*
 * Helper functions
 */

/*
 * Command functions
 */

RESULT optcl_command_get_config(const optcl_device *device,
				const optcl_mmc_get_config *command,
				optcl_mmc_response_get_config **response)
{
	RESULT error;
	uint8_t rt;
	uint8_t cdb[10];
	uint32_t data_length;
	uint16_t start_feature;
	uint32_t transfer_size;
	uint32_t alignment_mask;
	uint32_t max_transfer_len;
	uint8_t *mmc_response;
	optcl_adapter *adapter;
	optcl_list_iterator it;
	optcl_feature_descriptor *descriptor;
	optcl_mmc_response_get_config *nresponse0;
	optcl_mmc_response_get_config *nresponse1;

	assert(device != 0);
	assert(command != 0);
	assert(response != 0);

	if (device == 0 || command == 0 || response == 0) {
		return(E_INVALIDARG);
	}

	assert(
		command->rt == MMC_GET_CONFIG_RT_ALL 
		|| command->rt == MMC_GET_CONFIG_RT_CURRENT 
		|| command->rt == MMC_GET_CONFIG_RT_FROM
		);

	if (command->rt != MMC_GET_CONFIG_RT_ALL 
		&& command->rt != MMC_GET_CONFIG_RT_CURRENT 
		&& command->rt != MMC_GET_CONFIG_RT_FROM)
	{
		return(E_INVALIDARG);
	}

	assert(command->start_feature >= 0);

	if (command->start_feature < 0) {
		return(E_INVALIDARG);
	}

	error = optcl_device_get_adapter(device, &adapter);

	if (FAILED(error)) {
		return(error);
	}

	error = optcl_adapter_get_alignment_mask(adapter, &alignment_mask);

	if (FAILED(error)) {
		optcl_adapter_destroy(adapter);
		return(error);
	}

	error = optcl_adapter_get_max_transfer_len(adapter, &max_transfer_len);

	if (FAILED(error)) {
		optcl_adapter_destroy(adapter);
		return(error);
	}

	error = optcl_adapter_destroy(adapter);

	if (FAILED(error)) {
		return(error);
	}

	/*
	 * Execute command just to get data length
	 */

	rt = command->rt & 0x03;
	start_feature = command->start_feature;

	memset(cdb, 0, sizeof(cdb));

	cdb[0] = MMC_OPCODE_GET_CONFIG;
	cdb[1] = rt;
	cdb[2] = (uint8_t)(uint16_to_be(start_feature) >> 8);
	cdb[3] = (uint8_t)((uint16_to_be(start_feature) << 8) >> 8);
	cdb[8] = 8; /* enough to get feature descriptor header */

	mmc_response = _aligned_malloc(cdb[8], alignment_mask);

	if (mmc_response == 0) {
		return(E_OUTOFMEMORY);
	}

	error = optcl_device_command_execute(
		device, 
		cdb, 
		sizeof(cdb), 
		mmc_response, 
		cdb[8]
		);

	if (FAILED(error)) {
		_aligned_free(mmc_response);
		return(error);
	}

	nresponse0 = malloc(sizeof(optcl_mmc_response_get_config));

	if (nresponse0 == 0) {
		_aligned_free(mmc_response);
		return(E_OUTOFMEMORY);
	}

	memset(&nresponse0, 0, sizeof(nresponse0));

	error = optcl_list_create(0, &nresponse0->descriptors);

	if (FAILED(error)) {
		free(nresponse0);
		_aligned_free(mmc_response);
		return(error);
	}

	/*
	 * Set new data length and execute command
	 * 
	 * NOTE that in MMC-5 standard, the entire set of defined 
	 * feature descriptors amounts to less than 1 KB.
	 */

	data_length = int32_from_be((int32_t)(*mmc_response));

	_aligned_free(mmc_response);
	
	nresponse0->data_length = data_length;

	if (max_transfer_len > MAX_GET_CONFIG_TRANSFER_LEN)
		max_transfer_len = MAX_GET_CONFIG_TRANSFER_LEN;

	do {
		transfer_size = (data_length > max_transfer_len)
			? max_transfer_len
			: data_length;

		data_length -= max_transfer_len;

		/*
		 * Get the next chunk of data
		 */

		cdb[1] = rt;
		cdb[2] = (uint8_t)(uint16_to_be(start_feature) >> 8);
		cdb[3] = (uint8_t)((uint16_to_be(start_feature) << 8) >> 8);
		cdb[7] = (uint8_t)(uint16_to_be((uint16_t)transfer_size) >> 8);
		cdb[8] = (uint8_t)((uint16_to_be((uint16_t)transfer_size) << 8) >> 8);

		mmc_response = _aligned_malloc(transfer_size, alignment_mask);

		if (mmc_response == 0) {
			error = E_OUTOFMEMORY;
			break;
		}

		memset(mmc_response, 0, transfer_size);

		error = optcl_device_command_execute(
			device,
			cdb,
			sizeof(cdb),
			mmc_response,
			transfer_size
			);

		if (FAILED(error)) {
			_aligned_free(mmc_response);
			break;
		}

		rt = MMC_GET_CONFIG_RT_FROM;

		/*
		 * Process current chnk of data
		 */

		error = optcl_parse_get_config_data(
			mmc_response, 
			max_transfer_len, 
			&nresponse1
			);

		if (FAILED(error)) {
			_aligned_free(mmc_response);
			break;
		}

		error = optcl_list_get_tail_pos(nresponse1->descriptors, &it);

		if (FAILED(error)) {
			optcl_list_destroy(nresponse1->descriptors, 1);
			_aligned_free(mmc_response);
			free(nresponse1);
			break;
		}

		error = optcl_list_get_at_pos(nresponse1->descriptors, it, &descriptor);

		if (FAILED(error)) {
			optcl_list_destroy(nresponse1->descriptors, 1);
			_aligned_free(mmc_response);
			free(nresponse1);
			break;
		}

		start_feature = descriptor->feature_code + 1;

		nresponse0->current_profile = nresponse1->current_profile;
		
		error = optcl_list_append(nresponse0->descriptors, nresponse1->descriptors);

		if (FAILED(error)) {
			optcl_list_destroy(nresponse1->descriptors, 1);
			_aligned_free(mmc_response);
			free(nresponse1);
			break;
		}

		error = optcl_list_destroy(nresponse1->descriptors, 1);

		if (FAILED(error)) {
			_aligned_free(mmc_response);
			free(nresponse1);
			break;
		}

		free(nresponse1);
		_aligned_free(mmc_response);

	} while(data_length > 0);

	if (FAILED(error)) {
		optcl_list_destroy(nresponse0->descriptors, 1);
		free(nresponse0);
		return(error);
	}

	*response = nresponse0;

	return(error);
}

RESULT optcl_command_inquiry(const optcl_device *device, 
			     const optcl_mmc_inquiry *command, 
			     optcl_mmc_response_inquiry **response)
{
	RESULT error;
	uint8_t cdb[6];
	uint8_t *mmc_response;
	uint32_t alignment_mask;
	optcl_adapter *adapter;
	optcl_mmc_response_inquiry *nresponse;

	assert(device != 0);
	assert(command != 0);
	assert(response != 0);

	if (device == 0|| command == 0 || response == 0) {
		return(E_INVALIDARG);
	}

	assert(command->evpd == 0);
	assert(command->page_code == 0);

	if (command->evpd != 0 || command->page_code != 0) {
		return(E_INVALIDARG);
	}

	error = optcl_device_get_adapter(device, &adapter);

	if (FAILED(error)) {
		return(error);
	}

	error = optcl_adapter_get_alignment_mask(adapter, &alignment_mask);

	if (FAILED(error)) {
		optcl_adapter_destroy(adapter);
		return(error);
	}

	error = optcl_adapter_destroy(adapter);

	if (FAILED(error)) {
		return(error);
	}

	/*
	 * Execute command just to get additional length
	 */

	memset(cdb, 0, sizeof(cdb));

	cdb[0] = MMC_OPCODE_INQUIRY;
	cdb[4] = 5; /* the allocation length should be at least five */

	mmc_response = _aligned_malloc(cdb[4], alignment_mask);

	if (mmc_response == 0) {
		return(E_OUTOFMEMORY);
	}

	/* Get standard inquiry data additional length */
	error = optcl_device_command_execute(
		device, 
		cdb, 
		sizeof(cdb), 
		mmc_response,
		cdb[4]
		);

	if (FAILED(error)) {
		_aligned_free(mmc_response);
		return(error);
	}

	/* Set standard inquiry data length */
	cdb[4] = mmc_response[4] + 4;

	_aligned_free(mmc_response);

	mmc_response = _aligned_malloc((size_t)cdb[4], alignment_mask);

	if (mmc_response == 0) {
		return(E_OUTOFMEMORY);
	}

	/* Get standard inquiry data */
	error = optcl_device_command_execute(
		device, 
		cdb, 
		sizeof(cdb), 
		mmc_response,
		cdb[4]
		);

	if (FAILED(error)) {
		_aligned_free(mmc_response);
		return(error);
	}

	error = optcl_parse_inquiry_data(mmc_response, cdb[4], &nresponse);

	if (SUCCEEDED(error)) {
		*response = nresponse;
	}

	_aligned_free(mmc_response);

	return(error);
}
