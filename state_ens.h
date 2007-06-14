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

#ifndef _STATE_ENS_H_
#define _STATE_ENS_H_

#include "symbol.h"
#include "autalmot.h"


typedef struct stateid_t {
  Fst2Automaton * A;
  int no;
  struct stateid_t * next;
} stateid_t;


typedef struct state_ens_t {
  stateid_t * first;
  int size;
} state_ens_t;


typedef struct TRANS_t {
  symbol_t    * label;  /* label est partagé */
  state_ens_t * to;
  TRANS_t     * next;
} TRANS_t;


typedef struct STATE_t {
  state_ens_t * id;
  int flags;
  TRANS_t * trans;
  state_ens_t * transdef; // transition par defaut
} STATE_t;



/* tableau d'ensemble d'états
 * == automate sur un ensemble d'etats
 */

typedef struct state_ens_tab_t {
  state_ens_t ** tab;
  int tabsize;
  int nbelems;
} state_ens_tab_t;


stateid_t * stateid_new(Fst2Automaton * A, int no, stateid_t * next = NULL);
void stateid_delete(stateid_t * id);


state_ens_t * state_ens_new();
void state_ens_delete(state_ens_t * l);
void state_ens_add(state_ens_t * l, Fst2Automaton * A, int no);
bool state_ens_equals(state_ens_t * l1, state_ens_t * l2);

void state_ens_developp(state_ens_t * ens);



TRANS_t * TRANS_new(symbol_t * s, TRANS_t * next = NULL);
void TRANS_delete(TRANS_t * T);
void TRANSs_delete(TRANS_t * T);
TRANS_t * TRANS_lookup(TRANS_t * T, symbol_t * label);

state_ens_tab_t * state_ens_tab_new(int size = 16);
void state_ens_tab_delete(state_ens_tab_t * tab);
int state_ens_tab_add(state_ens_tab_t * t, state_ens_t * l);
int state_ens_tab_lookup(state_ens_tab_t * tab, state_ens_t * l);


STATE_t * STATE_new(state_ens_t * ens);
void STATE_delete(STATE_t * Q);

void trans_developp(transition_t * t1, transition_t * t2);
void trans_developp(transition_t * t1);

void developp_deftrans(Fst2Automaton * A, int q);

#endif
