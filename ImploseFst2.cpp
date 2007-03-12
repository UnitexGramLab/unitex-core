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

#include "Unicode.h"
#include "Copyright.h"
#include "utils.h"
#include "autalmot_old.h"
#include "fst2autalmot.h"
#include "implose.h"
#include "IOBuffer.h"


void usage() {
u_printf("%S", COPYRIGHT);
u_printf("Usage: ImploseFst2 <txtauto> -o <out>\n"
         "\n"
         "where :\n"
         " <txtauto>     : input text automaton FST2 file,\n"
         " <out>         : resulting text automaton\n"
         "\n"
         "\"Implose\" the specified text automaton by merging together lexical entries\n"
         "which differ only in their flexional features.\n"
         "The resulting text automaton is stored in <out>.\n\n");
}


int main(int argc, char ** argv) {

  setBufferMode();  

  char * txtname = NULL, * outname = NULL;

  argv++, argc--;

  if (argc == 0) { usage(); return 0; }

  while (argc) {

    if (**argv != '-') {

      txtname = *argv;

    } else {

      if (strcmp(*argv, "-h") == 0) {

	usage();
        return 0;

      } else if (strcmp(*argv, "-o") == 0) {

	*argv++, argc--;
	if (argc == 0) { fatal_error("'-o' needs an additionnal argument\n"); }

	outname = *argv;
      }
    }

    *argv++, argc--;
  }

  if (txtname == NULL) { fatal_error("no text automaton specified\n"); }

  
  if (outname == NULL) {

    int len = strlen(txtname);
    outname = (char *) malloc((len + 10) * sizeof(char));

    strcpy(outname, txtname);

    if (strcmp(txtname + len - 5, ".fst2") == 0) {
      strcpy(outname + len - 5, "-imp.fst2");
    } else {
      strcat(outname, "-imp.fst2");
    }
  }


  u_printf("loading '%s'\n", txtname);

  list_aut_old * txtauto = load_text_automaton(txtname, false);

  if (txtauto == NULL) { fatal_error("unable to load '%s'\n", txtname); }


  u_printf("implosion ....\n");

  for (int i = 0; i < txtauto->nb_aut; i++) {
    //debug("%d/%d\n", i, txtauto->nb_aut);
    implose(txtauto->les_aut[i]); 
  }

  if (text_output_fst2_fname(txtauto, outname) == -1) {
    fatal_error("unable to implose fst in '%s'\n", outname);
  }
  list_aut_old_delete(txtauto);

  u_printf("done. '%s' implosed in '%s'.\n", txtname, outname);

  return 0;
}


