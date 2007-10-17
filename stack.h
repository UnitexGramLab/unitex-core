 /*
  * Unitex
  *
  * Copyright (C) 2001-2004 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include <stdio.h>
#include <stdlib.h>
#include "Error.h"

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

/*
static inline void * stack_head(stack_type * stack) {
  if (stack->head == 0) {
    return (void *) UNDEF; 
  }
  return stack->data[stack->head - 1];
}*/


static inline void * stack_pop(stack_type * stack) {
if (stack->head==0) {
   fatal_error("Cannot pop an empty stack\n");
}
return stack->data[--stack->head];
}



//////////////////////////////////////////////////////////////////////////
//
// WARNING!!!
// All has been temporarily duplicated because of a gcc cast bug under AMD
// 64 that makes impossible to cast a void* to an int.
//

typedef struct stack_type_int {

  int* data;
  int head;
  int size;
} stack_type_int;



static inline stack_type_int * stack_new_int(int size = STACK_DEFAULT_SIZE) {

  if (size == 0) { size = 1; }

  stack_type_int * stack = (stack_type_int *) malloc(sizeof(stack_type_int));
  stack->size = size;
  stack->data = (int*) malloc(stack->size * sizeof(int));
  stack->head = 0;

  return stack;
}

static inline void stack_delete_int(stack_type_int * stack) { free(stack->data); free(stack); }

static inline void stack_clear_int(stack_type_int * stack) { stack->head = 0; }
static inline int stack_empty_int(stack_type_int * stack) { return (stack->head == 0); }

static inline void stack_push_int(stack_type_int * stack, int e) {

  if (stack->head == stack->size) {
    stack->size = 2 * stack->size;
    stack->data = (int*) realloc(stack->data, stack->size * sizeof(int));
  }

  stack->data[stack->head++] = e;
}


static inline int stack_head_int(stack_type_int * stack) {
  if (stack->head == 0) {
     fatal_error("Empty stack error in stack_head_int\n");
  }
  return stack->data[stack->head - 1];
}


static inline int stack_pop_int(stack_type_int * stack) {
  if (stack->head == 0) {
     fatal_error("Empty stack error in stack_head_int\n");
  }
  return stack->data[--stack->head];
}


#endif
