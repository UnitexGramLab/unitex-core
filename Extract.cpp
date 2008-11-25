 /*
  * Unitex
  *
  * Copyright (C) 2001-2008 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "IOBuffer.h"
#include "Error.h"
#include "Snt.h"
#include "getopt.h"


void usage() {
u_printf("%S",COPYRIGHT);
u_printf("Usage: Extract [OPTIONS] <text>\n"
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
         "units are supposed to be separated by the symbol {S}.\n");
}


int main(int argc, char **argv) {
/* Every Unitex program must start by this instruction,
 * in order to avoid display problems when called from
 * the graphical interface */
setBufferMode();
if (argc==1) {
   usage();
   return 0;
}

const char* optstring=":yni:o:h";
const struct option lopts[]= {
      {"yes",no_argument,NULL,'y'},
      {"no",no_argument,NULL,'n'},
      {"output",required_argument,NULL,'o'},
      {"index",required_argument,NULL,'i'},
      {"help",no_argument,NULL,'h'},
      {NULL,no_argument,NULL,0}
};
int val,index=-1;
char extract_matching_units=1;
char text_name[FILENAME_MAX]="";
char concord_ind[FILENAME_MAX]="";
char output[FILENAME_MAX]="";
optind=1;
while (EOF!=(val=getopt_long(argc,argv,optstring,lopts,&index))) {
   switch(val) {
   case 'y': extract_matching_units=1; break;
   case 'n': extract_matching_units=0; break;
   case 'o': if (optarg[0]=='\0') {
                fatal_error("You must specify a non empty output file name\n");
             }
             strcpy(output,optarg);
             break;
   case 'i': if (optarg[0]=='\0') {
                fatal_error("You must specify a non empty concordance file name\n");
             }
             strcpy(concord_ind,optarg);
             break;
   case 'h': usage(); return 0;
   case ':': if (index==-1) fatal_error("Missing argument for option -%c\n",optopt); 
             else fatal_error("Missing argument for option --%s\n",lopts[index].name);
   case '?': if (index==-1) fatal_error("Invalid option -%c\n",optopt); 
             else fatal_error("Invalid option --%s\n",optarg);
             break;
   }
   index=-1;
}

if (output[0]=='\0') {
   fatal_error("You must specify the output text file\n");
}
if (optind!=argc-1) {
   fatal_error("Invalid arguments: rerun with --help\n");
}
strcpy(text_name,argv[optind]);

struct snt_files* snt_files=new_snt_files(text_name);
FILE* text=fopen(snt_files->text_cod,"rb");
if (text==NULL) {
   error("Cannot open %s\n",snt_files->text_cod);
   return 1;
}
struct text_tokens* tok=load_text_tokens(snt_files->tokens_txt);
if (tok==NULL) {
   error("Cannot load token list %s\n",snt_files->tokens_txt);
   fclose(text);
   return 1;
}
if (tok->SENTENCE_MARKER==-1) {
   error("The text does not contain any sentence marker {S}\n");
   fclose(text);
   free_text_tokens(tok);
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
FILE* concord=u_fopen(concord_ind,U_READ);
if (concord==NULL) {
   error("Cannot open concordance %s\n",concord_ind);
   fclose(text);
   free_text_tokens(tok);
   return 1;
}
FILE* result=u_fopen(output,U_WRITE);
if (result==NULL) {
   error("Cannot write output file %s\n",output);
   fclose(text);
   u_fclose(concord);
   free_text_tokens(tok);
   return 1;
}
free_snt_files(snt_files);
extract_units(extract_matching_units,text,tok,concord,result);
fclose(text);
u_fclose(concord);
u_fclose(result);
free_text_tokens(tok);
u_printf("Done.\n");
return 0;
}

