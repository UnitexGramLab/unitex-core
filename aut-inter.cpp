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
#include "symbol.h"
#include "symbol_op.h"
#include "autalmot.h"
#include "state_ens.h"

static inline transition_t * trans_extract(transition_t ** trans, symbol_t * s) {

  if (*trans == NULL) { return NULL; }


  if (symbol_compare(s, (*trans)->label) == 0) { // match with first trans
    transition_t * res = *trans;
    *trans = (*trans)->next;
    return res;
  }

  for (transition_t * t = *trans; t->next; t = t->next) {
    if (symbol_compare(s, t->next->label) == 0) {
      transition_t * res = t->next;
      t->next = t->next->next;
      return res;
    }
  }

  return NULL;
}



static inline transition_t * trans_pop(transition_t ** trans) {

  if (*trans == NULL) { return NULL; }

  transition_t * res = *trans;
  *trans = (*trans)->next;
  return res;
}



static int inter_state(Fst2Automaton * res, const Fst2Automaton * A, int q1,
                       const Fst2Automaton * B, int q2, int ** corresp) {

  if (corresp[q1][q2] != -1) { return corresp[q1][q2]; }

  int q = corresp[q1][q2] = autalmot_add_state(res);

  if ((A->states[q1].flags & AUT_INITIAL) && (B->states[q2].flags & AUT_INITIAL)) {
    autalmot_set_initial(res, q);
  }

  if ((A->states[q1].flags & AUT_TERMINAL) && (B->states[q2].flags & AUT_TERMINAL)) {
    autalmot_set_terminal(res, q);
  }

  transition_t * transA = transitions_dup(A->states[q1].trans);
  transition_t * transB = transitions_dup(B->states[q2].trans);

  trans_developp(transA, transB);

  int to;
  transition_t * transa;
  transition_t * transb;

  while ((transa = trans_pop(& transA))) {

    /* extract from transB the same labeled trans */

    transb = trans_extract(& transB, transa->label);

    if (transb) {

      to = inter_state(res, A, transa->to, B, transb->to, corresp);

      add_transition(res, q, transa->label, to);

      transition_delete(transb);

    } else {                            // transa has no equiv in transB

      if (B->states[q2].defto != -1) {  // if B[q2] has a defaut trans it matches with transa

	to = inter_state(res, A, transa->to, B, B->states[q2].defto, corresp);

	add_transition(res, q, transa->label, to);
      }
    }

    transition_delete(transa);
  }

  if (A->states[q1].defto != -1) { // if A[q1] has une transition par défaut
    
    while ((transb = trans_pop(& transB))) {
      // elle concorde avec toutes les trans restantes dans B[q2]

      to = inter_state(res, A, A->states[q1].defto, B, transb->to, corresp);
      add_transition(res, q, transb->label, to);

      transition_delete(transb);
    }

    if (B->states[q2].defto != -1) { // <def> trans
      res->states[q].defto = inter_state(res, A, A->states[q1].defto,
                                         B, B->states[q2].defto,
                                         corresp);
    }

  } else { transitions_delete(transB); }

  return q;
}


Fst2Automaton * autalmot_intersection(const Fst2Automaton * A, const Fst2Automaton * B) {
  
  if ((A->nbinitials > 1) || (B->nbinitials > 1)) {
    error("a nbstates=%d & b->nbstates=%d\n", A->nbinitials, B->nbinitials);
    fatal_error("autalmot_inter: non deterministic auto\n");
  }


  Fst2Automaton * res = new_Fst2Automaton(A->name ? A->name : B->name, A->nbstates * B->nbstates);

  if (A->nbinitials == 0 || B->nbinitials == 0) {
    //    warning("aut-inter: auto is void\n");
    return res;
  }

  int ** corresp = (int **) xmalloc(A->nbstates * sizeof(int *));

  int i;
  for (i = 0; i < A->nbstates; i++) {

    corresp[i] = (int *) xmalloc(B->nbstates * sizeof(int));

    for (int j = 0; j < B->nbstates; j++) { corresp[i][j] = -1; }
  }

  inter_state(res, A, A->initials[0], B, B->initials[0], corresp);

  autalmot_resize(res);

  for (i = 0; i < A->nbstates; i++) { free(corresp[i]); }

  free(corresp);

  return res;
}




static int interStateAtom(Fst2Automaton * res, const Fst2Automaton * A, int q1, const Fst2Automaton * B,
                          int q2, int ** corresp) {

  if (corresp[q1][q2] != -1) { return corresp[q1][q2]; }

  int q = corresp[q1][q2] = autalmot_add_state(res);

  if ((A->states[q1].flags & AUT_INITIAL) && (B->states[q2].flags & AUT_INITIAL)) {
    autalmot_set_initial(res, q);
  }

  if ((A->states[q1].flags & AUT_TERMINAL) && (B->states[q2].flags & AUT_TERMINAL)) {
    autalmot_set_terminal(res, q);
  }


  for (transition_t * t1 = A->states[q1].trans; t1; t1 = t1->next) {

    //debug("process :"); symbol_dump(t1->label); endl();

    if (t1->label->POS->ignorable) { // skip ignorable tokens
      //debug("skip ignorable :"); symbol_dump(t1->label); endl();
      //debug("IGNORABLE\n");
      int to = interStateAtom(res, A, t1->to, B, q2, corresp);
      add_transition(res, q, t1->label, to);
      continue;
      //debug("CONTINUE\n");
    }
 

    bool found = false;

    for (transition_t * t2 = B->states[q2].trans; t2 && ! found; t2 = t2->next) {

      //debug("t2="); symbol_dump(t2->label); endl();

      if (symbol_in_symbol(t1->label, t2->label)) {
        //debug("  symbols matches\n");
	if (found) {
	  error("interStateAtom: non deterministic automaton\n");
	}

	found = true;

	int to = interStateAtom(res, A, t1->to, B, t2->to, corresp);
	add_transition(res, q, t1->label, to);
      } //else { debug("  DONT MATCH\n"); }
    }

//#warning "should no have def trans ???"
    if (! found && B->states[q2].defto != -1) {   
      int to = interStateAtom(res, A, t1->to, B, B->states[q2].defto, corresp);
      add_transition(res, q, t1->label, to);
    }
  }

  return q;
}


Fst2Automaton * interAutAtome(const Fst2Automaton * A, const Fst2Automaton * B) {

  if ((A->nbinitials > 1) || (B->nbinitials > 1)) {
    error("a nbstates=%d & b->nbstates=%d\n", A->nbinitials, B->nbinitials);
    fatal_error("autalmot_interAutAtome: non deterministic auto\n");
  }

  Fst2Automaton * res = new_Fst2Automaton(A->name, A->nbstates * B->nbstates);

  if (A->nbinitials == 0 || B->nbinitials == 0) {
    error("interAutAtome: auto is void\n");
    return res;
  }

  int ** corresp = (int **) xmalloc(A->nbstates * sizeof(int *));

  int i;
  for (i = 0; i < A->nbstates; i++) {

    corresp[i] = (int *) xmalloc(B->nbstates * sizeof(int));

    for (int j = 0; j < B->nbstates; j++) { corresp[i][j] = -1; }
  }


  interStateAtom(res, A, A->initials[0], B, B->initials[0], corresp);

  autalmot_resize(res);

  for (i = 0; i < A->nbstates; i++) { free(corresp[i]); }

  free(corresp);

  //  debug("out of interAutAtom:\n"); autalmot_dump(res);
  //  autalmot_dump_dot_fname(res, "interAutAtom.out");

  return res;				     
}
