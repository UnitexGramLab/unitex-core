/*
 * Unitex
 *
 * Copyright (C) 2001-2015 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * This structure represents a stack of unicode characters.
 */
struct stack_unichar {
   unichar* stack;
   /* Pointer on the top element, -1 if the stack is empty */
   int stack_pointer;
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
unichar pop(struct stack_unichar*);


static inline void resize(struct stack_unichar* stack,int minimum_new_size) {
int new_size=stack->capacity;
if (new_size==0) new_size=1;
while (new_size<minimum_new_size) {
	new_size=new_size*2;
}
if (new_size==stack->capacity) return;
stack->stack=(unichar*)realloc(stack->stack,new_size*sizeof(unichar));
if (stack->stack==NULL) {
	fatal_alloc_error("resize");
}
stack->capacity=new_size;
}

static inline void push(struct stack_unichar* stack,unichar c) {
if (stack==NULL) {
   fatal_error_NULL_push();
}
if (stack->stack_pointer==stack->capacity-1) {
	resize(stack,stack->capacity+1);
}
stack->stack[++(stack->stack_pointer)]=c;
}

} // namespace unitex

#endif
