 /*
  * Unitex
  *
  * Copyright (C) 2001-2010 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
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

#include "DutchCompounds.h"
#include "Error.h"
#include "List_ustring.h"


#define is_N 0
#define is_A 1
#define is_VP1s 2
#define is_ADV 3
#define INVALID_LEFT_COMPONENT 4


/**
 * This structure is used to englobe settings for the analysis of
 * Dutch unknown words.
 */
struct dutch_infos {
	/* The Dutch alphabet */
	Alphabet* alphabet;
	/* The .bin and .inf parts of the dictionary that will be used
	 * for the analysis */
	const unsigned char* bin;
	const struct INF_codes* inf;
	/* The file where to read the words to analyze from */
	U_FILE* unknown_word_list;
	/* The file where new dictionary lines will be written */
	U_FILE* output;
	/* The file where to print information about the analysis */
	U_FILE* info_output;
	/* The file where to write words that cannot be analyzed as
	 * compound words */
	U_FILE* new_unknown_word_list;
   /* The words that cannot appear in a decomposition, like single letters */
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
};


struct word_decomposition_list {
   struct word_decomposition* element;
   struct word_decomposition_list* next;
};


void analyse_dutch_unknown_words(struct dutch_infos*);
int analyse_dutch_word(unichar* word,struct dutch_infos*);
void explore_state_dutch(int offset,unichar* current_component,int pos_in_current_component,
                   unichar* word_to_analyze,int pos_in_word_to_analyze,unichar* analysis,
                   unichar* output_dela_line,struct word_decomposition_list** L,
                   int number_of_components,struct dutch_infos* infos);
void check_valid_right_component_dutch(char*,const struct INF_codes*);
char check_valid_right_component_for_an_INF_line_dutch(struct list_ustring*);
char check_valid_right_component_for_one_INF_code_dutch(unichar*);
void check_valid_left_component_dutch(char*,const struct INF_codes*);
char check_valid_left_component_for_an_INF_line_dutch(struct list_ustring*);
char check_valid_left_component_for_one_INF_code_dutch(unichar*);

struct word_decomposition* new_word_decomposition_dutch();
void free_word_decomposition_dutch(struct word_decomposition*);
struct word_decomposition_list* new_word_decomposition_list_dutch();
void free_word_decomposition_list_dutch(struct word_decomposition_list*);


/**
 * This function analyzes a list of unknown Dutch words.
 */
void analyse_dutch_unknown_words(Alphabet* alphabet,const unsigned char* bin,const struct INF_codes* inf,
								U_FILE* unknown_word_list,U_FILE* output,U_FILE* info_output,
								U_FILE* new_unknown_word_list,struct string_hash* forbidden_words) {
/* We create a structure that will contain all settings */
struct dutch_infos infos;
infos.alphabet=alphabet;
infos.bin=bin;
infos.inf=inf;
infos.unknown_word_list=unknown_word_list;
infos.forbidden_words=forbidden_words;
infos.output=output;
infos.info_output=info_output;
infos.new_unknown_word_list=new_unknown_word_list;
infos.valid_left_component=(char*)malloc(sizeof(char)*(inf->N));
if (infos.valid_left_component==NULL) {
	fatal_alloc_error("analyse_dutch_unknown_words");
}
infos.valid_right_component=(char*)malloc(sizeof(char)*(inf->N));
if (infos.valid_right_component==NULL) {
   fatal_alloc_error("analyse_dutch_unknown_words");
}
/* We look for all INF codes if they correspond to valid left/right
 * components of compounds words. */
check_valid_left_component_dutch(infos.valid_left_component,inf);
check_valid_right_component_dutch(infos.valid_right_component,inf);
/* Now we are ready to analyse the given word list */
analyse_dutch_unknown_words(&infos);
free(infos.valid_left_component);
free(infos.valid_right_component);
}


/**
 * This function checks all the INF codes of 'inf' and sets 'valid_left_component[i]'
 * to 1 if the i-th INF line contains at least one INF code that contains
 * the "N" grammatical code.
 */
void check_valid_right_component_dutch(char* valid_left_component,const struct INF_codes* inf) {
u_printf("Check valid right components...\n");
for (int i=0;i<inf->N;i++) {
   valid_left_component[i]=check_valid_right_component_for_an_INF_line_dutch(inf->codes[i]);
}
}


/**
 * Returns 1 if at least one of the INF codes of 'INF_codes' is a valid
 * right component, 0 otherwise.
 */
char check_valid_right_component_for_an_INF_line_dutch(struct list_ustring* INF_codes) {
while (INF_codes!=NULL) {
	if (check_valid_right_component_for_one_INF_code_dutch(INF_codes->string)) {
		return 1;
	}
	INF_codes=INF_codes->next;
}
return 0;
}


/**
 * Returns 1 if the given dictionary entry is a "N" one.
 */
char check_N_dutch(struct dela_entry* d) {
unichar t1[2];
u_strcpy(t1,"N");
return (char)dic_entry_contain_gram_code(d,t1);
}



/**
 * Returns 1 if the given dictionary entry is a "A" one.
 */
char check_A_dutch(struct dela_entry* d) {
unichar t1[2];
u_strcpy(t1,"A");
return (char)dic_entry_contain_gram_code(d,t1);
}


/**
 * Returns 1 if the given dictionary entry is a "V:P1s" one.
 */
char check_VP1s_dutch(struct dela_entry* d) {
unichar t1[2];
u_strcpy(t1,"V");
unichar t2[4];
u_strcpy(t2,"P1s");
return dic_entry_contain_gram_code(d,t1) && dic_entry_contain_inflectional_code(d,t2);
}


/**
 * Returns 1 if the given dictionary entry is a "ADV" one.
 */
char check_ADV_dutch(struct dela_entry* d) {
unichar t1[4];
u_strcpy(t1,"ADV");
return (char)dic_entry_contain_gram_code(d,t1);
}



/**
 * This function checks all the INF codes of 'inf' and sets 'valid_right_component[i]'
 * to 1 if the i-th INF line contains at least one INF code that
 * one of the following codes: "N:sia", "A:sio", "V:W" or "ADV".
 */
void check_valid_left_component_dutch(char* valid_right_component,const struct INF_codes* inf) {
u_printf("Check valid left components...\n");
for (int i=0;i<inf->N;i++) {
   valid_right_component[i]=check_valid_left_component_for_an_INF_line_dutch(inf->codes[i]);
}
}


/**
 * Returns 1 if at least one of the INF codes of 'INF_codes' is a valid
 * left component, 0 otherwise.
 */
char check_valid_left_component_for_an_INF_line_dutch(struct list_ustring* INF_codes) {
while (INF_codes!=NULL) {
	if (check_valid_left_component_for_one_INF_code_dutch(INF_codes->string)) {
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
int get_valid_left_component_type_for_one_INF_code_dutch(unichar* INF_code) {
/* We produce an artifical dictionary entry with the given INF code,
 * and then, we tokenize it in order to get grammatical and inflectional
 * codes in a structured way. */
unichar temp[2000];
u_strcpy(temp,"x,");
u_strcat(temp,INF_code);
struct dela_entry* d=tokenize_DELAF_line(temp,0);
int res;
/* Now we can test if the INF code corresponds to a valid left component */
if (check_N_dutch(d)) res=is_N;
else if (check_A_dutch(d)) res=is_A;
else if (check_VP1s_dutch(d)) res=is_VP1s;
else if (check_ADV_dutch(d)) res=is_ADV;
else res=INVALID_LEFT_COMPONENT;
/* Finally we free the artifical dictionary entry */
free_dela_entry(d);
return res;
}


/**
 * This function looks in the INF line number 'n' for the first INF code that
 * contains a valid left component.
 * 'code' is a string that will contains the selected code.
 **/
void get_first_valid_left_component_dutch(struct list_ustring* INF_codes,unichar* code) {
int tmp;
code[0]='\0';
while (INF_codes!=NULL) {
	tmp=get_valid_left_component_type_for_one_INF_code_dutch(INF_codes->string);
	if (tmp!=INVALID_LEFT_COMPONENT) {
		/* If we find a valid left component, then we return it */
		u_strcpy(code,INF_codes->string);
		return;
	}
	INF_codes=INF_codes->next;
}
}



/**
 * Returns 1 if the INF code refers to a valid left component, 0 otherwise.
 */
char check_valid_left_component_for_one_INF_code_dutch(unichar* INF_code) {
/* We produce an artifical dictionary entry with the given INF code,
 * and then, we tokenize it in order to get grammatical and inflectional
 * codes in a structured way. */
unichar temp[2000];
u_strcpy(temp,"x,");
u_strcat(temp,INF_code);
struct dela_entry* d=tokenize_DELAF_line(temp,0);
/* Now, we can use this structured representation to check if the INF code
 * corresponds to a valid left component. */
char res=check_N_dutch(d)||check_A_dutch(d)||check_VP1s_dutch(d)||check_ADV_dutch(d);
/* Finally, we free the artificial dictionary entry */
free_dela_entry(d);
return res;
}



/**
 * Returns 1 if the given INF code is a "N" one.
 */
char check_N_dutch(unichar* INF_code) {
/* We produce an artifical dictionary entry with the given INF code,
 * and then, we tokenize it in order to get grammatical and inflectional
 * codes in a structured way. */
unichar temp[2000];
u_strcpy(temp,"x,");
u_strcat(temp,INF_code);
struct dela_entry* d=tokenize_DELAF_line(temp,0);
char res=check_N_dutch(d);
/* We free the artifical dictionary entry */
free_dela_entry(d);
return res;
}




/**
 * Returns 1 if the INF code refers to a valid left component, 0 otherwise.
 */
char check_valid_right_component_for_one_INF_code_dutch(unichar* INF_code) {
/* We produce an artifical dictionary entry with the given INF code,
 * and then, we tokenize it in order to get grammatical and inflectional
 * codes in a structured way. */
unichar temp[2000];
u_strcpy(temp,"x,");
u_strcat(temp,INF_code);
struct dela_entry* d=tokenize_DELAF_line(temp,0);
char res=check_N_dutch(d);
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
void analyse_dutch_unknown_words(struct dutch_infos* infos) {
unichar line[10000];
u_printf("Analysing Dutch unknown words...\n");
int n=0;
/* We read each line of the unknown word list and we try to analyze it */
while (EOF!=u_fgets_limit2(line,10000,infos->unknown_word_list)) {
  if (!analyse_dutch_word(line,infos)) {
     /* If the analysis has failed, we store the word in the
      * new unknown word file */
     u_fprintf(infos->new_unknown_word_list,"%S\n",line);
  } else {
  		/* Otherwise, we increase the number of analyzed words */
  		n++;
  	}
}
u_printf("%d words decomposed as compound word%s\n",n,(n>1)?"s":"");
}


/**
 * This function tries to analyse an unknown Dutch word. If OK,
 * it returns 1 and print the dictionary entry to the output (and
 * information if an information file has been specified in 'infos');
 * returns 0 otherwise.
 */
int analyse_dutch_word(unichar* word,struct dutch_infos* infos) {
unichar decomposition[4096];
unichar dela_line[4096];
unichar correct_word[4096];
decomposition[0]='\0';
dela_line[0]='\0';
correct_word[0]='\0';
struct word_decomposition_list* l=NULL;
/* We look if there are decompositions for this word */
explore_state_dutch(4,correct_word,0,word,0,decomposition,dela_line,&l,1,infos);
if (l==NULL) {
	/* If there is no decomposition, we return */
	return 0;
}
/* Otherwise, we will choose the one to keep */
struct word_decomposition_list* tmp=l;
int n=1000;
/* First, we count the minimal number of components, because
 * we want to give priority to analysis with smallest number
 * of components. By the way, we note if there is a minimal
 * analysis ending by a noun or an adjective. */
while (tmp!=NULL) {
	if (tmp->element->n_parts<=n) {
		n=tmp->element->n_parts;
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
		if (infos->info_output!=NULL) {
			u_fprintf(infos->info_output,"%S = %S\n",word,tmp->element->decomposition);
		}
		u_fprintf(infos->output,"%S\n",tmp->element->dela_line);
	}
	tmp=tmp->next;
}
free_word_decomposition_list_dutch(l);
return 1;
}


/**
 * Allocates, initializes and returns a word decomposition structure.
 */
struct word_decomposition* new_word_decomposition_dutch() {
struct word_decomposition* tmp;
tmp=(struct word_decomposition*)malloc(sizeof(struct word_decomposition));
if (tmp==NULL) {
   fatal_alloc_error("new_word_decomposition_dutch");
}
tmp->n_parts=0;
tmp->decomposition[0]='\0';
tmp->dela_line[0]='\0';
return tmp;
}


/**
 * Frees a word decomposition structure.
 */
void free_word_decomposition_dutch(struct word_decomposition* t) {
if (t==NULL) return;
free(t);
}


/**
 * Allocates, initializes and returns a word decomposition list structure.
 */
struct word_decomposition_list* new_word_decomposition_list_dutch() {
struct word_decomposition_list* tmp;
tmp=(struct word_decomposition_list*)malloc(sizeof(struct word_decomposition_list));
if (tmp==NULL) {
   fatal_alloc_error("new_word_decomposition_list_dutch");
}
tmp->element=NULL;
tmp->next=NULL;
return tmp;
}


/**
 * Frees a word decomposition list.
 */
void free_word_decomposition_list_dutch(struct word_decomposition_list* l) {
struct word_decomposition_list* tmp;
while (l!=NULL) {
	free_word_decomposition_dutch(l->element);
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
void explore_state_dutch(int offset,unichar* current_component,int pos_in_current_component,
                   unichar* word_to_analyze,int pos_in_word_to_analyze,unichar* analysis,
                   unichar* output_dela_line,struct word_decomposition_list** L,
                   int number_of_components,struct dutch_infos* infos) {
int c;
int index,t;
c=infos->bin[offset]*256+infos->bin[offset+1];
if (!(c&32768)) {
	/* If we are in a final state, we compute the index of the
	 * corresponding INF line */
	index=infos->bin[offset+2]*256*256+infos->bin[offset+3]*256+infos->bin[offset+4];
	/* We can set the end of our current component */
	current_component[pos_in_current_component]='\0';
	/* We do not consider forbidden words */
   if (infos->forbidden_words==NULL || NO_VALUE_INDEX==get_value_index(current_component,infos->forbidden_words,DONT_INSERT)) {
		/* We don't consider components with a length of 1 */
		if (word_to_analyze[pos_in_word_to_analyze]=='\0') {
			/* If we have explored the entire original word */
   		/* And if we do not have forbidden word in last position */
			struct list_ustring* l=infos->inf->codes[index];
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
				/*
				 * Now we can build an analysis in the form of a word decomposition
				 * structure, but only if the last component is a valid
				 * right one or if it is a verb long enough, or if we find out
				 * that the word to analyze was in fact a simple word
				 * in the dictionary */
				if (check_valid_right_component_for_one_INF_code_dutch(l->string)
					|| number_of_components==1) {
					/*
					 * We set the number of components, the analysis, the actual
					 * DELA line and information about
					 */
					struct word_decomposition* wd=new_word_decomposition_dutch();
					wd->n_parts=number_of_components;
					u_strcpy(wd->decomposition,dec);
					u_strcpy(wd->dela_line,new_dela_line);
					/* Then we add the decomposition word structure to the list that
					 * contains all the analysis for the word to analyze */
					struct word_decomposition_list* wdl=new_word_decomposition_list_dutch();
					wdl->element=wd;
					wdl->next=(*L);
					(*L)=wdl;
				}
				/* We go on with the next INF code of the last component */
				l=l->next;
			}
			/* If are at the end of the word to analyze, we have nothing more to do */
			return;
		} else {
			/* If we are not at the end of the word to analyze, we must
			 * 1) look if the current component is a valid left one
			 * 2) explore the rest of the original word
			 */
			if (infos->valid_left_component[index]) {
				/* If we have a valid component, we look first if we are
				 * in the case of a word followed by a "s" */
				if (word_to_analyze[pos_in_word_to_analyze]=='s') {
               /* If we have such a word, we add it to the current analysis,
					 * putting "+++ s +++*/
					unichar dec[2000];
					u_strcpy(dec,analysis);
					if (dec[0]!='\0') {
						u_strcat(dec," +++");
					}
					/* In order to print the component in the analysis, we arbitrary
					 * take a valid left component among all those that are available
					 * for the current component */
					unichar sia_code[2000];
					unichar entry[2000];
					unichar line[2000];
					get_first_valid_left_component_dutch(infos->inf->codes[index],sia_code);
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
               u_strcat(dec_temp," +++ s");
					/* Then, we explore the dictionary in order to analyze the
					 * next component. We start at the root of the dictionary
					 * (offset=4) and we go back one position in the word to analyze.
					 */
					explore_state_dutch(4,temp,0,word_to_analyze,pos_in_word_to_analyze+1,
						dec_temp,line,L,number_of_components+1,infos);
				}
				/* Now, we try to analyze the component normally */
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
				get_first_valid_left_component_dutch(infos->inf->codes[index],sia_code);
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
				explore_state_dutch(4,temp,0,word_to_analyze,pos_in_word_to_analyze,
					dec_temp,line,L,number_of_components+1,infos);
			}
		}
	}
	/* Once we have finished to deal with the current final dictionary node,
	 * we go on because we may match a longer word */
	t=offset+5;
}
else {
	/* If the node is not a final one, we get compute the number of transitions by
	 * removing the highest bit */
	c=c-32768;
	t=offset+2;
}
/* We examine each transition that goes out from the node */
for (int i=0;i<c;i++) {
	if (is_equal_or_uppercase((unichar)(infos->bin[t]*256+infos->bin[t+1]),word_to_analyze[pos_in_word_to_analyze],infos->alphabet)) {
		/* If the transition's letter is case compatible with the current letter of the
		 * word to analyze, we follow it */
		index=infos->bin[t+2]*256*256+infos->bin[t+3]*256+infos->bin[t+4];
		current_component[pos_in_current_component]=(unichar)(infos->bin[t]*256+infos->bin[t+1]);
		explore_state_dutch(index,current_component,pos_in_current_component+1,word_to_analyze,pos_in_word_to_analyze+1,
			analysis,output_dela_line,L,number_of_components,infos);
	}
	/* We move the offset to the next transition */
	t=t+5;
}
}
