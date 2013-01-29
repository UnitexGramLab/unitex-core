/*
 * Unitex
 *
 * Copyright (C) 2001-2013 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "Text_tokens.h"
#include "ExtractUnits.h"
#include "Copyright.h"
#include "Error.h"
#include "Snt.h"
#include "UnitexGetOpt.h"
#include "Extract.h"
#include "LocateMatches.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

const char* usage_Extract =
         "Usage: Extract [OPTIONS] <text>\n"
         "\n"
         "  <text>: the .snt text to extract from the units\n"
         "\n"
         "OPTIONS:\n"
         "  -y/--yes: extract all matching units (default)\n"
         "  -n/--no: extract all unmatching units\n"
         "  -i X/--index=X: the .ind file that describes the concordance. By default,\n"
         "                  X is the concord.ind file located in the text directory.\n"
         "  -o OUT/--output=OUT: the text file where the units will be stored\n"
         "  -h/--help: this help\n"
         "\n"
         "\nExtract all the units that contain (or not) any part of a utterance. The\n"
         "units are supposed to be separated by the symbol {S}.\n";


static void usage() {
u_printf("%S",COPYRIGHT);
u_printf(usage_Extract);
}


const char* optstring_Extract=":yni:o:hk:q:";
const struct option_TS lopts_Extract[]= {
      {"yes",no_argument_TS,NULL,'y'},
      {"no",no_argument_TS,NULL,'n'},
      {"output",required_argument_TS,NULL,'o'},
      {"index",required_argument_TS,NULL,'i'},
      {"help",no_argument_TS,NULL,'h'},
      {"input_encoding",required_argument_TS,NULL,'k'},
      {"output_encoding",required_argument_TS,NULL,'q'},
      {NULL,no_argument_TS,NULL,0}
};


int main_Extract(int argc,char* const argv[]) {
if (argc==1) {
   usage();
   return 0;
}


int val,index=-1;
char extract_matching_units=1;
char text_name[FILENAME_MAX]="";
char concord_ind[FILENAME_MAX]="";
char output[FILENAME_MAX]="";
VersatileEncodingConfig vec=VEC_DEFAULT;
struct OptVars* vars=new_OptVars();
while (EOF!=(val=getopt_long_TS(argc,argv,optstring_Extract,lopts_Extract,&index,vars))) {
   switch(val) {
   case 'y': extract_matching_units=1; break;
   case 'n': extract_matching_units=0; break;
   case 'o': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty output file name\n");
             }
             strcpy(output,vars->optarg);
             break;
   case 'i': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty concordance file name\n");
             }
             strcpy(concord_ind,vars->optarg);
             break;
   case 'h': usage(); return 0;
   case ':': if (index==-1) fatal_error("Missing argument for option -%c\n",vars->optopt);
             else fatal_error("Missing argument for option --%s\n",lopts_Extract[index].name);
   case '?': if (index==-1) fatal_error("Invalid option -%c\n",vars->optopt);
             else fatal_error("Invalid option --%s\n",vars->optarg);
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
   }
   index=-1;
}

if (output[0]=='\0') {
   fatal_error("You must specify the output text file\n");
}
if (vars->optind!=argc-1) {
   fatal_error("Invalid arguments: rerun with --help\n");
}
strcpy(text_name,argv[vars->optind]);

struct snt_files* snt_files=new_snt_files(text_name);
ABSTRACTMAPFILE* text=af_open_mapfile(snt_files->text_cod,MAPFILE_OPTION_READ,0);
if (text==NULL) {
   error("Cannot open %s\n",snt_files->text_cod);
   return 1;
}
struct text_tokens* tok=load_text_tokens(&vec,snt_files->tokens_txt);
if (tok==NULL) {
   error("Cannot load token list %s\n",snt_files->tokens_txt);
   af_close_mapfile(text);
   return 1;
}
if (concord_ind[0]=='\0') {
   char tmp[FILENAME_MAX];
   get_extension(text_name,tmp);
   if (strcmp(tmp,"snt")) {
      fatal_error("Unable to find the concord.ind file. Please explicit it\n");
   }
   remove_extension(text_name,concord_ind);
   strcat(concord_ind,"_snt");
   strcat(concord_ind,PATH_SEPARATOR_STRING);
   strcat(concord_ind,"concord.ind");
}
U_FILE* concord=u_fopen(&vec,concord_ind,U_READ);
if (concord==NULL) {
   error("Cannot open concordance %s\n",concord_ind);
   af_close_mapfile(text);
   free_text_tokens(tok);
   return 1;
}
struct match_list* l=load_match_list(concord,NULL,NULL);
u_fclose(concord);
if (tok->SENTENCE_MARKER==-1) {
	/* We have a special case when the text has not been
	 * split into sentences: the result will be either an empty
	 * file or the text itself
	 */
	if ((extract_matching_units && l==NULL)
			|| (!extract_matching_units && l!=NULL)) {
		/* result = empty file */
		u_fempty(&vec,output);
		return 0;
	}
	free_match_list(l);
	af_copy(text_name,output);
	return 0;
}




U_FILE* result=u_fopen(&vec,output,U_WRITE);
if (result==NULL) {
   error("Cannot write output file %s\n",output);
   af_close_mapfile(text);
   u_fclose(concord);
   free_text_tokens(tok);
   return 1;
}
free_snt_files(snt_files);
extract_units(extract_matching_units,text,tok,result,l);
af_close_mapfile(text);
u_fclose(result);
free_text_tokens(tok);
free_OptVars(vars);
u_printf("Done.\n");
return 0;
}

} // namespace unitex
