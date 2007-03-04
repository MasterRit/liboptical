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

#define SENSEDATA_RESPONSE_DESCFORMAT			0x70	/* Descriptor format sense data			*/
#define SENSEDATA_RESPONSE_DESCFORMAT_DEFFERED		0x71	/* Descriptor format deferred sense data	*/
#define SENSEDATA_RESPONSE_FIXEDFORMAT			0x72	/* Fixed format sense data			*/
#define SENSEDATA_RESPONSE_FIXEDFORMAT_DEFERRED		0x73	/* Fixed format deferred sense data		*/
#define SENSEDATA_RESPONSE_VENDOR_SPECIFIC		0x7F	/* Vendor specific sense data			*/


/*
 * Sense keys
 */

#define SENSEDATA_SK_NO_SENSE			0x00
#define SENSEDATA_SK_RECOVERED_ERROR		0x01
#define SENSEDATA_SK_NOT_READY			0x02
#define SENSEDATA_SK_MEDIUM_ERROR		0x03
#define SENSEDATA_SK_HARDWARE_ERROR		0x04
#define SENSEDATA_SK_ILLEGAL_REQUEST		0x05
#define SENSEDATA_SK_UNIT_ATTENTION		0x06
#define SENSEDATA_SK_DATA_PROTECT		0x07
#define SENSEDATA_SK_BLANK_CHECK		0x08
#define SENSEDATA_SK_VENDOR_SPECIFIC		0x09
#define SENSEDATA_SK_COPY_ABORTED		0x0A
#define SENSEDATA_SK_ABORTED_COMMAND		0x0B

/* 0x0C Obsolete */

#define SENSEDATA_SK_VOLUME_OVERFLOW		0x0D
#define SENSEDATA_SK_MISCOMPARE			0x0E

/* 0x0F Reserved */


/*
 * Unit attention error codes
 */

/* Not ready to ready change, medium may have changed */
#define E_SENSE_NRTRC_MMHC	MAKE_SENSE_ERROCODE(SENSEDATA_SK_UNIT_ATTENTION, 0x28, 0x00)

/* Import or export element accessed */
#define E_SENSE_IOEEA		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_UNIT_ATTENTION, 0x28, 0x01)

/* Format - layer may have changed */
#define E_SENSE_FLMHC		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_UNIT_ATTENTION, 0x28, 0x02)

/* Power on, reset, or bus device reset occurred */
#define E_SENSE_POROBDRO	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_UNIT_ATTENTION, 0x29, 0x00)

/* Power on occurred */
#define E_SENSE_POO		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_UNIT_ATTENTION, 0x29, 0x01)

/* Bus reset occurred */
#define E_SENSE_BRO		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_UNIT_ATTENTION, 0x29, 0x02)

/* Bus device reset function occurred */
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
 * CDB or parameter validation error codes
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
#define E_SENSE_WE_RN		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_NOT_READY, 0x0C, 0x07)

/* Defects in error window */
#define E_SENSE_DIEW		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_NOT_READY, 0x0C, 0x0F)

/* Incompatible medium installed */
#define E_SENSE_IMI_2		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_NOT_READY, 0x30, 0x00)

/* Incompatible medium installed */
#define E_SENSE_IMI_5		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x30, 0x00)

/* Cannot read medium - unknown format */
#define E_SENSE_CRM_UF_2	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_NOT_READY, 0x30, 0x01)

/* Cannot read medium - unknown format */
#define E_SENSE_CRM_UF_5	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x30, 0x01)

/* Cannot read medium - incompatible format */
#define E_SENSE_CRM_IF_2	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_NOT_READY, 0x30, 0x02)

/* Cannot read medium - incompatible format */
#define E_SENSE_CRM_IF_5	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x30, 0x02)

/* Cleaning cartridge installed */
#define E_SENSE_CCI_2		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_NOT_READY, 0x30, 0x03)

/* Cleaning cartridge installed */
#define E_SENSE_CCI_5		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x30, 0x03)

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
#define E_SENSE_CF_2		MAKE_SENSE_ERRORCODE(SENSEDAT_SK_NOT_READY, 0x30, 0x07)

/* Cleaning failure */
#define E_SENSE_CF_5		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x30, 0x07)

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
 * Protocol error codes
 */

/* Command sequence error */
#define E_SENSE_CSE		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x2C, 0x00)

/* Current program area is not empty */
#define E_SENSE_CPAINE		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x2C, 0x03)

/* Current program area is empty */
#define E_SENSE_CPAIE		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x2C, 0x04)

/* Cannot write - application code mismatch */
#define E_SENSE_CW_ACM		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x30, 0x08)

/* Current session not fixated for append */
#define E_SENSE_CSNFFA		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x30, 0x09)

/* Medium not formatted */
#define E_SENSE_MNF		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x30, 0x10)

/* Saving parameters not supported */
#define E_SENSE_SPNS		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x39, 0x00)

/* Invalid bits in identify message */
#define E_SENSE_IBIIM		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x3D, 0x00)

/* Message error */
#define E_SENSE_ME		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x43, 0x00)

/* Medium removal prevented */
#define E_SENSE_MRP		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x53, 0x02)

/* Illegal mode for this track */
#define E_SENSE_IMFTT		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x64, 0x00)

/* Invalid packet size */
#define E_SENSE_IPS		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x64, 0x01)

/* Copy protection key exchange failure - authentication failure */
#define E_SENSE_CPKEF_AF	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x6F, 0x00)

/* Copy protection key exchange failure - key not present */
#define E_SENSE_CPKEF_KNP	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x6F, 0x01)

/* Copy protection key exchange failure - key not established */
#define E_SENSE_CPKEF_KNE	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x6F, 0x02)

/* Read of scrambled sector without authentication */
#define E_SENSE_ROSSWA		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x6F, 0x03)

/* Media region code is mismatched to logical unit region */
#define E_SENSE_MRCIMTLUR	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x6F, 0x04)

/* Logical unit region must be permanent, region reset count error */
#define E_SENSE_LURMBP_RRCE	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x6F, 0x05)

/* Insufficient block count for binding nonce recording */
#define E_SENSE_IBCFBNR		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x6F, 0x06)

/* Conflict in binding nonce recording */
#define E_SENSE_CIBNR		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x6F, 0x07)

/* Empty or partially written reserved track */
#define E_SENSE_EOPWRT		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x72, 0x04)

/* No more track reservations allowed */
#define E_SENSE_NMTRA		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x72, 0x05)


/*
 * General media access errors
 */

/* No reference position found */
#define E_SENSE_NRPF		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_MEDIUM_ERROR, 0x06, 0x00)

/* Track following error */
#define E_SENSE_TFE		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_HARDWARE_ERROR, 0x09, 0x00)

/* Tracking servo failure */
#define E_SENSE_TSF		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_HARDWARE_ERROR, 0x09, 0x01)

/* Focus servo failure */
#define E_SENSE_FSF		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_HARDWARE_ERROR, 0x09, 0x02)

/* Spindle servo failure */
#define E_SENSE_SSF		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_HARDWARE_ERROR, 0x09, 0x03)

/* Random positioning error */
#define E_SENSE_RPE_3		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_MEDIUM_ERROR, 0x15, 0x00)

/* Mechanical positioning error */
#define E_SENSE_MPE_3		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_MEDIUM_ERROR, 0x15, 0x01)

/* Medium format corrupted */
#define E_SENSE_MFC		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_MEDIUM_ERROR, 0x31, 0x00)

/* Format command failed */
#define E_SENSE_FCF		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_MEDIUM_ERROR, 0x31, 0x01)

/* Zoned formatting failed due to spare linking */
#define E_SENSE_ZFFDTSL		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_MEDIUM_ERROR, 0x31, 0x02)

/* Unable to recover table-of-contents */
#define E_SENSE_UTRTOC		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_MEDIUM_ERROR, 0x57, 0x00)

/* CD control error */
#define E_SENSE_CDCE		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_MEDIUM_ERROR, 0x73, 0x00)


/*
 * Reading error codes
 */

/* Unrecovered read error */
#define E_SENSE_URE		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_MEDIUM_ERROR, 0x11, 0x00)

/* Read retries exhausted */
#define E_SENSE_RRE		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_MEDIUM_ERROR, 0x11, 0x01)

/* Error too long to correct */
#define E_SENSE_ETLTC		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_MEDIUM_ERROR, 0x11, 0x02)

/* L-EC uncorrectable error */
#define E_SENSE_LECUE		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_MEDIUM_ERROR, 0x11, 0x05)

/* CIRC unrecovered error */
#define E_SENSE_CIRCUE		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_MEDIUM_ERROR, 0x11, 0x06)

/* Error reading UPC/EAN number */
#define E_SENSE_ER_UPC_EAN_N	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_MEDIUM_ERROR, 0x11, 0x0F)

/* Error reading ISRC number */
#define E_SENSE_ER_ISRC_N	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_MEDIUM_ERROR, 0x11, 0x10)

/* Read error - loss of streaming */
#define E_SENSE_RE_LOS		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ABORTED_COMMAND, 0x11, 0x11)

/* Positioning error detected by read of medium */
#define E_SENSE_PEDBROM		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_MEDIUM_ERROR, 0x15, 0x02)

/* Recovered data with no error correction applied */
#define E_SENSE_RDWNECA		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_RECOVERED_ERROR, 0x17, 0x00)

/* Recovered data with retries */
#define E_SENSE_RDWR		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_RECOVERED_ERROR, 0x17, 0x01)

/* Recovered data with positive head offset */
#define E_SENSE_RDWPHO		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_RECOVERED_ERROR, 0x17, 0x02)

/* Recovered data with negative head offset */
#define E_SENSE_RDWNHO		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_RECOVERED_ERROR, 0x17, 0x03)

/* Recovered data with retries and/or CIRC applied */
#define E_SENSE_RDWRAOCIRCA	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_RECOVERED_ERROR, 0x17, 0x04)

/* Recovered data using previous sector ID */
#define E_SENSE_RDUPSID		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_RECOVERED_ERROR, 0x17, 0x05)

/* Recovered data without ECC - recommend reassignment */
#define E_SENSE_RDWOECC_RR	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_RECOVERED_ERROR, 0x17, 0x07)

/* Recovered data without ECC - recommend rewrite */
#define E_SENSE_RDWOECC_RRW	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_RECOVERED_ERROR, 0x17, 0x08)

/* Recovered data without ECC - data rewritten */
#define E_SENSE_RDWOECC_DRW	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_RECOVERED_ERROR, 0x17, 0x09)

/* Recovered data with error correction applied */
#define E_SENSE_RDWECA		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_RECOVERED_ERROR, 0x18, 0x00)

/* Recovered data with error correction and retries applied */
#define E_SENSE_RDWECARA	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_RECOVERED_ERROR, 0x18, 0x01)

/* Recovered data - data auto-reallocated */
#define E_SENSE_RD_DAR		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_RECOVERED_ERROR, 0x18, 0x02)

/* Recovered data with CIRC */
#define E_SENSE_RDWCIRC		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_RECOVERED_ERROR, 0x18, 0x03)

/* Recovered data with L-EC */
#define E_SENSE_RDWLEC		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_RECOVERED_ERROR, 0x18, 0x04)

/* Recovered data - recommend reassignment */
#define E_SENSE_RD_RR		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_RECOVERED_ERROR, 0x18, 0x05)

/* Recovered data - recommend rewrite */
#define E_SENSE_RD_RRW		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_RECOVERED_ERROR, 0x18, 0x06)

/* Recovered data with linking */
#define E_SENSE_RDWL		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_RECOVERED_ERROR, 0x18, 0x08)

/* Blank check */
#define E_SENSE_BLANKCHECK	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_BLANK_CHECK, 0x00, 0x00)


/*
 * Writing error codes
 */

/* Write error */
#define E_SENSE_WE_3		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_MEDIUM_ERROR, 0x0C, 0x00)

/* Write error - recovery failed */
#define E_SENSE_WE_RF		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_MEDIUM_ERROR, 0x0C, 0x08)

/* Write error - loss of streaming */
#define E_SENSE_WE_LOS		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_MEDIUM_ERROR, 0x0C, 0x09)

/* Write error - padding blocks added */
#define E_SENSE_WE_PBA		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_MEDIUM_ERROR, 0x0C, 0x0A)

/* Write protected */
#define E_SENSE_WP		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_DATA_PROTECT, 0x27, 0x00)

/* Hardware write protected */
#define E_SENSE_HWWP		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_DATA_PROTECT, 0x27, 0x01)

/* Logical unit software write protected */
#define E_SENSE_LUSWP		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_DATA_PROTECT, 0x27, 0x02)

/* Associated write protect */
#define E_SENSE_AWP		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_DATA_PROTECT, 0x27, 0x03)

/* Persistent write protect */
#define E_SENSE_PWP		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_DATA_PROTECT, 0x27, 0x04)

/* Permanent write protect */
#define E_SENSE_PERMWP		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_DATA_PROTECT, 0x27, 0x05)

/* Conditional write protect */
#define E_SENSE_CWP		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_DATA_PROTECT, 0x27, 0x06)

/* No defect spare location available */
#define E_SENSE_NDSLA		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_MEDIUM_ERROR, 0x32, 0x00)

/* Erase failure */
#define E_SENSE_ERASEF		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_MEDIUM_ERROR, 0x51, 0x00)

/* Erase failure - incomplete erase operation detected */
#define E_SENSE_ERASEF_IEOD	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_MEDIUM_ERROR, 0x51, 0x01)

/* Failure prediction threshold exceeded */
#define E_SENSE_FPTE_1		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_RECOVERED_ERROR, 0x5D, 0x00)

/* Failure prediction threshold exceeded */
#define E_SENSE_FPTE_3		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_MEDIUM_ERROR, 0x5D, 0x00)

/* Media failure prediction threshold exceeded */
#define E_SENSE_MFPTE_1		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_RECOVERED_ERROR, 0x5D, 0x01)

/* Media failure prediction threshold exceeded */
#define E_SENSE_MFPTE_3		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_MEDIUM_ERROR, 0x5D, 0x01)

/* Logical unit failure prediction threshold exceeded */
#define E_SENSE_LUFPTE_1	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_RECOVERED_ERROR, 0x5D, 0x02)

/* Logical unit failure prediction threshold exceeded */
#define E_SENSE_LUFPTE_3	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_MEDIUM_ERROR, 0x5D, 0x02)

/* Failure prediction threshold exceeded - predicted spare area exhaustion */
#define E_SENSE_FPTE_PSAE_1	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_RECOVERED_ERROR, 0x5D, 0x03)

/* Failure prediction threshold exceeded - predicted spare area exhaustion */
#define E_SENSE_FPTE_PSAE_3	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_MEDIUM_ERROR, 0x5D, 0x03)

/* Failure prediction threshold exceeded (FALSE) */
#define E_SENSE_FPTE_FALSE	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_RECOVERED_ERROR, 0x5D, 0xFF)

/* Session fixation error */
#define E_SENSE_SFE		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_MEDIUM_ERROR, 0x72, 0x00)

/* Session fixation error writing lead-in */
#define E_SENSE_SFEWLEADIN	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_MEDIUM_ERROR, 0x72, 0x01)

/* Session fixation error writing lead-out */
#define E_SENSE_SFEWLEADOUT	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_MEDIUM_ERROR, 0x72, 0x02)

/* Session fixation error - incomplete track in session */
#define E_SENSE_SFE_ITIS	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x72, 0x03)

/* Empty or partially written reserved track */
#define E_SENSE_EOPWRT		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x72, 0x04)

/* No more track reservations allowed */
#define E_SENSE_NMTRA		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x72, 0x05)

/* RMZ extension is not allowed */
#define E_SENSE_RMZINA		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x72, 0x06)

/* No more test zone extensions are allowed */
#define E_SENSE_NMTZEAA		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x72, 0x07)

/* Power calibration area almost full */
#define E_SENSE_PCAAF		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_RECOVERED_ERROR, 0x73, 0x01)

/* Power calibration area is full */
#define E_SENSE_PCAIF		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_MEDIUM_ERROR, 0x73, 0x02)

/* Power calibration area error */
#define E_SENSE_PCAE		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_MEDIUM_ERROR, 0x73, 0x03)

/* Program memory area update failure */
#define E_SENSE_PMAUF		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_MEDIUM_ERROR, 0x73, 0x04)

/* Program memory area is full */
#define E_SENSE_PMAIF		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_MEDIUM_ERROR, 0x73, 0x05)

/* RMA/PMA is almost full */
#define E_SENSE_RMA_PMA_IAF	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_RECOVERED_ERROR, 0x73, 0x06)

/* Current power calibration area is almost full */
#define E_SENSE_CPCAIAF		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_MEDIUM_ERROR, 0x73, 0x10)

/* Current power calibration area is full */
#define E_SENSE_CPCAIF		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_MEDIUM_ERROR, 0x73, 0x11)

/* RDZ is full */
#define E_SENSE_RDZIF		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x73, 0x17)


/*
 * Hardware failure codes
 */

/* Cleaning requested */
#define E_SENSE_CR		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_HARDWARE_ERROR, 0x00, 0x17)

/* Logical unit does not respond to selection */
#define E_SENSE_LUDNRTS		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_HARDWARE_ERROR, 0x05, 0x00)

/* Logical unit communication failure */
#define E_SSENSE_LUCF		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_HARDWARE_ERROR, 0x08, 0x00)

/* Logical unit communication timeout */
#define E_SENSE_LUCT		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_HARDWARE_ERROR, 0x08, 0x01)

/* Logical unit communication parity error */
#define E_SENSE_LUCPE		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_HARDWARE_ERROR, 0x08, 0x02)

/* Logical unit communication CRC error (Ultra-DMA/32) */
#define E_SENSE_LUCCRCEUDMA32	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_HARDWARE_ERROR, 0x08, 0x03)

/* Head select fault */
#define E_SENSE_HSF		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_HARDWARE_ERROR, 0x09, 0x04)

/* Random positioning error */
#define E_SENSE_RPE_4		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_HARDWARE_ERROR, 0x15, 0x00)

/* Mechanical positioning error */
#define E_SENSE_MPE_4		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_HARDWARE_ERROR, 0x15, 0x01)

/* Synchronous data transfer error */
#define E_SENSE_SDTE		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_HARDWARE_ERROR, 0x1B, 0x00)

/* Mechanical positioning or changer error */
#define E_SENSE_MPOCE		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_HARDWARE_ERROR, 0x3B, 0x16)

/* Logical unit failure */
#define E_SENSE_LUF		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_HARDWARE_ERROR, 0x3E, 0x01)

/* Timeout on logical unit */
#define E_SENSE_TOLU		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_HARDWARE_ERROR, 0x3E, 0x02)

/* Diagnostic failure on component NN (0x80 - 0xFF) */
#define E_SENSE_DFOCNN		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_HARDWARE_ERROR, 0x40, 0xFF)

/* Internal target failure */
#define E_SENSE_ITF		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_HARDWARE_ERROR, 0x44, 0x00)

/* Unsuccessful soft reset */
#define E_SENSE_USR		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_HARDWARE_ERROR, 0x46, 0x00)

/* SCSI parity error */
#define E_SENSE_SCSIPE		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_HARDWARE_ERROR, 0x47, 0x00)

/* Command phase error */
#define E_SENSE_CPE		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_HARDWARE_ERROR, 0x4A, 0x00)

/* Data phase error */
#define E_SENSE_DPE		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_HARDWARE_ERROR, 0x4B, 0x00)

/* Logical unit failed self-configuration */
#define E_SENSE_LUFSC		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_HARDWARE_ERROR, 0x4C, 0x00)

/* Media load or eject failed */
#define E_SENSE_MLOEF		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_HARDWARE_ERROR, 0x53, 0x00)

/* Voltage fault */
#define E_SENSE_VF		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_HARDWARE_ERROR, 0x65, 0x00)

/*
 * Errors Associated with non-ATAPI Environments
 */

/* I/O process terminated */
#define E_SENSE_IOPT		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ABORTED_COMMAND, 0x00, 0x06)

/* Multiple peripheral device selected */
#define E_SENSE_MPDS		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x07, 0x00)

/* Warning */
#define E_SENSE_WARNING		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_RECOVERED_ERROR, 0x0B, 0x00)

/* Warning - Specified temperature exceeded */
#define E_SENSE_WARNING_STE	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_RECOVERED_ERROR, 0x0B, 0x01)

/* Warning - Enclosure degraded */
#define E_SENSE_WARNING_ED	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_RECOVERED_ERROR, 0x0B, 0x02)

/* Logical unit not supported */
#define E_SENSE_LUNS		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x25, 0x00)

/* Reservations preempted */
#define E_SENSE_RESP		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_UNIT_ATTENTION, 0x2A, 0x03)

/* Copy cannot execute since initiator cannot disconnect */
#define E_SENSE_CCESICD		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x2B, 0x00)

/* Commands cleared by another initiator */
#define E_SENSE_CCBAI		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_UNIT_ATTENTION, 0x2F, 0x00)

/* Enclosure failure */
#define E_SENSE_ENCLOSUREF	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_NO_SENSE, 0x34, 0x00)

/* Enclosure services failure */
#define E_SENSE_ESF		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_NO_SENSE, 0x35, 0x00)

/* Unsupported enclosure function */
#define E_SENSE_UEF		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_NO_SENSE, 0x35, 0x01)

/* Enclosure services unavailable */
#define E_SENSE_ESU		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_NO_SENSE, 0x35, 0x02)

/* Enclosure services transfer failure */
#define E_SENSE_ESTF		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_NO_SENSE, 0x35, 0x03)

/* Enclosure services transfer refused */
#define E_SENSE_ESTR		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_NO_SENSE, 0x35, 0x04)

/* Select or reselect failure */
#define E_SENSE_SORF		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ABORTED_COMMAND, 0x45, 0x00)

/* Initiator detected error message received */
#define E_SENSE_IDEMR		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ABORTED_COMMAND, 0x48, 0x00)

/* Invalid message error */
#define E_SENSE_IME		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ABORTED_COMMAND, 0x49, 0x00)

/* Tagged overlapped commands (NN = Queue tag) */
#define E_SENSE_TOC_NN		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ABORTED_COMMAND, 0x4D, 0xFF)


/*
 * Additional error codes
 */

/* No additional sense information */
#define E_SENSE_NOASI		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_NO_SENSE, 0x00, 0x00)

/* Operation in progress */
#define E_SENSE_OIP		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x00, 0x16)

/* No seek complete */
#define E_SENSE_NSC		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_MEDIUM_ERROR, 0x02, 0x00)

/* Logical unit not ready - self-test in progress */
#define E_SENSE_LUNR_STIP	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_NOT_READY, 0x04, 0x09)

/* Error log overflow */
#define E_SENSE_ELO		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_UNIT_ATTENTION, 0x0A, 0x00)

/* Warning - background self-test failed */
#define E_SENSE_WARNING_BSTF	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_RECOVERED_ERROR, 0x0B, 0x03)

/* Warning - background pre-scan detected medium error */
#define E_SENSE_WARNING_BPSDME	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_RECOVERED_ERROR, 0x0B, 0x04)

/* Warning - background medium scan detected medium error */
#define E_SENSE_WARNING_BMSDME	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_RECOVERED_ERROR, 0x0B, 0x05)

/* Write error */
#define E_SENSE_WE_1		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_RECOVERED_ERROR, 0x0C, 0x00)

/* Write error - recovered with auto-reallocation */
#define E_SENSE_WE_RWAR		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_RECOVERED_ERROR, 0x0C, 0x01)

/* Write error - auto reallocation failed */
#define E_SENSE_WE_ARF		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_RECOVERED_ERROR, 0x0C, 0x02)

/* Write error - recommend reassignment */
#define E_SENSE_WE_RR		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_RECOVERED_ERROR, 0x0C, 0x03)

/* Write error - loss of streaming */
#define E_SENSE_WE_LOS		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_MEDIUM_ERROR, 0x0C, 0x09)

/* Miscompare during verify operation */
#define E_SENSE_MDVO		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_COPY_ABORTED, 0x1D, 0x00)

/* Invalid release of persistent reservation */
#define E_SENSE_IROPR		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x26, 0x04)

/* Enclosure services checksum error */
#define E_SENSE_ESCE		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_NO_SENSE, 0x35, 0x05)

/* Rounded parameter */
#define E_SENSE_RP		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_RECOVERED_ERROR, 0x37, 0x00)

/* Medium not present - loadable */
#define E_SENSE_MNP_L		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_NOT_READY, 0x3A, 0x03)

/* Medium source element empty */
#define E_SENSE_MSEE		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_UNIT_ATTENTION, 0x3B, 0x0E)

/* Overlapped commands attempted */
#define E_SENSE_OCA		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ABORTED_COMMAND, 0x4E, 0x00)

/* System resource failure */
#define E_SENSE_SRF		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x55, 0x00)

/* Spare area exhaustion failure prediction threshold exceeded */
#define E_SENSE_SAEFPTE		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_RECOVERED_ERROR, 0x5D, 0x03)

/* End of user area encountered on this track */
#define E_SENSE_EOUAEOTT	MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x63, 0x00)

/* Packet does not fit in available space */
#define E_SENSE_PDNFIAS		MAKE_SENSE_ERRORCODE(SENSEDATA_SK_ILLEGAL_REQUEST, 0x63, 0x01)


/*
 * Sense data functions
 */

/* Parse error code from raw sense data */
extern RESULT optcl_sensedata_get_code(const uint8_t raw_data[], 
				       uint8_t size, 
				       RESULT *error_code);


#endif /* _SENSEDATA_H */
