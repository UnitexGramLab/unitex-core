/*
 * Unitex
 *
 * Copyright (C) 2001-2020 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "Stack_pointer.h"
#include "Error.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * Allocates, initializes and returns a stack of the given size.
 */
struct stack_pointer* new_stack_pointer(int capacity) {
struct stack_pointer* s=(struct stack_pointer*)malloc(sizeof(struct stack_pointer));
if (s==NULL) {
   fatal_alloc_error("new_stack_pointer");
}
s->stack=(void**)malloc(capacity*sizeof(void*));
if (s->stack==NULL) {
   fatal_alloc_error("new_stack_pointer");
}
s->stack_pointer_m=-1;
s->capacity=capacity;
return s;
}


/**
 * Frees the whole memory associated to the given stack.
 */
void free_stack_pointer(struct stack_pointer* stack) {
if (stack==NULL) return;
if (stack->stack!=NULL) free(stack->stack);
free(stack);
}


/**
 * Returns a non zero value if the stack is empty; 0 otherwise.
 */
int is_empty(const struct stack_pointer* stack) {
if (stack==NULL) {
   fatal_error("NULL error in is_empty\n");
}
return (stack->stack_pointer_m==-1);
}


/**
 * Empties the given stack.
 */
void empty(struct stack_pointer* stack) {
if (stack==NULL) {
   fatal_error("NULL error in empty\n");
}
stack->stack_pointer_m=-1;
}


/**
 * Returns a non zero value if the stack is full; 0 otherwise.
 */
int is_full(const struct stack_pointer* stack) {
if (stack==NULL) {
   fatal_error("NULL error in is_full\n");
}
return (stack->stack_pointer_m==stack->capacity-1);
}


/**
 * Pushes the given pointer on the given stack.
 */
void push(struct stack_pointer* stack,void* ptr) {
if (is_full(stack)) {
   fatal_error("Cannot push on a full stack\n");
}
stack->stack[++(stack->stack_pointer_m)]=ptr;
}


/**
 * Pops and returns the top element of the stack.
 */
void* pop(struct stack_pointer* stack) {
if (is_empty(stack)) {
   fatal_error("Cannot pop an empty stack\n");
}
return stack->stack[(stack->stack_pointer_m)--];
}

} // namespace unitex
