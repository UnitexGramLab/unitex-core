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

/*
 * author : Anthony Sigogne
 */

#include <stdio.h>
#include "Copyright.h"
#include "UnitexGetOpt.h"
#include "Error.h"
#include "Tfst.h"
#include "File.h"
#include "DELA.h"
#include "Unicode.h"
#include "TrainingTagger.h"
#include "TrainingProcess.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

const char* usage_TrainingTagger =
         "Usage: TrainingTagger [OPTIONS] <text>\n"
         "\n"
         "  <text>: the text corpus to use in input\n"
         "\n"
         "Output options:\n"
         "  -b/--binaries: indicates whether the program should compress data files into"
         " .bin files (default)\n"
         "  -n/--no_binaries: indicates whether the program should not compress data files into"
         " .bin files, in this case only .dic data files are generated\n"
         "  -a/--all: indicates whether the program should produce all data files (default)\n"
         "  -c/--cat: indicates whether the program should produce only data file with 'cat' tags\n"
         "  -m/--morph: indicates whether the program should produce only data file with 'morph' tags\n"
         "  -o XXX/--output=XXX: pattern used to name output tagger data files XXX_data_cat.bin"
         " and XXX_data_morph.bin (default=filename of text corpus without extension)\n"
         "  -s/--semitic: the output .bin will use the semitic compression algorithm\n"
         "  -V/--only-verify-arguments: only verify arguments syntax and exit\n"
         "  -h/--help: this help\n"
         "\n"
         "Extract statistics from a tagged corpus and save its into a tagger data file. "
         "These statistics are necessary in the tagging process in order to compute probabilities."
         "Tagger data files are saved in the same path as corpus file.\n\n";

static void usage() {
  display_copyright_notice();
  u_printf(usage_TrainingTagger);
}

const char* optstring_TrainingTagger=":o:Vhbnriask:q:";
const struct option_TS lopts_TrainingTagger[]= {
  {"output",required_argument_TS,NULL,'o'},
  {"binaries",no_argument_TS,NULL,'b'},
  {"no_binaries",no_argument_TS,NULL,'n'},
  {"cat",no_argument_TS,NULL,'r'},
  {"morph",no_argument_TS,NULL,'i'},
  {"all",no_argument_TS,NULL,'a'},
  {"semitic",no_argument_TS,NULL,'s'},
  {"input_encoding",required_argument_TS,NULL,'k'},
  {"output_encoding",required_argument_TS,NULL,'q'},
  {"only_verify_arguments",no_argument_TS,NULL,'V'},
  {"help",no_argument_TS,NULL,'h'},
  {NULL,no_argument_TS,NULL,0}
};


int main_TrainingTagger(int argc,char* const argv[]) {
if (argc==1) {
   usage();
   return SUCCESS_RETURN_CODE;
}

int val,index=-1,binaries=1,r_forms=1,i_forms=1;
int semitic=0;
char text[FILENAME_MAX]="";
char raw_forms[FILENAME_MAX]="";
char inflected_forms[FILENAME_MAX]="";
char output[FILENAME_MAX]="";
VersatileEncodingConfig vec=VEC_DEFAULT;
bool only_verify_arguments = false;
UnitexGetOpt options;
while (EOF!=(val=options.parse_long(argc,argv,optstring_TrainingTagger,lopts_TrainingTagger,&index))) {
   switch(val) {
   case 'o': if (options.vars()->optarg[0]=='\0') {
               error("You must specify a non empty pattern\n");
               return USAGE_ERROR_CODE;
             }
             strcpy(output,options.vars()->optarg);
             break;
   case 'b': binaries = 1;
			 break;
   case 'n': binaries = 0;
			 break;
   case 'a': break;
   case 'c': i_forms = 0;
			 break;
   case 'm': r_forms = 0;
   			 break;
   case 'S': semitic=1;
   			 break;
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
   case 'V': only_verify_arguments = true;
             break;
   case 'h': usage();
             return SUCCESS_RETURN_CODE;
   case ':': index==-1 ? error("Missing argument for option -%c\n",options.vars()->optopt) :
                         error("Missing argument for option --%s\n",lopts_TrainingTagger[index].name);
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

strcpy(text,argv[options.vars()->optind]);
U_FILE* input_text=u_fopen(&vec,text,U_READ);
if (input_text==NULL) {   
  error("cannot open file %s\n",text);
  return DEFAULT_ERROR_CODE;
}

if(output[0]=='\0'){
  remove_path_and_extension(text,output);
}

char path[FILENAME_MAX],filename[FILENAME_MAX];
get_path(text,path);
if(strlen(path) == 0){
	strcpy(path,".");
}
/* we create files which will contain statistics extracted from the tagged corpus */
U_FILE* rforms_file = NULL, *iforms_file = NULL;
if(r_forms == 1){
	sprintf(filename,"%s_data_cat.dic",output);
	new_file(path,filename,raw_forms);
	rforms_file=u_fopen(&vec,raw_forms,U_WRITE);
}
if(i_forms == 1){
	sprintf(filename,"%s_data_morph.dic",output);
	new_file(path,filename,inflected_forms);
	iforms_file=u_fopen(&vec,inflected_forms,U_WRITE);
}

u_printf("Gathering statistics from tagged corpus...\n");
do_training(input_text,rforms_file,iforms_file);

/* we close all files and then we sort text dictionaries */
u_fclose(input_text);
char disclaimer[FILENAME_MAX];
if(rforms_file != NULL){
	u_fclose(rforms_file);
	pseudo_main_SortTxt(&vec,0,0,NULL,NULL,0,raw_forms,0);
	strcpy(disclaimer,raw_forms);
	remove_extension(disclaimer);
	strcat(disclaimer,".txt");
	create_disclaimer(&vec,disclaimer);
}
if(iforms_file != NULL){
	u_fclose(iforms_file);
	pseudo_main_SortTxt(&vec,0,0,NULL,NULL,0,inflected_forms,0);
	strcpy(disclaimer,inflected_forms);
	remove_extension(disclaimer);
	strcat(disclaimer,".txt");
	create_disclaimer(&vec,disclaimer);
}

/* we compress dictionaries if option is specified by user (output is ".bin") */
if(binaries == 1){
/* simple forms dictionary */
if(r_forms == 1){
	pseudo_main_Compress(&vec,0,semitic,raw_forms,1);
}
/* compound forms dictionary */
if(i_forms == 1){
	pseudo_main_Compress(&vec,0,semitic,inflected_forms,1);
}
}

u_printf("Done.\n");
return SUCCESS_RETURN_CODE;
}

} // namespace unitex
