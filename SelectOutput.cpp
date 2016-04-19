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
#include <string.h>
#include <stdlib.h>
#include "Unicode.h"
#include "Copyright.h"
#include "Error.h"
#include "IOBuffer.h"
#include "AbstractFilePlugCallback.h"
#include "UnitexGetOpt.h"
#include "SelectOutput.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

const char* usage_SelectOutput =
  "Usage: SelectOutput [OPTIONS]\n"
  "\n"
  "OPTIONS:\n"
  "  -o [on/off]/--output=[on/off]: enable (on) or disable (off) standard output\n"
  "  -e [on/off]/--error=[on/off]: enable (on) or disable (off) error output\n"
  "  -V/--only-verify-arguments: only verify arguments syntax and exit\n"
  "  -h/--help: this help\n"     
  "\n";


static void usage() {
  display_copyright_notice();
  u_printf(usage_SelectOutput);
}


/* Undocumented short options are those given by the svn client. They
 * are listed here just to be safely ignored by getopt */
const char* optstring_SelectOutput=":Vho:e:k:q:";
const struct option_TS lopts_SelectOutput[] = {
  {"output",required_argument_TS,NULL,'o'},
  {"error",required_argument_TS,NULL,'e'},
  {"only_verify_arguments",no_argument_TS,NULL,'V'},
  {"help",no_argument_TS,NULL,'h'},
  {"input_encoding",required_argument_TS,NULL,'k'},
  {"output_encoding",required_argument_TS,NULL,'q'},
  {NULL,no_argument_TS,NULL,0}
};


/**
 * This is the customized diff program designed to compare grf files.
 */
int main_SelectOutput(int argc,char* const argv[]) {
if (argc==1) {
  usage();
  return SUCCESS_RETURN_CODE;
}

VersatileEncodingConfig vec=VEC_DEFAULT;
int val,index=-1;
bool only_verify_arguments = false;
UnitexGetOpt options;
while (EOF!=(val=options.parse_long(argc,argv,optstring_SelectOutput,lopts_SelectOutput,&index))) {
   switch(val) {
   case 'V': only_verify_arguments = true;
             break;    
   case 'h': usage(); 
             return SUCCESS_RETURN_CODE;
   case 'e':
   case 'o':
     {
       enum stdwrite_kind swk = (val == 'o') ? stdwrite_kind_out : stdwrite_kind_err;
       if (strcmp(options.vars()->optarg,"on") == 0)
       {
           SetStdWriteCB(swk,0,NULL,NULL);
       }
       else
       if (strcmp(options.vars()->optarg,"off") == 0)
       {
           SetStdWriteCB(swk,1,NULL,NULL);
       }
       else 
       {
           error("Invalid option --%s, must be 'on' or 'off'\n",options.vars()->optarg);
           return USAGE_ERROR_CODE;
       }
       break;
     }
   
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
   case ':': index==-1 ? error("Missing argument for option -%c\n",options.vars()->optopt) :
                         error("Missing argument for option --%s\n",lopts_SelectOutput[index].name);
             return USAGE_ERROR_CODE;            
   case '?': index==-1 ? error("Invalid option -%c\n",options.vars()->optopt) :
                         error("Invalid option --%s\n",options.vars()->optarg);
             return USAGE_ERROR_CODE;
   }
   index=-1;
}

  // keep this for further modifications
  if (only_verify_arguments) {
    // freeing all allocated memory
    return SUCCESS_RETURN_CODE;
  }


  return SUCCESS_RETURN_CODE;
}

} // namespace unitex



