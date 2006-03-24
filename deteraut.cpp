 /*
  * Unitex
  *
  * Copyright (C) 2001-2006 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "autalmot.h"
#include "deter.h"
#include "fst2autalmot.h"
#include "utils.h"

int main(int argc, char ** argv) {

  argv++, argc--;

  if (argc < 1) { die("bad args\n"); }

  tAutAlMot * a1, * a2; //, * a3;

  if ((a1 = load_grammar_automaton(*argv)) == NULL) { die("unable to open %s\n", *argv); }


  a1->initial[0] = 4;
  a1->type[0] &= ~(AUT_INITIAL);
  a1->type[4] |= ~(AUT_INITIAL);
  
  //  argv++, argc--; 
  //  if ((a2 = load_grammar_automaton(*argv)) == NULL) { die("unable to open %s\n", *argv); }

  argv++, argc--;

  a2 = deterCompl(a1);

  char * outname = argc ? *argv : (char *) "deter-out.fst2";

  autalmot_dump_fst2_fname(a2, outname);

  printf("determinisation in %s\n", outname);

  return 0;
}
