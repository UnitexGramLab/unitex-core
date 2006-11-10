 /*
  * Unitex
  *
  * Copyright (C) 2001-2006 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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

//---------------------------------------------------------------------------
#include "DictionaryTree.h"
#include "Error.h"
//---------------------------------------------------------------------------


/**
 * Allocates, initializes and returns a dictionary node.
 */
struct dictionary_node* new_dictionary_node() {
struct dictionary_node* a=(struct dictionary_node*)malloc(sizeof(struct dictionary_node));
if (a==NULL) {
	fatal_error("Not enough memory in new_dictionary_node\n");
}
a->single_INF_code_list=NULL;
a->offset=-1;
a->trans=NULL;
a->incoming=0;
return a;
}


/**
 * Allocates, initializes and returns a dictionary node transition.
 */
struct dictionary_node_transition* new_dictionary_node_transition() {
struct dictionary_node_transition* t=(struct dictionary_node_transition*)malloc(sizeof(struct dictionary_node_transition));
if (t==NULL) {
	fatal_error("Not enough memory in new_arbre_dico_trans\n");
}
t->letter='\0';
t->node=NULL;
t->next=NULL;
return t;
}


/**
 * Frees all the dictionary graph whose root is 'a'.
 */
void free_dictionary_node(struct dictionary_node* a) {
if (a==NULL) return;
(a->incoming)--;
if (a->incoming!=0) {
	/* We don't free a state that is still pointed by someone else 
	 * in order to avoid double freeing problems. */
	return;
}
free_list_int(a->single_INF_code_list);
free_dictionary_node_transition(a->trans);
free(a);
}


/**
 * Frees the dictionary node transition 't', and all the dictionary graph whose root is 
 * the dictionary node pointed out by 't'.
 */
void free_dictionary_node_transition(struct dictionary_node_transition* t) {
struct dictionary_node_transition* tmp;
while (t!=NULL) {
   free_dictionary_node(t->node);
   tmp=t;
   t=t->next;
   free(tmp);
}
}


/**
 * This function looks for a transition of the dictionary node '*n' that is 
 * tagged with 'c'. If the transition exists, it is returned. Otherwise, the
 * transition is created, inserted according to the Unicode order and returned.
 * In that case, the destination dictionary node is NULL. 
 */
struct dictionary_node_transition* get_transition(unichar c,struct dictionary_node** n) {
struct dictionary_node_transition* tmp;
if ((*n)->trans==NULL || c<((*n)->trans->letter)) {
   /* If we must insert at first position */
   tmp=new_dictionary_node_transition();
   tmp->letter=c;
   tmp->next=(*n)->trans;
   tmp->node=NULL;
   (*n)->trans=tmp;
   return tmp;
}
if (c==((*n)->trans->letter)) {
   /* If the transition exists, we return it */
   return (*n)->trans;
}
/* Otherwise, we look for the transition, or the correct
 * place to insert it */
tmp=(*n)->trans;
while (tmp->next!=NULL && c>tmp->next->letter) {
   tmp=tmp->next;
}
if (tmp->next==NULL || (tmp->next->letter)>c) {
   /* If we must insert between tmp and tmp->next */
   struct dictionary_node_transition* ptr;
   ptr=new_dictionary_node_transition();
   ptr->letter=c;
   ptr->next=tmp->next;
   ptr->node=NULL;
   tmp->next=ptr;
   return ptr;
}
/* If we have (tmp->next->c == c) */
return tmp->next;
}


/**
 * This structure is used to pass several constant parameters to the function
 * 'add_entry_to_dictionary_tree'.
 */
struct info {
	unichar* INF_code;
	struct string_hash* INF_code_list;
};


/**
 * This function explores a dictionary tree in order to insert an entry.
 * 'inflected' is the inflected form to insert, and 'pos' is the current position
 * in the string 'inflected'. 'node' is the current node in the dictionary tree.
 * 'infos' is used to access to constant parameters.
 */
void add_entry_to_dictionary_tree(unichar* inflected,int pos,struct dictionary_node* node,
                                  struct info* infos) {
if (inflected[pos]=='\0') {
   /* If we have reached the end of 'inflected', then we are in the
    * node where the INF code must be inserted */
   int N=get_value_index(infos->INF_code,infos->INF_code_list);
   if (node->single_INF_code_list==NULL) {
      /* If there is no INF code in the node, then
       * we add one and we return */
      node->single_INF_code_list=new_list_int(N);
      node->INF_code=N;
      return;
   }
   /* If there is an INF code list in the node ...*/
   if (is_in_list(N,node->single_INF_code_list)) {
      /* If the INF code has allready been taken into account for this node
       * (case of duplicates), we do nothing */
      return;
   }
   /* Otherwise, we add it to the INF code list */
   node->single_INF_code_list=head_insert(N,node->single_INF_code_list);
   /* And we update the global INF line for this node */
   unichar tmp[3000];
   u_strcpy(tmp,infos->INF_code_list->value[node->INF_code]);
   u_strcat_char(tmp,",");
   u_strcat(tmp,infos->INF_code);
   node->INF_code=get_value_index(tmp,infos->INF_code_list);
   return;
}
/* If we are not at the end of 'inflected', then we look for
 * the correct outgoing transition and we follow it */
struct dictionary_node_transition* t=get_transition(inflected[pos],&node);
if (t->node==NULL) {
   /* We create the node if necessary */
   t->node=new_dictionary_node();
   (t->node->incoming)++;
}
add_entry_to_dictionary_tree(inflected,pos+1,t->node,infos);
}


/**
 * This function inserts an entry in a dictionary tree. 'inflected' represents
 * the inflected form of the entry, and 'INF_code' represents the compressed line
 * that will be used for rebuilding the whole DELAF line. 'root' is the root
 * of the dictionary tree and 'INF_code_list' is the structure that contains
 * all the INF codes that are used in the given dictionary tree.
 */
void add_entry_to_dictionary_tree(unichar* inflected,unichar* INF_code,
                                  struct dictionary_node* root,struct string_hash* INF_code_list) {
struct info infos;
infos.INF_code=INF_code;
infos.INF_code_list=INF_code_list;
add_entry_to_dictionary_tree(inflected,0,root,&infos);
}

