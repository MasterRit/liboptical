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
 * Sense response codes
 */

#define SENSEDATA_RESPONSE_DESCFORMAT		0x70	/* Descriptor format sense data			*/
#define SENSEDATA_RESPONSE_DESCFORMAT_DEFFERED	0x71	/* Descriptor format deferred sense data	*/
#define SENSEDATA_RESPONSE_FIXEDFORMAT		0x72	/* Fixed format sense data			*/
#define SENSEDATA_RESPONSE_FIXEDFORMAT_DEFERRED 0x73	/* Fixed format deferred sense data		*/
#define SENSEDATA_RESPONSE_VENDOR_SPECIFIC	0x7F	/* Vendor specific sense data			*/


/*
 * Sense keys
 */

#define SENSEDATA_SK_NO_SENSE		0x00
#define SENSEDATA_SK_RECOVERED_ERROR	0x01
#define SENSEDATA_SK_NOT_READY		0x02
#define SENSEDATA_SK_MEDIUM_ERROR	0x03
#define SENSEDATA_SK_HARDWARE_ERROR	0x04
#define SENSEDATA_SK_ILLEGAL_REQUEST	0x05
#define SENSEDATA_SK_UNIT_ATTENTION	0x06
#define SENSEDATA_SK_DATA_PROTECT	0x07
#define SENSEDATA_SK_BLANK_CHECK	0x08
#define SENSEDATA_SK_VENDOR_SPECIFIC	0x09
#define SENSEDATA_SK_COPY_ABORTED	0x0A
#define SENSEDATA_SK_ABORTED_COMMAND	0x0B

/* 0x0C Obsolete */

#define SENSEDATA_SK_VOLUME_OVERFLOW	0x0D
#define SENSEDATA_SK_MISCOMPARE		0x0E

/* 0x0F Reserved */


/*
 * Unit attention sense data error codes
 */

/* Not ready to ready change, medium may have changed */
#define E_SENSE_NRTRC_MMHC	MAKE_SENSE_ERROCODE(SENSEDATA_SK_UNIT_ATTENTION, 0x28, 0x00)

/* Import or export element accessed */
#define E_SENSE_IOEEA		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_UNIT_ATTENTION, 0x28, 0x01)

/* Format - layer may have changed */
#define E_SENSE_FLMHC		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_UNIT_ATTENTION, 0x28, 0x02)

/* Power on, reset, or bus device reset occured */
#define E_SENSE_POROBDRO	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_UNIT_ATTENTION, 0x29, 0x00)

/* Power on occured */
#define E_SENSE_POO		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_UNIT_ATTENTION, 0x29, 0x01)

/* Bus reset occured */
#define E_SENSE_BRO		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_UNIT_ATTENTION, 0x29, 0x02)

/* Bus device reset function occured */
#define E_SENSE_BDRFO		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_UNIT_ATTENTION, 0x29, 0x03)

/* Device internal reset */
#define E_SENSE_DIR		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_UNIT_ATTENTION, 0x29, 0x04)

/* Parameters changed */
#define E_SENSE_PC		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_UNIT_ATTENTION, 0x2A, 0x00)

/* Mode parameters changed */
#define E_SENSE_MPC		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_UNIT_ATTENTION, 0x2A, 0x01)

/* Log parameters changed */
#define E_SENSE_LPC		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_UNIT_ATTENTION, 0x2A, 0x02)

/* Insufficient time for operation */
#define E_SENSE_ITFO		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_UNIT_ATTENTION, 0x2E, 0x00)

/* Medium destination element full */
#define E_SENSE_MDEF		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_UNIT_ATTENTION, 0x3B, 0x0D)

/* Medium source element full */
#define E_SENSE_MSEF		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_UNIT_ATTENTION, 0x3B, 0x0E)

/* End of medium reached */
#define E_SENSE_EOMR		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_UNIT_ATTENTION, 0x3B, 0x0F)

/* Medium magazine not accessible */
#define E_SENSE_MMNA		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_UNIT_ATTENTION, 0x3B, 0x11)

/* Medium magazine removed */
#define E_SENSE_MMR		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_UNIT_ATTENTION, 0x3B, 0x12)

/* Medium magazine inserted */
#define E_SENSE_MMI		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_UNIT_ATTENTION, 0x3B, 0x13)

/* Medium magazine locked */
#define E_SENSE_MML		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_UNIT_ATTENTION, 0x3B, 0x14)

/* Medium magazine unlocked */
#define E_SENSE_MMU		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_UNIT_ATTENTION, 0x3B, 0x15)

/* Target operating conditions have changed */
#define E_SENSE_TOCHC		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_UNIT_ATTENTION, 0x3F, 0x00)

/* Microcode has been changed */
#define E_SENSE_MHBC		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_UNIT_ATTENTION, 0x3F, 0x01)

/* Changed operating definition */
#define E_SENSE_COD		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_UNIT_ATTENTION, 0x3F, 0x02)

/* Inquiry data has changed */
#define E_SENSE_IDHC		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_UNIT_ATTENTION, 0x3F, 0x03)

/* Operator request or state change input */
#define E_SENSE_OROSCI		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_UNIT_ATTENTION, 0x5A, 0x00)

/* Operator medium removal request */
#define E_SENSE_OMRR		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_UNIT_ATTENTION, 0x5A, 0x01)

/* Operator selected write protect */
#define E_SENSE_OSWPROTECT	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_UNIT_ATTENTION, 0x5A, 0x02)

/* Operator selected write permit */
#define E_SENSE_OSWPERMIT	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_UNIT_ATTENTION, 0x5A, 0x03)

/* Log exception */
#define E_SENSE_LE		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_UNIT_ATTENTION, 0x5B, 0x00)

/* Threshold condition met */
#define E_SENSE_TCM		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_UNIT_ATTENTION, 0x5B, 0x01)

/* Log counter at maximum */
#define E_SENSE_LCAM		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_UNIT_ATTENTION, 0x5B, 0x02)

/* Log list codes exhausted */
#define E_SENSE_LLCE		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_UNIT_ATTENTION, 0x5B, 0x03)

/* Low power condition on */
#define E_SENSE_LPCO		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_UNIT_ATTENTION, 0x5E, 0x00)

/* Idle condition activated by timer */
#define E_SENSE_ICABT		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_UNIT_ATTENTION, 0x5E, 0x01)

/* Standby condition activated by timer */
#define E_SENSE_SCABT		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_UNIT_ATTENTION, 0x5E, 0x02)

/* Idle condition activated by command */
#define E_SENSE_ICABC		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_UNIT_ATTENTION, 0x5E, 0x03)

/* Standby condition activated by command */
#define E_SENSE_SCABC		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_UNIT_ATTENTION, 0x5E, 0x04)


/*
 * CDB or parameter validation sense data error codes
 */

/* Parameter list length error */
#define E_SENSE_PLLE		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x1A, 0x00)

/* Invalid command operation code */
#define E_SENSE_ICOC		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x20, 0x00)

/* Logical block address out of range */
#define E_SENSE_LBAOOR		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x21, 0x00)

/* Invalid element address */
#define E_SENSE_IEA		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x21, 0x01)

/* Invalid address for write */
#define E_SENSE_IAFW		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x21, 0x02)

/* Invalid write crossing layer jump */
#define E_SENSE_IWCLJ		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x21, 0x03)

/* Invalid function */
#define E_SENSE_IF		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x22, 0x00)

/* Invalid field in CDB */
#define E_SENSE_IFICDB		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x24, 0x00)

/* Invalid field in parameter list */
#define E_SENSE_IFIPL		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x26, 0x00)

/* Parameter not supported */
#define E_SENSE_PNS		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x26, 0x01)

/* Parameter value invalid */
#define E_SENSE_PVI		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x26, 0x02)

/* Threshold parameters not supported */
#define E_SENSE_TPNS		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x26, 0x03)

/*
 * Readiness error codes
 */

/* Logical unit not ready, cause not reportable */
#define E_SENSE_LUNR_CNR	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_NOT_READY, 0x04, 0x00)

/* Logical unit is in process of becoming ready */
#define E_SENSE_LUIIPOBR	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_NOT_READY, 0x04, 0x01)

/* Logical unit not ready, initializing command required */
#define E_SENSE_LUNR_ICR	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_NOT_READY, 0x04, 0x02)

/* Logical unit not ready, manual intervention required */
#define E_SENSE_LUNR_MIR	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_NOT_READY, 0x04, 0x03)

/* Logical unit not ready, format in progress */
#define E_SENSE_LUNR_FIP	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_NOT_READY, 0x04, 0x03)

/* Logical unit not ready, operation in progress */
#define E_SENSE_LUNR_OIP	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_NOT_READY, 0x04, 0x07)

/* Logical unit not ready, long write in progress */
#define E_SENSE_LUNR_LWIP	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_NOT_READY, 0x04, 0x08)

/* Write error recovery needed */
#define E_SENSE_WERN		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_NOT_READY, 0x0C, 0x07)

/* Defects in error window */
#define E_SENSE_DIEW		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_NOT_READY, 0x0C, 0x0F)

/* Incompatible medium installed */
#define E_SENSE_IMI		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_NOT_READY, 0x30, 0x00)

/* Cannot read medium - unknown format */
#define E_SENSE_CRM_UF		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_NOT_READY, 0x30, 0x01)

/* Cannot read medium - incompatible format */
#define E_SENSE_CRM_IF_2	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_NOT_READY, 0x30, 0x02)

/* Cannot read medium - incompatible format */
#define E_SENSE_CRM_IF_5	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x30, 0x02)

/* Cleaning cartridge installed */
#define E_SENSE_CCI		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_NOT_READY, 0x30, 0x03)

/* Cannot write medium - unknown format */
#define E_SENSE_CWM_UF_2	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_NOT_READY, 0x30, 0x04)

/* Cannot write medium - unknown format */
#define E_SENSE_CWM_UF_5	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x30, 0x04)

/* Cannot write medium - incompatible format */
#define E_SENSE_CWM_IF_2	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_NOT_READY, 0x30, 0x05)

/* Cannot write medium - incompatible format */
#define E_SENSE_CWM_IF_5	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x30, 0x05)

/* Cannot format medium - incompatible medium */
#define E_SENSE_CFM_IM_2	MAKE_SENSE_ERRORCODE(SENSEDAT_SK_NOT_READY, 0x30, 0x06)

/* Cannot format medium - incompatible medium */
#define E_SENSE_CFM_IM_5	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x30, 0x06)

/* Cleaning failure */
#define E_SENSE_CF		MAKE_SENSE_ERRORCODE(SENSEDAT_SK_NOT_READY, 0x30, 0x07)

/* Cannot write medium - unsupported medium version */
#define E_SENSE_CWM_UMV_2	MAKE_SENSE_ERRORCODE(SENSEDAT_SK_NOT_READY, 0x30, 0x11)

/* Cannot write medium - unsupported medium version */
#define E_SENSE_CWM_UMV_5	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x30, 0x11)

/* Medium not present */
#define E_SENSE_MNP		MAKE_SENSE_ERRORCODE(SENSEDAT_SK_NOT_READY, 0x3A, 0x00)

/* Medium not present - tray closed */
#define E_SENSE_MNP_TC		MAKE_SENSE_ERRORCODE(SENSEDAT_SK_NOT_READY, 0x3A, 0x01)

/* Medium not present - tray open */
#define E_SENSE_MNP_TO		MAKE_SENSE_ERRORCODE(SENSEDAT_SK_NOT_READY, 0x3A, 0x02)

/* Logical unit has not self-configured yet */
#define E_SENSE_LUHNSCY		MAKE_SENSE_ERRORCODE(SENSEDAT_SK_NOT_READY, 0x3E, 0x00)


/*
 * Sense data functions
 */

/* Parse error code from raw sense data */
extern RESULT optcl_sensedata_get_code(const uint8_t raw_data[], 
				       uint8_t size, 
				       RESULT *error_code);


#endif /* _SENSEDATA_H */
