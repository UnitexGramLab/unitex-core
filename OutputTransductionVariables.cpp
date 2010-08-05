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

#include "OutputTransductionVariables.h"
#include "Error.h"


static void add_output_variable_to_pending_list(OutputVarList* *list,Ustring* s);
static void remove_output_variable_from_pending_list(OutputVarList* *list,Ustring* s);


/**
 * Allocates and returns a structure representing the variables
 * whose names are in 'list'. The variable values are initialized
 * with empty strings.
 */
OutputVariables* new_OutputVariables(struct list_ustring* list) {
OutputVariables* v=(OutputVariables*)malloc(sizeof(OutputVariables));
if (v==NULL) {
   fatal_alloc_error("new_OutputVariables");
}
v->variable_index=new_string_hash(DONT_USE_VALUES);
int l=0;
while (list!=NULL) {
   get_value_index(list->string,v->variable_index);
   l++;
   list=list->next;
}
v->variables=(Ustring**)malloc(l*sizeof(Ustring*));
if (v->variables==NULL) {
   fatal_alloc_error("new_OutputVariables");
}
for (int i=0;i<l;i++) {
   v->variables[i]=new_Ustring();
}
v->pending=NULL;
v->is_pending=(char*)calloc(l,sizeof(char));
if (v->is_pending==NULL) {
   fatal_alloc_error("new_OutputVariables");
}
return v;
}


/**
 * Frees the memory associated to the given variables.
 */
void free_OutputVariables(OutputVariables* v) {
if (v==NULL) return;
int size=v->variable_index->size;
free_string_hash(v->variable_index);
for (int i=0;i<size;i++) {
	free_Ustring(v->variables[i]);
}
free(v->variables);
OutputVarList* l=v->pending;
OutputVarList* tmp;
while (l!=NULL) {
	tmp=l;
	l=l->next;
	/* Don't free l->var, since it has been freed by free_Ustring above */
	free(tmp);
}
free(v->is_pending);
free(v);
}


/**
 * Returns a pointer on the Ustring associated the variable whose name is 'name',
 * or NULL if the variable in not in the given variable set.
 */
Ustring* get_output_variable(OutputVariables* v,unichar* name) {
int n=get_value_index(name,v->variable_index,DONT_INSERT);
if (n==-1) {
   return NULL;
}
return v->variables[n];
}




/**
 * Allocates, initializes and returns a unichar array that is a copy of
 * the variable values. The array starts with a subarray of size 'n'
 * (n=number of variables) with for each variable 1 if it is pending and 0 otherwise.
 */
unichar* create_output_variable_backup(OutputVariables* v) {
if (v==NULL || v->variable_index==NULL) return NULL;
int l=v->variable_index->size;
if (l==0) return NULL;
int size=0;
for (int i=0;i<l;i++) {
	/* +2 = +1 for the \0 and +1 to indicate if the variable is pending or not */
	size=size+v->variables[i]->len+2;
}
unichar* backup=(unichar*)malloc(size*sizeof(unichar));
if (backup==NULL) {
   fatal_alloc_error("create_output_variable_backup");
}
for (int i=0;i<l;i++) {
	backup[i]=v->is_pending[i];
}
int pos=l;
for (int i=0;i<l;i++) {
	u_strcpy(backup+pos,v->variables[i]->str);
	pos=pos+v->variables[i]->len;
	*(backup+pos)='\0';
	pos++;
}
return backup;
}


/**
 * Frees the given variable backup.
 */
void free_output_variable_backup(unichar* backup) {
if (backup!=NULL) free(backup);
}


/**
 * Sets the variables with the values of the given backup.
 */
void install_output_variable_backup(OutputVariables* v,const unichar* backup) {
if (backup==NULL) return;
/* First, we free the previous pending list */
OutputVarList* tmp;
while (v->pending!=NULL) {
	tmp=v->pending;
	v->pending=v->pending->next;
	/* We must not free tmp->var */
	free(tmp);
}
/* Then we add all pending variables of the backup */
int l=v->variable_index->size;
for (int i=0;i<l;i++) {
	v->is_pending[i]=(backup[i]?1:0);
	if (backup[i]) {
		/* We also add pending variables to the 'pending' list */
		add_output_variable_to_pending_list(&(v->pending),v->variables[i]);
	}
}
int pos=l;
for (int i=0;i<l;i++) {
	u_strcpy(v->variables[i],backup+pos);
	pos=pos+v->variables[i]->len+1;
}
}


/**
 * Adds the given Ustring pointer to the list, with no duplicates.
 */
static void add_output_variable_to_pending_list(OutputVarList* *list,Ustring* s) {
while (*list != NULL) {
	if ((*list)->var==s) {
		/* The pointer is already in the list */
		return;
	}
	list=&((*list)->next);
}
(*list)=(OutputVarList*)malloc(sizeof(OutputVarList));
(*list)->var=s;
(*list)->next=NULL;
}


/**
 * Removes the given Ustring pointer from the list.
 */
static void remove_output_variable_from_pending_list(OutputVarList* *list,Ustring* s) {
while (*list != NULL) {
	if ((*list)->var==s) {
		/* We have found the pointer to be removed */
		OutputVarList* tmp=(*list);
		(*list)=(*list)->next;
		free(tmp);
		return;
	}
	list=&((*list)->next);
}
fatal_error("Non-existent Ustring pointer in remove_output_variable\n");
}


/**
 * Concatenates the given string to all strings of the given list.
 * Returns the length of 's'.
 */
unsigned int add_string_to_output_variables(OutputVariables* var,unichar* s) {
OutputVarList* list=var->pending;
if (list==NULL) {
	fatal_error("Should not invoke add_string_to_output_variables on an empty list\n");
}
if (s==NULL || s[0]=='\0') return 0;
unsigned int old=list->var->len;
OutputVarList* first=list;
while (list!=NULL) {
	u_strcat(list->var,s);
	list=list->next;
}
return first->var->len-old;
}


/**
 * Removes 'n' chars from all strings of the given list. If 'n'
 * is greater than the length of a string, the string is emptied.
 */
void remove_chars_from_output_variables(OutputVariables* var,unsigned int n) {
OutputVarList* list=var->pending;
if (n==0) return;
if (list==NULL) {
	fatal_error("Should not invoke remove_chars_from_output_variables on an empty list\n");
}
while (list!=NULL) {
	remove_n_chars(list->var,n);
	list=list->next;
}
}


/**
 * Sets the variable #index as being pending.
 */
void set_output_variable_pending(OutputVariables* var,int index) {
var->is_pending[index]=1;
add_output_variable_to_pending_list(&(var->pending),var->variables[index]);
}


/**
 * Unsets the variable #index as being pending.
 */
void unset_output_variable_pending(OutputVariables* var,int index) {
var->is_pending[index]=0;
remove_output_variable_from_pending_list(&(var->pending),var->variables[index]);
}


/**
 * Sets the given variable as being pending.
 */
void set_output_variable_pending(OutputVariables* var,const unichar* var_name) {
set_output_variable_pending(var,get_value_index(var_name,var->variable_index,DONT_INSERT));
}


/**
 * Unsets the given variable as being pending.
 */
void unset_output_variable_pending(OutputVariables* var,const unichar* var_name) {
unset_output_variable_pending(var,get_value_index(var_name,var->variable_index,DONT_INSERT));
}

