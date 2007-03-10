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
 * MMC opcodes
 */

#define MMC_OPCODE_BLANK		0x00A1
#define MMC_OPCODE_CLOSE_TRACK_SESSION	0x005B
#define MMC_OPCODE_INQUIRY		0x0012
#define MMC_OPCODE_GET_CONFIG		0x0046
#define MMC_OPCODE_GET_EVENT_STATUS	0x004A
#define MMC_OPCODE_REQUEST_SENSE	0x0003


/*
 * BLANK command command field flags
 */

#define MMC_BLANK_BLANK_DISK		0x00
#define MMC_BLANK_MINIMAL_BLANK_DISK	0x01
#define MMC_BLANK_BLANK_TRACK		0x02
#define MMC_BLANK_UNRESERVE_TRACK	0x03
#define MMC_BLANK_TRACK_TRAIL		0x04
#define MMC_BLANK_UNCLOSE_LAST_SESSION	0x05
#define MMC_BLANK_LAST_SESSION		0x06


/*
 * CLOSE TRACK SESSION command field flags
 */

#define CD_R_RW_CLOSE_LOGICAL_TRACK			0x01
#define CD_R_RW_CLOSE_SESSION_FINALIZE			0x02
#define DVD_R_RW_CLOSE_LOGICAL_TRACK			0x01
#define DVD_R_RW_CLOSE_SESSION_FINALIZE			0x02
#define DVD_R_RW_FINALIZE_DVD_RW_DISC			0x03
#define DVD_R_DL_CLOSE_LOGICAL_TRACK			0x01
#define DVD_R_DL_CLOSE_SESSION_FINALIZE			0x02
#define DVD_PLUS_R_CLOSE_LOGICAL_TRACK			0x01
#define DVD_PLUS_R_CLOSE_SESSION			0x02
#define DVD_PLUS_R_FINALIZE_30MM_RADIUS			0x05
#define DVD_PLUS_R_FINALIZE_DISC			0x06
#define DVD_PLUS_R_DL_CLOSE_LOGICAL_TRACK		0x01
#define DVD_PLUS_R_DL_CLOSE_SESSION			0x02
#define DVD_PLUS_R_DL_REC_EXT_LEADOUT			0x04
#define DVD_PLUS_R_DL_FINALIZE_30MM_RADIUS		0x05
#define DVD_PLUS_R_DL_FINALIZE_DISC			0x06
#define DVD_PLUS_RW_QUICKSTOP_BG_FORMAT			0x00
#define DVD_PLUS_RW_COMPSTOP_BG_FORMAT_30MM_RADIUS	0x02
#define DVD_PLUS_RW_COMPSTOP_BG_FORMAT			0x03
#define DVD_PLUS_RW_DL_QUICKSTOP_BG_FORMAT		0x00
#define DVD_PLUS_RW_DL_COMPSTOP_BG_FORMAT_30MM_RADIUS	0x02
#define DVD_PLUS_RW_DL_COMPSTOP_BG_FORMAT		0x03
#define HD_DVD_R_CLOSE_LOGICAL_TRACK			0x01
#define HD_DVD_R_CLOSE_SESSION				0x02
#define HD_DVD_R_FINALIZE_DISC				0x06
#define BD_R_CLOSE_LOGICAL_TRACK			0x01
#define BD_R_CLOSE_SESSION				0x02
#define BD_R_FINALIZE_DISC				0x06
#define MRW_STOP_BG_FORMAT				0x02


/*
 * GET CONFIGURATION command field flags
 */

#define MMC_GET_CONFIG_RT_ALL		0x00
#define MMC_GET_CONFIG_RT_CURRENT	0x01
#define MMC_GET_CONFIG_RT_FROM		0x02


/*
 * GET EVENT STATUS NOTIFICATION command field flags
 */

#define MMC_GET_EVENT_STATUS_OPCHANGE		0x01
#define MMC_GET_EVENT_STATUS_POWERMGMT		0x02
#define MMC_GET_EVENT_STATUS_EXTREQUEST		0x04
#define MMC_GET_EVENT_STATUS_MEDIA		0x08
#define MMC_GET_EVENT_STATUS_MULTIHOST		0x10
#define MMC_GET_EVENT_STATUS_DEVICEBUSY		0x20

#define EVENT_OC_EC_NOCHG		0x00
#define EVENT_OC_EC_CHANGED		0x02
#define EVENT_OC_OC_NOCHG		0x00
#define EVENT_OC_OC_FEATCHANGE		0x02
#define EVENT_PM_EC_NOCHG		0x00
#define EVENT_PM_EC_PWRCHGOK		0x01
#define EVENT_PM_EC_PWRCHGFAIL		0x02
#define EVENT_PM_PS_ACTIVE		0x01
#define EVENT_PM_PS_IDLE		0x02
#define EVENT_PM_PS_STANDBY		0x03
#define EVENT_PM_PS_SLEEP		0x04
#define EVENT_ER_EC_NOCHG		0x00
#define EVENT_ER_EC_DRIVEKEYDOWN	0x01
#define EVENT_ER_EC_DRIVEKEYUP		0x02
#define EVENT_ER_EC_EXTREQNOT		0x03
#define EVENT_ER_ERC_READY		0x00
#define EVENT_ER_ERC_OTHERPREVENT	0x01

#define EVENT_ER_ER_NOREQUEST		0x0000
#define EVENT_ER_ER_OVERRUN		0x0001
#define EVENT_ER_ER_PLAY		0x0101
#define EVENT_ER_ER_REWIND		0x0102
#define EVENT_ER_ER_FASTFORWARD		0x0103
#define EVENT_ER_ER_PAUSE		0x0104
#define EVENT_ER_ER_STOP		0x0106

/* 0x0200 - 0x02FF ASCII button */
/* 0xF000 - 0xFFFF Vendor unique */

#define EVENT_MEDIA_EC_NOCHG		0x00
#define EVENT_MEDIA_EC_EJECTREQUEST	0x01
#define EVENT_MEDIA_EC_NEWMEDIA		0x02
#define EVENT_MEDIA_EC_MEDIAREMOVAL	0x03
#define EVENT_MEDIA_EC_MEDIACHANGED	0x04
#define EVENT_MEDIA_EC_BGFORMATCOMPLETE	0x05
#define EVENT_MEDIA_EC_BGFORMATRESTART	0x06

#define EVENT_MH_EC_NOCHG		0x00
#define EVENT_MH_EC_CTRLREQUEST		0x01
#define EVENT_MH_EC_CTRLGRANT		0x02
#define EVENT_MH_EC_CTRLRELEASE		0x03
#define EVENT_MH_MHS_READY		0x00
#define EVENT_MH_MHS_OTHERPREVENT	0x01
#define EVENT_MH_MHP_NOREQUEST		0x00
#define EVENT_MH_MHP_LOW		0x01
#define EVENT_MH_MHP_MEDIUM		0x02
#define EVENT_MH_MHP_HIGH		0x03

#define EVENT_DB_EC_NOCHG		0x00
#define EVENT_DB_EC_CHANGE		0x01
#define EVENT_DB_DBS_NOTBUSY		0x00
#define EVENT_DB_DBS_BUSY		0x01


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
 * GET CONFIGURATION command structures
 */

typedef struct tag_mmc_get_configuration {
	uint8_t rt;
	uint16_t start_feature;
} optcl_mmc_get_configuration;

typedef struct tag_mmc_response_get_configuration {
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
	optcl_mmc_ges_header header;
	uint8_t event_class;
	optcl_list *descriptors;
} optcl_mmc_response_get_event_status;

typedef optcl_mmc_ges_header optcl_mmc_ges_descriptor;

/*
 * INQUIRY command structures
 */

typedef struct tag_mmc_inquiry {
	uint8_t evpd;
	uint8_t page_code;
} optcl_mmc_inquiry;

typedef struct tag_mmc_response_inquiry {
	uint8_t qualifier;	/* Peripheral qualifier */
	uint8_t device_type;
	uint8_t rmb;
	uint8_t version;
	uint8_t normaca;
	uint8_t hisup;
	uint8_t rdf;		/* Response data format */
	uint8_t additional_len;
	uint8_t sccs;
	uint8_t acc;
	uint8_t tpgs;
	uint8_t _3pc;
	uint8_t protect;
	uint8_t bque;
	uint8_t encserv;
	uint8_t vs;
	uint8_t multip;
	uint8_t mchngr;
	uint8_t addr16;
	uint8_t wbus16;
	uint8_t sync;
	uint8_t linked;
	uint8_t cmdque;
	uint8_t vendor[9];
	uint8_t product[17];
	uint32_t revision_level;
	uint8_t vendor_string[21];
	uint8_t clocking;
	uint8_t qas;
	uint8_t ius;
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
 * REQUEST SENSE command structures
 */

typedef struct tag_mmc_request_sense {
	bool_t desc;
} optcl_mmc_request_sense;

typedef struct tag_mmc_response_request_sense {
	uint8_t sk;
	uint8_t asc;
	uint8_t ascq;
} optcl_mmc_response_request_sense;


/*
 * Command functions
 */

extern RESULT optcl_command_blank(const optcl_device *device,
				  const optcl_mmc_blank *command);

extern RESULT optcl_command_close_track_session(const optcl_device *device,
						const optcl_mmc_close_track_session *command);

extern RESULT optcl_command_destroy_response(uint16_t opcode, void *response);

extern RESULT optcl_command_get_configuration(const optcl_device *device,
					      const optcl_mmc_get_configuration *command,
					      optcl_mmc_response_get_configuration **response);

extern RESULT optcl_command_get_event_status(const optcl_device *device,
					     const optcl_mmc_get_event_status *command,
					     optcl_mmc_response_get_event_status **response);

extern RESULT optcl_command_inquiry(const optcl_device *device, 
				    const optcl_mmc_inquiry *command, 
				    optcl_mmc_response_inquiry **response);

extern RESULT optcl_command_request_sense(const optcl_device *device,
					  const optcl_mmc_request_sense *command,
					  optcl_mmc_response_request_sense **response);

#endif /* _COMMAND_H */
