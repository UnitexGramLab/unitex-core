/*
 * Unitex
 *
 * Copyright (C) 2001-2010 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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


/**
 * Allocates and returns a structure representing the variables
 * whose names are in 'list'. The variable ranges are initialized with [-1;-1[
 */
Variables* new_Variables(struct list_ustring* list) {
Variables* v=(Variables*)malloc(sizeof(Variables));
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
}
return v;
}


/**
 * Frees the memory associated to the given variables.
 */
void free_Variables(Variables* v) {
if (v==NULL) return;
free_string_hash(v->variable_index);
free(v->variables);
free(v);
}


/**
 * Returns a pointer on the range of the variable whose name is 'name',
 * or NULL if the variable in not in the given variable set.
 */
struct transduction_variable* get_transduction_variable(Variables* v,unichar* name) {
int n=get_value_index(name,v->variable_index,DONT_INSERT);
if (n==-1) {
   return NULL;
}
return &(v->variables[n]);
}


/**
 * Sets the start value of the variable #n.
 */
void set_variable_start(Variables* v,int n,int value) {
v->variables[n].start_in_tokens=value;
}


/**
 * Sets the end value of the variable #n.
 */
void set_variable_end(Variables* v,int n,int value) {
v->variables[n].end_in_tokens=value;
}


/**
 * Returns the start value of the variable #n.
 */
int get_variable_start(Variables* v,int n) {
return v->variables[n].start_in_tokens;
}


/**
 * Returns the end value of the variable #n.
 */
int get_variable_end(Variables* v,int n) {
return v->variables[n].end_in_tokens;
}


/**
 * Allocates, initializes and returns an integer array that is a copy of
 * the variable ranges.
 */
int* create_variable_backup(Variables* v) {
if (v==NULL || v->variable_index==NULL) return NULL;
int l=v->variable_index->size;
int* backup=(int*)malloc(sizeof(int)*2*l);
if (backup==NULL) {
   fatal_alloc_error("create_variable_backup");
}

/* v->variables is an array of struct transduction_variable
   which is a structure of two int */
memcpy((void*)&backup[0],(void*)(&(v->variables[0])),sizeof(int)*2*l);

return backup;
}


/**
 * Frees the given variable backup.
 */
void free_variable_backup(int* backup) {
if (backup!=NULL) free(backup);
}


/**
 * Sets the variable ranges with the values of the given backup.
 */
void install_variable_backup(Variables* v,int* backup) {
if (backup==NULL) {
	fatal_error("NULL error in install_variable_backup\n");
}
int l=v->variable_index->size;

/* v->variables is an array of struct transduction_variable
   which is a structure of two int */
memcpy((void*)(&(v->variables[0])),(void*)&backup[0],sizeof(int)*2*l);
}


/**
 * Sets the variable ranges with the values of the given backup.
 */
void update_variable_backup(int* backup,Variables* v) {
if (backup==NULL) {
	fatal_error("NULL error in install_variable_backup\n");
}
int l=0;
if (v!=NULL)
  if (v->variable_index != NULL)
      l=v->variable_index->size;

/* v->variables is an array of struct transduction_variable
   which is a structure of two int */
memcpy((void*)&backup[0],(void*)(&(v->variables[0])),sizeof(int)*2*l);
}

/* to limit number of malloc, we define a pool of memory (like a stack)
 * we use this thing : free will be done in reverse order than alloc
 **/

/*
 * create_variable_backup_memory_reserve : build a reserve of memory
 * with space for nb_item_allocated int
 */


#define SUGGESTED_NB_VARIABLE_BACKUP_IN_RESERVE (32)
#define LIMIT_MIN_SUGGESTED_SIZE (16384)
#define LIMIT_MAX_SUGGESTED_SIZE (65536)


/*
 * is_enough_memory_in_reserve_for_Variable return 1 is there is sufficient space in reserve
 *  to create a backup of v
 * else, return 0
 */


/*
 * check if the reserve contain space and is correct to save variable v
 */
int is_enough_memory_in_reserve_for_two_set_variables(Variables* v,variable_backup_memory_reserve* r)
{
    return (((r->pos_used+1) < r->nb_backup_possible_array) && (v->variable_index->size == r->size_variable_index));
}

int suggest_size_backup_reserve(int size_one_backup)
{
/* we store the number of element after the list.
   we add 2 and not 1 for alignement and faster memcpy */

/* we suggest several SUGGESTED_NB_VARIABLE_BACKUP_IN_RESERVE because we want prevent malloc/free at each step */

int suggested_size = size_one_backup * SUGGESTED_NB_VARIABLE_BACKUP_IN_RESERVE;
if (suggested_size > LIMIT_MAX_SUGGESTED_SIZE)
    suggested_size = LIMIT_MAX_SUGGESTED_SIZE;
if (suggested_size < LIMIT_MIN_SUGGESTED_SIZE)
    suggested_size = LIMIT_MIN_SUGGESTED_SIZE;

suggested_size -= suggested_size % size_one_backup;
if (suggested_size < size_one_backup)
    suggested_size = size_one_backup;

return suggested_size;
}

variable_backup_memory_reserve* create_variable_backup_memory_reserve(Variables* v)
{
int size_variable_index = v->variable_index->size;
int size_unaligned = (size_variable_index*2);
int size_aligned = (int)(((((size_unaligned+4) * sizeof(int)) + 0x0f) & 0x7ffffff0) / sizeof(int));

int nb_item_allocated=suggest_size_backup_reserve(size_aligned);
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
 * create the backup, taking memory from reserve
 * we assume is_enough_memory_in_reserve_for_Variable was already called to verify
 */
int* create_variable_backup_using_reserve(Variables* v,variable_backup_memory_reserve* r) {
int l=v->variable_index->size;
if (l != r->size_variable_index) {
    fatal_error("bad size\n");
}

/* DIRTY==0 mean :
   - there is a least one backup
   - there is no modification made on the variable array since last backup
   - the last used array is not a "swapper" 
    so we can reuse the backup previously made !  
 */
if ((r->array_int[OFFSET_DIRTY+(r->pos_used * r->size_aligned)] == 0))
{
    r->array_int[OFFSET_COUNTER+(r->pos_used * r->size_aligned)]++;
    int* prev = &(r->array_int[OFFSET_BACKUP+((r->pos_used-1) * r->size_aligned)]);
    return prev;
}

/* v->variables is an array of struct transduction_variable
   which is a structure of two int */
int* ret = &(r->array_int[OFFSET_BACKUP+(r->pos_used * r->size_aligned)]);
memcpy((void*)ret,(void*)(&(v->variables[0])),r->size_copydata);

r->pos_used ++;
r->array_int[OFFSET_DIRTY+(r->pos_used * r->size_aligned)] = 0;
r->array_int[OFFSET_COUNTER+(r->pos_used * r->size_aligned)] = 0;
r->array_int[OFFSET_SWAPPER+(r->pos_used * r->size_aligned)] = 0;

return ret;
}

/*
 * free memory from reserve
 * return 0 if there is still used space in reserve
 * return 1 if the reserve can be free
 */
int free_variable_backup_using_reserve(variable_backup_memory_reserve* r)
{
if (r->array_int[OFFSET_COUNTER+(r->pos_used * r->size_aligned)]>0)
{
    r->array_int[OFFSET_COUNTER+(r->pos_used * r->size_aligned)]--;
    return 0;
}

r->pos_used --;
return (r->pos_used == 0) ? 1 : 0;
}


/*
 * This function select a new content for the set of variable, and return a
 * pointer to restore current set using restore_variable_array
 */
int* install_variable_backup_preserving(Variables* v,variable_backup_memory_reserve* r,int* data)
{
int *save = &(v->variables[0].start_in_tokens);
int* newptr = &(r->array_int[OFFSET_BACKUP+(r->pos_used * r->size_aligned)]);
memcpy((void*)newptr,(void*)data,r->size_copydata);
v->variables = (struct transduction_variable*)newptr;

r->pos_used ++;
r->array_int[OFFSET_DIRTY+(r->pos_used * r->size_aligned)] = 1;
r->array_int[OFFSET_COUNTER+(r->pos_used * r->size_aligned)] = 0;
r->array_int[OFFSET_SWAPPER+(r->pos_used * r->size_aligned)] = 1;

return save;
}

    
void restore_variable_array(Variables* v,variable_backup_memory_reserve* r,int* rest)
{
    r->pos_used--;

    v->variables = (struct transduction_variable*)rest;
    if (v->variables[0].start_in_tokens == UNDEF_VAR_BOUND)
        v->variables[0].start_in_tokens+=0;
}
