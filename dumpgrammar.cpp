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

#include "list_aut.h"
#include "entrsort.h"
#include "fst2autalmot.h"
#include "utils.h"


int main(int argc, char ** argv) {

  if (argc < 2) { die("usage: %s <txtauto>\n", *argv); }

  argv++, argc--;

  tAutAlMot * grm = load_grammar_automaton(*argv);

  autalmot_dump_plain(grm);
  autalmot_dump_dot_fname(grm, "grammar.dot");

  return 0;
}
