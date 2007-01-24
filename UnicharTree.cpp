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

#include "UnicharTree.h"
#include "Error.h"


struct arbre_char* new_arbre_char() {
struct arbre_char* a=(struct arbre_char*)malloc(sizeof(struct arbre_char));
if (a==NULL) {
   fatal_error("Not enough memory in new_arbre_char\n");
}
a->arr=NULL;
a->trans=NULL;
return a;
}


struct arbre_char_trans* new_arbre_char_trans(unichar c,struct arbre_char_trans* suivant) {
struct arbre_char_trans* t=(struct arbre_char_trans*)malloc(sizeof(struct arbre_char_trans));
t->c=c;
t->noeud=NULL;
t->suivant=suivant;
return t;
}



void free_arbre_char(struct arbre_char* a) {
if (a==NULL) return;
free_Fst2Transition(a->arr);
free_arbre_char_trans(a->trans);
free(a);
}


void free_arbre_char_trans(struct arbre_char_trans* t) {
struct arbre_char_trans* tmp;
while (t!=NULL) {
      free_arbre_char(t->noeud);
      tmp=t;
      t=t->suivant;
      free(tmp);
}
}



struct arbre_char_trans* get_transition(unichar c,struct arbre_char_trans* t) {
while (t!=NULL) {
      if (t->c==c) return t;
      t=t->suivant;
}
return NULL;
}



Fst2Transition get_liste_nbre(int etiq,int arr,Fst2Transition l) {
Fst2Transition tmp;
if (l==NULL) {
  // if etiq is not in the list we create it
  tmp=new_Fst2Transition(etiq,arr);
  tmp->next=NULL;
  return tmp;
}
if (l->tag_number==etiq) return l;
l->next=get_liste_nbre(etiq,arr,l->next);
return l;
}



void explorer_arbre_char(unichar* contenu,int pos,int etiq,int arr,struct arbre_char* noeud) {
if (noeud==NULL) {
   fprintf(stderr,"Erreur dans fonction explorer_arbre_char\n");
   return;
}
if (contenu[pos]=='\0') {
   // if we are at the end of the word
   // we must have a list because %a and %a/A would give two path for entry a
   noeud->arr=get_liste_nbre(etiq,arr,noeud->arr);
   return;
}
struct arbre_char_trans* t=get_transition(contenu[pos],noeud->trans);
if (t==NULL) {
  noeud->trans=new_arbre_char_trans(contenu[pos],noeud->trans);
  noeud->trans->noeud=new_arbre_char();
  t=noeud->trans;
}
explorer_arbre_char(contenu,pos+1,etiq,arr,t->noeud);
}



void inserer_etiquette(unichar* contenu,int etiq,int arr,struct arbre_char* noeud) {
explorer_arbre_char(contenu,0,etiq,arr,noeud);
}


void ajouter_a_RES(Fst2Transition l,Fst2Transition *RES) {
while (l!=NULL) {
      *RES=get_liste_nbre(l->tag_number,l->state_number,*RES);
      l=l->next;
}
}


void explorer_arbre(unichar* texte,int pos,struct arbre_char* noeud,Alphabet* alphabet,
                    int PARSING_MODE,int max_pos,Fst2Transition *result) {
if (noeud==NULL) {
   return;
}
if (PARSING_MODE) {
   // if we are in thai mode, we consider each possibility
   if (noeud->arr!=NULL && pos>max_pos) {
      max_pos=pos;
      //free_liste_nbre(RES);
      //RES=NULL;
   }
   if (pos==max_pos) ajouter_a_RES(noeud->arr,result);
} else {
   if (texte[pos]=='\0') {
     // if we are at the end of the word
     ajouter_a_RES(noeud->arr,result);
     return;
   }
}
struct arbre_char_trans* trans=noeud->trans;
while (trans!=NULL) {
  if (is_equal_or_uppercase(trans->c,texte[pos],alphabet)) {
     // if the transition can be followed
     explorer_arbre(texte,pos+1,trans->noeud,alphabet,PARSING_MODE,max_pos,result);
  }
  trans=trans->suivant;
}
}


Fst2Transition get_matching_etiquettes(unichar* texte,
                                           struct arbre_char* racine,
                                           Alphabet* alphabet,int PARSING_MODE) {
Fst2Transition RES=NULL;
explorer_arbre(texte,0,racine,alphabet,PARSING_MODE,0,&RES);
return RES;
}

