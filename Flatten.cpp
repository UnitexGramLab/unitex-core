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
       "  -h/--help: this help\n"
       "\n"
       "Flattens a FST2 grammar into a finite state transducer in the limit of\n"
       "a recursion depth. The grammar <fst2> is replaced by its flattened equivalent.\n"
       "If the flattening process is complete, the resulting grammar contains only one\n"
       "graph.\n";


static void usage() {
u_printf("%S",COPYRIGHT);
u_printf(usage_Flatten);
}


const char* optstring_Flatten=":frd:hk:q:";
const struct option_TS lopts_Flatten[]= {
      {"fst",no_argument_TS,NULL,'f'},
      {"rtn",no_argument_TS,NULL,'r'},
      {"depth",required_argument_TS,NULL,'d'},
      {"input_encoding",required_argument_TS,NULL,'k'},
      {"output_encoding",required_argument_TS,NULL,'q'},
      {"help",no_argument_TS,NULL,'h'},
      {NULL,no_argument_TS,NULL,0}
};


int main_Flatten(int argc,char* const argv[]) {
if (argc==1) {
   usage();
   return 0;
}

int RTN=1;
int depth=10;
VersatileEncodingConfig vec=VEC_DEFAULT;
int val,index=-1;
char foo;
struct OptVars* vars=new_OptVars();
while (EOF!=(val=getopt_long_TS(argc,argv,optstring_Flatten,lopts_Flatten,&index,vars))) {
   switch(val) {
   case 'f': RTN=0; break;
   case 'r': RTN=1; break;
   case 'd': if (1!=sscanf(vars->optarg,"%d%c",&depth,&foo) || depth<=0) {
                /* foo is used to check that the depth is not like "45gjh" */
                fatal_error("Invalid depth argument: %s\n",vars->optarg);
             }
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
             else fatal_error("Missing argument for option --%s\n",lopts_Flatten[index].name);
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

u_printf("Loading %s...\n",argv[vars->optind]);
struct FST2_free_info fst2_free;
Fst2* origin=load_abstract_fst2(&vec,argv[vars->optind],1,&fst2_free);
if (origin==NULL) {
   error("Cannot load %s\n",argv[vars->optind]);
   return 1;
}
char temp[FILENAME_MAX];
strcpy(temp,argv[vars->optind]);
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
   default: fatal_error("Internal state error in Flatten's main\n");
}
free_abstract_Fst2(origin,&fst2_free);
af_remove(argv[vars->optind]);
af_rename(temp,argv[vars->optind]);
free_OptVars(vars);
return 0;
}

} // namespace unitex
