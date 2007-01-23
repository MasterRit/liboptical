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
#include "errors.h"
#include "command.h"
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


/*
 * Helper functions
 */

static optcl_mmc_result_inquiry* parse_std_inquiry_data(const uint8_t *mmc_result, 
							int size)
{
	optcl_mmc_result_inquiry *result;

	assert(size >= 5);
	assert(mmc_result);

	if (!mmc_result || size < 5)
		return 0;

	result = malloc(sizeof(optcl_mmc_result_inquiry));

	if (!result)
		return 0;

	result->qualifier	= mmc_result[0] & 0xe0;		/* 11100000 */
	result->device_type	= mmc_result[0] & 0x1f;		/* 00011111 */
	result->rmb		= mmc_result[1] & 0x80;		/* 10000000 */
	result->version		= mmc_result[2];
	result->normaca		= mmc_result[3] & 0x20;		/* 00100000 */
	result->hisup		= mmc_result[3] & 0x10;		/* 00010000 */
	result->rdf		= mmc_result[3] & 0x0f;		/* 00001111 */
	result->additional_len	= mmc_result[4];
	result->sccs		= mmc_result[5] & 0x80;		/* 10000000 */
	result->acc		= mmc_result[5] & 0x40;		/* 01000000 */
	result->tpgs		= mmc_result[5] & 0x30;		/* 00110000 */
	result->_3pc		= mmc_result[5] & 0x08;		/* 00001000 */
	result->protect		= mmc_result[5] & 0x01;		/* 00000001 */
	result->bque		= mmc_result[6] & 0x80;		/* 10000000 */
	result->encserv		= mmc_result[6] & 0x40;		/* 01000000 */
	result->vs		= mmc_result[6] & 0x20;		/* 00100000 */
	result->multip		= mmc_result[6] & 0x10;		/* 00010000 */
	result->mchngr		= mmc_result[6] & 0x08;		/* 00001000 */
	result->addr16		= mmc_result[6] & 0x01;		/* 00000001 */
	result->wbus16		= mmc_result[7] & 0x20;		/* 00100000 */
	result->sync		= mmc_result[7] & 0x10;		/* 00010000 */
	result->linked		= mmc_result[7] & 0x08;		/* 00001000 */
	result->cmdque		= mmc_result[7] & 0x02;		/* 00000010 */

	/* NOTE result->vs is duplicated at mmc_result[7] & 0x01 ?? */

	strncpy_s((char*)result->vendor, 9, (char*)&mmc_result[8], 8);
	strncpy_s((char*)result->product, 17, (char*)&mmc_result[16], 16);
	strncpy_s((char*)result->vendor_string, 21, (char*)&mmc_result[36], 20);

	result->revision_level	= uint32_from_be(*(uint32_t*)&mmc_result[32]);

	result->clocking	= mmc_result[56] & 0x0c;	/* 00001100 */
	result->qas		= mmc_result[56] & 0x02;	/* 00000010 */
	result->ius		= mmc_result[56] & 0x01;	/* 00000001 */

	result->ver_desc1	= uint16_from_be(*(uint16_t*)&mmc_result[58]);
	result->ver_desc2	= uint16_from_be(*(uint16_t*)&mmc_result[60]);
	result->ver_desc3	= uint16_from_be(*(uint16_t*)&mmc_result[62]);
	result->ver_desc4	= uint16_from_be(*(uint16_t*)&mmc_result[64]);
	result->ver_desc5	= uint16_from_be(*(uint16_t*)&mmc_result[66]);
	result->ver_desc6	= uint16_from_be(*(uint16_t*)&mmc_result[68]);
	result->ver_desc7	= uint16_from_be(*(uint16_t*)&mmc_result[70]);
	result->ver_desc8	= uint16_from_be(*(uint16_t*)&mmc_result[72]);

	return result;
}

/*
 * Command functions
 */

int optcl_command_inquiry(const optcl_device *device, 
			  const optcl_mmc_inquiry *command, 
			  optcl_mmc_result_inquiry **result)
{
	int error;
	uint8_t msg[6];
	uint8_t *mmc_result;
	int alignment_mask;
	optcl_adapter *adapter;
	optcl_mmc_result_inquiry *nresult;

	assert(device);
	assert(command);
	assert(result);

	if (!device || !command || !result)
		return E_INVALIDARG;

	/*
	 * Execute command just to get additional length
	 */

	memset(msg, 0, sizeof(msg));

	msg[0] = MMC_OPCODE_INQUIRY;
	msg[4] = 5; /* the allocation length should be at least five */

	error = optcl_device_get_adapter(device, &adapter);

	if (FAILED(error))
		return error;

	error = optcl_adapter_get_alignment_mask(adapter, &alignment_mask);

	if (FAILED(error)) {
		optcl_adapter_destroy(adapter);
		return error;
	}

	error = optcl_adapter_destroy(adapter);

	if (FAILED(error))
		return error;

	mmc_result = _aligned_malloc(msg[4], alignment_mask);

	if (!mmc_result)
		return E_OUTOFMEMORY;

	/* Get standard inquiry data additional length */
	error = optcl_device_command_execute(
		device, 
		msg, 
		sizeof(msg), 
		mmc_result,
		msg[4]
		);

	if (FAILED(error)) {
		_aligned_free(mmc_result);
		return error;
	}

	/* Set standard inquiry data length */
	msg[4] = mmc_result[4] + 4;

	_aligned_free(mmc_result);

	mmc_result = _aligned_malloc((size_t)msg[4], alignment_mask);

	if (!mmc_result)
		return E_OUTOFMEMORY;

	/* Get standard inquiry data */
	error = optcl_device_command_execute(
		device, 
		msg, 
		sizeof(msg), 
		mmc_result,
		msg[4]
		);

	if (FAILED(error)) {
		_aligned_free(mmc_result);
		return error;
	}

	nresult = parse_std_inquiry_data(mmc_result, msg[4]);

	if (!nresult) {
		_aligned_free(mmc_result);
		return E_OUTOFMEMORY;
	}

	_aligned_free(mmc_result);

	*result = nresult;

	return SUCCESS;
}
