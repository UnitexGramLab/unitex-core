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
#include "Loading_dic.h"
#include "Fst2_tags_optimization.h"
#include "LocateConstants.h"
//---------------------------------------------------------------------------


unichar* get_formes_flechies(unichar* canonique,int i,struct noeud* n) {
struct noeud* sous_noeud;
sous_noeud=get_sous_noeud(n,canonique[i],0);
if (sous_noeud==NULL) {
   return NULL;
}
i++;
if (canonique[i]=='\0') {
   return sous_noeud->formes_flechies;
}
else return get_formes_flechies(canonique,i,sous_noeud);
}

/**
 * tests if the given string contains only one token
 * @param string  string to test
 * @param alph    pointer to Alphabet data structure
 */
static inline int est_un_token_simple(unichar* string, Alphabet* alph,int tokenization_mode) {
  if (est_un_mot_simple(string,alph,tokenization_mode))
    return 1;
  if (u_strlen(string) == 1)
    return 1;
  return 0;
}


void deuxieme_cas_prime(int e,Etiquette* etiquette,unichar* s,Alphabet* alph,
						struct string_hash* tok,struct DLC_tree_info* DLC_tree,
						int tokenization_mode) {
int num;
struct liste_nombres* ptr_num;
struct liste_nombres* ptr;
if (s[0]=='{' && u_strcmp_char(s,"{S}") && u_strcmp_char(s,"{STOP}")) {
   // case of a tag like {today,.ADV}
   num=get_hash_number(s,tok);
  if ((index_code_gramm[num]!=NULL)&&
      (index_code_gramm[num][etiquette[e]->numero/8]&(1<<(etiquette[e]->numero%8)))) {
      ptr_num=(struct liste_nombres*)malloc(sizeof(struct liste_nombres));
      ptr_num->n=num;
      ptr_num->suivant=etiquette[e]->numeros;
      etiquette[e]->numeros=ptr_num;
      etiquette[e]->nombre_mots++;
   }
   return;
}
// normal case
if (!est_un_token_simple(s,alph,tokenization_mode)) {
  if (conditional_insertion_in_DLC_tree(s,etiquette[e]->numero,pattern_compose_courant,alph,tok,
  						DLC_tree,tokenization_mode)) {
     etiquette[e]->pattern_compose=pattern_compose_courant;
  }
  return;
}
ptr=get_token_list_for_sequence(s,alph,tok);
struct liste_nombres* ptr_copy = ptr; // s.n.
while (ptr!=NULL) {
  num=ptr->n;
  if ((index_code_gramm[num]!=NULL)&&
      (index_code_gramm[num][etiquette[e]->numero/8]&(1<<(etiquette[e]->numero%8)))) {
    ptr_num=(struct liste_nombres*)malloc(sizeof(struct liste_nombres));
    ptr_num->n=num;
    ptr_num->suivant=etiquette[e]->numeros;
    etiquette[e]->numeros=ptr_num;
    etiquette[e]->nombre_mots++;
  }
  ptr=ptr->suivant;
}
free_liste_nombres(ptr_copy); // s.n.
}



void deuxieme_cas(int e,Etiquette* etiquette,Alphabet* alph,struct string_hash* tok,
				struct DLC_tree_info* DLC_tree,int tokenization_mode) {
unichar* s;
unichar tmp[2000];
int i,j;
s=get_formes_flechies(etiquette[e]->canonique,0,racine);
if (s==NULL) {
   etiquette[e]->numero=NOTHING_TAG;
   return;
}
i=0;
while (s[i]!='\0') {
  j=0;
  while ((s[i]!=SEPARATOR_CHAR)&&(s[i]!='\0')) {
    tmp[j++]=s[i++];
  }
  tmp[j]='\0';
  if (s[i]==SEPARATOR_CHAR) {
     i++;
  }
  deuxieme_cas_prime(e,etiquette,tmp,alph,tok,DLC_tree,tokenization_mode);
}
pattern_compose_courant++;
if (etiquette[e]->numeros==NULL) {
   etiquette[e]->numero=NOTHING_TAG;
}
else etiquette[e]->numero=LEXICAL_TAG;
}



void troisieme_cas_prime(int e,Etiquette* etiquette,unichar* s,Alphabet* alph,
						struct string_hash* tok,struct DLC_tree_info* DLC_tree,
						int tokenization_mode) {
int num;
struct liste_nombres* ptr_num;
struct liste_nombres* ptr;
if (s[0]=='{' && u_strcmp_char(s,"{S}") && u_strcmp_char(s,"{STOP}")) {
   // case of a tag like {today,.ADV}
   num=get_hash_number(s,tok);
   if ((index_code_gramm[num]!=NULL)&&
      (index_code_gramm[num][etiquette[e]->numero/8]&(1<<(etiquette[e]->numero%8)))) {
      ptr_num=(struct liste_nombres*)malloc(sizeof(struct liste_nombres));
      ptr_num->n=num;
      ptr_num->suivant=etiquette[e]->numeros;
      etiquette[e]->numeros=ptr_num;
      etiquette[e]->nombre_mots++;
   }
   return;
}
// normal case
if (!est_un_token_simple(s,alph,tokenization_mode)) {
  if (conditional_insertion_in_DLC_tree(s,etiquette[e]->numero,pattern_compose_courant,alph,tok,
  						DLC_tree,tokenization_mode)) {
     etiquette[e]->pattern_compose=pattern_compose_courant;
  }
  return;
}
ptr=get_token_list_for_sequence(s,alph,tok);
struct liste_nombres* ptr_copy = ptr; // s.n.
while (ptr!=NULL) {
  num=ptr->n;
  if ((index_code_gramm[num]!=NULL)&&
      (index_code_gramm[num][etiquette[e]->numero/8]&(1<<(etiquette[e]->numero%8)))) {
    ptr_num=(struct liste_nombres*)malloc(sizeof(struct liste_nombres));
    ptr_num->n=num;
    ptr_num->suivant=etiquette[e]->numeros;
    etiquette[e]->numeros=ptr_num;
    etiquette[e]->nombre_mots++;
  }
  ptr=ptr->suivant;
}
free_liste_nombres(ptr_copy); // s.n.
}



void troisieme_cas(int e,Etiquette* etiquette,Alphabet* alph,struct string_hash* tok,
					struct DLC_tree_info* DLC_tree,int tokenization_mode) {
unichar* s;
unichar tmp[2000];
int i,j;
s=get_formes_flechies(etiquette[e]->canonique,0,racine);
if (s==NULL) {
   etiquette[e]->numero=NOTHING_TAG;
   return;
}
i=0;
while (s[i]!='\0') {
  j=0;
  while ((s[i]!=SEPARATOR_CHAR)&&(s[i]!='\0')) {
     tmp[j++]=s[i++];
  }
  tmp[j]='\0';
  if (s[i]==SEPARATOR_CHAR) {
     i++;
  }
  if (tmp[0]=='{' && u_strcmp_char(tmp,"{S}") && u_strcmp_char(s,"{STOP}")) {
     // case of a token tag
     dic_entry* TMP=tokenize_tag_token(tmp);
     if (!u_strcmp(TMP->inflected,etiquette[e]->flechi)) {
        troisieme_cas_prime(e,etiquette,tmp,alph,tok,DLC_tree,tokenization_mode);
     }
     free_dic_entry(TMP);
  }
  else if (!u_strcmp(tmp,etiquette[e]->flechi)) {
     troisieme_cas_prime(e,etiquette,tmp,alph,tok,DLC_tree,tokenization_mode);
  }
}
pattern_compose_courant++;
if (etiquette[e]->numeros==NULL) {
   etiquette[e]->numero=NOTHING_TAG;
}
else etiquette[e]->numero=LEXICAL_TAG;
}



void quatrieme_cas_prime(int e,Etiquette* etiquette,unichar* s,Alphabet* alph,
						struct string_hash* tok,struct DLC_tree_info* DLC_tree,
						int tokenization_mode) {
int num;
struct liste_nombres* ptr_num;
struct liste_nombres* ptr;
if (s[0]=='{' && u_strcmp_char(s,"{S}") && u_strcmp_char(s,"{STOP}")) {
   // case of a tag like {today,.ADV}
   num=get_hash_number(s,tok);
   ptr_num=(struct liste_nombres*)malloc(sizeof(struct liste_nombres));
   ptr_num->n=num;
   ptr_num->suivant=etiquette[e]->numeros;
   etiquette[e]->numeros=ptr_num;
   etiquette[e]->nombre_mots++;
   return;
}
// normal case
//---mot compose
if (!est_un_token_simple(s,alph,tokenization_mode)) {
   add_compound_word_with_pattern(s,pattern_compose_courant,alph,tok,DLC_tree,tokenization_mode);
   etiquette[e]->pattern_compose=pattern_compose_courant;
   return;
}
//---mot simple
ptr=get_token_list_for_sequence(s,alph,tok);
struct liste_nombres* ptr_copy = ptr; // s.n.
while (ptr!=NULL) {
  num=ptr->n;
  ptr_num=(struct liste_nombres*)malloc(sizeof(struct liste_nombres));
  ptr_num->n=num;
  ptr_num->suivant=etiquette[e]->numeros;
  etiquette[e]->numeros=ptr_num;
  etiquette[e]->nombre_mots++;
  ptr=ptr->suivant;
}
free_liste_nombres(ptr_copy); // s.n.
}



void quatrieme_cas(int e,Etiquette* etiquette,Alphabet* alph,struct string_hash* tok,
					struct DLC_tree_info* DLC_tree,int tokenization_mode) {
unichar* s;
unichar tmp[2000];
int i,j;
s=get_formes_flechies(etiquette[e]->canonique,0,racine);
if (s==NULL) {
  etiquette[e]->numero=NOTHING_TAG;
  return;
}
i=0;
while (s[i]!='\0') {
  j=0;
  while ((s[i]!=SEPARATOR_CHAR)&&(s[i]!='\0'))
    tmp[j++]=s[i++];
  tmp[j]='\0';
  if (s[i]==SEPARATOR_CHAR) i++;
  quatrieme_cas_prime(e,etiquette,tmp,alph,tok,DLC_tree,tokenization_mode);
}
pattern_compose_courant++;
if (etiquette[e]->numeros==NULL)
  etiquette[e]->numero=NOTHING_TAG;
else etiquette[e]->numero=LEXICAL_TAG;
}



void cas_normal(int e,Etiquette* etiquette,Alphabet* alph,struct string_hash* tok,
				int case_variants_allowed,struct DLC_tree_info* DLC_tree,
				int tokenization_mode) {
int num;
struct liste_nombres* ptr_num;
struct liste_nombres* ptr;
unichar* s=etiquette[e]->contenu;
// first, we check if this tag can recognize some tag tokens
struct liste_nombres* L=tag_token_list;
while (L!=NULL) {
   unichar inflected[1000];
   unichar lemma[1000];
   unichar code_gramm[1000];
   tokenize_tag_token_into_3_parts(tok->tab[L->n],inflected,lemma,code_gramm);
   if ((case_variants_allowed && is_equal_or_uppercase(s,inflected,alph)) ||
       !u_strcmp(s,inflected)) {
      num=L->n;
      ptr_num=(struct liste_nombres*)malloc(sizeof(struct liste_nombres));
      ptr_num->n=num;
      ptr_num->suivant=etiquette[e]->numeros;
      etiquette[e]->numeros=ptr_num;
      etiquette[e]->nombre_mots++;
   }
   L=L->suivant;
}
if (!case_variants_allowed) {
   return;
}
// normal case
if (!est_un_token_simple(s,alph,tokenization_mode)) {
   add_compound_word_with_pattern(s,pattern_compose_courant,alph,tok,DLC_tree,tokenization_mode);
   etiquette[e]->pattern_compose=pattern_compose_courant;
   pattern_compose_courant++;
} else {
   ptr=get_token_list_for_sequence(etiquette[e]->contenu,alph,tok);
   struct liste_nombres* ptr_copy = ptr; // s.n.
   while (ptr!=NULL) {
     num=ptr->n;
     ptr_num=(struct liste_nombres*)malloc(sizeof(struct liste_nombres));
     ptr_num->n=num;
     ptr_num->suivant=etiquette[e]->numeros;
     etiquette[e]->numeros=ptr_num;
     etiquette[e]->nombre_mots++;
     ptr=ptr->suivant;
   }
   free_liste_nombres(ptr_copy); // s.n.
   if (etiquette[e]->numeros==NULL) {
      etiquette[e]->numero=NOTHING_TAG;
   }
   else {
      etiquette[e]->numero=LEXICAL_TAG;
      etiquette[e]->controle=(unsigned char)(etiquette[e]->controle|LEMMA_TAG_BIT_MASK);
   }
}
}



void replace_pattern_tags(Automate_fst2* automate,Alphabet* alph,struct string_hash* tok,
							struct DLC_tree_info* DLC_tree,int tokenization_mode) {
Etiquette* etiquette=automate->etiquette;
int i;
//printf("************** TRAITEMENT DES ANGLES ******************\n");
for (i=0;i<etiquette_courante;i++) {
  // cas des etiquettes entre angles
  if (etiquette[i]->controle&LEMMA_TAG_BIT_MASK) {
    // cas <manger>
    if (etiquette[i]->numero==LEXICAL_TAG) {
       //printf("1");
       quatrieme_cas(i,etiquette,alph,tok,DLC_tree,tokenization_mode);
    }
    else
    // cas <mange,manger.V>
    if ((etiquette[i]->canonique!=NULL)&&(etiquette[i]->flechi!=NULL)) {
       //printf("2");
       troisieme_cas(i,etiquette,alph,tok,DLC_tree,tokenization_mode);
    }
    else
    // cas <manger.V>
    if ((etiquette[i]->canonique!=NULL)&&(etiquette[i]->flechi==NULL)) {
       //printf("3");
       deuxieme_cas(i,etiquette,alph,tok,DLC_tree,tokenization_mode);
    }
  }
  else {
    // cas des etiquettes ou les variantes minuscules/majuscules sont permises
    // 32 :c'est une transition normale + !4 (variantes min/maj permises)

    if (etiquette[i]->controle&TOKEN_TAG_BIT_MASK) {
       //printf("4");
       cas_normal(i,etiquette,alph,tok,!(etiquette[i]->controle&RESPECT_CASE_TAG_BIT_MASK),
       			DLC_tree,tokenization_mode);
    }
  }
}
}

