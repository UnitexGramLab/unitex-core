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

#include "PatternTree.h"
#include "Error.h"
#include "BitArray.h"


/* This value is used for the initialization of constraint list cells */
#define UNDEFINED_PATTERN -1


void free_constraint_list(struct constraint_list*,Abstract_allocator prv_alloc);
void free_pattern_node_transition(struct pattern_node_transition*,Abstract_allocator prv_alloc);



/**
 * Allocates, initializes and returns a new pattern node.
 */
struct pattern_node* new_pattern_node(Abstract_allocator prv_alloc) {
struct pattern_node* n=(struct pattern_node*)malloc_cb(sizeof(struct pattern_node),prv_alloc);
if (n==NULL) {
   fatal_alloc_error("new_pattern_node");
}
n->constraints=NULL;
n->sons=NULL;
return n;
}


/**
 * Frees the whole memory associated to a pattern node.
 */
void free_pattern_node(struct pattern_node* node,Abstract_allocator prv_alloc) {
if (node==NULL) return;
free_constraint_list(node->constraints,prv_alloc);
free_pattern_node_transition(node->sons,prv_alloc);
free_cb(node,prv_alloc);
}


/**
 * Allocates, initializes and returns a new pattern node transition list.
 */
struct pattern_node_transition* new_pattern_node_transition(unichar* grammatical_code,
                                                      struct pattern_node* node,
                                                      struct pattern_node_transition* next,
                                                      Abstract_allocator prv_alloc) {
if (grammatical_code==NULL) {
   fatal_error("NULL grammatical/semantic code in new_pattern_node_transition\n");
}
struct pattern_node_transition* list=(struct pattern_node_transition*)malloc_cb(sizeof(struct pattern_node_transition),prv_alloc);
if (list==NULL) {
   fatal_alloc_error("new_pattern_node_transition");
}
list->grammatical_code=u_strdup(grammatical_code,prv_alloc);
list->node=node;
list->next=next;
return list;
}


/**
 * Frees the whole memory associated to a pattern node list.
 */
void free_pattern_node_transition(struct pattern_node_transition* list,Abstract_allocator prv_alloc) {
struct pattern_node_transition* tmp;
while (list!=NULL) {
   free_pattern_node(list->node,prv_alloc);
   free_cb(list->grammatical_code,prv_alloc);
   tmp=list;
   list=list->next;
   free_cb(tmp,prv_alloc);
}
}


/**
 * This function allocates a new constraint list cell and initializes it
 * from the given pattern.
 */
struct constraint_list* new_constraint_list(struct pattern* pattern,
                                            struct constraint_list* next,
                                            Abstract_allocator prv_alloc) {
if (pattern==NULL) {
   fatal_error("NULL pattern in new_constraint_list\n");
}
struct constraint_list* cell=(struct constraint_list*)malloc_cb(sizeof(struct constraint_list),prv_alloc);
if (cell==NULL) {
   fatal_alloc_error("new_constraint_list");
}
cell->inflected=u_strdup(pattern->inflected,prv_alloc);
cell->lemma=u_strdup(pattern->lemma,prv_alloc);
cell->forbidden_codes=clone(pattern->forbidden_codes,prv_alloc);
cell->inflectional_codes=clone(pattern->inflectional_codes,prv_alloc);
cell->pattern_number=UNDEFINED_PATTERN;
cell->next=next;
return cell;
}


/**
 * Frees the whole memory associated to the given constraint list.
 */
void free_constraint_list(struct constraint_list* list,Abstract_allocator prv_alloc) {
struct constraint_list* tmp;
while (list!=NULL) {
   if (list->inflected!=NULL) free_cb(list->inflected,prv_alloc);
   if (list->lemma!=NULL) free_cb(list->lemma,prv_alloc);
   free_list_ustring(list->forbidden_codes,prv_alloc);
   free_list_ustring(list->inflectional_codes,prv_alloc);
   tmp=list;
   list=list->next;
   free_cb(tmp,prv_alloc);
}
}


/**
 * This function explores a pattern tree in order to find the node that 
 * corresponds to the given gramatical codes, creating it if needed. Note
 * that the grammatical codes are supposed to be sorted.
 */
struct pattern_node* get_pattern_node(struct pattern_node* node,
                                      struct list_ustring* grammatical_nodes,
                                      Abstract_allocator prv_alloc) {
if (node==NULL) {
   fatal_error("NULL error in get_pattern_node\n");
}
if (grammatical_nodes==NULL) {
   /* If we are in the good node */
   return node;
}
struct pattern_node_transition* sons=node->sons;
while (sons!=NULL) {
   if (!u_strcmp(sons->grammatical_code,grammatical_nodes->string)) {
      /* If we have found the node with the correct code */
      node=sons->node;
      break;
   }
   sons=sons->next;
}
if (sons==NULL) {
   /* If we have not found the node, we must create it */
   struct pattern_node* new_node=new_pattern_node(prv_alloc);
   node->sons=new_pattern_node_transition(grammatical_nodes->string,new_node,node->sons,prv_alloc);
   node=new_node;
}
return get_pattern_node(node,grammatical_nodes->next,prv_alloc);
}


/**
 * This functions returns 1 if the given pattern matches the given constraints;
 * 0 otherwise. The fucntion assumes that both parameters are non NULL.
 */
int pattern_equals_to_constraints(struct pattern* pattern,struct constraint_list* constraints) {
if (u_strcmp(pattern->inflected,constraints->inflected)) return 0;
if (u_strcmp(pattern->lemma,constraints->lemma)) return 0;
if (!equal(pattern->forbidden_codes,constraints->forbidden_codes)) return 0;
if (!equal(pattern->inflectional_codes,constraints->inflectional_codes)) return 0;
return 1;
}


/**
 * This function looks for a set of contraints that corresponds to the given pattern.
 * If any, it returns the constraint set. Otherwise, a new constraint set is created,
 * inserted at the head of the list and returned.
 */
struct constraint_list* get_constraints(struct constraint_list** list,struct pattern* pattern,Abstract_allocator prv_alloc) {
struct constraint_list* tmp=(*list);
while (tmp!=NULL) {
   if (pattern_equals_to_constraints(pattern,tmp)) {
      return tmp;
   }
   tmp=tmp->next;
}
(*list)=new_constraint_list(pattern,*list,prv_alloc);
return (*list);
}


/**
 * This function adds the given pattern to the given pattern tree node,
 * if not already present. If the pattern is not in the tree, the function
 * adds it with the number '*pattern_number' and increases '*pattern_number'.
 * In any case, the number associated to the pattern is returned.
 * Note that this function only tolerates patterns with
 * grammatical/semantic/inflectional constraints.
 */
int add_pattern(int *pattern_number,struct pattern* pattern,struct pattern_node* root,Abstract_allocator prv_alloc) {
if (root==NULL) {
   fatal_error("NULL root in add_pattern\n");
}
if (pattern==NULL) {
   fatal_error("NULL pattern in add_pattern\n");
}
if (pattern->type!=CODE_PATTERN && pattern->type!=LEMMA_AND_CODE_PATTERN
    && pattern->type!=FULL_PATTERN && pattern->type!=INFLECTED_AND_LEMMA_PATTERN) {
   fatal_error("Invalid pattern type %d in add_pattern\n",pattern->type);
}
struct pattern_node* node=get_pattern_node(root,pattern->grammatical_codes,prv_alloc);
struct constraint_list* constraints=get_constraints(&(node->constraints),pattern,prv_alloc);
if (constraints->pattern_number==UNDEFINED_PATTERN) {
   constraints->pattern_number=(*pattern_number);
   (*pattern_number)++;
}
return constraints->pattern_number;
}


/**
 * This function returns 1 if the string s is found in the string array t;
 * 0 otherwise.
 */
int contains(unichar* s,unichar** t,int size) {
if (s==NULL) {
   fatal_error("NULL string in contains\n");
}
if (t==NULL) {
   fatal_error("NULL array in contains\n");
}
if (size==0) {
   fatal_error("Empty array in contains\n");
}
for (int i=0;i<size;i++) {
   if (!u_strcmp(s,t[i])) {
      return 1;
   }
}
return 0;
}


/**
 * This function returns 1 if the given set contains the given subset;
 * 0 otherwise.
 */
int contains_subset(unichar* set,unichar* subset) {
if (set==NULL) {
   fatal_error("NULL set in contains_subset\n");
}
if (subset==NULL) {
   fatal_error("NULL subset in contains_subset\n");
}
for (int i=0;subset[i]!='\0';i++) {
   if (u_strchr(set,subset[i])==NULL) return 0;
}
return 1;
}


/**
 * The function returns 1 if the contraint set can match the entry; 0
 * otherwise.
 */
int test_constraint_set(struct dela_entry* entry,
                        struct constraint_list* constraints) {
/* If there is a constraint on the inflected form, we test it */
if (constraints->inflected!=NULL) {
   if (u_strcmp(entry->inflected,constraints->inflected)) return 0;
}
/* If there is a constraint on the lemma, we test it */
if (constraints->lemma!=NULL) {
   if (u_strcmp(entry->lemma,constraints->lemma)) return 0;
}
/* Then, we verify that the entry contains no code that is forbidden
 * by the constraint set */
struct list_ustring* tmp=constraints->forbidden_codes;
while (tmp!=NULL) {
   if (contains(tmp->string,entry->semantic_codes,entry->n_semantic_codes)) return 0;
   tmp=tmp->next;
}
/* Finally, we must check if the inflectional codes of the entry
 * matches the ones of the constraint set */
tmp=constraints->inflectional_codes;
/* If there is no constraint about inflectional codes, we have finished */
if (tmp==NULL) {
   return 1;
}
/* Otherwise, we must test for each inflectional code of the constraint set
 * if it matches at least one inflectional code of the entry */
while (tmp!=NULL) {
   for (int i=0;i<entry->n_inflectional_codes;i++) {
      if (contains_subset(entry->inflectional_codes[i],tmp->string)) {
         return 1;
      }
   }
   tmp=tmp->next;
}
return 0;
}


/**
 * This function takes a contraint set list and check for each of them
 * if the given entry matches it. The function returns the list of
 * constraint set that match the entry.
 */
struct list_pointer* test_constraints(struct dela_entry* entry,
                                      struct constraint_list* constraints,
                                      struct list_pointer* result,
                                      Abstract_allocator prv_alloc) {
while (constraints!=NULL) {
   if (test_constraint_set(entry,constraints)) {
      result=new_list_pointer(constraints,result,prv_alloc);
   }
   constraints=constraints->next;
}
return result;
}


/**
 * This function computes the list of all the patterns like <A:ms> that can
 * match the given entry. The function returns a list that contains
 * the constraint set that match the entry.
 */
struct list_pointer* get_matching_patterns(struct dela_entry* entry,
                                           struct pattern_node* node,
                                           struct list_pointer* result,
                                           Abstract_allocator prv_alloc) {
if (node==NULL) {
   fatal_error("NULL node in get_matching_patterns\n");
}
result=test_constraints(entry,node->constraints,result,prv_alloc);
struct pattern_node_transition* tmp=node->sons;
while (tmp!=NULL) {
   if (contains(tmp->grammatical_code,entry->semantic_codes,entry->n_semantic_codes)) {
      result=get_matching_patterns(entry,tmp->node,result,prv_alloc);
   }
   tmp=tmp->next;
}
return result;
}


/**
 * This function computes the list of all the patterns like <A:ms> that can
 * match the given entry. The function returns a list that contains
 * the constraint set that match the entry.
 */
struct list_pointer* get_matching_patterns(struct dela_entry* entry,
                                           struct pattern_node* root,
                                           Abstract_allocator prv_alloc) {
return get_matching_patterns(entry,root,NULL,prv_alloc);
}
