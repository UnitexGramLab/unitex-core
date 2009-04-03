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

#ifdef __GNUC__  // gcc 
#include <unistd.h>

#elif  defined(__VISUALC__)

//  #ifdef __VISUALC__  // visual studio

#include <DIRECT.H>

#else    // Borland

#include <dir.h>
#endif

#include "Unicode.h"
#include "Copyright.h"
#include "Fst2Automaton.h"
#include "ElagFunctions.h"
#include "utils.h"
#include "Error.h"
#include "getopt.h"
#include "File.h"
#include "Tfst.h"


static void usage() {
u_printf("%S", COPYRIGHT);
u_printf("Usage: Elag [OPTIONS] <tfst>\n"
         "\n"
         "  <tfst>: input text automaton file\n"
         "\n"
         "OPTIONS:\n"
         "  -l LANG/--language=LANG: language definition file\n"
         "  -r RULES/--rules=RULES: compiled elag rules file\n"
         "  -o OUT/--output=OUT: resulting output .tfst file\n"
         "  -d DIR/--directory=DIR: directory where elag rules are located\n"
         "  -h/--help: this help\n"
         "\n"
         "Disambiguate the input text automaton <tfst> using the specified compiled elag rules.\n");
}


int main_Elag(int argc,char* argv[]) {
if (argc==1) {
   usage();
   return 0;
}

const char* optstring=":l:r:o:d:h";
const struct option lopts[]= {
      {"language",required_argument,NULL,'l'},
      {"rules",required_argument,NULL,'r'},
      {"output",required_argument,NULL,'o'},
      {"directory",required_argument,NULL,'d'},
      {"help",no_argument,NULL,'h'},
      {NULL,no_argument,NULL,0}
};
int val,index=-1;
char language[FILENAME_MAX]="";
char rule_file[FILENAME_MAX]="";
char output_tfst[FILENAME_MAX]="";
char directory[FILENAME_MAX]="";
optind=1;
while (EOF!=(val=getopt_long(argc,argv,optstring,lopts,&index))) {
   switch(val) {
   case 'l': if (optarg[0]=='\0') {
                fatal_error("You must specify a non empty language definition file\n");
             }
             strcpy(language,optarg);
             break;
   case 'r': if (optarg[0]=='\0') {
                fatal_error("You must specify a non empty rule file\n");
             }
             strcpy(rule_file,optarg);
             break;
   case 'o': if (optarg[0]=='\0') {
                fatal_error("You must specify a non empty output file\n");
             }
             strcpy(output_tfst,optarg);
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

if (language[0]=='\0') {
   fatal_error("You must define the language definition file\n");
}
if (rule_file[0]=='\0') {
   fatal_error("You must define the rule file\n");
}
if (optind!=argc-1) {
   fatal_error("Invalid arguments: rerun with --help\n");
}
char input_tfst[FILENAME_MAX];
strcpy(input_tfst,argv[optind]);

u_printf("Loading %s langage definition ...\n", language);
language_t* lang = load_language_definition(language);
set_current_language(lang);
if (output_tfst[0]=='\0') {
   remove_extension(input_tfst,output_tfst);
   strcat(output_tfst,"-elag.tfst");
}

if (directory[0]=='\0') {
   get_path(rule_file,directory);
   char tmp[FILENAME_MAX];
   strcpy(tmp,rule_file);
   remove_path(tmp,rule_file);
}
u_printf("Changing to %s directory\n",directory);
if (chdir(directory)==-1) {
   error("Unable to change to %s directory.\n", directory);
}
vector_ptr* grammars;
if ((grammars=load_elag_grammars(rule_file)) == NULL) {
   fatal_error("Unable to load grammar %s", rule_file);
}
u_printf("Grammars are loaded.\n");
remove_ambiguities(input_tfst,grammars,output_tfst);
free_vector_ptr(grammars,(release_f)free_Fst2Automaton);
return 0;
}
