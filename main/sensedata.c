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

#include <stdafx.h>

#include "errors.h"
#include "sensedata.h"
#include "types.h"

#include <assert.h>

/*
 * Sense data functions
 */

RESULT optcl_sensedata_get_code(const uint8_t raw_data[], 
				uint8_t size, 
				RESULT *error_code)
{
	RESULT error = SUCCESS;

	uint8_t asc = 0;
	uint8_t ascq = 0;
	uint8_t sk = 0;
	uint8_t addlen = 0;
	uint8_t response_code;

	assert(raw_data != 0);
	assert(error_code != 0);
	assert(size > 0);

	if (raw_data == 0 || error_code == 0 || size == 0) {
		return(E_INVALIDARG);
	}

	response_code = raw_data[0] & 0x7F;			/* 01111111 */

	switch(response_code) {
		case SENSEDATA_RESPONSE_DESCFORMAT:
		case SENSEDATA_RESPONSE_DESCFORMAT_DEFFERED: {
			if (size > 1) {
				sk = raw_data[2] & 0x0F;	/* 00001111 */
			}

			if (size > 2) {
				asc = raw_data[2];
			}

			if (size > 3) {
				ascq = raw_data[3];
			}

			break;
		}
		case SENSEDATA_RESPONSE_FIXEDFORMAT:
		case SENSEDATA_RESPONSE_FIXEDFORMAT_DEFERRED: {
			if (size > 2) {
				sk = raw_data[2] & 0x0F;	/* 00001111 */
			}

			if (size > 7) {
				addlen = raw_data[7];
			}

			assert(addlen + 7 == size);

			if (addlen + 7 != size) {
				error = E_SIZEMISMATCH;
				break;
			}

			if (addlen > 5) {
				asc = raw_data[12];
			}

			if (addlen > 6) {
				ascq = raw_data[13];
			}

			break;
		}
		case SENSEDATA_RESPONSE_VENDOR_SPECIFIC: {
			sk = asc = ascq = 0xFF;
			break;
		}
		default: {
			error = E_INVALIDRESPONSECODE;
			break;
		}
	}

	if (FAILED(error)) {
		return(error);
	}

	*error_code = MAKE_SENSE_ERROCODE(sk, asc, ascq);

	return(SUCCESS);
}
