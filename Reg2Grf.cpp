/*
 * Unitex
 *
 * Copyright (C) 2001-2021 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "UnitexGetOpt.h"
#include "Reg2Grf.h"
#include "Ustring.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {


const char* usage_Reg2Grf =
         "Usage: Reg2Grf <txt>\n"
         "\n"
         "  <txt>: unicode text file where the regular expression is stored.\n"
         "         We must use a file, because we cannot give Unicode\n"
         "         parameters on a command line.\n"
         "  -o X/--output=X: output filename .grf file (optional)\n"
         "\n"
         "OPTIONS:\n"
         "  -V/--only-verify-arguments: only verify arguments syntax and exit\n"
         "  -h/--help: this help\n"
         "\n"
         "Converts the regular expression into a graph named \"regexp.grf\"\n"
         "and stored in the same directory that <file> (or specified with -o).\n"
         "You can use the following operators:\n"
         " A+B          matches either the expression A or B\n"
         " A.B or A B   matches the concatenation of A and B\n"
         " A*           matches 0 more more times the expression A\n"
         " (A)          matches the expression A\n"
         "If you want to match any character of ( ) * + .\n"
         "you must use the \\ char: \\* will match the char *\n"
         "\nExample: \"(le+la) (<A:s>+<E>) <N:s>\"\n";


static void usage() {
  display_copyright_notice();
  u_printf(usage_Reg2Grf);
}

const char* optstring_Reg2Grf=":Vho:k:q:";
const struct option_TS lopts_Reg2Grf[]= {
  {"output", required_argument_TS, NULL,'o'},
  {"input_encoding",required_argument_TS,NULL,'k'},
  {"output_encoding",required_argument_TS,NULL,'q'},
  {"only_verify_arguments",no_argument_TS,NULL,'V'},
  {"help",no_argument_TS,NULL,'h'},
  {NULL,no_argument_TS,NULL,0}
};

int main_Reg2Grf(int argc,char* const argv[]) {
if (argc==1) {
   usage();
   return SUCCESS_RETURN_CODE;
}

VersatileEncodingConfig vec=VEC_DEFAULT;
char grf_name[FILENAME_MAX]="";
int val, index = -1;
bool only_verify_arguments = false;
UnitexGetOpt options;
while (EOF!=(val=options.parse_long(argc,argv,optstring_Reg2Grf,lopts_Reg2Grf,&index))) {
   switch(val) {
   case 'k': if (options.vars()->optarg[0]=='\0') {
                error("Empty input_encoding argument\n");
                return USAGE_ERROR_CODE;
             }
             decode_reading_encoding_parameter(&(vec.mask_encoding_compatibility_input),options.vars()->optarg);
             break;
   case 'q': if (options.vars()->optarg[0]=='\0') {
                error("Empty output_encoding argument\n");
                return USAGE_ERROR_CODE;
             }
             decode_writing_encoding_parameter(&(vec.encoding_output),&(vec.bom_output),options.vars()->optarg);
             break;
   case 'o': if (options.vars()->optarg[0] == '\0') {
                error("You must specify a non empty output filename\n");
                return USAGE_ERROR_CODE;
             }
             strcpy(grf_name, options.vars()->optarg);
             break;
   case 'V': only_verify_arguments = true;
             break;
   case 'h': usage();
             return SUCCESS_RETURN_CODE;
   case ':': index==-1 ? error("Missing argument for option -%c\n",options.vars()->optopt) :
                         error("Missing argument for option --%s\n",lopts_Reg2Grf[index].name);
             return USAGE_ERROR_CODE;
   case '?': index==-1 ? error("Invalid option -%c\n",options.vars()->optopt) :
                         error("Invalid option --%s\n",options.vars()->optarg);
             return USAGE_ERROR_CODE;
   }
   index=-1;
}

if (options.vars()->optind!=argc-1) {
   error("Invalid arguments: rerun with --help\n");
   return USAGE_ERROR_CODE;
}

if (only_verify_arguments) {
  // freeing all allocated memory
  return SUCCESS_RETURN_CODE;
}

U_FILE* f=u_fopen(&vec,argv[options.vars()->optind],U_READ);
if (f==NULL) {
   error("Cannot open file %s\n",argv[options.vars()->optind]);
   return DEFAULT_ERROR_CODE;
}

/* We read the regular expression in the file */
unichar* exp=readline_safe(f);
if (exp==NULL) {
   error("Empty file %s\n",argv[options.vars()->optind]);
   u_fclose(f);
   return DEFAULT_ERROR_CODE;
}
u_fclose(f);

if (grf_name[0] == '\0') {
  get_path(argv[options.vars()->optind],grf_name);
  strcat(grf_name,"regexp.grf");
}

if (!reg2grf(exp,grf_name,&vec)) {
    free(exp);
  return DEFAULT_ERROR_CODE;
}

free(exp);

u_printf("Expression converted.\n");
return SUCCESS_RETURN_CODE;
}

} // namespace unitex
