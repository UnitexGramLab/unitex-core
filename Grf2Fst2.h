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

#ifndef Grf2Fst2H
#define Grf2Fst2H

#include "UnitexGetOpt.h"
#include "FileEncoding.h"

extern const char* optstring_Grf2Fst2;
extern const struct option_TS lopts_Grf2Fst2[];
extern const char* usage_Grf2Fst2;

int main_Grf2Fst2(int argc,char* const argv[]);
int pseudo_main_Grf2Fst2(const VersatileEncodingConfig*,
                         const char* name,int yes_or_no,const char* alphabet,
                         int no_empty_graph_warning,int tfst_check,const char* pkgdir);

#endif

