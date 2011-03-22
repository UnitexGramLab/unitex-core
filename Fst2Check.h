/*
 * Unitex
 *
 * Copyright (C) 2001-2011 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

/*
 * File created and contributed by Gilles Vollant (Ergonotics SAS) 
 * as part of an UNITEX optimization and reliability effort
 *
 * additional information: http://www.ergonotics.com/unitex-contribution/
 * contact : unitex-contribution@ergonotics.com
 *
 */




#ifndef Fst2ChkH
#define Fst2ChkH

#include "UnitexGetOpt.h"
#include "FileEncoding.h"

extern const char* optstring_Fst2Check;
extern const struct option_TS lopts_Fst2Check[];
extern const char* usage_Fst2Check;

int main_Fst2Check(int argc,char* const argv[]);
int pseudo_main_Fst2Check(Encoding encoding_output,int bom_output,int mask_encoding_compatibility_input,
                          const char* fst2name,const char* output_name,int append,int display_statistics,
                          int yes_or_no,int no_empty_graph_warning,int tfst_check);
#endif

