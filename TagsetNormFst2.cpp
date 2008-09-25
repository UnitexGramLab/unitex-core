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



void usage() {
u_printf("%S",COPYRIGHT);
u_printf("Usage: TagsetNormFst2 -l <tagset> <txtauto>\n"
         "\n"
         "with:\n"
         "<tagset>  : tagset description file\n"
         "<txtauto> : text automaton to normalize\n"
         "\n"
         "Normalize the specified text automaton according to the tagset description\n"
         "file, discarding undeclared dictionary codes and incoherent lexical entries.\n"
         "Inflectional features are unfactorized so that '{rouge,.A:fs:ms}' will be\n"
         "divided into the 2 tags '{rouge,.A:fs}' and '{rouge,.A:ms}'.\n"
         "The text automaton is modified.\n");
}



int main(int argc, char ** argv) {
/* Every Unitex program must start by this instruction,
 * in order to avoid display problems when called from
 * the graphical interface */
setBufferMode();

char* txtauto=NULL;
char* langname=NULL;
argv++;
argc--;
if (argc==0) {
   usage();
   return 0;
}
while (argc) {
   if (**argv!='-') {
      /* Text automaton */
      txtauto=(*argv);
   } else {
      if (!strcmp(*argv, "-l")) {
         /* Tagset file */
         argv++;
         argc--;
         if (argc==0) {
            fatal_error("-l needs an argument\n");
         }
         langname=(*argv);
      } else if (!strcmp(*argv,"-h")) {
         usage();
         return 0;
      } else {
         fatal_error("Unknown argument: '%s'\n",*argv);
      }
   }
   argv++;
   argc--;
}	
if (!langname) {
   fatal_error("No tagset specified\n");
}
if (txtauto==NULL) {
   fatal_error("No text automaton specified\n");
}
char bak[FILENAME_MAX];
strcpy(bak,txtauto);
strcat(bak,".bak");
u_printf("Copying %s to %s ...\n",txtauto,bak);
copy_file(bak,txtauto);
u_printf("Loading tagset...\n");
language_t* language=load_language_definition(langname);
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




