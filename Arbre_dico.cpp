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
#include "Arbre_dico.h"
//---------------------------------------------------------------------------


struct liste_nbre* new_liste_nbre() {
struct liste_nbre* l=(struct liste_nbre*)malloc(sizeof(struct liste_nbre));
l->n=-1;
l->suivant=NULL;
return l;
}


struct arbre_dico* new_arbre_dico() {
struct arbre_dico* a=(struct arbre_dico*)malloc(sizeof(struct arbre_dico));
a->arr=NULL;
a->offset=-1;
a->trans=NULL;
return a;
}


struct arbre_dico_trans* new_arbre_dico_trans() {
struct arbre_dico_trans* t=(struct arbre_dico_trans*)malloc(sizeof(struct arbre_dico_trans));
t->c='\0';
t->noeud=NULL;
t->suivant=NULL;
return t;
}


void free_liste_nbre(struct liste_nbre* l) {
struct liste_nbre* tmp;
while (l!=NULL) {
      tmp=l;
      l=l->suivant;
      free(tmp);
}
}


void free_arbre_dico(struct arbre_dico* a) {
if (a==NULL) return;
free_liste_nbre(a->arr);
free_arbre_dico_trans(a->trans);
free(a);
}


void free_arbre_dico_non_rec(struct arbre_dico* a) {
  if (a==NULL)
    return;
  free_liste_nbre(a->arr);
  struct arbre_dico_trans* t;
  struct arbre_dico_trans* tmp;
  t = a->trans;
  while (t!=NULL) {
    tmp=t;
    t=t->suivant;
    free(tmp);
  }
  free(a);
}




void free_arbre_dico_trans(struct arbre_dico_trans* t) {
struct arbre_dico_trans* tmp;
while (t!=NULL) {
       free_arbre_dico(t->noeud);
      tmp=t;
      t=t->suivant;
      free(tmp);
}
}



struct arbre_dico_trans* get_transition(unichar c,struct arbre_dico** n) {
struct arbre_dico_trans* tmp;
if ((*n)->trans==NULL || c<((*n)->trans->c)) {
   // if we must insert at first position
   tmp=new_arbre_dico_trans();
   tmp->c=c;
   tmp->suivant=(*n)->trans;
   tmp->noeud=NULL;
   (*n)->trans=tmp;
   return tmp;
}
if (c==((*n)->trans->c)) {
   // nothing to do
   return (*n)->trans;
}
tmp=(*n)->trans;
while (tmp->suivant!=NULL && c>tmp->suivant->c) {
   tmp=tmp->suivant;
}
if (tmp->suivant==NULL || (tmp->suivant->c)>c) {
   // if we must insert between tmp and tmp->suivant
   struct arbre_dico_trans* ptr;
   ptr=new_arbre_dico_trans();
   ptr->c=c;
   ptr->suivant=tmp->suivant;
   ptr->noeud=NULL;
   tmp->suivant=ptr;
   return ptr;
}
// there we have (tmp->suivant->c == c)
return tmp->suivant;
}



int is_in_list(int n,struct liste_nbre* l) {
while (l!=NULL) {
   if (l->n==n) return 1;
   l=l->suivant;
}
return 0;
}



int OK=0;
unichar* compressed;
struct string_hash* hash;

void explorer_arbre_dico(unichar* contenu,int pos,struct arbre_dico* noeud) {
if (contenu[pos]=='\0') {
   unichar tmp[3000];
   int N=get_hash_number(compressed,hash);
   if (noeud->arr==NULL) {
      // if there is no node
      noeud->arr=new_liste_nbre();
      noeud->arr->n=N;
      noeud->hash_number=N;
      return;
   }
   if (is_in_list(N,noeud->arr)) {
      // if the compressed string has allready been taken into account for this node
      // (case of duplicates), we do nothing
      return;
   }
   u_strcpy(tmp,hash->tab[noeud->hash_number]);
   u_strcat_char(tmp,",");
   u_strcat(tmp,compressed);
   struct liste_nbre* l_tmp=new_liste_nbre();
   l_tmp->n=N;
   l_tmp->suivant=noeud->arr;
   noeud->arr=l_tmp;
   noeud->hash_number=noeud->arr->n;
   return;
}
struct arbre_dico_trans* t=get_transition(contenu[pos],&noeud);
if (t->noeud==NULL) {
  t->noeud=new_arbre_dico();
}
explorer_arbre_dico(contenu,pos+1,t->noeud);
int y=4;
}



void inserer_entree(unichar* contenu,unichar* compressed_,struct arbre_dico* noeud,struct string_hash* hash_) {
compressed=compressed_;
hash=hash_;
explorer_arbre_dico(contenu,0,noeud);
}

