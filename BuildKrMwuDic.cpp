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
#include "Unicode.h"
#include "Copyright.h"
#include "Error.h"
#include "UnitexGetOpt.h"
#include "BuildKrMwuDic.h"
#include "KrMwuDic.h"
#include "Alphabet.h"
#include "Korean.h"
#include "MF_Global.h"
#include "MF_LangMorpho.h"
#include "File.h"
#include "CompressedDic.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

const char* usage_BuildKrMwuDic =
        "Usage: BuildKrMwuDic [OPTIONS] <dic>\n"
        "\n"
        "  <dic>: text file describing a Korean compound word DELAS\n"
        "\n"
        "OPTIONS:\n"
        "  -o GRF/--output=GRF: output .grf file to produce\n"
        "  -d DIR/--directory=DIR: specifies the directory that contains the inflection graphs\n"
        "                          required to produce morphological variants of roots\n"
        "  -a ALPH/--alphabet=ALPH: specifies the alphabet file to use\n"
        "  -b BIN/--binary=BIN: binary simple word dictionary\n"
        "\n"
        " Graph recompilation options:\n"
        "  -f/--always-recompile-graphs:        forces graph recompiling even if the fst is up to date\n"
        "  -n/--never-recompile-graphs:         avoids graph recompiling even if the fst is not up to date\n"
        "  -t/--only-recompile-outdated-graphs: only recompiling when the fst is not up to date (default)\n"
        "\n"
        "  -V/--only-verify-arguments: only verify arguments syntax and exit\n"
        "  -h/--help: display this help and exit\n"
        "\n";


static void usage() {
  display_copyright_notice();
  u_printf(usage_BuildKrMwuDic);
}

const char* optstring_BuildKrMwuDic="o:d:a:b:Vhfntk:q:";
const struct option_TS lopts_BuildKrMwuDic[]= {
  {"output",required_argument_TS,NULL,'o'},
  {"directory",required_argument_TS,NULL,'d'},
  {"alphabet",required_argument_TS,NULL,'a'},
  {"binary",required_argument_TS,NULL,'b'},      
  {"only_verify_arguments",no_argument_TS,NULL,'V'},
  {"help",no_argument_TS,NULL,'h'},
  {"always-recompile-graphs",no_argument_TS,NULL,'f'},
  {"never-recompile-graphs",no_argument_TS,NULL,'n'},
  {"only-recompile-outdated-graphs",no_argument_TS,NULL,'t'},
  {"input_encoding",required_argument_TS,NULL,'k'},
  {"output_encoding",required_argument_TS,NULL,'q'},
  {NULL,no_argument_TS,NULL,0}
};


/**
 * The same than main, but no call to setBufferMode.
 */
int main_BuildKrMwuDic(int argc,char* const argv[]) {
if (argc==1) {
   usage();
   return SUCCESS_RETURN_CODE;
}

int val,index=-1;
char output[FILENAME_MAX]="";
char inflection_dir[FILENAME_MAX]="";
char alphabet[FILENAME_MAX]="";
char dic_bin[FILENAME_MAX]="";
char dic_inf[FILENAME_MAX]="";

// default policy is to compile only out of date graphs
GraphRecompilationPolicy graph_recompilation_policy = ONLY_OUT_OF_DATE;

VersatileEncodingConfig vec=VEC_DEFAULT;

bool only_verify_arguments = false;

UnitexGetOpt options;

while (EOF!=(val=options.parse_long(argc,argv,optstring_BuildKrMwuDic,lopts_BuildKrMwuDic,&index))) {
   switch(val) {
   case 'o': if (options.vars()->optarg[0]=='\0') {
                error("You must specify a non empty output file name\n");
                return USAGE_ERROR_CODE;
             }
             strcpy(output,options.vars()->optarg);
             break;
   case 'd': if (options.vars()->optarg[0]=='\0') {
                error("Empty inflection directory\n");
                return USAGE_ERROR_CODE;
             }
             strcpy(inflection_dir,options.vars()->optarg);
             break;
   case 'a': if (options.vars()->optarg[0]=='\0') {
                error("You must specify a non empty alphabet file name\n");
                return USAGE_ERROR_CODE;
             }
             strcpy(alphabet,options.vars()->optarg);
             break;
   case 'b': if (options.vars()->optarg[0]=='\0') {
                error("You must specify a non empty binary dictionary name\n");
                return USAGE_ERROR_CODE;
             }
             strcpy(dic_bin,options.vars()->optarg);
             remove_extension(dic_bin,dic_inf);
             strcat(dic_inf,".inf");
             break;
   case 'V': only_verify_arguments = true;
             break;
   case 'h': usage(); 
             return SUCCESS_RETURN_CODE;
   case 'f': graph_recompilation_policy = ALWAYS_RECOMPILE; break;
   case 'n': graph_recompilation_policy = NEVER_RECOMPILE;  break;
   case 't': graph_recompilation_policy = ONLY_OUT_OF_DATE; break;
   case ':': index==-1 ? error("Missing argument for option -%c\n",options.vars()->optopt) :
                         error("Missing argument for option --%s\n",lopts_BuildKrMwuDic[index].name);
             return USAGE_ERROR_CODE;
   case '?': index==-1 ? error("Invalid option -%c\n",options.vars()->optopt) :
                         error("Invalid option --%s\n",options.vars()->optarg);
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
   }
   index=-1;
}
if (options.vars()->optind!=argc-1) {
   error("Invalid arguments: rerun with --help\n");
   return USAGE_ERROR_CODE;
}
if (output[0]=='\0') {
   error("Output file must be specified\n");
   return USAGE_ERROR_CODE;
}
if (inflection_dir[0]=='\0') {
   error("Inflection directory must be specified\n");
   return USAGE_ERROR_CODE;
}
if (alphabet[0]=='\0') {
   error("Alphabet file must be specified\n");
   return USAGE_ERROR_CODE;
}
if (dic_bin[0]=='\0') {
   error("Binary dictionary must be specified\n");
   return USAGE_ERROR_CODE;
}

if (only_verify_arguments) {
  // freeing all allocated memory 
  return SUCCESS_RETURN_CODE;
}

U_FILE* delas=u_fopen(&vec,argv[options.vars()->optind],U_READ);
if (delas==NULL) {
   error("Cannot open %s\n",argv[options.vars()->optind]);
   return DEFAULT_ERROR_CODE;
}

U_FILE* grf=u_fopen(&vec,output,U_WRITE);
if (grf==NULL) {
   error("Cannot open %s\n",output);
   u_fclose(delas);  
   return DEFAULT_ERROR_CODE;
}

Alphabet* alph=load_alphabet(&vec,alphabet,1);
if (alph==NULL) {
   u_fclose(grf);
   u_fclose(delas);
   error("Cannot open alphabet file %s\n",alphabet);
   return DEFAULT_ERROR_CODE;
}
Korean* korean=new Korean(alph);

MultiFlex_ctx* multiFlex_ctx=new_MultiFlex_ctx(inflection_dir,
                                               NULL,
                                               NULL,
                                               &vec,
                                               korean,
                                               NULL,
                                               NULL,
                                               graph_recompilation_policy);

Dictionary* d=new_Dictionary(&vec,dic_bin,dic_inf);

create_mwu_dictionary(delas,grf,multiFlex_ctx,d);

free_Dictionary(d);
u_fclose(delas);
u_fclose(grf);
free_alphabet(alph);
delete korean;
for (int count_free_fst2=0;count_free_fst2<multiFlex_ctx->n_fst2;count_free_fst2++) {
    free_abstract_Fst2(multiFlex_ctx->fst2[count_free_fst2],&(multiFlex_ctx->fst2_free[count_free_fst2]));
    multiFlex_ctx->fst2[count_free_fst2]=NULL;
}
free_MultiFlex_ctx(multiFlex_ctx);
u_printf("Done.\n");
return SUCCESS_RETURN_CODE;
}

} // namespace unitex
