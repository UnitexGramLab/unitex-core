/*
 * Unitex
 *
 * Copyright (C) 2001-2011 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "Copyright.h"
#include "LinearAutomaton2Txt.h"
#include "Error.h"
#include "UnitexGetOpt.h"
#include "Tfst2Unambig.h"



const char* usage_Tfst2Unambig =
         "Usage: Tfst2Unambig  [OPTIONS] <tfst>\n"
         "\n"
         "  <tfst>: .tfst file representing the text automaton\n"
         "\n"
         "OPTIONS:\n"
         "  -o TXT/--out=TXT : output unicode text file\n"
         "  -h/--help: this help\n"
         "\n"
         "Converts a linear Unitex text automaton into a text file. If\n"
         "the automaton is not linear, the process is aborted.\n";


static void usage() {
u_printf("%S",COPYRIGHT);
u_printf(usage_Tfst2Unambig);
}


const char* optstring_Tfst2Unambig=":o:hk:q:";

const struct option_TS lopts_Tfst2Unambig[]= {
      {"out",required_argument_TS,NULL,'o'},
      {"input_encoding",required_argument_TS,NULL,'k'},
      {"output_encoding",required_argument_TS,NULL,'q'},
      {"help",no_argument_TS,NULL,'h'},
      {NULL,no_argument_TS,NULL,0}
};


int main_Tfst2Unambig(int argc,char* const argv[]) {
if (argc==1) {
   usage();
   return 0;
}

VersatileEncodingConfig vec={DEFAULT_MASK_ENCODING_COMPATIBILITY_INPUT,DEFAULT_ENCODING_OUTPUT,DEFAULT_BOM_OUTPUT};
int val,index=-1;
struct OptVars* vars=new_OptVars();
char* output=NULL;
while (EOF!=(val=getopt_long_TS(argc,argv,optstring_Tfst2Unambig,lopts_Tfst2Unambig,&index,vars))) {
   switch(val) {
   case 'o': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty output text file name\n");
             }
             output=strdup(vars->optarg);
             if (output==NULL) {
                fatal_alloc_error("main_Tfst2Unambig");
             }
             break;
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
   case 'h': usage(); return 0;
   case ':': if (index==-1) fatal_error("Missing argument for option -%c\n",vars->optopt);
             else fatal_error("Missing argument for option --%s\n",lopts_Tfst2Unambig[index].name);
   case '?': if (index==-1) fatal_error("Invalid option -%c\n",vars->optopt);
             else fatal_error("Invalid option --%s\n",vars->optarg);
             break;
   }
   index=-1;
}

if (vars->optind!=argc-1) {
   fatal_error("Invalid arguments: rerun with --help\n");
}

if (output==NULL) {
   fatal_error("You must specify an output text file\n");
}

u_printf("Loading text automaton...\n");
Tfst* tfst=open_text_automaton(&vec,argv[vars->optind]);
if (tfst==NULL) {
   error("Cannot load text automaton %s\n",argv[vars->optind]);
   free(output);
   free_OptVars(vars);
   return 1;
}
int res=isLinearAutomaton(tfst);
if (res!=LINEAR_AUTOMATON) {
   error("Error: the text automaton is not linear in sentence %d\n",res);
   close_text_automaton(tfst);
   free(output);
   free_OptVars(vars);
   return 1;
}
U_FILE* f=u_fopen(&vec,output,U_WRITE);
if (f==NULL) {
   error("Cannot create %s\n",output);
   close_text_automaton(tfst);
   free(output);
   free_OptVars(vars);
   return 1;
}
u_printf("Converting linear automaton into text...\n");
convertLinearAutomaton(tfst,f);
u_fclose(f);
close_text_automaton(tfst);
free(output);
free_OptVars(vars);
u_printf("Done.\n");
return 0;
}
