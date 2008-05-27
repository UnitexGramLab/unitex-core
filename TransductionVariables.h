 /*
  * Unitex
  *
  * Copyright (C) 2001-2008 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef TransductionVariablesH
#define TransductionVariablesH

#include "Unicode.h"
#include "Fst2.h"
#include "String_hash.h"
#include "List_ustring.h"


/* Value to use to indicate that a variable bound is undefined */
#define UNDEF_VAR_BOUND -666666


/**
 * This structure is used to associates ranges to variable names.
 */
typedef struct {
   /* For a given variable A, this array gives an index a to be used in
    * the 'variables' array */
   struct string_hash* variable_index;
   /* variables[a] gives the range associated to the variable #a */
   struct transduction_variable* variables;
} Variables;


/**
 * This structure defines the range of a variable in the text tokens.
 */
struct transduction_variable {
   /* Position of the first token of the sequence */
   int start;
   /* Position after the last token of the sequence, so that the sequence is in the
    * range [start;end[ */
   int end;
};



Variables* new_Variables(struct list_ustring*);
void free_Variables(Variables*);
struct transduction_variable* get_transduction_variable(Variables*,unichar*);
void set_variable_start(Variables*,int,int);
void set_variable_end(Variables*,int,int);
int get_variable_start(Variables*,int);
int get_variable_end(Variables*,int);

int* create_variable_backup(Variables*);
void free_variable_backup(int*);
void install_variable_backup(Variables*,int*);

#endif
