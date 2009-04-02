 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Text_tokens.h"
#include "Alphabet.h"
#include "Unicode.h"
#include "DELA_tree.h"
#include "DELA.h"
#include "String_hash.h"
#include "BuildTextAutomaton.h"
#include "NormalizationFst2.h"
#include "Fst2.h"
#include "File.h"
#include "Copyright.h"
#include "StringParsing.h"
#include "Error.h"
#include "Grf2Fst2_lib.h"
#include "Matches.h"
#include "getopt.h"
#include "LanguageDefinition.h"


int get_shift(int n_enter_char,int* enter_pos,int pos);

/**
 * This function tries to read a sentence from the given file.
 * It stops when it finds the EOF, a "{S}" or when MAX_TOKENS_IN_SENTENCE
 * tokens have been read. All the tokens that compose the sentence are stored
 * into 'buffer', except the "{S}", if any. The number of these tokens is stored
 * in '*N'. '*total' contains the whole number of integers read from the file.
 * 
 * The function returns 1 if a sentence was read; 0 otherwise.
 */
int read_sentence(int* buffer,int *N,int *total,FILE* f,int SENTENCE_MARKER) {
*total=0;
*N=0;
if (1!=fread(buffer,sizeof(int),1,f)) {
   /* If we are at the end of the file */
   return 0;
}
*total=1;
int length;
if (buffer[0]==SENTENCE_MARKER) {
   /* If the text starts by a {S}, we don't want to stop there */
   length=0;
} else {
   length=1;
}
while (length<MAX_TOKENS_IN_SENTENCE && 1==fread(buffer+length,sizeof(int),1,f) && buffer[length]!=SENTENCE_MARKER) {
   length++;
   (*total)++;
}
if (buffer[length]==SENTENCE_MARKER) {
   (*total)++;
}
if (length==0) return 0;
*N=length;
return 1;
}



void usage() {
u_printf("%S",COPYRIGHT);
u_printf("Usage: Txt2Tfst [OPTIONS] <txt>\n"
         "\n"
         "  <txt> : the text file\n"
         "\n"
         "OPTIONS:\n"
         "  -a ALPH/--alphabet=ALPH: the alphabet file\n"
         "  -c/---clean: cleans each sentence automaton, keeping best paths\n"
         "  -n XXX/--normalization_grammar=XXX: the .fst2 grammar used to normalize the text automaton\n"
         "  -t XXX/--tagset=XXX: use the XXX ELAG tagset file to normalize the dictionary entries\n"
         "  -h/--help: this help\n"
         "\n"
         "Constructs the text automaton. If the sentences of the text were delimited\n"
         "with the special tag {S}, the program produces one automaton per sentence.\n"
         "If not, the text is turned into %d token long automata. The result files\n"
         "named \"text.tfst\" and \"text.tind\" are stored is the text directory.\n"
         "\n"
         "Note that the program will also take into account the file \"tags.ind\", if any.\n",MAX_TOKENS_IN_SENTENCE);
}



int main_Txt2Tfst(int argc,char* argv[]) {
if (argc==1) {
   usage();
   return 0;
}

const char* optstring=":a:cn:t:h";
const struct option lopts[]={
   {"alphabet", required_argument, NULL, 'a'},
   {"clean", no_argument, NULL, 'c'},
   {"normalization_grammar", required_argument, NULL, 'n'},
   {"tagset", required_argument, NULL, 't'},
   {"help", no_argument, NULL, 'h'},
   {NULL, no_argument, NULL, 0}
};
char alphabet[FILENAME_MAX]="";
char norm[FILENAME_MAX]="";
char tagset[FILENAME_MAX]="";
int CLEAN=0;
int val,index=-1;
optind=1;
while (EOF!=(val=getopt_long(argc,argv,optstring,lopts,&index))) {
   switch(val) {
   case 'a': if (optarg[0]=='\0') {
                fatal_error("You must specify a non empty alphabet file name\n");
             }
             strcpy(alphabet,optarg);
             break;      
   case 'c': CLEAN=1; break;
   case 'n': if (optarg[0]=='\0') {
                fatal_error("You must specify a non empty normalization grammar name\n");
             }
             strcpy(norm,optarg);
             break;      
   case 't': if (optarg[0]=='\0') {
                fatal_error("You must specify a non empty tagset file name\n");
             }
             strcpy(tagset,optarg);
             break;      
   case 'h': usage(); return 0;
   case ':': if (index==-1) fatal_error("Missing argument for option -%c\n",optopt); 
             else fatal_error("Missing argument for option --%s\n",lopts[index].name);
   case '?': if (index==-1) fatal_error("Invalid option -%c\n",optopt); 
             else fatal_error("Invalid option --%s\n",optarg);
             break;
   }
   index=-1;
}

if (optind!=argc-1) {
   fatal_error("Invalid arguments: rerun with --help\n");
}
if (alphabet[0]=='\0') {
   fatal_error("You must specify the alphabet file\n");
}

struct DELA_tree* tree=new_DELA_tree();
int buffer[MAX_TOKENS_IN_SENTENCE];
char tokens_txt[FILENAME_MAX];
char text_cod[FILENAME_MAX];
char dlf[FILENAME_MAX];
char dlc[FILENAME_MAX];
char tags_ind[FILENAME_MAX];
get_snt_path(argv[optind],tokens_txt);
strcat(tokens_txt,"tokens.txt");
get_snt_path(argv[optind],text_cod);
strcat(text_cod,"text.cod");
get_snt_path(argv[optind],dlf);
strcat(dlf,"dlf");
get_snt_path(argv[optind],dlc);
strcat(dlc,"dlc");
get_snt_path(argv[optind],tags_ind);
strcat(tags_ind,"tags.ind");
load_DELA(dlf,tree);
load_DELA(dlc,tree);
u_printf("Loading %s...\n",tags_ind);
struct match_list* tag_list=NULL;
FILE* tag_file=u_fopen(tags_ind,U_READ);
if (tag_file!=NULL) {
   tag_list=load_match_list(tag_file,NULL);
   u_fclose(tag_file);
}
Alphabet* alph=load_alphabet(alphabet);
if (alph==NULL) {
   fatal_error("Cannot open %s\n",alphabet);
}
struct text_tokens* tokens=load_text_tokens(tokens_txt);
if (tokens==NULL) {
   fatal_error("Cannot open %s\n",tokens_txt);
}
FILE* f=fopen(text_cod,"rb");
if (f==NULL) {
   fatal_error("Cannot open %s\n",text_cod);
}
char text_tfst[FILENAME_MAX];
get_snt_path(argv[optind],text_tfst);
strcat(text_tfst,"text.tfst");
FILE* tfst=u_fopen(text_tfst,U_WRITE);
if (tfst==NULL) {
   u_fclose(f);
   fatal_error("Cannot create %s\n",text_tfst);
}
char text_tind[FILENAME_MAX];
get_snt_path(argv[optind],text_tind);
strcat(text_tind,"text.tind");
FILE* tind=fopen(text_tind,"wb");
if (tind==NULL) {
   u_fclose(f);
   u_fclose(tfst);
   fatal_error("Cannot create %s\n",text_tind);
}
struct normalization_tree* normalization_tree=NULL;
if (norm[0]!='\0') {
   normalization_tree=load_normalization_fst2(norm,alph,tokens);
}
char enter_pos_f[FILENAME_MAX];
get_snt_path(argv[optind],enter_pos_f);
strcat(enter_pos_f,"enter.pos");
FILE* f_enter=fopen(enter_pos_f,"rb");
int n_enter_char;
int* enter_pos=NULL;
if (f_enter==NULL) {
   error("Cannot open file %s\n",enter_pos);
   n_enter_char=0;
}
else {
   n_enter_char=get_file_size(f_enter)/4;
   enter_pos=(int*)malloc(sizeof(int)*n_enter_char);
   if (enter_pos==NULL) {
      fatal_error("Not enough memory in main\n");
   }
   if (n_enter_char!=(int)fread(enter_pos,sizeof(int),n_enter_char,f_enter)) {
      fatal_error("I/O error in main on file %d %s\n",n_enter_char,enter_pos_f);
   }
   fclose(f_enter);
}

language_t* language=NULL;
if (tagset[0]!='\0') {
   language=load_language_definition(tagset);
   set_current_language(language);
}

int sentence_number=1;
int N=0;
int total=0;
int current_global_position_in_tokens=0;
int current_global_position_in_chars=0;
/* We reserve the space for printing the number of sentence automata */
u_fprintf(tfst,"0000000000\n");
u_printf("Constructing text automaton...\n");
Ustring* text=new_Ustring(2048);
while (read_sentence(buffer,&N,&total,f,tokens->SENTENCE_MARKER)) {
   /* We compute and save the current sentence description */
   build_sentence_automaton(buffer,N,tokens,tree,alph,tfst,tind,sentence_number,CLEAN,
                            normalization_tree,&tag_list,
                            current_global_position_in_tokens,
                            current_global_position_in_chars+get_shift(n_enter_char,enter_pos,current_global_position_in_tokens),
                            language);
   if (sentence_number%100==0) u_printf("%d sentences read...        \r",sentence_number);
   sentence_number++;
   current_global_position_in_tokens=current_global_position_in_tokens+total;
   for (int y=0;y<total;y++) {
      current_global_position_in_chars=current_global_position_in_chars+u_strlen(tokens->token[buffer[y]]);
   }
}
u_printf("%d sentence%s read\n",sentence_number-1,(sentence_number-1)>1?"s":"");
fclose(f);
free(enter_pos);
free_Ustring(text);
u_fclose(tfst);
fclose(tind);
write_number_of_graphs(text_tfst,sentence_number-1);
free_DELA_tree(tree);
free_text_tokens(tokens);
free_alphabet(alph);
free_normalization_tree(normalization_tree);
free_language_t(language);
/* After the execution, tag_list should have been emptied, so that we don't
 * need to do it here */ 
return 0;
}



/**
 * This function takes an integer 'a' and an array 't' of size 'n'.
 * It returns the greatest value x so that t[x]<=a.
 */
int find_by_dichotomy(int a,int* t,int n) {
int start_position,middle_position;
if (t==NULL) {
   error("NULL array in find_by_dichotomy\n");
   return 0;
}
if (n==0) {
   return 0;
}
if (a<t[0]) return 0;
if (a>t[n-1]) return n;
n=n-1;
start_position=0;
while (start_position<=n) {
   middle_position=(start_position+n)/2;
   if (t[middle_position]==a) return middle_position;
   if (t[middle_position]<a) {
      start_position=middle_position+1;
   } else {
      n=middle_position-1;
   }
}
return n+1;
}


/**
 * This function takes the number of new lines in the text ('n_enter_char'),
 * the array 'enter_pos' that contains their positions in tokens and a position
 * 'pos'. It returns the number of new lines that occur before 'pos'.
 */
int get_shift(int n_enter_char,int* enter_pos,int pos) {
int res=find_by_dichotomy(pos,enter_pos,n_enter_char);
return res;
}
