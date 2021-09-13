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
#include <string.h>
#include <stdlib.h>
#include "Unicode.h"
#include "Copyright.h"
#include "Error.h"
#include "IOBuffer.h"
#include "GrfSvn_lib.h"
#include "Grf_lib.h"
#include "UnitexGetOpt.h"
#include "GrfDiff3.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

const char* usage_GrfDiff3 =
  "Usage: GrfDiff3 <mine> <base> <other>\n"
  "\n"
  "  <mine>: my .grf file\n"
  "  <other>: the other .grf file that may be conflicting\n"
  "  <base>: the common ancestor .grf file\n"
  "\n"
  "OPTIONS:\n"
  "--output X: saves the result, if any, in X instead of printing it on the output\n"
  "--conflicts X: saves the description of the conflicts, if any, in X\n"
  "--only-cosmetic: reports a conflict for any change that is not purely cosmetic\n"
  "  -V/--only-verify-arguments: only verify arguments syntax and exit\n"
  "  -h/--help: this help\n"
  "\n"
  "Tries to merge <mine> and <other>. In case of success, the result is printed on the\n"
  "standard output and 0 is returned. In case of unresolved conflicts, 1 is returned and\n"
  "nothing is printed. 2 is returned in case of error.\n";


static void usage() {
  display_copyright_notice();
  u_printf(usage_GrfDiff3);
}


/* Undocumented short options are those given by the svn client. They
 * are listed here just to be safely ignored by getopt */
const char* optstring_GrfDiff3=":VhEmL:k:q:";
const struct option_TS lopts_GrfDiff3[]= {
  {"output",required_argument_TS,NULL,1},
  {"conflicts",required_argument_TS,NULL,2},
  {"only-cosmetic",no_argument_TS,NULL,3},
  {"input_encoding",required_argument_TS,NULL,'k'},
  {"output_encoding",required_argument_TS,NULL,'q'},
  {"only_verify_arguments",no_argument_TS,NULL,'V'},
  {"help",no_argument_TS,NULL,'h'},
  {NULL,no_argument_TS,NULL,0}
};


/**
 * This is the customized diff3 program designed to merge grf files.
 */
int main_GrfDiff3(int argc,char* const argv[]) {
if (argc==1) {
    usage();
    return SUCCESS_RETURN_CODE;
}

VersatileEncodingConfig vec=VEC_DEFAULT;
int val,index=-1;
char output[FILENAME_MAX]="";
char conflicts[FILENAME_MAX]="";
int only_cosmetics=0;
bool only_verify_arguments = false;
UnitexGetOpt options;
while (EOF!=(val=options.parse_long(argc,argv,optstring_GrfDiff3,lopts_GrfDiff3,&index))) {
   switch(val) {
     case 'V': only_verify_arguments = true;
               break;
     case 'h': usage();
               return SUCCESS_RETURN_CODE;
     case 1: {
       strcpy(output,options.vars()->optarg);
       break;
     }
     case 2: {
       strcpy(conflicts,options.vars()->optarg);
       break;
     }
     case 3: only_cosmetics=1; break;
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
                           error("Missing argument for option --%s\n",lopts_GrfDiff3[index].name);
               return USAGE_ERROR_CODE;
     case '?': index==-1 ? error("Invalid option -%c\n",options.vars()->optopt) :
                           error("Invalid option --%s\n",options.vars()->optarg);
               return USAGE_ERROR_CODE;
   }
   index=-1;
}

if (options.vars()->optind!=argc-3) {
   error("Invalid arguments: rerun with --help\n");
   return USAGE_ERROR_CODE;
}

if (only_verify_arguments) {
  // freeing all allocated memory
  return SUCCESS_RETURN_CODE;
}

U_FILE* f=U_STDOUT;
if (output[0]!='\0') {
    f=u_fopen(&vec,output,U_WRITE);
    if (f==NULL) {
        error("Cannot create file %s\n",output);
    return DEFAULT_ERROR_CODE;
    }
}

U_FILE* f_conflicts=NULL;
if (conflicts[0]!='\0') {
    /* There is no point in encoding the conflict file in UTF16 */
    f_conflicts=u_fopen(UTF8,conflicts,U_WRITE);
    if (f_conflicts==NULL) {
    error("Cannot create file %s\n",conflicts);
    if (f!=U_STDOUT) {
     u_fclose(f);
    }
    return DEFAULT_ERROR_CODE;
    }
}

Grf* mine=load_Grf(&vec,argv[options.vars()->optind]);
if (mine==NULL) {
  error("Cannot load graph %s\n",argv[options.vars()->optind]);
  u_fclose(f_conflicts);
  if (f!=U_STDOUT) {
   u_fclose(f);
  }
  return DEFAULT_ERROR_CODE;
}

Grf* base=load_Grf(&vec,argv[options.vars()->optind+1]);
if (base==NULL) {
    free_Grf(mine);
  u_fclose(f_conflicts);
  if (f!=U_STDOUT) {
   u_fclose(f);
  }
  return DEFAULT_ERROR_CODE;
}

Grf* other=load_Grf(&vec,argv[options.vars()->optind+2]);
if (other==NULL) {
  free_Grf(base);
    free_Grf(mine);
  u_fclose(f_conflicts);
  if (f!=U_STDOUT) {
   u_fclose(f);
  }
  return DEFAULT_ERROR_CODE;
}

int res=diff3(f,f_conflicts,mine,base,other,only_cosmetics);

if (f!=U_STDOUT) {
    u_fclose(f);
    if (res!=0) {
        /* If the diff3 failed, we must remove the file */
        af_remove(output);
    }
}

if (f_conflicts!=NULL) {
    u_fclose(f_conflicts);
    if (res==0) {
        /* If the diff3 succeeded, we must remove the file */
        af_remove(conflicts);
    }
}

free_Grf(mine);
free_Grf(base);
free_Grf(other);
return res;
}

} // namespace unitex
