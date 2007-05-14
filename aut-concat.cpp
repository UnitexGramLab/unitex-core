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
#include "autalmot.h"
#include "symbol_op.h"


static symbol_t * symbols_from_transs(transition_t * trans) {

  symbol_t res;
  res.next = NULL;
  symbol_t * end = & res;

  while (trans) {
    concat_symbols(end, dup_symbols(trans->label), & end);
    trans = trans->next;
  }

  return res.next;
}


void developp_deftrans(autalmot_t * A, int q) {
  
  if (A->states[q].defto == -1) { return; }

  symbol_t * tsymbs = symbols_from_transs(A->states[q].trans);
  symbol_t * defdev = minus_symbols(tsymbs);

  autalmot_add_trans(A, q, defdev, A->states[q].defto);

  free_symbols(tsymbs);
  free_symbols(defdev);
}


static inline transition_t * trans_rewrite(transition_t * trans, int * corresp) {
  for (transition_t * t = trans; t; t = t->next) { t->to = corresp[t->to]; }
  return trans;
}


void autalmot_concat(autalmot_t * A, autalmot_t * B) {

  int oldnb = A->nbstates;

  autalmot_resize(A, A->nbstates + B->nbstates);

  int * corresp = (int *) xmalloc(B->nbstates * sizeof(int));

  int q;
  for (q = 0; q < B->nbstates; q++) { corresp[q] = autalmot_add_state(A); }

  for (q = 0; q < B->nbstates; q++) {
    A->states[corresp[q]].trans = trans_rewrite(transitions_dup(B->states[q].trans), corresp);
    A->states[corresp[q]].defto = (B->states[q].defto != -1) ? corresp[B->states[q].defto] :  -1;
    A->states[corresp[q]].flags = autalmot_is_final(B, q) ? AUT_FINAL : 0;
  }

  for (int i = 0; i < B->nbinitials; i++) { developp_deftrans(A, corresp[B->initials[i]]); }

  for (q = 0; q < oldnb; q++) {

    if (autalmot_is_final(A, q)) {

      autalmot_unset_final(A, q);

      developp_deftrans(A, q);

      for (int i = 0; i < B->nbinitials; i++) {
	transitions_concat(& A->states[q].trans, transitions_dup(A->states[corresp[B->initials[i]]].trans));
	if (autalmot_is_final(A, corresp[B->initials[i]])) { autalmot_set_final(A, q); }
      }
    }
  }

  //  autalmot_delete(B);

  free(corresp);
}









#if 0
static void _aut_concat(autalmot_t * res, autalmot_t * a, int q, int * corresp) {

  if (autalmot_is_terminal(a, q)) { autalmot_set_terminal(res, corresp[q]); }

  for (transition_t * t = a->states[q].trans; t; t = t->next) {

    if (corresp[t->to] == -1) {

      corresp[t->to] = autalmot_add_state(res);

      _aut_concat(res, a, t->to, corresp);
    }

    autalmot_add_trans(res, corresp[q], t->label, corresp[t->to]);
  }

  if (a->states[q].defto != -1) {

  }

}



void autalmot_concat(autalmot_t * A, autalmot_t * B) {

  if (B->nbinitials != 1) { die("autalmot_concat: bad automaton (%d initial states)\n", a->nbinitials); }

  autalmot_resize(A, A->nbstates + B->nbstates);

  int * corresp = (int *) xmalloc(B->nbstates * sizeof(int));

  for (int i = 0; i < B->nbstates; i++) { corresp[i] = -1; }

  int oldnb = A->nbstates;

  for (int q = 0; q < oldnb; q++) {

    if (autalmot_is_final(A, q)) {

      if (A->states[q].defto != -1) { developp_deftrans(A->states + q); }

      autalmot_unset_final(A, q);

      corresp[B->initial[0]] = q;

      _aut_concat(A, B, B->initial[0], corresp);
    }
  }
}
#endif
