/*
    feature.h - Optical device/media feature
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

#ifndef _FEATURE_H
#define _FEATURE_H

#include "errors.h"
#include "types.h"

/*
 * MMC Features
 */

#define FEATURE_PROFILE_LIST			0x0000
#define FEATURE_CORE				0x0001
#define FEATURE_MORPHING			0x0002
#define FEATURE_REMOVABLE_MEDIUM		0x0003
#define FEATURE_WRITE_PROTECT			0x0004

/* 0x0005 – 0x000F Reserved */

#define FEATURE_RANDOM_READABLE			0x0010

/* 0x0011 – 0x001C Reserved */

#define FEATURE_MULTI_READ			0x001D
#define FEATURE_CD_READ				0x001E
#define FEATURE_DVD_READ			0x001F
#define FEATURE_RANDOM_WRITABLE			0x0020
#define FEATURE_INC_STREAMING_WRITABLE		0x0021
#define FEATURE_SECTOR_ERASABLE			0x0022
#define FEATURE_FORMATTABLE			0x0023
#define FEATURE_HW_DEFECT_MANAGEMENT		0x0024
#define FEATURE_WRITE_ONCE			0x0025
#define FEATURE_RESTRICTED_OVERWRITE		0x0026
#define FEATURE_CDRW_CAV_WRITE			0x0027
#define FEATURE_MRW				0x0028
#define FEATURE_ENH_DEFECT_REPORTING		0x0029
#define FEATURE_DVD_PLUS_RW			0x002A
#define FEATURE_DVD_PLUS_R			0x002B
#define FEATURE_RIGID_RESTRICTED_OVERWRITE	0x002C
#define FEATURE_CD_TAO				0x002D
#define FEATURE_CD_MASTERING			0x002E
#define FEATURE_DVD_MINUS_R_MINUS_RW_WRITE	0x002F

/* 0x0030 – 0x0032 Legacy */

#define FEATURE_LAYER_JUMP_RECORDING		0x0033

/* 0x0034 – 0x0036 */

#define FEATURE_CDRW_MEDIA_WRITE_SUPPORT	0x0037
#define FEATURE_BDR_POW				0x0038

/* 0x0039 Reserved */

#define FEATURE_DVD_PLUS_RW_DUAL_LAYER		0x003A
#define FEATURE_DVD_PLUS_R_DUAL_LAYER		0x003B

/* 0x003C – 0x003F Reserved */

#define FEATURE_BD_READ				0x0040
#define FEATURE_BD_WRITE			0x0041
#define FEATURE_TSR				0x0042

/* 0x0043 – 0x004F Reserved */

#define FEATURE_HD_DVD_READ			0x0050
#define FEATURE_HD_DVD_WRITE			0x0051

/* 0x0052 – 0x007F Reserved */

#define FEATURE_HYBRID_DISC			0x0080

/* 0x0081 – 0x00FF Reserved */

#define FEATURE_POWER_MANAGEMENT		0x0100
#define FEATURE_SMART				0x0101
#define FEATURE_EMBEDDED_CHANGER		0x0102

/* 0x0103 Legacy */

#define FEATURE_MICROCODE_UPGRADE		0x0104
#define FEATURE_TIMEOUT				0x0105
#define FEATURE_DVD_CSS				0x0106
#define FEATURE_RT_STREAMING			0x0107
#define FEATURE_DRIVE_SERIAL_NUMBER		0x0108
#define FEATURE_MEDIA_SERIAL_NUMBER		0x0109
#define FEATURE_DCBS				0x010A
#define FEATURE_DVD_CPRM			0x010B
#define FEATURE_FIRMWARE_INFO			0x010C
#define FEATURE_AACS				0x010D

/* 0x010E – 0x010F Reserved */

#define FEATURE_VCPS				0x0110

/* 0x0111 – 0xFEFF Reserved */

/* 0xFF00 – 0xFFFF Vendor specific */

/*
 * Physical interface standard codes
 */

#define PIS_UNSPECIFIED				0x00000000L
#define PIS_SCSI_FAMILY				0x00000001L
#define PIS_ATAPI				0x00000002L
#define PIS_IEEE_1394_1995			0x00000003L
#define PIS_IEEE_1394A				0x00000004L
#define PIS_FIBRE_CHANNEL			0x00000005L
#define PIS_IEEE_1394B				0x00000006L
#define PIS_SERIAL_ATAPI			0x00000007L
#define PIS_USB					0x00000008L

/* 0x00000009 - 0x0000FFFE Reserved */

#define PIS_VENDOR_UNIQUE			0x0000FFFFL

/* 0x00010000 - 0x0001FFFF Defined by INCITS */
/* 0x00020000 - 0x0002FFFF Defined by SFF */
/* 0x00030000 - 0x0003FFFF Defined by IEEE */
/* 0x00040000 - 0xFFFFFFFF Reserved */

/*
 * Loading mechanism type 
 */

#define LMT_CADDY				0x0
#define LMT_TRAY				0x1
#define LMT_POP_UP				0x2

/* 0x3 Reserved */

#define LMT_CHANGER				0x4
#define LMT_CHANGER_MAGAZINE			0x5

/* 0x6 - 0x7 Reserved */


/* Feature descriptor header */
typedef struct tag_feature_descriptor {
	uint16_t feature_code;
	uint8_t version;
	uint8_t persistent;
	uint8_t current;
	uint8_t additional_length;
} optcl_feature_descriptor;

/* Feature typedef */
typedef optcl_feature_descriptor optcl_feature;

/*
 * Feature definitions
 */

/* Profile list */
typedef struct tag_feature_profile_list {
	optcl_feature_descriptor descriptor;
	uint8_t profile_count;
	uint16_t profile_numbers[64];
	bool_t current_profiles[64];
} optcl_feature_profile_list;

/* Core feature*/
typedef struct tag_feature_core {
	optcl_feature_descriptor descriptor;
	uint32_t phys_i_standard;
	uint8_t inq2;
	uint8_t dbe;
} optcl_feature_core;

/* Morphing feature */
typedef struct tag_feature_morphing {
	optcl_feature_descriptor descriptor;
	uint8_t ocevent;
	uint8_t async;
} optcl_feature_morphing;

/* Removable medium feature */
typedef struct tag_feature_removable_medium {
	optcl_feature_descriptor descriptor;
	uint8_t lmt;
	uint8_t eject;
	uint8_t pvnt_jmpr;
	uint8_t lock;
} optcl_feature_removable_medium;

/* Write protect feature */
typedef struct tag_feature_write_protect {
	optcl_feature_descriptor descriptor;
	uint8_t dwp;
	uint8_t wdcb;
	uint8_t spwp;
	uint8_t sswpp;
} optcl_feature_write_protect;

/* Random readable feature */
typedef struct tag_feature_random_readable {
	optcl_feature_descriptor descriptor;
	uint32_t logical_block_size;
	uint16_t blocking;
	uint8_t pp;
} optcl_feature_random_readable;

/* Multi read feature */
typedef struct tag_feature_multi_read {
	optcl_feature_descriptor descriptor;
} optcl_feature_multi_read;

/* CD read feature */
typedef struct tag_feature_cd_read {
	optcl_feature_descriptor descriptor;
	uint8_t dap;
	uint8_t c2_flags;
	uint8_t cd_text;
} optcl_feature_cd_read;

/* DVD read feature */
typedef struct tag_feature_dvd_read {
	optcl_feature_descriptor descriptor;
	uint8_t multi110;
	uint8_t dual_r;
} optcl_feature_dvd_read;

/* Random writable feature */
typedef struct tag_feature_random_writable {
	optcl_feature_descriptor descriptor;
	uint32_t last_logical_block;
	uint32_t logical_block_size;
	uint16_t blocking;
	uint8_t pp;
} optcl_feature_random_writable;

/* Incremental streaming writable feature */
typedef struct tag_feature_inc_streaming_writable {
	optcl_feature_descriptor descriptor;
	uint16_t supported_dbts;
	uint8_t trio;
	uint8_t arsv;
	uint8_t buf;
	uint8_t link_size_number;
	uint8_t link_sizes[256];
} optcl_feature_inc_streaming_writable;

/* Sector erasable feature */
typedef struct tag_feature_sector_erasable {
	optcl_feature_descriptor descriptor;
} optcl_feature_sector_erasable;

/* Formattable feature */
typedef struct tag_feature_formattable {
	optcl_feature_descriptor descriptor;
	uint8_t renosa;
	uint8_t expand;
	uint8_t qcert;
	uint8_t cert;
	uint8_t rrm;
} optcl_feature_formattable;

/* Hardware defect management feature */
typedef struct tag_feature_hw_defect_mngmnt {
	optcl_feature_descriptor descriptor;
	uint8_t ssa;
} optcl_feature_hw_defect_mngmnt;

/* Write once feature */
typedef struct tag_feature_write_once {
	optcl_feature_descriptor descriptor;
	uint32_t logical_block_size;
	uint16_t blocking;
	uint8_t pp;
} optcl_feature_write_once;

/* Restricted overwrite feature */
typedef struct tag_feature_restricted_ovr {
	optcl_feature_descriptor descriptor;
} optcl_feature_restricted_ovr;

/* CD-RW CAV write feature */
typedef struct tag_feature_cdrw_cav_write {
	optcl_feature_descriptor descriptor;
} optcl_feature_cdrw_cav_write;

/* MRW feature */
typedef struct tag_feature_mrw {
	optcl_feature_descriptor descriptor;
	uint8_t dvd_plus_write;
	uint8_t dvd_plus_read;
	uint8_t cd_write;
} optcl_feature_mrw;

/* Enhanced defect reporting feature */
typedef struct tag_feature_enh_defect_reporting {
	optcl_feature_descriptor descriptor;
	uint8_t drt_dm;
	uint8_t dbi_cache_zones_num;
	uint16_t entries_num;
} optcl_feature_enh_defect_reporting;

/* DVD+RW feature */
typedef struct tag_feature_dvd_plus_rw {
	optcl_feature_descriptor descriptor;
	uint8_t write;
	uint8_t quick_start;
	uint8_t close_only;
} optcl_feature_dvd_plus_rw;

/* DVD+R feature */
typedef struct tag_feature_dvd_plus_r {
	optcl_feature_descriptor descriptor;
	uint8_t write;
} optcl_feature_dvd_plus_r;

/* Rigid restricted overwrite feature */
typedef struct tag_feature_rigid_restricted_ovr {
	optcl_feature_descriptor descriptor;
	uint8_t dsdg;
	uint8_t dsdr;
	uint8_t intermediate;
	uint8_t blank;
} optcl_feature_rigid_restricted_ovr;

/* CD track at once feature */
typedef struct tag_feature_cd_tao {
	optcl_feature_descriptor descriptor;
	uint8_t buf;
	uint8_t rw_raw;
	uint8_t rw_pack;
	uint8_t test_write;
	uint8_t cd_rw;
	uint8_t rw_subcode;
	uint16_t data_type_supported;
} optcl_feature_cd_tao;

/* CD mastering (session at once) feature */
typedef struct tag_feature_cd_mastering {
	optcl_feature_descriptor descriptor;
	uint8_t buf;
	uint8_t sao;
	uint8_t raw_ms;
	uint8_t raw;
	uint8_t test_write;
	uint8_t cd_rw;
	uint8_t rw;
	uint32_t max_cue_length;
} optcl_feature_cd_mastering;

/* DVD-R/-RW write feature */
typedef struct tag_feature_dvd_minus_r_minus_rw_write {
	optcl_feature_descriptor descriptor;
	uint8_t buf;
	uint8_t rdl;
	uint8_t test_write;
	uint8_t dvd_rw;
} optcl_feature_dvd_minus_r_minus_rw_write;

/* Layer jump recording feature */
typedef struct tag_feature_layer_jmp_rec {
	optcl_feature_descriptor descriptor;
	uint8_t link_sizes_num;
	uint8_t link_sizes[256];
} optcl_feature_layer_jmp_rec;

/* CD-RW media write feature */
typedef struct tag_feature_cdrw_media_write {
	optcl_feature_descriptor descriptor;
	uint8_t subtype0;
	uint8_t subtype1;
	uint8_t subtype2;
	uint8_t subtype3;
	uint8_t subtype4;
	uint8_t subtype5;
	uint8_t subtype6;
	uint8_t subtype7;
} optcl_feature_cdrw_media_write;

/* BD-R pseudo-overwrite (POW) feature */
typedef struct tag_feature_bdr_pow {
	optcl_feature_descriptor descriptor;
} optcl_feature_bdr_pow;

/* DVD+RW dual layer feature */
typedef struct tag_feature_dvd_plus_rw_dual_layer {
	optcl_feature_descriptor descriptor;
	uint8_t write;
	uint8_t quick_start;
	uint8_t close_only;
} optcl_feature_dvd_plus_rw_dual_layer;

/* DVD+R dual layer feature */
typedef struct tag_feature_dvd_r_plus_dual_layer {
	optcl_feature_descriptor descriptor;
	uint8_t write;
} optcl_feature_dvd_r_plus_dual_layer;

/* BD read feature */
typedef struct tag_feature_bd_read {
	optcl_feature_descriptor descriptor;
	uint16_t bd_re_class0_bitmap;
	uint16_t bd_re_class1_bitmap;
	uint16_t bd_re_class2_bitmap;
	uint16_t bd_re_class3_bitmap;
	uint16_t bd_r_class0_bitmap;
	uint16_t bd_r_class1_bitmap;
	uint16_t bd_r_class2_bitmap;
	uint16_t bd_r_class3_bitmap;
	uint16_t bd_rom_class0_bitmap;
	uint16_t bd_rom_class1_bitmap;
	uint16_t bd_rom_class2_bitmap;
	uint16_t bd_rom_class3_bitmap;
} optcl_feature_bd_read;

/* BD write feature */
typedef struct tag_feature_bd_write {
	optcl_feature_descriptor descriptor;
	uint16_t bd_re_class0_bitmap;
	uint16_t bd_re_class1_bitmap;
	uint16_t bd_re_class2_bitmap;
	uint16_t bd_re_class3_bitmap;
	uint16_t bd_r_class0_bitmap;
	uint16_t bd_r_class1_bitmap;
	uint16_t bd_r_class2_bitmap;
	uint16_t bd_r_class3_bitmap;
} optcl_feature_bd_write;

/* TSR feature */
typedef struct tag_feature_tsr {
	optcl_feature_descriptor descriptor;
} optcl_feature_tsr;

/* HD DVD read feature */
typedef struct tag_feature_hd_dvd_read {
	optcl_feature_descriptor descriptor;
	uint8_t hd_dvd_r;
	uint8_t hd_dvd_ram;
} optcl_feature_hd_dvd_read;

/* HD DVD write feature */
typedef struct tag_feature_hd_dvd_write {
	optcl_feature_descriptor descriptor;
	uint8_t hd_dvd_r;
	uint8_t hd_dvd_ram;
} optcl_feature_hd_dvd_write;

/* Hybrid disk feature */
typedef struct tag_feature_hybrid_disk {
	optcl_feature_descriptor descriptor;
	uint8_t ri;
} optcl_feature_hybrid_disk;

/* Power management feature */
typedef struct tag_feature_power_mngmnt {
	optcl_feature_descriptor descriptor;
} optcl_feature_power_mngmnt;

/* S.M.A.R.T. feature */
typedef struct tag_feature_smart {
	optcl_feature_descriptor descriptor;
	uint8_t pp;
} optcl_feature_smart;

/* Embeded changer feature */
typedef struct tag_feature_changer {
	optcl_feature_descriptor descriptor;
	uint8_t scc;
	uint8_t sdp;
	uint8_t highest_slot_num;
} optcl_feature_changer;

/* Microcode upgrade feature */
typedef struct tag_feature_microcode_upgrade {
	optcl_feature_descriptor descriptor;
	uint8_t m5;
} optcl_feature_microcode_upgrade;

/* Timeout feature */
typedef struct tag_feature_timeout {
	optcl_feature_descriptor descriptor;
	uint8_t group3;
	uint16_t unit_length;
} optcl_feature_timeout;

/* DVD CSS feature */
typedef struct tag_feature_dvd_css {
	optcl_feature_descriptor descriptor;
	uint8_t css_version;
} optcl_feature_dvd_css;

/* Real time streaming feature */
typedef struct tag_feature_rt_streaming {
	optcl_feature_descriptor descriptor;
	uint8_t rbcb;
	uint8_t scs;
	uint8_t mp2a;
	uint8_t wspd;
	uint8_t sw;
} optcl_feature_rt_streaming;

/* Drive serial number feature */
typedef struct tag_feature_drive_serial_number {
	optcl_feature_descriptor descriptor;
	uint8_t serial_number[1024];
} optcl_feature_drive_serial_number;

/* Media serial number feature */
typedef struct tag_feature_media_serial_number {
	optcl_feature_descriptor descriptor;
} optcl_feature_media_serial_number;

/* Disc control blocks (DCBs) feature */
typedef struct tag_feature_dcbs {
	optcl_feature_descriptor descriptor;
	uint32_t dcb_entries[256];
} optcl_feature_dcbs;

/* DVD CPRM feature */
typedef struct tag_feature_dvd_cprm {
	optcl_feature_descriptor descriptor;
	uint8_t cprm_version;
} optcl_feature_dvd_cprm;

/* Firmware information feature */
typedef struct tag_feature_firmware_info {
	optcl_feature_descriptor descriptor;
	uint16_t century;
	uint16_t year;
	uint16_t month;
	uint16_t day;
	uint16_t hour;
	uint16_t minute;
	uint16_t second;
} optcl_feature_firmware_info;

/* AACS feature */
typedef struct tag_feature_aacs {
	optcl_feature_descriptor descriptor;
	uint8_t bng;
	uint8_t block_count;
	uint8_t agids_num;
	uint8_t aacs_version;
} optcl_feature_aacs;

/* VCPS feature */
typedef struct tag_feature_vcps {
	optcl_feature_descriptor descriptor;
} optcl_feature_vcps;


/*
 * Feature functions
 */

/* Copy feature */
extern RESULT optcl_feature_copy(optcl_feature **dest, 
				 const optcl_feature *src);

/* Create feature structure */
extern RESULT optcl_feature_create(uint16_t feature_code,
				   optcl_feature **feature);

/* Create feature from raw MMC data */
extern RESULT optcl_feature_create_from_raw(optcl_feature **feature,
					    const uint8_t mmc_data[],
					    uint32_t size);

/* Create feature descriptor from raw MMC data */
extern RESULT optcl_feature_create_descriptor(optcl_feature_descriptor **descriptor,
					      const uint8_t mmc_data[],
					      uint32_t size);


/* Destroy feature structure */
extern RESULT optcl_feature_destroy(optcl_feature *feature);


#endif /* _FEATURE_H */
