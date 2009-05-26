 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
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
#include "getopt.h"
#include "LocateTfst_lib.h"
#include "File.h"
#include "LocateConstants.h"


static void usage() {
u_printf("%S",COPYRIGHT);
u_printf("Usage: LocateTfst [OPTIONS] <fst2>\n"
         "\n"
         "  <fst2>: the grammar to be applied\n"
         "\n"
         "OPTIONS:\n"
         "  -t TFST/--text=TFST: the .tfst text automaton\n"
         "  -a ALPH/--alphabet=ALPH: the language alphabet file\n"
         "  -j JAMOTABLE/--jamo=JAMOTABLE: the Jamo table conversion to use\n"
         "                                 when we work on a Korean .tfst\n"
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
         "  -h/--help: this help\n"
         "\n"
         "Applies a grammar to a text automaton, and saves the matching sequence index in a\n"
         "file named 'concord.ind', just as Locate does.\n");
}



/*
 * This function behaves in the same way that a main one, except that it does
 * not invoke the setBufferMode function.
 */
int main_LocateTfst(int argc,char* argv[]) {
if (argc==1) {
   usage();
   return 0;
}

const char* optstring=":t:a:j:ln:SLAIMRXYZbzh";
const struct option_TS lopts[]= {
	  {"text",required_argument_TS,NULL,'t'},
	  {"alphabet",required_argument_TS,NULL,'a'},
	  {"jamo",required_argument_TS,NULL,'j'},
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
     {"help",no_argument_TS,NULL,'h'},
     {NULL,no_argument_TS,NULL,0}
};
int val,index=-1;
char text[FILENAME_MAX]="";
char alphabet[FILENAME_MAX]="";
char jamo_table[FILENAME_MAX]="";
MatchPolicy match_policy=LONGEST_MATCHES;
OutputPolicy output_policy=IGNORE_OUTPUTS;
AmbiguousOutputPolicy ambiguous_output_policy=ALLOW_AMBIGUOUS_OUTPUTS;
VariableErrorPolicy variable_error_policy=IGNORE_VARIABLE_ERRORS;
int search_limit=NO_MATCH_LIMIT;
struct OptVars* vars=new_OptVars();
char foo;
while (EOF!=(val=getopt_long_TS(argc,argv,optstring,lopts,&index,vars))) {
   switch(val) {
   case 't': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty .tfst name\n");
             }
             strcpy(text,vars->optarg);
             break;
   case 'a': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty alphabet name\n");
             }
             strcpy(alphabet,vars->optarg);
             break;
   case 'j': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty jamo table name\n");
             }
             strcpy(jamo_table,vars->optarg);
             break;
   case 'l': search_limit=NO_MATCH_LIMIT; break;
   case 'n': if (1!=sscanf(vars->optarg,"%d%c",&search_limit,&foo) || search_limit<=0) {
                /* foo is used to check that the search limit is not like "45gjh" */
                fatal_error("Invalid search limit argument: %s\n",vars->optarg);
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
   case 'h': usage(); return 0;
   case ':': if (index==-1) fatal_error("Missing argument for option -%c\n",vars->optopt);
             else fatal_error("Missing argument for option --%s\n",lopts[index].name);
   case '?': if (index==-1) fatal_error("Invalid option -%c\n",vars->optopt);
             else fatal_error("Invalid option --%s\n",vars->optarg);
             break;
   }
   index=-1;
}

if (alphabet[0]=='\0') {
   fatal_error("You must specify an alphabet file\n");
}
char grammar[FILENAME_MAX];
char output[FILENAME_MAX];
if (vars->optind!=argc-1) {
   fatal_error("Invalid arguments: rerun with --help\n");
}
strcpy(grammar,argv[vars->optind]);
get_path(text,output);
strcat(output,"concord.ind");

int OK=locate_tfst(text,grammar,alphabet,output,match_policy,output_policy,
                   ambiguous_output_policy,variable_error_policy,search_limit,jamo_table);

free_OptVars(vars);
return (!OK);
}
