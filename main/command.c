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
#include "sysdevice.h"
#include "types.h"

#include <assert.h>
#include <malloc.h>
#include <memory.h>
#include <string.h>


/*
 * MMC opcodes
 */

#define MMC_OPCODE_BLANK			0x00A1
#define MMC_OPCODE_CLOSE_TRACK_SESSION		0x005B
#define MMC_OPCODE_FORMAT_UNIT			0x0004
#define MMC_OPCODE_GET_CONFIG			0x0046
#define MMC_OPCODE_GET_EVENT_STATUS		0x004A
#define MMC_OPCODE_INQUIRY			0x0012
#define MMC_OPCODE_LOAD_UNLOAD			0x00A6
#define MMC_OPCODE_PREVENT_ALLOW_REMOVAL	0x001E
#define MMC_OPCODE_READ_10			0x0028
#define MMC_OPCODE_READ_12			0x0028
#define MMC_OPCODE_READ_BUFFER			0x003C
#define MMC_OPCODE_READ_BUFFER_CAPACITY		0x005C
#define MMC_OPCODE_REQUEST_SENSE		0x0003


/*
 * Constants used throughout the code
 */

#define READ_BLOCK_SIZE			2048U
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
 * Helper functions
 */

static int8_t equalfn_descriptors(const void *left, const void *right)
{
	optcl_feature_descriptor *ldesc = 0;
	optcl_feature_descriptor *rdesc = 0;

	assert(left != 0);
	assert(right != 0);

	ldesc = (optcl_feature_descriptor*)left;
	rdesc = (optcl_feature_descriptor*)right;

	if (ldesc == 0 && rdesc == 0) {
		return(0);
	} else if (ldesc == 0 && rdesc != 0) {
		return(-1);
	} else if (ldesc != 0 && rdesc == 0) {
		return(1);
	}

	if (ldesc->feature_code == rdesc->feature_code) {
		return(0);
	} else if (ldesc->feature_code < rdesc->feature_code) {
		return(-1);
	} else {
		return(1);
	}
}

/*
 * Parser functions
 */

static RESULT parse_raw_profile_list(const uint8_t mmc_data[], 
				     optcl_feature_profile_list **feature)
{
	RESULT error;

	uint32_t index;
	uint32_t offset;
	optcl_feature_profile_list *nfeature = 0;
	optcl_feature_descriptor *descriptor = 0;

	assert(mmc_data != 0);
	assert(feature != 0);

	if (mmc_data == 0 || feature == 0) {
		return(E_INVALIDARG);
	}

	nfeature = malloc(sizeof(optcl_feature_profile_list));

	if (nfeature == 0) {
		return(E_OUTOFMEMORY);
	}

	memset(nfeature, 0, sizeof(optcl_feature_profile_list));

	/* Parse feature descriptor */

	/* 
	 * Parse profile list feature data 
	 */

	index = 0;
	offset = 4;

	while (offset < nfeature->descriptor.additional_length + 4U) {
		
	}
}

static RESULT parse_raw_event_status_descriptor_data(uint8_t event_class,
						     const uint8_t raw_data[], 
						     size_t size, 
						     optcl_mmc_ges_descriptor **descriptor)
{
	RESULT error = SUCCESS;

	optcl_mmc_ges_media *media = 0;
	optcl_mmc_ges_multihost *multihost = 0;
	optcl_mmc_ges_device_busy *devicebusy = 0;
	optcl_mmc_ges_power_management *pwrmngmnt = 0;
	optcl_mmc_ges_operational_change *opchange = 0;
	optcl_mmc_ges_external_request *exterrequest = 0;

	assert(raw_data != 0);
	assert(size >= 4);
	assert(descriptor != 0);

	if (raw_data == 0 || descriptor == 0) {
		return(E_INVALIDARG);
	}

	if (size < 4) {
		return(E_SIZEMISMATCH);
	}

	switch(event_class) {
		case MMC_GET_EVENT_STATUS_OPCHANGE: {
			opchange = malloc(
				sizeof(optcl_mmc_ges_operational_change)
				);

			if (opchange == 0) {
				error = E_OUTOFMEMORY;
				break;
			}

			opchange->persistent_prev = bool_from_uint8(raw_data[1] & 0x80);/* 10000000 */
			opchange->event_code = raw_data[0] & 0x15;			/* 00001111 */
			opchange->status = raw_data[1] & 0x15;				/* 00001111 */
			opchange->change = uint16_from_be(*(uint16_t*)&raw_data[2]);

			*descriptor = (optcl_mmc_ges_descriptor*)opchange;

			break;
		}
		case MMC_GET_EVENT_STATUS_POWERMGMT: {
			pwrmngmnt = malloc(
				sizeof(optcl_mmc_ges_power_management)
				);

			if (pwrmngmnt == 0) {
				error = E_OUTOFMEMORY;
				break;
			}

			pwrmngmnt->event_code = raw_data[0] & 0x15;			/* 00001111 */
			pwrmngmnt->power_status = raw_data[1];

			*descriptor = (optcl_mmc_ges_descriptor*)pwrmngmnt;

			break;
		}
		case MMC_GET_EVENT_STATUS_EXTREQUEST: {
			exterrequest = malloc(
				sizeof(optcl_mmc_ges_external_request)
				);

			if (exterrequest == 0) {
				error = E_OUTOFMEMORY;
				break;
			}

			exterrequest->persistent_prev = bool_from_uint8(raw_data[1] & 0x80);/* 10000000 */
			exterrequest->event_code = raw_data[0] & 0x15;			/* 00001111 */
			exterrequest->ext_req_status = raw_data[0] & 0x15;		/* 00001111 */
			exterrequest->external_request = uint16_from_be(*(uint16_t*)&raw_data[2]);

			*descriptor = (optcl_mmc_ges_descriptor*)exterrequest;

			break;
		}
		case MMC_GET_EVENT_STATUS_MEDIA: {
			media = malloc(sizeof(optcl_mmc_ges_media));

			if (media == 0) {
				error = E_OUTOFMEMORY;
				break;
			}

			media->event_code = raw_data[0] & 0x15;				/* 00001111 */
			media->media_present = bool_from_uint8(raw_data[1] & 0x02);	/* 00000010 */
			media->tray_open = bool_from_uint8(raw_data[1] & 0x01);		/* 00000001 */
			media->start_slot = raw_data[2];
			media->end_slot = raw_data[3];

			*descriptor = (optcl_mmc_ges_descriptor*)media;

			break;
		}
		case MMC_GET_EVENT_STATUS_MULTIHOST: {
			multihost = malloc(sizeof(optcl_mmc_ges_multihost));

			if (multihost == 0) {
				error = E_OUTOFMEMORY;
				break;
			}

			multihost->event_code = raw_data[0] & 0x15;			/* 00001111 */
			multihost->persistent_prev = bool_from_uint8(raw_data[1] & 0x80);/* 10000000 */
			multihost->multi_host_status = raw_data[1] & 0x15;		/* 00001111 */
			multihost->multi_host_priority = uint16_from_be(*(uint16_t*)&raw_data[2]);

			*descriptor = (optcl_mmc_ges_descriptor*)multihost;

			break;
		}
		case MMC_GET_EVENT_STATUS_DEVICEBUSY: {
			devicebusy = malloc(
				sizeof(optcl_mmc_ges_device_busy)
				);

			if (devicebusy == 0) {
				error = E_OUTOFMEMORY;
				break;
			}

			devicebusy->event_code = raw_data[0] & 0x15;			/* 00001111 */
			devicebusy->busy_status = raw_data[1];
			devicebusy->time = uint16_from_be(*(uint16_t*)&raw_data[2]);

			*descriptor = (optcl_mmc_ges_descriptor*)devicebusy;

			break; 
		}
		default: {
			error = E_OUTOFRANGE;
			break;
		}
	}

	return(error);
}


/*
 * Parser functions
 */ 

static RESULT parse_raw_get_configuration_data(const uint8_t mmc_response[], 
					       uint32_t size,
					       optcl_mmc_response_get_configuration **response)
{
	RESULT error;
	RESULT destroy_error;

	uint32_t offset;
	uint16_t feature_code;
	uint8_t *raw_feature = 0;
	optcl_feature_descriptor *feature = 0;
	optcl_mmc_response_get_configuration *nresponse = 0;

	assert(response != 0);
	assert(mmc_response != 0);
	assert(size >= 8);
	assert(size % 4 == 0);

	if (mmc_response == 0 || response == 0 || size == 0 || (size % 4 != 0)) {
		return(E_INVALIDARG);
	}

	if (size < 8 || (size % 4) != 0) {
		return(E_FEATINVHEADER);
	}

	nresponse = malloc(sizeof(optcl_mmc_response_get_configuration));

	if (nresponse == 0) {
		return(E_OUTOFMEMORY);
	}

	nresponse->data_length = uint32_from_be(*(uint32_t*)&mmc_response[0]);
	nresponse->current_profile = uint16_from_be(*(uint16_t*)&mmc_response[6]);

	error = optcl_list_create(&equalfn_descriptors, &nresponse->descriptors);

	if (FAILED(error)) {
		free(nresponse);
		return(error);
	}

	/*
	 * Parse feature descriptor header
	 */

	offset = 8;

	while (offset + 3U < size) {

		feature = 0;
		raw_feature = (uint8_t*)&mmc_response[offset];
		feature_code = uint16_from_be(*(uint16_t*)&raw_feature[0]);

		error = optcl_feature_create_from_raw(&feature, raw_feature, size - offset);

		if (FAILED(error)) {
			break;
		}

		if (feature == 0) {
			error = E_UNEXPECTED;
			break;
		}

		error = optcl_list_add_tail(nresponse->descriptors, (const ptr_t)feature);

		if (FAILED(error)) {
			free(feature);
			break;
		}

		/* Set next feature offset */
		offset += mmc_response[offset + 3] + 4;
	}

	if (FAILED(error)) {
		destroy_error = optcl_list_destroy(nresponse->descriptors, 1);
		free(nresponse);
		return(SUCCEEDED(destroy_error) ? error : destroy_error);
	}

	*response = nresponse;

	return(SUCCESS);

}

static RESULT parse_raw_get_event_status_data(const uint8_t mmc_response[],
					      uint32_t size,
					      optcl_mmc_response_get_event_status **response)
{
	RESULT error;
	RESULT destroy_error;

	uint16_t offset;
	uint16_t descriptor_len;
	optcl_list *descriptors = 0;
	optcl_mmc_ges_descriptor *ndescriptor = 0;
	optcl_mmc_response_get_event_status *nresponse = 0;

	assert(mmc_response != 0);
	assert(response != 0);
	assert(size >= 4);

	if (mmc_response == 0 || response == 0 || size < 4) {
		return(E_INVALIDARG);
	}

	nresponse = malloc(sizeof(optcl_mmc_response_get_event_status));

	if (nresponse == 0) {
		return(E_OUTOFMEMORY);
	}

	memset(nresponse, 0, sizeof(optcl_mmc_response_get_event_status));

	descriptor_len = uint16_from_be(*(uint16_t*)&mmc_response[0]);

	nresponse->ges_header.descriptor_len = descriptor_len;
	nresponse->ges_header.nea = bool_from_uint8(mmc_response[2] & 0x80);	/* 10000000 */
	nresponse->ges_header.notification_class = mmc_response[2] & 0x07;	/* 00000111 */
	nresponse->ges_header.event_class = mmc_response[3];
	nresponse->event_class = nresponse->ges_header.event_class;

	error = optcl_list_create(0, &descriptors);

	if (FAILED(error)) {
		free(nresponse);
		return(error);
	}

	assert(descriptors);

	if (descriptors == 0) {
		free(nresponse);
		return(E_POINTER);
	}

	offset = 4;
	error = SUCCESS;

	while(offset < descriptor_len + 4) {
		error = parse_raw_event_status_descriptor_data(
			nresponse->event_class, 
			&mmc_response[offset], 
			size - offset, 
			&ndescriptor
			);

		if (FAILED(error)) {
			break;
		}

		error = optcl_list_add_tail(descriptors, (const ptr_t)ndescriptor);

		if (FAILED(error)) {
			break;
		}

		offset += 4;
	}

	if (FAILED(error)) {
		free(nresponse);
		destroy_error = optcl_list_destroy(descriptors, 1);
		return(SUCCEEDED(destroy_error) ? error : destroy_error);
	}

	nresponse->descriptors = descriptors;
	*response = nresponse;

	return(error);
}

static RESULT parse_raw_inquiry_data(const uint8_t mmc_response[], 
				     uint32_t size,
				     optcl_mmc_response_inquiry **response)
{
	optcl_mmc_response_inquiry *nresponse = 0;

	assert(mmc_response);
	assert(size >= 5);
	assert(response);

	if (mmc_response == 0 || response == 0 || size < 0) {
		return(E_INVALIDARG);
	}

	nresponse = malloc(sizeof(optcl_mmc_response_inquiry));

	if (nresponse == 0) {
		return(E_OUTOFMEMORY);
	}

	memset(nresponse, 0, sizeof(optcl_mmc_response_inquiry));

	nresponse->qualifier = mmc_response[0] & 0xE0;			/* 11100000 */
	nresponse->device_type = mmc_response[0] & 0x1F;		/* 00011111 */
	nresponse->rmb = bool_from_uint8(mmc_response[1] & 0x80);	/* 10000000 */
	nresponse->version = mmc_response[2];
	nresponse->normaca = mmc_response[3] & 0x20;			/* 00100000 */
	nresponse->hisup = bool_from_uint8(mmc_response[3] & 0x10);	/* 00010000 */
	nresponse->rdf = mmc_response[3] & 0x0F;			/* 00001111 */

	if (size > 4) {
		nresponse->additional_len = mmc_response[4];
	}

	if (size > 5) {
		nresponse->sccs	= bool_from_uint8(mmc_response[5] & 0x80);	/* 10000000 */
		nresponse->acc = bool_from_uint8(mmc_response[5] & 0x40);	/* 01000000 */
		nresponse->tpgs	= mmc_response[5] & 0x30;			/* 00110000 */
		nresponse->_3pc	= bool_from_uint8(mmc_response[5] & 0x08);	/* 00001000 */
		nresponse->protect = bool_from_uint8(mmc_response[5] & 0x01);	/* 00000001 */
	}

	if (size > 6) {
		nresponse->bque	= bool_from_uint8(mmc_response[6] & 0x80);	/* 10000000 */
		nresponse->encserv = bool_from_uint8(mmc_response[6] & 0x40);	/* 01000000 */
		nresponse->vs = bool_from_uint8(mmc_response[6] & 0x20);	/* 00100000 */
		nresponse->multip = bool_from_uint8(mmc_response[6] & 0x10);	/* 00010000 */
		nresponse->mchngr = bool_from_uint8(mmc_response[6] & 0x08);	/* 00001000 */
		nresponse->addr16 = bool_from_uint8(mmc_response[6] & 0x01);	/* 00000001 */
	}

	if (size > 7) {
		nresponse->wbus16 = bool_from_uint8(mmc_response[7] & 0x20);	/* 00100000 */
		nresponse->sync	= bool_from_uint8(mmc_response[7] & 0x10);	/* 00010000 */
		nresponse->linked = bool_from_uint8(mmc_response[7] & 0x08);	/* 00001000 */
		nresponse->cmdque = bool_from_uint8(mmc_response[7] & 0x02);	/* 00000010 */

		/* NOTE result->vs is duplicated at mmc_response[7] & 0x01 ?? */
	}

	if (size > 16) {
		xstrncpy((char*)nresponse->vendor, 9, (char*)&mmc_response[8], 8);
	}

	if (size > 32) {
		xstrncpy((char*)nresponse->product, 17, (char*)&mmc_response[16], 16);
	}

	if (size > 36) {
		nresponse->revision_level = uint32_from_le(*(uint32_t*)&mmc_response[32]);
	}

	if (size > 56) {
		xstrncpy((char*)nresponse->vendor_string, 21, (char*)&mmc_response[36], 20);
	}

	if (size > 56) {
		nresponse->clocking = mmc_response[56] & 0x0C;			/* 00001100 */
		nresponse->qas = bool_from_uint8(mmc_response[56] & 0x02);	/* 00000010 */
		nresponse->ius = bool_from_uint8(mmc_response[56] & 0x01);	/* 00000001 */
	}

	if (size > 60) {
		nresponse->ver_desc1 = uint16_from_le(*(uint16_t*)&mmc_response[58]);
	}

	if (size > 62) {
		nresponse->ver_desc2 = uint16_from_le(*(uint16_t*)&mmc_response[60]);
	}

	if (size > 64) {
		nresponse->ver_desc3 = uint16_from_le(*(uint16_t*)&mmc_response[62]);
	}

	if (size > 66) {
		nresponse->ver_desc4 = uint16_from_le(*(uint16_t*)&mmc_response[64]);
	}

	if (size > 68) {
		nresponse->ver_desc5 = uint16_from_le(*(uint16_t*)&mmc_response[66]);
	}

	if (size > 70) {
		nresponse->ver_desc6 = uint16_from_le(*(uint16_t*)&mmc_response[68]);
	}

	if (size > 72) {
		nresponse->ver_desc7 = uint16_from_le(*(uint16_t*)&mmc_response[70]);
	}

	if (size > 74) {
		nresponse->ver_desc8 = uint16_from_le(*(uint16_t*)&mmc_response[72]);
	}

	*response = nresponse;

	return(SUCCESS);
}

static RESULT parse_raw_read_buffer_data(const uint8_t mmc_response[],
					 uint32_t size,
					 uint8_t mode,
					 optcl_mmc_response_read_buffer **response)
{
	RESULT error = SUCCESS;

	ptr_t data = 0;
	optcl_mmc_response_read_buffer *nresponse = 0;

	assert(size > 0);
	assert(mmc_response != 0);
	assert(response != 0);

	if (mmc_response == 0 || response == 0 || size == 0) {
		return(E_INVALIDARG);
	}

	nresponse = malloc(sizeof(optcl_mmc_response_read_buffer));

	if (nresponse == 0) {
		return(E_OUTOFMEMORY);
	}

	memset(nresponse, 0, sizeof(optcl_mmc_response_read_buffer));

	nresponse->mode = mode;

	switch(mode) {
		case MMC_READ_BUFFER_MODE_COMBINED: {
			assert(size >= 4);

			if (size < 4) {
				error = E_SIZEMISMATCH;
				break;
			}

			nresponse->response.combined.buffer_capacity 
				= (uint32_t)(mmc_response[1] << 16 | mmc_response[2] << 8 | mmc_response[3]);

			if (size == 4) {
				break;
			}

			data = malloc(size - 4);

			if (data == 0) {
				error = E_OUTOFMEMORY;
				break;
			}
			
			xmemcpy(data, size - 4, &mmc_response[4], size - 4);

			nresponse->response.combined.buffer = data;

			break;
		}
		case MMC_READ_BUFFER_MODE_DATA: {
			data = malloc(size);

			if (data == 0) {
				error = E_OUTOFMEMORY;
				break;
			}

			xmemcpy(data, size, mmc_response, size);

			nresponse->response.data.buffer = data;

			break;
		}
		case MMC_READ_BUFFER_MODE_DESCRIPTOR: {
			nresponse->response.descriptor.offset_boundary = mmc_response[0];

			nresponse->response.descriptor.buffer_capacity 
				= (uint32_t)(mmc_response[1] << 16 | mmc_response[2] << 8 | mmc_response[3]);

			break;
		}
		case MMC_READ_BUFFER_MODE_ECHO: {
			data = malloc(size);

			if (data == 0) {
				error = E_OUTOFMEMORY;
				break;
			}

			xmemcpy(data, size, mmc_response, size);

			nresponse->response.echo.buffer = data;

			break;
				
		}
		case MMC_READ_BUFFER_MODE_ECHO_DESC: {
			nresponse->response.echo_desc.buffer_capacity
				= (uint32_t)((mmc_response[2] & 0x1F) | mmc_response[3]);

			break;
		}
		case MMC_READ_BUFFER_MODE_EXPANDER: {
			data = malloc(size);

			if (data == 0) {
				error = E_OUTOFMEMORY;
				break;
			}

			xmemcpy(data, size, mmc_response, size);

			nresponse->response.expander.buffer = data;

			break;
		}
		case MMC_READ_BUFFER_MODE_VENDOR: {
			data = malloc(size);

			if (data == 0) {
				error = E_OUTOFMEMORY;
				break;
			}

			xmemcpy(data, size, mmc_response, size);

			nresponse->response.vendor.buffer = data;

			break;
		}
		default: {
			assert(False);
			free(nresponse);
			return(E_OUTOFRANGE);
		}
	}

	*response = nresponse;

	return(error);
}


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

RESULT optcl_command_format_unit(const optcl_device *device,
				 const optcl_mmc_format_unit *command)
{
	RESULT error;
	RESULT destroy_error;

	cdb6 cdb;
	uint32_t alignment;
	uint8_t cdbparams[12];
	optcl_adapter *adapter = 0;

	assert(device != 0);
	assert(command != 0);

	if (device == 0 || command == 0) {
		return(E_INVALIDARG);
	}

	error = optcl_device_get_adapter(device, &adapter);

	if (FAILED(error)) {
		return(error);
	}

	assert(adapter);

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

	cdb[0] = MMC_OPCODE_FORMAT_UNIT;
	cdb[1] = (command->cmplist << 3) | 0x11;	/* 00010001 */

	memset(cdbparams, 0, sizeof(cdbparams));

	if (command->fov == True) {
		cdbparams[1] = 
			(command->dcrt << 5) 
			| (command->try_out << 2) 
			| (command->immed << 1) 
			| command->vs 
			| 0x10;				/* 00010000 */

		/* format descriptor length should be set to 8 */
		cdbparams[3] = 8;
	}

	/* number of blocks */
	cdbparams[4] = (uint8_t)(command->num_of_blocks >> 24);
	cdbparams[5] = (uint8_t)((command->num_of_blocks << 8) >> 24);
	cdbparams[6] = (uint8_t)((command->num_of_blocks << 16) >> 24);
	cdbparams[7] = (uint8_t)((command->num_of_blocks << 24) >> 24);

	cdbparams[8] = (command->format_type << 2) | command->format_subtype;
	
	switch(command->format_type) {
		case MMC_FORMAT_FULL_FORMAT:
		case MMC_FORMAT_SPARE_AREA_EXPANSION:
		case MMC_FORMAT_ZONE_REFORMAT:
		case MMC_FORMAT_ZONE_FORMAT:
		case MMC_FORMAT_CD_RW_DVD_RW_FULL_FORMAT:
		case MMC_FORMAT_CD_RW_DVD_RW_GROW_SESSION:
		case MMC_FORMAT_CD_RW_DVD_RW_ADD_SESSION:
		case MMC_FORMAT_DVD_RW_QUICK_GROW_LAST_BORDER:
		case MMC_FORMAT_DVD_RW_QUICK_ADD_BORDER:
		case MMC_FORMAT_DVD_RW_QUICK_FORMAT:
		case MMC_FORMAT_HD_DVD_R_TEST_ZONE_EXPANSION:
		case MMC_FORMAT_MRW_FORMAT:
		case MMC_FORMAT_BD_RE_FULL_FORMAT_WITH_SPARE_AREAS:
		case MMC_FORMAT_BD_RE_FULL_FORMAT_WITHOUT_SPARE_AREAS: {
			cdbparams[9] = (uint8_t)((command->type_dependant.other.type_dependent << 8) >> 24);
			cdbparams[10] = (uint8_t)((command->type_dependant.other.type_dependent << 16) >> 24);
			cdbparams[11] = (uint8_t)((command->type_dependant.other.type_dependent << 24) >> 24);
			break;
		}
		case MMC_FORMAT_FULL_FORMAT_WITH_SPARING_PARAMS: {
			cdbparams[9] = command->type_dependant.ff_with_sparing.m;
			cdbparams[11] = command->type_dependant.ff_with_sparing.n;
			break;

		}
		case MMC_FORMAT_DVD_PLUS_RW_BASIC_FORMAT: {
			cdbparams[11] = 
				(command->type_dependant.dvd_plus_rw_basicf.quick_start << 1) 
				| command->type_dependant.dvd_plus_rw_basicf.restart;
			break;
		}
		case MMC_FORMAT_BD_R_FULL_FORMAT_WITH_SPARE_AREAS: {
			cdbparams[9] = 
				(command->type_dependant.bd_r_with_spare_areas.isa_v << 7) 
				| command->type_dependant.bd_r_with_spare_areas.sadp;

			cdbparams[10] = 
				(command->type_dependant.bd_r_with_spare_areas.tdma_v << 7)
				| command->type_dependant.bd_r_with_spare_areas.tdmadp;
			break;
		}
		default: {
			assert(False);
			error = E_INVALIDARG;
			break;
		}
	}

	if (FAILED(error)) {
		return(error);
	}

	error = optcl_device_command_execute(
		device,
		cdb,
		sizeof(cdb),
		cdbparams,
		sizeof(cdbparams)
		);

	return(error);
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

		error = parse_raw_get_configuration_data(
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

	error = parse_raw_get_event_status_data(mmc_response, cdb[8], &nresponse);

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

	error = parse_raw_get_event_status_data(
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

	error = parse_raw_inquiry_data(mmc_response, cdb[4], &nresponse);

	if (SUCCEEDED(error)) {
		nresponse->header.command_opcode = MMC_OPCODE_INQUIRY;
		*response = nresponse;
	}

	xfree_aligned(mmc_response);

	return(error);
}

RESULT optcl_command_load_unload_medium(const optcl_device *device,
					const optcl_mmc_load_unload_medium *command)
{
	RESULT error;

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

	cdb[0] = MMC_OPCODE_LOAD_UNLOAD;
	cdb[1] = command->immed;
	cdb[4] = (command->load_unload << 1) | command->start;
	cdb[8] = command->slot;

	error = optcl_device_command_execute(
		device,
		cdb,
		sizeof(cdb),
		0,
		0
		);

	return(error);
}

RESULT optcl_command_prevent_allow_removal(const optcl_device *device,
					   const optcl_mmc_prevent_allow_removal *command)
{
	RESULT error;

	cdb6 cdb;

	assert(device != 0);
	assert(command != 0);

	if (device == 0 || command == 0) {
		return(E_INVALIDARG);
	}

	/*
	 * Execute command
	 */

	memset(cdb, 0, sizeof(cdb));

	cdb[0] = MMC_OPCODE_PREVENT_ALLOW_REMOVAL;
	cdb[4] = (command->persistent << 1) | command->prevent;

	error = optcl_device_command_execute(
		device,
		cdb,
		sizeof(cdb),
		0,
		0
		);

	return(error);
}

RESULT optcl_command_read_10(const optcl_device *device,
			     const optcl_mmc_read_10 *command,
			     optcl_mmc_response_read **response)
{
	RESULT error;
	RESULT destroy_error;

	cdb10 cdb;
	uint32_t alignment;
	ptr_t mmc_response = 0;
	uint32_t max_transfer_len;
	optcl_adapter *adapter = 0;
	optcl_mmc_response_read *nresponse = 0;

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

	nresponse = malloc(sizeof(optcl_mmc_response_read));

	if (nresponse == 0) {
		return(E_OUTOFMEMORY);
	}

	if (command->transfer_length * READ_BLOCK_SIZE > max_transfer_len) {
		return(E_INVALIDARG);
	}

	/*
	 * Execute command
	 */

	memset(cdb, 0, sizeof(cdb));

	cdb[0] = MMC_OPCODE_READ_10;
	cdb[1] = (command->fua << 3);
	cdb[2] = (uint8_t)(command->start_lba >> 24);
	cdb[3] = (uint8_t)((command->start_lba << 8) >> 24);
	cdb[4] = (uint8_t)((command->start_lba << 16) >> 24);
	cdb[5] = (uint8_t)((command->start_lba << 24) >> 24);
	cdb[7] = (uint8_t)(command->transfer_length >> 8);
	cdb[8] = (uint8_t)((command->transfer_length << 8) >> 8);
	
	mmc_response = xmalloc_aligned(
		command->transfer_length * READ_BLOCK_SIZE, 
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
		command->transfer_length * READ_BLOCK_SIZE
		);

	if (FAILED(error)) {
		free(nresponse);
		xfree_aligned(mmc_response);
	}

	nresponse->data = mmc_response;

	*response = nresponse;

	return(error);
}

RESULT optcl_command_read_12(const optcl_device *device,
			     const optcl_mmc_read_12 *command,
			     optcl_mmc_response_read **response)
{
	RESULT error;
	RESULT destroy_error;

	cdb12 cdb;
	uint32_t alignment;
	ptr_t mmc_response = 0;
	uint32_t max_transfer_len;
	optcl_adapter *adapter = 0;
	optcl_mmc_response_read *nresponse = 0;

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

	nresponse = malloc(sizeof(optcl_mmc_response_read));

	if (nresponse == 0) {
		return(E_OUTOFMEMORY);
	}

	if (command->transfer_length * READ_BLOCK_SIZE > max_transfer_len) {
		return(E_INVALIDARG);
	}

	/*
	 * Execute command
	 */

	memset(cdb, 0, sizeof(cdb));
	
	cdb[0] = MMC_OPCODE_READ_12;
	cdb[1] = (command->fua << 3);
	cdb[2] = (uint8_t)(command->start_lba >> 24);
	cdb[3] = (uint8_t)((command->start_lba << 8) >> 24);
	cdb[4] = (uint8_t)((command->start_lba << 16) >> 24);
	cdb[5] = (uint8_t)((command->start_lba << 24) >> 24);
	cdb[6] = (uint8_t)(command->transfer_length >> 24);
	cdb[7] = (uint8_t)((command->transfer_length << 8) >> 24);
	cdb[8] = (uint8_t)((command->transfer_length << 16) >> 24);
	cdb[9] = (uint8_t)((command->transfer_length << 24) >> 24);
	cdb[10] = (command->streaming << 7);

	mmc_response = xmalloc_aligned(
		command->transfer_length * READ_BLOCK_SIZE, 
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
		command->transfer_length * READ_BLOCK_SIZE
		);

	if (FAILED(error)) {
		free(nresponse);
		xfree_aligned(mmc_response);
	}

	nresponse->data = mmc_response;

	*response = nresponse;

	return(error);
}

RESULT optcl_command_read_buffer(const optcl_device *device,
				 const optcl_mmc_read_buffer *command,
				 optcl_mmc_response_read_buffer **response)
{
	RESULT error;
	RESULT destroy_error;

	cdb10 cdb;
	uint32_t alignment;
	uint32_t max_transfer_len;
	ptr_t mmc_response = 0;
	optcl_adapter *adapter = 0;
	optcl_mmc_response_read_buffer *nresponse = 0;

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

	if (command->allocation_len > max_transfer_len) {
		return(E_INVALIDARG);
	}

	if (command->mode == MMC_READ_BUFFER_MODE_DESCRIPTOR 
		&& command->allocation_len != 3 ) 
	{
		assert(False);
		return(E_INVALIDARG);
	} else if (command->mode == MMC_READ_BUFFER_MODE_ECHO_DESC
		&& command->allocation_len != 4) 
	{
		assert(False);
		return(E_INVALIDARG);
	}

	/*
	 * Execute command
	 */

	memset(cdb, 0, sizeof(cdb));

	cdb[0] = MMC_OPCODE_READ_BUFFER;
	cdb[1] = command->mode & 0x1F;
	cdb[2] = command->buffer_id;
	cdb[3] = (uint8_t)((command->buffer_offset << 8) >> 24);
	cdb[4] = (uint8_t)((command->buffer_offset << 16) >> 24);
	cdb[5] = (uint8_t)((command->buffer_offset << 24) >> 24);
	cdb[6] = (uint8_t)((command->allocation_len << 8) >> 24);
	cdb[7] = (uint8_t)((command->allocation_len << 16) >> 24);
	cdb[8] = (uint8_t)((command->allocation_len << 24) >> 24);

	mmc_response = xmalloc_aligned(command->allocation_len, alignment);

	if (mmc_response == 0) {
		return(E_OUTOFMEMORY);
	}

	error = optcl_device_command_execute(
		device,
		cdb,
		sizeof(cdb),
		mmc_response,
		4
		);

	if (FAILED(error)) {
		xfree_aligned(mmc_response);
		return(error);
	}

	error = parse_raw_read_buffer_data(
		mmc_response, 
		command->allocation_len, 
		command->mode, 
		&nresponse
		);

	xfree_aligned(mmc_response);

	if (FAILED(error)) {
		return(error);
	}

	assert(nresponse != 0);

	if (nresponse == 0) {
		return(E_POINTER);
	}

	*response = nresponse;

	return(error);
}

RESULT optcl_command_read_buffer_capacity(const optcl_device *device,
					  const optcl_mmc_read_buffer_capacity *command,
					  optcl_mmc_response_read_buffer_capacity **response)
{
	RESULT error;
	RESULT destroy_error;

	cdb10 cdb;
	uint32_t alignment;
	ptr_t mmc_response = 0;
	optcl_adapter *adapter = 0;
	optcl_mmc_response_read_buffer_capacity *nresponse = 0;

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

	cdb[0] = MMC_OPCODE_READ_BUFFER_CAPACITY;
	cdb[1] = command->block & 0x01;
	cdb[8] = 12; /* size of the response buffer */

	mmc_response = xmalloc_aligned(12, alignment);

	if (mmc_response == 0) {
		return(E_OUTOFMEMORY);
	}

	error = optcl_device_command_execute(
		device,
		cdb,
		sizeof(cdb),
		mmc_response,
		12
		);

	if (FAILED(error)) {
		xfree_aligned(mmc_response);
		return(error);
	}

	nresponse = malloc(sizeof(optcl_mmc_response_read_buffer_capacity));

	if (nresponse == 0) {
		return(E_OUTOFMEMORY);
	}

	nresponse->header.command_opcode = MMC_OPCODE_READ_BUFFER_CAPACITY;

	if (command->block) {
		nresponse->desc.block.data_length 
			= uint16_from_be(*(uint16_t*)&mmc_response[0]);

		nresponse->desc.block.block = bool_from_uint8(mmc_response[3]);

		nresponse->desc.block.available_buffer_len 
			= uint32_from_be(*(uint32_t*)&mmc_response[8]);
	} else {
		nresponse->desc.bytes.data_length
			= uint16_from_be(*(uint16_t*)&mmc_response[0]);

		nresponse->desc.bytes.buffer_len
			= uint32_from_be(*(uint32_t*)&mmc_response[4]);

		nresponse->desc.bytes.buffer_blank_len
			= uint32_from_be(*(uint32_t*)&mmc_response[8]);
	}

	*response = nresponse;

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

	assert(response->command_opcode == MMC_OPCODE_CLOSE_TRACK_SESSION);

	if (response->command_opcode != MMC_OPCODE_CLOSE_TRACK_SESSION) {
		return(E_CMNDINVOPCODE);
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

	assert(response->command_opcode == MMC_OPCODE_GET_CONFIG);

	if (response->command_opcode != MMC_OPCODE_GET_CONFIG) {
		return(E_CMNDINVOPCODE);
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

	assert(response->command_opcode == MMC_OPCODE_GET_EVENT_STATUS);

	if (response->command_opcode != MMC_OPCODE_GET_EVENT_STATUS) {
		return(E_CMNDINVOPCODE);
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

	assert(response->command_opcode == MMC_OPCODE_INQUIRY);

	if (response->command_opcode != MMC_OPCODE_INQUIRY) {
		return(E_CMNDINVOPCODE);
	}

	free(response);

	return(SUCCESS);
}

static RESULT deallocator_mmc_response_read_10(optcl_mmc_response *response)
{
	optcl_mmc_response_read *mmc_response = 0;

	if (response == 0) {
		return(SUCCESS);
	}

	assert(response->command_opcode == MMC_OPCODE_READ_10);

	if (response->command_opcode != MMC_OPCODE_READ_10) {
		return(E_CMNDINVOPCODE);
	}

	mmc_response = (optcl_mmc_response_read*)response;

	xfree_aligned(mmc_response->data);
	free(mmc_response);

	return(SUCCESS);
}

static RESULT deallocator_mmc_response_read_buffer(optcl_mmc_response *response)
{
	optcl_mmc_response_read_buffer *mmc_response = 0;

	if (response == 0) {
		return(SUCCESS);
	}

	assert(response->command_opcode == MMC_OPCODE_READ_BUFFER);

	if (response->command_opcode != MMC_OPCODE_READ_BUFFER) {
		return(E_CMNDINVOPCODE);
	}

	mmc_response = (optcl_mmc_response_read_buffer*)response;

	switch(mmc_response->mode) {
		case MMC_READ_BUFFER_MODE_COMBINED: {
			free(mmc_response->response.combined.buffer);
			break;
		}
		case MMC_READ_BUFFER_MODE_DATA: {
			free(mmc_response->response.data.buffer);
			break;
		}
		case MMC_READ_BUFFER_MODE_DESCRIPTOR: {
			break;
		}
		case MMC_READ_BUFFER_MODE_ECHO: {
			free(mmc_response->response.echo.buffer);
			break;
		}
		case MMC_READ_BUFFER_MODE_ECHO_DESC: {
			break;
		}
		case MMC_READ_BUFFER_MODE_EXPANDER: {
			free(mmc_response->response.expander.buffer);
			break;
		}
		case MMC_READ_BUFFER_MODE_VENDOR: {
			free(mmc_response->response.vendor.buffer);
			break;
		}
		default: {
			assert(False);
			return(E_OUTOFRANGE);
		}
	}

	free(mmc_response);

	return(SUCCESS);
}

static RESULT deallocator_mmc_response_read_buffer_capcity(optcl_mmc_response *response)
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

	assert(response->command_opcode == MMC_OPCODE_REQUEST_SENSE);

	if (response->command_opcode != MMC_OPCODE_REQUEST_SENSE) {
		return(E_CMNDINVOPCODE);
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
	{ MMC_OPCODE_READ_10,			deallocator_mmc_response_read_10		},
	{ MMC_OPCODE_READ_12,			deallocator_mmc_response_read_10		},
	{ MMC_OPCODE_READ_BUFFER,		deallocator_mmc_response_read_buffer		},
	{ MMC_OPCODE_READ_BUFFER_CAPACITY,	deallocator_mmc_response_read_buffer_capcity	},
	{ MMC_OPCODE_REQUEST_SENSE,		deallocator_mmc_response_request_sense		}
};


static response_deallocator get_response_deallocator(uint16_t opcode)
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
