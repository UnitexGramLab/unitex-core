 /*
  * Unitex
  *
  * Copyright (C) 2001-2008 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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



void usage() {
u_printf("%S",COPYRIGHT);
u_printf("Usage: Txt2Fst2 <text> <alphabet> [-clean] [norm]\n");
u_printf("     <text> : the text file\n");
u_printf("     <alphabet> : the alphabet file for the text language\n");
u_printf("     [-clean] : cleans each sentence automaton, keeping best paths.\n");
u_printf("     [norm] : the fst2 grammar used to normalize the text automaton.\n\n");
u_printf("Constructs the text automaton. If the sentences of the text were delimited\n");
u_printf("with the special tag {S}, the program produces one automaton per sentence.\n");
u_printf("If not, the text is turned into %d token long automata. The result file\n",MAX_TOKENS_IN_SENTENCE);
u_printf("named TEXT.FST2 is stored is the text directory.\n");
}



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


int main(int argc, char **argv) {
/* Every Unitex program must start by this instruction,
 * in order to avoid display problems when called from
 * the graphical interface */
setBufferMode();

if (argc<3 || argc>5) {
   usage();
   return 0;
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
get_snt_path(argv[1],tokens_txt);
strcat(tokens_txt,"tokens.txt");
get_snt_path(argv[1],text_cod);
strcat(text_cod,"text.cod");
get_snt_path(argv[1],dlf);
strcat(dlf,"dlf");
get_snt_path(argv[1],dlc);
strcat(dlc,"dlc");
get_snt_path(argv[1],tags_ind);
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
Alphabet* alph=load_alphabet(argv[2]);
if (alph==NULL) {
   fatal_error("Cannot open %s\n",argv[2]);
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
get_snt_path((const char*)argv[1],tmp);
strcat(tmp,"text.fst2");
FILE* out=u_fopen(tmp,U_WRITE);
if (out==NULL) {
   u_fclose(f);
   fatal_error("Cannot create %s\n",tmp);
}
int CLEAN=0;
struct normalization_tree* normalization_tree=NULL;
if (argc==5) {
   /* If there are optional parameters */
   if (!strcmp(argv[3],"-clean")) {
      CLEAN=1;
   } else {
      error("Invalid parameter %s\n",argv[3]);
   }
   normalization_tree=load_normalization_fst2(argv[4],alph,tokens);
}
else
if (argc==4) {
   if (!strcmp(argv[3],"-clean")) {
      CLEAN=1;
   } else {
      normalization_tree=load_normalization_fst2(argv[3],alph,tokens);
   }
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
for (int i=0;i<tags->size;i++) {
   unichar tmp[1024];
   escape(tags->value[i],tmp,P_SLASH);
   u_fprintf(out,"%%%S\n",tmp);
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


