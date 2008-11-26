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
#include "Copyright.h"
#include "Fst2Automaton.h"
#include "ElagFstFilesIO.h"
#include "utils.h"
#include "IOBuffer.h"
#include "Error.h"
#include "File.h"
#include "LanguageDefinition.h"
#include "SingleGraph.h"
#include "getopt.h"



void usage() {
u_printf("%S",COPYRIGHT);
u_printf("Usage: TagsetNormFst2 [OPTIONS] <txtauto>\n"
         "\n"
         "  <txtauto>: text automaton to normalize\n"
         "\n"
         "OPTIONS:\n"
         "  -t TAGSET/--tagset=TAGSET: tagset description file\n"
         "  -h/--help: this help\n"
         "\n"
         "Normalizes the specified text automaton according to the tagset description\n"
         "file, discarding undeclared dictionary codes and incoherent lexical entries.\n"
         "Inflectional features are unfactorized so that '{rouge,.A:fs:ms}' will be\n"
         "divided into the 2 tags '{rouge,.A:fs}' and '{rouge,.A:ms}'.\n"
         "The input text automaton is modified.\n");
}



int main(int argc,char* argv[]) {
/* Every Unitex program must start by this instruction,
 * in order to avoid display problems when called from
 * the graphical interface */
setBufferMode();

if (argc==1) {
   usage();
   return 0;
}

const char* optstring=":t:h";
const struct option lopts[]= {
      {"tagset",required_argument,NULL,'t'},
      {"help",no_argument,NULL,'h'},
      {NULL,no_argument,NULL,0}
};
char txtauto[FILENAME_MAX]="";
char tagset[FILENAME_MAX]="";
int val,index=-1;
optind=1;
while (EOF!=(val=getopt_long(argc,argv,optstring,lopts,&index))) {
   switch(val) {
   case 't': if (optarg[0]=='\0') {
                fatal_error("You must specify a non empty tagset file name\n");
             }
             strcpy(tagset,optarg);
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

if (optind!=argc-1) {
   fatal_error("Invalid arguments: rerun with --help\n");
}   

if (tagset[0]=='\0') {
   fatal_error("You must specify the tagset file\n");
}
strcpy(txtauto,argv[optind]);
char bak[FILENAME_MAX];
strcpy(bak,txtauto);
strcat(bak,".bak");
u_printf("Copying %s to %s ...\n",txtauto,bak);
copy_file(bak,txtauto);
u_printf("Loading tagset...\n");
language_t* language=load_language_definition(tagset);
set_current_language(language);
Elag_fst_file_in* txtin=load_fst_file(bak,FST_TEXT,LANGUAGE);
if (txtin==NULL) {
   fatal_error("Unable to load text automaton '%s'\n",bak);
}
Elag_fst_file_out* txtout=fst_file_out_open(txtauto,FST_TEXT);
if (txtout==NULL) {
   fatal_error("Unable to open '%s' for writing\n",txtauto);
}
Fst2Automaton* A;
u_printf("Cleaning text automaton...\n");
int current_sentence=0;
while ((A=load_automaton(txtin))!=NULL) {
   trim(A->automaton);
   if (A->automaton->number_of_states!=0) {
      minimize(A->automaton,1);
   }
   if (A->automaton->number_of_states==0) {
      error("Sentence %d is empty\n",current_sentence+1);
      SingleGraphState initial=add_state(A->automaton);
      set_initial_state(initial);
      SingleGraphState final=add_state(A->automaton);
      set_final_state(final);
      add_outgoing_transition(initial,-1,1);
   }
   fst_file_write(txtout,A);
   free_Fst2Automaton(A);
   current_sentence++;
   if (current_sentence%100==0) {
      u_printf("Sentence %d/%d ...      \r",current_sentence,txtin->nb_automata);
   }
}
u_printf("Sentence %d/%d.\nDone: text automaton is normalized.\n",txtin->nb_automata,txtin->nb_automata);
fst_file_close_in(txtin);
fst_file_close_out(txtout);
return 0;
}




