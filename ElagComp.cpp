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

#ifdef __GNUC__ 
#include <unistd.h>
#elif defined(__VISUALC__)
#include <DIRECT.H>
#else
#include <dir.h>
#endif

#include "Unicode.h"
#include "Copyright.h"
#include "LanguageDefinition.h"
#include "Fst2Automaton.h"
#include "ElagRulesCompilation.h"
#include "Utils.h"
#include "File.h"
#include "getopt.h"



static void usage() {
u_printf("%S",COPYRIGHT);
u_printf("Usage: ElagComp [OPTIONS]\n"
         "\n"
         "OPTIONS:\n"
         "  -r RULES/--rules=RULES: Elag .fst2 grammar list file\n"
         "  -g GRAMMAR/--grammar=GRAMMAR: Elag .fst2 grammar\n"
         "  -l LANG/--language=LANG: Elag language description file\n"
         "  -o OUT/--output=OUT: output file where the resulting compiled grammar is stored\n"
         "                       The default name is same as RULES except for the .rul extension\n"
         "  -d DIR/--directory=DIR: directory where Elag grammars are located\n"
         "  -h/--help: this help\n"
         "\n"
         "ElagComp compiles one Elag grammar specified by GRAMMAR or all the grammars\n"
         "specified in the RULES file. The result is stored into the file OUT\n"
         "for later use by the Elag text disambiguation program.\n");
}


int main_ElagComp(int argc,char* argv[]) {
if (argc==1) {
   usage();
   return 0;
}

const char* optstring=":l:r:o:d:g:h";
const struct option lopts[]= {
      {"language",required_argument,NULL,'l'},
      {"rulelist",required_argument,NULL,'r'},
      {"grammar",required_argument,NULL,'g'},
      {"output",required_argument,NULL,'o'},
      {"directory",required_argument,NULL,'d'},
      {"help",no_argument,NULL,'h'},
      {NULL,no_argument,NULL,0}
};
int val,index=-1;
char compilename[FILENAME_MAX]="";
char directory[FILENAME_MAX]="";
char grammar[FILENAME_MAX]="";
char rule_file[FILENAME_MAX]="";
char lang[FILENAME_MAX]="";
optind=1;
while (EOF!=(val=getopt_long(argc,argv,optstring,lopts,&index))) {
   switch(val) {
   case 'l': if (optarg[0]=='\0') {
                fatal_error("You must specify a non empty language definition file\n");
             }
             strcpy(lang,optarg);
             break;
   case 'r': if (optarg[0]=='\0') {
                fatal_error("You must specify a non empty rule file\n");
             }
             strcpy(rule_file,optarg);
             break;
   case 'g': if (optarg[0]=='\0') {
                fatal_error("You must specify a non empty grammar file name\n");
             }
             strcpy(grammar,optarg);
             break;
   case 'o': if (optarg[0]=='\0') {
                fatal_error("You must specify a non empty output file\n");
             }
             strcpy(compilename,optarg);
             break;
   case 'd': if (optarg[0]=='\0') {
                fatal_error("You must specify a non empty directory\n");
             }
             strcpy(directory,optarg);
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

if (lang[0]=='\0') {
   fatal_error("You must define the language definition file\n");
}
if ((rule_file[0]=='\0' && grammar[0]=='\0')
     || (rule_file[0]!='\0' && grammar[0]!='\0')) {
   fatal_error("You must define a rule list OR a grammar\n");
}
if (optind!=argc) {
   fatal_error("Invalid arguments: rerun with --help\n");
}

language_t* language=load_language_definition(lang);
set_current_language(language);
if (rule_file[0]=='\0' && grammar[0]=='\0') {
   fatal_error("You must specified a grammar or a rule file name\n");
}
if (rule_file[0]!='\0' && grammar[0]!='\0') {
   fatal_error("Cannot handle both a rule file and a grammar\n");
}
if (rule_file[0]!='\0') {
   /* If we work with a rule list */
   char rule_file_name[FILENAME_MAX];
   if (directory==NULL) {
      get_path(rule_file,directory);
      remove_path(rule_file,rule_file_name);
      strcpy(rule_file,rule_file_name);
   }
   if (chdir(directory)==-1) {
      fatal_error("Unable to change to %s directory\n",directory);
   }
   if (compilename[0]=='\0') {
      int l=strlen(rule_file);
      if (strcmp(rule_file+l-4,".lst")==0) {
         strcpy(compilename,rule_file);
         strcpy(compilename+l-4,".rul");
      } else {
         sprintf(compilename,"%s.rul",rule_file);
      }
   }
   if (compile_elag_rules(rule_file,compilename)==-1) {
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
   if (compile_elag_grammar(grammar,elg_file)==-1) {
     error("An error occured while compiling %s\n",grammar);
     return 1;
   }
   u_printf("Elag grammar is compiled into %s.\n",elg_file);
}
return 0;
}

