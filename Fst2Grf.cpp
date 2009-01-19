 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "Fst2.h"
#include "Sentence2Grf.h"
#include "File.h"
#include "List_int.h"
#include "Copyright.h"
#include "IOBuffer.h"
#include "Error.h"
#include "getopt.h"


void usage() {
u_printf("%S",COPYRIGHT);
u_printf("Usage: Fst2Grf [OPTIONS] <fst2>\n"
         "\n"
         "  <fst2>: the .fst2 file that contains the text automaton.\n"
         "\n"
         "OPTIONS:\n"
         "  -s N/--sentence=N: the number of the sentence to be converted.\n"
         "  -o XXX/--output=XXX:  name .grf file as XXX.grf and the .txt one as XXX.txt (default=cursentence)\n" 
         "  -f FONT/--font=FONT: use the font FONT in the output .grf (default=Times new Roman).\n"
         "  -h/--help: this help\n"
         "\n"
         "Converts a sentence automaton into a GRF file that can be viewed. The\n"
         "resulting file, named cursentence.grf, is stored in the same directory\n"
         "that <text automaton>. The text of the sentence is saved in the same\n"
         "directory, in a file named cursentence.txt.\n");
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

const char* optstring=":s:o:f:h";
const struct option lopts[]= {
      {"sentence",required_argument,NULL,'s'},
      {"output",required_argument,NULL,'o'},
      {"font",required_argument,NULL,'f'},
      {"help",no_argument,NULL,'h'},
      {NULL,no_argument,NULL,0}
};
int SENTENCE=-1;
char* fontname=NULL;
char* output=NULL;
int val,index=-1;
char foo;
optind=1;
while (EOF!=(val=getopt_long(argc,argv,optstring,lopts,&index))) {
   switch(val) {
   case 's': if (1!=sscanf(optarg,"%d%c",&SENTENCE,&foo) || SENTENCE<=0) {
                /* foo is used to check that the sentence number is not like "45gjh" */
                fatal_error("Invalid sentence number: %s\n",optarg);
             }
             break;
   case 'o': if (optarg[0]=='\0') {
                fatal_error("You must specify a non empty output name pattern\n");
             }
             output=strdup(optarg);
             break;
   case 'f': if (optarg[0]=='\0') {
                fatal_error("You must specify a non empty font name\n");
             }
             fontname=strdup(optarg);
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

if (SENTENCE==-1) {
   fatal_error("You must specify a sentence number\n");
}

if (optind!=argc-1) {
   error("Invalid arguments: rerun with --help\n");
   return 1;
}
char grf_name[FILENAME_MAX];
char txt_name[FILENAME_MAX];

get_path(argv[optind],grf_name);
get_path(argv[optind],txt_name);
if (output==NULL) {
   strcat(grf_name,"cursentence.grf");
   strcat(txt_name,"cursentence.txt");
} else {
   strcat(grf_name,output);
   strcat(grf_name,".grf");
   strcat(txt_name,output);
   strcat(txt_name,".txt");
}
if (fontname==NULL) {
   fontname=strdup("Times New Roman");
}
FILE* f=u_fopen(grf_name,U_WRITE);
if (f==NULL) {
   error("Cannot file %s\n",grf_name);
   return 1;
}
FILE* txt=u_fopen(txt_name,U_WRITE);
if (txt==NULL) {
   error("Cannot file %s\n",txt_name);
   u_fclose(f);
   return 1;
}
u_printf("Loading %s...\n",argv[optind]);
Fst2* fst2=load_one_sentence_from_fst2(argv[optind],SENTENCE);
if (fst2==NULL) {
   error("Cannot load text automata file %s\n",argv[optind]);
   u_fclose(f);
   u_fclose(txt);
   return 1;
}
u_fprintf(txt,"%S\n",fst2->graph_names[SENTENCE]);
u_fclose(txt);
u_printf("Creating GRF...\n");
sentence_to_grf(fst2,SENTENCE,fontname,f);
u_fclose(f);
free(fontname);
if (output!=NULL) {
   free(output);
}
free_Fst2(fst2);
u_printf("Done.\n");
return 0;
}

