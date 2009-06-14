 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
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
#include "Tfst.h"
#include "Sentence2Grf.h"
#include "File.h"
#include "List_int.h"
#include "Copyright.h"
#include "Error.h"
#include "getopt.h"
#include "Tfst2Grf.h"


const char* usage_Tfst2Grf =
         "Usage: Tfst2Grf [OPTIONS] <tfst>\n"
         "\n"
         "  <tfst>: the .tfst file that contains the text automaton.\n"
         "\n"
         "OPTIONS:\n"
         "  -s N/--sentence=N: the number of the sentence to be converted.\n"
         "  -o XXX/--output=XXX:  name .grf file as XXX.grf and the .txt one as XXX.txt (default=cursentence)\n"
         "  -f FONT/--font=FONT: use the font FONT in the output .grf (default=Times new Roman).\n"
         "  -z N/--fontsize=N: set the font size (default=10).\n"
		 "  -h/--help: this help\n"
         "\n"
         "Converts a sentence automaton into a GRF file that can be viewed. The\n"
         "resulting file, named cursentence.grf, is stored in the same directory\n"
         "that <text automaton>. The text of the sentence is saved in the same\n"
         "directory, in a file named cursentence.txt. The numbers of the tokens are\n"
         "saved in a file named cursentence.tok.\n";


static void usage() {
u_printf("%S",COPYRIGHT);
u_printf(usage_Tfst2Grf);
}


const char* optstring_Tfst2Grf=":s:o:f:z:h";
const struct option_TS lopts_Tfst2Grf[]= {
      {"sentence",required_argument_TS,NULL,'s'},
      {"output",required_argument_TS,NULL,'o'},
      {"font",required_argument_TS,NULL,'f'},
      {"fontsize",required_argument_TS,NULL,'z'},
      {"help",no_argument_TS,NULL,'h'},
      {NULL,no_argument_TS,NULL,0}
};


int main_Tfst2Grf(int argc,char* argv[]) {
if (argc==1) {
   usage();
   return 0;
}


int SENTENCE=-1;
int size=10;
char* fontname=NULL;
char* output=NULL;
int val,index=-1;
char foo;
struct OptVars* vars=new_OptVars();
while (EOF!=(val=getopt_long_TS(argc,argv,optstring_Tfst2Grf,lopts_Tfst2Grf,&index,vars))) {
   switch(val) {
   case 's': if (1!=sscanf(vars->optarg,"%d%c",&SENTENCE,&foo) || SENTENCE<=0) {
                /* foo is used to check that the sentence number is not like "45gjh" */
                fatal_error("Invalid sentence number: %s\n",vars->optarg);
             }
             break;
   case 'o': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty output name pattern\n");
             }
             output=strdup(vars->optarg);
             if (output==NULL) {
                fatal_alloc_error("main_Tfst2Grf");
             }
             break;
   case 'f': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty font name\n");
             }
             fontname=strdup(vars->optarg);
             if (fontname==NULL) {
                fatal_alloc_error("main_Tfst2Grf");
             }
             break;
   case 'z': if (1!=sscanf(vars->optarg,"%d%c",&size,&foo) || size<=0) {
                /* foo is used to check that the sentence number is not like "45gjh" */
                fatal_error("Invalid font size: %s\n",vars->optarg);
             }
             break;
   case 'h': usage(); return 0;
   case ':': if (index==-1) fatal_error("Missing argument for option -%c\n",vars->optopt);
             else fatal_error("Missing argument for option --%s\n",lopts_Tfst2Grf[index].name);
   case '?': if (index==-1) fatal_error("Invalid option -%c\n",vars->optopt);
             else fatal_error("Invalid option --%s\n",vars->optarg);
             break;
   }
   index=-1;
}

if (SENTENCE==-1) {
   fatal_error("You must specify a sentence number\n");
}

if (vars->optind!=argc-1) {
   error("Invalid arguments: rerun with --help\n");
   return 1;
}
char grf_name[FILENAME_MAX];
char txt_name[FILENAME_MAX];
char tok_name[FILENAME_MAX];

get_path(argv[vars->optind],grf_name);
get_path(argv[vars->optind],txt_name);
get_path(argv[vars->optind],tok_name);
if (output==NULL) {
   strcat(grf_name,"cursentence.grf");
   strcat(txt_name,"cursentence.txt");
   strcat(tok_name,"cursentence.tok");
} else {
   strcat(grf_name,output);
   strcat(grf_name,".grf");
   strcat(txt_name,output);
   strcat(txt_name,".txt");
   strcat(tok_name,output);
   strcat(tok_name,".tok");
}
if (fontname==NULL) {
   fontname=strdup("Times New Roman");
   if (fontname==NULL) {
      fatal_alloc_error("main_Tfst2Grf");
   }
}
U_FILE* f=u_fopen(UTF16_LE,grf_name,U_WRITE);
if (f==NULL) {
   error("Cannot file %s\n",grf_name);
   return 1;
}
U_FILE* txt=u_fopen(UTF16_LE,txt_name,U_WRITE);
if (txt==NULL) {
   error("Cannot file %s\n",txt_name);
   u_fclose(f);
   return 1;
}
U_FILE* tok=u_fopen(UTF16_LE,tok_name,U_WRITE);
if (tok==NULL) {
   error("Cannot file %s\n",tok_name);
   u_fclose(f);
   u_fclose(txt);
   return 1;
}
u_printf("Loading %s...\n",argv[vars->optind]);
Tfst* tfst=open_text_automaton(argv[vars->optind]);

load_sentence(tfst,SENTENCE);
u_fprintf(txt,"%S\n",tfst->text);
u_fclose(txt);

for (int i=0;i<tfst->tokens->nbelems;i++) {
   u_fprintf(tok,"%d %d\n",tfst->tokens->tab[i],tfst->token_sizes->tab[i]);
}
u_fclose(tok);

u_printf("Creating GRF...\n");
sentence_to_grf(tfst,fontname,size,f);
u_fclose(f);
free(fontname);
if (output!=NULL) {
   free(output);
}
close_text_automaton(tfst);
free_OptVars(vars);
u_printf("Done.\n");
return 0;
}

