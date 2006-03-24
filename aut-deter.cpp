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

#include <assert.h>

#include "autalmot.h"
#include "utils.h"
#include "state_ens.h"
#include "fst_file.h"



void autalmot_determinize(autalmot_t * A) {

  //  debug("entering determinize: aut = \n");
  //  autalmot_dump(A);
  //  autalmot_output_fst2(A, "deter-in.fst2", FST_GRAMMAR);

  autalmot_t * res = autalmot_new();

  state_ens_tab_t * TAB = state_ens_tab_new();

  state_ens_t * inits  = state_ens_new();
  int i;
  for (i = 0; i < A->nbinitials; i++) {
    state_ens_add(inits, A, A->initials[i]);
  }

  //  debug("inits = "); state_ens_dump(inits); endl();
 
  state_ens_tab_add(TAB, inits);

  state_ens_delete(inits);

  for (int curr = 0; curr < TAB->nbelems; curr++) {

    //  debug("curr = %d tabsize=%d ens=", curr, TAB->nbelems); state_ens_dump(TAB->tab[curr]); endl();

    STATE_t * Q = STATE_new(TAB->tab[curr]);

    int q = autalmot_add_state(res, Q->flags);


    for (TRANS_t * T = Q->trans; T; T = T->next) {

      //      debug("new trans:\n"); TRANS_dump(T); endl();

      int idx = state_ens_tab_lookup(TAB, T->to);

      if (idx == -1) { // new state
	idx = state_ens_tab_add(TAB, T->to);
      }

      autalmot_add_trans(res, q, T->label, idx);
    }


    if (Q->transdef->size) {

      int idx = state_ens_tab_lookup(TAB, Q->transdef);

      if (idx == -1) { // new state
	idx = state_ens_tab_add(TAB, Q->transdef);
      }

      autalmot_add_trans(res, q, SYMBOL_DEF, idx);      
    }

    STATE_delete(Q);
  }

  // done with determinization ???

  state_ens_tab_delete(TAB);


  /* empty A */

  for (i = 0; i < A->nbstates; i++) { transitions_delete(A->states[i].trans); }
  free(A->states);

  free(A->initials);


  /* copy res */

  A->states   = res->states;
  A->nbstates = res->nbstates;
  A->size     = res->size;

  A->initials   = res->initials;
  A->nbinitials = res->nbinitials;
  free(res);

  for (i = A->nbinitials; i > 1; i--) { autalmot_unset_initial(A, A->initials[i-1]); }

  assert(A->nbinitials == 1 && A->initials[0] == 0);

  //  debug("out of determinize, aut = \n");  autalmot_dump(A);
}
