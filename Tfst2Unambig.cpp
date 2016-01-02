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
#include "Tfst.h"
#include "Copyright.h"
#include "LinearAutomaton2Txt.h"
#include "Error.h"
#include "UnitexGetOpt.h"
#include "Tfst2Unambig.h"


#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

const char* usage_Tfst2Unambig =
         "Usage: Tfst2Unambig  [OPTIONS] <tfst>\n"
         "\n"
         "  <tfst>: .tfst file representing the text automaton\n"
         "\n"
         "OPTIONS:\n"
         "  -o TXT/--out=TXT : output unicode text file\n"
         "  -V/--only-verify-arguments: only verify arguments syntax and exit\n"
         "  -h/--help: this help\n"
         "\n"
         "Converts a linear Unitex text automaton into a text file. If\n"
         "the automaton is not linear, the process is aborted.\n";


static void usage() {
  display_copyright_notice();
  u_printf(usage_Tfst2Unambig);
}

const char* optstring_Tfst2Unambig=":o:Vhk:q:";

const struct option_TS lopts_Tfst2Unambig[]= {
  {"out",required_argument_TS,NULL,'o'},
  {"input_encoding",required_argument_TS,NULL,'k'},
  {"output_encoding",required_argument_TS,NULL,'q'},
  {"only_verify_arguments",no_argument_TS,NULL,'V'},
  {"help",no_argument_TS,NULL,'h'},
  {NULL,no_argument_TS,NULL,0}
};


int main_Tfst2Unambig(int argc,char* const argv[]) {
if (argc==1) {
   usage();
   return SUCCESS_RETURN_CODE;
}

VersatileEncodingConfig vec=VEC_DEFAULT;
int val,index=-1;
char* output=NULL;
bool only_verify_arguments = false;
UnitexGetOpt options;
while (EOF!=(val=options.parse_long(argc,argv,optstring_Tfst2Unambig,lopts_Tfst2Unambig,&index))) {
   switch(val) {
   case 'o': if (options.vars()->optarg[0]=='\0') {
                error("You must specify a non empty output text file name\n");
                free(output);
                return USAGE_ERROR_CODE;
             }
             output=strdup(options.vars()->optarg);
             if (output==NULL) {
                alloc_error("main_Tfst2Unambig");
                return ALLOC_ERROR_CODE;                
             }
             break;
   case 'k': if (options.vars()->optarg[0]=='\0') {
                error("Empty input_encoding argument\n");
                free(output);
                return USAGE_ERROR_CODE;
             }
             decode_reading_encoding_parameter(&(vec.mask_encoding_compatibility_input),options.vars()->optarg);
             break;
   case 'q': if (options.vars()->optarg[0]=='\0') {
                error("Empty output_encoding argument\n");
                free(output);
                return USAGE_ERROR_CODE;                
             }
             decode_writing_encoding_parameter(&(vec.encoding_output),&(vec.bom_output),options.vars()->optarg);
             break;
   case 'V': only_verify_arguments = true;
             break;
   case 'h': usage();
             free(output); 
             return SUCCESS_RETURN_CODE;
   case ':': index==-1 ? error("Missing argument for option -%c\n",options.vars()->optopt) :
                         error("Missing argument for option --%s\n",lopts_Tfst2Unambig[index].name);
             free(output);
             return USAGE_ERROR_CODE;
   case '?': index==-1 ? error("Invalid option -%c\n",options.vars()->optopt) :
                         error("Invalid option --%s\n",options.vars()->optarg);
             free(output);
             return USAGE_ERROR_CODE;
             break;
   }
   index=-1;
}

if (options.vars()->optind!=argc-1) {
   error("Invalid arguments: rerun with --help\n");
   free(output);
   return USAGE_ERROR_CODE;
}

if (output==NULL) {
   error("You must specify an output text file\n");
   free(output);
   return USAGE_ERROR_CODE;   
}

if (only_verify_arguments) {
  // freeing all allocated memory
  free(output);
  return SUCCESS_RETURN_CODE;
}

u_printf("Loading text automaton...\n");
Tfst* tfst=open_text_automaton(&vec,argv[options.vars()->optind]);
if (tfst==NULL) {
   error("Cannot load text automaton %s\n",argv[options.vars()->optind]);
   free(output);
   return DEFAULT_ERROR_CODE;
}

int res=isLinearAutomaton(tfst);
if (res!=LINEAR_AUTOMATON) {
   error("Error: the text automaton is not linear in sentence %d\n",res);
   close_text_automaton(tfst);
   free(output);
   return DEFAULT_ERROR_CODE;
}

U_FILE* f=u_fopen(&vec,output,U_WRITE);
if (f==NULL) {
   error("Cannot create %s\n",output);
   close_text_automaton(tfst);
   free(output);
   return DEFAULT_ERROR_CODE;
}

u_printf("Converting linear automaton into text...\n");
convertLinearAutomaton(tfst,f);
u_fclose(f);
close_text_automaton(tfst);
free(output);

u_printf("Done.\n");
return SUCCESS_RETURN_CODE;
}

} // namespace unitex
