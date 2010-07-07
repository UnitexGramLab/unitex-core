/*
 * Unitex
 *
 * Copyright (C) 2001-2010 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

/*
 * author : Anthony Sigogne
 */

#include <stdio.h>
#include "Copyright.h"
#include "getopt.h"
#include "Error.h"
#include "Tfst.h"
#include "File.h"
#include "DELA.h"
#include "Unicode.h"
#include "TrainingTagger.h"
#include "TrainingProcess.h"


const char* usage_TrainingTagger =
         "Usage: TrainingTagger [OPTIONS] <text>\n"
         "\n"
         "  <text>: the text corpus to use in input\n"
         "\n"
		 "Output options:\n"
		 "  -o XXX/--output=XXX: pattern used to name output tagger dictionaries XXX_simple.bin"
		 " and XXX_compound.bin (default=filename of text corpus without extension)\n"
		 "  -b/--binaries: indicates whether the program should compress dictionaries"
		 "  into .bin files (default)\n"
		 "  -n/--no_binaries: indicates whether the program should not compress dictionaries"
		 "  into .bin files\n"
		 "  -a/--all_dictionaries: indicates whether the program should produce simple and compound form"
         " tuples dictionaries (default)\n"
		 "  -s/--simple_forms: indicates whether the program should produce only simple form"
         " tuples dictionary\n"
		 "  -c/--compound_forms: indicates whether the program should produce only compound form"
		 " tuples dictionary\n"
		 "  -h/--help: this help\n"
		 "\n"
         "Extract statistics from a tagged corpus and save its into dictionaries. "
         "These statistics are necessary in the tagging process in order to compute probabilities."
         "Dictionaries are saved in the same path as corpus file.\n\n";



static void usage() {
u_printf("%S",COPYRIGHT);
u_printf(usage_TrainingTagger);
}


const char* optstring_TrainingTagger=":o:hbnsca";
const struct option_TS lopts_TrainingTagger[]= {
	  {"output",required_argument_TS,NULL,'o'},
	  {"binaries",no_argument_TS,NULL,'b'},
	  {"no_binaries",no_argument_TS,NULL,'n'},
	  {"simple_forms",no_argument_TS,NULL,'s'},
	  {"compound_forms",no_argument_TS,NULL,'c'},
	  {"all",no_argument_TS,NULL,'a'},
	  {"help",no_argument_TS,NULL,'h'},
      {NULL,no_argument_TS,NULL,0}
};


int main_TrainingTagger(int argc,char* const argv[]) {
if (argc==1) {
   usage();
   return 0;
}

int val,index=-1,binaries=1,s_forms=1,c_forms=1;
struct OptVars* vars=new_OptVars();
char text[FILENAME_MAX]="";
char simple_forms[FILENAME_MAX]="";
char compound_forms[FILENAME_MAX]="";
char output[FILENAME_MAX]="";
while (EOF!=(val=getopt_long_TS(argc,argv,optstring_TrainingTagger,lopts_TrainingTagger,&index,vars))) {
   switch(val) {
   case 'o': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty pattern\n");
             }
             strcpy(output,vars->optarg);
             break;
   case 'b': binaries = 1;
			 break;
   case 'n': binaries = 0;
			 break;
   case 'a': break;
   case 's': c_forms = 0;
			 break;
   case 'c': s_forms = 0;
   			 break;
   case 'h': usage(); return 0;
   case ':': if (index==-1) fatal_error("Missing argument for option -%c\n",vars->optopt);
             else fatal_error("Missing argument for option --%s\n",lopts_TrainingTagger[index].name);
   case '?': if (index==-1) fatal_error("Invalid option -%c\n",vars->optopt);
             else fatal_error("Invalid option --%s\n",vars->optarg);
             break;
   }
   index=-1;
}

if (vars->optind!=argc-1) {
   error("Invalid arguments: rerun with --help\n");
   return 1;
}
strcpy(text,argv[vars->optind]);
U_FILE* input_text=u_fopen(UTF16_LE,text,U_READ);

if(output[0]=='\0'){
	remove_path_and_extension(text,output);
}

char path[FILENAME_MAX],filename[FILENAME_MAX];
get_path(text,path);
if(strlen(path) == 0){
	strcpy(path,".");
}
/* we create files which will contain statistics extracted from the tagged corpus */
U_FILE* sforms_file = NULL, *cforms_file = NULL;
if(s_forms == 1){
	sprintf(filename,"%s_simple.dic",output);
	new_file(path,filename,simple_forms);
	sforms_file=u_fopen(UTF16_LE,simple_forms,U_WRITE);
}
if(c_forms == 1){
	sprintf(filename,"%s_compound.dic",output);
	new_file(path,filename,compound_forms);
	cforms_file=u_fopen(UTF16_LE,compound_forms,U_WRITE);
}

u_printf("Gathering statistics from tagged corpus...\n");
do_training(input_text,sforms_file,cforms_file);

/* we close all files and then we sort text dictionaries */
u_fclose(input_text);
char disclaimer[FILENAME_MAX];
if(sforms_file != NULL){
	u_fclose(sforms_file);
	pseudo_main_SortTxt(DEFAULT_ENCODING_OUTPUT,DEFAULT_BOM_OUTPUT,ALL_ENCODING_BOM_POSSIBLE,0,0,NULL,NULL,0,simple_forms);
	strcpy(disclaimer,simple_forms);
	remove_extension(disclaimer);
	strcat(disclaimer,".txt");
	create_disclaimer(disclaimer);
}
if(cforms_file != NULL){
	u_fclose(cforms_file);
	pseudo_main_SortTxt(DEFAULT_ENCODING_OUTPUT,DEFAULT_BOM_OUTPUT,ALL_ENCODING_BOM_POSSIBLE,0,0,NULL,NULL,0,compound_forms);
	strcpy(disclaimer,compound_forms);
	remove_extension(disclaimer);
	strcat(disclaimer,".txt");
	create_disclaimer(disclaimer);
}

/* we compress dictionaries if option is specified by user (output is ".bin") */
if(binaries == 1){
/* simple forms dictionary */
if(s_forms == 1){
	pseudo_main_Compress(DEFAULT_ENCODING_OUTPUT,DEFAULT_BOM_OUTPUT,ALL_ENCODING_BOM_POSSIBLE,0,simple_forms);
}
/* compound forms dictionary */
if(c_forms == 1){
	pseudo_main_Compress(DEFAULT_ENCODING_OUTPUT,DEFAULT_BOM_OUTPUT,ALL_ENCODING_BOM_POSSIBLE,0,compound_forms);
}
}
free_OptVars(vars);
u_printf("Done.\n");
return 0;
}

