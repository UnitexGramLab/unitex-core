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
#include "TextAutomaton.h"
#include "NormalizationFst2.h"
#include "Fst2.h"
#include "File.h"
#include "Copyright.h"
#include "IOBuffer.h"
#include "StringParsing.h"
#include "Error.h"
#include "Grf2Fst2_lib.h"
#include "Matches.h"
#include "getopt.h"



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
u_printf("Usage: Txt2Fst2 [OPTIONS] <txt>\n"
         "\n"
         "  <txt> : the text file\n"
         "\n"
         "OPTIONS:\n"
         "  -a ALPH/--alphabet=ALPH: the alphabet file\n"
         "  -c/---clean: cleans each sentence automaton, keeping best paths\n"
         "  -n XXX/--normalization_grammar=XXX: the .fst2 grammar used to normalize the text automaton\n"
         "  -h/--help: this help\n"
         "\n"
         "Constructs the text automaton. If the sentences of the text were delimited\n"
         "with the special tag {S}, the program produces one automaton per sentence.\n"
         "If not, the text is turned into %d token long automata. The result file\n"
         "named \"text.fst2\" is stored is the text directory.\n"
         "\n"
         "Note that the program will also take into account the file \"tags.ind\", if any.\n",MAX_TOKENS_IN_SENTENCE);
}



int main(int argc,char* argv[]) {
/* Every Unitex program must start by this instruction,
 * in order to avoid display problems when called from
 * the graphical interface */
setBufferMode();

if (argc==1) {
   usage();
   return 0;
}

const char* optstring=":a:cn:h";
const struct option lopts[]={
   {"alphabet", required_argument, NULL, 'a'},
   {"clean", no_argument, NULL, 'c'},
   {"normalization_grammar", required_argument, NULL, 'n'},
   {"help", no_argument, NULL, 'h'},
   {NULL, no_argument, NULL, 0}
};
char alphabet[FILENAME_MAX]="";
char norm[FILENAME_MAX]="";
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

struct string_hash* tags=new_string_hash();
/* We insert in any case the epsilon tag in preview of future operations on
 * the text automaton that could need this special tag */
unichar epsilon[4];
u_strcpy(epsilon,"<E>");
get_value_index(epsilon,tags);
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
char tmp[FILENAME_MAX];
get_snt_path(argv[optind],tmp);
strcat(tmp,"text.fst2");
FILE* out=u_fopen(tmp,U_WRITE);
if (out==NULL) {
   u_fclose(f);
   fatal_error("Cannot create %s\n",tmp);
}
struct normalization_tree* normalization_tree=NULL;
if (norm[0]!='\0') {
   normalization_tree=load_normalization_fst2(norm,alph,tokens);
}
int sentence_number=1;
int N=0;
int total=0;
int current_global_position=0;
/* We reserve the space for printing the number of sentence automata */
u_fprintf(out,"0000000000\n");
u_printf("Constructing text automaton...\n");
while (read_sentence(buffer,&N,&total,f,tokens->SENTENCE_MARKER)) {
   build_sentence_automaton(buffer,N,tokens,tree,tags,alph,out,sentence_number,CLEAN,
                            normalization_tree,&tag_list,current_global_position);
   if (sentence_number%100==0) u_printf("%d sentences read...        \r",sentence_number);
   sentence_number++;
   current_global_position=current_global_position+total;
}
u_printf("%d sentence%s read\n",sentence_number-1,(sentence_number-1)>1?"s":"");
fclose(f);
u_printf("Saving tags...\n");
unichar temp[4096];
for (int i=0;i<tags->size;i++) {
   escape(tags->value[i],temp,P_SLASH);
   u_fprintf(out,"%%%S\n",temp);
}
u_fprintf(out,"f\n");
u_fclose(out);
write_number_of_graphs(tmp,sentence_number-1);
free_DELA_tree(tree);
free_text_tokens(tokens);
free_alphabet(alph);
free_string_hash(tags);
free_normalization_tree(normalization_tree);
/* After the execution, tag_list should have been emptied, so that we don't
 * need to do it here */ 
return 0;
}


