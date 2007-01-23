/*
    sysdevice.c - Platform dependent device functions.
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

#include <stdafx.h>

#include "command.h"
#include "device.h"
#include "sysdevice.h"
#include "transport.h"

#include <assert.h>
#include <malloc.h>
#include <memory.h>
#include <stddef.h>
#include <string.h>

#include <windows.h>
#include <ntddscsi.h>

#pragma warning(push)
/*
 * warning C4201: nonstandard extension used : nameless struct/union
 */
#pragma warning(disable: 4201)
#include <initguid.h>
#include <winioctl.h>
#pragma warning(pop)

#pragma warning(push)
/*
 * warning C4201: nonstandard extension used : nameless struct/union
 */
#pragma warning(disable: 4201)
#include <setupapi.h>
#pragma warning(pop)

#pragma warning(push)
/* 
 * warning C4005: macro redefinitions of macros defined in errors.h
 * NOTE that we must include our errors.h after all standard Win32
 *	headers.
 */
#pragma warning(disable: 4005)
#include "errors.h"
#pragma warning(pop)


/*
 * SCSI pass through structures *mostly copy-pasted from winioctl.h
 */

#define SPT_SENSE_LENGTH	32
#define SPTWB_DATA_LENGTH	512

typedef struct _SCSI_PASS_THROUGH_WITH_BUFFERS {
    SCSI_PASS_THROUGH spt;
    ULONG filler;      /* realign buffers to double word boundary */
    UCHAR ucSenseBuf[SPT_SENSE_LENGTH];
    UCHAR ucDataBuf[SPTWB_DATA_LENGTH];
} SCSI_PASS_THROUGH_WITH_BUFFERS;

typedef struct _SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER {
    SCSI_PASS_THROUGH_DIRECT sptd;
    ULONG filler;      /* realign buffer to double word boundary */
    UCHAR ucSenseBuf[SPT_SENSE_LENGTH];
} SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER;

typedef enum _STORAGE_PROPERTY_ID {
    StorageDeviceProperty = 0,
    StorageAdapterProperty,
    StorageDeviceIdProperty,
    StorageDeviceUniqueIdProperty,              // See storduid.h for details
    StorageDeviceWriteCacheProperty,
    StorageMiniportProperty,
    StorageAccessAlignmentProperty
} STORAGE_PROPERTY_ID;

typedef enum _STORAGE_QUERY_TYPE {
    PropertyStandardQuery = 0,          // Retrieves the descriptor
    PropertyExistsQuery,                // Used to test whether the descriptor is supported
    PropertyMaskQuery,                  // Used to retrieve a mask of writeable fields in the descriptor
    PropertyQueryMaxDefined     // use to validate the value
} STORAGE_QUERY_TYPE;

typedef struct _STORAGE_PROPERTY_QUERY {
    STORAGE_PROPERTY_ID PropertyId;
    STORAGE_QUERY_TYPE QueryType;
    BYTE  AdditionalParameters[1];
} STORAGE_PROPERTY_QUERY;

typedef struct _STORAGE_ADAPTER_DESCRIPTOR {
    DWORD Version;
    DWORD Size;
    DWORD MaximumTransferLength;
    DWORD MaximumPhysicalPages;
    DWORD AlignmentMask;
    BOOLEAN AdapterUsesPio;
    BOOLEAN AdapterScansDown;
    BOOLEAN CommandQueueing;
    BOOLEAN AcceleratedTransfer;

#if (NTDDI_VERSION < NTDDI_WINXP)
    BOOLEAN BusType;
#else
    BYTE  BusType;
#endif

    WORD   BusMajorVersion;
    WORD   BusMinorVersion;
} STORAGE_ADAPTER_DESCRIPTOR, *PSTORAGE_ADAPTER_DESCRIPTOR;

#define IOCTL_STORAGE_QUERY_PROPERTY	\
	CTL_CODE(IOCTL_STORAGE_BASE, 0x0500, METHOD_BUFFERED, FILE_ANY_ACCESS)

/*
 * Helper functions
 */

int enumerate_device_adapter(const char *path, optcl_adapter **adapter)
{
	int error;
	ULONG bytes;
	BOOL success;
	HANDLE hDevice;
	UCHAR outBuf[512];
	optcl_adapter *nadapter;
	STORAGE_PROPERTY_QUERY query;
	PSTORAGE_ADAPTER_DESCRIPTOR adpDesc;

	assert(path);
	assert(adapter);

	if (!path || !adapter)
		return E_INVALIDARG;

	hDevice = CreateFileA(
                path,					// device interface name
                GENERIC_READ | GENERIC_WRITE,		// dwDesiredAccess
                FILE_SHARE_READ | FILE_SHARE_WRITE,	// dwShareMode
                NULL,					// lpSecurityAttributes
                OPEN_EXISTING,				// dwCreationDistribution
                0,					// dwFlagsAndAttributes
                NULL					// hTemplateFile
                );

	if (!hDevice) {
		return MAKE_ERRORCODE(
			SEVERITY_ERROR, 
			FACILITY_DEVICE, 
			GetLastError()
			);
	}
                
	query.PropertyId = StorageAdapterProperty;
	query.QueryType = PropertyStandardQuery;

	success = DeviceIoControl(
		hDevice,                
                IOCTL_STORAGE_QUERY_PROPERTY,
                &query,
                sizeof(query),
                &outBuf,                   
                sizeof(outBuf),                      
                &bytes,      
                NULL                    
                );

	CloseHandle(hDevice);

	if (!success) {
		return MAKE_ERRORCODE(
			SEVERITY_ERROR, 
			FACILITY_DEVICE, 
			GetLastError()
			);
	}

	adpDesc = (PSTORAGE_ADAPTER_DESCRIPTOR)outBuf;

	error = optcl_adapter_create(&nadapter);

	if (FAILED(error))
		return error;

	error = optcl_adapter_set_bus_type(nadapter, adpDesc->BusType);

	if (FAILED(error)) {
		optcl_adapter_destroy(nadapter);
		return error;
	}

	error = optcl_adapter_set_max_alignment_mask(
		nadapter, 
		adpDesc->AlignmentMask
		);

	if (FAILED(error)) {
		optcl_adapter_destroy(nadapter);
		return error;
	}

	error = optcl_adapter_set_max_physical_pages(
		nadapter, 
		adpDesc->MaximumPhysicalPages
		);

	if (FAILED(error)) {
		optcl_adapter_destroy(nadapter);
		return error;
	}

	error = optcl_adapter_set_max_transfer_length(
		nadapter, 
		adpDesc->MaximumTransferLength
		);

	if (FAILED(error)) {
		optcl_adapter_destroy(nadapter);
		return error;
	}

	*adapter = nadapter;

	return SUCCESS;
}

static int enumerate_device(int index, HDEVINFO hDevInfo, optcl_device **device)
{
	int error;
	BOOL status;
	char *devicepath;
	DWORD dwReqSize;
	DWORD dwErrorCode;
	optcl_device *ndevice;
	optcl_adapter *adapter;
	optcl_mmc_inquiry command;
	optcl_mmc_result_inquiry *result;
	SP_DEVICE_INTERFACE_DATA interfaceData;
	PSP_DEVICE_INTERFACE_DETAIL_DATA_A pInterfaceDetailData;

	assert(device);
	assert(hDevInfo);
	assert(index >= 0);

	if (!device || !hDevInfo || index < 0)
		return E_INVALIDARG;

	interfaceData.cbSize = sizeof(interfaceData);

	status = SetupDiEnumDeviceInterfaces( 
                hDevInfo,				/* Interface Device Info handle */
                0,					/* Device Info data */
                (LPGUID)&GUID_DEVINTERFACE_CDROM,	/* Interface registered by driver */
                index,					/* Member */
                &interfaceData				/* Device Interface Data */
                );

	if (!status) {
		return MAKE_ERRORCODE(
			SEVERITY_ERROR, 
			FACILITY_DEVICE, 
			GetLastError()
			);
	}

	
	/* Find out required buffer size, so pass NULL */
	status = SetupDiGetDeviceInterfaceDetail(
		hDevInfo,		/* Interface Device info handle */
		&interfaceData,		/* Interface data for the event class */
		NULL,			/* Checking for buffer size */
		0,			/* Checking for buffer size */
		&dwReqSize,		/* Buffer size required to get the detail data */
		NULL			/* Checking for buffer size */
		);

	/*
	 * This call returns ERROR_INSUFFICIENT_BUFFER with reqSize 
	 * set to the required buffer size. Ignore the above error and
	 * pass a bigger buffer to get the detail data
	 */

	dwErrorCode = GetLastError();

	if (!status && dwErrorCode != ERROR_INSUFFICIENT_BUFFER) {
		return MAKE_ERRORCODE(
			SEVERITY_ERROR,
			FACILITY_DEVICE,
			dwErrorCode
			);
	}

	/*
	 * Allocate memory to get the interface detail data
	 * This contains the devicepath we need to open the device
	 */

	pInterfaceDetailData = malloc(dwReqSize);

	if (!pInterfaceDetailData)
		return E_OUTOFMEMORY;

	pInterfaceDetailData->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA_A);

	status = SetupDiGetDeviceInterfaceDetailA(
		hDevInfo,			/* Interface Device info handle */
		&interfaceData,			/* Interface data for the event class */
		pInterfaceDetailData,		/* Interface detail data */
		dwReqSize,			/* Interface detail data size */
		&dwReqSize,			/* Buffer size required to get the detail data */
		NULL);				/* Interface device info */

	if (!status) {
		free(pInterfaceDetailData);

		return MAKE_ERRORCODE(
			SEVERITY_ERROR,
			FACILITY_DEVICE,
			GetLastError()
			);
	}

	/*
	 * Now we have the device path.
	 * Create device structure and execute MM INQUIRY command.
	 */

	devicepath = _strdup(pInterfaceDetailData->DevicePath);

	if (!devicepath && pInterfaceDetailData->DevicePath) {
		free(pInterfaceDetailData);
		return E_OUTOFMEMORY;
	}

	free(pInterfaceDetailData);

	error = optcl_device_create(&ndevice);

	if (FAILED(error)) {
		free(devicepath);
		return error;
	}

	error = optcl_device_set_path(ndevice, devicepath);

	if (FAILED(error)) {
		free(devicepath);
		optcl_device_destroy(ndevice);
		return error;
	}

	error = enumerate_device_adapter(devicepath, &adapter);

	if (FAILED(error)) {
		optcl_device_destroy(ndevice);
		return error;
	}

	error = optcl_device_set_adapter(ndevice, adapter);

	if (FAILED(error)) {
		optcl_device_destroy(ndevice);
		return error;
	}

	memset(&command, 0, sizeof(command));

	error = optcl_command_inquiry(ndevice, &command, &result);

	if (FAILED(error)) {
		optcl_device_destroy(ndevice);
		return error;
	}

	*device = ndevice;

	return SUCCESS;
}

/*
 * System device functions
 */

int optcl_device_enumerate(optcl_list **devices)
{
	int index;
	int error;
	HDEVINFO hIntDevInfo;
	optcl_device *ndevice;

	assert(devices);

	if (!devices)
		return E_INVALIDARG;

	hIntDevInfo = SetupDiGetClassDevs(
		&GUID_DEVINTERFACE_CDROM,
		NULL,	/* Enumerator */
		NULL,	/* Parent Window */
		/* Only Devices present & Interface class */
		(DIGCF_PRESENT | DIGCF_INTERFACEDEVICE)
		);

	if (hIntDevInfo == INVALID_HANDLE_VALUE) {
		return MAKE_ERRORCODE(
			SEVERITY_ERROR, 
			FACILITY_DEVICE, 
			GetLastError()
			);
	}

	error = optcl_list_create(0, devices);

	if (FAILED(error))
		return error;

	for(index = 0; ; ++index) {
		error = enumerate_device(index, hIntDevInfo, &ndevice);

		if (FAILED(error))
			break;

		error = optcl_list_add_tail(*devices, ndevice);

		if (FAILED(error))
			break;
	}

	if (FAILED(error))
		optcl_list_destroy(*devices, 1);

	return error;
}

int optcl_device_command_execute(const optcl_device *device, 
				 const void *cdb,
				 int cdb_size,
				 void *param,
				 int param_size)
{
	int error;
	char *path;
	DWORD bytes;
	BOOL success;
	HANDLE hDevice;
	DWORD dwErrorCode;
	SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER sptdwb;

	assert(cdb);
	assert(device);
	assert(cdb_size > 0);
	assert(param_size >= 0);

	if (!cdb || !device || cdb_size < 0 || param_size < 0)
		return E_INVALIDARG;

	error = optcl_device_get_path(device, &path);

	if (FAILED(error))
		return error;
	
	hDevice = CreateFileA(
		path,					/* device interface name */
                GENERIC_READ | GENERIC_WRITE,		/* dwDesiredAccess */
                FILE_SHARE_READ | FILE_SHARE_WRITE,	/* dwShareMode */
                NULL,					/* lpSecurityAttributes */
                OPEN_EXISTING,				/* dwCreationDistribution */
                0,					/* dwFlagsAndAttributes */
                NULL					/* hTemplateFile */
                );

	free(path);

	if (!hDevice) {
		return MAKE_ERRORCODE(
			SEVERITY_ERROR, 
			FACILITY_DEVICE, 
			GetLastError()
			);
	}

	memset(&sptdwb, 0, sizeof(sptdwb));
	memcpy(sptdwb.sptd.Cdb, cdb, cdb_size);

	sptdwb.sptd.CdbLength = (UCHAR)cdb_size;
	sptdwb.sptd.DataBuffer = param;
	sptdwb.sptd.DataIn = SCSI_IOCTL_DATA_UNSPECIFIED;
	sptdwb.sptd.DataTransferLength = param_size;
	sptdwb.sptd.Length = sizeof(sptdwb.sptd);
	sptdwb.sptd.SenseInfoLength = sizeof(sptdwb.ucSenseBuf);
	sptdwb.sptd.SenseInfoOffset = offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER, ucSenseBuf);
	sptdwb.sptd.TargetId = 1;
	sptdwb.sptd.TimeOutValue = 2;

	/* Execute command */
	success = DeviceIoControl(
		hDevice,
		IOCTL_SCSI_PASS_THROUGH_DIRECT,
		&sptdwb,
		sizeof(sptdwb),
		&sptdwb,
		sizeof(sptdwb),
		&bytes,
		FALSE
		);

	dwErrorCode = GetLastError();

	if (!success && dwErrorCode != ERROR_INSUFFICIENT_BUFFER) {
		error = MAKE_ERRORCODE(
			SEVERITY_ERROR, 
			FACILITY_DEVICE, 
			dwErrorCode
			);
	}

	if (!success && bytes != 0)
		error = E_UNEXPECTED;

	CloseHandle(hDevice);

	return error;
}
