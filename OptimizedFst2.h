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

#ifndef OptimizedFst2H
#define OptimizedFst2H

#include "Unicode.h"
#include "Fst2.h"
#include "TransductionVariables.h"
#include "MetaSymbols.h"
#include "Transitions.h"


/**
 * This structure defines a list of graph calls. For each call, we have the
 * graph number and the original fst2 transition with the original tag number
 * and the destination state.
 */
struct opt_graph_call {
   int graph_number;
   Transition* transition;
   struct opt_graph_call* next;
};


/**
 * This structure defines a list of metas. For each meta, we have the
 * meta, the negation mark, and the original fst2 transition with the
 * original tag number and the destination state.
 */
struct opt_meta {
  enum meta_symbol meta;
  char negation;
  Transition* transition;
  struct opt_meta* next;
};


/**
 * A pattern number, its negation mark and the transition that matches it.
 */
struct opt_pattern {
   int pattern_number;
   char negation;
   Transition* transition;
   struct opt_pattern* next;
};


/**
 * A token number and the transition that matches it.
 */
struct opt_token {
   int token_number;
   Transition* transition;
   struct opt_token* next;
};


/**
 * A variable start or end declaration.
 */
struct opt_variable {
   int variable_number;
   Transition* transition;
   struct opt_variable* next;
};


/**
 * This structure stores the information needed to deal with 
 * the context marks $[ $![ and $]
 * If the state has a transition with a positive context start mark $[, we compute
 * one time for all where the associated context end marks are. This is useful to
 * know where to go in the grammar when the context has been matched. The same is
 * done for the negative context mark, if any.
 */
struct opt_contexts {
   /* This is an array used to store both the positive context start marks and
    * their associated ending context marks.
    * 
    * positive_mark[i] will contain the context transition and positive_mark[i+1] will
    * contain the context end transitions. */
   Transition** positive_mark;
   /* Number of element of the previous array */
    int size_positive;
   /* The same as for positive context marks */
   Transition** negative_mark;
   int size_negative;
   Transition* end_mark;
};


/**
 * This structure defines an optimized state of a fst2. All transitions that outgo
 * the original fst2 state are grouped here by kind.
 */
struct optimizedFst2State {
  unsigned char control;
  struct opt_graph_call* graph_calls;
  struct opt_meta* metas;
  struct opt_pattern* patterns;
  struct opt_pattern* compound_patterns;
  struct opt_token* token_list;
  struct opt_variable* variable_starts;
  struct opt_variable* variable_ends;
  struct opt_contexts* contexts;
  int* tokens;
  int number_of_tokens;
  Transition** token_transitions;
};

typedef struct optimizedFst2State* OptimizedFst2State;


OptimizedFst2State* build_optimized_fst2_states(Variables*,Fst2*);
void free_optimized_states(OptimizedFst2State*,int);

#endif
