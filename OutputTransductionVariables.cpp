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


#ifdef TYPE_PACK_MULTIBITS
typedef TYPE_PACK_MULTIBITS uint_pack_multibits;
typedef TYPE_PACK_MULTIBITS uint_pack_multiunichar;
#else
#include <stdint.h>
typedef uint64_t uint_pack_multibits;
typedef uint64_t uint_pack_multiunichar;
#endif



#include "OutputTransductionVariables.h"
#include "Error.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {


static inline void copy_string(unichar* dest, const unichar* src, unsigned int len) {
#ifdef COPY_STRING_USE_MEMCPY
	memcpy(dest, src, (len + 1) * sizeof(unichar));
#else
	#define factor_unichar_to_pack (sizeof(uint_pack_multiunichar)/sizeof(unichar))

	unsigned int size_in_byte = (len + 1)*sizeof(unichar);
	unsigned int len_in_pack_multiunichar = (size_in_byte + sizeof(uint_pack_multiunichar) - 1) / sizeof(uint_pack_multiunichar);

	for (unsigned int i = 0; i < len_in_pack_multiunichar; i++)
	{
		uint_pack_multiunichar mc = *(((uint_pack_multiunichar*)src) + i);
		*(((uint_pack_multiunichar*)dest) + i) = mc;
	}
#endif
}

static inline void add_output_variable_to_pending_list(OutputVarList* *list,Ustring* s);
static inline void remove_output_variable_from_pending_list(OutputVarList* *list,Ustring* s);



#define my_around_align_ispending_array(x)  ((((x)+0x0f)/0x10)*0x10)
#define my_around_align(x,sz)  ((((x)+((sz)-1))/(sz))*(sz))
#define my_around_align_intptr_size_ispending_array(x)  ((((x)+sizeof(uint_pack_multibits)-1)/sizeof(uint_pack_multibits))*sizeof(uint_pack_multibits))

/**
 * Allocates and returns a structure representing the variables
 * whose names are in 'list'. The variable values are initialized
 * with empty strings.
 */
OutputVariables* new_OutputVariables(struct list_ustring* list,int* p_nbvar,vector_ptr* injected) {
OutputVariables* v=(OutputVariables*)malloc(sizeof(OutputVariables));
if (v==NULL) {
   fatal_alloc_error("new_OutputVariables");
}
v->variable_index=new_string_hash(DONT_USE_VALUES);
while (list!=NULL) {
   get_value_index(list->string,v->variable_index);
   list=list->next;
}
if (injected!=NULL) {
	for (int i=0;i<injected->nbelems;i+=2) {
		get_value_index((unichar*)(injected->tab[i]),v->variable_index);
	}
}
unsigned int nb_var=v->variable_index->size;
v->nb_var = (unsigned int)nb_var;
v->variables=(Ustring**)malloc(nb_var*sizeof(Ustring*));
if (v->variables==NULL) {
   fatal_alloc_error("new_OutputVariables");
}
for (unsigned int i=0;i<nb_var;i++) {
   v->variables[i]=new_Ustring();
}
v->pending=NULL;
v->is_pending=(char*)calloc(my_around_align_intptr_size_ispending_array(my_around_align_ispending_array(nb_var*sizeof(char))),1);
v->is_pending_array_size_intptr_size_rounded = my_around_align_intptr_size_ispending_array(my_around_align_ispending_array(nb_var*sizeof(char))) / sizeof(uint_pack_multibits);
// size for backup
size_t size_header = 2 * sizeof(int);
size_t pending_list_offset = size_header;
size_t size_pending_list = (nb_var + 1)*sizeof(int);
size_t size_string_index = ((nb_var + 1) * 2) * sizeof(int);
v->string_index_offset = pending_list_offset + size_pending_list;
v->unichars_offset = my_around_align(v->string_index_offset + size_string_index,0x10);

if (v->is_pending==NULL) {
   fatal_alloc_error("new_OutputVariables");
}
if (p_nbvar!=NULL) {
    *p_nbvar = nb_var;
}
if (injected!=NULL) {
	for (int i=0;i<injected->nbelems;i+=2) {
		int index=get_value_index((unichar*)(injected->tab[i]),v->variable_index);
		u_strcpy(v->variables[index],(unichar*)(injected->tab[i+1]));
	}
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
Ustring* get_output_variable(OutputVariables* v,const unichar* name) {
int n=get_value_index(name,v->variable_index,DONT_INSERT);
if (n==-1) {
   return NULL;
}
return v->variables[n];
}


#define OFFSET_NB_PENDING 0
#define OFFSET_NB_FILLED_STRING (sizeof(int))
#define OFFSET_PENDING_LIST (sizeof(int)*2)
/**
 * Allocates, initializes and returns a unichar array that is a copy of
 * the variable values. The array starts with a subarray of size 'n'
 * (n=number of variables) with for each variable 1 if it is pending and 0 otherwise.
 */
OutputVariablesBackup* create_output_variable_backup(OutputVariables* RESTRICT v,Abstract_allocator prv_alloc) {
if (v==NULL || v->variable_index==NULL) return NULL;
unsigned int nb_var=(unsigned int)v->variable_index->size;

if (nb_var==0) return NULL;
unsigned int size_strings=0;

for (unsigned int i=0;i<nb_var;i++) {
	unsigned int len = v->variables[i]->len;
	size_strings += len;
}
OutputVariablesBackup* backup=(OutputVariablesBackup*)malloc_cb(
	v->unichars_offset + (size_strings * sizeof(unichar)) +		
		((nb_var+1) * ( sizeof(uint_pack_multibits)+sizeof(unichar))),prv_alloc);
if (backup==NULL) {
   fatal_alloc_error("create_output_variable_backup");
}

unsigned int nb_pending = 0;
unsigned int * pending_var_list = (unsigned int *)(((char*)backup) + OFFSET_PENDING_LIST);

for (unsigned int i = 0; i<(unsigned int)nb_var; i++) {
	*(pending_var_list + nb_pending) = i;
	nb_pending+= v->is_pending[i];
}
*(pending_var_list + nb_pending) = nb_var;

*((unsigned int*)(((char*)backup) + OFFSET_NB_PENDING)) = nb_pending;

unsigned int* string_index = (unsigned int*)(((char*)backup) + (v->string_index_offset));
 
if ((!nb_pending) && (!size_strings)) {
	*string_index = nb_var;
	*((unsigned int*)(((char*)backup) + OFFSET_NB_FILLED_STRING)) = 0;
	return (OutputVariablesBackup*)backup;
}

unichar* backup_string = (unichar*)(((char*)backup) + v->unichars_offset);
unsigned int nb_string_filled = 0;
for (unsigned int i=0;i<(unsigned int)nb_var;i++) {
    {
		unsigned int len = v->variables[i]->len;
		if (len > 0)
		{
			*(string_index + (nb_string_filled * 2)) = i;
			*(string_index + (nb_string_filled * 2) + 1) = len;
			//memcpy(backup_string, v->variables[i]->str, sizeof(unichar)*(len + 1));
			copy_string(backup_string, v->variables[i]->str, len);
			backup_string += my_around_align(len + 1, sizeof(uint_pack_multibits));
			nb_string_filled++;
		}
    }
}

*(string_index + (nb_string_filled * 2)) = nb_var;

*((unsigned int*)(((char*)backup) + OFFSET_NB_FILLED_STRING)) = nb_string_filled;

return (OutputVariablesBackup*)backup;
}


/**
 * Frees the given variable backup.
 */
void free_output_variable_backup(OutputVariablesBackup* backup,Abstract_allocator prv_alloc) {
if (backup!=NULL) free_cb(backup,prv_alloc);
}


/**
 * Sets the variables with the values of the given backup.
 */
void install_output_variable_backup(OutputVariables* RESTRICT v,const OutputVariablesBackup* RESTRICT backup) {
if (backup==NULL) return;

/* First, we free the previous pending list */
OutputVarList* tmp;
while (v->pending!=NULL) {
	tmp=v->pending;
	v->pending=v->pending->next;
	/* We must not free tmp->var */
	free(tmp);
}

uint_pack_multibits* is_pending_erase = (uint_pack_multibits*)v->is_pending;
size_t limit = v->is_pending_array_size_intptr_size_rounded;
for (size_t i = 0; i < limit; i++) {
	*(is_pending_erase + i) = (uint_pack_multibits)0;
}

const unsigned int * pending_var_list = (const unsigned int*)(((const char*)backup) + OFFSET_PENDING_LIST);
const unsigned int nb_pending = *((const unsigned int*)(((const char*)backup) + OFFSET_NB_PENDING)) ;
const unsigned int nb_filled_strings = *((const unsigned int*)(((const char*)backup) + OFFSET_NB_FILLED_STRING)) ;
unsigned int nb_var = (unsigned int)v->variable_index->size;

// first int of backup is set to 0 if we have full empty backup
if ((!nb_pending) && (!nb_filled_strings)) {

	for (unsigned int i = 0; i < nb_var; i++) {
	    v->variables[i]->str[0] = 0;
	    v->variables[i]->len = 0 ;
	}
	return;
}

for (unsigned int loop_pending = 0; loop_pending < nb_pending; loop_pending++)
{
	unsigned int cur_pending_item = *(pending_var_list + loop_pending);
	v->is_pending[cur_pending_item] = 1;
	add_output_variable_to_pending_list(&(v->pending), v->variables[cur_pending_item]);
}


const unsigned int* string_index = (const unsigned int*)(((const char*)backup) + (v->string_index_offset));
const unichar* backup_string = (const unichar*)(((const char*)backup) + v->unichars_offset);
int pos_in_index = 0;
unsigned int cur_item_in_index = *(string_index + (pos_in_index * 2));

unsigned int i = 0;
for (;;) {
	while ((i+4) < cur_item_in_index) {
		v->variables[i]->len = 0;
		*(v->variables[i]->str) = 0;
		v->variables[i+1]->len = 0;
		*(v->variables[i+1]->str) = 0;
		v->variables[i+2]->len = 0;
		*(v->variables[i+2]->str) = 0;
		v->variables[i+3]->len = 0;
		*(v->variables[i+3]->str) = 0;
		i+=4;
	}
	while (i < cur_item_in_index) {
		v->variables[i]->len = 0;
		*(v->variables[i]->str) = 0;
		i++;
	}
	if (i == nb_var)
		break;

	Ustring * cur_ustr = v->variables[i];

	int cur_len_in_index = *(string_index + (pos_in_index * 2) + 1);
	if (cur_ustr->size < (unsigned int)cur_len_in_index)
		resize(cur_ustr, cur_len_in_index + 1);
	cur_ustr->len = cur_len_in_index;
	//memcpy(cur_ustr->str, backup_string, (cur_len_in_index + 1) * sizeof(unichar));
	copy_string(cur_ustr->str, backup_string, cur_len_in_index);
	//backup_string += cur_len_in_index + 1;
	backup_string += my_around_align(cur_len_in_index + 1, sizeof(uint_pack_multibits));

	pos_in_index++;
	cur_item_in_index = *(string_index + (pos_in_index * 2));
	
	i++;
}


}


/**
* Returns 1 if the given backup correspond to the same values than the given
* output variables; 0 otherwise.
*/
int same_output_variables(const OutputVariablesBackup* backup, OutputVariables* v) {
	if (v == NULL) {
		return backup == NULL;
	}

	unsigned int nb_var = (unsigned int)v->nb_var;

	if (nb_var == 0) return 1;


	const unsigned int * pending_var_list_backup = (const unsigned int*)(((const char*)backup) + OFFSET_PENDING_LIST);
	const unsigned int nb_pending_backup = *((const unsigned int*)(((const char*)backup) + OFFSET_NB_PENDING));
	const unsigned int nb_filled_strings_backup = *((const unsigned int*)(((const char*)backup) + OFFSET_NB_FILLED_STRING));



	// first int of backup is set to 0 if we have full empty backup
	if ((!nb_pending_backup) && (!nb_filled_strings_backup)) {
		for (unsigned int i = 0;i<nb_var;i++) {
			if (0 != v->is_pending[i]) {
				return 0;
			}
		}

		for (unsigned int i = 0;i<nb_var;i++) {
			if (v->variables[i]->str[0] != '\0') {
				return 0;
			}
		}

		return 1;
	}


	unsigned int nb_pending_v = 0;

	for (size_t i = 0; i<nb_var; i++) {
		if (v->is_pending[i]) {
			if (*(pending_var_list_backup + nb_pending_v) != i) {
				return 0;
			}
			nb_pending_v++;
		}
	}

	if (*(pending_var_list_backup + nb_pending_v) != nb_var) {
		return 0;
	}


	const unsigned int* string_index = (const unsigned int*)(((const char*)backup) + (v->string_index_offset));
	const unichar* backup_string = (const unichar*)(((const char*)backup) + v->unichars_offset);
	unsigned int pos_in_index = 0;
	unsigned int cur_item_in_index = *(string_index + (pos_in_index * 2));
	for (unsigned int i = 0; i<nb_var; i++) {
		Ustring * cur_ustr = v->variables[i];
		if (i == cur_item_in_index)
		{
			unsigned int cur_len_in_index = *(string_index + (pos_in_index * 2) + 1);
			if (cur_ustr->len != cur_len_in_index) {
				return 0;
			}
			if (memcmp(cur_ustr->str, backup_string, (cur_len_in_index + 1) * sizeof(unichar)) != 0) {
				return 0;
			}
			//backup_string += cur_len_in_index + 1;
			backup_string += my_around_align(cur_len_in_index + 1, sizeof(uint_pack_multibits));

			pos_in_index++;
			cur_item_in_index = *(string_index + (pos_in_index * 2));
		}
		else
		{
			if (cur_ustr->len != 0) {
				return 0;
			}
		}
	}


	return 1;
}


/**
 * Adds the given Ustring pointer to the list, with no duplicates.
 */
static inline void add_output_variable_to_pending_list(OutputVarList* *list,Ustring* s) {
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
static inline void remove_output_variable_from_pending_list(OutputVarList* *list,Ustring* s) {
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
unsigned int add_raw_string_to_output_variables(OutputVariables* var,unichar* s) {
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


} // namespace unitex
