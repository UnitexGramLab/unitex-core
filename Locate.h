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

#ifndef LocateH
#define LocateH

#include "UnitexGetOpt.h"
#include "FileEncoding.h"
#include "LocateConstants.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

extern const char* optstring_Locate;
extern const struct option_TS lopts_Locate[];
extern const char* usage_Locate;

int main_Locate(int argc,char* const argv[]);
int launch_locate_as_routine(const VersatileEncodingConfig*,const char*,const char*,const char*,
                             OutputPolicy,MatchPolicy,const char*,int,int,const char*,const char*,int);

} // namespace unitex

#endif

