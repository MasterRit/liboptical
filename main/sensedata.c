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

static const char* get_error_message(RESULT error_code);

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

    switch (response_code) {
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
    char *nmsg = 0;
    const char *msg = 0;

    assert(message != 0);

    if (message == 0) {
        return(E_INVALIDARG);
    }

    msg = get_error_message(error_code);

    if (msg == 0) {
        return(E_OUTOFRANGE);
    }

    nmsg = xstrdup(msg);

    if (nmsg == 0) {
        return(E_OUTOFMEMORY);
    }

    *message = nmsg;

    return(SUCCESS);
}


/*
 * Error messages table
 */

static struct error_code_entry __message_entries[] = {

    /* Unit attention error codes */

    { E_SENSE_NRTRC_MMHC,		"Not ready to ready change, medium may have changed" },
    { E_SENSE_IOEEA,		"Import or export element accessed" },
    { E_SENSE_FLMHC,		"Format - layer may have changed" },
    { E_SENSE_POROBDRO,		"Power on, reset, or bus device reset occurred" },
    { E_SENSE_POO,			"Power on occurred" },
    { E_SENSE_BRO,			"Bus reset occurred" },
    { E_SENSE_BDRFO,		"Bus device reset function occurred" },
    { E_SENSE_DIR,			"Device internal reset" },
    { E_SENSE_PC,			"Parameters changed" },
    { E_SENSE_MPC,			"Mode parameters changed" },
    { E_SENSE_LPC,			"Log parameters changed" },
    { E_SENSE_ITFO,			"Insufficient time for operation" },
    { E_SENSE_MDEF,			"Medium destination element full" },
    { E_SENSE_MSEF,			"Medium source element full" },
    { E_SENSE_EOMR,			"End of medium reached" },
    { E_SENSE_MMNA,			"Medium magazine not accessible" },
    { E_SENSE_MMR,			"Medium magazine removed" },
    { E_SENSE_MMI,			"Medium magazine inserted" },
    { E_SENSE_MML,			"Medium magazine locked" },
    { E_SENSE_MMU,			"Medium magazine unlocked" },
    { E_SENSE_TOCHC,		"Target operating conditions have changed" },
    { E_SENSE_MHBC,			"Microcode has been changed" },
    { E_SENSE_COD,			"Changed operating definition" },
    { E_SENSE_IDHC,			"Inquiry data has changed" },
    { E_SENSE_OROSCI,		"Operator request or state change input" },
    { E_SENSE_OMRR,			"Operator medium removal request" },
    { E_SENSE_OSWPROTECT,		"Operator selected write protect" },
    { E_SENSE_OSWPERMIT,		"Operator selected write permit" },
    { E_SENSE_LE,			"Log exception" },
    { E_SENSE_TCM,			"Threshold condition met" },
    { E_SENSE_LCAM,			"Log counter at maximum" },
    { E_SENSE_LLCE,			"Log list codes exhausted" },
    { E_SENSE_LPCO,			"Low power condition on" },
    { E_SENSE_ICABT,		"Idle condition activated by timer" },
    { E_SENSE_SCABT,		"Standby condition activated by timer" },
    { E_SENSE_ICABC,		"Idle condition activated by command" },
    { E_SENSE_SCABC,		"Standby condition activated by command" },

    /* CDB or parameter validation error codes */

    { E_SENSE_PLLE,			"Parameter list length error" },
    { E_SENSE_ICOC,			"Invalid command operation code" },
    { E_SENSE_LBAOOR,		"Logical block address out of range" },
    { E_SENSE_IEA,			"Invalid element address" },
    { E_SENSE_IAFW,			"Invalid address for write" },
    { E_SENSE_IWCLJ,		"Invalid write crossing layer jump" },
    { E_SENSE_IF,			"Invalid function" },
    { E_SENSE_IFICDB,		"Invalid field in CDB" },
    { E_SENSE_IFIPL,		"Invalid field in parameter list" },
    { E_SENSE_PNS,			"Parameter not supported" },
    { E_SENSE_PVI,			"Parameter value invalid" },
    { E_SENSE_TPNS,			"Threshold parameters not supported" },

    /* Readiness error codes */

    { E_SENSE_LUNR_CNR,		"Logical unit not ready, cause not reportable" },
    { E_SENSE_LUIIPOBR,		"Logical unit is in process of becoming ready" },
    { E_SENSE_LUNR_ICR,		"Logical unit not ready, initializing command required" },
    { E_SENSE_LUNR_MIR,		"Logical unit not ready, manual intervention required" },
    { E_SENSE_LUNR_FIP,		"Logical unit not ready, format in progress" },
    { E_SENSE_LUNR_OIP,		"Logical unit not ready, operation in progress" },
    { E_SENSE_LUNR_LWIP,		"Logical unit not ready, long write in progress" },
    { E_SENSE_WE_RN,		"Write error recovery needed" },
    { E_SENSE_DIEW,			"Defects in error window" },
    { E_SENSE_IMI_2,		"Incompatible medium installed" },
    { E_SENSE_IMI_5,		"Incompatible medium installed" },
    { E_SENSE_CRM_UF_2,		"Cannot read medium - unknown format" },
    { E_SENSE_CRM_UF_5,		"Cannot read medium - unknown format" },
    { E_SENSE_CRM_IF_2,		"Cannot read medium - incompatible format" },
    { E_SENSE_CRM_IF_5,		"Cannot read medium - incompatible format" },
    { E_SENSE_CCI_2,		"Cleaning cartridge installed" },
    { E_SENSE_CCI_5,		"Cleaning cartridge installed" },
    { E_SENSE_CWM_UF_2,		"Cannot write medium - unknown format" },
    { E_SENSE_CWM_UF_5,		"Cannot write medium - unknown format" },
    { E_SENSE_CWM_IF_2,		"Cannot write medium - incompatible format" },
    { E_SENSE_CWM_IF_5,		"Cannot write medium - incompatible format" },
    { E_SENSE_CFM_IM_2,		"Cannot format medium - incompatible medium" },
    { E_SENSE_CFM_IM_5,		"Cannot format medium - incompatible medium" },
    { E_SENSE_CF_2,			"Cleaning failure" },
    { E_SENSE_CF_5,			"Cleaning failure" },
    { E_SENSE_CWM_UMV_2,		"Cannot write medium - unsupported medium version" },
    { E_SENSE_CWM_UMV_5,		"Cannot write medium - unsupported medium version" },
    { E_SENSE_MNP,			"Medium not present" },
    { E_SENSE_MNP_TC,		"Medium not present - tray closed" },
    { E_SENSE_MNP_TO,		"Medium not present - tray open" },
    { E_SENSE_LUHNSCY,		"Logical unit has not self-configured yet" },

    /* Protocol error codes */

    { E_SENSE_CSE,			"Command sequence error" },
    { E_SENSE_CPAINE,		"Current program area is not empty" },
    { E_SENSE_CPAIE,		"Current program area is empty" },
    { E_SENSE_CW_ACM,		"Cannot write - application code mismatch" },
    { E_SENSE_CSNFFA,		"Current session not fixated for append" },
    { E_SENSE_MNF,			"Medium not formatted" },
    { E_SENSE_SPNS,			"Saving parameters not supported" },
    { E_SENSE_IBIIM,		"Invalid bits in identify message" },
    { E_SENSE_ME,			"Message error" },
    { E_SENSE_MRP,			"Medium removal prevented" },
    { E_SENSE_IMFTT,		"Illegal mode for this track" },
    { E_SENSE_IPS,			"Invalid packet size" },
    { E_SENSE_CPKEF_AF,		"Copy protection key exchange failure - authentication failure" },
    { E_SENSE_CPKEF_KNP,		"Copy protection key exchange failure - key not present" },
    { E_SENSE_CPKEF_KNE,		"Copy protection key exchange failure - key not established" },
    { E_SENSE_ROSSWA,		"Read of scrambled sector without authentication" },
    { E_SENSE_MRCIMTLUR,		"Media region code is mismatched to logical unit region" },
    { E_SENSE_LURMBP_RRCE,		"Logical unit region must be permanent, region reset count error" },
    { E_SENSE_IBCFBNR,		"Insufficient block count for binding nonce recording" },
    { E_SENSE_CIBNR,		"Conflict in binding nonce recording" },
    { E_SENSE_EOPWRT,		"Empty or partially written reserved track" },
    { E_SENSE_NMTRA,		"No more track reservations allowed" },
    { E_SENSE_NRPF,			"No reference position found" },
    { E_SENSE_TFE,			"Track following error" },
    { E_SENSE_TSF,			"Tracking servo failure" },
    { E_SENSE_FSF,			"Focus servo failure" },
    { E_SENSE_SSF,			"Spindle servo failure" },
    { E_SENSE_RPE_3,		"Random positioning error" },
    { E_SENSE_MPE_3,		"Mechanical positioning error" },
    { E_SENSE_MFC,			"Medium format corrupted" },
    { E_SENSE_FCF,			"Format command failed" },
    { E_SENSE_ZFFDTSL,		"Zoned formatting failed due to spare linking" },
    { E_SENSE_UTRTOC,		"Unable to recover table-of-contents" },
    { E_SENSE_CDCE,			"CD control error" },

    /* Reading error codes */

    { E_SENSE_URE,			"Unrecovered read error" },
    { E_SENSE_RRE,			"Read retries exhausted" },
    { E_SENSE_ETLTC,		"Error too long to correct" },
    { E_SENSE_LECUE,		"L-EC uncorrectable error" },
    { E_SENSE_CIRCUE,		"CIRC unrecovered error" },
    { E_SENSE_ER_UPC_EAN_N,		"Error reading UPC/EAN number" },
    { E_SENSE_ER_ISRC_N,		"Error reading ISRC number" },
    { E_SENSE_RE_LOS,		"Read error - loss of streaming" },
    { E_SENSE_PEDBROM,		"Positioning error detected by read of medium" },
    { E_SENSE_RDWNECA,		"Recovered data with no error correction applied" },
    { E_SENSE_RDWR,			"Recovered data with retries" },
    { E_SENSE_RDWPHO,		"Recovered data with positive head offset" },
    { E_SENSE_RDWNHO,		"Recovered data with negative head offset" },
    { E_SENSE_RDWRAOCIRCA,		"Recovered data with retries and/or CIRC applied" },
    { E_SENSE_RDUPSID,		"Recovered data using previous sector ID" },
    { E_SENSE_RDWOECC_RR,		"Recovered data without ECC - recommend reassignment" },
    { E_SENSE_RDWOECC_RRW,		"Recovered data without ECC - recommend rewrite" },
    { E_SENSE_RDWOECC_DRW,		"Recovered data without ECC - data rewritten" },
    { E_SENSE_RDWECA,		"Recovered data with error correction applied" },
    { E_SENSE_RDWECARA,		"Recovered data with error correction and retries applied" },
    { E_SENSE_RD_DAR,		"Recovered data - data auto-reallocated" },
    { E_SENSE_RDWCIRC,		"Recovered data with CIRC" },
    { E_SENSE_RDWLEC,		"Recovered data with L-EC" },
    { E_SENSE_RD_RR,		"Recovered data - recommend reassignment" },
    { E_SENSE_RD_RRW,		"Recovered data - recommend rewrite" },
    { E_SENSE_RDWL,			"Recovered data with linking" },
    { E_SENSE_BLANKCHECK,		"Blank check" },

    /* Writing error codes */

    { E_SENSE_WE_3,			"Write error" },
    { E_SENSE_WE_RF,		"Write error - recovery failed" },
    { E_SENSE_WE_LOS,		"Write error - loss of streaming" },
    { E_SENSE_WE_PBA,		"Write error - padding blocks added" },
    { E_SENSE_WP,			"Write protected" },
    { E_SENSE_HWWP,			"Hardware write protected" },
    { E_SENSE_LUSWP,		"Logical unit software write protected" },
    { E_SENSE_AWP,			"Associated write protect" },
    { E_SENSE_PWP,			"Persistent write protect" },
    { E_SENSE_PERMWP,		"Permanent write protect" },
    { E_SENSE_CWP,			"Conditional write protect" },
    { E_SENSE_NDSLA,		"No defect spare location available" },
    { E_SENSE_ERASEF,		"Erase failure" },
    { E_SENSE_ERASEF_IEOD,		"Erase failure - incomplete erase operation detected" },
    { E_SENSE_FPTE_1,		"Failure prediction threshold exceeded" },
    { E_SENSE_FPTE_3,		"Failure prediction threshold exceeded" },
    { E_SENSE_MFPTE_1,		"Media failure prediction threshold exceeded" },
    { E_SENSE_LUFPTE_1,		"Logical unit failure prediction threshold exceeded" },
    { E_SENSE_LUFPTE_3,		"Logical unit failure prediction threshold exceeded" },
    { E_SENSE_FPTE_PSAE_1,		"Failure prediction threshold exceeded - predicted spare area exhaustion" },
    { E_SENSE_FPTE_PSAE_3,		"Failure prediction threshold exceeded - predicted spare area exhaustion" },
    { E_SENSE_FPTE_FALSE,		"Failure prediction threshold exceeded (FALSE)" },
    { E_SENSE_SFE,			"Session fixation error" },
    { E_SENSE_SFEWLEADIN,		"Session fixation error writing lead-in" },
    { E_SENSE_SFEWLEADOUT,		"Session fixation error writing lead-out" },
    { E_SENSE_SFE_ITIS,		"Session fixation error - incomplete track in session" },
    { E_SENSE_EOPWRT,		"Empty or partially written reserved track" },
    { E_SENSE_NMTRA,		"No more track reservations allowed" },
    { E_SENSE_RMZINA,		"RMZ extension is not allowed" },
    { E_SENSE_NMTZEAA,		"No more test zone extensions are allowed" },
    { E_SENSE_PCAAF,		"Power calibration area almost full" },
    { E_SENSE_PCAIF,		"Power calibration area is full" },
    { E_SENSE_PCAE,			"Power calibration area error" },
    { E_SENSE_PMAUF,		"Program memory area update failure" },
    { E_SENSE_PMAIF,		"Program memory area is full" },
    { E_SENSE_RMA_PMA_IAF,		"RMA/PMA is almost full" },
    { E_SENSE_CPCAIAF,		"Current power calibration area is almost full" },
    { E_SENSE_CPCAIF,		"Current power calibration area is full" },
    { E_SENSE_RDZIF,		"RDZ is full" },

    /* Hardware failure codes */

    { E_SENSE_CR,			"Cleaning requested" },
    { E_SENSE_LUDNRTS,		"Logical unit does not respond to selection" },
    { E_SSENSE_LUCF,		"Logical unit communication failure" },
    { E_SENSE_LUCT,			"Logical unit communication timeout" },
    { E_SENSE_LUCPE,		"Logical unit communication parity error" },
    { E_SENSE_LUCCRCEUDMA32,	"Logical unit communication CRC error (Ultra-DMA/32)" },
    { E_SENSE_HSF,			"Head select fault" },
    { E_SENSE_RPE_4,		"Random positioning error" },
    { E_SENSE_MPE_4,		"Mechanical positioning error" },
    { E_SENSE_SDTE,			"Synchronous data transfer error" },
    { E_SENSE_MPOCE,		"Mechanical positioning or changer error" },
    { E_SENSE_LUF,			"Logical unit failure" },
    { E_SENSE_TOLU,			"Timeout on logical unit" },
    { E_SENSE_DFOCNN,		"Diagnostic failure on component NN (0x80 - 0xFF)" },
    { E_SENSE_ITF,			"Internal target failure" },
    { E_SENSE_USR,			"Unsuccessful soft reset" },
    { E_SENSE_SCSIPE,		"SCSI parity error" },
    { E_SENSE_CPE,			"Command phase error" },
    { E_SENSE_DPE,			"Data phase error" },
    { E_SENSE_LUFSC,		"Logical unit failed self-configuration" },
    { E_SENSE_MLOEF,		"Media load or eject failed" },
    { E_SENSE_VF,			"Voltage fault" },

    /* Errors Associated with non-ATAPI Environments */

    { E_SENSE_IOPT,			"I/O process terminated" },
    { E_SENSE_MPDS,			"Multiple peripheral device selected" },
    { E_SENSE_WARNING,		"Warning" },
    { E_SENSE_WARNING_STE,		"Warning - Specified temperature exceeded" },
    { E_SENSE_WARNING_ED,		"Warning - Enclosure degraded" },
    { E_SENSE_LUNS,			"Logical unit not supported" },
    { E_SENSE_RESP,			"Reservations preempted" },
    { E_SENSE_CCESICD,		"Copy cannot execute since initiator cannot disconnect" },
    { E_SENSE_CCBAI,		"Commands cleared by another initiator" },
    { E_SENSE_ENCLOSUREF,		"Enclosure failure" },
    { E_SENSE_ESF,			"Enclosure services failure" },
    { E_SENSE_UEF,			"Unsupported enclosure function" },
    { E_SENSE_ESU,			"Enclosure services unavailable" },
    { E_SENSE_ESTF,			"Enclosure services transfer failure" },
    { E_SENSE_ESTR,			"Enclosure services transfer refused" },
    { E_SENSE_SORF,			"Select or reselect failure" },
    { E_SENSE_IDEMR,		"Initiator detected error message received" },
    { E_SENSE_IME,			"Invalid message error" },
    { E_SENSE_TOC_NN,		"Tagged overlapped commands (NN = Queue tag)" },

    /* Additional error codes */

    { E_SENSE_NOASI,		"No additional sense information" },
    { E_SENSE_OIP,			"Operation in progress" },
    { E_SENSE_NSC,			"No seek complete" },
    { E_SENSE_LUNR_STIP,		"Logical unit not ready - self-test in progress" },
    { E_SENSE_ELO,			"Error log overflow" },
    { E_SENSE_WARNING_BSTF,		"Warning - background self-test failed" },
    { E_SENSE_WARNING_BPSDME,	"Warning - background pre-scan detected medium error" },
    { E_SENSE_WARNING_BMSDME,	"Warning - background medium scan detected medium error" },
    { E_SENSE_WE_1,			"Write error" },
    { E_SENSE_WE_RWAR,		"Write error - recovered with auto-reallocation" },
    { E_SENSE_WE_ARF,		"Write error - auto reallocation failed" },
    { E_SENSE_WE_RR,		"Write error - recommend reassignment" },
    { E_SENSE_WE_LOS,		"Write error - loss of streaming" },
    { E_SENSE_MDVO,			"Miscompare during verify operation" },
    { E_SENSE_IROPR,		"Invalid release of persistent reservation" },
    { E_SENSE_ESCE,			"Enclosure services checksum error" },
    { E_SENSE_RP,			"Rounded parameter" },
    { E_SENSE_MNP_L,		"Medium not present - loadable" },
    { E_SENSE_MSEE,			"Medium source element empty" },
    { E_SENSE_OCA,			"Overlapped commands attempted" },
    { E_SENSE_SRF,			"System resource failure" },
    { E_SENSE_SAEFPTE,		"Spare area exhaustion failure prediction threshold exceeded" },
    { E_SENSE_EOUAEOTT,		"End of user area encountered on this track" },
    { E_SENSE_PDNFIAS,		"Packet does not fit in available space" }
}; /* __message_entries */


/*
 * Helper functions
 */

static const char* get_error_message(RESULT error_code)
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
