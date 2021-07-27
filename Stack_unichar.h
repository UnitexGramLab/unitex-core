/*
 * Unitex
 *
 * Copyright (C) 2001-2021 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef Stack_unicharH
#define Stack_unicharH

#include <stdio.h>
#include "Unicode.h"
#include "base/integer/operation/round.h"


#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

#define STACK_MIN_CAPACITY      16

/**
 * calculate the stack size for a given length
 */
#define stack_capacity_for_size(n) next_greater_power_of_two_32(n, STACK_MIN_CAPACITY)

/**
* calculate the rounded stack size for a given length
*/
#define stack_rounded_capacity_for_size(n) next_smallest_power_of_two_32(n, STACK_MIN_CAPACITY)


/**
 * This structure represents a stack of unicode characters.
 */
struct stack_unichar {
   unichar* buffer;
   /* Pointer on the top element, -1 if the stack is empty */
   int top;
   /* Maximum capacity of the stack */
   int capacity;
};


struct stack_unichar* new_stack_unichar(int);
void free_stack_unichar(struct stack_unichar*);
int is_empty(const struct stack_unichar*);
void empty(struct stack_unichar* stack);
int is_full(const struct stack_unichar*);
void fatal_error_NULL_push();
void fatal_error_full_stack_push();
void push_array(struct stack_unichar*,const unichar*,unsigned int);
void push_array(struct stack_unichar*, const char*, unsigned int);
void push_stack(struct stack_unichar*,const struct stack_unichar*, unsigned int size);
unichar pop(struct stack_unichar*);

/**
 * Resizes the internal buffer of the given stack_unichar to the given minimum size.
 * The buffer size is never decreased. Note that you cannot set a size<1.
 */
static inline void resize(struct stack_unichar* stack, int size) {
  if (size < 1) {
    size = STACK_MIN_CAPACITY;
  }

  if (size <= stack->capacity) {
    return;
  }

  // minor capacity enlarging
  unsigned int stack_capacity = stack_rounded_capacity_for_size(size);

  stack->buffer = (unichar*) realloc(stack->buffer, stack_capacity * sizeof(unichar));

  if (stack->buffer == NULL) {
    fatal_alloc_error("resize stack_unichar");
  }

  stack->capacity = stack_capacity;
}

static inline void push(struct stack_unichar* stack, unichar c) {
  if (stack == NULL) {
    fatal_error_NULL_push();
  }
  if (stack->top == stack->capacity - 1) {
    resize(stack, stack->capacity + 1);
  }
  stack->buffer[++(stack->top)] = c;
}

} // namespace unitex

#endif
