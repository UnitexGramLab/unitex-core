 /*
  * Unitex
  *
  * Copyright (C) 2001-2010 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
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
#include <string.h>
#include <stdlib.h>
#include "Unicode.h"
#include "Fst2.h"
#include "Fst2Check.h"
#include "Copyright.h"
#include "Grf2Fst2_lib.h"
#include "Alphabet.h"
#include "File.h"
#include "LocateConstants.h"
#include "Error.h"
#include "Grf2Fst2.h"
#include "getopt.h"
#include "ProgramInvoker.h"


const char* usage_Grf2Fst2 =
         "Usage : Grf2Fst2 [OPTIONS] <grf>\n"
         "\n"
         "  <grf>: main graph of grammar (must be an absolute path)\n"
         "\n"
         "OPTIONS:\n"
         "  -y/--loop_check: enables the loops/left-recursion detection\n"
         "  -n/--no_loop_check: disables the loops/left-recursion detection (default)\n"
         "  -t/--tfst_check: checks if the given .grf can be considered as a valid sentence\n"
         "                   automaton\n"
         "  -e/--no_empty_graph_warning: no warning will be emitted when a graph matches <E>\n"
         "  -a ALPH/--alphabet=ALPH: name of the alphabet file to use for tokenizing\n"
         "                           lexical units.\n"
         "  -c/--char_by_char: lexical units are single letters. If both -a and -c options are\n"
         "                     unused, lexical units will be sequences of any unicode letters.\n"
         "  -d DIR/--pkgdir=DIR: path of the root dir of all grammar packages\n"
         "  -h/--help: this help\n"
         "\n"
         "Compiles the grammar <grf> and saves the result in a FST2 file\n"
         "stored in the same directory as <grf>.\n";


static void usage() {
u_printf("%S",COPYRIGHT);
u_printf(usage_Grf2Fst2);
}


/**
 * A convenient way to call the main function within a Unitex program.
 */
int pseudo_main_Grf2Fst2(Encoding encoding_output,int bom_output,int mask_encoding_compatibility_input,
                         char* name,int yes_or_no,char* alphabet,
                         int no_empty_graph_warning,int tfst_check) {
ProgramInvoker* invoker=new_ProgramInvoker(main_Grf2Fst2,"main_Grf2Fst2");
add_argument(invoker,name);
add_argument(invoker,yes_or_no?"-y":"-n");
char tmp[FILENAME_MAX];
{
    tmp[0]=0;
    get_reading_encoding_text(tmp,sizeof(tmp)-1,mask_encoding_compatibility_input);
    if (tmp[0] != '\0') {
        add_argument(invoker,"-k");
        add_argument(invoker,tmp);
    }

    tmp[0]=0;
    get_writing_encoding_text(tmp,sizeof(tmp)-1,encoding_output,bom_output);
    if (tmp[0] != '\0') {
        add_argument(invoker,"-q");
        add_argument(invoker,tmp);
    }
}
if (alphabet!=NULL) {
   sprintf(tmp,"-a=%s",alphabet);
   add_argument(invoker,tmp);
}
if (no_empty_graph_warning) {
   add_argument(invoker,"-e");
}
if (tfst_check) {
   add_argument(invoker,"-t");
}
int ret=invoke(invoker);
free_ProgramInvoker(invoker);
return ret;
}


const char* optstring_Grf2Fst2=":ynta:d:echk:q:";
const struct option_TS lopts_Grf2Fst2[]= {
      {"loop_check",no_argument_TS,NULL,'y'},
      {"no_loop_check",no_argument_TS,NULL,'n'},
      {"tfst_check",no_argument_TS,NULL,'t'},
      {"alphabet",required_argument_TS,NULL,'a'},
      {"pkgdir",required_argument_TS,NULL,'d'},
      {"no_empty_graph_warning",no_argument_TS,NULL,'e'},
      {"char_by_char",no_argument_TS,NULL,'c'},
      {"input_encoding",required_argument_TS,NULL,'k'},
      {"output_encoding",required_argument_TS,NULL,'q'},
      {"help",no_argument_TS,NULL,'h'},
      {NULL,no_argument_TS,NULL,0}
};


/**
 * The same than main, but no call to setBufferMode.
 */
int main_Grf2Fst2(int argc,char* argv[]) {
if (argc==1) {
   usage();
   return 0;
}
struct compilation_info* infos=new_compilation_info();
int check_recursion=0,tfst_check=0;
Encoding encoding_output = DEFAULT_ENCODING_OUTPUT;
int bom_output = DEFAULT_BOM_OUTPUT;
int mask_encoding_compatibility_input = DEFAULT_MASK_ENCODING_COMPATIBILITY_INPUT;

int val,index=-1;
struct OptVars* vars=new_OptVars();
while (EOF!=(val=getopt_long_TS(argc,argv,optstring_Grf2Fst2,lopts_Grf2Fst2,&index,vars))) {
   switch(val) {
   case 'y': check_recursion=1; break;
   case 'n': check_recursion=0; break;
   case 't': tfst_check=1;
             /* If we have a tfst sentence graph, we must not report
              * compilation failure in the case of an empty graph. It
              * may be because of a sentence graph previously emptied by ELAG */
             infos->no_empty_graph_warning=1;
             break;
   case 'e': infos->no_empty_graph_warning=1; break;
   case 'c': infos->tokenization_policy=CHAR_BY_CHAR_TOKENIZATION; break;
   case 'a': infos->tokenization_policy=WORD_BY_WORD_TOKENIZATION;
             if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty alphabet file\n");
             }
             infos->alphabet=load_alphabet(vars->optarg);
             if (infos->alphabet==NULL) {
                fatal_error("Cannot load alphabet file %s\n",vars->optarg);
             }
             break;
   case 'd': strcpy(infos->repository,vars->optarg); break;
   case 'h': usage(); return 0;
   case ':': if (index==-1) fatal_error("Missing argument for option -%c\n",vars->optopt);
             else fatal_error("Missing argument for option --%s\n",lopts_Grf2Fst2[index].name);
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
   case '?': if (index==-1) fatal_error("Invalid option -%c\n",vars->optopt);
             else fatal_error("Invalid option --%s\n",vars->optarg);
             break;
   }
   index=-1;
}

if (vars->optind!=argc-1) {
   error("Invalid arguments: rerun with --help\n");
   return 1;
}

char fst2_file_name[FILENAME_MAX];
remove_extension(argv[vars->optind],fst2_file_name);
strcat(fst2_file_name,".fst2");
infos->encoding_output = encoding_output;
infos->bom_output = bom_output;
infos->mask_encoding_compatibility_input = mask_encoding_compatibility_input;
if ((infos->fst2=u_fopen_creating_unitex_text_format(encoding_output,bom_output,fst2_file_name,U_WRITE))==NULL) {
   error("Cannot open file %s\n",fst2_file_name);
   return 1;
}
u_fprintf(infos->fst2,"0000000000\n");
int result=compile_grf(argv[vars->optind],infos);
if (result==0) {
   error("Compilation has failed\n");
   free_compilation_info(infos);
   u_fclose(infos->fst2);
   return 1;
}
free_alphabet(infos->alphabet);
write_tags(infos->fst2,infos->tags);
u_fclose(infos->fst2);
free_OptVars(vars);
write_number_of_graphs(fst2_file_name,infos->graph_names->size-1);
if (check_recursion) {
   if (!OK_for_Locate(fst2_file_name,infos->no_empty_graph_warning)) {
      free_compilation_info(infos);
      return 1;
   }
}
if (tfst_check) {
   if (!valid_sentence_automaton(fst2_file_name)) {
      free_compilation_info(infos);
      return 1;
   }
}
free_compilation_info(infos);
u_printf("Compilation has succeeded\n");
return 0;
}
