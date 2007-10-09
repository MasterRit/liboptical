/*
    profile.h - Optical device/media profile
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

#ifndef _PROFILE_H
#define _PROFILE_H

#include "feature.h"
#include "hashtable.h"

/*
 * Media profiles
 */

/* 0x0000 Reserved */

#define PROFILE_NONREMOVABLE_DISK		0x0001
#define PROFILE_REMOVABLE_DISK			0x0002
#define PROFILE_MO_ERASABLE			0x0003
#define PROFILE_OPTICAL_WRITE_ONCE		0x0004
#define PROFILE_AS_MO				0x0005

/* 0x0006 - 0x0007 Reserved */

#define PROFILE_CD_ROM				0x0008
#define PROFILE_CD_R				0x0009
#define PROFILE_CD_RW				0x000A

/* 0x000B - 0x000F Reserved */

#define PROFILE_DVD_ROM				0x0010
#define PROFILE_DVD_R_SEQREC			0x0011
#define PROFILE_DVD_RAM				0x0012
#define PROFILE_DVD_RW_RESTRICTED_OVR		0x0013
#define PROFILE_DVD_RW_SEQREC			0x0014
#define PROFILE_DVD_R_DUAL_LAYER_SEQREC		0x0015
#define PROFILE_DVD_R_DUAL_LAYER_JUMPREC	0x0016

/* 0x0017 - 0x0019 Reserved */

#define PROFILE_DVD_PLUS_RW			0x001A
#define PROFILE_DVD_PLUS_R			0x001B

/* 0x001C - 0x0029 Reserved */

#define PROFILE_DVD_PLUS_RW_DUAL_LAYER		0x002A
#define PROFILE_DVD_PLUS_R_DUAL_LAYER		0x002B

/* 0x002C - 0x003F Reserved */

#define PROFILE_BD_ROM				0x0040
#define PROFILE_BD_R_SRM			0x0041
#define PROFILE_BD_R_RRM			0x0042
#define PROFILE_BD_RE				0x0043

/* 0x0044 - 0x004F Reserved */

#define PROFILE_HD_DVD_ROM			0x0050
#define PROFILE_HD_DVD_R			0x0051
#define PROFILE_HD_DVD_RAM			0x0052

/* 0x0053 - 0xFFFE Reserved */

#define PROFILE_NON_STANDARD			0xFFFF


/* Device profile */
struct tag_profile;
typedef struct tag_profile optcl_profile;

/*
 * Profile functions
 */ 

/* Check if feature is present */
int optcl_profile_check_feature(const optcl_profile *profile, 
				int feature_code,
				int *present);

/* Clear profile structure */
int optcl_profile_clear(optcl_profile *profile);

/* Copy one profile structure to another one */
int optcl_profile_copy(optcl_profile *dest, const optcl_profile *src);

/* Create new profile structure */
int optcl_profile_create(optcl_profile **profile);

/* Destroy profile */
int optcl_profile_destroy(optcl_profile *profile);

/* Get profile features */
int optcl_profile_get_features(const optcl_profile *profile, 
			       optcl_hashtable **features);

/* Set device features */
int optcl_profile_set_features(optcl_profile *profile, 
			       optcl_hashtable *features);

#endif /* _PROFILE_H */

