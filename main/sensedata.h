/*
    sensedata.h - Sense data
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

#ifndef _SENSEDATA_H
#define _SENSEDATA_H


#include "errors.h"
#include "types.h"


/*
 * Sense descriptor types
 */

#define SENSEDATA_DESCTYPE_INFORMATION		0x00
#define SENSEDATA_DESCTYPE_CSI			0x01	/* Command specific information		*/
#define SENSEDATA_DESCTYPE_SKS			0x02	/* Sense key specific			*/
#define SENSEDATA_DESCTYPE_FRU			0x03	/* Field replacable unit		*/
#define SENSEDATA_DESCTYPE_STREAM		0x04	/* Stream commands			*/
#define SENSEDATA_DESCTYPE_BLOCK		0x05	/* Block commands			*/
#define SENSEDATA_DESCTYPE_OSD_OBJECT_ID	0x06	/* OSD object identification		*/
#define SENSEDATA_DESCTYPE_OSD_RICV		0x07	/* OSD response integrity check value	*/
#define SENSEDATA_DESCTYPE_OSD_ATTR_ID		0x08	/* OSD attribute identification		*/
#define SENSEDATA_DESCTYPE_ATA_RETURN		0x09	/* ATA return				*/

/* 0x0A - 0x7F Reserved		*/
/* 0x80 - 0xFF Vendor specific	*/

/*
 * Sense response codes
 */

#define SENSEDATA_RESPONSE_DESCFORMAT		0x70	/* Descriptor format sense data			*/
#define SENSEDATA_RESPONSE_DESCFORMAT_DEFFERED	0x71	/* Descriptor format deferred sense data	*/
#define SENSEDATA_RESPONSE_FIXEDFORMAT		0x72	/* Fixed format sense data			*/
#define SENSEDATA_RESPONSE_FIXEDFORMAT_DEFERRED 0x73	/* Fixed format deferred sense data		*/
#define SENSEDATA_RESPONSE_VENDOR_SPECIFIC	0x7F	/* Vendor specific sense data			*/


/*
 * Sense data functions
 */

/* Parse error code from raw sense data */
extern RESULT optcl_sensedata_get_code(const uint8_t raw_data[], uint8_t size);


#endif /* _SENSEDATA_H */
