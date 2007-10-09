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

#include "errors.h"
#include "feature.h"
#include "hashtable.h"
#include "profile.h"

#include <assert.h>

/* Device profile */
struct tag_profile {
	optcl_hashtable *features;
};

int optcl_profile_check_feature(const optcl_profile *profile, 
				int feature_code,
				int *present)
{
	int error;
	optcl_feature_descriptor *descriptor = 0;

	assert(profile);
	assert(present);

	if (!profile || !present)
		return E_INVALIDARG;

	assert(profile->features);

	if (!profile->features) {
		*present = 0;
		return SUCCESS;
	}

	error = optcl_hashtable_lookup(
		profile->features, 
		(ptr_t)&feature_code, 
		(pptr_t)&descriptor);

	if (FAILED(error))
		return error;

	*present = (descriptor) ? 1 : 0;

	return SUCCESS;
}

int optcl_profile_clear(optcl_profile *profile)
{
	int error;

	assert(profile);

	if (!profile)
		return E_INVALIDARG;

	if (profile->features) {
		error = optcl_hashtable_destroy(profile->features, 1);

		if (FAILED(error))
			return error;
	}

	profile->features = 0;

	return SUCCESS;
}

int optcl_profile_copy(optcl_profile *dest, const optcl_profile *src)
{
	//int error;

	//assert(src);
	//assert(dest);

	//if (!src || !dest)
	//	return E_INVALIDARG;

	return SUCCESS;

	(void)dest;
	(void)src;
}
