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

#ifndef PatternTreeH
#define PatternTreeH

#include "Unicode.h"
#include "List_ustring.h"
#include "Pattern.h"
#include "DELA.h"
#include "List_pointer.h"


/**
 * This structure defines a list of constraints that is associated to a given
 * grammatical/semantic code set.
 */
struct constraint_list {
   unichar* inflected;
   unichar* lemma;
   struct list_ustring* forbidden_codes;
   struct list_ustring* inflectional_codes;
   int pattern_number;
   struct constraint_list* next;
};


/**
 * As the list of grammatical/semantic codes is the only thing that is non optional,
 * we represent a set of patterns by a tree where each transition corresponds to a
 * grammatical/semantic code. Then, each node contains a list of constraints. Each
 * constraint is made of an optional inflected form, an optional lemma, an optional list
 * of forbidden grammatical/semantic codes and an optional list of inflectional codes.
 * Each such constraint is associated to a unique pattern number.
 * 
 * The following structure defines a node of a pattern tree.
 */
struct pattern_node {
   struct constraint_list* constraints;
   struct pattern_node_transition* sons;
};


/**
 * This structure defines a list of transitions in a pattern tree.
 */
struct pattern_node_transition {
   unichar* grammatical_code;
   struct pattern_node* node;
   struct pattern_node_transition* next;
};




struct pattern_node* new_pattern_node();
void free_pattern_node(struct pattern_node*);
int add_pattern(int*,struct pattern*,struct pattern_node*);                              
struct list_pointer* get_matching_patterns(struct dela_entry*,struct pattern_node*);


#endif

