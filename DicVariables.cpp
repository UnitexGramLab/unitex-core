/*
 * Unitex
 *
 * Copyright (C) 2001-2012 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
struct dic_variable* new_dic_variable(const unichar* name,struct dela_entry* dic_entry,
                                      struct dic_variable* next,int must_clone) {
struct dic_variable* tmp=(struct dic_variable*)malloc(sizeof(struct dic_variable));
if (tmp==NULL) {
   fatal_alloc_error("new_dic_variable");
}
tmp->name=u_strdup(name);
if (must_clone) {
	tmp->dic_entry=clone_dela_entry(dic_entry);
} else {
	tmp->dic_entry=dic_entry;
}
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
void set_dic_variable(unichar* name,struct dela_entry* dic_entry,struct dic_variable* *list,int must_clone) {
while (*list!=NULL) {
   if (!u_strcmp((*list)->name,name)) {
      /* If we have found the variable we were looking for */
      /* We have to free the previous value */
      free_dela_entry((*list)->dic_entry);
      if (must_clone) {
    	  (*list)->dic_entry=clone_dela_entry(dic_entry);
      } else {
    	  (*list)->dic_entry=dic_entry;
      }
      return;
   }
   list=&((*list)->next);
}
*list=new_dic_variable(name,dic_entry,NULL,must_clone);
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
struct dic_variable* clone_dic_variable_list(const struct dic_variable* list) {
if (list==NULL) return NULL;
return new_dic_variable(list->name,list->dic_entry,clone_dic_variable_list(list->next),1);
}


/**
 * Returns 1 if the two given dic variable lists are identical; 0 otherwise.
 */
int same_dic_variables(struct dic_variable* backup,struct dic_variable* v) {
while (backup!=NULL && v!=NULL) {
	if (u_strcmp(backup->name,v->name)
		|| !equal(backup->dic_entry,v->dic_entry)) return 0;
	backup=backup->next;
	v=v->next;
}

return (backup==NULL && v==NULL);
}
