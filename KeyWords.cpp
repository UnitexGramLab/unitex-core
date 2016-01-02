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
#include <string.h>
#include <stdlib.h>
#include "Error.h"
#include "UnitexGetOpt.h"
#include "File.h"
#include "Alphabet.h"
#include "Unicode.h"
#include "Copyright.h"
#include "KeyWords.h"
#include "KeyWords_lib.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

const char* usage_KeyWords =
  "Usage : KeyWords <tok_by_freq> <dic1> [<dic2> ...]\n"
  "\n"
  "  <tok_by_freq>: a tok_by_freq.txt file produced by Tokenize\n"
  "  <dicX>:        a DELAF in text format (dlf, dlc)\n"
  "\n"
  "OPTIONS:\n"
  "  -o OUT/--output=OUT: name of destination file (default=keywords.txt in the\n"
  "                       same directory than <tok_by_freq>\n"
  "  -a ALPH/--alphabet=ALPH: name of the alphabet file to use for tokenizing\n"
  "                           lexical units\n"
  "  -f CODE/--forbidden_code=CODE: the grammatical/semantic code that will be\n"
  "                                 used to filter tokens (default=XXX)\n"
  "  -c F/--cdic=F: a text file containing one compound word per line\n"
  "  -V/--only-verify-arguments: only verify arguments syntax and exit\n"
  "  -h/--help: this help\n"
  "\n"
  "Removes from the given token file every token that is part of a DELAF entry\n"
  "that contains the forbidden code. A frequency estimation is computed for compound\n"
  "words, and tokens are lemmatized.\n";


static void usage() {
  display_copyright_notice();
  u_printf(usage_KeyWords);
}


const char* optstring_KeyWords=":o:a:f:c:Vhk:q:";
const struct option_TS lopts_KeyWords[]= {
  {"output",required_argument_TS,NULL,'o'},
  {"alphabet",required_argument_TS,NULL,'a'},
  {"forbidden_code",required_argument_TS,NULL,'f'},
  {"cdic",required_argument_TS,NULL,'c'},
  {"input_encoding",required_argument_TS,NULL,'k'},
  {"output_encoding",required_argument_TS,NULL,'q'},
  {"only_verify_arguments",no_argument_TS,NULL,'V'},
  {"help",no_argument_TS,NULL,'h'},
  {NULL,no_argument_TS,NULL,0}
};


/**
 * The same than main, but no call to setBufferMode.
 */
int main_KeyWords(int argc,char* const argv[]) {
if (argc==1) {
   usage();
   return SUCCESS_RETURN_CODE;
}

VersatileEncodingConfig vec=VEC_DEFAULT;

char tokens[FILENAME_MAX];
char output[FILENAME_MAX]="";
char alph[FILENAME_MAX]="";
char cdic[FILENAME_MAX]="";
unichar* code=u_strdup("XXX");
int val,index=-1;
bool only_verify_arguments = false;
UnitexGetOpt options;

while (EOF!=(val=options.parse_long(argc,argv,optstring_KeyWords,lopts_KeyWords,&index))) {
   switch(val) {
   case 'o': if (options.vars()->optarg[0]=='\0') {
                error("You must specify a non empty output\n");
                free(code);
                return USAGE_ERROR_CODE;
             }
             strcpy(output,options.vars()->optarg);
             break;
   case 'a': if (options.vars()->optarg[0]=='\0') {
                error("You must specify a non empty alphabet file name\n");
                free(code);
                return USAGE_ERROR_CODE;                
             }
             strcpy(alph,options.vars()->optarg);
             break;
   case 'f': if (options.vars()->optarg[0]=='\0') {
                error("You must specify a non empty forbidden code\n");
                free(code);
                return USAGE_ERROR_CODE;                
             }
             free(code);
             code=u_strdup(options.vars()->optarg);
             break;
   case 'c': if (options.vars()->optarg[0]=='\0') {
                error("You must specify a non empty file name\n");
                free(code);
                return USAGE_ERROR_CODE;                
             }
             strcpy(cdic,options.vars()->optarg);
             break;
   case 'V': only_verify_arguments = true;
             break;
   case 'h': usage();
             free(code);
             return SUCCESS_RETURN_CODE;
   case 'k': if (options.vars()->optarg[0]=='\0') {
                error("Empty input_encoding argument\n");
                free(code);
                return USAGE_ERROR_CODE;                
             }
             decode_reading_encoding_parameter(&(vec.mask_encoding_compatibility_input),options.vars()->optarg);
             break;
   case 'q': if (options.vars()->optarg[0]=='\0') {
                error("Empty output_encoding argument\n");
                free(code);
                return USAGE_ERROR_CODE;                
             }
             decode_writing_encoding_parameter(&(vec.encoding_output),&(vec.bom_output),options.vars()->optarg);
             break;
   case ':': index==-1 ? error("Missing argument for option -%c\n",options.vars()->optopt) :
                         error("Missing argument for option --%s\n",lopts_KeyWords[index].name);
             free(code);
             return USAGE_ERROR_CODE;
             break;
   case '?': index==-1 ? error("Invalid option -%c\n",options.vars()->optopt) :
                         error("Invalid option --%s\n",options.vars()->optarg);
             free(code);
             return USAGE_ERROR_CODE;
             break;
   }
   index=-1;
}

if (options.vars()->optind==argc || options.vars()->optind==argc-1) {
   error("Invalid arguments: rerun with --help\n");
   free(code);
   return USAGE_ERROR_CODE;
}

if (only_verify_arguments) {
  // freeing all allocated memory
  free(code);
  return SUCCESS_RETURN_CODE;
}

Alphabet* alphabet=NULL;
if (alph[0]!='\0') {
  alphabet=load_alphabet(&vec,alph);
  if (alphabet==NULL) {
    error("Cannot load alphabet file %s\n",alph);
    free(code);
    return DEFAULT_ERROR_CODE;    
  }
}

strcpy(tokens,argv[(options.vars()->optind++)]);
if (output[0]=='\0') {
	get_path(tokens,output);
	strcat(output,"keywords.txt");
}
struct string_hash_ptr* keywords=load_tokens_by_freq(tokens,&vec);
filter_non_letter_keywords(keywords,alphabet);
if (cdic[0]!='\0') {
	load_compound_words(cdic,&vec,keywords);
}

for (;options.vars()->optind!=argc;(options.vars()->optind)++) {
	filter_keywords_with_dic(keywords,argv[options.vars()->optind],&vec,alphabet);
}
merge_case_equivalent_unknown_words(keywords,alphabet);
struct string_hash* forbidden_lemmas=compute_forbidden_lemmas(keywords,code);
remove_keywords_with_forbidden_lemma(keywords,forbidden_lemmas);
free_string_hash(forbidden_lemmas);
vector_ptr* sorted=sort_keywords(keywords);
U_FILE* f_output=u_fopen(&vec,output,U_WRITE);

if (f_output==NULL) {
	error("Cannot write in file %s\n",output);
  free_vector_ptr(sorted,(void(*)(void*))free_KeyWord_list);
  free_string_hash_ptr(keywords,(void(*)(void*))free_KeyWord_list);
  free_alphabet(alphabet);
  free(code);
  return DEFAULT_ERROR_CODE;  
}

dump_keywords(sorted,f_output);

u_fclose(f_output);
free_vector_ptr(sorted,(void(*)(void*))free_KeyWord_list);
free_string_hash_ptr(keywords,(void(*)(void*))free_KeyWord_list);
free_alphabet(alphabet);
free(code);

return SUCCESS_RETURN_CODE;
}

} // namespace unitex
