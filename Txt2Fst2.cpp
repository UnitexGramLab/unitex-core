 /*
  * Unitex
  *
  * Copyright (C) 2001-2007 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "Load_DLF_DLC.h"
#include "DELA.h"
#include "String_hash.h"
#include "Text_automaton.h"
#include "List_int.h"
#include "Normalization_transducer.h"
#include "Fst2.h"
#include "FileName.h"
#include "Copyright.h"
#include "IOBuffer.h"
#include "StringParsing.h"
#include "Error.h"


void usage() {
u_printf("%S",COPYRIGHT);
u_printf("Usage: Txt2Fst2 <text> <alphabet> [-clean] [norm]\n");
u_printf("     <text> : the text file\n");
u_printf("     <alphabet> : the alphabet file for the text language\n");
u_printf("     [-clean] : cleans each sentence automaton, keeping best paths.\n");
u_printf("     [norm] : the fst2 grammar used to normalize the text automaton.\n\n");
u_printf("Constructs the text automaton. If the sentences of the text were delimited\n");
u_printf("with the special tag {S}, the program produces one automaton per sentence.\n");
u_printf("If not, the text is turned into %d token long automata. The result file\n",MAX_TOKENS);
u_printf("named TEXT.FST2 is stored is the text directory.\n");
}



//
// this function try to read a sentence in the file
//
int lire_sentence(int buffer[],int* N,FILE* f,int SENTENCE_MARKER) {
int length=0;
if (1!=fread(buffer,sizeof(int),1,f)) {
   return 0;
}
if (buffer[0]!=SENTENCE_MARKER) {
   /* If the text starts by a {S}, we don't want to stop there */
   length=1;
}
while (length<MAX_TOKENS && 1==fread(buffer+length,sizeof(int),1,f) && buffer[length]!=SENTENCE_MARKER) {
   length++;
}
if (length==0) return 0;
*N=length;
return 1;
}



void ecrire_fichier_sortie_nombre_de_phrases_graphes(char name[],int n) {
FILE *f;
int i;
i=2+9*2; // *2 because of unicode +2 because of FF FE at file start
f=fopen((char*)name,"r+b");
do {
  fseek(f,i,0);
  i=i-2;
  u_fputc((unichar)((n%10)+'0'),f);
  n=n/10;
}
while (n);
fclose(f);
}



int main(int argc, char **argv) {
setBufferMode();

if (argc<3 || argc>5) {
   usage();
   return 0;
}
struct string_hash* etiquettes=new_string_hash();
// we insert in any case the epsilon in preview of future operations on
// the text automaton that could need this special tag
unichar epsilon[4];
u_strcpy(epsilon,"<E>");
get_value_index(epsilon,etiquettes);
struct noeud_dlf_dlc* racine;
racine=new_noeud_dlf_dlc();
struct text_tokens* tok;
Alphabet* alph;
int buffer[MAX_TOKENS];
int debut,N;
FILE* f;
FILE* out;
char tokens_txt[2000];
char text_cod[2000];
char dlf[2000];
char dlc[2000];
get_snt_path((const char*)argv[1],tokens_txt);
strcat(tokens_txt,"tokens.txt");
get_snt_path((const char*)argv[1],text_cod);
strcat(text_cod,"text.cod");
get_snt_path((const char*)argv[1],dlf);
strcat(dlf,"dlf");
get_snt_path((const char*)argv[1],dlc);
strcat(dlc,"dlc");

load_dlf_dlc(dlf,racine);
load_dlf_dlc(dlc,racine);
alph=load_alphabet(argv[2]);
if (alph==NULL) {
   fatal_error("Cannot open %s\n",argv[2]);
}
tok=load_text_tokens(tokens_txt);
if (tok==NULL) {
   fatal_error("Cannot open %s\n",tokens_txt);
}

f=fopen(text_cod,"rb");
if (f==NULL) {
   fatal_error("Cannot open %s\n",text_cod);
}
// on extrait le chemin du fichier texte
char tmp[1000];
get_snt_path((const char*)argv[1],tmp);
strcat(tmp,"text.fst2");
out=u_fopen(tmp,U_WRITE);
if (out==NULL) {
   u_fclose(f);
   fatal_error("Cannot create %s\n",tmp);
}
int CLEAN=0;
struct noeud_arbre_normalization* normalization_tree=NULL;

if (argc==5) {
   // if there are optional parameters
   if (!strcmp(argv[3],"-clean")) {
      // if the clean parameter is set
      CLEAN=1;
   } else {
      error("Invalid parameter %s\n",argv[3]);
   }
   normalization_tree=load_normalization_transducer(argv[4],alph,tok);
}
else
if (argc==4) {
   if (!strcmp(argv[3],"-clean")) {
      // if the clean parameter is set
      CLEAN=1;
   } else {
         normalization_tree=load_normalization_transducer(argv[3],alph,tok);
   }
}
debut=0;
int numero_phrase=1;
u_fprintf(out,"0000000000\n");
u_printf("Constructing text automaton...\n");
while (lire_sentence(buffer,&N,f,tok->SENTENCE_MARKER)) {
   construire_text_automaton(buffer,N,tok,racine,etiquettes,alph,out,numero_phrase,debut,debut+N-1,CLEAN,normalization_tree);
   if (numero_phrase%100==0) u_printf("%d sentences read...        \r",numero_phrase);
   numero_phrase++;
   debut=debut+N+1;
}
u_printf("%d sentence%s read\n",numero_phrase-1,(numero_phrase-1)>1?"s":"");
fclose(f);
u_printf("Saving tags...\n");
for (int i=0;i<etiquettes->size;i++) {
  unichar tmp[1024];
  escape(etiquettes->value[i],tmp,P_SLASH);
  u_fprintf(out,"%%%S\n",tmp);
}
u_fprintf(out,"f\n");
u_fclose(out);
ecrire_fichier_sortie_nombre_de_phrases_graphes(tmp,numero_phrase-1);
free_noeud_dlf_dlc(racine);
free_text_tokens(tok);
free_alphabet(alph);
free_string_hash(etiquettes);
free_noeud_arbre_normalization(normalization_tree);
return 0;
}


