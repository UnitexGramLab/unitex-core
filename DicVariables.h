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

#ifndef DicVariablesH
#define DicVariablesH

#include "Unicode.h"
#include "DELA.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * This structure represents a list of DELAF entry variables.
 */
struct dic_variable {
   /* The name of the variable */
   unichar* name;
   /* the corresponding struct dela_entry */
   struct dela_entry* dic_entry;
   struct dic_variable* next;
};


void clear_dic_variable_list(struct dic_variable* *list);
void set_dic_variable(const unichar* name,struct dela_entry* dic_entry,struct dic_variable* *list,int must_clone);
struct dela_entry* get_dic_variable(const unichar* name,struct dic_variable* list);
struct dic_variable* clone_dic_variable_list(const struct dic_variable* list);
int same_dic_variables(struct dic_variable* backup,struct dic_variable* v);

} // namespace unitex

#endif
