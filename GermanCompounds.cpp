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
#include "Compounds.h"
#include "GermanCompounds.h"
//---------------------------------------------------------------------------

Alphabet* german_alphabet;

//
// this function analyses the german compound words
//
void analyse_german_compounds(Alphabet* alph,unsigned char* bin,struct INF_codes* inf,
                                 FILE* words,FILE* result,FILE* debug,FILE* new_unknown_words) {
tableau_correct_left_component=(char*)malloc(sizeof(char)*(inf->N));
tableau_correct_right_component=(char*)malloc(sizeof(char)*(inf->N));
check_valid_left_component_german(tableau_correct_left_component,inf);
check_valid_right_component_german(tableau_correct_right_component,inf);
german_alphabet=alph;
inf_codes=inf;
analyse_german_word_list(bin,inf,words,result,debug,new_unknown_words);
free(tableau_correct_left_component);
free(tableau_correct_right_component);
}



//
// return 1 if at least one of the INF codes of l is a valid
// right component, 0 else
//
char check_valid_right_component_for_an_INF_line_german(struct token_list* l) {
while (l!=NULL) {
   if (check_valid_right_component_for_one_INF_code_german(l->token)) {
      return 1;
   }
   l=l->suivant;
}
return 0;
}



//
// this function check for all the INF codes that are:
// N+FF
//
void check_valid_right_component_german(char* tableau_sia,struct INF_codes* inf) {
printf("Check valid right components...\n");
for (int i=0;i<inf->N;i++) {
   tableau_correct_right_component[i]=check_valid_right_component_for_an_INF_line_german(inf->tab[i]);
}
}



//
// this function check for all the INF codes that are:
// N but not FF
//
void check_valid_left_component_german(char* tableau_sia,struct INF_codes* inf) {
printf("Check valid left components...\n");
for (int i=0;i<inf->N;i++) {
   tableau_sia[i]=check_valid_left_component_for_an_INF_line_german(inf->tab[i]);
}
}



//
// return 1 if at least one of the INF codes of l is a valid
// left component, 0 else
//
char check_valid_left_component_for_an_INF_line_german(struct token_list* l) {
while (l!=NULL) {
   if (check_valid_left_component_for_one_INF_code_german(l->token)) {
      return 1;
   }
   l=l->suivant;
}
return 0;
}



//
// this function look for the first sia code of the line number n and stores it in s
//
void get_first_sia_code_german(int n,unichar* s) {
// we initialize s to prevent errors, but this case should never happen
s[0]='\0';
struct token_list* l=inf_codes->tab[n];
while (l!=NULL) {
   if (check_valid_left_component_for_one_INF_code_german(l->token)) {
      u_strcpy(s,l->token);
      return;
   }
   l=l->suivant;
}
}



//
// returns 1 if the line is a N+FF one
//
char check_N_FF(dic_entry* d) {
unichar t1[2];
u_strcpy_char(t1,"N");
unichar t2[3];
u_strcpy_char(t2,"FF");
return (char)(dic_entry_contain_gram_code(d,t1) && dic_entry_contain_gram_code(d,t2));
}



//
// returns 1 if the INF code refers to a valid left component, 0 else
//
char check_valid_left_component_for_one_INF_code_german(unichar* s) {
unichar temp[2000];
u_strcpy_char(temp,"x,");
u_strcat(temp,s);
dic_entry* d=tokenize_DELA_line(temp);
char res=check_N_FF(d);
free_dic_entry(d);
return res;
}



//
// returns 1 if the line is a N  but not FF one
//
char check_N_not_FF(dic_entry* d) {
unichar t1[2];
u_strcpy_char(t1,"N");
unichar t2[3];
u_strcpy_char(t2,"FF");
return (char)(dic_entry_contain_gram_code(d,t1) && !(dic_entry_contain_gram_code(d,t2)));
}




//
// returns 1 if the INF code refers to a valid right component, 0 else
//
char check_valid_right_component_for_one_INF_code_german(unichar* s) {
unichar temp[2000];
u_strcpy_char(temp,"x,");
u_strcat(temp,s);
dic_entry* d=tokenize_DELA_line(temp);
char res=check_N_not_FF(d);
free_dic_entry(d);
return res;
}




//
// this function reads words in the word file and try analyse them
//
void analyse_german_word_list(unsigned char* bin,struct INF_codes* inf,
                                 FILE* words,FILE* result,FILE* debug,FILE* new_unknown_words) {
unichar s[1000];
tableau_bin=bin;
debug_file=debug;
result_file=result;
printf("Analysing german unknown words...\n");
int n=0;
while (next_word(words,s)) {
  if (!analyse_german_word(s)) {
     // if the analysis has failed, we store the word in the new unknown word file
     u_fprints(s,new_unknown_words);
     u_fprints_char("\n",new_unknown_words);
  } else {n++;}
}
printf("%d words decomposed as compound words\n",n);
}



//
// this function try to analyse an unknown german word
//
int analyse_german_word(unichar* mot) {
unichar decomposition[2000];
unichar dela_line[2000];
unichar correct_word[2000];
decomposition[0]='\0';
dela_line[0]='\0';
correct_word[0]='\0';
struct german_word_decomposition_list* l=NULL;
explore_state_german(4,correct_word,0,mot,0,decomposition,dela_line,&l,1);
if (l==NULL) {
   return 0;
}
struct german_word_decomposition_list* tmp=l;
while (tmp!=NULL) {
   {
      {
         if (debug_file!=NULL) {
            u_fprints(mot,debug_file);
            u_fprints_char(" = ",debug_file);
            u_fprints(tmp->element->decomposition,debug_file);
            u_fprints_char("\n",debug_file);
         }
         u_fprints(tmp->element->dela_line,result_file);
         u_fprints_char("\n",result_file);
      }
   }
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
void explore_state_german(int adresse,unichar* current_component,int pos_in_current_component,
                   unichar* original_word,int pos_in_original_word,unichar* decomposition,
                   unichar* dela_line,struct german_word_decomposition_list** L,int n_decomp) {
int c;
int index,t;
c=tableau_bin[adresse]*256+tableau_bin[adresse+1];
if (!(c&32768)) {
  // if we are in a terminal state
  index=tableau_bin[adresse+2]*256*256+tableau_bin[adresse+3]*256+tableau_bin[adresse+4];
  current_component[pos_in_current_component]='\0';
  if (pos_in_current_component>1) {
    // we don't consider words with a length of 1
    if (original_word[pos_in_original_word]=='\0') {
      // if we have explored the entire original word
      if (tableau_correct_right_component[index]) {
         // and if we have a valid right component
         struct token_list* l=inf_codes->tab[index];
         while (l!=NULL) {
            unichar dec[500];
            u_strcpy(dec,decomposition);
            if (dec[0]!='\0') {u_strcat_char(dec," +++ ");}
            unichar entry[500];
            uncompress_entry(current_component,l->token,entry);
            u_strcat(dec,entry);
            unichar inflected[500];
            unichar lemma[500];
            unichar codes[500];
            unichar new_dela_line[500];
            tokenize_DELA_line_into_3_parts(entry,inflected,lemma,codes);
            // change case if there is a prefix
            // prefixes are downcase, nouns (=suffixes) uppercase:
            // "investitionsObjekte" -> "Investitionsobjekte"
            if ( u_strlen(dela_line) != 0 ) {
              // capitalize dela_line
              dela_line[0] = u_toupper((unichar) dela_line[0]);
              // downcase lemma and inflected
              inflected[0] = u_tolower((unichar) inflected[0]);
              lemma[0] = u_tolower((unichar) lemma[0]);
            }
            u_strcpy(new_dela_line,dela_line);
            u_strcat(new_dela_line,inflected);
            u_strcat_char(new_dela_line,",");
            u_strcat(new_dela_line,dela_line);
            u_strcat(new_dela_line,lemma);
            u_strcat_char(new_dela_line,".");
            u_strcat(new_dela_line,codes);
            struct german_word_decomposition* wd=new_german_word_decomposition();
            wd->n_parts=n_decomp;
            u_strcpy(wd->decomposition,dec);
            u_strcpy(wd->dela_line,new_dela_line);
            if (check_valid_right_component_for_one_INF_code_german(l->token)) {
               // if we got a correct right component (N-FF)
               struct german_word_decomposition_list* wdl=new_german_word_decomposition_list();
               wdl->element=wd;
               wdl->suivant=(*L);
               (*L)=wdl;
            } else {
               free_german_word_decomposition(wd);
            }
            l=l->suivant;
         }
      }
    }
    else {
      // else, we must explore the rest of the original word
      if (tableau_correct_left_component[index]) {
         // but only if the current component was a valid left one
         // we go on with the next component
         unichar dec[2000];
         unichar line[500];
         u_strcpy(dec,decomposition);
         if (dec[0]!='\0') {u_strcat_char(dec," +++ ");}
         unichar sia_code[500];
         unichar entry[500];
         get_first_sia_code_german(index,sia_code);
         uncompress_entry(current_component,sia_code,entry);
         u_strcat(dec,entry);
         u_strcpy(line,dela_line);
         u_strcat(line,current_component);
         unichar temp[500];
         explore_state_german(4,temp,0,original_word,pos_in_original_word,
                  dec,line,L,n_decomp+1);
      }
    }
  }
  t=adresse+5;
}
else {
  c=c-32768;
  t=adresse+2;
}
if (original_word[pos_in_original_word]=='\0') {
   // if we have finished, we return
   return;
}
// if not, we go on with the next letter
for (int i=0;i<c;i++) {
  if (is_equal_or_case_equal((unichar)(tableau_bin[t]*256+tableau_bin[t+1]),original_word[pos_in_original_word],german_alphabet)
      || is_equal_or_case_equal(original_word[pos_in_original_word],(unichar)(tableau_bin[t]*256+tableau_bin[t+1]),german_alphabet)) {
    index=tableau_bin[t+2]*256*256+tableau_bin[t+3]*256+tableau_bin[t+4];
    current_component[pos_in_current_component]=(unichar)(tableau_bin[t]*256+tableau_bin[t+1]);
    explore_state_german(index,current_component,pos_in_current_component+1,original_word,pos_in_original_word+1,
                  decomposition,dela_line,L,n_decomp);
  }
  t=t+5;
}
}
