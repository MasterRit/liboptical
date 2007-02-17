/*
    sensedata.c - Sense data functions implementation
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

#include "sensedata.h"

#include <assert.h>
#include <malloc.h>
#include <string.h>


/*
 * Internal sense data structures
 */

typedef struct tag_fixed_sensedata {
	uint8_t response_code;
	uint8_t sk;
	uint8_t asc;
	uint8_t ascq;
	bool_t filemark;
	bool_t eom;
	bool_t ili;
	uint16_t information;
	uint16_t csi;
	uint8_t fruc;
	uint16_t skc;
	uint8_t data[238];
} optcl_fixed_sensedata;

typedef struct tag_descriptor_sensedata {
	uint8_t response_code;
	uint8_t sk;
	uint8_t asc;
	uint8_t ascq;
	optcl_list *descriptors;
} optcl_descriptor_sensedata;

typedef struct tag_vendor_sensedata {
	uint8_t data[256];
} optcl_vendor_sensedata;

typedef struct tag_sensedata {
	uint8_t response_code;
	uint8_t sk;
	uint8_t asc;
	uint8_t ascq;

	union tag_internal_sense_data {
		optcl_fixed_sensedata *fixed;
		optcl_descriptor_sensedata *descriptor;
		optcl_vendor_sensedata *vendor;
	} sense_data;

} optcl_sensedata;


/*
 * Sense data functions
 */

RESULT optcl_sensedata_create(uint8_t response_code, optcl_sensedata **sense)
{
	optcl_sensedata *nsense = 0;

	assert(sense != 0);

	if (sense == 0) {
		return(E_INVALIDARG);
	}

	nsense = malloc(sizeof(optcl_sensedata));

	if (nsense == 0) {
		return(E_OUTOFMEMORY);
	}

	memset(nsense, 0, sizeof(optcl_sensedata));


	nsense->response_code = response_code;


}

RESULT optcl_sensedata_create_from_raw(uint8_t raw_data[], 
				       uint8_t size, 
				       optcl_sensedata **sense)
{
}

RESULT optcl_sensedata_clear(optcl_sensedata *sense)
{
}

RESULT optcl_sensedata_copy(optcl_sensedata *dest, const optcl_sensedata *src)
{
}

RESULT optcl_sensedata_destroy(optcl_sensedata *sense)
{
}

RESULT optcl_sensedata_get_asc(const optcl_sensedata *sense, uint8_t *asc)
{
}

RESULT optcl_sensedata_get_ascq(const optcl_sensedata *sense, uint8_t *ascq)
{
}

RESULT optcl_sensedata_get_bytes(const optcl_sensedata *sense, uint8_t **bytes)
{
}

RESULT optcl_sensedata_get_csi(const optcl_sensedata *sense, uint16_t *csi)
{
}

RESULT optcl_sensedata_get_eom(const optcl_sensedata *sense, bool_t *eom)
{
}

RESULT optcl_sensedata_get_fruc(const optcl_sensedata *sense, uint8_t *fruc)
{
}

RESULT optcl_sensedata_get_filemark(const optcl_sensedata *sense, 
				    bool_t *filemark)
{
}

RESULT optcl_sensedata_get_ili(const optcl_sensedata *sense, bool_t *ili)
{
}

RESULT optcl_sensedata_get_information(const optcl_sensedata *sense, 
				       uint16_t *information)
{
}

RESULT optcl_sensedata_get_response_code(const optcl_sensedata *sense, 
					 uint8_t *response_code)
{
}

RESULT optcl_sensedata_get_sk(const optcl_sensedata *sense, uint8_t *sk)
{
}

RESULT optcl_sensedata_get_sks(const optcl_sensedata *sense, uint16_t *sks)
{
}

RESULT optcl_sensedata_get_sksv(const optcl_sensedata *sense, bool_t *sksv)
{
}

RESULT optcl_sensedata_get_valid(const optcl_sensedata *sense, bool_t *valid)
{
}

RESULT optcl_sensedata_get_descriptors(const optcl_sensedata *sense, 
				       optcl_list **descriptors)
{
}

RESULT optcl_sensedata_set_asc(optcl_sensedata *sense, uint8_t asc)
{
}

RESULT optcl_sensedata_set_ascq(optcl_sensedata *sense, uint8_t ascq)
{
}

RESULT optcl_sensedata_set_bytes(optcl_sensedata *sense, uint8_t *bytes)
{
}

RESULT optcl_sensedata_set_csi(optcl_sensedata *sense, uint16_t csi)
{
}

RESULT optcl_sensedata_set_descriptors(optcl_sensedata *sense, 
				       optcl_list *descriptors)
{
}

RESULT optcl_sensedata_set_eom(optcl_sensedata *sense, bool_t eom)
{
}

RESULT optcl_sensedata_set_fruc(optcl_sensedata *sense, uint8_t fruc)
{
}

RESULT optcl_sensedata_set_filemark(optcl_sensedata *sense, bool_t filemark)
{
}

RESULT optcl_sensedata_set_ili(optcl_sensedata *sense, bool_t ili)
{
}

RESULT optcl_sensedata_set_information(optcl_sensedata *sense, 
				       uint16_t information)
{
}

RESULT optcl_sensedata_set_response_code(optcl_sensedata *sense, 
					 uint8_t response_code)
{
}

RESULT optcl_sensedata_set_sk(optcl_sensedata *sense, uint8_t sk)
{
}

RESULT optcl_sensedata_set_sks(optcl_sensedata *sense, uint16_t sks)
{
}

RESULT optcl_sensedata_set_sksv(optcl_sensedata *sense, bool_t sksv)
{
}

RESULT optcl_sensedata_set_valid(optcl_sensedata *sense, bool_t valid)
{
}

RESULT optcl_sensedata_set_vendor(optcl_sensedata *sense,
				  uint8_t *raw)
{
}
