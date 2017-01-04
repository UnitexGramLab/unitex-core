/*
 * Unitex
 *
 * Copyright (C) 2001-2017 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "Copyright.h"
#include "Unicode.h"
#include "CodePages.h"
#include "File.h"
#include "Error.h"
#include "UnitexGetOpt.h"
#include "Convert.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

#define REPLACE_FILE 0
#define PREFIX_SRC 1
#define SUFFIX_SRC 2
#define PREFIX_DEST 3
#define SUFFIX_DEST 4
#define OUTPUT_EXPLICIT_FILENAME 5

const char* usage_Convert =
     "Usage: Convert [OPTIONS] <text_1> [<text_2> <text_3> ...]\n"
     "\n"
     "  <text_i>: text file to be converted\n"
     "\n"
     "OPTIONS:\n"
     "  -s X/--src=X: source encoding of the text file to be converted\n"
     "  -d X/--dest=X: encoding of the destination text file. The default value\n"
     "                 is LITTLE-ENDIAN\n"
     "\n"
     "Transliteration options (only for Arabic):\n"
     "  -F/--delaf: the input is a delaf, and we only want to transliterate the\n"
     "              inflected form and the lemma\n"
     "  -S/--delas: the input is a delas, and we only want to transliterate the\n"
     "              the lemma\n"
     "\n"
     "Output options:\n"
     "  -r/--replace: sources files will be replaced by destination files (default)\n"
     "  -o file/--output==file: name of destination file (only one file to convert)\n"
     "  --ps=PFX: source files will be renamed with the prefix PFX\n"
     "  --pd=PFX: destination files will be named with the prefix PFX\n"
     "  --ss=SFX: source files will be renamed with the suffix SFX\n"
     "  --sd=SFX: destination files will be named with the suffix SFX\n"
     "\n"
     "HTML options:\n"
     "  --dnc (Decode Normal Chars): things like '&eacute;' '&#120;' and '&#xF8;'\n"
     "        will be decoded as the one equivalent unicode character, except if\n"
     "        it represents an HTML control character\n"
     "  --dcc (Decode Control Chars): '&lt;' '&gt;' '&amp;' and '&quot;'\n"
     "        will be decoded as '<' '>' '&' and the quote (the same for their\n"
     "        decimal and hexadecimal representations)\n"
     "  --eac (Encode All Chars): every character that is not supported by the\n"
     "        output encoding will be coded as a string like '&#457;'\n"
     "  --ecc (Encode Control Chars): '<' '>' '&' and the quote will be encoded\n"
     "         by '&lt;' '&gt;' '&amp;' and '&quot;'\n"
     "\n"
     "Other options:\n"
     "  -m/--main-names: to get the list of the encoding main names\n"
     "  -a/--aliases: to get the whole list (main names+aliases)\n"
     "  -A/--all-infos: to display all the information about all the encodings\n"
     "  -i X/--info=X: to get information about the encoding X\n"
     "  -V/--only-verify-arguments: only verify arguments syntax and exit\n"
     "  -h/--help: this help\n"
     "\n"
     "Converts a text file into another encoding.\n";


static void usage() {
  display_copyright_notice();
  u_printf(usage_Convert);
}


const char* optstring_Convert=":s:d:ri:VhmaAo:k:q:FS";
const struct option_TS lopts_Convert[]= {
  {"src",required_argument_TS,NULL,'s'},
  {"dest",required_argument_TS,NULL,'d'},
  {"output",required_argument_TS,NULL,'o'},
  {"replace",no_argument_TS,NULL,'r'},
  {"ps",required_argument_TS,NULL,0},
  {"pd",required_argument_TS,NULL,1},
  {"ss",required_argument_TS,NULL,2},
  {"sd",required_argument_TS,NULL,3},
  {"dnc",no_argument_TS,NULL,4},
  {"dcc",no_argument_TS,NULL,5},
  {"eac",no_argument_TS,NULL,6},
  {"ecc",no_argument_TS,NULL,7},
  {"main-names",no_argument_TS,NULL,'m'},
  {"aliases",no_argument_TS,NULL,'a'},
  {"all-infos",no_argument_TS,NULL,'A'},
  {"info",required_argument_TS,NULL,'i'},
  {"only_verify_arguments",no_argument_TS,NULL,'V'},
  {"help",no_argument_TS,NULL,'h'},
  {"input_encoding",required_argument_TS,NULL,'k'},
  {"output_encoding",required_argument_TS,NULL,'q'},
  {"delaf",no_argument_TS,NULL,'F'},
  {"delas",no_argument_TS,NULL,'S'},
  {NULL,no_argument_TS,NULL,0}
};


int main_Convert(int argc,char* const argv[]) {
if (argc==1) {
  usage();
  return SUCCESS_RETURN_CODE;
}
/* First, we install all the available encoding */
void *encoding_ctx = install_all_encodings();
/* And we analyze the parameters */


int val,index=-1;
char src[1024]="";
char dest[1024]="";
char FX[128]="";
char input_name[FILENAME_MAX];
char output_name[FILENAME_MAX];
int output_mode=REPLACE_FILE;
int decode_normal_characters=0;
int decode_control_characters=0;
int encode_all_characters=0;
int encode_control_characters=0;
int format=CONV_REGULAR_FILE;
bool only_verify_arguments = false;
UnitexGetOpt options;
while (EOF!=(val=options.parse_long(argc,argv,optstring_Convert,lopts_Convert,&index))) {
   switch(val) {
   case 'k':
   case 's': if (options.vars()->optarg[0]=='\0') {
                error("You must specify a non empty source encoding\n");
                free_encodings_context(encoding_ctx);
                return USAGE_ERROR_CODE;
             }
             strcpy(src,options.vars()->optarg);
             break;
   case 'q':
   case 'd': if (options.vars()->optarg[0]=='\0') {
                error("You must specify a non empty destination encoding\n");
                free_encodings_context(encoding_ctx);
                return USAGE_ERROR_CODE;
             }
             strcpy(dest,options.vars()->optarg);
             break;
   case 'o': output_mode=OUTPUT_EXPLICIT_FILENAME; strcpy(output_name,options.vars()->optarg); break;
   case 'r': output_mode=REPLACE_FILE; break;
   case 0: output_mode=PREFIX_SRC; strcpy(FX,options.vars()->optarg); break;
   case 1: output_mode=PREFIX_DEST; strcpy(FX,options.vars()->optarg); break;
   case 2: output_mode=SUFFIX_SRC; strcpy(FX,options.vars()->optarg); break;
   case 3: output_mode=SUFFIX_DEST; strcpy(FX,options.vars()->optarg); break;
   case 4: decode_normal_characters=1; break;
   case 5: decode_control_characters=1; break;
   case 6: encode_all_characters=1; break;
   case 7: encode_control_characters=1; break;


   case 'm': print_encoding_main_names(encoding_ctx);
             free_encodings_context(encoding_ctx);
             return SUCCESS_RETURN_CODE;
   case 'a': print_encoding_main_names(encoding_ctx);
             print_encoding_aliases(encoding_ctx);
             free_encodings_context(encoding_ctx);
             return SUCCESS_RETURN_CODE;
   case 'A': print_information_for_all_encodings(encoding_ctx);
             free_encodings_context(encoding_ctx);
             return SUCCESS_RETURN_CODE;
   case 'i': if (options.vars()->optarg[0]=='\0') {
                error("You must specify a non empty encoding\n");
                free_encodings_context(encoding_ctx);
                return USAGE_ERROR_CODE;
             }
             print_encoding_infos(encoding_ctx,options.vars()->optarg);
             free_encodings_context(encoding_ctx);
             return SUCCESS_RETURN_CODE;
   case 'F': format=CONV_DELAF_FILE; break;
   case 'S': format=CONV_DELAS_FILE; break;
   case 'V': only_verify_arguments = true;
             break;
   case 'h': usage();
             free_encodings_context(encoding_ctx);
             return SUCCESS_RETURN_CODE;
   case ':': index==-1 ? error("Missing argument for option -%c\n",options.vars()->optopt) :
                         error("Missing argument for option --%s\n",lopts_Convert[index].name);
             free_encodings_context(encoding_ctx);
             return USAGE_ERROR_CODE;
   case '?': index==-1 ? error("Invalid option -%c\n",options.vars()->optopt) :
                         error("Invalid option --%s\n",options.vars()->optarg);
             free_encodings_context(encoding_ctx);
             return USAGE_ERROR_CODE;
   }
   index=-1;
}

if (src[0]=='\0') {
  error("You must specify the source encoding\n");
  free_encodings_context(encoding_ctx);
  return USAGE_ERROR_CODE;
}
if (dest[0]=='\0') {
   strcpy(dest,"utf16-le");
}
if (options.vars()->optind==argc) {
  error("Invalid arguments: rerun with --help\n");
  free_encodings_context(encoding_ctx);
  return USAGE_ERROR_CODE;
}
const struct encoding* src_encoding=get_encoding(encoding_ctx,src);
if (src_encoding==NULL) {
  error("%s is not a valid encoding name\n",src);
  free_encodings_context(encoding_ctx);
  return USAGE_ERROR_CODE;
}
const struct encoding* dest_encoding=get_encoding(encoding_ctx,dest);
if (dest_encoding==NULL) {
  error("%s is not a valid encoding name\n",dest);
  free_encodings_context(encoding_ctx);
  return USAGE_ERROR_CODE;
}

if ((output_mode == OUTPUT_EXPLICIT_FILENAME) && ((options.vars()->optind+1)!=argc)) {
  error("explicit output filename need exactly one input file\n");
  free_encodings_context(encoding_ctx);
  return USAGE_ERROR_CODE;
}

if (only_verify_arguments) {
  // freeing all allocated memory
  free_encodings_context(encoding_ctx);
  return SUCCESS_RETURN_CODE;
}

/*
 * Now we will transcode all the files described by the remaining
 * parameters.
 */
U_FILE* input=NULL;
U_FILE* output=NULL;
int error_code=CONVERSION_OK;
for (int i=options.vars()->optind;i<argc;i++) {
  /*
   * We set input and output file names according to the output mode
   */
  switch (output_mode) {
    case OUTPUT_EXPLICIT_FILENAME:
          strcpy(input_name,argv[i]);
          break;
    case REPLACE_FILE:
          strcpy(input_name,argv[i]);
          add_suffix_to_file_name(output_name,input_name,"_TEMP");
          break;
    case PREFIX_SRC:
          strcpy(output_name,argv[i]);
          add_prefix_to_file_name(input_name,output_name,FX);
          af_remove(input_name);
          af_rename(argv[i],input_name);
          break;
    case SUFFIX_SRC:
          strcpy(output_name,argv[i]);
          add_suffix_to_file_name(input_name,output_name,FX);
          af_remove(input_name);
          af_rename(argv[i],input_name);
          break;
    case PREFIX_DEST:
          strcpy(input_name,argv[i]);
          add_prefix_to_file_name(output_name,input_name,FX);
          break;
    case SUFFIX_DEST:
          strcpy(input_name,argv[i]);
          add_suffix_to_file_name(output_name,input_name,FX);
          break;
    default:
          error("Internal error in Convert\n");
          free_encodings_context(encoding_ctx);
          return DEFAULT_ERROR_CODE;
  }
  /*
   * We open files as binary ones. Note that we do not read the 2-bytes
   * header in the case of unicode files. This is delegated to the
   * conversion function.
   */
  input=u_fopen(BINARY,input_name,U_READ);
  int problem=0;
  if (input==NULL) {
    error("Cannot open %s\n",argv[i]);
    problem=1;
  }
  else {
    output=u_fopen(BINARY,output_name,U_WRITE);
    if (output==NULL) {
      error("Cannot write to file %s\n",output_name);
      problem=1;
      u_fclose(input);
    }
  }
  /*
   * We do the conversion and we close the files.
   */
  if (!problem) {
    error_code=convert(encoding_ctx,
                     input,
                     output,
                     src_encoding,
                     dest_encoding,
                     decode_normal_characters,
                     decode_control_characters,
                     encode_all_characters,
                     encode_control_characters,
                     format);
    u_fclose(input);
    u_fclose(output);
    switch(error_code) {
    case CONVERSION_OK:
          u_printf("%s converted\n",argv[i]);
          if (output_mode==REPLACE_FILE) {
            /* If we must replace the input file */
            if (af_remove(argv[i])!=0) {
              error("Cannot remove %s\n",argv[i]);
              free_encodings_context(encoding_ctx);
              return DEFAULT_ERROR_CODE;
            }
            if (af_rename(output_name,argv[i])!=0) {
              error("Cannot rename %s to %s\n",output_name,argv[i]);
              free_encodings_context(encoding_ctx);
              return DEFAULT_ERROR_CODE;
            }
          }
          break;
    case INPUT_FILE_NOT_IN_UTF16_LE:
          error("Error: %s is not a Unicode Little-Endian file\n",argv[i]);
          free_encodings_context(encoding_ctx);
          return DEFAULT_ERROR_CODE;
    case INPUT_FILE_NOT_IN_UTF16_BE:
          error("Error: %s is not a Unicode Big-Endian file\n",argv[i]);
          free_encodings_context(encoding_ctx);
          return DEFAULT_ERROR_CODE;
    case INPUT_FILE_NOT_IN_UTF8:
          error("Error: %s is not a UTF8 file\n",argv[i]);
          free_encodings_context(encoding_ctx);
          return DEFAULT_ERROR_CODE;
    default:
          error("Internal error in Convert\n");
          free_encodings_context(encoding_ctx);
          return DEFAULT_ERROR_CODE;
    }
    }
}
free_encodings_context(encoding_ctx);
return SUCCESS_RETURN_CODE;
}

} // namespace unitex
