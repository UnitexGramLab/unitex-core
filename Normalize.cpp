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
#include "File.h"
#include "Copyright.h"
#include "String_hash.h"
#include "IOBuffer.h"
#include "Error.h"
#include "NormalizeAsRoutine.h"
#include "getopt.h"

void usage() {
u_printf("%S",COPYRIGHT);
u_printf("Usage: Normalize [OPTIONS] <text>\n"
         "\n"
         "  <text>: text file to be normalized\n"
         "\n"
         "OPTIONS:\n"
         "  -n/--no_carridge_return: every separator sequence will be turned into a single space\n"
         "  -r XXX/--replacement_rules=XXX: specifies a configuration file XXX that contains\n"
         "                                  replacement instructions in the form of lines like:\n"
         "\n"
         "                                     input_sequence TABULATION output_sequence\n"
         "\n"
         "                                  By default, the program only replaces { and } by [ and ]\n"
         "  -h/--help: this help\n"
         "\n"
         "Turns every sequence of separator chars (space, tab, new line) into one.\n"
         "If a separator sequence contains a new line char, it is turned to a single new\n"
         "line (except with --no_carridge_return); if not, it is turned into a single space. As\n"
         "a side effect, new line sequences are converted into the Windows style: \\r\\n.\n"
         "If you specifies replacement rules with -f, they will be applied prior\n"
         "to the separator normalization, so you have to take care if you manipulate\n"
         "separators in your replacement rules.\n"
         "The result is stored in a file with the same name as <text>, but with .snt extension.\n");
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

const char* optstring=":nr:h";
const struct option lopts[]= {
      {"no_carridge_return",no_argument,NULL,'n'},
      {"replacement_rules",required_argument,NULL,'r'},
      {"help",no_argument,NULL,'h'},
      {NULL,no_argument,NULL,0}
};
int mode=KEEP_CARRIDGE_RETURN;
char rules[FILENAME_MAX]="";
int val,index=-1;
optind=1;
while (EOF!=(val=getopt_long(argc,argv,optstring,lopts,&index))) {
   switch(val) {
   case 'n': mode=REMOVE_CARRIDGE_RETURN; break;
   case 'r': if (optarg[0]=='\0') {
                fatal_error("You must specify a non empty replacement rule file name\n");
             }
             strcpy(rules,optarg);
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
char tmp_file[FILENAME_MAX];
get_extension(argv[optind],tmp_file);
if (!strcmp(tmp_file, ".snt")) {
   /* If the file to process has allready the .snt extension, we temporary rename it to
	 * .snt.normalizing */
	strcpy(tmp_file,argv[optind]);
	strcat(tmp_file,".normalizing");
	rename(argv[optind],tmp_file);
} else {
   strcpy(tmp_file,argv[optind]);
}
/* We set the destination file */
char dest_file[FILENAME_MAX];
remove_extension(argv[optind],dest_file);
strcat(dest_file,".snt");
u_printf("Normalizing %s...\n",argv[optind]);
normalize(tmp_file, dest_file, mode, rules);
u_printf("\n");
/* If we have used a temporary file, we delete it */
if (strcmp(tmp_file,argv[optind])) { 
   remove(tmp_file);
}
return 0;
}



