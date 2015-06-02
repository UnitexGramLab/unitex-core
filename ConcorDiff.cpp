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
#include "Diff.h"
#include "Error.h"
#include "UnitexGetOpt.h"
#include "ConcorDiff.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

const char* usage_ConcorDiff =
         "Usage: ConcorDiff [OPTIONS] <concor1> <concor2>\n"
         "\n"
         "  <concor1>: the first concord.ind file\n"
         "  <concor2>: the second concord.ind file\n"
         "\n"
         "OPTIONS:\n"
         "  -o X/--out=X: the result HTML file\n"
         "  -f FONT/--font=FONT: name of the font to use\n"
         "  -s N/--fontsize=N: size of the font to use\n"
         "  -d/--diff_only: don't show identical sequences\n"
         "  -h/--help: this help\n"
         "\n"
         "\nProduces an HTML file that shows differences between input\n"
         "concordance files.\n";


static void usage() {
display_copyright_notice();
u_printf(usage_ConcorDiff);
}


const char* optstring_ConcorDiff=":o:f:s:hk:q:d";
const struct option_TS lopts_ConcorDiff[]= {
      {"out",required_argument_TS,NULL,'o'},
      {"font",required_argument_TS,NULL,'f'},
      {"fontsize",required_argument_TS,NULL,'s'},
      {"diff_only",no_argument_TS,NULL,'d'},
      {"help",no_argument_TS,NULL,'h'},
      {"input_encoding",required_argument_TS,NULL,'k'},
      {"output_encoding",required_argument_TS,NULL,'q'},
      {NULL,no_argument_TS,NULL,0}
};


int main_ConcorDiff(int argc,char* const argv[]) {
if (argc==1) {
	usage();
	return 0;
}

int val,index=-1;
char* out=NULL;
char* font=NULL;
int size=0;
char foo;
int diff_only=0;
VersatileEncodingConfig vec=VEC_DEFAULT;
struct OptVars* vars=new_OptVars();
while (EOF!=(val=getopt_long_TS(argc,argv,optstring_ConcorDiff,lopts_ConcorDiff,&index,vars))) {
   switch(val) {
   case 'o': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty output file\n");
             }
             out=strdup(vars->optarg);
             if (out==NULL) {
                fatal_alloc_error("main_ConcorDiff");
             }
             break;
   case 'f': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty font name\n");
             }
             font=strdup(vars->optarg);
             if (font==NULL) {
                fatal_alloc_error("main_ConcorDiff");
             }
             break;
   case 's': if (1!=sscanf(vars->optarg,"%d%c",&size,&foo)
                 || size<=0) {
                /* foo is used to check that the font size is not like "45gjh" */
                fatal_error("Invalid font size argument: %s\n",vars->optarg);
             }
             break;
   case 'd': diff_only=1; break;
   case 'h': usage(); return 0;
   case ':': if (index==-1) fatal_error("Missing argument for option -%c\n",vars->optopt);
             else fatal_error("Missing argument for option --%s\n",lopts_ConcorDiff[index].name);
   case '?': if (index==-1) fatal_error("Invalid option -%c\n",vars->optopt);
             else fatal_error("Invalid option --%s\n",vars->optarg);
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
if (vars->optind!=argc-2) {
   error("Invalid arguments: rerun with --help\n");
   return 1;
}
diff(&vec,argv[vars->optind],argv[vars->optind+1],out,font,size,diff_only);
free(out);
free(font);
free_OptVars(vars);
return 0;
}

} // namespace unitex
