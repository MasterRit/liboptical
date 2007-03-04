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
#include "helpers.h"
#include "sensedata.h"
#include "types.h"

#include <assert.h>


/*
 * Internal structures
 */

struct error_code_entry {
	RESULT error_code;
	char *message;
};


/*
 * Forward declarations
 */

static char* get_error_message(RESULT error_code);

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

	*error_code = MAKE_SENSE_ERRORCODE(sk, asc, ascq);

	return(SUCCESS);
}

RESULT optcl_sensedata_get_formatted_msg(RESULT error_code,
					 char **message)
{
	char *msg = 0;

	assert(message != 0);

	if (message == 0) {
		return(E_INVALIDARG);
	}

	msg = get_error_message(error_code);

	if (msg == 0) {
		return(E_OUTOFRANGE);
	}

	*message = xstrdup(msg);

	if (*message == 0) {
		return(E_OUTOFMEMORY);
	}

	return(SUCCESS);
}


/*
 * Error messages table
 */

static struct error_code_entry __message_entries[] = {

	/* Unit attention error codes */

	{ E_SENSE_NRTRC_MMHC,		"Not ready to ready change, medium may have changed" }

};


/*
 * Helper functions
 */

static char* get_error_message(RESULT error_code)
{
	int i = 0;
	int elements = sizeof(__message_entries) / sizeof(__message_entries[0]);

	while (i < elements) {
		if (__message_entries[i].error_code == error_code) {
			break;
		}

		++i;
	}

	if (i == elements) {
		return(0);
	}

	return(__message_entries[i].message);
}
