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

#include <stdlib.h>
#include "Stack_int.h"
#include "Error.h"


/**
 * Allocates, initializes and returns a stack of the given size.
 */
struct stack_int* new_stack_int(int capacity) {
struct stack_int* s=(struct stack_int*)malloc(sizeof(struct stack_int));
if (s==NULL) {
   fatal_alloc_error("new_stack_int");
}
s->stack=(int*)malloc(capacity*sizeof(int));
if (s->stack==NULL) {
   fatal_alloc_error("new_stack_int");
}
s->stack_pointer=-1;
s->capacity=capacity;
return s;
}


/**
 * Frees the whole memory associated to the given stack.
 */
void free_stack_int(struct stack_int* stack) {
if (stack==NULL) return;
if (stack->stack!=NULL) free(stack->stack);
free(stack);
}


/**
 * Returns a non zero value if the stack is empty; 0 otherwise.
 */
int stacki_is_empty(struct stack_int* stack) {
if (stack==NULL) {
   fatal_error("NULL error in is_empty\n");
}
return (stack->stack_pointer==-1);
}


/**
 * Empties the given stack.
 */
void stacki_empty(struct stack_int* stack) {
if (stack==NULL) {
   fatal_error("NULL error in stacki_empty\n");
}
stack->stack_pointer=-1;
}


/**
 * Returns a non zero value if the stack is full; 0 otherwise.
 */
int stacki_is_full(struct stack_int* stack) {
if (stack==NULL) {
   fatal_error("NULL error in is_full\n");
}
return (stack->stack_pointer==stack->capacity-1);
}


/**
 * Pushes the given character on the given stack.
 */
void stacki_push(struct stack_int* stack,int n) {
if (stacki_is_full(stack)) {
   fatal_error("Cannot push on a full stack\n");
}
stack->stack[++(stack->stack_pointer)]=n;
}


/**
 * Pops and returns the top element of the stack.
 */
int stacki_pop(struct stack_int* stack) {
if (stacki_is_empty(stack)) {
   fatal_error("Cannot pop an empty stack\n");
}
return stack->stack[(stack->stack_pointer)--];
}

