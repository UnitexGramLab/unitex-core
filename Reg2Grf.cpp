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
#include "RegularExpressions.h"
#include "Copyright.h"
#include "Error.h"
#include "File.h"
#include "getopt.h"



void usage() {
u_printf("%S",COPYRIGHT);
u_printf("Usage: Reg2Grf <txt>\n"
         "\n"
         "  <txt>: unicode text file where the regular expression is stored.\n"
         "         We must use a file, because we cannot give Unicode\n"
         "         parameters on a command line.\n"
         "\n"
         "OPTIONS:\n"
         "  -h/--help: this help\n"
         "\n"
         "Converts the regular expression into a graph named \"regexp.grf\"\n"
         "and stored in the same directory that <file>. You can use the following\n"
         "operators:\n"
         " A+B          matches either the expression A or B\n"
         " A.B or A B   matches the concatenation of A and B\n"
         " A*           matches 0 more more times the expression A\n"
         " (A)          matches the expression A\n"
         "If you want to match any character of ( ) * + .\n"
         "you must use the \\ char: \\* will match the char *\n"   
         "\nExample: \"(le+la) (<A:s>+<E>) <N:s>\"\n");
}


int main_Reg2Grf(int argc,char* argv[]) {
if (argc==1) {
   usage();
   return 0;
}

const char* optstring=":h";
const struct option lopts[]= {
      {"help",no_argument,NULL,'h'},
      {NULL,no_argument,NULL,0}
};
int val,index=-1;
optind=1;
while (EOF!=(val=getopt_long(argc,argv,optstring,lopts,&index))) {
   switch(val) {
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

FILE* f=u_fopen(argv[optind],U_READ);
if (f==NULL) {
   fatal_error("Cannot open file %s\n",argv[optind]);
}
/* We read the regular expression in the file */
unichar exp[REG_EXP_MAX_LENGTH];
if ((REG_EXP_MAX_LENGTH-1)==u_fgets(exp,REG_EXP_MAX_LENGTH,f)) {
   fatal_error("Too long regular expression\n");
}
u_fclose(f);
char grf_name[FILENAME_MAX];
get_path(argv[optind],grf_name);
strcat(grf_name,"regexp.grf");
if (!reg2grf(exp,grf_name)) {
   return 1;
}
u_printf("Expression converted.\n");
return 0;
}

