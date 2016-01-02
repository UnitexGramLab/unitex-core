/*
 * Unitex
 *
 * Copyright (C) 2001-2016 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef PrlgH
#define PrlgH

#include "Vector.h"
#include "Unicode.h"
#include "FileEncoding.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * This structure is used to represent an anchor in a .xml PRLG
 * corpus.
 */
typedef struct {
	int offset;
	unichar* data;
} PRLG_DATA;


typedef struct {
	/* The data */
	vector_ptr* data;
	/* The width of the largest data, that will be used for alignment purpose */
	int max_width;
} PRLG;


PRLG* load_PRLG_data(VersatileEncodingConfig* vec,char* filename);
void free_PRLG(PRLG*);
const unichar* get_closest_PRLG_tag(PRLG* p,int offset);

} // namespace unitex

#endif

