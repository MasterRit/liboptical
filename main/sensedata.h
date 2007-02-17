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
#include "list.h"
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
 * Sense data structures
 */

typedef struct tag_sensedata optcl_sensedata;

typedef struct tag_sensedata_descriptor {
	uint8_t descriptor_type;
	bool_t valid;

	union tag_descriptor {
		uint32_t information;
		uint8_t vendor_specific[256];
	} descriptor_data;

} optcl_sensedata_descriptor;

/*
 * Sense data functions
 */

/* Create new sense data structure */
extern RESULT optcl_sensedata_create(uint8_t response_code, 
				     optcl_sensedata **sense);

/* Create new sense data structure from raw data */
extern RESULT optcl_sensedata_create_from_raw(uint8_t raw_data[], 
					      uint8_t size, 
					      optcl_sensedata **sense);

/* Clear sense data structure  */
extern RESULT optcl_sensedata_clear(optcl_sensedata *sense);

/* Copy sense data */
extern RESULT optcl_sensedata_copy(optcl_sensedata *dest, 
				   const optcl_sensedata *src);

/* Destroy sense data structure */
extern RESULT optcl_sensedata_destroy(optcl_sensedata *sense);

/* Get additional sense code */
extern RESULT optcl_sensedata_get_asc(const optcl_sensedata *sense, 
				      uint8_t *asc);

/* Get additional sense code qualifier */
extern RESULT optcl_sensedata_get_ascq(const optcl_sensedata *sense, 
				       uint8_t *ascq);

/* Get sense bytes */
extern RESULT optcl_sensedata_get_bytes(const optcl_sensedata *sense, 
					uint8_t **bytes);

/* Get command-specific information */
extern RESULT optcl_sensedata_get_csi(const optcl_sensedata *sense, 
				      uint16_t *csi);

/* Get sense data descriptors */
extern RESULT optcl_sensedata_get_descriptors(const optcl_sensedata *sense, 
					      optcl_list **descriptors);

/* Get eom field */
extern RESULT optcl_sensedata_get_eom(const optcl_sensedata *sense, 
				      bool_t *eom);

/* Get field replacable unit code */
extern RESULT optcl_sensedata_get_fruc(const optcl_sensedata *sense, 
				       uint8_t *fruc);

/* Get filemark field  */
extern RESULT optcl_sensedata_get_filemark(const optcl_sensedata *sense, 
					   bool_t *filemark);

/* Get ili field */
extern RESULT optcl_sensedata_get_ili(const optcl_sensedata *sense, 
				      bool_t *ili);

/* Get information field */
extern RESULT optcl_sensedata_get_information(const optcl_sensedata *sense, 
					      uint16_t *information);

/* Get response code */
extern RESULT optcl_sensedata_get_response_code(const optcl_sensedata *sense, 
						uint8_t *response_code);

/* Get sense key */
extern RESULT optcl_sensedata_get_sk(const optcl_sensedata *sense, 
				     uint8_t *sk);

/* Get sense key specific field */
extern RESULT optcl_sensedata_get_sks(const optcl_sensedata *sense, 
				      uint16_t *sks);

/* Get sksv field */
extern RESULT optcl_sensedata_get_sksv(const optcl_sensedata *sense, 
				       bool_t *sksv);

/* Get valid field */
extern RESULT optcl_sensedata_get_valid(const optcl_sensedata *sense, 
					bool_t *valid);

/* Get vendor specific sense data */
extern RESULT optcl_sensedata_get_vendor(const optcl_sensedata *sense,
					 uint8_t **raw);

/* Set additional sense code */
extern RESULT optcl_sensedata_set_asc(optcl_sensedata *sense, uint8_t asc);

/* Set additional sense code qualifier */
extern RESULT optcl_sensedata_set_ascq(optcl_sensedata *sense, uint8_t ascq);

/* Set sense bytes */
extern RESULT optcl_sensedata_set_bytes(optcl_sensedata *sense, 
					uint8_t *bytes);

/* Set command-specific information */
extern RESULT optcl_sensedata_set_csi(optcl_sensedata *sense, uint16_t csi);

/* Set sense data descriptors */
extern RESULT optcl_sensedata_set_descriptors(optcl_sensedata *sense, 
					      optcl_list *descriptors);

/* Set eom field */
extern RESULT optcl_sensedata_set_eom(optcl_sensedata *sense, bool_t eom);

/* Set field replacable unit code */
extern RESULT optcl_sensedata_set_fruc(optcl_sensedata *sense, uint8_t fruc);

/* Set filemark field  */
extern RESULT optcl_sensedata_set_filemark(optcl_sensedata *sense, 
					   bool_t filemark);

/* Set ili field */
extern RESULT optcl_sensedata_set_ili(optcl_sensedata *sense, bool_t ili);

/* Set information field */
extern RESULT optcl_sensedata_set_information(optcl_sensedata *sense, 
					      uint16_t information);

/* Set response code */
extern RESULT optcl_sensedata_set_response_code(optcl_sensedata *sense, 
						uint8_t response_code);

/* Set sense key */
extern RESULT optcl_sensedata_set_sk(optcl_sensedata *sense, uint8_t sk);

/* Set sense key specific field */
extern RESULT optcl_sensedata_set_sks(optcl_sensedata *sense, uint16_t sks);

/* Set sksv field */
extern RESULT optcl_sensedata_set_sksv(optcl_sensedata *sense, bool_t sksv);

/* Set valid field */
extern RESULT optcl_sensedata_set_valid(optcl_sensedata *sense, bool_t valid);

/* Set vendor specific sense data */
extern RESULT optcl_sensedata_set_vendor(optcl_sensedata *sense,
					 uint8_t *raw);

#endif /* _SENSEDATA_H */
