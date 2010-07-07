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
#include "Tagger.h"
#include "TaggingProcess.h"
#include "Error.h"
#include "Tfst.h"
#include "File.h"
#include "DELA.h"
#include "AbstractDelaLoad.h"
#include "Unicode.h"
#include "TfstStats.h"

const char* usage_Tagger =
         "Usage: Tagger [OPTIONS] <tfst>\n"
         "\n"
         "  <tfst>: the text automaton to use in input\n"
         "\n"
         "OPTIONS:\n"
		   "  -a ALPH/--alphabet=ALPH: the alphabet file\n"
		   "  -d DICT/--dictionary=DICT: use the .bin tagger dictionary containing tuples (unigrams,bigrams and trigrams)"
		   " with frequencies\n"
		   "  -t TAGSET/--tagset=TAGSET: use the TAGSET ELAG tagset file to normalize the dictionary entries\n"
		   "\n"
		   "Output options:\n"
		   "  -o OUT/--output=OUT: specifies the output .tfst file. By default, the input .tfst is replaced.\n"
		   "  -h/--help: this help\n"
		   "\n"
         "Applies statistical tagging to the given text automaton.\n"
		   "The output .tfst file is a linear text automaton.\n\n";



static void usage() {
u_printf("%S",COPYRIGHT);
u_printf(usage_Tagger);
}


const char* optstring_Tagger=":a:d:t:o:h";
const struct option_TS lopts_Tagger[]= {
	  {"alphabet", required_argument_TS, NULL, 'a'},
	  {"dictionary", required_argument_TS, NULL, 'd'},
	  {"tagset", required_argument_TS, NULL, 't'},
	  {"output",required_argument_TS,NULL,'o'},
      {"help",no_argument_TS,NULL,'h'},
      {NULL,no_argument_TS,NULL,0}
};


int main_Tagger(int argc,char* const argv[]) {
if (argc==1) {
   usage();
   return 0;
}

int val,index=-1;
struct OptVars* vars=new_OptVars();
char tfst[FILENAME_MAX]="";
char tind[FILENAME_MAX]="";
char tmp_tind[FILENAME_MAX]="";
char output_tind[FILENAME_MAX];
char tmp_tfst[FILENAME_MAX]="";
char output[FILENAME_MAX]="";
char temp[FILENAME_MAX]="";
char dictionary[FILENAME_MAX]="";
char alphabet[FILENAME_MAX]="";
char tagset[FILENAME_MAX]="";
Encoding enc = DEFAULT_ENCODING_OUTPUT;
int bom_output = DEFAULT_BOM_OUTPUT;
while (EOF!=(val=getopt_long_TS(argc,argv,optstring_Tagger,lopts_Tagger,&index,vars))) {
   switch(val) {
   case 'a': if (vars->optarg[0]=='\0') {
                   fatal_error("You must specify a non empty alphabet file name\n");
                }
                strcpy(alphabet,vars->optarg);
                break;
   case 'd': if (vars->optarg[0]=='\0') {
				  fatal_error("You must specify a non empty dictionary file name\n");
			   }
			   strcpy(dictionary,vars->optarg);
			   break;
   case 't': if (vars->optarg[0]=='\0') {
                   fatal_error("You must specify a non empty tagset file name\n");
                }
                strcpy(tagset,vars->optarg);
                break;
   case 'o': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty output file name\n");
             }
             strcpy(output,vars->optarg);
             break;
   case 'h': usage(); return 0;
   case ':': if (index==-1) fatal_error("Missing argument for option -%c\n",vars->optopt);
             else fatal_error("Missing argument for option --%s\n",lopts_Tagger[index].name);
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

if(alphabet[0] == '\0'){
	fatal_error("No alphabet file specified\n");
}
strcpy(tfst,argv[vars->optind]);

strcpy(tind,tfst);
remove_extension(tind);
strcat(tind,".tind");

Alphabet* alpha = load_alphabet(alphabet);

get_path(tfst,temp);
strcat(temp,"temp.tfst");

struct BIN_free_info bin_free;
const unsigned char* bin=load_abstract_BIN_file(dictionary,&bin_free);
if (bin==NULL) {
	fatal_error("");
}
remove_extension(dictionary);
strcat(dictionary,".inf");
struct INF_free_info inf_free;
const struct INF_codes* inf=load_abstract_INF_file(dictionary,&inf_free);
if (inf==NULL) {
	fatal_error("");
}

char* current_tfst = tfst;
int form_type = get_form_type(bin,inf,alpha);
if(form_type == 1){
	if(tagset[0] == '\0'){
		fatal_error("No tagset file specified\n");
	}
	/* if we use compound forms in the viterbi algorithm
	 * we must separate tags according to their morphological
	 * features (one tag per feature). Explode operation is
	 * necessary.*/
	if(tagset[0] == '\0'){
		fatal_error("-t option is mandatory when compound forms dictionary is used\n");
	}
	u_printf("Explodes tfst automaton according to tagset...\n");
	strcpy(tmp_tfst,tfst);
	remove_extension(tmp_tfst);
	strcat(tmp_tfst,"_explode.tfst");
	language_t* lang = load_language_definition(tagset);
	explode_tfst(tfst,tmp_tfst,enc,bom_output,lang,NULL);
	current_tfst = tmp_tfst;
	u_printf("\n");
}

Tfst* input_tfst = open_text_automaton(current_tfst);
if(input_tfst == NULL) {
	fatal_error("Cannot load input .tfst\n");
}


remove_extension(temp,tmp_tind);
strcat(tmp_tind,".tind");

U_FILE* out_tfst=u_fopen(UTF16_LE,temp,U_WRITE);
if (out_tfst==NULL) {
	fatal_error("Cannot create output .tfst\n");
}
U_FILE* out_tind=u_fopen(BINARY,tmp_tind,U_WRITE);
if (out_tind==NULL) {
	fatal_error("Cannot create output .tind\n");
}
Tfst* result=new_Tfst(out_tfst,out_tind,input_tfst->N);

u_printf("Tagging...\n");

/* We use this hash table to rebuild files tfst_tags_by_freq/alph.txt */
struct hash_table* form_frequencies=new_hash_table((HASH_FUNCTION)hash_unichar,(EQUAL_FUNCTION)u_equal,
        (FREE_FUNCTION)free,NULL,(KEYCOPY_FUNCTION)keycopy);

/* launches tagging process on the input tfst file */
do_tagging(input_tfst,result,bin,inf,alpha,form_type,form_frequencies);

close_text_automaton(input_tfst);
close_text_automaton(result);

/* We save statistics */
char tfst_tags_by_freq[FILENAME_MAX];
char tfst_tags_by_alph[FILENAME_MAX];
get_path(tfst,tfst_tags_by_freq);
if (output[0]!='\0') {
	   strcat(tfst_tags_by_freq,"tfst_tags_by_freq.new.txt");
} else {
	   strcat(tfst_tags_by_freq,"tfst_tags_by_freq.txt");
}
get_path(tfst,tfst_tags_by_alph);
if (output[0]!='\0') {
	   strcat(tfst_tags_by_alph,"tfst_tags_by_alph.new.txt");
} else {
	   strcat(tfst_tags_by_alph,"tfst_tags_by_alph.txt");
}
U_FILE* f_tfst_tags_by_freq=u_fopen_creating_versatile_encoding(enc,bom_output,tfst_tags_by_freq,U_WRITE);
if (f_tfst_tags_by_freq==NULL) {
	error("Cannot open %s\n",tfst_tags_by_freq);
}
U_FILE* f_tfst_tags_by_alph=u_fopen_creating_versatile_encoding(enc,bom_output,tfst_tags_by_alph,U_WRITE);
if (f_tfst_tags_by_alph==NULL) {
	error("Cannot open %s\n",tfst_tags_by_alph);
}
sort_and_save_tfst_stats(form_frequencies,f_tfst_tags_by_freq,f_tfst_tags_by_alph);
u_fclose(f_tfst_tags_by_freq);
u_fclose(f_tfst_tags_by_alph);
free_hash_table(form_frequencies);

if(output[0]=='\0'){
	af_remove(tfst);
	af_remove(tind);
	af_rename(temp,tfst);
	af_rename(tmp_tind,tind);
}
else {
	af_rename(temp,output);
	strcpy(output_tind,output);
	remove_extension(output_tind);
	strcat(output_tind,".tind");
	af_rename(tmp_tind,output_tind);
}
free_alphabet(alpha);
free_abstract_BIN(bin,&bin_free);
free_abstract_INF(inf,&inf_free);
free_OptVars(vars);
u_printf("Done.\n");
return 0;
}

