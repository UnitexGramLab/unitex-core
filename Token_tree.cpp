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
#include <stdlib.h>
#include <stdio.h>

#include "Token_tree.h"
#include "Error.h"
//---------------------------------------------------------------------------


struct token_tree_node* new_token_tree_node() {
struct token_tree_node* n;
n=(struct token_tree_node*)malloc(sizeof(struct token_tree_node));
n->final=0;
n->trans=NULL;
return n;
}


void free_token_tree_node(struct token_tree_node* n) {
if (n==NULL) return;
free_token_tree_trans(n->trans);
free(n);
}


struct token_tree_trans* new_token_tree_trans() {
struct token_tree_trans* t;
t=(struct token_tree_trans*)malloc(sizeof(struct token_tree_trans));
t->token=-1;
t->arr=NULL;
t->suivant=NULL;
return t;
}


void free_token_tree_trans(struct token_tree_trans* t) {
struct token_tree_trans* tmp;
while (t!=NULL) {
   free_token_tree_node(t->arr);
   tmp=t;
   t=t->suivant;
   free(tmp);
}
}


struct token_tree_trans* get_token_tree_trans(int token,struct token_tree_trans* t) {
while (t!=NULL) {
  if (t->token==token) {
     return t;
  }
  t=t->suivant;
}
return NULL;
}


int was_allready_in_token_tree(int* token,int pos,struct token_tree_node* n,int priority) {
if (n==NULL) {
   fprintf(stderr,"Internal error in was_allready_in_token_tree\n");
   fatal_error(1);
}
if (token[pos]==-1) {
   // if we are at the end of the token
   int ret=n->final;
   // anyway, this token must be marked as present in the tree
   if (n->final==0) n->final=(char)priority;
   return ret;
}
struct token_tree_trans* t=get_token_tree_trans(token[pos],n->trans);
if (t==NULL) {
   // if the transition does not exist, we create it
   t=new_token_tree_trans();
   t->token=token[pos];
   t->arr=new_token_tree_node();
   t->suivant=n->trans;
   n->trans=t;
}
return was_allready_in_token_tree(token,pos+1,t->arr,priority);
}
