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
#include "String_hash.h"
//---------------------------------------------------------------------------


struct arbre_hash* new_arbre_hash() {
struct arbre_hash* a;
a=(struct arbre_hash*)malloc(sizeof(struct arbre_hash));
a->final=-1;
a->trans=NULL;
return a;
}


struct arbre_hash_trans* new_arbre_hash_trans() {
struct arbre_hash_trans* a;
a=(struct arbre_hash_trans*)malloc(sizeof(struct arbre_hash_trans));
a->c='\0';
a->arr=NULL;
a->suivant=NULL;
return a;
}


void free_arbre_hash(struct arbre_hash*);

void free_arbre_hash_trans(struct arbre_hash_trans* t) {
struct arbre_hash_trans* tmp;
while (t!=NULL) {
      free_arbre_hash(t->arr);
      tmp=t;
      t=t->suivant;
      free(tmp);
}
}


void free_arbre_hash(struct arbre_hash* a) {
if (a==NULL) return;
free_arbre_hash_trans(a->trans);
free(a);
}



void free_string_hash(struct string_hash* s) {
if (s==NULL) return;
free_arbre_hash(s->racine);
for (int i=0;i<s->N;i++) {
  free(s->tab[i]);
}
free(s->tab);
free(s);
}



void free_string_hash_without_insert(struct string_hash* s) {
if (s==NULL) return;
free_arbre_hash(s->racine);
free(s);
}



struct string_hash* new_string_hash() {
struct string_hash* s;
s=(struct string_hash*)malloc(sizeof(struct string_hash));
s->N=0;
s->tab=(unichar**)malloc(1000000*sizeof(unichar*));
s->racine=new_arbre_hash();
return s;
}



struct string_hash* new_string_hash_N(int N) {
struct string_hash* s;
N=N+50000;
s=(struct string_hash*)malloc(sizeof(struct string_hash));
s->N=0;
s->tab=(unichar**)malloc(N*sizeof(unichar*));
s->racine=new_arbre_hash();
return s;
}

struct arbre_hash_trans* get_transition(unichar c,struct arbre_hash_trans* t) {
/*if (t==NULL) return NULL;
if (t->c==c) return t;
return get_transition(c,t->suivant);
*/
while (t!=NULL) {
   if (t->c==c) return t;
   t=t->suivant;
}
return NULL;
}



int inserer_si_absent(unichar* s,int pos,struct arbre_hash* noeud,struct string_hash* hash,int INSERT) {

if (noeud==NULL) {
   fprintf(stderr,"Erreur dans fonction inserer_si_absent\n");
   return -1;
}

  if (s[pos]=='\0') {
    // if we are at the end of the entry
    if (noeud->final!=-1) {
      // if the entry allready exists, we return its hash number
      return noeud->final;
    }
    noeud->final=hash->N;
    if (INSERT) {
      hash->tab[noeud->final]=(unichar*)malloc(sizeof(unichar)*(1+u_strlen(s)));
      u_strcpy(hash->tab[noeud->final],s);
    }
    (hash->N)++;
    return noeud->final;
  }

struct arbre_hash_trans* t=get_transition(s[pos],noeud->trans);
if (t==NULL) {
   t=new_arbre_hash_trans();
   t->c=s[pos];
   t->suivant=noeud->trans;
   t->arr=new_arbre_hash();
   noeud->trans=t;
}
return inserer_si_absent(s,pos+1,t->arr,hash,INSERT);
}




int get_hash_number(unichar* s,struct string_hash* hash) {
return inserer_si_absent(s,0,hash->racine,hash,1);
}


int get_hash_number_without_insert(unichar* s,struct string_hash* hash) {
return inserer_si_absent(s,0,hash->racine,hash,0);
}



int get_if_exists(unichar* s,int pos,struct arbre_hash* noeud) {
if (noeud==NULL) {
   return -1;
}
if (s[pos]=='\0') {
   // if we are at the end of the entry
   return noeud->final;
}
struct arbre_hash_trans* t=get_transition(s[pos],noeud->trans);
if (t!=NULL) {
   return get_if_exists(s,pos+1,t->arr);
}
return -1;
}



int get_token_number(unichar* s,struct string_hash* hash) {
return get_if_exists(s,0,hash->racine);
}


void sauver_lignes_hash(FILE *f,struct string_hash* hash) {
for (int i=0;i<hash->N;i++) {
  u_fprints(hash->tab[i],f);
  u_fprints_char("\n",f);
}
}



//
// loads the elements of a word list
//
struct string_hash* load_word_list(char* name) {
FILE* f=u_fopen(name,U_READ);
if (f==NULL) return NULL;
struct string_hash* res=new_string_hash();
unichar temp[4096];
while (u_read_line(f,temp)) {
   get_hash_number(temp,res);
}
u_fclose(f);
return res;
}






int exists_modulo_case(unichar* s,int pos,struct arbre_hash* noeud,
                              Alphabet* alph) {
if (noeud==NULL) {
   return 0;
}
if (s[pos]=='\0') {
   // if we are at the end of the entry
   return (noeud->final!=-1);
}
struct arbre_hash_trans* t=noeud->trans;
while (t!=NULL) {
   if (is_equal_or_uppercase(t->c,s[pos],alph)) {
      if (exists_modulo_case(s,pos+1,t->arr,alph)) {
         return 1;
      }
   }
   t=t->suivant;
}
return 0;
}


int is_in_string_hash_modulo_case(unichar* w,struct string_hash* hash,
                                  Alphabet* alph) {
return exists_modulo_case(w,0,hash->racine,alph);
}

