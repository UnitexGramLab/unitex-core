/*
 * Unitex
 *
 * Copyright (C) 2001-2021 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "TransductionVariables.h"
#include "Error.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * Allocates and returns a structure representing the variables
 * whose names are in 'list'. The variable ranges are initialized with [-1;-1[
 */
InputVariables* new_Variables(const struct list_ustring* list,int *p_nb_variable) {
if (p_nb_variable!=NULL) {
    *p_nb_variable=0;
}
InputVariables* v=(InputVariables*)malloc(sizeof(InputVariables));
if (v==NULL) {
   fatal_alloc_error("new_Variables");
}
v->variable_index=new_string_hash(DONT_USE_VALUES);
int l=0;
while (list!=NULL) {
   get_value_index(list->string,v->variable_index);
   l++;
   list=list->next;
}
v->variables=(struct transduction_variable*)malloc(l*sizeof(struct transduction_variable));
if (v->variables==NULL) {
   fatal_alloc_error("new_Variables");
}
for (int i=0;i<l;i++) {
   v->variables[i].start_in_tokens=UNDEF_VAR_BOUND;
   v->variables[i].end_in_tokens=UNDEF_VAR_BOUND;
   v->variables[i].start_in_chars=UNDEF_VAR_BOUND;
   v->variables[i].end_in_chars=UNDEF_VAR_BOUND;
}
if (p_nb_variable!=NULL) {
    *p_nb_variable=l;
}
return v;
}


void reset_Variables(InputVariables* v) {
if (v==NULL) return;
int n=v->variable_index->size;
for (int i=0;i<n;i++) {
       v->variables[i].start_in_tokens=UNDEF_VAR_BOUND;
       v->variables[i].end_in_tokens=UNDEF_VAR_BOUND;
       v->variables[i].start_in_chars=UNDEF_VAR_BOUND;
       v->variables[i].end_in_chars=UNDEF_VAR_BOUND;
}
}

/**
 * Frees the memory associated to the given variables.
 */
void free_Variables(InputVariables* v) {
if (v==NULL) return;
free_string_hash(v->variable_index);
free(v->variables);
free(v);
}


/**
 * Returns a pointer on the range of the variable whose name is 'name',
 * or NULL if the variable in not in the given variable set.
 */
struct transduction_variable* get_transduction_variable(InputVariables* v,const unichar* name) {
int n=get_value_index(name,v->variable_index,DONT_INSERT);
if (n==-1) {
   return NULL;
}
return &(v->variables[n]);
}

/**
 * Returns a pointer on the range of the variable whose name is 'name',
 * or NULL if the variable in not in the given variable set.
 * value_index is set to the index value associated to the given key or -1
 * if the variable in not in the given variable set.
 */
struct transduction_variable* get_transduction_variable(
    InputVariables* v, const unichar* name, int* value_index) {
  *value_index = get_value_index(name, v->variable_index, DONT_INSERT);
  if (*value_index == -1) {
    return NULL;
  }
  return &(v->variables[*value_index]);
}


/*
 * the function below are replaced by macro for performance
 * because only Visual Studio 2005 (and more) with /GL and /LTCG and GCC 4.5 (with -flto)
 * can auto inline between different source file
 *
 * more information :
 *
 * http://www.winimage.com/misc/articlegl.htm
 * http://gcc.gnu.org/wiki/LinkTimeOptimization
 */

/**
 * Sets the start value of the variable #n.
 */
/*
void set_variable_start(Variables* v,int n,int value) {
v->variables[n].start_in_tokens=value;
}*/


/**
 * Sets the start value of the variable #n.
 */
/*
void set_variable_start_in_chars(Variables* v,int n,int value) {
v->variables[n].start_in_chars=value;
}*/


/**
 * Sets the end value of the variable #n.
 */
/*
void set_variable_end(Variables* v,int n,int value) {
v->variables[n].end_in_tokens=value;
}*/


/**
 * Sets the end value of the variable #n.
 */
/*
void set_variable_end_in_chars(Variables* v,int n,int value) {
v->variables[n].end_in_chars=value;
}*/


/**
 * Returns the start value of the variable #n.
 */
/*
int get_variable_start(const Variables* v,int n) {
return v->variables[n].start_in_tokens;
}*/


/**
 * Returns the start value of the variable #n.
 */
/*
int get_variable_start_in_chars(const Variables* v,int n) {
return v->variables[n].start_in_chars;
}*/


/**
 * Returns the end value of the variable #n.
 */
/*
int get_variable_end(const Variables* v,int n) {
return v->variables[n].end_in_tokens;
}*/


/**
 * Returns the end value of the variable #n.
 */
/*
int get_variable_end_in_chars(const Variables* v,int n) {
return v->variables[n].end_in_chars;
}*/


#define NB_INT_BY_VARIABLES (4)

size_t get_expected_variable_backup_size_in_byte_for_nb_variable(int nb)
{
    return sizeof(int)*NB_INT_BY_VARIABLES*nb;
}


size_t get_variable_backup_size_in_byte(const InputVariables* v)
{
if (v==NULL || v->variable_index==NULL) return 0;
int l=v->variable_index->size;
return (sizeof(int)*NB_INT_BY_VARIABLES*l);
}

/**
 * Allocates, initializes and returns an integer array that is a copy of
 * the variable ranges.
 */
int* create_variable_backup(const InputVariables* v,Abstract_allocator prv_alloc_recycle) {
if (v==NULL || v->variable_index==NULL) return NULL;
int l=v->variable_index->size;
int* backup=(int*)malloc_cb(sizeof(int)*NB_INT_BY_VARIABLES*l,prv_alloc_recycle);
if (backup==NULL) {
   fatal_alloc_error("create_variable_backup");
}

/* v->variables is an array of struct transduction_variable
   which is a structure of (two before) NB_INT_BY_VARIABLES int */
memcpy((void*)&backup[0],(void*)(&(v->variables[0])),sizeof(int)*NB_INT_BY_VARIABLES*l);

return backup;
}

/**
 * initializes an integer array that is a copy of
 * the variable ranges.
 */
void init_variable_backup(int* backup,const InputVariables* v) {
if (v==NULL || v->variable_index==NULL) return ;
int l=v->variable_index->size;

/* v->variables is an array of struct transduction_variable
   which is a structure of (two before) NB_INT_BY_VARIABLES int */
memcpy((void*)&backup[0],(void*)(&(v->variables[0])),sizeof(int)*NB_INT_BY_VARIABLES*l);
}

/**
 * Frees the given variable backup.
 */
void free_variable_backup(int* backup,Abstract_allocator prv_alloc_recycle) {
if (backup!=NULL) free_cb(backup,prv_alloc_recycle);
}


/**
 * Sets the variable ranges with the values of the given backup.
 */
void install_variable_backup(InputVariables* v,const int* backup) {
if (backup==NULL) {
    fatal_error("NULL error in install_variable_backup\n");
}
int l=v->variable_index->size;

/* v->variables is an array of struct transduction_variable
   which is a structure of (two before) NB_INT_BY_VARIABLES int */
memcpy((void*)(&(v->variables[0])),(const void*)&backup[0],sizeof(int)*NB_INT_BY_VARIABLES*l);
}


/**
 * Sets the variable ranges with the values of the given backup.
 */
void update_variable_backup(int* backup,const InputVariables* v) {
if (backup==NULL) {
    fatal_error("NULL error in install_variable_backup\n");
}
int l=0;
if (v!=NULL)
  if (v->variable_index != NULL)
      l=v->variable_index->size;

/* v->variables is an array of struct transduction_variable
   which is a structure of (two before) NB_INT_BY_VARIABLES int */
memcpy((void*)&backup[0],(const void*)(&(v->variables[0])),sizeof(int)*NB_INT_BY_VARIABLES*l);
}

/* to limit number of malloc, we define a pool of memory (like a stack)
 * we use this thing : free will be done in reverse order than alloc
 **/

/*
 * create_variable_backup_memory_reserve : build a reserve of memory
 * with space for nb_item_allocated int
 */


#define SUGGESTED_NB_VARIABLE_BACKUP_IN_RESERVE_FIRST (64)
#define SUGGESTED_NB_VARIABLE_BACKUP_IN_RESERVE (32)
#define LIMIT_MIN_SUGGESTED_SIZE (16384)
#define LIMIT_MAX_SUGGESTED_SIZE (65536)


/*
 * is_enough_memory_in_reserve_for_Variable return 1 is there is sufficient space in reserve
 *  to create a backup of v
 * else, return 0
 */


int suggest_size_backup_reserve(int size_one_backup,int is_first)
{
/* we store the number of element after the list.
   we add 2 and not 1 for alignement and faster memcpy */

/* we suggest several SUGGESTED_NB_VARIABLE_BACKUP_IN_RESERVE because we want prevent malloc/free at each step */

int suggested_size = size_one_backup *
        (is_first ? SUGGESTED_NB_VARIABLE_BACKUP_IN_RESERVE_FIRST : SUGGESTED_NB_VARIABLE_BACKUP_IN_RESERVE);
if (suggested_size > LIMIT_MAX_SUGGESTED_SIZE)
    suggested_size = LIMIT_MAX_SUGGESTED_SIZE;
if (suggested_size < LIMIT_MIN_SUGGESTED_SIZE)
    suggested_size = LIMIT_MIN_SUGGESTED_SIZE;

suggested_size -= suggested_size % size_one_backup;
if (suggested_size < size_one_backup)
    suggested_size = size_one_backup;

return suggested_size;
}

variable_backup_memory_reserve* create_variable_backup_memory_reserve(const InputVariables* v,int is_first)
{
int size_variable_index = v->variable_index->size;
int size_unaligned = (size_variable_index*NB_INT_BY_VARIABLES);
int size_aligned = (int)(((((size_unaligned+4) * sizeof(int)) + 0x0f) & 0x7ffffff0) / sizeof(int));

int nb_item_allocated=suggest_size_backup_reserve(size_aligned,is_first);
variable_backup_memory_reserve* ptr = (variable_backup_memory_reserve*)
   malloc(sizeof(variable_backup_memory_reserve)+(sizeof(int)*nb_item_allocated));

if (ptr==NULL) {
   fatal_alloc_error("create_variable_backup_memory_reserve");
}

ptr->nb_backup_possible_array = nb_item_allocated / size_aligned;
ptr->pos_used = 0;
ptr->size_variable_index = size_variable_index;
ptr->size_copydata = (int)(size_unaligned*sizeof(int));
ptr->size_aligned = size_aligned;

/* we set Dirty to one, because where there is no filling varriable array, we
   cannot try reuse it */

ptr->array_int[OFFSET_DIRTY]=1;
ptr->array_int[OFFSET_COUNTER]=0;
ptr->array_int[OFFSET_SWAPPER]=0;

return ptr;
}

/*
 * clear the reserve from memory
 */
void free_reserve(variable_backup_memory_reserve*r)
{
    if (r->pos_used != 0) {
        fatal_alloc_error("free_reserve : used reserve");
}
free(r);
}


/*
 * This function select a new content for the set of variable, and return a
 * pointer to restore current set using restore_variable_array
 */
int* install_variable_backup_preserving(InputVariables* v,variable_backup_memory_reserve* r,const int* data)
{
int *save = (int*)&(v->variables[0]);
int* newptr = &(r->array_int[OFFSET_BACKUP+(r->pos_used * r->size_aligned)]);
memcpy((void*)newptr,(const void*)data,r->size_copydata);
v->variables = (struct transduction_variable*)newptr;

r->pos_used ++;
r->array_int[OFFSET_DIRTY+(r->pos_used * r->size_aligned)] = 1;
r->array_int[OFFSET_COUNTER+(r->pos_used * r->size_aligned)] = 0;
r->array_int[OFFSET_SWAPPER+(r->pos_used * r->size_aligned)] = 1;

return save;
}


void restore_variable_array(InputVariables* v,variable_backup_memory_reserve* r,int* rest)
{
    r->pos_used --;

    v->variables = (struct transduction_variable*)rest;
}


/**
 * Returns 1 if the given backup and the given variables correspond to the same
 * values.
 */
int same_input_variables(int* input_variable_backup,InputVariables* v) {
  // Variables is a structure to associate ranges to variable names,
  // it has two members: variable_index and variables. The variables
  // member is a structure used to define the range of a variable, it
  // has 4 members: start_in_tokens, end_in_tokens, start_in_chars and
  // end_in_chars. The next loop compares all the 4 members of
  // input_variable_backup and (int*)v->variables
  int* variables = (int*)v->variables;
  int number_of_members = NB_INT_BY_VARIABLES * v->variable_index->size;
  for (int i = 0; i < number_of_members; i+=NB_INT_BY_VARIABLES) {
	// start_in_tokens: same position of the first token
	if (variables [i]   != input_variable_backup[i])   return 0;
	// end_in_tokens: same  position after the last token
	if (variables [i+1] != input_variable_backup[i+1]) return 0;
	// start_in_chars: same starting position in chars in the first token
	if (variables [i+2] != input_variable_backup[i+2]) return 0;
	// end_in_chars: same ending position in chars in the last token
	if (variables [i+3] != input_variable_backup[i+3]) return 0;
  }
  return 1;
}

} // namespace unitex
