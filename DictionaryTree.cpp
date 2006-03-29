 /*
  * Unitex
  *
  * Copyright (C) 2001-2005 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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
//---------------------------------------------------------------------------


struct liste_nombres* new_liste_nbre() {
struct liste_nombres* l=(struct liste_nombres*)malloc(sizeof(struct liste_nombres));
l->n=-1;
l->suivant=NULL;
return l;
}


struct dictionary_node* new_arbre_dico() {
struct dictionary_node* a=(struct dictionary_node*)malloc(sizeof(struct dictionary_node));
a->single_INF_code_list=NULL;
a->offset=-1;
a->trans=NULL;
return a;
}


struct dictionary_tree_transition* new_arbre_dico_trans() {
struct dictionary_tree_transition* t=(struct dictionary_tree_transition*)malloc(sizeof(struct dictionary_tree_transition));
t->letter='\0';
t->node=NULL;
t->next=NULL;
return t;
}



void free_arbre_dico(struct dictionary_node* a) {
if (a==NULL) return;
free_liste_nombres(a->single_INF_code_list);
free_arbre_dico_trans(a->trans);
free(a);
}


void free_arbre_dico_non_rec(struct dictionary_node* a) {
  if (a==NULL)
    return;
  free_liste_nombres(a->single_INF_code_list);
  struct dictionary_tree_transition* t;
  struct dictionary_tree_transition* tmp;
  t = a->trans;
  while (t!=NULL) {
    tmp=t;
    t=t->next;
    free(tmp);
  }
  free(a);
}




void free_arbre_dico_trans(struct dictionary_tree_transition* t) {
struct dictionary_tree_transition* tmp;
while (t!=NULL) {
       free_arbre_dico(t->node);
      tmp=t;
      t=t->next;
      free(tmp);
}
}



struct dictionary_tree_transition* get_transition(unichar c,struct dictionary_node** n) {
struct dictionary_tree_transition* tmp;
if ((*n)->trans==NULL || c<((*n)->trans->letter)) {
   // if we must insert at first position
   tmp=new_arbre_dico_trans();
   tmp->letter=c;
   tmp->next=(*n)->trans;
   tmp->node=NULL;
   (*n)->trans=tmp;
   return tmp;
}
if (c==((*n)->trans->letter)) {
   // nothing to do
   return (*n)->trans;
}
tmp=(*n)->trans;
while (tmp->next!=NULL && c>tmp->next->letter) {
   tmp=tmp->next;
}
if (tmp->next==NULL || (tmp->next->letter)>c) {
   // if we must insert between tmp and tmp->suivant
   struct dictionary_tree_transition* ptr;
   ptr=new_arbre_dico_trans();
   ptr->letter=c;
   ptr->next=tmp->next;
   ptr->node=NULL;
   tmp->next=ptr;
   return ptr;
}
// there we have (tmp->suivant->c == c)
return tmp->next;
}



int is_in_list(int n,struct liste_nombres* l) {
while (l!=NULL) {
   if (l->n==n) return 1;
   l=l->suivant;
}
return 0;
}



int OK=0;
unichar* compressed;
struct string_hash* hash;

void explorer_arbre_dico(unichar* contenu,int pos,struct dictionary_node* noeud) {
if (contenu[pos]=='\0') {
   unichar tmp[3000];
   int N=get_hash_number(compressed,hash);
   if (noeud->single_INF_code_list==NULL) {
      // if there is no node
      noeud->single_INF_code_list=new_liste_nbre();
      noeud->single_INF_code_list->n=N;
      noeud->INF_code=N;
      return;
   }
   if (is_in_list(N,noeud->single_INF_code_list)) {
      // if the compressed string has allready been taken into account for this node
      // (case of duplicates), we do nothing
      return;
   }
   struct liste_nombres* l_tmp=new_liste_nbre();
   l_tmp->n=N;
   l_tmp->suivant=noeud->single_INF_code_list;
   noeud->single_INF_code_list=l_tmp;
   u_strcpy(tmp,hash->tab[noeud->INF_code]);
   u_strcat_char(tmp,",");
   u_strcat(tmp,compressed);
   noeud->INF_code=get_hash_number(tmp,hash);
   return;
}
struct dictionary_tree_transition* t=get_transition(contenu[pos],&noeud);
if (t->node==NULL) {
  t->node=new_arbre_dico();
}
explorer_arbre_dico(contenu,pos+1,t->node);
}



void inserer_entree(unichar* contenu,unichar* compressed_,struct dictionary_node* noeud,struct string_hash* hash_) {
compressed=compressed_;
hash=hash_;
explorer_arbre_dico(contenu,0,noeud);
}

