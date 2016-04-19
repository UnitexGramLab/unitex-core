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
#include <stdlib.h>
#include <string.h>
#include "File.h"
#include "Error.h"
#include "Unicode.h"
#include "UnitexGetOpt.h"
#include "Unxmlize.h"
#include "Xml.h"
#include "Copyright.h"
#include "Offsets.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

const char* usage_Unxmlize =
     "Usage: Unxmlize <file>\n"
     "\n"
     "  <file>: an unicode .xml or .html file to be processed\n"
     "\n"
     "OPTIONS:\n"
     "  -o TXT/--output=TXT: output file. By default, foo.xml => foo.txt\n"
     "  --input_offsets=XXX: base offset file to be used\n"
     "  --output_offsets=XXX: specifies the offset file to be produced\n"
     "  --PRLG=XXX: extracts to file XXX special information used in the\n"
     "              PRLG project on ancient Greek (requires --output_offsets)\n"
     "\n"
     "  -t/--html: consider the file as html file (disregard extension)\n"
     "  -x/--xml: consider the file as xml file (disregard extension)\n"
     "  -l/--tolerate: try tolerate somes markup langage malformation\n"
     "\n"
     "  --comments=IGNORE: every comment is removed (default)\n"
     "  --comments=SPACE: every comment is replaced by a single space\n"
     "  --scripts=IGNORE: every script block is removed\n"
     "  --scripts=SPACE: every comment is replaced by a single space (default for .html)\n"
     "    Note: by default, script tags are handled as normal tags (default for .xml)\n"
     "\n"
     "  --normal_tags=IGNORE: every other tag is removed (default for .xml)\n"
     "  --normal_tags=SPACE: every other tag is replaced by a single space(default for .html)\n"
     "  -V/--only-verify-arguments: only verify arguments syntax and exit\n"
     "  -h/--help: this help\n"
     "\n"
     "Removes all xml tags from the given file to produce a text file that\n"
     "can be processed by Unitex.\n";


static void usage() {
  display_copyright_notice();
  u_printf(usage_Unxmlize);
}


const char* optstring_Unxmlize=":o:Vh:k:q:txlp:c:s:n:@:$:";
const struct option_TS lopts_Unxmlize[]={
  {"output", required_argument_TS, NULL, 'o'},
  {"input_offsets",required_argument_TS,NULL,'$'},
  {"output_offsets",required_argument_TS,NULL,'@'},
  {"PRLG",required_argument_TS,NULL,'p'},
  {"input_encoding",required_argument_TS,NULL,'k'},
  {"output_encoding",required_argument_TS,NULL,'q'},
  {"comments",required_argument_TS,NULL,'c'},
  {"scripts",required_argument_TS,NULL,'s'},
  {"normal_tags",required_argument_TS,NULL,'n'},
  {"tolerate", no_argument_TS, NULL, 'l'},
  {"html", no_argument_TS, NULL, 't'},
  {"xml", no_argument_TS, NULL, 'x'},
  {"only_verify_arguments",no_argument_TS,NULL,'V'},
  {"help", no_argument_TS, NULL, 'h'},
  {NULL, no_argument_TS, NULL, 0}
};


int main_Unxmlize(int argc,char* const argv[]) {
if (argc==1) {
   usage();
   return SUCCESS_RETURN_CODE;
}

char output[FILENAME_MAX]="";
char input_offsets[FILENAME_MAX]="";
char output_offsets[FILENAME_MAX] = "";
char output_PRLG[FILENAME_MAX]="";
char comments=-1;
char scripts=-1;
char normal_tags=-1;
int force_html=0;
int force_xml=0;
int tolerate_markup_malformation=0;

VersatileEncodingConfig vec=VEC_DEFAULT;
int val,index=-1;
bool only_verify_arguments = false;
UnitexGetOpt options;
while (EOF!=(val=options.parse_long(argc,argv,optstring_Unxmlize,lopts_Unxmlize,&index))) {
   switch(val) {
   case 'o': if (options.vars()->optarg[0]=='\0') {
                error("You must specify a non empty output file name\n");
                return USAGE_ERROR_CODE;
             }
             strcpy(output,options.vars()->optarg);
             break;
   case '$': if (options.vars()->optarg[0]=='\0') {
                   error("You must specify a non empty input offset file name\n");
                   return USAGE_ERROR_CODE;
                }
                strcpy(input_offsets,options.vars()->optarg);
                break;
   case '@': if (options.vars()->optarg[0]=='\0') {
                   error("You must specify a non empty offset file name\n");
                   return USAGE_ERROR_CODE;
                }
                strcpy(output_offsets,options.vars()->optarg);
                break;
   case 'p': if (options.vars()->optarg[0]=='\0') {
                   error("You must specify a non empty PRLG file name\n");
                   return USAGE_ERROR_CODE;
                }
                strcpy(output_PRLG,options.vars()->optarg);
                break;
   case 'c': {
     if (!strcmp(options.vars()->optarg,"IGNORE")) {
       comments=UNXMLIZE_IGNORE;
     } else if (!strcmp(options.vars()->optarg,"SPACE")) {
       comments=UNXMLIZE_REPLACE_BY_SPACE;
     } else {
       error("Invalid argument for option --comments\n");
       return USAGE_ERROR_CODE;
     }
     break;
   }
   case 's': {
     if (!strcmp(options.vars()->optarg,"IGNORE")) {
       scripts=UNXMLIZE_IGNORE;
     } else if (!strcmp(options.vars()->optarg,"SPACE")) {
       scripts=UNXMLIZE_REPLACE_BY_SPACE;
     } else {
       error("Invalid argument for option --scripts\n");
       return USAGE_ERROR_CODE;
     }
     break;
   }
   case 'n': {
     if (!strcmp(options.vars()->optarg,"IGNORE")) {
       normal_tags=UNXMLIZE_IGNORE;
     } else if (!strcmp(options.vars()->optarg,"SPACE")) {
       normal_tags=UNXMLIZE_REPLACE_BY_SPACE;
     } else {
       error("Invalid argument for option --normal_tags\n");
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
   case 't': force_html=1; break;
   case 'x': force_xml=1; break;
   case 'l': tolerate_markup_malformation=1; break;
   case 'V': only_verify_arguments = true;
             break;
   case 'h': usage();
             return SUCCESS_RETURN_CODE;
   case ':': index==-1 ? error("Missing argument for option -%c\n",options.vars()->optopt) :
                         error("Missing argument for option --%s\n",lopts_Unxmlize[index].name);
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

U_FILE* f_input   = NULL;
U_FILE* f_output  = NULL;
U_FILE* f_offsets = NULL;
vector_offset* offsets=NULL;
vector_offset* v_input_offsets = NULL;
UnxmlizeOpts opts;
if (output[0]=='\0') {
    remove_extension(argv[options.vars()->optind],output);
    strcat(output,".txt");
}

f_input=u_fopen(&vec,argv[options.vars()->optind],U_READ);
if (f_input==NULL) {
  error("Cannot open file %s\n",argv[options.vars()->optind]);
  return DEFAULT_ERROR_CODE;
}

char extension[FILENAME_MAX];
get_extension(argv[options.vars()->optind],extension);
int consider_as_html=0;
if (force_xml!=0) {
} else {
  if (force_html!=0) {
    consider_as_html=1;
  } else if (!strcmp(extension,".html") || !strcmp(extension,".HTML") || !strcmp(extension,".htm") || !strcmp(extension,".HTM")) {
    consider_as_html=1;
  }
}

if (consider_as_html!=0) {
  opts.comments=UNXMLIZE_IGNORE;
  opts.scripts=UNXMLIZE_REPLACE_BY_SPACE;
  opts.normal_tags=UNXMLIZE_REPLACE_BY_SPACE;
} else {
  opts.comments=UNXMLIZE_IGNORE;
  opts.scripts=UNXMLIZE_DO_NOTHING;
  opts.normal_tags=UNXMLIZE_REPLACE_BY_SPACE;
}
if (comments!=-1) {
  opts.comments=comments;
}
if (scripts!=-1) {
  opts.scripts=scripts;
}
if (normal_tags!=-1) {
  opts.normal_tags=normal_tags;
}

f_output = u_fopen(&vec,output,U_WRITE);
if (f_output==NULL) {
   error("Cannot create text file %s\n",output);
   u_fclose(f_input);
   return DEFAULT_ERROR_CODE;
}

if (output_offsets[0]!='\0') {
  if (input_offsets[0] != '\0') {
    v_input_offsets = load_offsets(&vec, input_offsets);
  }
  f_offsets=u_fopen(&vec,output_offsets,U_WRITE);
  if (f_offsets==NULL) {
    error("Cannot create offset file %s\n",output_offsets);
    free_vector_offset(v_input_offsets);
    u_fclose(f_output);
    u_fclose(f_input);
    return DEFAULT_ERROR_CODE;
  }
  offsets=new_vector_offset();
}
unichar* PRLG_ptrs[10]={0,0,0,0,0,0,0,0,0,0};
unichar** PRLG=NULL;
U_FILE* f_PRLG=NULL;
if (output_PRLG[0]!='\0') {
  if (f_offsets==NULL) {
    error("Cannot use the --PRLG option if --output_offsets is not used\n");
    free_vector_offset(offsets);
    free_vector_offset(v_input_offsets);
    u_fclose(f_output);
    u_fclose(f_input);
    return DEFAULT_ERROR_CODE;
  }
  PRLG=PRLG_ptrs;
  f_PRLG=u_fopen(&vec,output_PRLG,U_WRITE);
  if (f_offsets==NULL) {
    error("Cannot create PRLG file %s\n",output_PRLG);
    free_vector_offset(offsets);
    u_fclose(f_offsets);
    free_vector_offset(v_input_offsets);
    u_fclose(f_output);
    u_fclose(f_input);
    return DEFAULT_ERROR_CODE;
  }
}
if (!unxmlize(f_input,f_output,offsets,&opts,PRLG,f_PRLG,tolerate_markup_malformation)) {
  error("The input file was not a valid xml one. Operation aborted.\n");

  for (int i=0;i<10;i++) {
    if (PRLG_ptrs[i]!=NULL) {
      free(PRLG_ptrs[i]);
    }
  }

  af_remove(output);
  if (f_output!=NULL) {
    af_remove(output_offsets);
  }

  u_fclose(f_PRLG);
  free_vector_offset(offsets);
  u_fclose(f_offsets);
  free_vector_offset(v_input_offsets);
  u_fclose(f_output);
  u_fclose(f_input);
  return DEFAULT_ERROR_CODE;
}

for (int i=0;i<10;i++) {
  if (PRLG_ptrs[i]!=NULL) {
    free(PRLG_ptrs[i]);
  }
}

if (offsets!=NULL) {
  process_offsets(v_input_offsets,offsets,f_offsets);
}

u_fclose(f_PRLG);
free_vector_offset(offsets);
u_fclose(f_offsets);
free_vector_offset(v_input_offsets);
u_fclose(f_output);
u_fclose(f_input);
u_printf("\nDone.\n");
return SUCCESS_RETURN_CODE;
}

} // namespace unitex
