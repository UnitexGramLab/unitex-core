 /*
  * Unitex
  *
  * Copyright (C) 2001-2007 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "utils.h"
#include "autalmot.h"

#define ACCESS                    (1)
#define COACCESS        (ACCESS << 1)
#define USEFUL    (ACCESS | COACCESS)



static void fill_intrans(Fst2Automaton * A, transition_t ** intrans) {

  int q;
  for (q = 0; q < A->nbstates; q++) { intrans[q] = NULL; }

  for (q = 0; q < A->nbstates; q++) {   

    for (transition_t * t = A->states[q].trans; t; t = t->next) {
      intrans[t->to] = transition_new(q, NULL, intrans[t->to]);
    }

    if (A->states[q].defto != -1) { intrans[A->states[q].defto] = transition_new(q, NULL, intrans[A->states[q].defto]); }
  }
}


/* marque les etats depuis lesquels on peut acceder a q
 * T est un tableau de transitions inversees
 */

static void mark_coaccessibles(transition_t ** T, int q, int * access) {

  //  debug("%d is coaccessible\n", q);

  if (access[q] & COACCESS) { return; }

  access[q] |= COACCESS;

  for (transition_t * t = T[q]; t; t = t->next) { mark_coaccessibles(T, t->to, access); }
}


/* marque les etats accessibles depuis q */

static void mark_accessibles(Fst2Automaton * A, int q, int * access) {

  //  debug("%d is accessible\n", q);

  if (access[q] & ACCESS) { return; }

  access[q] |= ACCESS;

  for (transition_t * t = A->states[q].trans; t; t = t->next) { mark_accessibles(A, t->to, access); }

  if (A->states[q].defto != -1) { mark_accessibles(A, A->states[q].defto, access); }
}


/*
 * supprimes les transitions qui vont vers des etats inutiles
 * reecrit la destination des transitions restantes
 */

static transition_t * trans_clean(transition_t * trans, int * access) { 

  while (trans && access[trans->to] == -1) {
    transition_t * next = trans->next;
    transition_delete(trans);
    trans = next;
  }

  for (transition_t * t = trans; t; t = t->next) {

    t->to = access[t->to];

    while (t->next && access[t->next->to] == -1) {
      transition_t * next = t->next->next;
      transition_delete(t->next);
      t->next = next;
    }
  }

  return trans;
}



void elag_trim(Fst2Automaton * A) {

  transition_t ** intrans = (transition_t **) xmalloc(A->nbstates * sizeof(transition_t *));
  fill_intrans(A, intrans);

  int q;

  int access[A->nbstates];
  for (q = 0; q < A->nbstates; q++) { access[q] = 0; }


  for (q = 0; q < A->nbstates; q++) {
    if (autalmot_is_initial(A, q))  {  mark_accessibles(A, q, access); }
    if (autalmot_is_terminal(A, q)) {  mark_coaccessibles(intrans, q, access); }
  }

  /* delete intrans */

  for (q = 0; q < A->nbstates; q++) {
    transitions_delete(intrans[q]);
  }
  free(intrans);



  int nbuseful = 0;

  for (q = 0; q <A->nbstates; q++) { access[q] = (access[q] == USEFUL) ? nbuseful++ : -1; }


  /* acces[q] = -1 si q est inutile
   *          =  nouvel id de q dans l'automate resultat sinon */

  if (nbuseful == 0) {
    //    warning("emonde auto: no useful state.\n");
    autalmot_empty(A);
    return;
  }

  //  debug("emondation: %d/%d useful states\n", nbuseful, A->nbstates);

  if (nbuseful == A->nbstates) { return; }

  state_t * nouvo = (state_t *) xmalloc(nbuseful * sizeof(state_t));


  for (q = 0; q < A->nbstates; q++) {

    if (access[q] == -1) { // q is useless

      transitions_delete(A->states[q].trans);

    } else { // q is useful : new id = access[q]

      nouvo[access[q]].flags = A->states[q].flags;

      if (A->states[q].defto == -1) {
	nouvo[access[q]].defto = -1;
      } else {
	nouvo[access[q]].defto = access[A->states[q].defto];
      	assert(nouvo[access[q]].defto != -1); // why ????
      }
      nouvo[access[q]].trans = trans_clean(A->states[q].trans, access);
    }
  }

  free(A->states); // les transitions sont deja supprimees ou reccuperees

  A->states = nouvo;
  A->size = A->nbstates = nbuseful;

  for (int i = 0; i < A->nbinitials; i++) { A->initials[i] = access[A->initials[i]]; }

  
}


