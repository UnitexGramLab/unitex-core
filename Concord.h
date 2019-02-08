/*
 * Unitex
 *
 * Copyright (C) 2001-2019 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef ConcordH
#define ConcordH

#include "UnitexGetOpt.h"
#include "FileEncoding.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

extern const char* optstring_Concord;
extern const struct option_TS lopts_Concord[];
extern const char* usage_Concord;

int main_Concord(int argc,char* const argv[]);
int pseudo_main_Concord(const VersatileEncodingConfig*,
                        const char* index_file,const char* font,int fontsize,
                        int left_context,int right_context,const char* sort_order,
                        const char* output,const char* directory,const char* alphabet,
                        int thai,int only_ambiguous,int only_matches);

} // namespace unitex

#endif

