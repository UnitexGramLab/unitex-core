/*
 * Unitex
 *
 * Copyright (C) 2001-2013 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
         "  -h/--help: this help\n"
         "\n"
         "Removes from the given token file every token that is part of a DELAF entry\n"
		 "that contains the forbidden code. A frequency estimation is computed for compound\n"
		 "words, and tokens are lemmatized.\n";


static void usage() {
u_printf("%S",COPYRIGHT);
u_printf(usage_KeyWords);
}


const char* optstring_KeyWords=":o:a:f:c:hk:q:";
const struct option_TS lopts_KeyWords[]= {
      {"output",required_argument_TS,NULL,'o'},
      {"alphabet",required_argument_TS,NULL,'a'},
      {"forbidden_code",required_argument_TS,NULL,'f'},
      {"cdic",required_argument_TS,NULL,'c'},
      {"input_encoding",required_argument_TS,NULL,'k'},
      {"output_encoding",required_argument_TS,NULL,'q'},
      {"help",no_argument_TS,NULL,'h'},
      {NULL,no_argument_TS,NULL,0}
};


/**
 * The same than main, but no call to setBufferMode.
 */
int main_KeyWords(int argc,char* const argv[]) {
if (argc==1) {
   usage();
   return 0;
}

VersatileEncodingConfig vec=VEC_DEFAULT;

char tokens[FILENAME_MAX];
char output[FILENAME_MAX]="";
char alph[FILENAME_MAX]="";
char cdic[FILENAME_MAX]="";
unichar* code=u_strdup("XXX");
int val,index=-1;
struct OptVars* vars=new_OptVars();
while (EOF!=(val=getopt_long_TS(argc,argv,optstring_KeyWords,lopts_KeyWords,&index,vars))) {
   switch(val) {
   case 'o': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty output\n");
             }
             strcpy(output,vars->optarg);
             break;
   case 'a': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty alphabet file name\n");
             }
             strcpy(alph,vars->optarg);
             break;
   case 'f': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty forbidden code\n");
             }
			 free(code);
			 code=u_strdup(vars->optarg);
			 break;
   case 'c': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty file name\n");
             }
             strcpy(cdic,vars->optarg);
             break;
   case 'h': usage(); return 0;
   case 'k': if (vars->optarg[0]=='\0') {
                fatal_error("Empty input_encoding argument\n");
             }
             decode_reading_encoding_parameter(&(vec.mask_encoding_compatibility_input),vars->optarg);
             break;
   case 'q': if (vars->optarg[0]=='\0') {
                fatal_error("Empty output_encoding argument\n");
             }
             decode_writing_encoding_parameter(&(vec.encoding_output),&(vec.bom_output),vars->optarg);
             break;
   case ':': if (index==-1) fatal_error("Missing argument for option -%c\n",vars->optopt);
             else fatal_error("Missing argument for option --%s\n",lopts_KeyWords[index].name);
   case '?': if (index==-1) fatal_error("Invalid option -%c\n",vars->optopt);
             else fatal_error("Invalid option --%s\n",vars->optarg);
             break;
   }
   index=-1;
}
Alphabet* alphabet=NULL;
if (alph[0]!='\0') {
	alphabet=load_alphabet(&vec,alph);
	if (alphabet==NULL) {
		fatal_error("Cannot load alphabet file %s\n",alph);
	}
}
if (vars->optind==argc-1 || vars->optind==argc-2) {
   error("Invalid arguments: rerun with --help\n");
   return 1;
}
strcpy(tokens,argv[(vars->optind++)]);
if (output[0]=='\0') {
	get_path(tokens,output);
	strcat(output,"keywords.txt");
}
struct string_hash_ptr* keywords=load_tokens_by_freq(tokens,&vec);
filter_non_letter_keywords(keywords,alphabet);
if (cdic[0]!='\0') {
	load_compound_words(cdic,&vec,keywords);
}

for (;vars->optind!=argc;(vars->optind)++) {
	filter_keywords_with_dic(keywords,argv[vars->optind],&vec,alphabet);
}
merge_case_equivalent_unknown_words(keywords,alphabet);
struct string_hash* forbidden_lemmas=compute_forbidden_lemmas(keywords,code);
remove_keywords_with_forbidden_lemma(keywords,forbidden_lemmas);
free_string_hash(forbidden_lemmas);
vector_ptr* sorted=sort_keywords(keywords);
U_FILE* f_output=u_fopen(&vec,output,U_WRITE);
if (f_output==NULL) {
	fatal_error("Cannot write in file %s\n",output);
}
dump_keywords(sorted,f_output);
u_fclose(f_output);
free_string_hash_ptr(keywords,(void(*)(void*))free_KeyWord_list);
free_vector_ptr(sorted,(void(*)(void*))free_KeyWord_list);
free(code);
free_alphabet(alphabet);
free_OptVars(vars);
return 0;
}

} // namespace unitex
