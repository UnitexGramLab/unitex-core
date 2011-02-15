/*
 * Unitex
 *
 * Copyright (C) 2001-2011 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "NorwegianCompounds.h"
#include "Error.h"
#include "List_ustring.h"


/**
 * As the PolyLex program was originaly designed for Norwegian,
 * complete information about the analysis of Norwegian words can be
 * found in:
 *
 * Paumier Sébastien & Harald Ulland, 2005. Analyse automatique de mots
 * polylexicaux en norvégien. Lingvistic� Investigationes 28:2,
 * Amsterdam-Philadelphia : John Benjamins Publishing Company.
 */


#define N_SIA 0
#define N_SIE 1
#define N_SIG 2
#define A_SIO 3
#define A_SIE 4
#define V_W 5
#define ADV 6
#define INVALID_LEFT_COMPONENT 7


/**
 * This structure is used to englobe settings for the analysis of
 * Norwegian unknown words.
 */
struct norwegian_infos {
	/* The norwegian alphabet */
	const Alphabet* alphabet;
	Dictionary* d;
	/* The file where to read the words to analyze from */
	U_FILE* unknown_word_list;
	/* The file where new dictionary lines will be written */
	U_FILE* output;
	/* The file where to print information about the analysis */
	U_FILE* info_output;
	/* The file where to write words that cannot be analyzed as
	 * compound words */
	U_FILE* new_unknown_word_list;
	/* Set of words that cannot be part of compound words */
	struct string_hash* forbidden_words;
	/* These arrays indicates for each INF code if it can be
	 * viewed as a valid code for a left/right component of a
	 * compound word. */
	char* valid_left_component;
	char* valid_right_component;
};


struct word_decomposition {
   int n_parts;
   unichar decomposition[4096];
   unichar dela_line[4096];
   int is_a_valid_right_N;
   int is_a_valid_right_A;
};


struct word_decomposition_list {
   struct word_decomposition* element;
   struct word_decomposition_list* next;
};


void analyse_norwegian_unknown_words(struct norwegian_infos*);
int analyse_norwegian_word(const unichar* word,struct norwegian_infos*);
void explore_state(int,unichar*,int,const unichar*,int,const unichar*,const unichar*,
					struct word_decomposition_list**,int,struct norwegian_infos*);
void check_valid_right_component(char*,const struct INF_codes*);
void check_valid_left_component(char*,const struct INF_codes*);
char check_valid_left_component_for_an_INF_line(const struct list_ustring*);
char check_valid_left_component_for_one_INF_code(const unichar*);
char check_valid_right_component_for_an_INF_line(const struct list_ustring*);
char check_valid_right_component_for_one_INF_code(const unichar*);
char check_Nsia(const struct dela_entry*);
char check_Nsie(const struct dela_entry*);
char check_Nsig(const struct dela_entry*);
char check_Asio(const struct dela_entry*);
char check_Asie(const struct dela_entry*);
char check_VW(const struct dela_entry*);
char check_ADV(const struct dela_entry*);
struct word_decomposition* new_word_decomposition();
void free_word_decomposition(struct word_decomposition*);
struct word_decomposition_list* new_word_decomposition_list();
void free_word_decomposition_list(struct word_decomposition_list*);


/**
 * This function analyzes a list of unknown Norwegian words.
 */
void analyse_norwegian_unknown_words(const Alphabet* alphabet,Dictionary* d,
								U_FILE* unknown_word_list,U_FILE* output,U_FILE* info_output,
								U_FILE* new_unknown_word_list,
								struct string_hash* forbidden_words) {
/* We create a structure that will contain all settings */
struct norwegian_infos infos;
infos.alphabet=alphabet;
infos.d=d;
infos.unknown_word_list=unknown_word_list;
infos.output=output;
infos.info_output=info_output;
infos.new_unknown_word_list=new_unknown_word_list;
infos.forbidden_words=forbidden_words;
infos.valid_left_component=(char*)malloc(sizeof(char)*(d->inf->N));
if (infos.valid_left_component==NULL) {
	fatal_alloc_error("analyse_norwegian_unknown_words");
}
infos.valid_right_component=(char*)malloc(sizeof(char)*(d->inf->N));
if (infos.valid_right_component==NULL) {
   fatal_alloc_error("analyse_norwegian_unknown_words");
}
/* We look for all INF codes if they correspond to valid left/right
 * components of compounds words. */
check_valid_left_component(infos.valid_left_component,d->inf);
check_valid_right_component(infos.valid_right_component,d->inf);
/* Now we are ready to analyse the given word list */
analyse_norwegian_unknown_words(&infos);
free(infos.valid_left_component);
free(infos.valid_right_component);
}


/**
 * This function checks all the INF codes of 'inf' and sets 'valid_left_component[i]'
 * to 1 if the i-th INF line contains at least one INF code that contains
 * one of the following grammatical codes: "N", "A" or "V".
 */
void check_valid_right_component(char* valid_left_component,const struct INF_codes* inf) {
u_printf("Check valid right components...\n");
for (int i=0;i<inf->N;i++) {
   valid_left_component[i]=check_valid_right_component_for_an_INF_line(inf->codes[i]);
}
}


/**
 * Returns 1 if at least one of the INF codes of 'INF_codes' is a valid
 * right component, 0 otherwise.
 */
char check_valid_right_component_for_an_INF_line(const struct list_ustring* INF_codes) {
while (INF_codes!=NULL) {
	if (check_valid_right_component_for_one_INF_code(INF_codes->string)) {
		return 1;
	}
	INF_codes=INF_codes->next;
}
return 0;
}


/**
 * This function checks all the INF codes of 'inf' and sets 'valid_right_component[i]'
 * to 1 if the i-th INF line contains at least one INF code that
 * one of the following codes: "N:sia", "A:sio", "V:W" or "ADV".
 */
void check_valid_left_component(char* valid_right_component,const struct INF_codes* inf) {
u_printf("Check valid left components...\n");
for (int i=0;i<inf->N;i++) {
   valid_right_component[i]=check_valid_left_component_for_an_INF_line(inf->codes[i]);
}
}


/**
 * Returns 1 if at least one of the INF codes of 'INF_codes' is a valid
 * left component, 0 otherwise.
 */
char check_valid_left_component_for_an_INF_line(const struct list_ustring* INF_codes) {
while (INF_codes!=NULL) {
	if (check_valid_left_component_for_one_INF_code(INF_codes->string)) {
		return 1;
	}
	INF_codes=INF_codes->next;
}
return 0;
}


/**
 * This function analyzes an INF code and returns a value that indicates
 * if it is a valid left component or not.
 */
int get_valid_left_component_type_for_one_INF_code(const unichar* INF_code) {
/* We produce an artifical dictionary entry with the given INF code,
 * and then, we tokenize it in order to get grammatical and inflectional
 * codes in a structured way. */
unichar temp[2000];
u_strcpy(temp,"x,");
u_strcat(temp,INF_code);
struct dela_entry* d=tokenize_DELAF_line(temp,0);
int res;
/* Now we can test if the INF code corresponds to a valid left component */
if (check_Nsia(d)) res=N_SIA;
else if (check_Nsie(d)) res=N_SIE;
else if (check_Nsig(d)) res=N_SIG;
else if (check_Asio(d)) res=A_SIO;
else if (check_Asie(d)) res=A_SIE;
else if (check_VW(d)) res=V_W;
else if (check_ADV(d)) res=ADV;
else res=INVALID_LEFT_COMPONENT;
/* Finally we free the artifical dictionary entry */
free_dela_entry(d);
return res;
}


/**
 * This function looks in the INF line number 'n' for the first INF code that
 * contains a valid left component. However, if there is one of the codes
 * "N:sia", "N:sie" or "N:sig", they will have the priority.
 * 'code' is a string that will contains the selected code.
 **/
void get_first_valid_left_component(struct list_ustring* INF_codes,unichar* code) {
int tmp;
code[0]='\0';
while (INF_codes!=NULL) {
	tmp=get_valid_left_component_type_for_one_INF_code(INF_codes->string);
	if (tmp==N_SIA || tmp==N_SIE || tmp==N_SIG) {
		/* If we find an N, then we return it */
		u_strcpy(code,INF_codes->string);
		return;
	}
	if (tmp!=INVALID_LEFT_COMPONENT) {
		/* If we find a valid left component, then we copy it,
		 * but we do not return now, since we can find later
		 * a N that has an higher priority */
		u_strcpy(code,INF_codes->string);
	}
	INF_codes=INF_codes->next;
}
}


/**
 * Returns 1 if the given dictionary entry is a "N:sia" one.
 */
char check_Nsia(const struct dela_entry* d) {
unichar t1[2];
u_strcpy(t1,"N");
unichar t2[4];
u_strcpy(t2,"sia");
return dic_entry_contain_gram_code(d,t1) && dic_entry_contain_inflectional_code(d,t2);
}


/**
 * Returns 1 if the given dictionary entry is a "N:sie" one.
 */
char check_Nsie(const struct dela_entry* d) {
unichar t1[2];
u_strcpy(t1,"N");
unichar t2[4];
u_strcpy(t2,"sie");
return dic_entry_contain_gram_code(d,t1) && dic_entry_contain_inflectional_code(d,t2);
}


/**
 * Returns 1 if the given dictionary entry is a "N:sig" one.
 */
char check_Nsig(const struct dela_entry* d) {
unichar t1[2];
u_strcpy(t1,"N");
unichar t2[4];
u_strcpy(t2,"sig");
return dic_entry_contain_gram_code(d,t1) && dic_entry_contain_inflectional_code(d,t2);
}


/**
 * Returns 1 if the given dictionary entry is a "A:sio" one.
 */
char check_Asio(const struct dela_entry* d) {
unichar t1[2];
u_strcpy(t1,"A");
unichar t2[4];
u_strcpy(t2,"sio");
return dic_entry_contain_gram_code(d,t1) && dic_entry_contain_inflectional_code(d,t2);
}


/**
 * Returns 1 if the given dictionary entry is a "A:sie" one.
 */
char check_Asie(const struct dela_entry* d) {
unichar t1[2];
u_strcpy(t1,"A");
unichar t2[4];
u_strcpy(t2,"sie");
return dic_entry_contain_gram_code(d,t1) && dic_entry_contain_inflectional_code(d,t2);
}


/**
 * Returns 1 if the given dictionary entry is a "V:W" one.
 */
char check_VW(const struct dela_entry* d) {
unichar t1[2];
u_strcpy(t1,"V");
unichar t2[2];
u_strcpy(t2,"W");
return dic_entry_contain_gram_code(d,t1) && dic_entry_contain_inflectional_code(d,t2);
}


/**
 * Returns 1 if the given dictionary entry is a "ADV" one.
 */
char check_ADV(const struct dela_entry* d) {
unichar t1[4];
u_strcpy(t1,"ADV");
return (char)dic_entry_contain_gram_code(d,t1);
}


/**
 * Returns 1 if the given dictionary entry is a "V" one that does
 * not have the inflectional code "Y".
 */
char check_V_but_not_Y(struct dela_entry* d) {
unichar t1[2];
u_strcpy(t1,"V");
unichar t2[2];
u_strcpy(t2,"Y");
return dic_entry_contain_gram_code(d,t1) && (!dic_entry_contain_inflectional_code(d,t2));
}


/**
 * Returns 1 if the INF code refers to a valid left component, 0 otherwise.
 */
char check_valid_left_component_for_one_INF_code(const unichar* INF_code) {
/* We produce an artifical dictionary entry with the given INF code,
 * and then, we tokenize it in order to get grammatical and inflectional
 * codes in a structured way. */
unichar temp[2000];
u_strcpy(temp,"x,");
u_strcat(temp,INF_code);
struct dela_entry* d=tokenize_DELAF_line(temp,0);
/* Now, we can use this structured representation to check if the INF code
 * corresponds to a valid left component. */
char res=check_Nsia(d)||check_Nsie(d)||check_Nsig(d)||check_Asio(d)||check_Asie(d)||check_VW(d)||check_ADV(d);
/* Finally, we free the artificial dictionary entry */
free_dela_entry(d);
return res;
}


/**
 * Returns 1 if the given dictionary entry is a "N" one.
 */
char check_N(struct dela_entry* d) {
unichar t1[2];
u_strcpy(t1,"N");
return (char)dic_entry_contain_gram_code(d,t1);
}


/**
 * Returns 1 if the line is a valid right "N" component.
 */
char check_N_right_component(unichar* s) {
/* We produce an artifical dictionary entry with the given INF code,
 * and then, we tokenize it in order to get grammatical and inflectional
 * codes in a structured way. */
unichar temp[2000];
u_strcpy(temp,"x,");
u_strcat(temp,s);
struct dela_entry* d=tokenize_DELAF_line(temp,0);
unichar t1[2];
u_strcpy(t1,"N");
unichar t2[4];
u_strcpy(t2,"sie");
char res=dic_entry_contain_gram_code(d,t1) && !dic_entry_contain_inflectional_code(d,t2);
/* We free the artifical dictionary entry */
free_dela_entry(d);
return res;
}


/**
 * Returns 1 if the line is a valid right "A" component.
 */
char check_A_right_component(unichar* s) {
/* We produce an artifical dictionary entry with the given INF code,
 * and then, we tokenize it in order to get grammatical and inflectional
 * codes in a structured way. */
unichar temp[2000];
u_strcpy(temp,"x,");
u_strcat(temp,s);
struct dela_entry* d=tokenize_DELAF_line(temp,0);
unichar t1[2];
u_strcpy(t1,"A");
unichar t2[4];
u_strcpy(t2,"sie");
char res=dic_entry_contain_gram_code(d,t1) && !dic_entry_contain_inflectional_code(d,t2);
/* We free the artifical dictionary entry */
free_dela_entry(d);
return res;
}


/**
 * Returns 1 if the given dictionary entry is a ":a" one.
 */
char check_a(struct dela_entry* d) {
unichar t1[2];
u_strcpy(t1,"a");
return (char)dic_entry_contain_inflectional_code(d,t1);
}


/**
 * Returns 1 if the given INF code is a "N" one.
 */
char check_N(unichar* INF_code) {
/* We produce an artifical dictionary entry with the given INF code,
 * and then, we tokenize it in order to get grammatical and inflectional
 * codes in a structured way. */
unichar temp[2000];
u_strcpy(temp,"x,");
u_strcat(temp,INF_code);
struct dela_entry* d=tokenize_DELAF_line(temp,0);
char res=check_N(d);
/* We free the artifical dictionary entry */
free_dela_entry(d);
return res;
}


/**
 * Returns 1 if the given INF code is a ":a" one.
 */
char check_a(unichar* INF_code) {
/* We produce an artifical dictionary entry with the given INF code,
 * and then, we tokenize it in order to get grammatical and inflectional
 * codes in a structured way. */
unichar temp[2000];
u_strcpy(temp,"x,");
u_strcat(temp,INF_code);
struct dela_entry* d=tokenize_DELAF_line(temp,0);
char res=check_a(d);
/* We free the artifical dictionary entry */
free_dela_entry(d);
return res;
}


/**
 * Returns 1 if the given dictionary entry is a "A" one.
 */
char check_A(struct dela_entry* d) {
unichar t1[2];
u_strcpy(t1,"A");
return (char)dic_entry_contain_gram_code(d,t1);
}



/**
 * Returns 1 if the INF code refers to a valid left component, 0 otherwise.
 */
char check_valid_right_component_for_one_INF_code(const unichar* INF_code) {
/* We produce an artifical dictionary entry with the given INF code,
 * and then, we tokenize it in order to get grammatical and inflectional
 * codes in a structured way. */
unichar temp[2000];
u_strcpy(temp,"x,");
u_strcat(temp,INF_code);
struct dela_entry* d=tokenize_DELAF_line(temp,0);
char res=(check_N(d)||check_A(d)/*||check_V_but_not_Y(d)*/)&&(!check_Nsie(d));
/* We free the artifical dictionary entry */
free_dela_entry(d);
return res;
}


/**
 * Returns 1 if the dictionary line refers to a verb with more than 4
 * letters and 0 otherwise.
 */
char verb_of_more_than_4_letters(unichar* line) {
/* We produce an artifical dictionary entry with the given INF code,
 * and then, we tokenize it in order to get grammatical and inflectional
 * codes in a structured way. */
struct dela_entry* d=tokenize_DELAF_line(line,0);
char res=check_V_but_not_Y(d) && u_strlen(d->inflected)>4;
/* We free the artifical dictionary entry */
free_dela_entry(d);
return res;
}


/**
 * This function reads words from the unknown word file and tries to
 * analyse them. The unknown word file is supposed to contain one word
 * per line. If a word cannot be analyzed, we print it to the new
 * unknown word list file.
 */
void analyse_norwegian_unknown_words(struct norwegian_infos* infos) {
unichar line[10000];
u_printf("Analysing norwegian unknown words...\n");
int n=0;
/* We read each line of the unknown word list and we try to analyze it */
while (EOF!=u_fgets_limit2(line,10000,infos->unknown_word_list)) {
  if (!analyse_norwegian_word(line,infos)) {
     /* If the analysis has failed, we store the word in the
      * new unknown word file */
     u_fprintf(infos->new_unknown_word_list,"%S\n",line);
  } else {
  		/* Otherwise, we increase the number of analyzed words */
  		n++;
  	}
}
u_printf("%d words decomposed as compound words\n",n);
}


/**
 * This function tries to analyse an unknown norwegian word. If OK,
 * it returns 1 and print the dictionary entry to the output (and
 * information if an information file has been specified in 'infos');
 * returns 0 otherwise.
 */
int analyse_norwegian_word(const unichar* word,struct norwegian_infos* infos) {
unichar decomposition[4096];
unichar dela_line[4096];
unichar correct_word[4096];
decomposition[0]='\0';
dela_line[0]='\0';
correct_word[0]='\0';
struct word_decomposition_list* l=NULL;
/* We look if there are decompositions for this word */
explore_state(4,correct_word,0,word,0,decomposition,dela_line,&l,1,infos);
if (l==NULL) {
	/* If there is no decomposition, we return */
	return 0;
}
/* Otherwise, we will choose the one to keep */
struct word_decomposition_list* tmp=l;
int n=1000;
int is_a_valid_right_N=0;
int is_a_valid_right_A=0;
/* First, we count the minimal number of components, because
 * we want to give priority to analysis with smallest number
 * of components. By the way, we note if there is a minimal
 * analysis ending by a noun or an adjective. */
while (tmp!=NULL) {
	if (tmp->element->n_parts<=n) {
		if (tmp->element->n_parts<n) {
			/* If we change of component number, we reset the
			 * 'is_a_valid_right_N' and 'is_a_valid_right_A' fields,
			 * because they only concern the head word. */
			is_a_valid_right_N=0;
			is_a_valid_right_A=0;
		}
		n=tmp->element->n_parts;
		if (tmp->element->is_a_valid_right_N) {
			is_a_valid_right_N=1;
		}
		if (tmp->element->is_a_valid_right_A) {
			is_a_valid_right_A=1;
		}
	}
	tmp=tmp->next;
}
tmp=l;
while (tmp!=NULL) {
	if (n==tmp->element->n_parts) {
		/* We only consider the words that have shortest decompositions.
		 * The test (tmp->element->n_parts==1) is used to
		 * match simple words that would have been wrongly considered
		 * as unknown words. */
		int OK=0;
		if (tmp->element->n_parts==1) {
			/* Simple words must be matched */
			OK=1;
		}
		else if (is_a_valid_right_N) {
			 	if (tmp->element->is_a_valid_right_N) {
					/* We give priority to analysis that ends with a noun */
					OK=1;
			 	}
			}
		else if (is_a_valid_right_A) {
				if (tmp->element->is_a_valid_right_A) {
					/* Our second priority goes to analysis that ends with an adjective */
					OK=1;
				}
			}
		else OK=1;
		/* We put a restriction on the grammatical code:
		 * we don't produce a x<A> or x<V> analysis when a x<N> exists */
		if (OK) {
			if (infos->info_output!=NULL) {
				u_fprintf(infos->info_output,"%S = %S\n",word,tmp->element->decomposition);
			}
			u_fprintf(infos->output,"%S\n",tmp->element->dela_line);
		}
	}
	tmp=tmp->next;
}
free_word_decomposition_list(l);
return 1;
}


/**
 * Allocates, initializes and returns a word decomposition structure.
 */
struct word_decomposition* new_word_decomposition() {
struct word_decomposition* tmp;
tmp=(struct word_decomposition*)malloc(sizeof(struct word_decomposition));
if (tmp==NULL) {
   fatal_alloc_error("new_word_decomposition");
}
tmp->n_parts=0;
tmp->decomposition[0]='\0';
tmp->dela_line[0]='\0';
tmp->is_a_valid_right_N=0;
tmp->is_a_valid_right_A=0;
return tmp;
}


/**
 * Frees a word decomposition structure.
 */
void free_word_decomposition(struct word_decomposition* t) {
if (t==NULL) return;
free(t);
}


/**
 * Allocates, initializes and returns a word decomposition list structure.
 */
struct word_decomposition_list* new_word_decomposition_list() {
struct word_decomposition_list* tmp;
tmp=(struct word_decomposition_list*)malloc(sizeof(struct word_decomposition_list));
if (tmp==NULL) {
   fatal_alloc_error("new_word_decomposition_list");
}
tmp->element=NULL;
tmp->next=NULL;
return tmp;
}


/**
 * Frees a word decomposition list.
 */
void free_word_decomposition_list(struct word_decomposition_list* l) {
struct word_decomposition_list* tmp;
while (l!=NULL) {
	free_word_decomposition(l->element);
	tmp=l->next;
	free(l);
	l=tmp;
}
}


/**
 * This explores the dictionary in order decompose the given word into a valid sequence
 * of simple words. For instance, if we have the word "Sommervarmt", we will first
 * explore the dictionary and find that "sommer" is a valid left component that
 * corresponds to the dictionary entry "sommer,.N:msia". Then we will
 * look if the following word "varmt" is in the dictionary. It is
 * the case, with the entry "varmt,varm.A:nsio". As we are at the end of the word to
 * analyze and as "varmt" is a valid rightmost component, we will generate an entry
 * according to the following things:
 *
 * 'output_dela_line'="sommervarmt,sommervarm.A:nsio"
 * 'analysis'="sommer,.N:msia +++ varmt,varm.A:nsio"
 * 'number_of_components'=2
 *
 * Note that the initial "S" was put in lowercase, because the dictionary
 * contains "sommer" and not "Sommer". The lemma is obtained with
 * the lemma of the rightmost component (here "varm"), and the word inherits
 * from the grammatical information of its rightmost component.
 *
 * 'offset': offset of the current node in the binary array 'infos->bin'
 * 'current_component': string that represents the current simple word
 * 'pos_in_current_component': position in the string 'current_component'
 * 'word_to_analyze': the word to analyze
 * 'pos_in_word_to_analyze': position in the string 'word_to_analyze'
 * 'analysis': string that represents the analysis as a concatenation like
 *             "sommer,.N:msia +++ varmt,varm.A:nsio"
 * 'output_dela_line': string that contains the final DELA line. The lemma is
 *                     obtained by replacing the rightmost term of
 *                     the word to analyze by its lemma.
 * 'L': list of all analysis for the given word
 * 'number_of_components': number of components that compose the word.
 * 'infos': global settings.
 */
void explore_state(int offset,unichar* current_component,int pos_in_current_component,
                   const unichar* word_to_analyze,int pos_in_word_to_analyze,const unichar* analysis,
                   const unichar* output_dela_line,struct word_decomposition_list** L,
                   int number_of_components,struct norwegian_infos* infos) {
int final,n_transitions,inf_number;
offset=read_dictionary_state(infos->d,offset,&final,&n_transitions,&inf_number);
if (final) {
	/* If we are in a final state, we can set the end of our current component */
	current_component[pos_in_current_component]='\0';
	/* We do not consider words of length 1 */
	if (pos_in_current_component>1) {
		/* We don't consider components with a length of 1 */
		if (word_to_analyze[pos_in_word_to_analyze]=='\0') {
			/* If we have explored the entire original word */
			if (get_value_index(current_component,infos->forbidden_words,DONT_INSERT)==NO_VALUE_INDEX) {
				/* And if we do not have forbidden word in last position */
				struct list_ustring* l=infos->d->inf->codes[inf_number];
				/* We will look at all the INF codes of the last component in order
				 * to produce analysis */
				while (l!=NULL) {
					unichar dec[2000];
					u_strcpy(dec,analysis);
					if (dec[0]!='\0') {
						/* If we have already something in the analysis (i.e. if
						 * we have not a simple word), we insert the concatenation
						 * mark before the entry to come */
						u_strcat(dec," +++ ");
					}
					unichar entry[2000];
					/* We get the dictionary line that corresponds to the current INF code */
					uncompress_entry(current_component,l->string,entry);
					/* And we add it to the analysis */
					u_strcat(dec,entry);
					unichar new_dela_line[2000];
					/* We copy the current output DELA line that contains
					 * the concatenation of the previous components */
					u_strcpy(new_dela_line,output_dela_line);
					/* Then we tokenize the DELA line that corresponds the current INF
					 * code in order to obtain its lemma and grammatical/inflectional
					 * information */
					struct dela_entry* tmp_entry=tokenize_DELAF_line(entry,1);
					/* We concatenate the inflected form of the last component to
					 * the output DELA line */
					u_strcat(new_dela_line,tmp_entry->inflected);
					/* We put the comma that separates the inflected form and the lemma */
					u_strcat(new_dela_line,",");
					/* And we build the lemma in the same way than the inflected form */
					u_strcat(new_dela_line,output_dela_line);
					u_strcat(new_dela_line,tmp_entry->lemma);
					/* We put the dot that separates the the lemma and the grammatical/inflectional
					 * information */
					u_strcat(new_dela_line,".");
					/* And finally we put the grammatical/inflectional information */
					u_strcat(new_dela_line,tmp_entry->semantic_codes[0]);
               int k;
               for (k=1;k<tmp_entry->n_semantic_codes;k++) {
                  u_strcat(new_dela_line,"+");
                  u_strcat(new_dela_line,tmp_entry->semantic_codes[k]);
               }
               for (k=0;k<tmp_entry->n_inflectional_codes;k++) {
                  u_strcat(new_dela_line,":");
                  u_strcat(new_dela_line,tmp_entry->inflectional_codes[k]);
               }
					free_dela_entry(tmp_entry);
					/*
					 * Now we can build an analysis in the form of a word decomposition
					 * structure, but only if the last component is a valid
					 * right one or if it is a verb long enough, or if we find out
					 * that the word to analyze was in fact a simple word
					 * in the dictionary */
					if (verb_of_more_than_4_letters(entry)
						|| check_valid_right_component_for_one_INF_code(l->string)
						|| number_of_components==1) {
						/*
						 * We set the number of components, the analysis, the actual
						 * DELA line and information about
						 */
						struct word_decomposition* wd=new_word_decomposition();
						wd->n_parts=number_of_components;
						u_strcpy(wd->decomposition,dec);
						u_strcpy(wd->dela_line,new_dela_line);
						wd->is_a_valid_right_N=check_N_right_component(l->string);
						wd->is_a_valid_right_A=check_A_right_component(l->string);
						/* Then we add the decomposition word structure to the list that
						 * contains all the analysis for the word to analyze */
						struct word_decomposition_list* wdl=new_word_decomposition_list();
						wdl->element=wd;
						wdl->next=(*L);
						(*L)=wdl;
					}
					/* We go on with the next INF code of the last component */
					l=l->next;
				}
			}
			/* If are at the end of the word to analyze, we have nothing more to do */
			return;
		} else {
			/* If we are not at the end of the word to analyze, we must
			 * 1) look if the current component is a valid left one
			 * 2) look if it is not a forbidden component and
			 * 3) explore the rest of the original word
			 */
			if (infos->valid_left_component[inf_number] &&
				(get_value_index(current_component,infos->forbidden_words,DONT_INSERT)==NO_VALUE_INDEX)) {
				/* If we have a valid component, we look first if we are
				 * in the case of a word ending by a double letter like "kupp" */
				if (pos_in_current_component>2 &&
					(current_component[pos_in_current_component-1]==current_component[pos_in_current_component-2])) {
					/* If we have such a word, we add it to the current analysis,
					 * putting "+++" if the current component is not the first one */
					unichar dec[2000];
					u_strcpy(dec,analysis);
					if (dec[0]!='\0') {
						u_strcat(dec," +++ ");
					}
					/* In order to print the component in the analysis, we arbitrary
					 * take a valid left component among all those that are available
					 * for the current component */
					unichar sia_code[2000];
					unichar entry[2000];
					unichar line[2000];
					get_first_valid_left_component(infos->d->inf->codes[inf_number],sia_code);
					uncompress_entry(current_component,sia_code,entry);
					u_strcat(dec,entry);
					u_strcpy(line,output_dela_line);
					u_strcat(line,current_component);
					/* As we have a double letter at the end of the word,
					 * we must remove a character */
					line[u_strlen(line)-1]='\0';
					unichar temp[2000];
					unichar dec_temp[2000];
					u_strcpy(dec_temp,dec);
					/* Then, we explore the dictionary in order to analyze the
					 * next component. We start at the root of the dictionary
					 * (offset=4) and we go back one position in the word to analyze.
					 * For instance, if we have "kupplaner", we read "kupp" and then
					 * we try to analyze "planner". */
					explore_state(4,temp,0,word_to_analyze,pos_in_word_to_analyze-1,
						dec_temp,line,L,number_of_components+1,infos);
				}
				/* Now, we try to analyze the component normally, even if
				 * it was ended by double letter, because we can have things
				 * like "oppbrent = opp,.ADV +++ brent,brenne.V:K" */
				unichar dec[2000];
				unichar line[2000];
				u_strcpy(dec,analysis);
				if (dec[0]!='\0') {
					/* We add the "+++" mark if the current component is not the first one */
					u_strcat(dec," +++ ");
				}
				unichar sia_code[2000];
				unichar entry[2000];
				/* In order to print the component in the analysis, we arbitrary
				 * take a valid left component among all those that are available
				 * for the current component */
				get_first_valid_left_component(infos->d->inf->codes[inf_number],sia_code);
				uncompress_entry(current_component,sia_code,entry);
				u_strcat(dec,entry);
				u_strcpy(line,output_dela_line);
				u_strcat(line,current_component);
				unichar temp[2000];
				unichar dec_temp[2000];
				u_strcpy(dec_temp,dec);
				/* Then, we explore the dictionary in order to analyze the
				 * next component. We start at the root of the dictionary
				 * (offset=4). */
				explore_state(4,temp,0,word_to_analyze,pos_in_word_to_analyze,
					dec_temp,line,L,number_of_components+1,infos);
			}
		}
	}
	/* Once we have finished to deal with the current final dictionary node,
	 * we go on because we may match a longer word */
}
/* We examine each transition that goes out from the node */
unichar c;
int adr;
for (int i=0;i<c;i++) {
	offset=read_dictionary_transition(infos->d,offset,&c,&adr);
	if (is_equal_or_uppercase(c,word_to_analyze[pos_in_word_to_analyze],infos->alphabet)) {
		/* If the transition's letter is case compatible with the current letter of the
		 * word to analyze, we follow it */
		current_component[pos_in_current_component]=c;
		explore_state(adr,current_component,pos_in_current_component+1,word_to_analyze,pos_in_word_to_analyze+1,
			analysis,output_dela_line,L,number_of_components,infos);
	}
}
}
