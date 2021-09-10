/*
 * Unitex
 *
 * Copyright (C) 2001-2021 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "UnitexGetOpt.h"

#ifdef __GNUC__
#include <unistd.h>
#elif ((defined(__VISUALC__)) || defined(_MSC_VER))
#include <direct.h>
#else
#include <dir.h>
#endif

#include "Unicode.h"
#include "Copyright.h"
#include "Fst2Automaton.h"
#include "HashTable.h"
#include "ElagFunctions.h"
#include "Error.h"
#include "File.h"
#include "Tfst.h"
#include "Elag.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

const char* usage_Elag =
         "Usage: Elag [OPTIONS] <tfst>\n"
         "\n"
         "  <tfst>: input text automaton file\n"
         "\n"
         "OPTIONS:\n"
         "  -l LANG/--language=LANG: language definition file\n"
         "  -r RULES/--rules=RULES: compiled elag rules file\n"
         "  -o OUT/--output=OUT: resulting output .tfst file\n"
         "  -S/--no_statistics: do not produce statistics file\n"
         "  -V/--only-verify-arguments: only verify arguments syntax and exit\n"
         "  -h/--help: this help\n"
         "\n"
         "Disambiguate the input text automaton <tfst> using the specified compiled elag rules.\n";


static void usage() {
  display_copyright_notice();
  u_printf(usage_Elag);
}


const char* optstring_Elag=":l:r:o:Vhk:q:S";
const struct option_TS lopts_Elag[]= {
  {"language",required_argument_TS,NULL,'l'},
  {"rules",required_argument_TS,NULL,'r'},
  {"output",required_argument_TS,NULL,'o'},
  {"input_encoding",required_argument_TS,NULL,'k'},
  {"output_encoding",required_argument_TS,NULL,'q'},
  {"only_verify_arguments",no_argument_TS,NULL,'V'},
  {"no_statistics",no_argument_TS,NULL,'S'},
  {"help",no_argument_TS,NULL,'h'},
  {NULL,no_argument_TS,NULL,0}
};


int main_Elag(int argc,char* const argv[]) {
if (argc==1) {
   usage();
   return SUCCESS_RETURN_CODE;
}

VersatileEncodingConfig vec=VEC_DEFAULT;
int val,index=-1;
int save_statistics=1;
char language[FILENAME_MAX]="";
char rule_file[FILENAME_MAX]="";
char output_tfst[FILENAME_MAX]="";
char directory[FILENAME_MAX]="";
bool only_verify_arguments = false;
UnitexGetOpt options;

while (EOF!=(val=options.parse_long(argc,argv,optstring_Elag,lopts_Elag,&index))) {
   switch(val) {
   case 'l': if (options.vars()->optarg[0]=='\0') {
                error("You must specify a non empty language definition file\n");
                return USAGE_ERROR_CODE;
             }
             strcpy(language,options.vars()->optarg);
             break;
   case 'r': if (options.vars()->optarg[0]=='\0') {
                error("You must specify a non empty rule file\n");
                return USAGE_ERROR_CODE;
             }
             strcpy(rule_file,options.vars()->optarg);
             break;
   case 'o': if (options.vars()->optarg[0]=='\0') {
                error("You must specify a non empty output file\n");
                return USAGE_ERROR_CODE;
             }
             strcpy(output_tfst,options.vars()->optarg);
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
   case 'S': save_statistics = 0;
             break;
   case 'V': only_verify_arguments = true;
             break;
   case 'h': usage();
             return SUCCESS_RETURN_CODE;
   case ':': index==-1 ? error("Missing argument for option -%c\n",options.vars()->optopt) :
                         error("Missing argument for option --%s\n",lopts_Elag[index].name);
             return USAGE_ERROR_CODE;
   case '?': index==-1 ? error("Invalid option -%c\n",options.vars()->optopt) :
                         error("Invalid option --%s\n",options.vars()->optarg);
             return USAGE_ERROR_CODE;
   }
   index=-1;
}

if (language[0]=='\0') {
   error("You must define the language definition file\n");
   return USAGE_ERROR_CODE;
}

if (rule_file[0]=='\0') {
   error("You must define the rule file\n");
   return USAGE_ERROR_CODE;
}

if (options.vars()->optind!=argc-1) {
   error("Invalid arguments: rerun with --help\n");
   return USAGE_ERROR_CODE;
}

if (only_verify_arguments) {
  // freeing all allocated memory
  return SUCCESS_RETURN_CODE;
}

char input_tfst[FILENAME_MAX];
strcpy(input_tfst,argv[options.vars()->optind]);

u_printf("Loading %s language definition ...\n",language);
language_t* lang = load_language_definition(&vec,language);
if (output_tfst[0]=='\0') {
   remove_extension(input_tfst,output_tfst);
   strcat(output_tfst,"-elag.tfst");
}

get_path(rule_file,directory);
vector_ptr* grammars;
if ((grammars=load_elag_grammars(&vec,rule_file,lang,directory)) == NULL) {
   error("Unable to load grammar %s", rule_file);
   free_language_t(lang);
   return DEFAULT_ERROR_CODE;
}
u_printf("Grammars are loaded.\n");

remove_ambiguities(input_tfst,grammars,output_tfst,&vec,lang,save_statistics);
free_vector_ptr(grammars,(release_f)free_Fst2Automaton_including_symbols);
free_language_t(lang);
return SUCCESS_RETURN_CODE;
}

} // namespace unitex
