/*
    media.h - Optical media
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

#ifndef _MEDIA_H
#define _MEDIA_H

#include "list.h"
#include "profile.h"


/* Media info */
typedef struct tag_media_info optcl_media_info;


/* Creates new media structure */
int optcl_media_info_create(optcl_media_info **media);

/* Clears media structure */
int optcl_media_info_clear(optcl_media_info *media);

/* Gets deep copy of a media structure */
int optcl_media_info_copy(optcl_media_info *dest, 
			  const optcl_media_info *src);

/* Destroys media structure */
int optcl_media_info_destroy(optcl_media_info *media);

/* Gets media type */
int optcl_media_info_get_type(const optcl_media_info *media, int *type);

/* Gets media profiles */
int optcl_media_info_get_profiles(const optcl_media_info *media, 
				  optcl_list **profiles);

/* Sets media type */
int optcl_media_info_set_type(optcl_media_info *media, int type);

/* Sets media profiles */
int optcl_media_info_set_profiles(optcl_media_info *media, 
				  const optcl_list *profiles);

#endif /* _MEDIA_H */
