/*
 * Unitex
 *
 * Copyright (C) 2001-2012 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
 *
 */

#ifndef XmlH
#define XmlH

#include "Unicode.h"
#include "Offsets.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/* Do we want to skip the content of a tag ? */
#define UNXMLIZE_IGNORE 0
/* Or do we prefer to replace it by a single space ? */
#define UNXMLIZE_REPLACE_BY_SPACE 1
/* Or do we prefer to let it unmodified ? */
#define UNXMLIZE_DO_NOTHING 2

typedef struct {
	char comments;
	char scripts;
	char normal_tags;
} UnxmlizeOpts;

int unxmlize(U_FILE* input,U_FILE* output,vector_offset* offsets,UnxmlizeOpts* options,
		unichar* bastien[],U_FILE* f_bastien, int tolerate_markup_malformation);

} // namespace unitex

#endif

