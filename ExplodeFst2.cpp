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

#include "Unicode.h"
#include "Copyright.h"
#include "ExplodeFst2Utils.h"
#include "utils.h"
#include "IOBuffer.h"
#include "getopt.h"


void usage() {
u_printf("%S",COPYRIGHT);
u_printf("Usage: ExplodeFst2 [OPTIONS] <txtauto>\n"
         "\n"
         "  <txtauto>: input text automaton file\n"
         "\n"
         "OPTIONS:\n"
         "  -o OUT/--output=OUT: resulting text automaton. By default, OUT is of the form \"text-exp.fst2\"\n"
         "  -h/--help: this help\n"
         "\n"
         "\"Explodes\" the factorized labels in the specified text automaton, setting\n"
         "apart their corresponding lexical entries. The resulting text automaton is stored\n"
         "in OUT.\n");
}


int main(int argc,char* argv[]) {
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
      {"output",required_argument,NULL,'o'},
      {"help",no_argument,NULL,'h'},
      {NULL,no_argument,NULL,0}
};
int val,index=-1;
char txtname[FILENAME_MAX]="";
char outname[FILENAME_MAX]="";
optind=1;
while (EOF!=(val=getopt_long(argc,argv,optstring,lopts,&index))) {
   switch(val) {
   case 'o': if (optarg[0]=='\0') {
                fatal_error("You must specify a non empty output file name: %s\n",optarg);
             }
             strcpy(outname,optarg);
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
strcpy(txtname,argv[optind]);
if (outname[0]=='\0') {
   int len = strlen(txtname);
   strcpy(outname,txtname);
   if (strcmp(txtname + len - 5, ".fst2") == 0) {
      strcpy(outname + len - 5, "-exp.fst2");
   } else {
      strcat(outname, "-exp.fst2");
   }
}
u_printf("Loading '%s'\n", txtname);
list_aut_old * txtauto=load_text_automaton(txtname);
if (txtauto==NULL) {
   fatal_error("Unable to load '%s'\n", txtname);
}
u_printf("Explosion ....\n");
if (text_output_fst2_fname(txtauto, outname) == -1) {
   fatal_error("Unable to explode fst in '%s'\n", outname);
}
list_aut_old_delete(txtauto);
u_printf("Done. '%s' is exploded in '%s'.\n", txtname, outname);
return 0;
}

