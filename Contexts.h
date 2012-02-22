/*
 * Unitex
 *
 * Copyright (C) 2001-2012 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef ContextsH
#define ContextsH

#include "Fst2.h"
#include "Transitions.h"
#include "AbstractAllocator.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * This structure stores the information needed to deal with 
 * the context marks $[ $![ and $]
 * If fst2 state has a transition with a positive context start mark $[, we compute
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


struct list_context {
	int n;
	unichar* output;
	struct list_context* next;
};

void get_reachable_closing_context_marks(Fst2*,int,Transition**,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
struct opt_contexts* new_opt_contexts(Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
void free_opt_contexts(struct opt_contexts*,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
void add_positive_context(Fst2*,struct opt_contexts**,Transition*,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
void add_negative_context(Fst2*,struct opt_contexts**,Transition*,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
struct opt_contexts** compute_contexts(Fst2*,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);

struct list_context* new_list_context(int n,struct list_context* next);
void free_list_context(struct list_context* l);

} // namespace unitex

#endif

