 /*
  * Unitex
  *
  * Copyright (C) 2001-2007 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "Regular_expression.h"
#include "Copyright.h"
#include "IOBuffer.h"
#include "Error.h"


void usage() {
u_printf("%S",COPYRIGHT);
u_printf("Usage: Reg2Grf <file>\n");
u_printf("     <file> : unicode text file where the regular expression is stored.\n");
u_printf("              We must use a file, because we cannot give Unicode\n");
u_printf("              parameters on a command line.\n\n");
u_printf("Converts the regular expression into a graph, named REGEXP.GRF\n");
u_printf("and stored in the same directory that <file>. You can use the following\n");
u_printf("operators:\n");
u_printf(" A+B          matches either the expression A or B\n");
u_printf(" A.B or A B   matches the concatenation of A and B\n");
u_printf(" A*           matches 0 more more times the expression A\n");
u_printf(" (A)          matches the expression A\n");
u_printf("If you want to match any character of ( ) * + .\n");
u_printf("you must use the \\ char: \\* will match the char *\n");   
u_printf("\nExample: \"(le+la) (<A:s>+<E>) <N:s>\"\n");
}


int main(int argc, char **argv) {
setBufferMode();

if (argc!=2) {
   usage();
   return 0;
}
FILE* f=u_fopen(argv[1],U_READ);
if (f==NULL) {
   fatal_error("Cannot open file %s\n",argv[1]);
}
// we read the regular expression in the file
unichar exp[1000];
int i=0;
int c;
while ((c=u_fgetc(f))!=EOF && c!='\n') {
   exp[i++]=(unichar)c;
}
exp[i]='\0';
u_fclose(f);
// we extract regexp file pathname
char nom_grf[1000];
strcpy(nom_grf,argv[1]);
i=strlen(nom_grf);
while (i>0 && nom_grf[i]!='/' && nom_grf[i]!='\\') i--;
if (i==0) {
   strcpy(nom_grf,"regexp.grf");
} else {
   nom_grf[i+1]='\0';
   strcat(nom_grf,"regexp.grf");
}

if (!reg2grf(exp,nom_grf)) {
   fatal_error("Error in the regular expression\n");
}
u_printf("Expression converted.\n");
return 0;
}
//---------------------------------------------------------------------------
