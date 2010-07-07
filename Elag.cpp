/*
 * Unitex
 *
 * Copyright (C) 2001-2010 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "getopt.h"

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
#include "ElagFunctions.h"
#include "Error.h"
#include "File.h"
#include "Tfst.h"
#include "Elag.h"


const char* usage_Elag =
         "Usage: Elag [OPTIONS] <tfst>\n"
         "\n"
         "  <tfst>: input text automaton file\n"
         "\n"
         "OPTIONS:\n"
         "  -l LANG/--language=LANG: language definition file\n"
         "  -r RULES/--rules=RULES: compiled elag rules file\n"
         "  -o OUT/--output=OUT: resulting output .tfst file\n"
         "  -h/--help: this help\n"
         "\n"
         "Disambiguate the input text automaton <tfst> using the specified compiled elag rules.\n";


static void usage() {
u_printf("%S",COPYRIGHT);
u_printf(usage_Elag);
}


const char* optstring_Elag=":l:r:o:hk:q:";
const struct option_TS lopts_Elag[]= {
      {"language",required_argument_TS,NULL,'l'},
      {"rules",required_argument_TS,NULL,'r'},
      {"output",required_argument_TS,NULL,'o'},
      {"input_encoding",required_argument_TS,NULL,'k'},
      {"output_encoding",required_argument_TS,NULL,'q'},
      {"help",no_argument_TS,NULL,'h'},
      {NULL,no_argument_TS,NULL,0}
};


int main_Elag(int argc,char* const argv[]) {
if (argc==1) {
   usage();
   return 0;
}

Encoding encoding_output = DEFAULT_ENCODING_OUTPUT;
int bom_output = DEFAULT_BOM_OUTPUT;
int mask_encoding_compatibility_input = DEFAULT_MASK_ENCODING_COMPATIBILITY_INPUT;
int val,index=-1;
char language[FILENAME_MAX]="";
char rule_file[FILENAME_MAX]="";
char output_tfst[FILENAME_MAX]="";
char directory[FILENAME_MAX]="";
struct OptVars* vars=new_OptVars();
while (EOF!=(val=getopt_long_TS(argc,argv,optstring_Elag,lopts_Elag,&index,vars))) {
   switch(val) {
   case 'l': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty language definition file\n");
             }
             strcpy(language,vars->optarg);
             break;
   case 'r': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty rule file\n");
             }
             strcpy(rule_file,vars->optarg);
             break;
   case 'o': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty output file\n");
             }
             strcpy(output_tfst,vars->optarg);
             break;
   case 'k': if (vars->optarg[0]=='\0') {
                fatal_error("Empty input_encoding argument\n");
             }
             decode_reading_encoding_parameter(&mask_encoding_compatibility_input,vars->optarg);
             break;
   case 'q': if (vars->optarg[0]=='\0') {
                fatal_error("Empty output_encoding argument\n");
             }
             decode_writing_encoding_parameter(&encoding_output,&bom_output,vars->optarg);
             break;
   case 'h': usage(); return 0;
   case ':': if (index==-1) fatal_error("Missing argument for option -%c\n",vars->optopt);
             else fatal_error("Missing argument for option --%s\n",lopts_Elag[index].name);
   case '?': if (index==-1) fatal_error("Invalid option -%c\n",vars->optopt);
             else fatal_error("Invalid option --%s\n",vars->optarg);
             break;
   }
   index=-1;
}

if (language[0]=='\0') {
   fatal_error("You must define the language definition file\n");
}
if (rule_file[0]=='\0') {
   fatal_error("You must define the rule file\n");
}
if (vars->optind!=argc-1) {
   fatal_error("Invalid arguments: rerun with --help\n");
}
char input_tfst[FILENAME_MAX];
strcpy(input_tfst,argv[vars->optind]);

u_printf("Loading %s language definition ...\n",language);
language_t* lang = load_language_definition(language);
if (output_tfst[0]=='\0') {
   remove_extension(input_tfst,output_tfst);
   strcat(output_tfst,"-elag.tfst");
}

get_path(rule_file,directory);
vector_ptr* grammars;
if ((grammars=load_elag_grammars(rule_file,lang,directory)) == NULL) {
   free_language_t(lang);
   free_OptVars(vars);
   fatal_error("Unable to load grammar %s", rule_file);
}
u_printf("Grammars are loaded.\n");
remove_ambiguities(input_tfst,grammars,output_tfst,encoding_output,bom_output,lang);
free_language_t(lang);
free_vector_ptr(grammars,(release_f)free_Fst2Automaton_including_symbols);
free_OptVars(vars);
return 0;
}
