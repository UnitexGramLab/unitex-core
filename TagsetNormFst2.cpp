 /*
  * Unitex
  *
  * Copyright (C) 2001-2007 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "autalmot.h"
#include "fst_file.h"
#include "utils.h"
#include "IOBuffer.h"
#include "Error.h"
#include "File.h"
#include "LanguageDefinition.h"



void usage() {
u_printf("%S",COPYRIGHT);
u_printf("Usage: TagsetNormFst2 -l <tagset> <txtauto>\n"
         "\n"
         "with:\n"
         "<tagset>  : tagset description file\n"
         "<txtauto> : text automaton to normalize\n"
         "\n"
         "Normalize the specified text automaton according to the tagset description file,\n"
         "discarding undeclared dictionary codes and incoherent lexical entries.\n"
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
copy_file(bak, txtauto);
u_printf("Loading tagset...\n");
language_t* tagset=load_language_definition(langname);
set_current_language(tagset);
fst_file_in_t* txtin=load_fst_file(bak,FST_TEXT);

  if (txtin == NULL) { fatal_error("tagsetnorm: unable to load text '%s'\n", bak); }

  fst_file_out_t * txtout = fst_file_out_open(txtauto, FST_TEXT);

  if (txtout == NULL) { fatal_error("unable to open '%s' for writing\n", txtauto); }

  autalmot_t * A;

  u_printf("cleaning text fsa\n");


  unichar EMPTY[] = { 'E', 'M', 'P', 'T', 'Y', 0 };
  symbol_t * symb = new_symbol_UNKNOWN(tagset, language_add_form(tagset, EMPTY));

  int no = 0;
  while ((A = fst_file_autalmot_load_next(txtin)) != NULL) {
    autalmot_emonde(A);

    if (A->nbstates == 0) {

      error("sentence %d is empty\n", no + 1);
      
      int q0 = autalmot_add_state(A, AUT_INITIAL);
      int q1 = autalmot_add_state(A, AUT_FINAL);

      autalmot_add_trans(A, q0, symb, q1);
    }

    fst_file_write(txtout, A);
    autalmot_delete(A);
    no++;
    if (no % 100 == 0) { u_printf("sentence %d/%d ...      \r", no, txtin->nb_automata); }
  }
  u_printf("sentence %d/%d.\ndone. text automaton is normalised.\n", txtin->nb_automata, txtin->nb_automata);

  fst_file_close(txtin);
  fst_file_close(txtout);

  return 0;
}




