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
#include "Diff.h"
#include "getopt.h"


static void usage() {
u_printf("%S",COPYRIGHT);
u_printf("Usage: ConcorDiff [OPTIONS] <concor1> <concor2>\n"
         "\n"
         "  <concor1>: the first concord.ind file\n"
         "  <concor2>: the second concord.ind file\n"
         "\n"
         "REQUIRED OPTIONS:\n"
         "  -o X/--out=X: the result HTML file\n"
         "  -f FONT/--font=FONT: name of the font to use\n"
         "  -s N/--fontsize=N: size of the font to use\n"
         "\n"
         "OPTIONS:\n"
         "  -h/--help: this help\n"
         "\n"
         "\nProduces an HTML file that shows differences between input\n"
         "concordance files.\n");
}



int main_ConcorDiff(int argc,char* argv[]) {
if (argc==1) {
	usage();
	return 0;
}

const char* optstring=":o:f:s:h";
const struct option lopts[]= {
      {"out",required_argument,NULL,'o'},
      {"font",required_argument,NULL,'f'},
      {"fontsize",required_argument,NULL,'s'},
      {"help",no_argument,NULL,'h'},
      {NULL,no_argument,NULL,0}
};
int val,index=-1;
char* out=NULL;
char* font=NULL;
int size=0;
char foo;
optind=1;
while (EOF!=(val=getopt_long(argc,argv,optstring,lopts,&index))) {
   switch(val) {
   case 'o': if (optarg[0]=='\0') {
                fatal_error("You must specify a non empty output file\n");
             }
             out=strdup(optarg);
             break;
   case 'f': if (optarg[0]=='\0') {
                fatal_error("You must specify a non empty font name\n");
             }
             font=strdup(optarg);
             break;
   case 's': if (1!=sscanf(optarg,"%d%c",&size,&foo)
                 || size<=0) {
                /* foo is used to check that the font size is not like "45gjh" */
                fatal_error("Invalid font size argument: %s\n",optarg);
             }
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

if (out==NULL) {
   fatal_error("You must specify the output file\n");
}
if (font==NULL) {
   fatal_error("You must specify the font to use\n");
}
if (size==0) {
   fatal_error("You must specify the font size to use\n");
}
if (optind!=argc-2) {
   error("Invalid arguments: rerun with --help\n");
   return 1;
}
diff(argv[optind],argv[optind+1],out,font,size);
free(out);
free(font);
return 0;
}
