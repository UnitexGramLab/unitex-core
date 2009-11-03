/*
  * Unitex
  *
  * Copyright (C) 2001-2003 Universit<E9> Paris-Est Marne-la-Vall<E9>e <unitex@univ-mlv.fr>
  *
  * This program is free software; you can redistribute it and/or
  * modify it under the terms of the GNU General Public License
  * as published by the Free Software Foundation; either version 2
  * of the License, or (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program; if not, write to the Free Software
  * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
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
#include "getopt.h"
#include "MultiFlex.h"
#include "MF_Global.h"


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
                     "              files and inflection graphs for single and compound words.\n"
		 "  -j TABLE/--jamo=TABLE: specifies the jamo conversion table to use for Korean\n"
		 "  -f FST2/--fst2=FST2: specifies the jamo->hangul transducer to use for Korean\n"
		 "  -s/--only-simple-words: the program will consider compound words as errors\n"
		 "  -c/--only-compound-words: the program will consider simple words as errors\n"
         "  -h/--help: this help\n"
         "\n"
         "Inflects a DELAS or DELAC into a DELAF or DELACF. Note that you can merge\n"
         "simple and compound words in a same dictionary.\n";


static void usage() {
u_printf("%S",COPYRIGHT);
u_printf(usage_MultiFlex);
}


const char* optstring_MultiFlex=":o:a:d:j:f:schk:q:";
const struct option_TS lopts_MultiFlex[]= {
      {"output",required_argument_TS,NULL,'o'},
      {"alphabet",required_argument_TS,NULL,'a'},
      {"directory",required_argument_TS,NULL,'d'},
      {"jamo",required_argument_TS,NULL,'j'},
      {"fst2",required_argument_TS,NULL,'f'},
      {"only-simple-words",no_argument_TS,NULL,'s'},
      {"only-compound-words",no_argument_TS,NULL,'c'},
      {"input_encoding",required_argument_TS,NULL,'k'},
      {"output_encoding",required_argument_TS,NULL,'q'},
      {"help",no_argument_TS,NULL,'h'},
      {NULL,no_argument_TS,NULL,0}
};


int main_MultiFlex(int argc,char* argv[]) {
if (argc==1) {
   usage();
   return 0;
}


char output[FILENAME_MAX]="";
char config_dir[FILENAME_MAX]="";
char alphabet[FILENAME_MAX]="";
char jamo_table[FILENAME_MAX]="";
char fst2[FILENAME_MAX]="";
MultiFlex_ctx* p_multiFlex_ctx;
//Current language's alphabet
Alphabet* alph=NULL;
int error_check_status=SIMPLE_AND_COMPOUND_WORDS;
Encoding encoding_output = DEFAULT_ENCODING_OUTPUT;
int bom_output = DEFAULT_BOM_OUTPUT;
int mask_encoding_compatibility_input = DEFAULT_MASK_ENCODING_COMPATIBILITY_INPUT;
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
   case 'j': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty jamo table file name\n");
             }
             strcpy(jamo_table,vars->optarg);
             break;
   case 'f': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty transducer file name\n");
             }
             strcpy(fst2,vars->optarg);
             break;
   case 's': error_check_status=ONLY_SIMPLE_WORDS; break;
   case 'c': error_check_status=ONLY_COMPOUND_WORDS; break;
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
p_multiFlex_ctx = (MultiFlex_ctx*)malloc(sizeof(MultiFlex_ctx));
if (p_multiFlex_ctx == NULL) {
   fatal_error("memory error\n");
}
int err;  //0 if a function completes with no error
//Load morphology description
char morphology[FILENAME_MAX];
new_file(config_dir,"Morphology.txt",morphology);
int config_files_status=CONFIG_FILES_OK;
struct l_morpho_t* pL_MORPHO=init_langage_morph();
if (pL_MORPHO == NULL) {
   fatal_error("init_langage_morph error\n");
}
err=read_language_morpho(pL_MORPHO,morphology);
if (err) {
   config_files_status=CONFIG_FILES_ERROR;
}
print_language_morpho(pL_MORPHO);
if (alphabet[0]!='\0') {
   //Load alphabet
   alph=load_alphabet(alphabet,1);  //To be done once at the beginning of the inflection
   if (alph==NULL) {
      error("Cannot open alphabet file %s\n",alphabet);
      free_language_morpho(pL_MORPHO);
      free_alphabet(alph);
      free(p_multiFlex_ctx);
      return 1;
   }
}
//Init equivalence files
char equivalences[FILENAME_MAX];
new_file(config_dir,"Equivalences.txt",equivalences);
err=d_init_morpho_equiv(pL_MORPHO,equivalences);
if (err) {
   config_files_status=CONFIG_FILES_ERROR;
}
d_class_equiv_T D_CLASS_EQUIV;
d_init_class_equiv(pL_MORPHO,&D_CLASS_EQUIV);
//Initialize the structure for inflection transducers
strcpy(p_multiFlex_ctx->inflection_directory,config_dir);
err=MU_graph_init_graphs(p_multiFlex_ctx);
if (err) {
   MU_graph_free_graphs(p_multiFlex_ctx);
   free(p_multiFlex_ctx);
   return 1;
}

/* Korean */
jamoCodage* jamo=NULL;
Jamo2Syl* jamo2syl=NULL;
if (jamo_table[0]!='\0') {
   if (alph==NULL) {
      fatal_error("Cannot initialize Korean data with a NULL alphabet\n");
   }
	jamo=new jamoCodage();
	jamo->loadJamoMap(jamo_table);
	/* We also initializes the Chinese -> Hangul table */
	jamo->cloneHJAMap(alph->korean_equivalent_syllab);
	if (fst2[0]=='\0') {
		fatal_error("You must specify the Korean transducer to use with -f\n");
	}
   jamo2syl=new Jamo2Syl();
   jamo2syl->init(jamo_table,fst2);
}

//DELAC inflection
err=inflect(argv[vars->optind],output,p_multiFlex_ctx,pL_MORPHO,alph,encoding_output, bom_output, mask_encoding_compatibility_input,
            config_files_status,&D_CLASS_EQUIV,
		    error_check_status,jamo,jamo2syl);
MU_graph_free_graphs(p_multiFlex_ctx);
for (int count_free_fst2=0;count_free_fst2<p_multiFlex_ctx->n_fst2;count_free_fst2++) {
    free_abstract_Fst2(p_multiFlex_ctx->fst2[count_free_fst2],&(p_multiFlex_ctx->fst2_free[count_free_fst2]));
    p_multiFlex_ctx->fst2[count_free_fst2] = NULL;
}
free_alphabet(alph);
free_language_morpho(pL_MORPHO);
free_OptVars(vars);
free(p_multiFlex_ctx);
if (jamo!=NULL) {
	delete jamo;
	delete jamo2syl;
}
u_printf("Done.\n");
return 0;
}

