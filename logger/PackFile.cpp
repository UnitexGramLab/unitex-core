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

 /*
 * File created and contributed by Gilles Vollant (Ergonotics SAS)
 * as part of an UNITEX optimization and reliability effort
 *
 * additional information: http://www.ergonotics.com/unitex-contribution/
 * contact : unitex-contribution@ergonotics.com
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>

#include "Unicode.h"
#include "Copyright.h"
#include "UnitexGetOpt.h"

#include "FilePackType.h"
#include "MzToolsUlp.h"
#include "PackFile.h"

#include "PackFileTool.h"

#ifndef NO_UNITEX_LOGGER

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/*
#ifndef HAS_LOGGER_NAMESPACE
#define HAS_LOGGER_NAMESPACE 1
#endif

namespace logger {
*/

#ifdef HAS_LOGGER_NAMESPACE
using namespace ::unitex::logger;
#endif

extern const char* optstring_PackFile;
extern const struct option_TS lopts_PackFile[];
extern const char* usage_PackFile;

/*



int buildPackFile(const char* packFile,int append_status,const char* global_comment,
                  const char* file_or_prefix_to_add,int add_one_file_only,const char* junk_prefix,
				  int quiet);*/


const char* usage_PackFile =
         "Usage : PackFile [OPTIONS] <ulpfile>\n"
         "\n"
         "  <ulpfile>: a an archive file to create\n"
         "\n"
         "OPTIONS:\n"
         "  -i X/--include=X: uses X as filename or prefix to include\n"
         "  -p/--prefix: mean include value is a prefix and not single filename\n"
         "  -a/--append: append file in existing archive\n"
         "  -m/--quiet: do not emit message when running\n"
         "  -g X/--global=X: uses X as archive global comment (cosmetic)\n"
         "  -j X/--junk_prefix=X: remove X at the beginning in the stored filename\n"
         "  -V/--only-verify-arguments: only verify arguments syntax and exit\n"
         "  -h/--help: this help\n"         
         "\n";

static void usage() {
 display_copyright_notice();
 u_printf(usage_PackFile);
}


const char* optstring_PackFile=":Vhi:pamg:j:k:q:";
const struct option_TS lopts_PackFile[]={
   {"include", required_argument_TS, NULL, 'i'},
   {"prefix",no_argument_TS,NULL,'p'},
   {"append",no_argument_TS,NULL,'a'},
   {"quiet",no_argument_TS,NULL,'m'},
   {"global",required_argument_TS,NULL,'g'},
   {"junk_prefix",required_argument_TS,NULL,'j'},
   {"input_encoding",required_argument_TS,NULL,'k'},
   {"output_encoding",required_argument_TS,NULL,'q'},
   {"only_verify_arguments",no_argument_TS,NULL,'V'},
   {"help", no_argument_TS, NULL, 'h'},
   {NULL, no_argument_TS, NULL, 0}
};

int main_PackFile(int argc,char* const argv[]) {
if (argc==1) {
   usage();
   return SUCCESS_RETURN_CODE;
}

char junk_prefix[FILENAME_MAX+0x20]="";
char include_filename[FILENAME_MAX+0x20]="";
char global_comment[FILENAME_MAX+0x20]="";

Encoding encoding_output = DEFAULT_ENCODING_OUTPUT;
int bom_output = DEFAULT_BOM_OUTPUT;
int mask_encoding_compatibility_input = DEFAULT_MASK_ENCODING_COMPATIBILITY_INPUT;
int val,index=-1;
int quiet=0;
int add_one_file_only=1;
int append=0;
bool only_verify_arguments = false;
UnitexGetOpt options;
while (EOF!=(val=options.parse_long(argc,argv,optstring_PackFile,lopts_PackFile,&index))) {
   switch(val) {

   case 'm': quiet=1; break;
   case 'p': add_one_file_only=0; break;
   case 'a': append=1; break;
   case 'i': if (options.vars()->optarg[0]=='\0') {
                error("You must specify a non empty include file name\n");
                return USAGE_ERROR_CODE;
             }
             strcpy(include_filename,options.vars()->optarg);
             break;
   case 'j': if (options.vars()->optarg[0]=='\0') {
                error("You must specify a non empty junk prefix file name\n");
                return USAGE_ERROR_CODE;
             }
             strcpy(junk_prefix,options.vars()->optarg);
             break;
   case 'g': if (options.vars()->optarg[0]=='\0') {
                   error("You must specify a non empty global comment\n");
                   return USAGE_ERROR_CODE;
                }
                strcpy(global_comment,options.vars()->optarg);
                break;
   case 'k': if (options.vars()->optarg[0]=='\0') {
                error("Empty input_encoding argument\n");
                return USAGE_ERROR_CODE;
             }
             decode_reading_encoding_parameter(&mask_encoding_compatibility_input,options.vars()->optarg);
             break;
   case 'q': if (options.vars()->optarg[0]=='\0') {
                error("Empty output_encoding argument\n");
                return USAGE_ERROR_CODE;
             }
             decode_writing_encoding_parameter(&encoding_output,&bom_output,options.vars()->optarg);
             break;
   case 'V': only_verify_arguments = true;
             break;             
   case 'h': usage(); 
             return SUCCESS_RETURN_CODE;
   case ':': index==-1 ? error("Missing argument for option -%c\n",options.vars()->optopt) :
                         error("Missing argument for option --%s\n",lopts_PackFile[index].name);
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

const char* ulpFile=argv[options.vars()->optind];

if (ulpFile == NULL) {
   error("Invalid arguments: rerun with --help\n");
   return USAGE_ERROR_CODE;
}

if ((*ulpFile)=='\0') {
   error("Invalid arguments: rerun with --help\n");
   return USAGE_ERROR_CODE;
}

if (only_verify_arguments) {
  // freeing all allocated memory
  return SUCCESS_RETURN_CODE;
}

int retValue = buildPackFile(ulpFile,append,
	                           global_comment,
                             include_filename,
                             add_one_file_only,
                             junk_prefix,
				                     quiet);
if (retValue == 0) {
	error("Error creating %s\n", ulpFile);
	return DEFAULT_ERROR_CODE;
} else {
	return SUCCESS_RETURN_CODE;
}

}

} // namespace unitex

#endif
