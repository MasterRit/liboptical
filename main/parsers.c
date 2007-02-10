/*
    parsers.c - Raw command data parsers implementation
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

#include "stdafx.h"

#include "debug.h"
#include "errors.h"
#include "feature.h"
#include "list.h"
#include "parsers.h"
#include "types.h"

#include <assert.h>
#include <malloc.h>
#include <string.h>

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

	if (mmc_data == 0 || feature) {
		return(E_INVALIDARG);
	}

	nfeature = malloc(sizeof(optcl_feature_profile_list));

	if (!nfeature) {
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

/*
 * Parser functions
 */ 

int optcl_parse_get_configuration_data(const uint8_t *mmc_response, 
				       uint32_t size,
				       optcl_mmc_response_get_configuration **response)
{
	RESULT error;
	RESULT destroy_error;
	uint32_t offset;
	uint8_t *raw_feature;
	uint16_t feature_code;
	optcl_feature_descriptor *feature;
	optcl_mmc_response_get_configuration *nresponse;

	assert(mmc_response);
	assert(size >= 8);
	assert(size % 4 == 0);
	assert(response);

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

int optcl_parse_inquiry_data(const uint8_t *mmc_response, 
			     uint32_t size,
			     optcl_mmc_response_inquiry **response)
{
	optcl_mmc_response_inquiry *nresponse;

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

	nresponse->qualifier = mmc_response[0] & 0xe0;			/* 11100000 */
	nresponse->device_type = mmc_response[0] & 0x1f;		/* 00011111 */
	nresponse->rmb = bool_from_uint8(mmc_response[1] & 0x80);	/* 10000000 */
	nresponse->version = mmc_response[2];
	nresponse->normaca = mmc_response[3] & 0x20;			/* 00100000 */
	nresponse->hisup = bool_from_uint8(mmc_response[3] & 0x10);	/* 00010000 */
	nresponse->rdf = mmc_response[3] & 0x0f;			/* 00001111 */

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
		strncpy_s((char*)nresponse->vendor, 9, (char*)&mmc_response[8], 8);
	}

	if (size > 32) {
		strncpy_s((char*)nresponse->product, 17, (char*)&mmc_response[16], 16);
	}

	if (size > 56) {
		strncpy_s((char*)nresponse->vendor_string, 21, (char*)&mmc_response[36], 20);
	}

	if (size > 36) {
		nresponse->revision_level = uint32_from_le(*(uint32_t*)&mmc_response[32]);
	}

	if (size > 56) {
		nresponse->clocking = mmc_response[56] & 0x0c;			/* 00001100 */
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
