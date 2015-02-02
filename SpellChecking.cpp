/*
 * Unitex
 *
 * Copyright (C) 2001-2015 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "SpellChecking.h"
#include "Ustring.h"
#include "CompressedDic.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

int default_scores[N_SPSubOp]={5,50,5,50,10,10,10,20,50};


typedef struct SPhypothesis {
	unichar* entry;
	int n_errors;
	int n_insert;
	int n_suppr;
	int n_swap;
	int n_change;
	vector_int* pairs;
	int score;
	struct SPhypothesis* next;
} SpellCheckHypothesis;


static SpellCheckHypothesis* new_SpellCheckHypothesis(unichar* entry,int current_errors,
		int current_SP_INSERT,int current_SP_SUPPR,int current_SP_SWAP,int current_SP_CHANGE,
		vector_int* pairs,int score[],SpellCheckHypothesis* next);
static void free_SpellCheckHypothesis(SpellCheckHypothesis* h);
static SpellCheckHypothesis* get_hypotheses(unichar* word,SpellCheckConfig* cfg);
static void filter_hypotheses(SpellCheckHypothesis* *list);
static void display_hypotheses(unichar* word,SpellCheckHypothesis* list,SpellCheckConfig* cfg);
static void free_hypotheses(SpellCheckHypothesis* list);
static void explore_dic(int offset,unichar* word,int pos_word,Dictionary* d,SpellCheckConfig* cfg,
		Ustring* output,SpellCheckHypothesis* *list,int base,Ustring* inflected);


/**
 * Performs spellchecking using the given configuration.
 */
void spellcheck(SpellCheckConfig* cfg) {
Ustring* line=new_Ustring(256);
cfg->tmp=new_Ustring(256);
cfg->inflected=new_Ustring(256);
cfg->output=new_Ustring(256);
cfg->pairs=new_vector_int(16);
while (EOF!=readline(line,cfg->in)) {
	if (line->str[0]=='\0') {
		/* Ignoring empty lines */
		continue;
	}
	SpellCheckHypothesis* list=get_hypotheses(line->str,cfg);
	filter_hypotheses(&list);
	display_hypotheses(line->str,list,cfg);
	free_hypotheses(list);
}
free_vector_int(cfg->pairs);
free_Ustring(line);
free_Ustring(cfg->tmp);
free_Ustring(cfg->inflected);
free_Ustring(cfg->output);
}


/**
 * Computes all hypotheses for the given word.
 */
static SpellCheckHypothesis* get_hypotheses(unichar* word,SpellCheckConfig* cfg) {
cfg->current_errors=0;
cfg->current_SP_INSERT=0;
cfg->current_SP_SUPPR=0;
cfg->current_SP_SWAP=0;
cfg->current_SP_CHANGE=0;
SPhypothesis* list=NULL;
int old_errors=cfg->max_errors;
if (u_strlen(word)<cfg->min_length1) {
	cfg->max_errors=0;
} else if (u_strlen(word)<cfg->min_length2) {
	if (cfg->max_errors>=1) cfg->max_errors=1;
} else if (u_strlen(word)<cfg->min_length3) {
	if (cfg->max_errors>=2) cfg->max_errors=2;
}
for (int i=0;i<cfg->n_dics;i++) {
	empty(cfg->output);
	empty(cfg->inflected);
	cfg->pairs->nbelems=0;
	explore_dic(cfg->dics[i]->initial_state_offset,word,0,cfg->dics[i],cfg,cfg->output,&list,0,cfg->inflected);
}
cfg->max_errors=old_errors;
return list;
}


/**
 * Deals with the matches associated to the current word.
 */
static void deal_with_matches(Dictionary* d,unichar* inflected,int inf_code,Ustring* output,
		SpellCheckConfig* cfg,int base,SpellCheckHypothesis* *list) {
struct list_ustring* inf_codes=NULL;
int should_free=get_inf_codes(d,inf_code,output,&inf_codes,base);
if (inf_codes==NULL) {
	fatal_error("Internal error in deal_with_matches: no inf codes associated to %S (base=%d,output=%S)\n",
			inflected,base,output->str);
}
struct list_ustring* tmp=inf_codes;
while (tmp!=NULL) {
	uncompress_entry(inflected,tmp->string,cfg->tmp);
	*list=new_SpellCheckHypothesis(cfg->tmp->str,cfg->current_errors,cfg->current_SP_INSERT,
			cfg->current_SP_SUPPR,cfg->current_SP_SWAP,cfg->current_SP_CHANGE,cfg->pairs,
			cfg->score,*list);
	tmp=tmp->next;
}
if (should_free) free_list_ustring(inf_codes);
}


/**
 * Returns 1 if the conditions are met to decide there is a swap
 * operation; 0 otherwise.
 *
 * c is the new character to be added to inflected.
 */
static int is_letter_swap(SpellCheckConfig* cfg,unichar* word,int pos_word,Ustring* inflected,unichar c) {
if (pos_word<1) return 0;
if (word[pos_word]==word[pos_word-1]) return 0;
if (c!=word[pos_word-1]) return 0;
if (inflected->len==0 || (inflected->str[inflected->len-1]!=word[pos_word])) return 0;
/* The previous operation must a SP_CHANGE_XXX one */
if (cfg->pairs->nbelems==0 ||
		!(cfg->pairs->tab[cfg->pairs->nbelems-2]==pos_word-1
				&& (cfg->pairs->tab[cfg->pairs->nbelems-1]==SP_CHANGE_DIACRITIC
						|| cfg->pairs->tab[cfg->pairs->nbelems-1]==SP_CHANGE_CASE
						|| cfg->pairs->tab[cfg->pairs->nbelems-1]==SP_CHANGE_KEYBOARD
						|| cfg->pairs->tab[cfg->pairs->nbelems-1]==SP_CHANGE_DEFAULT))) {
	return 0;
}
return 1;
}


/**
 * Explores the given dictionary to match the given word.
 */
static void explore_dic(int offset,unichar* word,int pos_word,Dictionary* d,SpellCheckConfig* cfg,
		Ustring* output,SpellCheckHypothesis* *list,int base,Ustring* inflected) {
int original_offset=offset;
int original_base=base;
int final,n_transitions,inf_code;
int z=save_output(output);
int size_pairs=cfg->pairs->nbelems;
offset=read_dictionary_state(d,offset,&final,&n_transitions,&inf_code);
if (final) {
	if (word[pos_word]=='\0') {
		/* If we have a match */
		deal_with_matches(d,inflected->str,inf_code,output,cfg,base,list);
	}
	base=output->len;
}
/* If we are at the end of the token, then we stop */
if (word[pos_word]=='\0') {
	return;
}
unsigned int l2=inflected->len;
unichar c;
int dest_offset;
for (int i=0;i<n_transitions;i++) {
	restore_output(z,output);
	offset=read_dictionary_transition(d,offset,&c,&dest_offset,output);
	/* For backup_output, see comment below */
	int backup_output=save_output(output);
	if (c==word[pos_word] || word[pos_word]==u_toupper(c)) {
		u_strcat(inflected,c);
		explore_dic(dest_offset,word,pos_word+1,d,cfg,output,list,base,inflected);
	} else {
		/* We deal with the SP_SWAP case, made of 2 SP_CHANGE_XXX */
		if (cfg->current_errors!=cfg->max_errors && cfg->current_SP_SWAP!=cfg->max_SP_SWAP
				&& is_letter_swap(cfg,word,pos_word,inflected,c)) {
			/* We don't modify the number of errors since we override an existing
			 * SP_CHANGE_XXX one */
			cfg->current_SP_SWAP++;
			/* We override the previous change */
			int a=cfg->pairs->tab[cfg->pairs->nbelems-2];
			int b=cfg->pairs->tab[cfg->pairs->nbelems-1];
			cfg->pairs->tab[cfg->pairs->nbelems-2]=pos_word-1;
			cfg->pairs->tab[cfg->pairs->nbelems-1]=SP_SWAP_DEFAULT;
			u_strcat(inflected,c);
			explore_dic(dest_offset,word,pos_word+1,d,cfg,output,list,base,inflected);
			cfg->pairs->tab[cfg->pairs->nbelems-2]=a;
			cfg->pairs->tab[cfg->pairs->nbelems-1]=b;
			cfg->current_SP_SWAP--;
		} else /* We deal with the SP_CHANGE case */
		       if (cfg->current_errors!=cfg->max_errors && cfg->current_SP_CHANGE!=cfg->max_SP_CHANGE
				/* We want letters, not spaces or anything else */
				&& is_letter(c,NULL)
		        /* We do not allow the replacement of a lowercase letter by an uppercase
		         * letter at the beginning of the word like Niserable, unless the whole word
		         * is in uppercase or the letter is the same, module the case */
		        && (cfg->allow_uppercase_initial || pos_word>0 || (!is_upper(word[0],NULL) || is_upper(word[1],NULL) || word[0]==u_toupper(c)))) {
			cfg->current_errors++;
			cfg->current_SP_CHANGE++;
			/* Now we test all possible kinds of change */
			vector_int_add(cfg->pairs,pos_word);
			u_strcat(inflected,c);
			/* We always add the default case */
			vector_int_add(cfg->pairs,SP_CHANGE_DEFAULT);
			int n_elem=cfg->pairs->nbelems;
			explore_dic(dest_offset,word,pos_word+1,d,cfg,output,list,base,inflected);
			/* Then we test the accent case */
			if (u_deaccentuate(c)==u_deaccentuate(word[pos_word])) {
				/* After a call to explore_dic, we must restore the output.
				 * But, when dealing with SP_CHANGE_XXX ops, we must restore the
				 * output including the output associated to the current transition,
				 * which is why we don't use z (output before the current transition)
				 * but backup_output */
				restore_output(backup_output,output);
			    cfg->pairs->nbelems=n_elem;
			    cfg->pairs->tab[cfg->pairs->nbelems-1]=SP_CHANGE_DIACRITIC;
			    explore_dic(dest_offset,word,pos_word+1,d,cfg,output,list,base,inflected);
			}
			/* And the case variations */
			if (u_tolower(c)==u_tolower(word[pos_word])) {
			    restore_output(backup_output,output);
			    cfg->pairs->nbelems=n_elem;
				cfg->pairs->tab[cfg->pairs->nbelems-1]=SP_CHANGE_CASE;
				explore_dic(dest_offset,word,pos_word+1,d,cfg,output,list,base,inflected);
			}
			/* And finally the position on keyboard */
			if (areCloseOnKeyboard(c,word[pos_word],cfg->keyboard)) {
			    restore_output(backup_output,output);
			    cfg->pairs->nbelems=n_elem;
				cfg->pairs->tab[cfg->pairs->nbelems-1]=SP_CHANGE_KEYBOARD;
				explore_dic(dest_offset,word,pos_word+1,d,cfg,output,list,base,inflected);
			}
			cfg->pairs->nbelems=size_pairs;
			cfg->current_errors--;
			cfg->current_SP_CHANGE--;
			/* End of the SP_CHANGE case */
		}
	}
    restore_output(backup_output,output);
	truncate(inflected,l2);
	/* Now we deal with the SP_SUPPR case */
	if (cfg->current_errors!=cfg->max_errors && cfg->current_SP_SUPPR!=cfg->max_SP_SUPPR
		/* We want letters, not spaces or anything else */
		&& is_letter(c,NULL)) {
		cfg->current_errors++;
		cfg->current_SP_SUPPR++;
		vector_int_add(cfg->pairs,pos_word);
		if (pos_word>=1 && c==word[pos_word-1]) {
			vector_int_add(cfg->pairs,SP_SUPPR_DOUBLE);
		} else {
			vector_int_add(cfg->pairs,SP_SUPPR_DEFAULT);
		}
		u_strcat(inflected,c);
		explore_dic(dest_offset,word,pos_word,d,cfg,output,list,original_base,inflected);
		truncate(inflected,l2);
		cfg->pairs->nbelems=size_pairs;
		cfg->current_errors--;
		cfg->current_SP_SUPPR--;
	}
}
restore_output(z,output);
/* Finally, we deal with the SP_INSERT case, by calling again the current
 * function with the same parameters, except pos_word that will be increased of 1 */
if (cfg->current_errors!=cfg->max_errors && cfg->current_SP_INSERT!=cfg->max_SP_INSERT
	/* We want letters, not spaces or anything else */
	&& is_letter(word[pos_word],NULL)
	/* We do not allow the insertion of a capital letter at the beginning of
	 * the word like Astreet, unless the whole word is in uppercase like ASTREET */
    && (cfg->allow_uppercase_initial || pos_word>0 || (!is_upper(word[0],NULL) || is_upper(word[1],NULL)))) {
	cfg->current_errors++;
	cfg->current_SP_INSERT++;
	vector_int_add(cfg->pairs,pos_word);
	if (pos_word>=1 && word[pos_word]==word[pos_word-1]) {
		vector_int_add(cfg->pairs,SP_INSERT_DOUBLE);
	} else {
		vector_int_add(cfg->pairs,SP_INSERT_DEFAULT);
	}
	explore_dic(original_offset,word,pos_word+1,d,cfg,output,list,original_base,inflected);
	truncate(inflected,l2);
	cfg->pairs->nbelems=size_pairs;
	cfg->current_errors--;
	cfg->current_SP_INSERT--;
}
/* Finally, we restore the output as it was when we enter the function */
restore_output(z,output);
}


/**
 * Removes all hypotheses that have more than n errors
 */
static SpellCheckHypothesis* remove_uninteresting_hypotheses(SpellCheckHypothesis* h,int n,int score) {
if (h==NULL) return NULL;
if (h->n_errors>n || h->score>score) {
	/* We have to remove the current hypothesis */
	SpellCheckHypothesis* tmp=h->next;
	free_SpellCheckHypothesis(h);
	return remove_uninteresting_hypotheses(tmp,n,score);
}
h->next=remove_uninteresting_hypotheses(h->next,n,score);
return h;
}


/**
 * Keeps best hypotheses.
 */
static void filter_hypotheses(SpellCheckHypothesis* *list) {
if (*list==NULL) return;
/* First of all, we compute the lowest number of errors */
int min=(*list)->n_errors;
int score_min=(*list)->score;
SpellCheckHypothesis* tmp=(*list)->next;
while (tmp!=NULL) {
	if (tmp->n_errors<min) min=tmp->n_errors;
	if (tmp->score<score_min) score_min=tmp->score;
	tmp=tmp->next;
}
/* Then, we remove all hypotheses involving more than 'min' errors,
 * or with a score greater than the minimum score */
*list=remove_uninteresting_hypotheses(*list,min,score_min);
}


/**
 * Prints the given hypotheses to the output, and if needed,
 * print the word to the modified input file.
 */
static void display_hypotheses(unichar* word,SpellCheckHypothesis* list,SpellCheckConfig* cfg) {
Ustring* line=new_Ustring(128);
int printed=0;
while (list!=NULL) {
	printed=1;
	struct dela_entry* entry=tokenize_DELAF_line(list->entry);
	if (entry==NULL) {
		fatal_error("Internal error in display_hypotheses; cannot tokenize entry:\n%S\n",list->entry);
	}
	unichar* inflected=entry->inflected;
	entry->inflected=u_strdup(word);
	entry->semantic_codes[entry->n_semantic_codes++]=u_strdup("SP_ERR");
	u_sprintf(line,"SP_INF=%S",inflected);
	entry->semantic_codes[entry->n_semantic_codes++]=u_strdup(line->str);
	dela_entry_to_string(line,entry);
	u_fprintf(cfg->out,"%S/score=%d\n",line->str,list->score);
	free(inflected);
	free_dela_entry(entry);
	list=list->next;
}
free_Ustring(line);
/* Now, we may have to print the word to the modified input file */
if (cfg->input_op=='M') {
	/* If we must keep matched words, then we print the word if it had matched */
	if (printed) u_fprintf(cfg->modified_input,"%S\n",word);
} else if (cfg->input_op=='U') {
	/* If we must keep unmatched words, then we print the word if it had matched */
	if (!printed) u_fprintf(cfg->modified_input,"%S\n",word);
}
}


/**
 * Creates a new hypothesis.
 */
SpellCheckHypothesis* new_SpellCheckHypothesis(unichar* entry,int current_errors,
		int current_SP_INSERT,int current_SP_SUPPR,int current_SP_SWAP,int current_SP_CHANGE,
		vector_int* pairs,int score[],SpellCheckHypothesis* next) {
SpellCheckHypothesis* h=(SpellCheckHypothesis*)malloc(sizeof(SpellCheckHypothesis));
h->entry=u_strdup(entry);
h->n_errors=current_errors;
h->n_insert=current_SP_INSERT;
h->n_suppr=current_SP_SUPPR;
h->n_swap=current_SP_SWAP;
h->n_change=current_SP_CHANGE;
h->pairs=vector_int_dup(pairs);
h->score=0;
for (int i=0;i<h->pairs->nbelems;i+=2) {
	h->score=h->score+score[h->pairs->tab[i+1]];
}
h->next=next;
return h;
}


/**
 * Frees all the memory associated to the given hypothesis,
 * but NOT to its next elements, if any.
 */
static void free_SpellCheckHypothesis(SpellCheckHypothesis* h) {
if (h==NULL) return;
free(h->entry);
free_vector_int(h->pairs);
free(h);
}


/**
 * Frees all the memory associated to the given hypothesis list.
 */
static void free_hypotheses(SpellCheckHypothesis* l) {
SpellCheckHypothesis* tmp;
while (l!=NULL) {
	tmp=l;
	l=l->next;
	free_SpellCheckHypothesis(tmp);
}
}


/**
 * Initializes the score array from the given values.
 */
void init_scores(int* score,int s1,int s2,int s3,int s4,int s5,int s6,int s7,int s8,int s9) {
score[SP_INSERT_DOUBLE]=s1;
score[SP_INSERT_DEFAULT]=s2;
score[SP_SUPPR_DOUBLE]=s3;
score[SP_SUPPR_DEFAULT]=s4;
score[SP_SWAP_DEFAULT]=s5;
score[SP_CHANGE_DIACRITIC]=s6;
score[SP_CHANGE_CASE]=s7;
score[SP_CHANGE_KEYBOARD]=s8;
score[SP_CHANGE_DEFAULT]=s9;
}

} // namespace unitex
