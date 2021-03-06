/*
    command.c - Multi-Media Commands - 5 (MMC-5).
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

#include "adapter.h"
#include "command.h"
#include "errors.h"
#include "feature.h"
#include "helpers.h"
#include "sysdevice.h"
#include "types.h"

#include <assert.h>
#include <malloc.h>
#include <memory.h>
#include <string.h>


/*
 * MMC opcodes
 */

#define MMC_OPCODE_BLANK			        0x00A1
#define MMC_OPCODE_CLOSE_TRACK_SESSION		0x005B
#define MMC_OPCODE_FORMAT_UNIT			    0x0004
#define MMC_OPCODE_GET_CONFIG			    0x0046
#define MMC_OPCODE_GET_EVENT_STATUS		    0x004A
#define MMC_OPCODE_GET_PERFORMANCE		    0x00AC
#define MMC_OPCODE_INQUIRY			        0x0012
#define MMC_OPCODE_LOAD_UNLOAD			    0x00A6
#define MMC_OPCODE_MECHANISM_STATUS		    0x00BD
#define MMC_OPCODE_MODE_SENSE			    0x005A
#define MMC_OPCODE_MODE_SELECT			    0x0055
#define MMC_OPCODE_PREVENT_ALLOW_REMOVAL	0x001E
#define MMC_OPCODE_READ_10			        0x0028
#define MMC_OPCODE_READ_12			        0x0028
#define MMC_OPCODE_READ_BUFFER			    0x003C
#define MMC_OPCODE_READ_BUFFER_CAPACITY		0x005C
#define MMC_OPCODE_READ_CAPACITY		    0x0025
#define MMC_OPCODE_READ_CD			        0x00BE
#define MMC_OPCODE_READ_MSN			        0x00AB
#define MMC_OPCODE_READ_TRACK_INFORMATION	0x0052
#define MMC_OPCODE_REPAIR_TRACK			    0x0058
#define MMC_OPCODE_REQUEST_SENSE		    0x0003
#define MMC_OPCODE_RESERVE_TRACK		    0x0053
#define MMC_OPCODE_SEEK				        0x002B
#define MMC_OPCODE_SEND_DISC_STRUCTURE		0x00BF
#define MMC_OPCODE_SEND_OPC_INFORMATION		0x0054
#define MMC_OPCODE_SET_CD_SPEED			    0x00BB
#define MMC_OPCODE_SET_READ_AHEAD		    0x00A7
#define MMC_OPCODE_SET_STREAMING		    0x00B6
#define MMC_OPCODE_START_STOP_UNIT		    0x001B
#define MMC_OPCODE_SYNCHRONIZE_CACHE		0x0035
#define MMC_OPCODE_TEST_UNIT_READY		    0x0000
#define MMC_OPCODE_VERIFY			        0x002F
#define MMC_OPCODE_WRITE			        0x002A
#define MMC_OPCODE_WRITE_12			        0x00AA
#define MMC_OPCODE_WRITE_AND_VERIFY_10		0x002E
#define MMC_OPCODE_WRITE_BUFFER			    0x003B


/*
 * Constants used throughout the code
 */

#define READ_BLOCK_SIZE			            2048U
#define MAX_SENSEDATA_LENGTH		        252
#define MAX_GET_CONFIG_TRANSFER_LEN	        65530
#define MECHSTATUS_RESPSIZE		            1032


/*
 * Internal types
 */

typedef uint8_t	cdb6[6];
typedef uint8_t cdb10[10];
typedef uint8_t cdb12[12];

typedef RESULT (*response_deallocator)(optcl_mmc_response *response);

struct response_deallocator_entry {
    uint16_t opcode;
    response_deallocator deallocator;
};


/*
 * Forward declarations
 */

static response_deallocator get_response_deallocator(uint16_t opcode);


/*
 * Helper functions
 */

static int8_t equalfn_descriptors(const ptr_t left, const ptr_t right)
{
    optcl_feature_descriptor *ldesc = (optcl_feature_descriptor*)left;
    optcl_feature_descriptor *rdesc = (optcl_feature_descriptor*)right;
    assert(ldesc != 0);
    assert(rdesc != 0);
    if (ldesc == 0 && rdesc == 0)
        return 0;
    else if (ldesc == 0 && rdesc != 0)
        return -1;
    else if (ldesc != 0 && rdesc == 0)
        return 1;

    if (ldesc->feature_code == rdesc->feature_code)
        return 0;
    else if (ldesc->feature_code < rdesc->feature_code)
        return -1;
    else
        return 1;
}

static RESULT create_dataout_from_descriptor(const optcl_mmc_msdesc_header *descriptor,
                                             pptr_t data_out,
                                             uint16_t *data_out_len)
{
    RESULT error = SUCCESS;

    ptr_t data = 0;
    uint16_t datalen = 0;
    optcl_mmc_msdesc_mrw *mrw = 0;
    optcl_mmc_msdesc_power *power = 0;
    optcl_mmc_msdesc_caching *caching = 0;
    optcl_mmc_msdesc_vendor *vendordesc = 0;
    optcl_mmc_msdesc_rwrecovery *rwrecovery = 0;
    optcl_mmc_msdesc_writeparams *writeparms = 0;
    optcl_mmc_msdesc_timeout_protect *timeoutprot = 0;
    optcl_mmc_msdesc_infoexceptions *infoexceptions = 0;

    assert(descriptor != 0);
    assert(data_out != 0);
    assert(data_out_len != 0);
    if (descriptor == 0 || data_out == 0 || data_out_len == 0)
        return E_INVALIDARG;

    switch (descriptor->page_code) {
        case SENSE_MODEPAGE_VENDOR: {
            vendordesc = (optcl_mmc_msdesc_vendor*)descriptor;
            datalen = vendordesc->page_len + 2;
            data = (ptr_t)malloc(datalen);
            if (data == 0) {
                error = E_OUTOFMEMORY;
                break;
            }

            memset(data, 0, datalen);
            data[0] = (vendordesc->ps << 7) | SENSE_MODEPAGE_VENDOR;
            data[1] = vendordesc->page_len + 1;
            xmemcpy(&data[2], datalen, vendordesc->vendor_data, 
                vendordesc->page_len);
            break;
        }
        case SENSE_MODEPAGE_RW_ERROR: {
            rwrecovery = (optcl_mmc_msdesc_rwrecovery*)descriptor;
            datalen = 12;
            data = (ptr_t)malloc(datalen);
            if (data == 0) {
                error = E_OUTOFMEMORY;
                break;
            }

            memset(data, 0, datalen);
            data[0] = (rwrecovery->ps << 7) | SENSE_MODEPAGE_RW_ERROR;
            data[1] = 0x0A;
            data[2] = (rwrecovery->awre << 7) | (rwrecovery->arre << 6) | 
                (rwrecovery->tb << 5) | (rwrecovery->rc << 4) | 
                (rwrecovery->per << 2) | (rwrecovery->dte << 1) | 
                rwrecovery->dcr;
            data[3] = rwrecovery->read_retry_count;
            data[7] = rwrecovery->emcdr;
            data[8] = rwrecovery->write_retry_count;
            data[9] = (uint8_t)(rwrecovery->window_size >> 16);
            data[10] = (uint8_t)(rwrecovery->window_size >> 8);
            data[11] = (uint8_t)(rwrecovery->window_size);
            break;
        }
        case SENSE_MODEPAGE_MRW: {
            mrw = (optcl_mmc_msdesc_mrw*)descriptor;
            datalen = 8;
            data = (ptr_t)malloc(datalen);
            if (data == 0) {
                error = E_OUTOFMEMORY;
                break;
            }

            memset(data, 0, datalen);
            data[0] = (mrw->ps << 7) | SENSE_MODEPAGE_MRW;
            data[1] = 0x06;
            data[3] = mrw->lba_space;
            break;
        }
        case SENSE_MODEPAGE_WRITE_PARAM: {
            writeparms = (optcl_mmc_msdesc_writeparams*)descriptor;
            datalen = 52;
            data = (ptr_t)malloc(datalen);
            if (data == 0) {
                error = E_OUTOFMEMORY;
                break;
            }

            memset(data, 0, datalen);
            data[0] = (writeparms->ps << 7) | SENSE_MODEPAGE_WRITE_PARAM;
            data[1] = 0x36;
            data[2] = (writeparms->bufe << 6) | (writeparms->ls_v << 5) | 
                (writeparms->test_write << 4) | (writeparms->write_type);
            data[3] = (writeparms->multi_session << 6) | (writeparms->fp << 5) | 
                (writeparms->copy << 4) | writeparms->track_mode;
            data[4] = writeparms->dbt;
            data[5] = writeparms->link_size;
            data[7] = writeparms->hac;
            data[8] = writeparms->session_fmt;
            data[10] = (uint8_t)(writeparms->packet_size >> 24);
            data[11] = (uint8_t)(writeparms->packet_size >> 16);
            data[12] = (uint8_t)(writeparms->packet_size >> 8);
            data[13] = (uint8_t)(writeparms->packet_size);
            data[14] = (uint8_t)(writeparms->audio_pause_len >> 8);
            data[15] = (uint8_t)(writeparms->audio_pause_len);
            xmemcpy(&data[16], 16, writeparms->mcn, 16);
            xmemcpy(&data[32], 15, writeparms->isrc, 16);
            data[48] = writeparms->subheader_0;
            data[49] = writeparms->subheader_1;
            data[50] = writeparms->subheader_2;
            data[51] = writeparms->subheader_3;
            break;
        }
        case SENSE_MODEPAGE_CACHING: {
            caching = (optcl_mmc_msdesc_caching*)descriptor;
            datalen = 12;
            data = (ptr_t)malloc(datalen);
            if (data == 0) {
                error = E_OUTOFMEMORY;
                break;
            }

            memset(data, 0, datalen);
            data[0] = (caching->ps << 7) | SENSE_MODEPAGE_CACHING;
            data[1] = 0x0A;
            data[2] = (caching->wce << 2) | caching->rcd;
            break;
        }
        case SENSE_MODEPAGE_PWR_CONDITION: {
            power = (optcl_mmc_msdesc_power*)descriptor;
            datalen = 12;
            data = (ptr_t)malloc(datalen);
            if (data == 0) {
                error = E_OUTOFMEMORY;
                break;
            }

            memset(data, 0, datalen);
            data[0] = (power->ps << 7) | (power->spf << 6) | SENSE_MODEPAGE_PWR_CONDITION;
            data[1] = 0x0A;
            data[3] = (power->idle << 1) | power->standby;
            data[4] = (uint8_t)(power->idle_timer >> 24);
            data[5] = (uint8_t)(power->idle_timer >> 16);
            data[6] = (uint8_t)(power->idle_timer >> 8);
            data[7] = (uint8_t)power->idle_timer;
            data[8] = (uint8_t)(power->standby_timer >> 24);
            data[9] = (uint8_t)(power->standby_timer >> 16);
            data[10] = (uint8_t)(power->standby_timer >> 8);
            data[11] = (uint8_t)power->standby_timer;
            break;
        }
        case SENSE_MODEPAGE_INFO_EXCEPTIONS: {
            infoexceptions = (optcl_mmc_msdesc_infoexceptions*)descriptor;
            datalen = 12;
            data = (ptr_t)malloc(datalen);
            if (data == 0) {
                error = E_OUTOFMEMORY;
                break;
            }

            memset(data, 0, datalen);
            data[0] = (infoexceptions->ps << 7) | (infoexceptions->spf << 6) | 
                SENSE_MODEPAGE_INFO_EXCEPTIONS;
            data[1] = 0x0A;
            data[2] = (infoexceptions->perf << 7) | (infoexceptions->ebf << 5) | 
                (infoexceptions->ewasc << 4) | (infoexceptions->dexcpt << 3) | 
                (infoexceptions->test << 2) | infoexceptions->logerr;
            data[3] = infoexceptions->mrie;
            data[4] = (uint8_t)(infoexceptions->interval_timer >> 24);
            data[5] = (uint8_t)(infoexceptions->interval_timer >> 16);
            data[6] = (uint8_t)(infoexceptions->interval_timer >> 8);
            data[7] = (uint8_t)infoexceptions->interval_timer;
            data[8] = (uint8_t)(infoexceptions->report_count >> 24);
            data[9] = (uint8_t)(infoexceptions->report_count >> 16);
            data[10] = (uint8_t)(infoexceptions->report_count >> 8);
            data[11] = (uint8_t)infoexceptions->report_count;
            break;
        }
        case SENSE_MODEPAGE_TIMEOUT_PROTECT: {
            timeoutprot = (optcl_mmc_msdesc_timeout_protect*)descriptor;
            datalen = 12;
            data = (ptr_t)malloc(datalen);
            if (data == 0) {
                error = E_OUTOFMEMORY;
                break;
            }

            memset(data, 0, datalen);
            data[0] = (timeoutprot->ps << 7) | SENSE_MODEPAGE_INFO_EXCEPTIONS;
            data[1] = 0x0A;
            data[4] = (timeoutprot->g3enable << 3) | (timeoutprot->tmoe << 2) | 
                (timeoutprot->disp << 1) | timeoutprot->swpp;
            data[6] = (uint8_t)(timeoutprot->group1_mintimeout >> 8);
            data[7] = (uint8_t)timeoutprot->group1_mintimeout;
            data[8] = (uint8_t)(timeoutprot->group2_mintimeout >> 8);
            data[9] = (uint8_t)timeoutprot->group2_mintimeout;
            data[10] = (uint8_t)(timeoutprot->group3_mintimeout >> 8);
            data[11] = (uint8_t)timeoutprot->group3_mintimeout;
            break;
        }
        default: {
            assert(False);
            error = E_OUTOFRANGE;
            break;
        }
    }

    if (FAILED(error)) {
        free(data);
        return error;
    }

    *data_out = data;
    *data_out_len = datalen;
    return error;
}

static RESULT create_dataout_mode_select(const optcl_mmc_mode_select *command,
                                         uint32_t alignment, 
                                         pptr_t data_out, 
                                         uint16_t *data_out_len)
{
    RESULT error;

    ptr_t data = 0;
    ptr_t descdata = 0;
    uint32_t offset = 0;
    uint16_t descdatalen;
    optcl_list_iterator it = 0;
    optcl_mmc_msdesc_header *descriptor = 0;

    assert(command != 0);
    assert(data_out != 0);
    assert(data_out_len != 0);
    if (command == 0 || data_out == 0 || data_out_len == 0)
        return E_INVALIDARG;

    assert(command->descriptors != 0);
    if (command->descriptors == 0)
        return E_POINTER;

    error = optcl_list_get_head_pos(command->descriptors, &it);
    if (FAILED(error))
        return error;

    while (it != 0) {
        error = optcl_list_get_at_pos(command->descriptors, it, (const pptr_t)&descriptor);
        if (FAILED(error))
            break;

        error = create_dataout_from_descriptor(descriptor, &descdata, &descdatalen);
        if (FAILED(error))
            break;

        if (offset > (uint32_t)(MAX_UINT16 - descdatalen)) {
            error = E_OVERFLOW;
            break;
        }

        if (descdatalen > 0) {
            data = (ptr_t)xrealloc_aligned(data, offset + descdatalen, alignment);
            if (data == 0) {
                error = E_OUTOFMEMORY;
                break;
            }
        }

        xmemcpy(data + offset, descdatalen, descdata, descdatalen);
        offset += descdatalen;
        free(descdata);
    }

    if (FAILED(error)) {
        free(descdata);
        xfree_aligned(data);
        return error;
    }

    *data_out = data;
    *data_out_len = (uint16_t)offset;
    return error;
}

static RESULT create_dataout_send_disc_structure(const optcl_mmc_send_disc_structure *command,
                                                 pptr_t data_out,
                                                 uint16_t *data_out_len)
{
    RESULT error = SUCCESS;

    ptr_t data = 0;
    uint16_t datalen = 0;

    assert(command != 0);
    assert(data_out != 0);
    assert(data_out_len != 0);
    if (command == 0 || data_out == 0 || data_out_len == 0)
        return E_INVALIDARG;

    if (command->media_type == MMC_SDS_MEDIA_TYPE_DVD_HDDVD) {
        switch (command->format_type) {
            case MMC_SDS_FMT_DVD_USD: {
                datalen = command->data.user_spec_data.data_len + 4;
                assert(datalen <= 2052);
                if (datalen > 2052) {
                    error = E_SIZEMISMATCH;
                    break;
                }

                data = (ptr_t)malloc(datalen);
                if (data == 0) {
                    error = E_OUTOFMEMORY;
                    break;
                }

                memset(data, 0, datalen);
                data[0] = (uint8_t)((datalen - 2) >> 8);
                data[1] = (uint8_t)(datalen - 2);
                xmemcpy(&data[4], command->data.user_spec_data.data_len, 
                    command->data.user_spec_data.data, 
                    command->data.user_spec_data.data_len);
                break;
            }
            case MMC_SDS_FMT_DVD_CM: {
                datalen = 8;
                data = (ptr_t)malloc(datalen);
                if (data == 0) {
                    error = E_OUTOFMEMORY;
                    break;
                }

                memset(data, 0, datalen);
                data[0] = (uint8_t)((datalen - 2) >> 8);
                data[1] = (uint8_t)(datalen - 2);
                data[4] = (uint8_t)((command->data.copyright_mngmt.cpm << 7 | 
                    command->data.copyright_mngmt.cgms << 4 | 
                    command->data.copyright_mngmt.adp_ty << 2));
                break;
            }
            case MMC_SDS_FMT_DVD_TIMESTAMP: {
                datalen = 22;
                data = (ptr_t)malloc(datalen);
                if (data == 0) {
                    error = E_OUTOFMEMORY;
                    break;
                }

                memset(data, 0, datalen);
                data[0] = (uint8_t)((datalen - 2) >> 8);
                data[1] = (uint8_t)(datalen - 2);
                data[8] = (uint8_t)(command->data.timestamp.year >> 24);
                data[9] = (uint8_t)(command->data.timestamp.year >> 16);
                data[10] = (uint8_t)(command->data.timestamp.year >> 8);
                data[11] = (uint8_t)(command->data.timestamp.year);
                data[12] = (uint8_t)(command->data.timestamp.month >> 8);
                data[13] = (uint8_t)(command->data.timestamp.month);
                data[14] = (uint8_t)(command->data.timestamp.day >> 8);
                data[15] = (uint8_t)(command->data.timestamp.day);
                data[16] = (uint8_t)(command->data.timestamp.hour >> 8);
                data[17] = (uint8_t)(command->data.timestamp.hour);
                data[18] = (uint8_t)(command->data.timestamp.minute >> 8);
                data[19] = (uint8_t)(command->data.timestamp.minute);
                data[20] = (uint8_t)(command->data.timestamp.second >> 8);
                data[21] = (uint8_t)(command->data.timestamp.second);
                break;
            }
            case MMC_SDS_FMT_DVD_LBI: {
                datalen = 12;
                data = (ptr_t)malloc(datalen);
                if (data == 0) {
                    error = E_OUTOFMEMORY;
                    break;
                }

                memset(data, 0, datalen);
                data[0] = (uint8_t)((datalen - 2) >> 8);
                data[1] = (uint8_t)(datalen - 2);
                data[8] = (uint8_t)(command->data.lbi.l0_area_capacity >> 24);
                data[9] = (uint8_t)(command->data.lbi.l0_area_capacity >> 16);
                data[10] = (uint8_t)(command->data.lbi.l0_area_capacity >> 8);
                data[11] = (uint8_t)(command->data.lbi.l0_area_capacity);
                break;
            }
            case MMC_SDS_FMT_DVD_SMASA: {
                datalen = 12;
                data = (ptr_t)malloc(datalen);
                if (data == 0) {
                    error = E_OUTOFMEMORY;
                    break;
                }

                memset(data, 0, datalen);
                data[0] = (uint8_t)((datalen - 2) >> 8);
                data[1] = (uint8_t)(datalen - 2);
                data[8] = (uint8_t)(command->data.smasa.smasa >> 24);
                data[9] = (uint8_t)(command->data.smasa.smasa >> 16);
                data[10] = (uint8_t)(command->data.smasa.smasa >> 8);
                data[11] = (uint8_t)(command->data.smasa.smasa);
                break;
            }
            case MMC_SDS_FMT_DVD_JIS: {
                datalen = 12;
                data = (ptr_t)malloc(datalen);
                if (data == 0) {
                    error = E_OUTOFMEMORY;
                    break;
                }

                memset(data, 0, datalen);
                data[0] = (uint8_t)((datalen - 2) >> 8);
                data[1] = (uint8_t)(datalen - 2);
                data[8] = (uint8_t)(command->data.jis.jis >> 24);
                data[9] = (uint8_t)(command->data.jis.jis >> 16);
                data[10] = (uint8_t)(command->data.jis.jis >> 8);
                data[11] = (uint8_t)(command->data.jis.jis);
                break;
            }
            case MMC_SDS_FMT_DVD_MLJA: {
                datalen = 12;
                data = (ptr_t)malloc(datalen);
                if (data == 0) {
                    error = E_OUTOFMEMORY;
                    break;
                }

                memset(data, 0, datalen);
                data[0] = (uint8_t)((datalen - 2) >> 8);
                data[1] = (uint8_t)(datalen - 2);
                data[8] = (uint8_t)(command->data.mlja.ljlba >> 24);
                data[9] = (uint8_t)(command->data.mlja.ljlba >> 16);
                data[10] = (uint8_t)(command->data.mlja.ljlba >> 8);
                data[11] = (uint8_t)(command->data.mlja.ljlba);
                break;
            }
            case MMC_SDS_FMT_DVD_RA: {
                datalen = 12;
                data = (ptr_t)malloc(datalen);
                if (data == 0) {
                    error = E_OUTOFMEMORY;
                    break;
                }

                memset(data, 0, datalen);
                data[0] = (uint8_t)((datalen - 2) >> 8);
                data[1] = (uint8_t)(datalen - 2);
                data[4] = (uint8_t)(command->data.remapping_address.apn >> 8);
                data[5] = (uint8_t)(command->data.remapping_address.apn);
                data[8] = (uint8_t)(
                    command->data.remapping_address.remapping_address >> 24);
                data[9] = (uint8_t)(
                    command->data.remapping_address.remapping_address >> 16);
                data[10] = (uint8_t)(
                    command->data.remapping_address.remapping_address >> 8);
                data[11] = (uint8_t)(
                    command->data.remapping_address.remapping_address);
                break;
            }
            case MMC_SDS_FMT_DVD_DCB: {
                datalen = command->data.dcb.dcb_len + 4;
                assert(datalen <= 32771);
                if (datalen > 32771) {
                    error = E_SIZEMISMATCH;
                    break;
                }

                data = (ptr_t)malloc(datalen);
                if (data == 0) {
                    error = E_OUTOFMEMORY;
                    break;
                }

                memset(data, 0, datalen);
                data[0] = (uint8_t)((datalen - 2) >> 8);
                data[1] = (uint8_t)(datalen - 2);
                data[2] = (uint8_t)(command->data.dcb.erase);

                xmemcpy(&data[4], command->data.dcb.dcb_len, 
                    command->data.dcb.dcb, command->data.dcb.dcb_len);
                break;
            }
            case MMC_SDS_FMT_DVD_WP: {
                datalen = 8;
                data = (ptr_t)malloc(datalen);
                if (data == 0) {
                    error = E_OUTOFMEMORY;
                    break;
                }

                memset(data, 0, datalen);
                data[0] = (uint8_t)((datalen - 2) >> 8);
                data[1] = (uint8_t)(datalen - 2);
                data[4] = (uint8_t)(command->data.write_protection.pwp << 1);
                break;
            }
            default: {
                assert(False);
                error = E_OUTOFRANGE;
                break;
            }
        }
    } else if (command->media_type == MMC_SDS_MEDIA_TYPE_BD) {
        switch (command->format_type) {
            case MMC_SDS_FMT_BD_TIMESTAMP: {
                datalen = 22;
                data = (ptr_t)malloc(datalen);
                if (data == 0) {
                    error = E_OUTOFMEMORY;
                    break;
                }

                memset(data, 0, datalen);
                data[0] = (uint8_t)((datalen - 2) >> 8);
                data[1] = (uint8_t)(datalen - 2);
                data[8] = (uint8_t)(command->data.timestamp.year >> 24);
                data[9] = (uint8_t)(command->data.timestamp.year >> 16);
                data[10] = (uint8_t)(command->data.timestamp.year >> 8);
                data[11] = (uint8_t)(command->data.timestamp.year);
                data[12] = (uint8_t)(command->data.timestamp.month >> 8);
                data[13] = (uint8_t)(command->data.timestamp.month);
                data[14] = (uint8_t)(command->data.timestamp.day >> 8);
                data[15] = (uint8_t)(command->data.timestamp.day);
                data[16] = (uint8_t)(command->data.timestamp.hour >> 8);
                data[17] = (uint8_t)(command->data.timestamp.hour);
                data[18] = (uint8_t)(command->data.timestamp.minute >> 8);
                data[19] = (uint8_t)(command->data.timestamp.minute);
                data[20] = (uint8_t)(command->data.timestamp.second >> 8);
                data[21] = (uint8_t)(command->data.timestamp.second);
                break;
            }
            case MMC_SDS_FMT_BD_PAC: {
                switch (command->pac_type) {
                    case PAC_GENERAL: {
                        assert(command->data.send_pac.pac_header_len <= 384);
                        if (command->data.send_pac.pac_header_len > 384) {
                            error = E_SIZEMISMATCH;
                            break;
                        }

                        datalen = command->data.send_pac.pac_header_len + 
                            command->data.send_pac.pac_info_len + 4;
                        data = (ptr_t)malloc(datalen);
                        if (data == 0) {
                            error = E_OUTOFMEMORY;
                            break;
                        }

                        memset(data, 0, datalen);
                        data[0] = (uint8_t)((datalen - 2) >> 8);
                        data[1] = (uint8_t)(datalen - 2);
                        data[2] = (uint8_t)(command->data.send_pac.erase);
                        xmemcpy(&data[4], command->data.send_pac.pac_header_len, 
                            command->data.send_pac.pac_header, 
                            command->data.send_pac.pac_header_len);
                        xmemcpy(&data[388], command->data.send_pac.pac_info_len,
                                command->data.send_pac.pac_info,
                                command->data.send_pac.pac_info_len);
                        break;
                    }
                    case PAC_DWP: {
                        assert(command->data.send_pac_dwp.pac_header_len <= 384);
                        if (command->data.send_pac_dwp.pac_header_len > 384) {
                            error = E_SIZEMISMATCH;
                            break;
                        }

                        datalen = 432;
                        data = (ptr_t)malloc(datalen);
                        if (data == 0) {
                            error = E_OUTOFMEMORY;
                            break;
                        }

                        memset(data, 0, datalen);
                        data[0] = (uint8_t)((datalen - 2) >> 8);
                        data[1] = (uint8_t)(datalen - 2);
                        data[2] = (uint8_t)(command->data.send_pac_dwp.erase | 
                            command->data.send_pac_dwp.vwe << 1);
                        xmemcpy(&data[4], 
                                command->data.send_pac_dwp.pac_header_len,
                                command->data.send_pac_dwp.pac_header,
                                command->data.send_pac_dwp.pac_header_len);
                        data[388] = command->data.send_pac_dwp.kpedf;
                        data[392] = command->data.send_pac_dwp.wpcb;
                        xmemcpy(&data[400], 
                                sizeof(command->data.send_pac_dwp.wp_password),
                                command->data.send_pac_dwp.wp_password,
                                sizeof(command->data.send_pac_dwp.wp_password));
                        break;
                    }
                    default: {
                        assert(False);
                        error = E_OUTOFRANGE;
                        break;
                    }
                }
                break;
            }
            default: {
                assert(False);
                error = E_OUTOFRANGE;
                break;
            }
        }
    } else {
        assert(False);
        return E_OUTOFRANGE;
    }

    if (FAILED(error)) {
        free(data);
        return error;
    }

    *data_out = data;
    *data_out_len = datalen;
    return error;
}

static RESULT create_dataout_write_buffer(const optcl_mmc_write_buffer *command,
                                          pptr_t data_out,
                                          uint32_t *data_out_len)
{
    RESULT error = SUCCESS;

    ptr_t data = 0;
    uint32_t datalen = 0;

    assert(command != 0);
    assert(data_out != 0);
    assert(data_out_len != 0);
    if (command == 0 || data_out == 0 || data_out_len == 0)
        return E_INVALIDARG;

    switch (command->mode) {
        case MMC_WRITE_BUFFER_MODE_COMBINED: {
            assert(command->dataout.combined.buffer_capacity <= MAX_UINT32 - 4);
            if (command->dataout.combined.buffer_capacity > MAX_UINT32 - 4) {
                error = E_OVERFLOW;
                break;
            }

            datalen = command->dataout.combined.buffer_capacity + 4;
            data = (ptr_t)malloc(datalen);
            if (data == 0) {
                error = E_OUTOFMEMORY;
                break;
            }

            memset(data, 0, datalen);
            data[1] = (uint8_t)(datalen >> 16);
            data[2] = (uint8_t)(datalen >> 8);
            data[3] = (uint8_t)datalen;
            xmemcpy(&data[4], datalen - 4, command->dataout.combined.buffer, 
                datalen - 4);
            break;
        }
        case MMC_WRITE_BUFFER_MODE_VENDOR: {
            assert(command->dataout.vendor.buffer_len <= MAX_UINT32);
            if (command->dataout.vendor.buffer_len > MAX_UINT32) {
                error = E_OVERFLOW;
                break;
            }

            datalen = command->dataout.vendor.buffer_len;
            data = (ptr_t)malloc(datalen);
            if (data == 0) {
                error = E_OUTOFMEMORY;
                break;
            }

            memset(data, 0, datalen);
            xmemcpy(data, datalen, command->dataout.vendor.buffer, datalen);
            break;
        }
        case MMC_WRITE_BUFFER_MODE_DATA: {
            assert(command->param_list_len <= (MAX_UINT32 >> 8));
            if (command->param_list_len > (MAX_UINT32 >> 8)) {
                error = E_OVERFLOW;
                break;
            }

            datalen = command->param_list_len;
            data = (ptr_t)malloc(datalen);
            if (data == 0)
                return E_OUTOFMEMORY;

            memset(data, 0, datalen);
            xmemcpy(data, datalen, command->dataout.data.buffer, 
                datalen);
            break;
        }
        case MMC_WRITE_BUFFER_MODE_MICROCODE:
        case MMC_WRITE_BUFFER_MODE_MICROCODE_SAVE: {
            datalen = command->dataout.microcode.microcode_len;
            data = (ptr_t)malloc(datalen);
            if (data == 0) {
                error = E_OUTOFMEMORY;
                break;
            }

            memset(data, 0, datalen);
            xmemcpy(data, datalen, command->dataout.microcode.microcode, 
                datalen);
            break;
        }
        case MMC_WRITE_BUFFER_MODE_MICROCODE_WOFF:
        case MMC_WRITE_BUFFER_MODE_MICROCODE_WOFF_SAVE: {
            datalen = command->param_list_len;
            data = (ptr_t)malloc(datalen);
            if (data == 0) {
                error = E_OUTOFMEMORY;
                break;
            }

            memset(data, 0, datalen);
            xmemcpy(data, datalen, command->dataout.microcode.microcode, 
                datalen);
            break;
        }
        case MMC_WRITE_BUFFER_MODE_ECHO: {
            datalen = command->param_list_len;
            data = (ptr_t)malloc(datalen);
            if (data == 0) {
                error = E_OUTOFMEMORY;
                break;
            }

            memset(data, 0, datalen);
            xmemcpy(data, datalen, command->dataout.echo.echo_buffer, 
                datalen);
            break;
        }
        case MMC_WRITE_BUFFER_MODE_EN_EXPANDER: {
            datalen = command->param_list_len;
            data = (ptr_t)malloc(datalen);
            if (data == 0) {
                error = E_OUTOFMEMORY;
                break;
            }

            memset(data, 0, datalen);
            xmemcpy(data, datalen, command->dataout.expander.expander_buffer, 
                datalen);
            break;
        }
        case MMC_WRITE_BUFFER_MODE_DIS_EXPANDER: {
            datalen = 0;
            data = 0;
            break;
        }
        case MMC_WRITE_BUFFER_MODE_APPLOG: {
            datalen = command->dataout.app_log_data.error_loc_len + 
                command->dataout.app_log_data.vendor_spec_len + 26;
            assert(command->param_list_len == datalen);
            if (command->param_list_len != datalen) {
                error = E_SIZEMISMATCH;
                break;
            }

            data = (ptr_t)malloc(datalen);
            if (data == 0) {
                error = E_OUTOFMEMORY;
                break;
            }

            memset(data, 0, datalen);
            xmemcpy(data, 8, command->dataout.app_log_data.t10_vendor_id, 8);
            data[8] = (uint8_t)(command->dataout.app_log_data.error_type >> 8);
            data[9] = (uint8_t)(command->dataout.app_log_data.error_type);
            xmemcpy(&data[12], 6, command->dataout.app_log_data.time_stamp, 6);
            data[20] = command->dataout.app_log_data.code_set & 0x0F;
            data[21] = command->dataout.app_log_data.error_loc_format;
            data[22] = (uint8_t)(command->dataout.app_log_data.error_loc_len >> 8);
            data[23] = (uint8_t)(command->dataout.app_log_data.error_loc_len);
            data[24] = (uint8_t)(command->dataout.app_log_data.vendor_spec_len >> 8);
            data[25] = (uint8_t)(command->dataout.app_log_data.vendor_spec_len);
            xmemcpy(&data[26], command->dataout.app_log_data.error_loc_len,
                command->dataout.app_log_data.error_location, 
                command->dataout.app_log_data.error_loc_len);
            xmemcpy(&data[command->dataout.app_log_data.error_loc_len + 26],
                command->dataout.app_log_data.vendor_spec_len,
                command->dataout.app_log_data.vendor_specific,
                command->dataout.app_log_data.vendor_spec_len);
            break;
        }
        default: {
            assert(False);
            error = E_OUTOFRANGE;
            break;
        }
    }

    if (FAILED(error)) {
        free(data);
        return error;
    }

    *data_out = data;
    *data_out_len = datalen;
    return error;
}

/*
 * Parser functions
 */

static RESULT parse_raw_event_status_descriptor_data(uint8_t event_class,
                                                     const uint8_t raw_data[],
                                                     size_t size,
                                                     optcl_mmc_ges_descriptor **descriptor)
{
    RESULT error = SUCCESS;

    optcl_mmc_ges_media *media = 0;
    optcl_mmc_ges_multihost *multihost = 0;
    optcl_mmc_ges_device_busy *devicebusy = 0;
    optcl_mmc_ges_power_management *pwrmngmnt = 0;
    optcl_mmc_ges_operational_change *opchange = 0;
    optcl_mmc_ges_external_request *exterrequest = 0;

    assert(raw_data != 0);
    assert(size >= 4);
    assert(descriptor != 0);
    if (raw_data == 0 || descriptor == 0)
        return E_INVALIDARG;

    if (size < 4)
        return E_SIZEMISMATCH;

    switch (event_class) {
        case MMC_GET_EVENT_STATUS_OPCHANGE: {
            opchange = (optcl_mmc_ges_operational_change*)
                malloc(sizeof(optcl_mmc_ges_operational_change));
            if (opchange == 0) {
                error = E_OUTOFMEMORY;
                break;
            }

            opchange->persistent_prev = bool_from_uint8(raw_data[1] & 0x80);    /* 10000000 */
            opchange->event_code = raw_data[0] & 0x15;			                /* 00001111 */
            opchange->status = raw_data[1] & 0x15;				                /* 00001111 */
            opchange->change = uint16_from_be(*(uint16_t*)&raw_data[2]);
            *descriptor = (optcl_mmc_ges_descriptor*)opchange;
            break;
        }
        case MMC_GET_EVENT_STATUS_POWERMGMT: {
            pwrmngmnt = (optcl_mmc_ges_power_management*)
                        malloc(sizeof(optcl_mmc_ges_power_management));
            if (pwrmngmnt == 0) {
                error = E_OUTOFMEMORY;
                break;
            }

            pwrmngmnt->event_code = raw_data[0] & 0x15;			                /* 00001111 */
            pwrmngmnt->power_status = raw_data[1];
            *descriptor = (optcl_mmc_ges_descriptor*)pwrmngmnt;
            break;
        }
        case MMC_GET_EVENT_STATUS_EXTREQUEST: {
            exterrequest = (optcl_mmc_ges_external_request*)
                           malloc(sizeof(optcl_mmc_ges_external_request));
            if (exterrequest == 0) {
                error = E_OUTOFMEMORY;
                break;
            }

            exterrequest->persistent_prev = bool_from_uint8(raw_data[1] & 0x80);/* 10000000 */
            exterrequest->event_code = raw_data[0] & 0x15;			            /* 00001111 */
            exterrequest->ext_req_status = raw_data[0] & 0x15;		            /* 00001111 */
            exterrequest->external_request = 
                uint16_from_be(*(uint16_t*)&raw_data[2]);
            *descriptor = (optcl_mmc_ges_descriptor*)exterrequest;
            break;
        }
        case MMC_GET_EVENT_STATUS_MEDIA: {
            media = (optcl_mmc_ges_media*)malloc(sizeof(optcl_mmc_ges_media));
            if (media == 0) {
                error = E_OUTOFMEMORY;
                break;
            }

            media->event_code = raw_data[0] & 0x15;				                /* 00001111 */
            media->media_present = bool_from_uint8(raw_data[1] & 0x02);	        /* 00000010 */
            media->tray_open = bool_from_uint8(raw_data[1] & 0x01);		        /* 00000001 */
            media->start_slot = raw_data[2];
            media->end_slot = raw_data[3];
            *descriptor = (optcl_mmc_ges_descriptor*)media;
            break;
        }
        case MMC_GET_EVENT_STATUS_MULTIHOST: {
            multihost = (optcl_mmc_ges_multihost*)
                        malloc(sizeof(optcl_mmc_ges_multihost));
            if (multihost == 0) {
                error = E_OUTOFMEMORY;
                break;
            }

            multihost->event_code = raw_data[0] & 0x15;			                /* 00001111 */
            multihost->persistent_prev = bool_from_uint8(raw_data[1] & 0x80);   /* 10000000 */
            multihost->multi_host_status = raw_data[1] & 0x15;		            /* 00001111 */
            multihost->multi_host_priority = 
                uint16_from_be(*(uint16_t*)&raw_data[2]);
            *descriptor = (optcl_mmc_ges_descriptor*)multihost;
            break;
        }
        case MMC_GET_EVENT_STATUS_DEVICEBUSY: {
            devicebusy = (optcl_mmc_ges_device_busy*)
                         malloc(sizeof(optcl_mmc_ges_device_busy));
            if (devicebusy == 0) {
                error = E_OUTOFMEMORY;
                break;
            }

            devicebusy->event_code = raw_data[0] & 0x15;			/* 00001111 */
            devicebusy->busy_status = raw_data[1];
            devicebusy->time = uint16_from_be(*(uint16_t*)&raw_data[2]);
            *descriptor = (optcl_mmc_ges_descriptor*)devicebusy;
            break;
        }
        default: {
            error = E_OUTOFRANGE;
            break;
        }
    }

    return error;
}

static RESULT parse_raw_get_configuration_data(const uint8_t mmc_response[],
                                               uint32_t size,
                                               optcl_mmc_response_get_configuration **response)
{
    RESULT error;
    RESULT destroy_error;

    uint32_t offset;
    uint16_t feature_code;
    uint8_t *raw_feature = 0;
    optcl_feature_descriptor *feature = 0;
    optcl_mmc_response_get_configuration *nresponse = 0;

    assert(response != 0);
    assert(mmc_response != 0);
    assert(size >= 8);
    assert(size % 4 == 0);
    if (mmc_response == 0 || response == 0 || size == 0 || (size % 4 != 0))
        return E_INVALIDARG;

    if (size < 8 || (size % 4) != 0)
        return E_FEATINVHEADER;

    nresponse = (optcl_mmc_response_get_configuration*)
                malloc(sizeof(optcl_mmc_response_get_configuration));

    if (nresponse == 0)
        return E_OUTOFMEMORY;

    nresponse->data_length = uint32_from_be(*(uint32_t*)&mmc_response[0]);
    nresponse->current_profile = uint16_from_be(*(uint16_t*)&mmc_response[6]);
    error = optcl_list_create(&equalfn_descriptors, &nresponse->descriptors);
    if (FAILED(error)) {
        free(nresponse);
        return error;
    }

    /*
     * Parse feature descriptor header
     */
    offset = 8;
    while (offset + 3U < size) {
        feature = 0;
        raw_feature = (uint8_t*)&mmc_response[offset];
        feature_code = uint16_from_be(*(uint16_t*)&raw_feature[0]);
        error = optcl_feature_create_from_raw(&feature, raw_feature, 
            size - offset);
        if (FAILED(error))
            break;

        if (feature == 0) {
            error = E_UNEXPECTED;
            break;
        }

        error = optcl_list_add_tail(nresponse->descriptors, (const ptr_t)feature);
        if (FAILED(error)) {
            free(feature);
            break;
        }

        /* Set next feature offset */
        offset += mmc_response[offset + 3] + 4;
    }

    if (FAILED(error)) {
        destroy_error = optcl_list_destroy(nresponse->descriptors, 1);
        free(nresponse);
        return SUCCEEDED(destroy_error) ? error : destroy_error;
    }

    *response = nresponse;
    return SUCCESS;
}

static RESULT parse_raw_get_event_status_data(const uint8_t mmc_response[],
                                              uint32_t size,
                                              optcl_mmc_response_get_event_status **response)
{
    RESULT error;
    RESULT destroy_error;

    uint16_t offset;
    uint16_t descriptor_len;
    optcl_list *descriptors = 0;
    optcl_mmc_ges_descriptor *ndescriptor = 0;
    optcl_mmc_response_get_event_status *nresponse = 0;

    assert(mmc_response != 0);
    assert(response != 0);
    assert(size >= 4);
    if (mmc_response == 0 || response == 0 || size < 4)
        return E_INVALIDARG;

    nresponse = (optcl_mmc_response_get_event_status*)
        malloc(sizeof(optcl_mmc_response_get_event_status));
    if (nresponse == 0)
        return E_OUTOFMEMORY;

    memset(nresponse, 0, sizeof(optcl_mmc_response_get_event_status));
    descriptor_len = uint16_from_be(*(uint16_t*)&mmc_response[0]);
    nresponse->ges_header.descriptor_len = descriptor_len;
    nresponse->ges_header.nea = bool_from_uint8(mmc_response[2] & 0x80);	    /* 10000000 */
    nresponse->ges_header.notification_class = mmc_response[2] & 0x07;	        /* 00000111 */
    nresponse->ges_header.event_class = mmc_response[3];
    nresponse->event_class = nresponse->ges_header.event_class;
    error = optcl_list_create(0, &descriptors);
    if (FAILED(error)) {
        free(nresponse);
        return error;
    }

    assert(descriptors);
    if (descriptors == 0) {
        free(nresponse);
        return E_POINTER;
    }

    offset = 4;
    error = SUCCESS;
    while (offset < descriptor_len + 4) {
        error = parse_raw_event_status_descriptor_data(nresponse->event_class,
            &mmc_response[offset], size - offset, &ndescriptor);
        if (FAILED(error))
            break;

        error = optcl_list_add_tail(descriptors, (const ptr_t)ndescriptor);
        if (FAILED(error))
            break;

        offset += 4;
    }

    if (FAILED(error)) {
        free(nresponse);
        destroy_error = optcl_list_destroy(descriptors, 1);
        return SUCCEEDED(destroy_error) ? error : destroy_error;
    }

    nresponse->descriptors = descriptors;
    *response = nresponse;
    return error;
}

static RESULT parse_raw_get_performance_data_pd(uint8_t data_type,
                                                const uint8_t mmc_response[],
                                                uint32_t size,
                                                optcl_mmc_response_get_performance *response)
{
    RESULT error;
    RESULT destroy_error;

    uint32_t offset = 8;
    optcl_list *descriptors = 0;
    optcl_mmc_gpdesc_pd *gpdesc = 0;

    assert(size >= 8);
    assert(mmc_response != 0);
    assert(response != 0);
    if (mmc_response == 0 || response == 0 || size >= 8)
        return E_INVALIDARG;

    error = optcl_list_create(0, &descriptors);
    if (FAILED(error))
        return error;

    assert(descriptors != 0);
    if (descriptors == 0)
        return E_POINTER;

    switch (data_type) {
        case PERFORMANCE_READ_NOMINAL:
        case PERFORMANCE_WRITE_NOMINAL:
        case PERFORMANCE_READ_ENTIRE:
        case PERFORMANCE_WRITE_ENTIRE: {
            assert((size - 8) % 16 == 0);
            if ((size - 8) % 16 != 0) {
                error = E_SIZEMISMATCH;
                break;
            }

            while (offset <= size - 16) {
                gpdesc = (optcl_mmc_gpdesc_pd*)
                    malloc(sizeof(optcl_mmc_gpdesc_pd));
                if (gpdesc == 0) {
                    error = E_OUTOFMEMORY;
                    break;
                }

                memset(gpdesc, 0, sizeof(optcl_mmc_gpdesc_pd));
                gpdesc->header.descriptor_type = MMC_GET_PERF_PERFOMANCE_DATA;
                gpdesc->data_type = data_type;
                gpdesc->nominal.start_lba = 
                    uint32_from_be(*(uint32_t*)&mmc_response[offset]);
                gpdesc->nominal.start_performance = 
                    uint32_from_be(*(uint32_t*)&mmc_response[offset + 4]);
                gpdesc->nominal.end_lba = 
                    uint32_from_be(*(uint32_t*)&mmc_response[offset + 8]);
                gpdesc->nominal.end_performance = 
                    uint32_from_be(*(uint32_t*)&mmc_response[offset + 12]);
                error = optcl_list_add_tail(descriptors, (const ptr_t)gpdesc);
                if (FAILED(error)) {
                    free(gpdesc);
                    break;
                }

                offset += 16;
            }

            break;
        }
        case PERFORMANCE_READ_EXCEPTIONS:
        case PERFORMANCE_WRITE_EXCEPTIONS: {
            assert((size - 8) % 6 == 0);
            if ((size - 8) % 6 != 0) {
                error = E_SIZEMISMATCH;
                break;
            }

            while (offset <= size - 6) {
                gpdesc = (optcl_mmc_gpdesc_pd*)
                    malloc(sizeof(optcl_mmc_gpdesc_pd));
                if (gpdesc == 0) {
                    error = E_OUTOFMEMORY;
                    break;
                }

                memset(gpdesc, 0, sizeof(optcl_mmc_gpdesc_pd));
                gpdesc->header.descriptor_type = MMC_GET_PERF_PERFOMANCE_DATA;
                gpdesc->data_type = data_type;
                gpdesc->exceptions.lba = 
                    uint32_from_be(*(uint32_t*)&mmc_response[offset]);
                gpdesc->exceptions.time = 
                    uint16_from_be(*(uint16_t*)&mmc_response[offset + 4]);
                error = optcl_list_add_tail(descriptors, (const ptr_t)gpdesc);
                if (FAILED(error)) {
                    free(gpdesc);
                    break;
                }

                offset += 6;
            }

            break;
        }
        default: {
            assert(False);
            error = E_OUTOFRANGE;
            break;
        }
    }

    if (FAILED(error)) {
        destroy_error = optcl_list_destroy(descriptors, 1);
        return SUCCEEDED(destroy_error) ? error : destroy_error;
    }

    assert(response->descriptors == 0);
    if (response->descriptors != 0) {
        destroy_error = optcl_list_destroy(descriptors, 1);
        return SUCCEEDED(destroy_error) ? E_POINTER : destroy_error;
    }

    response->descriptors = descriptors;
    return error;
}

static RESULT parse_raw_get_performance_data_uad(const uint8_t mmc_response[],
                                                 uint32_t size,
                                                 optcl_mmc_response_get_performance *response)
{
    RESULT error;
    RESULT destroy_error;

    uint32_t offset = 0;
    optcl_list *descriptors = 0;
    optcl_mmc_gpdesc_uad *gpdesc = 0;

    assert(mmc_response != 0);
    assert(response != 0);
    if (mmc_response == 0 || response == 0)
        return E_INVALIDARG;

    error = optcl_list_create(0, &descriptors);
    if (FAILED(error))
        return error;

    assert(descriptors != 0);
    if (descriptors == 0)
        return E_POINTER;

    while (offset <= size - 8) {
        gpdesc = (optcl_mmc_gpdesc_uad*)malloc(sizeof(optcl_mmc_gpdesc_uad));
        if (gpdesc == 0) {
            error = E_OUTOFMEMORY;
            break;
        }

        memset(gpdesc, 0, sizeof(optcl_mmc_gpdesc_uad));
        gpdesc->header.descriptor_type = MMC_GET_PERF_UNUSABLE_AREA_DATA;
        gpdesc->lba = uint32_from_be(*(uint32_t*)&mmc_response[offset]);
        gpdesc->upb_num = uint32_from_be(*(uint32_t*)&mmc_response[offset + 4]);
        error = optcl_list_add_tail(descriptors, (const ptr_t)gpdesc);
        if (FAILED(error)) {
            free(gpdesc);
            break;
        }

        offset += 8;
    }

    if (FAILED(error)) {
        destroy_error = optcl_list_destroy(descriptors, 1);
        return SUCCEEDED(destroy_error) ? error : destroy_error;
    }

    assert(response->descriptors == 0);
    if (response->descriptors != 0) {
        destroy_error = optcl_list_destroy(descriptors, 1);
        return SUCCEEDED(destroy_error) ? E_POINTER : destroy_error;
    }

    response->descriptors = descriptors;
    return(error);
}

static RESULT parse_raw_get_performance_data_dsd(const uint8_t mmc_response[],
                                                 uint32_t size,
                                                 optcl_mmc_response_get_performance *response)
{
    RESULT error;
    RESULT destroy_error;

    uint32_t offset = 0;
    optcl_list *descriptors = 0;
    optcl_mmc_gpdesc_dsd *gpdesc = 0;

    assert(mmc_response != 0);
    assert(response != 0);
    if (mmc_response == 0 || response == 0)
        return E_INVALIDARG;

    error = optcl_list_create(0, &descriptors);
    if (FAILED(error))
        return error;

    assert(descriptors != 0);
    if (descriptors == 0)
        return E_POINTER;

    while (offset <= size - 2048) {
        gpdesc = (optcl_mmc_gpdesc_dsd*)malloc(sizeof(optcl_mmc_gpdesc_dsd));
        if (gpdesc == 0) {
            error = E_OUTOFMEMORY;
            break;
        }

        memset(gpdesc, 0, sizeof(optcl_mmc_gpdesc_dsd));
        gpdesc->header.descriptor_type = MMC_GET_PERF_DEFECT_STATUS_DATA;
        gpdesc->start_lba = uint32_from_be(*(uint32_t*)&mmc_response[offset]);
        gpdesc->end_lba = uint32_from_be(*(uint32_t*)&mmc_response[offset + 4]);
        gpdesc->blocking_factor = mmc_response[offset + 8];
        gpdesc->fbo = mmc_response[offset + 9] & 0x07;
        xmemcpy(&gpdesc->defect_statuses[0], sizeof(gpdesc->defect_statuses),
            &mmc_response[offset + 10], sizeof(gpdesc->defect_statuses));
        error = optcl_list_add_tail(descriptors, (const ptr_t)gpdesc);
        if (FAILED(error)) {
            free(gpdesc);
            break;
        }

        offset += 2048;
    }

    if (FAILED(error)) {
        destroy_error = optcl_list_destroy(descriptors, 1);
        return SUCCEEDED(destroy_error) ? error : destroy_error;
    }

    assert(response->descriptors == 0);
    if (response->descriptors != 0) {
        destroy_error = optcl_list_destroy(descriptors, 1);
        return SUCCEEDED(destroy_error) ? E_POINTER : destroy_error;
    }

    response->descriptors = descriptors;
    return error;
}

static RESULT parse_raw_get_performance_data_wsd(const uint8_t mmc_response[],
                                                 uint32_t size,
                                                 optcl_mmc_response_get_performance *response)
{
    RESULT error;
    RESULT destroy_error;

    uint32_t offset =0;
    optcl_list *descriptors = 0;
    optcl_mmc_gpdesc_wsd *gpdesc = 0;

    assert(mmc_response != 0);
    assert(response != 0);
    if (mmc_response == 0 || response == 0)
        return E_INVALIDARG;

    error = optcl_list_create(0, &descriptors);
    if (FAILED(error))
        return error;

    assert(descriptors != 0);
    if (descriptors == 0)
        return E_POINTER;

    while (offset <= size - 16) {
        gpdesc = (optcl_mmc_gpdesc_wsd*)malloc(sizeof(optcl_mmc_gpdesc_wsd));
        if (gpdesc == 0) {
            error = E_OUTOFMEMORY;
            break;
        }

        memset(gpdesc, 0, sizeof(optcl_mmc_gpdesc_wsd));
        gpdesc->header.descriptor_type = MMC_GET_PERF_WRITE_SPEED_DESCRIPTOR;
        gpdesc->wrc = mmc_response[offset] & 0x18;
        gpdesc->rdd = bool_from_uint8(mmc_response[offset] & 0x04);
        gpdesc->exact = bool_from_uint8(mmc_response[offset] & 0x02);
        gpdesc->mrw = bool_from_uint8(mmc_response[offset] & 0x01);
        gpdesc->end_lba = uint32_from_be(*(uint32_t*)&mmc_response[offset + 4]);
        gpdesc->read_speed = uint32_from_be(*(uint32_t*)&mmc_response[offset + 8]);
        gpdesc->write_speed = uint32_from_be(*(uint32_t*)&mmc_response[offset + 12]);
        error = optcl_list_add_tail(descriptors, (const ptr_t)gpdesc);
        if (FAILED(error)) {
            free(gpdesc);
            break;
        }

        offset += 16;
    }

    if (FAILED(error)) {
        destroy_error = optcl_list_destroy(descriptors, 1);
        return SUCCEEDED(destroy_error) ? error : destroy_error;
    }

    assert(response->descriptors == 0);
    if (response->descriptors != 0) {
        destroy_error = optcl_list_destroy(descriptors, 1);
        return SUCCEEDED(destroy_error) ? E_POINTER : destroy_error;
    }

    response->descriptors = descriptors;
    return error;
}

static RESULT parse_raw_get_performance_data_dbi(const uint8_t mmc_response[],
                                                 uint32_t size,
                                                 optcl_mmc_response_get_performance *response)
{
    RESULT error;
    RESULT destroy_error;

    uint32_t offset = 8;
    optcl_list *descriptors = 0;
    optcl_mmc_gpdesc_dbi *gpdesc = 0;

    assert(size >= 8);
    assert(mmc_response != 0);
    assert(response != 0);
    if (mmc_response == 0 || response == 0 || size < 8)
        return E_INVALIDARG;

    error = optcl_list_create(0, &descriptors);
    if (FAILED(error))
        return error;

    assert(descriptors != 0);
    if (descriptors == 0)
        return E_POINTER;

    while (offset < size - 8) {
        gpdesc = (optcl_mmc_gpdesc_dbi*)malloc(sizeof(optcl_mmc_gpdesc_dbi));
        if (gpdesc == 0) {
            error = E_OUTOFMEMORY;
            break;
        }

        memset(gpdesc, 0, sizeof(optcl_mmc_gpdesc_dbi));
        gpdesc->header.descriptor_type = MMC_GET_PERF_DBI;
        gpdesc->start_lba = uint32_from_be(*(uint32_t*)&mmc_response[offset]);
        gpdesc->def_blocks_num = 
            uint16_from_be(*(uint16_t*)&mmc_response[offset + 4]);
        gpdesc->dbif = bool_from_uint8(mmc_response[offset + 6] & 0x10);
        gpdesc->error_level = mmc_response[offset + 6] & 0x0F;
        error = optcl_list_add_tail(descriptors, (const ptr_t)gpdesc);
        if (FAILED(error)) {
            free(gpdesc);
            break;
        }

        offset += 8;
    }

    if (FAILED(error)) {
        destroy_error = optcl_list_destroy(descriptors, 1);
        return SUCCEEDED(destroy_error) ? error : destroy_error;
    }

    assert(response->descriptors == 0);
    if (response->descriptors != 0) {
        destroy_error = optcl_list_destroy(descriptors, 1);
        return SUCCEEDED(destroy_error) ? E_POINTER : destroy_error;
    }

    response->descriptors = descriptors;
    return error;
}

static RESULT parse_raw_get_performance_data_dbicz(const uint8_t mmc_response[],
                                                   uint32_t size,
                                                   optcl_mmc_response_get_performance *response)
{
    RESULT error;
    RESULT destroy_error;

    uint32_t offset = 8;
    optcl_list *descriptors = 0;
    optcl_mmc_gpdesc_dbicz *gpdesc = 0;

    assert(size >= 8);
    assert(mmc_response != 0);
    assert(response != 0);
    if (mmc_response == 0 || response == 0 || size < 8)
        return E_INVALIDARG;

    error = optcl_list_create(0, &descriptors);
    if (FAILED(error))
        return error;

    assert(descriptors != 0);
    if (descriptors == 0)
        return E_POINTER;

    while (offset < size - 8) {
        gpdesc = (optcl_mmc_gpdesc_dbicz*)
            malloc(sizeof(optcl_mmc_gpdesc_dbicz));
        if (gpdesc == 0) {
            error = E_OUTOFMEMORY;
            break;
        }

        memset(gpdesc, 0, sizeof(optcl_mmc_gpdesc_dbicz));
        gpdesc->header.descriptor_type = MMC_GET_PERF_DBI;
        gpdesc->start_lba = uint32_from_be(*(uint32_t*)&mmc_response[offset]);
        error = optcl_list_add_tail(descriptors, (const ptr_t)gpdesc);
        if (FAILED(error)) {
            free(gpdesc);
            break;
        }

        offset += 8;
    }

    if (FAILED(error)) {
        destroy_error = optcl_list_destroy(descriptors, 1);
        return SUCCEEDED(destroy_error) ? error : destroy_error;
    }

    assert(response->descriptors == 0);
    if (response->descriptors != 0) {
        destroy_error = optcl_list_destroy(descriptors, 1);
        return SUCCEEDED(destroy_error) ? E_POINTER : destroy_error;
    }

    response->descriptors = descriptors;
    return error;
}

static RESULT parse_raw_get_performance_data(uint8_t type,
                                             uint8_t data_type,
                                             const uint8_t mmc_response[],
                                             uint32_t size,
                                             optcl_mmc_response_get_performance **response)
{
    RESULT error;
    optcl_mmc_response_get_performance *nresponse = 0;

    assert(mmc_response != 0);
    assert(response != 0);
    assert(size >= 8);
    if (mmc_response == 0 || response == 0 || size < 8)
        return E_INVALIDARG;

    nresponse = (optcl_mmc_response_get_performance*)
        malloc(sizeof(optcl_mmc_response_get_performance));
    if (nresponse == 0)
        return E_OUTOFMEMORY;

    memset(nresponse, 0, sizeof(optcl_mmc_response_get_performance));
    switch (type) {
        case MMC_GET_PERF_PERFOMANCE_DATA: {
            nresponse->gp_header.perf_header.perf_data_len =
                uint32_from_be(*(uint32_t*)&mmc_response[0]);
            nresponse->gp_header.perf_header.write = mmc_response[4] & 0x02;
            nresponse->gp_header.perf_header.except = mmc_response[4] & 0x01;
            error = parse_raw_get_performance_data_pd(data_type, mmc_response,
                size, nresponse);
            break;
        }
        case MMC_GET_PERF_UNUSABLE_AREA_DATA: {
            error = parse_raw_get_performance_data_uad(mmc_response, size, 
                nresponse);
            break;
        }
        case MMC_GET_PERF_DEFECT_STATUS_DATA: {
            error = parse_raw_get_performance_data_dsd(mmc_response, size,
                nresponse);
            break;
        }
        case MMC_GET_PERF_WRITE_SPEED_DESCRIPTOR: {
            error = parse_raw_get_performance_data_wsd(mmc_response, size,
                nresponse);
            break;
        }
        case MMC_GET_PERF_DBI: {
            nresponse->gp_header.dbi_header.dbi_data_len = 
                uint32_from_be(*(uint32_t*)&mmc_response[0]);
            error = parse_raw_get_performance_data_dbi(mmc_response,
                size, nresponse);
            break;
        }
        case MMC_GET_PERF_DBI_CACHE_ZONE: {
            error = parse_raw_get_performance_data_dbicz(mmc_response, size, 
                nresponse);
            break;
        }
        default: {
            assert(False);
            error = E_OUTOFRANGE;
            break;
        }
    }

    if (FAILED(error)) {
        free(nresponse);
        return error;
    }

    if (nresponse->descriptors == 0) {
        free(nresponse);
        return E_POINTER;
    }

    *response = nresponse;
    return error;
}

static RESULT parse_raw_inquiry_data(const uint8_t mmc_response[],
                                     uint32_t size,
                                     optcl_mmc_response_inquiry **response)
{
    optcl_mmc_response_inquiry *nresponse = 0;

    assert(mmc_response);
    assert(size >= 5);
    assert(response);
    if (mmc_response == 0 || response == 0 || size < 0)
        return E_INVALIDARG;

    nresponse = (optcl_mmc_response_inquiry*)
        malloc(sizeof(optcl_mmc_response_inquiry));
    if (nresponse == 0)
        return E_OUTOFMEMORY;

    memset(nresponse, 0, sizeof(optcl_mmc_response_inquiry));
    nresponse->qualifier = mmc_response[0] & 0xE0;			            /* 11100000 */
    nresponse->device_type = mmc_response[0] & 0x1F;		            /* 00011111 */
    nresponse->rmb = bool_from_uint8(mmc_response[1] & 0x80);	        /* 10000000 */
    nresponse->version = mmc_response[2];
    nresponse->normaca = mmc_response[3] & 0x20;			            /* 00100000 */
    nresponse->hisup = bool_from_uint8(mmc_response[3] & 0x10);	        /* 00010000 */
    nresponse->rdf = mmc_response[3] & 0x0F;			                /* 00001111 */
    if (size > 4)
        nresponse->additional_len = mmc_response[4];

    if (size > 5) {
        nresponse->sccs	= bool_from_uint8(mmc_response[5] & 0x80);	    /* 10000000 */
        nresponse->acc = bool_from_uint8(mmc_response[5] & 0x40);	    /* 01000000 */
        nresponse->tpgs	= mmc_response[5] & 0x30;			            /* 00110000 */
        nresponse->_3pc	= bool_from_uint8(mmc_response[5] & 0x08);	    /* 00001000 */
        nresponse->protect = bool_from_uint8(mmc_response[5] & 0x01);	/* 00000001 */
    }

    if (size > 6) {
        nresponse->bque	= bool_from_uint8(mmc_response[6] & 0x80);	    /* 10000000 */
        nresponse->encserv = bool_from_uint8(mmc_response[6] & 0x40);	/* 01000000 */
        nresponse->vs1 = bool_from_uint8(mmc_response[6] & 0x20);	    /* 00100000 */
        nresponse->multip = bool_from_uint8(mmc_response[6] & 0x10);	/* 00010000 */
        nresponse->mchngr = bool_from_uint8(mmc_response[6] & 0x08);	/* 00001000 */
        nresponse->addr16 = bool_from_uint8(mmc_response[6] & 0x01);	/* 00000001 */
    }

    if (size > 7) {
        nresponse->wbus16 = bool_from_uint8(mmc_response[7] & 0x20);	/* 00100000 */
        nresponse->sync	= bool_from_uint8(mmc_response[7] & 0x10);	    /* 00010000 */
        nresponse->linked = bool_from_uint8(mmc_response[7] & 0x08);	/* 00001000 */
        nresponse->cmdque = bool_from_uint8(mmc_response[7] & 0x02);	/* 00000010 */
        nresponse->vs2 = bool_from_uint8(mmc_response[7] & 0x01);	    /* 00000001 */
    }

    if (size > 16)
        xstrncpy((char*)nresponse->vendor, 9, (char*)&mmc_response[8], 8);

    if (size > 32)
        xstrncpy((char*)nresponse->product, 17, (char*)&mmc_response[16], 16);

    if (size > 36)
        nresponse->revision_level = uint32_from_le(*(uint32_t*)&mmc_response[32]);

    if (size > 56)
        xstrncpy((char*)nresponse->vendor_string, 21, (char*)&mmc_response[36], 20);

    if (size > 56) {
        nresponse->clocking = mmc_response[56] & 0x0C;			        /* 00001100 */
        nresponse->qas = bool_from_uint8(mmc_response[56] & 0x02);	    /* 00000010 */
        nresponse->ius = bool_from_uint8(mmc_response[56] & 0x01);	    /* 00000001 */
    }

    if (size > 60)
        nresponse->ver_desc1 = uint16_from_le(*(uint16_t*)&mmc_response[58]);

    if (size > 62)
        nresponse->ver_desc2 = uint16_from_le(*(uint16_t*)&mmc_response[60]);

    if (size > 64)
        nresponse->ver_desc3 = uint16_from_le(*(uint16_t*)&mmc_response[62]);

    if (size > 66)
        nresponse->ver_desc4 = uint16_from_le(*(uint16_t*)&mmc_response[64]);

    if (size > 68)
        nresponse->ver_desc5 = uint16_from_le(*(uint16_t*)&mmc_response[66]);

    if (size > 70)
        nresponse->ver_desc6 = uint16_from_le(*(uint16_t*)&mmc_response[68]);

    if (size > 72)
        nresponse->ver_desc7 = uint16_from_le(*(uint16_t*)&mmc_response[70]);

    if (size > 74)
        nresponse->ver_desc8 = uint16_from_le(*(uint16_t*)&mmc_response[72]);

    *response = nresponse;
    return SUCCESS;
}

static RESULT parse_raw_mode_sense_data(const uint8_t mmc_response[],
                                        uint32_t size,
                                        optcl_mmc_response_mode_sense **response)
{
    RESULT error = SUCCESS;
    RESULT destroy_error;

    uint8_t page_len;
    uint8_t page_code;
    uint32_t offset = 8;
    optcl_list *descriptors = 0;
    optcl_mmc_msdesc_mrw *mrw = 0;
    optcl_mmc_msdesc_power *power = 0;
    optcl_mmc_msdesc_caching *caching = 0;
    optcl_mmc_msdesc_vendor *vendordesc = 0;
    optcl_mmc_msdesc_rwrecovery *rwrecovery = 0;
    optcl_mmc_msdesc_writeparams *writeparms = 0;
    optcl_mmc_response_mode_sense *nresponse = 0;
    optcl_mmc_msdesc_timeout_protect *timeoutprot = 0;
    optcl_mmc_msdesc_infoexceptions *infoexceptions = 0;

    assert(mmc_response != 0);
    assert(response != 0);
    assert(size >= 8);
    if (mmc_response == 0 || response == 0 || size < 8)
        return E_INVALIDARG;

    error = optcl_list_create(0, &descriptors);
    if (FAILED(error))
        return error;

    nresponse = (optcl_mmc_response_mode_sense*)
        malloc(sizeof(optcl_mmc_response_mode_sense));
    if (nresponse == 0)
        return E_OUTOFMEMORY;

    memset(nresponse, 0, sizeof(optcl_mmc_response_mode_sense));
    nresponse->header.command_opcode = MMC_OPCODE_MODE_SENSE;
    while (offset < size) {
        page_code = mmc_response[offset] & 0x3F;
        switch (page_code) {
            case SENSE_MODEPAGE_VENDOR: {
                page_len = mmc_response[offset + 1];
                assert(page_len > 0 && page_len < 255);
                if (page_len == 0 || page_len >= 255) {
                    error = E_SIZEMISMATCH;
                    break;
                }

                assert(offset + page_len + 2 < size);
                if (offset + page_len + 2 >= size) {
                    error = E_OUTOFRANGE;
                    break;
                }

                vendordesc = (optcl_mmc_msdesc_vendor*)
                    malloc(sizeof(optcl_mmc_msdesc_vendor));
                if (vendordesc == 0) {
                    error = E_OUTOFMEMORY;
                    break;
                }

                memset(vendordesc, 0, sizeof(optcl_mmc_msdesc_vendor));
                vendordesc->header.page_code = page_code;
                vendordesc->ps = bool_from_uint8(mmc_response[offset] & 0x80);		/* 10000000 */
                vendordesc->page_len = page_len - 1;
                xmemcpy(vendordesc->vendor_data, sizeof(vendordesc->vendor_data),
                    &mmc_response[offset + 2], page_len - 1);
                error = optcl_list_add_tail(descriptors, (const ptr_t)vendordesc);
                if (FAILED(error)) {
                    free(vendordesc);
                    break;
                }

                offset += page_len + 2;
                break;
            }
            case SENSE_MODEPAGE_RW_ERROR: {
                assert(offset + 12 < size);
                if (offset + 12 >= size) {
                    error = E_OUTOFRANGE;
                    break;
                }

                rwrecovery = (optcl_mmc_msdesc_rwrecovery*)
                    malloc(sizeof(optcl_mmc_msdesc_rwrecovery));
                if (rwrecovery == 0) {
                    error = E_OUTOFMEMORY;
                    break;
                }

                memset(rwrecovery, 0, sizeof(optcl_mmc_msdesc_rwrecovery));
                rwrecovery->header.page_code = page_code;
                rwrecovery->ps = bool_from_uint8(mmc_response[offset] & 0x80);		    /* 10000000 */
                rwrecovery->awre = bool_from_uint8(mmc_response[offset + 2] & 0x80);	/* 10000000 */
                rwrecovery->arre = bool_from_uint8(mmc_response[offset + 2] & 0x40);	/* 01000000 */
                rwrecovery->tb = bool_from_uint8(mmc_response[offset + 2] & 0x20);	    /* 00100000 */
                rwrecovery->rc = bool_from_uint8(mmc_response[offset + 2] & 0x10);	    /* 00010000 */
                rwrecovery->per = bool_from_uint8(mmc_response[offset + 2] & 0x04);	    /* 00000100 */
                rwrecovery->dte = bool_from_uint8(mmc_response[offset + 2] & 0x02);	    /* 00000010 */
                rwrecovery->dcr = bool_from_uint8(mmc_response[offset + 2] & 0x01);	    /* 00000001 */
                rwrecovery->read_retry_count = mmc_response[offset + 3];
                rwrecovery->emcdr = mmc_response[offset + 7] & 0x03;			        /* 00000011 */
                rwrecovery->write_retry_count = mmc_response[offset + 8];
                rwrecovery->window_size = 
                    (uint32_t)((mmc_response[offset + 9] << 16) | 
                    (mmc_response[offset + 10] << 8) | 
                    mmc_response[offset + 11]);
                error = optcl_list_add_tail(descriptors, (const ptr_t)rwrecovery);
                if (FAILED(error)) {
                    free(rwrecovery);
                    break;
                }

                offset += 12;
                break;
            }
            case SENSE_MODEPAGE_MRW: {
                assert(offset + 8 < size);
                if (offset + 8 >= size) {
                    error = E_OUTOFRANGE;
                    break;
                }
                mrw = (optcl_mmc_msdesc_mrw*)
                    malloc(sizeof(optcl_mmc_msdesc_mrw));
                if (mrw == 0) {
                    error = E_OUTOFMEMORY;
                    break;
                }

                memset(mrw, 0, sizeof(optcl_mmc_msdesc_mrw));
                mrw->header.page_code = page_code;
                mrw->ps = bool_from_uint8(mmc_response[offset] & 0x80);             /* 10000000 */
                mrw->lba_space = bool_from_uint8(mmc_response[offset + 3] & 0x01);	/* 00000001 */
                error = optcl_list_add_tail(descriptors, (const ptr_t)mrw);
                if (FAILED(error)) {
                    free(mrw);
                    break;
                }

                offset += 8;
                break;
            }
            case SENSE_MODEPAGE_WRITE_PARAM: {
                assert(offset + 50 < size);
                if (offset + 50 >= size) {
                    error = E_OUTOFRANGE;
                    break;
                }

                writeparms = (optcl_mmc_msdesc_writeparams*)
                    malloc(sizeof(optcl_mmc_msdesc_writeparams));
                if (writeparms == 0) {
                    error = E_OUTOFMEMORY;
                    break;
                }

                memset(writeparms, 0, sizeof(optcl_mmc_msdesc_writeparams));
                page_len = mmc_response[offset + 1];
                writeparms->header.page_code = page_code;
                writeparms->ps = bool_from_uint8(mmc_response[offset] & 0x80);              /* 10000000 */
                writeparms->bufe = bool_from_uint8(mmc_response[offset + 2] & 0x40);        /* 01000000 */
                writeparms->ls_v = bool_from_uint8(mmc_response[offset + 2] & 0x20);        /* 00100000 */
                writeparms->test_write = bool_from_uint8(mmc_response[offset + 2] & 0x10);  /* 00010000 */
                writeparms->write_type = mmc_response[offset + 2] & 0x0F;                   /* 00001111 */
                writeparms->multi_session = mmc_response[offset + 3] & 0xC0;                /* 11000000 */
                writeparms->fp = mmc_response[offset + 3] & 0x20;                           /* 00100000 */
                writeparms->copy = mmc_response[offset + 3] & 0x10;                         /* 00010000 */
                writeparms->track_mode = mmc_response[offset + 3] & 0x0F;                   /* 00001111 */
                writeparms->dbt = mmc_response[offset + 4] & 0x0F;                          /* 00001111 */
                writeparms->link_size = mmc_response[offset + 5];
                writeparms->hac = mmc_response[offset + 7] & 0x3F;                          /* 00111111 */
                writeparms->session_fmt = mmc_response[offset + 8];
                writeparms->packet_size = 
                    uint32_from_be(*(uint32_t*)&mmc_response[offset + 10]);
                writeparms->audio_pause_len = 
                    uint16_from_be(*(uint16_t*)&mmc_response[offset + 14]);
                xmemcpy(writeparms->mcn, sizeof(writeparms->mcn), 
                    &mmc_response[offset + 16], 16);
                xmemcpy(writeparms->isrc, sizeof(writeparms->isrc), 
                    &mmc_response[offset + 32], 16);
                writeparms->subheader_0 = mmc_response[offset + 48];
                writeparms->subheader_1 = mmc_response[offset + 49];
                writeparms->subheader_2 = mmc_response[offset + 50];
                writeparms->subheader_3 = mmc_response[offset + 51];
                if (page_len == 0x56) {
                    xmemcpy(&writeparms->vendor_specific[0], 
                        sizeof(writeparms->vendor_specific), 
                        &mmc_response[offset + 52],
                        sizeof(writeparms->vendor_specific));
                }

                break;
            }
            case SENSE_MODEPAGE_CACHING: {
                assert(offset + 12 < size);
                if (offset + 12 >= size) {
                    error = E_OUTOFRANGE;
                    break;
                }

                caching = (optcl_mmc_msdesc_caching*)
                    malloc(sizeof(optcl_mmc_msdesc_caching));
                if (caching == 0) {
                    error = E_OUTOFMEMORY;
                    break;
                }

                memset(caching, 0, sizeof(optcl_mmc_msdesc_caching));
                page_len = mmc_response[offset + 1];
                caching->header.page_code = page_code;
                caching->ps = bool_from_uint8(mmc_response[offset] & 0x80);         /* 10000000 */
                caching->wce = bool_from_uint8(mmc_response[offset + 2] & 0x04);    /* 00000100 */
                caching->rcd = bool_from_uint8(mmc_response[offset + 2] & 0x01);    /* 00000001 */
                break;
            }
            case SENSE_MODEPAGE_PWR_CONDITION: {
                assert(offset + 12 < size);
                if (offset + 12 >= size) {
                    error = E_OUTOFRANGE;
                    break;
                }

                power = (optcl_mmc_msdesc_power*)
                    malloc(sizeof(optcl_mmc_msdesc_power));
                if (power == 0) {
                    error = E_OUTOFMEMORY;
                    break;
                }

                memset(power, 0, sizeof(optcl_mmc_msdesc_power));
                page_len = mmc_response[offset + 1];
                power->header.page_code = page_code;
                power->ps = bool_from_uint8(mmc_response[offset] & 0x80);           /* 10000000 */
                power->spf = bool_from_uint8(mmc_response[offset] & 0x40);          /* 01000000 */
                power->idle = bool_from_uint8(mmc_response[offset + 3] & 0x02);	    /* 00000010 */
                power->standby = bool_from_uint8(mmc_response[offset + 3] & 0x01);  /* 00000001 */
                power->idle_timer = 
                    uint32_from_be(*(uint32_t*)&mmc_response[offset + 4]);
                power->standby_timer = 
                    uint32_from_be(*(uint32_t*)&mmc_response[offset + 8]);
                break;
            }
            case SENSE_MODEPAGE_INFO_EXCEPTIONS: {
                assert(offset + 12 < size);
                if (offset + 12 >= size) {
                    error = E_OUTOFRANGE;
                    break;
                }
                infoexceptions = (optcl_mmc_msdesc_infoexceptions*)
                    malloc(sizeof(optcl_mmc_msdesc_infoexceptions));
                if (infoexceptions == 0) {
                    error = E_OUTOFMEMORY;
                    break;
                }

                memset(infoexceptions, 0, sizeof(optcl_mmc_msdesc_infoexceptions));
                page_len = mmc_response[offset + 1];
                infoexceptions->header.page_code = page_code;
                infoexceptions->ps = bool_from_uint8(mmc_response[offset] & 0x80);          /* 10000000 */
                infoexceptions->spf = bool_from_uint8(mmc_response[offset] & 0x40);         /* 01000000 */
                infoexceptions->perf = 
                    bool_from_uint8(mmc_response[offset + 2] & 0x80);                       /* 10000000 */
                infoexceptions->ebf = 
                    bool_from_uint8(mmc_response[offset + 2] & 0x20);                       /* 00100000 */
                infoexceptions->ewasc = 
                    bool_from_uint8(mmc_response[offset + 2] & 0x10);                       /* 00010000 */
                infoexceptions->dexcpt = 
                    bool_from_uint8(mmc_response[offset + 2] & 0x08);                       /* 00001000 */
                infoexceptions->test = 
                    bool_from_uint8(mmc_response[offset + 2] & 0x04);                       /* 00000100 */
                infoexceptions->logerr = 
                    bool_from_uint8(mmc_response[offset + 2] & 0x01);                       /* 00000001 */
                infoexceptions->mrie = mmc_response[offset + 3] & 0x0F;                     /* 00001111 */
                infoexceptions->interval_timer = 
                    uint32_from_be(*(uint32_t*)&mmc_response[offset + 4]);
                infoexceptions->report_count = 
                    uint32_from_be(*(uint32_t*)&mmc_response[offset + 8]);
                break;
            }
            case SENSE_MODEPAGE_TIMEOUT_PROTECT: {
                assert(offset + 12 < size);
                if (offset + 12 >= size) {
                    error = E_OUTOFRANGE;
                    break;
                }

                timeoutprot = (optcl_mmc_msdesc_timeout_protect*)
                    malloc(sizeof(optcl_mmc_msdesc_timeout_protect));
                if (timeoutprot == 0) {
                    error = E_OUTOFMEMORY;
                    break;
                }

                memset(timeoutprot, 0, sizeof(optcl_mmc_msdesc_timeout_protect));
                page_len = mmc_response[offset + 1];
                timeoutprot->header.page_code = page_code;
                timeoutprot->ps = bool_from_uint8(mmc_response[offset] & 0x80);             /* 10000000 */
                timeoutprot->g3enable = bool_from_uint8(mmc_response[offset + 4] & 0x08);   /* 00001000 */
                timeoutprot->tmoe = bool_from_uint8(mmc_response[offset + 4] & 0x04);       /* 00000100 */
                timeoutprot->disp = bool_from_uint8(mmc_response[offset + 4] & 0x02);       /* 00000010 */
                timeoutprot->swpp = bool_from_uint8(mmc_response[offset + 4] & 0x01);       /* 00000001 */
                break;
            }
            default: {
                assert(False);
                error = E_OUTOFRANGE;
                break;
            }
        }
    }

    if (FAILED(error)) {
        free(nresponse);
        destroy_error = optcl_list_destroy(descriptors, True);
        return SUCCEEDED(destroy_error) ? error : destroy_error;
    }

    nresponse->descriptors = descriptors;
    *response = nresponse;
    return error;
}

static RESULT parse_raw_read_buffer_data(const uint8_t mmc_response[],
                                         uint32_t size,
                                         uint8_t mode,
                                         optcl_mmc_response_read_buffer **response)
{
    RESULT error = SUCCESS;

    ptr_t data = 0;
    optcl_mmc_response_read_buffer *nresponse = 0;

    assert(size > 0);
    assert(mmc_response != 0);
    assert(response != 0);
    if (mmc_response == 0 || response == 0 || size == 0)
        return E_INVALIDARG;

    nresponse = (optcl_mmc_response_read_buffer*)
        malloc(sizeof(optcl_mmc_response_read_buffer));
    if (nresponse == 0)
        return E_OUTOFMEMORY;

    memset(nresponse, 0, sizeof(optcl_mmc_response_read_buffer));
    nresponse->mode = mode;
    switch (mode) {
        case MMC_READ_BUFFER_MODE_COMBINED: {
            assert(size >= 4);
            if (size < 4) {
                error = E_SIZEMISMATCH;
                break;
            }

            nresponse->readdata.combined.buffer_capacity = 
                (uint32_t)(mmc_response[1] << 16 | mmc_response[2] << 8 | 
                mmc_response[3]);
            if (size == 4)
                break;

            data = (ptr_t)malloc(size - 4);
            if (data == 0) {
                error = E_OUTOFMEMORY;
                break;
            }

            xmemcpy(data, size - 4, &mmc_response[4], size - 4);
            nresponse->readdata.combined.buffer = data;
            break;
        }
        case MMC_READ_BUFFER_MODE_DATA: {
            data = (ptr_t)malloc(size);
            if (data == 0) {
                error = E_OUTOFMEMORY;
                break;
            }

            xmemcpy(data, size - 4, &mmc_response[4], size - 4);
            nresponse->readdata.data.buffer_capacity =
                uint32_from_be_bytes(0, mmc_response[1], mmc_response[2], 
                    mmc_response[3]);
            nresponse->readdata.data.buffer = data;
            break;
        }
        case MMC_READ_BUFFER_MODE_DESCRIPTOR: {
            nresponse->readdata.descriptor.offset_boundary = mmc_response[0];
            nresponse->readdata.descriptor.buffer_capacity = 
                (uint32_t)(mmc_response[1] << 16 | mmc_response[2] << 8 | 
                    mmc_response[3]);
            break;
        }
        case MMC_READ_BUFFER_MODE_ECHO: {
            data = (ptr_t)malloc(size);
            if (data == 0) {
                error = E_OUTOFMEMORY;
                break;
            }

            xmemcpy(data, size, mmc_response, size);
            nresponse->readdata.echo.buffer = data;
            break;
        }
        case MMC_READ_BUFFER_MODE_ECHO_DESC: {
            nresponse->readdata.echo_desc.buffer_capacity = 
                (uint32_t)((mmc_response[2] & 0x1F) | mmc_response[3]);
            break;
        }
        case MMC_READ_BUFFER_MODE_EXPANDER: {
            data = (ptr_t)malloc(size);
            if (data == 0) {
                error = E_OUTOFMEMORY;
                break;
            }

            xmemcpy(data, size, mmc_response, size);
            nresponse->readdata.expander.buffer = data;
            break;
        }
        case MMC_READ_BUFFER_MODE_VENDOR: {
            data = (ptr_t)malloc(size);
            if (data == 0) {
                error = E_OUTOFMEMORY;
                break;
            }

            xmemcpy(data, size, mmc_response, size);
            nresponse->readdata.vendor.buffer = data;
            nresponse->readdata.vendor.buffer_len = size;
            break;
        }
        default: {
            assert(False);
            free(nresponse);
            return E_OUTOFRANGE;
        }
    }

    *response = nresponse;
    return error;
}


/*
 * Command functions
 */

RESULT optcl_command_blank(const optcl_device *device,
                           const optcl_mmc_blank *command)
{
    cdb12 cdb;

    assert(device != 0);
    assert(command != 0);
    if (device == 0 || command == 0)
        return E_INVALIDARG;

    /*
     * Execute command
     */
    memset(cdb, 0, sizeof(cdb));
    cdb[0] = MMC_OPCODE_BLANK;
    cdb[1] = (command->immed << 4) || command->blanking_type;
    cdb[2] = (uint8_t)(command->start_address >> 24);
    cdb[3] = (uint8_t)((command->start_address << 8) >> 24);
    cdb[4] = (uint8_t)((command->start_address << 16) >> 24);
    cdb[5] = (uint8_t)((command->start_address << 24) >> 24);
    return optcl_device_command_execute(device, cdb, sizeof(cdb), 0, 0);
}

RESULT optcl_command_close_track_session(const optcl_device *device,
                                         const optcl_mmc_close_track_session *command)
{
    cdb10 cdb;

    assert(device != 0);
    assert(command != 0);
    if (device == 0 || command == 0)
        return E_INVALIDARG;

    /*
     * Execute command
     */
    memset(&cdb[0], 0, sizeof(cdb));
    cdb[0] = MMC_OPCODE_CLOSE_TRACK_SESSION;
    cdb[1] = command->immed;
    cdb[2] = command->close_function & 0x07;
    cdb[4] = (uint8_t)(command->logical_track_number >> 8);
    cdb[5] = (uint8_t)command->logical_track_number;
    return optcl_device_command_execute(device, cdb, sizeof(cdb), 0, 0);
}

RESULT optcl_command_destroy_response(optcl_mmc_response *response)
{
    response_deallocator deallocator = 0;
    assert(response != 0);
    if (response == 0)
        return SUCCESS;

    deallocator = get_response_deallocator(response->command_opcode);
    assert(deallocator != 0);
    if (deallocator == 0)
        return E_OUTOFRANGE;

    return deallocator(response);
}

RESULT optcl_command_format_unit(const optcl_device *device,
                                 const optcl_mmc_format_unit *command)
{
    RESULT error;
    RESULT destroy_error;

    cdb6 cdb;
    uint32_t alignment;
    uint8_t cdbparams[12];
    optcl_adapter *adapter = 0;

    assert(device != 0);
    assert(command != 0);
    if (device == 0 || command == 0)
        return E_INVALIDARG;

    error = optcl_device_get_adapter(device, &adapter);
    if (FAILED(error))
        return error;

    assert(adapter);
    if (adapter == 0)
        return E_POINTER;

    error = optcl_adapter_get_alignment_mask(adapter, &alignment);
    if (FAILED(error)) {
        destroy_error = optcl_adapter_destroy(adapter);
        return SUCCEEDED(destroy_error) ? error : destroy_error;
    }

    error = optcl_adapter_destroy(adapter);
    if (FAILED(error))
        return error;

    /*
     * Execute command
     */
    memset(cdb, 0, sizeof(cdb));
    cdb[0] = MMC_OPCODE_FORMAT_UNIT;
    cdb[1] = (command->cmplist << 3) | 0x11;    /* 00010001 */
    memset(cdbparams, 0, sizeof(cdbparams));
    if (command->fov == True) {
        cdbparams[1] = (command->dcrt << 5) | (command->try_out << 2) | 
            (command->immed << 1) | command->vs | 0x10; /* 00010000 */
        /* format descriptor length should be set to 8 */
        cdbparams[3] = 8;
    }

    /* number of blocks */
    cdbparams[4] = (uint8_t)(command->num_of_blocks >> 24);
    cdbparams[5] = (uint8_t)((command->num_of_blocks << 8) >> 24);
    cdbparams[6] = (uint8_t)((command->num_of_blocks << 16) >> 24);
    cdbparams[7] = (uint8_t)((command->num_of_blocks << 24) >> 24);
    cdbparams[8] = (command->format_type << 2) | command->format_subtype;
    switch (command->format_type) {
        case MMC_FORMAT_FULL_FORMAT:
        case MMC_FORMAT_SPARE_AREA_EXPANSION:
        case MMC_FORMAT_ZONE_REFORMAT:
        case MMC_FORMAT_ZONE_FORMAT:
        case MMC_FORMAT_CD_RW_DVD_RW_FULL_FORMAT:
        case MMC_FORMAT_CD_RW_DVD_RW_GROW_SESSION:
        case MMC_FORMAT_CD_RW_DVD_RW_ADD_SESSION:
        case MMC_FORMAT_DVD_RW_QUICK_GROW_LAST_BORDER:
        case MMC_FORMAT_DVD_RW_QUICK_ADD_BORDER:
        case MMC_FORMAT_DVD_RW_QUICK_FORMAT:
        case MMC_FORMAT_HD_DVD_R_TEST_ZONE_EXPANSION:
        case MMC_FORMAT_MRW_FORMAT:
        case MMC_FORMAT_BD_RE_FULL_FORMAT_WITH_SPARE_AREAS:
        case MMC_FORMAT_BD_RE_FULL_FORMAT_WITHOUT_SPARE_AREAS: {
            cdbparams[9] = (uint8_t)((
                command->type_dependant.other.type_dependent << 8) >> 24);
            cdbparams[10] = (uint8_t)((
                command->type_dependant.other.type_dependent << 16) >> 24);
            cdbparams[11] = (uint8_t)((
                command->type_dependant.other.type_dependent << 24) >> 24);
            break;
        }
        case MMC_FORMAT_FULL_FORMAT_WITH_SPARING_PARAMS: {
            cdbparams[9] = command->type_dependant.ff_with_sparing.m;
            cdbparams[11] = command->type_dependant.ff_with_sparing.n;
            break;
        }
        case MMC_FORMAT_DVD_PLUS_RW_BASIC_FORMAT: {
            cdbparams[11] = 
                (command->type_dependant.dvd_plus_rw_basicf.quick_start << 1)
                | command->type_dependant.dvd_plus_rw_basicf.restart;
            break;
        }
        case MMC_FORMAT_BD_R_FULL_FORMAT_WITH_SPARE_AREAS: {
            cdbparams[9] =
                (command->type_dependant.bd_r_with_spare_areas.isa_v << 7)
                | command->type_dependant.bd_r_with_spare_areas.sadp;
            cdbparams[10] =
                (command->type_dependant.bd_r_with_spare_areas.tdma_v << 7)
                | command->type_dependant.bd_r_with_spare_areas.tdmadp;
            break;
        }
        default: {
            assert(False);
            error = E_INVALIDARG;
            break;
        }
    }

    if (FAILED(error))
        return error;

    error = optcl_device_command_execute(device, cdb, sizeof(cdb), cdbparams, 
        sizeof(cdbparams));
    return error;
}


RESULT optcl_command_get_configuration(const optcl_device *device,
                                       const optcl_mmc_get_configuration *command,
                                       optcl_mmc_response_get_configuration **response)
{
    RESULT error;
    RESULT destroy_error;

    cdb10 cdb;
    uint8_t rt;
    int32_t data_length;
    uint16_t start_feature;
    ptr_t mmc_response = 0;
    uint32_t transfer_size;
    uint32_t alignment_mask;
    uint32_t max_transfer_len;
    optcl_adapter *adapter = 0;
    optcl_list_iterator it = 0;
    optcl_feature_descriptor *descriptor = 0;
    optcl_mmc_response_get_configuration *nresponse0 = 0;
    optcl_mmc_response_get_configuration *nresponse1 = 0;

    assert(device != 0);
    assert(command != 0);
    assert(response != 0);
    if (device == 0 || command == 0 || response == 0)
        return E_INVALIDARG;

    assert(command->rt == MMC_GET_CONFIG_RT_ALL || 
        command->rt == MMC_GET_CONFIG_RT_CURRENT || 
        command->rt == MMC_GET_CONFIG_RT_FROM);
    if (command->rt != MMC_GET_CONFIG_RT_ALL && 
        command->rt != MMC_GET_CONFIG_RT_CURRENT && 
        command->rt != MMC_GET_CONFIG_RT_FROM) {
        return E_INVALIDARG;
    }

    error = optcl_device_get_adapter(device, &adapter);
    if (FAILED(error))
        return error;

    assert(adapter != 0);
    if (adapter == 0)
        return E_POINTER;

    error = optcl_adapter_get_alignment_mask(adapter, &alignment_mask);
    if (FAILED(error)) {
        destroy_error = optcl_adapter_destroy(adapter);
        return SUCCEEDED(destroy_error) ? error : destroy_error;
    }

    error = optcl_adapter_get_max_transfer_len(adapter, &max_transfer_len);
    if (FAILED(error)) {
        destroy_error = optcl_adapter_destroy(adapter);
        return SUCCEEDED(destroy_error) ? error : destroy_error;
    }

    error = optcl_adapter_destroy(adapter);
    if (FAILED(error))
        return error;

    /*
     * Execute command just to get data length
     */
    rt = command->rt & 0x03;
    start_feature = command->start_feature;
    memset(cdb, 0, sizeof(cdb));
    cdb[0] = MMC_OPCODE_GET_CONFIG;
    cdb[1] = rt;
    cdb[2] = (uint8_t)(start_feature >> 8);
    cdb[3] = (uint8_t)((start_feature << 8) >> 8);
    cdb[8] = 8; /* enough to get feature descriptor header */
    mmc_response = (ptr_t)xmalloc_aligned(cdb[8], alignment_mask);
    if (mmc_response == 0)
        return E_OUTOFMEMORY;

    error = optcl_device_command_execute(device, cdb, sizeof(cdb), mmc_response,
                cdb[8]);
    if (FAILED(error)) {
        xfree_aligned(mmc_response);
        return error;
    }

    nresponse0 = (optcl_mmc_response_get_configuration*)
        malloc(sizeof(optcl_mmc_response_get_configuration));
    if (nresponse0 == 0) {
        xfree_aligned(mmc_response);
        return E_OUTOFMEMORY;
    }

    memset(nresponse0, 0, sizeof(optcl_mmc_response_get_configuration));
    error = optcl_list_create(0, &nresponse0->descriptors);
    if (FAILED(error)) {
        free(nresponse0);
        xfree_aligned(mmc_response);
        return error;
    }

    /*
     * Set new data length and execute command
     *
     * NOTE that in MMC-5 standard, the entire set of defined
     * feature descriptors amounts to less than 1 KB.
     */
    data_length = uint32_from_be(*(int32_t*)&mmc_response[0]);
    xfree_aligned(mmc_response);
    nresponse0->data_length = data_length;
    if (max_transfer_len > MAX_GET_CONFIG_TRANSFER_LEN)
        max_transfer_len = MAX_GET_CONFIG_TRANSFER_LEN;

    do {
        transfer_size = (data_length > (int32_t)max_transfer_len) ? 
            max_transfer_len : data_length;
        data_length -= max_transfer_len;

        /*
         * Execute command to get next chunk of data
         */
        cdb[1] = rt;
        cdb[2] = (uint8_t)(start_feature >> 8);
        cdb[3] = (uint8_t)((start_feature << 8) >> 8);
        cdb[7] = (uint8_t)((uint16_t)transfer_size >> 8);
        cdb[8] = (uint8_t)((((uint16_t)transfer_size) << 8) >> 8);
        mmc_response = (ptr_t)xmalloc_aligned(transfer_size, alignment_mask);
        if (mmc_response == 0) {
            error = E_OUTOFMEMORY;
            break;
        }

        memset(mmc_response, 0, transfer_size);
        error = optcl_device_command_execute(device, cdb, sizeof(cdb), 
            mmc_response, transfer_size);
        if (FAILED(error)) {
            xfree_aligned(mmc_response);
            break;
        }

        rt = MMC_GET_CONFIG_RT_FROM;

        /*
         * Process current chunk of data
         */
        error = parse_raw_get_configuration_data(mmc_response, transfer_size,
            &nresponse1);
        if (FAILED(error)) {
            xfree_aligned(mmc_response);
            break;
        }

        error = optcl_list_get_tail_pos(nresponse1->descriptors, &it);
        if (FAILED(error)) {
            destroy_error = optcl_list_destroy(nresponse1->descriptors, True);
            error = (FAILED(destroy_error)) ? destroy_error : error;
            xfree_aligned(mmc_response);
            free(nresponse1);
            break;
        }

        error = optcl_list_get_at_pos(nresponse1->descriptors, it, (const pptr_t)&descriptor);
        if (FAILED(error)) {
            destroy_error = optcl_list_destroy(nresponse1->descriptors, True);
            error = (FAILED(destroy_error)) ? destroy_error : error;
            xfree_aligned(mmc_response);
            free(nresponse1);
            break;
        }

        start_feature = descriptor->feature_code + 1;
        nresponse0->current_profile = nresponse1->current_profile;
        error = optcl_list_append(nresponse0->descriptors, nresponse1->descriptors);
        if (FAILED(error)) {
            destroy_error = optcl_list_destroy(nresponse1->descriptors, True);
            error = (FAILED(destroy_error)) ? destroy_error : error;
            xfree_aligned(mmc_response);
            free(nresponse1);
            break;
        }

        error = optcl_list_destroy(nresponse1->descriptors, False);
        if (FAILED(error)) {
            xfree_aligned(mmc_response);
            free(nresponse1);
            break;
        }

        free(nresponse1);
        xfree_aligned(mmc_response);
    } while (data_length > 0);

    if (FAILED(error)) {
        destroy_error = optcl_list_destroy(nresponse0->descriptors, False);
        free(nresponse0);
        return SUCCEEDED(destroy_error) ? error : destroy_error;
    }

    nresponse0->header.command_opcode = MMC_OPCODE_GET_CONFIG;
    *response = nresponse0;
    return error;
}

RESULT optcl_command_get_event_status(const optcl_device *device,
                                      const optcl_mmc_get_event_status *command,
                                      optcl_mmc_response_get_event_status **response)
{
    RESULT error;

    cdb10 cdb;
    uint32_t alignment;
    uint16_t descriptor_len;
    ptr_t mmc_response = 0;
    optcl_adapter *adapter = 0;
    optcl_mmc_response_get_event_status *nresponse = 0;

    assert(device != 0);
    assert(command != 0);
    assert(response != 0);
    if (device == 0 || command == 0 || response == 0)
        return E_INVALIDARG;

    error = optcl_device_get_adapter(device, &adapter);
    if (FAILED(error))
        return error;

    assert(adapter != 0);
    if (adapter == 0)
        return E_POINTER;

    error = optcl_adapter_get_alignment_mask(adapter, &alignment);
    if (FAILED(error))
        return error;

    error = optcl_adapter_destroy(adapter);
    if (FAILED(error))
        return error;

    /*
     * Execute command
     */
    memset(cdb, 0, sizeof(cdb));
    cdb[0] = MMC_OPCODE_GET_EVENT_STATUS;
    cdb[1] = command->polled;
    cdb[4] = command->class_request;
    cdb[8] = 4; /* request only event header */
    mmc_response = (ptr_t)xmalloc_aligned(cdb[8], alignment);
    if (mmc_response == 0)
        return E_OUTOFMEMORY;

    error = optcl_device_command_execute(device, cdb, sizeof(cdb), 
        mmc_response, cdb[8]);
    if (FAILED(error)) {
        xfree_aligned(mmc_response);
        return error;
    }

    error = parse_raw_get_event_status_data(mmc_response, cdb[8], &nresponse);
    xfree_aligned(mmc_response);
    if (FAILED(error))
        return error;

    assert(nresponse != 0);
    if (nresponse == 0)
        return E_POINTER;

    /*
     * Execute command
     */
    descriptor_len = nresponse->ges_header.descriptor_len;
    cdb[7] = (uint8_t)(descriptor_len >> 8);
    cdb[8] = (uint8_t)((descriptor_len << 8) >> 8) + 4;
    mmc_response = (ptr_t)xmalloc_aligned(descriptor_len + 4, alignment);
    if (mmc_response == 0)
        return E_OUTOFMEMORY;

    error = optcl_device_command_execute(device, cdb, sizeof(cdb), 
        mmc_response, descriptor_len + 4);
    if (FAILED(error)) {
        xfree_aligned(mmc_response);
        return error;
    }

    free(nresponse);
    error = parse_raw_get_event_status_data(mmc_response, descriptor_len + 4, 
        &nresponse);
    xfree_aligned(mmc_response);
    if (FAILED(error))
        return error;

    nresponse->header.command_opcode = MMC_OPCODE_GET_EVENT_STATUS;
    *response = nresponse;
    return SUCCESS;
}

RESULT optcl_command_get_performance(const optcl_device *device,
                                     const optcl_mmc_get_performance *command,
                                     optcl_mmc_response_get_performance **response)
{
    RESULT error;
    RESULT destroy_error;

    cdb12 cdb;
    uint32_t alignment;
    uint32_t perf_data_len;
    uint32_t max_transfer_len;
    ptr_t mmc_response = 0;
    optcl_adapter *adapter = 0;
    optcl_mmc_response_get_performance *nresponse = 0;

    assert(device != 0);
    assert(command != 0);
    assert(response != 0);
    if (device == 0 || command == 0 || response == 0)
        return E_INVALIDARG;

    error = optcl_device_get_adapter(device, &adapter);
    if (FAILED(error))
        return error;

    assert(adapter != 0);
    if (adapter == 0)
        return E_POINTER;

    error = optcl_adapter_get_alignment_mask(adapter, &alignment);
    if (FAILED(error)) {
        destroy_error = optcl_adapter_destroy(adapter);
        return SUCCEEDED(destroy_error) ? error : destroy_error;
    }

    error = optcl_adapter_get_max_transfer_len(adapter, &max_transfer_len);
    if (FAILED(error)) {
        destroy_error = optcl_adapter_destroy(adapter);
        return SUCCEEDED(destroy_error) ? error : destroy_error;
    }

    error = optcl_adapter_destroy(adapter);
    if (FAILED(error))
        return error;

    /*
     * Execute command just to get response header
     */
    memset(cdb, 0, sizeof(cdb));
    cdb[0] = MMC_OPCODE_GET_PERFORMANCE;
    cdb[1] = command->data_type & 0x1F;
    cdb[2] = (uint8_t)(command->start_lba >> 24);
    cdb[3] = (uint8_t)((command->start_lba << 8) >> 24);
    cdb[4] = (uint8_t)((command->start_lba << 16) >> 24);
    cdb[5] = (uint8_t)((command->start_lba << 24) >> 24);
    cdb[10] = command->type;
    mmc_response = (ptr_t)xmalloc_aligned(8, alignment);
    if (mmc_response == 0)
        return E_OUTOFMEMORY;

    error = optcl_device_command_execute(device, cdb, sizeof(cdb), 
        mmc_response, 8);
    if (FAILED(error)) {
        xfree_aligned(mmc_response);
        return error;
    }

    assert(mmc_response != 0);
    if (mmc_response == 0) {
        xfree_aligned(mmc_response);
        return E_POINTER;
    }

    perf_data_len = uint32_from_be(*(uint32_t*)&mmc_response[0]);
    xfree_aligned(mmc_response);
    if (perf_data_len > 0xFFFFFFFB)
        return E_OVERFLOW;

    if (perf_data_len + 4 > max_transfer_len)
        return E_DEVINVALIDSIZE;

    /*
     * Execute command
     */

    cdb[8] = (uint8_t)(command->max_desc_num >> 8);
    cdb[9] = (uint8_t)((command->max_desc_num << 8) >> 8);
    mmc_response = (ptr_t)xmalloc_aligned(perf_data_len + 4, alignment);

    if (mmc_response == 0)
        return E_OUTOFMEMORY;

    error = optcl_device_command_execute(device, cdb, sizeof(cdb), 
        mmc_response, 8);
    if (FAILED(error)) {
        xfree_aligned(mmc_response);
        return error;
    }

    assert(mmc_response != 0);
    if (mmc_response == 0) {
        xfree_aligned(mmc_response);
        return E_POINTER;
    }

    error = parse_raw_get_performance_data(command->type, command->data_type, 
        mmc_response, perf_data_len + 4, &nresponse);
    xfree_aligned(mmc_response);
    if (FAILED(error))
        return error;

    assert(nresponse != 0);
    if (nresponse == 0)
        return E_POINTER;

    nresponse->header.command_opcode = MMC_OPCODE_GET_PERFORMANCE;
    *response = nresponse;
    return SUCCESS;
}

RESULT optcl_command_inquiry(const optcl_device *device,
                             const optcl_mmc_inquiry *command,
                             optcl_mmc_response_inquiry **response)
{
    RESULT error;
    RESULT destroy_error;

    cdb6 cdb;
    ptr_t mmc_response = 0;
    uint32_t alignment_mask;
    optcl_adapter *adapter;
    optcl_mmc_response_inquiry *nresponse;

    assert(device != 0);
    assert(command != 0);
    assert(response != 0);
    if (device == 0|| command == 0 || response == 0)
        return E_INVALIDARG;

    assert(command->evpd == 0);
    assert(command->page_code == 0);
    if (command->evpd != 0 || command->page_code != 0)
        return E_INVALIDARG;

    error = optcl_device_get_adapter(device, &adapter);
    if (FAILED(error))
        return error;

    assert(adapter != 0);
    if (adapter == 0)
        return E_POINTER;

    error = optcl_adapter_get_alignment_mask(adapter, &alignment_mask);
    if (FAILED(error)) {
        destroy_error = optcl_adapter_destroy(adapter);
        return SUCCEEDED(destroy_error) ? error : destroy_error;
    }

    error = optcl_adapter_destroy(adapter);
    if (FAILED(error))
        return error;

    /*
     * Execute command just to get additional length
     */

    memset(cdb, 0, sizeof(cdb));
    cdb[0] = MMC_OPCODE_INQUIRY;
    cdb[4] = 5; /* the allocation length should be at least five */
    mmc_response = (ptr_t)xmalloc_aligned(cdb[4], alignment_mask);
    if (mmc_response == 0)
        return E_OUTOFMEMORY;

    /* Get standard inquiry data additional length */
    error = optcl_device_command_execute(device, cdb, sizeof(cdb), 
        mmc_response, cdb[4]);
    if (FAILED(error)) {
        xfree_aligned(mmc_response);
        return error;
    }

    /* Set standard inquiry data length */
    cdb[4] = mmc_response[4] + 4;
    xfree_aligned(mmc_response);
    mmc_response = (ptr_t)xmalloc_aligned((size_t)cdb[4], alignment_mask);
    if (mmc_response == 0)
        return E_OUTOFMEMORY;

    /*
     * Execute command
     */

    /* Get standard inquiry data */
    error = optcl_device_command_execute(device, cdb, sizeof(cdb), 
        mmc_response, cdb[4]);
    if (FAILED(error)) {
        xfree_aligned(mmc_response);
        return error;
    }

    error = parse_raw_inquiry_data(mmc_response, cdb[4], &nresponse);
    if (SUCCEEDED(error)) {
        nresponse->header.command_opcode = MMC_OPCODE_INQUIRY;
        *response = nresponse;
    }

    xfree_aligned(mmc_response);
    return error;
}

RESULT optcl_command_load_unload_medium(const optcl_device *device,
                                        const optcl_mmc_load_unload_medium *command)
{
    RESULT error;

    cdb12 cdb;

    assert(device != 0);
    assert(command != 0);
    if (device == 0 || command == 0)
        return E_INVALIDARG;

    /*
     * Execute command
     */
    memset(cdb, 0, sizeof(cdb));
    cdb[0] = MMC_OPCODE_LOAD_UNLOAD;
    cdb[1] = command->immed;
    cdb[4] = (command->load_unload << 1) | command->start;
    cdb[8] = command->slot;
    error = optcl_device_command_execute(device, cdb, sizeof(cdb), 0, 0);
    return error;
}

RESULT optcl_command_mechanism_status(const optcl_device *device,
                                      optcl_mmc_response_mechanism_status **response)
{
    RESULT error;
    RESULT destroy_error;

    int i;
    cdb12 cdb;
    uint32_t alignment;
    uint16_t response_size;
    ptr_t mmc_response = 0;
    optcl_adapter *adapter = 0;
    optcl_mmc_response_mechanism_status *nresponse = 0;

    assert(device != 0);
    assert(response != 0);
    if (device == 0 || response == 0)
        return E_INVALIDARG;

    error = optcl_device_get_adapter(device, &adapter);
    if (FAILED(error))
        return error;

    assert(adapter != 0);
    if (adapter == 0)
        return E_POINTER;

    error = optcl_adapter_get_alignment_mask(adapter, &alignment);
    if (FAILED(error)) {
        destroy_error = optcl_adapter_destroy(adapter);
        return SUCCEEDED(destroy_error) ? error : destroy_error;
    }

    error = optcl_adapter_destroy(adapter);
    if (FAILED(error))
        return error;

    /*
     * Execute command
     */
    memset(cdb, 0, sizeof(cdb));
    response_size = MECHSTATUS_RESPSIZE;
    cdb[0] = MMC_OPCODE_MECHANISM_STATUS;
    cdb[8] = (uint8_t)(response_size >> 8);
    cdb[9] = (uint8_t)((response_size << 8) >> 8);
    mmc_response = (ptr_t)xmalloc_aligned(MECHSTATUS_RESPSIZE, alignment);
    if (mmc_response == 0)
        return E_OUTOFMEMORY;

    error = optcl_device_command_execute(device, cdb, sizeof(cdb), 
        mmc_response, MECHSTATUS_RESPSIZE);
    if (FAILED(error)) {
        xfree_aligned(mmc_response);
        return error;
    }

    /*
     * Parse raw data
     */
    nresponse = (optcl_mmc_response_mechanism_status*)
        malloc(sizeof(optcl_mmc_response_mechanism_status));
    if (nresponse == 0) {
        xfree_aligned(mmc_response);
        return E_OUTOFMEMORY;
    }

    memset(nresponse, 0, sizeof(optcl_mmc_response_mechanism_status));
    nresponse->header.command_opcode = MMC_OPCODE_MECHANISM_STATUS;
    nresponse->fault = bool_from_uint8(mmc_response[0] & 0x80);         /* 10000000 */
    nresponse->changer_state = mmc_response[0] & 0x60;                  /* 01100000 */
    nresponse->current_slot = ((mmc_response[1] & 0x07) << 5) | 
        (mmc_response[0] & 0x1F);
    nresponse->mechanism_state = mmc_response[1] & 0xE0;                /* 11100000 */
    nresponse->door_open = bool_from_uint8(mmc_response[1] & 0x10);     /* 00010000 */
    nresponse->current_lba = (mmc_response[2] << 16) | 
        (mmc_response[3] << 8) | mmc_response[4];
    nresponse->available_slots = mmc_response[5];
    nresponse->slot_table_len = uint16_from_be(*(uint16_t*)&mmc_response[6]);

    for (i = 0; i < nresponse->available_slots; ++i) {
        nresponse->slot_entries[i].disk_present = bool_from_uint8(mmc_response[8 + i * 4] & 0x80);
        nresponse->slot_entries[i].change = bool_from_uint8(mmc_response[8 + i * 4] & 0x01);
        nresponse->slot_entries[i].cwp_v = bool_from_uint8(mmc_response[9 + i * 4] & 0x02);
        nresponse->slot_entries[i].cwp = bool_from_uint8(mmc_response[9 + i * 4] & 0x01);
    }

    xfree_aligned(mmc_response);
    *response = nresponse;
    return error;
}

RESULT optcl_command_mode_sense_10(const optcl_device *device,
                                   const optcl_mmc_mode_sense *command,
                                   optcl_mmc_response_mode_sense **response)
{
    RESULT error;
    RESULT destroy_error;

    cdb10 cdb;
    uint32_t alignment;
    uint16_t mode_data_len;
    ptr_t mmc_response = 0;
    optcl_adapter *adapter = 0;
    optcl_mmc_response_mode_sense *nresponse = 0;

    assert(device != 0);
    assert(command != 0);
    assert(response != 0);
    if (device == 0 || command == 0 || response == 0)
        return E_INVALIDARG;

    error = optcl_device_get_adapter(device, &adapter);
    if (FAILED(error))
        return error;

    assert(adapter != 0);
    if (adapter == 0)
        return E_POINTER;

    error = optcl_adapter_get_alignment_mask(adapter, &alignment);
    if (FAILED(error)) {
        destroy_error = optcl_adapter_destroy(adapter);
        return SUCCEEDED(destroy_error) ? error : destroy_error;
    }

    error = optcl_adapter_destroy(adapter);
    if (FAILED(error))
        return error;

    /*
     * Execute command to get response buffer size
     */
    memset(cdb, 0, sizeof(cdb));
    cdb[0] = MMC_OPCODE_MODE_SENSE;
    cdb[1] = (command->dbd << 3) & 0x08;			                /* 00001000 */
    cdb[2] = (command->pc << 6) | command->page_code;
    cdb[4] = 8;	/* get header only */
    mmc_response = (ptr_t)xmalloc_aligned(8, alignment);
    if (mmc_response == 0)
        return E_OUTOFMEMORY;

    error = optcl_device_command_execute(device, cdb, sizeof(cdb), 
        mmc_response, 8);
    if (FAILED(error)) {
        xfree_aligned(mmc_response);
        return error;
    }

    mode_data_len = uint16_from_be(*(uint16_t*)&mmc_response[0]);
    xfree_aligned(mmc_response);
    mmc_response = (ptr_t)xmalloc_aligned(mode_data_len + 2, alignment);
    if (mmc_response == 0)
        return E_OUTOFMEMORY;

    /*
     * Execute command
     */
    error = optcl_device_command_execute(device, cdb, sizeof(cdb), 
        mmc_response, mode_data_len + 2);
    if (FAILED(error)) {
        xfree_aligned(mmc_response);
        return error;
    }

    error = parse_raw_mode_sense_data(mmc_response, mode_data_len + 2, 
        &nresponse);
    xfree_aligned(mmc_response);
    if (FAILED(error))
        return error;

    assert(nresponse != 0);
    if (nresponse == 0)
        return E_POINTER;

    *response = nresponse;
    return SUCCESS;
}

RESULT optcl_command_mode_select_10(const optcl_device *device,
                                    const optcl_mmc_mode_select *command)
{
    RESULT error;
    RESULT destroy_error;

    cdb10 cdb;
    ptr_t data_out = 0;
    uint32_t alignment;
    uint16_t data_out_len;
    optcl_adapter *adapter = 0;

    assert(device != 0);
    assert(command != 0);
    if (device == 0 || command == 0)
        return E_INVALIDARG;

    assert(command->descriptors != 0);
    if (command->descriptors == 0)
        return E_POINTER;

    error = optcl_device_get_adapter(device, &adapter);
    if (FAILED(error))
        return error;

    assert(adapter != 0);
    if (adapter == 0)
        return E_POINTER;

    error = optcl_adapter_get_alignment_mask(adapter, &alignment);
    if (FAILED(error)) {
        destroy_error = optcl_adapter_destroy(adapter);
        return SUCCEEDED(destroy_error) ? error : destroy_error;
    }

    error = optcl_adapter_destroy(adapter);
    if (FAILED(error))
        return error;

    error = create_dataout_mode_select(command, alignment, &data_out, &data_out_len);
    if (FAILED(error))
        return error;

    assert(data_out != 0);
    if (data_out == NULL)
        return E_POINTER;

    /*
     * Execute command
     */
    memset(cdb, 0, sizeof(cdb));
    cdb[0] = MMC_OPCODE_MODE_SELECT;
    cdb[1] = (command->pf << 4) | command->sp;
    cdb[7] = (uint8_t)(data_out_len >> 8);
    cdb[8] = (uint8_t)data_out_len;
    error = optcl_device_command_execute(device, cdb, sizeof(cdb), 
        data_out, data_out_len);
    xfree_aligned(data_out);
    return error;
}

RESULT optcl_command_prevent_allow_removal(const optcl_device *device,
        const optcl_mmc_prevent_allow_removal *command)
{
    RESULT error;
    cdb6 cdb;

    assert(device != 0);
    assert(command != 0);
    if (device == 0 || command == 0)
        return E_INVALIDARG;

    /*
     * Execute command
     */
    memset(cdb, 0, sizeof(cdb));
    cdb[0] = MMC_OPCODE_PREVENT_ALLOW_REMOVAL;
    cdb[4] = (command->persistent << 1) | command->prevent;
    error = optcl_device_command_execute(device, cdb, sizeof(cdb), 0, 0);
    return error;
}

RESULT optcl_command_read_10(const optcl_device *device,
                             const optcl_mmc_read_10 *command,
                             optcl_mmc_response_read **response)
{
    RESULT error;
    RESULT destroy_error;

    cdb10 cdb;
    uint32_t alignment;
    ptr_t mmc_response = 0;
    uint32_t transfer_size;
    uint32_t max_transfer_len;
    optcl_adapter *adapter = 0;
    optcl_mmc_response_read *nresponse = 0;

    assert(device != 0);
    assert(command != 0);
    assert(response != 0);
    if (device == 0 || command == 0 || response == 0)
        return E_INVALIDARG;

    error = optcl_device_get_adapter(device, &adapter);
    if (FAILED(error))
        return error;

    assert(adapter != 0);
    if (adapter == 0)
        return E_POINTER;

    error = optcl_adapter_get_alignment_mask(adapter, &alignment);
    if (FAILED(error)) {
        destroy_error = optcl_adapter_destroy(adapter);
        return SUCCEEDED(destroy_error) ? error : destroy_error;
    }

    error = optcl_adapter_get_max_transfer_len(adapter, &max_transfer_len);
    if (FAILED(error)) {
        destroy_error = optcl_adapter_destroy(adapter);
        return SUCCEEDED(destroy_error) ? error : destroy_error;
    }

    error = optcl_adapter_destroy(adapter);
    if (FAILED(error))
        return error;

    nresponse = (optcl_mmc_response_read*)
        malloc(sizeof(optcl_mmc_response_read));
    if (nresponse == 0)
        return E_OUTOFMEMORY;

    transfer_size = command->transfer_length * READ_BLOCK_SIZE;
    if (transfer_size > max_transfer_len)
        return E_INVALIDARG;

    /*
     * Execute command
     */
    memset(cdb, 0, sizeof(cdb));
    cdb[0] = MMC_OPCODE_READ_10;
    cdb[1] = (command->fua << 3);
    cdb[2] = (uint8_t)(command->start_lba >> 24);
    cdb[3] = (uint8_t)((command->start_lba << 8) >> 24);
    cdb[4] = (uint8_t)((command->start_lba << 16) >> 24);
    cdb[5] = (uint8_t)((command->start_lba << 24) >> 24);
    cdb[7] = (uint8_t)(command->transfer_length >> 8);
    cdb[8] = (uint8_t)((command->transfer_length << 8) >> 8);
    mmc_response = (ptr_t)xmalloc_aligned(transfer_size, alignment);
    if (mmc_response == 0)
        return E_OUTOFMEMORY;

    error = optcl_device_command_execute(device, cdb, sizeof(cdb), 
        mmc_response, transfer_size);
    if (FAILED(error)) {
        free(nresponse);
        xfree_aligned(mmc_response);
    }

    nresponse->data = (ptr_t)malloc(transfer_size);
    if (nresponse->data == 0) {
        free(nresponse);
        xfree_aligned(mmc_response);
        return E_OUTOFMEMORY;
    }

    xmemcpy(nresponse->data, transfer_size, mmc_response, transfer_size);
    xfree_aligned(mmc_response);
    *response = nresponse;
    return error;
}

RESULT optcl_command_read_12(const optcl_device *device,
                             const optcl_mmc_read_12 *command,
                             optcl_mmc_response_read **response)
{
    RESULT error;
    RESULT destroy_error;

    cdb12 cdb;
    uint32_t alignment;
    ptr_t mmc_response = 0;
    uint32_t max_transfer_len;
    optcl_adapter *adapter = 0;
    optcl_mmc_response_read *nresponse = 0;

    assert(device != 0);
    assert(command != 0);
    assert(response != 0);
    if (device == 0 || command == 0 || response == 0)
        return E_INVALIDARG;

    error = optcl_device_get_adapter(device, &adapter);
    if (FAILED(error))
        return error;

    assert(adapter != 0);
    if (adapter == 0)
        return E_POINTER;

    error = optcl_adapter_get_alignment_mask(adapter, &alignment);
    if (FAILED(error)) {
        destroy_error = optcl_adapter_destroy(adapter);
        return SUCCEEDED(destroy_error) ? error : destroy_error;
    }

    error = optcl_adapter_get_max_transfer_len(adapter, &max_transfer_len);
    if (FAILED(error)) {
        destroy_error = optcl_adapter_destroy(adapter);
        return SUCCEEDED(destroy_error) ? error : destroy_error;
    }

    error = optcl_adapter_destroy(adapter);
    if (FAILED(error))
        return error;

    nresponse = (optcl_mmc_response_read*)
        malloc(sizeof(optcl_mmc_response_read));
    if (nresponse == 0)
        return E_OUTOFMEMORY;

    if (command->transfer_length * READ_BLOCK_SIZE > max_transfer_len)
        return E_INVALIDARG;

    /*
     * Execute command
     */
    memset(cdb, 0, sizeof(cdb));
    cdb[0] = MMC_OPCODE_READ_12;
    cdb[1] = (command->fua << 3);
    cdb[2] = (uint8_t)(command->start_lba >> 24);
    cdb[3] = (uint8_t)((command->start_lba << 8) >> 24);
    cdb[4] = (uint8_t)((command->start_lba << 16) >> 24);
    cdb[5] = (uint8_t)((command->start_lba << 24) >> 24);
    cdb[6] = (uint8_t)(command->transfer_length >> 24);
    cdb[7] = (uint8_t)((command->transfer_length << 8) >> 24);
    cdb[8] = (uint8_t)((command->transfer_length << 16) >> 24);
    cdb[9] = (uint8_t)((command->transfer_length << 24) >> 24);
    cdb[10] = (command->streaming << 7);
    mmc_response = (ptr_t)xmalloc_aligned(
        command->transfer_length * READ_BLOCK_SIZE, alignment);
    if (mmc_response == 0)
        return E_OUTOFMEMORY;

    error = optcl_device_command_execute(device, cdb, sizeof(cdb), 
        mmc_response, command->transfer_length * READ_BLOCK_SIZE);
    if (FAILED(error)) {
        free(nresponse);
        xfree_aligned(mmc_response);
    }

    nresponse->data = mmc_response;
    *response = nresponse;
    return error;
}

RESULT optcl_command_read_buffer(const optcl_device *device,
                                 const optcl_mmc_read_buffer *command,
                                 optcl_mmc_response_read_buffer **response)
{
    RESULT error;
    RESULT destroy_error;

    cdb10 cdb;
    uint32_t alignment;
    uint32_t max_transfer_len;
    ptr_t mmc_response = 0;
    optcl_adapter *adapter = 0;
    optcl_mmc_response_read_buffer *nresponse = 0;

    assert(device != 0);
    assert(command != 0);
    assert(response != 0);
    if (device == 0 || command == 0 || response == 0)
        return E_INVALIDARG;

    error = optcl_device_get_adapter(device, &adapter);
    if (FAILED(error))
        return error;

    assert(adapter != 0);
    if (adapter == 0)
        return E_POINTER;

    error = optcl_adapter_get_alignment_mask(adapter, &alignment);
    if (FAILED(error)) {
        destroy_error = optcl_adapter_destroy(adapter);
        return SUCCEEDED(destroy_error) ? error : destroy_error;
    }

    error = optcl_adapter_get_max_transfer_len(adapter, &max_transfer_len);
    if (FAILED(error)) {
        destroy_error = optcl_adapter_destroy(adapter);
        return SUCCEEDED(destroy_error) ? error : destroy_error;
    }

    error = optcl_adapter_destroy(adapter);
    if (FAILED(error))
        return error;

    if (command->allocation_len > max_transfer_len)
        return E_INVALIDARG;

    if (command->mode == MMC_READ_BUFFER_MODE_DESCRIPTOR && 
        command->allocation_len != 3 ) {
        assert(False);
        return E_INVALIDARG;
    } else if (command->mode == MMC_READ_BUFFER_MODE_ECHO_DESC && 
        command->allocation_len != 4) {
        assert(False);
        return E_INVALIDARG;
    }

    /*
     * Execute command
     */
    memset(cdb, 0, sizeof(cdb));
    cdb[0] = MMC_OPCODE_READ_BUFFER;
    cdb[1] = command->mode & 0x1F;
    cdb[2] = command->buffer_id;
    cdb[3] = (uint8_t)((command->buffer_offset << 8) >> 24);
    cdb[4] = (uint8_t)((command->buffer_offset << 16) >> 24);
    cdb[5] = (uint8_t)((command->buffer_offset << 24) >> 24);
    cdb[6] = (uint8_t)((command->allocation_len << 8) >> 24);
    cdb[7] = (uint8_t)((command->allocation_len << 16) >> 24);
    cdb[8] = (uint8_t)((command->allocation_len << 24) >> 24);
    mmc_response = (ptr_t)xmalloc_aligned(command->allocation_len, alignment);
    if (mmc_response == 0)
        return E_OUTOFMEMORY;

    error = optcl_device_command_execute(device, cdb, sizeof(cdb), mmc_response, 4);
    if (FAILED(error)) {
        xfree_aligned(mmc_response);
        return error;
    }

    error = parse_raw_read_buffer_data(mmc_response, command->allocation_len, 
        command->mode, &nresponse);
    xfree_aligned(mmc_response);
    if (FAILED(error))
        return error;

    assert(nresponse != 0);
    if (nresponse == 0)
        return E_POINTER;

    *response = nresponse;
    return error;
}

RESULT optcl_command_read_buffer_capacity(const optcl_device *device,
                                          const optcl_mmc_read_buffer_capacity *command,
                                          optcl_mmc_response_read_buffer_capacity **response)
{
    RESULT error;
    RESULT destroy_error;

    cdb10 cdb;
    uint32_t alignment;
    ptr_t mmc_response = 0;
    optcl_adapter *adapter = 0;
    optcl_mmc_response_read_buffer_capacity *nresponse = 0;
    assert(device != 0);
    assert(command != 0);
    assert(response != 0);
    if (device == 0 || command == 0 || response == 0)
        return E_INVALIDARG;

    error = optcl_device_get_adapter(device, &adapter);
    if (FAILED(error))
        return error;

    assert(adapter != 0);
    if (adapter == 0)
        return E_POINTER;

    error = optcl_adapter_get_alignment_mask(adapter, &alignment);
    if (FAILED(error)) {
        destroy_error = optcl_adapter_destroy(adapter);
        return SUCCEEDED(destroy_error) ? error : destroy_error;
    }

    error = optcl_adapter_destroy(adapter);
    if (FAILED(error))
        return error;

    /*
     * Execute command
     */
    memset(cdb, 0, sizeof(cdb));
    cdb[0] = MMC_OPCODE_READ_BUFFER_CAPACITY;
    cdb[1] = command->block & 0x01;
    cdb[8] = 12; /* size of the response buffer */
    mmc_response = (ptr_t)xmalloc_aligned(12, alignment);
    if (mmc_response == 0)
        return E_OUTOFMEMORY;

    error = optcl_device_command_execute(device, cdb, sizeof(cdb), 
        mmc_response, 12);
    if (FAILED(error)) {
        xfree_aligned(mmc_response);
        return error;
    }

    nresponse = (optcl_mmc_response_read_buffer_capacity*)
        malloc(sizeof(optcl_mmc_response_read_buffer_capacity));
    if (nresponse == 0)
        return E_OUTOFMEMORY;

    nresponse->header.command_opcode = MMC_OPCODE_READ_BUFFER_CAPACITY;
    if (command->block) {
        nresponse->desc.block.data_length = 
            uint16_from_be(*(uint16_t*)&mmc_response[0]);
        nresponse->desc.block.block = bool_from_uint8(mmc_response[3]);
        nresponse->desc.block.available_buffer_len = 
            uint32_from_be(*(uint32_t*)&mmc_response[8]);
    } else {
        nresponse->desc.bytes.data_length = 
            uint16_from_be(*(uint16_t*)&mmc_response[0]);
        nresponse->desc.bytes.buffer_len = 
            uint32_from_be(*(uint32_t*)&mmc_response[4]);
        nresponse->desc.bytes.buffer_blank_len = 
            uint32_from_be(*(uint32_t*)&mmc_response[8]);
    }

    *response = nresponse;
    return error;
}

RESULT optcl_command_read_capacity(const optcl_device *device,
                                   optcl_mmc_response_read_capacity **response)
{
    RESULT error;
    RESULT destroy_error;

    cdb10 cdb;
    uint32_t alignment;
    ptr_t mmc_response = 0;
    optcl_adapter *adapter = 0;
    optcl_mmc_response_read_capacity *nresponse = 0;
    assert(device != 0);
    assert(response != 0);
    if (device == 0 || response == 0)
        return E_INVALIDARG;

    error = optcl_device_get_adapter(device, &adapter);
    if (FAILED(error))
        return error;

    error = optcl_adapter_get_alignment_mask(adapter, &alignment);
    if (FAILED(error)) {
        destroy_error = optcl_adapter_destroy(adapter);
        return SUCCEEDED(destroy_error) ? error : destroy_error;
    }

    error = optcl_adapter_destroy(adapter);
    if (FAILED(error))
        return error;

    /*
     * Execute command
     */
    memset(cdb, 0, sizeof(cdb));
    cdb[0] = MMC_OPCODE_READ_CAPACITY;
    mmc_response = (ptr_t)xmalloc_aligned(8, alignment);
    if (mmc_response == 0)
        return E_OUTOFMEMORY;

    error = optcl_device_command_execute(device, cdb, sizeof(cdb), 
        mmc_response, 8);
    if (FAILED(error)) {
        xfree_aligned(mmc_response);
        return error;
    }

    nresponse = (optcl_mmc_response_read_capacity*)
        malloc(sizeof(optcl_mmc_response_read_capacity));
    if (nresponse == 0) {
        xfree_aligned(mmc_response);
        return E_OUTOFMEMORY;
    }

    nresponse->header.command_opcode = MMC_OPCODE_READ_CAPACITY;
    nresponse->lba = uint32_from_be(*(uint32_t*)&mmc_response[0]);
    nresponse->block_len = uint32_from_be(*(uint32_t*)&mmc_response[4]);
    return error;
}

RESULT optcl_command_read_cd(const optcl_device *device,
                             const optcl_mmc_read_cd *command,
                             optcl_mmc_response_read_cd **response)
{
    return SUCCESS;
}

RESULT optcl_command_read_msn(const optcl_device *device,
                              optcl_mmc_response_read_msn **response)
{
    RESULT error;
    RESULT destroy_error;

    cdb12 cdb;
    ptr_t msn = 0;
    uint32_t msnlen;
    uint32_t alignment;
    ptr_t mmc_response = 0;
    optcl_adapter *adapter = 0;
    optcl_mmc_response_read_msn *nresponse = 0;

    assert(device != 0);
    assert(response != 0);
    if (device == 0 || response == 0)
        return E_INVALIDARG;

    error = optcl_device_get_adapter(device, &adapter);
    if (FAILED(error))
        return error;

    assert(adapter != 0);
    if (adapter == 0)
        return E_POINTER;

    error = optcl_adapter_get_alignment_mask(adapter, &alignment);
    if (FAILED(error)) {
        destroy_error = optcl_adapter_destroy(adapter);
        return SUCCEEDED(destroy_error) ? error : destroy_error;
    }

    error = optcl_adapter_destroy(adapter);
    if (FAILED(error))
        return error;

    /*
     * Execute command to get MSN length
     */
    mmc_response = (ptr_t)xmalloc_aligned(4, alignment);
    if (mmc_response == 0)
        return E_OUTOFMEMORY;

    memset(cdb, 0, sizeof(cdb));
    cdb[0] = MMC_OPCODE_READ_MSN;
    cdb[1] = 0x01;	/* service action (0x01) */
    cdb[9] = 0x04;	/* allocation length (0x0004) */
    error = optcl_device_command_execute(device, cdb, sizeof(cdb), 
        mmc_response, 4);
    if (FAILED(error)) {
        xfree_aligned(mmc_response);
        return error;
    }

    msnlen = uint32_from_be(*(uint32_t*)mmc_response);
    xfree_aligned(mmc_response);

    /*
     * Execute command to get MSN
     */
    mmc_response = (ptr_t)xmalloc_aligned(msnlen, alignment);
    if (mmc_response == 0)
        return E_OUTOFMEMORY;

    cdb[6] = (uint8_t)(msnlen >> 24);
    cdb[7] = (uint8_t)((msnlen << 8) >> 24);
    cdb[8] = (uint8_t)((msnlen << 16) >> 24);
    cdb[9] = (uint8_t)((msnlen << 24) >> 24);
    error = optcl_device_command_execute(device, cdb, sizeof(cdb), 
        mmc_response, msnlen);
    if (FAILED(error)) {
        xfree_aligned(mmc_response);
        return error;
    }

    msnlen = uint32_from_be(*(uint32_t*)mmc_response);
    msn = (ptr_t)malloc(msnlen);
    if (msn == 0) {
        xfree_aligned(mmc_response);
        return E_OUTOFMEMORY;
    }

    xmemcpy(msn, msnlen, mmc_response, msnlen);
    nresponse = (optcl_mmc_response_read_msn*)
        malloc(sizeof(optcl_mmc_response_read_msn));
    if (nresponse == 0) {
        free(msn);
        xfree_aligned(mmc_response);
        return E_OUTOFMEMORY;
    }

    nresponse->header.command_opcode = MMC_OPCODE_READ_MSN;
    nresponse->msn_len = (uint16_t)msnlen;
    nresponse->msn = msn;
    *response = nresponse;
    return error;
}

RESULT optcl_command_read_track_information(const optcl_device *device,
                                            const optcl_mmc_read_track_info *command,
                                            optcl_mmc_response_read_track_info **response)
{
    RESULT error;
    RESULT destroy_error;

    cdb10 cdb;
    uint32_t alignment;
    ptr_t mmc_response = 0;
    optcl_adapter *adapter = 0;
    optcl_mmc_response_read_track_info *nresponse = 0;
    assert(device != NULL);
    assert(command != NULL);
    assert(response != NULL);
    if (device == 0 || command == 0 || response == 0)
        return E_INVALIDARG;

    error = optcl_device_get_adapter(device, &adapter);
    if (FAILED(error))
        return error;

    assert(adapter != 0);
    if (adapter == 0)
        return E_POINTER;

    error = optcl_adapter_get_alignment_mask(adapter, &alignment);
    if (FAILED(error)) {
        destroy_error = optcl_adapter_destroy(adapter);
        return SUCCEEDED(destroy_error) ? error : destroy_error;
    }

    error = optcl_adapter_destroy(adapter);
    if (FAILED(error))
        return error;

    /*
     * Execute command
     */
    mmc_response = (ptr_t)xmalloc_aligned(48, alignment);
    if (mmc_response == 0)
        return E_OUTOFMEMORY;

    memset(cdb, 0, sizeof(cdb));
    cdb[0] = MMC_OPCODE_READ_TRACK_INFORMATION;
    cdb[1] = (uint8_t)(command->open << 2 | command->addrnum_type);
    cdb[2] = (uint8_t)(command->lbatsnum >> 24);
    cdb[3] = (uint8_t)((command->lbatsnum << 8) >> 24);
    cdb[4] = (uint8_t)((command->lbatsnum << 16) >> 24);
    cdb[5] = (uint8_t)((command->lbatsnum << 24) >> 24);
    cdb[7] = (uint8_t)(command->alloc_len >> 8);
    cdb[8] = (uint8_t)((command->alloc_len << 8) >> 8);
    error = optcl_device_command_execute(device, cdb, sizeof(cdb), 
        mmc_response, 48);
    if (FAILED(error)) {
        xfree_aligned(mmc_response);
        return error;
    }

    nresponse = (optcl_mmc_response_read_track_info*)
        malloc(sizeof(optcl_mmc_response_read_track_info));
    if (nresponse == 0) {
        xfree_aligned(mmc_response);
        return E_OUTOFMEMORY;
    }

    nresponse->ltn_lsb = mmc_response[2];
    nresponse->sn_lsb = mmc_response[3];
    nresponse->ljrs = mmc_response[5] & 0xC0;                           /* 11000000 */
    nresponse->damage = bool_from_uint8(mmc_response[5] & 0x20);        /* 00100000 */
    nresponse->copy = bool_from_uint8(mmc_response[5] & 0x10);          /* 00010000 */
    nresponse->track_mode = mmc_response[5] & 0x0F;                     /* 00001111 */
    nresponse->rt = bool_from_uint8(mmc_response[6] & 0x80);            /* 10000000 */
    nresponse->blank = bool_from_uint8(mmc_response[6] & 0x40);         /* 01000000 */
    nresponse->packet_inc = bool_from_uint8(mmc_response[6] & 0x20);    /* 00100000 */
    nresponse->fp = bool_from_uint8(mmc_response[6] & 0x10);            /* 00010000 */
    nresponse->data_mode = mmc_response[6] & 0x0F;                      /* 00001111 */
    nresponse->lra_v = bool_from_uint8(mmc_response[7] & 0x02);         /* 00000010 */
    nresponse->nwa_v = bool_from_uint8(mmc_response[7] & 0x01);         /* 00000001 */
    nresponse->ltsa = uint32_from_be(*(uint32_t*)&mmc_response[8]);
    nresponse->nwa = uint32_from_be(*(uint32_t*)&mmc_response[12]);
    nresponse->free_blocks = uint32_from_be(*(uint32_t*)&mmc_response[16]);
    nresponse->fps_bf = uint32_from_be(*(uint32_t*)&mmc_response[20]);
    nresponse->lts = uint32_from_be(*(uint32_t*)&mmc_response[24]);
    nresponse->lra = uint32_from_be(*(uint32_t*)&mmc_response[28]);
    nresponse->ltn_msb = mmc_response[32];
    nresponse->sn_msb = mmc_response[33];
    nresponse->rclba = uint32_from_be(*(uint32_t*)&mmc_response[36]);
    nresponse->nlja = uint32_from_be(*(uint32_t*)&mmc_response[40]);
    nresponse->llja = uint32_from_be(*(uint32_t*)&mmc_response[44]);
    xfree_aligned(mmc_response);
    *response = nresponse;

    return error;
}

RESULT optcl_command_repair_track(const optcl_device *device,
                                  const optcl_mmc_repair_track *command)
{
    RESULT error;
    cdb10 cdb;

    assert(device != 0);
    assert(command != 0);
    if (device == 0 || command == 0)
        return E_INVALIDARG;

    /*
     * Execute command
     */
    memset(cdb, 0, sizeof(cdb));
    cdb[0] = MMC_OPCODE_REPAIR_TRACK;
    cdb[1] = (uint8_t)(command->immed & 0x01);
    cdb[4] = (uint8_t)(command->ltn >> 8);
    cdb[5] = (uint8_t)((command->ltn << 8) >> 8);
    error = optcl_device_command_execute(device, cdb, sizeof(cdb), 0, 0);
    return error;
}

RESULT optcl_command_request_sense(const optcl_device *device,
                                   const optcl_mmc_request_sense *command,
                                   optcl_mmc_response_request_sense **response)
{
    RESULT error;
    RESULT sense_code;
    RESULT destroy_error;

    cdb6 cdb;
    uint32_t alignment;
    ptr_t mmc_response = 0;
    optcl_adapter *adapter = 0;
    optcl_mmc_response_request_sense *nresponse = 0;

    assert(device != 0);
    assert(command != 0);
    assert(response != 0);
    if (device == 0 || command == 0 || response == 0)
        return E_INVALIDARG;

    error = optcl_device_get_adapter(device, &adapter);
    if (FAILED(error))
        return error;

    assert(adapter != 0);
    if (adapter == 0)
        return E_POINTER;

    error = optcl_adapter_get_alignment_mask(adapter, &alignment);
    if (FAILED(error)) {
        destroy_error = optcl_adapter_destroy(adapter);
        return SUCCEEDED(destroy_error) ? error : destroy_error;
    }

    error = optcl_adapter_destroy(adapter);
    if (FAILED(error))
        return error;

    /*
     * Execute command
     */
    memset(cdb, 0, sizeof(cdb));
    cdb[0] = MMC_OPCODE_REQUEST_SENSE;
    cdb[1] = (uint8_t)command->desc;
    cdb[4] = MAX_SENSEDATA_LENGTH;
    mmc_response = (ptr_t)xmalloc_aligned(MAX_SENSEDATA_LENGTH, alignment);
    if (mmc_response == 0)
        return E_OUTOFMEMORY;

    error = optcl_device_command_execute(device, cdb, sizeof(cdb), mmc_response, 
        MAX_SENSEDATA_LENGTH);
    if (FAILED(error)) {
        xfree_aligned(mmc_response);
        return error;
    }

    error = optcl_sensedata_get_code(mmc_response, MAX_SENSEDATA_LENGTH, 
        &sense_code);
    xfree_aligned(mmc_response);
    if (FAILED(error))
        return error;

    nresponse = (optcl_mmc_response_request_sense*)
        malloc(sizeof(optcl_mmc_response_request_sense));
    if (nresponse == 0)
        return E_OUTOFMEMORY;

    nresponse->header.command_opcode = MMC_OPCODE_REQUEST_SENSE;
    nresponse->asc = ERROR_SENSE_ASC(sense_code);
    nresponse->ascq = ERROR_SENSE_ASCQ(sense_code);
    nresponse->sk = ERROR_SENSE_SK(sense_code);
    *response = nresponse;
    return SUCCESS;
}

RESULT optcl_command_reserve_track(const optcl_device *device,
                                   const optcl_mmc_reserve_track *command)
{
    RESULT error;
    cdb10 cdb;

    assert(device != 0);
    assert(command != 0);
    if (device == 0 || command == 0)
        return E_INVALIDARG;

    /*
     * Execute command
     */
    memset(cdb, 0, sizeof(cdb));
    cdb[0] = MMC_OPCODE_RESERVE_TRACK;
    cdb[1] = (uint8_t)((command->rmz << 1) | command->arsv);
    if (command->arsv) {
        cdb[2] = (uint8_t)(command->reservation.lba >> 24);
        cdb[3] = (uint8_t)((command->reservation.lba << 8) >> 24);
        cdb[4] = (uint8_t)((command->reservation.lba << 16) >> 24);
        cdb[5] = (uint8_t)((command->reservation.lba << 24) >> 24);
    } else {
        cdb[5] = (uint8_t)(command->reservation.size >> 24);
        cdb[6] = (uint8_t)((command->reservation.size << 8) >> 24);
        cdb[7] = (uint8_t)((command->reservation.size << 16) >> 24);
        cdb[8] = (uint8_t)((command->reservation.size << 24) >> 24);
    }

    error = optcl_device_command_execute(device, cdb, sizeof(cdb), 0, 0);
    return error;
}

RESULT optcl_command_seek(const optcl_device *device,
                          const optcl_mmc_seek *command)
{
    RESULT error;
    cdb10 cdb;

    assert(device != 0);
    assert(command != 0);
    if (device == 0 || command == 0)
        return E_INVALIDARG;

    /*
     * Execute command
     */
    memset(cdb, 0, sizeof(cdb));
    cdb[0] = MMC_OPCODE_SEEK;
    cdb[2] = (uint8_t)(command->lba >> 24);
    cdb[3] = (uint8_t)((command->lba << 8) >> 24);
    cdb[4] = (uint8_t)((command->lba << 16) >> 24);
    cdb[5] = (uint8_t)((command->lba << 24) >> 24);
    error = optcl_device_command_execute(device, cdb, sizeof(cdb), 0, 0);
    return error;
}

RESULT optcl_command_send_disc_structure(const optcl_device *device,
                                         const optcl_mmc_send_disc_structure *command)
{
    RESULT error;
    RESULT destroy_error;

    cdb12 cdb;
    ptr_t ndata = 0;
    ptr_t dataout = 0;
    uint32_t alignment;
    uint16_t dataout_len;
    optcl_adapter *adapter = 0;

    assert(device != 0);
    assert(command != 0);
    if (device == 0 || command == 0)
        return E_INVALIDARG;

    error = optcl_device_get_adapter(device, &adapter);
    if (FAILED(error))
        return error;

    assert(adapter != 0);
    if (adapter == 0)
        return E_POINTER;

    error = optcl_adapter_get_alignment_mask(adapter, &alignment);
    if (FAILED(error)) {
        destroy_error = optcl_adapter_destroy(adapter);
        return SUCCEEDED(destroy_error) ? error : destroy_error;
    }

    error = optcl_adapter_destroy(adapter);
    if (FAILED(error))
        return error;

    error = create_dataout_send_disc_structure(command, &ndata, &dataout_len);
    if (FAILED(error))
        return error;

    assert(ndata != 0);
    if (ndata == 0)
        return E_POINTER;

    assert(dataout_len > 0);
    if (dataout_len < 1) {
        free(ndata);
        return E_UNEXPECTED;
    }

    dataout = (ptr_t)xmalloc_aligned(dataout_len, alignment);
    if (ndata == 0) {
        free(ndata);
        return E_OUTOFMEMORY;
    }

    xmemcpy(dataout, dataout_len, ndata, dataout_len);
    free(ndata);

    /*
     * Execute command
     */
    memset(cdb, 0, sizeof(cdb));
    cdb[0] = MMC_OPCODE_SEND_DISC_STRUCTURE;
    cdb[1] = command->media_type & 0x0F;
    cdb[7] = command->format_type;
    cdb[8] = (uint8_t)(dataout_len >> 8);
    cdb[9] = (uint8_t)(dataout_len);
    error = optcl_device_command_execute(device, cdb, sizeof(cdb), dataout, 
        dataout_len);
    xfree_aligned(dataout);
    return error;
}

RESULT optcl_command_send_opc_information(const optcl_device *device,
                                          const optcl_mmc_send_opc_information *command)
{
    RESULT error;
    RESULT destroy_error;

    cdb10 cdb;
    ptr_t data = 0;
    uint32_t alignment;
    uint16_t param_list_len;
    optcl_adapter *adapter = 0;

    assert(device != 0);
    assert(command != 0);
    if (device == 0 || command == 0)
        return E_INVALIDARG;

    assert(command->opc_entry_num > 0);
    if (command->opc_entry_num < 1)
        return E_INVALIDARG;

    error = optcl_device_get_adapter(device, &adapter);
    if (FAILED(error))
        return error;

    assert(adapter != 0);
    if (adapter == 0)
        return E_POINTER;

    error = optcl_adapter_get_alignment_mask(adapter, &alignment);
    if (FAILED(error)) {
        destroy_error = optcl_adapter_destroy(adapter);
        return SUCCEEDED(destroy_error) ? error : destroy_error;
    }

    error = optcl_adapter_destroy(adapter);
    if (FAILED(error))
        return error;

    param_list_len = command->opc_entry_num * sizeof(command->opc_table_entries[0]);
    data = (ptr_t)xmalloc_aligned(param_list_len, alignment);
    if (data == 0)
        return E_OUTOFMEMORY;

    xmemcpy(data, param_list_len, &command->opc_table_entries, param_list_len);

    /*
     * Execute command
     */
    memset(cdb, 0, sizeof(cdb));
    cdb[0] = MMC_OPCODE_SEND_OPC_INFORMATION;
    cdb[1] = (uint8_t)(command->doopc);
    cdb[2] = (uint8_t)((command->exclude1 << 1) | command->exclude0);
    cdb[7] = (uint8_t)(param_list_len >> 8);
    cdb[8] = (uint8_t)((param_list_len << 8) >> 8);
    error = optcl_device_command_execute(device, cdb, sizeof(cdb), 
        data, param_list_len);
    xfree_aligned(data);

    return error;
}

RESULT optcl_command_set_cd_speed(const optcl_device *device,
                                  const optcl_mmc_set_cd_speed *command)
{
    RESULT error;
    cdb12 cdb;

    assert(device != 0);
    assert(command != 0);
    if (device == 0 || command == 0)
        return E_INVALIDARG;

    /*
     * Execute command
     */
    memset(cdb, 0, sizeof(cdb));
    cdb[0] = MMC_OPCODE_SET_CD_SPEED;
    cdb[1] = command->rotctrl & 0x03;
    cdb[2] = (uint8_t)(command->drive_read_speed >> 8);
    cdb[3] = (uint8_t)((command->drive_read_speed << 8) >> 8);
    cdb[4] = (uint8_t)(command->drive_write_speed >> 8);
    cdb[5] = (uint8_t)((command->drive_write_speed << 8) >> 8);
    error = optcl_device_command_execute(device, cdb, sizeof(cdb), 0, 0);

    return error;
}

RESULT optcl_command_set_read_ahead(const optcl_device *device,
                                    const optcl_mmc_set_read_ahead *command)
{
    RESULT error;
    cdb12 cdb;

    assert(device != 0);
    assert(command != 0);
    if (device == 0 || command == 0)
        return E_INVALIDARG;

    /*
     * Execute command
     */
    memset(cdb, 0, sizeof(cdb));
    cdb[0] = MMC_OPCODE_SET_READ_AHEAD;
    cdb[2] = (uint8_t)(command->trigger_lba >> 24);
    cdb[3] = (uint8_t)((command->trigger_lba << 8) >> 24);
    cdb[4] = (uint8_t)((command->trigger_lba << 16) >> 24);
    cdb[5] = (uint8_t)((command->trigger_lba << 24) >> 24);
    cdb[6] = (uint8_t)(command->read_ahead_lba >> 24);
    cdb[7] = (uint8_t)((command->read_ahead_lba << 8) >> 24);
    cdb[8] = (uint8_t)((command->read_ahead_lba << 16) >> 24);
    cdb[9] = (uint8_t)((command->read_ahead_lba << 24) >> 24);
    error = optcl_device_command_execute(device, cdb, sizeof(cdb), 0, 0);

    return error;
}

RESULT optcl_command_start_stop_unit(const optcl_device *device,
                                     const optcl_mmc_start_stop_unit *command)
{
    RESULT error;
    cdb6 cdb;

    assert(device != 0);
    assert(command != 0);
    if (device == 0 || command == 0)
        return E_INVALIDARG;

    /*
     * Execute command
     */
    memset(cdb, 0, sizeof(cdb));
    cdb[0] = MMC_OPCODE_START_STOP_UNIT;
    cdb[1] = (uint8_t)command->immed;
    cdb[3] = (uint8_t)command->fln;
    cdb[4] = (uint8_t)((command->pc << 4) | (command->fl << 2) | 
        (command->loej << 1) | command->start);
    error = optcl_device_command_execute(device, cdb, sizeof(cdb), 0, 0);

    return error;
}

RESULT optcl_command_set_streaming(const optcl_device *device,
                                   const optcl_mmc_set_streaming *command)
{
    RESULT error;
    RESULT destroy_error;

    cdb12 cdb;
    uint32_t i;
    uint32_t index;
    uint32_t start_lba;
    ptr_t data = 0;
    uint32_t alignment;
    uint32_t param_list_len;
    optcl_adapter *adapter = 0;

    assert(device != NULL);
    assert(command != NULL);
    if (device == 0 || command == 0)
        return E_INVALIDARG;

    error = optcl_device_get_adapter(device, &adapter);
    if (FAILED(error))
        return error;

    assert(adapter != 0);
    if (adapter == 0)
        return E_POINTER;

    error = optcl_adapter_get_alignment_mask(adapter, &alignment);
    if (FAILED(error)) {
        destroy_error = optcl_adapter_destroy(adapter);
        return SUCCEEDED(destroy_error) ? error : destroy_error;
    }

    error = optcl_adapter_destroy(adapter);
    if (FAILED(error))
        return error;

    if (command->type == MMC_SET_STREAMING_PERFORMANCE) {
        param_list_len = 28;
        data = (ptr_t)xmalloc_aligned(param_list_len, alignment);
        if (data == 0)
            return E_OUTOFMEMORY;

        memset(data, 0, param_list_len);
        data[0] = (uint8_t)((command->descriptors.performance.wrc << 3) | 
            (command->descriptors.performance.rdd << 2) | 
            (command->descriptors.performance.exact << 1) | 
            command->descriptors.performance.ra);
        data[4] = (uint8_t)(command->descriptors.performance.start_lba >> 24);
        data[5] = (uint8_t)((command->descriptors.performance.start_lba << 8) >> 24);
        data[6] = (uint8_t)((command->descriptors.performance.start_lba << 16) >> 24);
        data[7] = (uint8_t)((command->descriptors.performance.start_lba << 24) >> 24);
        data[8] = (uint8_t)(command->descriptors.performance.end_lba >> 24);
        data[9] = (uint8_t)((command->descriptors.performance.end_lba << 8) >> 24);
        data[10] = (uint8_t)((command->descriptors.performance.end_lba << 16) >> 24);
        data[11] = (uint8_t)((command->descriptors.performance.end_lba << 24) >> 24);
        data[12] = (uint8_t)(command->descriptors.performance.read_size >> 24);
        data[13] = (uint8_t)((command->descriptors.performance.read_size << 8) >> 24);
        data[14] = (uint8_t)((command->descriptors.performance.read_size << 16) >> 24);
        data[15] = (uint8_t)((command->descriptors.performance.read_size << 24) >> 24);
        data[16] = (uint8_t)(command->descriptors.performance.read_time >> 24);
        data[17] = (uint8_t)((command->descriptors.performance.read_time << 8) >> 24);
        data[18] = (uint8_t)((command->descriptors.performance.read_time << 16) >> 24);
        data[19] = (uint8_t)((command->descriptors.performance.read_time << 24) >> 24);
        data[20] = (uint8_t)(command->descriptors.performance.write_size >> 24);
        data[21] = (uint8_t)((command->descriptors.performance.write_size << 8) >> 24);
        data[22] = (uint8_t)((command->descriptors.performance.write_size << 16) >> 24);
        data[23] = (uint8_t)((command->descriptors.performance.write_size << 24) >> 24);
        data[24] = (uint8_t)(command->descriptors.performance.write_time >> 24);
        data[25] = (uint8_t)((command->descriptors.performance.write_time << 8) >> 24);
        data[26] = (uint8_t)((command->descriptors.performance.write_time << 16) >> 24);
        data[27] = (uint8_t)((command->descriptors.performance.write_time << 24) >> 24);
    } else if (command->type == MMC_SET_STREAMING_DBI_CACHE_ZONE) {
        param_list_len = command->descriptors.dbi_cache_zones.desc_num * 
            sizeof(command->descriptors.dbi_cache_zones.descriptors[0]);
        data = (ptr_t)xmalloc_aligned(param_list_len, alignment);
        if (data == 0)
            return E_OUTOFMEMORY;

        memset(data, 0, param_list_len);
        param_list_len -= 4;
        data[0] = (uint8_t)(param_list_len >> 24);
        data[1] = (uint8_t)((param_list_len << 8) >> 24);
        data[2] = (uint8_t)((param_list_len << 16) >> 24);
        data[3] = (uint8_t)((param_list_len << 24) >> 24);

        for (i = 0; i < command->descriptors.dbi_cache_zones.desc_num; ++i) {
            index = (i + 1) * 8;
            start_lba = command->descriptors.dbi_cache_zones.descriptors[i];
            data[index++] = (uint8_t)(start_lba >> 24);
            data[index++] = (uint8_t)((start_lba << 8) >> 24);
            data[index++] = (uint8_t)((start_lba << 16) >> 24);
            data[index++] = (uint8_t)((start_lba << 24) >> 24);
        }
    } else {
        return E_INVALIDARG;
    }

    /*
     * Execute command
     */
    memset(cdb, 0, sizeof(cdb));
    cdb[0] = MMC_OPCODE_SET_STREAMING;
    cdb[8] = command->type;
    cdb[9] = (uint8_t)((param_list_len << 16) >> 24);
    cdb[10] = (uint8_t)((param_list_len << 24) >> 24);
    error = optcl_device_command_execute(device, cdb, sizeof(cdb), data, 
        param_list_len);
    xfree_aligned(data);

    return error;
}

RESULT optcl_command_synchronize_cache(const optcl_device *device,
                                       const optcl_mmc_synchronize_cache *command)
{
    RESULT error;
    cdb10 cdb;

    assert(device != 0);
    assert(command != 0);
    if (device == 0 || command == 0)
        return E_INVALIDARG;

    /*
     * Execute command
     */
    memset(cdb, 0, sizeof(cdb));
    cdb[0] = MMC_OPCODE_SYNCHRONIZE_CACHE;
    cdb[1] = (uint8_t)(command->immed << 1);
    cdb[2] = (uint8_t)(command->lba >> 24);
    cdb[3] = (uint8_t)((command->lba << 8) >> 24);
    cdb[4] = (uint8_t)((command->lba << 16) >> 24);
    cdb[5] = (uint8_t)((command->lba << 24) >> 24);
    cdb[7] = (uint8_t)(command->num_of_blocks >> 8);
    cdb[8] = (uint8_t)((command->num_of_blocks << 8) >> 8);
    error = optcl_device_command_execute(device, cdb, sizeof(cdb), 0, 0);

    return error;
}

RESULT optcl_command_test_unit_ready(const optcl_device *device)
{
    RESULT error;
    cdb6 cdb;

    assert(device != 0);
    if (device == 0)
        return E_INVALIDARG;

    /*
     * Execute command
     */
    memset(cdb, 0, sizeof(cdb));
    cdb[0] = MMC_OPCODE_TEST_UNIT_READY;
    error = optcl_device_command_execute(device, cdb, sizeof(cdb), 0, 0);

    return error;
}

RESULT optcl_command_verify(const optcl_device *device,
                            const optcl_mmc_verify *command)
{
    RESULT error;
    cdb10 cdb;

    assert(device != 0);
    assert(command != 0);
    if (device == 0 || command == 0)
        return E_INVALIDARG;

    /*
     * Execute command
     */
    memset(cdb, 0, sizeof(cdb));
    cdb[0] = MMC_OPCODE_VERIFY;
    cdb[2] = (uint8_t)(command->lba >> 24);
    cdb[3] = (uint8_t)((command->lba << 8) >> 24);
    cdb[4] = (uint8_t)((command->lba << 16) >> 24);
    cdb[5] = (uint8_t)((command->lba << 24) >> 24);
    cdb[6] = (uint8_t)(command->g3tout << 7);
    cdb[7] = (uint8_t)(command->block_num >> 8);
    cdb[8] = (uint8_t)((command->block_num << 8) >> 8);
    error = optcl_device_command_execute(device, cdb, sizeof(cdb), 0, 0);

    return error;
}

RESULT optcl_command_write(const optcl_device *device,
                           const optcl_mmc_write *command,
                           ptr_t data,
                           uint32_t data_len)
{
    RESULT error;
    RESULT destroy_error;

    cdb10 cdb;
    ptr_t ndata = 0;
    uint32_t alignment;
    optcl_adapter *adapter = 0;

    assert(device != 0);
    assert(command != 0);
    assert(data != 0);
    assert(data_len > 0);
    if (device == 0 || command == 0 || data == 0 || data_len < 1)
        return E_INVALIDARG;

    error = optcl_device_get_adapter(device, &adapter);
    if (FAILED(error))
        return error;

    assert(adapter != 0);
    if (adapter == 0)
        return E_POINTER;

    error = optcl_adapter_get_alignment_mask(adapter, &alignment);
    if (FAILED(error)) {
        destroy_error = optcl_adapter_destroy(adapter);
        return SUCCEEDED(destroy_error) ? error : destroy_error;
    }

    error = optcl_adapter_destroy(adapter);
    if (FAILED(error))
        return error;

    ndata = (ptr_t)xmalloc_aligned(data_len, alignment);
    if (ndata == 0)
        return E_OUTOFMEMORY;

    xmemcpy(ndata, data_len, data, data_len);

    /*
     * Execute command
     */
    memset(cdb, 0, sizeof(cdb));
    cdb[0] = MMC_OPCODE_WRITE;
    cdb[1] = (uint8_t)((command->fua << 3) | (command->tsr << 2));
    cdb[2] = (uint8_t)(command->lba >> 24);
    cdb[3] = (uint8_t)((command->lba << 8) >> 24);
    cdb[4] = (uint8_t)((command->lba << 16) >> 24);
    cdb[5] = (uint8_t)((command->lba << 24) >> 24);
    cdb[7] = (uint8_t)(command->transfer_len >> 8);
    cdb[8] = (uint8_t)((command->transfer_len << 8) >> 8);
    error = optcl_device_command_execute(device, cdb, sizeof(cdb), 
        ndata, data_len);
    xfree_aligned(ndata);

    return error;
}

RESULT optcl_command_write_12(const optcl_device *device,
                              const optcl_mmc_write_12 *command,
                              ptr_t data,
                              uint32_t data_len)
{
    RESULT error;
    RESULT destroy_error;

    cdb12 cdb;
    ptr_t ndata = 0;
    uint32_t alignment;
    optcl_adapter *adapter = 0;

    assert(device != 0);
    assert(command != 0);
    assert(data != 0);
    assert(data_len > 0);
    if (device == 0 || command == 0 || data == 0 || data_len < 1)
        return E_INVALIDARG;

    error = optcl_device_get_adapter(device, &adapter);
    if (FAILED(error))
        return error;

    assert(adapter != 0);
    if (adapter == 0)
        return E_POINTER;

    error = optcl_adapter_get_alignment_mask(adapter, &alignment);
    if (FAILED(error)) {
        destroy_error = optcl_adapter_destroy(adapter);
        return SUCCEEDED(destroy_error) ? error : destroy_error;
    }

    error = optcl_adapter_destroy(adapter);
    if (FAILED(error))
        return error;

    ndata = (ptr_t)xmalloc_aligned(data_len, alignment);
    if (ndata == 0)
        return E_OUTOFMEMORY;

    xmemcpy(ndata, data_len, data, data_len);

    /*
     * Execute command
     */
    memset(cdb, 0, sizeof(cdb));
    cdb[0] = MMC_OPCODE_WRITE;
    cdb[1] = (uint8_t)((command->fua << 3) | (command->tsr << 2));
    cdb[2] = (uint8_t)(command->lba >> 24);
    cdb[3] = (uint8_t)((command->lba << 8) >> 24);
    cdb[4] = (uint8_t)((command->lba << 16) >> 24);
    cdb[5] = (uint8_t)((command->lba << 24) >> 24);
    cdb[6] = (uint8_t)(command->transfer_len >> 24);
    cdb[7] = (uint8_t)((command->transfer_len << 8) >> 24);
    cdb[8] = (uint8_t)((command->transfer_len << 16) >> 24);
    cdb[9] = (uint8_t)((command->transfer_len << 24) >> 24);
    cdb[10] = (uint8_t)((command->streaming << 7) | (command->vnr << 6));
    error = optcl_device_command_execute(device, cdb, sizeof(cdb), 
        ndata, data_len);
    xfree_aligned(ndata);

    return error;
}

RESULT optcl_command_write_and_verify_10(const optcl_device *device,
                                         const optcl_mmc_write_and_verify_10 *command,
                                         ptr_t data,
                                         uint32_t data_len)
{
    RESULT error;
    RESULT destroy_error;

    cdb10 cdb;
    ptr_t ndata = 0;
    uint32_t alignment;
    optcl_adapter *adapter = 0;

    assert(device != 0);
    assert(command != 0);
    assert(data != 0);
    assert(data_len > 0);
    if (device == 0 || command == 0 || data == 0 || data_len < 1)
        return E_INVALIDARG;

    error = optcl_device_get_adapter(device, &adapter);
    if (FAILED(error))
        return error;

    assert(adapter != 0);
    if (adapter == 0)
        return E_POINTER;

    error = optcl_adapter_get_alignment_mask(adapter, &alignment);
    if (FAILED(error)) {
        destroy_error = optcl_adapter_destroy(adapter);
        return SUCCEEDED(destroy_error) ? error : destroy_error;
    }

    error = optcl_adapter_destroy(adapter);
    if (FAILED(error))
        return error;

    ndata = (ptr_t)xmalloc_aligned(data_len, alignment);
    if (ndata == 0)
        return E_OUTOFMEMORY;

    xmemcpy(ndata, data_len, data, data_len);

    /*
     * Execute command
     */
    memset(cdb, 0, sizeof(cdb));
    cdb[0] = MMC_OPCODE_WRITE;
    cdb[2] = (uint8_t)(command->lba >> 24);
    cdb[3] = (uint8_t)((command->lba << 8) >> 24);
    cdb[4] = (uint8_t)((command->lba << 16) >> 24);
    cdb[5] = (uint8_t)((command->lba << 24) >> 24);
    cdb[7] = (uint8_t)(command->transfer_len >> 8);
    cdb[8] = (uint8_t)((command->transfer_len << 8) >> 8);
    error = optcl_device_command_execute(device, cdb, sizeof(cdb), 
        ndata, data_len);
    xfree_aligned(ndata);

    return error;
}

RESULT optcl_command_write_buffer(const optcl_device *device,
                                  const optcl_mmc_write_buffer *command)
{
    RESULT error;
    RESULT destroy_error;

    cdb10 cdb;
    ptr_t ndata = 0;
    ptr_t dataout = 0;
    uint32_t dataoutlen;
    uint32_t alignment;
    optcl_adapter *adapter = 0;

    assert(device != 0);
    assert(command != 0);
    if (device == 0 || command == 0)
        return E_INVALIDARG;

    error = optcl_device_get_adapter(device, &adapter);
    if (FAILED(error))
        return error;

    assert(adapter != 0);
    if (adapter == 0)
        return E_POINTER;

    error = optcl_adapter_get_alignment_mask(adapter, &alignment);
    if (FAILED(error)) {
        destroy_error = optcl_adapter_destroy(adapter);
        return SUCCEEDED(destroy_error) ? error : destroy_error;
    }

    error = optcl_adapter_destroy(adapter);
    if (FAILED(error))
        return error;

    error = create_dataout_write_buffer(command, &ndata, &dataoutlen);
    if (FAILED(error))
        return error;

    if (command->mode == MMC_WRITE_BUFFER_MODE_ECHO) {
        // Data in this mode should be aligned on 4 byte boundaries
        alignment = 4;
    } else if (command->mode != MMC_WRITE_BUFFER_MODE_DIS_EXPANDER) {
        dataout = (ptr_t)xmalloc_aligned(dataoutlen, alignment);
        if (dataout == 0) {
            free(ndata);
            return E_OUTOFMEMORY;
        }

        xmemcpy(dataout, dataoutlen, ndata, dataoutlen);
        free(ndata);
    } else if (ndata == 0 && dataoutlen != 0) {
        assert(False);
        return E_POINTER;
    }

    /*
     * Execute command
     */
    memset(cdb, 0, sizeof(cdb));
    cdb[0] = MMC_OPCODE_WRITE_BUFFER;
    cdb[1] = command->mode & 0x1F;
    cdb[2] = command->buffer_id;
    cdb[3] = (uint8_t)(command->buffer_offset >> 16);
    cdb[4] = (uint8_t)(command->buffer_offset >> 8);
    cdb[5] = (uint8_t)(command->buffer_offset);
    cdb[6] = (uint8_t)(command->param_list_len >> 16);
    cdb[7] = (uint8_t)(command->param_list_len >> 8);
    cdb[8] = (uint8_t)(command->param_list_len);
    error = optcl_device_command_execute(device, cdb, sizeof(cdb), 
        dataout, dataoutlen);
    xfree_aligned(dataout);

    return error;
}


/*
 * Deallocators
 */

static RESULT deallocator_mmc_response_close_track_session(optcl_mmc_response *response)
{
    if (response == 0)
        return SUCCESS;

    assert(response->command_opcode == MMC_OPCODE_CLOSE_TRACK_SESSION);
    if (response->command_opcode != MMC_OPCODE_CLOSE_TRACK_SESSION)
        return E_CMNDINVOPCODE;

    free(response);
    return SUCCESS;
}

static RESULT deallocator_mmc_response_get_configuration(optcl_mmc_response *response)
{
    RESULT error;
    optcl_mmc_response_get_configuration *mmc_response = 0;
    if (response == 0)
        return SUCCESS;

    assert(response->command_opcode == MMC_OPCODE_GET_CONFIG);
    if (response->command_opcode != MMC_OPCODE_GET_CONFIG)
        return E_CMNDINVOPCODE;

    mmc_response = (optcl_mmc_response_get_configuration*)response;
    assert(mmc_response->descriptors != 0);
    error = optcl_list_destroy(mmc_response->descriptors, True);
    free(mmc_response);
    return error;
}

static RESULT deallocator_mmc_response_get_event_status(optcl_mmc_response *response)
{
    RESULT error;
    optcl_mmc_response_get_event_status *mmc_response = 0;
    if (response == 0)
        return SUCCESS;

    assert(response->command_opcode == MMC_OPCODE_GET_EVENT_STATUS);
    if (response->command_opcode != MMC_OPCODE_GET_EVENT_STATUS)
        return E_CMNDINVOPCODE;

    mmc_response = (optcl_mmc_response_get_event_status*)response;
    assert(mmc_response->descriptors != 0);
    error = optcl_list_destroy(mmc_response->descriptors, True);
    free(mmc_response);
    return error;
}

static RESULT deallocator_mmc_response_get_performance(optcl_mmc_response *response)
{
    RESULT error;
    optcl_mmc_response_get_performance *mmc_response = 0;
    if (response == 0)
        return SUCCESS;

    assert(response->command_opcode == MMC_OPCODE_GET_PERFORMANCE);
    if (response->command_opcode != MMC_OPCODE_GET_PERFORMANCE)
        return E_CMNDINVOPCODE;

    mmc_response = (optcl_mmc_response_get_performance*)response;
    assert(mmc_response->descriptors != 0);
    error = optcl_list_destroy(mmc_response->descriptors, True);
    free(mmc_response);
    return error;
}

static RESULT deallocator_mmc_response_mechanism_status(optcl_mmc_response *response)
{
    if (response == 0)
        return SUCCESS;

    assert(response->command_opcode == MMC_OPCODE_MECHANISM_STATUS);
    if (response->command_opcode != MMC_OPCODE_MECHANISM_STATUS)
        return E_CMNDINVOPCODE;

    free(response);
    return SUCCESS;
}

static RESULT deallocator_mmc_response_sense_10(optcl_mmc_response *response)
{
    RESULT error;
    optcl_mmc_response_mode_sense *mmc_response = 0;
    if (response == 0)
        return SUCCESS;

    assert(response->command_opcode == MMC_OPCODE_MODE_SENSE);
    if (response->command_opcode != MMC_OPCODE_MODE_SENSE)
        return E_CMNDINVOPCODE;

    mmc_response = (optcl_mmc_response_mode_sense*)response;
    error = optcl_list_destroy(mmc_response->descriptors, True);
    free(mmc_response);
    return error;
}

static RESULT deallocator_mmc_response_inquiry(optcl_mmc_response *response)
{
    if (response == 0)
        return SUCCESS;

    assert(response->command_opcode == MMC_OPCODE_INQUIRY);
    if (response->command_opcode != MMC_OPCODE_INQUIRY)
        return E_CMNDINVOPCODE;

    free(response);
    return SUCCESS;
}

static RESULT deallocator_mmc_response_read_10(optcl_mmc_response *response)
{
    optcl_mmc_response_read *mmc_response = 0;
    if (response == 0)
        return SUCCESS;

    assert(response->command_opcode == MMC_OPCODE_READ_10);
    if (response->command_opcode != MMC_OPCODE_READ_10)
        return E_CMNDINVOPCODE;

    mmc_response = (optcl_mmc_response_read*)response;
    free(mmc_response->data);
    free(mmc_response);
    return SUCCESS;
}

static RESULT deallocator_mmc_response_read_buffer(optcl_mmc_response *response)
{
    optcl_mmc_response_read_buffer *mmc_response = 0;
    if (response == 0)
        return SUCCESS;

    assert(response->command_opcode == MMC_OPCODE_READ_BUFFER);
    if (response->command_opcode != MMC_OPCODE_READ_BUFFER)
        return E_CMNDINVOPCODE;

    mmc_response = (optcl_mmc_response_read_buffer*)response;
    switch (mmc_response->mode) {
        case MMC_READ_BUFFER_MODE_COMBINED: {
            free(mmc_response->readdata.combined.buffer);
            break;
        }
        case MMC_READ_BUFFER_MODE_DATA: {
            free(mmc_response->readdata.data.buffer);
            break;
        }
        case MMC_READ_BUFFER_MODE_DESCRIPTOR: {
            break;
        }
        case MMC_READ_BUFFER_MODE_ECHO: {
            free(mmc_response->readdata.echo.buffer);
            break;
        }
        case MMC_READ_BUFFER_MODE_ECHO_DESC: {
            break;
        }
        case MMC_READ_BUFFER_MODE_EXPANDER: {
            free(mmc_response->readdata.expander.buffer);
            break;
        }
        case MMC_READ_BUFFER_MODE_VENDOR: {
            free(mmc_response->readdata.vendor.buffer);
            break;
        }
        default: {
            assert(False);
            return E_OUTOFRANGE;
        }
    }

    free(mmc_response);
    return SUCCESS;
}

static RESULT deallocator_mmc_response_read_buffer_capacity(optcl_mmc_response *response)
{
    if (response == 0)
        return SUCCESS;

    free(response);
    return SUCCESS;
}

static RESULT deallocator_mmc_response_read_msn(optcl_mmc_response *response)
{
    optcl_mmc_response_read_msn *mmc_response = 0;
    if (response == 0)
        return SUCCESS;

    assert(response->command_opcode == MMC_OPCODE_READ_MSN);
    if (response->command_opcode != MMC_OPCODE_READ_MSN)
        return E_CMNDINVOPCODE;

    mmc_response = (optcl_mmc_response_read_msn*)response;
    free(mmc_response->msn);
    free(mmc_response);
    return SUCCESS;
}

static RESULT deallocator_mmc_response_read_track_information(optcl_mmc_response *response)
{
    optcl_mmc_response_read_track_info *mmc_response = 0;
    if (response == 0)
        return SUCCESS;

    assert(response->command_opcode == MMC_OPCODE_READ_TRACK_INFORMATION);
    if (response->command_opcode != MMC_OPCODE_READ_TRACK_INFORMATION)
        return E_CMNDINVOPCODE;

    mmc_response = (optcl_mmc_response_read_track_info*)response;
    free(mmc_response);
    return SUCCESS;
}

static RESULT deallocator_mmc_response_read_cd(optcl_mmc_response *response)
{
    return SUCCESS;
}

static RESULT deallocator_mmc_response_request_sense(optcl_mmc_response *response)
{
    if (response == 0)
        return SUCCESS;

    assert(response->command_opcode == MMC_OPCODE_REQUEST_SENSE);
    if (response->command_opcode != MMC_OPCODE_REQUEST_SENSE)
        return E_CMNDINVOPCODE;

    free(response);
    return SUCCESS;
}


/*
 * Response deallocator table
 */

static struct response_deallocator_entry __deallocator_table[] = {
    { MMC_OPCODE_CLOSE_TRACK_SESSION,   deallocator_mmc_response_close_track_session	},
    { MMC_OPCODE_INQUIRY,               deallocator_mmc_response_get_configuration      },
    { MMC_OPCODE_GET_CONFIG,            deallocator_mmc_response_get_event_status       },
    { MMC_OPCODE_GET_EVENT_STATUS,      deallocator_mmc_response_inquiry                },
    { MMC_OPCODE_GET_PERFORMANCE,       deallocator_mmc_response_get_performance        },
    { MMC_OPCODE_MECHANISM_STATUS,      deallocator_mmc_response_mechanism_status       },
    { MMC_OPCODE_MODE_SENSE,            deallocator_mmc_response_sense_10               },
    { MMC_OPCODE_READ_10,               deallocator_mmc_response_read_10                },
    { MMC_OPCODE_READ_12,               deallocator_mmc_response_read_10                },
    { MMC_OPCODE_READ_BUFFER,           deallocator_mmc_response_read_buffer            },
    { MMC_OPCODE_READ_BUFFER_CAPACITY,  deallocator_mmc_response_read_buffer_capacity   },
    { MMC_OPCODE_READ_CD,               deallocator_mmc_response_read_cd                },
    { MMC_OPCODE_READ_MSN,              deallocator_mmc_response_read_msn               },
    { MMC_OPCODE_READ_TRACK_INFORMATION,deallocator_mmc_response_read_track_information },
    { MMC_OPCODE_REQUEST_SENSE,         deallocator_mmc_response_request_sense          }
};

static response_deallocator get_response_deallocator(uint16_t opcode)
{
    int i = 0;
    int elements = sizeof(__deallocator_table) / sizeof(__deallocator_table[0]);
    while (i < elements) {
        if (__deallocator_table[i].opcode == opcode)
            break;

        ++i;
    }

    if (i == elements)
        return 0;

    return __deallocator_table[i].deallocator;
}
