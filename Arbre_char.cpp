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

#include "Arbre_char.h"
//---------------------------------------------------------------------------


struct liste_nbre* new_liste_nbre() {
struct liste_nbre* l=(struct liste_nbre*)malloc(sizeof(struct liste_nbre));
l->etiq=-1;
l->arr=-1;
l->suivant=NULL;
return l;
}


struct arbre_char* new_arbre_char() {
struct arbre_char* a=(struct arbre_char*)malloc(sizeof(struct arbre_char));
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


void free_liste_nbre(struct liste_nbre* l) {
struct liste_nbre* tmp;
while (l!=NULL) {
      tmp=l;
      l=l->suivant;
      free(tmp);
}
}


void free_arbre_char(struct arbre_char* a) {
if (a==NULL) return;
free_liste_nbre(a->arr);
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



struct liste_nbre* get_liste_nbre(int etiq,int arr,struct liste_nbre* l) {
struct liste_nbre* tmp;
if (l==NULL) {
  // if etiq is not in the list we create it
  tmp=new_liste_nbre();
  tmp->etiq=etiq;
  tmp->arr=arr;
  tmp->suivant=NULL;
  return tmp;
}
if (l->etiq==etiq) return l;
l->suivant=get_liste_nbre(etiq,arr,l->suivant);
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


struct liste_nbre* RES;

void ajouter_a_RES(struct liste_nbre* l) {
while (l!=NULL) {
      RES=get_liste_nbre(l->etiq,l->arr,RES);
      l=l->suivant;
}
}


void explorer_arbre(unichar* texte,int pos,struct arbre_char* noeud,Alphabet* alphabet,
                    int PARSING_MODE,int max_pos) {
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
   if (pos==max_pos) ajouter_a_RES(noeud->arr);
} else {
   if (texte[pos]=='\0') {
     // if we are at the end of the word
     ajouter_a_RES(noeud->arr);
     return;
   }
}
struct arbre_char_trans* trans=noeud->trans;
while (trans!=NULL) {
  if (is_equal_or_case_equal(trans->c,texte[pos],alphabet)) {
     // if the transition can be followed
     explorer_arbre(texte,pos+1,trans->noeud,alphabet,PARSING_MODE,max_pos);
  }
  trans=trans->suivant;
}
}


struct liste_nbre* get_matching_etiquettes(unichar* texte,
                                           struct arbre_char* racine,
                                           Alphabet* alphabet,int PARSING_MODE) {
RES=NULL;
explorer_arbre(texte,0,racine,alphabet,PARSING_MODE,0);
return RES;
}

