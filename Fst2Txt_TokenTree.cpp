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

#include "Fst2Txt_TokenTree.h"
#include "Error.h"


#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * Allocates, initializes and returns a new token tree.
 */
struct fst2txt_token_tree* new_fst2txt_token_tree(Abstract_allocator prv_alloc) {
struct fst2txt_token_tree* t=(struct fst2txt_token_tree*)malloc_cb(sizeof(struct fst2txt_token_tree),prv_alloc);
if (t==NULL) {
   fatal_alloc_error("new_fst2txt_token_tree");
}
t->hash=new_string_hash(DONT_USE_VALUES);
/* We set a small default capacity since there will be one structure of
 * this kind for each state of the fst2 */
t->capacity=2;
t->size=0;
t->transition_array=(Transition**)malloc_cb(t->capacity*sizeof(Transition*), prv_alloc);
if (t->transition_array==NULL) {
   fatal_alloc_error("new_fst2txt_token_tree");
}
return t;
}


/**
 * Frees all the memory asociated to the given token tree structure.
 */
void free_fst2txt_token_tree(struct fst2txt_token_tree* t, Abstract_allocator prv_alloc) {
if (t==NULL) return;
free_string_hash(t->hash);
for (int i=0;i<t->size;i++) {
   free_Transition_list(t->transition_array[i], prv_alloc);
}
free_cb(t->transition_array,prv_alloc);
free_cb(t,prv_alloc);
}


/**
 * This function adds the given token to the given token tree, if not already
 * present. Then, it adds the given transition to its transition list. 
 */
void add_tag(unichar* token,int tag_number,int dest_state,struct fst2txt_token_tree* tree, Abstract_allocator prv_alloc) {
int n=get_value_index(token,tree->hash);
if (n==tree->size) {
   /* If we have to create a new transition list because the token was not already in
    * the tree. */
   if (tree->size==tree->capacity) {
      /* If necessary, we double the size of the transition array */
      tree->capacity=2*tree->capacity;
      tree->transition_array=(Transition**)realloc_cb(tree->transition_array,(tree->capacity/2)*sizeof(Transition*),tree->capacity*sizeof(Transition*),prv_alloc);
      if (tree->transition_array==NULL) {
         fatal_alloc_error("add_tag");
      }
   }
   (tree->size)++;
   /* We don't forget to initialize the new transition list */
   tree->transition_array[n]=NULL;
}
/* We add the new transition, assuming that it is not already in the list, becauses
 * it would mean that the fst2 is not deterministic. */
tree->transition_array[n]=new_Transition(tag_number,dest_state,tree->transition_array[n],prv_alloc);
}


/**
 * This function explores a token tree, comparing it with the given token in order to
 * find out the tokens that match the text token, and then, to add to corresponding
 * transition to the result.
 */
void explore_token_tree(unichar* token,int pos,struct string_hash_tree_node* node,Alphabet* alphabet,
                        int max_pos,Transition** result,struct fst2txt_token_tree* tree, Abstract_allocator prv_alloc) {
if (node==NULL) {
   return;
}
if (token[pos]=='\0' && node->value_index!=-1) {
   /* If we are at the end of the word and if there is an associated
    * transition list */
   add_transitions_int(tree->transition_array[node->value_index],result,prv_alloc);
   return;
}
struct string_hash_tree_transition* trans=node->trans;
while (trans!=NULL) {
   if (is_equal_or_uppercase(trans->letter,token[pos],alphabet)) {
      /* If the transition can be followed */
      explore_token_tree(token,pos+1,trans->node,alphabet,max_pos,result,tree,prv_alloc);
   }
   trans=trans->next;
}
}


/**
 * This function takes a token and a token tree. It returns the list of transitions
 * that can be matched by this token tree.
 */
Transition* get_matching_tags(unichar* token,struct fst2txt_token_tree* tree,
                                 Alphabet* alphabet, Abstract_allocator prv_alloc) {
Transition* list=NULL;
explore_token_tree(token,0,tree->hash->root,alphabet,0,&list,tree,prv_alloc);
return list;
}

} // namespace unitex
