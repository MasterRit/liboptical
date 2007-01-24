/*
    profile.c - Optical device/media profile
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

#include "stdafx.h"

#include "errors.h"
#include "feature.h"

#include <assert.h>
#include <malloc.h>
#include <memory.h>


struct feature_sizes_entry {
	uint16_t code;
	uint16_t size;
};

static struct feature_sizes_entry __feature_struct_sizes[] = {
	{ FEATURE_PROFILE_LIST,			sizeof(optcl_feature_profile_list) },
	{ FEATURE_CORE,				sizeof(optcl_feature_core) },
	{ FEATURE_MORPHING,			sizeof(optcl_feature_morphing) },
	{ FEATURE_REMOVABLE_MEDIUM,		sizeof(optcl_feature_removable_medium) },
	{ FEATURE_WRITE_PROTECT,		sizeof(optcl_feature_write_protect) },
	{ FEATURE_RANDOM_READABLE,		sizeof(optcl_feature_random_readable) },
	{ FEATURE_MULTI_READ,			sizeof(optcl_feature_multy_read) },
	{ FEATURE_CD_READ,			sizeof(optcl_feature_cd_read) },
	{ FEATURE_DVD_READ,			sizeof(optcl_feature_dvd_read) },
	{ FEATURE_RANDOM_WRITEABLE,		sizeof(optcl_feature_random_writable) },
	{ FEATURE_INC_STREAMING_WRITABLE,	sizeof(optcl_feature_inc_streaming_writable) },
	{ FEATURE_SECTOR_ERASABLE,		sizeof(optcl_feature_sector_erasable) },
	{ FEATURE_FORMATTABLE,			sizeof(optcl_feature_formattable) },
	{ FEATURE_HW_DEFECT_MANAGEMENT,		sizeof(optcl_feature_hw_defect_mngmnt) },
	{ FEATURE_WRITE_ONCE,			sizeof(optcl_feature_write_once) },
	{ FEATURE_RESTRICTED_OVERWRITE,		sizeof(optcl_feature_restricted_ovr) },
	{ FEATURE_CDRW_CAV_WRITE,		sizeof(optcl_feature_cdrw_cav_write) },
	{ FEATURE_MRW,				sizeof(optcl_feature_mrw) },
	{ FEATURE_ENH_DEFECT_REPORTING,		sizeof(optcl_feature_enh_defect_reporting) },
	{ FEATURE_DVD_PLUS_RW,			sizeof(optcl_feature_dvd_plus_rw) },
	{ FEATURE_DVD_PLUS_R,			sizeof(optcl_feature_dvd_plus_r) },
	{ FEATURE_RIGID_RESTRICTED_OVERWRITE,	sizeof(optcl_feature_rigid_restricted_ovr) },
	{ FEATURE_CD_TAO,			sizeof(optcl_feature_cd_tao) },
	{ FEATURE_CD_MASTERING,			sizeof(optcl_feature_cd_mastering) },
	{ FEATURE_DVD_MINUS_R_MINUS_RW_WRITE,	sizeof(optcl_feature_dvd_minus_r_minus_rw_write) },
	{ FEATURE_LAYER_JUMP_RECORDING,		sizeof(optcl_feature_layer_jmp_rec) },
	{ FEATURE_CDRW_MEDIA_WRITE_SUPPORT,	sizeof(optcl_feature_cdrw_media_write) },
	{ FEATURE_BDR_POW,			sizeof(optcl_feature_bdr_pow) },
	{ FEATURE_DVD_PLUS_RW_DUAL_LAYER,	sizeof(optcl_feature_dvd_plus_rw_dual_layer) },
	{ FEATURE_DVD_PLUS_R_DUAL_LAYER,	sizeof(optcl_feature_dvd_r_plus_dual_layer) },
	{ FEATURE_BD_READ,			sizeof(optcl_feature_bd_read) },
	{ FEATURE_BD_WRITE,			sizeof(optcl_feature_bd_write) },
	{ FEATURE_TSR,				sizeof(optcl_feature_tsr) },
	{ FEATURE_HD_DVD_READ,			sizeof(optcl_feature_hd_dvd_read) },
	{ FEATURE_HD_DVD_WRITE,			sizeof(optcl_feature_hd_dvd_write) },
	{ FEATURE_HYBRID_DISC,			sizeof(optcl_feature_hybrid_disk) },
	{ FEATURE_POWER_MANAGEMENT,		sizeof(optcl_feature_power_mngmnt) },
	{ FEATURE_SMART,			sizeof(optcl_feature_smart) },
	{ FEATURE_EMBEDDED_CHANGER,		sizeof(optcl_feature_changer) },
	{ FEATURE_MICROCODE_UPGRADE,		sizeof(optcl_feature_microcode_upgrade) },
	{ FEATURE_TIMEOUT,			sizeof(optcl_feature_timeout) },
	{ FEATURE_DVD_CSS,			sizeof(optcl_feature_dvd_css) },
	{ FEATURE_RT_STREAMING,			sizeof(optcl_feature_rt_streaming) },
	{ FEATURE_DRIVE_SERIAL_NUMBER,		sizeof(optcl_feature_drive_serial_number) },
	{ FEATURE_MEDIA_SERIAL_NUMBER,		sizeof(optcl_feature_media_serial_number) },
	{ FEATURE_DCBS,				sizeof(optcl_feature_dcbs) },
	{ FEATURE_DVD_CPRM,			sizeof(optcl_feature_dvd_cprm) },
	{ FEATURE_FIRMWARE_INFO,		sizeof(optcl_feature_firmware_info) },
	{ FEATURE_AACS,				sizeof(optcl_feature_aacs) },
	{ FEATURE_VCPS,				sizeof(optcl_feature_vcps) }
};


static int get_feature_size(uint16_t feature_code)
{
	int i;
	int elements;

	elements = sizeof(__feature_struct_sizes) / sizeof(__feature_struct_sizes[0]);

	for(i = 0; i < elements; ++i) {
		if (__feature_struct_sizes[i].code == feature_code)
			break;
	}

	if (i >= elements)
		return -1;

	return __feature_struct_sizes[i].size;
}

int optcl_feature_copy(optcl_feature **dest, const optcl_feature *src)
{
	int size;

	assert(dest);
	assert(src);

	if (!dest || !src)
		return E_INVALIDARG;

	size = get_feature_size(src->feature_code);

	if (size <= 0)
		return E_DEVUNKNFEATURE;

	free(*dest);

	*dest = malloc(size);

	if (!*dest)
		return E_OUTOFMEMORY;

	memcpy(*dest, src, size);

	return SUCCESS;
}

int optcl_feature_create(optcl_feature **feature, uint16_t feature_code)
{
	int size;

	assert(feature);

	if (!feature)
		return E_INVALIDARG;

	size = get_feature_size(feature_code);

	if (size <= 0)
		return E_DEVUNKNFEATURE;

	*feature = malloc(size);

	if (!*feature)
		return E_OUTOFMEMORY;

	memset(*feature, 0, size);

	(*feature)->feature_code = feature_code;

	return SUCCESS;
}

int optcl_feature_destroy(optcl_feature *feature)
{
	assert(feature);

	if (!feature)
		return E_INVALIDARG;

	free(feature);

	return SUCCESS;
}
