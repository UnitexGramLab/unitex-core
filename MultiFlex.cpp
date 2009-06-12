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


//Current language's alphabet
Alphabet* alph;

// Directory containing the inflection tranducers and the 'Morphology' file
extern char inflection_directory[FILENAME_MAX];


static void usage() {
u_printf("%S",COPYRIGHT);
u_printf("Usage: MultiFlex [OPTIONS] <dela>\n"
         "\n"
         "  <dela>: the unicode DELAS or DELAC file to be inflected\n"
         "\n"
         "OPTIONS:\n"
         "  -o DELAF/--output=DELAF: the unicode resulting DELAF or DELACF dictionary\n"
         "  -a ALPH/--alphabet=ALPH: the alphabet file \n"
         "  -d DIR/--directory=DIR: the directory containing 'Morphology' and 'Equivalences'\n"
                     "              files and inflection graphs for single and compound words.\n"
		 "  -j TABLE/--jamo=TABLE: specifies the jamo conversion table to use for Korean\n"
		 "  -s/--only-simple-words: the program will consider compound words as errors\n"
		 "  -c/--only-compound-words: the program will consider simple words as errors\n"
         "  -h/--help: this help\n"
         "\n"
         "Inflects a DELAS or DELAC into a DELAF or DELACF. Note that you can merge\n"
         "simple and compound words in a same dictionary.\n");
}


int main_MultiFlex(int argc,char* argv[]) {
if (argc==1) {
   usage();
   return 0;
}

const char* optstring=":o:a:d:j:sch";
const struct option_TS lopts[]= {
      {"output",required_argument_TS,NULL,'o'},
      {"alphabet",required_argument_TS,NULL,'a'},
      {"directory",required_argument_TS,NULL,'d'},
      {"jamo",required_argument_TS,NULL,'j'},
      {"only-simple-words",no_argument_TS,NULL,'s'},
      {"only-compound-words",no_argument_TS,NULL,'c'},
      {"help",no_argument_TS,NULL,'h'},
      {NULL,no_argument_TS,NULL,0}
};
char output[FILENAME_MAX]="";
char config_dir[FILENAME_MAX]="";
char alphabet[FILENAME_MAX]="";
char jamo_table[FILENAME_MAX]="";
int error_check_status=SIMPLE_AND_COMPOUND_WORDS;
int val,index=-1;
struct OptVars* vars=new_OptVars();
while (EOF!=(val=getopt_long_TS(argc,argv,optstring,lopts,&index,vars))) {
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
   case 's': error_check_status=ONLY_SIMPLE_WORDS; break;
   case 'c': error_check_status=ONLY_COMPOUND_WORDS; break;
   case 'h': usage(); return 0;
   case ':': if (index==-1) fatal_error("Missing argument for option -%c\n",vars->optopt);
             else fatal_error("Missing argument for option --%s\n",lopts[index].name);
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
if (alphabet[0]=='\0') {
   fatal_error("You must specify the alphabet file\n");
}

int err;  //0 if a function completes with no error
//Load morphology description
char morphology[FILENAME_MAX];
new_file(config_dir,"Morphology.txt",morphology);
int config_files_status=CONFIG_FILES_OK;
err=read_language_morpho(morphology);
if (err) {
   config_files_status=CONFIG_FILES_ERROR;
}
print_language_morpho();
//Load alphabet
alph=load_alphabet(alphabet,1);  //To be done once at the beginning of the inflection
if (alph==NULL) {
   error("Cannot open alphabet file %s\n",alphabet);
   free_language_morpho();
   free_alphabet(alph);
   return 1;
}
//Init equivalence files
char equivalences[FILENAME_MAX];
new_file(config_dir,"Equivalences.txt",equivalences);
err=d_init_morpho_equiv(equivalences);
if (err) {
   config_files_status=CONFIG_FILES_ERROR;
}
d_class_equiv_T D_CLASS_EQUIV;
d_init_class_equiv(&D_CLASS_EQUIV);
//Initialize the structure for inflection transducers
strcpy(inflection_directory,config_dir);
err=MU_graph_init_graphs();
if (err) {
   MU_graph_free_graphs();
   return 1;
}

/* Korean */
jamoCodage* jamo=NULL;
if (jamo_table[0]!='\0') {
	jamo=new jamoCodage();
	jamo->loadJamoMap(jamo_table);
	/* We also initializes the Chinese -> Hangul table */
	jamo->cloneHJAMap(alph->korean_equivalent_syllab);
}

//DELAC inflection
err=inflect(argv[vars->optind],output,config_files_status,&D_CLASS_EQUIV,error_check_status,jamo);
MU_graph_free_graphs();
free_alphabet(alph);
free_language_morpho();
free_OptVars(vars);
if (jamo!=NULL) {
	delete jamo;
}
u_printf("Done.\n");
return 0;
}


