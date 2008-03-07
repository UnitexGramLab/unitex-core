 /*
  * Unitex
  *
  * Copyright (C) 2001-2008 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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


void autalmot_tri_topo(Fst2Automaton * A) {

  int * entrants = (int *) xmalloc(A->nbstates * sizeof(int));

  int i;
  for (i = 0; i < A->nbstates; i++) { entrants[i] = 0; }

  for (i = 0; i < A->nbstates; i++) {
    for (transition_t * t = A->states[i].trans; t; t = t->next) { entrants[t->to]++; }
    if (A->states[i].defto != -1) { entrants[A->states[i].defto]++; }
  }

  int * dico = (int *) xmalloc(A->nbstates * sizeof(int));

  int q;
  for (q = 0; q < A->nbstates; q++) {

    int old = 0;

    while (entrants[old] != 0) { old++; }

    dico[old]     =  q;
    entrants[old] = -1;

    for (transition_t * t = A->states[old].trans; t; t = t->next) { entrants[t->to]--; }
  }


  state_t * nouvo = (state_t *) xmalloc(A->nbstates * sizeof(state_t));

  for (q = 0; q < A->nbstates; q++) {

    nouvo[dico[q]].trans = A->states[q].trans;

    for (transition_t * t = nouvo[dico[q]].trans; t; t = t->next) { t->to = dico[t->to]; }

    int defto = A->states[q].defto;

    nouvo[dico[q]].defto = (defto == -1) ? -1 : dico[defto];

    nouvo[dico[q]].flags = A->states[q].flags;
  }


  for (i = 0; i < A->nbinitials; i++) { A->initials[i] = dico[A->initials[i]]; }

  free(A->states);
  A->states = nouvo;

  A->size = A->nbstates;
}
