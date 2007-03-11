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
#include "helpers.h"
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

#define MMC_OPCODE_BLANK		0x00A1
#define MMC_OPCODE_CLOSE_TRACK_SESSION	0x005B
#define MMC_OPCODE_INQUIRY		0x0012
#define MMC_OPCODE_GET_CONFIG		0x0046
#define MMC_OPCODE_GET_EVENT_STATUS	0x004A
#define MMC_OPCODE_REQUEST_SENSE	0x0003


/*
 * Constants used throughout the code
 */

#define MAX_SENSEDATA_LENGTH		252
#define MAX_GET_CONFIG_TRANSFER_LEN	65530

/*
 * Internal types
 */

typedef uint8_t	cdb6[6];
typedef uint8_t cdb10[10];
typedef uint8_t cdb12[12];

typedef RESULT (*response_deallocator)(optcl_mmc_response *response);

struct response_deallocator_entry {
	uint16_t opcode;
	response_deallocator deallocator;
};


/*
 * Forward declarations
 */

static response_deallocator get_response_deallocator(uint16_t opcode);


/*
 * Command functions
 */

RESULT optcl_command_blank(const optcl_device *device,
			   const optcl_mmc_blank *command)
{
	cdb12 cdb;

	assert(device != 0);
	assert(command != 0);

	if (device == 0 || command == 0) {
		return(E_INVALIDARG);
	}

	/*
	 * Execute command
	 */

	memset(cdb, 0, sizeof(cdb));

	cdb[0] = MMC_OPCODE_BLANK;
	cdb[1] = (command->immed << 4) || command->blanking_type;
	cdb[2] = (uint8_t)(command->start_address >> 24);
	cdb[3] = (uint8_t)((command->start_address << 8) >> 24);
	cdb[4] = (uint8_t)((command->start_address << 16) >> 24);
	cdb[5] = (uint8_t)((command->start_address << 24) >> 24);

	return(optcl_device_command_execute(
		device,
		cdb,
		sizeof(cdb),
		0,
		0
		));
}

RESULT optcl_command_close_track_session(const optcl_device *device,
					 const optcl_mmc_close_track_session *command)
{
	cdb10 cdb;

	assert(device != 0);
	assert(command != 0);

	if (device == 0 || command == 0) {
		return(E_INVALIDARG);
	}

	/*
	 * Execute command
	 */

	memset(&cdb[0], 0, sizeof(cdb));

	cdb[0] = MMC_OPCODE_CLOSE_TRACK_SESSION;
	cdb[1] = command->immed;
	cdb[2] = command->close_function & 0x07;
	cdb[4] = (uint8_t)(command->logical_track_number >> 8);
	cdb[5] = (uint8_t)command->logical_track_number;

	return(optcl_device_command_execute(
		device,
		cdb,
		sizeof(cdb),
		0,
		0
		));
}

RESULT optcl_command_destroy_response(optcl_mmc_response *response)
{
	response_deallocator deallocator = 0;

	if (response == 0) {
		return(SUCCESS);
	}

	deallocator = get_response_deallocator(response->command_opcode);

	assert(deallocator != 0);

	if (deallocator == 0) {
		return(E_OUTOFRANGE);
	}

	return(deallocator(response));
}

RESULT optcl_command_get_configuration(const optcl_device *device,
				       const optcl_mmc_get_configuration *command,
				       optcl_mmc_response_get_configuration **response)
{
	RESULT error;
	RESULT destroy_error;

	cdb10 cdb;
	uint8_t rt;
	int32_t data_length;
	uint16_t start_feature;
	ptr_t mmc_response = 0;
	uint32_t transfer_size;
	uint32_t alignment_mask;
	uint32_t max_transfer_len;
	optcl_adapter *adapter = 0;
	optcl_list_iterator it = 0;
	optcl_feature_descriptor *descriptor = 0;
	optcl_mmc_response_get_configuration *nresponse0 = 0;
	optcl_mmc_response_get_configuration *nresponse1 = 0;

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
		destroy_error = optcl_adapter_destroy(adapter);
		return(SUCCEEDED(destroy_error) ? error : destroy_error);
	}

	error = optcl_adapter_get_max_transfer_len(adapter, &max_transfer_len);

	if (FAILED(error)) {
		destroy_error = optcl_adapter_destroy(adapter);
		return(SUCCEEDED(destroy_error) ? error : destroy_error);
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
	cdb[2] = (uint8_t)(start_feature >> 8);
	cdb[3] = (uint8_t)((start_feature << 8) >> 8);
	cdb[8] = 8; /* enough to get feature descriptor header */

	mmc_response = xmalloc_aligned(cdb[8], alignment_mask);

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
		xfree_aligned(mmc_response);
		return(error);
	}

	nresponse0 = malloc(sizeof(optcl_mmc_response_get_configuration));

	if (nresponse0 == 0) {
		xfree_aligned(mmc_response);
		return(E_OUTOFMEMORY);
	}

	memset(nresponse0, 0, sizeof(optcl_mmc_response_get_configuration));

	error = optcl_list_create(0, &nresponse0->descriptors);

	if (FAILED(error)) {
		free(nresponse0);
		xfree_aligned(mmc_response);
		return(error);
	}

	/*
	 * Set new data length and execute command
	 * 
	 * NOTE that in MMC-5 standard, the entire set of defined 
	 * feature descriptors amounts to less than 1 KB.
	 */

	data_length = uint32_from_be(*(int32_t*)&mmc_response[0]);

	xfree_aligned(mmc_response);
	
	nresponse0->data_length = data_length;

	if (max_transfer_len > MAX_GET_CONFIG_TRANSFER_LEN)
		max_transfer_len = MAX_GET_CONFIG_TRANSFER_LEN;

	do {
		transfer_size = (data_length > (int32_t)max_transfer_len)
			? max_transfer_len
			: data_length;

		data_length -= max_transfer_len;

		/*
		 * Execute command to get next chunk of data
		 */

		cdb[1] = rt;
		cdb[2] = (uint8_t)(start_feature >> 8);
		cdb[3] = (uint8_t)((start_feature << 8) >> 8);
		cdb[7] = (uint8_t)((uint16_t)transfer_size >> 8);
		cdb[8] = (uint8_t)((((uint16_t)transfer_size) << 8) >> 8);

		mmc_response = xmalloc_aligned(transfer_size, alignment_mask);

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
			xfree_aligned(mmc_response);
			break;
		}

		rt = MMC_GET_CONFIG_RT_FROM;

		/*
		 * Process current chunk of data
		 */

		error = optcl_parse_get_configuration_data(
			mmc_response, 
			transfer_size, 
			&nresponse1
			);

		if (FAILED(error)) {
			xfree_aligned(mmc_response);
			break;
		}

		error = optcl_list_get_tail_pos(nresponse1->descriptors, &it);

		if (FAILED(error)) {
			destroy_error = optcl_list_destroy(nresponse1->descriptors, True);
			error = (FAILED(destroy_error)) ? destroy_error : error;
			xfree_aligned(mmc_response);
			free(nresponse1);
			break;
		}

		error = optcl_list_get_at_pos(nresponse1->descriptors, it, (const pptr_t)&descriptor);

		if (FAILED(error)) {
			destroy_error = optcl_list_destroy(nresponse1->descriptors, True);
			error = (FAILED(destroy_error)) ? destroy_error : error;
			xfree_aligned(mmc_response);
			free(nresponse1);
			break;
		}

		start_feature = descriptor->feature_code + 1;

		nresponse0->current_profile = nresponse1->current_profile;
		
		error = optcl_list_append(nresponse0->descriptors, nresponse1->descriptors);

		if (FAILED(error)) {
			destroy_error = optcl_list_destroy(nresponse1->descriptors, True);
			error = (FAILED(destroy_error)) ? destroy_error : error;
			xfree_aligned(mmc_response);
			free(nresponse1);
			break;
		}

		error = optcl_list_destroy(nresponse1->descriptors, False);

		if (FAILED(error)) {
			xfree_aligned(mmc_response);
			free(nresponse1);
			break;
		}

		free(nresponse1);
		xfree_aligned(mmc_response);

	} while(data_length > 0);

	if (FAILED(error)) {
		destroy_error = optcl_list_destroy(nresponse0->descriptors, False);
		free(nresponse0);
		return(SUCCEEDED(destroy_error) ? error : destroy_error);
	}

	nresponse0->header.command_opcode = MMC_OPCODE_GET_CONFIG;

	*response = nresponse0;

	return(error);
}

RESULT optcl_command_get_event_status(const optcl_device *device,
				      const optcl_mmc_get_event_status *command,
				      optcl_mmc_response_get_event_status **response)
{
	RESULT error;
	
	cdb10 cdb;
	uint32_t alignment;
	uint16_t descriptor_len;
	ptr_t mmc_response = 0;
	optcl_adapter *adapter = 0;
	optcl_mmc_response_get_event_status *nresponse = 0;

	assert(device != 0);
	assert(command != 0);
	assert(response != 0);

	if (device == 0 || command == 0 || response == 0) {
		return(E_INVALIDARG);
	}

	error = optcl_device_get_adapter(device, &adapter);

	if (FAILED(error)) {
		return(error);
	}

	error = optcl_adapter_get_alignment_mask(adapter, &alignment);

	if (FAILED(error)) {
		return(error);
	}

	error = optcl_adapter_destroy(adapter);

	if (FAILED(error)) {
		return(error);
	}

	/*
	 * Execute command
	 */

	memset(cdb, 0, sizeof(cdb));

	cdb[0] = MMC_OPCODE_GET_EVENT_STATUS;
	cdb[1] = command->polled;
	cdb[4] = command->class_request;
	cdb[8] = 4; /* request only event header */

	mmc_response = xmalloc_aligned(cdb[8], alignment);

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
		xfree_aligned(mmc_response);
		return(error);
	}

	error = optcl_parse_get_event_status(mmc_response, cdb[8], &nresponse);

	xfree_aligned(mmc_response);

	if (FAILED(error)) {
		return(error);
	}

	assert(nresponse != 0);

	if (nresponse == 0) {
		return(E_POINTER);
	}

	/*
	 * Execute command
	 */

	descriptor_len = nresponse->ges_header.descriptor_len;

	cdb[7] = (uint8_t)(descriptor_len >> 8);
	cdb[8] = (uint8_t)((descriptor_len << 8) >> 8) + 4;

	mmc_response = xmalloc_aligned(
		descriptor_len + 4, 
		alignment
		);

	if (mmc_response == 0) {
		return(E_OUTOFMEMORY);
	}

	error = optcl_device_command_execute(
		device,
		cdb,
		sizeof(cdb),
		mmc_response,
		descriptor_len + 4
		);

	if (FAILED(error)) {
		xfree_aligned(mmc_response);
		return(error);
	}

	free(nresponse);

	error = optcl_parse_get_event_status(
		mmc_response, 
		descriptor_len + 4, 
		&nresponse
		);

	xfree_aligned(mmc_response);

	if (FAILED(error)) {
		return(error);
	}

	nresponse->header.command_opcode = MMC_OPCODE_GET_EVENT_STATUS;

	*response = nresponse;

	return(SUCCESS);
}


RESULT optcl_command_inquiry(const optcl_device *device, 
			     const optcl_mmc_inquiry *command, 
			     optcl_mmc_response_inquiry **response)
{
	RESULT error;
	RESULT destroy_error;

	cdb6 cdb;
	ptr_t mmc_response = 0;
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
		destroy_error = optcl_adapter_destroy(adapter);
		return(SUCCEEDED(destroy_error) ? error : destroy_error);
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

	mmc_response = xmalloc_aligned(cdb[4], alignment_mask);

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
		xfree_aligned(mmc_response);
		return(error);
	}

	/* Set standard inquiry data length */
	cdb[4] = mmc_response[4] + 4;

	xfree_aligned(mmc_response);

	mmc_response = xmalloc_aligned((size_t)cdb[4], alignment_mask);

	if (mmc_response == 0) {
		return(E_OUTOFMEMORY);
	}

	/*
	 * Execute command
	 */

	/* Get standard inquiry data */
	error = optcl_device_command_execute(
		device, 
		cdb, 
		sizeof(cdb), 
		mmc_response,
		cdb[4]
		);

	if (FAILED(error)) {
		xfree_aligned(mmc_response);
		return(error);
	}

	error = optcl_parse_inquiry_data(mmc_response, cdb[4], &nresponse);

	if (SUCCEEDED(error)) {
		nresponse->header.command_opcode = MMC_OPCODE_INQUIRY;
		*response = nresponse;
	}

	xfree_aligned(mmc_response);

	return(error);
}

RESULT optcl_command_request_sense(const optcl_device *device,
				   const optcl_mmc_request_sense *command,
				   optcl_mmc_response_request_sense **response)
{
	RESULT error;
	RESULT sense_code;
	RESULT destroy_error;
	
	cdb6 cdb;
	uint32_t alignment;
	ptr_t mmc_response = 0;
	optcl_adapter *adapter = 0;
	optcl_mmc_response_request_sense *nresponse = 0;

	assert(device != 0);
	assert(command != 0);
	assert(response != 0);

	if (device == 0 || command == 0 || response == 0) {
		return(E_INVALIDARG);
	}

	error = optcl_device_get_adapter(device, &adapter);

	if (FAILED(error)) {
		return(error);
	}

	assert(adapter != 0);

	if (adapter == 0) {
		return(E_POINTER);
	}

	error = optcl_adapter_get_alignment_mask(adapter, &alignment);

	if (FAILED(error)) {
		destroy_error = optcl_adapter_destroy(adapter);
		return(SUCCEEDED(destroy_error) ? error : destroy_error);
	}

	error = optcl_adapter_destroy(adapter);

	if (FAILED(error)) {
		return(error);
	}

	/*
	 * Execute command
	 */

	memset(cdb, 0, sizeof(cdb));

	cdb[0] = MMC_OPCODE_REQUEST_SENSE;
	cdb[1] = (uint8_t)command->desc;
	cdb[4] = MAX_SENSEDATA_LENGTH;

	mmc_response = xmalloc_aligned(MAX_SENSEDATA_LENGTH, alignment);

	if (mmc_response == 0) {
		return(E_OUTOFMEMORY);
	}

	error = optcl_device_command_execute(
		device,
		cdb,
		sizeof(cdb),
		mmc_response,
		MAX_SENSEDATA_LENGTH
		);

	if (FAILED(error)) {
		xfree_aligned(mmc_response);
		return(error);
	}

	error = optcl_sensedata_get_code(mmc_response, MAX_SENSEDATA_LENGTH, &sense_code);

	xfree_aligned(mmc_response);

	if (FAILED(error)) {
		return(error);
	}

	nresponse = malloc(sizeof(optcl_mmc_response_request_sense));

	if (nresponse == 0) {
		return(E_OUTOFMEMORY);
	}

	nresponse->header.command_opcode = MMC_OPCODE_REQUEST_SENSE;

	nresponse->asc = ERROR_SENSE_ASC(sense_code);
	nresponse->ascq = ERROR_SENSE_ASCQ(sense_code);
	nresponse->sk = ERROR_SENSE_SK(sense_code);

	*response = nresponse;

	return(SUCCESS);
}


/*
 * Deallocators
 */

static RESULT deallocator_mmc_response_close_track_session(optcl_mmc_response *response)
{
	if (response == 0) {
		return(SUCCESS);
	}

	free(response);

	return(SUCCESS);
}

static RESULT deallocator_mmc_response_get_configuration(optcl_mmc_response *response)
{
	RESULT error;
	optcl_mmc_response_get_configuration *mmc_response = 0;

	if (response == 0) {
		return(SUCCESS);
	}

	mmc_response = (optcl_mmc_response_get_configuration*)response;

	assert(mmc_response->descriptors != 0);

	error = optcl_list_destroy(mmc_response->descriptors, 1);

	free(mmc_response);

	return(error);
}

static RESULT deallocator_mmc_response_get_event_status(optcl_mmc_response *response)
{
	RESULT error;
	optcl_mmc_response_get_event_status *mmc_response = 0;

	if (response == 0) {
		return(SUCCESS);
	}

	mmc_response = (optcl_mmc_response_get_event_status*)response;

	assert(mmc_response->descriptors != 0);

	error = optcl_list_destroy(mmc_response->descriptors, 1);

	free(mmc_response);

	return(error);
}

static RESULT deallocator_mmc_response_inquiry(optcl_mmc_response *response)
{
	if (response == 0) {
		return(SUCCESS);
	}

	free(response);

	return(SUCCESS);
}

static RESULT deallocator_mmc_response_request_sense(optcl_mmc_response *response) 
{
	if (response == 0) {
		return(SUCCESS);
	}

	free(response);

	return(SUCCESS);
}


/*
 * Response deallocator table
 */

static struct response_deallocator_entry __deallocator_table[] = {
	{ MMC_OPCODE_CLOSE_TRACK_SESSION,	deallocator_mmc_response_close_track_session	},
	{ MMC_OPCODE_INQUIRY,			deallocator_mmc_response_get_configuration	},
	{ MMC_OPCODE_GET_CONFIG,		deallocator_mmc_response_get_event_status	},
	{ MMC_OPCODE_GET_EVENT_STATUS,		deallocator_mmc_response_inquiry		},
	{ MMC_OPCODE_REQUEST_SENSE,		deallocator_mmc_response_request_sense		}
};


/*
 * Helper functions
 */

response_deallocator get_response_deallocator(uint16_t opcode)
{
	int i = 0;
	int elements = sizeof(__deallocator_table) / sizeof(__deallocator_table[0]);

	while(i < elements) {
		if (__deallocator_table[i].opcode == opcode) {
			break;
		}

		++i;
	}

	if (i == elements) {
		return(0);
	}

	return(__deallocator_table[i].deallocator);
}
