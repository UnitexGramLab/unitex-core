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

#include "ParsingInfo.h"
#include "Error.h"


/**
 * Allocates, initializes and returns a new parsing info structure.
 */
struct parsing_info* new_parsing_info(int pos,int pos_in_token,int state,int stack_pointer,unichar* stack,
                                      Variables* v,struct dela_entry* dic_entry,struct dic_variable* v2,
                                      int left_ctx_shift,int left_ctx_base) {
struct parsing_info* info;
info=(struct parsing_info*)malloc(sizeof(struct parsing_info));
if (info==NULL) {
   fatal_error("Not enough memory in new_parsing_info\n");
}
info->position=pos;
info->pos_in_token=pos_in_token;
info->state_number=state;
info->next=NULL;
info->stack_pointer=stack_pointer;
info->stack=u_strdup(stack);
info->variable_backup=create_variable_backup(v);
info->dic_entry=clone_dela_entry(dic_entry);
info->dic_variable_backup=clone_dic_variable_list(v2);
info->left_ctx_shift=left_ctx_shift;
info->left_ctx_base=left_ctx_base;
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
   clear_dic_variable_list(&(list->dic_variable_backup));
   free_dela_entry(list->dic_entry);
   free(list);
   list=tmp;
}
}


/**
 * Inserts an element in the given information list only if there is no element
 * with same end position of match.
 */
struct parsing_info* insert_if_absent(int pos,int pos_in_token,int state,struct parsing_info* list,int stack_pointer,
                                      unichar* stack,Variables* v,struct dic_variable* v2,
                                      int left_ctx_shift,int left_ctx_base) {
if (list==NULL) return new_parsing_info(pos,pos_in_token,state,stack_pointer,stack,v,NULL,v2,
                                        left_ctx_shift,left_ctx_base);
if (list->position==pos && list->pos_in_token==pos_in_token && list->state_number==state) {
   list->stack_pointer=stack_pointer;
   /* We free the previous stack */
   free(list->stack);
   list->stack=u_strdup(stack);
   free_variable_backup(list->variable_backup);
   list->variable_backup=create_variable_backup(v);
   clear_dic_variable_list(&list->dic_variable_backup);
   list->dic_variable_backup=clone_dic_variable_list(v2);
   if (list->dic_entry!=NULL) {
      fatal_error("Unexpected non NULL dic_entry in insert_if_absent\n");
   }
   list->left_ctx_shift=left_ctx_shift;
   list->left_ctx_base=left_ctx_base;
   return list;
}
list->next=insert_if_absent(pos,pos_in_token,state,list->next,stack_pointer,stack,v,v2,
      left_ctx_shift,left_ctx_base);
return list;
}


/**
 * Inserts an element in the given information list only if there is no element
 * with position and same stack. */
struct parsing_info* insert_if_different(int pos,int pos_in_token,int state,struct parsing_info* list,int stack_pointer,
                                         unichar* stack,Variables* v,struct dic_variable* v2,
                                         int left_ctx_shift,int left_ctx_base) {
if (list==NULL) return new_parsing_info(pos,pos_in_token,state,stack_pointer,stack,v,NULL,v2,
                                        left_ctx_shift,left_ctx_base);
if ((list->position==pos) /* If the length is the same... */
    && (list->pos_in_token==pos_in_token)
    && (list->state_number==state)
    && !(u_strcmp(list->stack,stack)) /* ...and if the stack content too */
    && list->left_ctx_shift==left_ctx_shift
    && list->left_ctx_base==left_ctx_base) { 
    /* then we overwrite the current list element */
   list->stack_pointer=stack_pointer;
   free_variable_backup(list->variable_backup);
   list->variable_backup=create_variable_backup(v);
   clear_dic_variable_list(&list->dic_variable_backup);
   list->dic_variable_backup=clone_dic_variable_list(v2);
   if (list->dic_entry!=NULL) {
      fatal_error("Unexpected non NULL dic_entry in insert_if_different\n");
   }
   return list;
}
/* Otherwise, we look in the rest of the list */
list->next=insert_if_different(pos,pos_in_token,state,list->next,stack_pointer,stack,v,v2,
                               left_ctx_shift,left_ctx_base);
return list;
}


/**
 * This function behaves in the same way than 'insert_if_absent', except that
 * we take a DELAF entry into account, and that we don't have to take care of
 * the stack and variables.
 */
struct parsing_info* insert_morphological_match(int pos,int pos_in_token,int state,struct parsing_info* list,
                                                struct dela_entry* dic_entry) {
if (list==NULL) return new_parsing_info(pos,pos_in_token,state,-1,NULL,NULL,dic_entry,NULL,-1,-1);
if (list->position==pos && list->pos_in_token==pos_in_token && list->state_number==state
    && list->dic_entry==dic_entry) {
    /* If the morphological match is already in the list, we do nothing.
     * Note that this may occur when we don't take DELAF entries into account 
     * (i.e. dic_entry==NULL) */
   return list;
}
list->next=insert_morphological_match(pos,pos_in_token,state,list->next,dic_entry);
return list;
}

