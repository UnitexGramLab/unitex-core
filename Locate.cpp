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
#include <time.h>

#include "Unicode.h"
#include "Alphabet.h"
#include "DELA.h"
#include "String_hash.h"
#include "LocatePattern.h"
#include "Fst2.h"
#include "Text_tokens.h"
#include "List_int.h"
#include "CompoundWordTree.h"
#include "Text_parsing.h"
#include "LocateMatches.h"
#include "TransductionVariables.h"
#include "TransductionStack.h"
#include "ParsingInfo.h"
#include "File.h"
#include "Copyright.h"
#include "Locate.h"
#include "Error.h"
#include "UnitexGetOpt.h"
#include "ProgramInvoker.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {


#define STRINGIZE2(s) #s
#define STRINGIZE(s) STRINGIZE2(s)

const char* usage_Locate =
         "Usage: Locate [OPTIONS] <fst2>\n"
         "\n"
         "  <fst2>: the grammar to be applied\n"
         "\n"
         "OPTIONS:\n"
         "  -t TXT/--text=TXT: the .snt text file\n"
         "  -a ALPH/--alphabet=ALPH: the language alphabet file\n"
         "  -m DIC/--morpho=DIC: specifies that DIC is a .bin dictionary\n"
         "                       to use in morphological mode. Use as many\n"
         "                       -m XXX as there are .bin to use. You can also\n"
         "                       separate several .bin with semi-colons.\n"
         "  -s/--start_on_space: enables morphological use of space\n"
         "  -x/--dont_start_on_space: disables morphological use of space (default)\n"
         "  -c/--char_by_char: uses char by char tokenization; useful for languages like Thai\n"
         "  -w/--word_by_word: uses word by word tokenization (default)\n"
         "  -d X/--sntdir=X: uses directory X instead of the text directory; note that X must be\n"
         "                   (back)slash terminated\n"
         "  -K/--korean: tells Locate that it works on Korean\n"
         "  -u X/--arabic_rules=X: Arabic typographic rule configuration file\n"
         "  -g minus/--negation_operator=minus: uses minus as negation operator for Unitex 2.0 graphs\n"
         "  -g tilde/--negation_operator=tilde: uses tilde as negation operator (default)\n"
         "\n"
         "Search limit options:\n"
         "  -l/--all: looks for all matches (default)\n"
         "  -n N/--number_of_matches=N: stops after the first N matches\n"
         "\n"
         "Maximum iterations per token options:\n"
         "  -o N/--stop_token_count=N : stops after N iterations on a token\n"
         "  -o N,M/--stop_token_count=N,M : emits a warning after N iterations on\n"
         "                                    a token and stops after M iterations\n"
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
         "  -p/--protect_dic_chars: when -M or -R mode is used, -p protects some input characters\n"
         "                          with a backslash. This is useful when Locate is called by Dico\n"
         "                          in order to avoid producing bad lines like \"3,14,.PI.NUM\"\n"
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
         "  -v X=Y/--variable=X=Y: sets an output variable named X with content Y. Note that\n"
         "                         Y must be ASCII\n"
         "\n"
         "  --stack_max=N: set max exploration step to save stack (default: " STRINGIZE(STACK_MAX) ")\n"
         "  --max_errors=N: set max number of error to display before exit (default: " STRINGIZE(MAX_ERRORS) ")\n"
         "  --max_matches_per_subgraph=N: set max matches per subgraph (default: " STRINGIZE(MAX_MATCHES_PER_SUBGRAPH) ")\n"
         "  --max_matches_at_token_pos=N: set max matches per token (default: " STRINGIZE(MAX_MATCHES_AT_TOKEN_POS) ")\n"
         "  --less_tolerant: set max matches per subgraph, max matches per token and\n"
         "                         max exploration step at half of default value.\n"
         "  --lesser_tolerant: set max matches per subgraph, max matches per token and\n"
         "                         max exploration step at a fifth of default value.\n"
         "  --least_tolerant: set max matches per subgraph, max matches per token and\n"
         "                         max exploration step at a tenth of default value.\n"
         "\n"
         "  -V/--only-verify-arguments: only verify arguments syntax and exit\n"
         "  -h/--help: this help\n"
         "\n"
         "Applies a grammar to a text, and saves the matching sequence index in a\n"
         "file named \"concord.ind\" stored in the text directory. A result info file\n"
         "named \"concord.n\" is also saved in the same directory.\n";


static void usage() {
  display_copyright_notice();
  u_printf(usage_Locate);
#ifndef REGEX_FACADE_ENGINE
  error("\nWARNING: on this system, morphological filters will not be taken into account,\n");
  error("         because wide characters are not supported\n");
#endif
}

const char* optstring_Locate=":t:a:m:SLAIMRXYZln:d:cewsxbzpKVhk:q:o:u:g:Tv:$:@:C:P:HQN+:";
const struct option_TS lopts_Locate[]= {
  {"text",required_argument_TS,NULL,'t'},
  {"alphabet",required_argument_TS,NULL,'a'},
  {"morpho",required_argument_TS,NULL,'m'},
  {"shortest_matches",no_argument_TS,NULL,'S'},
  {"longest_matches",no_argument_TS,NULL,'L'},
  {"all_matches",no_argument_TS,NULL,'A'},
  {"ignore",no_argument_TS,NULL,'I'},
  {"merge",no_argument_TS,NULL,'M'},
  {"replace",no_argument_TS,NULL,'R'},
  {"exit_on_variable_error",no_argument_TS,NULL,'X'},
  {"ignore_variable_errors",no_argument_TS,NULL,'Y'},
  {"backtrack_on_variable_errors",no_argument_TS,NULL,'Z'},
  {"all",no_argument_TS,NULL,'l'},
  {"number_of_matches",required_argument_TS,NULL,'n'},
  {"sntdir",required_argument_TS,NULL,'d'},
  {"char_by_char",no_argument_TS,NULL,'c'},
  {"word_by_word",no_argument_TS,NULL,'w'},
  {"start_on_space",no_argument_TS,NULL,'s'},
  {"dont_start_on_space",no_argument_TS,NULL,'x'},
  {"ambiguous_outputs",no_argument_TS,NULL,'b'},
  {"no_ambiguous_outputs",no_argument_TS,NULL,'z'},
  {"protect_dic_chars",no_argument_TS,NULL,'p'},
  {"korean",no_argument_TS,NULL,'K'},
  {"stop_token_count",required_argument_TS,NULL,'o'},
  {"input_encoding",required_argument_TS,NULL,'k'},
  {"output_encoding",required_argument_TS,NULL,'q'},
  {"arabic_rules",required_argument_TS,NULL,'u'},
  {"negation_operator",required_argument_TS,NULL,'g'},
  {"dont_use_locate_cache",no_argument_TS,NULL,'e'},
  {"dont_allow_trace",no_argument_TS,NULL,'T'},
  {"variable",required_argument_TS,NULL,'v'},
  {"stack_max",required_argument_TS,NULL,'$'},
  {"max_errors",required_argument_TS,NULL,'@'},
  {"max_matches_per_subgraph",required_argument_TS,NULL,'C'},
  {"max_matches_at_token_pos",required_argument_TS,NULL,'P'},
  {"less_tolerant",no_argument_TS,NULL,'H'},
  {"lesser_tolerant",no_argument_TS,NULL,'Q'},
  {"least_tolerant",no_argument_TS,NULL,'N'},
  {"trace_option",required_argument_TS,NULL,'+'},
  {"only_verify_arguments",no_argument_TS,NULL,'V'},
  {"help",no_argument_TS,NULL,'h'},
  {NULL,no_argument_TS,NULL,0}
};

char** new_locate_trace_param()
{
  char** empty_list_param_trace = (char**)malloc(sizeof(char*));
  if (empty_list_param_trace == NULL) {
    alloc_error("new_locate_trace_param");
    return NULL;
  }
  *empty_list_param_trace = NULL;
  return empty_list_param_trace;
}

char** add_locate_trace_param(char** list_param_trace, const char* add_param)
{
  size_t i = 0;
  while ((*(list_param_trace + i)) != NULL)
    i++;

  char** new_list_param_trace = (char**)realloc(list_param_trace,sizeof(char*)*(i+2));
  if (new_list_param_trace == NULL) {
    alloc_error("add_locate_trace_param");
    return NULL;
  }

  *(new_list_param_trace + i) = strdup(add_param);
  if ((*(new_list_param_trace + i)) == NULL) {
    alloc_error("add_locate_trace_param");
    return NULL;
  }

  *(new_list_param_trace + i + 1) = NULL;

  return new_list_param_trace;
}

void free_locate_trace_param(char** list_param_trace)
{
  size_t i = 0;
  while ((*(list_param_trace + i)) != NULL)
  {
    free(*(list_param_trace + i));
    i++;
  }
  free(list_param_trace);
}


/*
 * This function behaves in the same way that a main one, except that it does
 * not invoke the setBufferMode function.
 */
int main_Locate(int argc,char* const argv[]) {
if (argc==1) {
   usage();
   return SUCCESS_RETURN_CODE;
}

int val,index=-1;
char alph[FILENAME_MAX]="";
char text[FILENAME_MAX]="";
char dynamicSntDir[FILENAME_MAX]="";
char arabic_rules[FILENAME_MAX]="";
char* morpho_dic=NULL;
MatchPolicy match_policy=LONGEST_MATCHES;
OutputPolicy output_policy=IGNORE_OUTPUTS;
int search_limit=NO_MATCH_LIMIT;
TokenizationPolicy tokenization_policy=WORD_BY_WORD_TOKENIZATION;
SpacePolicy space_policy=DONT_START_WITH_SPACE;
AmbiguousOutputPolicy ambiguous_output_policy=ALLOW_AMBIGUOUS_OUTPUTS;
VariableErrorPolicy variable_error_policy=IGNORE_VARIABLE_ERRORS;
int protect_dic_chars=0;
int is_korean=0;
int max_count_call=MAX_COUNT_CALL;
int max_count_call_warning=MAX_COUNT_CALL_WARNING;
int stack_max=STACK_MAX;
int max_matches_at_token_pos=MAX_MATCHES_AT_TOKEN_POS;
int max_matches_per_subgraph=MAX_MATCHES_PER_SUBGRAPH;
int tolerance_divide_factor=1;
int max_errors=0;
int tilde_negation_operator=1;
int useLocateCache=1;
int selected_negation_operator=0;
int allow_trace=1;
char** list_param_trace=new_locate_trace_param();
char foo;
vector_ptr* injected_vars=new_vector_ptr();
VersatileEncodingConfig vec=VEC_DEFAULT;
bool only_verify_arguments = false;
UnitexGetOpt options;
while (EOF!=(val=options.parse_long(argc,argv,optstring_Locate,lopts_Locate,&index))) {
   switch(val) {
   case 't': if (options.vars()->optarg[0]=='\0') {
                error("You must specify a non empty text file name\n");
                free_vector_ptr(injected_vars,free);
                free_locate_trace_param(list_param_trace);
                free(morpho_dic);
                return USAGE_ERROR_CODE;
             }
             strcpy(text,options.vars()->optarg);
             break;
   case 'a': if (options.vars()->optarg[0]=='\0') {
                error("You must specify a non empty alphabet name\n");
                free_vector_ptr(injected_vars,free);
                free_locate_trace_param(list_param_trace);
                free(morpho_dic);
                return USAGE_ERROR_CODE;
             }
             strcpy(alph,options.vars()->optarg);
             break;
   case 'm': if (options.vars()->optarg[0]!='\0') {
                if (morpho_dic==NULL) {
                  morpho_dic=strdup(options.vars()->optarg);
                  if (morpho_dic==NULL) {
                    alloc_error("main_Locate");
                    free_vector_ptr(injected_vars,free);
                    free_locate_trace_param(list_param_trace);
                    free(morpho_dic);
                    return ALLOC_ERROR_CODE;
                  }
                }
                else
                {
                    morpho_dic = (char*)realloc((void*)morpho_dic,strlen(morpho_dic)+strlen(options.vars()->optarg)+2);
                    if (morpho_dic==NULL) {
                      alloc_error("main_Locate");
                      free_vector_ptr(injected_vars,free);
                      free_locate_trace_param(list_param_trace);
                      free(morpho_dic);
                      return ALLOC_ERROR_CODE;
                    }
                    strcat(morpho_dic,";");
                    strcat(morpho_dic,options.vars()->optarg);
                }
             }
             break;
   case 'g': if (options.vars()->optarg[0]=='\0') {
                error("You must specify an argument for negation operator\n");
                free_vector_ptr(injected_vars,free);
                free_locate_trace_param(list_param_trace);
                free(morpho_dic);
                return USAGE_ERROR_CODE;
             }
             selected_negation_operator=1;
             if ((strcmp(options.vars()->optarg,"minus")==0) || (strcmp(options.vars()->optarg,"-")==0)) {
                 tilde_negation_operator=0;
             }
             else
             if ((strcmp(options.vars()->optarg,"tilde")!=0) && (strcmp(options.vars()->optarg,"~")!=0)) {
                 error("You must specify a valid argument for negation operator\n");
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
   case 'l': search_limit=NO_MATCH_LIMIT; break;
   case 'e': useLocateCache=0; break;
   case 'T': allow_trace=0; break;
   case 'n': if (1!=sscanf(options.vars()->optarg,"%d%c",&search_limit,&foo) || search_limit<=0) {
                /* foo is used to check that the search limit is not like "45gjh" */
                error("Invalid search limit argument: %s\n",options.vars()->optarg);
                free_vector_ptr(injected_vars,free);
                free_locate_trace_param(list_param_trace);
                free(morpho_dic);
                return USAGE_ERROR_CODE;
             }
             break;
   case 'o': {
                int param1 = 0;
                int param2 = 0;
                int ret_scan = sscanf(options.vars()->optarg,"%d,%d%c",&param1,&param2,&foo);
                if (ret_scan == 2) {
                    max_count_call_warning = param1;
                    max_count_call = param2;
                    if (((max_count_call < -1)) || (max_count_call_warning < -1)) {
                        /* foo is used to check that the search limit is not like "45gjh" */
                        error("Invalid stop count argument: %s\n",options.vars()->optarg);
                        free_vector_ptr(injected_vars,free);
                        free_locate_trace_param(list_param_trace);
                        free(morpho_dic);
                        return USAGE_ERROR_CODE;
                    }
                }
                else
                    if (1!=sscanf(options.vars()->optarg,"%d%c",&max_count_call,&foo) || (max_count_call < -1)) {
                        /* foo is used to check that the search limit is not like "45gjh" */
                        error("Invalid stop count argument: %s\n",options.vars()->optarg);
                        free_vector_ptr(injected_vars,free);
                        free_locate_trace_param(list_param_trace);
                        free(morpho_dic);
                        return USAGE_ERROR_CODE;
                    }
             }
             break;
   case '$': if (1!=sscanf(options.vars()->optarg,"%d%c",&stack_max,&foo) || stack_max<=0) {
                /* foo is used to check that the param is not like "45gjh" */
                error("Invalid argument: %s\n",options.vars()->optarg);
                free_vector_ptr(injected_vars,free);
                free_locate_trace_param(list_param_trace);
                free(morpho_dic);
                return USAGE_ERROR_CODE;
             }
             break;
   case '@': if (1!=sscanf(options.vars()->optarg,"%d%c",&max_errors,&foo) || max_errors<=0) {
                /* foo is used to check that the param is not like "45gjh" */
                error("Invalid argument: %s\n",options.vars()->optarg);
                free_vector_ptr(injected_vars,free);
                free_locate_trace_param(list_param_trace);
                free(morpho_dic);
                return USAGE_ERROR_CODE;
             }
             break;
   case 'C': if (1!=sscanf(options.vars()->optarg,"%d%c",&max_matches_per_subgraph,&foo) || max_matches_per_subgraph<=0) {
                /* foo is used to check that the param is not like "45gjh" */
                error("Invalid argument: %s\n",options.vars()->optarg);
                free_vector_ptr(injected_vars,free);
                free_locate_trace_param(list_param_trace);
                free(morpho_dic);
                return USAGE_ERROR_CODE;
             }
             break;
   case 'P': if (1!=sscanf(options.vars()->optarg,"%d%c",&max_matches_at_token_pos,&foo) || max_matches_at_token_pos<=0) {
                /* foo is used to check that the param is not like "45gjh" */
                error("Invalid argument: %s\n",options.vars()->optarg);
                free_vector_ptr(injected_vars,free);
                free_locate_trace_param(list_param_trace);
                free(morpho_dic);
                return USAGE_ERROR_CODE;
             }
             break;
   case 'H': {
                tolerance_divide_factor=2;
             }
             break;
   case 'Q': {
                tolerance_divide_factor=5;
             }
             break;
   case 'N': {
                tolerance_divide_factor=10;
             }
             break;
   case 'd': if (options.vars()->optarg[0]=='\0') {
                error("You must specify a non empty snt dir name\n");
                free_vector_ptr(injected_vars,free);
                free_locate_trace_param(list_param_trace);
                free(morpho_dic);
                return USAGE_ERROR_CODE;
             }
             strcpy(dynamicSntDir,options.vars()->optarg);
             break;
   case 'c': tokenization_policy=CHAR_BY_CHAR_TOKENIZATION; break;
   case 'w': tokenization_policy=WORD_BY_WORD_TOKENIZATION; break;
   case 's': space_policy=START_WITH_SPACE; break;
   case 'x': space_policy=DONT_START_WITH_SPACE; break;
   case 'b': ambiguous_output_policy=ALLOW_AMBIGUOUS_OUTPUTS; break;
   case 'z': ambiguous_output_policy=IGNORE_AMBIGUOUS_OUTPUTS; break;
   case 'p': protect_dic_chars=1; break;
   case 'K': is_korean=1;
             break;
   case 'V': only_verify_arguments = true;
             break;
   case 'h': usage();
             free_vector_ptr(injected_vars,free);
             free_locate_trace_param(list_param_trace);
             free(morpho_dic);
             return SUCCESS_RETURN_CODE;
   case 'k': if (options.vars()->optarg[0]=='\0') {
                error("Empty input_encoding argument\n");
                free_vector_ptr(injected_vars,free);
                free_locate_trace_param(list_param_trace);
                free(morpho_dic);
                return USAGE_ERROR_CODE;
             }
             decode_reading_encoding_parameter(&(vec.mask_encoding_compatibility_input),options.vars()->optarg);
             break;
   case 'q': if (options.vars()->optarg[0]=='\0') {
                error("Empty output_encoding argument\n");
                free_vector_ptr(injected_vars,free);
                free_locate_trace_param(list_param_trace);
                free(morpho_dic);
                return USAGE_ERROR_CODE;
             }
             decode_writing_encoding_parameter(&(vec.encoding_output),&(vec.bom_output),options.vars()->optarg);
             break;
   case 'u': if (options.vars()->optarg[0]=='\0') {
                error("You must specify a non empty arabic rule configuration file name\n");
                free_vector_ptr(injected_vars,free);
                free_locate_trace_param(list_param_trace);
                free(morpho_dic);
                return USAGE_ERROR_CODE;
             }
             strcpy(arabic_rules,options.vars()->optarg);
             break;
   case '+': if (options.vars()->optarg[0]=='\0') {
                error("You must specify a non empty trace option\n");
                free_vector_ptr(injected_vars,free);
                free_locate_trace_param(list_param_trace);
                free(morpho_dic);
                return USAGE_ERROR_CODE;
             }
             list_param_trace=add_locate_trace_param(list_param_trace,options.vars()->optarg);
             break;
   case 'v': {
     unichar* key=u_strdup(options.vars()->optarg);
     unichar* value=u_strchr(key,'=');
     if (value==NULL) {
       error("Invalid variable injection: %s\n",options.vars()->optarg);
       free_vector_ptr(injected_vars,free);
       free_locate_trace_param(list_param_trace);
       free(morpho_dic);
       return USAGE_ERROR_CODE;
     }
     (*value)='\0';
     value++;
     value=u_strdup(value);
     vector_ptr_add(injected_vars,key);
     vector_ptr_add(injected_vars,value);
     break;
   }
   case ':': index==-1 ? error("Missing argument for option -%c\n",options.vars()->optopt) :
                         error("Missing argument for option --%s\n",lopts_Locate[index].name);
             free_vector_ptr(injected_vars,free);
             free_locate_trace_param(list_param_trace);
             free(morpho_dic);
             return USAGE_ERROR_CODE;
   case '?': index==-1 ? error("Invalid option -%c\n",options.vars()->optopt) :
                         error("Invalid option --%s\n",options.vars()->optarg);
             free_vector_ptr(injected_vars,free);
             free_locate_trace_param(list_param_trace);
             free(morpho_dic);
             return USAGE_ERROR_CODE;
   }
   index=-1;
}

stack_max /= tolerance_divide_factor;
max_matches_at_token_pos /= tolerance_divide_factor;
max_matches_per_subgraph /= tolerance_divide_factor;

if (options.vars()->optind!=argc-1) {
  error("Invalid arguments: rerun with --help\n");
  free_vector_ptr(injected_vars,free);
  free_locate_trace_param(list_param_trace);
  free(morpho_dic);
  return USAGE_ERROR_CODE;
}

if (text[0]=='\0') {
  error("You must specify a .snt text file\n");
  free_vector_ptr(injected_vars,free);
  free_locate_trace_param(list_param_trace);
  free(morpho_dic);
  return USAGE_ERROR_CODE;
}

if (only_verify_arguments) {
  // freeing all allocated memory
  free_vector_ptr(injected_vars,free);
  free_locate_trace_param(list_param_trace);
  free(morpho_dic);
  return SUCCESS_RETURN_CODE;
}

if (selected_negation_operator==0) {
  get_graph_compatibility_mode_by_file(&vec,&tilde_negation_operator);
}

size_t step_filename_buffer = (((FILENAME_MAX / 0x10) + 1) * 0x10);

char* buffer_filename = (char*)malloc(step_filename_buffer * 6);
if (buffer_filename == NULL) {
  alloc_error("main_Locate");
  free_vector_ptr(injected_vars,free);
  free_locate_trace_param(list_param_trace);
  free(morpho_dic);
  return ALLOC_ERROR_CODE;
}

char* staticSntDir = (buffer_filename + (step_filename_buffer * 0));
char* tokens_txt = (buffer_filename + (step_filename_buffer * 1));
char* text_cod = (buffer_filename + (step_filename_buffer * 2));
char* dlf = (buffer_filename + (step_filename_buffer * 3));
char* dlc = (buffer_filename + (step_filename_buffer * 4));
char* err = (buffer_filename + (step_filename_buffer * 5));

get_snt_path(text,staticSntDir);
if (dynamicSntDir[0]=='\0') {
   strcpy(dynamicSntDir,staticSntDir);
}

strcpy(tokens_txt,staticSntDir);
strcat(tokens_txt,"tokens.txt");

strcpy(text_cod,staticSntDir);
strcat(text_cod,"text.cod");

strcpy(dlf,staticSntDir);
strcat(dlf,"dlf");

strcpy(dlc,staticSntDir);
strcat(dlc,"dlc");

strcpy(err,staticSntDir);
strcat(err,"err");

int OK=locate_pattern(text_cod,
               tokens_txt,
               argv[options.vars()->optind],
               dlf,
               dlc,
               err,
               alph,
               match_policy,
               output_policy,
               &vec,
               dynamicSntDir,
               tokenization_policy,
               space_policy,
               search_limit,
               morpho_dic,
               ambiguous_output_policy,
               variable_error_policy,
               protect_dic_chars,
               is_korean,
               max_count_call,
               max_count_call_warning,
               stack_max,
               max_matches_at_token_pos,
               max_matches_per_subgraph,
               max_errors,
               arabic_rules,
               tilde_negation_operator,
               useLocateCache,
               allow_trace,
               list_param_trace,
               injected_vars);

free(buffer_filename);
free_vector_ptr(injected_vars,free);
free_locate_trace_param(list_param_trace);
free(morpho_dic);

return (!OK);
}


/**
 * Launches the Locate main function with the appropriate arguments.
 * This function is used to apply a .fst2 as dictionary in the Dico
 * program.
 *
 * @author Alexis Neme
 * Modified by Sébastien Paumier
 */
int launch_locate_as_routine(const VersatileEncodingConfig* vec,
                             const char* text_snt,const char* fst2,const char* alphabet,
                             OutputPolicy output_policy,MatchPolicy match_policy,const char* morpho_dic,
                             int protect_dic_chars,int is_korean,const char* arabic_rules,
                             const char* negation_operator,
                             int n_matches_max) {
/* We test if we are working on Thai, on the basis of the alphabet file */
char path[FILENAME_MAX];
char lang[FILENAME_MAX];
get_path(alphabet,path);
path[strlen(path)-1]='\0';
remove_path(path,lang);
int thai=0;
if (!strcmp(lang,"Thai")) {
   thai=1;
}
int md=0;
if (morpho_dic!=NULL) {
   md=1;
}
ProgramInvoker* invoker=new_ProgramInvoker(main_Locate,"main_Locate");
char tmp[FILENAME_MAX];
{
    tmp[0]=0;
    get_reading_encoding_text(tmp,sizeof(tmp)-1,vec->mask_encoding_compatibility_input);
    if (tmp[0] != '\0') {
        add_argument(invoker,"-k");
        add_argument(invoker,tmp);
    }

    tmp[0]=0;
    get_writing_encoding_text(tmp,sizeof(tmp)-1,vec->encoding_output,vec->bom_output);
    if (tmp[0] != '\0') {
        add_argument(invoker,"-q");
        add_argument(invoker,tmp);
    }
}
if (negation_operator != NULL) {
    if ((*negation_operator) != 0) {
        char negation_operator_argument[0x40];
        sprintf(negation_operator_argument,"--negation_operator=%s",negation_operator);
        add_argument(invoker,negation_operator_argument);
    }
}
/* If needed: just to know that the call come from here if necessary */
sprintf(tmp,"--text=%s",text_snt);
add_argument(invoker,tmp);
if (alphabet!=NULL && alphabet[0]!='\0') {
  sprintf(tmp,"-a%s",alphabet);
  add_argument(invoker,tmp);
}
/* We set the match policy */
switch(match_policy) {
case ALL_MATCHES: add_argument(invoker,"-A"); break;
case SHORTEST_MATCHES: add_argument(invoker,"-S"); break;
case LONGEST_MATCHES: add_argument(invoker,"-L"); break;
}
/* We set the output policy */
switch (output_policy) {
   case MERGE_OUTPUTS: add_argument(invoker,"-M"); break;
   case REPLACE_OUTPUTS: add_argument(invoker,"-R"); break;
   default: add_argument(invoker,"-I"); break;
}
/* We look for all the occurrences */
if (n_matches_max==-1) {
  add_argument(invoker,"--all");
} else {
  sprintf(tmp,"-n%d",n_matches_max);
  add_argument(invoker,tmp);
}
/* If needed, we add the -thai option */
if (thai) {
   add_argument(invoker,"--thai");
}
if (md) {
  add_argument(invoker,"-m");
  add_argument(invoker,morpho_dic);
}
if (protect_dic_chars) {
  add_argument(invoker,"-p");
}
if (is_korean) {
  add_argument(invoker,"-K");
}
if (arabic_rules && arabic_rules[0]!='\0') {
   sprintf(tmp,"--arabic_rules=%s",arabic_rules);
   add_argument(invoker,tmp);
}
add_argument(invoker,fst2);
/* Finally, we call the main function of Locate */
int ret=invoke(invoker);
free_ProgramInvoker(invoker);
return ret;
}

} // namespace unitex
