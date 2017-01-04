/*
 * Unitex
 *
 * Copyright (C) 2001-2017 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef SortTxtH
#define SortTxtH

#include "UnitexGetOpt.h"
#include "FileEncoding.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

extern const char* optstring_SortTxt;
extern const struct option_TS lopts_SortTxt[];
extern const char* usage_SortTxt;

int main_SortTxt(int argc,char* const argv[]);
int pseudo_main_SortTxt(const VersatileEncodingConfig*,
                        int duplicates,int reverse,char* sort_alphabet,char* line_info,int thai,char*,int);

} // namespace unitex

#endif

