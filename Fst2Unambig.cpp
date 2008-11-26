 /*
  * Unitex
  *
  * Copyright (C) 2001-2008 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "Fst2.h"
#include "Copyright.h"
#include "IOBuffer.h"
#include "LinearAutomaton2Txt.h"
#include "Error.h"
#include "getopt.h"


void usage() {
u_printf("%S",COPYRIGHT);
u_printf("Usage: Fst2Unambig  [OPTIONS] <fst2>\n"
         "\n"
         "  <fst2>: fst2 file representing the text automaton\n"
         "\n"
         "OPTIONS:\n"
         "  -o TXT/--out=TXT : output unicode text file\n"
         "  -h/--help: this help\n"
         "\n"
         "Converts a linear Unitex text automaton into a text file. If\n"
         "the automaton is not linear, the process is aborted.\n");
}



int main(int argc, char **argv) {
/* Every Unitex program must start by this instruction,
 * in order to avoid display problems when called from
 * the graphical interface */
setBufferMode();

if (argc==1) {
   usage();
   return 0;
}

const char* optstring=":o:h";
const struct option lopts[]= {
      {"out",required_argument,NULL,'o'},
      {"help",no_argument,NULL,'h'},
      {NULL,no_argument,NULL,0}
};
int val,index=-1;
optind=1;
char* output=NULL;
while (EOF!=(val=getopt_long(argc,argv,optstring,lopts,&index))) {
   switch(val) {
   case 'o': if (optarg[0]=='\0') {
                fatal_error("You must specify a non empty output text file name\n");
             }
             output=strdup(optarg);
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

if (optind!=argc-1) {
   fatal_error("Invalid arguments: rerun with --help\n");
}

if (output==NULL) {
   fatal_error("You must specify an output text file\n");
}

u_printf("Loading text automaton...\n");
Fst2* fst2=load_fst2(argv[optind],0);
if (fst2==NULL) {
   error("Cannot load text automaton %s\n",argv[optind]);
   return 1;
}
int res=isLinearAutomaton(fst2);
if (res!=LINEAR_AUTOMATON) {
   error("Error: the text automaton is not linear in sentence %d\n",res);
   free_Fst2(fst2);
   return 1;
}
FILE* f=u_fopen(output,U_WRITE);
if (f==NULL) {
   error("Cannot create %s\n",output);
   free_Fst2(fst2);
   return 1;
}
u_printf("Converting linear automaton into text...\n");
convertLinearAutomaton(fst2,f);
u_fclose(f);
free_Fst2(fst2);
free(output);
u_printf("Done.\n");
return 0;
}





