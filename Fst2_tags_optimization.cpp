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

//---------------------------------------------------------------------------
#include "Loading_dic.h"
#include "Fst2_tags_optimization.h"
#include "LocateConstants.h"
#include "BitArray.h"
#include "List_ustring.h"
#include "LemmaTree.h"
//---------------------------------------------------------------------------


/**
 * tests if the given string contains only one token
 * @param string  string to test
 * @param alph    pointer to Alphabet data structure
 */
static inline int est_un_token_simple(unichar* string, Alphabet* alph,int tokenization_mode) {
  if (is_a_simple_word(string,alph,tokenization_mode))
    return 1;
  if (u_strlen(string) == 1)
    return 1;
  return 0;
}


void deuxieme_cas_prime(int e,Fst2Tag* etiquette,unichar* s,Alphabet* alph,
						struct string_hash* tok,
						int tokenization_mode,struct locate_parameters* parameters) {
int num;
struct list_int* ptr_num;
struct list_int* ptr;
if (s[0]=='{' && u_strcmp(s,"{S}") && u_strcmp(s,"{STOP}")) {
   // case of a tag like {today,.ADV}
   num=get_value_index(s,tok);
  if ((parameters->matching_patterns[num]!=NULL)&&
      get_value(parameters->matching_patterns[num],etiquette[e]->number)) {
      ptr_num=(struct list_int*)malloc(sizeof(struct list_int));
      ptr_num->n=num;
      ptr_num->next=etiquette[e]->matching_tokens;
      etiquette[e]->matching_tokens=ptr_num;
      etiquette[e]->number_of_matching_tokens++;
   }
   return;
}
// normal case
if (!est_un_token_simple(s,alph,tokenization_mode)) {
  if (conditional_insertion_in_DLC_tree(s,etiquette[e]->number,parameters->current_compound_pattern,alph,tok,
  						parameters->DLC_tree,tokenization_mode,parameters->SPACE,parameters->pattern_tree_root)) {
     etiquette[e]->compound_pattern=parameters->current_compound_pattern;
  }
  return;
}
ptr=get_token_list_for_sequence(s,alph,tok);
struct list_int* ptr_copy = ptr; // s.n.
while (ptr!=NULL) {
  num=ptr->n;
  if ((parameters->matching_patterns[num]!=NULL)&&
      get_value(parameters->matching_patterns[num],etiquette[e]->number)) {
    ptr_num=(struct list_int*)malloc(sizeof(struct list_int));
    ptr_num->n=num;
    ptr_num->next=etiquette[e]->matching_tokens;
    etiquette[e]->matching_tokens=ptr_num;
    etiquette[e]->number_of_matching_tokens++;
  }
  ptr=ptr->next;
}
free_list_int(ptr_copy); // s.n.
}



void deuxieme_cas(int e,Fst2Tag* etiquette,Alphabet* alph,struct string_hash* tok,
				struct DLC_tree_info* DLC_tree,int tokenization_mode,struct lemma_node* root,
            struct locate_parameters* parameters) {
struct list_ustring* s=get_inflected_forms(etiquette[e]->lemma,root);
if (s==NULL) {
   etiquette[e]->number=NOTHING_TAG;
   return;
}
while (s!=NULL) {
   deuxieme_cas_prime(e,etiquette,s->string,alph,tok,tokenization_mode,parameters);
   s=s->next;
}
parameters->current_compound_pattern++;
if (etiquette[e]->matching_tokens==NULL) {
   etiquette[e]->number=NOTHING_TAG;
}
else etiquette[e]->number=LEXICAL_TAG;
}



void troisieme_cas_prime(int e,Fst2Tag* etiquette,unichar* s,Alphabet* alph,
						struct string_hash* tok,struct DLC_tree_info* DLC_tree,
						int tokenization_mode,struct locate_parameters* parameters) {
int num;
struct list_int* ptr_num;
struct list_int* ptr;
if (s[0]=='{' && u_strcmp(s,"{S}") && u_strcmp(s,"{STOP}")) {
   // case of a tag like {today,.ADV}
   num=get_value_index(s,tok);
   if ((parameters->matching_patterns[num]!=NULL)&&
      get_value(parameters->matching_patterns[num],etiquette[e]->number)) {
      ptr_num=(struct list_int*)malloc(sizeof(struct list_int));
      ptr_num->n=num;
      ptr_num->next=etiquette[e]->matching_tokens;
      etiquette[e]->matching_tokens=ptr_num;
      etiquette[e]->number_of_matching_tokens++;
   }
   return;
}
// normal case
if (!est_un_token_simple(s,alph,tokenization_mode)) {
  if (conditional_insertion_in_DLC_tree(s,etiquette[e]->number,parameters->current_compound_pattern,alph,tok,
  						parameters->DLC_tree,tokenization_mode,parameters->SPACE,parameters->pattern_tree_root)) {
     etiquette[e]->compound_pattern=parameters->current_compound_pattern;
  }
  return;
}
ptr=get_token_list_for_sequence(s,alph,tok);
struct list_int* ptr_copy = ptr; // s.n.
while (ptr!=NULL) {
  num=ptr->n;
  if ((parameters->matching_patterns[num]!=NULL)&&
      get_value(parameters->matching_patterns[num],etiquette[e]->number)) {
    ptr_num=(struct list_int*)malloc(sizeof(struct list_int));
    ptr_num->n=num;
    ptr_num->next=etiquette[e]->matching_tokens;
    etiquette[e]->matching_tokens=ptr_num;
    etiquette[e]->number_of_matching_tokens++;
  }
  ptr=ptr->next;
}
free_list_int(ptr_copy); // s.n.
}



void troisieme_cas(int e,Fst2Tag* etiquette,Alphabet* alph,struct string_hash* tok,
					struct DLC_tree_info* DLC_tree,int tokenization_mode,struct lemma_node* root,
               struct locate_parameters* parameters) {
struct list_ustring* s=get_inflected_forms(etiquette[e]->lemma,root);
if (s==NULL) {
   etiquette[e]->number=NOTHING_TAG;
   return;
}
while (s!=NULL) {
  if (s->string[0]=='{' && u_strcmp(s->string,"{S}") && u_strcmp(s->string,"{STOP}")) {
     // case of a token tag
     struct dela_entry* TMP=tokenize_tag_token(s->string);
     if (!u_strcmp(TMP->inflected,etiquette[e]->inflected)) {
        troisieme_cas_prime(e,etiquette,s->string,alph,tok,parameters->DLC_tree,tokenization_mode,parameters);
     }
     free_dela_entry(TMP);
  }
  else if (!u_strcmp(s->string,etiquette[e]->inflected)) {
     troisieme_cas_prime(e,etiquette,s->string,alph,tok,parameters->DLC_tree,tokenization_mode,parameters);
  }
  s=s->next;
}
(parameters->current_compound_pattern)++;
if (etiquette[e]->matching_tokens==NULL) {
   etiquette[e]->number=NOTHING_TAG;
}
else etiquette[e]->number=LEXICAL_TAG;
}



void quatrieme_cas_prime(int e,Fst2Tag* etiquette,unichar* s,Alphabet* alph,
						struct string_hash* tok,struct DLC_tree_info* DLC_tree,
						int tokenization_mode,struct locate_parameters* parameters) {
int num;
struct list_int* ptr_num;
struct list_int* ptr;
if (s[0]=='{' && u_strcmp(s,"{S}") && u_strcmp(s,"{STOP}")) {
   // case of a tag like {today,.ADV}
   num=get_value_index(s,tok);
   ptr_num=(struct list_int*)malloc(sizeof(struct list_int));
   ptr_num->n=num;
   ptr_num->next=etiquette[e]->matching_tokens;
   etiquette[e]->matching_tokens=ptr_num;
   etiquette[e]->number_of_matching_tokens++;
   return;
}
// normal case
//---mot compose
if (!est_un_token_simple(s,alph,tokenization_mode)) {
   add_compound_word_with_pattern(s,parameters->current_compound_pattern,alph,tok,parameters->DLC_tree,tokenization_mode,parameters->SPACE);
   etiquette[e]->compound_pattern=parameters->current_compound_pattern;
   return;
}
//---mot simple
ptr=get_token_list_for_sequence(s,alph,tok);
struct list_int* ptr_copy = ptr; // s.n.
while (ptr!=NULL) {
  num=ptr->n;
  ptr_num=(struct list_int*)malloc(sizeof(struct list_int));
  ptr_num->n=num;
  ptr_num->next=etiquette[e]->matching_tokens;
  etiquette[e]->matching_tokens=ptr_num;
  etiquette[e]->number_of_matching_tokens++;
  ptr=ptr->next;
}
free_list_int(ptr_copy); // s.n.
}



void quatrieme_cas(int e,Fst2Tag* etiquette,Alphabet* alph,struct string_hash* tok,
					struct DLC_tree_info* DLC_tree,int tokenization_mode,struct lemma_node* root,
               struct locate_parameters* parameters) {
struct list_ustring* s=get_inflected_forms(etiquette[e]->lemma,root);
if (s==NULL) {
   etiquette[e]->number=NOTHING_TAG;
   return;
}
while (s!=NULL) {
  quatrieme_cas_prime(e,etiquette,s->string,alph,tok,parameters->DLC_tree,tokenization_mode,parameters);
  s=s->next;
}
(parameters->current_compound_pattern)++;
if (etiquette[e]->matching_tokens==NULL)
  etiquette[e]->number=NOTHING_TAG;
else etiquette[e]->number=LEXICAL_TAG;
}



void cas_normal(int e,Fst2Tag* etiquette,Alphabet* alph,struct string_hash* tok,
				int case_variants_allowed,struct DLC_tree_info* DLC_tree,
				int tokenization_mode,struct list_int* tag_token_list,
            struct locate_parameters* parameters) {
int num;
struct list_int* ptr_num;
struct list_int* ptr;
unichar* s=etiquette[e]->input;
// first, we check if this tag can recognize some tag tokens
struct list_int* L=tag_token_list;
while (L!=NULL) {
   struct dela_entry* entry=tokenize_tag_token(tok->value[L->n]);
   if ((case_variants_allowed && is_equal_or_uppercase(s,entry->inflected,alph)) ||
       !u_strcmp(s,entry->inflected)) {
      num=L->n;
      ptr_num=(struct list_int*)malloc(sizeof(struct list_int));
      ptr_num->n=num;
      ptr_num->next=etiquette[e]->matching_tokens;
      etiquette[e]->matching_tokens=ptr_num;
      etiquette[e]->number_of_matching_tokens++;
   }
   free_dela_entry(entry);
   L=L->next;
}
if (!case_variants_allowed) {
   return;
}
// normal case
if (!est_un_token_simple(s,alph,tokenization_mode)) {
   add_compound_word_with_pattern(s,parameters->current_compound_pattern,alph,tok,DLC_tree,tokenization_mode,parameters->SPACE);
   etiquette[e]->compound_pattern=parameters->current_compound_pattern;
   (parameters->current_compound_pattern)++;
} else {
   ptr=get_token_list_for_sequence(etiquette[e]->input,alph,tok);
   struct list_int* ptr_copy = ptr; // s.n.
   while (ptr!=NULL) {
     num=ptr->n;
     ptr_num=(struct list_int*)malloc(sizeof(struct list_int));
     ptr_num->n=num;
     ptr_num->next=etiquette[e]->matching_tokens;
     etiquette[e]->matching_tokens=ptr_num;
     etiquette[e]->number_of_matching_tokens++;
     ptr=ptr->next;
   }
   free_list_int(ptr_copy); // s.n.
   if (etiquette[e]->matching_tokens==NULL) {
      etiquette[e]->number=NOTHING_TAG;
   }
   else {
      etiquette[e]->number=LEXICAL_TAG;
      etiquette[e]->control=(unsigned char)(etiquette[e]->control|LEMMA_TAG_BIT_MASK);
   }
}
}



void replace_pattern_tags(Fst2* automate,Alphabet* alph,struct string_hash* tok,
							int tokenization_mode,struct lemma_node* root,
                     struct locate_parameters* parameters) {
Fst2Tag* etiquette=automate->tags;
int i;
//printf("************** TRAITEMENT DES ANGLES ******************\n");
for (i=0;i</*etiquette_courante*/automate->number_of_tags;i++) {
  // cas des etiquettes entre angles
  if (etiquette[i]->control&LEMMA_TAG_BIT_MASK) {
    // cas <manger>
    if (etiquette[i]->number==LEXICAL_TAG) {
       quatrieme_cas(i,etiquette,alph,tok,parameters->DLC_tree,tokenization_mode,root,parameters);
    }
    else
    // cas <mange,manger.V>
    if ((etiquette[i]->lemma!=NULL)&&(etiquette[i]->inflected!=NULL)) {
       //printf("2");
       troisieme_cas(i,etiquette,alph,tok,parameters->DLC_tree,tokenization_mode,root,parameters);
    }
    else
    // cas <manger.V>
    if ((etiquette[i]->lemma!=NULL)&&(etiquette[i]->inflected==NULL)) {
       //printf("3\n");
       deuxieme_cas(i,etiquette,alph,tok,parameters->DLC_tree,tokenization_mode,root,parameters);
    }
  }
  else {
    // cas des etiquettes ou les variantes minuscules/majuscules sont permises
    // 32 :c'est une transition normale + !4 (variantes min/maj permises)

    if (etiquette[i]->control&TOKEN_TAG_BIT_MASK) {
       //printf("4");
       cas_normal(i,etiquette,alph,tok,!(etiquette[i]->control&RESPECT_CASE_TAG_BIT_MASK),
       			parameters->DLC_tree,tokenization_mode,parameters->tag_token_list,parameters);
    }
  }
}
}

