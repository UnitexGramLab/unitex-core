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
#include "Copyright.h"
#include "Fst2.h"
#include "AbstractFst2Load.h"
#include "FlattenFst2.h"
#include "Error.h"
#include "UnitexGetOpt.h"
#include "Flatten.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

const char* usage_Flatten =
       "Usage: Flatten [OPTIONS] <fst2>\n"
       "\n"
       "  <fst2>: compiled grammar to flatten;\n"
       "\n"
       "OPTIONS:\n"
       "  -f/--fst: if the grammar is not a finite-state one, the program\n"
       "            makes a finite-state approximation of it. The resulting\n"
       "            .fst2 will contain only one graph;\n"
       "  -r/--rtn: the grammar will be flattened according to the depth limit.\n"
       "            The resulting grammar may not be finite-state (default);\n"
       "  -d N/--depth=N: maximum subgraph depth to be flattened (default=10).\n"
       "  -V/--only-verify-arguments: only verify arguments syntax and exit\n"
       "  -h/--help: this help\n"
       "\n"
       "Flattens a FST2 grammar into a finite state transducer in the limit of\n"
       "a recursion depth. The grammar <fst2> is replaced by its flattened equivalent.\n"
       "If the flattening process is complete, the resulting grammar contains only one\n"
       "graph.\n";


static void usage() {
  display_copyright_notice();
  u_printf(usage_Flatten);
}


const char* optstring_Flatten=":frd:Vhk:q:";
const struct option_TS lopts_Flatten[]= {
  {"fst",no_argument_TS,NULL,'f'},
  {"rtn",no_argument_TS,NULL,'r'},
  {"depth",required_argument_TS,NULL,'d'},
  {"input_encoding",required_argument_TS,NULL,'k'},
  {"output_encoding",required_argument_TS,NULL,'q'},
  {"only_verify_arguments",no_argument_TS,NULL,'V'},
  {"help",no_argument_TS,NULL,'h'},
  {NULL,no_argument_TS,NULL,0}
};


int main_Flatten(int argc,char* const argv[]) {
if (argc==1) {
   usage();
   return SUCCESS_RETURN_CODE;
}

int RTN=1;
int depth=10;
VersatileEncodingConfig vec=VEC_DEFAULT;
int val,index=-1;
char foo;
bool only_verify_arguments = false;
UnitexGetOpt options;

while (EOF!=(val=options.parse_long(argc,argv,optstring_Flatten,lopts_Flatten,&index))) {
   switch(val) {
   case 'f': RTN=0; break;
   case 'r': RTN=1; break;
   case 'd': if (1!=sscanf(options.vars()->optarg,"%d%c",&depth,&foo) || depth<=0) {
                /* foo is used to check that the depth is not like "45gjh" */
                error("Invalid depth argument: %s\n",options.vars()->optarg);
                return USAGE_ERROR_CODE;
             }
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
   case 'h': usage(); return SUCCESS_RETURN_CODE;
   case ':': index==-1 ? error("Missing argument for option -%c\n",options.vars()->optopt) :
                         error("Missing argument for option --%s\n",lopts_Flatten[index].name);
             return USAGE_ERROR_CODE;
   case '?': index==-1 ? error("Invalid option -%c\n",options.vars()->optopt) :
                         error("Invalid option --%s\n",options.vars()->optarg);
             return USAGE_ERROR_CODE;
             break;
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

u_printf("Loading %s...\n",argv[options.vars()->optind]);
struct FST2_free_info fst2_free;
Fst2* origin=load_abstract_fst2(&vec,argv[options.vars()->optind],1,&fst2_free);
if (origin==NULL) {
   error("Cannot load %s\n",argv[options.vars()->optind]);
   return DEFAULT_ERROR_CODE;
}

char temp[FILENAME_MAX];
strcpy(temp,argv[options.vars()->optind]);
strcat(temp,".tmp.fst2");

switch (flatten_fst2(origin,depth,temp,&vec,RTN)) {
   case EQUIVALENT_FST:
      u_printf("The resulting grammar is an equivalent finite-state transducer.\n");
      break;
   case APPROXIMATIVE_FST:
      u_printf("The resulting grammar is a finite-state approximation.\n");
      break;
   case EQUIVALENT_RTN:
      u_printf("The resulting grammar is an equivalent FST2 (RTN).\n");
      break;
   default: 
      error("Internal state error in Flatten's main\n");
      free_abstract_Fst2(origin,&fst2_free);
      return DEFAULT_ERROR_CODE;
}

free_abstract_Fst2(origin,&fst2_free);
af_remove(argv[options.vars()->optind]);
af_rename(temp,argv[options.vars()->optind]);

return SUCCESS_RETURN_CODE;
}

} // namespace unitex
