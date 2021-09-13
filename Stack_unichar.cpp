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

#include <stdlib.h>
#include "Stack_unichar.h"
#include "Error.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * Allocates, initializes and returns a stack of the given size.
 */
struct stack_unichar* new_stack_unichar(int capacity) {
struct stack_unichar* s=(struct stack_unichar*)malloc(sizeof(struct stack_unichar));
if (s==NULL) {
   fatal_alloc_error("new_stack_unichar");
}
s->buffer=(unichar*)malloc(capacity*sizeof(unichar));
if (s->buffer==NULL) {
   fatal_alloc_error("new_stack_unichar");
}
s->top=-1;
s->capacity=capacity;
return s;
}


/**
 * Frees the whole memory associated to the given stack.
 */
void free_stack_unichar(struct stack_unichar* stack) {
if (stack==NULL) return;
if (stack->buffer!=NULL) free(stack->buffer);
free(stack);
}


/**
 * Returns a non zero value if the stack is empty; 0 otherwise.
 */
int is_empty(const struct stack_unichar* stack) {
if (stack==NULL) {
   fatal_error("NULL error in is_empty\n");
}
return (stack->top==-1);
}


/**
 * Empties the given stack.
 */
void empty(struct stack_unichar* stack) {
if (stack==NULL) {
   fatal_error("NULL error in empty\n");
}
stack->top=-1;
}


/**
 * Returns a non zero value if the stack is full; 0 otherwise.
 */
int is_full(const struct stack_unichar* stack) {
if (stack==NULL) {
   fatal_error("NULL error in is_full\n");
}
return (stack->top==stack->capacity-1);
}


/**
 * Display NULL in push fatal error
 */
void fatal_error_NULL_push()
{
   fatal_error("NULL error in push\n");
}


/**
 * Display full stack fatal error
 */
void fatal_error_full_stack_push()
{
   fatal_error("Cannot push on a full stack\n");
}


/**
 * Pushes the given source stack on the given destination stack.
 */
void push_stack(struct stack_unichar* dst,const struct stack_unichar* src, unsigned int size) {
  if (dst == NULL || src == NULL) {
    fatal_error("NULL error in push_stack\n");
  }

  // if the source stack is empty there is nothing to do
  if (is_empty(src)) {
    return;
  }

  // avoid to push more characters than the capacity of the given source stack
  if (src->capacity < (int) size ) {
    size = (unsigned int) src->capacity;
  }

  // if necessary, we enlarge the internal buffer
  if ((dst->top + (int) size + 1) >= dst->capacity) {
    resize(dst, dst->top + (int) size + 1);
  }

  // push the source stack into the destination one
  for (unsigned int i = 0; i < size; ++i) {
    dst->buffer[++(dst->top)] = *(src->buffer + i);
  }
}


/**
 * Pushes the given characters from an array on the given stack.
 */
void push_array(struct stack_unichar* stack,const unichar *array,unsigned int size) {
if (stack==NULL) {
   fatal_error("NULL error in push_array\n");
}
if ((stack->top + (int)size + 1) >= stack->capacity) {
   resize(stack,stack->top + (int)size + 1);
}

unsigned int i;
for (i=0;i<size;i++) {
   stack->buffer[++(stack->top)]=*(array+i);
}
}

void push_array(struct stack_unichar* stack, const char* array, unsigned int size) {
  if (stack==NULL) {
    fatal_error("NULL error in push_array\n");
  }
  if ((stack->top + (int)size + 1) >= stack->capacity) {
    resize(stack,stack->top + (int)size + 1);
  }
  for (unsigned int i=0; i<size; ++i) {
    stack->buffer[++(stack->top)] = (unichar) *(array+i);
  }
}


/**
 * Pops and returns the top element of the stack.
 */
unichar pop(struct stack_unichar* stack) {
if (is_empty(stack)) {
   fatal_error("Cannot pop an empty stack\n");
}
return stack->buffer[(stack->top)--];
}

} // namespace unitex
