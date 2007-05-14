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

#include "utils.h"
#include "aut-alphabet.h"

alphabet_t * alphabet_new() {

  alphabet_t * alph = (alphabet_t *) xmalloc(sizeof(alphabet_t));

  alph->ustr = new_Ustring();
  alph->hash  = hash_str_table_new();

  return alph;
}

void alphabet_delete(alphabet_t * alph) {

  free_Ustring(alph->ustr);
  hash_str_table_delete(alph->hash);

  free(alph);
}


int alphabet_lookup(alphabet_t * alph, symbol_t * s) {
  symbol_to_str(s, alph->ustr);
  return hash_str_table_idx_lookup(alph->hash, alph->ustr);
}


int alphabet_add(alphabet_t * alph, symbol_t * s) {
  symbol_to_str(s, alph->ustr);
  return hash_str_table_add(alph->hash, alph->ustr, s);
}


alphabet_t * alphabet_from_autalmot(autalmot_t * A) {

  alphabet_t * alph = alphabet_new();
  bool transdef = false;

  for (int i = 0; i < A->nbstates; i++) {

    for (transition_t * t = A->states[i].trans; t; t = t->next) {
      alphabet_add(alph, t->label);
    }

    if (!transdef && A->states[i].defto != -1) {
      transdef = true;
      alphabet_add(alph, SYMBOL_DEF);
    }
  }
  return alph;
}
