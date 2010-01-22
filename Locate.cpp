 /*
  * Unitex
  *
  * Copyright (C) 2001-2010 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
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
#include "getopt.h"
#include "ProgramInvoker.h"





const char* usage_Locate =
         "Usage: Locate [OPTIONS] <fst2>\n"
         "\n"
         "  <fst2>: the grammar to be applied\n"
         "\n"
         "OPTIONS:\n"
         "  -t TXT/--text=TXT: the .snt text file\n"
         "  -a ALPH/--alphabet=ALPH: the language alphabet file\n"
         "  -m X/--morpho=X: uses X as the .bin dictionary list to use in morphological\n"
         "                   mode. .bin names are supposed to be separated with semi-colons.\n"
         "  -s/--start_on_space: enables morphological use of space\n"
         "  -x/--dont_start_on_space: disables morphological use of space (default)\n"
         "  -c/--char_by_char: uses char by char tokenization; useful for languages like Thai\n"
         "  -w/--word_by_word: uses word by word tokenization (default)\n"
         "  -d X/--sntdir=X: uses directory X instead of the text directory; note that X must be\n"
         "                   (back)slash terminated\n"
         "  -K/--korean: tells Locate that it works on Korean\n"
         "  -f FST2/--fst2=FST2: specifies the jamo->hangul transducer to use for Korean\n"
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
         "  -h/--help: this help\n"
         "\n"
         "Applies a grammar to a text, and saves the matching sequence index in a\n"
         "file named \"concord.ind\" stored in the text directory. A result info file\n"
         "named \"concord.n\" is also saved in the same directory.\n";


static void usage() {
u_printf("%S",COPYRIGHT);
u_printf(usage_Locate);
#ifndef TRE_WCHAR
   error("\nWARNING: on this system, morphological filters will not be taken into account,\n");
   error("         because wide characters are not supported\n");
#endif
}


const char* optstring_Locate=":t:a:m:SLAIMRXYZln:d:cwsxbzpKf:hk:q:";
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
      {"fst2",required_argument_TS,NULL,'f'},
      {"input_encoding",required_argument_TS,NULL,'k'},
      {"output_encoding",required_argument_TS,NULL,'q'},
      {"help",no_argument_TS,NULL,'h'},
      {NULL,no_argument_TS,NULL,0}
};


/*
 * This function behaves in the same way that a main one, except that it does
 * not invoke the setBufferMode function.
 */
int main_Locate(int argc,char* argv[]) {
if (argc==1) {
   usage();
   return 0;
}


int val,index=-1;
char alph[FILENAME_MAX]="";
char text[FILENAME_MAX]="";
char dynamicSntDir[FILENAME_MAX]="";
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
char korean_fst2[FILENAME_MAX]="";
char foo;
Encoding encoding_output = DEFAULT_ENCODING_OUTPUT;
int bom_output = DEFAULT_BOM_OUTPUT;
int mask_encoding_compatibility_input = DEFAULT_MASK_ENCODING_COMPATIBILITY_INPUT;
struct OptVars* vars=new_OptVars();
while (EOF!=(val=getopt_long_TS(argc,argv,optstring_Locate,lopts_Locate,&index,vars))) {
   switch(val) {
   case 't': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty text file name\n");
             }
             strcpy(text,vars->optarg);
             break;
   case 'a': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty alphabet name\n");
             }
             strcpy(alph,vars->optarg);
             break;
   case 'm': if (vars->optarg[0]!='\0') {
                morpho_dic=strdup(vars->optarg);
                if (morpho_dic==NULL) {
                   fatal_alloc_error("main_Locate");
                }
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
   case 'n': if (1!=sscanf(vars->optarg,"%d%c",&search_limit,&foo) || search_limit<=0) {
                /* foo is used to check that the search limit is not like "45gjh" */
                fatal_error("Invalid search limit argument: %s\n",vars->optarg);
             }
             break;
   case 'd': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty snt dir name\n");
             }
             strcpy(dynamicSntDir,vars->optarg);
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
   case 'f': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty transducer file name\n");
             }
             strcpy(korean_fst2,vars->optarg);
             break;
   case 'h': usage(); return 0;
   case 'k': if (vars->optarg[0]=='\0') {
                fatal_error("Empty input_encoding argument\n");
             }
             decode_reading_encoding_parameter(&mask_encoding_compatibility_input,vars->optarg);
             break;
   case 'q': if (vars->optarg[0]=='\0') {
                fatal_error("Empty output_encoding argument\n");
             }
             decode_writing_encoding_parameter(&encoding_output,&bom_output,vars->optarg);
             break;
   case ':': if (index==-1) fatal_error("Missing argument for option -%c\n",vars->optopt);
             else fatal_error("Missing argument for option --%s\n",lopts_Locate[index].name);
   case '?': if (index==-1) fatal_error("Invalid option -%c\n",vars->optopt);
             else fatal_error("Invalid option --%s\n",vars->optarg);
             break;
   }
   index=-1;
}

if (text[0]=='\0') {
   fatal_error("You must specify a .snt text file\n");
}
if (vars->optind!=argc-1) {
   fatal_error("Invalid arguments: rerun with --help\n");
}

char staticSntDir[FILENAME_MAX];
char tokens_txt[FILENAME_MAX];
char text_cod[FILENAME_MAX];
char dlf[FILENAME_MAX];
char dlc[FILENAME_MAX];
char err[FILENAME_MAX];

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

int OK=locate_pattern(text_cod,tokens_txt,argv[vars->optind],dlf,dlc,err,alph,match_policy,output_policy,
               encoding_output,bom_output,mask_encoding_compatibility_input,
               dynamicSntDir,tokenization_policy,space_policy,search_limit,morpho_dic,
               ambiguous_output_policy,variable_error_policy,protect_dic_chars,korean_fst2,is_korean);
if (morpho_dic!=NULL) {
   free(morpho_dic);
}
free_OptVars(vars);
return (!OK);
}


/**
 * Launches the Locate main function with the appropriate arguments.
 * This function is used to apply a .fst2 as dictionary in the Dico
 * program.
 *
 * @author Alexis Neme
 * Modified by S�bastien Paumier
 */
int launch_locate_as_routine(Encoding encoding_output,int bom_output,int mask_encoding_compatibility_input,
                             char* text_snt,char* fst2,char* alphabet,
                              OutputPolicy output_policy,char* morpho_dic,
                              int protect_dic_chars,char* jamo,char* korean_fst2) {
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
    get_reading_encoding_text(tmp,sizeof(tmp)-1,mask_encoding_compatibility_input);
    if (tmp[0] != '\0') {
        add_argument(invoker,"-k");
        add_argument(invoker,tmp);
    }

    tmp[0]=0;
    get_writing_encoding_text(tmp,sizeof(tmp)-1,encoding_output,bom_output);
    if (tmp[0] != '\0') {
        add_argument(invoker,"-q");
        add_argument(invoker,tmp);
    }
}
/* If needed: just to know that the call come from here if necessary */
sprintf(tmp,"--text=%s",text_snt);
add_argument(invoker,tmp);
sprintf(tmp,"-a%s",alphabet);
add_argument(invoker,tmp);
/* We work in longuest match mode */
add_argument(invoker,"-L");
/* We set the output policy */
switch (output_policy) {
   case MERGE_OUTPUTS: add_argument(invoker,"-M"); break;
   case REPLACE_OUTPUTS: add_argument(invoker,"-R"); break;
   default: add_argument(invoker,"-I"); break;
}
/* We look for all the occurrences */
add_argument(invoker,"--all");
/* If needed, we add the -thai option */
if (thai) {
   add_argument(invoker,"--thai");
}
if (md) {
   sprintf(tmp,"--morpho=%s",morpho_dic);
   add_argument(invoker,tmp);
}
if (protect_dic_chars) {
	add_argument(invoker,"-p");
}
if (jamo!=NULL) {
	sprintf(tmp,"-j%s",jamo);
	add_argument(invoker,tmp);
}
if (korean_fst2!=NULL) {
   sprintf(tmp,"-f%s",korean_fst2);
   add_argument(invoker,tmp);
}
add_argument(invoker,fst2);
/* Finally, we call the main function of Locate */
int ret=invoke(invoker);
free_ProgramInvoker(invoker);
return ret;
}
