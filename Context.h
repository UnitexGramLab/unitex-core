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

#ifndef ContextH
#define ContextH

#include "TransductionStack.h"
#include "unicode.h"
#include "Liste_num.h"


/* Indicates that an opening positive context mark "$[" has
 * been found and that we look for the closing mark. */
#define INSIDE_POSITIVE_CONTEXT 0

/* Indicates that an opening negative context mark "$![" has
 * been found and that we look for the closing mark. */
#define INSIDE_NEGATIVE_CONTEXT 1

/* Indicates that an opening negative context mark "$![" has
 * been found and that there was a match failure, which is in fact
 * a success as we were in a negative context. */
#define FAILED_IN_NEGATIVE_CONTEXT 2

/* Indicates that an opening negative context mark "$![" has
 * been found and that we have reached the closing mark "$]",
 * which is a failure case as we were in a negative context. */
#define NEGATIVE_CONTEXT_HAS_MATCHED 3


/*
 * This structure is used to represent a context list being computed during
 * the locate pattern operation.
 */
struct context {
	/*
	 * This field indicates the current status of the context computation.
	 */
	unsigned char context_mode;
	/* 
	 * Position in the text where the locate must continue if the context
	 * was matched correctly (or not matched in the case of a negative one).
	 */
	int continue_position;
	/*
	 * Copy of the locate stack before the beginning of the context match.
	 */
	unichar stack[STACK_SIZE];
	/*
	 * Copy of the locate stack pointer before the beginning of the context match.
	 */
	int stack_pointer;
	/*
	 * Copy of the match list before the beginning of the context match.
	 */
	struct liste_num** list_of_matches;
	/*
	 * Copy of the number of matches before the beginning of the context match.
	 */
	int number_of_matches;
	/*
	 * 'depth' represents the level of context nesting. 0 means
	 * that the context is at top level.
	 */
	int depth;
	/*
	 * Next context of the list.
	 */
	struct context* next;
};


struct context* new_context(unsigned char,int,unichar*,int,struct liste_num**,int,struct context*);
struct context* remove_context(struct context*);
void free_context_list(struct context*);


#endif

