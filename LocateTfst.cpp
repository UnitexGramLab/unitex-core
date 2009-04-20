 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
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

#include "Copyright.h"
#include "Unicode.h"
#include "Error.h"
#include "getopt.h"
#include "LocateTfst_lib.h"
#include "File.h"


static void usage() {
u_printf("%S",COPYRIGHT);
u_printf("Usage: LocateTfst [OPTIONS] <tfst> <fst2>\n"
         "\n"
		 "  <tfst>: the text automaton\n"
         "  <fst2>: the grammar to be applied\n"
         "\n"
         "OPTIONS:\n"
         "  -a ALPH/--alphabet=ALPH: the language alphabet file\n"
         "  -h/--help: this help\n"
         "\n"
         "Applies a grammar to a text automaton, and saves the matching sequence index in a\n"
         "file named XXXXXXX.......\n");
}



/*
 * This function behaves in the same way that a main one, except that it does
 * not invoke the setBufferMode function.
 */
int main_LocateTfst(int argc,char* argv[]) {
if (argc==1) {
   usage();
   return 0;
}

const char* optstring=":a:h";
const struct option_TS lopts[]= {
      {"alphabet",required_argument_TS,NULL,'a'},
      {"help",no_argument_TS,NULL,'h'},
      {NULL,no_argument_TS,NULL,0}
};
int val,index=-1;
char alphabet[FILENAME_MAX]="";
struct OptVars* vars=new_OptVars();
while (EOF!=(val=getopt_long_TS(argc,argv,optstring,lopts,&index,vars))) {
   switch(val) {
   case 'a': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty alphabet name\n");
             }
             strcpy(alphabet,vars->optarg);
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

if (alphabet[0]=='\0') {
   fatal_error("You must specify an alphabet file\n");
}
char text[FILENAME_MAX];
char grammar[FILENAME_MAX];
char output[FILENAME_MAX];
if (vars->optind!=argc-2) {
   fatal_error("Invalid arguments: rerun with --help\n");
}
strcpy(text,argv[vars->optind]);
strcpy(grammar,argv[vars->optind+1]);
get_path(text,output);
strcat(output,"tfstconcord.txt");

int OK=locate_tfst(text,grammar,alphabet,output);

free_OptVars(vars);
return (!OK);
}
