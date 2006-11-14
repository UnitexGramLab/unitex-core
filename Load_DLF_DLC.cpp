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

#include "Load_DLF_DLC.h"
//---------------------------------------------------------------------------


struct noeud_dlf_dlc* new_noeud_dlf_dlc() {
struct noeud_dlf_dlc* n;
n=(struct noeud_dlf_dlc*)malloc(sizeof(struct noeud_dlf_dlc));
n->liste=NULL;
n->trans=NULL;
return n;
}


struct trans_dlf_dlc* new_trans_dlf_dlc() {
struct trans_dlf_dlc* t;
t=(struct trans_dlf_dlc*)malloc(sizeof(struct trans_dlf_dlc));
t->arr=NULL;
t->suivant=NULL;
return t;
}


struct liste_chaines* new_liste_chaines(struct dela_entry* entry) {
struct liste_chaines* l;
l=(struct liste_chaines*)malloc(sizeof(struct liste_chaines));
l->suivant=NULL;
l->entry=clone_dela_entry(entry);
return l;
}


void free_noeud_dlf_dlc(struct noeud_dlf_dlc* n) {
if (n==NULL) return;
free_liste_chaines(n->liste);
free_trans_dlf_dlc(n->trans);
free(n);
}


void free_trans_dlf_dlc(struct trans_dlf_dlc* t) {
struct trans_dlf_dlc* tmp;
while (t!=NULL) {
  tmp=t;
  t=t->suivant;
  free_noeud_dlf_dlc(tmp->arr);
  free(tmp);
}
}


void free_liste_chaines(struct liste_chaines* l) {
struct liste_chaines* tmp;
while (l!=NULL) {
  tmp=l;
  l=l->suivant;
  free_dela_entry(tmp->entry);
  free(tmp);
}
}



struct liste_chaines* inserer_si_absent_dans_liste_chaines(struct dela_entry* entry,
                                                           struct liste_chaines* l) {
if (l==NULL) return new_liste_chaines(entry);
if (equal(l->entry,entry)) return l;
l->suivant=inserer_si_absent_dans_liste_chaines(entry,l->suivant);
return l;
}



struct trans_dlf_dlc* get_trans_dlf_dlc(unichar c,struct trans_dlf_dlc* l) {
if (l==NULL) return NULL;
if (l->c==c) return l;
return get_trans_dlf_dlc(c,l->suivant);
}



void inserer_dlf_dlc(int pos,struct dela_entry* entry,struct noeud_dlf_dlc* n) {
if (entry->inflected[pos]=='\0') {
   // if we are at the end of the flechi string, we must insert the code
   n->liste=inserer_si_absent_dans_liste_chaines(entry,n->liste);
   return;
}
struct trans_dlf_dlc* trans;
trans=get_trans_dlf_dlc(entry->inflected[pos],n->trans);
if (trans==NULL) {
   // if the transition does not exist, we create it
   trans=new_trans_dlf_dlc();
   trans->c=entry->inflected[pos];
   trans->arr=new_noeud_dlf_dlc();
   trans->suivant=n->trans;
   n->trans=trans;
}
inserer_dlf_dlc(pos+1,entry,trans->arr);
}



void inserer_dans_arbre_dlf_dlc(struct dela_entry* entry,struct noeud_dlf_dlc* racine) {
inserer_dlf_dlc(0,entry,racine);
}



void load_dlf_dlc(char* nom,struct noeud_dlf_dlc* racine) {
FILE* f;
f=u_fopen(nom,U_READ);
if (f==NULL) {
   fprintf(stderr,"Cannot load dictionary %s\n",nom);
   return;
}
printf("Loading %s...\n",nom);
unichar ligne[1000];
while (EOF!=u_read_line(f,ligne)) {
   struct dela_entry* entry=tokenize_DELAF_line(ligne,1);
   inserer_dans_arbre_dlf_dlc(entry,racine);
   free_dela_entry(entry);
}
u_fclose(f);
}
