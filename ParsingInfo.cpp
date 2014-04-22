/*
 * Unitex
 *
 * Copyright (C) 2001-2014 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

size_t get_prefered_allocator_item_size_for_nb_variable(int nbvar)
{
    return AroundAllocAlign(sizeof(struct parsing_info)) + 
           AroundAllocAlign(get_expected_variable_backup_size_in_byte_for_nb_variable(nbvar)) +
           AroundAllocAlign(sizeof(unichar)*(SIZE_RESERVE_NB_UNICHAR_STACK_INSAMEALLOC+1));
}

size_t get_prefered_allocator_item_size_for_variable(Variables* v)
{
    return AroundAllocAlign(sizeof(struct parsing_info)) + 
           AroundAllocAlign(get_variable_backup_size_in_byte(v)) +
           AroundAllocAlign(sizeof(unichar)*(SIZE_RESERVE_NB_UNICHAR_STACK_INSAMEALLOC+1));
}

void update_parsing_info_stack(struct parsing_info*list,const unichar* new_stack_value)
{
   if (list->stack_must_be_free) {
       free(list->stack);
   }
   list->stack_must_be_free=0;
   list->stack = NULL;
   if (new_stack_value == NULL) {
       return;
   }
   list->stack = list->stack_internal_reserved_space;

   int stack_internal_reserved_space_size = list->stack_internal_reserved_space_size;
   unichar* stack_internal_reserved_space=list->stack_internal_reserved_space;
   int pos=0;
   for (;;)
   {
       unichar c=*(new_stack_value+pos);
       *((stack_internal_reserved_space)+pos)=c;
       if (c=='\0')
           return ;

       pos++;
       if (pos==stack_internal_reserved_space_size)
       {
           list->stack = u_strdup(new_stack_value);
           list->stack_must_be_free=1;
           return;
       }
   }
}

/**
 * Allocates, initializes and returns a new parsing info structure.
 */
struct parsing_info* new_parsing_info(int pos_in_tokens,int pos_in_chars,int state,int stack_pointer,unichar* stack,
                                      Variables* v,OutputVariables* output_var,struct dela_entry* dic_entry,
                                      struct dic_variable* v2,
                                      int left_ctx_shift,int left_ctx_base,unichar* jamo,int pos_int_jamo,
                                      vector_int* insertions,int weight,
                                      Abstract_allocator prv_alloc_recycle,Abstract_allocator prv_alloc_vector_int,Abstract_allocator prv_alloc_backup_growing_recycle) {
struct parsing_info* info;
unsigned char*buf;
buf=(unsigned char*)malloc_cb(get_prefered_allocator_item_size_for_variable(v),prv_alloc_recycle);
if (buf==NULL) {
   fatal_alloc_error("new_parsing_info");
}
info=(struct parsing_info*)buf;
info->weight=weight;
info->input_variable_backup = (int*)(buf + AroundAllocAlign(sizeof(struct parsing_info)));
info->stack_internal_reserved_space = (unichar*)(buf +
                                          AroundAllocAlign(sizeof(struct parsing_info)) +
                                          AroundAllocAlign(get_variable_backup_size_in_byte(v)));
info->stack_internal_reserved_space_size = SIZE_RESERVE_NB_UNICHAR_STACK_INSAMEALLOC;
/*
info=(struct parsing_info*)malloc_cb(sizeof(struct parsing_info),prv_alloc_recycle);
if (info==NULL) {
   fatal_alloc_error("new_parsing_info");
}*/

info->input_variable_backup_must_be_free=0;
info->stack_must_be_free=0;


info->pos_in_tokens=pos_in_tokens;
info->pos_in_chars=pos_in_chars;
info->state_number=state;
info->next=NULL;
info->stack_pointer=stack_pointer;
update_parsing_info_stack(info,stack);
//info->input_variable_backup=create_variable_backup(v,prv_alloc_recycle);
//info->input_variable_backup=create_variable_backup(v,prv_alloc_recycle);
init_variable_backup(info->input_variable_backup,v);
info->output_variable_backup=create_output_variable_backup(output_var,prv_alloc_backup_growing_recycle);
info->variable_backup_size=0;
if (v!=NULL)
  if (v->variable_index!=NULL)
      info->variable_backup_size=v->variable_index->size;
info->dic_entry=clone_dela_entry(dic_entry,prv_alloc_backup_growing_recycle);
info->dic_variable_backup=clone_dic_variable_list(v2);
info->left_ctx_shift=left_ctx_shift;
info->left_ctx_base=left_ctx_base;
info->jamo=jamo;
info->pos_in_jamo=pos_int_jamo;
info->insertions=NULL;
if (insertions!=NULL && insertions->nbelems!=0) {
	info->insertions=new_vector_int(insertions->nbelems, prv_alloc_vector_int);
	vector_int_copy(info->insertions,insertions, prv_alloc_vector_int);
} else {
	info->insertions=new_vector_int(1, prv_alloc_vector_int);
}
return info;
}


/**
 * Frees the whole memory associated to the given information list.
 */
void free_parsing_info(struct parsing_info* list,Abstract_allocator prv_alloc_recycle,Abstract_allocator prv_alloc_vector_int,Abstract_allocator prv_alloc_backup_growing_recycle) {
struct parsing_info* tmp;
while (list!=NULL) {
   tmp=list->next;
   if (list->input_variable_backup_must_be_free) {
       free_variable_backup(list->input_variable_backup,prv_alloc_recycle);
   }
   if (list->stack_must_be_free) {
       free(list->stack);
   }
   free_output_variable_backup(list->output_variable_backup,prv_alloc_backup_growing_recycle);
   clear_dic_variable_list(&(list->dic_variable_backup));
   free_dela_entry(list->dic_entry, prv_alloc_backup_growing_recycle);
   /* No free on list->jamo because it was only a pointer on the global jamo tag array */
   free_vector_int(list->insertions,prv_alloc_vector_int);
   free_cb(list,prv_alloc_recycle);
   list=tmp;
}
}


/**
 * Removes all elements that have a weight lesser than the given one.
 * Note that the list is supposed to be made of elements that all
 * have the same weight.
 */
static void filter_lesser_weights(int weight,struct parsing_info* *list,
									Abstract_allocator prv_alloc_recycle,Abstract_allocator prv_alloc_vector_int,Abstract_allocator prv_alloc_backup_growing_recycle) {
if (*list==NULL || (*list)->weight>=weight) return;
struct parsing_info* tmp;
while (*list!=NULL) {
	tmp=*list;
	(*list)=(*list)->next;
	tmp->next=NULL;
	free_parsing_info(tmp,prv_alloc_recycle, prv_alloc_vector_int,prv_alloc_backup_growing_recycle);
}
}


/**
 * Inserts an element in the given information list only if there is no element
 * with same end position of match.
 */
struct parsing_info* insert_if_absent(int pos,int pos_in_token,int state,struct parsing_info* list,int stack_pointer,
                                      unichar* stack,Variables* v,OutputVariables* output_var,
                                      struct dic_variable* v2,
                                      int left_ctx_shift,int left_ctx_base,unichar* jamo,int pos_in_jamo,
                                      vector_int* insertions,
                                      int weight,Abstract_allocator prv_alloc_recycle,Abstract_allocator prv_alloc_vector_int,Abstract_allocator prv_alloc_backup_growing_recycle) {
filter_lesser_weights(weight,&list,prv_alloc_recycle,prv_alloc_vector_int,prv_alloc_backup_growing_recycle);
if (list==NULL) return new_parsing_info(pos,pos_in_token,state,stack_pointer,stack,v,output_var,NULL,v2,
                                        left_ctx_shift,left_ctx_base,jamo,pos_in_jamo,insertions,
                                        weight,prv_alloc_recycle,prv_alloc_vector_int, prv_alloc_backup_growing_recycle);
if (list->pos_in_tokens==pos
	&& list->pos_in_chars==pos_in_token
	&& list->state_number==state
	&& list->jamo==jamo /* We can because we only work on pointers on unique elements */
	&& list->pos_in_jamo==pos_in_jamo
	&& same_input_variables(list->input_variable_backup,v)
	&& same_output_variables(list->output_variable_backup,output_var)
	&& same_dic_variables(list->dic_variable_backup,v2)) {
   list->stack_pointer=stack_pointer;
   /* We update the stack value */
   update_parsing_info_stack(list,stack);

   int v_variable_index_size=0;
   if (v!=NULL)
     if (v->variable_index!=NULL)
         v_variable_index_size=v->variable_index->size;

   if (list->variable_backup_size == v_variable_index_size) {
      update_variable_backup(list->input_variable_backup,v);
   }
   else {
      free_variable_backup(list->input_variable_backup,prv_alloc_recycle);
      list->input_variable_backup=create_variable_backup(v,prv_alloc_recycle);
      list->variable_backup_size=v_variable_index_size;
   }
   free_output_variable_backup(list->output_variable_backup,prv_alloc_backup_growing_recycle);
   list->output_variable_backup=create_output_variable_backup(output_var,prv_alloc_backup_growing_recycle);
   clear_dic_variable_list(&list->dic_variable_backup);
   list->dic_variable_backup=clone_dic_variable_list(v2);
   if (list->dic_entry!=NULL) {
      fatal_error("Unexpected non NULL dic_entry in insert_if_absent\n");
   }
   list->left_ctx_shift=left_ctx_shift;
   list->left_ctx_base=left_ctx_base;
   if (insertions!=NULL && insertions->nbelems!=0) {
	   if (list->insertions==NULL) {
		   list->insertions=new_vector_int(insertions->nbelems, prv_alloc_vector_int);
	   }
	   vector_int_copy(list->insertions,insertions, prv_alloc_vector_int);
   } else {
	   /* We always need such a vector, even empty */
	   if (list->insertions==NULL) list->insertions=new_vector_int(1, prv_alloc_vector_int);
   }
   return list;
}
list->next=insert_if_absent(pos,pos_in_token,state,list->next,stack_pointer,stack,v,output_var,v2,
                            left_ctx_shift,left_ctx_base,jamo,pos_in_jamo,insertions,weight,prv_alloc_recycle,prv_alloc_vector_int,prv_alloc_backup_growing_recycle);
return list;
}


/**
 * Inserts an element in the given information list only if there is no element
 * with position and same stack. */
struct parsing_info* insert_if_different(int pos,int pos_in_token,int state,struct parsing_info* list,int stack_pointer,
                                         unichar* stack,Variables* v,OutputVariables* output_var,
                                         struct dic_variable* v2,
                                         int left_ctx_shift,int left_ctx_base,
                                         unichar* jamo,int pos_in_jamo,
                                         vector_int* insertions,
                                         int weight,Abstract_allocator prv_alloc_recycle,Abstract_allocator prv_alloc_vector_int,Abstract_allocator prv_alloc_backup_growing_recycle) {
	filter_lesser_weights(weight,&list,prv_alloc_recycle,prv_alloc_vector_int,prv_alloc_backup_growing_recycle);
if (list==NULL) return new_parsing_info(pos,pos_in_token,state,stack_pointer,stack,v,output_var,NULL,v2,
                                        left_ctx_shift,left_ctx_base,jamo,pos_in_jamo,insertions,
                                        weight,prv_alloc_recycle,prv_alloc_vector_int,prv_alloc_backup_growing_recycle);
if ((list->pos_in_tokens==pos) /* If the length is the same... */
    && (list->pos_in_chars==pos_in_token)
    && (list->state_number==state)
    && !(u_strcmp(list->stack,stack)) /* ...and if the stack content too */
    && list->left_ctx_shift==left_ctx_shift
    && list->left_ctx_base==left_ctx_base
    && list->jamo==jamo /* See comment in insert_if_absent*/
    && list->pos_in_jamo==pos_in_jamo
    && same_input_variables(list->input_variable_backup,v)
	&& same_output_variables(list->output_variable_backup,output_var)
	&& same_dic_variables(list->dic_variable_backup,v2)) {
    /* then we overwrite the current list element */
   list->stack_pointer=stack_pointer;

   int v_variable_index_size=0;
   if (v!=NULL)
     if (v->variable_index!=NULL)
         v_variable_index_size=v->variable_index->size;

   if (list->variable_backup_size == v_variable_index_size) {
      update_variable_backup(list->input_variable_backup,v);
   }
   else {
      free_variable_backup(list->input_variable_backup,prv_alloc_recycle);
      list->input_variable_backup=create_variable_backup(v,prv_alloc_recycle);
      list->variable_backup_size=v_variable_index_size;
   }
   free_output_variable_backup(list->output_variable_backup,prv_alloc_backup_growing_recycle);
   list->output_variable_backup=create_output_variable_backup(output_var,prv_alloc_backup_growing_recycle);
   clear_dic_variable_list(&list->dic_variable_backup);
   list->dic_variable_backup=clone_dic_variable_list(v2);
   if (list->dic_entry!=NULL) {
      fatal_error("Unexpected non NULL dic_entry in insert_if_different\n");
   }
   if (insertions!=NULL && insertions->nbelems!=0) {
	   if (list->insertions==NULL) {
		   list->insertions=new_vector_int(insertions->nbelems, prv_alloc_vector_int);
	   }
	   vector_int_copy(list->insertions,insertions, prv_alloc_vector_int);
   }
   return list;
}
/* Otherwise, we look in the rest of the list */
list->next=insert_if_different(pos,pos_in_token,state,list->next,stack_pointer,stack,v,output_var,v2,
                               left_ctx_shift,left_ctx_base,jamo,pos_in_jamo,insertions,weight,prv_alloc_recycle,prv_alloc_vector_int,prv_alloc_backup_growing_recycle);
return list;
}


/**
 * This function behaves in the same way than 'insert_if_absent', except that
 * we take a DELAF entry into account, and that we don't have to take care of
 * the stack and variables.
 */
struct parsing_info* insert_morphological_match(int pos_in_tokens,int pos_in_chars,int state,struct parsing_info* list,
                                                struct dela_entry* dic_entry,unichar* jamo,int pos_in_jamo,
                                                Abstract_allocator prv_alloc_recycle,Abstract_allocator prv_alloc_vector_int,Abstract_allocator prv_alloc_backup_growing_recycle) {
if (list==NULL) return new_parsing_info(pos_in_tokens,pos_in_chars,state,-1,NULL,NULL,NULL,dic_entry,NULL,-1,-1,
		jamo,pos_in_jamo,NULL,-1,prv_alloc_recycle,prv_alloc_vector_int,prv_alloc_backup_growing_recycle);
if (list->pos_in_tokens==pos_in_tokens && list->pos_in_chars==pos_in_chars && list->state_number==state
    && list->dic_entry==dic_entry
    && list->jamo==jamo /* See comment in insert_if_absent*/
    && list->pos_in_jamo==pos_in_jamo) {
    /* If the morphological match is already in the list, we do nothing.
     * Note that this may occur when we don't take DELAF entries into account
     * (i.e. dic_entry==NULL) */
   return list;
}
list->next=insert_morphological_match(pos_in_tokens,pos_in_chars,state,list->next,dic_entry,jamo,pos_in_jamo,prv_alloc_recycle,prv_alloc_vector_int,prv_alloc_backup_growing_recycle);
return list;
}


// experimental no recursive code. To be check for performance and reliability
/*

struct parsing_info* insert_if_absent(int pos,int pos_in_token,int state,struct parsing_info* list,int stack_pointer,
                                      unichar* stack,Variables* v,OutputVariables* output_var,
                                      struct dic_variable* v2,
                                      int left_ctx_shift,int left_ctx_base,unichar* jamo,int pos_in_jamo,
                                      vector_int* insertions,
                                      int weight,Abstract_allocator prv_alloc_recycle,Abstract_allocator prv_alloc_vector_int) {
filter_lesser_weights(weight,&list,prv_alloc_recycle,prv_alloc_vector_int);

struct parsing_info**lnext=&list;
for (;;) {
  struct parsing_info*lcur=*lnext;

  if (lcur==NULL) {
	  *lnext=new_parsing_info(pos,pos_in_token,state,stack_pointer,stack,v,output_var,NULL,v2,
                                        left_ctx_shift,left_ctx_base,jamo,pos_in_jamo,insertions,
                                        weight,prv_alloc_recycle,prv_alloc_vector_int);
	  break;
  }
  if (lcur->position==pos && lcur->pos_in_token==pos_in_token && lcur->state_number==state
	&& lcur->jamo==jamo // We can because we only work on pointers on unique elements
	&& lcur->pos_in_jamo==pos_in_jamo) {
   lcur->stack_pointer=stack_pointer;
   // We update the stack value
   update_parsing_info_stack(lcur,stack);

   int v_variable_index_size=0;
   if (v!=NULL)
     if (v->variable_index!=NULL)
         v_variable_index_size=v->variable_index->size;

   if (lcur->variable_backup_size == v_variable_index_size) {
      update_variable_backup(lcur->input_variable_backup,v);
   }
   else {
      free_variable_backup(lcur->input_variable_backup,prv_alloc_recycle);
      lcur->input_variable_backup=create_variable_backup(v,prv_alloc_recycle);
      lcur->variable_backup_size=v_variable_index_size;
   }
   free_output_variable_backup(lcur->output_variable_backup);
   lcur->output_variable_backup=create_output_variable_backup(output_var);
   clear_dic_variable_list(&lcur->dic_variable_backup);
   lcur->dic_variable_backup=clone_dic_variable_list(v2);
   if (lcur->dic_entry!=NULL) {
      fatal_error("Unexpected non NULL dic_entry in insert_if_absent\n");
   }
   lcur->left_ctx_shift=left_ctx_shift;
   lcur->left_ctx_base=left_ctx_base;
   if (insertions!=NULL && insertions->nbelems!=0) {
	   if (lcur->insertions==NULL) {
		   lcur->insertions=new_vector_int(insertions->nbelems, prv_alloc_vector_int);
	   }
	   vector_int_copy(lcur->insertions,insertions, prv_alloc_vector_int);
   } else {
	   // We always need such a vector, even empty
	   if (lcur->insertions==NULL) lcur->insertions=new_vector_int(1, prv_alloc_vector_int);
   }
   break;
   }

   lnext=&(lcur->next);
}
return list;
}




struct parsing_info* insert_if_different(int pos,int pos_in_token,int state,struct parsing_info* list,int stack_pointer,
                                         unichar* stack,Variables* v,OutputVariables* output_var,
                                         struct dic_variable* v2,
                                         int left_ctx_shift,int left_ctx_base,
                                         unichar* jamo,int pos_in_jamo,
                                         vector_int* insertions,
                                         int weight,Abstract_allocator prv_alloc_recycle,Abstract_allocator prv_alloc_vector_int) {
	filter_lesser_weights(weight,&list,prv_alloc_recycle,prv_alloc_vector_int);

struct parsing_info**lnext=&list;
for (;;) {
  struct parsing_info*lcur=*lnext;
  if ((lcur)==NULL) {
	  *lnext=new_parsing_info(pos,pos_in_token,state,stack_pointer,stack,v,output_var,NULL,v2,
                                        left_ctx_shift,left_ctx_base,jamo,pos_in_jamo,insertions,
                                        weight,prv_alloc_recycle,prv_alloc_vector_int);
	  break;
  }

if ((lcur->position==pos) // If the length is the same...
    && (lcur->pos_in_token==pos_in_token)
    && (lcur->state_number==state)
    && !(u_strcmp(lcur->stack,stack)) // ...and if the stack content too
    && lcur->left_ctx_shift==left_ctx_shift
    && lcur->left_ctx_base==left_ctx_base
    && lcur->jamo==jamo // See comment in insert_if_absent
    && lcur->pos_in_jamo==pos_in_jamo) {
    // then we overwrite the current list element
   lcur->stack_pointer=stack_pointer;

   int v_variable_index_size=0;
   if (v!=NULL)
     if (v->variable_index!=NULL)
         v_variable_index_size=v->variable_index->size;

   if (lcur->variable_backup_size == v_variable_index_size) {
      update_variable_backup(lcur->input_variable_backup,v);
   }
   else {
      free_variable_backup(lcur->input_variable_backup,prv_alloc_recycle);
      lcur->input_variable_backup=create_variable_backup(v,prv_alloc_recycle);
      lcur->variable_backup_size=v_variable_index_size;
   }
   free_output_variable_backup(lcur->output_variable_backup);
   lcur->output_variable_backup=create_output_variable_backup(output_var);
   clear_dic_variable_list(&lcur->dic_variable_backup);
   lcur->dic_variable_backup=clone_dic_variable_list(v2);
   if (lcur->dic_entry!=NULL) {
      fatal_error("Unexpected non NULL dic_entry in insert_if_different\n");
   }
   if (insertions!=NULL && insertions->nbelems!=0) {
	   if (lcur->insertions==NULL) {
		   lcur->insertions=new_vector_int(insertions->nbelems, prv_alloc_vector_int);
	   }
	   vector_int_copy(list->insertions,insertions, prv_alloc_vector_int);
   }
   break;
}
lnext=&(lcur->next);
}
// Otherwise, we look in the rest of the list
return list;
}




												
struct parsing_info* insert_morphological_match(int pos,int pos_in_token,int state,struct parsing_info* list,
                                                struct dela_entry* dic_entry,unichar* jamo,int pos_in_jamo,
                                                Abstract_allocator prv_alloc_recycle,Abstract_allocator prv_alloc_vector_int) {
struct parsing_info**lnext=&list;
for (;;) {
  struct parsing_info*lcur=*lnext;
  if ((lcur)==NULL) {
		*lnext=new_parsing_info(pos,pos_in_token,state,-1,NULL,NULL,NULL,dic_entry,NULL,-1,-1,
		  jamo,pos_in_jamo,NULL,-1,prv_alloc_recycle,prv_alloc_vector_int);
		break;
  }
  if (lcur->position==pos && lcur->pos_in_token==pos_in_token && lcur->state_number==state
    && lcur->dic_entry==dic_entry
    && lcur->jamo==jamo // See comment in insert_if_absent
    && lcur->pos_in_jamo==pos_in_jamo) {
    // If the morphological match is already in the list, we do nothing.
    // Note that this may occur when we don't take DELAF entries into account
    // (i.e. dic_entry==NULL)
   break;   
  }
  lnext=&(lcur->next);
}
return list;
}

*/
} // namespace unitex
