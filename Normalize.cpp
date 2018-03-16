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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Unicode.h"
#include "File.h"
#include "Copyright.h"
#include "String_hash.h"
#include "Error.h"
#include "NormalizeAsRoutine.h"
#include "UnitexGetOpt.h"
#include "Normalize.h"
#include "Offsets.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

const char* usage_Normalize =
         "Usage: Normalize [OPTIONS] <text>\n"
         "\n"
         "  <text>: text file to be normalized\n"
         "\n"
         "OPTIONS:\n"
         "  -n/--no_carriage_return: every separator sequence will be turned into a single space\n"
         "  --input_offsets=XXX: base offset file to be used\n"
         "  --output_offsets=XXX: offset file to be produced\n"
         "  --no_separator_normalization: only applies replacement rules specified with -r\n"
         "  -r XXX/--replacement_rules=XXX: specifies a configuration file XXX that contains\n"
         "                                  replacement instructions in the form of lines like:\n"
         "                                  input_sequence TABULATION output_sequence\n"
         "                                  By default, the program only replaces { and } by [ and ]\n"
         "  -V/--only-verify-arguments: only verify arguments syntax and exit\n"
         "  -h/--help: this help\n"
         "\n"
         "Turns every sequence of separator chars (space, tab, new line) into one.\n"
         "If a separator sequence contains a new line char, it is turned to a single new\n"
         "line (except with --no_carriage_return); if not, it is turned into a single space. As\n"
         "a side effect, new line sequences are converted into the Windows style: \\r\\n.\n"
         "If you specifies replacement rules with -f, they will be applied prior\n"
         "to the separator normalization, so you have to take care if you manipulate\n"
         "separators in your replacement rules.\n"
         "The result is stored in a file with the same name as <text>, but with .snt extension.\n";


static void usage() {
  display_copyright_notice();
  u_printf(usage_Normalize);
}


const char* optstring_Normalize=":nlr:Vhk:q:@:";
const struct option_TS lopts_Normalize[]= {
  {"no_carriage_return",no_argument_TS,NULL,'n'},
  {"no_convert_lf_to_crlf",no_argument_TS,NULL,'l'},
  {"replacement_rules",required_argument_TS,NULL,'r'},
  {"input_encoding",required_argument_TS,NULL,'k'},
  {"output_encoding",required_argument_TS,NULL,'q'},
  {"input_offsets",required_argument_TS,NULL,'$'},
  {"output_offsets",required_argument_TS,NULL,'@'},
  {"no_separator_normalization",no_argument_TS,NULL,1},
  {"only_verify_arguments",no_argument_TS,NULL,'V'},
  {"help",no_argument_TS,NULL,'h'},
  {NULL,no_argument_TS,NULL,0}
};


int main_Normalize(int argc,char* const argv[]) {
if (argc==1) {
  usage();
  return SUCCESS_RETURN_CODE;
}
int mode=KEEP_CARRIAGE_RETURN;
int separator_normalization=1;
char rules[FILENAME_MAX]="";
char input_offsets[FILENAME_MAX]="";
char output_offsets[FILENAME_MAX]="";
VersatileEncodingConfig vec=VEC_DEFAULT;
int convLFtoCRLF=1;
int val,index=-1;
bool only_verify_arguments = false;
UnitexGetOpt options;
while (EOF!=(val=options.parse_long(argc,argv,optstring_Normalize,lopts_Normalize,&index))) {
   switch(val) {
   case 'l': convLFtoCRLF=0; break;
   case 'n': mode=REMOVE_CARRIAGE_RETURN; break;
   case 'r': if (options.vars()->optarg[0]=='\0') {
              error("You must specify a non empty replacement rule file name\n");
              return USAGE_ERROR_CODE;
             }
             strcpy(rules,options.vars()->optarg);
             break;
   case 1: separator_normalization=0; break;
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
   case '$': if (options.vars()->optarg[0]=='\0') {
              error("You must specify a non empty input offset file name\n");
              return USAGE_ERROR_CODE;
             }
             strcpy(input_offsets,options.vars()->optarg);
             break;
   case '@': if (options.vars()->optarg[0]=='\0') {
              error("You must specify a non empty output offset file name\n");
              return USAGE_ERROR_CODE;
             }
             strcpy(output_offsets,options.vars()->optarg);
             break;
   case 'V': only_verify_arguments = true;
             break;
   case 'h': usage();
             return SUCCESS_RETURN_CODE;
   case ':': index==-1 ? error("Missing argument for option -%c\n",options.vars()->optopt) :
                         error("Missing argument for option --%s\n",lopts_Normalize[index].name);
             return USAGE_ERROR_CODE;
             break;
   case '?': index==-1 ? error("Invalid option -%c\n",options.vars()->optopt) :
                         error("Invalid option --%s\n",options.vars()->optarg);
             return USAGE_ERROR_CODE;
             break;
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

vector_offset* v_input_offsets=NULL;
vector_offset* v_output_offsets=NULL;
U_FILE* f_output_offsets=NULL;

if (output_offsets[0]!='\0') {
  /* We deal with offsets only if we have to produce output offsets */
  if (input_offsets[0]!='\0') {
    v_input_offsets=load_offsets(&vec,input_offsets);
  }
  f_output_offsets=u_fopen(&vec, output_offsets, U_WRITE);
  if (f_output_offsets==NULL) {
    error("Cannot create offset file %s\n",output_offsets);
    return DEFAULT_ERROR_CODE;
  }
  v_output_offsets=new_vector_offset();
}
char tmp_file[FILENAME_MAX];
get_extension(argv[options.vars()->optind],tmp_file);
if (!strcmp(tmp_file, ".snt")) {
   /* If the file to process has already the .snt extension, we temporary rename it to
   * .snt.normalizing */
  strcpy(tmp_file,argv[options.vars()->optind]);
  strcat(tmp_file,".normalizing");
  af_rename(argv[options.vars()->optind],tmp_file);
} else {
   strcpy(tmp_file,argv[options.vars()->optind]);
}
/* We set the destination file */
char dest_file[FILENAME_MAX];
remove_extension(argv[options.vars()->optind],dest_file);
strcat(dest_file,".snt");
u_printf("Normalizing %s...\n",argv[options.vars()->optind]);

int return_value = normalize(tmp_file,
                             dest_file,
                             &vec,
                             mode,
                             convLFtoCRLF,
                             rules,
                             v_output_offsets,
                             separator_normalization);
u_printf("\n");
/* If we have used a temporary file, we delete it */
if (strcmp(tmp_file,argv[options.vars()->optind])) {
   af_remove(tmp_file);
}
process_offsets(v_input_offsets,v_output_offsets,f_output_offsets);
u_fclose(f_output_offsets);
free_vector_offset(v_input_offsets);
free_vector_offset(v_output_offsets);
u_printf((return_value==SUCCESS_RETURN_CODE) ? "Done.\n" : "Unsuccessfull.\n");

return return_value;
}

} // namespace unitex


