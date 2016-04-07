/*
 * Unitex
 *
 * Copyright (C) 2001-2016 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "Alphabet.h"
#include "DELA.h"
#include "AbstractDelaLoad.h"
#include "File.h"
#include "Copyright.h"
#include "LocateMatches.h"
#include "PortugueseNormalization.h"
#include "String_hash.h"
#include "Sentence2Grf.h"
#include "Fst2.h"
#include "NormalizationFst2.h"
#include "Text_tokens.h"
#include "List_int.h"
#include "Error.h"
#include "UnitexGetOpt.h"
#include "Reconstrucao.h"
#include "CompressedDic.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

const char* usage_Reconstrucao =
         "Usage: Reconstrucao [OPTIONS] <index>\n"
         "\n"
         "  <index> : the match list that describes the forms to be normalized. This\n"
         "            list must have been computed by Locate in MERGE or REPLACE mode.\n"
         "\n"
         "OPTIONS:\n"
         "  -a ALPH/--alphabet=ALPH: the alphabet file to use\n"
         "  -r ROOT/--root=ROOT: the .bin dictionary containing the radical forms\n"
         "  -d BIN/--dictionary=BIN: the .bin dictionary containing the complete forms\n"
         "  -p PRO/--pronoun_rules=PRO: the .fst2 grammar describing pronoun rewriting rules\n"
         "  -n NAS/--nasal_pronoun_rules=NAS: the .fst2 grammar describing nasal pronoun rewriting rules\n"
         "  -o OUT/--output=OUT:  the name of the .grf graph to be generated\n"
         "  -V/--only-verify-arguments: only verify arguments syntax and exit\n"
         "  -h/--help: this help\n"
         "\n"
         "Takes a list of multi-part verbs and creates an apropriate normalization grammar.\n";


static void usage() {
  display_copyright_notice();
  u_printf(usage_Reconstrucao);
}


const char* optstring_Reconstrucao=":a:r:d:p:n:o:Vhk:q:";
const struct option_TS lopts_Reconstrucao[]= {
  {"alphabet",required_argument_TS,NULL,'a'},
  {"root",required_argument_TS,NULL,'r'},
  {"dictionary",required_argument_TS,NULL,'d'},
  {"pronoun_rules",required_argument_TS,NULL,'p'},
  {"nasal_pronoun_rules",required_argument_TS,NULL,'n'},
  {"output",required_argument_TS,NULL,'o'},
  {"input_encoding",required_argument_TS,NULL,'k'},
  {"output_encoding",required_argument_TS,NULL,'q'},
  {"only_verify_arguments",no_argument_TS,NULL,'V'},
  {"help",no_argument_TS,NULL,'h'},
  {NULL,no_argument_TS,NULL,0}
};


int main_Reconstrucao(int argc,char* const argv[]) {
if (argc==1) {
   usage();
   return SUCCESS_RETURN_CODE;
}

char alphabet[FILENAME_MAX]="";
char root[FILENAME_MAX]="";
char dictionary[FILENAME_MAX]="";
char pronoun_rules[FILENAME_MAX]="";
char nasal_pronoun_rules[FILENAME_MAX]="";
char output[FILENAME_MAX]="";
VersatileEncodingConfig vec=VEC_DEFAULT;
int val,index=-1;
bool only_verify_arguments = false;
UnitexGetOpt options;
while (EOF!=(val=options.parse_long(argc,argv,optstring_Reconstrucao,lopts_Reconstrucao,&index))) {
   switch(val) {
   case 'a': if (options.vars()->optarg[0]=='\0') {
                error("You must specify a non empty alphabet file name\n");
                return USAGE_ERROR_CODE;
             }
             strcpy(alphabet,options.vars()->optarg);
             break;
   case 'r': if (options.vars()->optarg[0]=='\0') {
                error("You must specify a non empty root dictionary file name\n");
                return USAGE_ERROR_CODE;                
             }
             strcpy(root,options.vars()->optarg);
             break;
   case 'd': if (options.vars()->optarg[0]=='\0') {
                error("You must specify a non empty dictionary file name\n");
                return USAGE_ERROR_CODE;
             }
             strcpy(dictionary,options.vars()->optarg);
             break;
   case 'p': if (options.vars()->optarg[0]=='\0') {
                error("You must specify a non empty pronoun rewriting rule file name\n");
                return USAGE_ERROR_CODE;
             }
             strcpy(pronoun_rules,options.vars()->optarg);
             break;
   case 'n': if (options.vars()->optarg[0]=='\0') {
                error("You must specify a non empty nasal pronoun rewriting rule file name\n");
                return USAGE_ERROR_CODE;
             }
             strcpy(nasal_pronoun_rules,options.vars()->optarg);
             break;
   case 'o': if (options.vars()->optarg[0]=='\0') {
                error("You must specify a non empty output file name\n");
                return USAGE_ERROR_CODE;
             }
             strcpy(output,options.vars()->optarg);
             break;
   case 'k': if (options.vars()->optarg[0]=='\0') {
                error("Empty input_encoding argument\n");
                return USAGE_ERROR_CODE;                
             }
             decode_reading_encoding_parameter(&(vec.mask_encoding_compatibility_input),options.vars()->optarg);
             break;
   case 'q': if (options.vars()->optarg[0]=='\0') {
                error("Empty output_encoding argument\n");
                return USAGE_ERROR_CODE;
             }
             decode_writing_encoding_parameter(&(vec.encoding_output),&(vec.bom_output),options.vars()->optarg);
             break;
   case 'V': only_verify_arguments = true;
             break;  
   case 'h': usage(); 
             return SUCCESS_RETURN_CODE;
   case ':': index==-1 ? error("Missing argument for option -%c\n",options.vars()->optopt) :
                         error("Missing argument for option --%s\n",lopts_Reconstrucao[index].name);
             return USAGE_ERROR_CODE;
   case '?': index==-1 ? error("Invalid option -%c\n",options.vars()->optopt) :
                         error("Invalid option --%s\n",options.vars()->optarg);
             return USAGE_ERROR_CODE;
   }
   index=-1;
}

if (options.vars()->optind!=argc-1) {
   error("Invalid arguments: rerun with --help\n");
   return USAGE_ERROR_CODE;
}

if (root[0]=='\0') {
   error("You must specify the root .bin dictionary\n");
   return USAGE_ERROR_CODE;
}

if (dictionary[0]=='\0') {
   error("You must specify the .bin dictionary to use\n");
   return USAGE_ERROR_CODE;
}

if (pronoun_rules[0]=='\0') {
   error("You must specify the pronoun rule file\n");
   return USAGE_ERROR_CODE;
}

if (nasal_pronoun_rules[0]=='\0') {
   error("You must specify the nasal pronoun rule file\n");
   return USAGE_ERROR_CODE;
}

if (output[0]=='\0') {
   error("You must specify the output dictionary file name\n");
   return USAGE_ERROR_CODE;
}

if (only_verify_arguments) {
  // freeing all allocated memory
  return SUCCESS_RETURN_CODE;
}

Alphabet* alph=NULL;
if (alphabet[0]!='\0') {
   u_printf("Loading alphabet...\n");
   alph=load_alphabet(&vec,alphabet);
   if (alph==NULL) {
      error("Cannot load alphabet file %s\n",alphabet);
      return DEFAULT_ERROR_CODE;
   }
}

u_printf("Loading match list...\n");
U_FILE* f_list=u_fopen(&vec,argv[options.vars()->optind],U_READ);
if (f_list==NULL) {
   error("Cannot load match list %s\n",argv[options.vars()->optind]);
   free_alphabet(alph);
   return DEFAULT_ERROR_CODE;
}

OutputPolicy output_policy;
struct match_list* list=load_match_list(f_list,&output_policy,NULL);
u_fclose(f_list);

if (output_policy==IGNORE_OUTPUTS) {
   error("Invalid match list %s\n",argv[options.vars()->optind]);
   free_alphabet(alph);
   return DEFAULT_ERROR_CODE;
}

char root_inf_file[FILENAME_MAX];
remove_extension(root,root_inf_file);
strcat(root_inf_file,".inf");

u_printf("Loading radical form dictionary...\n");
Dictionary* root_dic=new_Dictionary(&vec,root,root_inf_file);
if ((*root)=='\0') {
	free_alphabet(alph);
	return DEFAULT_ERROR_CODE;
}

u_printf("Loading inflected form dictionary...\n");
char inflected_inf_file[FILENAME_MAX];
remove_extension(dictionary,inflected_inf_file);
strcat(inflected_inf_file,".inf");
Dictionary* inflected_dic=new_Dictionary(&vec,dictionary,inflected_inf_file);
if (inflected_dic==NULL) {
	free_Dictionary(root_dic);
	free_alphabet(alph);
	return DEFAULT_ERROR_CODE;
}

u_printf("Loading pronoun rewriting rule grammar...\n");
struct normalization_tree* rewriting_rules=load_normalization_transducer_string(&vec,pronoun_rules);
if (rewriting_rules==NULL) {
   error("Cannot load pronoun rewriting grammar %s\n",pronoun_rules);
   free_alphabet(alph);
   free_Dictionary(root_dic);
   free_Dictionary(inflected_dic);
   return DEFAULT_ERROR_CODE;
}

u_printf("Loading nasal pronoun rewriting rule grammar...\n");
struct normalization_tree* nasal_rewriting_rules=load_normalization_transducer_string(&vec,nasal_pronoun_rules);
if (rewriting_rules==NULL) {
   error("Cannot load nasal pronoun rewriting grammar %s\n",nasal_pronoun_rules);
   free_alphabet(alph);
   free_Dictionary(root_dic);
   free_Dictionary(inflected_dic);
   free_normalization_tree(rewriting_rules);
   return DEFAULT_ERROR_CODE;
}
u_printf("Constructing normalization grammar...\n");
build_portuguese_normalization_grammar(alph,list,root_dic,inflected_dic,output,
                                       &vec,rewriting_rules,nasal_rewriting_rules);
free_alphabet(alph);
free_Dictionary(root_dic);
free_Dictionary(inflected_dic);
free_normalization_tree(rewriting_rules);
free_normalization_tree(nasal_rewriting_rules);

return SUCCESS_RETURN_CODE;
}

} // namespace unitex

