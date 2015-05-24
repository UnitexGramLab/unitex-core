/*
 * Unitex
 *
 * Copyright (C) 2001-2015 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
   /* variables[a] gives the information associated to the variable #a */
   Ustring** variables;
   /* 'pending' is the list of variables for which we are currently between $|aa( and $|aa)
    * 'is_pending' is an array indicating by 1 or 0 if a variable is pending or not */
   OutputVarList* pending;
   char* is_pending;
   size_t is_pending_array_size_int_size_rounded;
} OutputVariables;


OutputVariables* new_OutputVariables(struct list_ustring*,int* p_nbvar,vector_ptr* injected);
void free_OutputVariables(OutputVariables*);
Ustring* get_output_variable(OutputVariables*,const unichar*);
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

unsigned int add_raw_string_to_output_variables(OutputVariables*,unichar*);
void remove_chars_from_output_variables(OutputVariables*,unsigned int);

} // namespace unitex

#endif
