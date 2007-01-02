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

#include "device.h"
#include "errors.h"
#include "sysdevice.h"
#include "transport.h"

#include <assert.h>
#include <malloc.h>
#include <memory.h>

#pragma warning(push)
/* 
 * warning C4005: macro redefinition of macros defined in errors.h
 */
#pragma warning(disable: 4005)
#include <windows.h>
#pragma warning(pop)

#pragma warning(push)
/*
 * warning C4201: nonstandard extension used : nameless struct/union
 */
#pragma warning(disable: 4201)
#include <winioctl.h>
#pragma warning(pop)

#include <setupapi.h>


static int enumerate_device(int index, HDEVINFO hDevInfo, optcl_device **device)
{
	BOOL status;
	HANDLE hDevice;
	DWORD dwReqSize;
	DWORD dwErrorCode;
	SP_DEVICE_INTERFACE_DATA interfaceData;
	PSP_DEVICE_INTERFACE_DETAIL_DATA pInterfaceDetailData;

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

	pInterfaceDetailData->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);

	status = SetupDiGetDeviceInterfaceDetail(
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
	 * Open the device interface to send Pass Through command.
	 */

	hDevice = CreateFile(
		pInterfaceDetailData->DevicePath,	/* device interface name */
		GENERIC_READ | GENERIC_WRITE,		/* dwDesiredAccess */
		FILE_SHARE_READ | FILE_SHARE_WRITE,	/* dwShareMode */
		NULL,					/* lpSecurityAttributes */
		OPEN_EXISTING,				/* dwCreationDistribution */
		0,					/* dwFlagsAndAttributes */
		NULL					/* hTemplateFile */
		);

	free(pInterfaceDetailData);

	if (hDevice == INVALID_HANDLE_VALUE) {
		return MAKE_ERRORCODE(
			SEVERITY_ERROR,
			FACILITY_DEVICE,
			GetLastError()
			);
	}

	return SUCCESS;
}

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
				 int mmc_opcode,
				 const void *argument,
				 int argsize,
				 void **result)
{
	int error;
	char *path;
	DWORD bytes;
	BOOL success;
	HANDLE hDevice;
	DWORD dwErrorCode;

	assert(device);
	assert(argument);

	if (!device || !argument)
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

	/* Get buffer size needed to hold output data */
	success = DeviceIoControl(
		hDevice,
		mmc_opcode,
		(void*)argument,
		argsize,
		NULL,
		0,
		&bytes,
		FALSE
		);

	dwErrorCode = GetLastError();

	if (!success && dwErrorCode != ERROR_INSUFFICIENT_BUFFER) {
		CloseHandle(hDevice);

		return MAKE_ERRORCODE(
			SEVERITY_ERROR, 
			FACILITY_DEVICE, 
			GetLastError()
			);
	}

	if (!success && bytes != 0) {
		CloseHandle(hDevice);
		return E_UNEXPECTED;
	}

	*result = malloc(bytes);

	if (!*result) {
		CloseHandle(hDevice);
		return E_OUTOFMEMORY;
	}

	/* Execute command */
	success = DeviceIoControl(
		hDevice,
		mmc_opcode,
		(void*)argument,
		argsize,
		*result,
		bytes,
		&bytes,
		FALSE
		);

	if (!success) {
		free(*result);
		CloseHandle(hDevice);

		return MAKE_ERRORCODE(
			SEVERITY_ERROR, 
			FACILITY_DEVICE, 
			GetLastError()
			);
	}

	CloseHandle(hDevice);

	return SUCCESS;
}
