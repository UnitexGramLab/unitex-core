/*
 * Unitex
 *
 * Copyright (C) 2001-2015 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "ElagComp.h"


#ifdef __GNUC__
#include <unistd.h>
#elif ((defined(__VISUALC__)) || defined(_MSC_VER))
#include <direct.h>
#else
#include <dir.h>
#endif

#include "Unicode.h"
#include "Copyright.h"
#include "LanguageDefinition.h"
#include "Fst2Automaton.h"
#include "ElagRulesCompilation.h"
#include "File.h"


#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

const char* usage_ElagComp =
         "Usage: ElagComp [OPTIONS]\n"
         "\n"
         "OPTIONS:\n"
         "  -r RULES/--rules=RULES: Elag .fst2 grammar list file\n"
         "  -g GRAMMAR/--grammar=GRAMMAR: Elag .fst2 grammar\n"
         "  -l LANG/--language=LANG: Elag language description file\n"
         "  -o OUT/--output=OUT: output file where the resulting compiled grammar is stored\n"
         "                       The default name is same as RULES except for the .rul extension\n"
         "  -h/--help: this help\n"
         "\n"
         "ElagComp compiles one Elag grammar specified by GRAMMAR or all the grammars\n"
         "specified in the RULES file. The result is stored into the file OUT\n"
         "for later use by the Elag text disambiguation program.\n";


static void usage() {
u_printf("%S",COPYRIGHT);
u_printf(usage_ElagComp);
}


const char* optstring_ElagComp=":l:r:o:g:hk:q:";
const struct option_TS lopts_ElagComp[]= {
      {"language",required_argument_TS,NULL,'l'},
      {"rulelist",required_argument_TS,NULL,'r'},
      {"grammar",required_argument_TS,NULL,'g'},
      {"output",required_argument_TS,NULL,'o'},
      {"input_encoding",required_argument_TS,NULL,'k'},
      {"output_encoding",required_argument_TS,NULL,'q'},
      {"help",no_argument_TS,NULL,'h'},
      {NULL,no_argument_TS,NULL,0}
};


int main_ElagComp(int argc,char* const argv[]) {
if (argc==1) {
   usage();
   return 0;
}

VersatileEncodingConfig vec=VEC_DEFAULT;
int val,index=-1;
char compilename[FILENAME_MAX]="";
char directory[FILENAME_MAX]="";
char grammar[FILENAME_MAX]="";
char rule_file[FILENAME_MAX]="";
char lang[FILENAME_MAX]="";
struct OptVars* vars=new_OptVars();
while (EOF!=(val=getopt_long_TS(argc,argv,optstring_ElagComp,lopts_ElagComp,&index,vars))) {
   switch(val) {
   case 'l': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty language definition file\n");
             }
             strcpy(lang,vars->optarg);
             break;
   case 'r': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty rule file\n");
             }
             strcpy(rule_file,vars->optarg);
             get_path(rule_file,directory);
             break;
   case 'g': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty grammar file name\n");
             }
             strcpy(grammar,vars->optarg);
             get_path(grammar,directory);
             break;
   case 'o': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty output file\n");
             }
             strcpy(compilename,vars->optarg);
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
             else fatal_error("Missing argument for option --%s\n",lopts_ElagComp[index].name);
   case '?': if (index==-1) fatal_error("Invalid option -%c\n",vars->optopt);
             else fatal_error("Invalid option --%s\n",vars->optarg);
             break;
   }
   index=-1;
}

if (lang[0]=='\0') {
   fatal_error("You must define the language definition file\n");
}
if ((rule_file[0]=='\0' && grammar[0]=='\0')
     || (rule_file[0]!='\0' && grammar[0]!='\0')) {
   fatal_error("You must define a rule list OR a grammar\n");
}
if (vars->optind!=argc) {
   fatal_error("Invalid arguments: rerun with --help\n");
}

language_t* language=load_language_definition(&vec,lang);
if (rule_file[0]=='\0' && grammar[0]=='\0') {
   fatal_error("You must specified a grammar or a rule file name\n");
}
if (rule_file[0]!='\0' && grammar[0]!='\0') {
   fatal_error("Cannot handle both a rule file and a grammar\n");
}
if (rule_file[0]!='\0') {
   /* If we work with a rule list */
   if (compilename[0]=='\0') {
      int l=(int)strlen(rule_file);
      if (strcmp(rule_file+l-4,".lst")==0) {
         strcpy(compilename,rule_file);
         strcpy(compilename+l-4,".rul");
      } else {
         sprintf(compilename,"%s.rul",rule_file);
      }
   }
   if (compile_elag_rules(rule_file,compilename,&vec,language)==-1) {
      error("An error occurred\n");
      return 1;
   }
   u_printf("\nElag grammars are compiled in %s.\n",compilename);
} else {
   /* If we must compile a single grammar */
   char elg_file[FILENAME_MAX];
   get_extension(grammar,elg_file);
   if (strcmp(elg_file,".fst2")) {
     fatal_error("Grammar '%s' should be a .fst2 file\n");
   }
   remove_extension(grammar,elg_file);
   strcat(elg_file,".elg");
   if (compile_elag_grammar(grammar,elg_file,&vec,language)==-1) {
     error("An error occured while compiling %s\n",grammar);
     free_language_t(language);
     free_OptVars(vars);
     return 1;
   }
   u_printf("Elag grammar is compiled into %s.\n",elg_file);
}
free_language_t(language);
free_OptVars(vars);
return 0;
}

} // namespace unitex
