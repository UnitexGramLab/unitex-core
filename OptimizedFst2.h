/*
 * Unitex
 *
 * Copyright (C) 2001-2016 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "OutputTransductionVariables.h"
#include "MetaSymbols.h"
#include "Transitions.h"
#include "Contexts.h"
#include "AbstractAllocator.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * This library builds optimized versions of fst2 by grouping
 * different types of transitions together. The main points here are:
 *
 * 1) Test the input only once if you have same input with different outputs.
 *    Note that this practice should be discouraged anyway.
 *
 * 2) Replaces all transitions containing a pattern with a lemma by the
 *    comprehensive list of tokens that can match it, including case variants.
 *
 * 3) Discard lexical transitions that cannot match any token in the text.
 *
 * Note that steps 2 & 3 can be done safely even if the morphological mode is used,
 * because for that mode, the original fst2 states are used, unmodified.
 *
 * After step 3 is complete, some optimized states may have no outgoing transition
 * at all. We remove recursively all transitions to such states. If the optimized graph
 * becomes then empty, we also remove from all optimized states all transitions tagged
 * by this graph call, which can also lead to useless states with no outgoing transitions.
 * The process is repeated until there is no more removal.
 */


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
  /* This extra field is only relevant for the $< tag. It is
   * used to know where the matching $> tags are.
   */
  Transition* morphological_mode_ends;
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
 * This structure defines an optimized state of a fst2. All transitions that outgo
 * the original fst2 state are grouped here by kind.
 */
struct optimizedFst2State {
  int original_fst2_state;
  unsigned char control;
  struct opt_graph_call* graph_calls;
  struct opt_meta* metas;
  struct opt_pattern* patterns;
  struct opt_pattern* compound_patterns;
  struct opt_token* token_list;
  struct opt_variable* input_variable_starts;
  struct opt_variable* input_variable_ends;
  struct opt_variable* output_variable_starts;
  struct opt_variable* output_variable_ends;
  struct opt_contexts* contexts;
  /* The following structure will contain all the
   * transitions as they were before the optimization, because
   * all those transitions will be needed by
   * the morphological mode
   */
  struct opt_graph_call* unoptimized_graph_calls;
  struct opt_meta* unoptimized_metas;
  struct opt_variable* unoptimized_input_variable_starts;
  struct opt_variable* unoptimized_input_variable_ends;
  struct opt_variable* unoptimized_output_variable_starts;
  struct opt_variable* unoptimized_output_variable_ends;

  int* tokens;
  int number_of_tokens;
  Transition** token_transitions;

  int graph_number;
  int pos_transition_in_graph;
  int pos_transition_in_fst2;
};

typedef struct optimizedFst2State* OptimizedFst2State;


OptimizedFst2State* build_optimized_fst2_states(Variables*,OutputVariables*,Fst2*,Abstract_allocator);
void free_optimized_states(OptimizedFst2State*,int,Abstract_allocator);

} // namespace unitex

#endif

