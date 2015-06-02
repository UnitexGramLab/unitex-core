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

/* Created by Agata Savary (savary@univ-tours.fr)
 */


#include <stdio.h>
#include <string.h>
#include "MF_LangMorpho.h"
#include "Unicode.h"
#include "MF_MU_graph.h"
#include "Alphabet.h"
#include "MF_DicoMorpho.h"
#include "MF_DLC_inflect.h"
#include "File.h"
#include "Korean.h"
#include "Copyright.h"
#include "Error.h"
#include "UnitexGetOpt.h"
#include "MultiFlex.h"
#include "MF_Global.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

// Directory containing the inflection tranducers and the 'Morphology' file
//extern char inflection_directory[FILENAME_MAX];


const char* usage_MultiFlex =
         "Usage: MultiFlex [OPTIONS] <dela>\n"
         "\n"
         "  <dela>: the unicode DELAS or DELAC file to be inflected\n"
         "\n"
         "OPTIONS:\n"
         "  -o DELAF/--output=DELAF: the unicode resulting DELAF or DELACF dictionary\n"
         "  -a ALPH/--alphabet=ALPH: the alphabet file \n"
         "  -d DIR/--directory=DIR: the directory containing 'Morphology' and 'Equivalences'\n"
         "                          files and inflection graphs for single and compound words.\n"
	       "  -K/--korean: tells MultiFlex that it works on Korean\n"
		     "  -s/--only-simple-words: the program will consider compound words as errors\n"
		     "  -c/--only-compound-words: the program will consider simple words as errors\n"     
         "  -p DIR/--pkgdir=DIR: path of the default graph repository\n"
         "  -r XXX/--named_repositories=XXX: declaration of named repositories. XXX is\n"
		     "                                   made of one or more X=Y sequences, separated by ;\n"
		     "                                   where X is the name of the repository denoted by\n"
		     "                                   the pathname Y. You can use this option several times\n"
         " Graph recompilation options:\n"
         "  -f/--always-recompile-graphs:        forces graph recompiling even if the fst is up to date\n"
         "  -n/--never-recompile-graphs:         avoids graph recompiling even if the fst is not up to date\n"
         "  -t/--only-recompile-outdated-graphs: only recompiling when the fst is not up to date (default)\n"
         "\n"
         "  -h/--help: display this help and exit\n"
         "\n"
         "Inflects a DELAS or DELAC into a DELAF or DELACF. Note that you can merge\n"
         "simple and compound words in a same dictionary.\n";


static void usage() {
display_copyright_notice();
u_printf(usage_MultiFlex);
}


const char* optstring_MultiFlex=":o:a:d:Kscfnthk:q:p:r:";
const struct option_TS lopts_MultiFlex[]= {
      {"output",required_argument_TS,NULL,'o'},
      {"alphabet",required_argument_TS,NULL,'a'},
      {"directory",required_argument_TS,NULL,'d'},
      {"korean",no_argument_TS,NULL,'K'},
      {"only-simple-words",no_argument_TS,NULL,'s'},
      {"only-compound-words",no_argument_TS,NULL,'c'},
      {"input_encoding",required_argument_TS,NULL,'k'},
      {"output_encoding",required_argument_TS,NULL,'q'},
      {"pkgdir",required_argument_TS,NULL,'p'},
      {"named_repositories",required_argument_TS,NULL,'r'},
      {"always-recompile-graphs",no_argument_TS,NULL,'f'},
      {"never-recompile-graphs",no_argument_TS,NULL,'n'},
      {"only-recompile-outdated-graphs",no_argument_TS,NULL,'t'},
      {"help",no_argument_TS,NULL,'h'},
      {NULL,no_argument_TS,NULL,0}
};


int main_MultiFlex(int argc,char* const argv[]) {
if (argc==1) {
   usage();
   return 0;
}

char output[FILENAME_MAX]="";
char config_dir[FILENAME_MAX]="";
char alphabet[FILENAME_MAX]="";
char pkgdir[FILENAME_MAX]="";
char* named=NULL;
int is_korean=0;
// default policy is to compile only out of date graphs
GraphRecompilationPolicy graph_recompilation_policy = ONLY_OUT_OF_DATE;
//Current language's alphabet
Alphabet* alph=NULL;
int error_check_status=SIMPLE_AND_COMPOUND_WORDS;
VersatileEncodingConfig vec=VEC_DEFAULT;
int val,index=-1;
struct OptVars* vars=new_OptVars();
while (EOF!=(val=getopt_long_TS(argc,argv,optstring_MultiFlex,lopts_MultiFlex,&index,vars))) {
   switch(val) {
   case 'o': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty DELAF file name\n");
             }
             strcpy(output,vars->optarg);
             break;
   case 'a': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty alphabet file name\n");
             }
             strcpy(alphabet,vars->optarg);
             break;
   case 'd': strcpy(config_dir,vars->optarg); break;
   case 'K': is_korean=1;
             break;
   case 's': error_check_status=ONLY_SIMPLE_WORDS; break;
   case 'c': error_check_status=ONLY_COMPOUND_WORDS; break;
   case 'f': graph_recompilation_policy = ALWAYS_RECOMPILE; break;
   case 'n': graph_recompilation_policy = NEVER_RECOMPILE;  break;
   case 't': graph_recompilation_policy = ONLY_OUT_OF_DATE; break;
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
   case 'p': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty package directory name\n");
             }
             strcpy(pkgdir,vars->optarg);
             break;
   case 'r': if (named==NULL) {
                  named=strdup(vars->optarg);
                  if (named==NULL) {
                     fatal_alloc_error("main_Grf2Fst2");
                  }
             } else {
            	 named = (char*)realloc((void*)named,strlen(named)+strlen(vars->optarg)+2);
                 if (named==NULL) {
                    fatal_alloc_error("main_MultiFlex");
                 }
                 strcat(named,";");
                 strcat(named,vars->optarg);
             }
             break;
   case 'h': usage(); return 0;
   case ':': if (index==-1) fatal_error("Missing argument for option -%c\n",vars->optopt);
             else fatal_error("Missing argument for option --%s\n",lopts_MultiFlex[index].name);
   case '?': if (index==-1) fatal_error("Invalid option -%c\n",vars->optopt);
             else fatal_error("Invalid option --%s\n",vars->optarg);
             break;
   }
   index=-1;
}

if (vars->optind!=argc-1) {
   fatal_error("Invalid arguments: rerun with --help\n");
}

if (output[0]=='\0') {
   fatal_error("You must specify the output DELAF name\n");
}

//Load morphology description
char morphology[FILENAME_MAX];
new_file(config_dir,"Morphology.txt",morphology);
//int config_files_status=CONFIG_FILES_OK;
if (alphabet[0]!='\0') {
   //Load alphabet
   alph=load_alphabet(&vec,alphabet,1);  //To be done once at the beginning of the inflection
   if (alph==NULL) {
      error("Cannot open alphabet file %s\n",alphabet);
      return 1;
   }
}
//Init equivalence files
char equivalences[FILENAME_MAX];
new_file(config_dir,"Equivalences.txt",equivalences);

/* Korean */
Korean* korean=NULL;
if (is_korean) {
   if (alph==NULL) {
      fatal_error("Cannot initialize Korean data with a NULL alphabet\n");
   }
	korean=new Korean(alph);
}
MultiFlex_ctx* p_multiFlex_ctx=new_MultiFlex_ctx(config_dir,
                                                 morphology,
                                                 equivalences,
                                                 &vec,
                                                 korean,
                                                 pkgdir,
                                                 named,
                                                 graph_recompilation_policy);


//DELAC inflection
int err = inflect(argv[vars->optind],output,p_multiFlex_ctx,alph,error_check_status);

// FIXME(martinec) err is never used 
((void)(err));

free(named);

for (int count_free_fst2=0;count_free_fst2<p_multiFlex_ctx->n_fst2;count_free_fst2++) {
    free_abstract_Fst2(p_multiFlex_ctx->fst2[count_free_fst2],&(p_multiFlex_ctx->fst2_free[count_free_fst2]));
    p_multiFlex_ctx->fst2[count_free_fst2] = NULL;
}
free_alphabet(alph);
free_OptVars(vars);
free_MultiFlex_ctx(p_multiFlex_ctx);
if (korean!=NULL) {
	delete korean;
}
u_printf("Done.\n");
return 0;
}

} // namespace unitex
