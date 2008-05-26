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

#include "DicVariables.h"
#include "Error.h"


/**
 * Allocates, initializes and returns a dic_variable.
 */
struct dic_variable* new_dic_variable(unichar* name,struct dela_entry* dic_entry,struct dic_variable* next) {
struct dic_variable* tmp=(struct dic_variable*)malloc(sizeof(struct dic_variable));
if (tmp==NULL) {
   fatal_error("Not enough memory in new_dic_variable\n");
}
tmp->name=u_strdup(name);
tmp->dic_entry=clone_dela_entry(dic_entry);
tmp->next=next;
return tmp;
}



/**
 * Frees a single dic_variable.
 */
void free_dic_variable(struct dic_variable* v) {
if (v==NULL) return;
free(v->name);
free_dela_entry(v->dic_entry);
free(v);
}



/**
 * Frees a whole dic variable list and sets it to NULL.
 */
void clear_dic_variable_list(struct dic_variable* *list) {
struct dic_variable* tmp;
while (*list!=NULL) {
   tmp=(*list)->next;
   free_dic_variable(*list);
   *list=tmp;
}
}



/**
 * Sets the given dic variable, inserting it in the variable list if absent.
 */
void set_dic_variable(unichar* name,struct dela_entry* dic_entry,struct dic_variable* *list) {
while (*list!=NULL) {
   if (!u_strcmp((*list)->name,name)) {
      /* If we have found the variable we were looking for */
      /* We do not have to free the previous value */
      (*list)->dic_entry=dic_entry;
      return;
   }
   list=&((*list)->next);
}
*list=new_dic_variable(name,dic_entry,NULL);
}



/**
 * Returns the struct dela_entry* associated to the given dic variable name, 
 * or NULL if not found.
 */
struct dela_entry* get_dic_variable(unichar* name,struct dic_variable* list) {
while (list!=NULL) {
   if (!u_strcmp(list->name,name)) {
      /* If we have found the variable we were looking for */
      return list->dic_entry;
   }
   list=list->next;
}
return NULL;
}



/**
 * Returns a copy of the given dic variable list.
 */
struct dic_variable* clone_dic_variable_list(struct dic_variable* list) {
if (list==NULL) return NULL;
return new_dic_variable(list->name,list->dic_entry,clone_dic_variable_list(list->next));
}
