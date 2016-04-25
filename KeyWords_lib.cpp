/*
 * Unitex
 *
 * Copyright (C) 2001-2016 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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


KeyWord* new_KeyWord(int weight,unichar* sequence,KeyWord* next) {
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
    res->value[index]=new_KeyWord(val,line->str+pos,value);
}
free_Ustring(line);
free_Ustring(lower);
u_fclose(f);
return res;
}


void add_keyword(KeyWord* *list,unichar* keyword,int weight) {
if (*list==NULL) {
    *list=new_KeyWord(weight,keyword,NULL);
    return;
}
if (!u_strcmp((*list)->sequence,keyword)) {
    /* The keyword is already there, we just have to update its weight */
    (*list)->weight+=weight;
    return;
}
add_keyword(&((*list))->next,keyword,weight);
}


/**
 * Loads a compound word file, adding each word to the keywords.
 */
void load_compound_words(char* name,VersatileEncodingConfig* vec,
        struct string_hash_ptr* keywords) {
U_FILE* f=u_fopen(vec,name,U_READ);
if (f==NULL) return;
Ustring* line=new_Ustring(256);
Ustring* lower=new_Ustring(256);
while (EOF!=readline(line,f)) {
    if (line->str[0]=='{') {
        /* We skip tags */
        continue;
    }
    u_strcpy(lower,line->str);
    u_tolower(lower->str);
    int index=get_value_index(lower->str,keywords,INSERT_IF_NEEDED,NULL);
    if (index==-1) {
        fatal_error("Internal error in load_tokens_by_freq\n");
    }
    KeyWord* value=(KeyWord*)keywords->value[index];
    add_keyword(&value,line->str,1);
    keywords->value[index]=value;
}
free_Ustring(line);
free_Ustring(lower);
u_fclose(f);
}


/**
 * Removes every keyword that is not made of letters, by turning
 * the corresponding sequence value to NULL in the 'keywords' structure.
 *
 * DON'T CALL IT AFTER load_compound_words!! It would remove all of them
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



/**
 * Loads the given DELAF and modifies the given keywords accordingly by
 * replacing any non removed token that appear in a DELAF entry
 * by its lemma. If there are ambiguities, several keywords are
 * generated. Doing that may merge keywords by adding their weights:
 * eats/2 + eaten/3 => eat/5
 */
void filter_keywords_with_dic(struct string_hash_ptr* keywords,char* name,
                        VersatileEncodingConfig* vec,Alphabet* alphabet) {
U_FILE* f=u_fopen(vec,name,U_READ);
if (f==NULL) {
    error("Cannot load file %s\n",name);
    return;
}
Ustring* line=new_Ustring(128);
while (EOF!=readline(line,f)) {
    struct dela_entry* e=tokenize_DELAF_line(line->str);
    if (e==NULL) continue;
    lemmatize(e,keywords,alphabet);
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
            u_fprintf(f,"%d\t%S\n",k->weight,k->sequence);
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


int last_index_of(unichar* s,unichar c) {
if (s==NULL) return -1;
int pos=u_strlen(s)-1;
while (pos>=0) {
    if (s[pos]==c) return pos;
    pos--;
}
return pos;
}


/**
 * Looks for a lemmatized keyword of the form XXX.YYY where YYY
 * is the forbidden code. Then, it returns 1 and XXX is copied into
 * res. Otherwise, 0 is returned.
 */
int get_forbidden_keyword(KeyWord* list,unichar* code,Ustring* res) {
if (list==NULL) return 0;
int pos=last_index_of(list->sequence,(unichar)'.');
if (pos!=-1 && !u_strcmp(code,list->sequence+pos+1)) {
    /* If the forbidden code has been found */
    u_strcpy(res,list->sequence);
    truncate(res,pos);
    return 1;
}
return 0;
}


/**
 * Looks for a keyword that has a forbidden lemma or is a forbidden lemma
 * if the keyword is not a lemmatized one of the form XXX.YYY
 */
int has_forbidden_lemma(KeyWord* list,struct string_hash* lemmas) {
if (list==NULL || list->sequence==NULL) return 0;
int pos=last_index_of(list->sequence,(unichar)'.');
if (pos==-1) {
    /* If the keyword is not lemmatized, we just test
     * if it is a forbidden lemma
     */
    return (-1!=get_value_index(list->sequence,lemmas,DONT_INSERT));
}
Ustring* tmp=new_Ustring(list->sequence);
truncate(tmp,pos);
int index=get_value_index(tmp->str,lemmas,DONT_INSERT);
free_Ustring(tmp);
return index!=-1;
}


static KeyWord* remove_keywords_with_forbidden_lemma_(KeyWord* list,
                            struct string_hash* lemmas) {
if (list==NULL) return NULL;
if (has_forbidden_lemma(list,lemmas)) {
    /* If the forbidden lemma has been found */
    KeyWord* next=list->next;
    free_KeyWord(list);
    return remove_keywords_with_forbidden_lemma_(next,lemmas);
}
list->next=remove_keywords_with_forbidden_lemma_(list->next,lemmas);
return list;
}


/**
 * We remove every keyword that is tagged with the forbidden code. If
 * a forbidden keyword has several tags, all of them are removed:
 *
 * the,.DET + the,.XXX => all 'the' keywords are removed
 */
struct string_hash* compute_forbidden_lemmas(struct string_hash_ptr* keywords,unichar* code) {
struct string_hash* hash=new_string_hash(DONT_USE_VALUES,DONT_ENLARGE);
Ustring* tmp=new_Ustring();
for (int i=0;i<keywords->size;i++) {
    KeyWord* list=(KeyWord*)(keywords->value[i]);
    while (list!=NULL) {
        if (get_forbidden_keyword(list,code,tmp)) {
            get_value_index(tmp->str,hash);
        }
        list=list->next;
    }
}
free_Ustring(tmp);
return hash;
}


void remove_keywords_with_forbidden_lemma(struct string_hash_ptr* keywords,
                    struct string_hash* lemmas) {
for (int i=0;i<keywords->size;i++) {
    KeyWord* list=(KeyWord*)(keywords->value[i]);
    list=remove_keywords_with_forbidden_lemma_(list,lemmas);
    keywords->value[i]=list;
}
}

} // namespace unitex


