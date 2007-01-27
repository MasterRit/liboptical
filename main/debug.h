/*
    debug.h - Debug functions
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

#ifndef _DEBUG_H
#define _DEBUG_H


#ifdef _DEBUG

#include "types.h"


/*
 * Debug functions
 */

/* Set log filename */
int optcl_debug_set_log_file(char *filename);

/* Log array of bytes to log file */
int optcl_debug_log_bytes(const char *message, 
			  const uint8_t *data, 
			  uint32_t size);


/*
 * Debug macros
 */

#define OPTCL_TRACE_ARRAY(array, size)		\
	optcl_debug_log_bytes("bytes:", array, size)

#define OPTCL_TRACE_ARRAY_MSG(msg, array, size)	\
	optcl_debug_log_bytes(msg, array, size)

#else

#define OPTCL_TRACE_ARRAY(array, size)
#define OPTCL_TRACE_ARRAY_MSG(msg, array, size)

#endif /* _DEBUG */


#endif /* _DEBUG_H */
