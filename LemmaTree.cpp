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

#include <stdio.h>
#include <stdlib.h>
#include "LemmaTree.h"
#include "Error.h"


void free_lemma_node_list(struct lemma_node_list*);


/**
 * Allocates, initializes and returns a new lemma_node.
 */
struct lemma_node* new_lemma_node() {
struct lemma_node* n;
n=(struct lemma_node*)malloc(sizeof(struct lemma_node));
if (n==NULL) {
   fatal_error("Not enough memory in new_lemma_node\n");
}
n->inflected_forms=NULL;
/* We initialize the letter field with 1 in order to avoid
 * bad surprise in the root */
n->letter=1;
n->sons=NULL;
return n;
}


/**
 * Frees the memory used by the whole given lemma tree.
 */
void free_lemma_node(struct lemma_node* node) {
if (node==NULL) return;
if (node->inflected_forms!=NULL) {
   free(node->inflected_forms);
}
free_lemma_node_list(node->sons);
free(node);
}


/**
 * Allocates, initializes and returns a new lemma_node_list.
 */
struct lemma_node_list* new_lemma_node_list(struct lemma_node_list* list,
                                            struct lemma_node* node) {
struct lemma_node_list* l;
l=(struct lemma_node_list*)malloc(sizeof(struct lemma_node_list));
if (l==NULL) {
   fatal_error("Not enough memory in new_lemma_node_list\n");
}
l->node=node;
l->next=list;
return l;
}


/**
 * Frees the memory used by the whole given lemma node list.
 */
void free_lemma_node_list(struct lemma_node_list* list) {
struct lemma_node_list* tmp;
while (list!=NULL) {
   tmp=list;
   list=list->next;
   free_lemma_node(tmp->node);
   free(tmp);
}
}


/**
 * This function takes a node in the lemma and a letter. It returns the destination
 * node that corresponds to this letter. If 'create_if_needed' is non null, then
 * the node will created if it does not exist. Otherwise, the function will return
 * NULL;
 */
struct lemma_node* get_lemma_node(struct lemma_node* node,unichar letter,int create_if_needed) {
struct lemma_node_list* ptr;
struct lemma_node* result;
if (node==NULL) {
   fatal_error("NULL error in get_node\n");
}
ptr=node->sons;
/* First, we look for the node */
while (ptr!=NULL) {
   if (ptr->node==NULL) {
      fatal_error("NULL error in get_node\n");
   }
   if ((ptr->node)->letter==letter) {
      break;
   }
   ptr=ptr->next;
}
if (ptr!=NULL) {
   /* If the node exists, we return it */
   return ptr->node;
}
/* Otherwise, we return NULL if we just wanted to know about its existence */
if (!create_if_needed) {
   return NULL;
}
/* Here, the node must be created, added to the node list of the current node
 * and returned */
result=new_lemma_node();
result->letter=letter;
node->sons=new_lemma_node_list(node->sons,result);
return result;
}


/**
 * This function adds the given inflected form to the list of inflected forms
 * associated to the given lemma. 'pos' is the current position in the inflected
 * form, and 'node' is the current node in the tree that contains the lemma.
 */
void add_inflected_form_for_lemma(unichar* lemma,int pos,struct lemma_node* node,unichar* inflected) {
struct lemma_node* dest_node;
dest_node=get_lemma_node(node,lemma[pos],1);
pos++;
if (lemma[pos]=='\0') {
   /* We add the inflected form to the list */
   dest_node->inflected_forms=sorted_insert(inflected,dest_node->inflected_forms);
}
else add_inflected_form_for_lemma(lemma,pos,dest_node,inflected);
}


/**
 * This function adds the given inflected form to the list of inflected forms
 * associated to the given lemma. 'root' is the tree that contains the lemma.
 */
void add_inflected_form_for_lemma(unichar* inflected,unichar* lemma,struct lemma_node* root) {
if (inflected==NULL || lemma==NULL) {
   fatal_error("NULL error in add_inflected_form_for_lemma\n");
}
if (inflected[0]=='\0') {
   fatal_error("Empty inflected form in add_inflected_form_for_lemma\n");
}
if (lemma[0]=='\0') {
   fatal_error("Empty lemma in add_inflected_form_for_lemma\n");
}
add_inflected_form_for_lemma(lemma,0,root,inflected);
}


/**
 * Returns the list of inflected form associated to the given lemma.
 */
struct list_ustring* get_inflected_forms(unichar* lemma,int pos,struct lemma_node* node) {
struct lemma_node* dest_node;
dest_node=get_lemma_node(node,lemma[pos],0);
if (dest_node==NULL) {
   return NULL;
}
pos++;
if (lemma[pos]=='\0') {
   return dest_node->inflected_forms;
}
else return get_inflected_forms(lemma,pos,dest_node);
}


/**
 * Returns the list of inflected form associated to the given lemma.
 */
struct list_ustring* get_inflected_forms(unichar* lemma,struct lemma_node* root) {
return get_inflected_forms(lemma,0,root);
}
