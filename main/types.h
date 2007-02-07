/*
    types.h - Platform dependent data types
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

#ifndef _TYPES_H
#define _TYPES_H

#ifdef WIN32

#define True			1
#define False			0

typedef unsigned __int8		bool_t;

typedef __int8			int8_t;
typedef __int16			int16_t;
typedef __int32			int32_t;
typedef __int64			int64_t;

typedef unsigned __int8		uint8_t;
typedef unsigned __int16	uint16_t;
typedef unsigned __int32	uint32_t;
typedef unsigned __int64	uint64_t;

#endif

/*
 * Macros to convert from little-endian SCSI values to host endiannes
 */

#ifdef WIN32
#define LITTLE_ENDIAN
#endif

#define uint16_swap_le_be(val)	((uint16_t)(				\
	((((uint16_t)(val) & (uint16_t)0x00ffU) << 8)		|	\
	 (((uint16_t)(val) & (uint16_t)0xff00U) >> 8))))

#define uint32_swap_le_be(val)  ((uint32_t)(				\
	((((uint32_t)(val) & (uint32_t)0x000000ffU) << 24)	|	\
	 (((uint32_t)(val) & (uint32_t)0x0000ff00U) <<  8)	|	\
	 (((uint32_t)(val) & (uint32_t)0x00ff0000U) >>  8)	|	\
	 (((uint32_t)(val) & (uint32_t)0xff000000U) >> 24))))

#define uint64_swap_le_be(val)	((uint64_t)(				\
	((((uint64_t)(val) & (uint64_t)0x00000000000000ffU) << 56) |	\
	 (((uint64_t)(val) & (uint64_t)0x000000000000ff00U) << 40) |	\
	 (((uint64_t)(val) & (uint64_t)0x0000000000ff0000U) << 24) |	\
	 (((uint64_t)(val) & (uint64_t)0x00000000ff000000U) <<  8) |	\
	 (((uint64_t)(val) & (uint64_t)0x000000ff00000000U) >>  8) |	\
	 (((uint64_t)(val) & (uint64_t)0x0000ff0000000000U) >> 24) |	\
	 (((uint64_t)(val) & (uint64_t)0x00ff000000000000U) >> 40) |	\
	 (((uint64_t)(val) & (uint64_t)0xff00000000000000U) >> 56))))


#ifdef LITTLE_ENDIAN

#define int16_to_le(val)	((int16_t)(val))
#define int32_to_le(val)	((int32_t)(val))
#define int64_to_le(val)	((int64_t)(val))

#define uint16_to_le(val)	((uint16_t)(val))
#define uint32_to_le(val)	((uint32_t)(val))
#define uint64_to_le(val)	((uint64_t)(val))

#define int16_from_le(val)	((int16_t)(val))
#define int32_from_le(val)	((int32_t)(val))
#define int64_from_le(val)	((int64_t)(val))

#define uint16_from_le(val)	((uint16_t)(val))
#define uint32_from_le(val)	((uint32_t)(val))
#define uint64_from_le(val)	((uint64_t)(val))

#define int16_to_be(val)	((int16_t)uint16_swap_le_be(val))
#define int32_to_be(val)	((int32_t)uint32_swap_le_be(val))
#define int64_to_be(val)	((int64_t)uint64_swap_le_be(val))

#define uint16_to_be(val)	((uint16_t)uint16_swap_le_be(val))
#define uint32_to_be(val)	((uint32_t)uint32_swap_le_be(val))
#define uint64_to_be(val)	((uint64_t)uint64_swap_le_be(val))

#define int16_from_be(val)	((int16_t)uint16_swap_le_be(val))
#define int32_from_be(val)	((int32_t)uint32_swap_le_be(val))
#define int64_from_be(val)	((int64_t)uint64_swap_le_be(val))

#define uint16_from_be(val)	((uint16_t)uint16_swap_le_be(val))
#define uint32_from_be(val)	((uint32_t)uint32_swap_le_be(val))
#define uint64_from_be(val)	((uint64_t)uint64_swap_le_be(val))

#else

#define int16_to_le(val)	((int16_t)uint16_swap_le_be(val))
#define int32_to_le(val)	((int32_t)uint32_swap_le_be(val))
#define int64_to_le(val)	((int64_t)uint64_swap_le_be(val))

#define uint16_to_le(val)	((uint16_t)uint16_swap_le_be(val))
#define uint32_to_le(val)	((uint32_t)uint32_swap_le_be(val))
#define uint64_to_le(val)	((uint64_t)uint64_swap_le_be(val))

#define int16_from_le(val)	((int16_t)uint16_swap_le_be(val))
#define int32_from_le(val)	((int32_t)uint32_swap_le_be(val))
#define int64_from_le(val)	((int64_t)uint64_swap_le_be(val))

#define uint16_from_le(val)	((uint16_t)uint16_swap_le_be(val))
#define uint32_from_le(val)	((uint32_t)uint32_swap_le_be(val))
#define uint64_from_le(val)	((uint64_t)uint64_swap_le_be(val))

#define int16_to_be(val)	((int16_t)(val))
#define int32_to_be(val)	((int32_t)(val))
#define int64_to_be(val)	((int64_t)(val))

#define uint16_to_be(val)	((uint16_t)(val))
#define uint32_to_be(val)	((uint32_t)(val))
#define uint64_to_be(val)	((uint64_t)(val))

#define int16_from_be(val)	((int16_t)(val))
#define int32_from_be(val)	((int32_t)(val))
#define int64_from_be(val)	((int64_t)(val))

#define uint16_from_be(val)	((uint16_t)(val))
#define uint32_from_be(val)	((uint32_t)(val))
#define uint64_from_be(val)	((uint64_t)(val))

#endif

/*
 * Helper macros
 */

#define uint32_from_le_bytes(b0, b1, b2, b3)	((uint32_t)uint32_from_le(		\
	(uint8_t)b0 << 24 | (uint8_t)b1 << 16 | (uint8_t)b2 << 8 | (uint8_t)b3		\
	))

#define uint32_from_be_bytes(b0, b1, b2, b3)	((uint32_t)uint32_from_be(		\
	(uint8_t)b3 << 24 | (uint8_t)b2 << 16 | (uint8_t)b1 << 8 | (uint8_t)b0		\
	))

#define bool_from_uint8(val)	((bool_t)((val) > 0))

#endif /* _TYPES_H */
