/*
    sysdevice.c - Platform dependent device functions.
    Copyright (C) 2007  Aleksandar Dezelin <dezelin@gmail.com>

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

#include "command.h"
#include "debug.h"
#include "device.h"
#include "errors.h"
#include "helpers.h"
#include "list.h"
#include "sensedata.h"
#include "sysdevice.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <hal/libhal.h>
#include <scsi/sg.h>
#include <sys/ioctl.h>


#define CDB_MAX_LENGTH		16U
#define SPT_SENSE_LENGTH	32U
#define SCSI_COMMAND_TIMEOUT	30000U
#define SCSI_GENERIC_POSTFIX 	"_scsi_generic"


static const char* __storage_types[] = {
    "storage.cdrom"
};


static RESULT destroy_devices_list(optcl_list *devices)
{
    RESULT error;
    RESULT destroy_error;
    optcl_device *device = 0;
    optcl_list_iterator pos = 0;

    assert(devices != 0);

    if (devices == 0) {
        return(E_INVALIDARG);
    }

    error = optcl_list_get_head_pos(devices, &pos);

    if (FAILED(error)) {
        return(error);
    }

    while (pos != 0) {
        error = optcl_list_get_at_pos(devices, pos, (const pptr_t)&device);

        if (FAILED(error)) {
            break;
        }

        error = optcl_device_destroy(device);

        if (FAILED(error)) {
            break;
        }

        error = optcl_list_get_next(devices, pos, &pos);

        if (FAILED(error)) {
            break;
        }
    }

    destroy_error = optcl_list_destroy(devices, False);

    return(SUCCEEDED(destroy_error) ? error : destroy_error);
}

static RESULT
enumerate_device_adapter(optcl_device *device, optcl_adapter **adapter)
{
    RESULT error;
    RESULT destroy_error;
    optcl_adapter *nadapter = 0;

    assert(device != 0);
    assert(adapter != 0);

    if (device == 0 || adapter == 0) {
        return(E_INVALIDARG);
    }

    error = optcl_adapter_create(&nadapter);

    if (FAILED(error)) {
        return(error);
    }

    assert(nadapter != 0);

    if (nadapter == 0) {
        return(E_POINTER);
    }

    error = optcl_adapter_set_bus_type(nadapter, ADAPTER_BUSTYPE_SCSI);

    if (FAILED(error)) {
        destroy_error = optcl_adapter_destroy(nadapter);
        return(SUCCEEDED(destroy_error) ? error : destroy_error);
    }

    error = optcl_adapter_set_max_alignment_mask(nadapter, sizeof(void*));

    if (FAILED(error)) {
        destroy_error = optcl_adapter_destroy(nadapter);
        return(SUCCEEDED(destroy_error) ? error : destroy_error);
    }

    error = optcl_adapter_set_max_physical_pages(nadapter, 32);

    if (FAILED(error)) {
        destroy_error = optcl_adapter_destroy(nadapter);
        return(SUCCEEDED(destroy_error) ? error : destroy_error);
    }

    error = optcl_adapter_set_max_transfer_length(nadapter, 32 * getpagesize());

    if (FAILED(error)) {
        destroy_error = optcl_adapter_destroy(nadapter);
        return(SUCCEEDED(destroy_error) ? error : destroy_error);
    }

    *adapter = nadapter;

    return(error);
}

static RESULT
enumerate_device_features(optcl_device *device)
{
    RESULT error;
    RESULT destroy_error;
    optcl_list_iterator it = 0;
    optcl_feature *feature = 0;
    optcl_mmc_get_configuration command;
    optcl_mmc_response_get_configuration *response = 0;

    assert(device != 0);

    if (device == 0) {
        return(E_INVALIDARG);
    }

    command.rt = MMC_GET_CONFIG_RT_ALL;
    command.start_feature = 0;

    error = optcl_command_get_configuration(device, &command, &response);

    if (FAILED(error)) {
        return(error);
    }

    if (response == 0) {
        return(E_POINTER);
    }

    error = optcl_list_get_head_pos(response->descriptors, &it);

    if (FAILED(error)) {
        destroy_error = optcl_list_destroy(response->descriptors, 1);
        free(response);
        return(SUCCEEDED(destroy_error) ? error : destroy_error);
    }

    while (it != 0) {
        error = optcl_list_get_at_pos(response->descriptors, it, (const pptr_t)&feature);

        if (FAILED(error)) {
            break;
        }

        if (feature == 0) {
            error = E_POINTER;
            break;
        }

        error = optcl_device_set_feature(device, feature->feature_code, feature);

        if (FAILED(error)) {
            break;
        }

        error = optcl_list_get_next(response->descriptors, it, &it);

        if (FAILED(error)) {
            break;
        }
    }

    destroy_error = optcl_list_destroy(response->descriptors, False);

    free(response);

    if (FAILED(destroy_error)) {
        return(destroy_error);
    }

    return(SUCCEEDED(destroy_error) ? error : destroy_error);
}

static RESULT
get_device_attributes(LibHalContext *hal_ctx, const char *udi, optcl_device *device)
{
    RESULT error;
    RESULT destroy_error;
    size_t nudi_len;
    char *tmp = 0;
    char *nudi = 0;
    char *udi_parent = 0;
    DBusError dbus_error;
    char *ndevice_path = 0;
    optcl_adapter *adapter = 0;
    optcl_mmc_inquiry command;
    optcl_mmc_response_inquiry *response;

    assert(hal_ctx != 0);
    assert(udi != 0);
    assert(device != 0);

    if (hal_ctx == 0 || udi == 0 || device == 0) {
        return(E_INVALIDARG);
    }

    dbus_error_init(&dbus_error);

    udi_parent = libhal_device_get_property_string(hal_ctx, udi,
                 "info.parent", &dbus_error);

    if (udi_parent == 0 || dbus_error_is_set(&dbus_error)) {
        error = E_DEVINVALIDPATH;
        goto error_exit;
    }

    nudi_len = strlen(udi_parent) + strlen(SCSI_GENERIC_POSTFIX) + 1;
    nudi = malloc(nudi_len);

    if (nudi == 0) {
        error = E_OUTOFMEMORY;
        goto error_exit;
    }

    memset(nudi, 0, nudi_len);
    xstrcat(nudi, nudi_len, udi_parent);
    xstrcat(nudi, nudi_len, SCSI_GENERIC_POSTFIX);

    ndevice_path = libhal_device_get_property_string(hal_ctx, nudi,
                   "linux.device_file", &dbus_error);

    if (ndevice_path == 0 || dbus_error_is_set(&dbus_error)) {
        free(ndevice_path);
        error = E_DEVINVALIDPATH;
        goto error_exit;
    }

    error = optcl_device_set_path(device, ndevice_path);

    if (FAILED(error)) {
        free(ndevice_path);
        goto error_exit;
    }

    error = optcl_device_set_type(device, DEVICE_TYPE_CD_DVD);

    if (FAILED(error)) {
        goto error_exit;
    }

    error = enumerate_device_adapter(device, &adapter);

    if (FAILED(error)) {
        goto error_exit;
    }

    error = optcl_device_set_adapter(device, adapter);

    if (FAILED(error)) {
        goto error_exit;
    }

    memset(&command, 0, sizeof(command));

    error = optcl_command_inquiry(device, &command, &response);

    if (FAILED(error)) {
        goto error_exit;
    }

    error = optcl_device_set_type(device, response->device_type);

    if (FAILED(error)) {
        goto error_exit;
    }

    tmp = xstrdup((char*)response->product);

    if (tmp == 0) {
        error = E_OUTOFMEMORY;
        goto error_exit;
    }

    error = optcl_device_set_product(device, tmp);

    if (FAILED(error)) {
        free(tmp);
        goto error_exit;
    }

    tmp = xstrdup((char*)response->vendor);

    error = optcl_device_set_vendor(device, tmp);

    if (FAILED(error)) {
        free(tmp);
        goto error_exit;
    }

    tmp = xstrdup((char*)response->vendor_string);

    error = optcl_device_set_vendor_string(device, tmp);

    if (FAILED(error)) {
        free(tmp);
        goto error_exit;
    }

    error = enumerate_device_features(device);

error_exit:

    if (nudi != 0) {
        free(nudi);
    }

    if (udi_parent != 0) {
        free(udi_parent);
    }

    if (response != 0) {
        destroy_error = optcl_command_destroy_response((optcl_mmc_response*)response);
        error = SUCCEEDED(destroy_error) ? error : destroy_error;
    }

    LIBHAL_FREE_DBUS_ERROR(&dbus_error);

    return(error);
}

static RESULT
append_enumerated_drive(LibHalContext *hal_ctx,
                        const char *type, optcl_list *devices)
{
    int i;
    RESULT error;
    int device_num;
    DBusError dbus_error;
    char **all_cdroms = 0;
    optcl_device *device = 0;

    assert(hal_ctx != 0);
    assert(type != 0);
    assert(devices != 0);

    if (hal_ctx == 0 || type == 0 || devices == 0) {
        return(E_INVALIDARG);
    }

    dbus_error_init(&dbus_error);

    all_cdroms = libhal_find_device_by_capability(hal_ctx,
                 type, &device_num, &dbus_error);

    if (all_cdroms == 0 || dbus_error_is_set(&dbus_error)) {
        LIBHAL_FREE_DBUS_ERROR(&dbus_error);
        return(E_DEVINVALIDPATH);
    }

    LIBHAL_FREE_DBUS_ERROR(&dbus_error);

    for (i = 0; i < device_num; ++i) {
        error = optcl_device_create(&device);

        if (FAILED(error)) {
            break;
        }

        error = get_device_attributes(hal_ctx, all_cdroms[i], device);

        if (FAILED(error)) {
            break;
        }

        error = optcl_list_add_tail(devices, (const ptr_t)device);

        if (FAILED(error)) {
            break;
        }
    }

    libhal_free_string_array(all_cdroms);

    return(error);
}

RESULT optcl_device_enumerate(optcl_list **devices)
{
    int i;
    RESULT error;
    RESULT destroy_error = SUCCESS;
    DBusError dbus_error;
    LibHalContext *hal_ctx = 0;
    optcl_list *ndevices = 0;
    DBusConnection *connection = 0;

    assert(devices != 0);

    if (devices == 0) {
        error = E_INVALIDARG;
        goto error_exit;
    }

    dbus_error_init(&dbus_error);

    connection = dbus_bus_get(DBUS_BUS_SYSTEM, &dbus_error);

    if (connection == 0) {
        error = E_OUTOFMEMORY;
        goto error_exit;
    }

    hal_ctx = libhal_ctx_new();

    if (hal_ctx == 0) {
        error = E_OUTOFMEMORY;
        goto error_exit;
    }

    if (!libhal_ctx_set_dbus_connection(hal_ctx, connection)) {
        error = E_OUTOFMEMORY;
        goto error_exit;
    }

    if (!libhal_ctx_init(hal_ctx, &dbus_error)) {
        error = E_OUTOFMEMORY;
        goto error_exit;
    }

    error = optcl_list_create(0, &ndevices);

    if (FAILED(error)) {
        goto error_exit;
    }

    for (i = 0; i < sizeof(__storage_types) / sizeof(__storage_types[0]); ++i) {
        error = append_enumerated_drive(hal_ctx,
                                        __storage_types[i], ndevices);

        if (FAILED(error)) {
            break;
        }
    }

error_exit:

    if (SUCCEEDED(error)) {
        *devices = ndevices;
    } else {
        destroy_error = destroy_devices_list(ndevices);
    }

    if (hal_ctx != 0) {
        libhal_ctx_shutdown(hal_ctx, &dbus_error);
        libhal_ctx_free(hal_ctx);
    }

    LIBHAL_FREE_DBUS_ERROR(&dbus_error);

    return(SUCCEEDED(destroy_error) ? error : destroy_error);
}

RESULT
optcl_device_command_execute(const optcl_device *device,
                             const uint8_t cdb[], uint32_t cdb_size, uint8_t param[], uint32_t param_size)
{
    RESULT error;
    RESULT sense_code;

    int sg_fd;
    int sg_error;
    char *path = 0;
    sg_io_hdr_t sg_hdr;
    uint8_t command[CDB_MAX_LENGTH];
    uint8_t sense_buffer[SPT_SENSE_LENGTH];

    assert(cdb != 0);
    assert(device != 0);
    assert(cdb_size > 0);
    assert(cdb_size <= sizeof(command));

    if (cdb == 0 || device == 0 || cdb_size == 0 || cdb_size > sizeof(command)) {
        return(E_INVALIDARG);
    }

    error = optcl_device_get_path(device, &path);

    if (FAILED(error)) {
        return(error);
    }

    sg_fd = open(path, O_RDWR | O_EXCL);

    free(path);

    if (sg_fd < 0) {
        return(MAKE_ERRORCODE(SEVERITY_ERROR, FACILITY_DEVICE, errno));
    }

    memset(&sg_hdr, 0, sizeof(sg_hdr));
    memset(sense_buffer, 0, sizeof(sense_buffer));
    xmemcpy(command, sizeof(command), cdb, cdb_size);

    sg_hdr.interface_id = 'S';
    sg_hdr.dxfer_direction = SG_DXFER_TO_FROM_DEV;
    sg_hdr.cmd_len = (uint8_t)cdb_size;
    sg_hdr.mx_sb_len = sizeof(sense_buffer);
    sg_hdr.dxfer_len = param_size;
    sg_hdr.dxferp = param;
    sg_hdr.cmdp = command;
    sg_hdr.sbp = sense_buffer;
    sg_hdr.flags = SG_FLAG_DIRECT_IO;
    sg_hdr.timeout = SCSI_COMMAND_TIMEOUT;

    OPTCL_TRACE_ARRAY_MSG("CDB bytes:", cdb, cdb_size);
    OPTCL_TRACE_ARRAY_MSG("CDB parameter bytes:", param, param_size);

    sg_error = ioctl(sg_fd, SG_IO, &sg_hdr);

    close(sg_fd);

    if (sg_error < 0) {
        OPTCL_TRACE_ARRAY_MSG("ioctl(SG_IO) error code:", (uint8_t*)&errno, sizeof(errno));
        error = MAKE_ERRORCODE(SEVERITY_ERROR, FACILITY_DEVICE, errno);
    }

    OPTCL_TRACE_ARRAY_MSG("Device response bytes:", param, sg_hdr.dxfer_len);
    OPTCL_TRACE_ARRAY_MSG("Sense bytes:", sense_buffer, sg_hdr.sb_len_wr);

    if (SUCCEEDED(error) && sg_hdr.sb_len_wr > 0) {
        error = optcl_sensedata_get_code(sense_buffer,
                                         sg_hdr.sb_len_wr, &sense_code);

        if (SUCCEEDED(error)) {
            error = sense_code;
        }
    }

    return(error);
}
