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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "unicode.h"
#include "Copyright.h"
#include "Alphabet.h"
#include "DELA.h"
#include "String_hash.h"
#include "NorwegianCompounds.h"
#include "GermanCompounds.h"
#include "FileName.h"
#include "GeneralDerivation.h"
#include "RussianCompounds.h"
#include "IOBuffer.h"


//---------------------------------------------------------------------------

// NORWEGIAN e:\norvegien\alphabet.txt e:\norvegien\Delaf.bin e:\norvegien\inconnus.txt e:\norvegien\resultat.dic e:\norvegien\decomp.txt
// GERMAN "e:\my unitex\german\alphabet.txt" "e:\my unitex\german\Dela\dela.bin" "e:\my unitex\german\corpus\kritik der reinen vernunft_snt\err.txt" "e:\my unitex\german\corpus\kritik der reinen vernunft_snt\result.dic" "e:\my unitex\german\decomp.txt"

//---------------------------------------------------------------------------
void usage() {
printf("%s",COPYRIGHT);
printf("Usage: PolyLex <lang> <alph> <dic> <list> <out> [<info>]\n");
printf("     <lang> : language to work on. Possible values are:\n");
printf("              GERMAN\n");
printf("              NORWEGIAN\n");
printf("              RUSSIAN\n");
printf("     <alph> : alphabet file of the language\n");
printf("     <dic> : dictionary .BIN to use\n");
printf("     <list> : text file containing the words to be analysed\n");
printf("     <out> : dictionary .DIC where the resulting lines will be stored. If\n");
printf("             this file allready exists, the lines are added at the end of it.\n");
printf("     [<info>] : if this optional parameter is precised, it is taken as\n");
printf("                the name of a file which will contain information about\n");
printf("                the analysis\n");
printf("\n");
printf("Tries to decompose some norwegian words as combinaisons of shortest words.\n");
printf("This words are removed from the <list> files.\n");
printf("NOTE: when the program is used for Nowegian words, it tries to read a text file\n");
printf("containing a list of forbidden words. This file is supposed to be named\n");
printf("'ForbiddenWords.txt' and stored in the same directory that <dic>.\n");
}


int main(int argc, char **argv) {
setBufferMode();

if (argc<6 || argc>7) {
   usage();
   return 0;
}
printf("Loading alphabet...\n");
Alphabet* alph=load_alphabet(argv[2]);
if (alph==NULL) {
   fprintf(stderr,"Cannot load alphabet file %s\n",argv[2]);
   return 1;
}
char temp[1024];
struct string_hash* forbiddenWords=NULL;
if (!strcmp(argv[1],"NORWEGIAN")) {
   get_path(argv[3],temp);
   strcat(temp,"ForbiddenWords.txt");
   forbiddenWords=load_word_list(temp);
}
printf("Loading BIN file...\n");
unsigned char* bin=load_BIN_file(argv[3]);
if (bin==NULL) {
   fprintf(stderr,"Cannot load bin file %s\n",argv[3]);
   free_alphabet(alph);
   free_string_hash(forbiddenWords);
   return 1;
}
strcpy(temp,argv[3]);
temp[strlen(argv[3])-3]='\0';
strcat(temp,"inf");
printf("Loading INF file...\n");
struct INF_codes* inf=load_INF_file(temp);
if (inf==NULL) {
   fprintf(stderr,"Cannot load inf file %s\n",temp);
   free_alphabet(alph);
   free(bin);
   free_string_hash(forbiddenWords);
   return 1;
}
char tmp[2000];
strcpy(tmp,argv[4]);
strcat(tmp,".tmp");
FILE* words=u_fopen(argv[4],U_READ);
if (words==NULL) {
   fprintf(stderr,"Cannot open word list file %s\n",argv[4]);
   free_alphabet(alph);
   free(bin);
   free_INF_codes(inf);
   free_string_hash(forbiddenWords);
   // here we return 0 in order to do not block the preprocessing
   // in the Unitex Java interface, if no dictionary was applied
   // so that there is no "err" file
   return 0;
}
FILE* new_unknown_words=u_fopen(tmp,U_WRITE);
if (new_unknown_words==NULL) {
   fprintf(stderr,"Cannot open temporary word list file %s\n",tmp);
   free_alphabet(alph);
   free(bin);
   free_INF_codes(inf);
   u_fclose(words);
   free_string_hash(forbiddenWords);
   return 1;
}

FILE* res=u_fopen(argv[5],U_APPEND);
if (res==NULL) {
   fprintf(stderr,"Cannot open result file %s\n",argv[5]);
   free_alphabet(alph);
   free(bin);
   free_INF_codes(inf);
   u_fclose(words);
   u_fclose(new_unknown_words);
   free_string_hash(forbiddenWords);
   return 1;
}
FILE* debug=NULL;
if (argc==7) {
   debug=u_fopen(argv[6],U_WRITE);
   if (debug==NULL) {
      fprintf(stderr,"Cannot open debug file %s\n",argv[6]);
   }
}

if (!strcmp(argv[1],"GERMAN")) {
   analyse_german_compounds(alph,bin,inf,words,res,debug,new_unknown_words);
}
else if (!strcmp(argv[1],"NORWEGIAN")) {
   analyse_norwegian_unknown_words(alph,bin,inf,words,res,debug,new_unknown_words,forbiddenWords);
}
else if (!strcmp(argv[1],"RUSSIAN")) {
  init_russian();
  analyse_compounds(alph,bin,inf,words,res,debug,new_unknown_words);
}
else {
   fprintf(stderr,"Invalid language argument: %s\n",argv[1]);
   free_alphabet(alph);
   free(bin);
   free_INF_codes(inf);
   u_fclose(words);
   u_fclose(new_unknown_words);
   u_fclose(res);
   if (debug!=NULL) {
      u_fclose(debug);
   }
   free_string_hash(forbiddenWords);
   return 1;
}

free_alphabet(alph);
free(bin);
free_INF_codes(inf);
u_fclose(words);
u_fclose(new_unknown_words);
free_string_hash(forbiddenWords);
remove(argv[4]);
rename(tmp,argv[4]);
u_fclose(res);
if (debug!=NULL) {
   u_fclose(debug);
}
return 0;
}
//---------------------------------------------------------------------------
