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

#include "Copyright.h"
#include "const.h"
#include "autalmot.h"
#include "fst_file.h"
#include "utils.h"
#include "IOBuffer.h"
#include "Error.h"



void usage() {
  printf("%s", COPYRIGHT);
  printf("Usage: TagsetNormFst2 -l <tagset> <txtauto>\n"
         "\n"
         "with:\n"
         "<tagset>  : tagset description file\n"
         "<txtauto> : text automaton to normalize\n"
         "\n"
         "Normalize the specified text automaton according to the tagset description file,\n"
         "discarding undeclared dictionary codes and incoherent lexical entries.\n"
         "The text automaton is modified.\n");
}


void copy_file(char * dest, char * src) {

  debug("copying %s into %s\n", src, dest);

  FILE * in = fopen(src, "rb");
  if (in == NULL) { fatal_error("unable to open '%s'\n", src); }

  FILE * out = fopen(dest, "wb");
  if (out == NULL) { fatal_error("unable to open '%s'\n", dest); }

  int c, n = 0;
  while ((c = getc(in)) != EOF) { putc(c, out); ++n; }
  fclose(in); fclose(out);
  debug("%d bytes copied into '%s'", n, dest);
}


int main(int argc, char ** argv) {
  setBufferMode();

  debug("tagsetnorm\n");

  char * txtauto    = NULL;
  char * langname   = NULL;

  argv++, argc--;

  if (argc == 0) { usage(); return 0; }
  
  while (argc) {

    if (**argv != '-') { // text automaton
      
      txtauto = *argv;

    } else {

      if (strcmp(*argv, "-l") == 0) { // file of compiled grammar names

	argv++, argc--;
	if (argc == 0) { fatal_error("-l needs an arg\n"); }
	langname = *argv;

      } else if (strcmp(*argv, "-h") == 0) {

        usage();
	return 0;

      }	else { fatal_error("unknow arg: '%s'\n", *argv); }

    }

    argv++, argc--;
  }	

  if (! langname) { fatal_error("no tagset specified\n"); }
  if (txtauto == NULL) { fatal_error("no text automaton specified\n"); }


  char bak[MAX_PATH];

  strcpy(bak, txtauto);
  strcat(bak, ".bak");

  printf("copying %s to %s ...\n", txtauto, bak);
  copy_file(bak, txtauto);

  printf("loading %s langage definition ...\n", langname);

  language_t * lang = language_load(langname);
  set_current_language(lang);

  fst_file_in_t * txtin = fst_file_in_open(bak, FST_TEXT);

  if (txtin == NULL) { fatal_error("tagsetnorm: unable to load text '%s'\n", bak); }

  fst_file_out_t * txtout = fst_file_out_open(txtauto, FST_TEXT);

  if (txtout == NULL) { fatal_error("unable to open '%s' for writing\n", txtauto); }

  autalmot_t * A;

  printf("cleaning text fsa\n");


  unichar EMPTY[] = { 'E', 'M', 'P', 'T', 'Y', 0 };
  symbol_t * symb = symbol_unknow_new(lang, language_add_form(lang, EMPTY));

  int no = 0;
  while ((A = fst_file_autalmot_load_next(txtin)) != NULL) {
    
    autalmot_emonde(A);

    if (A->nbstates == 0) {

      warning("sentence %d is empty\n", no + 1);
      
      int q0 = autalmot_add_state(A, AUT_INITIAL);
      int q1 = autalmot_add_state(A, AUT_FINAL);

      autalmot_add_trans(A, q0, symb, q1);
    }


    fst_file_write(txtout, A);
    autalmot_delete(A);
    no++;
    if (no % 100 == 0) { printf("sentence %d/%d ...      \r", no, txtin->nbelems); }
  }
  printf("sentence %d/%d.\ndone. text automaton is normalised.\n", txtin->nbelems, txtin->nbelems);

  fst_file_close(txtin);
  fst_file_close(txtout);

  return 0;
}




