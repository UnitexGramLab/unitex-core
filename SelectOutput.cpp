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
		 "\n";


static void usage() {
display_copyright_notice();
u_printf(usage_SelectOutput);
}


/* Undocumented short options are those given by the svn client. They
 * are listed here just to be safely ignored by getopt */
const char* optstring_SelectOutput=":ho:e:k:q:";
const struct option_TS lopts_SelectOutput[] = {
      {"output",required_argument_TS,NULL,'o'},
      {"error",required_argument_TS,NULL,'e'},
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
	return 0;
}
VersatileEncodingConfig vec=VEC_DEFAULT;
struct OptVars* vars=new_OptVars();
int val,index=-1;

while (EOF!=(val=getopt_long_TS(argc,argv,optstring_SelectOutput,lopts_SelectOutput,&index,vars))) {
   switch(val) {
   case 'h': usage(); return 0;
   case 'e':
   case 'o':
	   {
		   enum stdwrite_kind swk = (val == 'o') ? stdwrite_kind_out : stdwrite_kind_err;
		   if (strcmp(vars->optarg,"on") == 0)
		   {
		       SetStdWriteCB(swk,0,NULL,NULL);
		   }
		   else
		   if (strcmp(vars->optarg,"off") == 0)
		   {
		       SetStdWriteCB(swk,1,NULL,NULL);
		   }
		   else 
			   fatal_error("Invalid option --%s, must be 'on' or 'off'\n",vars->optarg);
		   break;
	   }
   
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
   case ':': if (index==-1) fatal_error("Missing argument for option -%c\n",vars->optopt);
             else fatal_error("Missing argument for option --%s\n",lopts_SelectOutput[index].name);
   case '?': if (index==-1) fatal_error("Invalid option -%c\n",vars->optopt);
             else fatal_error("Invalid option --%s\n",vars->optarg);
             break;
   }
   index=-1;
}

	return 0;
}

} // namespace unitex



