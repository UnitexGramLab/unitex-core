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
//---------------------------------------------------------------------------
#ifndef SingleGraphH
#define SingleGraphH
//---------------------------------------------------------------------------

#include "Fst2.h"


/** 
 * This structure represents a state of a single graph of a Fst2. The difference with
 * Fst2State is that it contains a list of reversed transitions, which is useful
 * for things like removing useless states.
 */
struct single_graph_state_ {
   /* Control byte used to know if the state is final */
   unsigned char control;
   /* List of outgoing transitions */
   Fst2Transition outgoing_transitions;
   /* List of reverted incoming transitions, i.e. if there is
    * a transition from A to B with the tag 56, then the transition
    * (56,A) will belong to B's reverted transitions */
   Fst2Transition reverted_incoming_transitions;
};
typedef struct single_graph_state_* SingleGraphState;



/**
 * Structure to hold an automaton (subgraph) when compiling and flattening.
 *
 * In opposite to struct fst2 (defined in Fst2.h) only one automaton
 * (subgraph) is represented.  It uses also a different (smarter and
 * smaller) representation of states, see SingleGraphState.
 */
struct single_graph_ {
   /* Array containing the states of the graph */
   SingleGraphState* states;
   /* Capacity of this array */
   int capacity;
   /* Actual size of this array */
   int number_of_states;
};
typedef struct single_graph_* SingleGraph;



SingleGraph new_SingleGraph();
void free_SingleGraph(SingleGraph);
int is_final_state(SingleGraphState);
void set_final_state(SingleGraphState);
int is_initial_state(SingleGraphState);
void set_initial_state(SingleGraphState);
int is_accessible_state(SingleGraphState);
void set_accessible_state(SingleGraphState);
int is_co_accessible_state(SingleGraphState);
void set_co_accessible_state(SingleGraphState);
SingleGraphState new_SingleGraphState();
void free_SingleGraphState(SingleGraphState);
void add_outgoing_transition(SingleGraphState,int,int);
void set_state_array_capacity(SingleGraph,int);
SingleGraphState add_state(SingleGraph);
void move_SingleGraph(SingleGraph,SingleGraph*);
void compute_reverse_transitions(SingleGraph);
void check_accessibility(SingleGraphState*,int);
void check_co_accessibility(SingleGraphState*,int);
void remove_epsilon_transitions(SingleGraph);
void remove_useless_states(SingleGraph);

#endif

