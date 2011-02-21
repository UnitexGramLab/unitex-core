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

#include "GermanCompounds.h"
#include "Error.h"
#include "Ustring.h"

void explore_state_german(int,unichar*,int,const unichar*,int,const unichar*,unichar*,
      struct german_word_decomposition_list**,int,const char*,const char*,const Alphabet*,
      Dictionary*,Ustring*,int);


//
// this function analyses the german compound words
//
void analyse_german_compounds(const Alphabet* alph,Dictionary* d,
                                 U_FILE* words,U_FILE* result,U_FILE* debug,U_FILE* new_unknown_words) {
char* tableau_correct_left_component=(char*)malloc(sizeof(char)*(d->inf->N));
if (tableau_correct_left_component==NULL) {
   fatal_alloc_error("analyse_german_compounds");
}
char* tableau_correct_right_component=(char*)malloc(sizeof(char)*(d->inf->N));
if (tableau_correct_right_component==NULL) {
   fatal_alloc_error("analyse_german_compounds");
}
check_valid_left_component_german(tableau_correct_left_component,d->inf);
check_valid_right_component_german(tableau_correct_right_component,d->inf);
analyse_german_word_list(d,words,result,debug,new_unknown_words,tableau_correct_left_component,
                         tableau_correct_right_component,alph);
free(tableau_correct_left_component);
free(tableau_correct_right_component);
}



//
// return 1 if at least one of the INF codes of l is a valid
// right component, 0 else
//
char check_valid_right_component_for_an_INF_line_german(struct list_ustring* l) {
while (l!=NULL) {
   if (check_valid_right_component_for_one_INF_code_german(l->string)) {
      return 1;
   }
   l=l->next;
}
return 0;
}



//
// this function check for all the INF codes that are:
// N+FF
//
void check_valid_right_component_german(char* tableau_correct_right_component,const struct INF_codes* inf) {
u_printf("Check valid right components...\n");
for (int i=0;i<inf->N;i++) {
   tableau_correct_right_component[i]=check_valid_right_component_for_an_INF_line_german(inf->codes[i]);
}
}



//
// this function check for all the INF codes that are:
// N but not FF
//
void check_valid_left_component_german(char* tableau_sia,const struct INF_codes* inf) {
u_printf("Check valid left components...\n");
for (int i=0;i<inf->N;i++) {
   tableau_sia[i]=check_valid_left_component_for_an_INF_line_german(inf->codes[i]);
}
}



//
// return 1 if at least one of the INF codes of l is a valid
// left component, 0 else
//
char check_valid_left_component_for_an_INF_line_german(const struct list_ustring* l) {
while (l!=NULL) {
   if (check_valid_left_component_for_one_INF_code_german(l->string)) {
      return 1;
   }
   l=l->next;
}
return 0;
}



//
// this function look for the first sia code of the line number n and stores it in s
//
void get_first_sia_code_german(int n,unichar* s,const struct INF_codes* inf_codes) {
// we initialize s to prevent errors, but this case should never happen
s[0]='\0';
struct list_ustring* l=inf_codes->codes[n];
while (l!=NULL) {
   if (check_valid_left_component_for_one_INF_code_german(l->string)) {
      u_strcpy(s,l->string);
      return;
   }
   l=l->next;
}
}



//
// returns 1 if the line is a N+FF one
//
char check_N_FF(const struct dela_entry* d) {
unichar t1[2];
u_strcpy(t1,"N");
unichar t2[3];
u_strcpy(t2,"FF");
return (char)(dic_entry_contain_gram_code(d,t1) && dic_entry_contain_gram_code(d,t2));
}



//
// returns 1 if the INF code refers to a valid left component, 0 else
//
char check_valid_left_component_for_one_INF_code_german(const unichar* s) {
unichar temp[2000];
u_strcpy(temp,"x,");
u_strcat(temp,s);
struct dela_entry* d=tokenize_DELAF_line(temp,0);
char res=check_N_FF(d);
free_dela_entry(d);
return res;
}



//
// returns 1 if the line is a N  but not FF one
//
char check_N_not_FF(const struct dela_entry* d) {
unichar t1[2];
u_strcpy(t1,"N");
unichar t2[3];
u_strcpy(t2,"FF");
return (char)(dic_entry_contain_gram_code(d,t1) && !(dic_entry_contain_gram_code(d,t2)));
}




//
// returns 1 if the INF code refers to a valid right component, 0 else
//
char check_valid_right_component_for_one_INF_code_german(const unichar* s) {
unichar temp[2000];
u_strcpy(temp,"x,");
u_strcat(temp,s);
struct dela_entry* d=tokenize_DELAF_line(temp,0);
char res=check_N_not_FF(d);
free_dela_entry(d);
return res;
}



//
// this function reads words in the word file and try analyse them
//
void analyse_german_word_list(Dictionary* d,
                              U_FILE* words,U_FILE* result,U_FILE* debug,U_FILE* new_unknown_words,
                              const char* left,const char* right,const Alphabet* alphabet) {
unichar s[1000];
u_printf("Analysing german unknown words...\n");
int n=0;
while (EOF!=u_fgets_limit2(s,1000,words)) {
  if (!analyse_german_word(s,debug,result,left,right,alphabet,d)) {
     // if the analysis has failed, we store the word in the new unknown word file
     u_fprintf(new_unknown_words,"%S\n",s);
  } else {n++;}
}
u_printf("%d words decomposed as compound words\n",n);
}



//
// this function try to analyse an unknown german word
//
int analyse_german_word(const unichar* mot,U_FILE* debug_file,U_FILE* result_file,const char* left,const char* right,
                        const Alphabet* alphabet,Dictionary* d) {
unichar decomposition[2000];
unichar dela_line[2000];
unichar correct_word[2000];
decomposition[0]='\0';
dela_line[0]='\0';
correct_word[0]='\0';
struct german_word_decomposition_list* l=NULL;
Ustring* ustr=new_Ustring();
explore_state_german(d->initial_state_offset,correct_word,0,mot,0,decomposition,dela_line,&l,1,
		left,right,alphabet,d,ustr,0);
free_Ustring(ustr);
if (l==NULL) {
   return 0;
}
struct german_word_decomposition_list* tmp=l;
while (tmp!=NULL) {
   if (debug_file!=NULL) {
      u_fprintf(debug_file,"%S = %S\n",mot,tmp->element->decomposition);
   }
   u_fprintf(result_file,"%S\n",tmp->element->dela_line);
   tmp=tmp->suivant;
}
if (l!=NULL) {
  free_german_word_decomposition_list(l);
}
return 1;
}




struct german_word_decomposition* new_german_word_decomposition() {
struct german_word_decomposition* tmp;
tmp=(struct german_word_decomposition*)malloc(sizeof(struct german_word_decomposition));
if (tmp==NULL) {
   fatal_alloc_error("new_german_word_decomposition");
}
tmp->n_parts=0;
tmp->decomposition[0]='\0';
tmp->dela_line[0]='\0';
return tmp;
}



void free_german_word_decomposition(struct german_word_decomposition* t) {
if (t==NULL) return;
free(t);
}



struct german_word_decomposition_list* new_german_word_decomposition_list() {
struct german_word_decomposition_list* tmp;
tmp=(struct german_word_decomposition_list*)malloc(sizeof(struct german_word_decomposition_list));
if (tmp==NULL) {
   fatal_alloc_error("new_german_word_decomposition_list");
}
tmp->element=NULL;
tmp->suivant=NULL;
return tmp;
}


void free_german_word_decomposition_list(struct german_word_decomposition_list* l) {
struct german_word_decomposition_list* tmp;
while (l!=NULL) {
   free_german_word_decomposition(l->element);
   tmp=l->suivant;
   free(l);
   l=tmp;
}
}



//
// this function explores the dictionary to decompose the word mot
//
void explore_state_german(int offset,unichar* current_component,int pos_in_current_component,
                   const unichar* original_word,int pos_in_original_word,const unichar* decomposition,
                   unichar* dela_line,struct german_word_decomposition_list** L,int n_decomp,
                   const char* left,const char* right,
                   const Alphabet* alphabet,
                   Dictionary* d,Ustring* ustr,int base) {
int final,n_transitions,inf_number;
int z=save_output(ustr);
offset=read_dictionary_state(d,offset,&final,&n_transitions,&inf_number);
if (final) {
  // if we are in a terminal state
  current_component[pos_in_current_component]='\0';
  if (pos_in_current_component>1) {
    // we don't consider words with a length of 1
    if (original_word[pos_in_original_word]=='\0') {
      // if we have explored the entire original word
      if (right[inf_number]) {
         // and if we have a valid right component
         struct list_ustring* l=d->inf->codes[inf_number];
         while (l!=NULL) {
            unichar dec[500];
            u_strcpy(dec,decomposition);
            if (dec[0]!='\0') {u_strcat(dec," +++ ");}
            unichar entry[500];
            uncompress_entry(current_component,l->string,entry);
            u_strcat(dec,entry);
            unichar new_dela_line[500];
            struct dela_entry* tmp_entry=tokenize_DELAF_line(entry,1);
            if (tmp_entry==NULL) {
               /* If there was an error in the dictionary, we skip the entry */
               l=l->next;
               continue;
            }
            // change case if there is a prefix
            // prefixes are downcase, nouns (=suffixes) uppercase:
            // "investitionsObjekte" -> "Investitionsobjekte"
            if ( u_strlen(dela_line) != 0 ) {
              // capitalize dela_line
              dela_line[0] = u_toupper((unichar) dela_line[0]);
              // downcase lemma and inflected
              tmp_entry->inflected[0] = u_tolower(tmp_entry->inflected[0]);
              tmp_entry->lemma[0] = u_tolower(tmp_entry->lemma[0]);
            }
            u_strcpy(new_dela_line,dela_line);
            u_strcat(new_dela_line,tmp_entry->inflected);
            u_strcat(new_dela_line,",");
            u_strcat(new_dela_line,dela_line);
            u_strcat(new_dela_line,tmp_entry->lemma);
            u_strcat(new_dela_line,".");
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
            struct german_word_decomposition* wd=new_german_word_decomposition();
            wd->n_parts=n_decomp;
            u_strcpy(wd->decomposition,dec);
            u_strcpy(wd->dela_line,new_dela_line);
            if (check_valid_right_component_for_one_INF_code_german(l->string)) {
               // if we got a correct right component (N-FF)
               struct german_word_decomposition_list* wdl=new_german_word_decomposition_list();
               wdl->element=wd;
               wdl->suivant=(*L);
               (*L)=wdl;
            } else {
               free_german_word_decomposition(wd);
            }
            l=l->next;
         }
      }
    }
    else {
      // else, we must explore the rest of the original word
      if (left[inf_number]) {
         // but only if the current component was a valid left one
         // we go on with the next component
         unichar dec[2000];
         unichar line[500];
         u_strcpy(dec,decomposition);
         if (dec[0]!='\0') {u_strcat(dec," +++ ");}
         unichar sia_code[500];
         unichar entry[500];
         get_first_sia_code_german(inf_number,sia_code,d->inf);
         uncompress_entry(current_component,sia_code,entry);
         u_strcat(dec,entry);
         u_strcpy(line,dela_line);
         u_strcat(line,current_component);
         unichar temp[500];
         Ustring* foo=new_Ustring();
         explore_state_german(d->initial_state_offset,temp,0,original_word,pos_in_original_word,
                  dec,line,L,n_decomp+1,left,right,alphabet,d,foo,0);
         free_Ustring(foo);
      }
    }
  }
  base=ustr->len;
}
if (original_word[pos_in_original_word]=='\0') {
   // if we have finished, we return
   return;
}
// if not, we go on with the next letter
unichar c;
int adr;
for (int i=0;i<n_transitions;i++) {
	offset=read_dictionary_transition(d,offset,&c,&adr,ustr);
  if (is_equal_or_uppercase(c,original_word[pos_in_original_word],alphabet)
      || is_equal_or_uppercase(original_word[pos_in_original_word],c,alphabet)) {
    current_component[pos_in_current_component]=c;
    explore_state_german(adr,current_component,pos_in_current_component+1,original_word,pos_in_original_word+1,
                  decomposition,dela_line,L,n_decomp,left,right,alphabet,d,ustr,base);
  }
  restore_output(z,ustr);
}
}
