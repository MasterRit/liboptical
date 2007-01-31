/*
    errors.h - Error codes for the library
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

#ifndef _ERRORS_H
#define _ERRORS_H

#include "types.h"

/* Error code data type */

typedef int32_t RESULT;

/*
 *  Values are 32 bit values layed out as follows:
 *
 *   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
 *   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 *  +-+-+-+-+-----------------------+-------------------------------+
 *  |S|R|C|R|     Facility          |               Code            |
 *  +-+-+-+-+-----------------------+-------------------------------+
 *
 *  where
 *
 *      Sev - is the severity code
 *
 *          0 - Success
 *          1 - Fail
 *
 *      C - is the OEM code flag
 *
 *      R - is a reserved bit
 *
 *      Facility - is the facility code
 *
 *      Code - is the facility's status code
 */

/* Severity codes */

#define SEVERITY_ERROR		0
#define SEVERITY_SUCCESS	1

/* Facility codes */

#define FACILITY_GENERAL	0
#define FACILITY_DEVICE		1
#define FACILITY_TRANSPORT	2
#define FACILITY_COLLECTIONS	3
#define FACILITY_FEATURES	4

/* Error status testing macros */

#define SUCCEEDED(e)		((bool_t)((RESULT)(e) < 0))
#define FAILED(e)		((bool_t)((RESULT)(e) >= 0))

#define IS_ERROR(e)		((bool_t)((RESULT)(e) >> 31 == SEVERITY_ERROR))

#define ERROR_CODE(e)		((int16_t)((RESULT)(e) & 0xFFFF))
#define ERROR_FACILITY(e)	((int16_t)((RESULT)((e) >> 16) & 0x1fff))
#define ERROR_SEVERITY(e)	((int8_t)(((RESULT)(e) >> 31) & 0x01))

/* Error formating helper macros */

#define MAKE_ERRORCODE(sev, fac, code)		\
	((RESULT)(((int32_t)(sev) << 30)	\
		| ((int32_t)(fac) << 16)	\
		| ((int32_t)(code))))

/* General status codes */

#define SUCCESS			((RESULT)(1 << 31))

/* General error codes */

#define E_INVALIDARG		\
	MAKE_ERRORCODE(SEVERITY_ERROR, FACILITY_GENERAL, 0)

#define E_OUTOFMEMORY		\
	MAKE_ERRORCODE(SEVERITY_ERROR, FACILITY_GENERAL, 1)

#define E_OUTOFRANGE		\
	MAKE_ERRORCODE(SEVERITY_ERROR, FACILITY_GENERAL, 2)

#define E_OVERFLOW		\
	MAKE_ERRORCODE(SEVERITY_ERROR, FACILITY_GENERAL, 3)

#define E_SIZEMISMATCH		\
	MAKE_ERRORCODE(SEVERITY_ERROR, FACILITY_GENERAL, 4)

#define E_UNEXPECTED		\
	MAKE_ERRORCODE(SEVERITY_ERROR, FACILITY_GENERAL, 5)

#define E_ACCESSDENIED		\
	MAKE_ERRORCODE(SEVERITY_ERROR, FACILITY_GENERAL, 6)

#define E_POINTER		\
	MAKE_ERRORCODE(SEVERITY_ERROR, FACILITY_GENERAL, 7)

/* FACILITY_DEVICE error codes */

#define E_DEVUNKNFEATURE	\
	MAKE_ERRORCODE(SEVERITY_ERROR, FACILITY_DEVICE, 1)

#define E_DEVNOMOREITEMS	\
	MAKE_ERRORCODE(SEVERITY_ERROR, FACILITY_DEVICE, 2)

#define E_DEVINVALIDPATH	\
	MAKE_ERRORCODE(SEVERITY_ERROR, FACILITY_DEVICE, 3)

#define E_DEVINVALIDSIZE	\
	MAKE_ERRORCODE(SEVERITY_ERROR, FACILITY_DEVICE, 4)

#define E_DEVNOMOREDATA		\
	MAKE_ERRORCODE(SEVERITY_ERROR, FACILITY_DEVICE, 259)

/* FACILITY_TRANSPORT error codes */

/* FACILITY_COLLECTIONS error codes */

#define E_COLLINVLDHASHTABLE	\
	MAKE_ERRORCODE(SEVERITY_ERROR, FACILITY_COLLECTIONS, 0)

/* FACILITY_FEATURES erro codes */

#define E_FEATINVHEADER		\
	MAKE_ERRORCODE(SEVERITY_ERROR, FACILITY_FEATURES, 0)

#define E_FEATTABLEFULL		\
	MAKE_ERRORCODE(SEVERITY_ERROR, FACILITY_FEATURES, 1)

#define E_FEATUNKCODE		\
	MAKE_ERRORCODE(SEVERITY_ERROR, FACILITY_FEATURES, 2)

#endif /* _ERRORS_H */
