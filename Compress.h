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

#ifndef CompressH
#define CompressH

#include "UnitexGetOpt.h"
#include "Unicode.h"

namespace unitex {

extern const char* optstring_Compress;
extern const struct option_TS lopts_Compress[];
extern const char* usage_Compress;

int main_Compress(int argc,char* const argv[]);
int pseudo_main_Compress(const VersatileEncodingConfig*,
                         int flip,int semitic,char* dic,int new_style_bin);

} // namespace unitex

#endif

