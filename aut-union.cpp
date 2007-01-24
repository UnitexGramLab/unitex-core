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


static inline transition_t * _trans_rewrite(transition_t * trans, int * corresp) {
  for (transition_t * t = trans; t; t = t->next) { t->to = corresp[t->to]; }
  return trans;
}


autalmot_t * autalmot_union(autalmot_t * A, autalmot_t * B) {

  autalmot_resize(A, A->nbstates + B->nbstates);

  int * corresp = (int *) xmalloc(B->nbstates * sizeof(int));

  int q;
  for (q = 0; q < B->nbstates; q++) { corresp[q] = autalmot_add_state(A, B->states[q].flags); }

  for (q = 0; q < B->nbstates; q++) {
    A->states[corresp[q]].trans = _trans_rewrite(B->states[q].trans, corresp);
    B->states[q].trans = NULL;
    A->states[corresp[q]].defto = (B->states[q].defto != -1) ? corresp[B->states[q].defto] : -1;
  }

  free(corresp);

  autalmot_delete(B);

  return A;
}

