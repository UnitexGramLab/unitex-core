/*
 * Unitex
 *
 * Copyright (C) 2001-2020 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

/*
 * File created and contributed by Gilles Vollant (Ergonotics SAS)
 * as part of an UNITEX optimization and reliability effort
 *
 * additional information: http://www.smartversion.com/unitex-contribution/
 * contact : info@winimage.com
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Unicode.h"
#include "Fst2.h"
#include "Fst2Check_lib.h"
#include "Copyright.h"
#include "Alphabet.h"
#include "File.h"
#include "LocateConstants.h"
#include "Error.h"
#include "UnitexGetOpt.h"
#include "ProgramInvoker.h"

#include "Fst2Check.h"


#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

void display_fst2_stat(Fst2* fst2,U_FILE*ferr)
{
int n=0;
for (int i=0;i<fst2->number_of_states;i++) {
Transition* t=fst2->states[i]->transitions; while (t!=NULL) {
n++;
t=t->next;
}
}
u_printf("%d graphs, %d states, %d transitions\n",fst2->number_of_graphs,fst2->number_of_states,n);
if (ferr != NULL) {
    u_fprintf(ferr,"%d graphs, %d states, %d transitions\n",fst2->number_of_graphs,fst2->number_of_states,n);
}
}


int display_fst2_file_stat(const VersatileEncodingConfig* vec,const char* name,U_FILE* ferr) {
struct FST2_free_info fst2_free;
Fst2* fst2=load_abstract_fst2(vec,name,1,&fst2_free);
char name_without_path[FILENAME_MAX];
remove_path(name,name_without_path);
if (fst2==NULL) {
    error("Cannot load graph %s\n",name);
    if (ferr != NULL)
        u_fprintf(ferr,"Cannot load graph %s\n",name);
    return 0;
}
u_printf("Statistics of graph %s: ",name_without_path);
if (ferr != NULL)
  u_fprintf(ferr,"Statistics of graph %s: ",name_without_path);
display_fst2_stat(fst2,ferr);

free_abstract_Fst2(fst2,&fst2_free);
return 1;
}


const char* usage_Fst2Check =
         "Usage : Fst2Check [OPTIONS] <fst2>\n"
         "\n"
         "  <fst2>: the grammar to be checked\n"
         "\n"
         "OPTIONS:\n"
         "  -y/--loop_check: enables the loops/left-recursion detection\n"
         "  -n/--no_loop_check: disables the loops/left-recursion detection (default)\n"
         "  -t/--tfst_check: checks if the given .fst2 can be considered as a valid sentence\n"
         "                   automaton\n"
         "  -e/--no_empty_graph_warning: no warning will be emitted when a graph matches <E>\n"
         "  -o OUT/--output=OUT: output file for error message\n"
         "  -a/--append: opens the message output file in append mode\n"
         "  -s/--statistics: displays statistics about the .fst2 file\n"
         "  -V/--only-verify-arguments: only verify arguments syntax and exit\n"
         "  -h/--help: this help\n"
         "\n"
         "This program checks if a .fst2 file has no error for Locate.\n";


static void usage() {
  display_copyright_notice();
  u_printf(usage_Fst2Check);
}


/**
 * A convenient way to call the main function within a Unitex program.
 */

int pseudo_main_Fst2Check(const VersatileEncodingConfig* vec,
                          const char* fst2name,const char* output_name,int append,int display_statistics,
                          int yes_or_no,int no_empty_graph_warning,int tfst_check) {
ProgramInvoker* invoker=new_ProgramInvoker(main_Fst2Check,"main_Fst2Check");
add_argument(invoker,fst2name);
add_argument(invoker,yes_or_no?"-y":"-n");
char tmp[FILENAME_MAX];
{
    tmp[0]=0;
    get_reading_encoding_text(tmp,sizeof(tmp)-1,vec->mask_encoding_compatibility_input);
    if (tmp[0] != '\0') {
        add_argument(invoker,"-k");
        add_argument(invoker,tmp);
    }

    tmp[0]=0;
    get_writing_encoding_text(tmp,sizeof(tmp)-1,vec->encoding_output,vec->bom_output);
    if (tmp[0] != '\0') {
        add_argument(invoker,"-q");
        add_argument(invoker,tmp);
    }
}

if (output_name!=NULL)
{
   add_argument(invoker,"-o");
   add_argument(invoker,output_name);
}

if (append) {
   add_argument(invoker,"-p");
}
if (display_statistics) {
   add_argument(invoker,"-s");
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

const char* optstring_Fst2Check=":ynatesVho:k:q:";
const struct option_TS lopts_Fst2Check[]= {
  {"append",no_argument_TS,NULL,'a'},
  {"statistics",no_argument_TS,NULL,'s'},
  {"loop_check",no_argument_TS,NULL,'y'},
  {"no_loop_check",no_argument_TS,NULL,'n'},
  {"tfst_check",no_argument_TS,NULL,'t'},
  {"no_empty_graph_warning",no_argument_TS,NULL,'e'},
  {"input_encoding",required_argument_TS,NULL,'k'},
  {"output_encoding",required_argument_TS,NULL,'q'},
  {"output",required_argument_TS,NULL,'o'},
  {"only_verify_arguments",no_argument_TS,NULL,'V'},
  {"help",no_argument_TS,NULL,'h'},
  {NULL,no_argument_TS,NULL,0}
};

/**
 * The same than main, but no call to setBufferMode.
 */
int main_Fst2Check(int argc,char* const argv[]) {
if (argc==1) {
   usage();
   return SUCCESS_RETURN_CODE;
}

int check_recursion=0,tfst_check=0;
int append_output=0;
int display_statistics=0;
char no_empty_graph_warning=0;
char output[FILENAME_MAX]="";
VersatileEncodingConfig vec=VEC_DEFAULT;
int val,index=-1;
bool only_verify_arguments = false;
UnitexGetOpt options;

while (EOF!=(val=options.parse_long(argc,argv,optstring_Fst2Check,lopts_Fst2Check,&index))) {
   switch(val) {
   case 'a': append_output=1; break;
   case 'y': check_recursion=1; break;
   case 'n': check_recursion=0; break;
   case 's': display_statistics=1; break;
   case 't': tfst_check=1;
             /* If we have a tfst sentence graph, we must not report
              * compilation failure in the case of an empty graph. It
              * may be because of a sentence graph previously emptied by ELAG */
             no_empty_graph_warning=1;
             break;
   case 'e': no_empty_graph_warning=1; break;
   case 'V': only_verify_arguments = true;
             break;
   case 'h': usage();
             return SUCCESS_RETURN_CODE;
   case ':': index==-1 ? error("Missing argument for option -%c\n",options.vars()->optopt) :
                         error("Missing argument for option --%s\n",lopts_Fst2Check[index].name);
             return USAGE_ERROR_CODE;
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
   case 'o': if (options.vars()->optarg[0]=='\0') {
                   error("You must specify a non empty output file name\n");
                   return USAGE_ERROR_CODE;
                }
                strcpy(output,options.vars()->optarg);
                break;
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

if (only_verify_arguments) {
  // freeing all allocated memory
  return SUCCESS_RETURN_CODE;
}

char fst2_file_name[FILENAME_MAX];
remove_extension(argv[options.vars()->optind],fst2_file_name);
strcpy(fst2_file_name,argv[options.vars()->optind]);
U_FILE* ferr=NULL;

if (output[0]!=0) {
  if (append_output == 0) {
      ferr=u_fopen(&vec,output,U_WRITE);
  }
  else {
      ferr=u_fopen(&vec,output,U_APPEND);
  }
}

if (display_statistics) {
    display_fst2_file_stat(&vec,fst2_file_name,ferr);
}

if (check_recursion) {
   if (!OK_for_Locate_write_error(&vec,fst2_file_name,no_empty_graph_warning,ferr)) {
      u_fclose(ferr);
      return DEFAULT_ERROR_CODE;
   }
}

if (tfst_check) {
   if (!valid_sentence_automaton_write_error(&vec,fst2_file_name,ferr)) {
      u_fclose(ferr);
      return DEFAULT_ERROR_CODE;
   }
}

if ((check_recursion) || (tfst_check)) {
  u_printf("%s fst2 check has succeeded\n",fst2_file_name);
}

u_fclose(ferr);
return SUCCESS_RETURN_CODE;
}

} // namespace unitex
