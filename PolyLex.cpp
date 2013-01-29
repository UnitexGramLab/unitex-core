/*
 * Unitex
 *
 * Copyright (C) 2001-2013 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "AbstractDelaLoad.h"
#include "String_hash.h"
#include "DutchCompounds.h"
#include "NorwegianCompounds.h"
#include "GermanCompounds.h"
#include "File.h"
#include "GeneralDerivation.h"
#include "RussianCompounds.h"
#include "Error.h"
#include "UnitexGetOpt.h"
#include "PolyLex.h"
#include "CompressedDic.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

enum {DUTCH,GERMAN,NORWEGIAN,RUSSIAN};


const char* usage_PolyLex =
         "Usage: PolyLex [OPTIONS] <list>\n"
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
         "'ForbiddenWords.txt' and stored in the same directory than BIN.\n";


static void usage() {
u_printf("%S",COPYRIGHT);
u_printf(usage_PolyLex);
}


const char* optstring_PolyLex=":DGNRa:d:o:i:hk:q:";
const struct option_TS lopts_PolyLex[]= {
      {"dutch",no_argument_TS,NULL,'D'},
      {"german",no_argument_TS,NULL,'G'},
      {"norwegian",no_argument_TS,NULL,'N'},
      {"russian",no_argument_TS,NULL,'R'},
      {"alphabet",required_argument_TS,NULL,'a'},
      {"dictionary",required_argument_TS,NULL,'d'},
      {"output",required_argument_TS,NULL,'o'},
      {"info",required_argument_TS,NULL,'i'},
      {"input_encoding",required_argument_TS,NULL,'k'},
      {"output_encoding",required_argument_TS,NULL,'q'},
      {"help",no_argument_TS,NULL,'h'},
      {NULL,no_argument_TS,NULL,0}
};


int main_PolyLex(int argc,char* const argv[]) {
if (argc==1) {
   usage();
   return 0;
}

int language=-1;
char alphabet[FILENAME_MAX]="";
char name_bin[FILENAME_MAX]="";
char output[FILENAME_MAX]="";
char info[FILENAME_MAX]="";
VersatileEncodingConfig vec=VEC_DEFAULT;
int val,index=-1;
struct OptVars* vars=new_OptVars();
while (EOF!=(val=getopt_long_TS(argc,argv,optstring_PolyLex,lopts_PolyLex,&index,vars))) {
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
             strcpy(name_bin,vars->optarg);
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
   case 'k': if (vars->optarg[0]=='\0') {
                fatal_error("Empty input_encoding argument\n");
             }
             decode_reading_encoding_parameter(&(vec.mask_encoding_compatibility_input),vars->optarg);
             break;
   case 'q': if (vars->optarg[0]=='\0') {
                fatal_error("Empty output_encoding argument\n");
             }
             decode_writing_encoding_parameter(&(vec.encoding_output),&(vec.bom_output),vars->optarg);
             break;
   case 'h': usage(); return 0;
   case ':': if (index==-1) fatal_error("Missing argument for option -%c\n",vars->optopt);
             else fatal_error("Missing argument for option --%s\n",lopts_PolyLex[index].name);
   case '?': if (index==-1) fatal_error("Invalid option -%c\n",vars->optopt);
             else fatal_error("Invalid option --%s\n",vars->optarg);
             break;
   }
   index=-1;
}

if (vars->optind!=argc-1) {
   fatal_error("Invalid arguments: rerun with --help\n");
}

if (name_bin[0]=='\0') {
   fatal_error("You must specify the .bin dictionary to use\n");
}
if (output[0]=='\0') {
   fatal_error("You must specify the output dictionary file name\n");
}
if (language==-1) {
   fatal_error("You must specify the language\n");
}

Alphabet* alph=NULL;
if (alphabet[0]!='\0') {
   u_printf("Loading alphabet...\n");
   alph=load_alphabet(&vec,alphabet);
   if (alph==NULL) {
      fatal_error("Cannot load alphabet file %s\n",alphabet);
   }
}
char name_inf[FILENAME_MAX];
struct string_hash* forbiddenWords=NULL;
if (language==DUTCH || language==NORWEGIAN) {
   get_path(name_bin,name_inf);
   strcat(name_inf,"ForbiddenWords.txt");
   forbiddenWords=load_key_list(&vec,name_inf);
   if (forbiddenWords==NULL) {
	   /* If there was no file, we don't want to block the process */
	   forbiddenWords=new_string_hash(DONT_USE_VALUES);
   }
}
strcpy(name_inf,name_bin);
name_inf[strlen(name_bin)-3]='\0';
strcat(name_inf,"inf");
Dictionary* d=new_Dictionary(&vec,name_bin,name_inf);
if (d==NULL) {
	fatal_error("Cannot load dictionary %s\n",name_bin);
	   free_alphabet(alph);
	   free_string_hash(forbiddenWords);
	   return 1;
}
char tmp[FILENAME_MAX];
strcpy(tmp,argv[vars->optind]);
strcat(tmp,".tmp");
U_FILE* words=u_fopen(&vec,argv[vars->optind],U_READ);
if (words==NULL) {
   error("Cannot open word list file %s\n",argv[vars->optind]);
   free_alphabet(alph);
   /*free_abstract_BIN(bin,&bin_free);
   free_abstract_INF(inf,&inf_free);*/
   free_Dictionary(d);
   free_string_hash(forbiddenWords);
   // here we return 0 in order to do not block the preprocessing
   // in the Unitex Java interface, if no dictionary was applied
   // so that there is no "err" file
   return 0;
}
U_FILE* new_unknown_words=u_fopen(&vec,tmp,U_WRITE);
if (new_unknown_words==NULL) {
   error("Cannot open temporary word list file %s\n",tmp);
   free_alphabet(alph);
   free_Dictionary(d);
   u_fclose(words);
   free_string_hash(forbiddenWords);
   return 1;
}

U_FILE* res=u_fopen(&vec,output,U_APPEND);
if (res==NULL) {
   error("Cannot open result file %s\n",output);
   free_alphabet(alph);
   free_Dictionary(d);
   u_fclose(words);
   u_fclose(new_unknown_words);
   free_string_hash(forbiddenWords);
   return 1;
}
U_FILE* debug=NULL;
if ((*info)!='\0') {
   debug=u_fopen(&vec,info,U_WRITE);
   if (debug==NULL) {
      error("Cannot open debug file %s\n",info);
   }
}
struct utags UTAG;

switch(language) {
case DUTCH: analyse_dutch_unknown_words(alph,d,words,res,debug,new_unknown_words,forbiddenWords); break;
case GERMAN: analyse_german_compounds(alph,d,words,res,debug,new_unknown_words); break;
case NORWEGIAN: analyse_norwegian_unknown_words(alph,d,words,res,debug,new_unknown_words,forbiddenWords); break;
case RUSSIAN:
   init_russian(&UTAG);
   analyse_compounds(alph,d,words,res,debug,new_unknown_words,UTAG);
   break;
}

free_alphabet(alph);
free_Dictionary(d);
u_fclose(words);
u_fclose(new_unknown_words);
free_string_hash(forbiddenWords);
af_remove(argv[vars->optind]);
af_rename(tmp,argv[vars->optind]);
u_fclose(res);
if (debug!=NULL) {
   u_fclose(debug);
}
free_OptVars(vars);
return 0;
}

} // namespace unitex
