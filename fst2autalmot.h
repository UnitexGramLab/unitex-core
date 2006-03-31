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

#ifndef _FST2AUTALMOT_H_
#define _FST2AUTALMOT_H_

#include "Fst2.h"
#include "autalmot_old.h"

tAutAlMot * fst2AutAlMot(Automate_fst2 * A, int nb, tAlphMot * alphabetLu);

list_aut_old  * load_text_automaton(char * fname, bool developp = true);
tAutAlMot * load_grammar_automaton(char * fname);

void text_output_fst2(list_aut_old * txt, FILE * f);
int  text_output_fst2_fname(list_aut_old * txt, char * fname);

#endif
