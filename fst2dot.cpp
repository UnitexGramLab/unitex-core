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
#include "Fst2.h"
#include "utils.h"
#include "unicode.h"

void fst2_output_dot(Fst2 * A, FILE * f) {


  fprintf(f, "# FST2 output\n\n");

  for (int i = 1; i <= A->number_of_graphs; i++) {

    int base = A->initial_states[i];

    fprintf(f,
	    "digraph G%d {\n"
	    "  graph [ center = true, orientation = landscape, rankdir = LR ];\n"
	    "  node  [ shape  = circle ];\n\n", i);

    for (int q = 0; q < A->number_of_states_by_graphs[i]; q++) {

      int qq = base + q;

      fprintf(f, "\n  %d [ label=\"%d\" ", q, q);      
      if (is_final_state(A->states[qq])) { fprintf(f, "shape=\"doublecircle\" "); }
      fprintf(f, "];\n");

      for (struct fst2Transition * trans = A->states[qq]->transitions; trans; trans = trans->next) {
	i_fprintf(f, "  %d -> %d [ label=\"%S\" ];\n", q, trans->state_number - base, A->tags[trans->tag_number]->input);
      }
    }
    fprintf(f, "}\n\n");
  }
}



int main(int argc, char ** argv) {

  char * progname = *argv;

  argv++, argc--;

  if (argc < 1) {
    fprintf(stderr, "usage: %s <auto>\n", progname); 
    exit(1);
  }

  char * fname = *argv;

  Fst2 * A = load_fst2(fname, 1);

  if (A == NULL) { die("cannot load %s\n", fname); }

  fst2_output_dot(A, stdout);
  return 0;
}
