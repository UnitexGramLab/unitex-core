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
#include "Unicode.h"
#include "Copyright.h"
#include "Fst2.h"
#include "FlattenFst2.h"
#include "Error.h"
#include "getopt.h"


static void usage() {
u_printf("%S",COPYRIGHT);
u_printf("Usage: Flatten [OPTIONS] <fst2>\n"
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
       "graph.\n");
}



int main_Flatten(int argc,char* argv[]) {
if (argc==1) {
   usage();
   return 0;
}

const char* optstring=":frd:h";
const struct option lopts[]= {
      {"fst",no_argument,NULL,'f'},
      {"rtn",no_argument,NULL,'r'},
      {"depth",required_argument,NULL,'d'},
      {"help",no_argument,NULL,'h'},
      {NULL,no_argument,NULL,0}
};
int RTN=1;
int depth=10;
int val,index=-1;
char foo;
struct OptVars* vars=new_OptVars();
while (EOF!=(val=getopt_long_TS(argc,argv,optstring,lopts,&index,vars))) {
   switch(val) {
   case 'f': RTN=0; break;
   case 'r': RTN=1; break;
   case 'd': if (1!=sscanf(vars->optarg,"%d%c",&depth,&foo) || depth<=0) {
                /* foo is used to check that the depth is not like "45gjh" */
                fatal_error("Invalid depth argument: %s\n",vars->optarg);
             }
             break;
   case 'h': usage(); return 0;
   case ':': if (index==-1) fatal_error("Missing argument for option -%c\n",vars->optopt); 
             else fatal_error("Missing argument for option --%s\n",lopts[index].name);
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
Fst2* origin=load_fst2(argv[vars->optind],1);
if (origin==NULL) {
   error("Cannot load %s\n",argv[vars->optind]);
   return 1;
}
char temp[FILENAME_MAX];
strcpy(temp,argv[vars->optind]);
strcat(temp,".tmp.fst2");
switch (flatten_fst2(origin,depth,temp,RTN)) {
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
free_Fst2(origin);
remove(argv[vars->optind]);
rename(temp,argv[vars->optind]);
free_OptVars(vars);
return 0;
}
