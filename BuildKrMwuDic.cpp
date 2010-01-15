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
#include "Copyright.h"
#include "Error.h"
#include "getopt.h"
#include "BuildKrMwuDic.h"



const char* usage_BuildKrMwuDic =
         "Usage: BuildKrMwuDic [OPTIONS] xxxxx\n"
        "\n"
        " ???????"
        "\n"
        "OPTIONS:\n"
        "\n";


static void usage() {
u_printf("%S",COPYRIGHT);
u_printf(usage_BuildKrMwuDic);
}





const char* optstring_BuildKrMwuDic="hk:q:";
const struct option_TS lopts_BuildKrMwuDic[]= {
      {"help",no_argument_TS,NULL,'h'},
      {"input_encoding",required_argument_TS,NULL,'k'},
      {"output_encoding",required_argument_TS,NULL,'q'},
      {NULL,no_argument_TS,NULL,0}
};


/**
 * The same than main, but no call to setBufferMode.
 */
int main_BuildKrMwuDic(int argc,char* argv[]) {
if (argc==1) {
   usage();
   return 0;
}


int val,index=-1;
Encoding encoding_output = DEFAULT_ENCODING_OUTPUT;
int bom_output = DEFAULT_BOM_OUTPUT;
int mask_encoding_compatibility_input = DEFAULT_MASK_ENCODING_COMPATIBILITY_INPUT;
struct OptVars* vars=new_OptVars();
while (EOF!=(val=getopt_long_TS(argc,argv,optstring_BuildKrMwuDic,lopts_BuildKrMwuDic,&index,vars))) {
   switch(val) {
   case 'h': usage(); return 0;
   case ':': if (index==-1) fatal_error("Missing argument for option -%c\n",vars->optopt);
             else fatal_error("Missing argument for option --%s\n",lopts_BuildKrMwuDic[index].name);
   case '?': if (index==-1) fatal_error("Invalid option -%c\n",vars->optopt);
             else fatal_error("Invalid option --%s\n",vars->optarg);
             break;
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
   }
   index=-1;
}
if (vars->optind!=argc-1) {
   error("Invalid arguments: rerun with --help\n");
   return 1;
}
free_OptVars(vars);
u_printf("Done.\n");
return 0;
}

