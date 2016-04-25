/*
 * Unitex
 *
 * Copyright (C) 2001-2016 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
         "  -V/--only-verify-arguments: only verify arguments syntax and exit\n"
         "  -h/--help: this help\n"
         "\n"
         "\nProduces an HTML file that shows differences between input\n"
         "concordance files.\n";


static void usage() {
  display_copyright_notice();
  u_printf(usage_ConcorDiff);
}


const char* optstring_ConcorDiff=":o:f:s:Vhk:q:d";
const struct option_TS lopts_ConcorDiff[]= {
  {"out",required_argument_TS,NULL,'o'},
  {"font",required_argument_TS,NULL,'f'},
  {"fontsize",required_argument_TS,NULL,'s'},
  {"diff_only",no_argument_TS,NULL,'d'},
  {"only_verify_arguments",no_argument_TS,NULL,'V'},
  {"help",no_argument_TS,NULL,'h'},
  {"input_encoding",required_argument_TS,NULL,'k'},
  {"output_encoding",required_argument_TS,NULL,'q'},
  {NULL,no_argument_TS,NULL,0}
};


int main_ConcorDiff(int argc,char* const argv[]) {
if (argc==1) {
    usage();
    return SUCCESS_RETURN_CODE;
}

int val,index=-1;
char* out=NULL;
char* font=NULL;
int size=0;
char foo;
int diff_only=0;
VersatileEncodingConfig vec=VEC_DEFAULT;
bool only_verify_arguments = false;
UnitexGetOpt options;
while (EOF!=(val=options.parse_long(argc,argv,optstring_ConcorDiff,lopts_ConcorDiff,&index))) {
   switch(val) {
   case 'o': if (options.vars()->optarg[0]=='\0') {
                error("You must specify a non empty output file\n");
                free(font);
                free(out);
                return USAGE_ERROR_CODE;
             }
             out=strdup(options.vars()->optarg);
             if (out==NULL) {
                alloc_error("main_ConcorDiff");
                free(font);
                free(out);
                return ALLOC_ERROR_CODE;
             }
             break;
   case 'f': if (options.vars()->optarg[0]=='\0') {
                error("You must specify a non empty font name\n");
                free(font);
                free(out);
                return USAGE_ERROR_CODE;
             }
             font=strdup(options.vars()->optarg);
             if (font==NULL) {
                alloc_error("main_ConcorDiff");
                free(font);
                free(out);
                return ALLOC_ERROR_CODE;
             }
             break;
   case 's': if (1!=sscanf(options.vars()->optarg,"%d%c",&size,&foo)
                 || size<=0) {
                /* foo is used to check that the font size is not like "45gjh" */
                error("Invalid font size argument: %s\n",options.vars()->optarg);
                free(font);
                free(out);
                return USAGE_ERROR_CODE;
             }
             break;
   case 'd': diff_only=1; break;
   case 'V': only_verify_arguments = true;
             break;
   case 'h': usage();
             free(font);
             free(out);
             return SUCCESS_RETURN_CODE;
   case ':': index==-1 ? error("Missing argument for option -%c\n",options.vars()->optopt) :
                         error("Missing argument for option --%s\n",lopts_ConcorDiff[index].name);
             free(font);
             free(out);
             return USAGE_ERROR_CODE;
   case '?': index==-1 ? error("Invalid option -%c\n",options.vars()->optopt) :
                         error("Invalid option --%s\n",options.vars()->optarg);
             free(font);
             free(out);
             return USAGE_ERROR_CODE;
   case 'k': if (options.vars()->optarg[0]=='\0') {
               error("Empty input_encoding argument\n");
               free(font);
               free(out);
               return USAGE_ERROR_CODE;
             }
             decode_reading_encoding_parameter(&(vec.mask_encoding_compatibility_input),options.vars()->optarg);
             break;
   case 'q': if (options.vars()->optarg[0]=='\0') {
               error("Empty output_encoding argument\n");
               free(font);
               free(out);
               return USAGE_ERROR_CODE;
             }
             decode_writing_encoding_parameter(&(vec.encoding_output),&(vec.bom_output),options.vars()->optarg);
             break;
   }
   index=-1;
}

if (out==NULL) {
   error("You must specify the output file\n");
   free(font);
   return USAGE_ERROR_CODE;
}
if (font==NULL) {
   error("You must specify the font to use\n");
   free(out);
   return USAGE_ERROR_CODE;
}
if (size==0) {
   error("You must specify the font size to use\n");
   free(font);
   free(out);
   return USAGE_ERROR_CODE;
}

if (options.vars()->optind!=argc-2) {
   error("Invalid arguments: rerun with --help\n");
   free(font);
   free(out);
   return USAGE_ERROR_CODE;
}

if (only_verify_arguments) {
  // freeing all allocated memory
  free(font);
  free(out);
  return SUCCESS_RETURN_CODE;
}

diff(&vec,argv[options.vars()->optind],argv[options.vars()->optind+1],out,font,size,diff_only);

free(font);
free(out);
return SUCCESS_RETURN_CODE;
}

} // namespace unitex
