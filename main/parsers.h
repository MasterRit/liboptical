/*
    parsers.h - Raw command data parsers
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

#ifndef _PARSERS_H
#define _PARSERS_H

#include "command.h"


/* MMC_OPCODE_GET_CONFIG data parser */
extern RESULT optcl_parse_get_config_data(const uint8_t *mmc_response, 
					  uint32_t size,
					  optcl_mmc_response_get_config **response);

/* MMC_OPCODE_INQUIRY data parser */
extern RESULT optcl_parse_inquiry_data(const uint8_t *mmc_response, 
				       uint32_t size,
				       optcl_mmc_response_inquiry **response);


#endif /* _PARSERS_H */
