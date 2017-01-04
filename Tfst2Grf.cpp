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
#include "Unicode.h"
#include "Tfst.h"
#include "Sentence2Grf.h"
#include "File.h"
#include "List_int.h"
#include "Copyright.h"
#include "Error.h"
#include "UnitexGetOpt.h"
#include "Tfst2Grf.h"
#include "Grf_lib.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

const char* usage_Tfst2Grf =
         "Usage: Tfst2Grf [OPTIONS] <tfst>\n"
         "\n"
         "  <tfst>: the .tfst file that contains the text automaton.\n"
         "\n"
         "OPTIONS:\n"
         "  -s N/--sentence=N: the number of the sentence to be converted.\n"
         "  -o XXX/--output=XXX:  name .grf file as XXX.grf and the .txt one as XXX.txt (default=cursentence)\n"
         "  -f FONT/--font=FONT: use the font FONT in the output .grf (default=Times new Roman).\n"
         "  -z N/--fontsize=N: set the font size (default=10).\n"
         "  -V/--only-verify-arguments: only verify arguments syntax and exit\n"
         "  -h/--help: this help\n"
         "\n"
         "Converts a sentence automaton into a GRF file that can be viewed. The\n"
         "resulting file, named cursentence.grf, is stored in the same directory\n"
         "that <text automaton>. The text of the sentence is saved in the same\n"
         "directory, in a file named cursentence.txt. The numbers of the tokens are\n"
         "saved in a file named cursentence.tok.\n";


static void usage() {
  display_copyright_notice();
  u_printf(usage_Tfst2Grf);
}


const char* optstring_Tfst2Grf=":s:o:f:z:Vhk:q:";
const struct option_TS lopts_Tfst2Grf[]= {
  {"sentence",required_argument_TS,NULL,'s'},
  {"output",required_argument_TS,NULL,'o'},
  {"font",required_argument_TS,NULL,'f'},
  {"fontsize",required_argument_TS,NULL,'z'},
  {"input_encoding",required_argument_TS,NULL,'k'},
  {"output_encoding",required_argument_TS,NULL,'q'},
  {"only_verify_arguments",no_argument_TS,NULL,'V'},
  {"help",no_argument_TS,NULL,'h'},
  {NULL,no_argument_TS,NULL,0}
};


int main_Tfst2Grf(int argc,char* const argv[]) {
if (argc==1) {
   usage();
   return SUCCESS_RETURN_CODE;
}

int SENTENCE=-1;
int size=10;
char* fontname=NULL;
char* output=NULL;
int is_sequence_automaton=0;
VersatileEncodingConfig vec=VEC_DEFAULT;
int val,index=-1;
char foo;
bool only_verify_arguments = false;
UnitexGetOpt options;
while (EOF!=(val=options.parse_long(argc,argv,optstring_Tfst2Grf,lopts_Tfst2Grf,&index))) {
   switch(val) {
   case 's': if (1!=sscanf(options.vars()->optarg,"%d%c",&SENTENCE,&foo) || SENTENCE<=0) {
                /* foo is used to check that the sentence number is not like "45gjh" */
                error("Invalid sentence number: %s\n",options.vars()->optarg);
                free(fontname);
                free(output);
                return USAGE_ERROR_CODE;
             }
             break;
   case 'o': if (options.vars()->optarg[0]=='\0') {
                error("You must specify a non empty output name pattern\n");
                free(fontname);
                return USAGE_ERROR_CODE;
             }
             output=strdup(options.vars()->optarg);
             if (output==NULL) {
                alloc_error("main_Tfst2Grf");
                free(fontname);
                return ALLOC_ERROR_CODE;
             }
             break;
   case 'f': if (options.vars()->optarg[0]=='\0') {
                error("You must specify a non empty font name\n");
                free(output);
                return USAGE_ERROR_CODE;
             }
             fontname=strdup(options.vars()->optarg);
             if (fontname==NULL) {
                alloc_error("main_Tfst2Grf");
                free(output);
                return ALLOC_ERROR_CODE;
             }
             break;
   case 'z': if (1!=sscanf(options.vars()->optarg,"%d%c",&size,&foo) || size<=0) {
                /* foo is used to check that the sentence number is not like "45gjh" */
                error("Invalid font size: %s\n",options.vars()->optarg);
                free(fontname);
                free(output);
                return USAGE_ERROR_CODE;
             }
             break;
   case 'k': if (options.vars()->optarg[0]=='\0') {
                error("Empty input_encoding argument\n");
                free(fontname);
                free(output);
                return USAGE_ERROR_CODE;
             }
             decode_reading_encoding_parameter(&(vec.mask_encoding_compatibility_input),options.vars()->optarg);
             break;
   case 'q': if (options.vars()->optarg[0]=='\0') {
                error("Empty output_encoding argument\n");
                free(fontname);
                free(output);
                return USAGE_ERROR_CODE;
             }
             decode_writing_encoding_parameter(&(vec.encoding_output),&(vec.bom_output),options.vars()->optarg);
             break;
   case 'V': only_verify_arguments = true;
             break;
   case 'h': usage();
             free(fontname);
             free(output);
             return SUCCESS_RETURN_CODE;
   case ':': index==-1 ? error("Missing argument for option -%c\n",options.vars()->optopt) :
                         error("Missing argument for option --%s\n",lopts_Tfst2Grf[index].name);
             free(fontname);
             free(output);
             return USAGE_ERROR_CODE;
   case '?': index==-1 ? error("Invalid option -%c\n",options.vars()->optopt) :
                         error("Invalid option --%s\n",options.vars()->optarg);
             free(fontname);
             free(output);
             return USAGE_ERROR_CODE;
   }
   index=-1;
}

if (SENTENCE==-1) {
  error("You must specify a sentence number\n");
  free(fontname);
  free(output);
  return USAGE_ERROR_CODE;
}

if (options.vars()->optind!=argc-1) {
  error("Invalid arguments: rerun with --help\n");
  free(fontname);
  free(output);
  return USAGE_ERROR_CODE;
}

if (only_verify_arguments) {
  // freeing all allocated memory
  free(fontname);
  free(output);
  return SUCCESS_RETURN_CODE;
}

char grf_name[FILENAME_MAX];
char txt_name[FILENAME_MAX];
char tok_name[FILENAME_MAX];
char start_name[FILENAME_MAX];

get_path(argv[options.vars()->optind],grf_name);
get_path(argv[options.vars()->optind],txt_name);
get_path(argv[options.vars()->optind],tok_name);
get_path(argv[options.vars()->optind],start_name);
if (output==NULL) {
   strcat(grf_name,"cursentence.grf");
   strcat(txt_name,"cursentence.txt");
   strcat(tok_name,"cursentence.tok");
   strcat(start_name,"cursentence.start");
} else {
   strcat(grf_name,output);
   strcat(grf_name,".grf");
   strcat(txt_name,output);
   strcat(txt_name,".txt");
   strcat(tok_name,output);
   strcat(tok_name,".tok");
   strcat(start_name,output);
   strcat(start_name,".start");
}

if (fontname==NULL) {
   fontname=strdup("Times New Roman");
   if (fontname==NULL) {
      alloc_error("main_Tfst2Grf");
      free(output);
      return ALLOC_ERROR_CODE;
   }
}

U_FILE* f=u_fopen(&vec,grf_name,U_WRITE);
if (f==NULL) {
   error("Cannot open file %s\n",grf_name);
   free(fontname);
   free(output);
   return DEFAULT_ERROR_CODE;
}

U_FILE* txt=u_fopen(&vec,txt_name,U_WRITE);
if (txt==NULL) {
   error("Cannot open file %s\n",txt_name);
   u_fclose(f);
   free(fontname);
   free(output);
   return DEFAULT_ERROR_CODE;
}

U_FILE* tok=u_fopen(&vec,tok_name,U_WRITE);
if (tok==NULL) {
   error("Cannot open file %s\n",tok_name);
   u_fclose(f);
   u_fclose(txt);
   free(fontname);
   free(output);
   return DEFAULT_ERROR_CODE;
}

u_printf("Loading %s...\n",argv[options.vars()->optind]);
Tfst* tfst=open_text_automaton(&vec,argv[options.vars()->optind]);

load_sentence(tfst,SENTENCE);
u_fprintf(txt,"%S\n",tfst->text);
u_fclose(txt);

U_FILE* start=u_fopen(&vec,start_name,U_WRITE);
if (start==NULL) {
   error("Cannot open file %s\n",start_name);
   close_text_automaton(tfst);
   u_fclose(tok);
   u_fclose(f);
   free(fontname);
   free(output);
   return DEFAULT_ERROR_CODE;
}

u_fprintf(start,"%d %d\n",tfst->offset_in_tokens,tfst->offset_in_chars);
u_fclose(start);

for (int i=0;i<tfst->tokens->nbelems;i++) {
   u_fprintf(tok,"%d %d\n",tfst->tokens->tab[i],tfst->token_sizes->tab[i]);
}
u_fclose(tok);

u_printf("Creating GRF...\n");
Grf* grf=sentence_to_grf(tfst,fontname,size,is_sequence_automaton);
save_Grf(f,grf);

free_Grf(grf);
u_fclose(f);
free(fontname);
free(output);
close_text_automaton(tfst);

u_printf("Done.\n");
return SUCCESS_RETURN_CODE;
}

} // namespace unitex
