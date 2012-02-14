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

#ifndef MF_Operators_UtilsH
#define MF_Operators_UtilsH

#include "Unicode.h"

namespace unitex {

#define POUND 0x00A3
#define L1 100
#define L2 300
#define L3 500
#define L4 1000
#define IMMEDIAT 0
#define SHORTEST 1
#define LONGEST 2
#define PROTEGE 4
#define VERBOSE 0



int compare(unichar *, unichar **);
int filtrer(unichar * ,unichar **);
int get_indice_var_op(unichar *);
int flex_op_with_var(unichar (*)[L1],unichar *,unichar *,int *,int *,unsigned int *);
unsigned int get_flag_var(int,unsigned int);

} // namespace unitex

#endif


