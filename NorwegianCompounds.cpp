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
#include "NorwegianCompounds.h"
//---------------------------------------------------------------------------

Alphabet* norwegian_alphabet;
//
// this function analyses the norwegian compound words
//
void analyse_norwegian_compounds(Alphabet* alph,unsigned char* bin,struct INF_codes* inf,
                                 FILE* words,FILE* result,FILE* debug,FILE* new_unknown_words,
                                 struct string_hash* forbiddenWords) {
tableau_correct_left_component=(char*)malloc(sizeof(char)*(inf->N));
tableau_correct_right_component=(char*)malloc(sizeof(char)*(inf->N));
check_valid_left_component(tableau_correct_left_component,inf);
check_valid_right_component(tableau_correct_right_component,inf);
norwegian_alphabet=alph;
inf_codes=inf;
analyse_norwegian_word_list(bin,inf,words,result,debug,new_unknown_words,forbiddenWords);
free(tableau_correct_left_component);
free(tableau_correct_right_component);
}



//
// return 1 if at least one of the INF codes of l is a valid
// right component, 0 else
//
char check_valid_right_component_for_an_INF_line(struct token_list* l) {
while (l!=NULL) {
   if (check_valid_right_component_for_one_INF_code(l->token)) {
      return 1;
   }
   l=l->suivant;
}
return 0;
}



//
// this function check for all the INF codes that are:
// N
// or A
// or V
//
void check_valid_right_component(char* tableau_sia,struct INF_codes* inf) {
printf("Check valid right components...\n");
for (int i=0;i<inf->N;i++) {
   tableau_correct_right_component[i]=check_valid_right_component_for_an_INF_line(inf->tab[i]);
}
}



//
// this function check for all the INF codes that are:
// N:sia
// or A:sio
// or V:W
// or ADV
//
void check_valid_left_component(char* tableau_sia,struct INF_codes* inf) {
printf("Check valid left components...\n");
for (int i=0;i<inf->N;i++) {
   tableau_sia[i]=check_valid_left_component_for_an_INF_line(inf->tab[i]);
}
}



//
// return 1 if at least one of the INF codes of l is a valid
// left component, 0 else
//
char check_valid_left_component_for_an_INF_line(struct token_list* l) {
while (l!=NULL) {
   if (check_valid_left_component_for_one_INF_code(l->token)) {
      return 1;
   }
   l=l->suivant;
}
return 0;
}





/*
 * This function looks for the first 'sia' code of a line of the INF file.
 * If there is N and ADV, we give the preference to N.
 *
 * @param n number of the line
 * @param s a string that will contains the selected code
 **/
void get_first_sia_code(int n,unichar* s) {
// we initialize s to prevent errors, but this case should never happen
int tmp;
struct token_list* l=inf_codes->tab[n];
while (l!=NULL) {
   tmp=get_valid_left_component_type_for_one_INF_code(l->token);
   if (tmp==N_SIA || tmp==N_SIE || tmp==N_SIG) {
      // if we find an N, then we return it
      u_strcpy(s,l->token);
      return;
   }
   if (tmp!=INVALID_LEFT_COMPONENT) {
      u_strcpy(s,l->token);
   }
   l=l->suivant;
}
}



//
// returns 1 if the line is a N:sia one
//
char check_Nsia(dic_entry* d) {
unichar t1[2];
u_strcpy_char(t1,"N");
unichar t2[4];
u_strcpy_char(t2,"sia");
return dic_entry_contain_gram_code(d,t1) && dic_entry_contain_flex_code(d,t2);
}



//
// returns 1 if the line is a N:sie one
//
char check_Nsie(dic_entry* d) {
unichar t1[2];
u_strcpy_char(t1,"N");
unichar t2[4];
u_strcpy_char(t2,"sie");
return dic_entry_contain_gram_code(d,t1) && dic_entry_contain_flex_code(d,t2);
}



//
// returns 1 if the line is a N:sig one
//
char check_Nsig(dic_entry* d) {
unichar t1[2];
u_strcpy_char(t1,"N");
unichar t2[4];
u_strcpy_char(t2,"sig");
return dic_entry_contain_gram_code(d,t1) && dic_entry_contain_flex_code(d,t2);
}



//
// returns 1 if the line is a A:sio one
//
char check_Asio(dic_entry* d) {
unichar t1[2];
u_strcpy_char(t1,"A");
unichar t2[4];
u_strcpy_char(t2,"sio");
return dic_entry_contain_gram_code(d,t1) && dic_entry_contain_flex_code(d,t2);
}


//
// returns 1 if the line is a A:sie one
//
char check_Asie(dic_entry* d) {
unichar t1[2];
u_strcpy_char(t1,"A");
unichar t2[4];
u_strcpy_char(t2,"sie");
return dic_entry_contain_gram_code(d,t1) && dic_entry_contain_flex_code(d,t2);
}



//
// returns 1 if the line is a V:W one
//
char check_VW(dic_entry* d) {
unichar t1[2];
u_strcpy_char(t1,"V");
unichar t2[2];
u_strcpy_char(t2,"W");
return dic_entry_contain_gram_code(d,t1) && dic_entry_contain_flex_code(d,t2);
}



//
// returns 1 if the line is a ADV one
//
char check_ADV(dic_entry* d) {
unichar t1[4];
u_strcpy_char(t1,"ADV");
return (char)dic_entry_contain_gram_code(d,t1);
}


//
// returns 1 if the line is a V but not Y one
//
char check_V_but_not_Y(dic_entry* d) {
unichar t1[2];
u_strcpy_char(t1,"V");
unichar t2[2];
u_strcpy_char(t2,"Y");
return dic_entry_contain_gram_code(d,t1) && (!dic_entry_contain_flex_code(d,t2));
}



//
// returns 1 if the INF code refers to a valid left component, 0 else
//
char check_valid_left_component_for_one_INF_code(unichar* s) {
unichar temp[2000];
u_strcpy_char(temp,"x,");
u_strcat(temp,s);
dic_entry* d=tokenize_DELA_line(temp);
char res=check_Nsia(d)||check_Nsie(d)||check_Nsig(d)||check_Asio(d)||check_Asie(d)||check_VW(d)||check_ADV(d);
free_dic_entry(d);
return res;
}


/*
 * @param s string containing an INF code 
 * @return a code indicating the type of left component of s
 **/
int get_valid_left_component_type_for_one_INF_code(unichar* s) {
unichar temp[2000];
u_strcpy_char(temp,"x,");
u_strcat(temp,s);
dic_entry* d=tokenize_DELA_line(temp);
int res;
if (check_Nsia(d)) res=N_SIA;
else if (check_Nsie(d)) res=N_SIE;
else if (check_Nsig(d)) res=N_SIG;
else if (check_Asio(d)) res=A_SIO;
else if (check_Asie(d)) res=A_SIE;
else if (check_VW(d)) res=V_W;
else if (check_ADV(d)) res=ADV;
else res=INVALID_LEFT_COMPONENT;
free_dic_entry(d);
return res;
}



//
// returns 1 if the line is a N one
//
char check_N(dic_entry* d) {
unichar t1[2];
u_strcpy_char(t1,"N");
return (char)dic_entry_contain_gram_code(d,t1);
}


//
// returns 1 if the line is a valid right N component
//
char check_N_right_component(unichar* s) {
unichar temp[2000];
u_strcpy_char(temp,"x,");
u_strcat(temp,s);
dic_entry* d=tokenize_DELA_line(temp);
unichar t1[2];
u_strcpy_char(t1,"N");
unichar t2[4];
u_strcpy_char(t2,"sie");
return dic_entry_contain_gram_code(d,t1) && !dic_entry_contain_flex_code(d,t2);
}


//
// returns 1 if the line is a valid right A component
//
char check_A_right_component(unichar* s) {
unichar temp[2000];
u_strcpy_char(temp,"x,");
u_strcat(temp,s);
dic_entry* d=tokenize_DELA_line(temp);
unichar t1[2];
u_strcpy_char(t1,"A");
unichar t2[4];
u_strcpy_char(t2,"sie");
return dic_entry_contain_gram_code(d,t1) && !dic_entry_contain_flex_code(d,t2);
}


//
// returns 1 if the line is a :a one
//
char check_a(dic_entry* d) {
unichar t1[2];
u_strcpy_char(t1,"a");
return (char)dic_entry_contain_flex_code(d,t1);
}


//
// returns 1 if the line is a N one
//
char check_N(unichar* s) {
unichar temp[2000];
u_strcpy_char(temp,"x,");
u_strcat(temp,s);
dic_entry* d=tokenize_DELA_line(temp);
return check_N(d);
}


//
// returns 1 if the line is a :a one
//
char check_a(unichar* s) {
unichar temp[2000];
u_strcpy_char(temp,"x,");
u_strcat(temp,s);
dic_entry* d=tokenize_DELA_line(temp);
return check_a(d);
}



//
// returns 1 if the line is a A one
//
char check_A(dic_entry* d) {
unichar t1[2];
u_strcpy_char(t1,"A");
return (char)dic_entry_contain_gram_code(d,t1);
}



//
// returns 1 if the INF code refers to a valid right component, 0 else
//
char check_valid_right_component_for_one_INF_code(unichar* s) {
unichar temp[2000];
u_strcpy_char(temp,"x,");
u_strcat(temp,s);
dic_entry* d=tokenize_DELA_line(temp);
char res=(check_N(d)||check_A(d)/*||check_V_but_not_Y(d)*/)&&(!check_Nsie(d));
free_dic_entry(d);
return res;
}


//
// returns 1 if the dictionary line refers to a verb with more than 4
// letters, 0 otherwise
//
char verb_of_more_than_4_letters(unichar* line) {
dic_entry* d=tokenize_DELA_line(line);
char res=check_V_but_not_Y(d) && u_strlen(d->inflected)>4;
free_dic_entry(d);
return res;
}


/* ALREADY defined in Compounds.cpp

//
// this function reads a token in a file and returns 1 on success, 0 else
//
int next_word(FILE* words,unichar* s) {
int c;
// we jump all the separators
while ((c=u_fgetc(words))!=EOF && (c==' ' || c=='\n' || c=='\t'));
if (c==EOF) {
   // if we are at the end of the file, we return
   return 0;
}
int i=0;
while (c!=EOF && c!=' ' && c!='\n' && c!='\t') {
   s[i++]=(unichar)c;
   c=u_fgetc(words);
}
s[i]='\0';
return 1;
}
*/


//
// this function reads words in the word file and try analyse them
//
void analyse_norwegian_word_list(unsigned char* bin,struct INF_codes* inf,
                                 FILE* words,FILE* result,FILE* debug,
                                 FILE* new_unknown_words,struct string_hash* forbiddenWords) {
unichar s[1000];
tableau_bin=bin;
debug_file=debug;
result_file=result;
printf("Analysing norwegian unknown words...\n");
int n=0;
while (next_word(words,s)) {
  if (!analyse_norwegian_word(s,forbiddenWords)) {
     // if the analysis has failed, we store the word in the new unknown word file
     u_fprints(s,new_unknown_words);
     u_fprints_char("\n",new_unknown_words);
  } else {n++;}
}
printf("%d words decomposed as compound words\n",n);
}



//
// this function try to analyse an unknown norwegian word
//
int analyse_norwegian_word(unichar* mot,struct string_hash* forbiddenWords) {
unichar decomposition[2000];
unichar dela_line[2000];
unichar correct_word[2000];
decomposition[0]='\0';
dela_line[0]='\0';
correct_word[0]='\0';
struct word_decomposition_list* l=NULL;
explore_state(4,correct_word,0,mot,0,decomposition,dela_line,&l,1,forbiddenWords);
if (l==NULL) {
   return 0;
}
struct word_decomposition_list* tmp=l;
int n=1000;
int is_a_valid_right_N=0;
int is_a_valid_right_A=0;
while (tmp!=NULL) {
   if (tmp->element->n_parts<=n) {
      if (tmp->element->n_parts<n) {
         // if we change of component number, we reset the 
         // is_a_valid_right_N and is_a_valid_right_A fields, because
         // they only concern the head word
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
   tmp=tmp->suivant;
}
tmp=l;
while (tmp!=NULL) {
   if (n==tmp->element->n_parts) {
      // we only consider the words that have shortest decompositions
      // the test (tmp->element->n_parts==1) is used to
      // match simple words that would have been wrongly considered
      // as unknown words
      int OK=0;
      if (tmp->element->n_parts==1) {
         OK=1; // simple words must be matched
      }
      else if (is_a_valid_right_N) {
              if (tmp->element->is_a_valid_right_N) {
                 OK=1; // we give priority to x<N>
              }
      }
      else if (is_a_valid_right_A) {
              if (tmp->element->is_a_valid_right_A) {
                 OK=1; // we give secondly priority to x<A>
              }
      } else OK=1;
      if (OK) {
         // we also put a restriction on the grammatical code:
         // we don't produce a x<A> or x<V> sequence when a x<N> exists
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
return 1;
}



//
// this function extract a substring of mot into mot2
//
void extraire(unichar* mot,unichar* mot2,int pos) {
int i;
i=pos;
do {
  mot2[i-pos]=mot[i];
  i++;
}
while (mot[i-1]!='\0');
}



//
// this function writes the dictionary lines that have been produced
//
void ecrire_ligne_dico_sortie(unichar* ligne_dico,int N) {
unichar tmp[2000];
struct token_list* l=inf_codes->tab[N];
while (l!=NULL) {
   uncompress_entry(ligne_dico,l->token,tmp);
   u_fprints(tmp,result_file);
   u_fprints_char("\n",result_file);
   l=l->suivant;
}
}



struct word_decomposition* new_word_decomposition() {
struct word_decomposition* tmp;
tmp=(struct word_decomposition*)malloc(sizeof(struct word_decomposition));
tmp->n_parts=0;
tmp->decomposition[0]='\0';
tmp->dela_line[0]='\0';
tmp->is_a_valid_right_N=0;
tmp->is_a_valid_right_A=0;
return tmp;
}



void free_word_decomposition(struct word_decomposition* t) {
if (t==NULL) return;
free(t);
}



struct word_decomposition_list* new_word_decomposition_list() {
struct word_decomposition_list* tmp;
tmp=(struct word_decomposition_list*)malloc(sizeof(struct word_decomposition_list));
tmp->element=NULL;
tmp->suivant=NULL;
return tmp;
}


void free_word_decomposition_list(struct word_decomposition_list* l) {
struct word_decomposition_list* tmp;
while (l==NULL) {
   free_word_decomposition(l->element);
   tmp=l->suivant;
   free(l);
   l=tmp;
}
}



//
// this function explores the dictionary to decompose the word mot
//
void explore_state(int adresse,unichar* current_component,int pos_in_current_component,
                   unichar* original_word,int pos_in_original_word,unichar* decomposition,
                   unichar* dela_line,struct word_decomposition_list** L,int n_decomp,
                   struct string_hash* forbiddenWords) {
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
      if (get_token_number(current_component,forbiddenWords)==-1)
          // DEPRECATED
          /*
            u_strcmp_char(current_component,"ar") &&
            u_strcmp_char(current_component,"ende") &&
            u_strcmp_char(current_component,"ert") &&
            u_strcmp_char(current_component,"te") &&
            u_strcmp_char(current_component,"ve") &&
            u_strcmp_char(current_component,"vis") &&
            tableau_correct_right_component[index])*/

            {

         // and if we do not have forbidden word in last position
         // and if we have a valid right component
         struct token_list* l=inf_codes->tab[index];
         while (l!=NULL) {
            unichar dec[200];
            u_strcpy(dec,decomposition);
            if (dec[0]!='\0') {u_strcat_char(dec," +++ ");}
            unichar entry[200];
            uncompress_entry(current_component,l->token,entry);
            u_strcat(dec,entry);
            unichar inflected[200];
            unichar lemma[200];
            unichar codes[200];
            unichar new_dela_line[200];
            u_strcpy(new_dela_line,dela_line);
            tokenize_DELA_line_into_3_parts(entry,inflected,lemma,codes);
            u_strcat(new_dela_line,inflected);
            u_strcat_char(new_dela_line,",");
            u_strcat(new_dela_line,dela_line);
            u_strcat(new_dela_line,lemma);
            u_strcat_char(new_dela_line,".");
            u_strcat(new_dela_line,codes);

            struct word_decomposition* wd=new_word_decomposition();
            wd->n_parts=n_decomp;
            u_strcpy(wd->decomposition,dec);
            u_strcpy(wd->dela_line,new_dela_line);
            //wd->is_an_N=(codes[0]=='N' && (codes[1]=='\0' || codes[1]=='+' || codes[1]==':'));
            wd->is_a_valid_right_N=check_N_right_component(l->token);
            wd->is_a_valid_right_A=check_A_right_component(l->token);

            if (verb_of_more_than_4_letters(entry)
                || check_valid_right_component_for_one_INF_code(l->token)
                || wd->n_parts==1) {
               // if we got a correct right component (N, A, or V)
               struct word_decomposition_list* wdl=new_word_decomposition_list();
               wdl->element=wd;
               wdl->suivant=(*L);
               (*L)=wdl;
            } else {
               free_word_decomposition(wd);
            }
            l=l->suivant;
         }
      }
    }
    else {
      // else, we must explore the rest of the original word
      if (tableau_correct_left_component[index] &&
          (get_token_number(current_component,forbiddenWords)==-1)) {
         // but only if the current component was a valid left one (not a
         // forbidden word and correct codes)
         if (pos_in_current_component>2 &&
             (current_component[pos_in_current_component-1]==current_component[pos_in_current_component-2])) {
            // we handle the case of a word ending by a double letter
            unichar dec[200];
            unichar line[200];
            u_strcpy(dec,decomposition);
            if (dec[0]!='\0') {u_strcat_char(dec," +++ ");}
            unichar sia_code[200];
            unichar entry[200];
            get_first_sia_code(index,sia_code);
            uncompress_entry(current_component,sia_code,entry);
            u_strcat(dec,entry);
            u_strcpy(line,dela_line);
            u_strcat(line,current_component);
            // we must remove a character
            line[u_strlen(line)-1]='\0';
            unichar temp[200];
            unichar dec_temp[200];
            u_strcpy(dec_temp,dec);
            explore_state(4,temp,0,original_word,pos_in_original_word-1,
                  dec_temp,line,L,n_decomp+1,forbiddenWords);
         }

         // we go on with the next component
         unichar dec[200];
         unichar line[200];
         u_strcpy(dec,decomposition);
         if (dec[0]!='\0') {u_strcat_char(dec," +++ ");}
         unichar sia_code[200];
         unichar entry[200];
         get_first_sia_code(index,sia_code);
         uncompress_entry(current_component,sia_code,entry);
         u_strcat(dec,entry);
         u_strcpy(line,dela_line);
         u_strcat(line,current_component);
         unichar temp[200];
         unichar dec_temp[200];
         u_strcpy(dec_temp,dec);
         explore_state(4,temp,0,original_word,pos_in_original_word,
                  dec_temp,line,L,n_decomp+1,forbiddenWords);

         // DEPRECATED
         // we test if we can have the insertion of e or s
         /*unichar u=original_word[pos_in_original_word];
         if (u=='e' || u=='E' || u=='s' || u=='S') {
             if (u=='e' || u=='E') {u_strcat_char(line,"e");u_strcat_char(dec," +++ e");}
             else {u_strcat_char(line,"s");u_strcat_char(dec," +++ s");}
             explore_state(4,temp,0,original_word,pos_in_original_word+1,
                  dec,line,L,n_decomp+1);
         }*/
         
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
  if (is_equal_or_case_equal((unichar)(tableau_bin[t]*256+tableau_bin[t+1]),original_word[pos_in_original_word],norwegian_alphabet)) {
    index=tableau_bin[t+2]*256*256+tableau_bin[t+3]*256+tableau_bin[t+4];
    current_component[pos_in_current_component]=(unichar)(tableau_bin[t]*256+tableau_bin[t+1]);
    explore_state(index,current_component,pos_in_current_component+1,original_word,pos_in_original_word+1,
                  decomposition,dela_line,L,n_decomp,forbiddenWords);
  }
  t=t+5;
}
}
