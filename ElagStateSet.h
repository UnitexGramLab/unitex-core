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

#ifndef ElagStateSetH
#define ElagStateSetH

/**
 * This library is designed to manipulate set of ELAG grammars' states
 * and transitions during algorithms like determinization.
 */

#include "SingleGraph.h"
#include "symbol.h"


#define AUT_INITIAL    1
#define AUT_FINAL      2


/**
 * This structure represents a list of states. Each state
 * is characterized by the automaton it belongs to and its
 * state number in this automaton.
 */
typedef struct state_id_ {
   SingleGraph automaton;
   int state_number;
   struct state_id_* next;
} state_id;


/**
 * This structure represents a state set.
 */
typedef struct {
   /* The list of states that is supposed to be sorted by
    * increasing state numbers */
   state_id* state_list;
   /* The size of this list */
   int size;
} state_set;


/**
 * During the determinization, this structure represents a
 * transition list in the new automaton. It is tagged with a symbol_t*
 * and it points to a state set.
 */
typedef struct TRANS_t_ {
  symbol_t* label;
  state_set* destination;
  struct TRANS_t_* next;
} TRANS_t;


/**
 * During the determinization, this structure represents a state
 * of the new automaton that corresponds to a state set in the
 * original automaton.
 */
typedef struct {
   state_set* original_state_set;
   int flags;
   TRANS_t* transitions;
   state_set* default_transition;
} STATE_t;


/**
 * This structure represents an array containing state sets.
 */
typedef struct {
   state_set** state_sets;
   /* The number of state sets in the array */
   int size;
   /* The maximum size of the array */
   int capacity;
} state_set_array;




state_id* new_state_id(SingleGraph,int,state_id* next=NULL);
void free_state_id(state_id*);

state_set* new_state_set();
void free_state_set(state_set*);
void state_set_add(state_set*,SingleGraph,int);
bool state_set_equals(state_set*,state_set*);

TRANS_t* new_TRANS_t(symbol_t*,TRANS_t* next=NULL);
void free_TRANS_t(TRANS_t*);
void free_TRANS_t_list(TRANS_t*);
TRANS_t* TRANS_t_lookup(TRANS_t*,symbol_t*);

state_set_array* new_state_set_array(int capacity=16);
void free_state_set_array(state_set_array*);
int state_set_array_add(state_set_array*,state_set*);
int state_set_array_lookup(state_set_array*,state_set*);

STATE_t* new_STATE_t(state_set*);
void free_STATE_t(STATE_t*);

void flatten_transition(Transition*);
void expand_transitions(Transition*,Transition*);

void debug_print(state_set*);

#endif
