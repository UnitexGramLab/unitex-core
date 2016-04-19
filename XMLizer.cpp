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
#include <stdlib.h>
#include <string.h>
#include "Copyright.h"
#include "Error.h"
#include "File.h"
#include "Fst2Txt_TokenTree.h"
#include "Fst2TxtAsRoutine.h"
#include "LocateConstants.h"
#include "NormalizeAsRoutine.h"
#include "ParsingInfo.h"
#include "Transitions.h"
#include "Unicode.h"
#include "UnitexGetOpt.h"
#include "XMLizer.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

#define XML 0
#define TEI 1

int xmlize(const VersatileEncodingConfig*,const char*,const char*,int);


/* Headers (XML & TEI) Variables
 * ----------------------------- */

static const char *xml_open = "<?xml version='1.0' encoding='UTF-8'?>\n<text>";
static const char *xml_close = "</text>";
static const char *tei_open = "<?xml version='1.0' encoding='UTF-8'?>\n<tei>\n<teiHeader/>\n<text>\n<body>\n<div xml:id=\"d1\">\n";
static const char *tei_close = "</div>\n</body>\n</text>\n</tei>";



const char* usage_XMLizer =
  "Usage: XMLizer [OPTIONS] <txt>\n"
  "\n"
  "  <txt>: the input text file\n"
  "\n"
  "OPTIONS:\n"
  "  -x/--xml: build a simple XML file from a raw text file\n"
  "  -t/--tei: build a minimal TEI file from a raw text file (default)\n"
  "  -n XXX/--normalization=XXX: optional configuration file for the normalization process\n"
  "  -o OUT/--output=OUT: optional output file name (default: file.txt > file.xml)\n"
  "  -a ALPH/--alphabet=ALPH: alphabet file\n"
  "  -s SEG/--segmentation_grammar=SEG: .fst2 segmentation grammar\n"
  "  -V/--only-verify-arguments: only verify arguments syntax and exit\n"
  "  -h/--help: this help\n"
  "\n"
  "Produces a TEI or simple XML file from the given raw text file.\n";


static void usage() {
  display_copyright_notice();
  u_printf(usage_XMLizer);
}


const char* optstring_XMLizer = ":xtn:o:a:s:Vhk:q:";
const struct option_TS lopts_XMLizer[] = {
  {"xml", no_argument_TS, NULL, 'x'},
  {"tei", no_argument_TS, NULL, 't'},
  {"normalization", required_argument_TS, NULL, 'n'},
  {"output", required_argument_TS, NULL, 'o'},
  {"alphabet", required_argument_TS, NULL, 'a'},
  {"segmentation_grammar", required_argument_TS, NULL, 's'},
  {"input_encoding",required_argument_TS,NULL,'k'},
  {"output_encoding",required_argument_TS,NULL,'q'},
  {"only_verify_arguments",no_argument_TS,NULL,'V'},
  {"help", no_argument_TS, NULL, 'h'},
  {NULL, no_argument_TS, NULL, 0}
};


int main_XMLizer(int argc,char* const argv[]) {
if (argc==1) {
   usage();
   return SUCCESS_RETURN_CODE;
}

int output_style=TEI;
char output[FILENAME_MAX]="";
char alphabet[FILENAME_MAX]="";
char normalization[FILENAME_MAX]="";
char segmentation[FILENAME_MAX]="";
VersatileEncodingConfig vec=VEC_DEFAULT;
int convLFtoCRLF=1;
int val,index=-1;
bool only_verify_arguments = false;
UnitexGetOpt options;
while (EOF!=(val=options.parse_long(argc,argv,optstring_XMLizer,lopts_XMLizer,&index))) {
   switch(val) {
   case 'x': output_style=XML; break;
   case 't': output_style=TEI; break;
   case 'n': if (options.vars()->optarg[0]=='\0') {
                error("You must specify a non empty normalization grammar name\n");
                return USAGE_ERROR_CODE;
             }
             strcpy(normalization,options.vars()->optarg);
             break;
   case 'o': if (options.vars()->optarg[0]=='\0') {
                error("You must specify a non empty output file name\n");
                return USAGE_ERROR_CODE;
             }
             strcpy(output,options.vars()->optarg);
             break;
   case 'a': if (options.vars()->optarg[0]=='\0') {
                error("You must specify a non empty alphabet file name\n");
                return USAGE_ERROR_CODE;
             }
             strcpy(alphabet,options.vars()->optarg);
             break;
   case 's': if (options.vars()->optarg[0]=='\0') {
                error("You must specify a non empty segmentation grammar name\n");
                return USAGE_ERROR_CODE;
             }
             strcpy(segmentation,options.vars()->optarg);
             break;
   case 'V': only_verify_arguments = true;
             break;
   case 'h': usage();
             return SUCCESS_RETURN_CODE;
   case ':': index==-1 ? error("Missing argument for option -%c\n",options.vars()->optopt) :
                         error("Missing argument for option --%s\n",lopts_XMLizer[index].name);
             return USAGE_ERROR_CODE;
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
   case '?': index==-1  ? error("Invalid option -%c\n",options.vars()->optopt) :
                          error("Invalid option --%s\n",options.vars()->optarg);
             return USAGE_ERROR_CODE;

   }
   index=-1;
}

if (options.vars()->optind!=argc-1) {
   error("Invalid arguments: rerun with --help\n");
   return USAGE_ERROR_CODE;
}

if (segmentation[0]=='\0') {
   error("You must specify the segmentation grammar to use\n");
   return USAGE_ERROR_CODE;
}

if (only_verify_arguments) {
  // freeing all allocated memory
  return SUCCESS_RETURN_CODE;
}

char input[FILENAME_MAX];
strcpy(input,argv[options.vars()->optind]);
char snt[FILENAME_MAX];
remove_extension(input,snt);
strcat(snt,"_tmp.snt");
char tmp[FILENAME_MAX];
remove_extension(input,tmp);
strcat(tmp,".tmp");
normalize(input,snt,&vec,KEEP_CARRIAGE_RETURN,convLFtoCRLF,normalization,NULL,1);
struct fst2txt_parameters* p=new_fst2txt_parameters();
p->vec=vec;
p->input_text_file=strdup(snt);
if (p->input_text_file ==NULL) {
   alloc_error("main_XMLizer");
   free_fst2txt_parameters(p);
   return ALLOC_ERROR_CODE;
}

p->output_text_file_is_temp=1;
p->output_text_file=strdup(tmp);
if (p->output_text_file==NULL) {
   alloc_error("main_XMLizer");
   free_fst2txt_parameters(p);
   return ALLOC_ERROR_CODE;
}
p->fst_file=strdup(segmentation);
if (p->fst_file==NULL) {
   alloc_error("main_XMLizer");
   free_fst2txt_parameters(p);
   return ALLOC_ERROR_CODE;
}
p->alphabet_file=strdup(alphabet);
if (p->alphabet_file==NULL) {
   alloc_error("main_XMLizer");
   free_fst2txt_parameters(p);
   return ALLOC_ERROR_CODE;
}

p->output_policy=MERGE_OUTPUTS;
p->tokenization_policy=WORD_BY_WORD_TOKENIZATION;
p->space_policy=DONT_START_WITH_SPACE;

main_fst2txt(p);

free_fst2txt_parameters(p);

if (output[0]=='\0') {
  remove_extension(input,output);
  strcat(output,".xml");
}

int return_value = xmlize(&vec,snt,output,output_style);

af_remove(snt);
af_remove(tmp);

return return_value;
}



int xmlize(const VersatileEncodingConfig* vec,const char* fin,const char* fout,int ouput_style) {
  U_FILE* input = u_fopen(vec, fin, U_READ);
  if (input == NULL) {
    error("Input file '%s' not found!\n", fin);
    return DEFAULT_ERROR_CODE;
  }

  U_FILE* output = u_fopen(UTF8, fout, U_WRITE);
  if (output == NULL) {
    error("Cannot open output file '%s'!\n", fout);
    u_fclose(input);
    return DEFAULT_ERROR_CODE;
  } else // FIXME(johndoe) put breaks

  if(ouput_style==XML) {
     u_fprintf(output, xml_open);
  }
  else {
     u_fprintf(output, tei_open);
  }

  int sentence_count = 1;
  int sentence_count_relative = 1;
  int paragraph_count = 1;

  u_fprintf(output, "<p><s id=\"n%d\" xml:id=\"d1p%ds%d\">",sentence_count++,paragraph_count,sentence_count_relative++);

  int current_state = 0;
  unichar c;
  int i;
  while ((i = u_fgetc(input)) != EOF) {
    c = (unichar)i;
    switch (current_state) {
      case 0: {
        if ( c == '{') current_state = 1;
        else if(c == '&') u_fprintf(output, "&amp;");
        else if(c == '<') u_fprintf(output, "&lt;");
        else if(c == '>') u_fprintf(output, "&gt;");
        else u_fputc(c, output);
        break;
      }
      case 1: {
        if (c == 'S') current_state = 2;
        else {
          u_fputc('{', output);
          u_fputc(c, output);
          current_state = 0;
        }
        break;
      }
      case 2: {
        if (c == '}') current_state = 3;
        else {
          u_fputc('{', output);
          u_fputc('S', output);
          u_fputc(c, output);
          current_state = 0;
        }
        break;
      }
      case 3: {
        if (c == '{') current_state = 4;
        else if (c == '\n' || c == ' ' || c == '\t') {
          u_fputc(c, output);
          current_state = 3;
        }
        else {
          u_fprintf(output, "</s><s id=\"n%d\" xml:id=\"d1p%ds%d\">",sentence_count++,paragraph_count,sentence_count_relative++);
          u_fputc(c, output);
          current_state = 0;
        }
        break;
      }
      case 4: {
        if (c == 'S') current_state = 7;
        else if (c == 'P') current_state = 5;
        else {
          u_fputc('{', output);
          u_fputc(c, output);
          current_state = 0;
        }
        break;
      }
      case 5: {
        if (c == '}') {
          u_fprintf(output, "</s></p>\n");
          paragraph_count++;
          sentence_count_relative=1;
          current_state = 6;
        } else {
          u_fputc('{', output);
          u_fputc('P', output);
          u_fputc(c, output);
          current_state = 0;
        }
        break;
      }
      case 6: {
        if (c == '\n' || c == ' ' || c == '\t') u_fputc(c, output);
        else {
          u_fprintf(output, "<p><s id=\"n%d\" xml:id=\"d1p%ds%d\">",sentence_count++,paragraph_count,sentence_count_relative++);
          u_fputc(c, output);
          current_state = 0;
        }
        break;
      }
      case 7: {
        if (c == '}') {
          current_state = 3;
        }
        else {
          u_fputc('{', output);
          u_fputc('S', output);
          u_fputc(c, output);
          current_state = 0;
        }
        break;
      }
    }
  }

  if (current_state == 3) {
    //...
  } else if (current_state == 6) {
    //...
  } else {
    u_fprintf(output, "</s></p>\n");
  }

  if(ouput_style==XML) {
     u_fprintf(output, xml_close);
  }
  else {
     u_fprintf(output, tei_close);
  }

  u_fclose(input);
  u_fclose(output);
  u_printf("Done.\n");
  return SUCCESS_RETURN_CODE;
}

} // namespace unitex
