 /*
  * Unitex
  *
  * Copyright (C) 2001-2007 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
   fatal_error("Not enough memory in new_Variables\n");
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
   fatal_error("Not enough memory in new_Variables\n");
}
for (int i=0;i<l;i++) {
   v->variables[i].start=-1;
   v->variables[i].end=-1;
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
if (v->variable_index==NULL) return NULL;
int l=v->variable_index->size;
int* backup=(int*)malloc(sizeof(int)*2*l);
if (backup==NULL) {
   fatal_error("Not enough memory in create_variable_backup\n");
}
int j=0;
for (int i=0;i<l;i++) {
   backup[j++]=v->variables[i].start;
   backup[j++]=v->variables[i].end;
}
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
int j=0;
int l=v->variable_index->size;
for (int i=0;i<l;i++) {
   v->variables[i].start=backup[j++];
   v->variables[i].end=backup[j++];
}
}


