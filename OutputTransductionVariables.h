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

#ifndef OutputTransductionVariablesH
#define OutputTransductionVariablesH


#include "AbstractCallbackFuncModifier.h"
#include "Unicode.h"
#include "String_hash.h"
#include "List_ustring.h"
#include "Ustring.h"
#include "Vector.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * This structure is used to store the list of current pending output variables while
 * exploring a grammar
 */
typedef struct output_var_list {
    Ustring* var;
    struct output_var_list* next;
} OutputVarList;


/**
 * This structure is used to associates string values to variable names.
 */
typedef struct {
   /* For a given variable A, this array gives an index a to be used in
    * the 'variables' array */
   struct string_hash* variable_index;
   /* 'pending' is the list of variables for which we are currently between $|aa( and $|aa)
    * 'is_pending' is an array indicating by 1 or 0 if a variable is pending or not */

   OutputVarList* pending;
   OutputVarList* recycle_allocation;
   char* is_pending;

   // somes size and pos precalculated for backup
   size_t is_pending_array_size_intptr_size_rounded;
   size_t string_index_offset;
   size_t unichars_offset;

   size_t nb_var;
   /* variables[a] gives the information associated to the variable #a */
   Ustring variables_[1];
} OutputVariables;


/**
 * swap the string of one variable with another string
 */
void swap_output_variable_content(OutputVariables*v, int index, Ustring* swap_string);

OutputVariables* new_OutputVariables(struct list_ustring*,int* p_nbvar,vector_ptr* injected);
void free_OutputVariables(OutputVariables*);
const Ustring* get_output_variable(OutputVariables*,const unichar*);
Ustring* get_mutable_output_variable(OutputVariables* v, const unichar* name);

typedef struct
{
    void *_dummy;
} OutputVariablesBackup;
int same_output_variables(const OutputVariablesBackup* output_variable_backup,OutputVariables* v);

/**
 * Returns 1 if there are some pending variables; 0 otherwise.
 */
static inline int capture_mode(OutputVariables* v) {
return v->pending!=NULL;
}

OutputVariablesBackup* create_output_variable_backup(OutputVariables* RESTRICT,Abstract_allocator);
void free_output_variable_backup(OutputVariablesBackup*,Abstract_allocator);
void install_output_variable_backup(OutputVariables* RESTRICT,const OutputVariablesBackup* RESTRICT);

void set_output_variable_pending(OutputVariables* var,int index);
void unset_output_variable_pending(OutputVariables* var,int index);
void set_output_variable_pending(OutputVariables* var,const unichar* var_name);
void unset_output_variable_pending(OutputVariables* var,const unichar* var_name);

unsigned int add_raw_string_to_output_variables(OutputVariables*,unichar*,unsigned int);
void remove_chars_from_output_variables(OutputVariables*,unsigned int);

} // namespace unitex

#endif
