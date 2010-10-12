/*
 * Unitex
 *
 * Copyright (C) 2001-2010 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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




const char* usage_Reg2Grf =
         "Usage: Reg2Grf <txt>\n"
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
         "\nExample: \"(le+la) (<A:s>+<E>) <N:s>\"\n";


static void usage() {
u_printf("%S",COPYRIGHT);
u_printf(usage_Reg2Grf);
}


const char* optstring_Reg2Grf=":hk:q:";
const struct option_TS lopts_Reg2Grf[]= {
      {"input_encoding",required_argument_TS,NULL,'k'},
      {"output_encoding",required_argument_TS,NULL,'q'},
      {"help",no_argument_TS,NULL,'h'},
      {NULL,no_argument_TS,NULL,0}
};


int main_Reg2Grf(int argc,char* const argv[]) {
if (argc==1) {
   usage();
   return 0;
}

Encoding encoding_output = DEFAULT_ENCODING_OUTPUT;
int bom_output = DEFAULT_BOM_OUTPUT;
int mask_encoding_compatibility_input = DEFAULT_MASK_ENCODING_COMPATIBILITY_INPUT;

int val,index=-1;
struct OptVars* vars=new_OptVars();
while (EOF!=(val=getopt_long_TS(argc,argv,optstring_Reg2Grf,lopts_Reg2Grf,&index,vars))) {
   switch(val) {
   case 'k': if (vars->optarg[0]=='\0') {
                fatal_error("Empty input_encoding argument\n");
             }
             decode_reading_encoding_parameter(&mask_encoding_compatibility_input,vars->optarg);
             break;
   case 'q': if (vars->optarg[0]=='\0') {
                fatal_error("Empty output_encoding argument\n");
             }
             decode_writing_encoding_parameter(&encoding_output,&bom_output,vars->optarg);
             break;
   case 'h': usage(); return 0;
   case ':': if (index==-1) fatal_error("Missing argument for option -%c\n",vars->optopt);
             else fatal_error("Missing argument for option --%s\n",lopts_Reg2Grf[index].name);
   case '?': if (index==-1) fatal_error("Invalid option -%c\n",vars->optopt);
             else fatal_error("Invalid option --%s\n",vars->optarg);
             break;
   }
   index=-1;
}

if (vars->optind!=argc-1) {
   fatal_error("Invalid arguments: rerun with --help\n");
}

U_FILE* f=u_fopen_existing_versatile_encoding(mask_encoding_compatibility_input,argv[vars->optind],U_READ);
if (f==NULL) {
   fatal_error("Cannot open file %s\n",argv[vars->optind]);
}
/* We read the regular expression in the file */
unichar exp[REG_EXP_MAX_LENGTH];
if ((REG_EXP_MAX_LENGTH-1)==u_fgets(exp,REG_EXP_MAX_LENGTH,f)) {
   fatal_error("Too long regular expression\n");
}
u_fclose(f);
char grf_name[FILENAME_MAX];
get_path(argv[vars->optind],grf_name);
strcat(grf_name,"regexp.grf");
if (!reg2grf(exp,grf_name,encoding_output,bom_output)) {
   return 1;
}
free_OptVars(vars);
u_printf("Expression converted.\n");
return 0;
}

