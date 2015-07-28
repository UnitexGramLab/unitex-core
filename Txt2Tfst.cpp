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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Text_tokens.h"
#include "Alphabet.h"
#include "Unicode.h"
#include "DELA_tree.h"
#include "DELA.h"
#include "String_hash.h"
#include "BuildTextAutomaton.h"
#include "NormalizationFst2.h"
#include "Fst2.h"
#include "File.h"
#include "Copyright.h"
#include "StringParsing.h"
#include "Error.h"
#include "Grf2Fst2_lib.h"
#include "LocateMatches.h"
#include "UnitexGetOpt.h"
#include "LanguageDefinition.h"
#include "NewLineShifts.h"
#include "Txt2Tfst.h"
#include "Korean.h"
#include "HashTable.h"
#include "TfstStats.h"
#include "Offsets.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * This function tries to read a sentence from the given file.
 * It stops when it finds the EOF, a "{S}" or when MAX_TOKENS_IN_SENTENCE
 * tokens have been read. All the tokens that compose the sentence are stored
 * into 'buffer', except the "{S}", if any. The number of these tokens is stored
 * in '*N'. '*total' contains the whole number of integers read from the file.
 *
 * The function returns 1 if a sentence was read; 0 otherwise.
 */
int read_sentence(int* buffer,int *N,int *total,U_FILE* f,int SENTENCE_MARKER,int SPACE) {
*total=0;
*N=0;
if (1!=fread(buffer,sizeof(int),1,f)) {
   /* If we are at the end of the file */
   return 0;
}
*total=1;
int length;
if (buffer[0]==SENTENCE_MARKER) {
   /* If the text starts by a {S}, we don't want to stop there */
   length=0;
} else {
   length=1;
}
int control=-1;
while (length<MAX_TOKENS_IN_SENTENCE && 1==(control=(int)fread(buffer+length,sizeof(int),1,f)) && buffer[length]!=SENTENCE_MARKER) {
   length++;
   (*total)++;
}
if (length<MAX_TOKENS_IN_SENTENCE && control==1 && buffer[length]==SENTENCE_MARKER) {
   (*total)++;
}
if (control==0) {
	/* If we have reached the end of file, we make sure that we really have a sentence
	 * and not just only remaining spaces after the last {S} */
	int only_spaces=1;
	for (int i=0;i<length;i++) {
		if (buffer[i]!=SPACE) {
			only_spaces=0;
			break;
		}
	}
	if (only_spaces) return 0;
}
if (length==0) return 0;
*N=length;
return 1;
}

#define STR_VALUE_MACRO(x) #x
#define STR_VALUE_MACRO_STRING(x) STR_VALUE_MACRO(x)

const char* usage_Txt2Tfst =
         "Usage: Txt2Tfst [OPTIONS] <snt>\n"
         "\n"
         "  <snt> : the .snt text file\n"
         "\n"
         "OPTIONS:\n"
         "  -a ALPH/--alphabet=ALPH: the alphabet file\n"
         "  -c/---clean: cleans each sentence automaton, keeping best paths\n"
         "  -n XXX/--normalization_grammar=XXX: the .fst2 grammar used to normalize the text automaton\n"
         "  -t XXX/--tagset=XXX: use the XXX ELAG tagset file to normalize the dictionary entries\n"
         "  -K/--korean: tells Txt2Tfst that it works on Korean\n"
         "  -V/--only-verify-arguments: only verify arguments syntax and exit\n"
         "  -h/--help: this help\n"
         "\n"
         "Constructs the text automaton. If the sentences of the text were delimited\n"
         "with the special tag {S}, the program produces one automaton per sentence.\n"
         "If not, the text is turned into " STR_VALUE_MACRO_STRING(MAX_TOKENS_IN_SENTENCE) " token long automata. The result files\n"
         "named \"text.tfst\" and \"text.tind\" are stored is the text directory.\n"
         "\n"
         "Note that the program will also take into account the file \"tags.ind\", if any.\n";

static void usage() {
  display_copyright_notice();
  u_printf(usage_Txt2Tfst);
}


const char* optstring_Txt2Tfst=":a:cn:t:KVhk:q:";
const struct option_TS lopts_Txt2Tfst[]={
  {"alphabet", required_argument_TS, NULL, 'a'},
  {"clean", no_argument_TS, NULL, 'c'},
  {"normalization_grammar", required_argument_TS, NULL, 'n'},
  {"tagset", required_argument_TS, NULL, 't'},
  {"korean", no_argument_TS, NULL, 'K'},
  {"only_verify_arguments",no_argument_TS,NULL,'V'},
  {"help", no_argument_TS, NULL, 'h'},
  {"input_encoding",required_argument_TS,NULL,'k'},
  {"output_encoding",required_argument_TS,NULL,'q'},
  {NULL, no_argument_TS, NULL, 0}
};


int main_Txt2Tfst(int argc,char* const argv[]) {
if (argc==1) {
   usage();
   return SUCCESS_RETURN_CODE;
}

char alphabet[FILENAME_MAX]="";
char norm[FILENAME_MAX]="";
char tagset[FILENAME_MAX]="";
int is_korean=0;
int CLEAN=0;
VersatileEncodingConfig vec=VEC_DEFAULT;
int val,index=-1;
bool only_verify_arguments = false;
UnitexGetOpt options;
while (EOF!=(val=options.parse_long(argc,argv,optstring_Txt2Tfst,lopts_Txt2Tfst,&index))) {
   switch(val) {
   case 'a': if (options.vars()->optarg[0]=='\0') {
                error("You must specify a non empty alphabet file name\n");
                return USAGE_ERROR_CODE;
             }
             strcpy(alphabet,options.vars()->optarg);
             break;
   case 'c': CLEAN=1; break;
   case 'n': if (options.vars()->optarg[0]=='\0') {
                error("You must specify a non empty normalization grammar name\n");
                return USAGE_ERROR_CODE;
             }
             strcpy(norm,options.vars()->optarg);
             break;
   case 't': if (options.vars()->optarg[0]=='\0') {
                error("You must specify a non empty tagset file name\n");
                return USAGE_ERROR_CODE;
             }
             strcpy(tagset,options.vars()->optarg);
             break;
   case 'K': is_korean=1;
             break;
   case 'V': only_verify_arguments = true;
             break;
   case 'h': usage();
             return SUCCESS_RETURN_CODE;
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
   case ':': index==-1 ? error("Missing argument for option -%c\n",options.vars()->optopt) :
                         error("Missing argument for option --%s\n",lopts_Txt2Tfst[index].name);
             return USAGE_ERROR_CODE;
   case '?': index==-1 ? error("Invalid option -%c\n",options.vars()->optopt) :
                         error("Invalid option --%s\n",options.vars()->optarg);
             return USAGE_ERROR_CODE;
   }
   index=-1;
}

if (options.vars()->optind!=argc-1) {
   error("Invalid arguments: rerun with --help\n");
   return USAGE_ERROR_CODE;
}

if (is_korean) {
   if (alphabet[0]=='\0') {
      error("-a option is mandatory when -k is used\n");
      return USAGE_ERROR_CODE;
   }
   if (norm[0]!='\0') {
      // TODO(martinec) change this by a warning
      error("-n option is ignored when -k is used\n");
   }
   if (tagset[0]!='\0') {
      // TODO(martinec) change this by a warning    
      error("-t option is ignored when -k is used\n");
   }
}

if (only_verify_arguments) {
  // freeing all allocated memory
  return SUCCESS_RETURN_CODE;
}

struct DELA_tree* tree=new_DELA_tree();
int buffer[MAX_TOKENS_IN_SENTENCE];
char tokens_txt[FILENAME_MAX];
char text_cod[FILENAME_MAX];
char dlf[FILENAME_MAX];
char dlc[FILENAME_MAX];
char tags_ind[FILENAME_MAX];
get_snt_path(argv[options.vars()->optind],tokens_txt);
strcat(tokens_txt,"tokens.txt");
get_snt_path(argv[options.vars()->optind],text_cod);
strcat(text_cod,"text.cod");
get_snt_path(argv[options.vars()->optind],dlf);
strcat(dlf,"dlf");
get_snt_path(argv[options.vars()->optind],dlc);
strcat(dlc,"dlc");
get_snt_path(argv[options.vars()->optind],tags_ind);
strcat(tags_ind,"tags.ind");
struct match_list* tag_list=NULL;
load_DELA(&vec,dlf,tree);
load_DELA(&vec,dlc,tree);

u_printf("Loading %s...\n",tags_ind);
U_FILE* tag_file=u_fopen(&vec,tags_ind,U_READ);
if (tag_file!=NULL) {
   tag_list=load_match_list(tag_file,NULL,NULL);
   u_fclose(tag_file);
}

Alphabet* alph=NULL;
if (alphabet[0]!='\0') {
   alph=load_alphabet(&vec,alphabet,is_korean);
   if (alph==NULL) {
      error("Cannot open %s\n",alphabet);
      free_DELA_tree(tree);
      return DEFAULT_ERROR_CODE;
   }
}

Korean* korean=NULL;
if (is_korean) {
   korean=new Korean(alph);
}

struct text_tokens* tokens=load_text_tokens(&vec,tokens_txt);
if (tokens==NULL) {
   error("Cannot open %s\n",tokens_txt);
   delete korean;
   free_alphabet(alph);
   free_DELA_tree(tree);
   return DEFAULT_ERROR_CODE;
}

U_FILE* f=u_fopen(BINARY,text_cod,U_READ);
if (f==NULL) {
  error("Cannot open %s\n",text_cod);
  free_text_tokens(tokens);
  delete korean;
  free_alphabet(alph);
  free_DELA_tree(tree);
  return DEFAULT_ERROR_CODE;
}

char text_tfst[FILENAME_MAX];
get_snt_path(argv[options.vars()->optind],text_tfst);
strcat(text_tfst,"text.tfst");
U_FILE* tfst=u_fopen(&vec,text_tfst,U_WRITE);
if (tfst==NULL) {
  error("Cannot create %s\n",text_tfst);
  u_fclose(f);
  free_text_tokens(tokens);
  delete korean;
  free_alphabet(alph);
  free_DELA_tree(tree);
  return DEFAULT_ERROR_CODE;
}

char text_tind[FILENAME_MAX];
get_snt_path(argv[options.vars()->optind],text_tind);
strcat(text_tind,"text.tind");
U_FILE* tind=u_fopen(BINARY,text_tind,U_WRITE);
if (tind==NULL) {
  error("Cannot create %s\n",text_tind);
  u_fclose(tfst);
  u_fclose(f);
  free_text_tokens(tokens);
  delete korean;
  free_alphabet(alph);
  free_DELA_tree(tree);
  return DEFAULT_ERROR_CODE;   
}

struct normalization_tree* normalization_tree=NULL;
if (norm[0]!='\0') {
   normalization_tree=load_normalization_fst2(&vec,norm,alph,tokens);
}

char enter_pos_f[FILENAME_MAX];
get_snt_path(argv[options.vars()->optind],enter_pos_f);
strcat(enter_pos_f,"enter.pos");
U_FILE* f_enter=u_fopen(BINARY,enter_pos_f,U_READ);
int n_enter_char;
int* enter_pos=NULL;
if (f_enter==NULL) {
   error("Cannot open file %s\n",enter_pos);
   n_enter_char=0;
}
else {
   n_enter_char=(int)(get_file_size(f_enter)/4);
   enter_pos=(int*)malloc(sizeof(int)*n_enter_char);
   if (enter_pos==NULL  || 
      (enter_pos!=NULL  && n_enter_char!=(int)fread(enter_pos,sizeof(int),n_enter_char,f_enter))) {
      enter_pos==NULL ? alloc_error("main_Txt2Tfst") :
                        error("I/O error in main on file %d %s\n",n_enter_char,enter_pos_f);
      free_normalization_tree(normalization_tree);
      u_fclose(tind);
      u_fclose(tfst);
      u_fclose(f);
      free_text_tokens(tokens);
      delete korean;
      free_alphabet(alph);
      free_DELA_tree(tree);
      return DEFAULT_ERROR_CODE;
   }
   u_fclose(f_enter);
}

char snt_offsets_pos[FILENAME_MAX];
get_snt_path(argv[options.vars()->optind],snt_offsets_pos);
strcat(snt_offsets_pos,"snt_offsets.pos");

vector_int* snt_offsets=load_snt_offsets(snt_offsets_pos);
if (snt_offsets==NULL) {
	error("Cannot load offset file %s\n",snt_offsets_pos);
  free(enter_pos);
  free_normalization_tree(normalization_tree);
  u_fclose(tind);
  u_fclose(tfst);
  u_fclose(f);
  free_text_tokens(tokens);
  delete korean;
  free_alphabet(alph);
  free_DELA_tree(tree);
  return DEFAULT_ERROR_CODE;  
}

language_t* language=NULL;
if (tagset[0]!='\0') {
   language=load_language_definition(&vec,tagset);
}

int sentence_number=1;
int N=0;
int total=0;
int current_global_position_in_tokens=0;
int current_global_position_in_chars=0;
/* We reserve the space for printing the number of sentence automata */
u_fprintf(tfst,"0000000000\n");
u_printf("Constructing text automaton...\n");
Ustring* text=new_Ustring(2048);
struct hash_table* form_frequencies=new_hash_table((HASH_FUNCTION)hash_unichar,(EQUAL_FUNCTION)u_equal,
        (FREE_FUNCTION)free,NULL,(KEYCOPY_FUNCTION)keycopy);

while (read_sentence(buffer,&N,&total,f,tokens->SENTENCE_MARKER,tokens->SPACE)) {
   /* We compute and save the current sentence description */
   build_sentence_automaton(buffer,N,tokens,tree,alph,tfst,tind,sentence_number,CLEAN,
            normalization_tree,&tag_list,
            current_global_position_in_tokens,
            current_global_position_in_chars+get_shift(n_enter_char,enter_pos,current_global_position_in_tokens,snt_offsets),
            language,korean,form_frequencies);
   if (sentence_number%100==0) u_printf("%d sentences read...        \r",sentence_number);
   sentence_number++;
   current_global_position_in_tokens=current_global_position_in_tokens+total;
   for (int y=0;y<total;y++) {
      current_global_position_in_chars=current_global_position_in_chars+u_strlen(tokens->token[buffer[y]]);
   }
}
u_printf("%d sentence%s read\n",sentence_number-1,(sentence_number-1)>1?"s":"");
u_fclose(f);
free(enter_pos);
free_vector_int(snt_offsets);
/* Finally, we save statistics */
char tfst_tags_by_freq[FILENAME_MAX];
char tfst_tags_by_alph[FILENAME_MAX];
get_snt_path(argv[options.vars()->optind],tfst_tags_by_freq);
strcat(tfst_tags_by_freq,"tfst_tags_by_freq.txt");
get_snt_path(argv[options.vars()->optind],tfst_tags_by_alph);
strcat(tfst_tags_by_alph,"tfst_tags_by_alph.txt");
U_FILE* f_tfst_tags_by_freq=u_fopen(&vec,tfst_tags_by_freq,U_WRITE);
if (f_tfst_tags_by_freq==NULL) {
	error("Cannot open %s\n",tfst_tags_by_freq);
}
U_FILE* f_tfst_tags_by_alph=u_fopen(&vec,tfst_tags_by_alph,U_WRITE);
if (f_tfst_tags_by_alph==NULL) {
	error("Cannot open %s\n",tfst_tags_by_alph);
}

sort_and_save_tfst_stats(form_frequencies,f_tfst_tags_by_freq,f_tfst_tags_by_alph);

u_fclose(f_tfst_tags_by_alph);
u_fclose(f_tfst_tags_by_freq);
free_hash_table(form_frequencies);
free_Ustring(text);
free_language_t(language);
free_normalization_tree(normalization_tree);
u_fclose(tind);
// close tfst before call write_number_of_graphs()
u_fclose(tfst);
write_number_of_graphs(&vec,text_tfst,sentence_number-1,0);
free_text_tokens(tokens);
delete korean;
free_alphabet(alph);
free_DELA_tree(tree);

/* After the execution, tag_list should have been emptied, so that we don't
 * need to do it here */
return SUCCESS_RETURN_CODE;
}

} // namespace unitex
