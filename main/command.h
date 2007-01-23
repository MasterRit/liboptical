/*
    command.h - Multi-Media Commands - 5 (MMC-5).
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

#ifndef _COMMAND_H
#define _COMMAND_H

#include "device.h"


typedef struct tag_mmc_inquiry {
	uint8_t evpd;
	uint8_t page_code;
	uint16_t allocation_length;
	uint8_t control;
} optcl_mmc_inquiry;

typedef struct tag_mmc_result_inquiry {
	uint8_t qualifier;	/* Peripheral qualifier */
	uint8_t device_type;
	uint8_t rmb;
	uint8_t version;
	uint8_t normaca;
	uint8_t hisup;
	uint8_t rdf;		/* Response data format */
	uint8_t additional_len;
	uint8_t sccs;
	uint8_t acc;
	uint8_t tpgs;
	uint8_t _3pc;
	uint8_t protect;
	uint8_t bque;
	uint8_t encserv;
	uint8_t vs;
	uint8_t multip;
	uint8_t mchngr;
	uint8_t addr16;
	uint8_t wbus16;
	uint8_t sync;
	uint8_t linked;
	uint8_t cmdque;
	uint8_t vendor[9];
	uint8_t product[17];
	uint32_t revision_level;
	uint8_t vendor_string[21];
	uint8_t clocking;
	uint8_t qas;
	uint8_t ius;
	uint16_t ver_desc1;
	uint16_t ver_desc2;
	uint16_t ver_desc3;
	uint16_t ver_desc4;
	uint16_t ver_desc5;
	uint16_t ver_desc6;
	uint16_t ver_desc7;
	uint16_t ver_desc8;
} optcl_mmc_result_inquiry;

/*
 * Command functions
 */

int optcl_command_inquiry(const optcl_device *device, 
			  const optcl_mmc_inquiry *command, 
			  optcl_mmc_result_inquiry **result);

#endif /* _COMMAND_H */
