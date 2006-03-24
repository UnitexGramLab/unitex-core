 /*
  * Unitex
  *
  * Copyright (C) 2001-2004 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef _STACK_H_
#define _STACK_H_

#include "const.h"


#define STACK_DEFAULT_SIZE  64


typedef struct stack_type {

  void ** data;
  int head;
  int size;
} stack_type;



static inline stack_type * stack_new(int size = STACK_DEFAULT_SIZE) {

  if (size == 0) { size = 1; }

  stack_type * stack = (stack_type *) malloc(sizeof(stack_type));
  stack->size = size;
  stack->data = (void **) malloc(stack->size * sizeof(void *));
  stack->head = 0;

  return stack;
}

static inline void stack_delete(stack_type * stack) { free(stack->data); free(stack); }

static inline void stack_clear(stack_type * stack) { stack->head = 0; }
static inline int stack_empty(stack_type * stack) { return (stack->head == 0); }

static inline void stack_push(stack_type * stack, void * e) {

  if (stack->head == stack->size) {
    stack->size = 2 * stack->size;
    stack->data = (void **) realloc(stack->data, stack->size * sizeof(void *));
  }

  stack->data[stack->head++] = e;
}


static inline void * stack_head(stack_type * stack) {
  if (stack->head == 0) { return (void *) UNDEF; }
  return stack->data[stack->head - 1];
}


static inline void * stack_pop(stack_type * stack) {
  if (stack->head == 0) { return (void *) UNDEF; }
  return stack->data[--stack->head];
}


#endif
