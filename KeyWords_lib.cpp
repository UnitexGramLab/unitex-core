/*
 * Unitex
 *
 * Copyright (C) 2001-2012 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include <stdlib.h>
#include "KeyWords_lib.h"
#include "Ustring.h"
#include "Error.h"
#include "DELA.h"
#include "Tokenization.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

#define UNKNOWN_WORD 0
#define PART_OF_A_LEMMATIZED_KEYWORD 1
#define LEMMATIZED_KEYWORD 2


KeyWord* new_KeyWord(double weight,unichar* sequence,KeyWord* next) {
KeyWord* k=(KeyWord*)malloc(sizeof(KeyWord));
if (k==NULL) {
	fatal_alloc_error("new_KeyWord");
}
k->weight=weight;
k->sequence=u_strdup(sequence);
if (sequence!=NULL && k->sequence==NULL) {
	fatal_alloc_error("new_KeyWord");
}
k->lemmatized=UNKNOWN_WORD;
k->next=next;
return k;
}


void free_KeyWord(KeyWord* k) {
if (k==NULL) return;
free(k->sequence);
free(k);
}


void free_KeyWord_list(KeyWord* k) {
while (k!=NULL) {
	KeyWord* tmp=k;
	k=k->next;
	free_KeyWord(tmp);
}
}


/**
 * Loads the initial keyword list from a tok_by_freq.txt file,
 * and turns all those tokens in a list whose primary key is the
 * lower case token:
 * The/20 THE/2 the/50 => the->(The/20 THE/2 the/50)
 */
struct string_hash_ptr* load_tokens_by_freq(char* name,VersatileEncodingConfig* vec) {
U_FILE* f=u_fopen(vec,name,U_READ);
if (f==NULL) return NULL;
Ustring* line=new_Ustring(128);
Ustring* lower=new_Ustring(128);
struct string_hash_ptr* res=new_string_hash_ptr(1024);
int val,pos;
/* We skip the first line of the file, containing the number
 * of tokens
 */
if (EOF==readline(line,f)) {
	fatal_error("Invalid empty file %s\n",name);
}
while (EOF!=readline(line,f)) {
	if (1!=u_sscanf(line->str,"%d%n",&val,&pos)) {
		fatal_error("Invalid line in file %s:\n%S\n",name,line->str);
	}
	u_strcpy(lower,line->str+pos);
	u_tolower(lower->str);
	int index=get_value_index(lower->str,res,INSERT_IF_NEEDED,NULL);
	if (index==-1) {
		fatal_error("Internal error in load_tokens_by_freq\n");
	}
	KeyWord* value=(KeyWord*)res->value[index];
	res->value[index]=new_KeyWord((double)val,line->str+pos,value);
}
free_Ustring(line);
free_Ustring(lower);
u_fclose(f);
return res;
}


/**
 * Removes every keyword that is not made of letters, by turning
 * the corresponding sequence value to NULL in the 'keywords' structure.
 */
void filter_non_letter_keywords(struct string_hash_ptr* keywords,Alphabet* alphabet) {
for (int i=0;i<keywords->size;i++) {
	KeyWord* k=(KeyWord*)(keywords->value[i]);
	while (k!=NULL) {
		if (k->sequence!=NULL && !is_sequence_of_letters(k->sequence,alphabet)) {
			free(k->sequence);
			k->sequence=NULL;
		}
		k=k->next;
	}
}
}


void remove_keyword(unichar* keyword,struct string_hash_ptr* keywords) {
unichar* lower=u_strdup(keyword);
u_tolower(lower);
KeyWord* k=(KeyWord*)get_value(lower,keywords);
free(lower);
if (k==NULL) return;
while (k!=NULL) {
	if (k->sequence!=NULL && !u_strcmp(keyword,k->sequence)) {
		free(k->sequence);
		k->sequence=NULL;
		return;
	}
	k=k->next;
}
}

void remove_keywords(struct list_ustring* list,struct string_hash_ptr* keywords) {
while (list!=NULL) {
	remove_keyword(list->string,keywords);
	list=list->next;
}
}


void lemmatize(struct dela_entry* e,struct string_hash_ptr* keywords,Alphabet* alphabet) {
unichar* lower=u_strdup(e->inflected);
u_tolower(lower);
KeyWord* k_inflected=(KeyWord*)get_value(lower,keywords);
free(lower);
if (k_inflected==NULL) return;
Ustring* tmp=new_Ustring(64);
u_sprintf(tmp,"%S.%S",e->lemma,e->semantic_codes[0]);
KeyWord* k_lemma=(KeyWord*)get_value(tmp->str,keywords);
if (k_lemma==NULL) {
	k_lemma=new_KeyWord(0,tmp->str,NULL);
	k_lemma->lemmatized=LEMMATIZED_KEYWORD;
	get_value_index(tmp->str,keywords,INSERT_IF_NEEDED,k_lemma);
}
/* Now, we look for all the case compatible tokens, and we add
 * their weights to the new lemmatized element
 */
while (k_inflected!=NULL) {
	if (k_inflected->sequence!=NULL && is_equal_or_uppercase(e->inflected,k_inflected->sequence,alphabet)) {
		/* We have a match */
		k_lemma->weight+=k_inflected->weight;
		k_inflected->lemmatized=1;
	}
	k_inflected=k_inflected->next;
}
free_Ustring(tmp);
}



void lemmatize_compound_word(struct dela_entry* e,struct list_ustring* tokens,
							struct string_hash_ptr* keywords,Alphabet* alphabet) {
double weight=0;
while (tokens!=NULL) {
	unichar* lower=u_strdup(tokens->string);
	u_tolower(lower);
	KeyWord* k_inflected=(KeyWord*)get_value(lower,keywords);
	free(lower);
	/* Now, we look for all the case compatible tokens, and we add
	 * their weights to the new lemmatized element
	 */
	while (k_inflected!=NULL) {
		if (k_inflected->sequence!=NULL && is_equal_or_uppercase(tokens->string,k_inflected->sequence,alphabet)) {
			/* We have a match */
			if (weight==0 || k_inflected->weight<weight) {
				weight=k_inflected->weight;
			}
			k_inflected->lemmatized=1;
		}
		k_inflected=k_inflected->next;
	}
	tokens=tokens->next;
}
if (weight==0) {
	/* If all the tokens that made the compound form were forbidden
	 * ones, we have nothing to do */
	return;
}
/* We finally create or update the compound word lemma */
Ustring* tmp=new_Ustring(64);
u_sprintf(tmp,"%S.%S",e->lemma,e->semantic_codes[0]);
KeyWord* k_lemma=(KeyWord*)get_value(tmp->str,keywords);
if (k_lemma==NULL) {
	k_lemma=new_KeyWord(0,tmp->str,NULL);
	k_lemma->lemmatized=LEMMATIZED_KEYWORD;
	get_value_index(tmp->str,keywords,INSERT_IF_NEEDED,k_lemma);
}
k_lemma->weight=weight;
free_Ustring(tmp);
}



/**
 * Loads the given DELAF and modifies the given keywords accordingly by:
 * 1) removing every token that is the inflected form of a DELAF
 *    entry containing the given forbidden code (or a token of it
 *    if the inflected form is a compound word)
 * 2) replacing any non removed token that appear in a DELAF entry
 *    by its lemma. If there are ambiguities, several keywords are
 *    generated. Doing that may merge keywords by adding their weights:
 *    eats/2 + eaten/3 => eat/5
 */
void filter_keywords_with_dic(struct string_hash_ptr* keywords,char* name,
						VersatileEncodingConfig* vec,Alphabet* alphabet,
						unichar* forbidden_code) {
U_FILE* f=u_fopen(vec,name,U_READ);
if (f==NULL) {
	error("Cannot load file %s\n",name);
	return;
}
Ustring* line=new_Ustring(128);
while (EOF!=readline(line,f)) {
	struct dela_entry* e=tokenize_DELAF_line(line->str);
	unichar tmp[1024];
	u_strcpy(tmp,line->str);
	if (e==NULL) continue;
	struct list_ustring* tokens=tokenize_word_by_word(e->inflected,alphabet);
	if (dic_entry_contain_gram_code(e,forbidden_code)) {
		/* We must remove keywords */
		if (tokens->next==NULL) {
			/* Simple word */
			remove_keyword(e->inflected,keywords);
		} else {
			/* Compound word */
			remove_keywords(tokens,keywords);
		}
	} else {
		/* We must deal with a DELAF entry to keep */
		if (tokens->next==NULL) {
			/* Simple word */
			lemmatize(e,keywords,alphabet);
		} else {
			/* Compound word */
			lemmatize_compound_word(e,tokens,keywords,alphabet);
		}
	}
	free_list_ustring(tokens);
	free_dela_entry(e);
}
free_Ustring(line);
u_fclose(f);
}


KeyWord* locate_candidate(unichar* a,KeyWord* list,Alphabet* alphabet) {
while (list!=NULL) {
	if (list->sequence!=NULL && list->lemmatized==UNKNOWN_WORD) {
		/* We have a potential match */
		if (is_equal_or_uppercase(a,list->sequence,alphabet)) {
			/* We have a=Fogg and list->sequence=FOGG
			 * Our candidate must be replaced by a */
			free(list->sequence);
			list->sequence=u_strdup(a);
			return list;
		}
		if (is_equal_or_uppercase(list->sequence,a,alphabet)) {
			/* We have a=FOGG and list->sequence=Fogg
			 * Our candidate is already the good one to keep */
			return list;
		}
	}
	list=list->next;
}
return NULL;
}


void merge_case_equivalent_unknown_words(struct string_hash_ptr* keywords,Alphabet* alphabet) {
for (int i=0;i<keywords->size;i++) {
	KeyWord* k=(KeyWord*)keywords->value[i];
	while (k!=NULL) {
		if (k->sequence!=NULL && k->lemmatized==UNKNOWN_WORD) {
			/* We have found a candidate for merging */
			KeyWord* candidate=locate_candidate(k->sequence,k->next,alphabet);
			if (candidate!=NULL) {
				candidate->weight+=k->weight;
				free(k->sequence);
				k->sequence=NULL;
			}
		}
		k=k->next;
	}
}

}


void dump_keywords(vector_ptr* keywords,U_FILE* f) {
for (int i=0;i<keywords->nbelems;i++) {
	KeyWord* k=(KeyWord*)keywords->tab[i];
	while (k!=NULL) {
		if (k->sequence!=NULL && k->lemmatized!=PART_OF_A_LEMMATIZED_KEYWORD) {
			u_fprintf(f,"%g\t%S\n",k->weight,k->sequence);
		}
		k=k->next;
	}
}
}


int cmp_keywords(KeyWord* *a,KeyWord* *b) {
return (*b)->weight-(*a)->weight;
}


/**
 * We build an array of single keywords (lists of only one element),
 * sorted by descending weight.
 */
vector_ptr* sort_keywords(struct string_hash_ptr* keywords) {
vector_ptr* res=new_vector_ptr();
for (int i=0;i<keywords->size;i++) {
	KeyWord* k=(KeyWord*)(keywords->value[i]);
	while (k!=NULL) {
		if (k->sequence!=NULL && k->lemmatized!=PART_OF_A_LEMMATIZED_KEYWORD) {
			vector_ptr_add(res,new_KeyWord(k->weight,k->sequence,NULL));
		}
		k=k->next;
	}
}
qsort(res->tab,res->nbelems,sizeof(KeyWord*),(int(*)(const void*,const void*))cmp_keywords);
return res;
}


} // namespace unitex


