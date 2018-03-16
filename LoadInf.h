/*
 * Unitex
 *
 * Copyright (C) 2001-2018 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef LoadInfH
#define LoadInfH

#include "Unicode.h"
#include "List_ustring.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * This structure is used to store all the INF codes of an .inf file.
 */
struct INF_codes {
    /* Array containing for each line of the .inf file the reversed list of its
     * components. For instance, if the first line contains:
     *
     * .N+NA+z1:fs,.N+Loc:fs
     *
     * codes[0] will contain the following list:
     *
     *  ".N+Loc:fs"   -->   ".N+NA+z1:fs"   -->   NULL
     *
     */
    struct list_ustring** codes;
    /* Number of lines in the .inf file */
    int N;
};



struct INF_codes* load_INF_file(const VersatileEncodingConfig*,const char*,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
void free_INF_codes(struct INF_codes*,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
struct list_ustring* tokenize_compressed_info(const unichar* line,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);

} // namespace unitex

#endif

