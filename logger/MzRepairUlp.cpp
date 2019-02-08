/*
 * Unitex
 *
 * Copyright (C) 2001-2019 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
 * additional information: http://www.smartversion.com/unitex-contribution/
 * contact : info@winimage.com
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
#include "MzRepairUlp.h"

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
using namespace ::unitex::logger;

extern const char* optstring_MzRepairUlp;
extern const struct option_TS lopts_MzRepairUlp[];
extern const char* usage_MzRepairUlp;


const char* usage_MzRepairUlp =
         "Usage : MzRepairUlp [OPTIONS] <ulpfile>\n"
         "\n"
             "  <ulpfile>: a corrupted ulp file (often, a crashing log)\n"
         "\n"
         "OPTIONS:\n"
         "  -o X/--output=X: uses X as filename for fixed .ulp file (<ulpfile>.repair by default)\n"
         "  -t X/--temp=X: uses X as filename for temporary file (<ulpfile>.build by default)\n"
         "  -m/--quiet: do not emit message when running\n"
         "  -v/--verbose: emit message when running\n"
         "  -V/--only-verify-arguments: only verify arguments syntax and exit\n"
         "  -h/--help: this help\n"
         "\n";

static void usage() {
  display_copyright_notice();
  u_printf(usage_MzRepairUlp);
}


const char* optstring_MzRepairUlp=":Vhmvo:t:k:q:";
const struct option_TS lopts_MzRepairUlp[]={
   {"output", required_argument_TS, NULL, 'o'},
   {"temp",required_argument_TS,NULL,'t'},
   {"input_encoding",required_argument_TS,NULL,'k'},
   {"output_encoding",required_argument_TS,NULL,'q'},
   {"quiet",no_argument_TS,NULL,'m'},
   {"verbose",no_argument_TS,NULL,'v'},
   { "only_verify_arguments",no_argument_TS,NULL,'V'},
   {"help", no_argument_TS, NULL, 'h'},
   {NULL, no_argument_TS, NULL, 0}
};

int main_MzRepairUlp(int argc,char* const argv[]) {
if (argc==1) {
   usage();
   return SUCCESS_RETURN_CODE;
}

char outputFile[FILENAME_MAX+0x20]="";
char tempFile[FILENAME_MAX+0x20]="";

Encoding encoding_output = DEFAULT_ENCODING_OUTPUT;
int bom_output = DEFAULT_BOM_OUTPUT;
int mask_encoding_compatibility_input = DEFAULT_MASK_ENCODING_COMPATIBILITY_INPUT;
int val,index=-1;
int quiet=0;
int verbose=0;
bool only_verify_arguments = false;
UnitexGetOpt options;
while (EOF!=(val=options.parse_long(argc,argv,optstring_MzRepairUlp,lopts_MzRepairUlp,&index))) {
   switch(val) {

   case 'm': quiet=1; break;
   case 'v': verbose=1; break;
   case 'o': if (options.vars()->optarg[0]=='\0') {
               error("You must specify a non empty output file name\n");
               return USAGE_ERROR_CODE;
             }
             strcpy(outputFile,options.vars()->optarg);
             break;
   case 'd': if (options.vars()->optarg[0]=='\0') {
                  error("You must specify a non empty temp file name\n");
                  return USAGE_ERROR_CODE;
                }
                strcpy(tempFile,options.vars()->optarg);
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
                         error("Missing argument for option --%s\n",lopts_MzRepairUlp[index].name);
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

const char* ulpFile=argv[options.vars()->optind];

if (outputFile[0]=='\0') {
    strcpy(outputFile,ulpFile);
    strcat(outputFile,".repair");
}

if (tempFile[0]=='\0') {
    strcpy(tempFile,outputFile);
    strcat(tempFile,".build");
}

int retRepair=0;
uLong nRecovered=0;
uLong bytesRecovered=0;

retRepair=ulpRepair(ulpFile,(const char*)outputFile,(const char*)tempFile,&nRecovered,&bytesRecovered);
if ((retRepair!=0)) {
    if ((verbose==1) || (quiet == 0)) {
        u_printf("error in UlpRepair from %s to %s : return value = %d",ulpFile,outputFile,retRepair);
    }
}

if (retRepair==0) {
    if ((verbose==1) && (quiet == 0)) {
        u_printf("success in UlpRepair from %s to %s : return value = %d",ulpFile,outputFile,retRepair);
    }
}
return retRepair;
}

} // namespace unitex
//} // namespace logger

#endif
