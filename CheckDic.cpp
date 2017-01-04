/*
 * Unitex
 *
 * Copyright (C) 2001-2017 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "File.h"
#include "DELA.h"
#include "String_hash.h"
#include "Copyright.h"
#include "Error.h"
#include "UnitexGetOpt.h"
#include "CheckDic.h"
#include "Alphabet.h"
#include "Ustring.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/* Maximum size of a DIC line */
#define CHECKDIC_LINE_SIZE 10000


const char* usage_CheckDic =
         "Usage: CheckDic [OPTIONS] <dela>\n"
         "\n"
         "  <dela> : name of the unicode text dictionary (must be a full path)\n"
         "\n"
         "OPTIONS:\n"
         "  -s/--delas: checks a non inflected dictionary\n"
         "  -f/--delaf: checks an inflected dictionary\n"
         "  -a ALPH/--alphabet=ALPH: alphabet file to use\n"
         "  -r/--strict: strict syntax checking against unprotected dot and comma\n"
         "  -t/--tolerate: tolerates unprotected dot and comma (default)\n"
         "  -n/--no_space_warning: tolerates spaces in grammatical/semantic/inflectional codes\n"
         "  -p/--skip_path: doesn't display the full pathname of dictionary\n"
         "  -V/--only-verify-arguments: only verify arguments syntax and exit\n"
         "  -h/--help: this help\n"
         "\n"
         "Checks the format of <dela> and produces a file named CHECK_DIC.TXT\n"
         "that contains check result informations. This file is stored in the\n"
         "<dela> directory.\n";


static void usage() {
  display_copyright_notice();
  u_printf(usage_CheckDic);
}


const char* optstring_CheckDic=":sfpa:Vhrtk:nq:";
const struct option_TS lopts_CheckDic[]= {
  {"delas",no_argument_TS,NULL,'s'},
  {"delaf",no_argument_TS,NULL,'f'},
  {"skip_path",no_argument_TS,NULL,'p'},
  {"alphabet",required_argument_TS,NULL,'a'},
  {"only_verify_arguments",no_argument_TS,NULL,'V'},
  {"help",no_argument_TS,NULL,'h'},
  {"tolerate",no_argument_TS,NULL,'t'},
  {"no_space_warning",no_argument_TS,NULL,'n'},
  {"strict",no_argument_TS,NULL,'r'},
  {"input_encoding",required_argument_TS,NULL,'k'},
  {"output_encoding",required_argument_TS,NULL,'q'},
  {NULL,no_argument_TS,NULL,0}
};

int main_CheckDic(int argc,char* const argv[]) {
if (argc==1) {
   usage();
   return SUCCESS_RETURN_CODE;
}

int is_a_DELAF=-1;
int strict_unprotected=0;
int skip_path=0;
char alph[FILENAME_MAX]="";
VersatileEncodingConfig vec=VEC_DEFAULT;
int val,index=-1;
int space_warnings=1;
bool only_verify_arguments = false;
UnitexGetOpt options;
while (EOF!=(val=options.parse_long(argc,argv,optstring_CheckDic,lopts_CheckDic,&index))) {
   switch(val) {
   case 'f': is_a_DELAF=1; break;
   case 's': is_a_DELAF=0; break;
   case 'V': only_verify_arguments = true;
             break;
   case 'h': usage();
             return SUCCESS_RETURN_CODE;
   case 'r': strict_unprotected=1; break;
   case 't': strict_unprotected=0; break;
   case 'n': space_warnings=0; break;
   case 'p': skip_path=1; break;
   case 'a': if (options.vars()->optarg[0]=='\0') {
                error("Empty alphabet argument\n");
                return USAGE_ERROR_CODE;
             }
             strcpy(alph,options.vars()->optarg);
             break;
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
                         error("Missing argument for option --%s\n",lopts_CheckDic[index].name);
             return USAGE_ERROR_CODE;
   case '?': index==-1 ? error("Invalid option -%c\n",options.vars()->optopt):
                         error("Invalid option --%s\n",options.vars()->optarg);
             return USAGE_ERROR_CODE;
   }
   index=-1;
}

if (is_a_DELAF==-1 || options.vars()->optind!=argc-1) {
   error("Invalid arguments: rerun with --help\n");
   return USAGE_ERROR_CODE;
}

if (only_verify_arguments) {
  // freeing all allocated memory
  return SUCCESS_RETURN_CODE;
}

U_FILE* dic=u_fopen(&vec,argv[options.vars()->optind],U_READ);
if (dic==NULL) {
  error("Cannot open dictionary %s\n",argv[options.vars()->optind]);
  return DEFAULT_ERROR_CODE;
}

char output_filename[FILENAME_MAX];
get_path(argv[options.vars()->optind],output_filename);
strcat(output_filename,"CHECK_DIC.TXT");
U_FILE* out=u_fopen(&vec,output_filename,U_WRITE);
if (out==NULL) {
  error("Cannot create %s\n",output_filename);
  u_fclose(dic);
  return DEFAULT_ERROR_CODE;
}

Alphabet* alphabet0=NULL;
if (alph[0]!='\0') {
   alphabet0=load_alphabet(&vec,alph,1);
}

u_printf("Checking %s...\n",argv[options.vars()->optind]);
int line_number=1;

/*
 * We declare and initialize an array in order to know which
 * letters are used in the dictionary.
 */
int i;
char* alphabet=(char*)malloc(sizeof(char)*MAX_NUMBER_OF_UNICODE_CHARS);
if (alphabet==NULL) {
  alloc_error("CheckDic's main");
  u_fclose(dic);
  u_fclose(out);
  free_alphabet(alphabet0);
  return ALLOC_ERROR_CODE;
}
memset(alphabet,0,sizeof(char)*MAX_NUMBER_OF_UNICODE_CHARS);
/*
 * We use two structures for the storage of the codes found in the
 * dictionary. Note that 'semantic_codes' is used to store both grammatical and
 * semantic codes.
 */
struct string_hash* semantic_codes=new_string_hash();
struct string_hash* inflectional_codes=new_string_hash();
struct string_hash* simple_lemmas=new_string_hash(DONT_USE_VALUES);
struct string_hash* compound_lemmas=new_string_hash(DONT_USE_VALUES);
int n_simple_entries=0;
int n_compound_entries=0;
/*
 * We read all the lines and check them.
 */
Ustring* line=new_Ustring(DIC_LINE_SIZE);
while (EOF!=readline(line,dic)) {
   if (line->str[0]=='\0') {
    /* If we have an empty line, we print a unicode error message
     * into the output file */
    u_fprintf(out,"Line %d: empty line\n",line_number);
  }
  else if (line->str[0]=='/') {
    /* If a line starts with '/', it is a commment line, so
     * we ignore it */
  }
  else {
    /* If we have a line to check, we check it according to the
     * dictionary type */
    check_DELA_line(line->str,out,is_a_DELAF,line_number,alphabet,semantic_codes,
                    inflectional_codes,simple_lemmas,compound_lemmas,
                    &n_simple_entries,&n_compound_entries,alphabet0,strict_unprotected);
  }
  /* At regular intervals, we display a message on the standard
   * output to show that the program is working */
  if (line_number%10000==0) {
    u_printf("%d lines read...\r",line_number);
  }
  line_number++;
}
free_Ustring(line);
u_printf("%d lines read\n",line_number-1);
u_fclose(dic);
/*
 * Once we have checked all the lines, we print some informations
 * in the output file.
 */
u_fprintf(out,"-----------------------------------\n");
u_fprintf(out,"-------------  Stats  -------------\n");
u_fprintf(out,"-----------------------------------\n");
if (skip_path != 0) {
    char filename_without_path[FILENAME_MAX];
    remove_path(argv[options.vars()->optind],filename_without_path);
    u_fprintf(out,"File: %s\n",filename_without_path);
}
else {
    u_fprintf(out,"File: %s\n",argv[options.vars()->optind]);
}
u_fprintf(out,"Type: %s\n",is_a_DELAF?"DELAF":"DELAS");
u_fprintf(out,"%d line%s read\n",line_number-1,(line_number-1>1)?"s":"");
u_fprintf(out,"%d simple entr%s ",n_simple_entries,(n_simple_entries>1)?"ies":"y");
u_fprintf(out,"for %d distinct lemma%s\n",simple_lemmas->size,(simple_lemmas->size>1)?"s":"");
u_fprintf(out,"%d compound entr%s ",n_compound_entries,(n_compound_entries>1)?"ies":"y");
u_fprintf(out,"for %d distinct lemma%s\n",compound_lemmas->size,(compound_lemmas->size>1)?"s":"");
/**
 * We print the list of the characters that are used, with
 * their unicode numbers shown in hexadecimal. This can be useful
 * to detect different characters that are graphically identical
 * like 'A' (upper of latin 'a' or upper of greek alpha ?).
 */
u_fprintf(out,"-----------------------------------\n");
u_fprintf(out,"----  All chars used in forms  ----\n");
u_fprintf(out,"-----------------------------------\n");
for (i=0;i<MAX_NUMBER_OF_UNICODE_CHARS;i++) {
  if (alphabet[i]) {
      u_fprintf(out,"%C (%04X)\n",i,i);
  }
}
/*
 * Then we print the list of all grammatical and semantic codes used in the
 * dictionary. If a code contains a non ASCII character, a space or a tabulation,
 * we print a warning.
 */
u_fprintf(out,"-------------------------------------------------------------\n");
u_fprintf(out,"----  %3d grammatical/semantic code%s",semantic_codes->size,(semantic_codes->size>1)?"s used in dictionary  ----\n":" used in dictionary  -----\n");
u_fprintf(out,"-------------------------------------------------------------\n");
unichar comment[2000];
for (i=0;i<semantic_codes->size;i++) {
  /* We print the code, followed if necessary by a warning */
  u_fputs(semantic_codes->value[i],out);
  if (warning_on_code(semantic_codes->value[i],comment,space_warnings)) {
    u_fprintf(out," %S",comment);
  }
  u_fprintf(out,"\n");
}
/*
 * Finally, we print the list of inflectional codes,
 * with warnings in the case of non ASCII letters, spaces
 * or tabulations.
 */
u_fprintf(out,"-----------------------------------------------------\n");
u_fprintf(out,"----  %3d inflectional code%s",inflectional_codes->size,(inflectional_codes->size>1)?"s used in dictionary  ----\n":" used in dictionary  -----\n");
u_fprintf(out,"-----------------------------------------------------\n");


for (i=0;i<inflectional_codes->size;i++) {
  u_fputs(inflectional_codes->value[i],out);
  if (warning_on_code(inflectional_codes->value[i],comment,space_warnings)) {
    u_fprintf(out," %S",comment);
  }
  u_fprintf(out,"\n");
}
u_fclose(out);
u_printf("Done.\n");

free(alphabet);
free_alphabet(alphabet0);

/* Note that we don't free anything below since it would only waste time */
#if (defined(UNITEX_LIBRARY) || defined(UNITEX_RELEASE_MEMORY_AT_EXIT))
/* cleanup for no leak on library */
free_string_hash(semantic_codes);
free_string_hash(inflectional_codes);
free_string_hash(simple_lemmas);
free_string_hash(compound_lemmas);
#endif
return SUCCESS_RETURN_CODE;
}

} // namespace unitex

