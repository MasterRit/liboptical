/*
    command.h - Multi-Media Commands - 5 (MMC-5).
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

#ifndef _COMMAND_H
#define _COMMAND_H

#include "device.h"
#include "errors.h"
#include "list.h"
#include "sensedata.h"
#include "types.h"


/*
 * BLANK command command field flags
 */

#define MMC_BLANK_BLANK_DISK			0x00
#define MMC_BLANK_MINIMAL_BLANK_DISK		0x01
#define MMC_BLANK_BLANK_TRACK			0x02
#define MMC_BLANK_UNRESERVE_TRACK		0x03
#define MMC_BLANK_TRACK_TRAIL			0x04
#define MMC_BLANK_UNCLOSE_LAST_SESSION		0x05
#define MMC_BLANK_LAST_SESSION			0x06


/*
 * CLOSE TRACK SESSION command field flags
 */

#define MMC_CTS_CD_R_RW_CLOSE_LOGICAL_TRACK			0x01
#define MMC_CTS_CD_R_RW_CLOSE_SESSION_FINALIZE			0x02
#define MMC_CTS_DVD_R_RW_CLOSE_LOGICAL_TRACK			0x01
#define MMC_CTS_DVD_R_RW_CLOSE_SESSION_FINALIZE			0x02
#define MMC_CTS_DVD_R_RW_FINALIZE_DVD_RW_DISC			0x03
#define MMC_CTS_DVD_R_DL_CLOSE_LOGICAL_TRACK			0x01
#define MMC_CTS_DVD_R_DL_CLOSE_SESSION_FINALIZE			0x02
#define MMC_CTS_DVD_PLUS_R_CLOSE_LOGICAL_TRACK			0x01
#define MMC_CTS_DVD_PLUS_R_CLOSE_SESSION			0x02
#define MMC_CTS_DVD_PLUS_R_FINALIZE_30MM_RADIUS			0x05
#define MMC_CTS_DVD_PLUS_R_FINALIZE_DISC			0x06
#define MMC_CTS_DVD_PLUS_R_DL_CLOSE_LOGICAL_TRACK		0x01
#define MMC_CTS_DVD_PLUS_R_DL_CLOSE_SESSION			0x02
#define MMC_CTS_DVD_PLUS_R_DL_REC_EXT_LEADOUT			0x04
#define MMC_CTS_DVD_PLUS_R_DL_FINALIZE_30MM_RADIUS		0x05
#define MMC_CTS_DVD_PLUS_R_DL_FINALIZE_DISC			0x06
#define MMC_CTS_DVD_PLUS_RW_QUICKSTOP_BG_FORMAT			0x00
#define MMC_CTS_DVD_PLUS_RW_COMPSTOP_BG_FORMAT_30MM_RADIUS	0x02
#define MMC_CTS_DVD_PLUS_RW_COMPSTOP_BG_FORMAT			0x03
#define MMC_CTS_DVD_PLUS_RW_DL_QUICKSTOP_BG_FORMAT		0x00
#define MMC_CTS_DVD_PLUS_RW_DL_COMPSTOP_BG_FORMAT_30MM_RADIUS	0x02
#define MMC_CTS_DVD_PLUS_RW_DL_COMPSTOP_BG_FORMAT		0x03
#define MMC_CTS_HD_DVD_R_CLOSE_LOGICAL_TRACK			0x01
#define MMC_CTS_HD_DVD_R_CLOSE_SESSION				0x02
#define MMC_CTS_HD_DVD_R_FINALIZE_DISC				0x06
#define MMC_CTS_BD_R_CLOSE_LOGICAL_TRACK			0x01
#define MMC_CTS_BD_R_CLOSE_SESSION				0x02
#define MMC_CTS_BD_R_FINALIZE_DISC				0x06
#define MMC_CTS_MRW_STOP_BG_FORMAT				0x02


/*
 * FORMAT UNIT command field flags
 */

/* Format types */
#define MMC_FORMAT_FULL_FORMAT					0x00
#define MMC_FORMAT_SPARE_AREA_EXPANSION				0x01
#define MMC_FORMAT_ZONE_REFORMAT				0x04
#define MMC_FORMAT_ZONE_FORMAT					0x05
#define MMC_FORMAT_CD_RW_DVD_RW_FULL_FORMAT			0x10
#define MMC_FORMAT_CD_RW_DVD_RW_GROW_SESSION			0x11
#define MMC_FORMAT_CD_RW_DVD_RW_ADD_SESSION			0x12
#define MMC_FORMAT_DVD_RW_QUICK_GROW_LAST_BORDER		0x13
#define MMC_FORMAT_DVD_RW_QUICK_ADD_BORDER			0x14
#define MMC_FORMAT_DVD_RW_QUICK_FORMAT				0x15
#define MMC_FORMAT_HD_DVD_R_TEST_ZONE_EXPANSION			0x16
#define MMC_FORMAT_FULL_FORMAT_WITH_SPARING_PARAMS		0x20
#define MMC_FORMAT_MRW_FORMAT					0x24
#define MMC_FORMAT_DVD_PLUS_RW_BASIC_FORMAT			0x26
#define MMC_FORMAT_BD_RE_FULL_FORMAT_WITH_SPARE_AREAS		0x30
#define MMC_FORMAT_BD_RE_FULL_FORMAT_WITHOUT_SPARE_AREAS	0x31
#define MMC_FORMAT_BD_R_FULL_FORMAT_WITH_SPARE_AREAS		0x32

/* Format sub-types for BD-R discs*/
#define MMC_FORMAT_SUBTYPE_BD_R_SRM_PLUS_POW			0x00
#define MMC_FORMAT_SUBTYPE_BD_R_SRM_POW				0x01
#define MMC_FORMAT_SUBTYPE_BD_R_RRM				0x02

/* Format sub-types for BD-RE discs with spare areas */
#define MMC_FORMAT_SUBTYPE_BD_RE_QUICK_REFORMAT			0x00
#define MMC_FORMAT_SUBTYPE_BD_RE_NO_CERTIFICATION		0x01
#define MMC_FORMAT_SUBTYPE_BD_RE_FULL_CERTIFICATION		0x02
#define MMC_FORMAT_SUBTYPE_BD_RE_QUICK_CERTIFICATION		0x03


/*
 * GET CONFIGURATION command field flags
 */

#define MMC_GET_CONFIG_RT_ALL			0x00
#define MMC_GET_CONFIG_RT_CURRENT		0x01
#define MMC_GET_CONFIG_RT_FROM			0x02


/*
 * GET EVENT STATUS NOTIFICATION command field flags
 */

#define MMC_GET_EVENT_STATUS_OPCHANGE		0x01
#define MMC_GET_EVENT_STATUS_POWERMGMT		0x02
#define MMC_GET_EVENT_STATUS_EXTREQUEST		0x04
#define MMC_GET_EVENT_STATUS_MEDIA		0x08
#define MMC_GET_EVENT_STATUS_MULTIHOST		0x10
#define MMC_GET_EVENT_STATUS_DEVICEBUSY		0x20

#define EVENT_OC_EC_NOCHG			0x00
#define EVENT_OC_EC_CHANGED			0x02
#define EVENT_OC_OC_NOCHG			0x00
#define EVENT_OC_OC_FEATCHANGE			0x02
#define EVENT_PM_EC_NOCHG			0x00
#define EVENT_PM_EC_PWRCHGOK			0x01
#define EVENT_PM_EC_PWRCHGFAIL			0x02
#define EVENT_PM_PS_ACTIVE			0x01
#define EVENT_PM_PS_IDLE			0x02
#define EVENT_PM_PS_STANDBY			0x03
#define EVENT_PM_PS_SLEEP			0x04
#define EVENT_ER_EC_NOCHG			0x00
#define EVENT_ER_EC_DRIVEKEYDOWN		0x01
#define EVENT_ER_EC_DRIVEKEYUP			0x02
#define EVENT_ER_EC_EXTREQNOT			0x03
#define EVENT_ER_ERC_READY			0x00
#define EVENT_ER_ERC_OTHERPREVENT		0x01

#define EVENT_ER_ER_NOREQUEST			0x0000
#define EVENT_ER_ER_OVERRUN			0x0001
#define EVENT_ER_ER_PLAY			0x0101
#define EVENT_ER_ER_REWIND			0x0102
#define EVENT_ER_ER_FASTFORWARD			0x0103
#define EVENT_ER_ER_PAUSE			0x0104
#define EVENT_ER_ER_STOP			0x0106

/* 0x0200 - 0x02FF ASCII button */
/* 0xF000 - 0xFFFF Vendor unique */

#define EVENT_MEDIA_EC_NOCHG			0x00
#define EVENT_MEDIA_EC_EJECTREQUEST		0x01
#define EVENT_MEDIA_EC_NEWMEDIA			0x02
#define EVENT_MEDIA_EC_MEDIAREMOVAL		0x03
#define EVENT_MEDIA_EC_MEDIACHANGED		0x04
#define EVENT_MEDIA_EC_BGFORMATCOMPLETE		0x05
#define EVENT_MEDIA_EC_BGFORMATRESTART		0x06

#define EVENT_MH_EC_NOCHG			0x00
#define EVENT_MH_EC_CTRLREQUEST			0x01
#define EVENT_MH_EC_CTRLGRANT			0x02
#define EVENT_MH_EC_CTRLRELEASE			0x03
#define EVENT_MH_MHS_READY			0x00
#define EVENT_MH_MHS_OTHERPREVENT		0x01
#define EVENT_MH_MHP_NOREQUEST			0x00
#define EVENT_MH_MHP_LOW			0x01
#define EVENT_MH_MHP_MEDIUM			0x02
#define EVENT_MH_MHP_HIGH			0x03

#define EVENT_DB_EC_NOCHG			0x00
#define EVENT_DB_EC_CHANGE			0x01
#define EVENT_DB_DBS_NOTBUSY			0x00
#define EVENT_DB_DBS_BUSY			0x01


/*
 * GET PERFORMANCE command field flags
 */

#define MMC_GET_PERF_PERFOMANCE_DATA		0x00
#define MMC_GET_PERF_UNUSABLE_AREA_DATA		0x01
#define MMC_GET_PERF_DEFECT_STATUS_DATA		0x02
#define MMC_GET_PERF_WRITE_SPEED_DESCRIPTOR	0x03
#define MMC_GET_PERF_DBI			0x04
#define MMC_GET_PERF_DBI_CACHE_ZONE		0x05

/* 0x06 - 0xFF Reserved */

#define PERFORMANCE_READ_NOMINAL		0x10
#define PERFORMANCE_READ_ENTIRE			0x11
#define PERFORMANCE_READ_EXCEPTIONS		0x12
#define PERFORMANCE_WRITE_NOMINAL		0x14
#define PERFORMANCE_WRITE_ENTIRE		0x15
#define PERFORMANCE_WRITE_EXCEPTIONS		0x16

#define UAE_PBI					0x00
#define UAE_SAI					0x01
#define UAE_DBI					0x02

#define WRC_DEFAULT				0x00
#define WRC_CAV					0x01

#define ELT_RECOVERED_LIGHT			0x00
#define ELT_RECOVERED_HEAVY_DEFECTS		0x01
#define ELT_RECOVERED_UNRECOVERED		0x02
#define ELT_RECOVERED_WRITE_ERROR		0x03

/*
 * MECHANICAL STATUS command field flags
 */

#define CHANGER_STATE_READY			0x00
#define CHANGER_STATE_LOADING			0x01
#define CHANGER_STATE_UNLOADING			0x02
#define CHANGER_STATE_INITIALIZING		0x03

#define MECHANISM_STATE_IDLE			0x00
#define MECHANISM_STATE_LEGACY_PLAYING		0x01
#define MECHANISM_STATE_LEGACY_SCANNING		0x02
#define MECHANISM_STATE_LEGACY_ACTIVE		0x03

/* 0x04 - 0x06 reserved */

#define MECHANISM_STATE_LEGACY_NO_STATE		0x07


/*
 * MODE SENSE command field flags
 */

#define SENSE_PAGECTRL_CURRENT			0x00
#define SENSE_PAGECTRL_CHANGEABLE		0x01
#define SENSE_PAGECTRL_DEFAULT			0x02
#define SENSE_PAGECTRL_SAVED			0x03

#define SENSE_MODEPAGE_VENDOR			0x00
#define SENSE_MODEPAGE_RW_ERROR			0x01
#define SENSE_MODEPAGE_MRW			0x02
#define SENSE_MODEPAGE_WRITE_PARAM		0x05
#define SENSE_MODEPAGE_CACHING			0x08
#define SENSE_MODEPAGE_PWR_CONDITION		0x1A
#define SENSE_MODEPAGE_INFO_EXCEPTIONS		0x1C
#define SENSE_MODEPAGE_TIMEOUT_PROTECT		0x1D

#define SENSE_WT_PACKET				0x00
#define SENSE_WT_TAO				0x01
#define SENSE_WT_SAO				0x02
#define SENSE_WT_RAW				0x03
#define SENSE_WT_LJR				0x04

/* 0x05 - 0xFF Reserved */

#define SENSE_MS_NB0PTR				0x00
#define SENSE_MS_B0PTR_FF			0x01

/* 0x02 - Reserved */

#define SENSE_MS_B0PTR_NPPA			0x03

#define SENSE_DBT_RAW				0x00
#define SENSE_DBT_RAW_PQSC			0x01
#define SENSE_DBT_RAW_PWSC			0x02
#define SENSE_DBT_RAW_PWSC_RAW			0x03

/* 0x04 - 0x06 Reserved */

#define SENSE_DBT_VENDOR0			0x07
#define SENSE_DBT_MODE1				0x08
#define SENSE_DBT_MODE2				0x09
#define SENSE_DBT_MODE2_2048			0x0A
#define SENSE_DBT_MODE2_2056			0x0B
#define SENSE_DBT_MODE2_2324			0x0C
#define SENSE_DBT_MODE_MIXED			0x0D

/* 0x0E - reserved */

#define SENSE_DBT_VENDOR2			0x0F

#define SENSE_SFC_CDDA_CDROM			0x00
#define SENSE_SFC_CDI				0x10
#define SENSE_SFC_CDROMXA			0x20

#define SENSE_MRIE_NROIEC			0x00
#define SENSE_MRIE_AER				0x01
#define SENSE_MRIE_GUA				0x02
#define SENSE_MRIE_CGRE				0x03
#define SENSE_MRIE_UGRE				0x04
#define SENSE_MRIE_GNS				0x05
#define SENSE_MRIE_ORIECOR			0x06

/* 0x07 - 0x0B Reserved */
/* 0x0C - 0x0F Vendor specific */


/*
 * READ BUFFER command field flags
 */

#define MMC_READ_BUFFER_MODE_COMBINED		0x00
#define MMC_READ_BUFFER_MODE_VENDOR		0x01
#define MMC_READ_BUFFER_MODE_DATA		0x02
#define MMC_READ_BUFFER_MODE_DESCRIPTOR		0x03
#define MMC_READ_BUFFER_MODE_ECHO		0x0A
#define MMC_READ_BUFFER_MODE_ECHO_DESC		0x0B
#define MMC_READ_BUFFER_MODE_EXPANDER		0x1A

/* 0x04 - 0x09 Reserved */
/* 0x0C - 0x19 Reserved */
/* 0x1B - 0x1F Reserved */


/*
 * READ CD command field flags
 */

/* Expected sector types definitions */
#define MMC_READ_CD_EST_ALL		0x00
#define MMC_READ_CD_EST_CDDA		0x01
#define MMC_READ_CD_EST_MODE1		0x02
#define MMC_READ_CD_EST_MODE2_FORMLESS	0x03
#define MMC_READ_CD_EST_MODE2_FORM1	0x04
#define MMC_READ_CD_EST_MODE2_FORM2	0x05

/* 0x06 - 0x07 Reserved */

/* Main Channel Selection Bits */
#define MMC_READ_CD_MCSB_NO_HEADER		0x00
#define MMC_READ_CD_MCSB_4BYTE_HEADER		0x01
#define MMC_READ_CD_MCSB_8BYTE_SUBHEADER	0x02
#define MMC_READ_CD_MCSB_BOTH			0x03

/* C2 Error Information */
#define MMC_READ_CD_C2EI_NO_ERROR		0x00
#define MMC_READ_CD_C2EI_C2EC294		0x01
#define MMC_READ_CD_C2EI_C2EC296		0x02


/*
 * SET READ SPEED command field flags
 */

#define MMC_SET_CD_SPEED_RC_CLV_NPCAV	0x00
#define MMC_SET_CD_SPEED_RC_PCAV	0x01

/* 0x02 - 0x03 Reserved */


/*
 * Common to all commands
 */

typedef struct tag_mmc_response {
	uint16_t command_opcode;
} optcl_mmc_response;


/*
 * BLANK command structures
 */

typedef struct tag_mmc_blank {
	bool_t immed;
	uint8_t blanking_type;
	uint32_t start_address;
} optcl_mmc_blank;


/*
 * CLOSE TRACK SESSION command structures
 */

typedef struct tag_mmc_close_track_session {
	bool_t immed;
	uint8_t close_function;
	uint16_t logical_track_number;
} optcl_mmc_close_track_session;


/*
 * FORMAT UNIT command structures
 */

typedef struct tag_mmc_format_unit {
	bool_t cmplist;
	bool_t fov;
	bool_t dcrt;
	bool_t try_out;
	bool_t immed;
	bool_t vs;
	uint32_t num_of_blocks;
	uint8_t format_type;
	uint8_t format_subtype;

	union tag_type_dependent {
		struct tag_ff_with_sparing {
			uint8_t m;
			uint8_t n;
		} ff_with_sparing;

		struct tag_dvd_plus_rw_basicf {
			bool_t quick_start;
			bool_t restart;
		} dvd_plus_rw_basicf;

		struct tag_bd_r_with_spare_areas {
			bool_t isa_v;
			bool_t tdma_v;
			uint8_t sadp;
			uint8_t tdmadp;
		} bd_r_with_spare_areas;

		struct tag_other {
			uint32_t type_dependent;
		} other;
	} type_dependant;
} optcl_mmc_format_unit;


/*
 * GET CONFIGURATION command structures
 */

typedef struct tag_mmc_get_configuration {
	uint8_t rt;
	uint16_t start_feature;
} optcl_mmc_get_configuration;

typedef struct tag_mmc_response_get_configuration {
	optcl_mmc_response header;
	uint32_t data_length;
	uint16_t current_profile;
	optcl_list *descriptors;
} optcl_mmc_response_get_configuration;


/*
 * GET EVENT STATUS NOTIFICATION command structures
 */

typedef struct tag_mmc_get_event_status {
	bool_t polled;
	uint8_t class_request;
	uint16_t allocation_length;
} optcl_mmc_get_event_status;

typedef struct tag_mmc_ges_header {
	bool_t nea;
	uint8_t event_class;
	uint8_t notification_class;
	uint16_t descriptor_len;
} optcl_mmc_ges_header;

typedef struct tag_mmc_ges_operational_change {
	optcl_mmc_ges_header header;
	uint8_t status;
	uint8_t event_code;
	bool_t persistent_prev;
	uint16_t change;
} optcl_mmc_ges_operational_change;

typedef struct tag_mmc_ges_power_management {
	optcl_mmc_ges_header header;
	uint8_t event_code;
	uint8_t power_status;
} optcl_mmc_ges_power_management;

typedef struct tag_mmc_ges_external_request {
	optcl_mmc_ges_header header;
	bool_t persistent_prev;
	uint8_t event_code;
	uint8_t ext_req_status;
	uint16_t external_request;
} optcl_mmc_ges_external_request;

typedef struct tag_mmc_ges_media {
	optcl_mmc_ges_header header;
	uint8_t event_code;
	bool_t media_present;
	bool_t tray_open;
	uint8_t start_slot;
	uint8_t end_slot;
} optcl_mmc_ges_media;

typedef struct tag_mmc_ges_multihost {
	optcl_mmc_ges_header header;
	bool_t persistent_prev;
	uint8_t event_code;
	uint8_t multi_host_status;
	uint16_t multi_host_priority;
} optcl_mmc_ges_multihost;

typedef struct tag_mmc_ges_device_busy {
	optcl_mmc_ges_header header;
	uint8_t event_code;
	uint8_t busy_status;
	uint16_t time;
} optcl_mmc_ges_device_busy;

typedef struct tag_mmc_response_get_event_status {
	optcl_mmc_response header;
	optcl_mmc_ges_header ges_header;
	uint8_t event_class;
	optcl_list *descriptors;
} optcl_mmc_response_get_event_status;

typedef optcl_mmc_ges_header optcl_mmc_ges_descriptor;


/*
 * GET PERFORMANCE command structures
 */

typedef struct tag_mmc_get_performance {
	uint8_t data_type;
	uint32_t start_lba;
	uint16_t max_desc_num;
	uint8_t type;
} optcl_mmc_get_performance;

typedef struct tag_mmc_gpdesc_header {
	uint8_t descriptor_type;
} optcl_mmc_gpdesc_header;

typedef struct tag_mmc_gpdesc_pd {
	optcl_mmc_gpdesc_header header;
	uint8_t data_type;

	union tag_nominal {
		uint32_t start_lba;
		uint32_t end_lba;
		uint32_t start_performance;
		uint32_t end_performance;
	} nominal;
	union tag_exceptions {
		uint32_t lba;
		uint16_t time;
	} exceptions;
}  optcl_mmc_gpdesc_pd;

typedef struct tag_mmc_gpdesc_uad {
	optcl_mmc_gpdesc_header header;
	uint32_t lba;
	uint32_t upb_num;
} optcl_mmc_gpdesc_uad;

typedef struct tag_mmc_gpdesc_dsd {
	optcl_mmc_gpdesc_header header;
	uint32_t start_lba;
	uint32_t end_lba;
	uint8_t blocking_factor;
	uint8_t fbo;
	uint8_t defect_statuses[2038];
} optcl_mmc_gpdesc_dsd;

typedef struct tag_mmc_gpdesc_wsd {
	optcl_mmc_gpdesc_header header;
	uint8_t wrc;
	bool_t rdd;
	bool_t exact;
	bool_t mrw;
	uint32_t end_lba;
	uint32_t read_speed;
	uint32_t write_speed;
} optcl_mmc_gpdesc_wsd;

typedef struct tag_mmc_gpdesc_dbi {
	optcl_mmc_gpdesc_header header;
	uint32_t start_lba;
	uint16_t def_blocks_num;
	bool_t dbif;
	uint8_t error_level;
} optcl_mmc_gpdesc_dbi;

typedef struct tag_mmc_gpdesc_dbicz {
	optcl_mmc_gpdesc_header header;
	uint32_t start_lba;
} optcl_mmc_gpdesc_dbicz;

typedef struct tag_mmc_response_get_performance {
	optcl_mmc_response header;
	uint8_t type;

	union tag_gp_header {
		struct tag_perf_header {
			uint32_t perf_data_len;
			bool_t write;
			bool_t except;
		} perf_header;
		struct tag_dbi_header {
			uint32_t dbi_data_len;
		} dbi_header;
		struct tag_dbicz_header {
			uint32_t dbicz_data_len;
		} dbicz_header;
	} gp_header;

	optcl_list *descriptors;
} optcl_mmc_response_get_performance;


/*
 * INQUIRY command structures
 */

typedef struct tag_mmc_inquiry {
	uint8_t evpd;
	uint8_t page_code;
} optcl_mmc_inquiry;

typedef struct tag_mmc_response_inquiry {
	optcl_mmc_response header;
	uint8_t qualifier;	/* Peripheral qualifier */
	uint8_t device_type;
	bool_t rmb;
	uint8_t version;
	bool_t normaca;
	bool_t hisup;
	uint8_t rdf;		/* Response data format */
	uint8_t additional_len;
	bool_t sccs;
	bool_t acc;
	bool_t tpgs;
	bool_t _3pc;
	bool_t protect;
	bool_t bque;
	bool_t encserv;
	bool_t vs1;
	bool_t vs2;
	bool_t multip;
	bool_t mchngr;
	bool_t addr16;
	bool_t wbus16;
	bool_t sync;
	bool_t linked;
	bool_t cmdque;
	uint8_t vendor[9];
	uint8_t product[17];
	uint32_t revision_level;
	uint8_t vendor_string[21];
	uint8_t clocking;
	bool_t qas;
	bool_t ius;
	uint16_t ver_desc1;
	uint16_t ver_desc2;
	uint16_t ver_desc3;
	uint16_t ver_desc4;
	uint16_t ver_desc5;
	uint16_t ver_desc6;
	uint16_t ver_desc7;
	uint16_t ver_desc8;
} optcl_mmc_response_inquiry;


/*
 * LOAD/UNLOAD MEDIUM command structures
 */

typedef struct tag_mmc_load_unload_medium {
	bool_t immed;
	bool_t load_unload;
	bool_t start;
	uint8_t slot;
} optcl_mmc_load_unload_medium;


/*
 * MECHANISM STATUS command structures
 */

typedef struct tag_mmc_response_mechanism_status {
	optcl_mmc_response header;
	bool_t fault;
	uint8_t changer_state;
	uint8_t current_slot;
	uint8_t mechanism_state;
	bool_t door_open;
	uint32_t current_lba;
	uint8_t available_slots;
	uint16_t slot_table_len;

	struct tag_slot_entry {
		bool_t disk_present;
		bool_t change;
		bool_t cwp_v;
		bool_t cwp;
	} slot_entries[256];
} optcl_mmc_response_mechanism_status;


/*
 * MODE SENSE command structures
 */

typedef struct tag_mmc_mode_sense {
	bool_t dbd;
	uint8_t pc;
	uint8_t page_code;
} optcl_mmc_mode_sense;

typedef struct tag_mmc_msdesc_header {
	uint8_t page_code;
} optcl_mmc_msdesc_header;

typedef struct tag_mmc_msdesc_vendor {
	optcl_mmc_msdesc_header header;
	bool_t ps;
	uint8_t page_len;
	uint8_t vendor_data[254];
} optcl_mmc_msdesc_vendor;

typedef struct tag_mmc_msdesc_rwrecovery {
	optcl_mmc_msdesc_header header;
	bool_t ps;
	bool_t awre;
	bool_t arre;
	bool_t tb;
	bool_t rc;
	bool_t per;
	bool_t dte;
	bool_t dcr;
	uint8_t emcdr;
	uint8_t read_retry_count;
	uint8_t write_retry_count;
	uint32_t window_size;
} optcl_mmc_msdesc_rwrecovery;

typedef struct tag_mmc_msdesc_mrw {
	optcl_mmc_msdesc_header header;
	bool_t ps;
	bool_t lba_space;
} optcl_mmc_msdesc_mrw;

typedef struct tag_mmc_msdesc_writeparams {
	optcl_mmc_msdesc_header header;
	bool_t ps;
	bool_t bufe;
	bool_t ls_v;
	bool_t test_write;
	uint8_t write_type;
	uint8_t multi_session;
	bool_t fp;
	bool_t copy;
	uint8_t track_mode;
	uint8_t dbt;
	uint8_t link_size;
	uint8_t hac;
	uint8_t session_fmt;
	uint32_t packet_size;
	uint16_t audio_pause_len;
	uint8_t mcn[16];
	uint8_t isrc[16];
	uint8_t subheader_0;
	uint8_t subheader_1;
	uint8_t subheader_2;
	uint8_t subheader_3;
	uint8_t vendor_specific[4];
} optcl_mmc_msdesc_writeparams;

typedef struct tag_mmc_msdesc_caching {
	optcl_mmc_msdesc_header header;
	bool_t ps;
	bool_t wce;
	bool_t rcd;
} optcl_mmc_msdesc_caching;

typedef struct tag_mmc_msdesc_power {
	optcl_mmc_msdesc_header header;
	bool_t ps;
	bool_t spf;
	bool_t idle;
	bool_t standby;
	uint32_t idle_timer;
	uint32_t standby_timer;
} optcl_mmc_msdesc_power;

typedef struct tag_mmc_msdesc_infoexceptions {
	optcl_mmc_msdesc_header header;
	bool_t ps;
	bool_t spf;
	bool_t perf;
	bool_t ebf;
	bool_t ewasc;
	bool_t dexcpt;
	bool_t test;
	bool_t logerr;
	uint8_t mrie;
	uint32_t interval_timer;
	uint32_t report_count;
} optcl_mmc_msdesc_infoexceptions;

typedef struct tag_mmc_msdesc_timeout_protect {
	optcl_mmc_msdesc_header header;
	bool_t ps;
	bool_t g3enable;
	bool_t tmoe;
	bool_t disp;
	bool_t swpp;
	uint16_t group1_mintimeout;
	uint16_t group2_mintimeout;
	uint16_t group3_mintimeout;
} optcl_mmc_msdesc_timeout_protect;

typedef struct tag_mmc_response_mode_sense {
	optcl_mmc_response header;
	optcl_list *descriptors;
} optcl_mmc_response_mode_sense;

/*
 * MODE SELECT command structures
 */

typedef struct tag_mmc_mode_select {
	bool_t pf;
	bool_t sp;
	optcl_list *descriptors;
} optcl_mmc_mode_select;


/*
 * PREVENT ALLOW MEDIA REMOVAL command structures
 */

typedef struct tag_mmc_prevent_allow_removal {
	bool_t persistent;
	bool_t prevent;
} optcl_mmc_prevent_allow_removal;


/*
 * READ command structures
 */

typedef struct tag_mmc_read_10 {
	bool_t fua;
	uint32_t start_lba;
	uint16_t transfer_length;
} optcl_mmc_read_10;

typedef struct tag_mmc_read_12 {
	bool_t fua;
	uint32_t start_lba;
	uint32_t transfer_length;
	bool_t streaming;
} optcl_mmc_read_12;

typedef struct tag_mmc_response_read {
	optcl_mmc_response header;
	ptr_t data;
} optcl_mmc_response_read;


/*
 * READ BUFFER command structures
 */

typedef struct tag_mmc_read_buffer {
	uint8_t mode;
	uint8_t buffer_id;
	uint32_t buffer_offset;
	uint32_t allocation_len;
} optcl_mmc_read_buffer;

typedef struct tag_mmc_response_read_buffer {
	optcl_mmc_response header;
	uint8_t mode;

	union tag_response {
		struct tag_combined {
			uint32_t buffer_capacity;
			ptr_t buffer;
		} combined;

		struct tag_data {
			ptr_t buffer;
		} data;

		struct tag_descriptor {
			uint8_t offset_boundary;
			uint32_t buffer_capacity;
		} descriptor;

		struct tag_echo {
			ptr_t buffer;
		} echo;

		struct tag_echo_desc {
			uint32_t buffer_capacity;
		} echo_desc;

		struct tag_expander {
			ptr_t buffer;
		} expander;

		struct tag_vendor {
			ptr_t buffer;
		} vendor;
	} response;
} optcl_mmc_response_read_buffer;


/*
 * READ BUFFER CAPACITY command structures
 */

typedef struct tag_mmc_read_buffer_capacity {
	bool_t block;
} optcl_mmc_read_buffer_capacity;

typedef struct tag_mmc_resopnse_read_buffer_capacity {
	optcl_mmc_response header;

	union tag_desc {
		struct tag_block {
			bool_t block;
			uint16_t data_length;
			uint32_t available_buffer_len;
		} block;

		struct tag_bytes {
			uint16_t data_length;
			uint32_t buffer_len;
			uint32_t buffer_blank_len;
		} bytes;
	} desc;
} optcl_mmc_response_read_buffer_capacity;


/*
 * READ CAPACITY command structures
 */

typedef struct tag_mmc_response_read_capacity {
	optcl_mmc_response header;
	uint32_t lba;
	uint32_t block_len;
} optcl_mmc_response_read_capacity;


/*
 * READ CD command structures
 */

typedef struct tag_mmc_read_cd {
	uint8_t est;
	bool_t dap;
	uint32_t starting_lba;
	uint32_t transfer_len;
	bool_t sync;
	uint8_t header_codes;
	bool_t user_data;
	bool_t edc_ecc;
	uint8_t c2_error_info;
	uint8_t subchannel_sel;
} optcl_mmc_read_cd;


/*
 * REPAIR TRACK command structures
 */

typedef struct tag_mmc_repair_track {
	bool_t immed;
	uint16_t ltn;
} optcl_mmc_repair_track;


/*
 * REQUEST SENSE command structures
 */

typedef struct tag_mmc_request_sense {
	bool_t desc;
} optcl_mmc_request_sense;

typedef struct tag_mmc_response_request_sense {
	optcl_mmc_response header;
	uint8_t sk;
	uint8_t asc;
	uint8_t ascq;
} optcl_mmc_response_request_sense;


/*
 * SEEK command structures
 */

typedef struct tag_mmc_seek {
	uint32_t lba;
} optcl_mmc_seek;

/*
 * SET CD SPEED command structures
 */

typedef struct tag_mmc_set_cd_cpeed {
	uint8_t rotctrl;
	uint16_t drive_read_speed;
	uint16_t drive_write_speed;
} optcl_mmc_set_cd_speed;

/*
 * SET READ AHEAD command structures
 */

typedef struct tag_mmc_set_read_ahead {
	uint32_t trigger_lba;
	uint32_t read_ahead_lba;
} optcl_mmc_set_read_ahead;

/*
 * VERIFY command structures
 */

typedef struct tag_mmc_verify {
	uint32_t lba;
	bool_t g3tout;
	uint16_t block_num;
} optcl_mmc_verify;


/*
 * Command functions
 */

extern RESULT optcl_command_blank(const optcl_device *device,
				  const optcl_mmc_blank *command);

extern RESULT optcl_command_close_track_session(const optcl_device *device,
						const optcl_mmc_close_track_session *command);

extern RESULT optcl_command_destroy_response(optcl_mmc_response *response);

extern RESULT optcl_command_format_unit(const optcl_device *device,
					const optcl_mmc_format_unit *command);

extern RESULT optcl_command_get_configuration(const optcl_device *device,
					      const optcl_mmc_get_configuration *command,
					      optcl_mmc_response_get_configuration **response);

extern RESULT optcl_command_get_event_status(const optcl_device *device,
					     const optcl_mmc_get_event_status *command,
					     optcl_mmc_response_get_event_status **response);

extern RESULT optcl_command_get_performance(const optcl_device *device,
					    const optcl_mmc_get_performance *command,
					    optcl_mmc_response_get_performance **response);

extern RESULT optcl_command_inquiry(const optcl_device *device, 
				    const optcl_mmc_inquiry *command, 
				    optcl_mmc_response_inquiry **response);

extern RESULT optcl_command_load_unload_medium(const optcl_device *device,
					       const optcl_mmc_load_unload_medium *command);

extern RESULT optcl_command_mechanism_status(const optcl_device *device,
					     optcl_mmc_response_mechanism_status **response);

extern RESULT optcl_command_mode_sense_10(const optcl_device *device,
					  const optcl_mmc_mode_sense *command,
					  optcl_mmc_response_mode_sense **response);

extern RESULT optcl_command_mode_select_10(const optcl_device *device,
					   const optcl_mmc_mode_select *command);

extern RESULT optcl_command_prevent_allow_removal(const optcl_device *device,
						  const optcl_mmc_prevent_allow_removal *command);

extern RESULT optcl_command_read_10(const optcl_device *device,
				    const optcl_mmc_read_10 *command,
				    optcl_mmc_response_read **response);

extern RESULT optcl_command_read_12(const optcl_device *device,
				    const optcl_mmc_read_12 *command,
				    optcl_mmc_response_read **response);

extern RESULT optcl_command_read_buffer(const optcl_device *device,
					const optcl_mmc_read_buffer *command,
					optcl_mmc_response_read_buffer **response);

extern RESULT optcl_command_read_buffer_capacity(const optcl_device *device,
						 const optcl_mmc_read_buffer_capacity *command,
						 optcl_mmc_response_read_buffer_capacity **response);

extern RESULT optcl_command_read_capacity(const optcl_device *device,
					  optcl_mmc_response_read_capacity **response);

extern RESULT optcl_command_repair_track(const optcl_device *device,
					 const optcl_mmc_repair_track *command);

extern RESULT optcl_command_request_sense(const optcl_device *device,
					  const optcl_mmc_request_sense *command,
					  optcl_mmc_response_request_sense **response);

extern RESULT optcl_command_seek(const optcl_device *device,
				 const optcl_mmc_seek *command);

extern RESULT optcl_command_set_cd_speed(const optcl_device *device,
					 const optcl_mmc_set_cd_speed *command);

extern RESULT optcl_command_set_read_ahead(const optcl_device *device,
					   const optcl_mmc_set_read_ahead *command);

extern RESULT optcl_command_test_unit_ready(const optcl_device *device);

extern RESULT optcl_command_verify(const optcl_device *device,
				   const optcl_mmc_verify *command);

#endif /* _COMMAND_H */
