 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
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
#include "Unicode.h"
#include "Copyright.h"
#include "Alphabet.h"
#include "DELA.h"
#include "String_hash.h"
#include "DutchCompounds.h"
#include "NorwegianCompounds.h"
#include "GermanCompounds.h"
#include "File.h"
#include "GeneralDerivation.h"
#include "RussianCompounds.h"
#include "Error.h"
#include "getopt.h"

enum {DUTCH,GERMAN,NORWEGIAN,RUSSIAN};


static void usage() {
u_printf("%S",COPYRIGHT);
u_printf("Usage: PolyLex [OPTIONS] <list>\n"
         "\n"
         "  <list>: text file containing the words to be analysed\n"
         "\n"
         "OPTIONS:\n"
         "  -a ALPH/--alphabet=ALPH: alphabet file of the language\n"
         "  -d BIN/--dictionary=BIN: .bin dictionary to use\n"
         "  -o OUT/--output=OUT: text DELAF dictionary where the resulting lines will be stored. If\n"
         "                       this file already exists, the lines are added at the end of it.\n"
         "  -i INFO/--info=INFO: if this optional parameter is precised, it is taken as\n"
         "                       the name of a file which will contain information about\n"
         "                       the analysis\n"
         "  -h/--help: this help\n"
         "\n"
         "Language options:\n"
         "  -D/--dutch\n"
         "  -G/--german\n"
         "  -N/--norwegian\n"
         "  -R/--russian\n"
         "\n"
         "Tries to decompose some words as combinaisons of shortest words.\n"
         "This words are removed from the <list> files.\n"
         "NOTE: when the program is used for Dutch or Norwegian words, it tries to read a text file\n"
         "containing a list of forbidden words. This file is supposed to be named\n"
         "'ForbiddenWords.txt' and stored in the same directory than BIN.\n");
}


int main_PolyLex(int argc,char* argv[]) {
if (argc==1) {
   usage();
   return 0;
}

const char* optstring=":DGNRa:d:o:i:h";
const struct option_TS lopts[]= {
      {"dutch",no_argument_TS,NULL,'D'},
      {"german",no_argument_TS,NULL,'G'},
      {"norwegian",no_argument_TS,NULL,'N'},
      {"russian",no_argument_TS,NULL,'R'},
      {"alphabet",required_argument_TS,NULL,'a'},
      {"dictionary",required_argument_TS,NULL,'d'},
      {"output",required_argument_TS,NULL,'o'},
      {"info",required_argument_TS,NULL,'i'},
      {"help",no_argument_TS,NULL,'h'},
      {NULL,no_argument_TS,NULL,0}
};
int language=-1;
char alphabet[FILENAME_MAX]="";
char dictionary[FILENAME_MAX]="";
char output[FILENAME_MAX]="";
char info[FILENAME_MAX]="";
int val,index=-1;
struct OptVars* vars=new_OptVars();
while (EOF!=(val=getopt_long_TS(argc,argv,optstring,lopts,&index,vars))) {
   switch(val) {
   case 'D': language=DUTCH; break;
   case 'G': language=GERMAN; break;
   case 'N': language=NORWEGIAN; break;
   case 'R': language=RUSSIAN; break;
   case 'a': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty alphabet file name\n");
             }
             strcpy(alphabet,vars->optarg);
             break;
   case 'd': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty dictionary file name\n");
             }
             strcpy(dictionary,vars->optarg);
             break;
   case 'o': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty output file name\n");
             }
             strcpy(output,vars->optarg);
             break;
   case 'i': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty information file name\n");
             }
             strcpy(info,vars->optarg);
             break;
   case 'h': usage(); return 0;
   case ':': if (index==-1) fatal_error("Missing argument for option -%c\n",vars->optopt);
             else fatal_error("Missing argument for option --%s\n",lopts[index].name);
   case '?': if (index==-1) fatal_error("Invalid option -%c\n",vars->optopt);
             else fatal_error("Invalid option --%s\n",vars->optarg);
             break;
   }
   index=-1;
}

if (vars->optind!=argc-1) {
   fatal_error("Invalid arguments: rerun with --help\n");
}

if (alphabet[0]=='\0') {
   fatal_error("You must specify the alphabet file\n");
}
if (dictionary[0]=='\0') {
   fatal_error("You must specify the .bin dictionary to use\n");
}
if (output[0]=='\0') {
   fatal_error("You must specify the output dictionary file name\n");
}
if (language==-1) {
   fatal_error("You must specify the language\n");
}

u_printf("Loading alphabet...\n");
Alphabet* alph=load_alphabet(alphabet);
if (alph==NULL) {
   fatal_error("Cannot load alphabet file %s\n",alphabet);
}
char temp[FILENAME_MAX];
struct string_hash* forbiddenWords=NULL;
if (language==DUTCH || language==NORWEGIAN) {
   get_path(dictionary,temp);
   strcat(temp,"ForbiddenWords.txt");
   forbiddenWords=load_key_list(temp);
}
u_printf("Loading BIN file...\n");
unsigned char* bin=load_BIN_file(dictionary);
if (bin==NULL) {
   error("Cannot load bin file %s\n",dictionary);
   free_alphabet(alph);
   free_string_hash(forbiddenWords);
   return 1;
}
strcpy(temp,dictionary);
temp[strlen(dictionary)-3]='\0';
strcat(temp,"inf");
u_printf("Loading INF file...\n");
struct INF_codes* inf=load_INF_file(temp);
if (inf==NULL) {
   error("Cannot load inf file %s\n",temp);
   free_alphabet(alph);
   free(bin);
   free_string_hash(forbiddenWords);
   return 1;
}
char tmp[FILENAME_MAX];
strcpy(tmp,argv[vars->optind]);
strcat(tmp,".tmp");
U_FILE* words=u_fopen(UTF16_LE,argv[vars->optind],U_READ);
if (words==NULL) {
   error("Cannot open word list file %s\n",argv[vars->optind]);
   free_alphabet(alph);
   free(bin);
   free_INF_codes(inf);
   free_string_hash(forbiddenWords);
   // here we return 0 in order to do not block the preprocessing
   // in the Unitex Java interface, if no dictionary was applied
   // so that there is no "err" file
   return 0;
}
U_FILE* new_unknown_words=u_fopen(UTF16_LE,tmp,U_WRITE);
if (new_unknown_words==NULL) {
   error("Cannot open temporary word list file %s\n",tmp);
   free_alphabet(alph);
   free(bin);
   free_INF_codes(inf);
   u_fclose(words);
   free_string_hash(forbiddenWords);
   return 1;
}

U_FILE* res=u_fopen(UTF16_LE,output,U_APPEND);
if (res==NULL) {
   error("Cannot open result file %s\n",output);
   free_alphabet(alph);
   free(bin);
   free_INF_codes(inf);
   u_fclose(words);
   u_fclose(new_unknown_words);
   free_string_hash(forbiddenWords);
   return 1;
}
U_FILE* debug=NULL;
if (info!=NULL) {
   debug=u_fopen(UTF16_LE,info,U_WRITE);
   if (debug==NULL) {
      error("Cannot open debug file %s\n",info);
   }
}
struct utags UTAG;

switch(language) {
case DUTCH: analyse_dutch_unknown_words(alph,bin,inf,words,res,debug,new_unknown_words,forbiddenWords); break;
case GERMAN: analyse_german_compounds(alph,bin,inf,words,res,debug,new_unknown_words); break;
case NORWEGIAN: analyse_norwegian_unknown_words(alph,bin,inf,words,res,debug,new_unknown_words,forbiddenWords); break;
case RUSSIAN:
   init_russian(&UTAG);
   analyse_compounds(alph,bin,inf,words,res,debug,new_unknown_words,UTAG);
   break;
}

free_alphabet(alph);
free(bin);
free_INF_codes(inf);
u_fclose(words);
u_fclose(new_unknown_words);
free_string_hash(forbiddenWords);
remove(output);
rename(tmp,output);
u_fclose(res);
if (debug!=NULL) {
   u_fclose(debug);
}
free_OptVars(vars);
return 0;
}

