 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
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

#ifndef SingleGraphH
#define SingleGraphH

#include "Transitions.h"
#include "List_int.h"


/* Here are bit masks that can be used to mark states. Note that lower
 * bit masks are reserved. */
#define MARK1_BIT_MASK 16
#define MARK2_BIT_MASK 32
#define MARK3_BIT_MASK 64
#define MARK4_BIT_MASK 128


/**
 * This structure represents a state of a single graph of a fst2. The difference with
 * Fst2State is that it contains a list of reversed transitions, which is useful
 * for things like removing useless states.
 */
struct single_graph_state_ {
   /* Control byte used to know if the state is final */
   unsigned char control;

   /* List of outgoing transitions */
   Transition* outgoing_transitions;

   /* List of reverted incoming transitions, i.e. if there is
    * a transition from A to B with the tag 56, then the transition
    * (56,A) will belong to B's reverted transitions */
   Transition* reverted_incoming_transitions;

   /* This is the number of the state pointed out by the default transition,
    * or -1 if none. This field is used by the complementation algorithm. */
   int default_state;
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
   /* Indicates if transitions are tagged by integers or by pointers */
   TagType tag_type;
};
typedef struct single_graph_* SingleGraph;



SingleGraph new_SingleGraph(int,TagType);
SingleGraph new_SingleGraph(int);
SingleGraph new_SingleGraph(TagType);
SingleGraph new_SingleGraph();
void free_SingleGraph(SingleGraph,void(*f)(symbol_t*)=NULL);

int is_final_state(SingleGraphState);
void set_final_state(SingleGraphState);
void unset_final_state(SingleGraphState);
int is_initial_state(SingleGraphState);
void set_initial_state(SingleGraphState);
void unset_initial_state(SingleGraphState);
int is_accessible_state(SingleGraphState);
void set_accessible_state(SingleGraphState);
int is_co_accessible_state(SingleGraphState);
void set_co_accessible_state(SingleGraphState);
void reset_accessibility_info(SingleGraphState);

SingleGraphState new_SingleGraphState();
void free_SingleGraphState(SingleGraphState,void (*free_elag_symbol)(symbol_t*)=NULL);
void add_outgoing_transition(SingleGraphState,int,int);
void add_outgoing_transition(SingleGraphState,symbol_t*,int);
void add_all_outgoing_transitions(SingleGraphState,symbol_t*,int);
struct list_int* get_initial_states(SingleGraph);
int get_initial_state(SingleGraph);
void set_state_array_capacity(SingleGraph,int);
void resize(SingleGraph);
SingleGraphState add_state(SingleGraph);
void move_SingleGraph(SingleGraph,SingleGraph*,void (*free_elag_symbol)(symbol_t*)=NULL);

void compute_reverse_transitions(SingleGraph);
void check_accessibility(SingleGraphState*,int);
void check_co_accessibility(SingleGraphState*,int);
void remove_epsilon_transitions(SingleGraph,int);
void remove_useless_states(SingleGraph);
void reverse(SingleGraph);
SingleGraph clone(SingleGraph,symbol_t*(*clone_elag_symbol)(const symbol_t*)=NULL);

void trim(SingleGraph);
void determinize(SingleGraph);
void minimize(SingleGraph,int);
void topological_sort(SingleGraph);
void build_union(SingleGraph,SingleGraph);

//SingleGraph get_subgraph(Fst2*,int);
void save_fst2_subgraph(U_FILE*,SingleGraph,int,unichar*);

double evaluate_ambiguity(SingleGraph,int*,int*);

#endif

