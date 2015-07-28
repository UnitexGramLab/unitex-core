/*
 * Unitex
 *
 * Copyright (C) 2001-2015 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "Error.h"
#include "UnitexGetOpt.h"
#include "LocateTfst_lib.h"
#include "File.h"
#include "LocateConstants.h"
#include "LocateTfst.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

const char* usage_LocateTfst =
  "Usage: LocateTfst [OPTIONS] <fst2>\n"
  "\n"
  "  <fst2>: the grammar to be applied\n"
  "\n"
  "OPTIONS:\n"
  "  -t TFST/--text=TFST: the .tfst text automaton\n"
  "  -a ALPH/--alphabet=ALPH: the language alphabet file\n"
  "  -K/--korean: tells LocateTfst that it works on Korean\n"
  "  -g minus/--negation_operator=minus: uses minus as negation operator for Unitex 2.0 graphs\n"
  "  -g tilde/--negation_operator=tilde: uses tilde as negation operator (default)\n"
  "  --single_tags_only: skips all results that match more than one text tag\n"
  "  --dont_match_word_boundaries: allows 'air'+'port' in a graph to match 'airport' in the TFST\n"
  "\n"
  "Search limit options:\n"
  "  -l/--all: looks for all matches (default)\n"
  "  -n N/--number_of_matches=N: stops after the first N matches\n"
  "\n"
  "Matching mode options:\n"
  "  -S/--shortest_matches\n"
  "  -L/--longest_matches (default)\n"
  "  -A/--all_matches\n"
  "\n"
  "Output options:\n"
  "  -I/--ignore (default)\n"
  "  -M/--merge\n"
  "  -R/--replace\n"
  "\n"
  "Ambiguous output options:\n"
  "  -b/--ambiguous_outputs: allows the production of several matches with same input\n"
  "                          but different outputs (default)\n"
  "  -z/--no_ambiguous_outputs: forbids ambiguous outputs\n"
  "\n"
  "Variable error options:\n"
  "These options have no effect if the output mode is --ignore; otherwise, they rule\n"
  "the behavior of the Locate program when an output is found that contains a reference\n"
  "to a variable that is not correctly defined.\n"
  "  -X/--exit_on_variable_error: kills the program\n"
  "  -Y/--ignore_variable_errors: acts as if the variable has an empty content (default)\n"
  "  -Z/--backtrack_on_variable_errors: stop exploring the current path of the grammar\n"
  "\n"
  "Variable injection:\n"
  "  -v X=Y/--variable=X=Y: sets an output variable named X with content Y\n"
  "\n"
  "Tagging option:\n"
  "  --tagging: indicates that the concordance must be a tagging one, containing\n"
  "             additional information on the start and end states of each match\n"
  "\n"
  "  -h/--help: this help\n"
  "\n"
  "Applies a grammar to a text automaton, and saves the matching sequence index in a\n"
  "file named 'concord.ind', just as Locate does.\n";


static void usage() {
  display_copyright_notice();
  u_printf(usage_LocateTfst);
}


const char* optstring_LocateTfst=":t:a:Kln:SLAIMRXYZbzVhg:k:q:v:";
const struct option_TS lopts_LocateTfst[]= {
  {"text",required_argument_TS,NULL,'t'},
  {"alphabet",required_argument_TS,NULL,'a'},
  {"korean",no_argument_TS,NULL,'K'},
  {"all",no_argument_TS,NULL,'l'},
  {"number_of_matches",required_argument_TS,NULL,'n'},
  {"shortest_matches",no_argument_TS,NULL,'S'},
  {"longest_matches",no_argument_TS,NULL,'L'},
  {"all_matches",no_argument_TS,NULL,'A'},
  {"ignore",no_argument_TS,NULL,'I'},
  {"merge",no_argument_TS,NULL,'M'},
  {"replace",no_argument_TS,NULL,'R'},
  {"exit_on_variable_error",no_argument_TS,NULL,'X'},
  {"ignore_variable_errors",no_argument_TS,NULL,'Y'},
  {"backtrack_on_variable_errors",no_argument_TS,NULL,'Z'},
  {"ambiguous_outputs",no_argument_TS,NULL,'b'},
  {"no_ambiguous_outputs",no_argument_TS,NULL,'z'},
  {"input_encoding",required_argument_TS,NULL,'k'},
  {"output_encoding",required_argument_TS,NULL,'q'},
  {"only_verify_arguments",no_argument_TS,NULL,'V'},
  {"help",no_argument_TS,NULL,'h'},
  {"negation_operator",required_argument_TS,NULL,'g'},
  {"variable",required_argument_TS,NULL,'v'},
  {"tagging",no_argument_TS,NULL,1},
  {"single_tags_only",no_argument_TS,NULL,2},
  {"dont_match_word_boundaries", no_argument_TS, NULL,3},
  {NULL,no_argument_TS,NULL,0}
};


/*
 * This function behaves in the same way that a main one, except that it does
 * not invoke the setBufferMode function.
 */
int main_LocateTfst(int argc,char* const argv[]) {
if (argc==1) {
   usage();
   return SUCCESS_RETURN_CODE;
}

VersatileEncodingConfig vec=VEC_DEFAULT;
int val,index=-1;
char text[FILENAME_MAX]="";
char alphabet[FILENAME_MAX]="";
int is_korean=0;
int tilde_negation_operator=1;
int selected_negation_operator=0;
int tagging=0;
int single_tags_only=0;
int match_word_boundaries=1;
MatchPolicy match_policy=LONGEST_MATCHES;
OutputPolicy output_policy=IGNORE_OUTPUTS;
AmbiguousOutputPolicy ambiguous_output_policy=ALLOW_AMBIGUOUS_OUTPUTS;
VariableErrorPolicy variable_error_policy=IGNORE_VARIABLE_ERRORS;
int search_limit=NO_MATCH_LIMIT;
char foo;
vector_ptr* injected=new_vector_ptr();
bool only_verify_arguments = false;
UnitexGetOpt options;
while (EOF!=(val=options.parse_long(argc,argv,optstring_LocateTfst,lopts_LocateTfst,&index))) {
   switch(val) {
   case 't': if (options.vars()->optarg[0]=='\0') {
                error("You must specify a non empty .tfst name\n");
                free_vector_ptr(injected);
                return USAGE_ERROR_CODE;
             }
             strcpy(text,options.vars()->optarg);
             break;
   case 'a': if (options.vars()->optarg[0]=='\0') {
                error("You must specify a non empty alphabet name\n");
                free_vector_ptr(injected);
                return USAGE_ERROR_CODE;
             }
             strcpy(alphabet,options.vars()->optarg);
             break;
   case 'K': is_korean=1;
   	   	   	  match_word_boundaries=0;
              break;
   case 'l': search_limit=NO_MATCH_LIMIT; break;
   case 'g': if (options.vars()->optarg[0]=='\0') {
                error("You must specify an argument for negation operator\n");
                free_vector_ptr(injected);
                return USAGE_ERROR_CODE;
             }
             selected_negation_operator=1;
             if ((strcmp(options.vars()->optarg,"minus")==0) || (strcmp(options.vars()->optarg,"-")==0)) {
                 tilde_negation_operator=0;
             }
             else
             if ((strcmp(options.vars()->optarg,"tilde")!=0) && (strcmp(options.vars()->optarg,"~")!=0)) {
                 error("You must specify a valid argument for negation operator\n");
                 free_vector_ptr(injected);
                 return USAGE_ERROR_CODE;                 
             }
             break;
   case 'n': if (1!=sscanf(options.vars()->optarg,"%d%c",&search_limit,&foo) || search_limit<=0) {
                /* foo is used to check that the search limit is not like "45gjh" */
                error("Invalid search limit argument: %s\n",options.vars()->optarg);
                free_vector_ptr(injected);
                return USAGE_ERROR_CODE;                
             }
             break;
   case 'S': match_policy=SHORTEST_MATCHES; break;
   case 'L': match_policy=LONGEST_MATCHES; break;
   case 'A': match_policy=ALL_MATCHES; break;
   case 'I': output_policy=IGNORE_OUTPUTS; break;
   case 'M': output_policy=MERGE_OUTPUTS; break;
   case 'R': output_policy=REPLACE_OUTPUTS; break;
   case 'X': variable_error_policy=EXIT_ON_VARIABLE_ERRORS; break;
   case 'Y': variable_error_policy=IGNORE_VARIABLE_ERRORS; break;
   case 'Z': variable_error_policy=BACKTRACK_ON_VARIABLE_ERRORS; break;
   case 'b': ambiguous_output_policy=ALLOW_AMBIGUOUS_OUTPUTS; break;
   case 'z': ambiguous_output_policy=IGNORE_AMBIGUOUS_OUTPUTS; break;
   case 'V': only_verify_arguments = true;
             break;
   case 'h': usage(); 
             return SUCCESS_RETURN_CODE;
   case 1: tagging=1; break;
   case 2: single_tags_only=1; break;
   case 3: match_word_boundaries=0; break;
   case 'k': if (options.vars()->optarg[0]=='\0') {
                error("Empty input_encoding argument\n");
                free_vector_ptr(injected);
                return USAGE_ERROR_CODE;                
             }
             decode_reading_encoding_parameter(&(vec.mask_encoding_compatibility_input),options.vars()->optarg);
             break;
   case 'q': if (options.vars()->optarg[0]=='\0') {
                error("Empty output_encoding argument\n");
                free_vector_ptr(injected);
                return USAGE_ERROR_CODE;                
             }
             decode_writing_encoding_parameter(&(vec.encoding_output),&(vec.bom_output),options.vars()->optarg);
             break;
   case 'v': {
	   unichar* key=u_strdup(options.vars()->optarg);
	   unichar* value=u_strchr(key,'=');
	   if (value==NULL) {
		   error("Invalid variable injection: %s\n",options.vars()->optarg);
       free_vector_ptr(injected);
       return USAGE_ERROR_CODE;       
	   }
	   (*value)='\0';
	   value++;
	   value=u_strdup(value);
	   vector_ptr_add(injected,key);
	   vector_ptr_add(injected,value);
	   break;
   }
   case ':': index==-1 ? error("Missing argument for option -%c\n",options.vars()->optopt) :
                         error("Missing argument for option --%s\n",lopts_LocateTfst[index].name);
             free_vector_ptr(injected);
             return USAGE_ERROR_CODE;
   case '?': index==-1 ? error("Invalid option -%c\n",options.vars()->optopt) :
                         error("Invalid option --%s\n",options.vars()->optarg);
             free_vector_ptr(injected);
             return USAGE_ERROR_CODE;
             break;
   }
   index=-1;
}

if (options.vars()->optind!=argc-1) {
   error("Invalid arguments: rerun with --help\n");
   free_vector_ptr(injected);
   return USAGE_ERROR_CODE;
}

if (only_verify_arguments) {
  // freeing all allocated memory
  free_vector_ptr(injected);
  return SUCCESS_RETURN_CODE;
}

if (selected_negation_operator==0) {
    get_graph_compatibility_mode_by_file(&vec,&tilde_negation_operator);
}

char grammar[FILENAME_MAX];
char output[FILENAME_MAX];
strcpy(grammar,argv[options.vars()->optind]);
get_path(text,output);
strcat(output,"concord.ind");

int OK=locate_tfst(text,
                   grammar,
                   alphabet,
                   output,
                   &vec,
                   match_policy,
                   output_policy,
                   ambiguous_output_policy,
                   variable_error_policy,
                   search_limit,
                   is_korean,
                   tilde_negation_operator,
                   injected,
                   tagging,
                   single_tags_only,
                   match_word_boundaries);

free_vector_ptr(injected);

return (!OK);
}

} // namespace unitex
