/*
 * Unitex
 *
 * Copyright (C) 2001-2018 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "Af_stdio.h"
#include "DirHelper.h"

#include "FilePackType.h"
#include "FileUnPack.h"
#include "UnpackFileTool.h"

#include "UnpackFile.h"

#ifndef NO_UNITEX_LOGGER

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

#ifdef HAS_LOGGER_NAMESPACE
using namespace ::unitex::logger;
#endif
/*
#ifndef HAS_LOGGER_NAMESPACE
#define HAS_LOGGER_NAMESPACE 1
#endif

namespace logger {
*/

extern const char* optstring_UnpackFile;
extern const struct option_TS lopts_UnpackFile[];
extern const char* usage_UnpackFile;

const char* usage_UnpackFile =
         "Usage : UnpackFile [OPTIONS] <ulpfile>\n"
         "\n"
         "  <ulpfile>: an ulp (or uncompressed zipfile) archive file to extract\n"
         "\n"
         "OPTIONS:\n"
         "  -d X/--extractdir=X: uses X as prefix for destination (include / or \\ after)\n"
         "  -j/--junk: remove filepath from filename inside archive\n"
         "  -p/--pathseparator: transform path separator to current platform standard\n"
         "  -l/--list: just list content of archive\n"
         "  -n/--only_filename: just list filename for -l/--list\n"
         "  -o/--output: output filename for -l/--list\n"
         "  -m/--quiet: do not emit message when running\n"
         "  -v/--verbose: emit message when running\n"
         "  -V/--only-verify-arguments: only verify arguments syntax and exit\n"
         "  -h/--help: this help\n"
         "\n";

static void usage() {
  display_copyright_notice();
  u_printf(usage_UnpackFile);
}

const char* optstring_UnpackFile=":Vhd:f:jpmlk:q:o:n";
const struct option_TS lopts_UnpackFile[]={
   {"extractdir", required_argument_TS, NULL, 'd'},
   {"selectfile", required_argument_TS, NULL, 'f'},
   {"junk",no_argument_TS,NULL,'j'},
   {"pathseparator",no_argument_TS,NULL,'p'},
   {"quiet",no_argument_TS,NULL,'m'},
   {"list",no_argument_TS,NULL,'l'},
   {"only_filename",no_argument_TS,NULL,'n'},
   {"output", required_argument_TS, NULL,'o'},
   {"input_encoding",required_argument_TS,NULL,'k'},
   {"output_encoding",required_argument_TS,NULL,'q'},
   {"only_verify_arguments",no_argument_TS,NULL,'V'},
   {"help", no_argument_TS, NULL, 'h'},
   {NULL, no_argument_TS, NULL, 0}
};

int main_UnpackFile(int argc,char* const argv[]) {
if (argc==1) {
   usage();
   return SUCCESS_RETURN_CODE;
}

char outputDir[FILENAME_MAX+0x20]="";
char selectFile[FILENAME_MAX + 0x20] = "";
char outputList[FILENAME_MAX + 0x20] = "";

Encoding encoding_output = DEFAULT_ENCODING_OUTPUT;
int bom_output = DEFAULT_BOM_OUTPUT;
int mask_encoding_compatibility_input = DEFAULT_MASK_ENCODING_COMPATIBILITY_INPUT;
int val,index=-1;
int quiet=0;
int list=0;
int junk_path_in_pack_archive=0;
int transform_path_separator=0;
int list_only_filename=0;
bool only_verify_arguments = false;
UnitexGetOpt options;
while (EOF!=(val=options.parse_long(argc,argv,optstring_UnpackFile,lopts_UnpackFile,&index))) {
   switch(val) {
     case 'm': quiet=1; break;
     case 'l': list=1; break;
     case 'j': junk_path_in_pack_archive=1; break;
     case 'p': transform_path_separator = 1; break;
     case 'n': list_only_filename = 1; break;
     case 'f': if (options.vars()->optarg[0]=='\0') {
                  error("You must specify a non empty selected file\n");
                  return USAGE_ERROR_CODE;
               }
               strcpy(selectFile,options.vars()->optarg);
               break;
     case 'd': if (options.vars()->optarg[0]=='\0') {
                  error("You must specify a non empty output prefix\n");
                  return USAGE_ERROR_CODE;
               }
               strcpy(outputDir,options.vars()->optarg);
               break;
     case 'o': if (options.vars()->optarg[0]=='\0') {
                  error("You must specify a non empty output list filename\n");
                  return USAGE_ERROR_CODE;
               }
               strcpy(outputList, options.vars()->optarg);
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
                           error("Missing argument for option --%s\n",lopts_UnpackFile[index].name);
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

int retValue = 0;
if (list != 0) {
    retValue = do_list_file_in_pack_archive_to_file(ulpFile,outputList,list_only_filename);
} else {
    if (selectFile[0] != '\0') {
        retValue = do_extract_from_pack_archive_onefile(ulpFile,junk_path_in_pack_archive,outputDir,selectFile,transform_path_separator,quiet);
  } else {
        retValue = do_extract_from_pack_archive(ulpFile,junk_path_in_pack_archive,outputDir,transform_path_separator,quiet);
  }
}

if (retValue != UNZ_OK) {
    error("error in processing %s",ulpFile);
}

return retValue;
}



} // namespace unitex
//} // namespace logger

#endif
