/*
    profile.c - Optical device/media profile
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

#include "errors.h"
#include "feature.h"

#include <assert.h>
#include <malloc.h>
#include <memory.h>
#include <string.h>


/*
 * Internal feature structures
 */

typedef RESULT (*raw_feature_parser)(const uint8_t mmc_data[], 
				     uint32_t size, 
				     optcl_feature_descriptor **response);

struct feature_sizes_entry {
	uint16_t code;
	uint16_t size;
	raw_feature_parser parser;
};

/*
 * Table of feature helper functions forward declarations
 */

static int32_t get_feature_size(uint16_t feature_code);
static raw_feature_parser get_feature_parser(uint16_t feature_code);


/*
 * Parse helper functions
 */

static bool_t check_feature_descriptor(const optcl_feature_descriptor *descriptor,
				       uint16_t feature_code)
{
	assert(descriptor != 0);

	if (descriptor == 0) {
		return(False);
	}

	return((bool_t)(descriptor->feature_code == feature_code));
}

/*
 * Raw feature data parsers
 */

static RESULT parse_feature_descriptor(const uint8_t mmc_data[],
				       uint32_t size,
				       optcl_feature_descriptor **descriptor)
{
	optcl_feature_descriptor *ndescriptor = 0;

	assert(mmc_data != 0);
	assert(descriptor != 0);
	assert(size >= 4);

	if (mmc_data == 0 || descriptor == 0 || size < 4) {
		return(E_INVALIDARG);
	}

	assert(mmc_data[3] % 4 == 0);

	if (mmc_data[3] % 4 != 0) {
		return(E_FEATINVHEADER);
	}

	ndescriptor = malloc(sizeof(optcl_feature_descriptor));

	if (ndescriptor == 0) {
		return(E_OUTOFMEMORY);
	}

	ndescriptor->feature_code	= uint16_from_be(*(const uint16_t*)&mmc_data[0]);
	ndescriptor->current		= bool_from_uint8(mmc_data[2] & 0x01);	/* 00000001 */
	ndescriptor->persistent		= bool_from_uint8(mmc_data[2] & 0x02);	/* 00000010 */
	ndescriptor->version		= mmc_data[2] & 0x3c;			/* 00111100 */
	ndescriptor->additional_length	= mmc_data[3];

	*descriptor = ndescriptor;

	return(SUCCESS);
}

static RESULT parse_profile_list(const uint8_t mmc_data[],
				 uint32_t size,
				 optcl_feature_descriptor **response)
{
	uint8_t i;
	bool_t check;
	RESULT error;
	uint32_t offset;
	optcl_feature_profile_list *feature = 0;
	optcl_feature_descriptor *descriptor = 0;

	assert(size >= 4);
	assert(mmc_data != 0);
	assert(response != 0);

	if (mmc_data == 0 || response == 0 || size < 4) {
		return(E_INVALIDARG);
	}

	error = parse_feature_descriptor(mmc_data, size, &descriptor);

	if (FAILED(error)) {
		return(error);
	}

	check = check_feature_descriptor(descriptor, FEATURE_PROFILE_LIST);

	assert(check == True);

	if (check != True) {
		free(descriptor);
		return(E_FEATINVHEADER);
	}

	error = optcl_feature_create(FEATURE_PROFILE_LIST, (optcl_feature**)&feature);

	if (FAILED(error)) {
		free(descriptor);
		return(error);
	}

	assert(feature != 0);

	if (feature == 0) {
		free(descriptor);
		return(E_POINTER);
	}

	memcpy(&feature->descriptor, descriptor, sizeof(optcl_feature_descriptor));

	free(descriptor);

	for(i = 0; i < feature->descriptor.additional_length / 4; ++i) {
		offset = (i + 1) * 4;

		feature->profile_numbers[i] = 
			uint16_from_be(*(uint16_t*)&mmc_data[offset]);

		feature->current_profiles[i] = 
			bool_from_uint8(mmc_data[offset + 2] & 0x01);	/* 00000001 */
	}

	feature->profile_count = i + 1;

	*response = (optcl_feature_descriptor*)feature;

	return(SUCCESS);
}

static RESULT parse_core(const uint8_t mmc_data[],
			 uint32_t size,
			 optcl_feature_descriptor **response)
{
	RESULT error;
	bool_t check;
	optcl_feature_core *feature = 0;
	optcl_feature_descriptor *descriptor = 0;

	assert(size >= 4);
	assert(mmc_data != 0);
	assert(response != 0);

	if (mmc_data == 0 || response == 0 || size < 4) {
		return(E_INVALIDARG);
	}

	error = parse_feature_descriptor(mmc_data, size, &descriptor);

	if (FAILED(error)) {
		return(error);
	}

	check = check_feature_descriptor(descriptor, FEATURE_CORE);

	assert(check == True);

	if (check != True) {
		free(descriptor);
		return(E_FEATINVHEADER);
	}

	error = optcl_feature_create(FEATURE_CORE, (optcl_feature**)&feature);

	if (FAILED(error)) {
		free(descriptor);
		return(error);
	}

	if (feature == 0) {
		free(descriptor);
		return(E_POINTER);
	}

	memcpy(&feature->descriptor, descriptor, sizeof(optcl_feature_descriptor));

	free(descriptor);

	if (feature->descriptor.additional_length > 0) {
		feature->phys_i_standard = uint32_from_be(*(uint32_t*)&mmc_data[4]);
	}

	if (feature->descriptor.additional_length > 4) {
		feature->inq2 = bool_from_uint8(mmc_data[8] & 0x02);	/* 00000010 */
		feature->dbe = bool_from_uint8(mmc_data[8] & 0x01);	/* 00000001 */
	}

	*response = (optcl_feature_descriptor*)feature;

	return(SUCCESS);
}

static RESULT parse_morphing(const uint8_t mmc_data[],
			     uint32_t size,
			     optcl_feature_descriptor **response)
{
	RESULT error;
	bool_t check;
	optcl_feature_morphing *feature = 0;
	optcl_feature_descriptor *descriptor = 0;

	assert(size >= 4);
	assert(mmc_data != 0);
	assert(response != 0);

	if (mmc_data == 0 || response == 0 || size < 4) {
		return(E_INVALIDARG);
	}

	error = parse_feature_descriptor(mmc_data, size, &descriptor);

	if (FAILED(error)) {
		return(error);
	}

	check = check_feature_descriptor(descriptor, FEATURE_MORPHING);

	assert(check == True);

	if (check != True) {
		free(descriptor);
		return(E_FEATINVHEADER);
	}

	error = optcl_feature_create(FEATURE_MORPHING, (optcl_feature**)&feature);

	if (FAILED(error)) {
		free(descriptor);
		return(error);
	}

	if (feature == 0) {
		free(descriptor);
		return(E_POINTER);
	}

	memcpy(&feature->descriptor, descriptor, sizeof(optcl_feature_descriptor));

	free(descriptor);

	if (feature->descriptor.additional_length > 0) {
		feature->ocevent = bool_from_uint8(mmc_data[4] & 0x02);	/* 00000010 */
		feature->async = bool_from_uint8(mmc_data[4] & 0x01);	/* 00000001 */
	}

	*response = (optcl_feature_descriptor*)feature;

	return(SUCCESS);
}

static RESULT parse_removable_medium(const uint8_t mmc_data[],
				     uint32_t size,
				     optcl_feature_descriptor **response)
{
	RESULT error;
	bool_t check;
	optcl_feature_descriptor *descriptor = 0;
	optcl_feature_removable_medium *feature = 0;

	assert(size >= 4);
	assert(mmc_data != 0);
	assert(response != 0);

	if (mmc_data == 0 || response == 0 || size < 4) {
		return(E_INVALIDARG);
	}

	error = parse_feature_descriptor(mmc_data, size, &descriptor);

	if (FAILED(error)) {
		return(error);
	}

	check = check_feature_descriptor(descriptor, FEATURE_REMOVABLE_MEDIUM);

	assert(check == True);

	if (check != True) {
		free(descriptor);
		return(E_FEATINVHEADER);
	}

	error = optcl_feature_create(FEATURE_REMOVABLE_MEDIUM, (optcl_feature**)&feature);

	if (FAILED(error)) {
		free(descriptor);
		return(error);
	}

	if (feature == 0) {
		free(descriptor);
		return(E_POINTER);
	}

	memcpy(&feature->descriptor, descriptor, sizeof(optcl_feature_descriptor));

	free(descriptor);

	if (feature->descriptor.additional_length > 0) {
		feature->lmt = mmc_data[4] & 0xe0;				/* 11100000 */
		feature->eject = bool_from_uint8(mmc_data[4] & 0x08);		/* 00001000 */
		feature->pvnt_jmpr = bool_from_uint8(mmc_data[4] & 0x04);	/* 00000100 */
		feature->lock = bool_from_uint8(mmc_data[4] & 0x01);		/* 00000001 */
	}

	*response = (optcl_feature_descriptor*)feature;

	return(SUCCESS);
}

static RESULT parse_write_protect(const uint8_t mmc_data[],
				  uint32_t size,
				  optcl_feature_descriptor **response)
{
	RESULT error;
	bool_t check;
	optcl_feature_write_protect *feature = 0;
	optcl_feature_descriptor *descriptor = 0;

	assert(size >= 4);
	assert(mmc_data != 0);
	assert(response != 0);

	if (mmc_data == 0 || response == 0 || size < 4) {
		return(E_INVALIDARG);
	}

	error = parse_feature_descriptor(mmc_data, size, &descriptor);

	if (FAILED(error)) {
		return(error);
	}

	check = check_feature_descriptor(descriptor, FEATURE_WRITE_PROTECT);

	if (check != True) {
		free(descriptor);
		return(E_FEATINVHEADER);
	}

	error = optcl_feature_create(FEATURE_WRITE_PROTECT, (optcl_feature**)&feature);

	if (FAILED(error)) {
		free(descriptor);
		return(error);
	}

	if (feature == 0) {
		free(descriptor);
		return(E_POINTER);
	}

	memcpy(&feature->descriptor, descriptor, sizeof(optcl_feature_descriptor));

	free(descriptor);

	if (feature->descriptor.additional_length > 0) {
		feature->dwp = bool_from_uint8(mmc_data[4] & 0x08);	/* 00001000 */
		feature->wdcb = bool_from_uint8(mmc_data[4] & 0x04);	/* 00000100 */
		feature->spwp = bool_from_uint8(mmc_data[4] & 0x02);	/* 00000010 */
		feature->sswpp = bool_from_uint8(mmc_data[4] & 0x01);	/* 00000001 */
	}

	*response = (optcl_feature_descriptor*)feature;

	return(SUCCESS);
}

static RESULT parse_random_readable(const uint8_t mmc_data[],
				    uint32_t size,
				    optcl_feature_descriptor **response)
{
	RESULT error;
	bool_t check;
	optcl_feature_descriptor *descriptor = 0;
	optcl_feature_random_readable *feature = 0;

	assert(size >= 4);
	assert(mmc_data != 0);
	assert(response != 0);

	if (mmc_data == 0 || response == 0 || size < 4) {
		return(E_INVALIDARG);
	}

	error = parse_feature_descriptor(mmc_data, size, &descriptor);

	if (FAILED(error)) {
		return(error);
	}

	check = check_feature_descriptor(descriptor, FEATURE_RANDOM_READABLE);

	if (check != True) {
		free(descriptor);
		return(E_FEATINVHEADER);
	}

	error = optcl_feature_create(FEATURE_RANDOM_READABLE, (optcl_feature**)&feature);

	if (FAILED(error)) {
		free(descriptor);
		return(E_FEATINVHEADER);
	}

	if (feature == 0) {
		free(descriptor);
		return(E_POINTER);
	}

	memcpy(&feature->descriptor, descriptor, sizeof(optcl_feature_descriptor));

	free(descriptor);

	if (feature->descriptor.additional_length > 0) {
		feature->logical_block_size = uint32_from_be(*(uint32_t*)&mmc_data[4]);
	}

	if (feature->descriptor.additional_length > 4) {
		feature->blocking = uint16_from_be(*(uint16_t*)&mmc_data[8]);
		feature->pp = bool_from_uint8(mmc_data[10] & 0x01);	/* 00000001 */
	}

	*response = (optcl_feature_descriptor*)feature;

	return(SUCCESS);
}

static RESULT parse_multi_read(const uint8_t mmc_data[],
			       uint32_t size,
			       optcl_feature_descriptor **response)
{
	RESULT error;
	bool_t check;
	optcl_feature_descriptor *descriptor = 0;
	optcl_feature_multi_read *feature = 0;

	assert(size >= 4);
	assert(mmc_data != 0);
	assert(response != 0);

	if (mmc_data == 0 || response == 0 || size < 4) {
		return(E_INVALIDARG);
	}

	error = parse_feature_descriptor(mmc_data, size, &descriptor);

	if (FAILED(error)) {
		return(error);
	}

	check = check_feature_descriptor(descriptor, FEATURE_MULTI_READ);

	if (check != True) {
		free(descriptor);
		return(E_FEATINVHEADER);
	}

	error = optcl_feature_create(FEATURE_MULTI_READ, (optcl_feature**)&feature);

	if (FAILED(error)) {
		free(descriptor);
		return(error);
	}

	if (feature == 0) {
		free(descriptor);
		return(E_POINTER);
	}

	memcpy(&feature->descriptor, descriptor, sizeof(optcl_feature_descriptor));

	free(descriptor);

	*response = (optcl_feature_descriptor*)feature;

	return(SUCCESS);
}

static RESULT parse_cd_read(const uint8_t mmc_data[],
			    uint32_t size,
			    optcl_feature_descriptor **response)
{
	RESULT error;
	bool_t check;
	optcl_feature_descriptor *descriptor = 0;
	optcl_feature_cd_read *feature = 0;

	assert(size >= 4);
	assert(mmc_data != 0);
	assert(response != 0);

	if (mmc_data == 0 || response == 0 || size < 4) {
		return(E_INVALIDARG);
	}

	error = parse_feature_descriptor(mmc_data, size, &descriptor);

	if (FAILED(error)) {
		return(error);
	}

	check = check_feature_descriptor(descriptor, FEATURE_CD_READ);

	if (check != True) {
		free(descriptor);
		return(E_FEATINVHEADER);
	}

	error = optcl_feature_create(FEATURE_CD_READ, (optcl_feature**)&feature);

	if (FAILED(error)) {
		free(descriptor);
		return(error);
	}

	memcpy(&feature->descriptor, descriptor, sizeof(optcl_feature_descriptor));

	free(descriptor);

	if (feature->descriptor.additional_length > 0) {
		feature->dap = bool_from_uint8(mmc_data[4] & 0x80);		/* 10000000 */
		feature->c2_flags = bool_from_uint8(mmc_data[4] & 0x02);	/* 00000010 */
		feature->cd_text = bool_from_uint8(mmc_data[4] & 0x01);		/* 00000001 */
	}

	*response = (optcl_feature_descriptor*)feature;

	return(SUCCESS);
}

static RESULT parse_dvd_read(const uint8_t mmc_data[],
			     uint32_t size,
			     optcl_feature_descriptor **response)
{
	RESULT error;
	bool_t check;
	optcl_feature_descriptor *descriptor = 0;
	optcl_feature_dvd_read *feature = 0;

	assert(size >= 4);
	assert(mmc_data != 0);
	assert(response != 0);

	if (mmc_data == 0 || response == 0 || size < 4) {
		return(E_INVALIDARG);
	}

	error = parse_feature_descriptor(mmc_data, size, &descriptor);

	if (FAILED(error)) {
		return(error);
	}

	check = check_feature_descriptor(descriptor, FEATURE_DVD_READ);

	if (check != True) {
		free(descriptor);
		return(E_FEATINVHEADER);
	}

	error = optcl_feature_create(FEATURE_DVD_READ, (optcl_feature**)&feature);

	if (FAILED(error)) {
		free(descriptor);
		return(error);
	}

	if (feature == 0) {
		free(descriptor);
		return(E_POINTER);
	}

	memcpy(&feature->descriptor, descriptor, sizeof(optcl_feature_descriptor));

	free(descriptor);

	if (feature->descriptor.additional_length > 0) {
		feature->multi110 = bool_from_uint8(mmc_data[4] & 0x01);	/* 00000001 */
		feature->dual_r	= bool_from_uint8(mmc_data[6] & 0x01);		/* 00000001 */
	}

	*response = (optcl_feature_descriptor*)feature;

	return(SUCCESS);
}

static RESULT parse_random_writable(const uint8_t mmc_data[],
				    uint32_t size,
				    optcl_feature_descriptor **response)
{
	RESULT error;
	bool_t check;
	optcl_feature_descriptor *descriptor = 0;
	optcl_feature_random_writable *feature = 0;

	assert(size >= 4);
	assert(mmc_data != 0);
	assert(response != 0);

	if (mmc_data == 0 || response == 0 || size < 4) {
		return(E_INVALIDARG);
	}

	error = parse_feature_descriptor(mmc_data, size, &descriptor);

	if (FAILED(error)) {
		return(error);
	}

	check = check_feature_descriptor(descriptor, FEATURE_RANDOM_WRITABLE);

	if (check != True) {
		free(descriptor);
		return(E_FEATINVHEADER);
	}

	error = optcl_feature_create(FEATURE_RANDOM_WRITABLE, (optcl_feature**)&feature);

	if (FAILED(error)) {
		free(descriptor);
		return(error);
	}

	if (feature == 0) {
		free(descriptor);
		return(error);
	}

	memcpy(&feature->descriptor, descriptor, sizeof(optcl_feature_descriptor));

	free(descriptor);

	if (feature->descriptor.additional_length > 0) {
		feature->last_logical_block = uint32_from_be(*(uint32_t*)&mmc_data[4]);
	}

	if (feature->descriptor.additional_length > 4) {
		feature->logical_block_size = uint32_from_be(*(uint32_t*)&mmc_data[8]);
	}

	if (feature->descriptor.additional_length > 8) {
		feature->blocking = uint16_from_be(*(uint16_t*)&mmc_data[12]);
		feature->pp = bool_from_uint8(mmc_data[14] & 0x01);	/* 00000001 */
	}

	*response = (optcl_feature_descriptor*)feature;

	return(SUCCESS);
}

static RESULT parse_inc_streaming_writable(const uint8_t mmc_data[],
					   uint32_t size,
					   optcl_feature_descriptor **response)
{
	RESULT error;
	bool_t check;
	optcl_feature_descriptor *descriptor = 0;
	optcl_feature_inc_streaming_writable *feature = 0;

	assert(size >= 4);
	assert(mmc_data != 0);
	assert(response != 0);

	if (mmc_data == 0 || response == 0 || size < 4) {
		return(E_INVALIDARG);
	}

	error = parse_feature_descriptor(mmc_data, size, &descriptor);

	if (FAILED(error)) {
		return(error);
	}

	check = check_feature_descriptor(descriptor, FEATURE_INC_STREAMING_WRITABLE);

	if (check != True) {
		free(descriptor);
		return(E_FEATINVHEADER);
	}

	error = optcl_feature_create(FEATURE_INC_STREAMING_WRITABLE, (optcl_feature**)&feature);

	if (FAILED(error)) {
		free(descriptor);
		return(error);
	}

	if (feature == 0) {
		free(descriptor);
		return(E_POINTER);
	}

	memcpy(&feature->descriptor, descriptor, sizeof(optcl_feature_descriptor));

	free(descriptor);

	if (feature->descriptor.additional_length > 0) {
		feature->supported_dbts	= uint16_from_be(*(uint16_t*)&mmc_data[4]);
		feature->trio = bool_from_uint8(mmc_data[6] & 0x04);	/* 00000100 */
		feature->arsv = bool_from_uint8(mmc_data[6] & 0x02);	/* 00000010 */
		feature->buf = bool_from_uint8(mmc_data[6] & 0x01);	/* 00000001 */
		feature->link_size_number = mmc_data[7];
	}

	if (feature->descriptor.additional_length > feature->link_size_number + 4) {
		memcpy(&feature->link_sizes, &mmc_data[8], feature->link_size_number);
	}

	*response = (optcl_feature_descriptor*)feature;

	return(SUCCESS);
}

static RESULT parse_sector_erasable(const uint8_t mmc_data[],
				    uint32_t size,
				    optcl_feature_descriptor **response)
{
	RESULT error;
	bool_t check;
	optcl_feature_descriptor *descriptor = 0;
	optcl_feature_sector_erasable *feature = 0;

	assert(size >= 4);
	assert(mmc_data != 0);
	assert(response != 0);

	if (mmc_data == 0 || response == 0 || size < 4) {
		return(E_INVALIDARG);
	}

	error = parse_feature_descriptor(mmc_data, size, &descriptor);

	if (FAILED(error)) {
		return(error);
	}

	check = check_feature_descriptor(descriptor, FEATURE_SECTOR_ERASABLE);

	if (check != True) {
		free(descriptor);
		return(E_FEATINVHEADER);
	}

	error = optcl_feature_create(FEATURE_SECTOR_ERASABLE, (optcl_feature**)&feature);

	if (FAILED(error)) {
		free(descriptor);
		return(error);
	}

	if (feature == 0) {
		free(descriptor);
		return(error);
	}

	memcpy(&feature->descriptor, descriptor, sizeof(optcl_feature_descriptor));

	free(descriptor);

	*response = (optcl_feature_descriptor*)feature;

	return(SUCCESS);
}

static RESULT parse_formattable(const uint8_t mmc_data[],
				uint32_t size,
				optcl_feature_descriptor **response)
{
	RESULT error;
	bool_t check;
	optcl_feature_descriptor *descriptor = 0;
	optcl_feature_formattable *feature = 0;

	assert(size >= 4);
	assert(mmc_data != 0);
	assert(response != 0);

	if (mmc_data == 0 || response == 0 || size < 4) {
		return(E_INVALIDARG);
	}

	error = parse_feature_descriptor(mmc_data, size, &descriptor);

	if (FAILED(error)) {
		return(error);
	}

	check = check_feature_descriptor(descriptor, FEATURE_FORMATTABLE);

	if (check != True) {
		free(descriptor);
		return(E_FEATINVHEADER);
	}

	error = optcl_feature_create(FEATURE_FORMATTABLE, (optcl_feature**)&feature);

	if (FAILED(error)) {
		free(descriptor);
		return(error);
	}

	if (feature == 0) {
		free(descriptor);
		return(error);
	}

	memcpy(&feature->descriptor, descriptor, sizeof(optcl_feature_descriptor));

	free(descriptor);

	if (feature->descriptor.additional_length > 0) {
		feature->renosa = bool_from_uint8(mmc_data[4] & 0x08);	/* 00001000 */
		feature->expand = bool_from_uint8(mmc_data[4] & 0x04);	/* 00000100 */
		feature->qcert = bool_from_uint8(mmc_data[4] & 0x02);	/* 00000010 */
		feature->cert = bool_from_uint8(mmc_data[4] & 0x01);	/* 00000001 */
	}

	if (feature->descriptor.additional_length > 4) {
		feature->rrm = bool_from_uint8(mmc_data[8] & 0x01);	/* 00000001 */
	}

	*response = (optcl_feature_descriptor*)feature;

	return(SUCCESS);
}

static RESULT parse_hw_defect_management(const uint8_t mmc_data[],
					 uint32_t size,
					 optcl_feature_descriptor **response)
{
	RESULT error;
	bool_t check;
	optcl_feature_descriptor *descriptor = 0;
	optcl_feature_hw_defect_mngmnt *feature = 0;

	assert(size >= 4);
	assert(mmc_data != 0);
	assert(response != 0);

	if (mmc_data == 0 || response == 0 || size < 4) {
		return(E_INVALIDARG);
	}

	error = parse_feature_descriptor(mmc_data, size, &descriptor);

	if (FAILED(error)) {
		return(error);
	}

	check = check_feature_descriptor(descriptor, FEATURE_HW_DEFECT_MANAGEMENT);

	if (check != True) {
		free(descriptor);
		return(E_FEATINVHEADER);
	}

	error = optcl_feature_create(FEATURE_HW_DEFECT_MANAGEMENT, (optcl_feature**)&feature);

	if (FAILED(error)) {
		free(descriptor);
		return(error);
	}

	if (feature == 0) {
		free(descriptor);
		return(error);
	}

	memcpy(&feature->descriptor, descriptor, sizeof(optcl_feature_descriptor));

	free(descriptor);

	if (feature->descriptor.additional_length > 0) {
		feature->ssa = bool_from_uint8(mmc_data[4] & 0x80);	/* 10000000 */
	}

	*response = (optcl_feature_descriptor*)feature;

	return(SUCCESS);
}

static RESULT parse_write_once(const uint8_t mmc_data[],
			       uint32_t size,
			       optcl_feature_descriptor **response)
{
	RESULT error;
	bool_t check;
	optcl_feature_descriptor *descriptor = 0;
	optcl_feature_write_once *feature = 0;

	assert(size >= 4);
	assert(mmc_data != 0);
	assert(response != 0);

	if (mmc_data == 0 || response == 0 || size < 4) {
		return(E_INVALIDARG);
	}

	error = parse_feature_descriptor(mmc_data, size, &descriptor);

	if (FAILED(error)) {
		return(error);
	}

	check = check_feature_descriptor(descriptor, FEATURE_WRITE_ONCE);

	if (check != True) {
		free(descriptor);
		return(E_FEATINVHEADER);
	}

	error = optcl_feature_create(FEATURE_WRITE_ONCE, (optcl_feature**)&feature);

	if (FAILED(error)) {
		free(descriptor);
		return(error);
	}

	if (feature == 0) {
		free(descriptor);
		return(error);
	}

	memcpy(&feature->descriptor, descriptor, sizeof(optcl_feature_descriptor));

	free(descriptor);

	if (feature->descriptor.additional_length > 0) {
		feature->logical_block_size = uint32_from_be(*(uint32_t*)&mmc_data[4]);
	}

	if (feature->descriptor.additional_length > 4) {
		feature->blocking = uint16_from_be(*(uint16_t*)&mmc_data[8]);
		feature->pp = bool_from_uint8(mmc_data[10] & 0x01);	/* 00000001 */
	}

	*response = (optcl_feature_descriptor*)feature;

	return(SUCCESS);
}

static RESULT parse_restricted_overwrite(const uint8_t mmc_data[],
					 uint32_t size,
					 optcl_feature_descriptor **response)
{
	RESULT error;
	bool_t check;
	optcl_feature_descriptor *descriptor = 0;
	optcl_feature_restricted_ovr *feature = 0;

	assert(size >= 4);
	assert(mmc_data != 0);
	assert(response != 0);

	if (mmc_data == 0 || response == 0 || size < 4) {
		return(E_INVALIDARG);
	}

	error = parse_feature_descriptor(mmc_data, size, &descriptor);

	if (FAILED(error)) {
		return(error);
	}

	check = check_feature_descriptor(descriptor, FEATURE_RESTRICTED_OVERWRITE);

	if (check != True) {
		free(descriptor);
		return(E_FEATINVHEADER);
	}

	error = optcl_feature_create(FEATURE_RESTRICTED_OVERWRITE, (optcl_feature**)&feature);

	if (FAILED(error)) {
		free(descriptor);
		return(error);
	}

	if (feature == 0) {
		free(descriptor);
		return(error);
	}

	memcpy(&feature->descriptor, descriptor, sizeof(optcl_feature_descriptor));

	free(descriptor);

	*response = (optcl_feature_descriptor*)feature;

	return(SUCCESS);
}

static RESULT parse_cdrw_cav_write(const uint8_t mmc_data[],
				   uint32_t size,
				   optcl_feature_descriptor **response)
{
	RESULT error;
	bool_t check;
	optcl_feature_descriptor *descriptor = 0;
	optcl_feature_cdrw_cav_write *feature = 0;

	assert(size >= 4);
	assert(mmc_data != 0);
	assert(response != 0);

	if (mmc_data == 0 || response == 0 || size < 4) {
		return(E_INVALIDARG);
	}

	error = parse_feature_descriptor(mmc_data, size, &descriptor);

	if (FAILED(error)) {
		return(error);
	}

	check = check_feature_descriptor(descriptor, FEATURE_CDRW_CAV_WRITE);

	if (check != True) {
		free(descriptor);
		return(E_FEATINVHEADER);
	}

	error = optcl_feature_create(FEATURE_CDRW_CAV_WRITE, (optcl_feature**)&feature);

	if (FAILED(error)) {
		free(descriptor);
		return(error);
	}

	if (feature == 0) {
		free(descriptor);
		return(error);
	}

	memcpy(&feature->descriptor, descriptor, sizeof(optcl_feature_descriptor));

	free(descriptor);

	*response = (optcl_feature_descriptor*)feature;

	return(SUCCESS);
}

static RESULT parse_mrw(const uint8_t mmc_data[],
			uint32_t size,
			optcl_feature_descriptor **response)
{
	RESULT error;
	bool_t check;
	optcl_feature_descriptor *descriptor = 0;
	optcl_feature_mrw *feature = 0;

	assert(size >= 4);
	assert(mmc_data != 0);
	assert(response != 0);

	if (mmc_data == 0 || response == 0 || size < 4) {
		return(E_INVALIDARG);
	}

	error = parse_feature_descriptor(mmc_data, size, &descriptor);

	if (FAILED(error)) {
		return(error);
	}

	check = check_feature_descriptor(descriptor, FEATURE_MRW);

	if (check != True) {
		free(descriptor);
		return(E_FEATINVHEADER);
	}

	error = optcl_feature_create(FEATURE_MRW, (optcl_feature**)&feature);

	if (FAILED(error)) {
		free(descriptor);
		return(error);
	}

	if (feature == 0) {
		free(descriptor);
		return(error);
	}

	memcpy(&feature->descriptor, descriptor, sizeof(optcl_feature_descriptor));

	free(descriptor);

	if (feature->descriptor.additional_length > 0) {
		feature->dvd_plus_write	= bool_from_uint8(mmc_data[4] & 0x04);	/* 00000100 */
		feature->dvd_plus_read = bool_from_uint8(mmc_data[4] & 0x02);	/* 00000010 */
		feature->cd_write = bool_from_uint8(mmc_data[4] & 0x01);	/* 00000001 */
	}

	*response = (optcl_feature_descriptor*)feature;

	return(SUCCESS);
}

static RESULT parse_enh_defect_reporting(const uint8_t mmc_data[],
					 uint32_t size,
					 optcl_feature_descriptor **response)
{
	RESULT error;
	bool_t check;
	optcl_feature_descriptor *descriptor = 0;
	optcl_feature_enh_defect_reporting *feature = 0;

	assert(size >= 4);
	assert(mmc_data != 0);
	assert(response != 0);

	if (mmc_data == 0 || response == 0 || size < 4) {
		return(E_INVALIDARG);
	}

	error = parse_feature_descriptor(mmc_data, size, &descriptor);

	if (FAILED(error)) {
		return(error);
	}

	check = check_feature_descriptor(descriptor, FEATURE_ENH_DEFECT_REPORTING);

	if (check != True) {
		free(descriptor);
		return(E_FEATINVHEADER);
	}

	error = optcl_feature_create(FEATURE_ENH_DEFECT_REPORTING, (optcl_feature**)&feature);

	if (FAILED(error)) {
		free(descriptor);
		return(error);
	}

	if (feature == 0) {
		free(descriptor);
		return(error);
	}

	memcpy(&feature->descriptor, descriptor, sizeof(optcl_feature_descriptor));

	free(descriptor);

	if (feature->descriptor.additional_length > 0) {
		feature->drt_dm	= bool_from_uint8(mmc_data[4] & 0x01);	/* 00000001 */
		feature->dbi_cache_zones_num = mmc_data[5];
		feature->entries_num = uint16_from_be(*(uint16_t*)&mmc_data[6]);
	}

	*response = (optcl_feature_descriptor*)feature;

	return(SUCCESS);
}

static RESULT parse_dvd_plus_rw(const uint8_t mmc_data[],
				uint32_t size,
				optcl_feature_descriptor **response)
{
	RESULT error;
	bool_t check;
	optcl_feature_descriptor *descriptor = 0;
	optcl_feature_dvd_plus_rw *feature = 0;

	assert(size >= 4);
	assert(mmc_data != 0);
	assert(response != 0);

	if (mmc_data == 0 || response == 0 || size < 4) {
		return(E_INVALIDARG);
	}

	error = parse_feature_descriptor(mmc_data, size, &descriptor);

	if (FAILED(error)) {
		return(error);
	}

	check = check_feature_descriptor(descriptor, FEATURE_DVD_PLUS_RW);

	if (check != True) {
		free(descriptor);
		return(E_FEATINVHEADER);
	}

	error = optcl_feature_create(FEATURE_DVD_PLUS_RW, (optcl_feature**)&feature);

	if (FAILED(error)) {
		free(descriptor);
		return(error);
	}

	if (feature == 0) {
		free(descriptor);
		return(error);
	}

	memcpy(&feature->descriptor, descriptor, sizeof(optcl_feature_descriptor));

	free(descriptor);

	if (feature->descriptor.additional_length > 0) {
		feature->write = bool_from_uint8(mmc_data[4] & 0x01);		/* 00000001 */
		feature->quick_start = bool_from_uint8(mmc_data[5] & 0x02);	/* 00000010 */
		feature->close_only = bool_from_uint8(mmc_data[5] & 0x01);	/* 00000001 */
	}

	*response = (optcl_feature_descriptor*)feature;

	return(SUCCESS);
}

static RESULT parse_dvd_plus_r(const uint8_t mmc_data[],
			       uint32_t size,
			       optcl_feature_descriptor **response)
{
	RESULT error;
	bool_t check;
	optcl_feature_descriptor *descriptor = 0;
	optcl_feature_dvd_plus_r *feature = 0;

	assert(size >= 4);
	assert(mmc_data != 0);
	assert(response != 0);

	if (mmc_data == 0 || response == 0 || size < 4) {
		return(E_INVALIDARG);
	}

	error = parse_feature_descriptor(mmc_data, size, &descriptor);

	if (FAILED(error)) {
		return(error);
	}

	check = check_feature_descriptor(descriptor, FEATURE_DVD_PLUS_R);

	if (check != True) {
		free(descriptor);
		return(E_FEATINVHEADER);
	}

	error = optcl_feature_create(FEATURE_DVD_PLUS_R, (optcl_feature**)&feature);

	if (FAILED(error)) {
		free(descriptor);
		return(error);
	}

	if (feature == 0) {
		free(descriptor);
		return(error);
	}

	memcpy(&feature->descriptor, descriptor, sizeof(optcl_feature_descriptor));

	free(descriptor);

	if (feature->descriptor.additional_length > 0) {
		feature->write = bool_from_uint8(mmc_data[4] & 0x01);	/* 00000001 */
	}
	
	*response = (optcl_feature_descriptor*)feature;

	return(SUCCESS);
}

static RESULT parse_rigid_restricted_overwrite(const uint8_t mmc_data[],
					       uint32_t size,
					       optcl_feature_descriptor **response)
{
	RESULT error;
	bool_t check;
	optcl_feature_descriptor *descriptor = 0;
	optcl_feature_rigid_restricted_ovr *feature = 0;

	assert(size >= 4);
	assert(mmc_data != 0);
	assert(response != 0);

	if (mmc_data == 0 || response == 0 || size < 4) {
		return(E_INVALIDARG);
	}

	error = parse_feature_descriptor(mmc_data, size, &descriptor);

	if (FAILED(error)) {
		return(error);
	}

	check = check_feature_descriptor(descriptor, FEATURE_RIGID_RESTRICTED_OVERWRITE);

	if (check != True) {
		free(descriptor);
		return(E_FEATINVHEADER);
	}

	error = optcl_feature_create(FEATURE_RIGID_RESTRICTED_OVERWRITE, (optcl_feature**)&feature);

	if (FAILED(error)) {
		free(descriptor);
		return(error);
	}

	if (feature == 0) {
		free(descriptor);
		return(error);
	}

	memcpy(&feature->descriptor, descriptor, sizeof(optcl_feature_descriptor));

	free(descriptor);

	if (feature->descriptor.additional_length > 0) {
		feature->dsdg = bool_from_uint8(mmc_data[4] & 0x08);		/* 00001000 */
		feature->dsdr = bool_from_uint8(mmc_data[4] & 0x04);		/* 00000100 */
		feature->intermediate = bool_from_uint8(mmc_data[4] & 0x02);	/* 00000010 */
		feature->blank = bool_from_uint8(mmc_data[4] & 0x01);		/* 00000001 */
	}
	
	*response = (optcl_feature_descriptor*)feature;

	return(SUCCESS);
}

static RESULT parse_cd_tao(const uint8_t mmc_data[],
			   uint32_t size,
			   optcl_feature_descriptor **response)
{
	RESULT error;
	bool_t check;
	optcl_feature_descriptor *descriptor = 0;
	optcl_feature_cd_tao *feature = 0;

	assert(size >= 4);
	assert(mmc_data != 0);
	assert(response != 0);

	if (mmc_data == 0 || response == 0 || size < 4) {
		return(E_INVALIDARG);
	}

	error = parse_feature_descriptor(mmc_data, size, &descriptor);

	if (FAILED(error)) {
		return(error);
	}

	check = check_feature_descriptor(descriptor, FEATURE_CD_TAO);

	if (check != True) {
		free(descriptor);
		return(E_FEATINVHEADER);
	}

	error = optcl_feature_create(FEATURE_CD_TAO, (optcl_feature**)&feature);

	if (FAILED(error)) {
		free(descriptor);
		return(error);
	}

	if (feature == 0) {
		free(descriptor);
		return(error);
	}

	memcpy(&feature->descriptor, descriptor, sizeof(optcl_feature_descriptor));

	free(descriptor);

	if (feature->descriptor.additional_length > 0) {
		feature->buf = bool_from_uint8(mmc_data[4] & 0x40);		/* 01000000 */
		feature->rw_raw = bool_from_uint8(mmc_data[4] & 0x10);		/* 00010000 */
		feature->rw_pack = bool_from_uint8(mmc_data[4] & 0x08);		/* 00001000 */
		feature->test_write = bool_from_uint8(mmc_data[4] & 0x04);	/* 00000100 */
		feature->cd_rw = bool_from_uint8(mmc_data[4] & 0x02);		/* 00000010 */
		feature->rw_subcode = bool_from_uint8(mmc_data[4] & 0x01);	/* 00000001 */
		feature->data_type_supported = uint16_from_be(*(uint16_t*)&mmc_data[6]);
	}

	*response = (optcl_feature_descriptor*)feature;

	return(SUCCESS);
}

static RESULT parse_cd_mastering(const uint8_t mmc_data[],
				 uint32_t size,
				 optcl_feature_descriptor **response)
{
	RESULT error;
	bool_t check;
	optcl_feature_descriptor *descriptor = 0;
	optcl_feature_cd_mastering *feature = 0;

	assert(size >= 4);
	assert(mmc_data != 0);
	assert(response != 0);

	if (mmc_data == 0 || response == 0 || size < 4) {
		return(E_INVALIDARG);
	}

	error = parse_feature_descriptor(mmc_data, size, &descriptor);

	if (FAILED(error)) {
		return(error);
	}

	check = check_feature_descriptor(descriptor, FEATURE_CD_MASTERING);

	if (check != True) {
		free(descriptor);
		return(E_FEATINVHEADER);
	}

	error = optcl_feature_create(FEATURE_CD_MASTERING, (optcl_feature**)&feature);

	if (FAILED(error)) {
		free(descriptor);
		return(error);
	}

	if (feature == 0) {
		free(descriptor);
		return(error);
	}

	memcpy(&feature->descriptor, descriptor, sizeof(optcl_feature_descriptor));

	free(descriptor);

	if (feature->descriptor.additional_length > 0) {
		feature->buf = bool_from_uint8(mmc_data[4] & 0x40);		/* 01000000 */
		feature->sao = bool_from_uint8(mmc_data[4] & 0x20);		/* 00100000 */
		feature->raw_ms = bool_from_uint8(mmc_data[4] & 0x10);		/* 00010000 */
		feature->raw = bool_from_uint8(mmc_data[4] & 0x08);		/* 00001000 */
		feature->test_write = bool_from_uint8(mmc_data[4] & 0x04);	/* 00000100 */
		feature->cd_rw = bool_from_uint8(mmc_data[4] & 0x02);		/* 00000010 */
		feature->rw = bool_from_uint8(mmc_data[4] & 0x01);		/* 00000001 */
		
		feature->max_cue_length = uint32_from_be_bytes(
			(uint8_t)0, 
			mmc_data[5], 
			mmc_data[6], 
			mmc_data[7]
			);
	}

	*response = (optcl_feature_descriptor*)feature;

	return(SUCCESS);
}

static RESULT parse_dvd_minus_r_minus_rw_write(const uint8_t mmc_data[],
					       uint32_t size,
					       optcl_feature_descriptor **response)
{
	RESULT error;
	bool_t check;
	optcl_feature_descriptor *descriptor = 0;
	optcl_feature_dvd_minus_r_minus_rw_write *feature = 0;

	assert(size >= 4);
	assert(mmc_data != 0);
	assert(response != 0);

	if (mmc_data == 0 || response == 0 || size < 4) {
		return(E_INVALIDARG);
	}

	error = parse_feature_descriptor(mmc_data, size, &descriptor);

	if (FAILED(error)) {
		return(error);
	}

	check = check_feature_descriptor(descriptor, FEATURE_DVD_MINUS_R_MINUS_RW_WRITE);

	if (check != True) {
		free(descriptor);
		return(E_FEATINVHEADER);
	}

	error = optcl_feature_create(FEATURE_DVD_MINUS_R_MINUS_RW_WRITE, (optcl_feature**)&feature);

	if (FAILED(error)) {
		free(descriptor);
		return(error);
	}

	if (feature == 0) {
		free(descriptor);
		return(error);
	}

	memcpy(&feature->descriptor, descriptor, sizeof(optcl_feature_descriptor));

	free(descriptor);

	if (feature->descriptor.additional_length > 0) {
		feature->buf = bool_from_uint8(mmc_data[4] & 0x40);		/* 01000000 */
		feature->rdl = bool_from_uint8(mmc_data[4] & 0x10);		/* 00010000 */
		feature->test_write = bool_from_uint8(mmc_data[4] & 0x08);	/* 00001000 */
		feature->dvd_rw	= bool_from_uint8(mmc_data[4] & 0x04);		/* 00000100 */
	}

	*response = (optcl_feature_descriptor*)feature;

	return(SUCCESS);
}

static RESULT parse_layer_jump_recording(const uint8_t mmc_data[],
					 uint32_t size,
					 optcl_feature_descriptor **response)
{
	RESULT error;
	bool_t check;
	optcl_feature_descriptor *descriptor = 0;
	optcl_feature_layer_jmp_rec *feature = 0;

	assert(size >= 4);
	assert(mmc_data != 0);
	assert(response != 0);

	if (mmc_data == 0 || response == 0 || size < 4) {
		return(E_INVALIDARG);
	}

	error = parse_feature_descriptor(mmc_data, size, &descriptor);

	if (FAILED(error)) {
		return(error);
	}

	check = check_feature_descriptor(descriptor, FEATURE_LAYER_JUMP_RECORDING);

	if (check != True) {
		free(descriptor);
		return(E_FEATINVHEADER);
	}

	error = optcl_feature_create(FEATURE_LAYER_JUMP_RECORDING, (optcl_feature**)&feature);

	if (FAILED(error)) {
		free(descriptor);
		return(error);
	}

	if (feature == 0) {
		free(descriptor);
		return(error);
	}

	memcpy(&feature->descriptor, descriptor, sizeof(optcl_feature_descriptor));

	free(descriptor);

	if (feature->descriptor.additional_length > 0) {
		feature->link_sizes_num = mmc_data[7];
	}

	if (feature->descriptor.additional_length > feature->link_sizes_num + 4) {
		memcpy(feature->link_sizes, &mmc_data[8], feature->link_sizes_num);
	}

	*response = (optcl_feature_descriptor*)feature;

	return(SUCCESS);
}

static RESULT parse_cdrw_media_write_support(const uint8_t mmc_data[],
					     uint32_t size,
					     optcl_feature_descriptor **response)
{
	RESULT error;
	bool_t check;
	optcl_feature_descriptor *descriptor = 0;
	optcl_feature_cdrw_media_write *feature = 0;

	assert(size >= 4);
	assert(mmc_data != 0);
	assert(response != 0);

	if (mmc_data == 0 || response == 0 || size < 4) {
		return(E_INVALIDARG);
	}

	error = parse_feature_descriptor(mmc_data, size, &descriptor);

	if (FAILED(error)) {
		return(error);
	}

	check = check_feature_descriptor(descriptor, FEATURE_CDRW_MEDIA_WRITE_SUPPORT);

	if (check != True) {
		free(descriptor);
		return(E_FEATINVHEADER);
	}

	error = optcl_feature_create(FEATURE_CDRW_MEDIA_WRITE_SUPPORT, (optcl_feature**)&feature);

	if (FAILED(error)) {
		free(descriptor);
		return(error);
	}

	if (feature == 0) {
		free(descriptor);
		return(error);
	}

	memcpy(&feature->descriptor, descriptor, sizeof(optcl_feature_descriptor));

	free(descriptor);

	if (feature->descriptor.additional_length > 0) {
		feature->subtype7 = bool_from_uint8(mmc_data[5] & 0x80);	/* 10000000 */
		feature->subtype6 = bool_from_uint8(mmc_data[5] & 0x40);	/* 01000000 */
		feature->subtype5 = bool_from_uint8(mmc_data[5] & 0x20);	/* 00100000 */
		feature->subtype4 = bool_from_uint8(mmc_data[5] & 0x10);	/* 00010000 */
		feature->subtype3 = bool_from_uint8(mmc_data[5] & 0x08);	/* 00001000 */
		feature->subtype2 = bool_from_uint8(mmc_data[5] & 0x04);	/* 00000100 */
		feature->subtype1 = bool_from_uint8(mmc_data[5] & 0x02);	/* 00000010 */
		feature->subtype0 = bool_from_uint8(mmc_data[5] & 0x01);	/* 00000001 */
	}

	*response = (optcl_feature_descriptor*)feature;

	return(SUCCESS);
}

static RESULT parse_bdr_pow(const uint8_t mmc_data[],
			    uint32_t size,
			    optcl_feature_descriptor **response)
{
	RESULT error;
	bool_t check;
	optcl_feature_descriptor *descriptor = 0;
	optcl_feature_bdr_pow *feature = 0;

	assert(size >= 4);
	assert(mmc_data != 0);
	assert(response != 0);

	if (mmc_data == 0 || response == 0 || size < 4) {
		return(E_INVALIDARG);
	}

	error = parse_feature_descriptor(mmc_data, size, &descriptor);

	if (FAILED(error)) {
		return(error);
	}

	check = check_feature_descriptor(descriptor, FEATURE_BDR_POW);

	if (check != True) {
		free(descriptor);
		return(E_FEATINVHEADER);
	}

	error = optcl_feature_create(FEATURE_BDR_POW, (optcl_feature**)&feature);

	if (FAILED(error)) {
		free(descriptor);
		return(error);
	}

	if (feature == 0) {
		free(descriptor);
		return(error);
	}

	memcpy(&feature->descriptor, descriptor, sizeof(optcl_feature_descriptor));

	free(descriptor);

	*response = (optcl_feature_descriptor*)feature;

	return(SUCCESS);
}

static RESULT parse_dvd_plus_rw_dual_layer(const uint8_t mmc_data[],
					   uint32_t size,
					   optcl_feature_descriptor **response)
{
	RESULT error;
	bool_t check;
	optcl_feature_descriptor *descriptor = 0;
	optcl_feature_dvd_plus_rw_dual_layer *feature = 0;

	assert(size >= 4);
	assert(mmc_data != 0);
	assert(response != 0);

	if (mmc_data == 0 || response == 0 || size < 4) {
		return(E_INVALIDARG);
	}

	error = parse_feature_descriptor(mmc_data, size, &descriptor);

	if (FAILED(error)) {
		return(error);
	}

	check = check_feature_descriptor(descriptor, FEATURE_DVD_PLUS_RW_DUAL_LAYER);

	if (check != True) {
		free(descriptor);
		return(E_FEATINVHEADER);
	}

	error = optcl_feature_create(FEATURE_DVD_PLUS_RW_DUAL_LAYER, (optcl_feature**)&feature);

	if (FAILED(error)) {
		free(descriptor);
		return(error);
	}

	if (feature == 0) {
		free(descriptor);
		return(error);
	}

	memcpy(&feature->descriptor, descriptor, sizeof(optcl_feature_descriptor));

	free(descriptor);

	if (feature->descriptor.additional_length > 0) {
		feature->write = bool_from_uint8(mmc_data[4] & 0x01);		/* 00000001 */
		feature->quick_start = bool_from_uint8(mmc_data[5] & 0x02);	/* 00000010 */
		feature->close_only = bool_from_uint8(mmc_data[5] & 0x01);	/* 00000001 */
	}

	*response = (optcl_feature_descriptor*)feature;

	return(SUCCESS);
}

static RESULT parse_dvd_plus_r_dual_layer(const uint8_t mmc_data[],
					  uint32_t size,
					  optcl_feature_descriptor **response)
{
	RESULT error;
	bool_t check;
	optcl_feature_descriptor *descriptor = 0;
	optcl_feature_dvd_r_plus_dual_layer *feature = 0;

	assert(size >= 4);
	assert(mmc_data != 0);
	assert(response != 0);

	if (mmc_data == 0 || response == 0 || size < 4) {
		return(E_INVALIDARG);
	}

	error = parse_feature_descriptor(mmc_data, size, &descriptor);

	if (FAILED(error)) {
		return(error);
	}

	check = check_feature_descriptor(descriptor, FEATURE_DVD_PLUS_R_DUAL_LAYER);

	if (check != True) {
		free(descriptor);
		return(E_FEATINVHEADER);
	}

	error = optcl_feature_create(FEATURE_DVD_PLUS_R_DUAL_LAYER, (optcl_feature**)&feature);

	if (FAILED(error)) {
		free(descriptor);
		return(error);
	}

	if (feature == 0) {
		free(descriptor);
		return(error);
	}

	memcpy(&feature->descriptor, descriptor, sizeof(optcl_feature_descriptor));

	free(descriptor);

	if (feature->descriptor.additional_length > 0) {
		feature->write = bool_from_uint8(mmc_data[4] & 0x01);	/* 00000001 */
	}

	*response = (optcl_feature_descriptor*)feature;

	return(SUCCESS);
}

static RESULT parse_bd_read(const uint8_t mmc_data[],
			    uint32_t size,
			    optcl_feature_descriptor **response)
{
	RESULT error;
	bool_t check;
	optcl_feature_descriptor *descriptor = 0;
	optcl_feature_bd_read *feature = 0;

	assert(size >= 4);
	assert(mmc_data != 0);
	assert(response != 0);

	if (mmc_data == 0 || response == 0 || size < 4) {
		return(E_INVALIDARG);
	}

	error = parse_feature_descriptor(mmc_data, size, &descriptor);

	if (FAILED(error)) {
		return(error);
	}

	check = check_feature_descriptor(descriptor, FEATURE_BD_READ);

	if (check != True) {
		free(descriptor);
		return(E_FEATINVHEADER);
	}

	error = optcl_feature_create(FEATURE_BD_READ, (optcl_feature**)&feature);

	if (FAILED(error)) {
		free(descriptor);
		return(error);
	}

	if (feature == 0) {
		free(descriptor);
		return(error);
	}

	memcpy(&feature->descriptor, descriptor, sizeof(optcl_feature_descriptor));

	free(descriptor);

	if (feature->descriptor.additional_length > 4) {
		feature->bd_re_class0_bitmap = uint16_from_be(*(uint16_t*)&mmc_data[8]);
		feature->bd_re_class1_bitmap = uint16_from_be(*(uint16_t*)&mmc_data[10]);
	}

	if (feature->descriptor.additional_length > 8) {
		feature->bd_re_class2_bitmap = uint16_from_be(*(uint16_t*)&mmc_data[12]);
		feature->bd_re_class3_bitmap = uint16_from_be(*(uint16_t*)&mmc_data[14]);
	}

	if (feature->descriptor.additional_length > 12) {
		feature->bd_r_class0_bitmap = uint16_from_be(*(uint16_t*)&mmc_data[16]);
		feature->bd_r_class1_bitmap = uint16_from_be(*(uint16_t*)&mmc_data[18]);
	}

	if (feature->descriptor.additional_length > 16) {
		feature->bd_r_class2_bitmap = uint16_from_be(*(uint16_t*)&mmc_data[20]);
		feature->bd_r_class3_bitmap = uint16_from_be(*(uint16_t*)&mmc_data[22]);
	}

	if (feature->descriptor.additional_length > 20) {
		feature->bd_rom_class0_bitmap = uint16_from_be(*(uint16_t*)&mmc_data[24]);
		feature->bd_rom_class1_bitmap = uint16_from_be(*(uint16_t*)&mmc_data[26]);
	}

	if (feature->descriptor.additional_length > 24) {
		feature->bd_rom_class2_bitmap = uint16_from_be(*(uint16_t*)&mmc_data[28]);
		feature->bd_rom_class3_bitmap = uint16_from_be(*(uint16_t*)&mmc_data[30]);
	}

	*response = (optcl_feature_descriptor*)feature;

	return(SUCCESS);
}

static RESULT parse_bd_write(const uint8_t mmc_data[],
			     uint32_t size,
			     optcl_feature_descriptor **response)
{
	RESULT error;
	bool_t check;
	optcl_feature_descriptor *descriptor = 0;
	optcl_feature_bd_write *feature = 0;

	assert(size >= 4);
	assert(mmc_data != 0);
	assert(response != 0);

	if (mmc_data == 0 || response == 0 || size < 4) {
		return(E_INVALIDARG);
	}

	error = parse_feature_descriptor(mmc_data, size, &descriptor);

	if (FAILED(error)) {
		return(error);
	}

	check = check_feature_descriptor(descriptor, FEATURE_BD_WRITE);

	if (check != True) {
		free(descriptor);
		return(E_FEATINVHEADER);
	}

	error = optcl_feature_create(FEATURE_BD_WRITE, (optcl_feature**)&feature);

	if (FAILED(error)) {
		free(descriptor);
		return(error);
	}

	if (feature == 0) {
		free(descriptor);
		return(error);
	}

	memcpy(&feature->descriptor, descriptor, sizeof(optcl_feature_descriptor));

	free(descriptor);

	if (feature->descriptor.additional_length > 0) {
		feature->svnr = bool_from_uint8(mmc_data[4] & 0x01);	/* 00000001 */
	}

	if (feature->descriptor.additional_length > 4) {
		feature->bd_re_class0_bitmap = uint16_from_be(*(uint16_t*)&mmc_data[8]);
		feature->bd_re_class1_bitmap = uint16_from_be(*(uint16_t*)&mmc_data[10]);
	}

	if (feature->descriptor.additional_length > 8) {
		feature->bd_re_class2_bitmap = uint16_from_be(*(uint16_t*)&mmc_data[12]);
		feature->bd_re_class3_bitmap = uint16_from_be(*(uint16_t*)&mmc_data[14]);
	}

	if (feature->descriptor.additional_length > 12) {
		feature->bd_r_class0_bitmap = uint16_from_be(*(uint16_t*)&mmc_data[16]);
		feature->bd_r_class1_bitmap = uint16_from_be(*(uint16_t*)&mmc_data[18]);
	}

	if (feature->descriptor.additional_length > 16) {
		feature->bd_r_class2_bitmap = uint16_from_be(*(uint16_t*)&mmc_data[20]);
		feature->bd_r_class3_bitmap = uint16_from_be(*(uint16_t*)&mmc_data[22]);
	}

	*response = (optcl_feature_descriptor*)feature;

	return(SUCCESS);
}

static RESULT parse_tsr(const uint8_t mmc_data[],
			uint32_t size,
			optcl_feature_descriptor **response)
{
	RESULT error;
	bool_t check;
	optcl_feature_descriptor *descriptor = 0;
	optcl_feature_tsr *feature = 0;

	assert(size >= 4);
	assert(mmc_data != 0);
	assert(response != 0);

	if (mmc_data == 0 || response == 0 || size < 4) {
		return(E_INVALIDARG);
	}

	error = parse_feature_descriptor(mmc_data, size, &descriptor);

	if (FAILED(error)) {
		return(error);
	}

	check = check_feature_descriptor(descriptor, FEATURE_TSR);

	if (check != True) {
		free(descriptor);
		return(E_FEATINVHEADER);
	}

	error = optcl_feature_create(FEATURE_TSR, (optcl_feature**)&feature);

	if (FAILED(error)) {
		free(descriptor);
		return(error);
	}

	if (feature == 0) {
		free(descriptor);
		return(error);
	}

	memcpy(&feature->descriptor, descriptor, sizeof(optcl_feature_descriptor));

	free(descriptor);

	*response = (optcl_feature_descriptor*)feature;

	return(SUCCESS);
}

static RESULT parse_hd_dvd_read(const uint8_t mmc_data[],
				uint32_t size,
				optcl_feature_descriptor **response)
{
	RESULT error;
	bool_t check;
	optcl_feature_descriptor *descriptor = 0;
	optcl_feature_hd_dvd_read *feature = 0;

	assert(size >= 4);
	assert(mmc_data != 0);
	assert(response != 0);

	if (mmc_data == 0 || response == 0 || size < 4) {
		return(E_INVALIDARG);
	}

	error = parse_feature_descriptor(mmc_data, size, &descriptor);

	if (FAILED(error)) {
		return(error);
	}

	check = check_feature_descriptor(descriptor, FEATURE_HD_DVD_READ);

	if (check != True) {
		free(descriptor);
		return(E_FEATINVHEADER);
	}

	error = optcl_feature_create(FEATURE_HD_DVD_READ, (optcl_feature**)&feature);

	if (FAILED(error)) {
		free(descriptor);
		return(error);
	}

	if (feature == 0) {
		free(descriptor);
		return(error);
	}

	memcpy(&feature->descriptor, descriptor, sizeof(optcl_feature_descriptor));

	free(descriptor);

	if (feature->descriptor.additional_length > 0) {
		feature->hd_dvd_r = bool_from_uint8(mmc_data[4] & 0x01);	/* 00000001 */
		feature->hd_dvd_ram = bool_from_uint8(mmc_data[6] & 0x01);	/* 00000001 */
	}

	*response = (optcl_feature_descriptor*)feature;

	return(SUCCESS);
}

static RESULT parse_hd_dvd_write(const uint8_t mmc_data[],
				 uint32_t size,
				 optcl_feature_descriptor **response)
{
	RESULT error;
	bool_t check;
	optcl_feature_descriptor *descriptor = 0;
	optcl_feature_hd_dvd_write *feature = 0;

	assert(size >= 4);
	assert(mmc_data != 0);
	assert(response != 0);

	if (mmc_data == 0 || response == 0 || size < 4) {
		return(E_INVALIDARG);
	}

	error = parse_feature_descriptor(mmc_data, size, &descriptor);

	if (FAILED(error)) {
		return(error);
	}

	check = check_feature_descriptor(descriptor, FEATURE_HD_DVD_WRITE);

	if (check != True) {
		free(descriptor);
		return(E_FEATINVHEADER);
	}

	error = optcl_feature_create(FEATURE_HD_DVD_WRITE, (optcl_feature**)&feature);

	if (FAILED(error)) {
		free(descriptor);
		return(error);
	}

	if (feature == 0) {
		free(descriptor);
		return(error);
	}

	memcpy(&feature->descriptor, descriptor, sizeof(optcl_feature_descriptor));

	free(descriptor);

	if (feature->descriptor.additional_length > 0) {
		feature->hd_dvd_r = bool_from_uint8(mmc_data[4] & 0x01);	/* 00000001 */
		feature->hd_dvd_ram = bool_from_uint8(mmc_data[6] & 0x01);	/* 00000001 */
	}

	*response = (optcl_feature_descriptor*)feature;

	return(SUCCESS);
}

static RESULT parse_hybrid_disk(const uint8_t mmc_data[],
				uint32_t size,
				optcl_feature_descriptor **response)
{
	RESULT error;
	bool_t check;
	optcl_feature_descriptor *descriptor = 0;
	optcl_feature_hybrid_disk *feature = 0;

	assert(size >= 4);
	assert(mmc_data != 0);
	assert(response != 0);

	if (mmc_data == 0 || response == 0 || size < 4) {
		return(E_INVALIDARG);
	}

	error = parse_feature_descriptor(mmc_data, size, &descriptor);

	if (FAILED(error)) {
		return(error);
	}

	check = check_feature_descriptor(descriptor, FEATURE_HYBRID_DISC);

	if (check != True) {
		free(descriptor);
		return(E_FEATINVHEADER);
	}

	error = optcl_feature_create(FEATURE_HYBRID_DISC, (optcl_feature**)&feature);

	if (FAILED(error)) {
		free(descriptor);
		return(error);
	}

	if (feature == 0) {
		free(descriptor);
		return(error);
	}

	memcpy(&feature->descriptor, descriptor, sizeof(optcl_feature_descriptor));

	free(descriptor);

	if (feature->descriptor.additional_length > 0) {
		feature->ri = bool_from_uint8(mmc_data[4] & 0x01);	/* 00000001 */
	}

	*response = (optcl_feature_descriptor*)feature;

	return(SUCCESS);
}

static RESULT parse_power_management(const uint8_t mmc_data[],
				     uint32_t size,
				     optcl_feature_descriptor **response)
{
	RESULT error;
	bool_t check;
	optcl_feature_descriptor *descriptor = 0;
	optcl_feature_power_mngmnt *feature = 0;

	assert(size >= 4);
	assert(mmc_data != 0);
	assert(response != 0);

	if (mmc_data == 0 || response == 0 || size < 4) {
		return(E_INVALIDARG);
	}

	error = parse_feature_descriptor(mmc_data, size, &descriptor);

	if (FAILED(error)) {
		return(error);
	}

	check = check_feature_descriptor(descriptor, FEATURE_POWER_MANAGEMENT);

	if (check != True) {
		free(descriptor);
		return(E_FEATINVHEADER);
	}

	error = optcl_feature_create(FEATURE_POWER_MANAGEMENT, (optcl_feature**)&feature);

	if (FAILED(error)) {
		free(descriptor);
		return(error);
	}

	if (feature == 0) {
		free(descriptor);
		return(error);
	}

	memcpy(&feature->descriptor, descriptor, sizeof(optcl_feature_descriptor));

	free(descriptor);

	*response = (optcl_feature_descriptor*)feature;

	return(SUCCESS);
}

static RESULT parse_smart(const uint8_t mmc_data[],
			  uint32_t size,
			  optcl_feature_descriptor **response)
{
	RESULT error;
	bool_t check;
	optcl_feature_descriptor *descriptor = 0;
	optcl_feature_smart *feature = 0;

	assert(size >= 4);
	assert(mmc_data != 0);
	assert(response != 0);

	if (mmc_data == 0 || response == 0 || size < 4) {
		return(E_INVALIDARG);
	}

	error = parse_feature_descriptor(mmc_data, size, &descriptor);

	if (FAILED(error)) {
		return(error);
	}

	check = check_feature_descriptor(descriptor, FEATURE_SMART);

	if (check != True) {
		free(descriptor);
		return(E_FEATINVHEADER);
	}

	error = optcl_feature_create(FEATURE_SMART, (optcl_feature**)&feature);

	if (FAILED(error)) {
		free(descriptor);
		return(error);
	}

	if (feature == 0) {
		free(descriptor);
		return(error);
	}

	memcpy(&feature->descriptor, descriptor, sizeof(optcl_feature_descriptor));

	free(descriptor);

	if (feature->descriptor.additional_length > 0) {
		feature->pp = bool_from_uint8(mmc_data[4] & 0x01);	/* 00000001 */
	}

	*response = (optcl_feature_descriptor*)feature;

	return(SUCCESS);
}

static RESULT parse_embedded_changer(const uint8_t mmc_data[],
				     uint32_t size,
				     optcl_feature_descriptor **response)
{
	RESULT error;
	bool_t check;
	optcl_feature_descriptor *descriptor = 0;
	optcl_feature_embedded_changer *feature = 0;

	assert(size >= 4);
	assert(mmc_data != 0);
	assert(response != 0);

	if (mmc_data == 0 || response == 0 || size < 4) {
		return(E_INVALIDARG);
	}

	error = parse_feature_descriptor(mmc_data, size, &descriptor);

	if (FAILED(error)) {
		return(error);
	}

	check = check_feature_descriptor(descriptor, FEATURE_EMBEDDED_CHANGER);

	if (check != True) {
		free(descriptor);
		return(E_FEATINVHEADER);
	}

	error = optcl_feature_create(FEATURE_EMBEDDED_CHANGER, (optcl_feature**)&feature);

	if (FAILED(error)) {
		free(descriptor);
		return(error);
	}

	if (feature == 0) {
		free(descriptor);
		return(error);
	}

	memcpy(&feature->descriptor, descriptor, sizeof(optcl_feature_descriptor));

	free(descriptor);

	if (feature->descriptor.additional_length > 0) {
		feature->scc = bool_from_uint8(mmc_data[4] & 0x10);	/* 00010000 */
		feature->sdp = bool_from_uint8(mmc_data[4] & 0x04);	/* 00000100 */
		feature->highest_slot_num = mmc_data[7] & 0x1F;		/* 00001111 */
	}

	*response = (optcl_feature_descriptor*)feature;

	return(SUCCESS);
}

static RESULT parse_microcode_upgrade(const uint8_t mmc_data[],
				      uint32_t size,
				      optcl_feature_descriptor **response)
{
	RESULT error;
	bool_t check;
	optcl_feature_descriptor *descriptor = 0;
	optcl_feature_microcode_upgrade *feature = 0;

	assert(size >= 4);
	assert(mmc_data != 0);
	assert(response != 0);

	if (mmc_data == 0 || response == 0 || size < 4) {
		return(E_INVALIDARG);
	}

	error = parse_feature_descriptor(mmc_data, size, &descriptor);

	if (FAILED(error)) {
		return(error);
	}

	check = check_feature_descriptor(descriptor, FEATURE_MICROCODE_UPGRADE);

	if (check != True) {
		free(descriptor);
		return(E_FEATINVHEADER);
	}

	error = optcl_feature_create(FEATURE_MICROCODE_UPGRADE, (optcl_feature**)&feature);

	if (FAILED(error)) {
		free(descriptor);
		return(error);
	}

	if (feature == 0) {
		free(descriptor);
		return(error);
	}

	memcpy(&feature->descriptor, descriptor, sizeof(optcl_feature_descriptor));

	free(descriptor);

	if (feature->descriptor.additional_length > 0) {
		feature->m5 = bool_from_uint8(mmc_data[4] & 0x01);	/* 00000001 */
	}

	*response = (optcl_feature_descriptor*)feature;

	return(SUCCESS);
}

static RESULT parse_timeout(const uint8_t mmc_data[],
			    uint32_t size,
			    optcl_feature_descriptor **response)
{
	RESULT error;
	bool_t check;
	optcl_feature_descriptor *descriptor = 0;
	optcl_feature_timeout *feature = 0;

	assert(size >= 4);
	assert(mmc_data != 0);
	assert(response != 0);

	if (mmc_data == 0 || response == 0 || size < 4) {
		return(E_INVALIDARG);
	}

	error = parse_feature_descriptor(mmc_data, size, &descriptor);

	if (FAILED(error)) {
		return(error);
	}

	check = check_feature_descriptor(descriptor, FEATURE_TIMEOUT);

	if (check != True) {
		free(descriptor);
		return(E_FEATINVHEADER);
	}

	error = optcl_feature_create(FEATURE_TIMEOUT, (optcl_feature**)&feature);

	if (FAILED(error)) {
		free(descriptor);
		return(error);
	}

	if (feature == 0) {
		free(descriptor);
		return(error);
	}

	memcpy(&feature->descriptor, descriptor, sizeof(optcl_feature_descriptor));

	free(descriptor);

	if (feature->descriptor.additional_length > 0) {
		feature->group3	= bool_from_uint8(mmc_data[4] & 0x01);	/* 00000001 */
		feature->unit_length = uint16_from_be(*(uint16_t*)&mmc_data[6]);
	}

	*response = (optcl_feature_descriptor*)feature;

	return(SUCCESS);
}

static RESULT parse_dvd_css(const uint8_t mmc_data[],
			    uint32_t size,
			    optcl_feature_descriptor **response)
{
	RESULT error;
	bool_t check;
	optcl_feature_descriptor *descriptor = 0;
	optcl_feature_dvd_css *feature = 0;

	assert(size >= 4);
	assert(mmc_data != 0);
	assert(response != 0);

	if (mmc_data == 0 || response == 0 || size < 4) {
		return(E_INVALIDARG);
	}

	error = parse_feature_descriptor(mmc_data, size, &descriptor);

	if (FAILED(error)) {
		return(error);
	}

	check = check_feature_descriptor(descriptor, FEATURE_DVD_CSS);

	if (check != True) {
		free(descriptor);
		return(E_FEATINVHEADER);
	}

	error = optcl_feature_create(FEATURE_DVD_CSS, (optcl_feature**)&feature);

	if (FAILED(error)) {
		free(descriptor);
		return(error);
	}

	if (feature == 0) {
		free(descriptor);
		return(error);
	}

	memcpy(&feature->descriptor, descriptor, sizeof(optcl_feature_descriptor));

	free(descriptor);

	if (feature->descriptor.additional_length > 0) {
		feature->css_version = mmc_data[7];
	}

	*response = (optcl_feature_descriptor*)feature;

	return(SUCCESS);
}

static RESULT parse_rt_streaming(const uint8_t mmc_data[],
				 uint32_t size,
				 optcl_feature_descriptor **response)
{
	RESULT error;
	bool_t check;
	optcl_feature_descriptor *descriptor = 0;
	optcl_feature_rt_streaming *feature = 0;

	assert(size >= 4);
	assert(mmc_data != 0);
	assert(response != 0);

	if (mmc_data == 0 || response == 0 || size < 4) {
		return(E_INVALIDARG);
	}

	error = parse_feature_descriptor(mmc_data, size, &descriptor);

	if (FAILED(error)) {
		return(error);
	}

	check = check_feature_descriptor(descriptor, FEATURE_RT_STREAMING);

	if (check != True) {
		free(descriptor);
		return(E_FEATINVHEADER);
	}

	error = optcl_feature_create(FEATURE_RT_STREAMING, (optcl_feature**)&feature);

	if (FAILED(error)) {
		free(descriptor);
		return(error);
	}

	if (feature == 0) {
		free(descriptor);
		return(error);
	}

	memcpy(&feature->descriptor, descriptor, sizeof(optcl_feature_descriptor));

	free(descriptor);

	if (feature->descriptor.additional_length > 0) {
		feature->rbcb = bool_from_uint8(mmc_data[4] & 0x10);	/* 00010000 */
		feature->scs = bool_from_uint8(mmc_data[4] & 0x08);	/* 00001000 */
		feature->mp2a = bool_from_uint8(mmc_data[4] & 0x04);	/* 00000100 */
		feature->wspd = bool_from_uint8(mmc_data[4] & 0x02);	/* 00000010 */
		feature->sw = bool_from_uint8(mmc_data[4] & 0x01);	/* 00000001 */
	}

	*response = (optcl_feature_descriptor*)feature;

	return(SUCCESS);
}

static RESULT parse_drive_serial_number(const uint8_t mmc_data[],
					uint32_t size,
					optcl_feature_descriptor **response)
{
	RESULT error;
	bool_t check;
	uint32_t length;
	optcl_feature_descriptor *descriptor = 0;
	optcl_feature_drive_serial_number *feature = 0;

	assert(size >= 4);
	assert(mmc_data != 0);
	assert(response != 0);

	if (mmc_data == 0 || response == 0 || size < 4) {
		return(E_INVALIDARG);
	}

	error = parse_feature_descriptor(mmc_data, size, &descriptor);

	if (FAILED(error)) {
		return(error);
	}

	check = check_feature_descriptor(descriptor, FEATURE_DRIVE_SERIAL_NUMBER);

	if (check != True) {
		free(descriptor);
		return(E_FEATINVHEADER);
	}

	error = optcl_feature_create(FEATURE_DRIVE_SERIAL_NUMBER, (optcl_feature**)&feature);

	if (FAILED(error)) {
		free(descriptor);
		return(error);
	}

	if (feature == 0) {
		free(descriptor);
		return(error);
	}

	memcpy(&feature->descriptor, descriptor, sizeof(optcl_feature_descriptor));

	free(descriptor);

	if (feature->descriptor.additional_length > 0) {
		length = (feature->descriptor.additional_length < sizeof(feature->serial_number))
			? feature->descriptor.additional_length
			: sizeof(feature->serial_number);

		strncpy_s(
			(char*)&feature->serial_number,
			sizeof(feature->serial_number),
			(const char*)&mmc_data[4], 
			length
			);
	}

	*response = (optcl_feature_descriptor*)feature;

	return(SUCCESS);
}

static RESULT parse_media_serial_number(const uint8_t mmc_data[],
					uint32_t size,
					optcl_feature_descriptor **response)
{
	RESULT error;
	bool_t check;
	optcl_feature_descriptor *descriptor = 0;
	optcl_feature_media_serial_number *feature = 0;

	assert(size >= 4);
	assert(mmc_data != 0);
	assert(response != 0);

	if (mmc_data == 0 || response == 0 || size < 4) {
		return(E_INVALIDARG);
	}

	error = parse_feature_descriptor(mmc_data, size, &descriptor);

	if (FAILED(error)) {
		return(error);
	}

	check = check_feature_descriptor(descriptor, FEATURE_MEDIA_SERIAL_NUMBER);

	if (check != True) {
		free(descriptor);
		return(E_FEATINVHEADER);
	}

	error = optcl_feature_create(FEATURE_MEDIA_SERIAL_NUMBER, (optcl_feature**)&feature);

	if (FAILED(error)) {
		free(descriptor);
		return(error);
	}

	if (feature == 0) {
		free(descriptor);
		return(error);
	}

	memcpy(&feature->descriptor, descriptor, sizeof(optcl_feature_descriptor));

	free(descriptor);

	*response = (optcl_feature_descriptor*)feature;

	return(SUCCESS);
}

static RESULT parse_dcbs(const uint8_t mmc_data[],
			 uint32_t size,
			 optcl_feature_descriptor **response)
{
	int i;
	RESULT error;
	bool_t check;
	optcl_feature_descriptor *descriptor = 0;
	optcl_feature_dcbs *feature = 0;

	assert(size >= 4);
	assert(mmc_data != 0);
	assert(response != 0);

	if (mmc_data == 0 || response == 0 || size < 4) {
		return(E_INVALIDARG);
	}

	error = parse_feature_descriptor(mmc_data, size, &descriptor);

	if (FAILED(error)) {
		return(error);
	}

	check = check_feature_descriptor(descriptor, FEATURE_DCBS);

	if (check != True) {
		free(descriptor);
		return(E_FEATINVHEADER);
	}

	error = optcl_feature_create(FEATURE_DCBS, (optcl_feature**)&feature);

	if (FAILED(error)) {
		free(descriptor);
		return(error);
	}

	if (feature == 0) {
		free(descriptor);
		return(error);
	}

	memcpy(&feature->descriptor, descriptor, sizeof(optcl_feature_descriptor));

	free(descriptor);

	feature->dcb_entries_num = feature->descriptor.additional_length / 4;

	if (feature->descriptor.additional_length >= feature->dcb_entries_num * 4) {
		for(i = 0; i < feature->dcb_entries_num; ++i) {
			feature->dcb_entries[i] = uint32_from_be(*(uint32_t*)&mmc_data[(i + 1) * 4]);
		}
	}

	*response = (optcl_feature_descriptor*)feature;

	return(SUCCESS);
}

static RESULT parse_dvd_cprm(const uint8_t mmc_data[],
			     uint32_t size,
			     optcl_feature_descriptor **response)
{
	RESULT error;
	bool_t check;
	optcl_feature_descriptor *descriptor = 0;
	optcl_feature_dvd_cprm *feature = 0;

	assert(size >= 4);
	assert(mmc_data != 0);
	assert(response != 0);

	if (mmc_data == 0 || response == 0 || size < 4) {
		return(E_INVALIDARG);
	}

	error = parse_feature_descriptor(mmc_data, size, &descriptor);

	if (FAILED(error)) {
		return(error);
	}

	check = check_feature_descriptor(descriptor, FEATURE_DVD_CPRM);

	if (check != True) {
		free(descriptor);
		return(E_FEATINVHEADER);
	}

	error = optcl_feature_create(FEATURE_DVD_CPRM, (optcl_feature**)&feature);

	if (FAILED(error)) {
		free(descriptor);
		return(error);
	}

	if (feature == 0) {
		free(descriptor);
		return(error);
	}

	memcpy(&feature->descriptor, descriptor, sizeof(optcl_feature_descriptor));

	free(descriptor);

	if (feature->descriptor.additional_length > 4) {
		feature->cprm_version = mmc_data[7];
	}

	*response = (optcl_feature_descriptor*)feature;

	return(SUCCESS);
}

static RESULT parse_firmware_info(const uint8_t mmc_data[],
				  uint32_t size,
				  optcl_feature_descriptor **response)
{
	RESULT error;
	bool_t check;
	optcl_feature_descriptor *descriptor = 0;
	optcl_feature_firmware_info *feature = 0;

	assert(size >= 4);
	assert(mmc_data != 0);
	assert(response != 0);

	if (mmc_data == 0 || response == 0 || size < 4) {
		return(E_INVALIDARG);
	}

	error = parse_feature_descriptor(mmc_data, size, &descriptor);

	if (FAILED(error)) {
		return(error);
	}

	check = check_feature_descriptor(descriptor, FEATURE_FIRMWARE_INFO);

	if (check != True) {
		free(descriptor);
		return(E_FEATINVHEADER);
	}

	error = optcl_feature_create(FEATURE_FIRMWARE_INFO, (optcl_feature**)&feature);

	if (FAILED(error)) {
		free(descriptor);
		return(error);
	}

	if (feature == 0) {
		free(descriptor);
		return(error);
	}

	memcpy(&feature->descriptor, descriptor, sizeof(optcl_feature_descriptor));

	free(descriptor);

	if (feature->descriptor.additional_length > 0) {
		feature->century = uint16_from_be(*(uint16_t*)&mmc_data[4]);
		feature->year = uint16_from_be(*(uint16_t*)&mmc_data[6]);
	}

	if (feature->descriptor.additional_length > 4) {
		feature->month = uint16_from_be(*(uint16_t*)&mmc_data[8]);
		feature->day = uint16_from_be(*(uint16_t*)&mmc_data[10]);
	}

	if (feature->descriptor.additional_length > 8) {
		feature->hour = uint16_from_be(*(uint16_t*)&mmc_data[12]);
		feature->minute	= uint16_from_be(*(uint16_t*)&mmc_data[14]);
	}

	if (feature->descriptor.additional_length > 12) {
		feature->second	= uint16_from_be(*(uint16_t*)&mmc_data[16]);
	}

	*response = (optcl_feature_descriptor*)feature;

	return(SUCCESS);
}

static RESULT parse_aacs(const uint8_t mmc_data[],
			 uint32_t size,
			 optcl_feature_descriptor **response)
{
	RESULT error;
	bool_t check;
	optcl_feature_descriptor *descriptor = 0;
	optcl_feature_aacs *feature = 0;

	assert(size >= 4);
	assert(mmc_data != 0);
	assert(response != 0);

	if (mmc_data == 0 || response == 0 || size < 4) {
		return(E_INVALIDARG);
	}

	error = parse_feature_descriptor(mmc_data, size, &descriptor);

	if (FAILED(error)) {
		return(error);
	}

	check = check_feature_descriptor(descriptor, FEATURE_AACS);

	if (check != True) {
		free(descriptor);
		return(E_FEATINVHEADER);
	}

	error = optcl_feature_create(FEATURE_AACS, (optcl_feature**)&feature);

	if (FAILED(error)) {
		free(descriptor);
		return(error);
	}

	if (feature == 0) {
		free(descriptor);
		return(error);
	}

	memcpy(&feature->descriptor, descriptor, sizeof(optcl_feature_descriptor));

	free(descriptor);

	if (feature->descriptor.additional_length > 0) {
		feature->bng = bool_from_uint8(mmc_data[4] & 0x01);	/* 00000001 */
		feature->block_count = mmc_data[5];
		feature->agids_num = mmc_data[6] & 0x0F;		/* 00001111 */
		feature->aacs_version = mmc_data[7];
	}

	*response = (optcl_feature_descriptor*)feature;

	return(SUCCESS);
}

static RESULT parse_vcps(const uint8_t mmc_data[],
			 uint32_t size,
			 optcl_feature_descriptor **response)
{
	RESULT error;
	bool_t check;
	optcl_feature_descriptor *descriptor = 0;
	optcl_feature_vcps *feature = 0;

	assert(size >= 4);
	assert(mmc_data != 0);
	assert(response != 0);

	if (mmc_data == 0 || response == 0 || size < 4) {
		return(E_INVALIDARG);
	}

	error = parse_feature_descriptor(mmc_data, size, &descriptor);

	if (FAILED(error)) {
		return(error);
	}

	check = check_feature_descriptor(descriptor, FEATURE_VCPS);

	if (check != True) {
		free(descriptor);
		return(E_FEATINVHEADER);
	}

	error = optcl_feature_create(FEATURE_VCPS, (optcl_feature**)&feature);

	if (FAILED(error)) {
		free(descriptor);
		return(error);
	}

	if (feature == 0) {
		free(descriptor);
		return(error);
	}

	memcpy(&feature->descriptor, descriptor, sizeof(optcl_feature_descriptor));

	free(descriptor);

	*response = (optcl_feature_descriptor*)feature;

	return(SUCCESS);
}


/*
 * Feature functions
 */

RESULT optcl_feature_copy(optcl_feature **dest, const optcl_feature *src)
{
	int size;

	assert(dest != 0);
	assert(src != 0);

	if (dest == 0 || src == 0) {
		return(E_INVALIDARG);
	}

	size = get_feature_size(src->feature_code);

	if (size <= 0) {
		return(E_DEVUNKNFEATURE);
	}

	free(*dest);

	*dest = malloc(size);

	if (*dest == 0) {
		return(E_OUTOFMEMORY);
	}

	memcpy(*dest, src, size);

	return(SUCCESS);
}

RESULT optcl_feature_create(uint16_t feature_code, optcl_feature **feature)
{
	int size;
	optcl_feature *nfeature = 0;

	assert(feature != 0);

	if (feature == 0) {
		return(E_INVALIDARG);
	}

	size = get_feature_size(feature_code);

	if (size <= 0) {
		return(E_DEVUNKNFEATURE);
	}

	nfeature = malloc(size);

	if (nfeature == 0) {
		return(E_OUTOFMEMORY);
	}

	memset(nfeature, 0, size);

	nfeature->feature_code = feature_code;

	*feature = nfeature;

	return(SUCCESS);
}

RESULT optcl_feature_create_from_raw(optcl_feature **feature,
				     const uint8_t mmc_data[],
				     uint32_t size)
{
	RESULT error;
	optcl_feature *nfeature = 0;
	raw_feature_parser parser = 0;

	assert(feature != 0);
	assert(mmc_data != 0);

	if (feature == 0 || mmc_data == 0) {
		return(E_INVALIDARG);
	}

	error = parse_feature_descriptor(mmc_data, size, &nfeature);

	if (FAILED(error)) {
		return(error);
	}

	parser = get_feature_parser(nfeature->feature_code);

	/*
	 * NOTE If parser == 0 then it's 
	 * probably an unrecognized vendor specific feature
	 */
	if (parser != 0) {
		error = parser(mmc_data, nfeature->additional_length + 4, &nfeature);

		if (FAILED(error)) {
			free(nfeature);
			return(error);
		}
	}

	*feature = nfeature;

	return(SUCCESS);
}

RESULT optcl_feature_create_descriptor(optcl_feature_descriptor **descriptor,
				       const uint8_t mmc_data[],
				       uint32_t size)
{
	assert(descriptor != 0);
	assert(mmc_data != 0);
	assert(size >= 4);

	if (descriptor == 0 || mmc_data == 0 || size < 4) {
		return E_INVALIDARG;
	}

	return(parse_feature_descriptor(mmc_data, size, descriptor));
}

RESULT optcl_feature_destroy(optcl_feature *feature)
{
	assert(feature != 0);

	if (feature == 0) {
		return(E_INVALIDARG);
	}

	free(feature);

	return(SUCCESS);
}

/*
 * Table of features
 */

static struct feature_sizes_entry __feature_table[] = {
	{ FEATURE_PROFILE_LIST,			sizeof(optcl_feature_profile_list),		parse_profile_list			},
	{ FEATURE_CORE,				sizeof(optcl_feature_core),			parse_core				},
	{ FEATURE_MORPHING,			sizeof(optcl_feature_morphing),			parse_morphing				},
	{ FEATURE_REMOVABLE_MEDIUM,		sizeof(optcl_feature_removable_medium),		parse_removable_medium			},
	{ FEATURE_WRITE_PROTECT,		sizeof(optcl_feature_write_protect),		parse_write_protect			},
	{ FEATURE_RANDOM_READABLE,		sizeof(optcl_feature_random_readable),		parse_random_readable			},
	{ FEATURE_MULTI_READ,			sizeof(optcl_feature_multi_read),		parse_multi_read			},
	{ FEATURE_CD_READ,			sizeof(optcl_feature_cd_read),			parse_cd_read				},
	{ FEATURE_DVD_READ,			sizeof(optcl_feature_dvd_read),			parse_dvd_read				},
	{ FEATURE_RANDOM_WRITABLE,		sizeof(optcl_feature_random_writable),		parse_random_writable			},
	{ FEATURE_INC_STREAMING_WRITABLE,	sizeof(optcl_feature_inc_streaming_writable),	parse_inc_streaming_writable		},
	{ FEATURE_SECTOR_ERASABLE,		sizeof(optcl_feature_sector_erasable),		parse_sector_erasable			},
	{ FEATURE_FORMATTABLE,			sizeof(optcl_feature_formattable),		parse_formattable			},
	{ FEATURE_HW_DEFECT_MANAGEMENT,		sizeof(optcl_feature_hw_defect_mngmnt),		parse_hw_defect_management		},
	{ FEATURE_WRITE_ONCE,			sizeof(optcl_feature_write_once),		parse_write_once			},
	{ FEATURE_RESTRICTED_OVERWRITE,		sizeof(optcl_feature_restricted_ovr),		parse_restricted_overwrite		},
	{ FEATURE_CDRW_CAV_WRITE,		sizeof(optcl_feature_cdrw_cav_write),		parse_cdrw_cav_write			},
	{ FEATURE_MRW,				sizeof(optcl_feature_mrw),			parse_mrw				},
	{ FEATURE_ENH_DEFECT_REPORTING,		sizeof(optcl_feature_enh_defect_reporting),	parse_enh_defect_reporting		},
	{ FEATURE_DVD_PLUS_RW,			sizeof(optcl_feature_dvd_plus_rw),		parse_dvd_plus_rw			},
	{ FEATURE_DVD_PLUS_R,			sizeof(optcl_feature_dvd_plus_r),		parse_dvd_plus_r			},
	{ FEATURE_RIGID_RESTRICTED_OVERWRITE,	sizeof(optcl_feature_rigid_restricted_ovr),	parse_rigid_restricted_overwrite	},
	{ FEATURE_CD_TAO,			sizeof(optcl_feature_cd_tao),			parse_cd_tao				},
	{ FEATURE_CD_MASTERING,			sizeof(optcl_feature_cd_mastering),		parse_cd_mastering			},
	{ FEATURE_DVD_MINUS_R_MINUS_RW_WRITE,	sizeof(optcl_feature_dvd_minus_r_minus_rw_write),parse_dvd_minus_r_minus_rw_write	},
	{ FEATURE_LAYER_JUMP_RECORDING,		sizeof(optcl_feature_layer_jmp_rec),		parse_layer_jump_recording		},
	{ FEATURE_CDRW_MEDIA_WRITE_SUPPORT,	sizeof(optcl_feature_cdrw_media_write),		parse_cdrw_media_write_support		},
	{ FEATURE_BDR_POW,			sizeof(optcl_feature_bdr_pow),			parse_bdr_pow				},
	{ FEATURE_DVD_PLUS_RW_DUAL_LAYER,	sizeof(optcl_feature_dvd_plus_rw_dual_layer),	parse_dvd_plus_rw_dual_layer		},
	{ FEATURE_DVD_PLUS_R_DUAL_LAYER,	sizeof(optcl_feature_dvd_r_plus_dual_layer),	parse_dvd_plus_r_dual_layer		},
	{ FEATURE_BD_READ,			sizeof(optcl_feature_bd_read),			parse_bd_read				},
	{ FEATURE_BD_WRITE,			sizeof(optcl_feature_bd_write),			parse_bd_write				},
	{ FEATURE_TSR,				sizeof(optcl_feature_tsr),			parse_tsr				},
	{ FEATURE_HD_DVD_READ,			sizeof(optcl_feature_hd_dvd_read),		parse_hd_dvd_read			},
	{ FEATURE_HD_DVD_WRITE,			sizeof(optcl_feature_hd_dvd_write),		parse_hd_dvd_write			},
	{ FEATURE_HYBRID_DISC,			sizeof(optcl_feature_hybrid_disk),		parse_hybrid_disk			},
	{ FEATURE_POWER_MANAGEMENT,		sizeof(optcl_feature_power_mngmnt),		parse_power_management			},
	{ FEATURE_SMART,			sizeof(optcl_feature_smart),			parse_smart				},
	{ FEATURE_EMBEDDED_CHANGER,		sizeof(optcl_feature_embedded_changer),		parse_embedded_changer			},
	{ FEATURE_MICROCODE_UPGRADE,		sizeof(optcl_feature_microcode_upgrade),	parse_microcode_upgrade			},
	{ FEATURE_TIMEOUT,			sizeof(optcl_feature_timeout),			parse_timeout				},
	{ FEATURE_DVD_CSS,			sizeof(optcl_feature_dvd_css),			parse_dvd_css				},
	{ FEATURE_RT_STREAMING,			sizeof(optcl_feature_rt_streaming),		parse_rt_streaming			},
	{ FEATURE_DRIVE_SERIAL_NUMBER,		sizeof(optcl_feature_drive_serial_number),	parse_drive_serial_number		},
	{ FEATURE_MEDIA_SERIAL_NUMBER,		sizeof(optcl_feature_media_serial_number),	parse_media_serial_number		},
	{ FEATURE_DCBS,				sizeof(optcl_feature_dcbs),			parse_dcbs				},
	{ FEATURE_DVD_CPRM,			sizeof(optcl_feature_dvd_cprm),			parse_dvd_cprm				},
	{ FEATURE_FIRMWARE_INFO,		sizeof(optcl_feature_firmware_info),		parse_firmware_info			},
	{ FEATURE_AACS,				sizeof(optcl_feature_aacs),			parse_aacs				},
	{ FEATURE_VCPS,				sizeof(optcl_feature_vcps),			parse_vcps				}
};

/*
 * Table of feature helper functions
 */

static int32_t get_feature_size(uint16_t feature_code)
{
	int i;
	int elements;

	elements = sizeof(__feature_table) / sizeof(__feature_table[0]);

	i = 0;

	while (i < elements) {
		if (__feature_table[i].code == feature_code) {
			break;
		}

		++i;
	}

	if (i == elements) {
		return(-1);
	}

	return(__feature_table[i].size);
}

static raw_feature_parser get_feature_parser(uint16_t feature_code)
{
	int i;
	int elements;

	elements = sizeof(__feature_table) / sizeof(__feature_table[0]);

	i = 0;

	while (i < elements) {
		if (__feature_table[i].code == feature_code) {
			break;
		}

		++i;
	}

	if (i == elements) {
		return(0);
	}

	return(__feature_table[i].parser);
}
