 /*
  * Unitex
  *
  * Copyright (C) 2001-2007 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "ParsingInfo.h"
#include "Error.h"


/**
 * Allocates, initializes and returns a new parsing info structure.
 */
struct parsing_info* new_parsing_info(int pos,int stack_pointer,unichar* stack,Variables* v) {
struct parsing_info* info;
info=(struct parsing_info*)malloc(sizeof(struct parsing_info));
if (info==NULL) {
   fatal_error("Not enough memory in new_parsing_info\n");
}
//u_printf("%S\n",stack);
info->position=pos;
info->next=NULL;
info->stack_pointer=stack_pointer;
info->stack=u_strdup(stack);
info->variable_backup=create_variable_backup(v);
return info;
}


/**
 * Frees the whole memory associated to the given information list.
 */
void free_parsing_info(struct parsing_info* list) {
struct parsing_info* tmp;
while (list!=NULL) {
   tmp=list->next;
   free(list->stack);
   free_variable_backup(list->variable_backup);
   free(list);
   list=tmp;
}
}


/**
 * Inserts an element in the given information list only if there is no element
 * with same end position of match.
 */
struct parsing_info* insert_if_absent(int pos,struct parsing_info* list,int stack_pointer,
                                      unichar* stack,Variables* v) {
if (list==NULL) return new_parsing_info(pos,stack_pointer,stack,v);
if (list->position==pos) {
   list->stack_pointer=stack_pointer;
   /* We free the previous stack */
   free(list->stack);
   list->stack=u_strdup(stack);
   free_variable_backup(list->variable_backup);
   list->variable_backup=create_variable_backup(v);
   return list;
}
list->next=insert_if_absent(pos,list->next,stack_pointer,stack,v);
return list;
}


/**
 * Inserts an element in the given information list only if there is no element
 * with position and same stack. */
struct parsing_info* insert_if_different(int pos,struct parsing_info* list,int stack_pointer,
                                         unichar* stack,Variables* v) {
if (list==NULL) return new_parsing_info(pos,stack_pointer,stack,v);
if ((list->position==pos) /* If the length is the same... */
    && !(u_strcmp(list->stack,stack))) { /* ...and if the stack content too */
    /* then we overwrite the current list element */
   list->stack_pointer=stack_pointer;
//    /* We free the previous stack */
//    free(list->stack);
//    list->stack=u_strdup(stack);
   free_variable_backup(list->variable_backup);
   list->variable_backup=create_variable_backup(v);
   return list;
}
/* Otherwise, we look in the rest of the list */
list->next=insert_if_different(pos,list->next,stack_pointer,stack,v);
return list;
}



