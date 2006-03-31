 /*
  * Unitex
  *
  * Copyright (C) 2001-2006 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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

//---------------------------------------------------------------------------
#ifndef TransductionVariablesH
#define TransductionVariablesH
//---------------------------------------------------------------------------

#include "unicode.h"
#include "Fst2.h"
#include "String_hash.h"


#define N_MAX_TRANSDUCTION_VARIABLES 1000

struct transduction_variable {
  int start;
  int end;
};


extern struct transduction_variable* tab_transduction_variable[N_MAX_TRANSDUCTION_VARIABLES];
extern struct string_hash* transduction_variable_index;

void init_transduction_variable_index(struct variable_list*);
struct transduction_variable* get_transduction_variable(unichar*);
void free_transduction_variable_index();
void set_variable_start(int,int);
void set_variable_end(int,int);
int get_variable_start(int);
int get_variable_end(int);


int* create_variable_backup();
void free_variable_backup(int*);
void install_variable_backup(int*);

#endif
