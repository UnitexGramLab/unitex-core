 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
   v->variables[i].start=UNDEF_VAR_BOUND;
   v->variables[i].end=UNDEF_VAR_BOUND;
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
v->variables[n].start=value;
}


/**
 * Sets the end value of the variable #n.
 */
void set_variable_end(Variables* v,int n,int value) {
v->variables[n].end=value;
}


/**
 * Returns the start value of the variable #n.
 */
int get_variable_start(Variables* v,int n) {
return v->variables[n].start;
}


/**
 * Returns the end value of the variable #n.
 */
int get_variable_end(Variables* v,int n) {
return v->variables[n].end;
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


/* to limit number of malloc, we define a pool of memory (like a stack)
 * we use this thing : free will be done in reverse order than alloc
 **/

/*
 * create_variable_backup_memory_reserve : build a reserve of memory
 * with space for nb_item_allocated int
 */
variable_backup_memory_reserve* create_variable_backup_memory_reserve(int nb_item_allocated)
{
variable_backup_memory_reserve* ptr = (variable_backup_memory_reserve*)
   malloc(sizeof(variable_backup_memory_reserve)+(sizeof(int)*nb_item_allocated));

if (ptr==NULL) {
   fatal_alloc_error("create_variable_backup_memory_reserve");
}
ptr->nb_item_allocated = nb_item_allocated;
ptr->pos_used = 0;
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

#define SUGGESTED_NB_VARIABLE_BACKUP_IN_RESERVE (32)
#define LIMIT_MIN_SUGGESTED_SIZE (16384)
#define LIMIT_MAX_SUGGESTED_SIZE (65536)
/*
 * is_enough_memory_in_reserve_for_Variable return 1 is there is sufficient space in reserve
 *  to create a backup of v
 * else, return 0
 * *needed will contain the minimal space needed
 * *suggested_alloc is a suggestion of nb_item to allocate
 */
int is_enough_memory_in_reserve_for_Variable(Variables* v,variable_backup_memory_reserve* r)
{
    return ((r->pos_used + (v->variable_index->size)) <= r->nb_item_allocated) ;
}

int suggest_size_backup_reserve(Variables*v)
{
/* we store the number of element after the list.
   we add 2 and not 1 for alignement and faster memcpy */
int size_one_backup = 0;
if (v!=NULL)
  size_one_backup = ((v->variable_index->size*2) + 2);

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

/*
 * create the backup, taking memory from reserve
 */
int* create_variable_backup_using_reserve(Variables* v,variable_backup_memory_reserve* r) {
if (is_enough_memory_in_reserve_for_Variable(v,r)==0)
  fatal_error("not enough space in create_variable_backup_using_reserve\n");

int l=v->variable_index->size;
/* v->variables is an array of struct transduction_variable 
   which is a structure of two int */
int* ret = &(r->array_int[r->pos_used]);
memcpy((void*)ret,(void*)(&(v->variables[0])),sizeof(int)*2*l);
*(ret + (l*2))=l;

r->pos_used += (2*l)+2;

/* we store the number of element after the list. */

return ret;
}

/*
 * free memory from reserve
 * return 0 if there is still used space in reserve
 * return 1 if the reserve can be free
 */
int free_variable_backup_using_reserve(int*backup,variable_backup_memory_reserve* r)
{
if (r == NULL)
    fatal_error("free_variable_backup_using_reserve NULL parameter");
if (r->pos_used<2)
    fatal_error("free_variable_backup_using_reserve using empty reserve");

int size_transition = r->array_int[r->pos_used - 2];
int size_int = ((size_transition*2)+2);

if (r->pos_used < size_int)
    fatal_error("free_variable_backup_using_reserve using corrupt reserve");
if (backup != (&(r->array_int[r->pos_used - size_int])))
    fatal_error("free_variable_backup_using_reserve using not latest allocated");
r->pos_used -= size_int;
return (r->pos_used == 0) ? 1 : 0;
}
