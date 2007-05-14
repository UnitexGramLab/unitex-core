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
#include "symbol_op.h"
#include "state_ens.h"




inline stateid_t * stateid_new(autalmot_t * A, int no, stateid_t * next) {
  stateid_t * id = (stateid_t *) xmalloc(sizeof(stateid_t));
  id->A    = A;
  id->no   = no;
  id->next = next;
  return id;
}


inline void stateid_delete(stateid_t * id) { free(id); }




state_ens_t * state_ens_new() {

  state_ens_t * res = (state_ens_t *) xmalloc(sizeof(state_ens_t));

  res->first = NULL;
  res->size  = 0;

  return res;
}


void state_ens_delete(state_ens_t * ens) {

  while (ens->first) {
    stateid_t * next = ens->first->next;
    stateid_delete(ens->first);
    ens->first = next;
  }

  free(ens);
}


state_ens_t * state_ens_dup(const state_ens_t * ens) {

  if (ens == NULL) { return NULL; }

  state_ens_t * res = state_ens_new();

  for (stateid_t * id = ens->first; id; id = id->next) {
    state_ens_add(res, id->A, id->no);
  }


  return res;
}


void state_ens_add(state_ens_t * E, autalmot_t * A, int no) {

  if (E->size == 0) {
    E->first = stateid_new(A, no, NULL);
    E->size++;
    return;
  }

  if (no == E->first->no) { return; }

  if (no < E->first->no) {

    E->first = stateid_new(A, no, E->first);

  } else {

    stateid_t * id;
    for (id = E->first; id->next && (id->next->no < no); id = id->next);

    if (id->next && (id->next->no == no)) { return; }

    id->next = stateid_new(A, no, id->next);
  }

  E->size++;

}


bool state_ens_equals(state_ens_t * E1, state_ens_t * E2) {

  if (E1 == E2) { return true; }

  if (E1->size  != E2->size) { return false; }

  stateid_t * id1, * id2;

  for (id1 = E1->first, id2 = E2->first; id1 && id2; id1 = id1->next, id2 = id2->next) {
    if ((id1->A != id2->A) || (id1->no != id2->no)) { return false; }
  }

  return (id1 == id2); // == NULL
}




TRANS_t * TRANS_new(symbol_t * s, TRANS_t * next) {
  TRANS_t * T = (TRANS_t *) xmalloc(sizeof(TRANS_t));
  T->label = s;
  T->to = state_ens_new();
  T->next = next;
  return T;
}


void TRANS_delete(TRANS_t * T) {
  state_ens_delete(T->to);
  free(T);
}

void TRANSs_delete(TRANS_t * T) {
  while (T) {
    TRANS_t * tmp = T->next;
    TRANS_delete(T);
    T = tmp;
  }
}


TRANS_t * TRANS_lookup(TRANS_t * T, symbol_t * s) {

  while (T) {
    if (symbol_compare(T->label, s) == 0) { break; }
    T = T->next;
  }
  return T;
}


state_ens_tab_t * state_ens_tab_new(int size) {

  state_ens_tab_t * res = (state_ens_tab_t *) xmalloc(sizeof(state_ens_tab_t));

  if (size <= 0) { size = 1; }

  res->tab = (state_ens_t **) xmalloc(sizeof(state_ens_t *) * size);
  res->tabsize = size;
  res->nbelems = 0;

  return res;
}


void state_ens_tab_delete(state_ens_tab_t * tab) {

  for (int i = 0; i < tab->nbelems; i++) { state_ens_delete(tab->tab[i]); }
  free(tab->tab);
  free(tab);
}


int state_ens_tab_add(state_ens_tab_t * tab, state_ens_t * E) {

  while (tab->nbelems >= tab->tabsize) {
    tab->tabsize = tab->tabsize * 2;
    tab->tab     = (state_ens_t **) xrealloc(tab->tab, tab->tabsize * sizeof(state_ens_t *));
  }

  tab->tab[tab->nbelems++] = state_ens_dup(E);
  return tab->nbelems - 1;
}


int state_ens_tab_lookup(state_ens_tab_t * tab, state_ens_t * E) {

  for (int i = 0; i < tab->nbelems; i++) {
    if (state_ens_equals(tab->tab[i], E)) { return i; }
  }
  return -1;
}




/* devoloppementd des symboles et transitions */


static inline void replace_symbol(symbol_t * a, symbol_t * b) {

  symbol_t * next = a->next;

  empty_symbol(a);
  copy_symbol(a, b);
  free_symbol(b);

  concat_symbols(a, next);
}



/* compare a et b et les developpe si necessaire
 * quand on developpe un symbole on le remplace par une suite de symboles
 * 2 a 2 disjoints dont l'union constitue le symbole d'origine
 */


static void symbol_dev_symbol(symbol_t * a, symbol_t * b) {

  //  debug("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
  //  debug("symbol_dev_symbol: too much check\n");

  //  errprintf("A= '"); symbol_dump(a); errprintf("' B = '"); symbol_dump(b); errprintf("'\n");

  if (symbol_compare(a, b) == 0) { /*errprintf("SYMBOLS EQUALS!\n"); */ return; }

  symbol_t * i = symbol_inter_symbol(a, b);

  //  errprintf("I = '"); symbol_dump(i); errprintf("'\n");

  if (i == NULL) { /* errprintf("SYMBOL DISJOINTS!\n");*/ return; } // symbol disjoints

  if (i->next) { fatal_error("symbol_developp: i->next\n"); }
 
  symbol_t * aminusb = symbol_minus_symbol(a, i);
  //  errprintf("A \\ B = "); symbols_dump(aminusb); endl();

  symbol_t * bminusa = symbol_minus_symbol(b, i);
  //  errprintf("B \\ A = "); symbols_dump(bminusa); endl();

  if (! aminusb) {
    if (symbol_compare(a, i)) {
      fatal_error("A != I et A \\ I = null\n");
    }
  }

  if (! bminusa) {
    if (symbol_compare(b, i)) {
      fatal_error("B != I et B \\ I = null\n");
    }
  }


  symbol_t * nouvo = dup_symbol(i);

  concat_symbols(nouvo, aminusb);

  replace_symbol(a, nouvo);

  concat_symbols(i, bminusa);

  replace_symbol(b, i);

}


static inline void symbol_dev_symbols(symbol_t * a, symbol_t * b) {
  while (b) {
    symbol_dev_symbol(a, b);
    b = b->next;
  }
}


static inline void symbols_developp(symbol_t * a, symbol_t * b) {
  while (a) {
    symbol_dev_symbols(a, b);
    a = a->next;
  }
}


static inline void symbols_developp(symbol_t * a) {
  if (a == NULL) { return; }
  while (a->next) { symbol_dev_symbols(a, a->next); a = a->next; }
}


/* like transition_new but don't dup symbol */

static transition_t * _trans_new(int to, symbol_t * label, transition_t * next) {

  transition_t * trans = (transition_t *) xmalloc(sizeof(transition_t));

  trans->to    = to;
  trans->label = label;
  trans->next  = next;

  return trans;
}


static void trans_flatten(transition_t * trans) {

  while (trans) {

    if (trans->label == NULL) { fatal_error("trans_flatten: NULL labeled transition\n"); }

    if (trans->label->next) {
      trans->next = _trans_new(trans->to, trans->label->next, trans->next);
      trans->label->next = NULL;
    }

    trans = trans->next;
  }
}


/* lookup for a transition labeled s */

static inline transition_t * trans_lookup(transition_t * trans, symbol_t * s) {

  while (trans) {
    if (symbol_compare(trans->label, s) == 0) { break; }
  }
  return trans;
}


/*inline*/ void trans_developp(transition_t * _t1, transition_t * _t2) {

  for (transition_t * t1 = _t1; t1; t1 = t1->next) {
    for (transition_t * t2 = _t2; t2; t2 = t2->next) {
      //      debug("DEVEL "); symbol_dump(t1->label); errprintf("x"); symbol_dump(t2->label); endl();
      symbols_developp(t1->label, t2->label);
    }
  }

  trans_flatten(_t1);
  trans_flatten(_t2);
}




void trans_developp(transition_t * trans) {

  transition_t * t1, * t2;

  //  debug("trans_developp("); transitions_dump(trans); errprintf(")\n");

  for (t1 = trans; t1; t1 = t1->next) { symbols_developp(t1->label); }

  for (t1 = trans; t1; t1 = t1->next) {
    for (t2 = t1->next; t2; t2 = t2->next) {
      symbols_developp(t1->label, t2->label);
    }
  }

  trans_flatten(trans);
}



void trans_developp(state_t * q1, state_t * q2) {

  //  debug("DEVEL %d   %d +++++++\n", q1, q2);

  trans_developp(q1->trans, q2->trans);

  if (q1->defto != -1) { // q1 a une transition par defaut

    /* pour chaque symbole s en partance de q2 mais pas de q1
     * on rajoute la transition (q1, s, q1->defto)
     */

    for (transition_t * t = q2->trans; t; t = t->next) {
      if (trans_lookup(q1->trans, t->label) == NULL) {
	q1->trans = transition_new(q1->defto, t->label, q1->trans);
      }
    }
  }

  if (q2->defto != -1) { // q2 a une transition par defaut

    /* idem
     */

    for (transition_t * t = q1->trans; t; t = t->next) {
      if (trans_lookup(q2->trans, t->label) == NULL) {
	q2->trans = transition_new(q2->defto, t->label, q2->trans);
      }
    }
  }
}


// STATE


static void trans_developp(STATE_t * Q) {

  /* developpe les transitions */

  for (stateid_t * id = Q->id->first; id; id = id->next) {
    state_t * q = id->A->states + id->no;

    //    errprintf("\n<<<<<<<<<<<<<<<<<<< q=%d\n", id->no);
    //    debug("trans_devel : "); transitions_dump(q->trans); endl(); endl();

    trans_developp(q->trans);

    //    debug("after devel: "); transitions_dump(q->trans); endl(); endl();
    //    errprintf(">>>>>>>>>>>>>>>>>>>>\n\n\n");
  }

  for (stateid_t * id1 = Q->id->first; id1; id1 = id1->next) {
    for (stateid_t * id2 = id1->next; id2; id2 = id2->next) {

      //      debug("DEVEL %dx%d\n", id1->no, id2->no);

      state_t * q1 = id1->A->states + id1->no;
      state_t * q2 = id2->A->states + id2->no;

      trans_developp(q1, q2);
    }
  }
  /* tous les symboles sont developpes */  
}





/* les transitions doivent etre developpees */

static void make_TRANS(STATE_t * Q) {

  //  debug("make_TRANS\n");

  for (stateid_t * id = Q->id->first; id; id = id->next) {

    //    debug("id=%d\n", id->no);

    state_t * q = id->A->states + id->no;

    if (q->defto != -1) { state_ens_add(Q->transdef, id->A, q->defto); }

    for (transition_t * t = q->trans; t; t = t->next) {

      TRANS_t * T;
      if ((T = TRANS_lookup(Q->trans, t->label)) == NULL) { // new symbol
	//	debug("new symmbol: "); symbols_dump(t->label); endl();
	T = TRANS_new(t->label, Q->trans);
	Q->trans = T;
      }

      state_ens_add(T->to, id->A, t->to);
    }
  }
}




STATE_t * STATE_new(state_ens_t * ens) {

  //  debug("STATE_new("); state_ens_dump(id); errprintf(")\n");

  STATE_t * res = (STATE_t *) xmalloc(sizeof(STATE_t));

  res->id = state_ens_dup(ens);

  res->trans = NULL;
  res->transdef = state_ens_new();

  /* flags stuffs
   * tous les etats sont initiaux => Q est initial
   * un des etats est terminal    => Q est terminal
   */

  res->flags = AUT_INITIAL;

  for (stateid_t * id = res->id->first; id; id = id->next) {
    if (!(id->A->states[id->no].flags & AUT_INITIAL)) { res->flags &= ~(AUT_INITIAL); }
    if (id->A->states[id->no].flags & AUT_TERMINAL)   { res->flags |= AUT_TERMINAL;   }
  }


  /* developpe les transitions dans l'automate original */

  /*
  debug("before trans_devel :\n");
  for (stateid_t * id = res->id->first; id; id = id->next) {
    debug("%d: ", id->no); transitions_dump(id->A->states[id->no].trans); endl();
  }
  */
  trans_developp(res);

  /*
  debug("after trans_devel :\n");
  for (stateid_t * id = res->id->first; id; id = id->next) {
    debug("%d: ", id->no); transitions_dump(id->A->states[id->no].trans); endl();
  }
  */

  make_TRANS(res);

  /*
  debug("after make_TRANS:\n");
  TRANSs_dump(res->trans); endl();
  */

  return res;
}


void STATE_delete(STATE_t * Q) {
  state_ens_delete(Q->id);
  TRANSs_delete(Q->trans);
  state_ens_delete(Q->transdef);
  free(Q);
}




