/*
 * Unitex
 *
 * Copyright (C) 2001-2013 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef TransitionsH
#define TransitionsH

#include "Symbol.h"
#include "Symbol_op.h"
#include "AbstractAllocator.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * This library provides functions and types for manipulating 
 * automaton transitions that can be tagged either by integers
 * or by pointers.
 */


/* This enumeration is used to indicate if we use transitions tagged
 * by integers or bu pointers */
typedef enum {
   INT_TAGS,
   PTR_TAGS
} TagType;


/*
 * This structure represents a transition list in an automaton.
 * Transitions can either be tagged by integers or elag symbols.
 */
struct transition_ {
   union {
      /* Number of the transition tag */
      int tag_number;
      
      /* Pointer label */
      symbol_t* label;
   };
   
   /*
    * Number of the state pointed by the transition. Note that, in a fst2, this
    * number is ABSOLUTE. For instance, if the subgraph number 3 starts
    * at the state number 45, the 6th state of this subgraph will have the
    * number 45+6=51.
    * 
    */
   int state_number;
   
   /* Next transition of the list */
   struct transition_* next;
};
typedef struct transition_ Transition;


Transition* new_Transition(int,int,Transition*,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
Transition* new_Transition(int,int,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
Transition* new_Transition(const symbol_t*,int,Transition*,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
Transition* new_Transition_no_dup(symbol_t*,int,Transition*,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
Transition* new_Transition(const symbol_t*,int,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
void free_Transition_list(Transition*,void(*)(symbol_t*),Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
void free_Transition_list(Transition*,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
void free_Transition(Transition*,void(*free_elag_symbol)(symbol_t*)=NULL,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
void add_transition_if_not_present(Transition**,int,int,Abstract_allocator prv_alloc);
void add_transition_if_not_present(Transition**,symbol_t*,int,Abstract_allocator prv_alloc);
Transition* clone_transition(const Transition*,symbol_t*(*)(const symbol_t*),Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
Transition* clone_transition_list(const Transition*,int*,symbol_t*(*)(const symbol_t*),Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
void concat(Transition**,Transition*);
void renumber_transitions(Transition*,int,int);
Transition* shift_destination_states(Transition*,int);
void add_transitions_int(Transition*,Transition**,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
void add_transitions_ptr(Transition*,Transition**,Abstract_allocator prv_alloc);
Transition* reverse_list(Transition*);

} // namespace unitex

#endif
